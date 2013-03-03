/*******************************************************************************

    Copyright 2012-2013 Ben Wojtowicz

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
    04/21/2012    Ben Wojtowicz    Added Turbo encode/decode and PDSCH decode
    05/28/2012    Ben Wojtowicz    Rearranged to match spec order, added
                                   initial transmit functionality, and fixed
                                   a bug related to PDCCH sizes.
    06/11/2012    Ben Wojtowicz    Enabled fftw input, output, and plan in
                                   phy_struct.
    10/06/2012    Ben Wojtowicz    Added random access and paging PDCCH
                                   decoding, soft turbo decoding, and PDSCH
                                   decoding per allocation.
    11/10/2012    Ben Wojtowicz    Added TBS, MCS, and N_prb calculations for
                                   SI PDSCH messages, added more sample defines,
                                   and re-factored the coarse timing and freq
                                   search to find more than 1 eNB.
    12/01/2012    Ben Wojtowicz    Added ability to preconfigure CRS.
    12/26/2012    Ben Wojtowicz    Started supporting N_sc_rb for normal and
                                   extended CP and fixed several FIXMEs.
    03/03/2013    Ben Wojtowicz    Added support for multiple sampling rates,
                                   pre-permutation of PDCCH encoding, and
                                   integer frequency offset detection.

*******************************************************************************/

#ifndef __LIBLTE_PHY_H__
#define __LIBLTE_PHY_H__

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "liblte_common.h"
#include "liblte_rrc.h"
#include "fftw3.h"

/*******************************************************************************
                              DEFINES
*******************************************************************************/

// FFT sizes
#define LIBLTE_PHY_FFT_SIZE_1_92MHZ  128
#define LIBLTE_PHY_FFT_SIZE_3_84MHZ  256
#define LIBLTE_PHY_FFT_SIZE_7_68MHZ  512
#define LIBLTE_PHY_FFT_SIZE_15_36MHZ 1024
#define LIBLTE_PHY_FFT_SIZE_30_72MHZ 2048

// N_rb_dl
#define LIBLTE_PHY_N_RB_DL_1_4MHZ 6
#define LIBLTE_PHY_N_RB_DL_3MHZ   15
#define LIBLTE_PHY_N_RB_DL_5MHZ   25
#define LIBLTE_PHY_N_RB_DL_10MHZ  50
#define LIBLTE_PHY_N_RB_DL_15MHZ  75
#define LIBLTE_PHY_N_RB_DL_20MHZ  100
#define LIBLTE_PHY_N_RB_DL_MAX    110

// N_sc_rb
#define LIBLTE_PHY_N_SC_RB_NORMAL_CP 12
// FIXME: Add Extended CP

// RNTI
#define LIBLTE_PHY_RA_RNTI_START 0x0001
#define LIBLTE_PHY_RA_RNTI_END   0x003C
#define LIBLTE_PHY_P_RNTI        0xFFFE
#define LIBLTE_PHY_SI_RNTI       0xFFFF

// N_ant
#define LIBLTE_PHY_N_ANT_MAX 4

