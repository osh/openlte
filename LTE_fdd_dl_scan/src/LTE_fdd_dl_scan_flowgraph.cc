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

    File: LTE_fdd_dl_scan_flowgraph.cc

    Description: Contains all the implementations for the LTE FDD DL Scanner
                 gnuradio flowgraph.

    Revision History
    ----------    -------------    --------------------------------------------
    02/26/2013    Ben Wojtowicz    Created file
    07/21/2013    Ben Wojtowicz    Added support for HackRF Jawbreaker
    08/26/2013    Ben Wojtowicz    Updates to support GnuRadio 3.7 and added a
                                   new hardware discovery mechanism.
    11/13/2013    Ben Wojtowicz    Added support for USRP B2X0.
    11/30/2013    Ben Wojtowicz    Added support for bladeRF.
    04/12/2014    Ben Wojtowicz    Pulled in a patch from Jevgenij for
                                   supporting non-B2X0 USRPs.

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_dl_scan_flowgraph.h"
#include "uhd/usrp/multi_usrp.hpp"

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
    LTE_fdd_dl_scan_interface    *interface = LTE_fdd_dl_scan_interface::get_instance();
    uhd::device_addr_t            hint;
    LTE_FDD_DL_SCAN_STATUS_ENUM   err           = LTE_FDD_DL_SCAN_STATUS_FAIL;
    LTE_FDD_DL_SCAN_HW_TYPE_ENUM  hardware_type = LTE_FDD_DL_SCAN_HW_TYPE_UNKNOWN;
    double                        mcr;

    if(!started)
    {
        if(NULL == top_block.get())
        {
            top_block = gr::make_top_block("top");
        }
        if(NULL == samp_src.get())
        {
            osmosdr::source::sptr tmp_src0 = osmosdr::source::make("uhd");
            BOOST_FOREACH(const uhd::device_addr_t &dev, uhd::device::find(hint))
            {
                uhd::usrp::multi_usrp::make(dev)->set_master_clock_rate(30720000);
                mcr = uhd::usrp::multi_usrp::make(dev)->get_master_clock_rate();
            }
            if(0 != tmp_src0->get_sample_rates().size() &&
               1 >= fabs(mcr - 30720000))
            {
                hardware_type = LTE_FDD_DL_SCAN_HW_TYPE_USRP_B;
                samp_src      = tmp_src0;
            }else{
                BOOST_FOREACH(const uhd::device_addr_t &dev, uhd::device::find(hint))
                {
                    uhd::usrp::multi_usrp::make(dev)->set_master_clock_rate(100000000);
                    mcr = uhd::usrp::multi_usrp::make(dev)->get_master_clock_rate();
                }
                if(0 != tmp_src0->get_sample_rates().size() &&
                   1 >= fabs(mcr - 100000000))
                {
                    hardware_type = LTE_FDD_DL_SCAN_HW_TYPE_USRP_N;
                    samp_src      = tmp_src0;
                }else{
                    osmosdr::source::sptr tmp_src1 = osmosdr::source::make("hackrf");
                    if(0 != tmp_src1->get_sample_rates().size())
                    {
                        hardware_type = LTE_FDD_DL_SCAN_HW_TYPE_HACKRF;
                        samp_src      = tmp_src1;
                    }else{
                        osmosdr::source::sptr tmp_src2 = osmosdr::source::make("bladerf");
                        if(0 != tmp_src2->get_sample_rates().size())
                        {
                            hardware_type = LTE_FDD_DL_SCAN_HW_TYPE_BLADERF;
                            samp_src      = tmp_src2;
                        }else{
                            osmosdr::source::sptr tmp_src3 = osmosdr::source::make("rtl=0");
                            if(0 != tmp_src3->get_sample_rates().size())
                            {
                                hardware_type = LTE_FDD_DL_SCAN_HW_TYPE_RTL_SDR;
                                samp_src      = tmp_src3;
                            }else{
                                samp_src = osmosdr::source::make();
                            }
                        }
                    }
                }
            }
        }
        if(NULL == state_machine.get())
        {
            switch(hardware_type)
            {
            case LTE_FDD_DL_SCAN_HW_TYPE_USRP_B:
                state_machine = LTE_fdd_dl_scan_make_state_machine(15360000);
                break;
            case LTE_FDD_DL_SCAN_HW_TYPE_USRP_N:
                state_machine = LTE_fdd_dl_scan_make_state_machine(15360000);
                break;
            case LTE_FDD_DL_SCAN_HW_TYPE_HACKRF:
                state_machine = LTE_fdd_dl_scan_make_state_machine(15360000);
                break;
            case LTE_FDD_DL_SCAN_HW_TYPE_BLADERF:
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
                case LTE_FDD_DL_SCAN_HW_TYPE_USRP_B:
                    samp_src->set_sample_rate(15360000);
                    samp_src->set_gain_mode(false);
                    samp_src->set_gain(35);
                    samp_src->set_bandwidth(10000000);
                    break;
                case LTE_FDD_DL_SCAN_HW_TYPE_USRP_N:
                    samp_src->set_sample_rate(25000000);
                    samp_src->set_gain_mode(false);
                    samp_src->set_gain(35);
                    samp_src->set_bandwidth(10000000);
                    break;
                case LTE_FDD_DL_SCAN_HW_TYPE_HACKRF:
                    samp_src->set_sample_rate(15360000);
                    samp_src->set_gain_mode(false);
                    samp_src->set_gain(14);
                    samp_src->set_dc_offset_mode(osmosdr::source::DCOffsetAutomatic);
                    break;
                case LTE_FDD_DL_SCAN_HW_TYPE_BLADERF:
                    samp_src->set_sample_rate(15360000);
                    samp_src->set_gain_mode(false);
                    samp_src->set_gain(6, "LNA");
                    samp_src->set_gain(33, "VGA1");
                    samp_src->set_gain(3, "VGA2");
                    samp_src->set_bandwidth(10000000);
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

                if(LTE_FDD_DL_SCAN_HW_TYPE_USRP_N == hardware_type)
                {
                    resample_taps    = gr::filter::firdes::low_pass(384, 1, 0.00065, 0.0013, gr::filter::firdes::WIN_KAISER, 5);
                    resampler_filter = gr::filter::rational_resampler_base_ccf::make(384, 625, resample_taps);
                    top_block->connect(samp_src, 0, resampler_filter, 0);
                    top_block->connect(resampler_filter, 0, state_machine, 0);
                }else{
                    top_block->connect(samp_src, 0, state_machine, 0);
                }

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
