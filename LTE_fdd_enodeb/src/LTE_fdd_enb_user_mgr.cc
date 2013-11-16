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

    File: LTE_fdd_enb_user_mgr.cc

    Description: Contains all the implementations for the LTE FDD eNodeB
                 user manager.

    Revision History
    ----------    -------------    --------------------------------------------
    11/10/2013    Ben Wojtowicz    Created file

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_enb_user_mgr.h"
#include "liblte_mac.h"
#include <boost/lexical_cast.hpp>

/*******************************************************************************
                              DEFINES
*******************************************************************************/


/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/


/*******************************************************************************
                              GLOBAL VARIABLES
*******************************************************************************/

LTE_fdd_enb_user_mgr* LTE_fdd_enb_user_mgr::instance = NULL;
boost::mutex          user_mgr_instance_mutex;

/*******************************************************************************
                              CLASS IMPLEMENTATIONS
*******************************************************************************/

/*******************/
/*    Singleton    */
/*******************/
LTE_fdd_enb_user_mgr* LTE_fdd_enb_user_mgr::get_instance(void)
{
    boost::mutex::scoped_lock lock(user_mgr_instance_mutex);

    if(NULL == instance)
    {
        instance = new LTE_fdd_enb_user_mgr();
    }

    return(instance);
}
void LTE_fdd_enb_user_mgr::cleanup(void)
{
    boost::mutex::scoped_lock lock(user_mgr_instance_mutex);

    if(NULL != instance)
    {
        delete instance;
        instance = NULL;
    }
}

/********************************/
/*    Constructor/Destructor    */
/********************************/
LTE_fdd_enb_user_mgr::LTE_fdd_enb_user_mgr()
{
    next_c_rnti = LIBLTE_MAC_C_RNTI_START;
}
LTE_fdd_enb_user_mgr::~LTE_fdd_enb_user_mgr()
{
}

