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

    File: LTE_fdd_enb_user_mgr.h

    Description: Contains all the definitions for the LTE FDD eNodeB
                 user manager.

    Revision History
    ----------    -------------    --------------------------------------------
    11/09/2013    Ben Wojtowicz    Created file
    05/04/2014    Ben Wojtowicz    Added C-RNTI timeout timers.

*******************************************************************************/

#ifndef __LTE_FDD_ENB_USER_MGR_H__
#define __LTE_FDD_ENB_USER_MGR_H__

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_enb_interface.h"
#include "LTE_fdd_enb_user.h"
#include <boost/thread/mutex.hpp>
#include <string>

/*******************************************************************************
                              DEFINES
*******************************************************************************/


/*******************************************************************************
                              FORWARD DECLARATIONS
*******************************************************************************/


/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/


/*******************************************************************************
                              CLASS DECLARATIONS
*******************************************************************************/

class LTE_fdd_enb_user_mgr
{
public:
    // Singleton
    static LTE_fdd_enb_user_mgr* get_instance(void);
    static void cleanup(void);

    // External interface
    LTE_FDD_ENB_ERROR_ENUM get_free_c_rnti(uint16 *c_rnti);
    void assign_c_rnti(uint16 c_rnti, LTE_fdd_enb_user *user);
    LTE_FDD_ENB_ERROR_ENUM free_c_rnti(uint16 c_rnti);
    LTE_FDD_ENB_ERROR_ENUM add_user(std::string imsi);
    LTE_FDD_ENB_ERROR_ENUM add_user(uint16 c_rnti);
    LTE_FDD_ENB_ERROR_ENUM find_user(std::string imsi, LTE_fdd_enb_user **user);
    LTE_FDD_ENB_ERROR_ENUM find_user(uint16 c_rnti, LTE_fdd_enb_user **user);
    LTE_FDD_ENB_ERROR_ENUM del_user(std::string imsi);
    LTE_FDD_ENB_ERROR_ENUM del_user(uint16 c_rnti);

private:
    // Singleton
    static LTE_fdd_enb_user_mgr *instance;
    LTE_fdd_enb_user_mgr();
    ~LTE_fdd_enb_user_mgr();

    // C-RNTI Timer
    void handle_c_rnti_timer_expiry(uint32 timer_id);

    // User storage
    std::map<std::string, LTE_fdd_enb_user*> user_map;
    std::map<uint16, LTE_fdd_enb_user*>      c_rnti_map;
    std::map<uint32, uint16>                 timer_id_map;
    boost::mutex                             user_mutex;
    boost::mutex                             c_rnti_mutex;
    boost::mutex                             timer_id_mutex;
    uint16                                   next_c_rnti;
};

#endif /* __LTE_FDD_ENB_USER_MGR_H__ */
