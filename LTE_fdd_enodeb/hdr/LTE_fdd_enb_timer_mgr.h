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

    File: LTE_fdd_enb_timer_mgr.h

    Description: Contains all the definitions for the LTE FDD eNodeB
                 timer manager.

    Revision History
    ----------    -------------    --------------------------------------------
    05/04/2014    Ben Wojtowicz    Created file
    06/15/2014    Ben Wojtowicz    Added millisecond resolution.

*******************************************************************************/

#ifndef __LTE_FDD_ENB_TIMER_MGR_H__
#define __LTE_FDD_ENB_TIMER_MGR_H__

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_enb_interface.h"
#include "LTE_fdd_enb_timer.h"
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

class LTE_fdd_enb_timer_mgr
{
public:
    // Singleton
    static LTE_fdd_enb_timer_mgr* get_instance(void);
    static void cleanup(void);

    // External Interface
    LTE_FDD_ENB_ERROR_ENUM start_timer(uint32 m_seconds, LTE_fdd_enb_timer_cb cb, uint32 *timer_id);
    LTE_FDD_ENB_ERROR_ENUM stop_timer(uint32 timer_id);
    void handle_tick(void);

private:
    // Singleton
    static LTE_fdd_enb_timer_mgr *instance;
    LTE_fdd_enb_timer_mgr();
    ~LTE_fdd_enb_timer_mgr();

    // Timer Storage
    boost::mutex                         timer_mutex;
    std::map<uint32, LTE_fdd_enb_timer*> timer_map;
    uint32                               next_timer_id;
};

#endif /* __LTE_FDD_ENB_TIMER_MGR_H__ */
