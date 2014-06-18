#line 2 "LTE_fdd_enb_mme.cc" // Make __FILE__ omit the path
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

    File: LTE_fdd_enb_mme.cc

    Description: Contains all the implementations for the LTE FDD eNodeB
                 mobility management entity layer.

    Revision History
    ----------    -------------    --------------------------------------------
    11/10/2013    Ben Wojtowicz    Created file
    01/18/2014    Ben Wojtowicz    Added level to debug prints.
    06/15/2014    Ben Wojtowicz    Added RRC NAS message handler.

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_enb_mme.h"
#include "LTE_fdd_enb_interface.h"
#include "liblte_mme.h"

/*******************************************************************************
                              DEFINES
*******************************************************************************/


/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/


/*******************************************************************************
                              GLOBAL VARIABLES
*******************************************************************************/

LTE_fdd_enb_mme* LTE_fdd_enb_mme::instance = NULL;
boost::mutex     mme_instance_mutex;

/*******************************************************************************
                              CLASS IMPLEMENTATIONS
*******************************************************************************/

/*******************/
/*    Singleton    */
/*******************/
LTE_fdd_enb_mme* LTE_fdd_enb_mme::get_instance(void)
{
    boost::mutex::scoped_lock lock(mme_instance_mutex);

    if(NULL == instance)
    {
        instance = new LTE_fdd_enb_mme();
    }

    return(instance);
}
void LTE_fdd_enb_mme::cleanup(void)
{
    boost::mutex::scoped_lock lock(mme_instance_mutex);

    if(NULL != instance)
    {
        delete instance;
        instance = NULL;
    }
}

/********************************/
/*    Constructor/Destructor    */
/********************************/
LTE_fdd_enb_mme::LTE_fdd_enb_mme()
{
    started = false;
}
LTE_fdd_enb_mme::~LTE_fdd_enb_mme()
{
    stop();
}

/********************/
/*    Start/Stop    */
/********************/
void LTE_fdd_enb_mme::start(void)
{
    boost::mutex::scoped_lock lock(start_mutex);
    LTE_fdd_enb_msgq_cb       rrc_cb(&LTE_fdd_enb_msgq_cb_wrapper<LTE_fdd_enb_mme, &LTE_fdd_enb_mme::handle_rrc_msg>, this);

    if(!started)
    {
        started       = true;
        rrc_comm_msgq = new LTE_fdd_enb_msgq("rrc_mme_mq",
                                             rrc_cb);
        mme_rrc_mq    = new boost::interprocess::message_queue(boost::interprocess::open_only,
                                                               "mme_rrc_mq");
    }
}
void LTE_fdd_enb_mme::stop(void)
{
    boost::mutex::scoped_lock lock(start_mutex);

    if(started)
    {
        started = false;
        delete rrc_comm_msgq;
    }
}

/***********************/
/*    Communication    */
/***********************/
void LTE_fdd_enb_mme::handle_rrc_msg(LTE_FDD_ENB_MESSAGE_STRUCT *msg)
{
    LTE_fdd_enb_interface *interface = LTE_fdd_enb_interface::get_instance();

    switch(msg->type)
    {
    case LTE_FDD_ENB_MESSAGE_TYPE_MME_NAS_MSG_READY:
        handle_nas_msg(&msg->msg.mme_nas_msg_ready);
        delete msg;
        break;
    default:
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                  LTE_FDD_ENB_DEBUG_LEVEL_MME,
                                  __FILE__,
                                  __LINE__,
                                  "Received invalid RRC message %s",
                                  LTE_fdd_enb_message_type_text[msg->type]);
        delete msg;
        break;
    }
}

/****************************/
/*    External Interface    */
/****************************/
void LTE_fdd_enb_mme::update_sys_info(void)
{
    LTE_fdd_enb_cnfg_db *cnfg_db = LTE_fdd_enb_cnfg_db::get_instance();

    sys_info_mutex.lock();
    cnfg_db->get_sys_info(sys_info);
    sys_info_mutex.unlock();
}

