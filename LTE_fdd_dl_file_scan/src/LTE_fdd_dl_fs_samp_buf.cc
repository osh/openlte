/*******************************************************************************

    Copyright 2012 Ben Wojtowicz

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

    File: LTE_fdd_dl_fs_samp_buf.cc

    Description: Contains all the implementations for the LTE FDD DL File
                 Scanner sample buffer block.

    Revision History
    ----------    -------------    --------------------------------------------
    03/23/2012    Ben Wojtowicz    Created file.
    04/21/2012    Ben Wojtowicz    Fixed bug in frequency offset removal, added
                                   PDSCH decode, general cleanup
    06/28/2012    Ben Wojtowicz    Fixed a bug that was causing I and Q to swap
    08/19/2012    Ben Wojtowicz    Added states and state memory and added
                                   decoding of all SIBs.
    10/06/2012    Ben Wojtowicz    Updated to use the latest LTE library.
    11/10/2012    Ben Wojtowicz    Using the latest libraries to decode more
                                   than 1 eNB

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_dl_fs_samp_buf.h"
#include "gr_io_signature.h"

/*******************************************************************************
                              DEFINES
*******************************************************************************/

#define ONE_SUBFRAME_NUM_SAMPS               (30720)
#define ONE_FRAME_NUM_SAMPS                  (10 * ONE_SUBFRAME_NUM_SAMPS)
#define COARSE_TIMING_SEARCH_NUM_SAMPS       (12 * ONE_SUBFRAME_NUM_SAMPS)
#define PSS_AND_FINE_TIMING_SEARCH_NUM_SAMPS (COARSE_TIMING_SEARCH_NUM_SAMPS)
#define SSS_SEARCH_NUM_SAMPS                 (COARSE_TIMING_SEARCH_NUM_SAMPS)
#define BCH_DECODE_NUM_SAMPS                 (2 * ONE_FRAME_NUM_SAMPS)
#define PDSCH_DECODE_SIB1_NUM_SAMPS          (2 * ONE_FRAME_NUM_SAMPS)
#define PDSCH_DECODE_SI_GENERIC_NUM_SAMPS    (ONE_FRAME_NUM_SAMPS)

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


LTE_fdd_dl_fs_samp_buf_sptr LTE_fdd_dl_fs_make_samp_buf()
{
    return LTE_fdd_dl_fs_samp_buf_sptr(new LTE_fdd_dl_fs_samp_buf());
}

LTE_fdd_dl_fs_samp_buf::LTE_fdd_dl_fs_samp_buf()
    : gr_sync_block ("samp_buf",
                     gr_make_io_signature(MIN_IN,  MAX_IN,  sizeof(int8)),
                     gr_make_io_signature(MIN_OUT, MAX_OUT, sizeof(int8)))
{
    uint32 i;

    // Initialize the LTE library
    liblte_phy_init(&phy_struct);

    // Initialize the sample buffer
    i_buf           = (float *)malloc(LTE_FDD_DL_FS_SAMP_BUF_SIZE*sizeof(float));
    q_buf           = (float *)malloc(LTE_FDD_DL_FS_SAMP_BUF_SIZE*sizeof(float));
    samp_buf_w_idx  = 0;
    samp_buf_r_idx  = 0;
    last_samp_was_i = false;

    // Variables
    init();
    corr_peak_idx = 0;
    for(i=0; i<LIBLTE_PHY_N_MAX_ROUGH_CORR_SEARCH_PEAKS; i++)
    {
        timing_struct.freq_offset[i] = 0;
    }
}
LTE_fdd_dl_fs_samp_buf::~LTE_fdd_dl_fs_samp_buf()
{
    // Cleanup the LTE library
    liblte_phy_cleanup(phy_struct);

    // Free the sample buffer
    free(i_buf);
    free(q_buf);
}

int32 LTE_fdd_dl_fs_samp_buf::work(int32                      ninput_items,
                                   gr_vector_const_void_star &input_items,
                                   gr_vector_void_star       &output_items)
{
    LIBLTE_PHY_SUBFRAME_STRUCT  subframe;
    LIBLTE_PHY_PCFICH_STRUCT    pcfich;
    LIBLTE_PHY_PHICH_STRUCT     phich;
    LIBLTE_PHY_PDCCH_STRUCT     pdcch;
    const int8                 *in  = (const int8 *)input_items[0];
    float                       pss_thresh;
    uint32                      i;
    uint32                      pss_symb;
    uint32                      frame_start_idx;
    uint32                      num_samps_needed = COARSE_TIMING_SEARCH_NUM_SAMPS;
    uint32                      samps_to_copy;
    uint8                       sfn_offset;
    bool                        process_samples = false;
    bool                        copy_input      = false;

    if(samp_buf_w_idx < (LTE_FDD_DL_FS_SAMP_BUF_SIZE-((ninput_items+1)/2)))
    {
        copy_input_to_samp_buf(in, ninput_items);

        // Check if buffer is full enough
        if(samp_buf_w_idx >= (LTE_FDD_DL_FS_SAMP_BUF_SIZE-((ninput_items+1)/2)))
        {
            process_samples = true;
            copy_input      = false;
        }
    }else{
        // Buffer is too full process samples, then copy
        process_samples = true;
        copy_input      = true;
    }

    if(process_samples)
    {
        if(LTE_FDD_DL_FS_SAMP_BUF_STATE_COARSE_TIMING_SEARCH != state)
        {
            // Correct frequency error
            freq_shift(0, LTE_FDD_DL_FS_SAMP_BUF_SIZE, timing_struct.freq_offset[corr_peak_idx]);
        }

        // Get number of samples needed for each state
        switch(state)
        {
        case LTE_FDD_DL_FS_SAMP_BUF_STATE_COARSE_TIMING_SEARCH:
            num_samps_needed = COARSE_TIMING_SEARCH_NUM_SAMPS;
            break;
        case LTE_FDD_DL_FS_SAMP_BUF_STATE_PSS_AND_FINE_TIMING_SEARCH:
            num_samps_needed = PSS_AND_FINE_TIMING_SEARCH_NUM_SAMPS;
            break;
        case LTE_FDD_DL_FS_SAMP_BUF_STATE_SSS_SEARCH:
            num_samps_needed = SSS_SEARCH_NUM_SAMPS;
            break;
        case LTE_FDD_DL_FS_SAMP_BUF_STATE_BCH_DECODE:
            num_samps_needed = BCH_DECODE_NUM_SAMPS;
            break;
        case LTE_FDD_DL_FS_SAMP_BUF_STATE_PDSCH_DECODE_SIB1:
            num_samps_needed = PDSCH_DECODE_SIB1_NUM_SAMPS;
            break;
        case LTE_FDD_DL_FS_SAMP_BUF_STATE_PDSCH_DECODE_SI_GENERIC:
            num_samps_needed = PDSCH_DECODE_SI_GENERIC_NUM_SAMPS;
            break;
        }

        while(samp_buf_r_idx < (samp_buf_w_idx - num_samps_needed))
        {
            if(mib_printed  == true          &&
               sib1_printed == true          &&
               sib2_printed == true          &&
               sib3_printed == sib3_expected &&
               sib4_printed == sib4_expected &&
               sib8_printed == sib8_expected)
            {
                corr_peak_idx++;
                init();
            }

            switch(state)
            {
            case LTE_FDD_DL_FS_SAMP_BUF_STATE_COARSE_TIMING_SEARCH:
                if(LIBLTE_SUCCESS == liblte_phy_find_coarse_timing_and_freq_offset(phy_struct,
                                                                                   i_buf,
                                                                                   q_buf,
                                                                                   &timing_struct) &&
                   corr_peak_idx < timing_struct.n_corr_peaks)
                {
                    // Correct frequency error
                    freq_shift(0, LTE_FDD_DL_FS_SAMP_BUF_SIZE, timing_struct.freq_offset[corr_peak_idx]);

                    // Search for PSS and fine timing
                    state            = LTE_FDD_DL_FS_SAMP_BUF_STATE_PSS_AND_FINE_TIMING_SEARCH;
                    num_samps_needed = PSS_AND_FINE_TIMING_SEARCH_NUM_SAMPS;
                }else{
                    // Stay in coarse timing search
                    samp_buf_r_idx   += COARSE_TIMING_SEARCH_NUM_SAMPS;
                    num_samps_needed  = COARSE_TIMING_SEARCH_NUM_SAMPS;
                }
                break;
            case LTE_FDD_DL_FS_SAMP_BUF_STATE_PSS_AND_FINE_TIMING_SEARCH:
                if(LIBLTE_SUCCESS == liblte_phy_find_pss_and_fine_timing(phy_struct,
                                                                         i_buf,
                                                                         q_buf,
                                                                         timing_struct.symb_starts[corr_peak_idx],
                                                                         &N_id_2,
                                                                         &pss_symb,
                                                                         &pss_thresh))
                {
                    // Search for SSS
                    state            = LTE_FDD_DL_FS_SAMP_BUF_STATE_SSS_SEARCH;
                    num_samps_needed = SSS_SEARCH_NUM_SAMPS;
                }else{
                    // Go back to coarse timing search
                    state             = LTE_FDD_DL_FS_SAMP_BUF_STATE_COARSE_TIMING_SEARCH;
                    samp_buf_r_idx   += COARSE_TIMING_SEARCH_NUM_SAMPS;
                    num_samps_needed  = COARSE_TIMING_SEARCH_NUM_SAMPS;
                }
                break;
            case LTE_FDD_DL_FS_SAMP_BUF_STATE_SSS_SEARCH:
                if(LIBLTE_SUCCESS == liblte_phy_find_sss(phy_struct,
                                                         i_buf,
                                                         q_buf,
                                                         N_id_2,
                                                         timing_struct.symb_starts[corr_peak_idx],
                                                         pss_thresh,
                                                         &N_id_1,
                                                         &frame_start_idx))
                {
                    N_id_cell = 3*N_id_1 + N_id_2;

                    // Decode BCH
                    state            = LTE_FDD_DL_FS_SAMP_BUF_STATE_BCH_DECODE;
                    while(frame_start_idx < samp_buf_r_idx)
                    {
                        frame_start_idx += ONE_FRAME_NUM_SAMPS;
                    }
                    samp_buf_r_idx   = frame_start_idx;
                    num_samps_needed = BCH_DECODE_NUM_SAMPS;
                }else{
                    // Go back to coarse timing search
                    state             = LTE_FDD_DL_FS_SAMP_BUF_STATE_COARSE_TIMING_SEARCH;
                    samp_buf_r_idx   += COARSE_TIMING_SEARCH_NUM_SAMPS;
                    num_samps_needed  = COARSE_TIMING_SEARCH_NUM_SAMPS;
                }
                break;
            case LTE_FDD_DL_FS_SAMP_BUF_STATE_BCH_DECODE:
                if(LIBLTE_SUCCESS == liblte_phy_get_subframe_and_ce(phy_struct,
                                                                    i_buf,
                                                                    q_buf,
                                                                    samp_buf_r_idx,
                                                                    0,
                                                                    FFT_pad_size,
                                                                    N_rb_dl,
                                                                    N_id_cell,
                                                                    4,
                                                                    &subframe) &&
                   LIBLTE_SUCCESS == liblte_phy_bch_channel_decode(phy_struct,
                                                                   &subframe,
                                                                   N_id_cell,
                                                                   &N_ant,
                                                                   rrc_msg.msg,
                                                                   &rrc_msg.N_bits,
                                                                   &sfn_offset) &&
                   LIBLTE_SUCCESS == liblte_rrc_unpack_bcch_bch_msg(&rrc_msg,
                                                                    &mib))
                {
                    switch(mib.dl_bw)
                    {
                    case LIBLTE_RRC_DL_BANDWIDTH_6:
                        N_rb_dl      = LIBLTE_PHY_N_RB_DL_1_4MHZ;
                        FFT_pad_size = LIBLTE_PHY_FFT_PAD_SIZE_1_4MHZ;
                        break;
                    case LIBLTE_RRC_DL_BANDWIDTH_15:
                        N_rb_dl      = LIBLTE_PHY_N_RB_DL_3MHZ;
                        FFT_pad_size = LIBLTE_PHY_FFT_PAD_SIZE_3MHZ;
                        break;
                    case LIBLTE_RRC_DL_BANDWIDTH_25:
                        N_rb_dl      = LIBLTE_PHY_N_RB_DL_5MHZ;
                        FFT_pad_size = LIBLTE_PHY_FFT_PAD_SIZE_5MHZ;
                        break;
                    case LIBLTE_RRC_DL_BANDWIDTH_50:
                        N_rb_dl      = LIBLTE_PHY_N_RB_DL_10MHZ;
                        FFT_pad_size = LIBLTE_PHY_FFT_PAD_SIZE_10MHZ;
                        break;
                    case LIBLTE_RRC_DL_BANDWIDTH_75:
                        N_rb_dl      = LIBLTE_PHY_N_RB_DL_15MHZ;
                        FFT_pad_size = LIBLTE_PHY_FFT_PAD_SIZE_15MHZ;
                        break;
                    case LIBLTE_RRC_DL_BANDWIDTH_100:
                        N_rb_dl      = LIBLTE_PHY_N_RB_DL_20MHZ;
                        FFT_pad_size = LIBLTE_PHY_FFT_PAD_SIZE_20MHZ;
                        break;
                    }
                    sfn = (mib.sfn_div_4 << 2) + sfn_offset;
                    switch(mib.phich_config.res)
                    {
                    case LIBLTE_RRC_PHICH_RESOURCE_1_6:
                        phich_res = 1/6;
                        break;
                    case LIBLTE_RRC_PHICH_RESOURCE_1_2:
                        phich_res = 1/2;
                        break;
                    case LIBLTE_RRC_PHICH_RESOURCE_1:
                        phich_res = 1;
                        break;
                    case LIBLTE_RRC_PHICH_RESOURCE_2:
                        phich_res = 2;
                        break;
                    }
                    print_mib(&mib);

                    // Decode PDSCH for SIB1
                    state = LTE_FDD_DL_FS_SAMP_BUF_STATE_PDSCH_DECODE_SIB1;
                    if((sfn % 2) != 0)
                    {
                        samp_buf_r_idx += 307200;
                        sfn++;
                    }
                    num_samps_needed = PDSCH_DECODE_SIB1_NUM_SAMPS;
                }else{
                    // Go back to coarse timing search
                    state             = LTE_FDD_DL_FS_SAMP_BUF_STATE_COARSE_TIMING_SEARCH;
                    samp_buf_r_idx   += COARSE_TIMING_SEARCH_NUM_SAMPS;
                    num_samps_needed  = COARSE_TIMING_SEARCH_NUM_SAMPS;
                }
                break;
            case LTE_FDD_DL_FS_SAMP_BUF_STATE_PDSCH_DECODE_SIB1:
                if(LIBLTE_SUCCESS == liblte_phy_get_subframe_and_ce(phy_struct,
                                                                    i_buf,
                                                                    q_buf,
                                                                    samp_buf_r_idx,
                                                                    5,
                                                                    FFT_pad_size,
                                                                    N_rb_dl,
                                                                    N_id_cell,
                                                                    N_ant,
                                                                    &subframe) &&
                   LIBLTE_SUCCESS == liblte_phy_pdcch_channel_decode(phy_struct,
                                                                     &subframe,
                                                                     N_id_cell,
                                                                     N_ant,
                                                                     LIBLTE_PHY_N_SC_RB_NORMAL_CP,
                                                                     N_rb_dl,
                                                                     phich_res,
                                                                     mib.phich_config.dur,
                                                                     &pcfich,
                                                                     &phich,
                                                                     &pdcch) &&
                   LIBLTE_SUCCESS == liblte_phy_pdsch_channel_decode(phy_struct,
                                                                     &subframe,
                                                                     &pdcch.alloc[0],
                                                                     pdcch.N_symbs,
                                                                     N_id_cell,
                                                                     N_ant,
                                                                     LIBLTE_PHY_N_SC_RB_NORMAL_CP,
                                                                     N_rb_dl,
                                                                     rrc_msg.msg,
                                                                     &rrc_msg.N_bits) &&
                   LIBLTE_SUCCESS == liblte_rrc_unpack_bcch_dlsch_msg(&rrc_msg,
                                                                      &bcch_dlsch_msg))
                {
                    if(1                                == bcch_dlsch_msg.N_sibs &&
                       LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1 == bcch_dlsch_msg.sibs[0].sib_type)
                    {
                        print_sib1((LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1_STRUCT *)&bcch_dlsch_msg.sibs[0].sib);
                    }

                    // Decode all PDSCHs
                    state            = LTE_FDD_DL_FS_SAMP_BUF_STATE_PDSCH_DECODE_SI_GENERIC;
                    N_sfr            = 0;
                    num_samps_needed = PDSCH_DECODE_SI_GENERIC_NUM_SAMPS;
                }else{
                    // Try to decode SIB1 again
                    samp_buf_r_idx   += PDSCH_DECODE_SIB1_NUM_SAMPS;
                    sfn              += 2;
                    num_samps_needed  = PDSCH_DECODE_SIB1_NUM_SAMPS;
                }
                break;
            case LTE_FDD_DL_FS_SAMP_BUF_STATE_PDSCH_DECODE_SI_GENERIC:
                if(LIBLTE_SUCCESS == liblte_phy_get_subframe_and_ce(phy_struct,
                                                                    i_buf,
                                                                    q_buf,
                                                                    samp_buf_r_idx,
                                                                    N_sfr,
                                                                    FFT_pad_size,
                                                                    N_rb_dl,
                                                                    N_id_cell,
                                                                    N_ant,
                                                                    &subframe) &&
                   LIBLTE_SUCCESS == liblte_phy_pdcch_channel_decode(phy_struct,
                                                                     &subframe,
                                                                     N_id_cell,
                                                                     N_ant,
                                                                     LIBLTE_PHY_N_SC_RB_NORMAL_CP,
                                                                     N_rb_dl,
                                                                     phich_res,
                                                                     mib.phich_config.dur,
                                                                     &pcfich,
                                                                     &phich,
                                                                     &pdcch) &&
                   LIBLTE_SUCCESS == liblte_phy_pdsch_channel_decode(phy_struct,
                                                                     &subframe,
                                                                     &pdcch.alloc[0],
                                                                     pdcch.N_symbs,
                                                                     N_id_cell,
                                                                     N_ant,
                                                                     LIBLTE_PHY_N_SC_RB_NORMAL_CP,
                                                                     N_rb_dl,
                                                                     rrc_msg.msg,
                                                                     &rrc_msg.N_bits) &&
                   LIBLTE_SUCCESS == liblte_rrc_unpack_bcch_dlsch_msg(&rrc_msg,
                                                                      &bcch_dlsch_msg))
                {
                    for(i=0; i<bcch_dlsch_msg.N_sibs; i++)
                    {
                        switch(bcch_dlsch_msg.sibs[i].sib_type)
                        {
                        case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1:
                            print_sib1((LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1_STRUCT *)&bcch_dlsch_msg.sibs[i].sib);
                            break;
                        case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_2:
                            print_sib2((LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_2_STRUCT *)&bcch_dlsch_msg.sibs[i].sib);
                            break;
                        case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_3:
                            print_sib3((LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_3_STRUCT *)&bcch_dlsch_msg.sibs[i].sib);
                            break;
                        case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_4:
                            print_sib4((LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_4_STRUCT *)&bcch_dlsch_msg.sibs[i].sib);
                            break;
                        case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_8:
                            print_sib8((LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_8_STRUCT *)&bcch_dlsch_msg.sibs[i].sib);
                            break;
                        default:
                            printf("Not handling SIB %u\n", bcch_dlsch_msg.sibs[i].sib_type);
                            break;
                        }
                    }
                }

                // Keep trying to decode PDSCHs
                state            = LTE_FDD_DL_FS_SAMP_BUF_STATE_PDSCH_DECODE_SI_GENERIC;
                num_samps_needed = PDSCH_DECODE_SI_GENERIC_NUM_SAMPS;
                N_sfr++;
                if(N_sfr >= 10)
                {
                    N_sfr = 0;
                    sfn++;
                    samp_buf_r_idx += PDSCH_DECODE_SI_GENERIC_NUM_SAMPS;
                }
                break;
            }
        }

        // Copy remaining samples to beginning of buffer
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

        if(true == copy_input)
        {
            copy_input_to_samp_buf(in, ninput_items);
        }
    }

    // Tell runtime system how many input items we consumed
    consume_each(ninput_items);

    // Tell runtime system how many output items we produced.
    return(0);
}

