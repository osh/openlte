#line 2 "LTE_fdd_enb_rb.cc" // Make __FILE__ omit the path
/*******************************************************************************

    Copyright 2014 Ben Wojtowicz

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*******************************************************************************

    File: LTE_fdd_enb_rb.cc

    Description: Contains all the implementations for the LTE FDD eNodeB
                 radio bearer class.

    Revision History
    ----------    -------------    --------------------------------------------
    05/04/2014    Ben Wojtowicz    Created file
    06/15/2014    Ben Wojtowicz    Added more states and procedures, QoS, MME,
                                   RLC, and uplink scheduling functionality.

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_enb_rb.h"
#include "LTE_fdd_enb_timer_mgr.h"
#include "LTE_fdd_enb_user.h"
#include "LTE_fdd_enb_mac.h"

/*******************************************************************************
                              DEFINES
*******************************************************************************/


/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/


/*******************************************************************************
                              GLOBAL VARIABLES
*******************************************************************************/


/*******************************************************************************
                              CLASS IMPLEMENTATIONS
*******************************************************************************/

/********************************/
/*    Constructor/Destructor    */
/********************************/
LTE_fdd_enb_rb::LTE_fdd_enb_rb(LTE_FDD_ENB_RB_ENUM  _rb,
                               LTE_fdd_enb_user    *_user)
{
    LTE_fdd_enb_timer_mgr *timer_mgr = LTE_fdd_enb_timer_mgr::get_instance();

    rb   = _rb;
    user = _user;

    timer_mgr->stop_timer(ul_sched_timer_id);

    if(LTE_FDD_ENB_RB_SRB0 == rb)
    {
        rrc_procedure = LTE_FDD_ENB_RRC_PROC_IDLE;
        rrc_state     = LTE_FDD_ENB_RRC_STATE_IDLE;
        pdcp_config   = LTE_FDD_ENB_PDCP_CONFIG_N_A;
        rlc_config    = LTE_FDD_ENB_RLC_CONFIG_TM;
        mac_config    = LTE_FDD_ENB_MAC_CONFIG_TM;
    }else if(LTE_FDD_ENB_RB_SRB1 == rb){
        rrc_procedure = LTE_FDD_ENB_RRC_PROC_IDLE;
        rrc_state     = LTE_FDD_ENB_RRC_STATE_IDLE;
        pdcp_config   = LTE_FDD_ENB_PDCP_CONFIG_N_A;
        rlc_config    = LTE_FDD_ENB_RLC_CONFIG_AM;
        mac_config    = LTE_FDD_ENB_MAC_CONFIG_TM;
    }

    // RLC
    rlc_reception_buffer.clear();
    rlc_vrr              = 0;
    rlc_vrmr             = rlc_vrr + LIBLTE_RLC_AM_WINDOW_SIZE;
    rlc_vrh              = 0;
    rlc_first_segment_sn = 0xFFFF;
    rlc_last_segment_sn  = 0xFFFF;

    // Setup the QoS
    avail_qos[0] = (LTE_FDD_ENB_QOS_STRUCT){ 0,   0};
    avail_qos[1] = (LTE_FDD_ENB_QOS_STRUCT){20, 176};
    qos          = LTE_FDD_ENB_QOS_NONE;
}
LTE_fdd_enb_rb::~LTE_fdd_enb_rb()
{
}

/******************/
/*    Identity    */
/******************/
LTE_FDD_ENB_RB_ENUM LTE_fdd_enb_rb::get_rb_id(void)
{
    return(rb);
}

