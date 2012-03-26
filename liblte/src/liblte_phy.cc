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

    File: liblte_phy.cc

    Description: Contains all the implementations for the LTE Physical Layer
                 library.

    Revision History
    ----------    -------------    --------------------------------------------
    02/26/2012    Ben Wojtowicz    Created file.

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "liblte_phy.h"
#include <math.h>

/*******************************************************************************
                              DEFINES
*******************************************************************************/

#define N_RB_DL_MAX         110
#define N_SYMB_DL_NORMAL_CP 7
#define N_CP_L_ELSE         144
#define N_CP_L_0            160

/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/

typedef enum{
    PRE_CODER_TYPE_TX_DIVERSITY = 0,
    PRE_CODER_TYPE_SPATIAL_MULTIPLEXING,
}PRE_CODER_TYPE_ENUM;

typedef enum{
    MODULATION_TYPE_BPSK = 0,
    MODULATION_TYPE_QPSK,
    MODULATION_TYPE_16QAM,
    MODULATION_TYPE_64QAM,
}MODULATION_TYPE_ENUM;

/*******************************************************************************
                              GLOBAL VARIABLES
*******************************************************************************/


/*******************************************************************************
                              LOCAL FUNCTION PROTOTYPES
*******************************************************************************/

/*********************************************************************
    Name: generate_pss

    Description: Generates an LTE primary synchronization signal

    Document Reference: 3GPP TS 36.211 v10.1.0 section 6.11.1.1
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM generate_pss(uint32  N_id_2,
                               float  *pss_re,
                               float  *pss_im);

/*********************************************************************
    Name: generate_sss

    Description: Generates LTE secondary synchronization signals

    Document Reference: 3GPP TS 36.211 v10.1.0 section 6.11.2.1
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM generate_sss(LIBLTE_PHY_STRUCT *phy_struct,
                               uint32             N_id_1,
                               uint32             N_id_2,
                               float             *sss_re_0,
                               float             *sss_im_0,
                               float             *sss_re_5,
                               float             *sss_im_5);

/*********************************************************************
    Name: generate_crs

    Description: Generates LTE cell specific reference signals

    Document Reference: 3GPP TS 36.211 v10.1.0 section 6.10.1.1
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM generate_crs(uint32  N_s,
                               uint32  L,
                               uint32  N_id_cell,
                               float  *crs_re,
                               float  *crs_im);

/*********************************************************************
    Name: generate_prs_c

    Description: Generates the psuedo random sequence c

    Document Reference: 3GPP TS 36.211 v10.1.0 section 7.2
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM generate_prs_c(uint32  c_init,
                                 uint32  len,
                                 uint32 *c);

/*********************************************************************
    Name: pre_coder

    Description: Generates a block of vectors to be mapped onto
                 resources on each antenna port

    Document Reference: 3GPP TS 36.211 v10.1.0 section 6.3.4

    NOTES: Currently only supports signle antenna or TX diversity
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM pre_coder(float               *x_re,
                            float               *x_im,
                            uint32               M_layer_symb,
                            uint32               N_ant,
                            PRE_CODER_TYPE_ENUM  type,
                            float               *y_re,
                            float               *y_im,
                            uint32              *M_ap_symb);

/*********************************************************************
    Name: pre_decoder_and_matched_filter

    Description: Matched filters and unmaps a block of vectors from
                 resources on each antenna port

    Document Reference: 3GPP TS 36.211 v10.1.0 section 6.3.4

    NOTES: Currently only supports signle antenna or TX diversity
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM pre_decoder_and_matched_filter(float               *y_re,
                                                 float               *y_im,
                                                 float               *h_re,
                                                 float               *h_im,
                                                 uint32               M_ap_symb,
                                                 uint32               N_ant,
                                                 PRE_CODER_TYPE_ENUM  type,
                                                 float               *x_re,
                                                 float               *x_im,
                                                 uint32              *M_layer_symb);

/*********************************************************************
    Name: layer_mapper

    Description: Maps complex-valued modulation symbols onto one or
                 several layers

    Document Reference: 3GPP TS 36.211 v10.1.0 section 6.3.3

    NOTES: Currently only supports single antenna or TX diversity
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM layer_mapper(float               *d_re,
                               float               *d_im,
                               uint32               M_symb,
                               uint32               N_ant,
                               uint32               N_codewords,
                               PRE_CODER_TYPE_ENUM  type,
                               float               *x_re,
                               float               *x_im,
                               uint32              *M_layer_symb);

/*********************************************************************
    Name: layer_demapper

    Description: De-maps one or several layers into complex-valued
                 modulation symbols

    Document Reference: 3GPP TS 36.211 v10.1.0 section 6.3.3

    NOTES: Currently only supports single antenna or TX diversity
*********************************************************************/
// Defines
#define NULL_SYMB 10000
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM layer_demapper(float               *x_re,
                                 float               *x_im,
                                 uint32               M_layer_symb,
                                 uint32               N_ant,
                                 uint32               N_codewords,
                                 PRE_CODER_TYPE_ENUM  type,
                                 float               *d_re,
                                 float               *d_im,
                                 uint32              *M_symb);

/*********************************************************************
    Name: modulation_mapper

    Description: Maps binary digits to complex-valued modulation
                 symbols

    Document Reference: 3GPP TS 36.211 v10.1.0 section 7.1

    NOTES: Currently only supports BPSK and QPSK
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM modulation_mapper(uint8                *bits,
                                    uint32                N_bits,
                                    MODULATION_TYPE_ENUM  type,
                                    float                *d_re,
                                    float                *d_im,
                                    uint32               *M_symb);

/*********************************************************************
    Name: modulation_demapper

    Description: Maps complex-valued modulation symbols to binary
                 digits

    Document Reference: 3GPP TS 36.211 v10.1.0 section 7.1

    NOTES: Currently only supports BPSK and QPSK
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM modulation_demapper(float                *d_re,
                                      float                *d_im,
                                      uint32                M_symb,
                                      MODULATION_TYPE_ENUM  type,
                                      int8                 *bits,
                                      uint32               *N_bits);

/*********************************************************************
    Name: get_soft_decision

    Description: Determines the magnitude of the soft decision

    Document Reference: N/A
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
float get_soft_decision(float p1_re,
                        float p1_im,
                        float p2_re,
                        float p2_im,
                        float max_dist);

/*********************************************************************
    Name: bch_channel_encode

    Description: Channel encodes the broadcast channel

    Document Reference: 3GPP TS 36.212 v10.1.0 section 5.3.1
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM bch_channel_encode(LIBLTE_PHY_STRUCT *phy_struct,
                                     float             *in_bits,
                                     uint32             in_bits_len,
                                     uint8              N_ant,
                                     uint8             *out_bits,
                                     uint32            *out_bits_len);

/*********************************************************************
    Name: bch_channel_decode

    Description: Channel decodes the broadcast channel

    Document Reference: 3GPP TS 36.212 v10.1.0 section 5.3.1
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM bch_channel_decode(LIBLTE_PHY_STRUCT *phy_struct,
                                     float             *in_bits,
                                     uint32             in_bits_len,
                                     uint8             *N_ant,
                                     uint8             *out_bits,
                                     uint32            *out_bits_len);

/*********************************************************************
    Name: rate_match_conv

    Description: Rate matches convolutionally encoded data

    Document Reference: 3GPP TS 36.212 v10.1.0 Section 5.1.4.2
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
void rate_match_conv(LIBLTE_PHY_STRUCT *phy_struct,
                     float             *d_bits,
                     uint32             num_d_bits,
                     uint32             num_e_bits,
                     float             *e_bits);

/*********************************************************************
    Name: rate_unmatch_conv

    Description: Rate unmatches convolutionally encoded data

    Document Reference: 3GPP TS 36.212 v10.1.0 Section 5.1.4.2
*********************************************************************/
// Defines
#define NULL_BIT 10000
// Enums
// Structs
// Functions
void rate_unmatch_conv(LIBLTE_PHY_STRUCT *phy_struct,
                       float             *e_bits,
                       uint32             num_e_bits,
                       uint32             num_c_bits,
                       float             *d_bits,
                       uint32            *num_d_bits);

/*********************************************************************
    Name: viterbi_decode

    Description: Viterbi decodes a convolutionally coded input bit
                 array using the provided parameters

    Document Reference: N/A
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
void viterbi_decode(LIBLTE_PHY_STRUCT *phy_struct,
                    float             *d_bits,
                    uint32             num_d_bits,
                    uint32             constraint_len,
                    uint32             rate,
                    uint32            *g,
                    uint8             *c_bits,
                    uint32            *num_c_bits);

/*********************************************************************
    Name: calc_crc

    Description: Calculates one of the LTE CRCs

    Document Reference: 3GPP TS 36.212 v10.1.0 Section 5.1.1
*********************************************************************/
// Defines
#define CRC24A 0x01864CFB
#define CRC24B 0x01800063
#define CRC16  0x00011021
#define CRC8   0x0000019B
// Enums
// Structs
// Functions
void calc_crc(uint8  *a_bits,
              uint32  num_a_bits,
              uint32  crc,
              uint8  *p_bits,
              uint32  num_p_bits);

/*********************************************************************
    Name: samples_to_symbols

    Description: Converts I/Q samples to subcarrier symbols

    Document Reference: 3GPP TS 36.211 v10.1.0 section 6.12
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM samples_to_symbols(LIBLTE_PHY_STRUCT *phy_struct,
                                     float             *samps_re,
                                     float             *samps_im,
                                     uint32             slot_start_idx,
                                     uint32             symbol_offset,
                                     uint32             FFT_pad_size,
                                     uint8              scale,
                                     float             *symb_re,
                                     float             *symb_im);

/*******************************************************************************
                              LIBRARY FUNCTIONS
*******************************************************************************/

