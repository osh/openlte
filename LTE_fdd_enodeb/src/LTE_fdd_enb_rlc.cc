/*******************************************************************************

    Copyright 2013-2014 Ben Wojtowicz

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

    File: LTE_fdd_enb_rlc.cc

    Description: Contains all the implementations for the LTE FDD eNodeB
                 radio link control layer.

    Revision History
    ----------    -------------    --------------------------------------------
    11/10/2013    Ben Wojtowicz    Created file
    01/18/2014    Ben Wojtowicz    Added level to debug prints.
    05/04/2014    Ben Wojtowicz    Added communication to MAC and PDCP.

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_enb_rlc.h"
#include "LTE_fdd_enb_interface.h"

/*******************************************************************************
                              DEFINES
*******************************************************************************/


/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/


/*******************************************************************************
                              GLOBAL VARIABLES
*******************************************************************************/

LTE_fdd_enb_rlc* LTE_fdd_enb_rlc::instance = NULL;
boost::mutex     rlc_instance_mutex;

/*******************************************************************************
                              CLASS IMPLEMENTATIONS
*******************************************************************************/

/*******************/
/*    Singleton    */
/*******************/
LTE_fdd_enb_rlc* LTE_fdd_enb_rlc::get_instance(void)
{
    boost::mutex::scoped_lock lock(rlc_instance_mutex);

    if(NULL == instance)
    {
        instance = new LTE_fdd_enb_rlc();
    }

    return(instance);
}
void LTE_fdd_enb_rlc::cleanup(void)
{
    boost::mutex::scoped_lock lock(rlc_instance_mutex);

    if(NULL != instance)
    {
        delete instance;
        instance = NULL;
    }
}

/********************************/
/*    Constructor/Destructor    */
/********************************/
LTE_fdd_enb_rlc::LTE_fdd_enb_rlc()
{
    started = false;
}
LTE_fdd_enb_rlc::~LTE_fdd_enb_rlc()
{
    stop();
}

/********************/
/*    Start/Stop    */
/********************/
void LTE_fdd_enb_rlc::start(void)
{
    boost::mutex::scoped_lock lock(start_mutex);
    msgq_cb                   mac_cb(&msgq_cb_wrapper<LTE_fdd_enb_rlc, &LTE_fdd_enb_rlc::handle_mac_msg>, this);
    msgq_cb                   pdcp_cb(&msgq_cb_wrapper<LTE_fdd_enb_rlc, &LTE_fdd_enb_rlc::handle_pdcp_msg>, this);

    if(!started)
    {
        started        = true;
        mac_comm_msgq  = new LTE_fdd_enb_msgq("mac_rlc_mq",
                                              mac_cb);
        pdcp_comm_msgq = new LTE_fdd_enb_msgq("pdcp_rlc_mq",
                                              pdcp_cb);
        rlc_mac_mq     = new boost::interprocess::message_queue(boost::interprocess::open_only,
                                                                "rlc_mac_mq");
        rlc_pdcp_mq    = new boost::interprocess::message_queue(boost::interprocess::open_only,
                                                                "rlc_pdcp_mq");
    }
}
void LTE_fdd_enb_rlc::stop(void)
{
    boost::mutex::scoped_lock lock(start_mutex);

    if(started)
    {
        started = false;
        delete mac_comm_msgq;
        delete pdcp_comm_msgq;
    }
}

/***********************/
/*    Communication    */
/***********************/
void LTE_fdd_enb_rlc::handle_mac_msg(LTE_FDD_ENB_MESSAGE_STRUCT *msg)
{
    LTE_fdd_enb_interface *interface = LTE_fdd_enb_interface::get_instance();

    if(LTE_FDD_ENB_DEST_LAYER_RLC == msg->dest_layer ||
       LTE_FDD_ENB_DEST_LAYER_ANY == msg->dest_layer)
    {
        switch(msg->type)
        {
        case LTE_FDD_ENB_MESSAGE_TYPE_RLC_PDU_READY:
            handle_pdu_ready(&msg->msg.rlc_pdu_ready);
            delete msg;
            break;
        default:
            interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                      LTE_FDD_ENB_DEBUG_LEVEL_RLC,
                                      __FILE__,
                                      __LINE__,
                                      "Received invalid MAC message %s",
                                      LTE_fdd_enb_message_type_text[msg->type]);
            delete msg;
            break;
        }
    }else{
        // Forward message to PDCP
        rlc_pdcp_mq->send(&msg, sizeof(msg), 0);
    }
}
void LTE_fdd_enb_rlc::handle_pdcp_msg(LTE_FDD_ENB_MESSAGE_STRUCT *msg)
{
    LTE_fdd_enb_interface *interface = LTE_fdd_enb_interface::get_instance();

    if(LTE_FDD_ENB_DEST_LAYER_RLC == msg->dest_layer ||
       LTE_FDD_ENB_DEST_LAYER_ANY == msg->dest_layer)
    {
        switch(msg->type)
        {
        case LTE_FDD_ENB_MESSAGE_TYPE_RLC_SDU_READY:
            handle_sdu_ready(&msg->msg.rlc_sdu_ready);
            delete msg;
            break;
        default:
            interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                      LTE_FDD_ENB_DEBUG_LEVEL_RLC,
                                      __FILE__,
                                      __LINE__,
                                      "Received invalid PDCP message %s",
                                      LTE_fdd_enb_message_type_text[msg->type]);
            delete msg;
            break;
        }
    }else{
        // Forward message to MAC
        rlc_mac_mq->send(&msg, sizeof(msg), 0);
    }
}