// Symbol, CP, Slot, Subframe, and Frame timing
// Generic
#define LIBLTE_PHY_N_SLOTS_PER_SUBFR 2
#define LIBLTE_PHY_N_SUBFR_PER_FRAME 10
// 20MHz and 15MHz bandwidths
#define LIBLTE_PHY_N_SAMPS_PER_SYMB_30_72MHZ  2048
#define LIBLTE_PHY_N_SAMPS_CP_L_0_30_72MHZ    160
#define LIBLTE_PHY_N_SAMPS_CP_L_ELSE_30_72MHZ 144
#define LIBLTE_PHY_N_SAMPS_PER_SLOT_30_72MHZ  15360
#define LIBLTE_PHY_N_SAMPS_PER_SUBFR_30_72MHZ (LIBLTE_PHY_N_SAMPS_PER_SLOT_30_72MHZ*LIBLTE_PHY_N_SLOTS_PER_SUBFR)
#define LIBLTE_PHY_N_SAMPS_PER_FRAME_30_72MHZ (LIBLTE_PHY_N_SAMPS_PER_SUBFR_30_72MHZ*LIBLTE_PHY_N_SUBFR_PER_FRAME)
// 10MHz bandwidth
#define LIBLTE_PHY_N_SAMPS_PER_SYMB_15_36MHZ  1024
#define LIBLTE_PHY_N_SAMPS_CP_L_0_15_36MHZ    80
#define LIBLTE_PHY_N_SAMPS_CP_L_ELSE_15_36MHZ 72
#define LIBLTE_PHY_N_SAMPS_PER_SLOT_15_36MHZ  7680
#define LIBLTE_PHY_N_SAMPS_PER_SUBFR_15_36MHZ (LIBLTE_PHY_N_SAMPS_PER_SLOT_15_36MHZ*LIBLTE_PHY_N_SLOTS_PER_SUBFR)
#define LIBLTE_PHY_N_SAMPS_PER_FRAME_15_36MHZ (LIBLTE_PHY_N_SAMPS_PER_SUBFR_15_36MHZ*LIBLTE_PHY_N_SUBFR_PER_FRAME)
// 5MHz bandwidth
#define LIBLTE_PHY_N_SAMPS_PER_SYMB_7_68MHZ  512
#define LIBLTE_PHY_N_SAMPS_CP_L_0_7_68MHZ    40
#define LIBLTE_PHY_N_SAMPS_CP_L_ELSE_7_68MHZ 36
#define LIBLTE_PHY_N_SAMPS_PER_SLOT_7_68MHZ  3840
#define LIBLTE_PHY_N_SAMPS_PER_SUBFR_7_68MHZ (LIBLTE_PHY_N_SAMPS_PER_SLOT_7_68MHZ*LIBLTE_PHY_N_SLOTS_PER_SUBFR)
#define LIBLTE_PHY_N_SAMPS_PER_FRAME_7_68MHZ (LIBLTE_PHY_N_SAMPS_PER_SUBFR_7_68MHZ*LIBLTE_PHY_N_SUBFR_PER_FRAME)
// 3MHz bandwidth
#define LIBLTE_PHY_N_SAMPS_PER_SYMB_3_84MHZ  256
#define LIBLTE_PHY_N_SAMPS_CP_L_0_3_84MHZ    20
#define LIBLTE_PHY_N_SAMPS_CP_L_ELSE_3_84MHZ 18
#define LIBLTE_PHY_N_SAMPS_PER_SLOT_3_84MHZ  1920
#define LIBLTE_PHY_N_SAMPS_PER_SUBFR_3_84MHZ (LIBLTE_PHY_N_SAMPS_PER_SLOT_3_84MHZ*LIBLTE_PHY_N_SLOTS_PER_SUBFR)
#define LIBLTE_PHY_N_SAMPS_PER_FRAME_3_84MHZ (LIBLTE_PHY_N_SAMPS_PER_SUBFR_3_84MHZ*LIBLTE_PHY_N_SUBFR_PER_FRAME)
// 1.4MHz bandwidth
#define LIBLTE_PHY_N_SAMPS_PER_SYMB_1_92MHZ  128
#define LIBLTE_PHY_N_SAMPS_CP_L_0_1_92MHZ    10
#define LIBLTE_PHY_N_SAMPS_CP_L_ELSE_1_92MHZ 9
#define LIBLTE_PHY_N_SAMPS_PER_SLOT_1_92MHZ  960
#define LIBLTE_PHY_N_SAMPS_PER_SUBFR_1_92MHZ (LIBLTE_PHY_N_SAMPS_PER_SLOT_1_92MHZ*LIBLTE_PHY_N_SLOTS_PER_SUBFR)
#define LIBLTE_PHY_N_SAMPS_PER_FRAME_1_92MHZ (LIBLTE_PHY_N_SAMPS_PER_SUBFR_1_92MHZ*LIBLTE_PHY_N_SUBFR_PER_FRAME)

