#line 2 "LTE_fdd_enb_pdcp.cc" // Make __FILE__ omit the path
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

    File: LTE_fdd_enb_pdcp.cc

    Description: Contains all the implementations for the LTE FDD eNodeB
                 packet data convergence protocol layer.

    Revision History
    ----------    -------------    --------------------------------------------
    11/10/2013    Ben Wojtowicz    Created file
    01/18/2014    Ben Wojtowicz    Added level to debug prints.
    05/04/2014    Ben Wojtowicz    Added communication to RLC and RRC.
    06/15/2014    Ben Wojtowicz    Added simple header parsing.

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_enb_pdcp.h"
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

LTE_fdd_enb_pdcp* LTE_fdd_enb_pdcp::instance = NULL;
boost::mutex      pdcp_instance_mutex;

/*******************************************************************************
                              CLASS IMPLEMENTATIONS
*******************************************************************************/

/*******************/
/*    Singleton    */
/*******************/
LTE_fdd_enb_pdcp* LTE_fdd_enb_pdcp::get_instance(void)
{
    boost::mutex::scoped_lock lock(pdcp_instance_mutex);

    if(NULL == instance)
    {
        instance = new LTE_fdd_enb_pdcp();
    }

    return(instance);
}
void LTE_fdd_enb_pdcp::cleanup(void)
{
    boost::mutex::scoped_lock lock(pdcp_instance_mutex);

    if(NULL != instance)
    {
        delete instance;
        instance = NULL;
    }
}

/********************************/
/*    Constructor/Destructor    */
/********************************/
LTE_fdd_enb_pdcp::LTE_fdd_enb_pdcp()
{
    started = false;
}
LTE_fdd_enb_pdcp::~LTE_fdd_enb_pdcp()
{
    stop();
}

/********************/
/*    Start/Stop    */
/********************/
void LTE_fdd_enb_pdcp::start(void)
{
    boost::mutex::scoped_lock lock(start_mutex);
    LTE_fdd_enb_msgq_cb       rlc_cb(&LTE_fdd_enb_msgq_cb_wrapper<LTE_fdd_enb_pdcp, &LTE_fdd_enb_pdcp::handle_rlc_msg>, this);
    LTE_fdd_enb_msgq_cb       rrc_cb(&LTE_fdd_enb_msgq_cb_wrapper<LTE_fdd_enb_pdcp, &LTE_fdd_enb_pdcp::handle_rrc_msg>, this);

    if(!started)
    {
        started       = true;
        rlc_comm_msgq = new LTE_fdd_enb_msgq("rlc_pdcp_mq",
                                             rlc_cb);
        rrc_comm_msgq = new LTE_fdd_enb_msgq("rrc_pdcp_mq",
                                             rrc_cb);
        pdcp_rlc_mq   = new boost::interprocess::message_queue(boost::interprocess::open_only,
                                                               "pdcp_rlc_mq");
        pdcp_rrc_mq   = new boost::interprocess::message_queue(boost::interprocess::open_only,
                                                               "pdcp_rrc_mq");
    }
}
void LTE_fdd_enb_pdcp::stop(void)
{
    boost::mutex::scoped_lock lock(start_mutex);

    if(started)
    {
        started = false;
        delete rlc_comm_msgq;
        delete rrc_comm_msgq;
    }
}