/******************************/
/*    RRC Message Handlers    */
/******************************/
void LTE_fdd_enb_mme::handle_nas_msg(LTE_FDD_ENB_MME_NAS_MSG_READY_MSG_STRUCT *nas_msg)
{
    LTE_fdd_enb_interface  *interface = LTE_fdd_enb_interface::get_instance();
    LIBLTE_BYTE_MSG_STRUCT *msg;
    uint8                   pd;
    uint8                   msg_type;

    // FIXME
    LIBLTE_MME_ATTACH_REQUEST_MSG_STRUCT           attach_req;
    LIBLTE_MME_PDN_CONNECTIVITY_REQUEST_MSG_STRUCT pdn_con_req;

    if(LTE_FDD_ENB_ERROR_NONE == nas_msg->rb->get_next_mme_nas_msg(&msg))
    {
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                  LTE_FDD_ENB_DEBUG_LEVEL_MME,
                                  __FILE__,
                                  __LINE__,
                                  msg,
                                  "Received NAS message for RNTI=%u and RB=%s",
                                  nas_msg->user->get_c_rnti(),
                                  LTE_fdd_enb_rb_text[nas_msg->rb->get_rb_id()]);

        liblte_mme_parse_msg_header(msg, &pd, &msg_type);

        if(LIBLTE_MME_MSG_TYPE_ATTACH_REQUEST == msg_type)
        {
            liblte_mme_unpack_attach_request_msg(msg, &attach_req);
            liblte_mme_parse_msg_header(&attach_req.esm_msg, &pd, &msg_type);
            if(LIBLTE_MME_MSG_TYPE_PDN_CONNECTIVITY_REQUEST == msg_type)
            {
                liblte_mme_unpack_pdn_connectivity_request_msg(&attach_req.esm_msg, &pdn_con_req);

                if(LIBLTE_MME_EPS_MOBILE_ID_TYPE_GUTI == attach_req.eps_mobile_id.type_of_id)
                {
                    interface->send_ctrl_info_msg("attach_request m_tmsi=%08X",
                                                  attach_req.eps_mobile_id.guti.m_tmsi);
                }else if(LIBLTE_MME_EPS_MOBILE_ID_TYPE_IMSI == attach_req.eps_mobile_id.type_of_id){
                    interface->send_ctrl_info_msg("attach_request imsi=%u%u%u%u%u%u%u%u%u%u%u%u%u%u%u",
                                                  attach_req.eps_mobile_id.imsi[0],
                                                  attach_req.eps_mobile_id.imsi[1],
                                                  attach_req.eps_mobile_id.imsi[2],
                                                  attach_req.eps_mobile_id.imsi[3],
                                                  attach_req.eps_mobile_id.imsi[4],
                                                  attach_req.eps_mobile_id.imsi[5],
                                                  attach_req.eps_mobile_id.imsi[6],
                                                  attach_req.eps_mobile_id.imsi[7],
                                                  attach_req.eps_mobile_id.imsi[8],
                                                  attach_req.eps_mobile_id.imsi[9],
                                                  attach_req.eps_mobile_id.imsi[10],
                                                  attach_req.eps_mobile_id.imsi[11],
                                                  attach_req.eps_mobile_id.imsi[12],
                                                  attach_req.eps_mobile_id.imsi[13],
                                                  attach_req.eps_mobile_id.imsi[14]);
                }else{
                    interface->send_ctrl_info_msg("attach_request imei=%u%u%u%u%u%u%u%u%u%u%u%u%u%u%u",
                                                  attach_req.eps_mobile_id.imei[0],
                                                  attach_req.eps_mobile_id.imei[1],
                                                  attach_req.eps_mobile_id.imei[2],
                                                  attach_req.eps_mobile_id.imei[3],
                                                  attach_req.eps_mobile_id.imei[4],
                                                  attach_req.eps_mobile_id.imei[5],
                                                  attach_req.eps_mobile_id.imei[6],
                                                  attach_req.eps_mobile_id.imei[7],
                                                  attach_req.eps_mobile_id.imei[8],
                                                  attach_req.eps_mobile_id.imei[9],
                                                  attach_req.eps_mobile_id.imei[10],
                                                  attach_req.eps_mobile_id.imei[11],
                                                  attach_req.eps_mobile_id.imei[12],
                                                  attach_req.eps_mobile_id.imei[13],
                                                  attach_req.eps_mobile_id.imei[14]);
                }
            }
        }

        // Delete the NAS message
        nas_msg->rb->delete_next_mme_nas_msg();
    }else{
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                  LTE_FDD_ENB_DEBUG_LEVEL_MME,
                                  __FILE__,
                                  __LINE__,
                                  "Received NAS message with no message queued");
    }
}
