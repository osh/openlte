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

    File: LTE_fdd_dl_scan_state_machine.cc

    Description: Contains all the implementations for the LTE FDD DL Scanner
                 state machine block.

    Revision History
    ----------    -------------    --------------------------------------------
    02/26/2013    Ben Wojtowicz    Created file

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_dl_scan_state_machine.h"
#include "LTE_fdd_dl_scan_flowgraph.h"
#include "gr_io_signature.h"

/*******************************************************************************
                              DEFINES
*******************************************************************************/

#define ONE_SUBFRAME_NUM_SAMPS               (LIBLTE_PHY_N_SAMPS_PER_SUBFR_1_92MHZ)
#define ONE_FRAME_NUM_SAMPS                  (10 * ONE_SUBFRAME_NUM_SAMPS)
#define FREQ_CHANGE_WAIT_NUM_SAMPS           (100 * ONE_FRAME_NUM_SAMPS)
#define COARSE_TIMING_N_SLOTS                (80)
#define COARSE_TIMING_SEARCH_NUM_SAMPS       (((COARSE_TIMING_N_SLOTS/2)+2) * ONE_SUBFRAME_NUM_SAMPS)
#define PSS_AND_FINE_TIMING_SEARCH_NUM_SAMPS (COARSE_TIMING_SEARCH_NUM_SAMPS)
#define SSS_SEARCH_NUM_SAMPS                 (COARSE_TIMING_SEARCH_NUM_SAMPS)
#define BCH_DECODE_NUM_SAMPS                 (2 * ONE_FRAME_NUM_SAMPS)
#define PDSCH_DECODE_SIB1_NUM_SAMPS          (2 * ONE_FRAME_NUM_SAMPS)
#define PDSCH_DECODE_SI_GENERIC_NUM_SAMPS    (ONE_FRAME_NUM_SAMPS)
#define MAX_ATTEMPTS                         (5)
#define SAMP_BUF_SIZE                        (307200*10)

/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/


/*******************************************************************************
                              GLOBAL VARIABLES
*******************************************************************************/

// minimum and maximum number of input and output streams
static const int32 MIN_IN  = 1;
static const int32 MAX_IN  = 1;
static const int32 MIN_OUT = 0;
static const int32 MAX_OUT = 0;

/*******************************************************************************
                              CLASS IMPLEMENTATIONS
*******************************************************************************/

LTE_fdd_dl_scan_state_machine_sptr LTE_fdd_dl_scan_make_state_machine()
{
    return LTE_fdd_dl_scan_state_machine_sptr(new LTE_fdd_dl_scan_state_machine());
}

LTE_fdd_dl_scan_state_machine::LTE_fdd_dl_scan_state_machine()
    : gr_sync_block ("LTE_fdd_dl_scan_state_machine",
                     gr_make_io_signature(MIN_IN,  MAX_IN,  sizeof(gr_complex)),
                     gr_make_io_signature(MIN_OUT, MAX_OUT, sizeof(gr_complex)))
{
    uint32 i;

    // Initialize the LTE library
    liblte_phy_init(&phy_struct,
                    LIBLTE_PHY_FS_1_92MHZ,
                    LIBLTE_PHY_INIT_N_ID_CELL_UNKNOWN,
                    4,
                    LIBLTE_PHY_N_RB_DL_1_4MHZ,
                    LIBLTE_PHY_N_SC_RB_NORMAL_CP,
                    liblte_rrc_phich_resource_num[LIBLTE_RRC_PHICH_RESOURCE_1]);

    // Initialize the sample buffer
    i_buf          = (float *)malloc(SAMP_BUF_SIZE*sizeof(float));
    q_buf          = (float *)malloc(SAMP_BUF_SIZE*sizeof(float));
    samp_buf_w_idx = 0;
    samp_buf_r_idx = 0;

    // Variables
    init();
    send_cnf        = true;
    N_decoded_chans = 0;
    corr_peak_idx   = 0;
    for(i=0; i<LIBLTE_PHY_N_MAX_ROUGH_CORR_SEARCH_PEAKS; i++)
    {
        timing_struct.freq_offset[i] = 0;
    }
}
LTE_fdd_dl_scan_state_machine::~LTE_fdd_dl_scan_state_machine()
{
    // Cleanup the LTE library
    liblte_phy_cleanup(phy_struct);

    // Free the sample buffer
    free(i_buf);
    free(q_buf);
}

