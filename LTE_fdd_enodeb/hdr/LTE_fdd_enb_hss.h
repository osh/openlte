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

    File: LTE_fdd_enb_hss.h

    Description: Contains all the definitions for the LTE FDD eNodeB
                 home subscriber server.

    Revision History
    ----------    -------------    --------------------------------------------
    06/15/2014    Ben Wojtowicz    Created file

*******************************************************************************/

#ifndef __LTE_FDD_ENB_HSS_H__
#define __LTE_FDD_ENB_HSS_H__

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_enb_interface.h"
#include "LTE_fdd_enb_user.h"
#include <list>

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

class LTE_fdd_enb_hss
{
public:
    // Singleton
    static LTE_fdd_enb_hss* get_instance(void);
    static void cleanup(void);

    // External interface
    LTE_FDD_ENB_ERROR_ENUM add_user(std::string imsi);
    LTE_FDD_ENB_ERROR_ENUM find_user(std::string imsi, LTE_fdd_enb_user **user);
    LTE_FDD_ENB_ERROR_ENUM del_user(std::string imsi);
    std::string print_all_users(void);

private:
    // Singleton
    static LTE_fdd_enb_hss *instance;
    LTE_fdd_enb_hss();
    ~LTE_fdd_enb_hss();

    // Allowed users
    boost::mutex                  user_mutex;
    std::list<LTE_fdd_enb_user *> user_list;
};

#endif /* __LTE_FDD_ENB_HSS_H__ */