/****************************/
/*    External Interface    */
/****************************/
void LTE_fdd_enb_rlc::update_sys_info(void)
{
    LTE_fdd_enb_cnfg_db *cnfg_db = LTE_fdd_enb_cnfg_db::get_instance();

    sys_info_mutex.lock();
    cnfg_db->get_sys_info(sys_info);
    sys_info_mutex.unlock();
}

/******************************/
/*    MAC Message Handlers    */
/******************************/
void LTE_fdd_enb_rlc::handle_pdu_ready(LTE_FDD_ENB_RLC_PDU_READY_MSG_STRUCT *pdu_ready)
{
    LTE_fdd_enb_interface *interface = LTE_fdd_enb_interface::get_instance();
    LIBLTE_MSG_STRUCT     *pdu;

    if(LTE_FDD_ENB_ERROR_NONE == pdu_ready->rb->get_next_rlc_pdu(&pdu))
    {
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                  LTE_FDD_ENB_DEBUG_LEVEL_RLC,
                                  __FILE__,
                                  __LINE__,
                                  pdu,
                                  "Received PDU for RNTI=%u and RB=%s",
                                  pdu_ready->user->get_c_rnti(),
                                  LTE_fdd_enb_rb_text[pdu_ready->rb->get_rb_id()]);

        switch(pdu_ready->rb->get_rlc_config())
        {
        case LTE_FDD_ENB_RLC_CONFIG_TM:
            handle_tm_pdu(pdu, pdu_ready->user, pdu_ready->rb);
            break;
        case LTE_FDD_ENB_RLC_CONFIG_UM:
            handle_um_pdu(pdu, pdu_ready->user, pdu_ready->rb);
            break;
        case LTE_FDD_ENB_RLC_CONFIG_AM:
            handle_am_pdu(pdu, pdu_ready->user, pdu_ready->rb);
            break;
        default:
            interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                      LTE_FDD_ENB_DEBUG_LEVEL_RLC,
                                      __FILE__,
                                      __LINE__,
                                      pdu,
                                      "Received PDU for RNTI=%u, RB=%s, and invalid rlc_config=%s",
                                      pdu_ready->user->get_c_rnti(),
                                      LTE_fdd_enb_rb_text[pdu_ready->rb->get_rb_id()],
                                      LTE_fdd_enb_rlc_config_text[pdu_ready->rb->get_rlc_config()]);
            break;
        }

        // Delete the PDU
        pdu_ready->rb->delete_next_rlc_pdu();
    }else{
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                  LTE_FDD_ENB_DEBUG_LEVEL_RLC,
                                  __FILE__,
                                  __LINE__,
                                  "Received pdu_ready message with no PDU queued");
    }
}
void LTE_fdd_enb_rlc::handle_tm_pdu(LIBLTE_MSG_STRUCT *pdu,
                                    LTE_fdd_enb_user  *user,
                                    LTE_fdd_enb_rb    *rb)
{
    LTE_FDD_ENB_PDCP_PDU_READY_MSG_STRUCT pdcp_pdu_ready;

    // Queue the PDU for PDCP
    rb->queue_pdcp_pdu(pdu);

    // Signal PDCP
    pdcp_pdu_ready.user = user;
    pdcp_pdu_ready.rb   = rb;
    LTE_fdd_enb_msgq::send(rlc_pdcp_mq,
                           LTE_FDD_ENB_MESSAGE_TYPE_PDCP_PDU_READY,
                           LTE_FDD_ENB_DEST_LAYER_PDCP,
                           (LTE_FDD_ENB_MESSAGE_UNION *)&pdcp_pdu_ready,
                           sizeof(LTE_FDD_ENB_PDCP_PDU_READY_MSG_STRUCT));
}
void LTE_fdd_enb_rlc::handle_um_pdu(LIBLTE_MSG_STRUCT *pdu,
                                    LTE_fdd_enb_user  *user,
                                    LTE_fdd_enb_rb    *rb)
{
    LTE_fdd_enb_interface *interface = LTE_fdd_enb_interface::get_instance();

    interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                              LTE_FDD_ENB_DEBUG_LEVEL_RLC,
                              __FILE__,
                              __LINE__,
                              "Not handling PDUs for RLC UM config");
}
void LTE_fdd_enb_rlc::handle_am_pdu(LIBLTE_MSG_STRUCT *pdu,
                                    LTE_fdd_enb_user  *user,
                                    LTE_fdd_enb_rb    *rb)
{
    LTE_fdd_enb_interface *interface = LTE_fdd_enb_interface::get_instance();

    interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                              LTE_FDD_ENB_DEBUG_LEVEL_RLC,
                              __FILE__,
                              __LINE__,
                              "Not handling PDUs for RLC AM config");
}

