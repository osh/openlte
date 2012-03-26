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

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_dl_fs_samp_buf.h"
#include "gr_io_signature.h"

/*******************************************************************************
                              DEFINES
*******************************************************************************/


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
    // Initialize the LTE library
    liblte_phy_init(&phy_struct);

    // Allocate the sample buffer
    i_buf           = (float *)malloc(LTE_FDD_DL_FS_SAMP_BUF_SIZE*sizeof(float));
    q_buf           = (float *)malloc(LTE_FDD_DL_FS_SAMP_BUF_SIZE*sizeof(float));
    samp_buf_idx    = 0;
    last_samp_was_i = false;
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
    LIBLTE_PHY_SLOT_STRUCT  slot;
    LIBLTE_RRC_MIB_STRUCT   mib;
    const int8             *in  = (const int8 *)input_items[0];
    float                   freq_offset;
    float                   f_samp_re;
    float                   f_samp_im;
    float                   pss_thresh;
    uint32                  i;
    uint32                  sfn;
    uint32                  symb_starts[7];
    uint32                  N_id_cell;
    uint32                  N_id_1;
    uint32                  N_id_2;
    uint32                  pss_symb;
    uint32                  frame_start_idx;
    uint32                  offset;
    uint8                   sfn_offset;
    uint8                   N_ant;

    if(samp_buf_idx < (LTE_FDD_DL_FS_SAMP_BUF_SIZE-((ninput_items+1)/2)))
    {
        if(true == last_samp_was_i)
        {
            q_buf[samp_buf_idx++] = (float)in[0];
            offset                = 1;
        }else{
            offset = 0;
        }

        for(i=0; i<(ninput_items-offset)/2; i++)
        {
            i_buf[samp_buf_idx]   = (float)in[i*2+offset];
            q_buf[samp_buf_idx++] = (float)in[i*2+offset+1];
        }

        if(((ninput_items-offset) % 2) != 0)
        {
            i_buf[samp_buf_idx] = (float)in[ninput_items-1];
            last_samp_was_i     = true;
        }else{
            last_samp_was_i = false;
        }
    }else{
        if(LIBLTE_SUCCESS == liblte_phy_find_coarse_timing_and_freq_offset(phy_struct,
                                                                           i_buf,
                                                                           q_buf,
                                                                           symb_starts,
                                                                           &freq_offset))
        {
            // Correct frequency error
            for(i=0; i<LTE_FDD_DL_FS_SAMP_BUF_SIZE; i++)
            {
                f_samp_re = cosf(i*(-freq_offset)*2*M_PI*(0.0005/15360));
                f_samp_im = sinf(i*(-freq_offset)*2*M_PI*(0.0005/15360));
                i_buf[i] = i_buf[i]*f_samp_re + q_buf[i]*f_samp_im;
                q_buf[i] = q_buf[i]*f_samp_re - i_buf[i]*f_samp_im;
            }
            if(LIBLTE_SUCCESS == liblte_phy_find_pss_and_fine_timing(phy_struct,
                                                                     i_buf,
                                                                     q_buf,
                                                                     symb_starts,
                                                                     &N_id_2,
                                                                     &pss_symb,
                                                                     &pss_thresh) &&
               LIBLTE_SUCCESS == liblte_phy_find_sss(phy_struct,
                                                     i_buf,
                                                     q_buf,
                                                     N_id_2,
                                                     symb_starts,
                                                     pss_thresh,
                                                     &N_id_1,
                                                     &frame_start_idx))
            {
                N_id_cell = 3*N_id_1 + N_id_2;
                if(LIBLTE_SUCCESS == liblte_phy_get_subframe_and_ce(phy_struct,
                                                                    i_buf,
                                                                    q_buf,
                                                                    frame_start_idx,
                                                                    0,
                                                                    LIBLTE_FFT_PAD_SIZE_1_4MHZ,
                                                                    LIBLTE_N_RB_DL_1_4MHZ,
                                                                    N_id_cell,
                                                                    4,
                                                                    &slot) &&
                   LIBLTE_SUCCESS == liblte_phy_bch_channel_decode(phy_struct,
                                                                   &slot,
                                                                   N_id_cell,
                                                                   &N_ant,
                                                                   rrc_msg.msg,
                                                                   &rrc_msg.num_bits,
                                                                   &sfn_offset) &&
                   LIBLTE_SUCCESS == liblte_rrc_unpack_bch_bcch_msg(&rrc_msg,
                                                                    &mib))
                {
                    sfn = (mib.sfn_div_4 << 2) + sfn_offset;
                    printf("DL FDD LTE Channel found with Physical Cell ID %u, %u TX antenna(s), ",
                           N_id_cell,
                           N_ant);
                    switch(mib.dl_bw)
                    {
                    case LIBLTE_RRC_DL_BANDWIDTH_6:
                        printf("1.4MHz bandwidth, ");
                        break;
                    case LIBLTE_RRC_DL_BANDWIDTH_15:
                        printf("3MHz bandwidth, ");
                        break;
                    case LIBLTE_RRC_DL_BANDWIDTH_25:
                        printf("5MHz bandwidth, ");
                        break;
                    case LIBLTE_RRC_DL_BANDWIDTH_50:
                        printf("10MHz bandwidth, ");
                        break;
                    case LIBLTE_RRC_DL_BANDWIDTH_75:
                        printf("15MHz bandwidth, ");
                        break;
                    case LIBLTE_RRC_DL_BANDWIDTH_100:
                        printf("20MHz bandwidth, ");
                        break;
                    }
                    if(mib.phich_config.dur == LIBLTE_RRC_PHICH_DURATION_NORMAL)
                    {
                        printf("normal PHICH duration, ");
                    }else{
                        printf("extended PHICH duration, ");
                    }
                    switch(mib.phich_config.res)
                    {
                    case LIBLTE_RRC_PHICH_RESOURCE_1_6:
                        printf("and 1/6 PHICH resource ");
                        break;
                    case LIBLTE_RRC_PHICH_RESOURCE_1_2:
                        printf("and 1/2 PHICH resource ");
                        break;
                    case LIBLTE_RRC_PHICH_RESOURCE_1:
                        printf("and 1 PHICH resource ");
                        break;
                    case LIBLTE_RRC_PHICH_RESOURCE_2:
                        printf("and 2 PHICH resources ");
                        break;
                    }
                    printf("in SFN %u\n", sfn);
                }else{
                    printf("Could not decode channel\n");
                }
            }else{
                printf("Could not decode channel\n");
            }
        }else{
            printf("Could not decode channel\n");
        }

        for(i=0; i<LTE_FDD_DL_FS_SAMP_BUF_SIZE; i++)
        {
            i_buf[i] = 0;
            q_buf[i] = 0;
        }
        samp_buf_idx    = 0;
        last_samp_was_i = false;
    }

    // Tell runtime system how many input items we consumed
    consume_each(ninput_items);

    // Tell runtime system how many output items we produced.
    return(0);
}
