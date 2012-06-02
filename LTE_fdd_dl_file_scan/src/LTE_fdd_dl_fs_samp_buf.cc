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
    LIBLTE_PHY_SUBFRAME_STRUCT               subframe;
    LIBLTE_PHY_PCFICH_STRUCT                 pcfich;
    LIBLTE_PHY_PHICH_STRUCT                  phich;
    LIBLTE_PHY_PDCCH_STRUCT                  pdcch;
    LIBLTE_RRC_MIB_STRUCT                    mib;
    LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1_STRUCT  sib1;
    const int8                              *in  = (const int8 *)input_items[0];
    float                                    freq_offset;
    float                                    f_samp_re;
    float                                    f_samp_im;
    float                                    pss_thresh;
    float                                    tmp_i;
    float                                    tmp_q;
    float                                    phich_res;
    uint32                                   i;
    uint32                                   j;
    uint32                                   sfn;
    uint32                                   symb_starts[7];
    uint32                                   N_id_cell;
    uint32                                   N_id_1;
    uint32                                   N_id_2;
    uint32                                   pss_symb;
    uint32                                   frame_start_idx;
    uint32                                   N_rb_dl      = LIBLTE_PHY_N_RB_DL_1_4MHZ;
    uint32                                   FFT_pad_size = LIBLTE_PHY_FFT_PAD_SIZE_1_4MHZ;
    uint32                                   offset;
    uint8                                    sfn_offset;
    uint8                                    N_ant;

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
        // Process samples to prevent I/Q swap
        if((ninput_items % 2) != 0)
        {
            if(true == last_samp_was_i)
            {
                last_samp_was_i = false;
            }else{
                last_samp_was_i = true;
            }
        }

        if(LIBLTE_SUCCESS == liblte_phy_find_coarse_timing_and_freq_offset(phy_struct,
                                                                           i_buf,
                                                                           q_buf,
                                                                           symb_starts,
                                                                           &freq_offset))
        {
            // Correct frequency error
            for(i=0; i<LTE_FDD_DL_FS_SAMP_BUF_SIZE; i++)
            {
                f_samp_re = cosf((i+1)*(-freq_offset)*2*M_PI*(0.0005/15360));
                f_samp_im = sinf((i+1)*(-freq_offset)*2*M_PI*(0.0005/15360));
                tmp_i     = i_buf[i];
                tmp_q     = q_buf[i];
                i_buf[i]  = tmp_i*f_samp_re + tmp_q*f_samp_im;
                q_buf[i]  = tmp_q*f_samp_re - tmp_i*f_samp_im;
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
                    if((sfn % 2) != 0)
                    {
                        frame_start_idx += 307200;
                        sfn++;
                    }
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
                    printf("DL LTE Channel found:\n");
                    printf("\t%-30s=%12.2f\n", "Frequency Offset", freq_offset);
                    printf("\t%-30s=%12u\n", "System Frame Number", sfn);
                    printf("\t%-30s=%12u\n", "Physical Cell ID", N_id_cell);
                    printf("\t%-30s=%12u\n", "Number of TX Antennas", N_ant);
                    switch(mib.dl_bw)
                    {
                    case LIBLTE_RRC_DL_BANDWIDTH_6:
                        printf("\t%-30s=%12f\n", "Bandwidth (MHz)", 1.4);
                        break;
                    case LIBLTE_RRC_DL_BANDWIDTH_15:
                        printf("\t%-30s=%12u\n", "Bandwidth (MHz)", 3);
                        break;
                    case LIBLTE_RRC_DL_BANDWIDTH_25:
                        printf("\t%-30s=%12u\n", "Bandwidth (MHz)", 5);
                        break;
                    case LIBLTE_RRC_DL_BANDWIDTH_50:
                        printf("\t%-30s=%12u\n", "Bandwidth (MHz)", 10);
                        break;
                    case LIBLTE_RRC_DL_BANDWIDTH_75:
                        printf("\t%-30s=%12u\n", "Bandwidth (MHz)", 15);
                        break;
                    case LIBLTE_RRC_DL_BANDWIDTH_100:
                        printf("\t%-30s=%12u\n", "Bandwidth (MHz)", 20);
                        break;
                    }
                    if(mib.phich_config.dur == LIBLTE_RRC_PHICH_DURATION_NORMAL)
                    {
                        printf("\t%-30s=%12s\n", "PHICH Duration", "Normal");
                    }else{
                        printf("\t%-30s=%12s\n", "PHICH Duration", "Extended");
                    }
                    switch(mib.phich_config.res)
                    {
                    case LIBLTE_RRC_PHICH_RESOURCE_1_6:
                        printf("\t%-30s=%12s\n", "PHICH Resource", "1/6");
                        break;
                    case LIBLTE_RRC_PHICH_RESOURCE_1_2:
                        printf("\t%-30s=%12s\n", "PHICH Resource", "1/2");
                        break;
                    case LIBLTE_RRC_PHICH_RESOURCE_1:
                        printf("\t%-30s=%12u\n", "PHICH Resource", 1);
                        break;
                    case LIBLTE_RRC_PHICH_RESOURCE_2:
                        printf("\t%-30s=%12u\n", "PHICH Resource", 2);
                        break;
                    }
                    if(LIBLTE_SUCCESS == liblte_phy_get_subframe_and_ce(phy_struct,
                                                                        i_buf,
                                                                        q_buf,
                                                                        frame_start_idx-1,
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
                                                                         &pdcch,
                                                                         N_id_cell,
                                                                         N_ant,
                                                                         LIBLTE_PHY_N_SC_RB_NORMAL_CP,
                                                                         N_rb_dl,
                                                                         rrc_msg.msg,
                                                                         &rrc_msg.N_bits) &&
                       LIBLTE_SUCCESS == liblte_rrc_unpack_bcch_dlsch_msg(&rrc_msg,
                                                                          &sib1))
                    {
                        printf("\t%-30s\n", "PLMN Identity List:");
                        for(i=0; i<sib1.N_plmn_ids; i++)
                        {
                            printf("\t\t%03X-", sib1.plmn_id[i].id.mcc & 0x0FFF);
                            if((sib1.plmn_id[i].id.mnc & 0xFF00) == 0xFF00)
                            {
                                printf("%02X, ", sib1.plmn_id[i].id.mnc & 0x00FF);
                            }else{
                                printf("%03X, ", sib1.plmn_id[i].id.mnc & 0x0FFF);
                            }
                            if(LIBLTE_RRC_RESV_FOR_OPER == sib1.plmn_id[i].resv_for_oper)
                            {
                                printf("reserved for operator use\n");
                            }else{
                                printf("not reserved for operator use\n");
                            }
                        }
                        printf("\t%-30s=%12u\n", "Tracking Area Code", sib1.tracking_area_code);
                        printf("\t%-30s=%12u\n", "Cell Identity", sib1.cell_id);
                        switch(sib1.cell_barred)
                        {
                        case LIBLTE_RRC_CELL_BARRED:
                            printf("\t%-30s=%12s\n", "Cell Barred", "TRUE");
                            break;
                        case LIBLTE_RRC_CELL_NOT_BARRED:
                            printf("\t%-30s=%12s\n", "Cell Barred", "FALSE");
                            break;
                        }
                        switch(sib1.intra_freq_reselection)
                        {
                        case LIBLTE_RRC_INTRA_FREQ_RESELECTION_ALLOWED:
                            printf("\t%-30s=%12s\n", "Intra Frequency Reselection", "Allowed");
                            break;
                        case LIBLTE_RRC_INTRA_FREQ_RESELECTION_NOT_ALLOWED:
                            printf("\t%-30s=%12s\n", "Intra Frequency Reselection", "Not Allowed");
                            break;
                        }
                        if(true == sib1.csg_indication)
                        {
                            printf("\t%-30s=%12s\n", "CSG Indication", "TRUE");
                        }else{
                            printf("\t%-30s=%12s\n", "CSG Indication", "FALSE");
                        }
                        if(LIBLTE_RRC_CSG_IDENTITY_NOT_PRESENT != sib1.csg_id)
                        {
                            printf("\t%-30s=%12u\n", "CSG Identity", sib1.csg_id);
                        }
                        printf("\t%-30s=%12d\n", "Q Rx Lev Min", sib1.q_rx_lev_min);
                        printf("\t%-30s=%12u\n", "Q Rx Lev Min Offset", sib1.q_rx_lev_min_offset);
                        printf("\t%-30s=%12d\n", "P Max", sib1.p_max);
                        printf("\t%-30s=%12u\n", "Frequency Band", sib1.freq_band_indicator);
                        printf("\t%-30s\n", "Scheduling Info List:");
                        for(i=0; i<sib1.N_sched_info; i++)
                        {
                            switch(sib1.sched_info[i].si_periodicity)
                            {
                            case LIBLTE_RRC_SI_PERIODICITY_RF8:
                                printf("\t\t%s = %s\n", "SI Periodcity", "RF8");
                                break;
                            case LIBLTE_RRC_SI_PERIODICITY_RF16:
                                printf("\t\t%s = %s\n", "SI Periodcity", "RF16");
                                break;
                            case LIBLTE_RRC_SI_PERIODICITY_RF32:
                                printf("\t\t%s = %s\n", "SI Periodcity", "RF32");
                                break;
                            case LIBLTE_RRC_SI_PERIODICITY_RF64:
                                printf("\t\t%s = %s\n", "SI Periodcity", "RF64");
                                break;
                            case LIBLTE_RRC_SI_PERIODICITY_RF128:
                                printf("\t\t%s = %s\n", "SI Periodcity", "RF128");
                                break;
                            case LIBLTE_RRC_SI_PERIODICITY_RF256:
                                printf("\t\t%s = %s\n", "SI Periodcity", "RF256");
                                break;
                            case LIBLTE_RRC_SI_PERIODICITY_RF512:
                                printf("\t\t%s = %s\n", "SI Periodcity", "RF512");
                                break;
                            }
                            for(j=0; j<sib1.sched_info[i].N_sib_mapping_info; j++)
                            {
                                switch(sib1.sched_info[i].sib_mapping_info[j].sib_type)
                                {
                                case LIBLTE_RRC_SIB_TYPE_3:
                                    printf("\t\t\t%s = %s\n", "SIB Type", "3");
                                    break;
                                case LIBLTE_RRC_SIB_TYPE_4:
                                    printf("\t\t\t%s = %s\n", "SIB Type", "4");
                                    break;
                                case LIBLTE_RRC_SIB_TYPE_5:
                                    printf("\t\t\t%s = %s\n", "SIB Type", "5");
                                    break;
                                case LIBLTE_RRC_SIB_TYPE_6:
                                    printf("\t\t\t%s = %s\n", "SIB Type", "6");
                                    break;
                                case LIBLTE_RRC_SIB_TYPE_7:
                                    printf("\t\t\t%s = %s\n", "SIB Type", "7");
                                    break;
                                case LIBLTE_RRC_SIB_TYPE_8:
                                    printf("\t\t\t%s = %s\n", "SIB Type", "8");
                                    break;
                                case LIBLTE_RRC_SIB_TYPE_9:
                                    printf("\t\t\t%s = %s\n", "SIB Type", "9");
                                    break;
                                case LIBLTE_RRC_SIB_TYPE_10:
                                    printf("\t\t\t%s = %s\n", "SIB Type", "10");
                                    break;
                                case LIBLTE_RRC_SIB_TYPE_11:
                                    printf("\t\t\t%s = %s\n", "SIB Type", "11");
                                    break;
                                case LIBLTE_RRC_SIB_TYPE_12_v920:
                                    printf("\t\t\t%s = %s\n", "SIB Type", "12-v920");
                                    break;
                                case LIBLTE_RRC_SIB_TYPE_13_v920:
                                    printf("\t\t\t%s = %s\n", "SIB Type", "13-v920");
                                    break;
                                case LIBLTE_RRC_SIB_TYPE_SPARE_5:
                                    printf("\t\t\t%s = %s\n", "SIB Type", "SPARE5");
                                    break;
                                case LIBLTE_RRC_SIB_TYPE_SPARE_4:
                                    printf("\t\t\t%s = %s\n", "SIB Type", "SPARE4");
                                    break;
                                case LIBLTE_RRC_SIB_TYPE_SPARE_3:
                                    printf("\t\t\t%s = %s\n", "SIB Type", "SPARE3");
                                    break;
                                case LIBLTE_RRC_SIB_TYPE_SPARE_2:
                                    printf("\t\t\t%s = %s\n", "SIB Type", "SPARE2");
                                    break;
                                case LIBLTE_RRC_SIB_TYPE_SPARE_1:
                                    printf("\t\t\t%s = %s\n", "SIB Type", "SPARE1");
                                    break;
                                }
                            }
                        }
                        if(false == sib1.tdd)
                        {
                            printf("\t%-30s=%12s\n", "Duplexing Mode", "FDD");
                        }else{
                            printf("\t%-30s=%12s\n", "Duplexing Mode", "TDD");
                            switch(sib1.sf_assignment)
                            {
                            case LIBLTE_RRC_SUBFRAME_ASSIGNMENT_0:
                                printf("\t%-30s=%12s\n", "Subframe Assignment", "SA0");
                                break;
                            case LIBLTE_RRC_SUBFRAME_ASSIGNMENT_1:
                                printf("\t%-30s=%12s\n", "Subframe Assignment", "SA1");
                                break;
                            case LIBLTE_RRC_SUBFRAME_ASSIGNMENT_2:
                                printf("\t%-30s=%12s\n", "Subframe Assignment", "SA2");
                                break;
                            case LIBLTE_RRC_SUBFRAME_ASSIGNMENT_3:
                                printf("\t%-30s=%12s\n", "Subframe Assignment", "SA3");
                                break;
                            case LIBLTE_RRC_SUBFRAME_ASSIGNMENT_4:
                                printf("\t%-30s=%12s\n", "Subframe Assignment", "SA4");
                                break;
                            case LIBLTE_RRC_SUBFRAME_ASSIGNMENT_5:
                                printf("\t%-30s=%12s\n", "Subframe Assignment", "SA5");
                                break;
                            case LIBLTE_RRC_SUBFRAME_ASSIGNMENT_6:
                                printf("\t%-30s=%12s\n", "Subframe Assignment", "SA6");
                                break;
                            }
                            switch(sib1.special_sf_patterns)
                            {
                            case LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_0:
                                printf("\t%-30s=%12s\n", "Special Subframe Patterns", "SSP0");
                                break;
                            case LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_1:
                                printf("\t%-30s=%12s\n", "Special Subframe Patterns", "SSP1");
                                break;
                            case LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_2:
                                printf("\t%-30s=%12s\n", "Special Subframe Patterns", "SSP2");
                                break;
                            case LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_3:
                                printf("\t%-30s=%12s\n", "Special Subframe Patterns", "SSP3");
                                break;
                            case LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_4:
                                printf("\t%-30s=%12s\n", "Special Subframe Patterns", "SSP4");
                                break;
                            case LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_5:
                                printf("\t%-30s=%12s\n", "Special Subframe Patterns", "SSP5");
                                break;
                            case LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_6:
                                printf("\t%-30s=%12s\n", "Special Subframe Patterns", "SSP6");
                                break;
                            case LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_7:
                                printf("\t%-30s=%12s\n", "Special Subframe Patterns", "SSP7");
                                break;
                            case LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_8:
                                printf("\t%-30s=%12s\n", "Special Subframe Patterns", "SSP8");
                                break;
                            }
                        }
                        switch(sib1.si_window_length)
                        {
                        case LIBLTE_RRC_SI_WINDOW_LENGTH_MS1:
                            printf("\t%-30s=%12s\n", "SI Window Length", "MS1");
                            break;
                        case LIBLTE_RRC_SI_WINDOW_LENGTH_MS2:
                            printf("\t%-30s=%12s\n", "SI Window Length", "MS2");
                            break;
                        case LIBLTE_RRC_SI_WINDOW_LENGTH_MS5:
                            printf("\t%-30s=%12s\n", "SI Window Length", "MS5");
                            break;
                        case LIBLTE_RRC_SI_WINDOW_LENGTH_MS10:
                            printf("\t%-30s=%12s\n", "SI Window Length", "MS10");
                            break;
                        case LIBLTE_RRC_SI_WINDOW_LENGTH_MS15:
                            printf("\t%-30s=%12s\n", "SI Window Length", "MS15");
                            break;
                        case LIBLTE_RRC_SI_WINDOW_LENGTH_MS20:
                            printf("\t%-30s=%12s\n", "SI Window Length", "MS20");
                            break;
                        case LIBLTE_RRC_SI_WINDOW_LENGTH_MS40:
                            printf("\t%-30s=%12s\n", "SI Window Length", "MS40");
                            break;
                        }
                        printf("\t%-30s=%12u\n", "SI Value Tag", sib1.system_info_value_tag);
                    }else{
                        printf("Could not decode SIB1\n");
                    }
                }else{
                    printf("Could not decode MIB\n");
                }
            }else{
                printf("Could not find PSS/SSS\n");
            }
        }else{
            printf("Could not find coarse timing and frequency offset\n");
        }

        for(i=0; i<LTE_FDD_DL_FS_SAMP_BUF_SIZE; i++)
        {
            i_buf[i] = 0;
            q_buf[i] = 0;
        }
        samp_buf_idx    = 0;
    }

    // Tell runtime system how many input items we consumed
    consume_each(ninput_items);

    // Tell runtime system how many output items we produced.
    return(0);
}
