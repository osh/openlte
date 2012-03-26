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

    File: liblte_phy.h

    Description: Contains all the definitions for the LTE Physical Layer
                 library.

    Revision History
    ----------    -------------    --------------------------------------------
    02/26/2012    Ben Wojtowicz    Created file.

*******************************************************************************/

#ifndef __LIBLTE_PHY_H__
#define __LIBLTE_PHY_H__

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "liblte_common.h"
#include "fftw3.h"

/*******************************************************************************
                              DEFINES
*******************************************************************************/

#define LIBLTE_FFT_PAD_SIZE_1_4MHZ 988
#define LIBLTE_FFT_PAD_SIZE_3MHZ   934
#define LIBLTE_FFT_PAD_SIZE_5MHZ   874
#define LIBLTE_FFT_PAD_SIZE_10MHZ  724
#define LIBLTE_FFT_PAD_SIZE_15MHZ  574
#define LIBLTE_FFT_PAD_SIZE_20MHZ  424
#define LIBLTE_N_RB_DL_1_4MHZ      6
#define LIBLTE_N_RB_DL_3MHZ        15
#define LIBLTE_N_RB_DL_5MHZ        25
#define LIBLTE_N_RB_DL_10MHZ       50
#define LIBLTE_N_RB_DL_15MHZ       75
#define LIBLTE_N_RB_DL_20MHZ       100
#define LIBLTE_N_SC_RB_NORMAL_CP   12

/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/

typedef struct{
    float symb_re[16][LIBLTE_N_RB_DL_20MHZ*LIBLTE_N_SC_RB_NORMAL_CP];
    float symb_im[16][LIBLTE_N_RB_DL_20MHZ*LIBLTE_N_SC_RB_NORMAL_CP];
    float ce_re[4][16][LIBLTE_N_RB_DL_20MHZ*LIBLTE_N_SC_RB_NORMAL_CP];
    float ce_im[4][16][LIBLTE_N_RB_DL_20MHZ*LIBLTE_N_SC_RB_NORMAL_CP];
}LIBLTE_PHY_SLOT_STRUCT;

/*******************************************************************************
                              DECLARATIONS
*******************************************************************************/

/*********************************************************************
    Name: liblte_phy_init

    Description: Initializes the LTE Physical Layer library.

    Document Reference: N/A
*********************************************************************/
// Defines
typedef struct{
    // Timing
    float timing_corr_freq_err[15360];
    float timing_abs_corr[15360];

    // PSS
    float pss_mod_re[3][LIBLTE_N_RB_DL_20MHZ*LIBLTE_N_SC_RB_NORMAL_CP];
    float pss_mod_im[3][LIBLTE_N_RB_DL_20MHZ*LIBLTE_N_SC_RB_NORMAL_CP];

    // SSS
    float sss_mod_re_0[168][LIBLTE_N_RB_DL_20MHZ*LIBLTE_N_SC_RB_NORMAL_CP];
    float sss_mod_im_0[168][LIBLTE_N_RB_DL_20MHZ*LIBLTE_N_SC_RB_NORMAL_CP];
    float sss_mod_re_5[168][LIBLTE_N_RB_DL_20MHZ*LIBLTE_N_SC_RB_NORMAL_CP];
    float sss_mod_im_5[168][LIBLTE_N_RB_DL_20MHZ*LIBLTE_N_SC_RB_NORMAL_CP];
    float sss_re_0[63];
    float sss_im_0[63];
    float sss_re_5[63];
    float sss_im_5[63];
    uint8 sss_x_s_tilda[31];
    uint8 sss_x_c_tilda[31];
    uint8 sss_x_z_tilda[31];
    int8  sss_s_tilda[31];
    int8  sss_c_tilda[31];
    int8  sss_z_tilda[31];
    int8  sss_s0_m0[31];
    int8  sss_s1_m1[31];
    int8  sss_c0[31];
    int8  sss_c1[31];
    int8  sss_z1_m0[31];
    int8  sss_z1_m1[31];

    // Channel Estimate
    float ce_crs_re[16][LIBLTE_N_RB_DL_20MHZ*LIBLTE_N_SC_RB_NORMAL_CP];
    float ce_crs_im[16][LIBLTE_N_RB_DL_20MHZ*LIBLTE_N_SC_RB_NORMAL_CP];
    float ce_mag[5][LIBLTE_N_RB_DL_20MHZ*LIBLTE_N_SC_RB_NORMAL_CP];
    float ce_ang[5][LIBLTE_N_RB_DL_20MHZ*LIBLTE_N_SC_RB_NORMAL_CP];

    // BCH Decode
    float  bch_y_est_re[240];
    float  bch_y_est_im[240];
    float  bch_c_est_re[4][240];
    float  bch_c_est_im[4][240];
    float  bch_x_re[480];
    float  bch_x_im[480];
    float  bch_d_re[480];
    float  bch_d_im[480];
    float  bch_descramb_bits[1920];
    float  bch_d_bits[1920];
    uint32 bch_c[1920];
    int8   bch_soft_bits[480];

    // Rate Match
    float rm_tmp[1024];
    float rm_sb_mat[32][32];
    float rm_sb_perm_mat[32][32];
    float rm_w_dum[3*1024];
    float rm_w[3*1024];
    float rm_v[3][1024];

    // Viterbi decode
    float vd_path_metric[128][2048];
    float vd_br_metric[128][2];
    float vd_p_metric[128][2];
    float vd_w_metric[128][2];
    float vd_tb_state[2048];
    uint8 vd_st_output[128][2][3];

//    // Samples to Symbols
//    fftw_complex *in;
//    fftw_complex *out;
//    fftw_plan     plan;

    // Generic
    float symb_re[LIBLTE_N_RB_DL_20MHZ*LIBLTE_N_SC_RB_NORMAL_CP];
    float symb_im[LIBLTE_N_RB_DL_20MHZ*LIBLTE_N_SC_RB_NORMAL_CP];
}LIBLTE_PHY_STRUCT;
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_phy_init(LIBLTE_PHY_STRUCT **phy_struct);

