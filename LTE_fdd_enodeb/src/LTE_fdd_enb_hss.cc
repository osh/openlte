#line 2 "LTE_fdd_enb_hss.cc" // Make __FILE__ omit the path
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

    File: LTE_fdd_enb_hss.cc

    Description: Contains all the implementations for the LTE FDD eNodeB
                 home subscriber server.

    Revision History
    ----------    -------------    --------------------------------------------
    06/15/2014    Ben Wojtowicz    Created file

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_enb_hss.h"
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

LTE_fdd_enb_hss* LTE_fdd_enb_hss::instance = NULL;
boost::mutex     hss_instance_mutex;

/*******************************************************************************
                              CLASS IMPLEMENTATIONS
*******************************************************************************/

/*******************/
/*    Singleton    */
/*******************/
LTE_fdd_enb_hss* LTE_fdd_enb_hss::get_instance(void)
{
    boost::mutex::scoped_lock lock(hss_instance_mutex);

    if(NULL == instance)
    {
        instance = new LTE_fdd_enb_hss();
    }

    return(instance);
}
void LTE_fdd_enb_hss::cleanup(void)
{
    boost::mutex::scoped_lock lock(hss_instance_mutex);

    if(NULL != instance)
    {
        delete instance;
        instance = NULL;
    }
}

/********************************/
/*    Constructor/Destructor    */
/********************************/
LTE_fdd_enb_hss::LTE_fdd_enb_hss()
{
    user_list.clear();
}
LTE_fdd_enb_hss::~LTE_fdd_enb_hss()
{
}

/****************************/
/*    External Interface    */
/****************************/
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_hss::add_user(std::string imsi)
{
    LTE_fdd_enb_user       *new_user = NULL;
    LTE_FDD_ENB_ERROR_ENUM  err      = LTE_FDD_ENB_ERROR_BAD_ALLOC;

    new_user = new LTE_fdd_enb_user(imsi);

    if(NULL != new_user)
    {
        user_mutex.lock();
        user_list.push_back(new_user);
        user_mutex.unlock();

        err = LTE_FDD_ENB_ERROR_NONE;
    }

    return(err);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_hss::find_user(std::string        imsi,
                                                  LTE_fdd_enb_user **user)
{
    boost::mutex::scoped_lock               lock(user_mutex);
    std::list<LTE_fdd_enb_user *>::iterator iter;
    LTE_FDD_ENB_ERROR_ENUM                  err = LTE_FDD_ENB_ERROR_USER_NOT_FOUND;

    for(iter=user_list.begin(); iter!=user_list.end(); iter++)
    {
        if((*iter)->get_imsi() == imsi)
        {
            *user = (*iter);
            err   = LTE_FDD_ENB_ERROR_NONE;
            break;
        }
    }

    return(err);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_hss::del_user(std::string imsi)
{
    boost::mutex::scoped_lock                lock(user_mutex);
    std::list<LTE_fdd_enb_user *>::iterator  iter;
    LTE_fdd_enb_user                        *user = NULL;
    LTE_FDD_ENB_ERROR_ENUM                   err  = LTE_FDD_ENB_ERROR_USER_NOT_FOUND;

    for(iter=user_list.begin(); iter!=user_list.end(); iter++)
    {
        if((*iter)->get_imsi() == imsi)
        {
            user = (*iter);
            user_list.erase(iter);
            delete user;
            err = LTE_FDD_ENB_ERROR_NONE;
            break;
        }
    }

    return(err);
}
std::string LTE_fdd_enb_hss::print_all_users(void)
{
    boost::mutex::scoped_lock               lock(user_mutex);
    std::list<LTE_fdd_enb_user *>::iterator iter;
    std::string                             output;

    output = boost::lexical_cast<std::string>(user_list.size());
    for(iter=user_list.begin(); iter!=user_list.end(); iter++)
    {
        output += "\n";
        output += (*iter)->get_imsi();
    }

    return(output);
}
