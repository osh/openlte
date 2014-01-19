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

    File: LTE_fdd_enb_radio.h

    Description: Contains all the definitions for the LTE FDD eNodeB
                 radio device control.

    Revision History
    ----------    -------------    --------------------------------------------
    11/09/2013    Ben Wojtowicz    Created file
    01/18/2014    Ben Wojtowicz    Handling EARFCN updates and multiple
                                   antennas.

*******************************************************************************/

#ifndef __LTE_FDD_ENB_RADIO_H__
#define __LTE_FDD_ENB_RADIO_H__

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_enb_interface.h"
#include "liblte_phy.h"
#include <gnuradio/gr_complex.h>
#include <uhd/usrp/multi_usrp.hpp>
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

typedef struct{
    std::string name;
}LTE_FDD_ENB_RADIO_STRUCT;

typedef struct{
    LTE_FDD_ENB_RADIO_STRUCT radio[100];
    uint32                   num_radios;
}LTE_FDD_ENB_AVAILABLE_RADIOS_STRUCT;

typedef struct{
    float  i_buf[4][LIBLTE_PHY_N_SAMPS_PER_SUBFR_30_72MHZ];
    float  q_buf[4][LIBLTE_PHY_N_SAMPS_PER_SUBFR_30_72MHZ];
    uint16 fn_combo;
}LTE_FDD_ENB_RADIO_TX_BUF_STRUCT;

typedef struct{
    float  i_buf[LIBLTE_PHY_N_SAMPS_PER_SUBFR_30_72MHZ];
    float  q_buf[LIBLTE_PHY_N_SAMPS_PER_SUBFR_30_72MHZ];
    uint16 fn_combo;
}LTE_FDD_ENB_RADIO_RX_BUF_STRUCT;

/*******************************************************************************
                              CLASS DECLARATIONS
*******************************************************************************/

class LTE_fdd_enb_radio
{
public:
    // Singleton
    static LTE_fdd_enb_radio* get_instance(void);
    static void cleanup(void);

    // Start/Stop
    bool is_started(void);
    LTE_FDD_ENB_ERROR_ENUM start(void);
    LTE_FDD_ENB_ERROR_ENUM stop(void);

    // External interface
    LTE_FDD_ENB_AVAILABLE_RADIOS_STRUCT get_available_radios(void);
    LTE_FDD_ENB_RADIO_STRUCT get_selected_radio(void);
    uint32 get_selected_radio_idx(void);
    LTE_FDD_ENB_ERROR_ENUM set_selected_radio_idx(uint32 idx);
    uint32 get_tx_gain(void);
    LTE_FDD_ENB_ERROR_ENUM set_tx_gain(uint32 gain);
    uint32 get_rx_gain(void);
    LTE_FDD_ENB_ERROR_ENUM set_rx_gain(uint32 gain);
    uint32 get_sample_rate(void);
    void set_earfcns(int64 dl_earfcn, int64 ul_earfcn);
    void send(LTE_FDD_ENB_RADIO_TX_BUF_STRUCT *buf);

private:
    // Singleton
    static LTE_fdd_enb_radio *instance;
    LTE_fdd_enb_radio();
    ~LTE_fdd_enb_radio();

    // Start/Stop
    boost::mutex start_mutex;
    bool         started;

    // Radios
    uhd::usrp::multi_usrp::sptr         usrp;
    uhd::tx_streamer::sptr              tx_stream;
    uhd::rx_streamer::sptr              rx_stream;
    LTE_FDD_ENB_AVAILABLE_RADIOS_STRUCT available_radios;
    uint32                              selected_radio_idx;

    // Radio thread
    static void*     radio_thread_func(void *inputs);
    pthread_t        radio_thread;
    gr_complex       tx_buf[LIBLTE_PHY_N_SAMPS_PER_SUBFR_30_72MHZ];
    gr_complex       rx_buf[LIBLTE_PHY_N_SAMPS_PER_SUBFR_30_72MHZ];
    uhd::time_spec_t next_tx_ts;
    int64            N_ant;
    uint32           N_tx_samps;
    uint32           N_rx_samps;
    uint32           N_samps_per_subfr;
    uint32           fs;
    uint32           tx_gain;
    uint32           rx_gain;
    uint16           next_tx_fn_combo;
};

#endif /* __LTE_FDD_ENB_RADIO_H__ */