/*********************************************************************
    Name: liblte_phy_cleanup

    Description: Cleans up the LTE Physical Layer library.

    Document Reference: N/A
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_phy_cleanup(LIBLTE_PHY_STRUCT *phy_struct);

/*********************************************************************
    Name: liblte_phy_find_coarse_timing_and_freq_offset

    Description: Finds coarse time syncronization and frequency offset
                 by auto-correlating to find the cyclic prefix on
                 reference signal symbols.

    Document Reference: 3GPP TS 36.211 v10.1.0
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_phy_find_coarse_timing_and_freq_offset(LIBLTE_PHY_STRUCT *phy_struct,
                                                                float             *i_samps,
                                                                float             *q_samps,
                                                                uint32            *symb_starts,
                                                                float             *freq_offset);

/*********************************************************************
    Name: liblte_phy_find_pss_and_fine_timing

    Description: Searches for the Primary Synchronization Signal and
                 determines fine timing.

    Document Reference: 3GPP TS 36.211 v10.1.0
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_phy_find_pss_and_fine_timing(LIBLTE_PHY_STRUCT *phy_struct,
                                                      float             *i_samps,
                                                      float             *q_samps,
                                                      uint32            *symb_starts,
                                                      uint32            *N_id_2,
                                                      uint32            *pss_symb,
                                                      float             *pss_thresh);

/*********************************************************************
    Name: liblte_phy_find_sss

    Description: Searches for the Secondary Synchronization Signal.

    Document Reference: 3GPP TS 36.211 v10.1.0
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_phy_find_sss(LIBLTE_PHY_STRUCT *phy_struct,
                                      float             *i_samps,
                                      float             *q_samps,
                                      uint32             N_id_2,
                                      uint32            *symb_starts,
                                      float              pss_thresh,
                                      uint32            *N_id_1,
                                      uint32            *frame_start_idx);

/*********************************************************************
    Name: liblte_phy_get_subframe_and_ce

    Description: Resolves all symbols and channel estimates for a
                 particular subframe

    Document Reference: 3GPP TS 36.211 v10.1.0
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_phy_get_subframe_and_ce(LIBLTE_PHY_STRUCT      *phy_struct,
                                                 float                  *i_samps,
                                                 float                  *q_samps,
                                                 uint32                  frame_start_idx,
                                                 uint8                   subfr_num,
                                                 uint32                  FFT_pad_size,
                                                 uint32                  N_rb_dl,
                                                 uint32                  N_id_cell,
                                                 uint8                   N_ant,
                                                 LIBLTE_PHY_SLOT_STRUCT *slot);

/*********************************************************************
    Name: liblte_phy_bch_channel_decode

    Description: Demodulates and decodes the Physical Broadcast
                 Channel

    Document Reference: 3GPP TS 36.211 v10.1.0
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_phy_bch_channel_decode(LIBLTE_PHY_STRUCT      *phy_struct,
                                                LIBLTE_PHY_SLOT_STRUCT *slot,
                                                uint32                  N_id_cell,
                                                uint8                  *N_ant,
                                                uint8                  *bits,
                                                uint32                 *num_bits,
                                                uint8                  *offset);

#endif /* __LIBLTE_PHY_H__ */