void LTE_fdd_dl_fs_samp_buf::init(void)
{
    state                   = LTE_FDD_DL_FS_SAMP_BUF_STATE_COARSE_TIMING_SEARCH;
    phich_res               = 0;
    N_rb_dl                 = LIBLTE_PHY_N_RB_DL_1_4MHZ;
    FFT_pad_size            = LIBLTE_PHY_FFT_PAD_SIZE_1_4MHZ;
    sfn                     = 0;
    N_sfr                   = 0;
    N_ant                   = 0;
    N_id_cell               = 0;
    N_id_1                  = 0;
    N_id_2                  = 0;
    prev_si_value_tag       = 0;
    prev_si_value_tag_valid = false;
    mib_printed             = false;
    sib1_printed            = false;
    sib2_printed            = false;
    sib3_printed            = false;
    sib3_expected           = false;
    sib4_printed            = false;
    sib4_expected           = false;
    sib8_printed            = false;
    sib8_expected           = false;
}

void LTE_fdd_dl_fs_samp_buf::copy_input_to_samp_buf(const int8 *in, int32 ninput_items)
{
    uint32 i;
    uint32 offset;

    if(true == last_samp_was_i)
    {
        q_buf[samp_buf_w_idx++] = (float)in[0];
        offset                  = 1;
    }else{
        offset = 0;
    }

    for(i=0; i<(ninput_items-offset)/2; i++)
    {
        i_buf[samp_buf_w_idx]   = (float)in[i*2+offset];
        q_buf[samp_buf_w_idx++] = (float)in[i*2+offset+1];
    }

    if(((ninput_items-offset) % 2) != 0)
    {
        i_buf[samp_buf_w_idx] = (float)in[ninput_items-1];
        last_samp_was_i       = true;
    }else{
        last_samp_was_i = false;
    }
}

void LTE_fdd_dl_fs_samp_buf::freq_shift(uint32 start_idx, uint32 num_samps, float freq_offset)
{
    float  f_samp_re;
    float  f_samp_im;
    float  tmp_i;
    float  tmp_q;
    uint32 i;

    for(i=start_idx; i<(start_idx+num_samps); i++)
    {
        f_samp_re = cosf((i+1)*(-freq_offset)*2*M_PI*(0.0005/15360));
        f_samp_im = sinf((i+1)*(-freq_offset)*2*M_PI*(0.0005/15360));
        tmp_i     = i_buf[i];
        tmp_q     = q_buf[i];
        i_buf[i]  = tmp_i*f_samp_re + tmp_q*f_samp_im;
        q_buf[i]  = tmp_q*f_samp_re - tmp_i*f_samp_im;
    }
}

void LTE_fdd_dl_fs_samp_buf::print_mib(LIBLTE_RRC_MIB_STRUCT *mib)
{
    if(false == mib_printed)
    {
        printf("DL LTE Channel found [%u]:\n", corr_peak_idx);
        printf("\tMIB Decoded:\n");
        printf("\t\t%-40s=%20.2f\n", "Frequency Offset", timing_struct.freq_offset[corr_peak_idx]);
        printf("\t\t%-40s=%20u\n", "System Frame Number", sfn);
        printf("\t\t%-40s=%20u\n", "Physical Cell ID", N_id_cell);
        printf("\t\t%-40s=%20u\n", "Number of TX Antennas", N_ant);
        switch(mib->dl_bw)
        {
        case LIBLTE_RRC_DL_BANDWIDTH_6:
            printf("\t\t%-40s=%20s\n", "Bandwidth", "1.4MHz");
            break;
        case LIBLTE_RRC_DL_BANDWIDTH_15:
            printf("\t\t%-40s=%20s\n", "Bandwidth", "3MHz");
            break;
        case LIBLTE_RRC_DL_BANDWIDTH_25:
            printf("\t\t%-40s=%20s\n", "Bandwidth", "5MHz");
            break;
        case LIBLTE_RRC_DL_BANDWIDTH_50:
            printf("\t\t%-40s=%20s\n", "Bandwidth", "10MHz");
            break;
        case LIBLTE_RRC_DL_BANDWIDTH_75:
            printf("\t\t%-40s=%20s\n", "Bandwidth", "15MHz");
            break;
        case LIBLTE_RRC_DL_BANDWIDTH_100:
            printf("\t\t%-40s=%20s\n", "Bandwidth", "20MHz");
            break;
        }
        if(mib->phich_config.dur == LIBLTE_RRC_PHICH_DURATION_NORMAL)
        {
            printf("\t\t%-40s=%20s\n", "PHICH Duration", "Normal");
        }else{
            printf("\t\t%-40s=%20s\n", "PHICH Duration", "Extended");
        }
        switch(mib->phich_config.res)
        {
        case LIBLTE_RRC_PHICH_RESOURCE_1_6:
            printf("\t\t%-40s=%20s\n", "PHICH Resource", "1/6");
            break;
        case LIBLTE_RRC_PHICH_RESOURCE_1_2:
            printf("\t\t%-40s=%20s\n", "PHICH Resource", "1/2");
            break;
        case LIBLTE_RRC_PHICH_RESOURCE_1:
            printf("\t\t%-40s=%20u\n", "PHICH Resource", 1);
            break;
        case LIBLTE_RRC_PHICH_RESOURCE_2:
            printf("\t\t%-40s=%20u\n", "PHICH Resource", 2);
            break;
        }

        mib_printed = true;
    }
}

void LTE_fdd_dl_fs_samp_buf::print_sib1(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1_STRUCT *sib1)
{
    uint32 i;
    uint32 j;
    uint32 si_win_len;
    uint32 si_periodicity_T;

    if(true              == prev_si_value_tag_valid &&
       prev_si_value_tag != sib1->system_info_value_tag)
    {
        printf("\tSystem Info value tag changed\n");
        sib1_printed = false;
        sib2_printed = false;
        sib3_printed = false;
        sib4_printed = false;
        sib8_printed = false;
    }

    if(false == sib1_printed)
    {
        printf("\tSIB1 Decoded:\n");
        printf("\t\t%-40s\n", "PLMN Identity List:");
        for(i=0; i<sib1->N_plmn_ids; i++)
        {
            printf("\t\t\t%03X-", sib1->plmn_id[i].id.mcc & 0x0FFF);
            if((sib1->plmn_id[i].id.mnc & 0xFF00) == 0xFF00)
            {
                printf("%02X, ", sib1->plmn_id[i].id.mnc & 0x00FF);
            }else{
                printf("%03X, ", sib1->plmn_id[i].id.mnc & 0x0FFF);
            }
            if(LIBLTE_RRC_RESV_FOR_OPER == sib1->plmn_id[i].resv_for_oper)
            {
                printf("reserved for operator use\n");
            }else{
                printf("not reserved for operator use\n");
            }
        }
        printf("\t\t%-40s=%20u\n", "Tracking Area Code", sib1->tracking_area_code);
        printf("\t\t%-40s=%20u\n", "Cell Identity", sib1->cell_id);
        switch(sib1->cell_barred)
        {
        case LIBLTE_RRC_CELL_BARRED:
            printf("\t\t%-40s=%20s\n", "Cell Barred", "Barred");
            break;
        case LIBLTE_RRC_CELL_NOT_BARRED:
            printf("\t\t%-40s=%20s\n", "Cell Barred", "Not Barred");
            break;
        }
        switch(sib1->intra_freq_reselection)
        {
        case LIBLTE_RRC_INTRA_FREQ_RESELECTION_ALLOWED:
            printf("\t\t%-40s=%20s\n", "Intra Frequency Reselection", "Allowed");
            break;
        case LIBLTE_RRC_INTRA_FREQ_RESELECTION_NOT_ALLOWED:
            printf("\t\t%-40s=%20s\n", "Intra Frequency Reselection", "Not Allowed");
            break;
        }
        if(true == sib1->csg_indication)
        {
            printf("\t\t%-40s=%20s\n", "CSG Indication", "TRUE");
        }else{
            printf("\t\t%-40s=%20s\n", "CSG Indication", "FALSE");
        }
        if(LIBLTE_RRC_CSG_IDENTITY_NOT_PRESENT != sib1->csg_id)
        {
            printf("\t\t%-40s=%20u\n", "CSG Identity", sib1->csg_id);
        }
        printf("\t\t%-40s=%17ddBm\n", "Q Rx Lev Min", sib1->q_rx_lev_min);
        printf("\t\t%-40s=%18udB\n", "Q Rx Lev Min Offset", sib1->q_rx_lev_min_offset);
        if(true == sib1->p_max_present)
        {
            printf("\t\t%-40s=%17ddBm\n", "P Max", sib1->p_max);
        }
        printf("\t\t%-40s=%20u\n", "Frequency Band", sib1->freq_band_indicator);
        switch(sib1->si_window_length)
        {
        case LIBLTE_RRC_SI_WINDOW_LENGTH_MS1:
            si_win_len = 1;
            printf("\t\t%-40s=%20s\n", "SI Window Length", "1ms");
            break;
        case LIBLTE_RRC_SI_WINDOW_LENGTH_MS2:
            si_win_len = 2;
            printf("\t\t%-40s=%20s\n", "SI Window Length", "2ms");
            break;
        case LIBLTE_RRC_SI_WINDOW_LENGTH_MS5:
            si_win_len = 5;
            printf("\t\t%-40s=%20s\n", "SI Window Length", "5ms");
            break;
        case LIBLTE_RRC_SI_WINDOW_LENGTH_MS10:
            si_win_len = 10;
            printf("\t\t%-40s=%20s\n", "SI Window Length", "10ms");
            break;
        case LIBLTE_RRC_SI_WINDOW_LENGTH_MS15:
            si_win_len = 15;
            printf("\t\t%-40s=%20s\n", "SI Window Length", "15ms");
            break;
        case LIBLTE_RRC_SI_WINDOW_LENGTH_MS20:
            si_win_len = 20;
            printf("\t\t%-40s=%20s\n", "SI Window Length", "20ms");
            break;
        case LIBLTE_RRC_SI_WINDOW_LENGTH_MS40:
            si_win_len = 40;
            printf("\t\t%-40s=%20s\n", "SI Window Length", "40ms");
            break;
        }
        printf("\t\t%-40s\n", "Scheduling Info List:");
        for(i=0; i<sib1->N_sched_info; i++)
        {
            switch(sib1->sched_info[i].si_periodicity)
            {
            case LIBLTE_RRC_SI_PERIODICITY_RF8:
                si_periodicity_T = 8;
                printf("\t\t\t%s = %s\n", "SI Periodcity", "8 frames");
                break;
            case LIBLTE_RRC_SI_PERIODICITY_RF16:
                si_periodicity_T = 16;
                printf("\t\t\t%s = %s\n", "SI Periodcity", "16 frames");
                break;
            case LIBLTE_RRC_SI_PERIODICITY_RF32:
                si_periodicity_T = 32;
                printf("\t\t\t%s = %s\n", "SI Periodcity", "32 frames");
                break;
            case LIBLTE_RRC_SI_PERIODICITY_RF64:
                si_periodicity_T = 64;
                printf("\t\t\t%s = %s\n", "SI Periodcity", "64 frames");
                break;
            case LIBLTE_RRC_SI_PERIODICITY_RF128:
                si_periodicity_T = 128;
                printf("\t\t\t%s = %s\n", "SI Periodcity", "128 frames");
                break;
            case LIBLTE_RRC_SI_PERIODICITY_RF256:
                si_periodicity_T = 256;
                printf("\t\t\t%s = %s\n", "SI Periodcity", "256 frames");
                break;
            case LIBLTE_RRC_SI_PERIODICITY_RF512:
                si_periodicity_T = 512;
                printf("\t\t\t%s = %s\n", "SI Periodcity", "512 frames");
                break;
            }
            printf("\t\t\tSI Window Starts at N_subframe = %u, SFN mod %u = %u\n", (i * si_win_len) % 10, si_periodicity_T, (i * si_win_len)/10);
            if(0 == i)
            {
                printf("\t\t\t\t%s = %s\n", "SIB Type", "2");
            }
            for(j=0; j<sib1->sched_info[i].N_sib_mapping_info; j++)
            {
                switch(sib1->sched_info[i].sib_mapping_info[j].sib_type)
                {
                case LIBLTE_RRC_SIB_TYPE_3:
                    printf("\t\t\t\t%s = %s\n", "SIB Type", "3");
                    sib3_expected = true;
                    break;
                case LIBLTE_RRC_SIB_TYPE_4:
                    printf("\t\t\t\t%s = %s\n", "SIB Type", "4");
                    sib4_expected = true;
                    break;
                case LIBLTE_RRC_SIB_TYPE_5:
                    printf("\t\t\t\t%s = %s\n", "SIB Type", "5");
                    break;
                case LIBLTE_RRC_SIB_TYPE_6:
                    printf("\t\t\t\t%s = %s\n", "SIB Type", "6");
                    break;
                case LIBLTE_RRC_SIB_TYPE_7:
                    printf("\t\t\t\t%s = %s\n", "SIB Type", "7");
                    break;
                case LIBLTE_RRC_SIB_TYPE_8:
                    printf("\t\t\t\t%s = %s\n", "SIB Type", "8");
                    sib8_expected = true;
                    break;
                case LIBLTE_RRC_SIB_TYPE_9:
                    printf("\t\t\t\t%s = %s\n", "SIB Type", "9");
                    break;
                case LIBLTE_RRC_SIB_TYPE_10:
                    printf("\t\t\t\t%s = %s\n", "SIB Type", "10");
                    break;
                case LIBLTE_RRC_SIB_TYPE_11:
                    printf("\t\t\t\t%s = %s\n", "SIB Type", "11");
                    break;
                case LIBLTE_RRC_SIB_TYPE_12_v920:
                    printf("\t\t\t\t%s = %s\n", "SIB Type", "12-v920");
                    break;
                case LIBLTE_RRC_SIB_TYPE_13_v920:
                    printf("\t\t\t\t%s = %s\n", "SIB Type", "13-v920");
                    break;
                case LIBLTE_RRC_SIB_TYPE_SPARE_5:
                    printf("\t\t\t\t%s = %s\n", "SIB Type", "SPARE5");
                    break;
                case LIBLTE_RRC_SIB_TYPE_SPARE_4:
                    printf("\t\t\t\t%s = %s\n", "SIB Type", "SPARE4");
                    break;
                case LIBLTE_RRC_SIB_TYPE_SPARE_3:
                    printf("\t\t\t\t%s = %s\n", "SIB Type", "SPARE3");
                    break;
                case LIBLTE_RRC_SIB_TYPE_SPARE_2:
                    printf("\t\t\t\t%s = %s\n", "SIB Type", "SPARE2");
                    break;
                case LIBLTE_RRC_SIB_TYPE_SPARE_1:
                    printf("\t\t\t\t%s = %s\n", "SIB Type", "SPARE1");
                    break;
                }
            }
        }
        if(false == sib1->tdd)
        {
            printf("\t\t%-40s=%20s\n", "Duplexing Mode", "FDD");
        }else{
            printf("\t\t%-40s=%20s\n", "Duplexing Mode", "TDD");
            switch(sib1->sf_assignment)
            {
            case LIBLTE_RRC_SUBFRAME_ASSIGNMENT_0:
                printf("\t\t%-40s=%20s\n", "Subframe Assignment", "SA0");
                break;
            case LIBLTE_RRC_SUBFRAME_ASSIGNMENT_1:
                printf("\t\t%-40s=%20s\n", "Subframe Assignment", "SA1");
                break;
            case LIBLTE_RRC_SUBFRAME_ASSIGNMENT_2:
                printf("\t\t%-40s=%20s\n", "Subframe Assignment", "SA2");
                break;
            case LIBLTE_RRC_SUBFRAME_ASSIGNMENT_3:
                printf("\t\t%-40s=%20s\n", "Subframe Assignment", "SA3");
                break;
            case LIBLTE_RRC_SUBFRAME_ASSIGNMENT_4:
                printf("\t\t%-40s=%20s\n", "Subframe Assignment", "SA4");
                break;
            case LIBLTE_RRC_SUBFRAME_ASSIGNMENT_5:
                printf("\t\t%-40s=%20s\n", "Subframe Assignment", "SA5");
                break;
            case LIBLTE_RRC_SUBFRAME_ASSIGNMENT_6:
                printf("\t\t%-40s=%20s\n", "Subframe Assignment", "SA6");
                break;
            }
            switch(sib1->special_sf_patterns)
            {
            case LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_0:
                printf("\t\t%-40s=%20s\n", "Special Subframe Patterns", "SSP0");
                break;
            case LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_1:
                printf("\t\t%-40s=%20s\n", "Special Subframe Patterns", "SSP1");
                break;
            case LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_2:
                printf("\t\t%-40s=%20s\n", "Special Subframe Patterns", "SSP2");
                break;
            case LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_3:
                printf("\t\t%-40s=%20s\n", "Special Subframe Patterns", "SSP3");
                break;
            case LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_4:
                printf("\t\t%-40s=%20s\n", "Special Subframe Patterns", "SSP4");
                break;
            case LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_5:
                printf("\t\t%-40s=%20s\n", "Special Subframe Patterns", "SSP5");
                break;
            case LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_6:
                printf("\t\t%-40s=%20s\n", "Special Subframe Patterns", "SSP6");
                break;
            case LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_7:
                printf("\t\t%-40s=%20s\n", "Special Subframe Patterns", "SSP7");
                break;
            case LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_8:
                printf("\t\t%-40s=%20s\n", "Special Subframe Patterns", "SSP8");
                break;
            }
        }
        printf("\t\t%-40s=%20u\n", "SI Value Tag", sib1->system_info_value_tag);
        prev_si_value_tag       = sib1->system_info_value_tag;
        prev_si_value_tag_valid = true;

        sib1_printed = true;
    }
}