int32 LTE_fdd_dl_scan_state_machine::work(int32                      ninput_items,
                                          gr_vector_const_void_star &input_items,
                                          gr_vector_void_star       &output_items)
{
    LTE_fdd_dl_scan_flowgraph  *flowgraph = LTE_fdd_dl_scan_flowgraph::get_instance();
    LIBLTE_PHY_SUBFRAME_STRUCT  subframe;
    LIBLTE_PHY_PCFICH_STRUCT    pcfich;
    LIBLTE_PHY_PHICH_STRUCT     phich;
    LIBLTE_PHY_PDCCH_STRUCT     pdcch;
    const gr_complex           *in  = (const gr_complex *)input_items[0];
    float                       pss_thresh;
    float                       freq_offset;
    int32                       done_flag = 0;
    uint32                      i;
    uint32                      pss_symb;
    uint32                      frame_start_idx;
    uint32                      samps_to_copy;
    uint32                      N_rb_dl;
    uint8                       sfn_offset;
    bool                        process_samples = false;
    bool                        copy_input      = false;
    bool                        switch_freq     = false;

    if(freq_change_wait_done)
    {
        if(samp_buf_w_idx < (SAMP_BUF_SIZE-(ninput_items+1)))
        {
            copy_input_to_samp_buf(in, ninput_items);

            // Check if buffer is full enough
            if(samp_buf_w_idx >= (SAMP_BUF_SIZE-(ninput_items+1)))
            {
                process_samples = true;
                copy_input      = false;
            }
        }else{
            // Buffer is too full process samples, then copy
            process_samples = true;
            copy_input      = true;
        }
    }else{
        freq_change_wait_cnt += ninput_items;
        if(freq_change_wait_cnt >= FREQ_CHANGE_WAIT_NUM_SAMPS)
        {
            freq_change_wait_done = true;
        }
    }

    if(process_samples)
    {
        if(LTE_FDD_DL_SCAN_STATE_MACHINE_STATE_COARSE_TIMING_SEARCH != state)
        {
            // Correct frequency error
            freq_shift(0, SAMP_BUF_SIZE, timing_struct.freq_offset[corr_peak_idx]);
        }

        while(samp_buf_r_idx < (samp_buf_w_idx - N_samps_needed) &&
              samp_buf_w_idx != 0)
        {
            if(mib_rxed  == true                   &&
               sib1_rxed == true                   &&
               sib2_rxed == true                   &&
               sib3_rxed == chan_data.sib3_present &&
               sib4_rxed == chan_data.sib4_present &&
               sib5_rxed == chan_data.sib5_present &&
               sib6_rxed == chan_data.sib6_present &&
               sib7_rxed == chan_data.sib7_present &&
               sib8_rxed == chan_data.sib8_present)
            {
                chan_data.freq_offset = timing_struct.freq_offset[corr_peak_idx];
                channel_found(switch_freq, done_flag);
            }

            switch(state)
            {
            case LTE_FDD_DL_SCAN_STATE_MACHINE_STATE_COARSE_TIMING_SEARCH:
                if(LIBLTE_SUCCESS == liblte_phy_find_coarse_timing_and_freq_offset(phy_struct,
                                                                                   i_buf,
                                                                                   q_buf,
                                                                                   COARSE_TIMING_N_SLOTS,
                                                                                   &timing_struct))
                {
                    if(corr_peak_idx < timing_struct.n_corr_peaks)
                    {
                        // Correct frequency error
                        freq_shift(0, SAMP_BUF_SIZE, timing_struct.freq_offset[corr_peak_idx]);

                        // Search for PSS and fine timing
                        state          = LTE_FDD_DL_SCAN_STATE_MACHINE_STATE_PSS_AND_FINE_TIMING_SEARCH;
                        N_samps_needed = PSS_AND_FINE_TIMING_SEARCH_NUM_SAMPS;
                    }else{
                        channel_not_found(switch_freq, done_flag);
                    }
                }else{
                    // Stay in coarse timing search
                    samp_buf_r_idx = 0;
                    samp_buf_w_idx = 0;
                    N_samps_needed = COARSE_TIMING_SEARCH_NUM_SAMPS;
                    N_attempts++;
                    if(N_attempts > MAX_ATTEMPTS)
                    {
                        channel_not_found(switch_freq, done_flag);
                    }
                }
                break;
            case LTE_FDD_DL_SCAN_STATE_MACHINE_STATE_PSS_AND_FINE_TIMING_SEARCH:
                if(LIBLTE_SUCCESS == liblte_phy_find_pss_and_fine_timing(phy_struct,
                                                                         i_buf,
                                                                         q_buf,
                                                                         timing_struct.symb_starts[corr_peak_idx],
                                                                         &N_id_2,
                                                                         &pss_symb,
                                                                         &pss_thresh,
                                                                         &freq_offset))
                {
                    if(fabs(freq_offset) > 100)
                    {
                        freq_shift(0, SAMP_BUF_SIZE, freq_offset);
                        timing_struct.freq_offset[corr_peak_idx] += freq_offset;
                    }

                    // Search for SSS
                    state          = LTE_FDD_DL_SCAN_STATE_MACHINE_STATE_SSS_SEARCH;
                    N_samps_needed = SSS_SEARCH_NUM_SAMPS;
                }else{
                    // Go back to coarse timing search
                    state          = LTE_FDD_DL_SCAN_STATE_MACHINE_STATE_COARSE_TIMING_SEARCH;
                    samp_buf_r_idx = 0;
                    samp_buf_w_idx = 0;
                    N_samps_needed = COARSE_TIMING_SEARCH_NUM_SAMPS;
                    N_attempts++;
                    if(N_attempts > MAX_ATTEMPTS)
                    {
                        channel_not_found(switch_freq, done_flag);
                    }
                }
                break;
            case LTE_FDD_DL_SCAN_STATE_MACHINE_STATE_SSS_SEARCH:
                if(LIBLTE_SUCCESS == liblte_phy_find_sss(phy_struct,
                                                         i_buf,
                                                         q_buf,
                                                         N_id_2,
                                                         timing_struct.symb_starts[corr_peak_idx],
                                                         pss_thresh,
                                                         &N_id_1,
                                                         &frame_start_idx))
                {
                    chan_data.N_id_cell = 3*N_id_1 + N_id_2;

                    for(i=0; i<N_decoded_chans; i++)
                    {
                        if(chan_data.N_id_cell == decoded_chans[i])
                        {
                            break;
                        }
                    }
                    if(i != N_decoded_chans)
                    {
                        // Go back to coarse timing search
                        state = LTE_FDD_DL_SCAN_STATE_MACHINE_STATE_COARSE_TIMING_SEARCH;
                        corr_peak_idx++;
                        init();
                    }else{
                        // Decode BCH
                        state = LTE_FDD_DL_SCAN_STATE_MACHINE_STATE_BCH_DECODE;
                        while(frame_start_idx < samp_buf_r_idx)
                        {
                            frame_start_idx += ONE_FRAME_NUM_SAMPS;
                        }
                        samp_buf_r_idx = frame_start_idx;
                        N_samps_needed = BCH_DECODE_NUM_SAMPS;
                    }
                }else{
                    // Go back to coarse timing search
                    state          = LTE_FDD_DL_SCAN_STATE_MACHINE_STATE_COARSE_TIMING_SEARCH;
                    samp_buf_r_idx = 0;
                    samp_buf_w_idx = 0;
                    N_samps_needed = COARSE_TIMING_SEARCH_NUM_SAMPS;
                    N_attempts++;
                    if(N_attempts > MAX_ATTEMPTS)
                    {
                        channel_not_found(switch_freq, done_flag);
                    }
                }
                break;
            case LTE_FDD_DL_SCAN_STATE_MACHINE_STATE_BCH_DECODE:
                if(LIBLTE_SUCCESS == liblte_phy_get_subframe_and_ce(phy_struct,
                                                                    i_buf,
                                                                    q_buf,
                                                                    samp_buf_r_idx,
                                                                    0,
                                                                    chan_data.N_id_cell,
                                                                    4,
                                                                    &subframe) &&
                   LIBLTE_SUCCESS == liblte_phy_bch_channel_decode(phy_struct,
                                                                   &subframe,
                                                                   chan_data.N_id_cell,
                                                                   &chan_data.N_ant,
                                                                   rrc_msg.msg,
                                                                   &rrc_msg.N_bits,
                                                                   &sfn_offset) &&
                   LIBLTE_SUCCESS == liblte_rrc_unpack_bcch_bch_msg(&rrc_msg,
                                                                    &mib))
                {
                    switch(mib.dl_bw)
                    {
                    case LIBLTE_RRC_DL_BANDWIDTH_6:
                        N_rb_dl = LIBLTE_PHY_N_RB_DL_1_4MHZ;
                        break;
                    case LIBLTE_RRC_DL_BANDWIDTH_15:
                        N_rb_dl = LIBLTE_PHY_N_RB_DL_3MHZ;
                        break;
                    case LIBLTE_RRC_DL_BANDWIDTH_25:
                        N_rb_dl = LIBLTE_PHY_N_RB_DL_5MHZ;
                        break;
                    case LIBLTE_RRC_DL_BANDWIDTH_50:
                        N_rb_dl = LIBLTE_PHY_N_RB_DL_10MHZ;
                        break;
                    case LIBLTE_RRC_DL_BANDWIDTH_75:
                        N_rb_dl = LIBLTE_PHY_N_RB_DL_15MHZ;
                        break;
                    case LIBLTE_RRC_DL_BANDWIDTH_100:
                        N_rb_dl = LIBLTE_PHY_N_RB_DL_20MHZ;
                        break;
                    }
                    chan_data.sfn = (mib.sfn_div_4 << 2) + sfn_offset;
                    phich_res     = liblte_rrc_phich_resource_num[mib.phich_config.res];
                    save_mib(&mib);
                    if(LIBLTE_SUCCESS == liblte_phy_update_n_rb_dl(phy_struct, N_rb_dl))
                    {
                        // Add this channel to the list of decoded channels
                        decoded_chans[N_decoded_chans++] = chan_data.N_id_cell;
                        if(LTE_FDD_DL_SCAN_STATE_MACHINE_N_DECODED_CHANS_MAX == N_decoded_chans)
                        {
                            channel_not_found(switch_freq, done_flag);
                        }

                        // Decode PDSCH for SIB1
                        state = LTE_FDD_DL_SCAN_STATE_MACHINE_STATE_PDSCH_DECODE_SIB1;
                        if((chan_data.sfn % 2) != 0)
                        {
                            samp_buf_r_idx += 307200;
                            chan_data.sfn++;
                        }
                        N_samps_needed = PDSCH_DECODE_SIB1_NUM_SAMPS;
                    }else{
                        chan_data.freq_offset = timing_struct.freq_offset[corr_peak_idx];
                        channel_found(switch_freq, done_flag);
                    }
                }else{
                    // Try next MIB
                    state            = LTE_FDD_DL_SCAN_STATE_MACHINE_STATE_BCH_DECODE;
                    frame_start_idx += ONE_FRAME_NUM_SAMPS;
                    samp_buf_r_idx   = frame_start_idx;
                    N_samps_needed   = BCH_DECODE_NUM_SAMPS;
                    N_bch_attempts++;
                    if(N_bch_attempts > MAX_ATTEMPTS)
                    {
                        // Go back to coarse timing
                        state          = LTE_FDD_DL_SCAN_STATE_MACHINE_STATE_COARSE_TIMING_SEARCH;
                        samp_buf_r_idx = 0;
                        samp_buf_w_idx = 0;
                        N_samps_needed = COARSE_TIMING_SEARCH_NUM_SAMPS;
                        N_attempts++;
                        if(N_attempts > MAX_ATTEMPTS)
                        {
                            channel_not_found(switch_freq, done_flag);
                        }
                    }
                }
                break;
            case LTE_FDD_DL_SCAN_STATE_MACHINE_STATE_PDSCH_DECODE_SIB1:
                if(LIBLTE_SUCCESS == liblte_phy_get_subframe_and_ce(phy_struct,
                                                                    i_buf,
                                                                    q_buf,
                                                                    samp_buf_r_idx,
                                                                    5,
                                                                    chan_data.N_id_cell,
                                                                    chan_data.N_ant,
                                                                    &subframe) &&
                   LIBLTE_SUCCESS == liblte_phy_pdcch_channel_decode(phy_struct,
                                                                     &subframe,
                                                                     chan_data.N_id_cell,
                                                                     chan_data.N_ant,
                                                                     phich_res,
                                                                     mib.phich_config.dur,
                                                                     &pcfich,
                                                                     &phich,
                                                                     &pdcch) &&
                   LIBLTE_SUCCESS == liblte_phy_pdsch_channel_decode(phy_struct,
                                                                     &subframe,
                                                                     &pdcch.alloc[0],
                                                                     pdcch.N_symbs,
                                                                     chan_data.N_id_cell,
                                                                     chan_data.N_ant,
                                                                     rrc_msg.msg,
                                                                     &rrc_msg.N_bits) &&
                   LIBLTE_SUCCESS == liblte_rrc_unpack_bcch_dlsch_msg(&rrc_msg,
                                                                      &bcch_dlsch_msg))
                {
                    if(1                                == bcch_dlsch_msg.N_sibs &&
                       LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1 == bcch_dlsch_msg.sibs[0].sib_type)
                    {
                        save_sib1((LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1_STRUCT *)&bcch_dlsch_msg.sibs[0].sib);
                    }

                    // Decode all PDSCHs
                    state          = LTE_FDD_DL_SCAN_STATE_MACHINE_STATE_PDSCH_DECODE_SI_GENERIC;
                    N_sfr          = 0;
                    N_samps_needed = PDSCH_DECODE_SI_GENERIC_NUM_SAMPS;
                }else{
                    // Try to decode SIB1 again
                    samp_buf_r_idx += PDSCH_DECODE_SIB1_NUM_SAMPS;
                    chan_data.sfn  += 2;
                    N_samps_needed  = PDSCH_DECODE_SIB1_NUM_SAMPS;
                }
                break;
            case LTE_FDD_DL_SCAN_STATE_MACHINE_STATE_PDSCH_DECODE_SI_GENERIC:
                if(LIBLTE_SUCCESS == liblte_phy_get_subframe_and_ce(phy_struct,
                                                                    i_buf,
                                                                    q_buf,
                                                                    samp_buf_r_idx,
                                                                    N_sfr,
                                                                    chan_data.N_id_cell,
                                                                    chan_data.N_ant,
                                                                    &subframe) &&
                   LIBLTE_SUCCESS == liblte_phy_pdcch_channel_decode(phy_struct,
                                                                     &subframe,
                                                                     chan_data.N_id_cell,
                                                                     chan_data.N_ant,
                                                                     phich_res,
                                                                     mib.phich_config.dur,
                                                                     &pcfich,
                                                                     &phich,
                                                                     &pdcch) &&
                   LIBLTE_SUCCESS == liblte_phy_pdsch_channel_decode(phy_struct,
                                                                     &subframe,
                                                                     &pdcch.alloc[0],
                                                                     pdcch.N_symbs,
                                                                     chan_data.N_id_cell,
                                                                     chan_data.N_ant,
                                                                     rrc_msg.msg,
                                                                     &rrc_msg.N_bits))
                {
                    if(LIBLTE_PHY_SI_RNTI == pdcch.alloc[0].rnti &&
                       LIBLTE_SUCCESS     == liblte_rrc_unpack_bcch_dlsch_msg(&rrc_msg,
                                                                              &bcch_dlsch_msg))
                    {
                        for(i=0; i<bcch_dlsch_msg.N_sibs; i++)
                        {
                            switch(bcch_dlsch_msg.sibs[i].sib_type)
                            {
                            case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1:
                                save_sib1((LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1_STRUCT *)&bcch_dlsch_msg.sibs[i].sib);
                                break;
                            case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_2:
                                save_sib2((LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_2_STRUCT *)&bcch_dlsch_msg.sibs[i].sib);
                                break;
                            case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_3:
                                save_sib3((LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_3_STRUCT *)&bcch_dlsch_msg.sibs[i].sib);
                                break;
                            case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_4:
                                save_sib4((LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_4_STRUCT *)&bcch_dlsch_msg.sibs[i].sib);
                                break;
                            case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_5:
                                save_sib5((LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_5_STRUCT *)&bcch_dlsch_msg.sibs[i].sib);
                                break;
                            case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_6:
                                save_sib6((LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_6_STRUCT *)&bcch_dlsch_msg.sibs[i].sib);
                                break;
                            case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_7:
                                save_sib7((LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_7_STRUCT *)&bcch_dlsch_msg.sibs[i].sib);
                                break;
                            case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_8:
                                save_sib8((LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_8_STRUCT *)&bcch_dlsch_msg.sibs[i].sib);
                                break;
                            default:
                                printf("Not handling SIB %u\n", bcch_dlsch_msg.sibs[i].sib_type);
                                break;
                            }
                        }
                    }
                }

                // Keep trying to decode PDSCHs
                state          = LTE_FDD_DL_SCAN_STATE_MACHINE_STATE_PDSCH_DECODE_SI_GENERIC;
                N_samps_needed = PDSCH_DECODE_SI_GENERIC_NUM_SAMPS;
                N_sfr++;
                if(N_sfr >= 10)
                {
                    N_sfr = 0;
                    chan_data.sfn++;
                    samp_buf_r_idx += PDSCH_DECODE_SI_GENERIC_NUM_SAMPS;
                }
                break;
            }

            if(switch_freq)
            {
                samp_buf_w_idx = 0;
                samp_buf_r_idx = 0;
                init();
                N_decoded_chans = 0;
                corr_peak_idx   = 0;
                for(i=0; i<LIBLTE_PHY_N_MAX_ROUGH_CORR_SEARCH_PEAKS; i++)
                {
                    timing_struct.freq_offset[i] = 0;
                }
                break;
            }

            if(-1 == done_flag)
            {
                break;
            }
        }

        // Copy remaining samples to beginning of buffer
        if(samp_buf_r_idx > 100)
        {
            samp_buf_r_idx -= 100;
            samps_to_copy   = samp_buf_w_idx - samp_buf_r_idx;
            samp_buf_w_idx  = 0;
            freq_shift(samp_buf_r_idx, samps_to_copy, -timing_struct.freq_offset[corr_peak_idx]);
            for(i=0; i<samps_to_copy; i++)
            {
                i_buf[samp_buf_w_idx]   = i_buf[samp_buf_r_idx];
                q_buf[samp_buf_w_idx++] = q_buf[samp_buf_r_idx++];
            }
            samp_buf_r_idx = 100;
        }

        if(true == copy_input)
        {
            copy_input_to_samp_buf(in, ninput_items);
        }
    }

    if(!flowgraph->is_started())
    {
        done_flag = -1;
    }

    // Tell runtime system how many input items we consumed
    consume_each(ninput_items);

    // Tell runtime system how many output items we produced.
    return(done_flag);
}

