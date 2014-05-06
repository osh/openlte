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

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_enb_rb.h"

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
LTE_fdd_enb_rb::LTE_fdd_enb_rb(LTE_FDD_ENB_RB_ENUM _rb)
{
    rb = _rb;

    if(LTE_FDD_ENB_RB_SRB0 == rb)
    {
        rrc_procedure = LTE_FDD_ENB_RRC_PROC_IDLE;
        rrc_state     = LTE_FDD_ENB_RRC_STATE_IDLE;
        pdcp_config   = LTE_FDD_ENB_PDCP_CONFIG_N_A;
        rlc_config    = LTE_FDD_ENB_RLC_CONFIG_TM;
        mac_config    = LTE_FDD_ENB_MAC_CONFIG_TM;
    }
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
/*    RRC    */
/*************/
void LTE_fdd_enb_rb::queue_rrc_pdu(LIBLTE_MSG_STRUCT *pdu)
{
    queue_msg(pdu, &rrc_pdu_queue_mutex, &rrc_pdu_queue);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_rb::get_next_rrc_pdu(LIBLTE_MSG_STRUCT **pdu)
{
    return(get_next_msg(&rrc_pdu_queue_mutex, &rrc_pdu_queue, pdu));
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_rb::delete_next_rrc_pdu(void)
{
    return(delete_next_msg(&rrc_pdu_queue_mutex, &rrc_pdu_queue));
}
void LTE_fdd_enb_rb::set_rrc_procedure(LTE_FDD_ENB_RRC_PROC_ENUM procedure,
                                       uint16                    rnti)
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
                              rnti);
    rrc_procedure = procedure;
}
LTE_FDD_ENB_RRC_PROC_ENUM LTE_fdd_enb_rb::get_rrc_procedure(void)
{
    return(rrc_procedure);
}
void LTE_fdd_enb_rb::set_rrc_state(LTE_FDD_ENB_RRC_STATE_ENUM state,
                                   uint16                     rnti)
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
                              rnti);
    rrc_state = state;
}
LTE_FDD_ENB_RRC_STATE_ENUM LTE_fdd_enb_rb::get_rrc_state(void)
{
    return(rrc_state);
}

/**************/
/*    PDCP    */
/**************/
void LTE_fdd_enb_rb::queue_pdcp_pdu(LIBLTE_MSG_STRUCT *pdu)
{
    queue_msg(pdu, &pdcp_pdu_queue_mutex, &pdcp_pdu_queue);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_rb::get_next_pdcp_pdu(LIBLTE_MSG_STRUCT **pdu)
{
    return(get_next_msg(&pdcp_pdu_queue_mutex, &pdcp_pdu_queue, pdu));
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_rb::delete_next_pdcp_pdu(void)
{
    return(delete_next_msg(&pdcp_pdu_queue_mutex, &pdcp_pdu_queue));
}
void LTE_fdd_enb_rb::queue_pdcp_sdu(LIBLTE_MSG_STRUCT *sdu)
{
    queue_msg(sdu, &pdcp_sdu_queue_mutex, &pdcp_sdu_queue);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_rb::get_next_pdcp_sdu(LIBLTE_MSG_STRUCT **sdu)
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
void LTE_fdd_enb_rb::queue_rlc_pdu(LIBLTE_MSG_STRUCT *pdu)
{
    queue_msg(pdu, &rlc_pdu_queue_mutex, &rlc_pdu_queue);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_rb::get_next_rlc_pdu(LIBLTE_MSG_STRUCT **pdu)
{
    return(get_next_msg(&rlc_pdu_queue_mutex, &rlc_pdu_queue, pdu));
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_rb::delete_next_rlc_pdu(void)
{
    return(delete_next_msg(&rlc_pdu_queue_mutex, &rlc_pdu_queue));
}
void LTE_fdd_enb_rb::queue_rlc_sdu(LIBLTE_MSG_STRUCT *sdu)
{
    queue_msg(sdu, &rlc_sdu_queue_mutex, &rlc_sdu_queue);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_rb::get_next_rlc_sdu(LIBLTE_MSG_STRUCT **sdu)
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

/*************/
/*    MAC    */
/*************/
void LTE_fdd_enb_rb::queue_mac_sdu(LIBLTE_MSG_STRUCT *sdu)
{
    queue_msg(sdu, &mac_sdu_queue_mutex, &mac_sdu_queue);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_rb::get_next_mac_sdu(LIBLTE_MSG_STRUCT **sdu)
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

/*****************/
/*    Generic    */
/*****************/
void LTE_fdd_enb_rb::queue_msg(LIBLTE_MSG_STRUCT              *msg,
                               boost::mutex                   *mutex,
                               std::list<LIBLTE_MSG_STRUCT *> *queue)
{
    boost::mutex::scoped_lock  lock(*mutex);
    LIBLTE_MSG_STRUCT         *loc_msg;

    loc_msg = new LIBLTE_MSG_STRUCT;
    memcpy(loc_msg, msg, sizeof(LIBLTE_MSG_STRUCT));

    queue->push_back(loc_msg);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_rb::get_next_msg(boost::mutex                    *mutex,
                                                    std::list<LIBLTE_MSG_STRUCT *>  *queue,
                                                    LIBLTE_MSG_STRUCT              **msg)
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
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_rb::delete_next_msg(boost::mutex                   *mutex,
                                                       std::list<LIBLTE_MSG_STRUCT *> *queue)
{
    boost::mutex::scoped_lock  lock(*mutex);
    LTE_FDD_ENB_ERROR_ENUM     err = LTE_FDD_ENB_ERROR_NO_MSG_IN_QUEUE;
    LIBLTE_MSG_STRUCT         *msg;

    if(0 != queue->size())
    {
        msg = queue->front();
        queue->pop_front();
        delete msg;
        err = LTE_FDD_ENB_ERROR_NONE;
    }

    return(err);
}
