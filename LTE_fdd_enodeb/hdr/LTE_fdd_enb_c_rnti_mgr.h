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

    File: LTE_fdd_enb_c_rnti_mgr.h

    Description: Contains all the definitions for the LTE FDD eNodeB
                 C-RNTI manager.

    Revision History
    ----------    -------------    --------------------------------------------
    11/09/2013    Ben Wojtowicz    Created file

*******************************************************************************/

#ifndef __LTE_FDD_ENB_C_RNTI_MGR_H__
#define __LTE_FDD_ENB_C_RNTI_MGR_H__

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_enb_user.h"
#include "typedefs.h"
#include <boost/thread/mutex.hpp>

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

class LTE_fdd_enb_c_rnti_mgr
{
public:
    // Singleton
    static LTE_fdd_enb_c_rnti_mgr* get_instance(void);
    static void cleanup(void);

    // External interface
    void add_c_rnti(uint16 c_rnti, LTE_fdd_enb_user *user);
    void update_c_rnti_user(uint16 c_rnti, LTE_fdd_enb_user *user);
    void del_c_rnti(uint16 c_rnti);

private:
    // Singleton
    static LTE_fdd_enb_c_rnti_mgr *instance;
    LTE_fdd_enb_c_rnti_mgr();
    ~LTE_fdd_enb_c_rnti_mgr();

    // User map
    std::map<uint16, LTE_fdd_enb_user*> c_rnti_map;
    boost::mutex                        c_rnti_mutex;
};

#endif /* __LTE_FDD_ENB_C_RNTI_MGR_H__ */