void LTE_fdd_dl_scan_state_machine::init(void)
{
    state                  = LTE_FDD_DL_SCAN_STATE_MACHINE_STATE_COARSE_TIMING_SEARCH;
    phich_res              = 0;
    chan_data.sfn          = 0;
    N_sfr                  = 0;
    chan_data.N_ant        = 0;
    chan_data.N_id_cell    = 0;
    N_id_1                 = 0;
    N_id_2                 = 0;
    N_attempts             = 0;
    N_bch_attempts         = 0;
    N_samps_needed         = COARSE_TIMING_SEARCH_NUM_SAMPS;
    freq_change_wait_cnt   = 0;
    freq_change_wait_done  = false;
    mib_rxed               = false;
    sib1_rxed              = false;
    chan_data.sib1_present = false;
    sib2_rxed              = false;
    chan_data.sib2_present = false;
    sib3_rxed              = false;
    chan_data.sib3_present = false;
    sib4_rxed              = false;
    chan_data.sib4_present = false;
    sib5_rxed              = false;
    chan_data.sib5_present = false;
    sib6_rxed              = false;
    chan_data.sib6_present = false;
    sib7_rxed              = false;
    chan_data.sib7_present = false;
    sib8_rxed              = false;
    chan_data.sib8_present = false;
}

