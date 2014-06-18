#line 2 "LTE_fdd_enb_timer_mgr.cc" // Make __FILE__ omit the path
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

    File: LTE_fdd_enb_timer_mgr.cc

    Description: Contains all the implementations for the LTE FDD eNodeB
                 timer manager.

    Revision History
    ----------    -------------    --------------------------------------------
    05/04/2014    Ben Wojtowicz    Created file
    06/15/2014    Ben Wojtowicz    Added millisecond resolution.

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_enb_timer_mgr.h"
#include <list>

/*******************************************************************************
                              DEFINES
*******************************************************************************/


/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/


/*******************************************************************************
                              GLOBAL VARIABLES
*******************************************************************************/

LTE_fdd_enb_timer_mgr* LTE_fdd_enb_timer_mgr::instance = NULL;
boost::mutex           timer_mgr_instance_mutex;

/*******************************************************************************
                              CLASS IMPLEMENTATIONS
*******************************************************************************/

/*******************/
/*    Singleton    */
/*******************/
LTE_fdd_enb_timer_mgr* LTE_fdd_enb_timer_mgr::get_instance(void)
{
    boost::mutex::scoped_lock lock(timer_mgr_instance_mutex);

    if(NULL == instance)
    {
        instance = new LTE_fdd_enb_timer_mgr();
    }

    return(instance);
}
void LTE_fdd_enb_timer_mgr::cleanup(void)
{
    boost::mutex::scoped_lock lock(timer_mgr_instance_mutex);

    if(NULL != instance)
    {
        delete instance;
        instance = NULL;
    }
}

/********************************/
/*    Constructor/Destructor    */
/********************************/
LTE_fdd_enb_timer_mgr::LTE_fdd_enb_timer_mgr()
{
    next_timer_id = 0;
}
LTE_fdd_enb_timer_mgr::~LTE_fdd_enb_timer_mgr()
{
}

/****************************/
/*    External Interface    */
/****************************/
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_timer_mgr::start_timer(uint32                m_seconds,
                                                          LTE_fdd_enb_timer_cb  cb,
                                                          uint32               *timer_id)
{
    boost::mutex::scoped_lock                        lock(timer_mutex);
    std::map<uint32, LTE_fdd_enb_timer *>::iterator  iter      = timer_map.find(next_timer_id);
    LTE_fdd_enb_timer                               *new_timer = NULL;
    LTE_FDD_ENB_ERROR_ENUM                           err       = LTE_FDD_ENB_ERROR_BAD_ALLOC;

    while(timer_map.end() != iter)
    {
        next_timer_id++;
        iter = timer_map.find(next_timer_id);
    }
    new_timer = new LTE_fdd_enb_timer(m_seconds, next_timer_id, cb);

    if(NULL != new_timer)
    {
        *timer_id                  = next_timer_id;
        timer_map[next_timer_id++] = new_timer;
        err                        = LTE_FDD_ENB_ERROR_NONE;
    }

    return(err);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_timer_mgr::stop_timer(uint32 timer_id)
{
    boost::mutex::scoped_lock                       lock(timer_mutex);
    std::map<uint32, LTE_fdd_enb_timer *>::iterator iter = timer_map.find(timer_id);
    LTE_FDD_ENB_ERROR_ENUM                          err  = LTE_FDD_ENB_ERROR_TIMER_NOT_FOUND;

    if(timer_map.end() != iter)
    {
        delete (*iter).second;
        timer_map.erase(iter);
        err = LTE_FDD_ENB_ERROR_NONE;
    }

    return(err);
}
void LTE_fdd_enb_timer_mgr::handle_tick(void)
{
    std::map<uint32, LTE_fdd_enb_timer *>::iterator iter;
    std::list<uint32>                               expired_list;

    timer_mutex.lock();
    for(iter=timer_map.begin(); iter!=timer_map.end(); iter++)
    {
        (*iter).second->increment();

        if((*iter).second->expired())
        {
            expired_list.push_back((*iter).first);
        }
    }
    timer_mutex.unlock();

    // Delete expired timers
    while(0 != expired_list.size())
    {
        iter = timer_map.find(expired_list.front());
        if(timer_map.end() != iter)
        {
            (*iter).second->call_callback();
            delete (*iter).second;
            timer_map.erase(iter);
        }
        expired_list.pop_front();
    }
}