/***********************/
/*    Communication    */
/***********************/
void LTE_fdd_enb_pdcp::handle_rlc_msg(LTE_FDD_ENB_MESSAGE_STRUCT *msg)
{
    LTE_fdd_enb_interface *interface = LTE_fdd_enb_interface::get_instance();

    if(LTE_FDD_ENB_DEST_LAYER_PDCP == msg->dest_layer ||
       LTE_FDD_ENB_DEST_LAYER_ANY  == msg->dest_layer)
    {
        switch(msg->type)
        {
        case LTE_FDD_ENB_MESSAGE_TYPE_PDCP_PDU_READY:
            handle_pdu_ready(&msg->msg.pdcp_pdu_ready);
            delete msg;
            break;
        default:
            interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                      LTE_FDD_ENB_DEBUG_LEVEL_PDCP,
                                      __FILE__,
                                      __LINE__,
                                      "Received invalid RLC message %s",
                                      LTE_fdd_enb_message_type_text[msg->type]);
            delete msg;
            break;
        }
    }else{
        // Forward message to RRC
        pdcp_rrc_mq->send(&msg, sizeof(msg), 0);
    }
}
void LTE_fdd_enb_pdcp::handle_rrc_msg(LTE_FDD_ENB_MESSAGE_STRUCT *msg)
{
    LTE_fdd_enb_interface *interface = LTE_fdd_enb_interface::get_instance();

    if(LTE_FDD_ENB_DEST_LAYER_PDCP == msg->dest_layer ||
       LTE_FDD_ENB_DEST_LAYER_ANY  == msg->dest_layer)
    {
        switch(msg->type)
        {
        case LTE_FDD_ENB_MESSAGE_TYPE_PDCP_SDU_READY:
            handle_sdu_ready(&msg->msg.pdcp_sdu_ready);
            delete msg;
            break;
        default:
            interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                      LTE_FDD_ENB_DEBUG_LEVEL_PDCP,
                                      __FILE__,
                                      __LINE__,
                                      "Received invalid RRC message %s",
                                      LTE_fdd_enb_message_type_text[msg->type]);
            delete msg;
            break;
        }
    }else{
        // Forward message to RLC
        pdcp_rlc_mq->send(&msg, sizeof(msg), 0);
    }
}

/****************************/
/*    External Interface    */
/****************************/
void LTE_fdd_enb_pdcp::update_sys_info(void)
{
    LTE_fdd_enb_cnfg_db *cnfg_db = LTE_fdd_enb_cnfg_db::get_instance();

    sys_info_mutex.lock();
    cnfg_db->get_sys_info(sys_info);
    sys_info_mutex.unlock();
}

/******************************/
/*    RLC Message Handlers    */
/******************************/
void LTE_fdd_enb_pdcp::handle_pdu_ready(LTE_FDD_ENB_PDCP_PDU_READY_MSG_STRUCT *pdu_ready)
{
    LTE_fdd_enb_interface                *interface = LTE_fdd_enb_interface::get_instance();
    LTE_FDD_ENB_RRC_PDU_READY_MSG_STRUCT  rrc_pdu_ready;
    LIBLTE_BIT_MSG_STRUCT                *pdu;

    if(LTE_FDD_ENB_ERROR_NONE == pdu_ready->rb->get_next_pdcp_pdu(&pdu))
    {
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                  LTE_FDD_ENB_DEBUG_LEVEL_PDCP,
                                  __FILE__,
                                  __LINE__,
                                  pdu,
                                  "Received PDU for RNTI=%u and RB=%s",
                                  pdu_ready->user->get_c_rnti(),
                                  LTE_fdd_enb_rb_text[pdu_ready->rb->get_rb_id()]);

        if(LTE_FDD_ENB_RB_SRB0 == pdu_ready->rb->get_rb_id())
        {
            // Queue the SDU for RRC
            pdu_ready->rb->queue_rrc_pdu(pdu);

            // Signal RRC
            rrc_pdu_ready.user = pdu_ready->user;
            rrc_pdu_ready.rb   = pdu_ready->rb;
            LTE_fdd_enb_msgq::send(pdcp_rrc_mq,
                                   LTE_FDD_ENB_MESSAGE_TYPE_RRC_PDU_READY,
                                   LTE_FDD_ENB_DEST_LAYER_RRC,
                                   (LTE_FDD_ENB_MESSAGE_UNION *)&rrc_pdu_ready,
                                   sizeof(LTE_FDD_ENB_RRC_PDU_READY_MSG_STRUCT));

            // Delete the PDU
            pdu_ready->rb->delete_next_pdcp_pdu();
        }else if(LTE_FDD_ENB_RB_SRB1 == pdu_ready->rb->get_rb_id()){
            // Parse the header
            // FIXME
            memmove(&pdu->msg[0], &pdu->msg[8], pdu->N_bits-8);
            pdu->N_bits -= 8;

            // Queue the SDU for RRC
            pdu_ready->rb->queue_rrc_pdu(pdu);

            // Signal RRC
            rrc_pdu_ready.user = pdu_ready->user;
            rrc_pdu_ready.rb   = pdu_ready->rb;
            LTE_fdd_enb_msgq::send(pdcp_rrc_mq,
                                   LTE_FDD_ENB_MESSAGE_TYPE_RRC_PDU_READY,
                                   LTE_FDD_ENB_DEST_LAYER_RRC,
                                   (LTE_FDD_ENB_MESSAGE_UNION *)&rrc_pdu_ready,
                                   sizeof(LTE_FDD_ENB_RRC_PDU_READY_MSG_STRUCT));
        }else{
            interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                      LTE_FDD_ENB_DEBUG_LEVEL_PDCP,
                                      __FILE__,
                                      __LINE__,
                                      pdu,
                                      "Received PDU for RNTI=%u with invalid RB=%s",
                                      pdu_ready->user->get_c_rnti(),
                                      LTE_fdd_enb_rb_text[pdu_ready->rb->get_rb_id()]);
        }
    }
}