void LTE_fdd_dl_fs_samp_buf::print_sib2(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_2_STRUCT *sib2)
{
    uint32 coeff = 0;
    uint32 T     = 0;
    uint32 i;

    if(false == sib2_printed)
    {
        printf("\tSIB2 Decoded:\n");
        if(true == sib2->ac_barring_info_present)
        {
            if(true == sib2->ac_barring_for_emergency)
            {
                printf("\t\t%-40s=%20s\n", "AC Barring for Emergency", "Barred");
            }else{
                printf("\t\t%-40s=%20s\n", "AC Barring for Emergency", "Not Barred");
            }
            if(true == sib2->ac_barring_for_mo_signalling.enabled)
            {
                printf("\t\t%-40s=%20s\n", "AC Barring for MO Signalling", "Barred");
                switch(sib2->ac_barring_for_mo_signalling.factor)
                {
                case LIBLTE_RRC_AC_BARRING_FACTOR_P00:
                    printf("\t\t\t%-40s=%20s\n", "Factor", "0.00");
                    break;
                case LIBLTE_RRC_AC_BARRING_FACTOR_P05:
                    printf("\t\t\t%-40s=%20s\n", "Factor", "0.05");
                    break;
                case LIBLTE_RRC_AC_BARRING_FACTOR_P10:
                    printf("\t\t\t%-40s=%20s\n", "Factor", "0.10");
                    break;
                case LIBLTE_RRC_AC_BARRING_FACTOR_P15:
                    printf("\t\t\t%-40s=%20s\n", "Factor", "0.15");
                    break;
                case LIBLTE_RRC_AC_BARRING_FACTOR_P20:
                    printf("\t\t\t%-40s=%20s\n", "Factor", "0.20");
                    break;
                case LIBLTE_RRC_AC_BARRING_FACTOR_P25:
                    printf("\t\t\t%-40s=%20s\n", "Factor", "0.25");
                    break;
                case LIBLTE_RRC_AC_BARRING_FACTOR_P30:
                    printf("\t\t\t%-40s=%20s\n", "Factor", "0.30");
                    break;
                case LIBLTE_RRC_AC_BARRING_FACTOR_P40:
                    printf("\t\t\t%-40s=%20s\n", "Factor", "0.40");
                    break;
                case LIBLTE_RRC_AC_BARRING_FACTOR_P50:
                    printf("\t\t\t%-40s=%20s\n", "Factor", "0.50");
                    break;
                case LIBLTE_RRC_AC_BARRING_FACTOR_P60:
                    printf("\t\t\t%-40s=%20s\n", "Factor", "0.60");
                    break;
                case LIBLTE_RRC_AC_BARRING_FACTOR_P70:
                    printf("\t\t\t%-40s=%20s\n", "Factor", "0.70");
                    break;
                case LIBLTE_RRC_AC_BARRING_FACTOR_P75:
                    printf("\t\t\t%-40s=%20s\n", "Factor", "0.75");
                    break;
                case LIBLTE_RRC_AC_BARRING_FACTOR_P80:
                    printf("\t\t\t%-40s=%20s\n", "Factor", "0.80");
                    break;
                case LIBLTE_RRC_AC_BARRING_FACTOR_P85:
                    printf("\t\t\t%-40s=%20s\n", "Factor", "0.85");
                    break;
                case LIBLTE_RRC_AC_BARRING_FACTOR_P90:
                    printf("\t\t\t%-40s=%20s\n", "Factor", "0.90");
                    break;
                case LIBLTE_RRC_AC_BARRING_FACTOR_P95:
                    printf("\t\t\t%-40s=%20s\n", "Factor", "0.95");
                    break;
                }
                switch(sib2->ac_barring_for_mo_signalling.time)
                {
                case LIBLTE_RRC_AC_BARRING_TIME_S4:
                    printf("\t\t\t%-40s=%20s\n", "Time", "4s");
                    break;
                case LIBLTE_RRC_AC_BARRING_TIME_S8:
                    printf("\t\t\t%-40s=%20s\n", "Time", "8s");
                    break;
                case LIBLTE_RRC_AC_BARRING_TIME_S16:
                    printf("\t\t\t%-40s=%20s\n", "Time", "16s");
                    break;
                case LIBLTE_RRC_AC_BARRING_TIME_S32:
                    printf("\t\t\t%-40s=%20s\n", "Time", "32s");
                    break;
                case LIBLTE_RRC_AC_BARRING_TIME_S64:
                    printf("\t\t\t%-40s=%20s\n", "Time", "64s");
                    break;
                case LIBLTE_RRC_AC_BARRING_TIME_S128:
                    printf("\t\t\t%-40s=%20s\n", "Time", "128s");
                    break;
                case LIBLTE_RRC_AC_BARRING_TIME_S256:
                    printf("\t\t\t%-40s=%20s\n", "Time", "256s");
                    break;
                case LIBLTE_RRC_AC_BARRING_TIME_S512:
                    printf("\t\t\t%-40s=%20s\n", "Time", "512s");
                    break;
                }
                printf("\t\t\t%-40s=%20u\n", "Special AC", sib2->ac_barring_for_mo_signalling.for_special_ac);
            }else{
                printf("\t\t%-40s=%20s\n", "AC Barring for MO Signalling", "Not Barred");
            }
            if(true == sib2->ac_barring_for_mo_data.enabled)
            {
                printf("\t\t%-40s=%20s\n", "AC Barring for MO Data", "Barred");
                switch(sib2->ac_barring_for_mo_data.factor)
                {
                case LIBLTE_RRC_AC_BARRING_FACTOR_P00:
                    printf("\t\t\t%-40s=%20s\n", "Factor", "0.00");
                    break;
                case LIBLTE_RRC_AC_BARRING_FACTOR_P05:
                    printf("\t\t\t%-40s=%20s\n", "Factor", "0.05");
                    break;
                case LIBLTE_RRC_AC_BARRING_FACTOR_P10:
                    printf("\t\t\t%-40s=%20s\n", "Factor", "0.10");
                    break;
                case LIBLTE_RRC_AC_BARRING_FACTOR_P15:
                    printf("\t\t\t%-40s=%20s\n", "Factor", "0.15");
                    break;
                case LIBLTE_RRC_AC_BARRING_FACTOR_P20:
                    printf("\t\t\t%-40s=%20s\n", "Factor", "0.20");
                    break;
                case LIBLTE_RRC_AC_BARRING_FACTOR_P25:
                    printf("\t\t\t%-40s=%20s\n", "Factor", "0.25");
                    break;
                case LIBLTE_RRC_AC_BARRING_FACTOR_P30:
                    printf("\t\t\t%-40s=%20s\n", "Factor", "0.30");
                    break;
                case LIBLTE_RRC_AC_BARRING_FACTOR_P40:
                    printf("\t\t\t%-40s=%20s\n", "Factor", "0.40");
                    break;
                case LIBLTE_RRC_AC_BARRING_FACTOR_P50:
                    printf("\t\t\t%-40s=%20s\n", "Factor", "0.50");
                    break;
                case LIBLTE_RRC_AC_BARRING_FACTOR_P60:
                    printf("\t\t\t%-40s=%20s\n", "Factor", "0.60");
                    break;
                case LIBLTE_RRC_AC_BARRING_FACTOR_P70:
                    printf("\t\t\t%-40s=%20s\n", "Factor", "0.70");
                    break;
                case LIBLTE_RRC_AC_BARRING_FACTOR_P75:
                    printf("\t\t\t%-40s=%20s\n", "Factor", "0.75");
                    break;
                case LIBLTE_RRC_AC_BARRING_FACTOR_P80:
                    printf("\t\t\t%-40s=%20s\n", "Factor", "0.80");
                    break;
                case LIBLTE_RRC_AC_BARRING_FACTOR_P85:
                    printf("\t\t\t%-40s=%20s\n", "Factor", "0.85");
                    break;
                case LIBLTE_RRC_AC_BARRING_FACTOR_P90:
                    printf("\t\t\t%-40s=%20s\n", "Factor", "0.90");
                    break;
                case LIBLTE_RRC_AC_BARRING_FACTOR_P95:
                    printf("\t\t\t%-40s=%20s\n", "Factor", "0.95");
                    break;
                }
                switch(sib2->ac_barring_for_mo_data.time)
                {
                case LIBLTE_RRC_AC_BARRING_TIME_S4:
                    printf("\t\t\t%-40s=%20s\n", "Time", "4s");
                    break;
                case LIBLTE_RRC_AC_BARRING_TIME_S8:
                    printf("\t\t\t%-40s=%20s\n", "Time", "8s");
                    break;
                case LIBLTE_RRC_AC_BARRING_TIME_S16:
                    printf("\t\t\t%-40s=%20s\n", "Time", "16s");
                    break;
                case LIBLTE_RRC_AC_BARRING_TIME_S32:
                    printf("\t\t\t%-40s=%20s\n", "Time", "32s");
                    break;
                case LIBLTE_RRC_AC_BARRING_TIME_S64:
                    printf("\t\t\t%-40s=%20s\n", "Time", "64s");
                    break;
                case LIBLTE_RRC_AC_BARRING_TIME_S128:
                    printf("\t\t\t%-40s=%20s\n", "Time", "128s");
                    break;
                case LIBLTE_RRC_AC_BARRING_TIME_S256:
                    printf("\t\t\t%-40s=%20s\n", "Time", "256s");
                    break;
                case LIBLTE_RRC_AC_BARRING_TIME_S512:
                    printf("\t\t\t%-40s=%20s\n", "Time", "512s");
                    break;
                }
                printf("\t\t\t%-40s=%20u\n", "Special AC", sib2->ac_barring_for_mo_data.for_special_ac);
            }else{
                printf("\t\t%-40s=%20s\n", "AC Barring for MO Data", "Not Barred");
            }
        }
        switch(sib2->rr_config_common_sib.rach_cnfg.num_ra_preambles)
        {
        case LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N4:
            printf("\t\t%-40s=%20s\n", "Number of RACH Preambles", "4");
            break;
        case LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N8:
            printf("\t\t%-40s=%20s\n", "Number of RACH Preambles", "8");
            break;
        case LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N12:
            printf("\t\t%-40s=%20s\n", "Number of RACH Preambles", "12");
            break;
        case LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N16:
            printf("\t\t%-40s=%20s\n", "Number of RACH Preambles", "16");
            break;
        case LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N20:
            printf("\t\t%-40s=%20s\n", "Number of RACH Preambles", "20");
            break;
        case LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N24:
            printf("\t\t%-40s=%20s\n", "Number of RACH Preambles", "24");
            break;
        case LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N28:
            printf("\t\t%-40s=%20s\n", "Number of RACH Preambles", "28");
            break;
        case LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N32:
            printf("\t\t%-40s=%20s\n", "Number of RACH Preambles", "32");
            break;
        case LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N36:
            printf("\t\t%-40s=%20s\n", "Number of RACH Preambles", "36");
            break;
        case LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N40:
            printf("\t\t%-40s=%20s\n", "Number of RACH Preambles", "40");
            break;
        case LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N44:
            printf("\t\t%-40s=%20s\n", "Number of RACH Preambles", "44");
            break;
        case LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N48:
            printf("\t\t%-40s=%20s\n", "Number of RACH Preambles", "48");
            break;
        case LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N52:
            printf("\t\t%-40s=%20s\n", "Number of RACH Preambles", "52");
            break;
        case LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N56:
            printf("\t\t%-40s=%20s\n", "Number of RACH Preambles", "56");
            break;
        case LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N60:
            printf("\t\t%-40s=%20s\n", "Number of RACH Preambles", "60");
            break;
        case LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N64:
            printf("\t\t%-40s=%20s\n", "Number of RACH Preambles", "64");
            break;
        }
        if(true == sib2->rr_config_common_sib.rach_cnfg.preambles_group_a_cnfg.present)
        {
            switch(sib2->rr_config_common_sib.rach_cnfg.preambles_group_a_cnfg.size_of_ra)
            {
            case LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_N4:
                printf("\t\t%-40s=%20s\n", "Size of RACH Preambles Group A", "4");
                break;
            case LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_N8:
                printf("\t\t%-40s=%20s\n", "Size of RACH Preambles Group A", "8");
                break;
            case LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_N12:
                printf("\t\t%-40s=%20s\n", "Size of RACH Preambles Group A", "12");
                break;
            case LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_N16:
                printf("\t\t%-40s=%20s\n", "Size of RACH Preambles Group A", "16");
                break;
            case LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_N20:
                printf("\t\t%-40s=%20s\n", "Size of RACH Preambles Group A", "20");
                break;
            case LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_N24:
                printf("\t\t%-40s=%20s\n", "Size of RACH Preambles Group A", "24");
                break;
            case LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_N28:
                printf("\t\t%-40s=%20s\n", "Size of RACH Preambles Group A", "28");
                break;
            case LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_N32:
                printf("\t\t%-40s=%20s\n", "Size of RACH Preambles Group A", "32");
                break;
            case LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_N36:
                printf("\t\t%-40s=%20s\n", "Size of RACH Preambles Group A", "36");
                break;
            case LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_N40:
                printf("\t\t%-40s=%20s\n", "Size of RACH Preambles Group A", "40");
                break;
            case LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_N44:
                printf("\t\t%-40s=%20s\n", "Size of RACH Preambles Group A", "44");
                break;
            case LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_N48:
                printf("\t\t%-40s=%20s\n", "Size of RACH Preambles Group A", "48");
                break;
            case LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_N52:
                printf("\t\t%-40s=%20s\n", "Size of RACH Preambles Group A", "52");
                break;
            case LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_N56:
                printf("\t\t%-40s=%20s\n", "Size of RACH Preambles Group A", "56");
                break;
            case LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_N60:
                printf("\t\t%-40s=%20s\n", "Size of RACH Preambles Group A", "60");
                break;
            }
            switch(sib2->rr_config_common_sib.rach_cnfg.preambles_group_a_cnfg.msg_size)
            {
            case LIBLTE_RRC_MESSAGE_SIZE_GROUP_A_B56:
                printf("\t\t%-40s=%20s\n", "Message Size Group A", "56 bits");
                break;
            case LIBLTE_RRC_MESSAGE_SIZE_GROUP_A_B144:
                printf("\t\t%-40s=%20s\n", "Message Size Group A", "144 bits");
                break;
            case LIBLTE_RRC_MESSAGE_SIZE_GROUP_A_B208:
                printf("\t\t%-40s=%20s\n", "Message Size Group A", "208 bits");
                break;
            case LIBLTE_RRC_MESSAGE_SIZE_GROUP_A_B256:
                printf("\t\t%-40s=%20s\n", "Message Size Group A", "256 bits");
                break;
            }
            switch(sib2->rr_config_common_sib.rach_cnfg.preambles_group_a_cnfg.msg_pwr_offset_group_b)
            {
            case LIBLTE_RRC_MESSAGE_POWER_OFFSET_GROUP_B_MINUS_INFINITY:
                printf("\t\t%-40s=%20s\n", "Message Power Offset Group B", "-Infinity dB");
                break;
            case LIBLTE_RRC_MESSAGE_POWER_OFFSET_GROUP_B_DB0:
                printf("\t\t%-40s=%20s\n", "Message Power Offset Group B", "0dB");
                break;
            case LIBLTE_RRC_MESSAGE_POWER_OFFSET_GROUP_B_DB5:
                printf("\t\t%-40s=%20s\n", "Message Power Offset Group B", "5dB");
                break;
            case LIBLTE_RRC_MESSAGE_POWER_OFFSET_GROUP_B_DB8:
                printf("\t\t%-40s=%20s\n", "Message Power Offset Group B", "8dB");
                break;
            case LIBLTE_RRC_MESSAGE_POWER_OFFSET_GROUP_B_DB10:
                printf("\t\t%-40s=%20s\n", "Message Power Offset Group B", "10dB");
                break;
            case LIBLTE_RRC_MESSAGE_POWER_OFFSET_GROUP_B_DB12:
                printf("\t\t%-40s=%20s\n", "Message Power Offset Group B", "12dB");
                break;
            case LIBLTE_RRC_MESSAGE_POWER_OFFSET_GROUP_B_DB15:
                printf("\t\t%-40s=%20s\n", "Message Power Offset Group B", "15dB");
                break;
            case LIBLTE_RRC_MESSAGE_POWER_OFFSET_GROUP_B_DB18:
                printf("\t\t%-40s=%20s\n", "Message Power Offset Group B", "18dB");
                break;
            }
        }
        switch(sib2->rr_config_common_sib.rach_cnfg.pwr_ramping_step)
        {
        case LIBLTE_RRC_POWER_RAMPING_STEP_DB0:
            printf("\t\t%-40s=%20s\n", "Power Ramping Step", "0dB");
            break;
        case LIBLTE_RRC_POWER_RAMPING_STEP_DB2:
            printf("\t\t%-40s=%20s\n", "Power Ramping Step", "2dB");
            break;
        case LIBLTE_RRC_POWER_RAMPING_STEP_DB4:
            printf("\t\t%-40s=%20s\n", "Power Ramping Step", "4dB");
            break;
        case LIBLTE_RRC_POWER_RAMPING_STEP_DB6:
            printf("\t\t%-40s=%20s\n", "Power Ramping Step", "6dB");
            break;
        }
        switch(sib2->rr_config_common_sib.rach_cnfg.preamble_init_rx_target_pwr)
        {
        case LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N120:
            printf("\t\t%-40s=%20s\n", "Preamble init target RX power", "-120dBm");
            break;
        case LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N118:
            printf("\t\t%-40s=%20s\n", "Preamble init target RX power", "-118dBm");
            break;
        case LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N116:
            printf("\t\t%-40s=%20s\n", "Preamble init target RX power", "-116dBm");
            break;
        case LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N114:
            printf("\t\t%-40s=%20s\n", "Preamble init target RX power", "-114dBm");
            break;
        case LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N112:
            printf("\t\t%-40s=%20s\n", "Preamble init target RX power", "-112dBm");
            break;
        case LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N110:
            printf("\t\t%-40s=%20s\n", "Preamble init target RX power", "-110dBm");
            break;
        case LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N108:
            printf("\t\t%-40s=%20s\n", "Preamble init target RX power", "-108dBm");
            break;
        case LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N106:
            printf("\t\t%-40s=%20s\n", "Preamble init target RX power", "-106dBm");
            break;
        case LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N104:
            printf("\t\t%-40s=%20s\n", "Preamble init target RX power", "-104dBm");
            break;
        case LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N102:
            printf("\t\t%-40s=%20s\n", "Preamble init target RX power", "-102dBm");
            break;
        case LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N100:
            printf("\t\t%-40s=%20s\n", "Preamble init target RX power", "-100dBm");
            break;
        case LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N98:
            printf("\t\t%-40s=%20s\n", "Preamble init target RX power", "-98dBm");
            break;
        case LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N96:
            printf("\t\t%-40s=%20s\n", "Preamble init target RX power", "-96dBm");
            break;
        case LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N94:
            printf("\t\t%-40s=%20s\n", "Preamble init target RX power", "-94dBm");
            break;
        case LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N92:
            printf("\t\t%-40s=%20s\n", "Preamble init target RX power", "-92dBm");
            break;
        case LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N90:
            printf("\t\t%-40s=%20s\n", "Preamble init target RX power", "-90dBm");
            break;
        }
        switch(sib2->rr_config_common_sib.rach_cnfg.preamble_trans_max)
        {
        case LIBLTE_RRC_PREAMBLE_TRANS_MAX_N3:
            printf("\t\t%-40s=%20s\n", "Preamble TX Max", "3");
            break;
        case LIBLTE_RRC_PREAMBLE_TRANS_MAX_N4:
            printf("\t\t%-40s=%20s\n", "Preamble TX Max", "4");
            break;
        case LIBLTE_RRC_PREAMBLE_TRANS_MAX_N5:
            printf("\t\t%-40s=%20s\n", "Preamble TX Max", "5");
            break;
        case LIBLTE_RRC_PREAMBLE_TRANS_MAX_N6:
            printf("\t\t%-40s=%20s\n", "Preamble TX Max", "6");
            break;
        case LIBLTE_RRC_PREAMBLE_TRANS_MAX_N7:
            printf("\t\t%-40s=%20s\n", "Preamble TX Max", "7");
            break;
        case LIBLTE_RRC_PREAMBLE_TRANS_MAX_N8:
            printf("\t\t%-40s=%20s\n", "Preamble TX Max", "8");
            break;
        case LIBLTE_RRC_PREAMBLE_TRANS_MAX_N10:
            printf("\t\t%-40s=%20s\n", "Preamble TX Max", "10");
            break;
        case LIBLTE_RRC_PREAMBLE_TRANS_MAX_N20:
            printf("\t\t%-40s=%20s\n", "Preamble TX Max", "20");
            break;
        case LIBLTE_RRC_PREAMBLE_TRANS_MAX_N50:
            printf("\t\t%-40s=%20s\n", "Preamble TX Max", "50");
            break;
        case LIBLTE_RRC_PREAMBLE_TRANS_MAX_N100:
            printf("\t\t%-40s=%20s\n", "Preamble TX Max", "100");
            break;
        case LIBLTE_RRC_PREAMBLE_TRANS_MAX_N200:
            printf("\t\t%-40s=%20s\n", "Preamble TX Max", "200");
            break;
        }
        switch(sib2->rr_config_common_sib.rach_cnfg.ra_resp_win_size)
        {
        case LIBLTE_RRC_RA_RESPONSE_WINDOW_SIZE_SF2:
            printf("\t\t%-40s=%20s\n", "RA Response Window Size", "2 Subframes");
            break;
        case LIBLTE_RRC_RA_RESPONSE_WINDOW_SIZE_SF3:
            printf("\t\t%-40s=%20s\n", "RA Response Window Size", "3 Subframes");
            break;
        case LIBLTE_RRC_RA_RESPONSE_WINDOW_SIZE_SF4:
            printf("\t\t%-40s=%20s\n", "RA Response Window Size", "4 Subframes");
            break;
        case LIBLTE_RRC_RA_RESPONSE_WINDOW_SIZE_SF5:
            printf("\t\t%-40s=%20s\n", "RA Response Window Size", "5 Subframes");
            break;
        case LIBLTE_RRC_RA_RESPONSE_WINDOW_SIZE_SF6:
            printf("\t\t%-40s=%20s\n", "RA Response Window Size", "6 Subframes");
            break;
        case LIBLTE_RRC_RA_RESPONSE_WINDOW_SIZE_SF7:
            printf("\t\t%-40s=%20s\n", "RA Response Window Size", "7 Subframes");
            break;
        case LIBLTE_RRC_RA_RESPONSE_WINDOW_SIZE_SF8:
            printf("\t\t%-40s=%20s\n", "RA Response Window Size", "8 Subframes");
            break;
        case LIBLTE_RRC_RA_RESPONSE_WINDOW_SIZE_SF10:
            printf("\t\t%-40s=%20s\n", "RA Response Window Size", "10 Subframes");
            break;
        }
        switch(sib2->rr_config_common_sib.rach_cnfg.mac_con_res_timer)
        {
        case LIBLTE_RRC_MAC_CONTENTION_RESOLUTION_TIMER_SF8:
            printf("\t\t%-40s=%20s\n", "MAC Contention Resolution Timer", "8 Subframes");
            break;
        case LIBLTE_RRC_MAC_CONTENTION_RESOLUTION_TIMER_SF16:
            printf("\t\t%-40s=%20s\n", "MAC Contention Resolution Timer", "16 Subframes");
            break;
        case LIBLTE_RRC_MAC_CONTENTION_RESOLUTION_TIMER_SF24:
            printf("\t\t%-40s=%20s\n", "MAC Contention Resolution Timer", "24 Subframes");
            break;
        case LIBLTE_RRC_MAC_CONTENTION_RESOLUTION_TIMER_SF32:
            printf("\t\t%-40s=%20s\n", "MAC Contention Resolution Timer", "32 Subframes");
            break;
        case LIBLTE_RRC_MAC_CONTENTION_RESOLUTION_TIMER_SF40:
            printf("\t\t%-40s=%20s\n", "MAC Contention Resolution Timer", "40 Subframes");
            break;
        case LIBLTE_RRC_MAC_CONTENTION_RESOLUTION_TIMER_SF48:
            printf("\t\t%-40s=%20s\n", "MAC Contention Resolution Timer", "48 Subframes");
            break;
        case LIBLTE_RRC_MAC_CONTENTION_RESOLUTION_TIMER_SF56:
            printf("\t\t%-40s=%20s\n", "MAC Contention Resolution Timer", "56 Subframes");
            break;
        case LIBLTE_RRC_MAC_CONTENTION_RESOLUTION_TIMER_SF64:
            printf("\t\t%-40s=%20s\n", "MAC Contention Resolution Timer", "64 Subframes");
            break;
        }
        printf("\t\t%-40s=%20u\n", "Max num HARQ TX for Message 3", sib2->rr_config_common_sib.rach_cnfg.max_harq_msg3_tx);
        switch(sib2->rr_config_common_sib.bcch_cnfg.modification_period_coeff)
        {
        case LIBLTE_RRC_MODIFICATION_PERIOD_COEFF_N2:
            printf("\t\t%-40s=%20s\n", "Modification Period Coeff", "2");
            coeff = 2;
            break;
        case LIBLTE_RRC_MODIFICATION_PERIOD_COEFF_N4:
            printf("\t\t%-40s=%20s\n", "Modification Period Coeff", "4");
            coeff = 4;
            break;
        case LIBLTE_RRC_MODIFICATION_PERIOD_COEFF_N8:
            printf("\t\t%-40s=%20s\n", "Modification Period Coeff", "8");
            coeff = 8;
            break;
        case LIBLTE_RRC_MODIFICATION_PERIOD_COEFF_N16:
            printf("\t\t%-40s=%20s\n", "Modification Period Coeff", "16");
            coeff = 16;
            break;
        }
        switch(sib2->rr_config_common_sib.pcch_cnfg.default_paging_cycle)
        {
        case LIBLTE_RRC_DEFAULT_PAGING_CYCLE_RF32:
            printf("\t\t%-40s=%20s\n", "Default Paging Cycle", "32 Frames");
            T = 32;
            break;
        case LIBLTE_RRC_DEFAULT_PAGING_CYCLE_RF64:
            printf("\t\t%-40s=%20s\n", "Default Paging Cycle", "64 Frames");
            T = 64;
            break;
        case LIBLTE_RRC_DEFAULT_PAGING_CYCLE_RF128:
            printf("\t\t%-40s=%20s\n", "Default Paging Cycle", "128 Frames");
            T = 128;
            break;
        case LIBLTE_RRC_DEFAULT_PAGING_CYCLE_RF256:
            printf("\t\t%-40s=%20s\n", "Default Paging Cycle", "256 Frames");
            T = 256;
            break;
        }
        printf("\t\t%-40s=%13u Frames\n", "Modification Period", coeff * T);
        switch(sib2->rr_config_common_sib.pcch_cnfg.nB)
        {
        case LIBLTE_RRC_NB_FOUR_T:
            printf("\t\t%-40s=%13u Frames\n", "nB", 4*T);
            break;
        case LIBLTE_RRC_NB_TWO_T:
            printf("\t\t%-40s=%13u Frames\n", "nB", 2*T);
            break;
        case LIBLTE_RRC_NB_ONE_T:
            printf("\t\t%-40s=%13u Frames\n", "nB", T);
            break;
        case LIBLTE_RRC_NB_HALF_T:
            printf("\t\t%-40s=%13u Frames\n", "nB", T/2);
            break;
        case LIBLTE_RRC_NB_QUARTER_T:
            printf("\t\t%-40s=%13u Frames\n", "nB", T/4);
            break;
        case LIBLTE_RRC_NB_ONE_EIGHTH_T:
            printf("\t\t%-40s=%13u Frames\n", "nB", T/8);
            break;
        case LIBLTE_RRC_NB_ONE_SIXTEENTH_T:
            printf("\t\t%-40s=%13u Frames\n", "nB", T/16);
            break;
        case LIBLTE_RRC_NB_ONE_THIRTY_SECOND_T:
            printf("\t\t%-40s=%13u Frames\n", "nB", T/32);
            break;
        }
        printf("\t\t%-40s=%20u\n", "Root Sequence Index", sib2->rr_config_common_sib.prach_cnfg.root_sequence_index);
        printf("\t\t%-40s=%20u\n", "PRACH Config Index", sib2->rr_config_common_sib.prach_cnfg.prach_cnfg_info.prach_config_index);
        if(true == sib2->rr_config_common_sib.prach_cnfg.prach_cnfg_info.high_speed_flag)
        {
            printf("\t\t%-40s=%20s\n", "High Speed Flag", "Restricted Set");
        }else{
            printf("\t\t%-40s=%20s\n", "High Speed Flag", "Unrestricted Set");
        }
        printf("\t\t%-40s=%20u\n", "Ncs Configuration", sib2->rr_config_common_sib.prach_cnfg.prach_cnfg_info.zero_correlation_zone_config);
        printf("\t\t%-40s=%20u\n", "PRACH Freq Offset", sib2->rr_config_common_sib.prach_cnfg.prach_cnfg_info.prach_freq_offset);
        printf("\t\t%-40s=%17ddBm\n", "Reference Signal Power", sib2->rr_config_common_sib.pdsch_cnfg.rs_power);
        printf("\t\t%-40s=%20u\n", "Pb", sib2->rr_config_common_sib.pdsch_cnfg.p_b);
        printf("\t\t%-40s=%20u\n", "Nsb", sib2->rr_config_common_sib.pusch_cnfg.n_sb);
        switch(sib2->rr_config_common_sib.pusch_cnfg.hopping_mode)
        {
        case LIBLTE_RRC_HOPPING_MODE_INTER_SUBFRAME:
            printf("\t\t%-40s=%20s\n", "Hopping Mode", "Inter Subframe");
            break;
        case LIBLTE_RRC_HOPPING_MODE_INTRA_AND_INTER_SUBFRAME:
            printf("\t\t%-40s= %s\n", "Hopping Mode", "Intra and Inter Subframe");
            break;
        }
        printf("\t\t%-40s=%20u\n", "PUSCH Nrb Hopping Offset", sib2->rr_config_common_sib.pusch_cnfg.pusch_hopping_offset);
        if(true == sib2->rr_config_common_sib.pusch_cnfg.enable_64_qam)
        {
            printf("\t\t%-40s=%20s\n", "64QAM", "Allowed");
        }else{
            printf("\t\t%-40s=%20s\n", "64QAM", "Not Allowed");
        }
        if(true == sib2->rr_config_common_sib.pusch_cnfg.ul_rs.group_hopping_enabled)
        {
            printf("\t\t%-40s=%20s\n", "Group Hopping", "Enabled");
        }else{
            printf("\t\t%-40s=%20s\n", "Group Hopping", "Disabled");
        }
        printf("\t\t%-40s=%20u\n", "Group Assignment PUSCH", sib2->rr_config_common_sib.pusch_cnfg.ul_rs.group_assignment_pusch);
        if(true == sib2->rr_config_common_sib.pusch_cnfg.ul_rs.sequence_hopping_enabled)
        {
            printf("\t\t%-40s=%20s\n", "Sequence Hopping", "Enabled");
        }else{
            printf("\t\t%-40s=%20s\n", "Sequence Hopping", "Disabled");
        }
        printf("\t\t%-40s=%20u\n", "Cyclic Shift", sib2->rr_config_common_sib.pusch_cnfg.ul_rs.cyclic_shift);
        switch(sib2->rr_config_common_sib.pucch_cnfg.delta_pucch_shift)
        {
        case LIBLTE_RRC_DELTA_PUCCH_SHIFT_DS1:
            printf("\t\t%-40s=%20s\n", "Delta PUCCH Shift", "1");
            break;
        case LIBLTE_RRC_DELTA_PUCCH_SHIFT_DS2:
            printf("\t\t%-40s=%20s\n", "Delta PUCCH Shift", "2");
            break;
        case LIBLTE_RRC_DELTA_PUCCH_SHIFT_DS3:
            printf("\t\t%-40s=%20s\n", "Delta PUCCH Shift", "3");
            break;
        }
        printf("\t\t%-40s=%20u\n", "N_rb_cqi", sib2->rr_config_common_sib.pucch_cnfg.n_rb_cqi);
        printf("\t\t%-40s=%20u\n", "N_cs_an", sib2->rr_config_common_sib.pucch_cnfg.n_cs_an);
        printf("\t\t%-40s=%20u\n", "N1 PUCCH AN", sib2->rr_config_common_sib.pucch_cnfg.n1_pucch_an);
        if(true == sib2->rr_config_common_sib.srs_ul_cnfg.present)
        {
            switch(sib2->rr_config_common_sib.srs_ul_cnfg.bw_cnfg)
            {
            case LIBLTE_RRC_SRS_BW_CONFIG_0:
                printf("\t\t%-40s=%20s\n", "SRS Bandwidth Config", "0");
                break;
            case LIBLTE_RRC_SRS_BW_CONFIG_1:
                printf("\t\t%-40s=%20s\n", "SRS Bandwidth Config", "1");
                break;
            case LIBLTE_RRC_SRS_BW_CONFIG_2:
                printf("\t\t%-40s=%20s\n", "SRS Bandwidth Config", "2");
                break;
            case LIBLTE_RRC_SRS_BW_CONFIG_3:
                printf("\t\t%-40s=%20s\n", "SRS Bandwidth Config", "3");
                break;
            case LIBLTE_RRC_SRS_BW_CONFIG_4:
                printf("\t\t%-40s=%20s\n", "SRS Bandwidth Config", "4");
                break;
            case LIBLTE_RRC_SRS_BW_CONFIG_5:
                printf("\t\t%-40s=%20s\n", "SRS Bandwidth Config", "5");
                break;
            case LIBLTE_RRC_SRS_BW_CONFIG_6:
                printf("\t\t%-40s=%20s\n", "SRS Bandwidth Config", "6");
                break;
            case LIBLTE_RRC_SRS_BW_CONFIG_7:
                printf("\t\t%-40s=%20s\n", "SRS Bandwidth Config", "7");
                break;
            }
            switch(sib2->rr_config_common_sib.srs_ul_cnfg.subfr_cnfg)
            {
            case LIBLTE_RRC_SRS_SUBFR_CONFIG_0:
                printf("\t\t%-40s=%20s\n", "SRS Subframe Config", "0");
                break;
            case LIBLTE_RRC_SRS_SUBFR_CONFIG_1:
                printf("\t\t%-40s=%20s\n", "SRS Subframe Config", "1");
                break;
            case LIBLTE_RRC_SRS_SUBFR_CONFIG_2:
                printf("\t\t%-40s=%20s\n", "SRS Subframe Config", "2");
                break;
            case LIBLTE_RRC_SRS_SUBFR_CONFIG_3:
                printf("\t\t%-40s=%20s\n", "SRS Subframe Config", "3");
                break;
            case LIBLTE_RRC_SRS_SUBFR_CONFIG_4:
                printf("\t\t%-40s=%20s\n", "SRS Subframe Config", "4");
                break;
            case LIBLTE_RRC_SRS_SUBFR_CONFIG_5:
                printf("\t\t%-40s=%20s\n", "SRS Subframe Config", "5");
                break;
            case LIBLTE_RRC_SRS_SUBFR_CONFIG_6:
                printf("\t\t%-40s=%20s\n", "SRS Subframe Config", "6");
                break;
            case LIBLTE_RRC_SRS_SUBFR_CONFIG_7:
                printf("\t\t%-40s=%20s\n", "SRS Subframe Config", "7");
                break;
            case LIBLTE_RRC_SRS_SUBFR_CONFIG_8:
                printf("\t\t%-40s=%20s\n", "SRS Subframe Config", "8");
                break;
            case LIBLTE_RRC_SRS_SUBFR_CONFIG_9:
                printf("\t\t%-40s=%20s\n", "SRS Subframe Config", "9");
                break;
            case LIBLTE_RRC_SRS_SUBFR_CONFIG_10:
                printf("\t\t%-40s=%20s\n", "SRS Subframe Config", "10");
                break;
            case LIBLTE_RRC_SRS_SUBFR_CONFIG_11:
                printf("\t\t%-40s=%20s\n", "SRS Subframe Config", "11");
                break;
            case LIBLTE_RRC_SRS_SUBFR_CONFIG_12:
                printf("\t\t%-40s=%20s\n", "SRS Subframe Config", "12");
                break;
            case LIBLTE_RRC_SRS_SUBFR_CONFIG_13:
                printf("\t\t%-40s=%20s\n", "SRS Subframe Config", "13");
                break;
            case LIBLTE_RRC_SRS_SUBFR_CONFIG_14:
                printf("\t\t%-40s=%20s\n", "SRS Subframe Config", "14");
                break;
            case LIBLTE_RRC_SRS_SUBFR_CONFIG_15:
                printf("\t\t%-40s=%20s\n", "SRS Subframe Config", "15");
                break;
            }
            if(true == sib2->rr_config_common_sib.srs_ul_cnfg.ack_nack_simul_tx)
            {
                printf("\t\t%-40s=%20s\n", "Simultaneous AN and SRS", "True");
            }else{
                printf("\t\t%-40s=%20s\n", "Simultaneous AN and SRS", "False");
            }
            if(true == sib2->rr_config_common_sib.srs_ul_cnfg.max_up_pts_present)
            {
                printf("\t\t%-40s=%20s\n", "SRS Max Up PTS", "True");
            }else{
                printf("\t\t%-40s=%20s\n", "SRS Max Up PTS", "False");
            }
        }
        printf("\t\t%-40s=%17ddBm\n", "P0 Nominal PUSCH", sib2->rr_config_common_sib.ul_pwr_ctrl.p0_nominal_pusch);
        switch(sib2->rr_config_common_sib.ul_pwr_ctrl.alpha)
        {
        case LIBLTE_RRC_UL_POWER_CONTROL_ALPHA_0:
            printf("\t\t%-40s=%20s\n", "Alpha", "0");
            break;
        case LIBLTE_RRC_UL_POWER_CONTROL_ALPHA_04:
            printf("\t\t%-40s=%20s\n", "Alpha", "0.4");
            break;
        case LIBLTE_RRC_UL_POWER_CONTROL_ALPHA_05:
            printf("\t\t%-40s=%20s\n", "Alpha", "0.5");
            break;
        case LIBLTE_RRC_UL_POWER_CONTROL_ALPHA_06:
            printf("\t\t%-40s=%20s\n", "Alpha", "0.6");
            break;
        case LIBLTE_RRC_UL_POWER_CONTROL_ALPHA_07:
            printf("\t\t%-40s=%20s\n", "Alpha", "0.7");
            break;
        case LIBLTE_RRC_UL_POWER_CONTROL_ALPHA_08:
            printf("\t\t%-40s=%20s\n", "Alpha", "0.8");
            break;
        case LIBLTE_RRC_UL_POWER_CONTROL_ALPHA_09:
            printf("\t\t%-40s=%20s\n", "Alpha", "0.9");
            break;
        case LIBLTE_RRC_UL_POWER_CONTROL_ALPHA_1:
            printf("\t\t%-40s=%20s\n", "Alpha", "1");
            break;
        }
        printf("\t\t%-40s=%17ddBm\n", "P0 Nominal PUCCH", sib2->rr_config_common_sib.ul_pwr_ctrl.p0_nominal_pucch);
        switch(sib2->rr_config_common_sib.ul_pwr_ctrl.delta_flist_pucch.format_1)
        {
        case LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_1_NEG_2:
            printf("\t\t%-40s=%20s\n", "Delta F PUCCH Format 1", "-2dB");
            break;
        case LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_1_0:
            printf("\t\t%-40s=%20s\n", "Delta F PUCCH Format 1", "0dB");
            break;
        case LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_1_2:
            printf("\t\t%-40s=%20s\n", "Delta F PUCCH Format 1", "2dB");
            break;
        }
        switch(sib2->rr_config_common_sib.ul_pwr_ctrl.delta_flist_pucch.format_1b)
        {
        case LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_1B_1:
            printf("\t\t%-40s=%20s\n", "Delta F PUCCH Format 1B", "1dB");
            break;
        case LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_1B_3:
            printf("\t\t%-40s=%20s\n", "Delta F PUCCH Format 1B", "3dB");
            break;
        case LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_1B_5:
            printf("\t\t%-40s=%20s\n", "Delta F PUCCH Format 1B", "5dB");
            break;
        }
        switch(sib2->rr_config_common_sib.ul_pwr_ctrl.delta_flist_pucch.format_2)
        {
        case LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2_NEG_2:
            printf("\t\t%-40s=%20s\n", "Delta F PUCCH Format 2", "-2dB");
            break;
        case LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2_0:
            printf("\t\t%-40s=%20s\n", "Delta F PUCCH Format 2", "0dB");
            break;
        case LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2_1:
            printf("\t\t%-40s=%20s\n", "Delta F PUCCH Format 2", "1dB");
            break;
        case LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2_2:
            printf("\t\t%-40s=%20s\n", "Delta F PUCCH Format 2", "2dB");
            break;
        }
        switch(sib2->rr_config_common_sib.ul_pwr_ctrl.delta_flist_pucch.format_2a)
        {
        case LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2A_NEG_2:
            printf("\t\t%-40s=%20s\n", "Delta F PUCCH Format 2A", "-2dB");
            break;
        case LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2A_0:
            printf("\t\t%-40s=%20s\n", "Delta F PUCCH Format 2A", "0dB");
            break;
        case LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2A_2:
            printf("\t\t%-40s=%20s\n", "Delta F PUCCH Format 2A", "2dB");
            break;
        }
        switch(sib2->rr_config_common_sib.ul_pwr_ctrl.delta_flist_pucch.format_2b)
        {
        case LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2B_NEG_2:
            printf("\t\t%-40s=%20s\n", "Delta F PUCCH Format 2B", "-2dB");
            break;
        case LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2B_0:
            printf("\t\t%-40s=%20s\n", "Delta F PUCCH Format 2B", "0dB");
            break;
        case LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2B_2:
            printf("\t\t%-40s=%20s\n", "Delta F PUCCH Format 2B", "2dB");
            break;
        }
        printf("\t\t%-40s=%18ddB\n", "Delta Preamble Message 3", sib2->rr_config_common_sib.ul_pwr_ctrl.delta_preamble_msg3);
        switch(sib2->rr_config_common_sib.ul_cp_length)
        {
        case LIBLTE_RRC_UL_CP_LENGTH_1:
            printf("\t\t%-40s=%20s\n", "UL CP Length", "Normal");
            break;
        case LIBLTE_RRC_UL_CP_LENGTH_2:
            printf("\t\t%-40s=%20s\n", "UL CP Length", "Extended");
            break;
        }
        switch(sib2->ue_timers_and_constants.t300)
        {
        case LIBLTE_RRC_T300_MS100:
            printf("\t\t%-40s=%20s\n", "T300", "100ms");
            break;
        case LIBLTE_RRC_T300_MS200:
            printf("\t\t%-40s=%20s\n", "T300", "200ms");
            break;
        case LIBLTE_RRC_T300_MS300:
            printf("\t\t%-40s=%20s\n", "T300", "300ms");
            break;
        case LIBLTE_RRC_T300_MS400:
            printf("\t\t%-40s=%20s\n", "T300", "400ms");
            break;
        case LIBLTE_RRC_T300_MS600:
            printf("\t\t%-40s=%20s\n", "T300", "600ms");
            break;
        case LIBLTE_RRC_T300_MS1000:
            printf("\t\t%-40s=%20s\n", "T300", "1000ms");
            break;
        case LIBLTE_RRC_T300_MS1500:
            printf("\t\t%-40s=%20s\n", "T300", "1500ms");
            break;
        case LIBLTE_RRC_T300_MS2000:
            printf("\t\t%-40s=%20s\n", "T300", "2000ms");
            break;
        }
        switch(sib2->ue_timers_and_constants.t301)
        {
        case LIBLTE_RRC_T301_MS100:
            printf("\t\t%-40s=%20s\n", "T301", "100ms");
            break;
        case LIBLTE_RRC_T301_MS200:
            printf("\t\t%-40s=%20s\n", "T301", "200ms");
            break;
        case LIBLTE_RRC_T301_MS300:
            printf("\t\t%-40s=%20s\n", "T301", "300ms");
            break;
        case LIBLTE_RRC_T301_MS400:
            printf("\t\t%-40s=%20s\n", "T301", "400ms");
            break;
        case LIBLTE_RRC_T301_MS600:
            printf("\t\t%-40s=%20s\n", "T301", "600ms");
            break;
        case LIBLTE_RRC_T301_MS1000:
            printf("\t\t%-40s=%20s\n", "T301", "1000ms");
            break;
        case LIBLTE_RRC_T301_MS1500:
            printf("\t\t%-40s=%20s\n", "T301", "1500ms");
            break;
        case LIBLTE_RRC_T301_MS2000:
            printf("\t\t%-40s=%20s\n", "T301", "2000ms");
            break;
        }
        switch(sib2->ue_timers_and_constants.t310)
        {
        case LIBLTE_RRC_T310_MS0:
            printf("\t\t%-40s=%20s\n", "T310", "0ms");
            break;
        case LIBLTE_RRC_T310_MS50:
            printf("\t\t%-40s=%20s\n", "T310", "50ms");
            break;
        case LIBLTE_RRC_T310_MS100:
            printf("\t\t%-40s=%20s\n", "T310", "100ms");
            break;
        case LIBLTE_RRC_T310_MS200:
            printf("\t\t%-40s=%20s\n", "T310", "200ms");
            break;
        case LIBLTE_RRC_T310_MS500:
            printf("\t\t%-40s=%20s\n", "T310", "500ms");
            break;
        case LIBLTE_RRC_T310_MS1000:
            printf("\t\t%-40s=%20s\n", "T310", "1000ms");
            break;
        case LIBLTE_RRC_T310_MS2000:
            printf("\t\t%-40s=%20s\n", "T310", "2000ms");
            break;
        }
        switch(sib2->ue_timers_and_constants.n310)
        {
        case LIBLTE_RRC_N310_N1:
            printf("\t\t%-40s=%20s\n", "N310", "1");
            break;
        case LIBLTE_RRC_N310_N2:
            printf("\t\t%-40s=%20s\n", "N310", "2");
            break;
        case LIBLTE_RRC_N310_N3:
            printf("\t\t%-40s=%20s\n", "N310", "3");
            break;
        case LIBLTE_RRC_N310_N4:
            printf("\t\t%-40s=%20s\n", "N310", "4");
            break;
        case LIBLTE_RRC_N310_N6:
            printf("\t\t%-40s=%20s\n", "N310", "6");
            break;
        case LIBLTE_RRC_N310_N8:
            printf("\t\t%-40s=%20s\n", "N310", "8");
            break;
        case LIBLTE_RRC_N310_N10:
            printf("\t\t%-40s=%20s\n", "N310", "10");
            break;
        case LIBLTE_RRC_N310_N20:
            printf("\t\t%-40s=%20s\n", "N310", "20");
            break;
        }
        switch(sib2->ue_timers_and_constants.t311)
        {
        case LIBLTE_RRC_T311_MS1000:
            printf("\t\t%-40s=%20s\n", "T311", "1000ms");
            break;
        case LIBLTE_RRC_T311_MS3000:
            printf("\t\t%-40s=%20s\n", "T311", "3000ms");
            break;
        case LIBLTE_RRC_T311_MS5000:
            printf("\t\t%-40s=%20s\n", "T311", "5000ms");
            break;
        case LIBLTE_RRC_T311_MS10000:
            printf("\t\t%-40s=%20s\n", "T311", "10000ms");
            break;
        case LIBLTE_RRC_T311_MS15000:
            printf("\t\t%-40s=%20s\n", "T311", "15000ms");
            break;
        case LIBLTE_RRC_T311_MS20000:
            printf("\t\t%-40s=%20s\n", "T311", "20000ms");
            break;
        case LIBLTE_RRC_T311_MS30000:
            printf("\t\t%-40s=%20s\n", "T311", "30000ms");
            break;
        }
        switch(sib2->ue_timers_and_constants.n311)
        {
        case LIBLTE_RRC_N311_N1:
            printf("\t\t%-40s=%20s\n", "N311", "1");
            break;
        case LIBLTE_RRC_N311_N2:
            printf("\t\t%-40s=%20s\n", "N311", "2");
            break;
        case LIBLTE_RRC_N311_N3:
            printf("\t\t%-40s=%20s\n", "N311", "3");
            break;
        case LIBLTE_RRC_N311_N4:
            printf("\t\t%-40s=%20s\n", "N311", "4");
            break;
        case LIBLTE_RRC_N311_N5:
            printf("\t\t%-40s=%20s\n", "N311", "5");
            break;
        case LIBLTE_RRC_N311_N6:
            printf("\t\t%-40s=%20s\n", "N311", "6");
            break;
        case LIBLTE_RRC_N311_N8:
            printf("\t\t%-40s=%20s\n", "N311", "8");
            break;
        case LIBLTE_RRC_N311_N10:
            printf("\t\t%-40s=%20s\n", "N311", "10");
            break;
        }
        if(true == sib2->arfcn_value_eutra.present)
        {
            printf("\t\t%-40s=%20u\n", "UL ARFCN", sib2->arfcn_value_eutra.value);
        }
        if(true == sib2->ul_bw.present)
        {
            switch(sib2->ul_bw.bw)
            {
            case LIBLTE_RRC_UL_BW_N6:
                printf("\t\t%-40s=%20s\n", "UL Bandwidth", "1.6MHz");
                break;
            case LIBLTE_RRC_UL_BW_N15:
                printf("\t\t%-40s=%20s\n", "UL Bandwidth", "3MHz");
                break;
            case LIBLTE_RRC_UL_BW_N25:
                printf("\t\t%-40s=%20s\n", "UL Bandwidth", "5MHz");
                break;
            case LIBLTE_RRC_UL_BW_N50:
                printf("\t\t%-40s=%20s\n", "UL Bandwidth", "10MHz");
                break;
            case LIBLTE_RRC_UL_BW_N75:
                printf("\t\t%-40s=%20s\n", "UL Bandwidth", "15MHz");
                break;
            case LIBLTE_RRC_UL_BW_N100:
                printf("\t\t%-40s=%20s\n", "UL Bandwidth", "20MHz");
                break;
            }
        }
        printf("\t\t%-40s=%20u\n", "Additional Spectrum Emission", sib2->additional_spectrum_emission);
        if(0 != sib2->mbsfn_subfr_cnfg_list_size)
        {
            printf("\t\t%s:\n", "MBSFN Subframe Config List");
        }
        for(i=0; i<sib2->mbsfn_subfr_cnfg_list_size; i++)
        {
            switch(sib2->mbsfn_subfr_cnfg[i].radio_fr_alloc_period)
            {
            case LIBLTE_RRC_RADIO_FRAME_ALLOCATION_PERIOD_N1:
                printf("\t\t\t%-40s=%20s\n", "Radio Frame Alloc Period", "1");
                break;
            case LIBLTE_RRC_RADIO_FRAME_ALLOCATION_PERIOD_N2:
                printf("\t\t\t%-40s=%20s\n", "Radio Frame Alloc Period", "2");
                break;
            case LIBLTE_RRC_RADIO_FRAME_ALLOCATION_PERIOD_N4:
                printf("\t\t\t%-40s=%20s\n", "Radio Frame Alloc Period", "4");
                break;
            case LIBLTE_RRC_RADIO_FRAME_ALLOCATION_PERIOD_N8:
                printf("\t\t\t%-40s=%20s\n", "Radio Frame Alloc Period", "8");
                break;
            case LIBLTE_RRC_RADIO_FRAME_ALLOCATION_PERIOD_N16:
                printf("\t\t\t%-40s=%20s\n", "Radio Frame Alloc Period", "16");
                break;
            case LIBLTE_RRC_RADIO_FRAME_ALLOCATION_PERIOD_N32:
                printf("\t\t\t%-40s=%20s\n", "Radio Frame Alloc Period", "32");
                break;
            }
            printf("\t\t\t%-40s=%20u\n", "Radio Frame Alloc Offset", sib2->mbsfn_subfr_cnfg[i].subfr_alloc);
            if(LIBLTE_RRC_SUBFRAME_ALLOCATION_NUM_FRAMES_ONE == sib2->mbsfn_subfr_cnfg[i].subfr_alloc_num_frames)
            {
                printf("\t\t\t%-40s=%20u\n", "Subframe Alloc ONE FRAME", sib2->mbsfn_subfr_cnfg[i].subfr_alloc);
            }else{
                printf("\t\t\t%-40s=%20u\n", "Subframe Alloc FOUR FRAMES", sib2->mbsfn_subfr_cnfg[i].subfr_alloc);
            }
        }
        switch(sib2->time_alignment_timer)
        {
        case LIBLTE_RRC_TIME_ALIGNMENT_TIMER_SF500:
            printf("\t\t%-40s=%20s\n", "Time Alignment Timer", "500 Subframes");
            break;
        case LIBLTE_RRC_TIME_ALIGNMENT_TIMER_SF750:
            printf("\t\t%-40s=%20s\n", "Time Alignment Timer", "750 Subframes");
            break;
        case LIBLTE_RRC_TIME_ALIGNMENT_TIMER_SF1280:
            printf("\t\t%-40s=%20s\n", "Time Alignment Timer", "1280 Subframes");
            break;
        case LIBLTE_RRC_TIME_ALIGNMENT_TIMER_SF1920:
            printf("\t\t%-40s=%20s\n", "Time Alignment Timer", "1920 Subframes");
            break;
        case LIBLTE_RRC_TIME_ALIGNMENT_TIMER_SF2560:
            printf("\t\t%-40s=%20s\n", "Time Alignment Timer", "2560 Subframes");
            break;
        case LIBLTE_RRC_TIME_ALIGNMENT_TIMER_SF5120:
            printf("\t\t%-40s=%20s\n", "Time Alignment Timer", "5120 Subframes");
            break;
        case LIBLTE_RRC_TIME_ALIGNMENT_TIMER_SF10240:
            printf("\t\t%-40s=%20s\n", "Time Alignment Timer", "10240 Subframes");
            break;
        case LIBLTE_RRC_TIME_ALIGNMENT_TIMER_INFINITY:
            printf("\t\t%-40s=%20s\n", "Time Alignment Timer", "Infinity");
            break;
        }

        sib2_printed = true;
    }
}