/*************/
/*    MME    */
/*************/
void LTE_fdd_enb_rb::queue_mme_nas_msg(LIBLTE_BYTE_MSG_STRUCT *nas_msg)
{
    queue_msg(nas_msg, &mme_nas_msg_queue_mutex, &mme_nas_msg_queue);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_rb::get_next_mme_nas_msg(LIBLTE_BYTE_MSG_STRUCT **nas_msg)
{
    return(get_next_msg(&mme_nas_msg_queue_mutex, &mme_nas_msg_queue, nas_msg));
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_rb::delete_next_mme_nas_msg(void)
{
    return(delete_next_msg(&mme_nas_msg_queue_mutex, &mme_nas_msg_queue));
}

/*************/
/*    RRC    */
/*************/
void LTE_fdd_enb_rb::queue_rrc_pdu(LIBLTE_BIT_MSG_STRUCT *pdu)
{
    queue_msg(pdu, &rrc_pdu_queue_mutex, &rrc_pdu_queue);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_rb::get_next_rrc_pdu(LIBLTE_BIT_MSG_STRUCT **pdu)
{
    return(get_next_msg(&rrc_pdu_queue_mutex, &rrc_pdu_queue, pdu));
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_rb::delete_next_rrc_pdu(void)
{
    return(delete_next_msg(&rrc_pdu_queue_mutex, &rrc_pdu_queue));
}
void LTE_fdd_enb_rb::set_rrc_procedure(LTE_FDD_ENB_RRC_PROC_ENUM procedure)
{
    LTE_fdd_enb_interface *interface = LTE_fdd_enb_interface::get_instance();

    interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                              LTE_FDD_ENB_DEBUG_LEVEL_RB,
                              __FILE__,
                              __LINE__,
                              "%s RRC procedure moving from %s to %s for RNTI=%u",
                              LTE_fdd_enb_rb_text[rb],
                              LTE_fdd_enb_rrc_proc_text[rrc_procedure],
                              LTE_fdd_enb_rrc_proc_text[procedure],
                              user->get_c_rnti());
    rrc_procedure = procedure;
}
LTE_FDD_ENB_RRC_PROC_ENUM LTE_fdd_enb_rb::get_rrc_procedure(void)
{
    return(rrc_procedure);
}
void LTE_fdd_enb_rb::set_rrc_state(LTE_FDD_ENB_RRC_STATE_ENUM state)
{
    LTE_fdd_enb_interface *interface = LTE_fdd_enb_interface::get_instance();

    interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                              LTE_FDD_ENB_DEBUG_LEVEL_RB,
                              __FILE__,
                              __LINE__,
                              "%s RRC state moving from %s to %s for RNTI=%u",
                              LTE_fdd_enb_rb_text[rb],
                              LTE_fdd_enb_rrc_state_text[rrc_state],
                              LTE_fdd_enb_rrc_state_text[state],
                              user->get_c_rnti());
    rrc_state = state;
}
LTE_FDD_ENB_RRC_STATE_ENUM LTE_fdd_enb_rb::get_rrc_state(void)
{
    return(rrc_state);
}

/**************/
/*    PDCP    */
/**************/
void LTE_fdd_enb_rb::queue_pdcp_pdu(LIBLTE_BIT_MSG_STRUCT *pdu)
{
    queue_msg(pdu, &pdcp_pdu_queue_mutex, &pdcp_pdu_queue);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_rb::get_next_pdcp_pdu(LIBLTE_BIT_MSG_STRUCT **pdu)
{
    return(get_next_msg(&pdcp_pdu_queue_mutex, &pdcp_pdu_queue, pdu));
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_rb::delete_next_pdcp_pdu(void)
{
    return(delete_next_msg(&pdcp_pdu_queue_mutex, &pdcp_pdu_queue));
}
void LTE_fdd_enb_rb::queue_pdcp_sdu(LIBLTE_BIT_MSG_STRUCT *sdu)
{
    queue_msg(sdu, &pdcp_sdu_queue_mutex, &pdcp_sdu_queue);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_rb::get_next_pdcp_sdu(LIBLTE_BIT_MSG_STRUCT **sdu)
{
    return(get_next_msg(&pdcp_sdu_queue_mutex, &pdcp_sdu_queue, sdu));
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_rb::delete_next_pdcp_sdu(void)
{
    return(delete_next_msg(&pdcp_sdu_queue_mutex, &pdcp_sdu_queue));
}
LTE_FDD_ENB_PDCP_CONFIG_ENUM LTE_fdd_enb_rb::get_pdcp_config(void)
{
    return(pdcp_config);
}

/*************/
/*    RLC    */
/*************/
void LTE_fdd_enb_rb::queue_rlc_pdu(LIBLTE_BIT_MSG_STRUCT *pdu)
{
    queue_msg(pdu, &rlc_pdu_queue_mutex, &rlc_pdu_queue);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_rb::get_next_rlc_pdu(LIBLTE_BIT_MSG_STRUCT **pdu)
{
    return(get_next_msg(&rlc_pdu_queue_mutex, &rlc_pdu_queue, pdu));
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_rb::delete_next_rlc_pdu(void)
{
    return(delete_next_msg(&rlc_pdu_queue_mutex, &rlc_pdu_queue));
}
void LTE_fdd_enb_rb::queue_rlc_sdu(LIBLTE_BIT_MSG_STRUCT *sdu)
{
    queue_msg(sdu, &rlc_sdu_queue_mutex, &rlc_sdu_queue);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_rb::get_next_rlc_sdu(LIBLTE_BIT_MSG_STRUCT **sdu)
{
    return(get_next_msg(&rlc_sdu_queue_mutex, &rlc_sdu_queue, sdu));
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_rb::delete_next_rlc_sdu(void)
{
    return(delete_next_msg(&rlc_sdu_queue_mutex, &rlc_sdu_queue));
}
LTE_FDD_ENB_RLC_CONFIG_ENUM LTE_fdd_enb_rb::get_rlc_config(void)
{
    return(rlc_config);
}
uint16 LTE_fdd_enb_rb::get_rlc_vrr(void)
{
    return(rlc_vrr);
}
void LTE_fdd_enb_rb::set_rlc_vrr(uint16 vrr)
{
    rlc_vrr  = vrr;
    rlc_vrmr = rlc_vrr + LIBLTE_RLC_AM_WINDOW_SIZE;
}
uint16 LTE_fdd_enb_rb::get_rlc_vrmr(void)
{
    return(rlc_vrmr);
}
uint16 LTE_fdd_enb_rb::get_rlc_vrh(void)
{
    return(rlc_vrh);
}
void LTE_fdd_enb_rb::set_rlc_vrh(uint16 vrh)
{
    rlc_vrh = vrh;
}
void LTE_fdd_enb_rb::rlc_add_to_reception_buffer(LIBLTE_RLC_AMD_PDU_STRUCT *amd_pdu)
{
    LIBLTE_BIT_MSG_STRUCT *new_pdu = NULL;

    new_pdu = new LIBLTE_BIT_MSG_STRUCT;

    if(NULL != new_pdu)
    {
        memcpy(new_pdu, &amd_pdu->data, sizeof(LIBLTE_BIT_MSG_STRUCT));
        rlc_reception_buffer[amd_pdu->hdr.sn] = new_pdu;

        if(LIBLTE_RLC_FI_FIELD_FULL_SDU == amd_pdu->hdr.fi)
        {
            rlc_first_segment_sn = amd_pdu->hdr.sn;
            rlc_last_segment_sn  = amd_pdu->hdr.sn;
        }else if(LIBLTE_RLC_FI_FIELD_FIRST_SDU_SEGMENT == amd_pdu->hdr.fi){
            rlc_first_segment_sn = amd_pdu->hdr.sn;
        }else if(LIBLTE_RLC_FI_FIELD_LAST_SDU_SEGMENT == amd_pdu->hdr.fi){
            rlc_last_segment_sn = amd_pdu->hdr.sn;
        }
    }
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_rb::rlc_reassemble(LIBLTE_BIT_MSG_STRUCT *sdu)
{
    std::map<uint16, LIBLTE_BIT_MSG_STRUCT *>::iterator iter;
    LTE_FDD_ENB_ERROR_ENUM                              err = LTE_FDD_ENB_ERROR_CANT_REASSEMBLE_SDU;
    uint32                                              i;

    if(0xFFFF != rlc_first_segment_sn &&
       0xFFFF != rlc_last_segment_sn)
    {
        // Reorder and reassemble the SDU
        for(i=rlc_first_segment_sn; i<=rlc_last_segment_sn; i++)
        {
            // FIXME: Error handling
            iter = rlc_reception_buffer.find(i);
            memcpy(&sdu->msg[sdu->N_bits], (*iter).second->msg, (*iter).second->N_bits);
            sdu->N_bits += (*iter).second->N_bits;
            delete (*iter).second;
            rlc_reception_buffer.erase(iter);
        }

        // Clear the first/last segment SNs
        rlc_first_segment_sn = 0xFFFF;
        rlc_last_segment_sn  = 0xFFFF;

        err = LTE_FDD_ENB_ERROR_NONE;
    }

    return(err);
}

/*************/
/*    MAC    */
/*************/
void LTE_fdd_enb_rb::queue_mac_sdu(LIBLTE_BIT_MSG_STRUCT *sdu)
{
    queue_msg(sdu, &mac_sdu_queue_mutex, &mac_sdu_queue);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_rb::get_next_mac_sdu(LIBLTE_BIT_MSG_STRUCT **sdu)
{
    return(get_next_msg(&mac_sdu_queue_mutex, &mac_sdu_queue, sdu));
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_rb::delete_next_mac_sdu(void)
{
    return(delete_next_msg(&mac_sdu_queue_mutex, &mac_sdu_queue));
}
LTE_FDD_ENB_MAC_CONFIG_ENUM LTE_fdd_enb_rb::get_mac_config(void)
{
    return(mac_config);
}
void LTE_fdd_enb_rb::start_ul_sched_timer(uint32 m_seconds)
{
    LTE_fdd_enb_timer_mgr *timer_mgr = LTE_fdd_enb_timer_mgr::get_instance();
    LTE_fdd_enb_timer_cb   timer_expiry_cb(&LTE_fdd_enb_timer_cb_wrapper<LTE_fdd_enb_rb, &LTE_fdd_enb_rb::handle_ul_sched_timer_expiry>, this);

    ul_sched_timer_m_seconds = m_seconds;
    timer_mgr->start_timer(ul_sched_timer_m_seconds, timer_expiry_cb, &ul_sched_timer_id);
}
void LTE_fdd_enb_rb::handle_ul_sched_timer_expiry(uint32 timer_id)
{
    LTE_fdd_enb_mac *mac = LTE_fdd_enb_mac::get_instance();

    mac->sched_ul(user, avail_qos[qos].bits_per_subfn);
    if(LTE_FDD_ENB_RRC_PROC_IDLE  != rrc_procedure &&
       LTE_FDD_ENB_RRC_STATE_IDLE != rrc_state)
    {
        start_ul_sched_timer(ul_sched_timer_m_seconds);
    }
}

/*****************/
/*    Generic    */
/*****************/
void LTE_fdd_enb_rb::queue_msg(LIBLTE_BIT_MSG_STRUCT              *msg,
                               boost::mutex                       *mutex,
                               std::list<LIBLTE_BIT_MSG_STRUCT *> *queue)
{
    boost::mutex::scoped_lock  lock(*mutex);
    LIBLTE_BIT_MSG_STRUCT     *loc_msg;

    loc_msg = new LIBLTE_BIT_MSG_STRUCT;
    memcpy(loc_msg, msg, sizeof(LIBLTE_BIT_MSG_STRUCT));

    queue->push_back(loc_msg);
}
void LTE_fdd_enb_rb::queue_msg(LIBLTE_BYTE_MSG_STRUCT              *msg,
                               boost::mutex                        *mutex,
                               std::list<LIBLTE_BYTE_MSG_STRUCT *> *queue)
{
    boost::mutex::scoped_lock  lock(*mutex);
    LIBLTE_BYTE_MSG_STRUCT    *loc_msg;

    loc_msg = new LIBLTE_BYTE_MSG_STRUCT;
    memcpy(loc_msg, msg, sizeof(LIBLTE_BYTE_MSG_STRUCT));

    queue->push_back(loc_msg);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_rb::get_next_msg(boost::mutex                        *mutex,
                                                    std::list<LIBLTE_BIT_MSG_STRUCT *>  *queue,
                                                    LIBLTE_BIT_MSG_STRUCT              **msg)
{
    boost::mutex::scoped_lock lock(*mutex);
    LTE_FDD_ENB_ERROR_ENUM    err = LTE_FDD_ENB_ERROR_NO_MSG_IN_QUEUE;

    if(0 != queue->size())
    {
        *msg = queue->front();
        err  = LTE_FDD_ENB_ERROR_NONE;
    }

    return(err);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_rb::get_next_msg(boost::mutex                         *mutex,
                                                    std::list<LIBLTE_BYTE_MSG_STRUCT *>  *queue,
                                                    LIBLTE_BYTE_MSG_STRUCT              **msg)
{
    boost::mutex::scoped_lock lock(*mutex);
    LTE_FDD_ENB_ERROR_ENUM    err = LTE_FDD_ENB_ERROR_NO_MSG_IN_QUEUE;

    if(0 != queue->size())
    {
        *msg = queue->front();
        err  = LTE_FDD_ENB_ERROR_NONE;
    }

    return(err);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_rb::delete_next_msg(boost::mutex                       *mutex,
                                                       std::list<LIBLTE_BIT_MSG_STRUCT *> *queue)
{
    boost::mutex::scoped_lock  lock(*mutex);
    LTE_FDD_ENB_ERROR_ENUM     err = LTE_FDD_ENB_ERROR_NO_MSG_IN_QUEUE;
    LIBLTE_BIT_MSG_STRUCT     *msg;

    if(0 != queue->size())
    {
        msg = queue->front();
        queue->pop_front();
        delete msg;
        err = LTE_FDD_ENB_ERROR_NONE;
    }

    return(err);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_rb::delete_next_msg(boost::mutex                        *mutex,
                                                       std::list<LIBLTE_BYTE_MSG_STRUCT *> *queue)
{
    boost::mutex::scoped_lock  lock(*mutex);
    LTE_FDD_ENB_ERROR_ENUM     err = LTE_FDD_ENB_ERROR_NO_MSG_IN_QUEUE;
    LIBLTE_BYTE_MSG_STRUCT    *msg;

    if(0 != queue->size())
    {
        msg = queue->front();
        queue->pop_front();
        delete msg;
        err = LTE_FDD_ENB_ERROR_NONE;
    }

    return(err);
}
void LTE_fdd_enb_rb::set_qos(LTE_FDD_ENB_QOS_ENUM _qos)
{
    qos = _qos;
    if(qos != LTE_FDD_ENB_QOS_NONE)
    {
        start_ul_sched_timer(avail_qos[qos].tti_frequency-1);
    }
}
LTE_FDD_ENB_QOS_ENUM LTE_fdd_enb_rb::get_qos(void)
{
    return(qos);
}
