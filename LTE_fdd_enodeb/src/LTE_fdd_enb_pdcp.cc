/*******************************************************************************

    Copyright 2013 Ben Wojtowicz

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

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_enb_pdcp.h"

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
    msgq_cb                   rlc_cb(&msgq_cb_wrapper<LTE_fdd_enb_pdcp, &LTE_fdd_enb_pdcp::handle_rlc_msg>, this);
    msgq_cb                   rrc_cb(&msgq_cb_wrapper<LTE_fdd_enb_pdcp, &LTE_fdd_enb_pdcp::handle_rrc_msg>, this);

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
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                  __FILE__,
                                  __LINE__,
                                  "Received RLC message %s",
                                  LTE_fdd_enb_message_type_text[msg->type]);
        delete msg;
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
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                  __FILE__,
                                  __LINE__,
                                  "Received RRC message %s",
                                  LTE_fdd_enb_message_type_text[msg->type]);
        delete msg;
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