void LTE_fdd_dl_scan_state_machine::copy_input_to_samp_buf(const gr_complex *in, int32 ninput_items)
{
    uint32 i;

    for(i=0; i<ninput_items; i++)
    {
        i_buf[samp_buf_w_idx]   = in[i].real();
        q_buf[samp_buf_w_idx++] = in[i].imag();
    }
}

void LTE_fdd_dl_scan_state_machine::freq_shift(uint32 start_idx, uint32 num_samps, float freq_offset)
{
    float  f_samp_re;
    float  f_samp_im;
    float  tmp_i;
    float  tmp_q;
    uint32 i;

    for(i=start_idx; i<(start_idx+num_samps); i++)
    {
        f_samp_re = cosf((i+1)*(freq_offset)*2*M_PI/phy_struct->fs);
        f_samp_im = sinf((i+1)*(freq_offset)*2*M_PI/phy_struct->fs);
        tmp_i     = i_buf[i];
        tmp_q     = q_buf[i];
        i_buf[i]  = tmp_i*f_samp_re + tmp_q*f_samp_im;
        q_buf[i]  = tmp_q*f_samp_re - tmp_i*f_samp_im;
    }
}

void LTE_fdd_dl_scan_state_machine::save_mib(LIBLTE_RRC_MIB_STRUCT *mib)
{
    if(false == mib_rxed)
    {
        memcpy(&chan_data.mib, mib, sizeof(chan_data.mib));
        mib_rxed = true;
    }
}