/****************************/
/*    External Interface    */
/****************************/
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_user_mgr::get_free_c_rnti(uint16 &c_rnti)
{
    boost::mutex::scoped_lock                     lock(c_rnti_mutex);
    std::map<uint16, LTE_fdd_enb_user*>::iterator iter;
    LTE_FDD_ENB_ERROR_ENUM                        err          = LTE_FDD_ENB_ERROR_NO_FREE_C_RNTI;
    uint16                                        start_c_rnti = next_c_rnti;

    while(c_rnti_map.end() != iter)
    {
        iter = c_rnti_map.find(next_c_rnti++);

        if(LIBLTE_MAC_C_RNTI_END < next_c_rnti)
        {
            next_c_rnti = LIBLTE_MAC_C_RNTI_START;
        }
        if(next_c_rnti == start_c_rnti)
        {
            break;
        }
    }

    if(next_c_rnti != start_c_rnti)
    {
        c_rnti = next_c_rnti-1;
        err    = LTE_FDD_ENB_ERROR_NONE;
    }

    return(err);
}
void LTE_fdd_enb_user_mgr::assign_c_rnti(uint16            c_rnti,
                                         LTE_fdd_enb_user *user)
{
    boost::mutex::scoped_lock lock(c_rnti_mutex);

    c_rnti_map[c_rnti] = user;
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_user_mgr::free_c_rnti(uint16 c_rnti)
{
    boost::mutex::scoped_lock                     lock(c_rnti_mutex);
    std::map<uint16, LTE_fdd_enb_user*>::iterator iter = c_rnti_map.find(c_rnti);
    LTE_FDD_ENB_ERROR_ENUM                        err  = LTE_FDD_ENB_ERROR_C_RNTI_NOT_FOUND;

    if(c_rnti_map.end() != iter)
    {
        c_rnti_map.erase(iter);
        err = LTE_FDD_ENB_ERROR_NONE;
    }

    return(err);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_user_mgr::add_user(std::string imsi)
{
    LTE_fdd_enb_user       *new_user = NULL;
    LTE_FDD_ENB_ERROR_ENUM  err      = LTE_FDD_ENB_ERROR_NONE;

    new_user = new LTE_fdd_enb_user(imsi);

    if(NULL != new_user)
    {
        user_mutex.lock();
        user_map[imsi] = new_user;
        user_mutex.unlock();
    }else{
        err = LTE_FDD_ENB_ERROR_BAD_ALLOC;
    }

    return(err);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_user_mgr::add_user(uint16 c_rnti)
{
    LTE_fdd_enb_interface  *interface = LTE_fdd_enb_interface::get_instance();
    LTE_fdd_enb_user       *new_user  = NULL;
    std::string             fake_imsi;
    LTE_FDD_ENB_ERROR_ENUM  err = LTE_FDD_ENB_ERROR_NONE;

    try
    {
        fake_imsi  = "F";
        fake_imsi += boost::lexical_cast<std::string>(c_rnti);

        new_user = new LTE_fdd_enb_user(fake_imsi);

        if(NULL != new_user)
        {
            user_mutex.lock();
            user_map[fake_imsi] = new_user;
            user_mutex.unlock();

            // FIXME: Need timer to control how long this RNTI stays allocated
            assign_c_rnti(c_rnti, new_user);
        }else{
            err = LTE_FDD_ENB_ERROR_BAD_ALLOC;
        }
    }catch(...){
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                  __FILE__,
                                  __LINE__,
                                  "Exception to fake imsi");
        err = LTE_FDD_ENB_ERROR_EXCEPTION;
    }

    return(err);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_user_mgr::find_user(std::string        imsi,
                                                       LTE_fdd_enb_user **user)
{
    boost::mutex::scoped_lock                          lock(user_mutex);
    std::map<std::string, LTE_fdd_enb_user*>::iterator iter = user_map.find(imsi);
    LTE_FDD_ENB_ERROR_ENUM                             err  = LTE_FDD_ENB_ERROR_USER_NOT_FOUND;

    if(user_map.end() != iter)
    {
        *user = (*iter).second;
        err   = LTE_FDD_ENB_ERROR_NONE;
    }

    return(err);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_user_mgr::find_user(uint16             c_rnti,
                                                       LTE_fdd_enb_user **user)
{
    boost::mutex::scoped_lock                     lock(c_rnti_mutex);
    std::map<uint16, LTE_fdd_enb_user*>::iterator iter = c_rnti_map.find(c_rnti);
    LTE_FDD_ENB_ERROR_ENUM                        err  = LTE_FDD_ENB_ERROR_USER_NOT_FOUND;

    if(c_rnti_map.end() != iter &&
       NULL             != (*iter).second)
    {
        *user = (*iter).second;
        err   = LTE_FDD_ENB_ERROR_NONE;
    }

    return(err);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_user_mgr::del_user(std::string imsi)
{
    boost::mutex::scoped_lock                          lock(user_mutex);
    std::map<std::string, LTE_fdd_enb_user*>::iterator iter = user_map.find(imsi);
    LTE_FDD_ENB_ERROR_ENUM                             err  = LTE_FDD_ENB_ERROR_USER_NOT_FOUND;

    if(user_map.end() != iter)
    {
        delete (*iter).second;
        user_map.erase(iter);
        err = LTE_FDD_ENB_ERROR_NONE;
    }

    return(err);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_user_mgr::del_user(uint16 c_rnti)
{
    LTE_fdd_enb_interface                              *interface = LTE_fdd_enb_interface::get_instance();
    boost::mutex::scoped_lock                           lock(user_mutex);
    std::map<std::string, LTE_fdd_enb_user*>::iterator  iter;
    std::string                                         fake_imsi;
    LTE_FDD_ENB_ERROR_ENUM                              err = LTE_FDD_ENB_ERROR_USER_NOT_FOUND;

    try
    {
        fake_imsi  = "F";
        fake_imsi += boost::lexical_cast<std::string>(c_rnti);

        iter = user_map.find(fake_imsi);

        if(user_map.end() != iter)
        {
            delete (*iter).second;
            user_map.erase(iter);
            err = LTE_FDD_ENB_ERROR_NONE;
        }
    }catch(...){
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                  __FILE__,
                                  __LINE__,
                                  "Exception to fake imsi");
        err = LTE_FDD_ENB_ERROR_EXCEPTION;
    }

    return(err);
}