/*********************************************************************
    Name: liblte_phy_init

    Description: Initializes the LTE Physical Layer library.

    Document Reference: N/A
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_phy_init(LIBLTE_PHY_STRUCT **phy_struct)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(phy_struct != NULL)
    {
        *phy_struct = (LIBLTE_PHY_STRUCT *)malloc(sizeof(LIBLTE_PHY_STRUCT));

//        /* Samples to symbols */
//        phy_struct->in   = (fftw_complex *)fftw_malloc(sizeof(fftw_complex)*2048*20);
//        phy_struct->out  = (fftw_complex *)fftw_malloc(sizeof(fftw_complex)*2048*20);
//        phy_struct->plan = fftw_plan_dft_1d(2048,
//                                            phy_struct->in,
//                                            phy_struct->out,
//                                            FFTW_FORWARD,
//                                            FFTW_ESTIMATE);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    Name: liblte_phy_cleanup

    Description: Cleans up the LTE Physical Layer library.

    Document Reference: N/A
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_phy_cleanup(LIBLTE_PHY_STRUCT *phy_struct)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(phy_struct != NULL)
    {
//        /* Samples to symbols */
//        fftw_destroy_plan(phy_struct->plan);
//        fftw_free(phy_struct->in);
//        fftw_free(phy_struct->out);

        free(phy_struct);
        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    Name: liblte_phy_find_coarse_timing_and_freq_offset

    Description: Finds coarse time syncronization and frequency offset
                 by auto-correlating to find the cyclic prefix on
                 reference signal symbols.

    Document Reference: 3GPP TS 36.211 v10.1.0
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_phy_find_coarse_timing_and_freq_offset(LIBLTE_PHY_STRUCT *phy_struct,
                                                                float             *i_samps,
                                                                float             *q_samps,
                                                                uint32            *symb_starts,
                                                                float             *freq_offset)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;
    float             corr_re;
    float             corr_im;
    float             abs_corr_max;
    int32             abs_corr_idx[2];
    uint32            slot;
    uint32            i;
    uint32            j;
    uint32            k;
    uint32            idx;

    if(phy_struct  != NULL &&
       i_samps     != NULL &&
       q_samps     != NULL &&
       symb_starts != NULL &&
       freq_offset != NULL)
    {
        // Rough correlation
        memset(phy_struct->timing_abs_corr, 0, 15360*sizeof(float));
        for(slot=0; slot<10; slot++)
        {
            for(i=0; i<15360; i+=40)
            {
                corr_re = 0;
                corr_im = 0;
                for(j=0; j<144; j++)
                {
                    idx      = (slot*15360) + i + j;
                    corr_re += i_samps[idx]*i_samps[idx+2048] + q_samps[idx]*q_samps[idx+2048];
                    corr_im += i_samps[idx]*q_samps[idx+2048] - q_samps[idx]*i_samps[idx+2048];
                }
                phy_struct->timing_abs_corr[i] += corr_re*corr_re + corr_im*corr_im;
            }
        }

        // Find first and second max
        for(i=0; i<2; i++)
        {
            abs_corr_idx[i] = 0;
            abs_corr_max    = 0;
            for(j=0; j<7680; j++)
            {
                if(phy_struct->timing_abs_corr[(i*7680)+j] > abs_corr_max)
                {
                    abs_corr_max    = phy_struct->timing_abs_corr[(i*7680)+j];
                    abs_corr_idx[i] = (i*7680)+j;
                }
            }
        }

        // Fine correlation and fractional frequency offset
        memset(phy_struct->timing_abs_corr, 0, 15360*sizeof(float));
        memset(phy_struct->timing_corr_freq_err, 0, 15360*sizeof(float));
        for(slot=0; slot<10; slot++)
        {
            for(i=0; i<2; i++)
            {
                if((abs_corr_idx[i] - 40) < 0)
                {
                    abs_corr_idx[i] = 40;
                }
                if((abs_corr_idx[i] + 40) > 15359)
                {
                    abs_corr_idx[i] = 15359 - 40;
                }
                for(j=abs_corr_idx[i]-40; j<(uint32)(abs_corr_idx[i]+41); j++)
                {
                    corr_re = 0;
                    corr_im = 0;
                    for(k=0; k<144; k++)
                    {
                        idx      = (slot*15360) + j + k;
                        corr_re += i_samps[idx]*i_samps[idx+2048] + q_samps[idx]*q_samps[idx+2048];
                        corr_im += i_samps[idx]*q_samps[idx+2048] - q_samps[idx]*i_samps[idx+2048];
                    }
                    phy_struct->timing_abs_corr[j]      += corr_re*corr_re + corr_im*corr_im;
                    phy_struct->timing_corr_freq_err[j] += atan2f(corr_im, corr_re)/(2048*2*M_PI*(0.0005/15360));
                }
            }
        }

        // Find first and second max
        for(i=0; i<2; i++)
        {
            abs_corr_idx[i] = 0;
            abs_corr_max    = 0;
            for(j=0; j<7680; j++)
            {
                if(phy_struct->timing_abs_corr[(i*7680)+j] > abs_corr_max)
                {
                    abs_corr_max    = phy_struct->timing_abs_corr[(i*7680)+j];
                    abs_corr_idx[i] = (i*7680)+j;
                }
            }
        }

        // Determine frequency offset
        // FIXME: No integer offset is calculated
        *freq_offset = ((phy_struct->timing_corr_freq_err[abs_corr_idx[0]]/10 +
                         phy_struct->timing_corr_freq_err[abs_corr_idx[1]]/10)/2);

        // Determine the symbol start locations from the correlation peaks
        // FIXME: Needs some work
        while(abs_corr_idx[0] > 0)
        {
            abs_corr_idx[0] -= 2192;
        }
        for(i=0; i<7; i++)
        {
            symb_starts[i] = abs_corr_idx[0] + ((i+1)*2192);
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    Name: liblte_phy_find_pss_and_fine_timing

    Description: Searches for the Primary Synchronization Signal and
                 determines fine timing.

    Document Reference: 3GPP TS 36.211 v10.1.0
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_phy_find_pss_and_fine_timing(LIBLTE_PHY_STRUCT *phy_struct,
                                                      float             *i_samps,
                                                      float             *q_samps,
                                                      uint32            *symb_starts,
                                                      uint32            *N_id_2,
                                                      uint32            *pss_symb,
                                                      float             *pss_thresh)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;
    float             corr_re;
    float             corr_im;
    float             abs_corr;
    float             corr_max;
    float             pss_re[63];
    float             pss_im[63];
    int32             i;
    uint32            j;
    uint32            k;
    uint32            z;
    uint32            N_s;
    uint32            N_symb;
    uint32            pss_timing_idx;
    int8              timing;

    if(phy_struct  != NULL &&
       i_samps     != NULL &&
       q_samps     != NULL &&
       symb_starts != NULL &&
       N_id_2      != NULL &&
       pss_symb    != NULL &&
       pss_thresh  != NULL)
    {
        // Generate PSS
        memset(phy_struct->pss_mod_re, 0, sizeof(float)*3*LIBLTE_N_RB_DL_20MHZ*LIBLTE_N_SC_RB_NORMAL_CP);
        memset(phy_struct->pss_mod_im, 0, sizeof(float)*3*LIBLTE_N_RB_DL_20MHZ*LIBLTE_N_SC_RB_NORMAL_CP);
        for(i=0; i<3; i++)
        {
            generate_pss(i, pss_re, pss_im);
            for(j=0; j<62; j++)
            {
                k                            = j - 31 + (LIBLTE_N_RB_DL_20MHZ*LIBLTE_N_SC_RB_NORMAL_CP)/2;
                phy_struct->pss_mod_re[i][k] = pss_re[j];
                phy_struct->pss_mod_im[i][k] = pss_im[j];
            }
        }

        // Demod symbols and correlate with PSS
        corr_max = 0;
        for(i=0; i<12; i++)
        {
            for(j=0; j<N_SYMB_DL_NORMAL_CP; j++)
            {
                samples_to_symbols(phy_struct,
                                   i_samps,
                                   q_samps,
                                   symb_starts[j]+(15360*i),
                                   0,
                                   LIBLTE_FFT_PAD_SIZE_20MHZ,
                                   0,
                                   phy_struct->symb_re,
                                   phy_struct->symb_im);

                for(k=0; k<3; k++)
                {
                    corr_re = 0;
                    corr_im = 0;
                    for(z=0; z<LIBLTE_N_RB_DL_20MHZ*LIBLTE_N_SC_RB_NORMAL_CP; z++)
                    {
                        corr_re += (phy_struct->symb_re[z]*phy_struct->pss_mod_re[k][z] +
                                    phy_struct->symb_im[z]*phy_struct->pss_mod_im[k][z]);
                        corr_im += (phy_struct->symb_re[z]*phy_struct->pss_mod_im[k][z] -
                                    phy_struct->symb_im[z]*phy_struct->pss_mod_re[k][z]);
                    }
                    abs_corr = sqrt(corr_re*corr_re + corr_im*corr_im);
                    if(abs_corr > corr_max)
                    {
                        corr_max  = abs_corr;
                        *pss_symb = (i*N_SYMB_DL_NORMAL_CP)+j;
                        *N_id_2   = k;
                    }
                }
            }
        }

        // Find optimal timing
        N_s      = (*pss_symb)/7;
        N_symb   = (*pss_symb)%7;
        corr_max = 0;
        timing   = 0;
        for(i=-40; i<40; i++)
        {
            samples_to_symbols(phy_struct,
                               i_samps,
                               q_samps,
                               symb_starts[N_symb]+i+(15360*N_s),
                               0,
                               LIBLTE_FFT_PAD_SIZE_20MHZ,
                               0,
                               phy_struct->symb_re,
                               phy_struct->symb_im);

            corr_re = 0;
            corr_im = 0;
            for(j=0; j<LIBLTE_N_RB_DL_20MHZ*LIBLTE_N_SC_RB_NORMAL_CP; j++)
            {
                corr_re += (phy_struct->symb_re[j]*phy_struct->pss_mod_re[*N_id_2][j] +
                            phy_struct->symb_im[j]*phy_struct->pss_mod_im[*N_id_2][j]);
                corr_im += (phy_struct->symb_re[j]*phy_struct->pss_mod_im[*N_id_2][j] -
                            phy_struct->symb_im[j]*phy_struct->pss_mod_re[*N_id_2][j]);
            }
            abs_corr = sqrt(corr_re*corr_re + corr_im*corr_im);
            if(abs_corr > corr_max)
            {
                corr_max = abs_corr;
                timing   = i;
            }
        }
        *pss_thresh = corr_max;

        // Construct fine symbol start locations
        pss_timing_idx = symb_starts[N_symb]+(15360*N_s)+timing;
        symb_starts[0] = pss_timing_idx + (2048+144)*1 - 15360;
        symb_starts[1] = pss_timing_idx + (2048+144)*1 + 2048+160- 15360;
        symb_starts[2] = pss_timing_idx + (2048+144)*2 + 2048+160- 15360;
        symb_starts[3] = pss_timing_idx + (2048+144)*3 + 2048+160- 15360;
        symb_starts[4] = pss_timing_idx + (2048+144)*4 + 2048+160- 15360;
        symb_starts[5] = pss_timing_idx + (2048+144)*5 + 2048+160- 15360;
        symb_starts[6] = pss_timing_idx + (2048+144)*6 + 2048+160- 15360;
        while(symb_starts[0] < 0)
        {
            for(i=0; i<7; i++)
            {
                symb_starts[i] = symb_starts[i] + 307200;
            }
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    Name: liblte_phy_find_sss

    Description: Searches for the Secondary Synchronization Signal.

    Document Reference: 3GPP TS 36.211 v10.1.0
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_phy_find_sss(LIBLTE_PHY_STRUCT *phy_struct,
                                      float             *i_samps,
                                      float             *q_samps,
                                      uint32             N_id_2,
                                      uint32            *symb_starts,
                                      float              pss_thresh,
                                      uint32            *N_id_1,
                                      uint32            *frame_start_idx)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;
    float             sss_thresh;
    float             corr_re;
    float             corr_im;
    float             abs_corr;
    uint32            i;
    uint32            j;
    uint32            k;

    if(phy_struct      != NULL &&
       i_samps         != NULL &&
       q_samps         != NULL &&
       symb_starts     != NULL &&
       N_id_1          != NULL &&
       frame_start_idx != NULL)
    {
        // Generate secondary synchronization signals
        memset(phy_struct->sss_mod_re_0, 0, sizeof(float)*168*LIBLTE_N_RB_DL_20MHZ*LIBLTE_N_SC_RB_NORMAL_CP);
        memset(phy_struct->sss_mod_im_0, 0, sizeof(float)*168*LIBLTE_N_RB_DL_20MHZ*LIBLTE_N_SC_RB_NORMAL_CP);
        memset(phy_struct->sss_mod_re_5, 0, sizeof(float)*168*LIBLTE_N_RB_DL_20MHZ*LIBLTE_N_SC_RB_NORMAL_CP);
        memset(phy_struct->sss_mod_im_5, 0, sizeof(float)*168*LIBLTE_N_RB_DL_20MHZ*LIBLTE_N_SC_RB_NORMAL_CP);
        for(i=0; i<168; i++)
        {
            generate_sss(phy_struct,
                         i,
                         N_id_2,
                         phy_struct->sss_re_0,
                         phy_struct->sss_im_0,
                         phy_struct->sss_re_5,
                         phy_struct->sss_re_5);
            for(j=0; j<62; j++)
            {
                k                              = j - 31 + (LIBLTE_N_RB_DL_20MHZ*LIBLTE_N_SC_RB_NORMAL_CP)/2;
                phy_struct->sss_mod_re_0[i][k] = phy_struct->sss_re_0[j];
                phy_struct->sss_mod_im_0[i][k] = phy_struct->sss_im_0[j];
                phy_struct->sss_mod_re_5[i][k] = phy_struct->sss_re_5[j];
                phy_struct->sss_mod_im_5[i][k] = phy_struct->sss_im_5[j];
            }
        }
        sss_thresh = pss_thresh * 0.9;

        // Demod symbol and search for secondary synchronization signals
        samples_to_symbols(phy_struct,
                           i_samps,
                           q_samps,
                           symb_starts[5],
                           0,
                           LIBLTE_FFT_PAD_SIZE_20MHZ,
                           0,
                           phy_struct->symb_re,
                           phy_struct->symb_im);
        for(i=0; i<168; i++)
        {
            corr_re = 0;
            corr_im = 0;
            for(j=0; j<(LIBLTE_N_RB_DL_20MHZ*LIBLTE_N_SC_RB_NORMAL_CP); j++)
            {
                corr_re += (phy_struct->symb_re[j]*phy_struct->sss_mod_re_0[i][j] +
                            phy_struct->symb_im[j]*phy_struct->sss_mod_im_0[i][j]);
                corr_im += (phy_struct->symb_re[j]*phy_struct->sss_mod_im_0[i][j] -
                            phy_struct->symb_im[j]*phy_struct->sss_mod_re_0[i][j]);
            }
            abs_corr = sqrt(corr_re*corr_re + corr_im*corr_im);
            if(abs_corr > sss_thresh)
            {
                *N_id_1          = i;
                *frame_start_idx = symb_starts[5] - ((2048+N_CP_L_ELSE)*4 + 2048+N_CP_L_0);
                break;
            }

            corr_re = 0;
            corr_im = 0;
            for(j=0; j<(LIBLTE_N_RB_DL_20MHZ*LIBLTE_N_SC_RB_NORMAL_CP); j++)
            {
                corr_re += (phy_struct->symb_re[j]*phy_struct->sss_mod_re_5[i][j] +
                            phy_struct->symb_im[j]*phy_struct->sss_mod_im_5[i][j]);
                corr_im += (phy_struct->symb_re[j]*phy_struct->sss_mod_im_5[i][j] -
                            phy_struct->symb_im[j]*phy_struct->sss_mod_re_5[i][j]);
            }
            abs_corr = sqrt(corr_re*corr_re + corr_im*corr_im);
            if(abs_corr > sss_thresh)
            {
                *N_id_1          = i;
                *frame_start_idx = symb_starts[5] - ((2048+N_CP_L_ELSE)*4 + 2048+N_CP_L_0) - 15360*10;
                break;
            }
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    Name: liblte_phy_get_subframe_and_ce

    Description: Resolves all symbols and channel estimates for a
                 particular subframe

    Document Reference: 3GPP TS 36.211 v10.1.0
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_phy_get_subframe_and_ce(LIBLTE_PHY_STRUCT      *phy_struct,
                                                 float                  *i_samps,
                                                 float                  *q_samps,
                                                 uint32                  frame_start_idx,
                                                 uint8                   subfr_num,
                                                 uint32                  FFT_pad_size,
                                                 uint32                  N_rb_dl,
                                                 uint32                  N_id_cell,
                                                 uint8                   N_ant,
                                                 LIBLTE_PHY_SLOT_STRUCT *slot)
{
    LIBLTE_ERROR_ENUM  err = LIBLTE_ERROR_INVALID_INPUTS;
    float             *sym_re;
    float             *sym_im;
    float             *rs_re;
    float             *rs_im;
    float              tmp_re;
    float              tmp_im;
    float              frac_mag = 0;
    float              frac_ang = 0;
    float              ce_mag;
    float              ce_ang;
    uint32             v_shift         = N_id_cell % 6;
    uint32             subfr_start_idx = frame_start_idx + subfr_num*30720;
    uint32             N_sym;
    uint32             m_prime;
    uint32             i;
    uint32             j;
    uint32             k = 0;
    uint32             p;
    uint32             v[5];
    uint32             slot_n[5];
    uint32             z;

    if(phy_struct != NULL &&
       i_samps    != NULL &&
       q_samps    != NULL &&
       (N_ant     == 1    ||
        N_ant     == 2    ||
        N_ant     == 4)   &&
       slot       != NULL)
    {
        for(i=0; i<16; i++)
        {
            // Demodulate symbols
            samples_to_symbols(phy_struct,
                               i_samps,
                               q_samps,
                               subfr_start_idx + (i/7)*15360,
                               i%7,
                               FFT_pad_size,
                               0,
                               &slot->symb_re[i][0],
                               &slot->symb_im[i][0]);
        }

        // Generate cell specific reference signals
        generate_crs((subfr_num*2+0)%20, 0, N_id_cell, phy_struct->ce_crs_re[0],  phy_struct->ce_crs_im[0]);
        generate_crs((subfr_num*2+0)%20, 1, N_id_cell, phy_struct->ce_crs_re[1],  phy_struct->ce_crs_im[1]);
        generate_crs((subfr_num*2+0)%20, 4, N_id_cell, phy_struct->ce_crs_re[4],  phy_struct->ce_crs_im[4]);
        generate_crs((subfr_num*2+1)%20, 0, N_id_cell, phy_struct->ce_crs_re[7],  phy_struct->ce_crs_im[7]);
        generate_crs((subfr_num*2+1)%20, 1, N_id_cell, phy_struct->ce_crs_re[8],  phy_struct->ce_crs_im[8]);
        generate_crs((subfr_num*2+1)%20, 4, N_id_cell, phy_struct->ce_crs_re[11], phy_struct->ce_crs_im[11]);
        generate_crs((subfr_num*2+2)%20, 0, N_id_cell, phy_struct->ce_crs_re[14], phy_struct->ce_crs_im[14]);
        generate_crs((subfr_num*2+2)%20, 1, N_id_cell, phy_struct->ce_crs_re[15], phy_struct->ce_crs_im[15]);

        // Determine channel estimates
        for(p=0; p<N_ant; p++)
        {
            // Define v, crs, sym, and N_sym
            if(p == 0)
            {
                v[0]      = 0;
                v[1]      = 3;
                v[2]      = 0;
                v[3]      = 3;
                v[4]      = 0;
                slot_n[0] = 0;
                slot_n[1] = 4;
                slot_n[2] = 7;
                slot_n[3] = 11;
                slot_n[4] = 14;
                N_sym     = 5;
            }else if(p == 1){
                v[0]      = 3;
                v[1]      = 0;
                v[2]      = 3;
                v[3]      = 0;
                v[4]      = 3;
                slot_n[0] = 0;
                slot_n[1] = 4;
                slot_n[2] = 7;
                slot_n[3] = 11;
                slot_n[4] = 14;
                N_sym     = 5;
            }else if(p == 2){
                v[0]      = 0;
                v[1]      = 3;
                v[2]      = 0;
                slot_n[0] = 1;
                slot_n[1] = 8;
                slot_n[2] = 15;
                N_sym     = 3;
            }else{ // p == 3
                v[0]      = 3;
                v[1]      = 6;
                v[2]      = 3;
                slot_n[0] = 1;
                slot_n[1] = 8;
                slot_n[2] = 15;
                N_sym     = 3;
            }

            for(i=0; i<N_sym; i++)
            {
                sym_re = &slot->symb_re[slot_n[i]][0];
                sym_im = &slot->symb_im[slot_n[i]][0];
                rs_re  = &phy_struct->ce_crs_re[slot_n[i]][0];
                rs_im  = &phy_struct->ce_crs_im[slot_n[i]][0];

                for(j=0; j<2*N_rb_dl; j++)
                {
                    k                        = 6*j + (v[i] + v_shift)%6;
                    m_prime                  = j + N_RB_DL_MAX - N_rb_dl;
                    tmp_re                   = sym_re[k]*rs_re[m_prime] + sym_im[k]*rs_im[m_prime];
                    tmp_im                   = sym_im[k]*rs_re[m_prime] - sym_re[k]*rs_im[m_prime];
                    phy_struct->ce_mag[i][k] = sqrt(tmp_re*tmp_re + tmp_im*tmp_im);
                    phy_struct->ce_ang[i][k] = atan2f(tmp_im, tmp_re);

                    // Unwrap phase
                    if(j > 0)
                    {
                        while((phy_struct->ce_ang[i][k] - phy_struct->ce_ang[i][k-6]) > M_PI)
                        {
                            phy_struct->ce_ang[i][k] -= 2*M_PI;
                        }
                        while((phy_struct->ce_ang[i][k] - phy_struct->ce_ang[i][k-6]) < -M_PI)
                        {
                            phy_struct->ce_ang[i][k] += 2*M_PI;
                        }

                        // Linearly interpolate between CRSs
                        frac_mag = (phy_struct->ce_mag[i][k] - phy_struct->ce_mag[i][k-6])/6;
                        frac_ang = (phy_struct->ce_ang[i][k] - phy_struct->ce_ang[i][k-6])/6;
                        for(z=1; z<6; z++)
                        {
                            phy_struct->ce_mag[i][k-z] = phy_struct->ce_mag[i][k-(z-1)] - frac_mag;
                            phy_struct->ce_ang[i][k-z] = phy_struct->ce_ang[i][k-(z-1)] - frac_ang;
                        }
                    }

                    // Linearly interpolate before 1st CRS
                    if(j == 1)
                    {
                        for(z=1; z<((v[i] + v_shift)%6)+1; z++)
                        {
                            phy_struct->ce_mag[i][k-6-z] = phy_struct->ce_mag[i][k-6-(z-1)] - frac_mag;
                            phy_struct->ce_ang[i][k-6-z] = phy_struct->ce_ang[i][k-6-(z-1)] - frac_ang;
                        }
                    }
                }

                // Linearly interpolate after last CRS
                for(z=1; z<(5-(v[i] + v_shift)%6)+1; z++)
                {
                    phy_struct->ce_mag[i][k+z] = phy_struct->ce_mag[i][k+(z-1)] - frac_mag;
                    phy_struct->ce_ang[i][k+z] = phy_struct->ce_ang[i][k+(z-1)] - frac_ang;
                }
            }

            // Linearly interpolate between symbols to construct all channel estimates
            if(N_sym == 3)
            {
                for(j=0; j<N_rb_dl*LIBLTE_N_SC_RB_NORMAL_CP; j++)
                {
                    // Construct symbol 1 and 8 channel estimates directly
                    slot->ce_re[p][1][j] = phy_struct->ce_mag[0][j]*cosf(phy_struct->ce_ang[0][j]);
                    slot->ce_im[p][1][j] = phy_struct->ce_mag[0][j]*sinf(phy_struct->ce_ang[0][j]);
                    slot->ce_re[p][8][j] = phy_struct->ce_mag[1][j]*cosf(phy_struct->ce_ang[1][j]);
                    slot->ce_im[p][8][j] = phy_struct->ce_mag[1][j]*sinf(phy_struct->ce_ang[1][j]);

                    // Interpolate for symbol 2, 3, 4, 5, 6, and 7 channel estimates
                    frac_mag = (phy_struct->ce_mag[1][j] - phy_struct->ce_mag[0][j])/7;
                    frac_ang = (phy_struct->ce_ang[1][j] - phy_struct->ce_ang[0][j]);
                    if(frac_ang >= M_PI)
                    {
                        // Wrap angle
                        frac_ang -= 2*M_PI;
                    }else if(frac_ang <= -M_PI){
                        // Wrap angle
                        frac_ang += 2*M_PI;
                    }
                    frac_ang /= 7;
                    ce_mag    = phy_struct->ce_mag[1][j];
                    ce_ang    = phy_struct->ce_ang[1][j];
                    for(z=7; z>1; z--)
                    {
                        ce_mag               -= frac_mag;
                        ce_ang               -= frac_ang;
                        slot->ce_re[p][z][j]  = ce_mag*cosf(ce_ang);
                        slot->ce_im[p][z][j]  = ce_mag*sinf(ce_ang);
                    }

                    // Interpolate for symbol 0 channel estimate
                    // FIXME: Use previous slot to do this correctly
                    ce_mag               = phy_struct->ce_mag[0][j] - frac_mag;
                    ce_ang               = phy_struct->ce_ang[0][j] - frac_ang;
                    slot->ce_re[p][0][j] = ce_mag*cosf(ce_ang);
                    slot->ce_im[p][0][j] = ce_mag*sinf(ce_ang);

                    // Interpolate for symbol 9, 10, 11, 12, and 13 channel estimates
                    frac_mag = (phy_struct->ce_mag[2][j] - phy_struct->ce_mag[1][j])/7;
                    frac_ang = (phy_struct->ce_ang[2][j] - phy_struct->ce_ang[1][j]);
                    if(frac_ang >= M_PI)
                    {
                        // Wrap angle
                        frac_ang -= 2*M_PI;
                    }else if(frac_ang <= -M_PI){
                        // Wrap angle
                        frac_ang += 2*M_PI;
                    }
                    frac_ang /= 7;
                    ce_mag    = phy_struct->ce_mag[2][j] - frac_mag;
                    ce_ang    = phy_struct->ce_ang[2][j] - frac_ang;
                    for(z=13; z>8; z--)
                    {
                        ce_mag               -= frac_mag;
                        ce_ang               -= frac_ang;
                        slot->ce_re[p][z][j]  = ce_mag*cosf(ce_ang);
                        slot->ce_im[p][z][j]  = ce_mag*sinf(ce_ang);
                    }
                }
            }else{
                for(j=0; j<N_rb_dl*LIBLTE_N_SC_RB_NORMAL_CP; j++)
                {
                    // Construct symbol 0, 4, 7, and 11 channel estimates directly
                    slot->ce_re[p][0][j]  = phy_struct->ce_mag[0][j]*cosf(phy_struct->ce_ang[0][j]);
                    slot->ce_im[p][0][j]  = phy_struct->ce_mag[0][j]*sinf(phy_struct->ce_ang[0][j]);
                    slot->ce_re[p][4][j]  = phy_struct->ce_mag[1][j]*cosf(phy_struct->ce_ang[1][j]);
                    slot->ce_im[p][4][j]  = phy_struct->ce_mag[1][j]*sinf(phy_struct->ce_ang[1][j]);
                    slot->ce_re[p][7][j]  = phy_struct->ce_mag[2][j]*cosf(phy_struct->ce_ang[2][j]);
                    slot->ce_im[p][7][j]  = phy_struct->ce_mag[2][j]*sinf(phy_struct->ce_ang[2][j]);
                    slot->ce_re[p][11][j] = phy_struct->ce_mag[3][j]*cosf(phy_struct->ce_ang[3][j]);
                    slot->ce_im[p][11][j] = phy_struct->ce_mag[3][j]*sinf(phy_struct->ce_ang[3][j]);

                    // Interpolate for symbol 1, 2, and 3 channel estimates
                    frac_mag = (phy_struct->ce_mag[1][j] - phy_struct->ce_mag[0][j])/4;
                    frac_ang = (phy_struct->ce_ang[1][j] - phy_struct->ce_ang[0][j]);
                    if(frac_ang >= M_PI)
                    {
                        // Wrap phase
                        frac_ang -= 2*M_PI;
                    }else if(frac_ang <= -M_PI){
                        // Wrap phase
                        frac_ang += 2*M_PI;
                    }
                    frac_ang /= 4;
                    ce_mag    = phy_struct->ce_mag[1][j];
                    ce_ang    = phy_struct->ce_ang[1][j];
                    for(z=3; z>0; z--)
                    {
                        ce_mag               -= frac_mag;
                        ce_ang               -= frac_ang;
                        slot->ce_re[p][z][j]  = ce_mag*cosf(ce_ang);
                        slot->ce_im[p][z][j]  = ce_mag*sinf(ce_ang);
                    }

                    // Interpolate for symbol 5 and 6 channel estimates
                    frac_mag = (phy_struct->ce_mag[2][j] - phy_struct->ce_mag[1][j])/3;
                    frac_ang = (phy_struct->ce_ang[2][j] - phy_struct->ce_ang[1][j]);
                    if(frac_ang >= M_PI)
                    {
                        // Wrap angle
                        frac_ang -= 2*M_PI;
                    }else if(frac_ang <= -M_PI){
                        // Wrap angle
                        frac_ang += 2*M_PI;
                    }
                    frac_ang /= 3;
                    ce_mag    = phy_struct->ce_mag[2][j];
                    ce_ang    = phy_struct->ce_ang[2][j];
                    for(z=6; z>4; z--)
                    {
                        ce_mag               -= frac_mag;
                        ce_ang               -= frac_ang;
                        slot->ce_re[p][z][j]  = ce_mag*cosf(ce_ang);
                        slot->ce_im[p][z][j]  = ce_mag*sinf(ce_ang);
                    }

                    // Interpolate for symbol 8, 9, and 10 channel estimates
                    frac_mag = (phy_struct->ce_mag[3][j] - phy_struct->ce_mag[2][j])/4;
                    frac_ang = (phy_struct->ce_ang[3][j] - phy_struct->ce_ang[2][j]);
                    if(frac_ang >= M_PI)
                    {
                        // Wrap angle
                        frac_ang -= 2*M_PI;
                    }else if(frac_ang <= M_PI){
                        // Wrap angle
                        frac_ang += 2*M_PI;
                    }
                    frac_ang /= 4;
                    ce_mag    = phy_struct->ce_mag[3][j];
                    ce_ang    = phy_struct->ce_ang[3][j];
                    for(z=10; z>7; z--)
                    {
                        ce_mag               -= frac_mag;
                        ce_ang               -= frac_ang;
                        slot->ce_re[p][z][j]  = ce_mag*cosf(ce_ang);
                        slot->ce_im[p][z][j]  = ce_mag*sinf(ce_ang);
                    }

                    // Interpolate for symbol 12 and 13 channel estimates
                    frac_mag = (phy_struct->ce_mag[4][j] - phy_struct->ce_mag[3][j])/3;
                    frac_ang = (phy_struct->ce_ang[4][j] - phy_struct->ce_ang[3][j]);
                    if(frac_ang >= M_PI)
                    {
                        // Wrap angle
                        frac_ang -= 2*M_PI;
                    }else if(frac_ang <= -M_PI){
                        // Wrap angle
                        frac_ang += 2*M_PI;
                    }
                    frac_ang /= 3;
                    ce_mag    = phy_struct->ce_mag[4][j];
                    ce_ang    = phy_struct->ce_ang[4][j];
                    for(z=13; z>11; z--)
                    {
                        ce_mag               -= frac_mag;
                        ce_ang               -= frac_ang;
                        slot->ce_re[p][z][j]  = ce_mag*cosf(ce_ang);
                        slot->ce_im[p][z][j]  = ce_ang*sinf(ce_ang);
                    }
                }
            }
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    Name: liblte_phy_bch_channel_decode

    Description: Demodulates and decodes the Physical Broadcast
                 Channel

    Document Reference: 3GPP TS 36.211 v10.1.0
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_phy_bch_channel_decode(LIBLTE_PHY_STRUCT      *phy_struct,
                                                LIBLTE_PHY_SLOT_STRUCT *slot,
                                                uint32                  N_id_cell,
                                                uint8                  *N_ant,
                                                uint8                  *bits,
                                                uint32                 *num_bits,
                                                uint8                  *offset)
{
    LIBLTE_ERROR_ENUM  err = LIBLTE_ERROR_INVALID_INPUTS;
    uint32             i;
    uint32             j;
    uint32             p;
    uint32             idx;
    uint32             M_layer_symb;
    uint32             M_symb;
    uint32             N_bits;

    if(phy_struct != NULL &&
       slot       != NULL &&
       N_id_cell  >= 0    &&
       N_id_cell  <= 503  &&
       N_ant      != NULL &&
       bits       != NULL &&
       num_bits   != NULL &&
       offset     != NULL)
    {
        err = LIBLTE_ERROR_DECODE_FAIL;

        // Unmap PBCH and channel estimates from resource elements
        idx = 0;
        for(i=0; i<72; i++) // FIXME: This is assuming 1.4MHz
        {
            if((N_id_cell % 3) != (i % 3))
            {
                phy_struct->bch_y_est_re[idx]    = slot->symb_re[7][i];
                phy_struct->bch_y_est_im[idx]    = slot->symb_im[7][i];
                phy_struct->bch_y_est_re[idx+48] = slot->symb_re[8][i];
                phy_struct->bch_y_est_im[idx+48] = slot->symb_im[8][i];
                for(p=0; p<4; p++)
                {
                    phy_struct->bch_c_est_re[p][idx]    = slot->ce_re[p][7][i];
                    phy_struct->bch_c_est_im[p][idx]    = slot->ce_im[p][7][i];
                    phy_struct->bch_c_est_re[p][idx+48] = slot->ce_re[p][8][i];
                    phy_struct->bch_c_est_im[p][idx+48] = slot->ce_im[p][8][i];
                }
                idx++;
            }
            phy_struct->bch_y_est_re[i+96]  = slot->symb_re[9][i];
            phy_struct->bch_y_est_im[i+96]  = slot->symb_im[9][i];
            phy_struct->bch_y_est_re[i+168] = slot->symb_re[10][i];
            phy_struct->bch_y_est_im[i+168] = slot->symb_im[10][i];
            for(p=0; p<4; p++)
            {
                phy_struct->bch_c_est_re[p][i+96]  = slot->ce_re[p][9][i];
                phy_struct->bch_c_est_im[p][i+96]  = slot->ce_im[p][9][i];
                phy_struct->bch_c_est_re[p][i+168] = slot->ce_re[p][10][i];
                phy_struct->bch_c_est_im[p][i+168] = slot->ce_im[p][10][i];
            }
        }

        // Generate the scrambling sequence
        generate_prs_c(N_id_cell, 1920, phy_struct->bch_c);

        // Try decoding with 1, 2, and 4 antennas
        for(p=1; p<5; p++)
        {
            if(p != 3)
            {
                pre_decoder_and_matched_filter(phy_struct->bch_y_est_re,
                                               phy_struct->bch_y_est_im,
                                               phy_struct->bch_c_est_re[0],
                                               phy_struct->bch_c_est_im[0],
                                               240,
                                               p,
                                               PRE_CODER_TYPE_TX_DIVERSITY,
                                               phy_struct->bch_x_re,
                                               phy_struct->bch_x_im,
                                               &M_layer_symb);
                layer_demapper(phy_struct->bch_x_re,
                               phy_struct->bch_x_im,
                               M_layer_symb,
                               p,
                               1,
                               PRE_CODER_TYPE_TX_DIVERSITY,
                               phy_struct->bch_d_re,
                               phy_struct->bch_d_im,
                               &M_symb);
                modulation_demapper(phy_struct->bch_d_re,
                                    phy_struct->bch_d_im,
                                    M_symb,
                                    MODULATION_TYPE_QPSK,
                                    phy_struct->bch_soft_bits,
                                    &N_bits);

                // Try decoding at each offset
                *N_ant = 0;
                for(i=0; i<4; i++)
                {
                    for(j=0; j<1920; j++)
                    {
                        phy_struct->bch_descramb_bits[j] = NULL_BIT;
                    }
                    for(j=0; j<480; j++)
                    {
                        phy_struct->bch_descramb_bits[(i*480)+j] = (float)phy_struct->bch_soft_bits[j]*(1-2*(float)phy_struct->bch_c[(i*480)+j]);
                    }
                    if(LIBLTE_SUCCESS == bch_channel_decode(phy_struct,
                                                            phy_struct->bch_descramb_bits,
                                                            1920,
                                                            N_ant,
                                                            bits,
                                                            num_bits))
                    {
                        *offset = i;
                        break;
                    }
                }
                if(*N_ant != 0)
                {
                    err = LIBLTE_SUCCESS;
                    break;
                }
            }
        }
    }

    return(err);
}

/*******************************************************************************
                              LOCAL FUNCTIONS
*******************************************************************************/

/*********************************************************************
    Name: generate_pss

    Description: Generates an LTE primary synchronization signal

    Document Reference: 3GPP TS 36.211 v10.1.0 section 6.11.1.1
*********************************************************************/
LIBLTE_ERROR_ENUM generate_pss(uint32  N_id_2,
                               float  *pss_re,
                               float  *pss_im)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;
    float             root_idx;
    uint32            i;

    if(pss_re != NULL &&
       pss_im != NULL &&
       N_id_2 >= 0    &&
       N_id_2 <= 2)
    {
        if(N_id_2 == 0)
        {
            root_idx = 25;
        }else if(N_id_2 == 1){
            root_idx = 29;
        }else{
            root_idx = 34;
        }

        for(i=0; i<31; i++)
        {
            pss_re[i] = cosf(-M_PI*root_idx*i*(i+1)/63);
            pss_im[i] = sinf(-M_PI*root_idx*i*(i+1)/63);
        }
        for(i=31; i<62; i++)
        {
            pss_re[i] = cosf(-M_PI*root_idx*(i+1)*(i+2)/63);
            pss_im[i] = sinf(-M_PI*root_idx*(i+1)*(i+2)/63);
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    Name: generate_sss

    Description: Generates LTE secondary synchronization signals

    Document Reference: 3GPP TS 36.211 v10.1.0 section 6.11.2.1
*********************************************************************/
LIBLTE_ERROR_ENUM generate_sss(LIBLTE_PHY_STRUCT *phy_struct,
                               uint32             N_id_1,
                               uint32             N_id_2,
                               float             *sss_re_0,
                               float             *sss_im_0,
                               float             *sss_re_5,
                               float             *sss_im_5)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;
    uint32            i;
    uint32            q_prime;
    uint32            q;
    uint32            m_prime;
    uint32            m0;
    uint32            m1;

    if(phy_struct != NULL &&
       sss_re_0   != NULL &&
       sss_im_0   != NULL &&
       sss_re_5   != NULL &&
       sss_im_5   != NULL &&
       N_id_1     >= 0    &&
       N_id_1     <= 167  &&
       N_id_2     >= 0    &&
       N_id_2     <= 2)
    {
        // Generate m0 and m1
        q_prime = N_id_1/30;
        q       = (N_id_1 + (q_prime*(q_prime+1)/2))/30;
        m_prime = N_id_1 + (q*(q+1)/2);
        m0      = m_prime % 31;
        m1      = (m0 + (m_prime/31) + 1) % 31;

        // Generate s_tilda
        memset(phy_struct->sss_x_s_tilda, 0, sizeof(uint8)*31);
        phy_struct->sss_x_s_tilda[4] = 1;
        for(i=0; i<26; i++)
        {
            phy_struct->sss_x_s_tilda[i+5] = (phy_struct->sss_x_s_tilda[i+2] +
                                              phy_struct->sss_x_s_tilda[i]) % 2;
        }
        for(i=0; i<31; i++)
        {
            phy_struct->sss_s_tilda[i] = 1 - 2*phy_struct->sss_x_s_tilda[i];
        }

        // Generate c_tilda
        memset(phy_struct->sss_x_c_tilda, 0, sizeof(uint8)*31);
        phy_struct->sss_x_c_tilda[4] = 1;
        for(i=0; i<26; i++)
        {
            phy_struct->sss_x_c_tilda[i+5] = (phy_struct->sss_x_c_tilda[i+3] +
                                              phy_struct->sss_x_c_tilda[i]) % 2;
        }
        for(i=0; i<31; i++)
        {
            phy_struct->sss_c_tilda[i] = 1 - 2*phy_struct->sss_x_c_tilda[i];
        }

        // Generate z_tilda
        memset(phy_struct->sss_x_z_tilda, 0, sizeof(uint8)*31);
        phy_struct->sss_x_z_tilda[4] = 1;
        for(i=0; i<26; i++)
        {
            phy_struct->sss_x_z_tilda[i+5] = (phy_struct->sss_x_z_tilda[i+4] +
                                              phy_struct->sss_x_z_tilda[i+2] +
                                              phy_struct->sss_x_z_tilda[i+1] +
                                              phy_struct->sss_x_z_tilda[i]) % 2;
        }
        for(i=0; i<31; i++)
        {
            phy_struct->sss_z_tilda[i] = 1 - 2*phy_struct->sss_x_z_tilda[i];
        }

        // Generate s0_m0 and s1_m1
        for(i=0; i<31; i++)
        {
            phy_struct->sss_s0_m0[i] = phy_struct->sss_s_tilda[(i + m0) % 31];
            phy_struct->sss_s1_m1[i] = phy_struct->sss_s_tilda[(i + m1) % 31];
        }

        // Generate c0 and c1
        for(i=0; i<31; i++)
        {
            phy_struct->sss_c0[i] = phy_struct->sss_c_tilda[(i + N_id_2) % 31];
            phy_struct->sss_c1[i] = phy_struct->sss_c_tilda[(i + N_id_2 + 3) % 31];
        }

        // Generate z1_m0 and z1_m1
        for(i=0; i<31; i++)
        {
            phy_struct->sss_z1_m0[i] = phy_struct->sss_z_tilda[(i + (m0 % 8)) % 31];
            phy_struct->sss_z1_m1[i] = phy_struct->sss_z_tilda[(i + (m1 % 8)) % 31];
        }

        // Generate SSS
        for(i=0; i<31; i++)
        {
            sss_re_0[2*i]   = phy_struct->sss_s0_m0[i]*phy_struct->sss_c0[i];
            sss_im_0[2*i]   = 0;
            sss_re_0[2*i+1] = phy_struct->sss_s1_m1[i]*phy_struct->sss_c1[i]*phy_struct->sss_z1_m0[i];
            sss_im_0[2*i+1] = 0;

            sss_re_5[2*i]   = phy_struct->sss_s1_m1[i]*phy_struct->sss_c0[i];
            sss_im_5[2*i]   = 0;
            sss_re_5[2*i+1] = phy_struct->sss_s0_m0[i]*phy_struct->sss_c1[i]*phy_struct->sss_z1_m1[i];
            sss_im_5[2*i+1] = 0;
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    Name: generate_crs

    Description: Generates LTE cell specific reference signals

    Document Reference: 3GPP TS 36.211 v10.1.0 section 6.10.1.1
*********************************************************************/
LIBLTE_ERROR_ENUM generate_crs(uint32  N_s,
                               uint32  L,
                               uint32  N_id_cell,
                               float  *crs_re,
                               float  *crs_im)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;
    float             one_over_sqrt_2 = 1/sqrt(2);
    uint32            N_cp = 1; // FIXME: Only supporting normal cp
    uint32            c_init;
    uint32            len = 2*N_RB_DL_MAX;
    uint32            c[2*len];
    uint32            i;

    if(crs_re    != NULL &&
       crs_im    != NULL &&
       N_s       >= 0    &&
       N_s       <= 19   &&
       L         >= 0    &&
       L         <= 6    &&
       N_id_cell >= 0    &&
       N_id_cell <= 503)
    {
        // Calculate c_init
        c_init = 1024 * (7 * (N_s+1) + L + 1) * (2 * N_id_cell + 1) + 2*N_id_cell + N_cp;

        // Generate the psuedo random sequence c
        generate_prs_c(c_init, 2*len, c);

        // Construct the reference signals
        for(i=0; i<len; i++)
        {
            crs_re[i] = one_over_sqrt_2*(1 - 2*(float)c[2*i]);
            crs_im[i] = one_over_sqrt_2*(1 - 2*(float)c[2*i+1]);
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    Name: generate_prs_c

    Description: Generates the psuedo random sequence c

    Document Reference: 3GPP TS 36.211 v10.1.0 section 7.2
*********************************************************************/
LIBLTE_ERROR_ENUM generate_prs_c(uint32  c_init,
                                 uint32  len,
                                 uint32 *c)
{
    LIBLTE_ERROR_ENUM  err = LIBLTE_ERROR_INVALID_INPUTS;
    uint32             i;
    uint32             j;
    uint8              x1[31];
    uint8              x2[31];
    uint8              new_bit1;
    uint8              new_bit2;

    if(c != NULL)
    {
        // Initialize the m-sequences
        for(i=0; i<31; i++)
        {
            x1[i] = 0;
            x2[i] = (c_init & (1<<i))>>i;
        }
        x1[0] = 1;

        // Advance m-sequences
        for(i=0; i<(1600-31); i++)
        {
            new_bit1 = x1[3] ^ x1[0];
            new_bit2 = x2[3] ^ x2[2] ^ x2[1] ^ x2[0];

            for(j=0; j<30; j++)
            {
                x1[j] = x1[j+1];
                x2[j] = x2[j+1];
            }
            x1[30] = new_bit1;
            x2[30] = new_bit2;
        }

        // Generate c
        for(i=0; i<len; i++)
        {
            new_bit1 = x1[3] ^ x1[0];
            new_bit2 = x2[3] ^ x2[2] ^ x2[1] ^ x2[0];

            for(j=0; j<30; j++)
            {
                x1[j] = x1[j+1];
                x2[j] = x2[j+1];
            }
            x1[30] = new_bit1;
            x2[30] = new_bit2;

            c[i] = new_bit1 ^ new_bit2;
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    Name: pre_coder

    Description: Generates a block of vectors to be mapped onto
                 resources on each antenna port

    Document Reference: 3GPP TS 36.211 v10.1.0 section 6.3.4

    NOTES: Currently only supports signle antenna or TX diversity
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM pre_coder(float               *x_re,
                            float               *x_im,
                            uint32               M_layer_symb,
                            uint32               N_ant,
                            PRE_CODER_TYPE_ENUM  type,
                            float               *y_re,
                            float               *y_im,
                            uint32              *M_ap_symb)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    // FIXME

    return(err);
}

/*********************************************************************
    Name: pre_decoder_and_matched_filter

    Description: Matched filters and unmaps a block of vectors from
                 resources on each antenna port

    Document Reference: 3GPP TS 36.211 v10.1.0 section 6.3.4

    NOTES: Currently only supports single antenna or TX diversity
*********************************************************************/
LIBLTE_ERROR_ENUM pre_decoder_and_matched_filter(float               *y_re,
                                                 float               *y_im,
                                                 float               *h_re,
                                                 float               *h_im,
                                                 uint32               M_ap_symb,
                                                 uint32               N_ant,
                                                 PRE_CODER_TYPE_ENUM  type,
                                                 float               *x_re,
                                                 float               *x_im,
                                                 uint32              *M_layer_symb)
{
    LIBLTE_ERROR_ENUM  err = LIBLTE_ERROR_INVALID_INPUTS;
    float             *h_re_ptr[N_ant];
    float             *h_im_ptr[N_ant];
    float             *x_re_ptr[N_ant];
    float             *x_im_ptr[N_ant];
    float              h0_abs;
    float              h1_abs;
    float              h2_abs;
    float              h3_abs;
    float              h_norm;
    float              h_norm_0_2;
    float              h_norm_1_3;
    uint32             i;
    uint32             p;

    if(y_re         != NULL                        &&
       y_im         != NULL                        &&
       h_re         != NULL                        &&
       h_im         != NULL                        &&
       (N_ant       == 1                           ||
        N_ant       == 2                           ||
        N_ant       == 4)                          &&
       type         == PRE_CODER_TYPE_TX_DIVERSITY &&
       x_re         != NULL                        &&
       x_im         != NULL                        &&
       M_layer_symb != NULL)
    {
        // Index all arrays
        for(p=0; p<N_ant; p++)
        {
            h_re_ptr[p] = &h_re[p*M_ap_symb];
            h_im_ptr[p] = &h_im[p*M_ap_symb];
            x_re_ptr[p] = &x_re[p*(M_ap_symb/N_ant)];
            x_im_ptr[p] = &x_im[p*(M_ap_symb/N_ant)];
        }

        if(N_ant == 1)
        {
            // 3GPP TS 36.211 v10.1.0 section 6.3.4.1
            *M_layer_symb = M_ap_symb;
            for(i=0; i<*M_layer_symb; i++)
            {
                x_re_ptr[0][i] = y_re[i]*h_re_ptr[0][i] - y_im[i]*h_im_ptr[0][i];
                x_im_ptr[0][i] = y_im[i]*h_re_ptr[0][i] + y_re[i]*h_im_ptr[0][i];
            }
        }else if(N_ant == 2){
            // 3GPP TS 36.211 v10.1.0 section 6.3.4.3
            *M_layer_symb = M_ap_symb/2;
            for(i=0; i<*M_layer_symb; i++)
            {
                h0_abs         = (h_re_ptr[0][i*2] * h_re_ptr[0][i*2] +
                                  h_im_ptr[0][i*2] * h_im_ptr[0][i*2]);
                h1_abs         = (h_re_ptr[1][i*2] * h_re_ptr[1][i*2] +
                                  h_im_ptr[1][i*2] * h_im_ptr[1][i*2]);
                h_norm         = sqrt(h0_abs*h0_abs + h1_abs*h1_abs);
                x_re_ptr[0][i] = (h_re_ptr[0][i*2] * y_re[i*2+0] +
                                  h_im_ptr[0][i*2] * y_im[i*2+0] +
                                  h_re_ptr[1][i*2] * y_re[i*2+1] +
                                  h_im_ptr[1][i*2] * y_im[i*2+1]) / h_norm;
                x_im_ptr[0][i] = (h_re_ptr[0][i*2] * y_im[i*2+0] -
                                  h_im_ptr[0][i*2] * y_re[i*2+0] -
                                  h_re_ptr[1][i*2] * y_im[i*2+1] +
                                  h_im_ptr[1][i*2] * y_re[i*2+1]) / h_norm;
                x_re_ptr[1][i] = (-h_re_ptr[1][i*2] * y_re[i*2+0] -
                                  h_im_ptr[1][i*2] * y_im[i*2+0] +
                                  h_re_ptr[0][i*2] * y_re[i*2+1] +
                                  h_im_ptr[0][i*2] * y_im[i*2+1]) / h_norm;
                x_im_ptr[1][i] = (h_re_ptr[1][i*2] * y_im[i*2+0] -
                                  h_im_ptr[1][i*2] * y_re[i*2+0] +
                                  h_re_ptr[0][i*2] * y_im[i*2+1] -
                                  h_im_ptr[0][i*2] * y_re[i*2+1]) / h_norm;
            }
        }else{ // N_ant == 4
            // 3GPP TS 36.211 v10.1.0 section 6.3.4.3
            *M_layer_symb = M_ap_symb/4;
            for(i=0; i<*M_layer_symb; i++)
            {
                h0_abs         = (h_re_ptr[0][i*4+0] * h_re_ptr[0][i*4+0] +
                                  h_im_ptr[0][i*4+0] * h_im_ptr[0][i*4+0]);
                h1_abs         = (h_re_ptr[1][i*4+2] * h_re_ptr[1][i*4+2] +
                                  h_im_ptr[1][i*4+2] * h_im_ptr[1][i*4+2]);
                h2_abs         = (h_re_ptr[2][i*4+0] * h_re_ptr[2][i*4+0] +
                                  h_im_ptr[2][i*4+0] * h_im_ptr[2][i*4+0]);
                h3_abs         = (h_re_ptr[3][i*4+2] * h_re_ptr[3][i*4+2] +
                                  h_im_ptr[3][i*4+2] * h_im_ptr[3][i*4+2]);
                h_norm_0_2     = sqrt(h0_abs*h0_abs + h2_abs*h2_abs);
                h_norm_1_3     = sqrt(h1_abs*h1_abs + h3_abs*h3_abs);
                x_re_ptr[0][i] = (h_re_ptr[0][i*4+0] * y_re[i*4+0] +
                                  h_im_ptr[0][i*4+0] * y_im[i*4+0] +
                                  h_re_ptr[2][i*4+0] * y_re[i*4+1] +
                                  h_im_ptr[2][i*4+0] * y_im[i*4+1]) / h_norm_0_2;
                x_im_ptr[0][i] = (h_re_ptr[0][i*4+0] * y_im[i*4+0] -
                                  h_im_ptr[0][i*4+0] * y_re[i*4+0] -
                                  h_re_ptr[2][i*4+0] * y_im[i*4+1] +
                                  h_im_ptr[2][i*4+0] * y_re[i*4+1]) / h_norm_0_2;
                x_re_ptr[1][i] = (-h_re_ptr[2][i*4+0] * y_re[i*4+0] -
                                  h_im_ptr[2][i*4+0] * y_im[i*4+0] +
                                  h_re_ptr[0][i*4+0] * y_re[i*4+1] +
                                  h_im_ptr[0][i*4+0] * y_im[i*4+1]) / h_norm_0_2;
                x_im_ptr[1][i] = (-h_re_ptr[2][i*4+0] * y_im[i*4+0] +
                                  h_im_ptr[2][i*4+0] * y_re[i*4+0] -
                                  h_re_ptr[0][i*4+0] * y_re[i*4+1] +
                                  h_im_ptr[0][i*4+0] * y_im[i*4+1]) / h_norm_0_2;
                x_re_ptr[2][i] = (h_re_ptr[1][i*4+2] * y_re[i*4+2] +
                                  h_im_ptr[1][i*4+2] * y_im[i*4+2] +
                                  h_re_ptr[3][i*4+2] * y_re[i*4+3] +
                                  h_im_ptr[3][i*4+2] * y_im[i*4+3]) / h_norm_1_3;
                x_im_ptr[2][i] = (h_re_ptr[1][i*4+2] * y_im[i*4+2] -
                                  h_im_ptr[1][i*4+2] * y_re[i*4+2] -
                                  h_re_ptr[3][i*4+2] * y_im[i*4+3] +
                                  h_im_ptr[3][i*4+2] * y_re[i*4+3]) / h_norm_1_3;
                x_re_ptr[3][i] = (-h_re_ptr[3][i*4+2] * y_re[i*4+2] -
                                  h_im_ptr[3][i*4+2] * y_im[i*4+2] +
                                  h_re_ptr[1][i*4+2] * y_re[i*4+3] +
                                  h_im_ptr[1][i*4+2] * y_im[i*4+3]) / h_norm_1_3;
                x_im_ptr[3][i] = (-h_re_ptr[3][i*4+2] * y_im[i*4+2] +
                                  h_im_ptr[3][i*4+2] * y_re[i*4+2] -
                                  h_re_ptr[1][i*4+2] * y_re[i*4+3] +
                                  h_im_ptr[1][i*4+2] * y_im[i*4+3]) / h_norm_1_3;
            }
            if((M_ap_symb % 4) != 0)
            {
                *M_layer_symb  = (M_ap_symb+2)/4;
                h0_abs         = (h_re_ptr[0][i*4+0] * h_re_ptr[0][i*4+0] +
                                  h_im_ptr[0][i*4+0] * h_im_ptr[0][i*4+0]);
                h2_abs         = (h_re_ptr[2][i*4+0] * h_re_ptr[2][i*4+0] +
                                  h_im_ptr[2][i*4+0] * h_im_ptr[2][i*4+0]);
                h_norm_0_2     = sqrt(h0_abs*h0_abs + h2_abs*h2_abs);
                x_re_ptr[0][i] = (h_re_ptr[0][i*4+0] * y_re[i*4+0] +
                                  h_im_ptr[0][i*4+0] * y_im[i*4+0] +
                                  h_re_ptr[2][i*4+0] * y_re[i*4+1] +
                                  h_im_ptr[2][i*4+0] * y_im[i*4+1]) / h_norm_0_2;
                x_im_ptr[0][i] = (h_re_ptr[0][i*4+0] * y_im[i*4+0] -
                                  h_im_ptr[0][i*4+0] * y_re[i*4+0] -
                                  h_re_ptr[2][i*4+0] * y_im[i*4+1] +
                                  h_im_ptr[2][i*4+0] * y_re[i*4+1]) / h_norm_0_2;
                x_re_ptr[1][i] = (-h_re_ptr[2][i*4+0] * y_re[i*4+0] -
                                  h_im_ptr[2][i*4+0] * y_im[i*4+0] +
                                  h_re_ptr[0][i*4+0] * y_re[i*4+1] +
                                  h_im_ptr[0][i*4+0] * y_im[i*4+1]) / h_norm_0_2;
                x_im_ptr[1][i] = (-h_re_ptr[2][i*4+0] * y_im[i*4+0] +
                                  h_im_ptr[2][i*4+0] * y_re[i*4+0] -
                                  h_re_ptr[0][i*4+0] * y_re[i*4+1] +
                                  h_im_ptr[0][i*4+0] * y_im[i*4+1]) / h_norm_0_2;
                x_re_ptr[2][i] = NULL_SYMB;
                x_im_ptr[2][i] = NULL_SYMB;
                x_re_ptr[3][i] = NULL_SYMB;
                x_im_ptr[3][i] = NULL_SYMB;
            }
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    Name: layer_mapper

    Description: Maps complex-valued modulation symbols onto one or
                 several layers

    Document Reference: 3GPP TS 36.211 v10.1.0 section 6.3.3

    NOTES: Currently only supports single antenna or TX diversity
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM layer_mapper(float               *d_re,
                               float               *d_im,
                               uint32               M_symb,
                               uint32               N_ant,
                               uint32               N_codewords,
                               PRE_CODER_TYPE_ENUM  type,
                               float               *x_re,
                               float               *x_im,
                               uint32              *M_layer_symb)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    // FIXME

    return(err);
}

/*********************************************************************
    Name: layer_demapper

    Description: De-maps one or several layers into complex-valued
                 modulation symbols

    Document Reference: 3GPP TS 36.211 v10.1.0 section 6.3.3

    NOTES: Currently only supports single antenna or TX diversity
*********************************************************************/
LIBLTE_ERROR_ENUM layer_demapper(float               *x_re,
                                 float               *x_im,
                                 uint32               M_layer_symb,
                                 uint32               N_ant,
                                 uint32               N_codewords,
                                 PRE_CODER_TYPE_ENUM  type,
                                 float               *d_re,
                                 float               *d_im,
                                 uint32              *M_symb)
{
    LIBLTE_ERROR_ENUM  err = LIBLTE_ERROR_INVALID_INPUTS;
    float             *x_re_ptr[N_ant];
    float             *x_im_ptr[N_ant];
    uint32             i;
    uint32             p;

    if(x_re        != NULL                        &&
       x_im        != NULL                        &&
       (N_ant      == 1                           ||
        N_ant      == 2                           ||
        N_ant      == 4)                          &&
       N_codewords == 1                           &&
       type        == PRE_CODER_TYPE_TX_DIVERSITY &&
       d_re        != NULL                        &&
       d_im        != NULL)
    {
        // Index all arrays
        for(p=0; p<N_ant; p++)
        {
            x_re_ptr[p] = &x_re[p*M_layer_symb];
            x_im_ptr[p] = &x_im[p*M_layer_symb];
        }

        // 3GPP TS 36.211 v10.1.0 sections 6.3.3.1 and 6.3.3.3
        *M_symb = M_layer_symb*N_ant;
        if(N_ant                       == 4         &&
           x_re_ptr[2][M_layer_symb-1] == NULL_SYMB &&
           x_im_ptr[2][M_layer_symb-1] == NULL_SYMB &&
           x_re_ptr[3][M_layer_symb-1] == NULL_SYMB &&
           x_im_ptr[3][M_layer_symb-1] == NULL_SYMB)
        {
            *M_symb = *M_symb - 2;
        }
        for(i=0; i<M_layer_symb; i++)
        {
            for(p=0; p<N_ant; p++)
            {
                d_re[i*N_ant+p] = x_re_ptr[p][i];
                d_im[i*N_ant+p] = x_im_ptr[p][i];
            }
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    Name: modulation_mapper

    Description: Maps binary digits to complex-valued modulation
                 symbols

    Document Reference: 3GPP TS 36.211 v10.1.0 section 7.1

    NOTES: Currently only supports BPSK and QPSK
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM modulation_mapper(uint8                *bits,
                                    uint32                N_bits,
                                    MODULATION_TYPE_ENUM  type,
                                    float                *d_re,
                                    float                *d_im,
                                    uint32               *M_symb)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    // FIXME

    return(err);
}

/*********************************************************************
    Name: modulation_demapper

    Description: Maps complex-valued modulation symbols to binary
                 digits

    Document Reference: 3GPP TS 36.211 v10.1.0 section 7.1

    NOTES: Currently only supports BPSK and QPSK
*********************************************************************/
LIBLTE_ERROR_ENUM modulation_demapper(float                *d_re,
                                      float                *d_im,
                                      uint32                M_symb,
                                      MODULATION_TYPE_ENUM  type,
                                      int8                 *bits,
                                      uint32               *N_bits)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;
    float             ang;
    float             sd;
    float             act_d_re;
    float             act_d_im;
    float             one_over_sqrt_2 = 1/sqrt(2);
    uint32            i;

    if(d_re   != NULL                  &&
       d_im   != NULL                  &&
       (type  == MODULATION_TYPE_BPSK  ||
        type  == MODULATION_TYPE_QPSK) &&
       bits   != NULL                  &&
       N_bits != NULL)
    {
        if(MODULATION_TYPE_BPSK == type)
        {
            // 3GPP TS 36.211 v10.1.0 section 7.1.1
            *N_bits = M_symb;
            for(i=0; i<M_symb; i++)
            {
                ang = atan2f(d_im[i], d_re[i]);
                if((ang > -M_PI/4) && (ang < 3*M_PI/4))
                {
                    act_d_re = +one_over_sqrt_2;
                    act_d_im = +one_over_sqrt_2;
                    sd       = get_soft_decision(d_re[i], d_im[i], act_d_re, act_d_im, 1);
                    bits[i]  = +(int8)(127*sd);
                }else{
                    act_d_re = -one_over_sqrt_2;
                    act_d_im = -one_over_sqrt_2;
                    sd       = get_soft_decision(d_re[i], d_im[i], act_d_re, act_d_im, 1);
                    bits[i]  = -(int8)(127*sd);
                }
            }
        }else{ // MODULATION_TYPE_QPSK == type
            // 3GPP TS 36.211 v10.1.0 section 7.1.2
            *N_bits = M_symb*2;
            for(i=0; i<M_symb; i++)
            {
                ang = atan2f(d_im[i], d_re[i]);
                if((ang >= 0) && (ang < M_PI/2))
                {
                    act_d_re    = +one_over_sqrt_2;
                    act_d_im    = +one_over_sqrt_2;
                    sd          = get_soft_decision(d_re[i], d_im[i], act_d_re, act_d_im, 1);
                    bits[i*2+0] = +(int8)(127*sd);
                    bits[i*2+1] = +(int8)(127*sd);
                }else if((ang >= -M_PI/2) && (ang < 0)){
                    act_d_re    = +one_over_sqrt_2;
                    act_d_im    = -one_over_sqrt_2;
                    sd          = get_soft_decision(d_re[i], d_im[i], act_d_re, act_d_im, 1);
                    bits[i*2+0] = +(int8)(127*sd);
                    bits[i*2+1] = -(int8)(127*sd);
                }else if((ang >= M_PI/2) && (ang < M_PI)){
                    act_d_re    = -one_over_sqrt_2;
                    act_d_im    = +one_over_sqrt_2;
                    sd          = get_soft_decision(d_re[i], d_im[i], act_d_re, act_d_im, 1);
                    bits[i*2+0] = -(int8)(127*sd);
                    bits[i*2+1] = +(int8)(127*sd);
                }else{
                    act_d_re    = -one_over_sqrt_2;
                    act_d_im    = -one_over_sqrt_2;
                    sd          = get_soft_decision(d_re[i], d_im[i], act_d_re, act_d_im, 1);
                    bits[i*2+0] = -(int8)(127*sd);
                    bits[i*2+1] = -(int8)(127*sd);
                }
            }
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    Name: get_soft_decision

    Description: Determines the magnitude of the soft decision

    Document Reference: N/A
*********************************************************************/
float get_soft_decision(float p1_re,
                        float p1_im,
                        float p2_re,
                        float p2_im,
                        float max_dist)
{
    float diff_re;
    float diff_im;
    float dist;

    diff_re = p1_re - p2_re;
    diff_im = p1_im - p2_im;
    dist    = sqrt(diff_re*diff_re + diff_im*diff_im);

    if(dist >= max_dist)
    {
        dist = max_dist - (max_dist/120);
    }

    return(max_dist - dist);
}

/*********************************************************************
    Name: bch_channel_encode

    Description: Channel encodes the broadcast channel

    Document Reference: 3GPP TS 36.212 v10.1.0 section 5.3.1
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM bch_channel_encode(LIBLTE_PHY_STRUCT *phy_struct,
                                     float             *in_bits,
                                     uint32             in_bits_len,
                                     uint8              N_ant,
                                     uint8             *out_bits,
                                     uint32            *out_bits_len)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    // FIXME

    return(err);
}

/*********************************************************************
    Name: bch_channel_decode

    Description: Channel decodes the broadcast channel

    Document Reference: 3GPP TS 36.212 v10.1.0 section 5.3.1
*********************************************************************/
LIBLTE_ERROR_ENUM bch_channel_decode(LIBLTE_PHY_STRUCT *phy_struct,
                                     float             *in_bits,
                                     uint32             in_bits_len,
                                     uint8             *N_ant,
                                     uint8             *out_bits,
                                     uint32            *out_bits_len)
{
    LIBLTE_ERROR_ENUM  err = LIBLTE_ERROR_INVALID_INPUTS;
    uint32             ber_1;
    uint32             ber_2;
    uint32             ber_4;
    uint32             num_d_bits;
    uint32             num_c_bits;
    uint32             i;
    uint32             g[3] = {0133, 0171, 0165}; // Numbers are in octal
    uint8             *a_bits;
    uint8             *p_bits;
    uint8              c_bits[40];
    uint8              calc_p_bits[16];
    uint8              ant_mask_1[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    uint8              ant_mask_2[16] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
    uint8              ant_mask_4[16] = {0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1};

    if(in_bits      != NULL &&
       N_ant        != NULL &&
       out_bits     != NULL &&
       out_bits_len != NULL)
    {
        // Rate unmatch to get the d_bits
        rate_unmatch_conv(phy_struct, in_bits, in_bits_len, 40, phy_struct->bch_d_bits, &num_d_bits);

        // Viterbi decode the d_bits to get the c_bits
        viterbi_decode(phy_struct, phy_struct->bch_d_bits, num_d_bits, 7, 3, g, c_bits, &num_c_bits);

        // Recover a_bits and p_bits
        a_bits = &c_bits[0];
        p_bits = &c_bits[24];

        // Calculate p_bits
        calc_crc(a_bits, 24, CRC16, calc_p_bits, 16);

        // Try all p_bit masks
        ber_1 = 0;
        ber_2 = 0;
        ber_4 = 0;
        for(i=0; i<16; i++)
        {
            ber_1 += p_bits[i] ^ (calc_p_bits[i] ^ ant_mask_1[i]);
            ber_2 += p_bits[i] ^ (calc_p_bits[i] ^ ant_mask_2[i]);
            ber_4 += p_bits[i] ^ (calc_p_bits[i] ^ ant_mask_4[i]);
        }

        if(ber_1 == 0 || ber_2 == 0 || ber_4 == 0)
        {
            for(i=0; i<24; i++)
            {
                out_bits[i] = a_bits[i];
            }
            *out_bits_len = 24;
            err           = LIBLTE_SUCCESS;
        }else{
            err = LIBLTE_ERROR_INVALID_CRC;
        }
        if(ber_1 == 0)
        {
            *N_ant = 1;
        }else if(ber_2 == 0){
            *N_ant = 2;
        }else if(ber_4 == 0){
            *N_ant = 4;
        }
    }

    return(err);
}

/*********************************************************************
    Name: rate_match_conv

    Description: Rate matches convolutionally encoded data

    Document Reference: 3GPP TS 36.212 v10.1.0 Section 5.1.4.2
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
void rate_match_conv(LIBLTE_PHY_STRUCT *phy_struct,
                     float             *d_bits,
                     uint32             num_d_bits,
                     uint32             num_e_bits,
                     float             *e_bits)
{
    // FIXME
}

/*********************************************************************
    Name: rate_unmatch_conv

    Description: Rate unmatches convolutionally encoded data

    Document Reference: 3GPP TS 36.212 v10.1.0 Section 5.1.4.2
*********************************************************************/
void rate_unmatch_conv(LIBLTE_PHY_STRUCT *phy_struct,
                       float             *e_bits,
                       uint32             num_e_bits,
                       uint32             num_c_bits,
                       float             *d_bits,
                       uint32            *num_d_bits)
{
    uint32 C_cc_sb = 32; // Step 1: Assign C_cc_sb to 32
    uint32 R_cc_sb;
    uint32 w_idx = 0;
    uint32 d_idx;
    uint32 N_dummy;
    uint32 idx;
    uint32 K_pi;
    uint32 K_w;
    uint32 i;
    uint32 j;
    uint32 k;
    uint32 x;
    uint8  ic_perm[32] = {1,17,9,25,5,21,13,29,3,19,11,27,7,23,15,31,0,16,8,24,4,20,12,28,2,18,10,26,6,22,14,30};

    // In order to undo bit collection, selection, and transmission
    // a dummy block must be sub-block interleaved to determine
    // where NULL bits are to be inserted
    // Sub-block interleaving
    // Step 2: Determine the number of rows
    R_cc_sb = 0;
    while(num_c_bits > (C_cc_sb*R_cc_sb))
    {
        R_cc_sb++;
    }

    // Steps 3, 4, and 5
    for(x=0; x<3; x++)
    {
        // Step 3: Pack data into matrix and pad with dummy
        if(num_c_bits < (C_cc_sb*R_cc_sb))
        {
            N_dummy = C_cc_sb*R_cc_sb - num_c_bits;
        }else{
            N_dummy = 0;
        }
        for(i=0; i<N_dummy; i++)
        {
            phy_struct->rm_tmp[i] = NULL_BIT;
        }
        for(i=N_dummy; i<num_c_bits; i++)
        {
            phy_struct->rm_tmp[i] = 0;
        }
        idx = 0;
        for(i=0; i<R_cc_sb; i++)
        {
            for(j=0; j<C_cc_sb; j++)
            {
                phy_struct->rm_sb_mat[i][j] = phy_struct->rm_tmp[idx++];
            }
        }

        // Step 4: Inter-column permutation
        for(i=0; i<R_cc_sb; i++)
        {
            for(j=0; j<C_cc_sb; j++)
            {
                phy_struct->rm_sb_perm_mat[i][j] = phy_struct->rm_sb_mat[i][ic_perm[j]];
            }
        }

        // Step 5: Read out the bits
        for(j=0; j<C_cc_sb; j++)
        {
            for(i=0; i<R_cc_sb; i++)
            {
                phy_struct->rm_w_dum[w_idx] = phy_struct->rm_sb_perm_mat[i][j];
                phy_struct->rm_w[w_idx]     = NULL_BIT;
                w_idx++;
            }
        }
    }

    // Undo bit collection, selection, and transmission by
    // recreating the circular buffer
    K_pi = R_cc_sb*C_cc_sb;
    K_w  = 3*K_pi;
    k    = 0;
    j    = 0;
    while(k < num_e_bits)
    {
        if(phy_struct->rm_w_dum[j%K_w] != NULL_BIT)
        {
            // Soft combine the inputs
            if(phy_struct->rm_w[j%K_w] == NULL_BIT)
            {
                phy_struct->rm_w[j%K_w] = e_bits[k];
            }else if(e_bits[k] != NULL_BIT){
                phy_struct->rm_w[j%K_w] += e_bits[k];
            }
            k++;
        }
        j++;
    }

    // Recreate the sub-block interleaver output
    for(i=0; i<K_pi; i++)
    {
        phy_struct->rm_v[0][i] = phy_struct->rm_w[i];
        phy_struct->rm_v[1][i] = phy_struct->rm_w[i+K_pi];
        phy_struct->rm_v[2][i] = phy_struct->rm_w[i+2*K_pi];
    }

    // Sub-block deinterleaving
    // Steps 5, 4, and 3
    for(x=0; x<3; x++)
    {
        // Step 5: Load the permuted matrix
        idx = 0;
        for(j=0; j<C_cc_sb; j++)
        {
            for(i=0; i<R_cc_sb; i++)
            {
                phy_struct->rm_sb_perm_mat[i][j] = phy_struct->rm_v[x][idx++];
            }
        }

        // Step 4: Undo permutation
        for(i=0; i<R_cc_sb; i++)
        {
            for(j=0; j<C_cc_sb; j++)
            {
                phy_struct->rm_sb_mat[i][ic_perm[j]] = phy_struct->rm_sb_perm_mat[i][j];
            }
        }

        // Step 3: Unpack the data and remove dummy
        if(num_c_bits < (C_cc_sb*R_cc_sb))
        {
            N_dummy = C_cc_sb*R_cc_sb - num_c_bits;
        }else{
            N_dummy = 0;
        }
        idx = 0;
        for(i=0; i<R_cc_sb; i++)
        {
            for(j=0; j<C_cc_sb; j++)
            {
                phy_struct->rm_tmp[idx++] = phy_struct->rm_sb_mat[i][j];
            }
        }
        d_idx = 0;
        for(i=N_dummy; i<C_cc_sb*R_cc_sb; i++)
        {
            d_bits[d_idx*3+x] = phy_struct->rm_tmp[i];
            d_idx++;
        }
    }
    *num_d_bits = d_idx*3;
}

/*********************************************************************
    Name: viterbi_decode

    Description: Viterbi decodes a convolutionally coded input bit
                 array using the provided parameters

    Document Reference: N/A
*********************************************************************/
void viterbi_decode(LIBLTE_PHY_STRUCT *phy_struct,
                    float             *d_bits,
                    uint32             num_d_bits,
                    uint32             constraint_len,
                    uint32             rate,
                    uint32            *g,
                    uint8             *c_bits,
                    uint32            *num_c_bits)
{
    float  init_min;
    float  tmp1;
    float  tmp2;
    int32  i;
    uint32 j;
    uint32 k;
    uint32 o;
    uint32 num_states = 1<<(constraint_len-1);
    uint32 idx;
    uint32 tb_state_len;
    uint8  in_path;
    uint8  prev_state;
    uint8  in_bit;
    uint8  prev_state_0;
    uint8  prev_state_1;
    uint8  s_reg[constraint_len];
    uint8  g_array[3][7];

    // Convert g to binary
    for(i=0; i<(int32)rate; i++)
    {
        for(j=0; j<constraint_len; j++)
        {
            g_array[i][j] = (g[i] >> (constraint_len-j-1)) & 1;
        }
    }

    // Precalculate state transition outputs
    for(i=0; i<(int32)num_states; i++)
    {
        // Determine the input path
        if(i < (num_states/2))
        {
            in_path = 0;
        }else{
            in_path = 1;
        }

        // Determine the outputs based on the previous state and input path
        for(j=0; j<2; j++)
        {
            prev_state = ((i << 1) + j) % num_states;
            for(k=0; k<constraint_len; k++)
            {
                s_reg[k] = (prev_state >> (constraint_len-k-1)) & 1;
            }
            s_reg[0]   = in_path;
            for(k=0; k<rate; k++)
            {
                phy_struct->vd_st_output[i][j][k] = 0;
                for(o=0; o<constraint_len; o++)
                {
                    phy_struct->vd_st_output[i][j][k] += s_reg[o]*g_array[k][o];
                }
                phy_struct->vd_st_output[i][j][k] %= 2;
            }
        }
    }

    // Calculate branch and path metrics
    for(i=0; i<(int32)num_states; i++)
    {
        for(j=0; j<(num_d_bits/rate)+10; j++)
        {
            phy_struct->vd_path_metric[i][j] = 0;
        }
    }
    for(i=0; i<(int32)(num_d_bits/rate); i++)
    {
        for(j=0; j<num_states; j++)
        {
            phy_struct->vd_br_metric[j][0] = 0;
            phy_struct->vd_br_metric[j][1] = 0;
            phy_struct->vd_p_metric[j][0]  = 0;
            phy_struct->vd_p_metric[j][1]  = 0;
            phy_struct->vd_w_metric[j][0]  = 0;
            phy_struct->vd_w_metric[j][1]  = 0;

            // Calculate the accumulated branch metrics for each state
            for(k=0; k<2; k++)
            {
                prev_state                    = ((j<<1)+k) % num_states;
                phy_struct->vd_p_metric[j][k] = phy_struct->vd_path_metric[prev_state][i];
                for(o=0; o<rate; o++)
                {
                    if(d_bits[i*rate + o] >= 0)
                    {
                        in_bit = 0;
                    }else{
                        in_bit = 1;
                    }
                    phy_struct->vd_br_metric[j][k] += (phy_struct->vd_st_output[j][k][o]+in_bit)%2;
                    phy_struct->vd_w_metric[j][k]  += fabs(d_bits[i*rate + o]);
                }
            }

            // Keep the smallest branch metric as the path metric, weight the branch metric
            tmp1 = phy_struct->vd_br_metric[j][0] + phy_struct->vd_p_metric[j][0];
            tmp2 = phy_struct->vd_br_metric[j][1] + phy_struct->vd_p_metric[j][1];
            if(tmp1 > tmp2)
            {
                phy_struct->vd_path_metric[j][i+1] = phy_struct->vd_p_metric[j][1] + phy_struct->vd_w_metric[j][1]*phy_struct->vd_br_metric[j][1];
            }else{
                phy_struct->vd_path_metric[j][i+1] = phy_struct->vd_p_metric[j][0] + phy_struct->vd_w_metric[j][0]*phy_struct->vd_br_metric[j][0];
            }
        }
    }

    // Find the minimum metric for the last iteration
    init_min                     = 1000000;
    idx                          = 0;
    phy_struct->vd_tb_state[idx] = 1000000;
    for(i=0; i<(int32)num_states; i++)
    {
        if(phy_struct->vd_path_metric[i][(num_d_bits/rate)] < init_min)
        {
            init_min                       = phy_struct->vd_path_metric[i][(num_d_bits/rate)];
            phy_struct->vd_tb_state[idx++] = i;
        }
    }

    // Traceback to find the minimum path metrics at each iteration
    for(i=(num_d_bits/rate)-1; i>=0; i--)
    {
        prev_state_0 = ((((uint8)phy_struct->vd_tb_state[idx-1])<<1) + 0) % num_states;
        prev_state_1 = ((((uint8)phy_struct->vd_tb_state[idx-1])<<1) + 1) % num_states;

        // Keep the smallest state
        if(phy_struct->vd_path_metric[prev_state_0][i] > phy_struct->vd_path_metric[prev_state_1][i])
        {
            phy_struct->vd_tb_state[idx++] = prev_state_1;
        }else{
            phy_struct->vd_tb_state[idx++] = prev_state_0;
        }
    }
    tb_state_len = idx;

    // Read through the traceback to determine the input bits
    idx = 0;
    for(i=tb_state_len-2; i>=0; i--)
    {
        // If transition has resulted in a lower valued state,
        // the output is 0 and vice-versa
        if(phy_struct->vd_tb_state[i] < phy_struct->vd_tb_state[i+1])
        {
            c_bits[idx++] = 0;
        }else if(phy_struct->vd_tb_state[i] > phy_struct->vd_tb_state[i+1]){
            c_bits[idx++] = 1;
        }else{
            // Check to see if the transition has resulted in the same state
            // In this case, if state is 0 then output is 0
            if(phy_struct->vd_tb_state[i] == 0)
            {
                c_bits[idx++] = 0;
            }else{
                c_bits[idx++] = 1;
            }
        }
    }
    *num_c_bits = idx;
}

/*********************************************************************
    Name: calc_crc

    Description: Calculates one of the LTE CRCs

    Document Reference: 3GPP TS 36.212 v10.1.0 Section 5.1.1
*********************************************************************/
void calc_crc(uint8  *a_bits,
              uint32  num_a_bits,
              uint32  crc,
              uint8  *p_bits,
              uint32  num_p_bits)
{
    uint32 i;
    uint32 crc_rem   = 0;
    uint32 crc_check = (1 << num_p_bits);
    uint8  tmp_array[num_a_bits+num_p_bits];

    // Initialize tmp_array
    memset(tmp_array, 0, num_a_bits+num_p_bits);
    memcpy(tmp_array, a_bits, num_a_bits);

    for(i=0; i<num_a_bits+num_p_bits; i++)
    {
        crc_rem <<= 1;
        crc_rem  |= tmp_array[i];

        if(crc_rem & crc_check)
        {
            crc_rem ^= crc;
        }
    }

    for(i=0; i<num_p_bits; i++)
    {
        p_bits[i] = (crc_rem >> (num_p_bits-1-i)) & 1;
    }
}

/*********************************************************************
    Name: samples_to_symbols

    Description: Converts I/Q samples to subcarrier symbols

    Document Reference: 3GPP TS 36.211 v10.1.0 section 6.12
*********************************************************************/
LIBLTE_ERROR_ENUM samples_to_symbols(LIBLTE_PHY_STRUCT *phy_struct,
                                     float             *samps_re,
                                     float             *samps_im,
                                     uint32             slot_start_idx,
                                     uint32             symbol_offset,
                                     uint32             FFT_pad_size,
                                     uint8              scale,
                                     float             *symb_re,
                                     float             *symb_im)
{
    LIBLTE_ERROR_ENUM  err = LIBLTE_ERROR_INVALID_INPUTS;
    fftw_plan          plan;
    fftw_complex      *in;
    fftw_complex      *out;
    uint32             CP_len;
    uint32             index;
    uint32             i;

    if(phy_struct != NULL &&
       samps_re   != NULL &&
       samps_im   != NULL &&
       symb_re    != NULL &&
       symb_im    != NULL)
    {
        // Calculate index and CP length
        if((symbol_offset % 7) == 0)
        {
            CP_len = 160;
        }else{
            CP_len = 144;
        }
        index = slot_start_idx + (2048+144)*symbol_offset;
        if(symbol_offset > 0)
        {
            index += 16;
        }

        // FIXME
        in = (fftw_complex *)fftw_malloc(sizeof(fftw_complex)*2048*20);
        out = (fftw_complex *)fftw_malloc(sizeof(fftw_complex)*2048*20);
        plan = fftw_plan_dft_1d(2048, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
        for(i=0; i<2048; i++)
        {
            in[i][0] = samps_re[index+CP_len+i];
            in[i][1] = samps_im[index+CP_len+i];
        }
        fftw_execute(plan);
        for(i=0; i<1024-FFT_pad_size; i++)
        {
            // Postive spectrum
            symb_re[i+(1024-FFT_pad_size)] = out[i+1][0];
            symb_im[i+(1024-FFT_pad_size)] = out[i+1][1];

            // Negative spectrum
            symb_re[(1024-FFT_pad_size)-i-1] = out[2048-i-1][0];
            symb_im[(1024-FFT_pad_size)-i-1] = out[2048-i-1][1];
        }
        fftw_destroy_plan(plan);
        fftw_free(in);
        fftw_free(out);

        if(scale == 1)
        {
            for(i=0; i<2*(1024-FFT_pad_size); i++)
            {
                symb_re[i] = cosf(atan2f(symb_im[i], symb_re[i]));
                symb_im[i] = sinf(atan2f(symb_im[i], symb_re[i]));
            }
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
