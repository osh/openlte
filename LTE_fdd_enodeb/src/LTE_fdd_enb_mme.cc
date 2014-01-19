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

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_enb_mme.h"
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
    msgq_cb                   rrc_cb(&msgq_cb_wrapper<LTE_fdd_enb_mme, &LTE_fdd_enb_mme::handle_rrc_msg>, this);

    if(!started)
    {
        started       = true;
        rrc_comm_msgq = new LTE_fdd_enb_msgq("mme_rrc_mq",
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

    interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                              LTE_FDD_ENB_DEBUG_LEVEL_MME,
                              __FILE__,
                              __LINE__,
                              "Received RRC message %s",
                              LTE_fdd_enb_message_type_text[msg->type]);
    delete msg;
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
