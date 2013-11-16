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

    File: LTE_fdd_enb_rrc.cc

    Description: Contains all the implementations for the LTE FDD eNodeB
                 radio resource control layer.

    Revision History
    ----------    -------------    --------------------------------------------
    11/10/2013    Ben Wojtowicz    Created file

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_enb_rrc.h"

/*******************************************************************************
                              DEFINES
*******************************************************************************/


/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/


/*******************************************************************************
                              GLOBAL VARIABLES
*******************************************************************************/

LTE_fdd_enb_rrc* LTE_fdd_enb_rrc::instance = NULL;
boost::mutex     rrc_instance_mutex;

/*******************************************************************************
                              CLASS IMPLEMENTATIONS
*******************************************************************************/

/*******************/
/*    Singleton    */
/*******************/
LTE_fdd_enb_rrc* LTE_fdd_enb_rrc::get_instance(void)
{
    boost::mutex::scoped_lock lock(rrc_instance_mutex);

    if(NULL == instance)
    {
        instance = new LTE_fdd_enb_rrc();
    }

    return(instance);
}
void LTE_fdd_enb_rrc::cleanup(void)
{
    boost::mutex::scoped_lock lock(rrc_instance_mutex);

    if(NULL != instance)
    {
        delete instance;
        instance = NULL;
    }
}

/********************************/
/*    Constructor/Destructor    */
/********************************/
LTE_fdd_enb_rrc::LTE_fdd_enb_rrc()
{
    started = false;
}
LTE_fdd_enb_rrc::~LTE_fdd_enb_rrc()
{
    stop();
}

/********************/
/*    Start/Stop    */
/********************/
void LTE_fdd_enb_rrc::start(void)
{
    boost::mutex::scoped_lock lock(start_mutex);
    msgq_cb                   pdcp_cb(&msgq_cb_wrapper<LTE_fdd_enb_rrc, &LTE_fdd_enb_rrc::handle_pdcp_msg>, this);
    msgq_cb                   mme_cb(&msgq_cb_wrapper<LTE_fdd_enb_rrc, &LTE_fdd_enb_rrc::handle_mme_msg>, this);

    if(!started)
    {
        started        = true;
        pdcp_comm_msgq = new LTE_fdd_enb_msgq("pdcp_rrc_mq",
                                              pdcp_cb);
        mme_comm_msgq  = new LTE_fdd_enb_msgq("mme_rrc_mq",
                                              mme_cb);
        rrc_pdcp_mq    = new boost::interprocess::message_queue(boost::interprocess::open_only,
                                                                "rrc_pdcp_mq");
        rrc_mme_mq     = new boost::interprocess::message_queue(boost::interprocess::open_only,
                                                                "rrc_mme_mq");
    }
}
void LTE_fdd_enb_rrc::stop(void)
{
    boost::mutex::scoped_lock lock(start_mutex);

    if(started)
    {
        started = false;
        delete pdcp_comm_msgq;
        delete mme_comm_msgq;
    }
}

/***********************/
/*    Communication    */
/***********************/
void LTE_fdd_enb_rrc::handle_pdcp_msg(LTE_FDD_ENB_MESSAGE_STRUCT *msg)
{
    LTE_fdd_enb_interface *interface = LTE_fdd_enb_interface::get_instance();

    if(LTE_FDD_ENB_DEST_LAYER_RRC == msg->dest_layer ||
       LTE_FDD_ENB_DEST_LAYER_ANY == msg->dest_layer)
    {
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                  __FILE__,
                                  __LINE__,
                                  "Received PDCP message %s",
                                  LTE_fdd_enb_message_type_text[msg->type]);
        delete msg;
    }else{
        // Forward message to MME
        rrc_mme_mq->send(&msg, sizeof(msg), 0);
    }
}
void LTE_fdd_enb_rrc::handle_mme_msg(LTE_FDD_ENB_MESSAGE_STRUCT *msg)
{
    LTE_fdd_enb_interface *interface = LTE_fdd_enb_interface::get_instance();

    if(LTE_FDD_ENB_DEST_LAYER_RRC == msg->dest_layer ||
       LTE_FDD_ENB_DEST_LAYER_ANY == msg->dest_layer)
    {
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                  __FILE__,
                                  __LINE__,
                                  "Received MME message %s",
                                  LTE_fdd_enb_message_type_text[msg->type]);
        delete msg;
    }else{
        // Forward message to PDCP
        rrc_pdcp_mq->send(&msg, sizeof(msg), 0);
    }
}

/****************************/
/*    External Interface    */
/****************************/
void LTE_fdd_enb_rrc::update_sys_info(void)
{
    LTE_fdd_enb_cnfg_db *cnfg_db = LTE_fdd_enb_cnfg_db::get_instance();

    sys_info_mutex.lock();
    cnfg_db->get_sys_info(sys_info);
    sys_info_mutex.unlock();
}