void LTE_fdd_dl_fs_samp_buf::print_sib3(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_3_STRUCT *sib3)
{
    if(false == sib3_printed)
    {
        printf("\tSIB3 Decoded:\n");
        switch(sib3->q_hyst)
        {
        case LIBLTE_RRC_Q_HYST_DB_0:
            printf("\t\t%-40s=%20s\n", "Q-Hyst", "0dB");
            break;
        case LIBLTE_RRC_Q_HYST_DB_1:
            printf("\t\t%-40s=%20s\n", "Q-Hyst", "1dB");
            break;
        case LIBLTE_RRC_Q_HYST_DB_2:
            printf("\t\t%-40s=%20s\n", "Q-Hyst", "2dB");
            break;
        case LIBLTE_RRC_Q_HYST_DB_3:
            printf("\t\t%-40s=%20s\n", "Q-Hyst", "3dB");
            break;
        case LIBLTE_RRC_Q_HYST_DB_4:
            printf("\t\t%-40s=%20s\n", "Q-Hyst", "4dB");
            break;
        case LIBLTE_RRC_Q_HYST_DB_5:
            printf("\t\t%-40s=%20s\n", "Q-Hyst", "5dB");
            break;
        case LIBLTE_RRC_Q_HYST_DB_6:
            printf("\t\t%-40s=%20s\n", "Q-Hyst", "6dB");
            break;
        case LIBLTE_RRC_Q_HYST_DB_8:
            printf("\t\t%-40s=%20s\n", "Q-Hyst", "8dB");
            break;
        case LIBLTE_RRC_Q_HYST_DB_10:
            printf("\t\t%-40s=%20s\n", "Q-Hyst", "10dB");
            break;
        case LIBLTE_RRC_Q_HYST_DB_12:
            printf("\t\t%-40s=%20s\n", "Q-Hyst", "12dB");
            break;
        case LIBLTE_RRC_Q_HYST_DB_14:
            printf("\t\t%-40s=%20s\n", "Q-Hyst", "14dB");
            break;
        case LIBLTE_RRC_Q_HYST_DB_16:
            printf("\t\t%-40s=%20s\n", "Q-Hyst", "16dB");
            break;
        case LIBLTE_RRC_Q_HYST_DB_18:
            printf("\t\t%-40s=%20s\n", "Q-Hyst", "18dB");
            break;
        case LIBLTE_RRC_Q_HYST_DB_20:
            printf("\t\t%-40s=%20s\n", "Q-Hyst", "20dB");
            break;
        case LIBLTE_RRC_Q_HYST_DB_22:
            printf("\t\t%-40s=%20s\n", "Q-Hyst", "22dB");
            break;
        case LIBLTE_RRC_Q_HYST_DB_24:
            printf("\t\t%-40s=%20s\n", "Q-Hyst", "24dB");
            break;
        }
        if(true == sib3->speed_state_resel_params.present)
        {
            switch(sib3->speed_state_resel_params.mobility_state_params.t_eval)
            {
            case LIBLTE_RRC_T_EVALUATION_S30:
                printf("\t\t%-40s=%20s\n", "T-Evaluation", "30s");
                break;
            case LIBLTE_RRC_T_EVALUATION_S60:
                printf("\t\t%-40s=%20s\n", "T-Evaluation", "60s");
                break;
            case LIBLTE_RRC_T_EVALUATION_S120:
                printf("\t\t%-40s=%20s\n", "T-Evaluation", "120s");
                break;
            case LIBLTE_RRC_T_EVALUATION_S180:
                printf("\t\t%-40s=%20s\n", "T-Evaluation", "180s");
                break;
            case LIBLTE_RRC_T_EVALUATION_S240:
                printf("\t\t%-40s=%20s\n", "T-Evaluation", "240s");
                break;
            default:
                printf("\t\t%-40s=%20s\n", "T-Evaluation", "Invalid");
                break;
            }
            switch(sib3->speed_state_resel_params.mobility_state_params.t_hyst_normal)
            {
            case LIBLTE_RRC_T_HYST_NORMAL_S30:
                printf("\t\t%-40s=%20s\n", "T-Hyst Normal", "30s");
                break;
            case LIBLTE_RRC_T_HYST_NORMAL_S60:
                printf("\t\t%-40s=%20s\n", "T-Hyst Normal", "60s");
                break;
            case LIBLTE_RRC_T_HYST_NORMAL_S120:
                printf("\t\t%-40s=%20s\n", "T-Hyst Normal", "120s");
                break;
            case LIBLTE_RRC_T_HYST_NORMAL_S180:
                printf("\t\t%-40s=%20s\n", "T-Hyst Normal", "180s");
                break;
            case LIBLTE_RRC_T_HYST_NORMAL_S240:
                printf("\t\t%-40s=%20s\n", "T-Hyst Normal", "240s");
                break;
            default:
                printf("\t\t%-40s=%20s\n", "T-Hyst Normal", "Invalid");
                break;
            }
            printf("\t\t%-40s=%20u\n", "N-Cell Change Medium", sib3->speed_state_resel_params.mobility_state_params.n_cell_change_medium);
            printf("\t\t%-40s=%20u\n", "N-Cell Change High", sib3->speed_state_resel_params.mobility_state_params.n_cell_change_high);
            switch(sib3->speed_state_resel_params.q_hyst_sf.medium)
            {
            case LIBLTE_RRC_SF_MEDIUM_DB_N6:
                printf("\t\t%-40s=%20s\n", "Q-Hyst SF Medium", "-6dB");
                break;
            case LIBLTE_RRC_SF_MEDIUM_DB_N4:
                printf("\t\t%-40s=%20s\n", "Q-Hyst SF Medium", "-4dB");
                break;
            case LIBLTE_RRC_SF_MEDIUM_DB_N2:
                printf("\t\t%-40s=%20s\n", "Q-Hyst SF Medium", "-2dB");
                break;
            case LIBLTE_RRC_SF_MEDIUM_DB_0:
                printf("\t\t%-40s=%20s\n", "Q-Hyst SF Medium", "0dB");
                break;
            }
            switch(sib3->speed_state_resel_params.q_hyst_sf.high)
            {
            case LIBLTE_RRC_SF_HIGH_DB_N6:
                printf("\t\t%-40s=%20s\n", "Q-Hyst SF High", "-6dB");
                break;
            case LIBLTE_RRC_SF_HIGH_DB_N4:
                printf("\t\t%-40s=%20s\n", "Q-Hyst SF High", "-4dB");
                break;
            case LIBLTE_RRC_SF_HIGH_DB_N2:
                printf("\t\t%-40s=%20s\n", "Q-Hyst SF High", "-2dB");
                break;
            case LIBLTE_RRC_SF_HIGH_DB_0:
                printf("\t\t%-40s=%20s\n", "Q-Hyst SF High", "0dB");
                break;
            }
        }
        if(true == sib3->s_non_intra_search_present)
        {
            printf("\t\t%-40s=%18udB\n", "S-Non Intra Search", sib3->s_non_intra_search);
        }
        printf("\t\t%-40s=%18udB\n", "Threshold Serving Low", sib3->thresh_serving_low);
        printf("\t\t%-40s=%20u\n", "Cell Reselection Priority", sib3->cell_resel_prio);
        printf("\t\t%-40s=%17ddBm\n", "Q Rx Lev Min", sib3->q_rx_lev_min);
        if(true == sib3->p_max_present)
        {
            printf("\t\t%-40s=%17ddBm\n", "P Max", sib3->p_max);
        }
        if(true == sib3->s_intra_search)
        {
            printf("\t\t%-40s=%18udB\n", "S-Intra Search", sib3->s_intra_search);
        }
        if(true == sib3->allowed_meas_bw_present)
        {
            switch(sib3->allowed_meas_bw)
            {
            case LIBLTE_RRC_ALLOWED_MEAS_BANDWIDTH_MBW6:
                printf("\t\t%-40s=%20s\n", "Allowed Meas Bandwidth", "1.6MHz");
                break;
            case LIBLTE_RRC_ALLOWED_MEAS_BANDWIDTH_MBW15:
                printf("\t\t%-40s=%20s\n", "Allowed Meas Bandwidth", "3MHz");
                break;
            case LIBLTE_RRC_ALLOWED_MEAS_BANDWIDTH_MBW25:
                printf("\t\t%-40s=%20s\n", "Allowed Meas Bandwidth", "5MHz");
                break;
            case LIBLTE_RRC_ALLOWED_MEAS_BANDWIDTH_MBW50:
                printf("\t\t%-40s=%20s\n", "Allowed Meas Bandwidth", "10MHz");
                break;
            case LIBLTE_RRC_ALLOWED_MEAS_BANDWIDTH_MBW75:
                printf("\t\t%-40s=%20s\n", "Allowed Meas Bandwidth", "15MHz");
                break;
            case LIBLTE_RRC_ALLOWED_MEAS_BANDWIDTH_MBW100:
                printf("\t\t%-40s=%20s\n", "Allowed Meas Bandwidth", "20MHz");
                break;
            }
        }
        if(true == sib3->presence_ant_port_1)
        {
            printf("\t\t%-40s=%20s\n", "Presence Antenna Port 1", "True");
        }else{
            printf("\t\t%-40s=%20s\n", "Presence Antenna Port 1", "False");
        }
        switch(sib3->neigh_cell_cnfg)
        {
        case 0:
            printf("\t\t%-40s= %s\n", "Neighbor Cell Config", "Not all neighbor cells have the same MBSFN alloc");
            break;
        case 1:
            printf("\t\t%-40s= %s\n", "Neighbor Cell Config", "MBSFN allocs are identical for all neighbor cells");
            break;
        case 2:
            printf("\t\t%-40s= %s\n", "Neighbor Cell Config", "No MBSFN allocs are present in neighbor cells");
            break;
        case 3:
            printf("\t\t%-40s= %s\n", "Neighbor Cell Config", "Different UL/DL allocs in neighbor cells for TDD");
            break;
        }
        printf("\t\t%-40s=%19us\n", "T-Reselection EUTRA", sib3->t_resel_eutra);
        if(true == sib3->t_resel_eutra_sf_present)
        {
            switch(sib3->t_resel_eutra_sf.sf_medium)
            {
            case LIBLTE_RRC_SSSF_MEDIUM_0DOT25:
                printf("\t\t%-40s=%20s\n", "T-Reselection EUTRA SF Medium", "0.25");
                break;
            case LIBLTE_RRC_SSSF_MEDIUM_0DOT5:
                printf("\t\t%-40s=%20s\n", "T-Reselection EUTRA SF Medium", "0.5");
                break;
            case LIBLTE_RRC_SSSF_MEDIUM_0DOT75:
                printf("\t\t%-40s=%20s\n", "T-Reselection EUTRA SF Medium", "0.75");
                break;
            case LIBLTE_RRC_SSSF_MEDIUM_1DOT0:
                printf("\t\t%-40s=%20s\n", "T-Reselection EUTRA SF Medium", "1.0");
                break;
            }
            switch(sib3->t_resel_eutra_sf.sf_high)
            {
            case LIBLTE_RRC_SSSF_HIGH_0DOT25:
                printf("\t\t%-40s=%20s\n", "T-Reselection EUTRA SF High", "0.25");
                break;
            case LIBLTE_RRC_SSSF_HIGH_0DOT5:
                printf("\t\t%-40s=%20s\n", "T-Reselection EUTRA SF High", "0.5");
                break;
            case LIBLTE_RRC_SSSF_HIGH_0DOT75:
                printf("\t\t%-40s=%20s\n", "T-Reselection EUTRA SF High", "0.75");
                break;
            case LIBLTE_RRC_SSSF_HIGH_1DOT0:
                printf("\t\t%-40s=%20s\n", "T-Reselection EUTRA SF High", "1.0");
                break;
            }
        }

        sib3_printed = true;
    }
}

