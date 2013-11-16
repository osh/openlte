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

    File: LTE_fdd_enb_c_rnti_mgr.cc

    Description: Contains all the implementations for the LTE FDD eNodeB
                 C-RNTI manager.

    Revision History
    ----------    -------------    --------------------------------------------
    11/10/2013    Ben Wojtowicz    Created file

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_enb_c_rnti_mgr.h"

/*******************************************************************************
                              DEFINES
*******************************************************************************/


/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/


/*******************************************************************************
                              GLOBAL VARIABLES
*******************************************************************************/

LTE_fdd_enb_c_rnti_mgr* LTE_fdd_enb_c_rnti_mgr::instance = NULL;
boost::mutex            c_rnti_mgr_instance_mutex;

/*******************************************************************************
                              CLASS IMPLEMENTATIONS
*******************************************************************************/

/*******************/
/*    Singleton    */
/*******************/
LTE_fdd_enb_c_rnti_mgr* LTE_fdd_enb_c_rnti_mgr::get_instance(void)
{
    boost::mutex::scoped_lock lock(c_rnti_mgr_instance_mutex);

    if(NULL == instance)
    {
        instance = new LTE_fdd_enb_c_rnti_mgr();
    }

    return(instance);
}
void LTE_fdd_enb_c_rnti_mgr::cleanup(void)
{
    boost::mutex::scoped_lock lock(c_rnti_mgr_instance_mutex);

    if(NULL != instance)
    {
        delete instance;
        instance = NULL;
    }
}

/********************************/
/*    Constructor/Destructor    */
/********************************/
LTE_fdd_enb_c_rnti_mgr::LTE_fdd_enb_c_rnti_mgr()
{
}
LTE_fdd_enb_c_rnti_mgr::~LTE_fdd_enb_c_rnti_mgr()
{
}

/****************************/
/*    External Interface    */
/****************************/
void LTE_fdd_enb_c_rnti_mgr::add_c_rnti(uint16            c_rnti,
                                        LTE_fdd_enb_user *user)
{
    boost::mutex::scoped_lock lock(c_rnti_mutex);

    c_rnti_map[c_rnti] = user;
}
void LTE_fdd_enb_c_rnti_mgr::update_c_rnti_user(uint16            c_rnti,
                                                LTE_fdd_enb_user *user)
{
    boost::mutex::scoped_lock                     lock(c_rnti_mutex);
    std::map<uint16, LTE_fdd_enb_user*>::iterator iter = c_rnti_map.find(c_rnti);

    if(c_rnti_map.end() != iter)
    {
        (*iter).second = user;
    }
}
void LTE_fdd_enb_c_rnti_mgr::del_c_rnti(uint16 c_rnti)
{
    boost::mutex::scoped_lock                     lock(c_rnti_mutex);
    std::map<uint16, LTE_fdd_enb_user*>::iterator iter = c_rnti_map.find(c_rnti);

    if(c_rnti_map.end() != iter)
    {
        // FIXME: Remove entry from map
    }
}
