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
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                  LTE_FDD_ENB_DEBUG_LEVEL_RLC,
                                  __FILE__,
                                  __LINE__,
                                  "Received MAC message %s",
                                  LTE_fdd_enb_message_type_text[msg->type]);
        delete msg;
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
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                  LTE_FDD_ENB_DEBUG_LEVEL_RLC,
                                  __FILE__,
                                  __LINE__,
                                  "Received PDCP message %s",
                                  LTE_fdd_enb_message_type_text[msg->type]);
        delete msg;
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