void LTE_fdd_dl_fs_samp_buf::print_sib4(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_4_STRUCT *sib4)
{
    uint32 i;
    uint32 stop;

    if(false == sib4_printed)
    {
        printf("\tSIB4 Decoded:\n");
        if(0 != sib4->intra_freq_neigh_cell_list_size)
        {
            printf("\t\tList of intra-frequency neighboring cells:\n");
        }
        for(i=0; i<sib4->intra_freq_neigh_cell_list_size; i++)
        {
            printf("\t\t\t%s = %u\n", "Physical Cell ID", sib4->intra_freq_neigh_cell_list[i].phys_cell_id);
            switch(sib4->intra_freq_neigh_cell_list[i].q_offset_range)
            {
            case LIBLTE_RRC_Q_OFFSET_RANGE_DB_N24:
                printf("\t\t\t\t%s = %s\n", "Q Offset Range", "-24dB");
                break;
            case LIBLTE_RRC_Q_OFFSET_RANGE_DB_N22:
                printf("\t\t\t\t%s = %s\n", "Q Offset Range", "-22dB");
                break;
            case LIBLTE_RRC_Q_OFFSET_RANGE_DB_N20:
                printf("\t\t\t\t%s = %s\n", "Q Offset Range", "-20dB");
                break;
            case LIBLTE_RRC_Q_OFFSET_RANGE_DB_N18:
                printf("\t\t\t\t%s = %s\n", "Q Offset Range", "-18dB");
                break;
            case LIBLTE_RRC_Q_OFFSET_RANGE_DB_N16:
                printf("\t\t\t\t%s = %s\n", "Q Offset Range", "-16dB");
                break;
            case LIBLTE_RRC_Q_OFFSET_RANGE_DB_N14:
                printf("\t\t\t\t%s = %s\n", "Q Offset Range", "-14dB");
                break;
            case LIBLTE_RRC_Q_OFFSET_RANGE_DB_N12:
                printf("\t\t\t\t%s = %s\n", "Q Offset Range", "-12dB");
                break;
            case LIBLTE_RRC_Q_OFFSET_RANGE_DB_N10:
                printf("\t\t\t\t%s = %s\n", "Q Offset Range", "-10dB");
                break;
            case LIBLTE_RRC_Q_OFFSET_RANGE_DB_N8:
                printf("\t\t\t\t%s = %s\n", "Q Offset Range", "-8dB");
                break;
            case LIBLTE_RRC_Q_OFFSET_RANGE_DB_N6:
                printf("\t\t\t\t%s = %s\n", "Q Offset Range", "-6dB");
                break;
            case LIBLTE_RRC_Q_OFFSET_RANGE_DB_N5:
                printf("\t\t\t\t%s = %s\n", "Q Offset Range", "-5dB");
                break;
            case LIBLTE_RRC_Q_OFFSET_RANGE_DB_N4:
                printf("\t\t\t\t%s = %s\n", "Q Offset Range", "-4dB");
                break;
            case LIBLTE_RRC_Q_OFFSET_RANGE_DB_N3:
                printf("\t\t\t\t%s = %s\n", "Q Offset Range", "-3dB");
                break;
            case LIBLTE_RRC_Q_OFFSET_RANGE_DB_N2:
                printf("\t\t\t\t%s = %s\n", "Q Offset Range", "-2dB");
                break;
            case LIBLTE_RRC_Q_OFFSET_RANGE_DB_N1:
                printf("\t\t\t\t%s = %s\n", "Q Offset Range", "-1dB");
                break;
            case LIBLTE_RRC_Q_OFFSET_RANGE_DB_0:
                printf("\t\t\t\t%s = %s\n", "Q Offset Range", "0dB");
                break;
            case LIBLTE_RRC_Q_OFFSET_RANGE_DB_1:
                printf("\t\t\t\t%s = %s\n", "Q Offset Range", "1dB");
                break;
            case LIBLTE_RRC_Q_OFFSET_RANGE_DB_2:
                printf("\t\t\t\t%s = %s\n", "Q Offset Range", "2dB");
                break;
            case LIBLTE_RRC_Q_OFFSET_RANGE_DB_3:
                printf("\t\t\t\t%s = %s\n", "Q Offset Range", "3dB");
                break;
            case LIBLTE_RRC_Q_OFFSET_RANGE_DB_4:
                printf("\t\t\t\t%s = %s\n", "Q Offset Range", "4dB");
                break;
            case LIBLTE_RRC_Q_OFFSET_RANGE_DB_5:
                printf("\t\t\t\t%s = %s\n", "Q Offset Range", "5dB");
                break;
            case LIBLTE_RRC_Q_OFFSET_RANGE_DB_6:
                printf("\t\t\t\t%s = %s\n", "Q Offset Range", "6dB");
                break;
            case LIBLTE_RRC_Q_OFFSET_RANGE_DB_8:
                printf("\t\t\t\t%s = %s\n", "Q Offset Range", "8dB");
                break;
            case LIBLTE_RRC_Q_OFFSET_RANGE_DB_10:
                printf("\t\t\t\t%s = %s\n", "Q Offset Range", "10dB");
                break;
            case LIBLTE_RRC_Q_OFFSET_RANGE_DB_12:
                printf("\t\t\t\t%s = %s\n", "Q Offset Range", "12dB");
                break;
            case LIBLTE_RRC_Q_OFFSET_RANGE_DB_14:
                printf("\t\t\t\t%s = %s\n", "Q Offset Range", "14dB");
                break;
            case LIBLTE_RRC_Q_OFFSET_RANGE_DB_16:
                printf("\t\t\t\t%s = %s\n", "Q Offset Range", "16dB");
                break;
            case LIBLTE_RRC_Q_OFFSET_RANGE_DB_18:
                printf("\t\t\t\t%s = %s\n", "Q Offset Range", "18dB");
                break;
            case LIBLTE_RRC_Q_OFFSET_RANGE_DB_20:
                printf("\t\t\t\t%s = %s\n", "Q Offset Range", "20dB");
                break;
            case LIBLTE_RRC_Q_OFFSET_RANGE_DB_22:
                printf("\t\t\t\t%s = %s\n", "Q Offset Range", "22dB");
                break;
            case LIBLTE_RRC_Q_OFFSET_RANGE_DB_24:
                printf("\t\t\t\t%s = %s\n", "Q Offset Range", "24dB");
                break;
            }
        }
        if(0 != sib4->intra_freq_black_cell_list_size)
        {
            printf("\t\tList of blacklisted intra-frequency neighboring cells:\n");
        }
        for(i=0; i<sib4->intra_freq_black_cell_list_size; i++)
        {
            stop = sib4->intra_freq_black_cell_list[i].start;
            switch(sib4->intra_freq_black_cell_list[i].range)
            {
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_N4:
                stop += 4;
                break;
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_N8:
                stop += 8;
                break;
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_N12:
                stop += 12;
                break;
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_N16:
                stop += 16;
                break;
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_N24:
                stop += 24;
                break;
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_N32:
                stop += 32;
                break;
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_N48:
                stop += 48;
                break;
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_N64:
                stop += 64;
                break;
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_N84:
                stop += 84;
                break;
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_N96:
                stop += 96;
                break;
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_N128:
                stop += 128;
                break;
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_N168:
                stop += 168;
                break;
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_N252:
                stop += 252;
                break;
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_N504:
                stop += 504;
                break;
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_SPARE2:
                break;
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_SPARE1:
                break;
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_N1:
                break;
            }
            printf("\t\t\t%u - %u\n", sib4->intra_freq_black_cell_list[i].start, stop);
        }
        if(true == sib4->csg_phys_cell_id_range_present)
        {
            stop = sib4->csg_phys_cell_id_range.start;
            switch(sib4->csg_phys_cell_id_range.range)
            {
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_N4:
                stop += 4;
                break;
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_N8:
                stop += 8;
                break;
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_N12:
                stop += 12;
                break;
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_N16:
                stop += 16;
                break;
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_N24:
                stop += 24;
                break;
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_N32:
                stop += 32;
                break;
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_N48:
                stop += 48;
                break;
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_N64:
                stop += 64;
                break;
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_N84:
                stop += 84;
                break;
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_N96:
                stop += 96;
                break;
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_N128:
                stop += 128;
                break;
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_N168:
                stop += 168;
                break;
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_N252:
                stop += 252;
                break;
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_N504:
                stop += 504;
                break;
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_SPARE2:
                break;
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_SPARE1:
                break;
            case LIBLTE_RRC_PHYS_CELL_ID_RANGE_N1:
                break;
            }
            printf("\t\t%-40s= %u - %u\n", "CSG Phys Cell ID Range", sib4->csg_phys_cell_id_range.start, stop);
        }

        sib4_printed = true;
    }
}

