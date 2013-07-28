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

    File: LTE_fdd_dl_scan_flowgraph.h

    Description: Contains all the definitions for the LTE FDD DL Scanner
                 gnuradio flowgraph.

    Revision History
    ----------    -------------    --------------------------------------------
    02/26/2013    Ben Wojtowicz    Created file
    07/21/2013    Ben Wojtowicz    Added support for HackRF Jawbreaker

*******************************************************************************/

#ifndef __LTE_FDD_DL_SCAN_FLOWGRAPH_H__
#define __LTE_FDD_DL_SCAN_FLOWGRAPH_H__

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_dl_scan_interface.h"
#include "LTE_fdd_dl_scan_state_machine.h"
#include <boost/thread/mutex.hpp>
#include <gr_top_block.h>
#include <osmosdr_source_c.h>

/*******************************************************************************
                              DEFINES
*******************************************************************************/


/*******************************************************************************
                              FORWARD DECLARATIONS
*******************************************************************************/


/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/

typedef enum{
    LTE_FDD_DL_SCAN_HW_TYPE_RTL_SDR = 0,
    LTE_FDD_DL_SCAN_HW_TYPE_HACKRF,
    LTE_FDD_DL_SCAN_HW_TYPE_UNKNOWN,
}LTE_FDD_DL_SCAN_HW_TYPE_ENUM;

/*******************************************************************************
                              CLASS DECLARATIONS
*******************************************************************************/

class LTE_fdd_dl_scan_flowgraph
{
public:
    // Singleton
    static LTE_fdd_dl_scan_flowgraph* get_instance(void);
    static void cleanup(void);

    // Flowgraph
    bool is_started(void);
    LTE_FDD_DL_SCAN_STATUS_ENUM start(uint16 dl_earfcn);
    LTE_FDD_DL_SCAN_STATUS_ENUM stop(void);
    void update_center_freq(uint16 dl_earfcn);

private:
    // Singleton
    static LTE_fdd_dl_scan_flowgraph *instance;
    LTE_fdd_dl_scan_flowgraph();
    ~LTE_fdd_dl_scan_flowgraph();

    // Run
    static void* run_thread(void *inputs);

    // Variables
    gr_top_block_sptr                  top_block;
    osmosdr_source_c_sptr              samp_src;
    LTE_fdd_dl_scan_state_machine_sptr state_machine;

    pthread_t    start_thread;
    boost::mutex start_mutex;
    bool         started;
};

#endif /* __LTE_FDD_DL_SCAN_FLOWGRAPH_H__ */