void LTE_fdd_dl_scan_state_machine::save_sib1(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1_STRUCT *sib1)
{
    uint32 i;
    uint32 j;

    if(true                                 == sib1_rxed &&
       chan_data.sib1.system_info_value_tag != sib1->system_info_value_tag)
    {
        sib1_rxed = false;
        sib2_rxed = false;
        sib3_rxed = false;
        sib4_rxed = false;
        sib5_rxed = false;
        sib6_rxed = false;
        sib7_rxed = false;
        sib8_rxed = false;
    }

    if(false == sib1_rxed)
    {
        chan_data.sib1_present = true;
        chan_data.sib2_present = true;
        for(i=0; i<sib1->N_sched_info; i++)
        {
            for(j=0; j<sib1->sched_info[i].N_sib_mapping_info; j++)
            {
                switch(sib1->sched_info[i].sib_mapping_info[j].sib_type)
                {
                case LIBLTE_RRC_SIB_TYPE_3:
                    chan_data.sib3_present = true;
                    break;
                case LIBLTE_RRC_SIB_TYPE_4:
                    chan_data.sib4_present = true;
                    break;
                case LIBLTE_RRC_SIB_TYPE_5:
                    chan_data.sib5_present = true;
                    break;
                case LIBLTE_RRC_SIB_TYPE_6:
                    chan_data.sib6_present = true;
                    break;
                case LIBLTE_RRC_SIB_TYPE_7:
                    chan_data.sib7_present = true;
                    break;
                case LIBLTE_RRC_SIB_TYPE_8:
                    chan_data.sib8_present = true;
                    break;
                }
            }
        }

        memcpy(&chan_data.sib1, sib1, sizeof(chan_data.sib1));
        sib1_rxed = true;
    }
}