/******************************/
/*    RRC Message Handlers    */
/******************************/
void LTE_fdd_enb_pdcp::handle_sdu_ready(LTE_FDD_ENB_PDCP_SDU_READY_MSG_STRUCT *sdu_ready)
{
    LTE_fdd_enb_interface                *interface = LTE_fdd_enb_interface::get_instance();
    LTE_FDD_ENB_RLC_SDU_READY_MSG_STRUCT  rlc_sdu_ready;
    LIBLTE_BIT_MSG_STRUCT                *sdu;
    uint32                                i;

    if(LTE_FDD_ENB_ERROR_NONE == sdu_ready->rb->get_next_pdcp_sdu(&sdu))
    {
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                  LTE_FDD_ENB_DEBUG_LEVEL_PDCP,
                                  __FILE__,
                                  __LINE__,
                                  sdu,
                                  "Received SDU for RNTI=%u and RB=%s",
                                  sdu_ready->user->get_c_rnti(),
                                  LTE_fdd_enb_rb_text[sdu_ready->rb->get_rb_id()]);

        if(LTE_FDD_ENB_RB_SRB0 == sdu_ready->rb->get_rb_id())
        {
            // Make sure the SDU is byte aligned
            if((sdu->N_bits % 8) != 0)
            {
                for(i=0; i<8-(sdu->N_bits % 8); i++)
                {
                    sdu->msg[sdu->N_bits + i] = 0;
                }
                sdu->N_bits += 8 - (sdu->N_bits % 8);
            }

            // Queue the PDU for RLC
            sdu_ready->rb->queue_rlc_sdu(sdu);

            // Signal RLC
            rlc_sdu_ready.user = sdu_ready->user;
            rlc_sdu_ready.rb   = sdu_ready->rb;
            LTE_fdd_enb_msgq::send(pdcp_rlc_mq,
                                   LTE_FDD_ENB_MESSAGE_TYPE_RLC_SDU_READY,
                                   LTE_FDD_ENB_DEST_LAYER_RLC,
                                   (LTE_FDD_ENB_MESSAGE_UNION *)&rlc_sdu_ready,
                                   sizeof(LTE_FDD_ENB_RLC_SDU_READY_MSG_STRUCT));

            // Delete the SDU
            sdu_ready->rb->delete_next_pdcp_sdu();
        }else{
            interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                      LTE_FDD_ENB_DEBUG_LEVEL_PDCP,
                                      __FILE__,
                                      __LINE__,
                                      sdu,
                                      "Received SDU for RNTI=%u with invalid RB=%s",
                                      sdu_ready->user->get_c_rnti(),
                                      LTE_fdd_enb_rb_text[sdu_ready->rb->get_rb_id()]);
        }
    }
}