/*******************************/
/*    PDCP Message Handlers    */
/*******************************/
void LTE_fdd_enb_rlc::handle_sdu_ready(LTE_FDD_ENB_RLC_SDU_READY_MSG_STRUCT *sdu_ready)
{
    LTE_fdd_enb_interface *interface = LTE_fdd_enb_interface::get_instance();
    LIBLTE_MSG_STRUCT     *sdu;

    if(LTE_FDD_ENB_ERROR_NONE == sdu_ready->rb->get_next_rlc_sdu(&sdu))
    {
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                  LTE_FDD_ENB_DEBUG_LEVEL_RLC,
                                  __FILE__,
                                  __LINE__,
                                  sdu,
                                  "Received SDU for RNTI=%u and RB=%s",
                                  sdu_ready->user->get_c_rnti(),
                                  LTE_fdd_enb_rb_text[sdu_ready->rb->get_rb_id()]);

        switch(sdu_ready->rb->get_rlc_config())
        {
        case LTE_FDD_ENB_RLC_CONFIG_TM:
            handle_tm_sdu(sdu, sdu_ready->user, sdu_ready->rb);
            break;
        case LTE_FDD_ENB_RLC_CONFIG_UM:
            handle_um_sdu(sdu, sdu_ready->user, sdu_ready->rb);
            break;
        case LTE_FDD_ENB_RLC_CONFIG_AM:
            handle_am_sdu(sdu, sdu_ready->user, sdu_ready->rb);
            break;
        default:
            interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                      LTE_FDD_ENB_DEBUG_LEVEL_RLC,
                                      __FILE__,
                                      __LINE__,
                                      "Received SDU for RNTI=%u, RB=%s, and invalid rlc_config=%s",
                                      sdu_ready->user->get_c_rnti(),
                                      LTE_fdd_enb_rb_text[sdu_ready->rb->get_rb_id()],
                                      LTE_fdd_enb_rlc_config_text[sdu_ready->rb->get_rlc_config()]);
            break;
        }

        // Delete the SDU
        sdu_ready->rb->delete_next_rlc_sdu();
    }else{
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                  LTE_FDD_ENB_DEBUG_LEVEL_RLC,
                                  __FILE__,
                                  __LINE__,
                                  "Received sdu_ready message with no SDU queued");
    }
}
void LTE_fdd_enb_rlc::handle_tm_sdu(LIBLTE_MSG_STRUCT *sdu,
                                    LTE_fdd_enb_user  *user,
                                    LTE_fdd_enb_rb    *rb)
{
    LTE_FDD_ENB_MAC_SDU_READY_MSG_STRUCT mac_sdu_ready;

    // Queue the SDU for MAC
    rb->queue_mac_sdu(sdu);

    // Signal MAC
    mac_sdu_ready.user = user;
    mac_sdu_ready.rb   = rb;
    LTE_fdd_enb_msgq::send(rlc_mac_mq,
                           LTE_FDD_ENB_MESSAGE_TYPE_MAC_SDU_READY,
                           LTE_FDD_ENB_DEST_LAYER_MAC,
                           (LTE_FDD_ENB_MESSAGE_UNION *)&mac_sdu_ready,
                           sizeof(LTE_FDD_ENB_MAC_SDU_READY_MSG_STRUCT));
}
void LTE_fdd_enb_rlc::handle_um_sdu(LIBLTE_MSG_STRUCT *sdu,
                                    LTE_fdd_enb_user  *user,
                                    LTE_fdd_enb_rb    *rb)
{
    LTE_fdd_enb_interface *interface = LTE_fdd_enb_interface::get_instance();

    interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                              LTE_FDD_ENB_DEBUG_LEVEL_RLC,
                              __FILE__,
                              __LINE__,
                              "Not handling SDUs for RLC UM config");
}
void LTE_fdd_enb_rlc::handle_am_sdu(LIBLTE_MSG_STRUCT *sdu,
                                    LTE_fdd_enb_user  *user,
                                    LTE_fdd_enb_rb    *rb)
{
    LTE_fdd_enb_interface *interface = LTE_fdd_enb_interface::get_instance();

    interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                              LTE_FDD_ENB_DEBUG_LEVEL_RLC,
                              __FILE__,
                              __LINE__,
                              "Not handling SDUs for RLC AM config");
}