/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/

typedef enum{
    LIBLTE_PHY_FS_30_72MHZ = 0, // 20MHz and 15MHz bandwidths
    LIBLTE_PHY_FS_15_36MHZ,     // 10MHz bandwidth
    LIBLTE_PHY_FS_7_68MHZ,      // 5MHz bandwidth
    LIBLTE_PHY_FS_3_84MHZ,      // 3MHz bandwidth
    LIBLTE_PHY_FS_1_92MHZ,      // 1.4MHz bandwidth
}LIBLTE_PHY_FS_ENUM;

typedef enum{
    LIBLTE_PHY_PRE_CODER_TYPE_TX_DIVERSITY = 0,
    LIBLTE_PHY_PRE_CODER_TYPE_SPATIAL_MULTIPLEXING,
}LIBLTE_PHY_PRE_CODER_TYPE_ENUM;

typedef enum{
    LIBLTE_PHY_MODULATION_TYPE_BPSK = 0,
    LIBLTE_PHY_MODULATION_TYPE_QPSK,
    LIBLTE_PHY_MODULATION_TYPE_16QAM,
    LIBLTE_PHY_MODULATION_TYPE_64QAM,
}LIBLTE_PHY_MODULATION_TYPE_ENUM;

typedef enum{
    LIBLTE_PHY_CHAN_TYPE_DLSCH = 0,
    LIBLTE_PHY_CHAN_TYPE_PCH,
}LIBLTE_PHY_CHAN_TYPE_ENUM;

typedef struct{
    // Receive
    float rx_symb_re[16][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];
    float rx_symb_im[16][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];
    float rx_ce_re[LIBLTE_PHY_N_ANT_MAX][16][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];
    float rx_ce_im[LIBLTE_PHY_N_ANT_MAX][16][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];

    // Transmit
    float tx_symb_re[LIBLTE_PHY_N_ANT_MAX][16][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];
    float tx_symb_im[LIBLTE_PHY_N_ANT_MAX][16][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];

    // Common
    uint32 num;
}LIBLTE_PHY_SUBFRAME_STRUCT;

/*******************************************************************************
                              DECLARATIONS
*******************************************************************************/