void LTE_fdd_dl_fs_samp_buf::print_sib8(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_8_STRUCT *sib8)
{
    uint32 i;
    uint32 j;
    uint32 k;

    if(false == sib8_printed)
    {
        printf("\tSIB8 Decoded:\n");
        if(true == sib8->sys_time_info_present)
        {
            if(true == sib8->sys_time_info_cdma2000.cdma_eutra_sync)
            {
                printf("\t\t%-40s=%20s\n", "CDMA EUTRA sync", "True");
            }else{
                printf("\t\t%-40s=%20s\n", "CDMA EUTRA sync", "False");
            }
            if(true == sib8->sys_time_info_cdma2000.system_time_async)
            {
                printf("\t\t%-40s=%14llu chips\n", "System Time", sib8->sys_time_info_cdma2000.system_time * 8);
            }else{
                printf("\t\t%-40s=%17llu ms\n", "System Time", sib8->sys_time_info_cdma2000.system_time * 10);
            }
        }
        if(true == sib8->search_win_size_present)
        {
            printf("\t\t%-40s=%20u\n", "Search Window Size", sib8->search_win_size);
        }
        if(true == sib8->params_hrpd_present)
        {
            if(true == sib8->pre_reg_info_hrpd.pre_reg_allowed)
            {
                printf("\t\t%-40s=%20s\n", "Pre Registration", "Allowed");
            }else{
                printf("\t\t%-40s=%20s\n", "Pre Registration", "Not Allowed");
            }
            if(true == sib8->pre_reg_info_hrpd.pre_reg_zone_id_present)
            {
                printf("\t\t%-40s=%20u\n", "Pre Registration Zone ID", sib8->pre_reg_info_hrpd.pre_reg_zone_id);
            }
            if(0 != sib8->pre_reg_info_hrpd.secondary_pre_reg_zone_id_list_size)
            {
                printf("\t\tSecondary Pre Registration Zone IDs:\n");
            }
            for(i=0; i<sib8->pre_reg_info_hrpd.secondary_pre_reg_zone_id_list_size; i++)
            {
                printf("\t\t\t%u\n", sib8->pre_reg_info_hrpd.secondary_pre_reg_zone_id_list[i]);
            }
            if(true == sib8->cell_resel_params_hrpd_present)
            {
                printf("\t\tBand Class List:\n");
                for(i=0; i<sib8->cell_resel_params_hrpd.band_class_list_size; i++)
                {
                    switch(sib8->cell_resel_params_hrpd.band_class_list[i].band_class)
                    {
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC0:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "0");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC1:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "1");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC2:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "2");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC3:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "3");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC4:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "4");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC5:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "5");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC6:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "6");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC7:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "7");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC8:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "8");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC9:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "9");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC10:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "10");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC11:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "11");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC12:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "12");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC13:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "13");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC14:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "14");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC15:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "15");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC16:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "16");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC17:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "17");
                        break;
                    default:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "Invalid");
                        break;
                    }
                    if(true == sib8->cell_resel_params_hrpd.band_class_list[i].cell_resel_prio_present)
                    {
                        printf("\t\t\t%-40s=%20u\n", "Cell Reselection Priority", sib8->cell_resel_params_hrpd.band_class_list[i].cell_resel_prio);
                    }
                    printf("\t\t\t%-40s=%20u\n", "Threshold X High", sib8->cell_resel_params_hrpd.band_class_list[i].thresh_x_high);
                    printf("\t\t\t%-40s=%20u\n", "Threshold X Low", sib8->cell_resel_params_hrpd.band_class_list[i].thresh_x_low);
                }
                printf("\t\tNeighbor Cell List:\n");
                for(i=0; i<sib8->cell_resel_params_hrpd.neigh_cell_list_size; i++)
                {
                    switch(sib8->cell_resel_params_hrpd.neigh_cell_list[i].band_class)
                    {
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC0:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "0");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC1:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "1");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC2:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "2");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC3:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "3");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC4:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "4");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC5:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "5");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC6:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "6");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC7:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "7");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC8:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "8");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC9:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "9");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC10:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "10");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC11:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "11");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC12:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "12");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC13:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "13");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC14:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "14");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC15:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "15");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC16:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "16");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC17:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "17");
                        break;
                    default:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "Invalid");
                        break;
                    }
                    printf("\t\t\tNeighbor Cells Per Frequency List\n");
                    for(j=0; j<sib8->cell_resel_params_hrpd.neigh_cell_list[i].neigh_cells_per_freq_list_size; j++)
                    {
                        printf("\t\t\t\t%-40s=%20u\n", "ARFCN", sib8->cell_resel_params_hrpd.neigh_cell_list[i].neigh_cells_per_freq_list[j].arfcn);
                        printf("\t\t\t\tPhys Cell ID List\n");
                        for(k=0; k<sib8->cell_resel_params_hrpd.neigh_cell_list[i].neigh_cells_per_freq_list[j].phys_cell_id_list_size; k++)
                        {
                            printf("\t\t\t\t\t%u\n", sib8->cell_resel_params_hrpd.neigh_cell_list[i].neigh_cells_per_freq_list[j].phys_cell_id_list[k]);
                        }
                    }
                }
                printf("\t\t%-40s=%19us\n", "T Reselection", sib8->cell_resel_params_hrpd.t_resel_cdma2000);
                if(true == sib8->cell_resel_params_hrpd.t_resel_cdma2000_sf_present)
                {
                    switch(sib8->cell_resel_params_hrpd.t_resel_cdma2000_sf.sf_medium)
                    {
                    case LIBLTE_RRC_SSSF_MEDIUM_0DOT25:
                        printf("\t\t%-40s=%20s\n", "T Reselection Scale Factor Medium", "0.25");
                        break;
                    case LIBLTE_RRC_SSSF_MEDIUM_0DOT5:
                        printf("\t\t%-40s=%20s\n", "T Reselection Scale Factor Medium", "0.5");
                        break;
                    case LIBLTE_RRC_SSSF_MEDIUM_0DOT75:
                        printf("\t\t%-40s=%20s\n", "T Reselection Scale Factor Medium", "0.75");
                        break;
                    case LIBLTE_RRC_SSSF_MEDIUM_1DOT0:
                        printf("\t\t%-40s=%20s\n", "T Reselection Scale Factor Medium", "1.0");
                        break;
                    }
                    switch(sib8->cell_resel_params_hrpd.t_resel_cdma2000_sf.sf_high)
                    {
                    case LIBLTE_RRC_SSSF_HIGH_0DOT25:
                        printf("\t\t%-40s=%20s\n", "T Reselection Scale Factor High", "0.25");
                        break;
                    case LIBLTE_RRC_SSSF_HIGH_0DOT5:
                        printf("\t\t%-40s=%20s\n", "T Reselection Scale Factor High", "0.5");
                        break;
                    case LIBLTE_RRC_SSSF_HIGH_0DOT75:
                        printf("\t\t%-40s=%20s\n", "T Reselection Scale Factor High", "0.75");
                        break;
                    case LIBLTE_RRC_SSSF_HIGH_1DOT0:
                        printf("\t\t%-40s=%20s\n", "T Reselection Scale Factor High", "1.0");
                        break;
                    }
                }
            }
        }
        if(true == sib8->params_1xrtt_present)
        {
            printf("\t\tCSFB Registration Parameters\n");
            if(true == sib8->csfb_reg_param_1xrtt_present)
            {
                printf("\t\t\t%-40s=%20u\n", "SID", sib8->csfb_reg_param_1xrtt.sid);
                printf("\t\t\t%-40s=%20u\n", "NID", sib8->csfb_reg_param_1xrtt.nid);
                if(true == sib8->csfb_reg_param_1xrtt.multiple_sid)
                {
                    printf("\t\t\t%-40s=%20s\n", "Multiple SIDs", "True");
                }else{
                    printf("\t\t\t%-40s=%20s\n", "Multiple SIDs", "False");
                }
                if(true == sib8->csfb_reg_param_1xrtt.multiple_nid)
                {
                    printf("\t\t\t%-40s=%20s\n", "Multiple NIDs", "True");
                }else{
                    printf("\t\t\t%-40s=%20s\n", "Multiple NIDs", "False");
                }
                if(true == sib8->csfb_reg_param_1xrtt.home_reg)
                {
                    printf("\t\t\t%-40s=%20s\n", "Home Reg", "True");
                }else{
                    printf("\t\t\t%-40s=%20s\n", "Home Reg", "False");
                }
                if(true == sib8->csfb_reg_param_1xrtt.foreign_sid_reg)
                {
                    printf("\t\t\t%-40s=%20s\n", "Foreign SID Reg", "True");
                }else{
                    printf("\t\t\t%-40s=%20s\n", "Foreign SID Reg", "False");
                }
                if(true == sib8->csfb_reg_param_1xrtt.foreign_nid_reg)
                {
                    printf("\t\t\t%-40s=%20s\n", "Foreign NID Reg", "True");
                }else{
                    printf("\t\t\t%-40s=%20s\n", "Foreign NID Reg", "False");
                }
                if(true == sib8->csfb_reg_param_1xrtt.param_reg)
                {
                    printf("\t\t\t%-40s=%20s\n", "Parameter Reg", "True");
                }else{
                    printf("\t\t\t%-40s=%20s\n", "Parameter Reg", "False");
                }
                if(true == sib8->csfb_reg_param_1xrtt.power_up_reg)
                {
                    printf("\t\t\t%-40s=%20s\n", "Power Up Reg", "True");
                }else{
                    printf("\t\t\t%-40s=%20s\n", "Power Up Reg", "False");
                }
                printf("\t\t\t%-40s=%20u\n", "Registration Period", sib8->csfb_reg_param_1xrtt.reg_period);
                printf("\t\t\t%-40s=%20u\n", "Registration Zone", sib8->csfb_reg_param_1xrtt.reg_zone);
                printf("\t\t\t%-40s=%20u\n", "Total Zones", sib8->csfb_reg_param_1xrtt.total_zone);
                printf("\t\t\t%-40s=%20u\n", "Zone Timer", sib8->csfb_reg_param_1xrtt.zone_timer);
            }
            if(true == sib8->long_code_state_1xrtt_present)
            {
                printf("\t\t%-40s=%20llu\n", "Long Code State", sib8->long_code_state_1xrtt);
            }
            if(true == sib8->cell_resel_params_1xrtt_present)
            {
                printf("\t\tBand Class List:\n");
                for(i=0; i<sib8->cell_resel_params_1xrtt.band_class_list_size; i++)
                {
                    switch(sib8->cell_resel_params_1xrtt.band_class_list[i].band_class)
                    {
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC0:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "0");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC1:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "1");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC2:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "2");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC3:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "3");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC4:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "4");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC5:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "5");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC6:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "6");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC7:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "7");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC8:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "8");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC9:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "9");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC10:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "10");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC11:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "11");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC12:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "12");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC13:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "13");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC14:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "14");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC15:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "15");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC16:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "16");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC17:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "17");
                        break;
                    default:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "Invalid");
                        break;
                    }
                    if(true == sib8->cell_resel_params_1xrtt.band_class_list[i].cell_resel_prio_present)
                    {
                        printf("\t\t\t%-40s=%20u\n", "Cell Reselection Priority", sib8->cell_resel_params_1xrtt.band_class_list[i].cell_resel_prio);
                    }
                    printf("\t\t\t%-40s=%20u\n", "Threshold X High", sib8->cell_resel_params_1xrtt.band_class_list[i].thresh_x_high);
                    printf("\t\t\t%-40s=%20u\n", "Threshold X Low", sib8->cell_resel_params_1xrtt.band_class_list[i].thresh_x_low);
                }
                printf("\t\tNeighbor Cell List:\n");
                for(i=0; i<sib8->cell_resel_params_1xrtt.neigh_cell_list_size; i++)
                {
                    switch(sib8->cell_resel_params_1xrtt.neigh_cell_list[i].band_class)
                    {
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC0:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "0");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC1:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "1");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC2:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "2");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC3:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "3");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC4:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "4");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC5:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "5");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC6:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "6");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC7:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "7");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC8:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "8");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC9:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "9");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC10:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "10");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC11:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "11");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC12:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "12");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC13:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "13");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC14:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "14");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC15:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "15");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC16:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "16");
                        break;
                    case LIBLTE_RRC_BAND_CLASS_CDMA2000_BC17:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "17");
                        break;
                    default:
                        printf("\t\t\t%-40s=%20s\n", "Band Class", "Invalid");
                        break;
                    }
                    printf("\t\t\tNeighbor Cells Per Frequency List\n");
                    for(j=0; j<sib8->cell_resel_params_1xrtt.neigh_cell_list[i].neigh_cells_per_freq_list_size; j++)
                    {
                        printf("\t\t\t\t%-40s=%20u\n", "ARFCN", sib8->cell_resel_params_1xrtt.neigh_cell_list[i].neigh_cells_per_freq_list[j].arfcn);
                        printf("\t\t\t\tPhys Cell ID List\n");
                        for(k=0; k<sib8->cell_resel_params_1xrtt.neigh_cell_list[i].neigh_cells_per_freq_list[j].phys_cell_id_list_size; k++)
                        {
                            printf("\t\t\t\t\t%u\n", sib8->cell_resel_params_1xrtt.neigh_cell_list[i].neigh_cells_per_freq_list[j].phys_cell_id_list[k]);
                        }
                    }
                }
                printf("\t\t%-40s=%19us\n", "T Reselection", sib8->cell_resel_params_1xrtt.t_resel_cdma2000);
                if(true == sib8->cell_resel_params_1xrtt.t_resel_cdma2000_sf_present)
                {
                    switch(sib8->cell_resel_params_1xrtt.t_resel_cdma2000_sf.sf_medium)
                    {
                    case LIBLTE_RRC_SSSF_MEDIUM_0DOT25:
                        printf("\t\t%-40s=%20s\n", "T Reselection Scale Factor Medium", "0.25");
                        break;
                    case LIBLTE_RRC_SSSF_MEDIUM_0DOT5:
                        printf("\t\t%-40s=%20s\n", "T Reselection Scale Factor Medium", "0.5");
                        break;
                    case LIBLTE_RRC_SSSF_MEDIUM_0DOT75:
                        printf("\t\t%-40s=%20s\n", "T Reselection Scale Factor Medium", "0.75");
                        break;
                    case LIBLTE_RRC_SSSF_MEDIUM_1DOT0:
                        printf("\t\t%-40s=%20s\n", "T Reselection Scale Factor Medium", "1.0");
                        break;
                    }
                    switch(sib8->cell_resel_params_1xrtt.t_resel_cdma2000_sf.sf_high)
                    {
                    case LIBLTE_RRC_SSSF_HIGH_0DOT25:
                        printf("\t\t%-40s=%20s\n", "T Reselection Scale Factor High", "0.25");
                        break;
                    case LIBLTE_RRC_SSSF_HIGH_0DOT5:
                        printf("\t\t%-40s=%20s\n", "T Reselection Scale Factor High", "0.5");
                        break;
                    case LIBLTE_RRC_SSSF_HIGH_0DOT75:
                        printf("\t\t%-40s=%20s\n", "T Reselection Scale Factor High", "0.75");
                        break;
                    case LIBLTE_RRC_SSSF_HIGH_1DOT0:
                        printf("\t\t%-40s=%20s\n", "T Reselection Scale Factor High", "1.0");
                        break;
                    }
                }
            }
        }

        sib8_printed = true;
    }
}
