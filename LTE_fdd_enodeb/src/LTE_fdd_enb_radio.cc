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

    File: LTE_fdd_enb_radio.cc

    Description: Contains all the implementations for the LTE FDD eNodeB
                 radio device control.

    Revision History
    ----------    -------------    --------------------------------------------
    11/10/2013    Ben Wojtowicz    Created file
    12/30/2013    Ben Wojtowicz    Changed the setting of the thread priority
                                   to use the uhd method and fixed a bug with
                                   baseband saturation in transmit.
    01/18/2014    Ben Wojtowicz    Handling multiple antennas, added ability to
                                   update EARFCNs, and fixed sleep time for
                                   no_rf case.
    04/12/2014    Ben Wojtowicz    Pulled in a patch from Max Suraev for more
                                   descriptive start failures.

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_enb_radio.h"
#include "LTE_fdd_enb_phy.h"
#include "liblte_interface.h"
#include <uhd/device.hpp>
#include <uhd/types/device_addr.hpp>
#include <uhd/property_tree.hpp>
#include <uhd/utils/thread_priority.hpp>

/*******************************************************************************
                              DEFINES
*******************************************************************************/

// Change this to 1 to turn on RADIO DEBUG
#define EXTRA_RADIO_DEBUG 0

/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/


/*******************************************************************************
                              GLOBAL VARIABLES
*******************************************************************************/

LTE_fdd_enb_radio* LTE_fdd_enb_radio::instance = NULL;
boost::mutex       radio_instance_mutex;

/*******************************************************************************
                              CLASS IMPLEMENTATIONS
*******************************************************************************/

/*******************/
/*    Singleton    */
/*******************/
LTE_fdd_enb_radio* LTE_fdd_enb_radio::get_instance(void)
{
    boost::mutex::scoped_lock lock(radio_instance_mutex);

    if(NULL == instance)
    {
        instance = new LTE_fdd_enb_radio();
    }

    return(instance);
}
void LTE_fdd_enb_radio::cleanup(void)
{
    boost::mutex::scoped_lock lock(radio_instance_mutex);

    if(NULL != instance)
    {
        delete instance;
        instance = NULL;
    }
}

/********************************/
/*    Constructor/Destructor    */
/********************************/
LTE_fdd_enb_radio::LTE_fdd_enb_radio()
{
    // Setup radios
    available_radios.num_radios = 0;
    get_available_radios();

    // Setup radio thread
    get_sample_rate();
    N_tx_samps = 0;
    N_rx_samps = 0;
    tx_gain    = 0;
    rx_gain    = 0;

    // Start/Stop
    started = false;
}
LTE_fdd_enb_radio::~LTE_fdd_enb_radio()
{
    stop();
}

