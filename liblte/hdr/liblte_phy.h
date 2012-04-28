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
    04/21/2012    Ben Wojtowicz    Added Turbo encode/decode and PDSCH decode

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

#define LIBLTE_PHY_FFT_PAD_SIZE_1_4MHZ 988
#define LIBLTE_PHY_FFT_PAD_SIZE_3MHZ   934
#define LIBLTE_PHY_FFT_PAD_SIZE_5MHZ   874
#define LIBLTE_PHY_FFT_PAD_SIZE_10MHZ  724
#define LIBLTE_PHY_FFT_PAD_SIZE_15MHZ  574
#define LIBLTE_PHY_FFT_PAD_SIZE_20MHZ  424
#define LIBLTE_PHY_N_RB_DL_1_4MHZ      6
#define LIBLTE_PHY_N_RB_DL_3MHZ        15
#define LIBLTE_PHY_N_RB_DL_5MHZ        25
#define LIBLTE_PHY_N_RB_DL_10MHZ       50
#define LIBLTE_PHY_N_RB_DL_15MHZ       75
#define LIBLTE_PHY_N_RB_DL_20MHZ       100
#define LIBLTE_PHY_N_RB_DL_MAX         110
#define LIBLTE_PHY_N_SC_RB_NORMAL_CP   12
#define LIBLTE_PHY_SI_RNTI             0xFFFF
#define LIBLTE_PHY_P_RNTI              0xFFFE

/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/

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
    float  symb_re[16][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];
    float  symb_im[16][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];
    float  ce_re[4][16][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];
    float  ce_im[4][16][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];
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
typedef struct{
    // Timing
    float timing_corr_freq_err[15360];
    float timing_abs_corr[15360];

    // PSS
    float pss_mod_re[3][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];
    float pss_mod_im[3][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];

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

    // Channel Estimate
    float ce_crs_re[16][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];
    float ce_crs_im[16][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];
    float ce_mag[5][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];
    float ce_ang[5][LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];

    // BCH
    float  bch_y_est_re[240];
    float  bch_y_est_im[240];
    float  bch_c_est_re[4][240];
    float  bch_c_est_im[4][240];
    float  bch_x_re[480];
    float  bch_x_im[480];
    float  bch_d_re[480];
    float  bch_d_im[480];
    float  bch_descramb_bits[1920];
    float  bch_rx_d_bits[1920];
    uint32 bch_c[1920];
    uint8  bch_tx_d_bits[1920];
    uint8  bch_c_bits[40];
    int8   bch_soft_bits[480];

    // PDCCH
    float  pdcch_reg_y_est_re[225][4];
    float  pdcch_reg_y_est_im[225][4];
    float  pdcch_reg_c_est_re[4][225][4];
    float  pdcch_reg_c_est_im[4][225][4];
    float  pdcch_shift_y_est_re[225][4];
    float  pdcch_shift_y_est_im[225][4];
    float  pdcch_shift_c_est_re[4][225][4];
    float  pdcch_shift_c_est_im[4][225][4];
    float  pdcch_perm_y_est_re[225][4];
    float  pdcch_perm_y_est_im[225][4];
    float  pdcch_perm_c_est_re[4][225][4];
    float  pdcch_perm_c_est_im[4][225][4];
    float  pdcch_cce_y_est_re[25][288];
    float  pdcch_cce_y_est_im[25][288];
    float  pdcch_cce_c_est_re[4][25][288];
    float  pdcch_cce_c_est_im[4][25][288];
    float  pdcch_y_est_re[288];
    float  pdcch_y_est_im[288];
    float  pdcch_c_est_re[4][288];
    float  pdcch_c_est_im[4][288];
    float  pdcch_x_re[576];
    float  pdcch_x_im[576];
    float  pdcch_d_re[576];
    float  pdcch_d_im[576];
    float  pdcch_descramb_bits[576];
    uint32 pdcch_c[1152];
    uint8  pdcch_dci[100]; // FIXME: This is a guess at worst case
    uint8  pdcch_reg_vec[225];
    int8   pdcch_soft_bits[576];

    // DCI
    float dci_rx_d_bits[576];
    uint8 dci_tx_d_bits[576];
    uint8 dci_c_bits[192];

    // PDSCH
    float  pdsch_y_est_re[5000];
    float  pdsch_y_est_im[5000];
    float  pdsch_c_est_re[4][5000];
    float  pdsch_c_est_im[4][5000];
    float  pdsch_x_re[10000];
    float  pdsch_x_im[10000];
    float  pdsch_d_re[10000];
    float  pdsch_d_im[10000];
    float  pdsch_descramb_bits[10000];
    uint32 pdsch_c[10000];
    int8   pdsch_soft_bits[10000];

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

    // Turbo encode
    uint8 te_z[6144];
    uint8 te_fb1[6144];
    uint8 te_c_prime[6144];
    uint8 te_z_prime[6144];
    uint8 te_x_prime[6144];

    // Turbo decode
    float td_vitdec_in[18432];
    float td_in_int[6144];
    float td_in_calc_1[6144];
    float td_in_calc_2[6144];
    float td_in_calc_3[6144];
    float td_in_int_1[6144];
    float td_int_calc_1[6144];
    float td_int_calc_2[6144];
    uint8 td_in_act_1[6144];
    uint8 td_fb_1[6144];
    uint8 td_int_act_1[6144];
    uint8 td_int_act_2[6144];
    uint8 td_fb_int_1[6144];
    uint8 td_fb_int_2[6144];

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
    float symb_re[LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];
    float symb_im[LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP];
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
                                                      float             *pss_thresh);

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
                                                 uint32                      FFT_pad_size,
                                                 uint32                      N_rb_dl,
                                                 uint32                      N_id_cell,
                                                 uint8                       N_ant,
                                                 LIBLTE_PHY_SUBFRAME_STRUCT *subframe);

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
    Name: liblte_phy_pdcch_channel_decode

    Description: Demodulates and decodes all of the Physical Downlink
                 Control Channel (PCFICH, PHICH, and PDCCH)

    Document Reference: 3GPP TS 36.211 v10.1.0 sections 6.7, 6.8, and
                        6.9