/*********************************************************************
    Name: liblte_phy_init

    Description: Initializes the LTE Physical Layer library.

    Document Reference: N/A
*********************************************************************/
// Defines
#define LIBLTE_PHY_INIT_N_ID_CELL_UNKNOWN 0xFFFF
// Enums
// Structs
typedef struct{
    // PDSCH
    float  pdsch_y_est_re[5000];
    float  pdsch_y_est_im[5000];
    float  pdsch_c_est_re[LIBLTE_PHY_N_ANT_MAX][5000];
    float  pdsch_c_est_im[LIBLTE_PHY_N_ANT_MAX][5000];
    float  pdsch_y_re[LIBLTE_PHY_N_ANT_MAX][5000];
    float  pdsch_y_im[LIBLTE_PHY_N_ANT_MAX][5000];
    float  pdsch_x_re[10000];
    float  pdsch_x_im[10000];
    float  pdsch_d_re[10000];
    float  pdsch_d_im[10000];
    float  pdsch_descramb_bits[10000];
    uint32 pdsch_c[10000];
    uint8  pdsch_encode_bits[10000];
    uint8  pdsch_scramb_bits[10000];
    int8   pdsch_soft_bits[10000];

    // BCH
    float  bch_y_est_re[240];
    float  bch_y_est_im[240];
    float  bch_c_est_re[LIBLTE_PHY_N_ANT_MAX][240];
    float  bch_c_est_im[LIBLTE_PHY_N_ANT_MAX][240];
    float  bch_y_re[LIBLTE_PHY_N_ANT_MAX][240];
    float  bch_y_im[LIBLTE_PHY_N_ANT_MAX][240];
    float  bch_x_re[480];
    float  bch_x_im[480];
    float  bch_d_re[480];
    float  bch_d_im[480];
    float  bch_descramb_bits[1920];
    float  bch_rx_d_bits[1920];
    uint32 bch_c[1920];
    uint32 bch_N_bits;
    uint8  bch_tx_d_bits[1920];
    uint8  bch_c_bits[40];
    uint8  bch_encode_bits[1920];
    uint8  bch_scramb_bits[480];
    int8   bch_soft_bits[480];

    // PDCCH
    // FIXME: Sizes
    float  pdcch_reg_y_est_re[550][4];
    float  pdcch_reg_y_est_im[550][4];
    float  pdcch_reg_c_est_re[LIBLTE_PHY_N_ANT_MAX][550][4];
    float  pdcch_reg_c_est_im[LIBLTE_PHY_N_ANT_MAX][550][4];
    float  pdcch_shift_y_est_re[550][4];
    float  pdcch_shift_y_est_im[550][4];
    float  pdcch_shift_c_est_re[LIBLTE_PHY_N_ANT_MAX][550][4];
    float  pdcch_shift_c_est_im[LIBLTE_PHY_N_ANT_MAX][550][4];
    float  pdcch_perm_y_est_re[550][4];
    float  pdcch_perm_y_est_im[550][4];
    float  pdcch_perm_c_est_re[LIBLTE_PHY_N_ANT_MAX][550][4];
    float  pdcch_perm_c_est_im[LIBLTE_PHY_N_ANT_MAX][550][4];
    float  pdcch_cce_y_est_re[50][576];
    float  pdcch_cce_y_est_im[50][576];
    float  pdcch_cce_c_est_re[LIBLTE_PHY_N_ANT_MAX][50][576];
    float  pdcch_cce_c_est_im[LIBLTE_PHY_N_ANT_MAX][50][576];
    float  pdcch_y_est_re[576];
    float  pdcch_y_est_im[576];
    float  pdcch_c_est_re[LIBLTE_PHY_N_ANT_MAX][576];
    float  pdcch_c_est_im[LIBLTE_PHY_N_ANT_MAX][576];
    float  pdcch_y_re[LIBLTE_PHY_N_ANT_MAX][576];
    float  pdcch_y_im[LIBLTE_PHY_N_ANT_MAX][576];
    float  pdcch_cce_re[LIBLTE_PHY_N_ANT_MAX][50][576];
    float  pdcch_cce_im[LIBLTE_PHY_N_ANT_MAX][50][576];
    float  pdcch_reg_re[LIBLTE_PHY_N_ANT_MAX][550][4];
    float  pdcch_reg_im[LIBLTE_PHY_N_ANT_MAX][550][4];
    float  pdcch_perm_re[LIBLTE_PHY_N_ANT_MAX][550][4];
    float  pdcch_perm_im[LIBLTE_PHY_N_ANT_MAX][550][4];
    float  pdcch_shift_re[LIBLTE_PHY_N_ANT_MAX][550][4];
    float  pdcch_shift_im[LIBLTE_PHY_N_ANT_MAX][550][4];
    float  pdcch_x_re[576];
    float  pdcch_x_im[576];
    float  pdcch_d_re[576];
    float  pdcch_d_im[576];
    float  pdcch_descramb_bits[576];
    uint32 pdcch_c[1152];
    uint32 pdcch_permute_map[550][550];
    uint16 pdcch_reg_vec[550];
    uint16 pdcch_reg_perm_vec[550];
    uint8  pdcch_dci[100]; // FIXME: This is a guess at worst case
    uint8  pdcch_encode_bits[576];
    uint8  pdcch_scramb_bits[576];
    int8   pdcch_soft_bits[576];

    // CRS & Channel Estimate
    float crs_re[14][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];
    float crs_im[14][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];
    float ce_crs_re[16][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];
    float ce_crs_im[16][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];
    float ce_mag[5][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];
    float ce_ang[5][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];

    // PSS
    float pss_mod_re_n1[3][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];
    float pss_mod_im_n1[3][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];
    float pss_mod_re[3][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];
    float pss_mod_im[3][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];
    float pss_mod_re_p1[3][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];
    float pss_mod_im_p1[3][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];

    // SSS
    float sss_mod_re_0[168][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];
    float sss_mod_im_0[168][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];
    float sss_mod_re_5[168][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];
    float sss_mod_im_5[168][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];
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

    // Timing
    float timing_abs_corr[LIBLTE_PHY_N_SAMPS_PER_SLOT_30_72MHZ*2];

    // CRS Storage
    float  crs_re_storage[20][3][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];
    float  crs_im_storage[20][3][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];
    uint32 N_id_cell_crs;

    // Samples to Symbols & Symbols to Samples
    fftw_complex *s2s_in;
    fftw_complex *s2s_out;
    fftw_plan     symbs_to_samps_plan;
    fftw_plan     samps_to_symbs_plan;

    // Viterbi decode
    float vd_path_metric[128][2048];
    float vd_br_metric[128][2];
    float vd_p_metric[128][2];
    float vd_br_weight[128][2];
    float vd_w_metric[128][2048];
    float vd_tb_state[2048];
    float vd_tb_weight[2048];
    uint8 vd_st_output[128][2][3];

    // Turbo encode
    uint8 te_z[6144];
    uint8 te_fb1[6144];
    uint8 te_c_prime[6144];
    uint8 te_z_prime[6144];
    uint8 te_x_prime[6144];

    // Turbo decode
    int8 td_vitdec_in[18432];
    int8 td_in_int[6144];
    int8 td_in_calc_1[6144];
    int8 td_in_calc_2[6144];
    int8 td_in_calc_3[6144];
    int8 td_in_int_1[6144];
    int8 td_int_calc_1[6144];
    int8 td_int_calc_2[6144];
    int8 td_in_act_1[6144];
    int8 td_fb_1[6144];
    int8 td_int_act_1[6144];
    int8 td_int_act_2[6144];
    int8 td_fb_int_1[6144];
    int8 td_fb_int_2[6144];

    // Rate Match Turbo
    uint8 rmt_tmp[6144];
    uint8 rmt_sb_mat[32][192];
    uint8 rmt_sb_perm_mat[32][192];
    uint8 rmt_y[6144];
    uint8 rmt_w[18432];

    // Rate Unmatch Turbo
    float rut_tmp[6144];
    float rut_sb_mat[32][192];
    float rut_sb_perm_mat[32][192];
    float rut_y[6144];
    float rut_w_dum[18432];
    float rut_w[18432];
    float rut_v[3][6144];

    // Rate Match Conv
    uint8 rmc_tmp[1024];
    uint8 rmc_sb_mat[32][32];
    uint8 rmc_sb_perm_mat[32][32];
    uint8 rmc_w[3*1024];

    // Rate Unatch Conv
    float ruc_tmp[1024];
    float ruc_sb_mat[32][32];
    float ruc_sb_perm_mat[32][32];
    float ruc_w_dum[3*1024];
    float ruc_w[3*1024];
    float ruc_v[3][1024];

    // DLSCH
    // FIXME: Sizes
    float  dlsch_rx_d_bits[75376];
    float  dlsch_rx_e_bits[5][18432];
    uint32 dlsch_N_c_bits[5];
    uint32 dlsch_N_e_bits[5];
    uint8  dlsch_b_bits[30720];
    uint8  dlsch_c_bits[5][6144];
    uint8  dlsch_tx_d_bits[75376];
    uint8  dlsch_tx_e_bits[5][18432];

    // DCI
    float dci_rx_d_bits[576];
    uint8 dci_tx_d_bits[576];
    uint8 dci_c_bits[192];

    // Generic
    float  rx_symb_re[LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];
    float  rx_symb_im[LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];
    uint32 fs;
    uint32 N_samps_per_symb;
    uint32 N_samps_cp_l_0;
    uint32 N_samps_cp_l_else;
    uint32 N_samps_per_slot;
    uint32 N_samps_per_subfr;
    uint32 N_samps_per_frame;
    uint32 N_rb_dl;
    uint32 N_sc_rb;
    uint32 FFT_pad_size;
    uint32 FFT_size;
}LIBLTE_PHY_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_phy_init(LIBLTE_PHY_STRUCT  **phy_struct,
                                  LIBLTE_PHY_FS_ENUM   fs,
                                  uint16               N_id_cell,
                                  uint8                N_ant,
                                  uint32               N_rb_dl,
                                  uint32               N_sc_rb,
                                  float                phich_res);

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
    Name: liblte_phy_update_n_rb_dl

    Description: Updates N_rb_dl and all associated variables.

    Document Reference: N/A
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_phy_update_n_rb_dl(LIBLTE_PHY_STRUCT *phy_struct,
                                            uint32             N_rb_dl);