/********************/
/*    Start/Stop    */
/********************/
bool LTE_fdd_enb_radio::is_started(void)
{
    boost::mutex::scoped_lock lock(start_mutex);

    return(started);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_radio::start(void)
{
    boost::mutex::scoped_lock  lock(start_mutex);
    LTE_fdd_enb_cnfg_db       *cnfg_db = LTE_fdd_enb_cnfg_db::get_instance();
    uhd::device_addr_t         hint;
    uhd::device_addrs_t        devs = uhd::device::find(hint);
    uhd::stream_args_t         stream_args("fc32");
    LTE_FDD_ENB_ERROR_ENUM     err = LTE_FDD_ENB_ERROR_CANT_START;
    int64                      dl_earfcn;
    int64                      ul_earfcn;

    if(false == started)
    {
        if(0 != selected_radio_idx)
        {
            started = true;
            start_mutex.unlock();
            try
            {
                // Get the DL and UL EARFCNs
                cnfg_db->get_param(LTE_FDD_ENB_PARAM_DL_EARFCN, dl_earfcn);
                cnfg_db->get_param(LTE_FDD_ENB_PARAM_UL_EARFCN, ul_earfcn);

                // Get the number of TX antennas
                cnfg_db->get_param(LTE_FDD_ENB_PARAM_N_ANT, N_ant);

                // Setup the USRP
                usrp = uhd::usrp::multi_usrp::make(devs[selected_radio_idx-1]);
                usrp->set_master_clock_rate(30720000);
                if(2.0 >= fabs(usrp->get_master_clock_rate() - 30720000.0))
                {
                    usrp->set_tx_rate(get_sample_rate());
                    usrp->set_rx_rate(get_sample_rate());
                    usrp->set_tx_freq((double)liblte_interface_dl_earfcn_to_frequency(dl_earfcn));
                    usrp->set_rx_freq((double)liblte_interface_ul_earfcn_to_frequency(ul_earfcn));
                    usrp->set_tx_gain(tx_gain);
                    usrp->set_rx_gain(rx_gain);

                    // Setup the TX and RX streams
                    tx_stream  = usrp->get_tx_stream(stream_args);
                    rx_stream  = usrp->get_rx_stream(stream_args);
                    N_tx_samps = tx_stream->get_max_num_samps();
                    N_rx_samps = rx_stream->get_max_num_samps();
                    if(N_rx_samps > LIBLTE_PHY_N_SAMPS_PER_SUBFR_1_92MHZ &&
                       N_tx_samps > LIBLTE_PHY_N_SAMPS_PER_SUBFR_1_92MHZ)
                    {
                        N_rx_samps = LIBLTE_PHY_N_SAMPS_PER_SUBFR_1_92MHZ;
                        N_tx_samps = LIBLTE_PHY_N_SAMPS_PER_SUBFR_1_92MHZ;

                        // Kick off the receiving thread
                        pthread_create(&radio_thread, NULL, &radio_thread_func, NULL);

                        err = LTE_FDD_ENB_ERROR_NONE;
                    }else{
                        started = false;
                    }
                }else{
                    started = false;
                    err     = LTE_FDD_ENB_ERROR_MASTER_CLOCK_FAIL;
                }
            }catch(...){
                started = false;
                return(err);
            }
        }else{
            // Kick off the receiving thread
            pthread_create(&radio_thread, NULL, &radio_thread_func, NULL);

            err     = LTE_FDD_ENB_ERROR_NONE;
            started = true;
        }
    }

    return(err);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_radio::stop(void)
{
    boost::mutex::scoped_lock lock(start_mutex);
    uhd::stream_cmd_t         cmd = uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS;
    LTE_FDD_ENB_ERROR_ENUM    err = LTE_FDD_ENB_ERROR_CANT_STOP;

    if(started)
    {
        started = false;
        if(0 != selected_radio_idx)
        {
            usrp->issue_stream_cmd(cmd);
        }
        sleep(1);
        pthread_cancel(radio_thread);
        pthread_join(radio_thread, NULL);
        err = LTE_FDD_ENB_ERROR_NONE;
    }

    return(err);
}

/****************************/
/*    External Interface    */
/****************************/
LTE_FDD_ENB_AVAILABLE_RADIOS_STRUCT LTE_fdd_enb_radio::get_available_radios(void)
{
    uhd::device_addr_t  hint;
    uhd::device_addrs_t devs = uhd::device::find(hint);
    size_t              i;
    uint32              orig_num_radios = available_radios.num_radios;

    available_radios.num_radios = 0;
    available_radios.radio[available_radios.num_radios++].name = "no_rf";
    for(i=0; i<devs.size(); i++)
    {
        available_radios.radio[available_radios.num_radios++].name = devs[i].to_string();
    }

    if(orig_num_radios != available_radios.num_radios)
    {
        if(1 == available_radios.num_radios)
        {
            selected_radio_idx = 0;
        }else{
            selected_radio_idx = 1;
        }
    }

    return(available_radios);
}
LTE_FDD_ENB_RADIO_STRUCT LTE_fdd_enb_radio::get_selected_radio(void)
{
    uhd::device_addr_t  hint;
    uhd::device_addrs_t devs = uhd::device::find(hint);

    if(available_radios.num_radios != devs.size()+1)
    {
        get_available_radios();
    }

    return(available_radios.radio[selected_radio_idx]);
}
uint32 LTE_fdd_enb_radio::get_selected_radio_idx(void)
{
    uhd::device_addr_t  hint;
    uhd::device_addrs_t devs = uhd::device::find(hint);

    if(available_radios.num_radios != devs.size()+1)
    {
        get_available_radios();
    }

    return(selected_radio_idx);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_radio::set_selected_radio_idx(uint32 idx)
{
    uhd::device_addr_t     hint;
    uhd::device_addrs_t    devs = uhd::device::find(hint);
    LTE_FDD_ENB_ERROR_ENUM err  = LTE_FDD_ENB_ERROR_OUT_OF_BOUNDS;

    if(available_radios.num_radios != devs.size()+1)
    {
        get_available_radios();
    }

    if(idx < available_radios.num_radios)
    {
        selected_radio_idx = idx;
        err                = LTE_FDD_ENB_ERROR_NONE;
    }

    return(err);
}
uint32 LTE_fdd_enb_radio::get_tx_gain(void)
{
    boost::mutex::scoped_lock lock(start_mutex);
    uint32                    gain;

    if(!started)
    {
        gain = tx_gain;
    }else{
        if(0 == selected_radio_idx)
        {
            gain = tx_gain;
        }else{
            gain = (uint32)usrp->get_tx_gain();
        }
    }

    return(gain);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_radio::set_tx_gain(uint32 gain)
{
    boost::mutex::scoped_lock lock(start_mutex);

    tx_gain = gain;
    if(started)
    {
        if(0 != selected_radio_idx)
        {
            usrp->set_tx_gain(gain);
        }
    }

    return(LTE_FDD_ENB_ERROR_NONE);
}
uint32 LTE_fdd_enb_radio::get_rx_gain(void)
{
    boost::mutex::scoped_lock lock(start_mutex);
    uint32                    gain;

    if(!started)
    {
        gain = rx_gain;
    }else{
        if(0 == selected_radio_idx)
        {
            gain = rx_gain;
        }else{
            gain = (uint32)usrp->get_rx_gain();
        }
    }

    return(gain);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_radio::set_rx_gain(uint32 gain)
{
    boost::mutex::scoped_lock lock(start_mutex);

    rx_gain = gain;
    if(started)
    {
        if(0 != selected_radio_idx)
        {
            usrp->set_rx_gain(gain);
        }
    }

    return(LTE_FDD_ENB_ERROR_NONE);
}
uint32 LTE_fdd_enb_radio::get_sample_rate(void)
{
    boost::mutex::scoped_lock  lock(start_mutex);
    LTE_fdd_enb_cnfg_db       *cnfg_db = LTE_fdd_enb_cnfg_db::get_instance();
    int64                      dl_bw;

    if(!started)
    {
        cnfg_db->get_param(LTE_FDD_ENB_PARAM_DL_BW, dl_bw);
        switch(dl_bw)
        {
        case LIBLTE_RRC_DL_BANDWIDTH_100:
            // Intentional fall-thru
        case LIBLTE_RRC_DL_BANDWIDTH_75:
            fs                = 30720000;
            N_samps_per_subfr = LIBLTE_PHY_N_SAMPS_PER_SUBFR_30_72MHZ;
            break;
        case LIBLTE_RRC_DL_BANDWIDTH_50:
            fs                = 15360000;
            N_samps_per_subfr = LIBLTE_PHY_N_SAMPS_PER_SUBFR_15_36MHZ;
            break;
        case LIBLTE_RRC_DL_BANDWIDTH_25:
            fs                = 7680000;
            N_samps_per_subfr = LIBLTE_PHY_N_SAMPS_PER_SUBFR_7_68MHZ;
            break;
        case LIBLTE_RRC_DL_BANDWIDTH_15:
            fs                = 3840000;
            N_samps_per_subfr = LIBLTE_PHY_N_SAMPS_PER_SUBFR_3_84MHZ;
            break;
        case LIBLTE_RRC_DL_BANDWIDTH_6:
            // Intentional fall-thru
        default:
            fs                = 1920000;
            N_samps_per_subfr = LIBLTE_PHY_N_SAMPS_PER_SUBFR_1_92MHZ;
            break;
        }
    }

    return(fs);
}
void LTE_fdd_enb_radio::set_earfcns(int64 dl_earfcn,
                                    int64 ul_earfcn)
{
    if(started &&
       0 != selected_radio_idx)
    {
        usrp->set_tx_freq((double)liblte_interface_dl_earfcn_to_frequency(dl_earfcn));
        usrp->set_rx_freq((double)liblte_interface_ul_earfcn_to_frequency(ul_earfcn));
    }
}
void LTE_fdd_enb_radio::send(LTE_FDD_ENB_RADIO_TX_BUF_STRUCT *buf)
{
#if EXTRA_RADIO_DEBUG
    LTE_fdd_enb_interface *interface = LTE_fdd_enb_interface::get_instance();
#endif
    uhd::tx_metadata_t     metadata;
    uint32                 samps_to_send = N_samps_per_subfr;
    uint32                 idx           = 0;
    uint32                 i;
    uint32                 p;
    uint16                 N_skipped_subfrs;

    if(0 != selected_radio_idx)
    {
        // Setup metadata
        metadata.has_time_spec  = true;
        metadata.start_of_burst = false;
        metadata.end_of_burst   = false;

        // Check fn_combo
        if(buf->fn_combo != next_tx_fn_combo)
        {
            if(buf->fn_combo > next_tx_fn_combo)
            {
                N_skipped_subfrs = buf->fn_combo - next_tx_fn_combo;
            }else{
                N_skipped_subfrs = (buf->fn_combo + LTE_FDD_ENB_FN_COMBO_MAX + 1) - next_tx_fn_combo;
            }

            next_tx_ts       += uhd::time_spec_t::from_ticks(N_skipped_subfrs*N_samps_per_subfr, fs);
            next_tx_fn_combo  = (buf->fn_combo + 1) % (LTE_FDD_ENB_FN_COMBO_MAX + 1);
        }else{
            next_tx_fn_combo = (next_tx_fn_combo + 1) % (LTE_FDD_ENB_FN_COMBO_MAX + 1);
        }

        while(samps_to_send > N_tx_samps)
        {
            metadata.time_spec = next_tx_ts;
            for(i=0; i<N_tx_samps; i++)
            {
                tx_buf[i] = gr_complex(buf->i_buf[0][idx+i]/50.0, buf->q_buf[0][idx+i]/50.0);
                for(p=1; p<N_ant; p++)
                {
                    tx_buf[i] += gr_complex(buf->i_buf[p][idx+i]/50.0, buf->q_buf[p][idx+i]/50.0);
                }
                tx_buf[i] /= N_ant;
            }
#if EXTRA_RADIO_DEBUG
            interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                      LTE_FDD_ENB_DEBUG_LEVEL_RADIO,
                                      __FILE__,
                                      __LINE__,
                                      "Sending subfr %lld %u",
                                      metadata.time_spec.to_ticks(fs),
                                      buf->fn_combo);
#endif
            tx_stream->send(tx_buf, N_tx_samps, metadata);
            idx           += N_tx_samps;
            samps_to_send -= N_tx_samps;
            next_tx_ts    += uhd::time_spec_t::from_ticks(N_tx_samps, fs);
        }
        if(0 != samps_to_send)
        {
            metadata.time_spec = next_tx_ts;
            for(i=0; i<samps_to_send; i++)
            {
                tx_buf[i] = gr_complex(buf->i_buf[0][idx+i]/50.0, buf->q_buf[0][idx+i]/50.0);
                for(p=1; p<N_ant; p++)
                {
                    tx_buf[i] += gr_complex(buf->i_buf[p][idx+i]/50.0, buf->q_buf[p][idx+i]/50.0);
                }
                tx_buf[i] /= N_ant;
            }
#if EXTRA_RADIO_DEBUG
            interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                      LTE_FDD_ENB_DEBUG_LEVEL_RADIO,
                                      __FILE__,
                                      __LINE__,
                                      "Sending subfr %lld %u",
                                      metadata.time_spec.to_ticks(fs),
                                      buf->fn_combo);
#endif
            tx_stream->send(tx_buf, samps_to_send, metadata);
            next_tx_ts += uhd::time_spec_t::from_ticks(samps_to_send, fs);
        }
    }
}

/**********************/
/*    Radio Thread    */
/**********************/
void* LTE_fdd_enb_radio::radio_thread_func(void *inputs)
{
    LTE_fdd_enb_interface           *interface = LTE_fdd_enb_interface::get_instance();
    LTE_fdd_enb_phy                 *phy       = LTE_fdd_enb_phy::get_instance();
    LTE_fdd_enb_radio               *radio     = LTE_fdd_enb_radio::get_instance();
    LTE_FDD_ENB_RADIO_TX_BUF_STRUCT  tx_radio_buf[2];
    LTE_FDD_ENB_RADIO_RX_BUF_STRUCT  rx_radio_buf[2];
    struct timespec                  sleep_time;
    struct timespec                  time_rem;
    struct sched_param               priority;
    uhd::rx_metadata_t               metadata;
    uhd::time_spec_t                 next_rx_ts;
    uhd::time_spec_t                 next_rx_subfr_ts;
    uhd::time_spec_t                 check_ts;
    uhd::stream_cmd_t                cmd = uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS;
    int64                            next_rx_ts_ticks;
    int64                            metadata_ts_ticks;
    uint32                           i;
    uint32                           N_subfrs_dropped;
    uint32                           recv_size   = radio->N_rx_samps;
    uint32                           samp_rate   = radio->fs;
    uint32                           buf_idx     = 0;
    uint32                           recv_idx    = 0;
    uint32                           samp_idx    = 0;
    uint32                           num_samps   = 0;
    uint32                           radio_idx   = radio->get_selected_radio_idx();
    uint16                           rx_fn_combo = (LTE_FDD_ENB_FN_COMBO_MAX + 1) - 2;
    bool                             not_done    = true;
    bool                             init_needed = true;
    bool                             rx_synced   = false;

    // Set highest priority
    priority.sched_priority = 99;
    pthread_setschedparam(radio->radio_thread, SCHED_FIFO, &priority);

    // Setup sleep time for no_rf device
    sleep_time.tv_sec  = 0;
    sleep_time.tv_nsec = 1000000;

    while(not_done &&
          radio->is_started())
    {
        if(0 == radio_idx)
        {
            if(init_needed)
            {
                // Signal PHY to generate first subframe
                phy->radio_interface(&tx_radio_buf[1]);
                init_needed = false;
            }
            rx_radio_buf[buf_idx].fn_combo = rx_fn_combo;
            phy->radio_interface(&tx_radio_buf[buf_idx], &rx_radio_buf[buf_idx]);
            buf_idx     = (buf_idx + 1) % 2;
            rx_fn_combo = (rx_fn_combo + 1) % (LTE_FDD_ENB_FN_COMBO_MAX + 1);
            nanosleep(&sleep_time, &time_rem);
        }else{
            if(init_needed)
            {
                // Setup time specs
                radio->next_tx_ts  = uhd::time_spec_t::from_ticks(samp_rate, samp_rate); // 1 second to make sure everything is setup
                next_rx_ts         = radio->next_tx_ts;
                next_rx_ts        -= uhd::time_spec_t::from_ticks(radio->N_samps_per_subfr*2, samp_rate); // Retard RX by 2 subframes
                next_rx_subfr_ts   = next_rx_ts;

                // Reset USRP time
                radio->usrp->set_time_now(uhd::time_spec_t::from_ticks(0, samp_rate));

                // Signal PHY to generate first subframe
                phy->radio_interface(&tx_radio_buf[1]);

                // Start streaming
                cmd.stream_now = true;
                radio->usrp->issue_stream_cmd(cmd);

                init_needed = false;
            }

            if(!rx_synced)
            {
                num_samps  = radio->rx_stream->recv(radio->rx_buf, recv_size, metadata);
                check_ts   = metadata.time_spec;
                check_ts  += uhd::time_spec_t::from_ticks(num_samps, samp_rate);

                if(check_ts.to_ticks(samp_rate) == next_rx_ts.to_ticks(samp_rate))
                {
                    interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                              LTE_FDD_ENB_DEBUG_LEVEL_RADIO,
                                              __FILE__,
                                              __LINE__,
                                              "RX synced %lld %lld",
                                              check_ts.to_ticks(samp_rate),
                                              next_rx_ts.to_ticks(samp_rate));
                    samp_idx  = 0;
                    rx_synced = true;
                }else{
                    check_ts += uhd::time_spec_t::from_ticks(radio->N_rx_samps, samp_rate);
                    if(check_ts.to_ticks(samp_rate) > next_rx_ts.to_ticks(samp_rate))
                    {
                        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                                  LTE_FDD_ENB_DEBUG_LEVEL_RADIO,
                                                  __FILE__,
                                                  __LINE__,
                                                  "RX modifying recv_size to sync %lld %lld",
                                                  check_ts.to_ticks(samp_rate),
                                                  next_rx_ts.to_ticks(samp_rate));
                        check_ts  -= uhd::time_spec_t::from_ticks(radio->N_rx_samps, samp_rate);
                        recv_size  = (uint32)(next_rx_ts.to_ticks(samp_rate) - check_ts.to_ticks(samp_rate));
                    }
                }
            }else{
                num_samps = radio->rx_stream->recv(radio->rx_buf, radio->N_rx_samps, metadata);
                if(0 != num_samps)
                {
                    if(num_samps != radio->N_rx_samps)
                    {
                        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                                  LTE_FDD_ENB_DEBUG_LEVEL_RADIO,
                                                  __FILE__,
                                                  __LINE__,
                                                  "RX packet size issue %u %u",
                                                  num_samps,
                                                  radio->N_rx_samps);
                    }
                    next_rx_ts_ticks  = next_rx_ts.to_ticks(samp_rate);
                    metadata_ts_ticks = metadata.time_spec.to_ticks(samp_rate);
                    if((next_rx_ts_ticks - metadata_ts_ticks) > 1)
                    {
                        // FIXME: Not sure this will ever happen
                        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                                  LTE_FDD_ENB_DEBUG_LEVEL_RADIO,
                                                  __FILE__,
                                                  __LINE__,
                                                  "RX old time spec %lld %lld",
                                                  metadata_ts_ticks,
                                                  next_rx_ts_ticks);
                    }else if((metadata_ts_ticks - next_rx_ts_ticks) > 1){
                        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                                  LTE_FDD_ENB_DEBUG_LEVEL_RADIO,
                                                  __FILE__,
                                                  __LINE__,
                                                  "RX overrun %lld %lld",
                                                  metadata_ts_ticks,
                                                  next_rx_ts_ticks);

                        // Determine how many subframes we are going to drop
                        N_subfrs_dropped = ((metadata_ts_ticks - next_rx_ts_ticks)/radio->N_samps_per_subfr) + 2;

                        // Jump the rx_fn_combo
                        rx_fn_combo = (rx_fn_combo + N_subfrs_dropped) % (LTE_FDD_ENB_FN_COMBO_MAX + 1);

                        // Align the samples coming from the radio
                        rx_synced         = false;
                        next_rx_subfr_ts += uhd::time_spec_t::from_ticks(N_subfrs_dropped*radio->N_samps_per_subfr, samp_rate);
                        next_rx_ts        = next_rx_subfr_ts;
                        check_ts          = metadata.time_spec;
                        check_ts         += uhd::time_spec_t::from_ticks(num_samps + radio->N_rx_samps, samp_rate);

                        if(check_ts.to_ticks(samp_rate) > next_rx_ts.to_ticks(samp_rate))
                        {
                            check_ts  -= uhd::time_spec_t::from_ticks(radio->N_rx_samps, samp_rate);
                            recv_size  = (uint32)(next_rx_ts.to_ticks(samp_rate) - check_ts.to_ticks(samp_rate));
                        }else{
                            recv_size = radio->N_rx_samps;
                        }
                    }else{
                        // FIXME: May need to realign to 1920 sample boundries
                        recv_idx    = 0;
                        next_rx_ts += uhd::time_spec_t::from_ticks(num_samps, samp_rate);
                        while(num_samps > 0)
                        {
                            if((samp_idx + num_samps) <= radio->N_samps_per_subfr)
                            {
                                for(i=0; i<num_samps; i++)
                                {
                                    rx_radio_buf[buf_idx].i_buf[samp_idx+i] = radio->rx_buf[recv_idx+i].real();
                                    rx_radio_buf[buf_idx].q_buf[samp_idx+i] = radio->rx_buf[recv_idx+i].imag();
                                }
                                samp_idx += num_samps;

                                if(samp_idx == radio->N_samps_per_subfr)
                                {
#if EXTRA_RADIO_DEBUG
                                    interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                                              LTE_FDD_ENB_DEBUG_LEVEL_RADIO,
                                                              __FILE__,
                                                              __LINE__,
                                                              "Receiving subfr %lld %u",
                                                              next_rx_subfr_ts.to_ticks(samp_rate),
                                                              rx_fn_combo);
#endif
                                    rx_radio_buf[buf_idx].fn_combo = rx_fn_combo;
                                    phy->radio_interface(&tx_radio_buf[buf_idx], &rx_radio_buf[buf_idx]);
                                    buf_idx           = (buf_idx + 1) % 2;
                                    rx_fn_combo       = (rx_fn_combo + 1) % (LTE_FDD_ENB_FN_COMBO_MAX + 1);
                                    samp_idx          = 0;
                                    next_rx_subfr_ts += uhd::time_spec_t::from_ticks(radio->N_samps_per_subfr, samp_rate);
                                }
                                num_samps = 0;
                            }else{
                                for(i=0; i<(radio->N_samps_per_subfr - samp_idx); i++)
                                {
                                    rx_radio_buf[buf_idx].i_buf[samp_idx+i] = radio->rx_buf[recv_idx+i].real();
                                    rx_radio_buf[buf_idx].q_buf[samp_idx+i] = radio->rx_buf[recv_idx+i].imag();
                                }

#if EXTRA_RADIO_DEBUG
                                interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                                          LTE_FDD_ENB_DEBUG_LEVEL_RADIO,
                                                          __FILE__,
                                                          __LINE__,
                                                          "Receiving subfr %lld %u",
                                                          next_rx_subfr_ts.to_ticks(samp_rate),
                                                          rx_fn_combo);
#endif
                                rx_radio_buf[buf_idx].fn_combo = rx_fn_combo;
                                phy->radio_interface(&tx_radio_buf[buf_idx], &rx_radio_buf[buf_idx]);
                                buf_idx           = (buf_idx + 1) % 2;
                                rx_fn_combo       = (rx_fn_combo + 1) % (LTE_FDD_ENB_FN_COMBO_MAX + 1);
                                num_samps        -= (radio->N_samps_per_subfr - samp_idx);
                                recv_idx          = (radio->N_samps_per_subfr - samp_idx);
                                samp_idx          = 0;
                                next_rx_subfr_ts += uhd::time_spec_t::from_ticks(radio->N_samps_per_subfr, samp_rate);
                            }
                        }
                    }
                }else{
                    interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                              LTE_FDD_ENB_DEBUG_LEVEL_RADIO,
                                              __FILE__,
                                              __LINE__,
                                              "RX error %u",
                                              (uint32)metadata.error_code);
                }
            }
        }
    }
}
