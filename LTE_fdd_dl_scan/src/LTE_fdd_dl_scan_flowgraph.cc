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

    File: LTE_fdd_dl_scan_flowgraph.cc

    Description: Contains all the implementations for the LTE FDD DL Scanner
                 gnuradio flowgraph.

    Revision History
    ----------    -------------    --------------------------------------------
    02/26/2013    Ben Wojtowicz    Created file
    07/21/2013    Ben Wojtowicz    Added support for HackRF Jawbreaker

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_dl_scan_flowgraph.h"

/*******************************************************************************
                              DEFINES
*******************************************************************************/


/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/


/*******************************************************************************
                              GLOBAL VARIABLES
*******************************************************************************/

LTE_fdd_dl_scan_flowgraph* LTE_fdd_dl_scan_flowgraph::instance = NULL;
boost::mutex               flowgraph_instance_mutex;

/*******************************************************************************
                              CLASS IMPLEMENTATIONS
*******************************************************************************/

// Singleton
LTE_fdd_dl_scan_flowgraph* LTE_fdd_dl_scan_flowgraph::get_instance(void)
{
    boost::mutex::scoped_lock lock(flowgraph_instance_mutex);

    if(NULL == instance)
    {
        instance = new LTE_fdd_dl_scan_flowgraph();
    }

    return(instance);
}
void LTE_fdd_dl_scan_flowgraph::cleanup(void)
{
    boost::mutex::scoped_lock lock(flowgraph_instance_mutex);

    if(NULL != instance)
    {
        delete instance;
        instance = NULL;
    }
}

// Constructor/Destructor
LTE_fdd_dl_scan_flowgraph::LTE_fdd_dl_scan_flowgraph()
{
    started = false;
}
LTE_fdd_dl_scan_flowgraph::~LTE_fdd_dl_scan_flowgraph()
{
    boost::mutex::scoped_lock lock(start_mutex);

    if(started)
    {
        start_mutex.unlock();
        stop();
    }
}

// Flowgraph
bool LTE_fdd_dl_scan_flowgraph::is_started(void)
{
    boost::mutex::scoped_lock lock(start_mutex);

    return(started);
}
LTE_FDD_DL_SCAN_STATUS_ENUM LTE_fdd_dl_scan_flowgraph::start(uint16 dl_earfcn)
{
    boost::mutex::scoped_lock     lock(start_mutex);
    LTE_fdd_dl_scan_interface    *interface     = LTE_fdd_dl_scan_interface::get_instance();
    LTE_FDD_DL_SCAN_STATUS_ENUM   err           = LTE_FDD_DL_SCAN_STATUS_FAIL;
    LTE_FDD_DL_SCAN_HW_TYPE_ENUM  hardware_type = LTE_FDD_DL_SCAN_HW_TYPE_UNKNOWN;
    std::vector<double>           range;

    if(!started)
    {
        if(NULL == top_block.get())
        {
            top_block = gr_make_top_block("top");
        }
        if(NULL == samp_src.get())
        {
            samp_src = osmosdr_make_source_c();
            if(0 != samp_src->get_num_channels())
            {
                range = samp_src->get_sample_rates().values();
                if(range[range.size()-1] == 20000000)
                {
                    hardware_type = LTE_FDD_DL_SCAN_HW_TYPE_HACKRF;
                }else if(range[range.size()-1] == 2400000){
                    hardware_type = LTE_FDD_DL_SCAN_HW_TYPE_RTL_SDR;
                }
            }
        }
        if(NULL == state_machine.get())
        {
            switch(hardware_type)
            {
            case LTE_FDD_DL_SCAN_HW_TYPE_HACKRF:
                state_machine = LTE_fdd_dl_scan_make_state_machine(15360000);
                break;
            case LTE_FDD_DL_SCAN_HW_TYPE_UNKNOWN:
            default:
                printf("Unknown hardware, treating like RTL-SDR\n");
            case LTE_FDD_DL_SCAN_HW_TYPE_RTL_SDR:
                state_machine = LTE_fdd_dl_scan_make_state_machine(1920000);
                break;
            }
        }

        if(NULL != top_block.get() &&
           NULL != samp_src.get()  &&
           NULL != state_machine.get())
        {
            if(0 != samp_src->get_num_channels())
            {
                switch(hardware_type)
                {
                case LTE_FDD_DL_SCAN_HW_TYPE_HACKRF:
                    samp_src->set_sample_rate(15360000);
                    samp_src->set_gain_mode(false);
                    samp_src->set_gain(14);
                    samp_src->set_dc_offset_mode(osmosdr_source_c::DCOffsetAutomatic);
                    break;
                case LTE_FDD_DL_SCAN_HW_TYPE_UNKNOWN:
                default:
                    printf("Unknown hardware, treating like RTL-SDR\n");
                case LTE_FDD_DL_SCAN_HW_TYPE_RTL_SDR:
                    samp_src->set_sample_rate(1920000);
                    samp_src->set_gain_mode(true);
                    break;
                }
                samp_src->set_center_freq(liblte_interface_dl_earfcn_to_frequency(dl_earfcn));
                top_block->connect(samp_src, 0, state_machine, 0);
                if(0 == pthread_create(&start_thread, NULL, &run_thread, this))
                {
                    err     = LTE_FDD_DL_SCAN_STATUS_OK;
                    started = true;
                }else{
                    top_block->disconnect_all();
                }
            }else{
                samp_src.reset();
            }
        }
    }

    return(err);
}
LTE_FDD_DL_SCAN_STATUS_ENUM LTE_fdd_dl_scan_flowgraph::stop(void)
{
    boost::mutex::scoped_lock   lock(start_mutex);
    LTE_FDD_DL_SCAN_STATUS_ENUM err = LTE_FDD_DL_SCAN_STATUS_FAIL;

    if(started)
    {
        started = false;
        start_mutex.unlock();
        sleep(1); // Wait for state_machine to exit
        pthread_cancel(start_thread);
        pthread_join(start_thread, NULL);
        top_block.reset();
        err = LTE_FDD_DL_SCAN_STATUS_OK;
    }

    return(err);
}
void LTE_fdd_dl_scan_flowgraph::update_center_freq(uint16 dl_earfcn)
{
    boost::mutex::scoped_lock lock(start_mutex);

    if(started &&
       NULL != samp_src.get())
    {
        samp_src->set_center_freq(liblte_interface_dl_earfcn_to_frequency(dl_earfcn));
    }
}

// Run
void* LTE_fdd_dl_scan_flowgraph::run_thread(void *inputs)
{
    LTE_fdd_dl_scan_interface *interface = LTE_fdd_dl_scan_interface::get_instance();
    LTE_fdd_dl_scan_flowgraph *flowgraph = (LTE_fdd_dl_scan_flowgraph *)inputs;

    // Disable cancellation while running, state machine block will respond to the stop
    if(0 == pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL))
    {
        flowgraph->top_block->run(10000);

        if(0 != pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL))
        {
            printf("flowgraph: issue with enable cancel state\n");
        }
    }else{
        printf("flowgraph: issue with disable cancel state\n");
    }

    // Tear down flowgraph
    flowgraph->top_block->stop();
    flowgraph->top_block->disconnect_all();
    flowgraph->samp_src.reset();
    flowgraph->state_machine.reset();

    // Wait for flowgraph to be stopped
    if(flowgraph->is_started())
    {
        interface->send_ctrl_info_msg("scan_done");

        while(flowgraph->is_started())
        {
            sleep(1);
        }
    }

    return(NULL);
}