/*********************************************************************
    Name: liblte_phy_pdsch_channel_encode

    Description: Encodes and modulates the Physical Downlink Shared
                 Channel

    Document Reference: 3GPP TS 36.211 v10.1.0 sections 6.3 and 6.4
*********************************************************************/
// Defines
#define LIBLTE_PHY_PDCCH_MAX_ALLOC 10
// Enums
// Structs
typedef struct{
    LIBLTE_RRC_MSG_STRUCT           msg;
    LIBLTE_PHY_PRE_CODER_TYPE_ENUM  pre_coder_type;
    LIBLTE_PHY_MODULATION_TYPE_ENUM mod_type;
    uint32                          tbs;
    uint32                          rv_idx;
    uint32                          N_prb;
    uint32                          prb[LIBLTE_PHY_N_RB_DL_MAX];
    uint32                          N_codewords;
    uint32                          tx_mode;
    uint16                          rnti;
    uint8                           mcs;
}LIBLTE_PHY_ALLOCATION_STRUCT;

typedef struct{
    LIBLTE_PHY_ALLOCATION_STRUCT alloc[LIBLTE_PHY_PDCCH_MAX_ALLOC];
    uint32                       N_symbs;
    uint32                       N_alloc;
}LIBLTE_PHY_PDCCH_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_phy_pdsch_channel_encode(LIBLTE_PHY_STRUCT          *phy_struct,
                                                  LIBLTE_PHY_PDCCH_STRUCT    *pdcch,
                                                  uint32                      N_id_cell,
                                                  uint8                       N_ant,
                                                  LIBLTE_PHY_SUBFRAME_STRUCT *subframe);