void LTE_fdd_dl_scan_state_machine::save_sib2(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_2_STRUCT *sib2)
{
    if(false == sib2_rxed)
    {
        memcpy(&chan_data.sib2, sib2, sizeof(chan_data.sib2));
        sib2_rxed = true;
    }
}

void LTE_fdd_dl_scan_state_machine::save_sib3(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_3_STRUCT *sib3)
{
    if(false == sib3_rxed)
    {
        memcpy(&chan_data.sib3, sib3, sizeof(chan_data.sib3));
        sib3_rxed = true;
    }
}

void LTE_fdd_dl_scan_state_machine::save_sib4(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_4_STRUCT *sib4)
{
    if(false == sib4_rxed)
    {
        memcpy(&chan_data.sib4, sib4, sizeof(chan_data.sib4));
        sib4_rxed = true;
    }
}

void LTE_fdd_dl_scan_state_machine::save_sib5(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_5_STRUCT *sib5)
{
    if(false == sib5_rxed)
    {
        memcpy(&chan_data.sib5, sib5, sizeof(chan_data.sib5));
        sib5_rxed = true;
    }
}

void LTE_fdd_dl_scan_state_machine::save_sib6(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_6_STRUCT *sib6)
{
    if(false == sib6_rxed)
    {
        memcpy(&chan_data.sib6, sib6, sizeof(chan_data.sib6));
        sib6_rxed = true;
    }
}