*********************************************************************/
// Defines
#define LIBLTE_PHY_PDCCH_MAX_ALLOC 10
// Enums
typedef enum{
    LIBLTE_PHY_DCI_CA_NOT_PRESENT = 0,
    LIBLTE_PHY_DCI_CA_PRESENT,
}LIBLTE_PHY_DCI_CA_PRESENCE_ENUM;
// Structs
typedef struct{
    uint32 N_reg;
    uint32 cfi;
}LIBLTE_PHY_PCFICH_STRUCT;

typedef struct{
    uint32 N_group;
    uint32 N_reg;
}LIBLTE_PHY_PHICH_STRUCT;

typedef struct{
    LIBLTE_PHY_PRE_CODER_TYPE_ENUM  pre_coder_type;
    LIBLTE_PHY_MODULATION_TYPE_ENUM mod_type;
    uint32                          tbs;
    uint32                          rv_idx;
    uint32                          N_prb;
    uint32                          prb[LIBLTE_PHY_N_RB_DL_MAX];
    uint32                          N_codewords;
    uint32                          tx_mode;
    uint16                          rnti;
}LIBLTE_PHY_ALLOCATION_STRUCT;

typedef struct{
    LIBLTE_PHY_ALLOCATION_STRUCT alloc[LIBLTE_PHY_PDCCH_MAX_ALLOC];
    uint32                       N_symbs;
    uint32                       N_reg;
    uint32                       N_cce;
    uint32                       N_alloc;
}LIBLTE_PHY_PDCCH_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_phy_pdcch_channel_decode(LIBLTE_PHY_STRUCT              *phy_struct,
                                                  LIBLTE_PHY_SUBFRAME_STRUCT     *subframe,
                                                  uint32                          N_id_cell,
                                                  uint8                           N_ant,
                                                  uint32                          N_sc_rb,
                                                  uint32                          N_rb_dl,
                                                  float                           phich_res,
                                                  LIBLTE_RRC_PHICH_DURATION_ENUM  phich_dur,
                                                  LIBLTE_PHY_PCFICH_STRUCT       *pcfich,
                                                  LIBLTE_PHY_PHICH_STRUCT        *phich,
                                                  LIBLTE_PHY_PDCCH_STRUCT        *pdcch);

/*********************************************************************
    Name: liblte_phy_pdsch_channel_decode

    Description: Demodulates and decodes the Physical Downlink Shared
                 Channel

    Document Reference: 3GPP TS 36.211 v10.1.0 section 6.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_phy_pdsch_channel_decode(LIBLTE_PHY_STRUCT          *phy_struct,
                                                  LIBLTE_PHY_SUBFRAME_STRUCT *subframe,
                                                  LIBLTE_PHY_PDCCH_STRUCT    *pdcch,
                                                  uint32                      N_id_cell,
                                                  uint8                       N_ant,
                                                  uint32                      N_sc_rb,
                                                  uint32                      N_rb_dl,
                                                  uint8                      *out_bits,
                                                  uint32                     *N_out_bits);

#endif /* __LIBLTE_PHY_H__ */