/*********************************************************************
    Name: liblte_phy_pdsch_channel_decode

    Description: Demodulates and decodes the Physical Downlink Shared
                 Channel

    Document Reference: 3GPP TS 36.211 v10.1.0 sections 6.3 and 6.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_phy_pdsch_channel_decode(LIBLTE_PHY_STRUCT            *phy_struct,
                                                  LIBLTE_PHY_SUBFRAME_STRUCT   *subframe,
                                                  LIBLTE_PHY_ALLOCATION_STRUCT *alloc,
                                                  uint32                        N_pdcch_symbs,
                                                  uint32                        N_id_cell,
                                                  uint8                         N_ant,
                                                  uint8                        *out_bits,
                                                  uint32                       *N_out_bits);

/*********************************************************************
    Name: liblte_phy_bch_channel_encode

    Description: Encodes and modulates the Physical Broadcast
                 Channel

    Document Reference: 3GPP TS 36.211 v10.1.0 section 6.6
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_phy_bch_channel_encode(LIBLTE_PHY_STRUCT          *phy_struct,
                                                uint8                      *in_bits,
                                                uint32                      N_in_bits,
                                                uint32                      N_id_cell,
                                                uint8                       N_ant,
                                                LIBLTE_PHY_SUBFRAME_STRUCT *subframe,
                                                uint32                      sfn);

/*********************************************************************
    Name: liblte_phy_bch_channel_decode

    Description: Demodulates and decodes the Physical Broadcast
                 Channel

    Document Reference: 3GPP TS 36.211 v10.1.0 section 6.6
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_phy_bch_channel_decode(LIBLTE_PHY_STRUCT          *phy_struct,
                                                LIBLTE_PHY_SUBFRAME_STRUCT *subframe,
                                                uint32                      N_id_cell,
                                                uint8                      *N_ant,
                                                uint8                      *out_bits,
                                                uint32                     *N_out_bits,
                                                uint8                      *offset);

/*********************************************************************
    Name: liblte_phy_pdcch_channel_encode

    Description: Encodes and modulates all of the Physical Downlink
                 Control Channels (PCFICH, PHICH, and PDCCH)

    Document Reference: 3GPP TS 36.211 v10.1.0 sections 6.7, 6.8, and
                        6.9
                        3GPP TS 36.212 v10.1.0 section 5.1.4.2.1
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_PHY_DCI_CA_NOT_PRESENT = 0,
    LIBLTE_PHY_DCI_CA_PRESENT,
}LIBLTE_PHY_DCI_CA_PRESENCE_ENUM;
// Structs
typedef struct{
    uint32 cfi;
}LIBLTE_PHY_PCFICH_STRUCT;

typedef struct{
    // FIXME: Place holder
}LIBLTE_PHY_PHICH_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_phy_pdcch_channel_encode(LIBLTE_PHY_STRUCT              *phy_struct,
                                                  LIBLTE_PHY_PCFICH_STRUCT       *pcfich,
                                                  LIBLTE_PHY_PHICH_STRUCT        *phich,
                                                  LIBLTE_PHY_PDCCH_STRUCT        *pdcch,
                                                  uint32                          N_id_cell,
                                                  uint8                           N_ant,
                                                  float                           phich_res,
                                                  LIBLTE_RRC_PHICH_DURATION_ENUM  phich_dur,
                                                  LIBLTE_PHY_SUBFRAME_STRUCT     *subframe);

/*********************************************************************
    Name: liblte_phy_pdcch_channel_decode

    Description: Demodulates and decodes all of the Physical Downlink
                 Control Channels (PCFICH, PHICH, and PDCCH)

    Document Reference: 3GPP TS 36.211 v10.1.0 sections 6.7, 6.8, and
                        6.9
                        3GPP TS 36.212 v10.1.0 section 5.1.4.2.1
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_phy_pdcch_channel_decode(LIBLTE_PHY_STRUCT              *phy_struct,
                                                  LIBLTE_PHY_SUBFRAME_STRUCT     *subframe,
                                                  uint32                          N_id_cell,
                                                  uint8                           N_ant,
                                                  float                           phich_res,
                                                  LIBLTE_RRC_PHICH_DURATION_ENUM  phich_dur,
                                                  LIBLTE_PHY_PCFICH_STRUCT       *pcfich,
                                                  LIBLTE_PHY_PHICH_STRUCT        *phich,
                                                  LIBLTE_PHY_PDCCH_STRUCT        *pdcch);

/*********************************************************************
    Name: liblte_phy_map_crs

    Description: Maps the Cell Specific Reference Signals to the
                 subframe

    Document Reference: 3GPP TS 36.211 v10.1.0 section 6.10.1
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_phy_map_crs(LIBLTE_PHY_STRUCT          *phy_struct,
                                     LIBLTE_PHY_SUBFRAME_STRUCT *subframe,
                                     uint32                      N_id_cell,
                                     uint8                       N_ant);

/*********************************************************************
    Name: liblte_phy_map_pss

    Description: Maps the Primary Synchronization Signal to the
                 subframe.

    Document Reference: 3GPP TS 36.211 v10.1.0 section 6.11.1
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_phy_map_pss(LIBLTE_PHY_STRUCT          *phy_struct,
                                     LIBLTE_PHY_SUBFRAME_STRUCT *subframe,
                                     uint32                      N_id_2,
                                     uint8                       N_ant);

/*********************************************************************
    Name: liblte_phy_find_pss_and_fine_timing

    Description: Searches for the Primary Synchronization Signal and
                 determines fine timing.

    Document Reference: 3GPP TS 36.211 v10.1.0 section 6.11.1
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
                                                      float             *pss_thresh,
                                                      float             *freq_offset);

/*********************************************************************
    Name: liblte_phy_map_sss

    Description: Maps the Secondary Synchronization Signal to the
                 subframe.

    Document Reference: 3GPP TS 36.211 v10.1.0 section 6.11.2
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_phy_map_sss(LIBLTE_PHY_STRUCT          *phy_struct,
                                     LIBLTE_PHY_SUBFRAME_STRUCT *subframe,
                                     uint32                      N_id_1,
                                     uint32                      N_id_2,
                                     uint8                       N_ant);

/*********************************************************************
    Name: liblte_phy_find_sss

    Description: Searches for the Secondary Synchronization Signal.

    Document Reference: 3GPP TS 36.211 v10.1.0 section 6.11.2
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
    Name: liblte_phy_find_coarse_timing_and_freq_offset

    Description: Finds coarse time syncronization and frequency offset
                 by auto-correlating to find the cyclic prefix on
                 reference signal symbols.

    Document Reference: 3GPP TS 36.211 v10.1.0
*********************************************************************/
// Defines
#define LIBLTE_PHY_N_MAX_ROUGH_CORR_SEARCH_PEAKS 5
// Enums
// Structs
typedef struct{
    float  freq_offset[LIBLTE_PHY_N_MAX_ROUGH_CORR_SEARCH_PEAKS];
    uint32 symb_starts[LIBLTE_PHY_N_MAX_ROUGH_CORR_SEARCH_PEAKS][7];
    uint32 n_corr_peaks;
}LIBLTE_PHY_COARSE_TIMING_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_phy_find_coarse_timing_and_freq_offset(LIBLTE_PHY_STRUCT               *phy_struct,
                                                                float                           *i_samps,
                                                                float                           *q_samps,
                                                                uint32                           N_slots,
                                                                LIBLTE_PHY_COARSE_TIMING_STRUCT *timing_struct);