void LTE_fdd_dl_scan_state_machine::save_sib7(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_7_STRUCT *sib7)
{
    if(false == sib7_rxed)
    {
        memcpy(&chan_data.sib7, sib7, sizeof(chan_data.sib7));
        sib7_rxed = true;
    }
}

void LTE_fdd_dl_scan_state_machine::save_sib8(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_8_STRUCT *sib8)
{
    if(false == sib8_rxed)
    {
        memcpy(&chan_data.sib8, sib8, sizeof(chan_data.sib8));
        sib8_rxed = true;
    }
}

void LTE_fdd_dl_scan_state_machine::channel_found(bool  &switch_freq,
                                                  int32 &done_flag)
{
    LTE_fdd_dl_scan_interface *interface = LTE_fdd_dl_scan_interface::get_instance();

    // Send the channel information
    interface->send_ctrl_channel_found_msg(&chan_data);

    // Initialize for the next channel
    init();

    // Determine if a frequency change is needed
    corr_peak_idx++;
    if(corr_peak_idx >= timing_struct.n_corr_peaks)
    {
        // Change frequency
        corr_peak_idx = 0;
        if(LTE_FDD_DL_SCAN_STATUS_OK == interface->switch_to_next_freq())
        {
            switch_freq = true;
            send_cnf    = true;
        }else{
            done_flag = -1;
        }
    }else{
        send_cnf = false;
    }
}

void LTE_fdd_dl_scan_state_machine::channel_not_found(bool  &switch_freq,
                                                      int32 &done_flag)
{
    LTE_fdd_dl_scan_interface *interface = LTE_fdd_dl_scan_interface::get_instance();

    if(send_cnf)
    {
        // Send the channel information
        interface->send_ctrl_channel_not_found_msg();
    }

    // Initialize for the next channel
    init();

    // Change frequency
    corr_peak_idx = 0;
    if(LTE_FDD_DL_SCAN_STATUS_OK == interface->switch_to_next_freq())
    {
        switch_freq = true;
        send_cnf    = true;
    }else{
        done_flag = -1;
    }
}