/*********************************************************************
    Name: liblte_phy_create_subframe

    Description: Creates the baseband signal for a particular
                 subframe

    Document Reference: 3GPP TS 36.211 v10.1.0
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_phy_create_subframe(LIBLTE_PHY_STRUCT          *phy_struct,
                                             LIBLTE_PHY_SUBFRAME_STRUCT *subframe,
                                             uint8                       ant,
                                             float                      *i_samps,
                                             float                      *q_samps);

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
LIBLTE_ERROR_ENUM liblte_phy_get_subframe_and_ce(LIBLTE_PHY_STRUCT          *phy_struct,
                                                 float                      *i_samps,
                                                 float                      *q_samps,
                                                 uint32                      frame_start_idx,
                                                 uint8                       subfr_num,
                                                 uint32                      N_id_cell,
                                                 uint8                       N_ant,
                                                 LIBLTE_PHY_SUBFRAME_STRUCT *subframe);

/*********************************************************************
    Name: liblte_phy_get_tbs_mcs_and_n_prb_for_si

    Description: Determines the transport block size, modulation and
                 coding scheme, and the number of PRBs needed to send
                 a certain number of bits for SI

    Document Reference: 3GPP TS 36.213 v10.3.0 section 7.1.7
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_phy_get_tbs_mcs_and_n_prb_for_si(uint32  N_bits,
                                                          uint32  N_subframe,
                                                          uint32  N_rb_dl,
                                                          uint32 *tbs,
                                                          uint8  *mcs,
                                                          uint32 *N_prb);

#endif /* __LIBLTE_PHY_H__ */
