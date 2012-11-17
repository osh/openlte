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

    File: LTE_fdd_dl_fg_samp_buf.cc

    Description: Contains all the implementations for the LTE FDD DL File
                 Generator sample buffer block.

    Revision History
    ----------    -------------    --------------------------------------------
    06/16/2012    Ben Wojtowicz    Created file.
    08/19/2012    Ben Wojtowicz    Using the latest liblte library.
    11/10/2012    Ben Wojtowicz    Added SIB2 support and changed the parameter
                                   input method to be "interactive"

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_dl_fg_samp_buf.h"
#include "gr_io_signature.h"
#include <errno.h>

/*******************************************************************************
                              DEFINES
*******************************************************************************/

#define MAX_NOUTPUT_ITEMS 10000

/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/


/*******************************************************************************
                              GLOBAL VARIABLES
*******************************************************************************/

// minimum and maximum number of input and output streams
static const int32 MIN_IN  = 0;
static const int32 MAX_IN  = 0;
static const int32 MIN_OUT = 1;
static const int32 MAX_OUT = 1;

/*******************************************************************************
                              CLASS IMPLEMENTATIONS
*******************************************************************************/

LTE_fdd_dl_fg_samp_buf_sptr LTE_fdd_dl_fg_make_samp_buf()
{
    return LTE_fdd_dl_fg_samp_buf_sptr(new LTE_fdd_dl_fg_samp_buf());
}

LTE_fdd_dl_fg_samp_buf::LTE_fdd_dl_fg_samp_buf()
    : gr_sync_block ("samp_buf",
                     gr_make_io_signature(MIN_IN,  MAX_IN,  sizeof(int8)),
                     gr_make_io_signature(MIN_OUT, MAX_OUT, sizeof(int8)))
{
    // Initialize the LTE library
    liblte_phy_init(&phy_struct);

    // Initialize the LTE parameters
    // General
    bandwidth    = 10;
    N_rb_dl      = LIBLTE_PHY_N_RB_DL_10MHZ;
    FFT_pad_size = LIBLTE_PHY_FFT_PAD_SIZE_10MHZ;
    sfn          = 0;
    N_frames     = 30;
    N_ant        = 1;
    N_id_cell    = 0;
    N_id_2       = (N_id_cell % 3);
    N_id_1       = (N_id_cell - N_id_2)/3;
    // MIB
    mib.dl_bw            = LIBLTE_RRC_DL_BANDWIDTH_50;
    mib.phich_config.dur = LIBLTE_RRC_PHICH_DURATION_NORMAL;
    mib.phich_config.res = LIBLTE_RRC_PHICH_RESOURCE_1;
    phich_res            = 1;
    mib.sfn_div_4        = sfn/4;
    // SIB1
    sib1.N_plmn_ids                       = 1;
    sib1.plmn_id[0].id.mcc                = 0xF001;
    sib1.plmn_id[0].id.mnc                = 0xFF01;
    sib1.plmn_id[0].resv_for_oper         = LIBLTE_RRC_NOT_RESV_FOR_OPER;
    sib1.N_sched_info                     = 1;
    sib1.sched_info[0].N_sib_mapping_info = 0;
    sib1.sched_info[0].si_periodicity     = LIBLTE_RRC_SI_PERIODICITY_RF8;
    si_periodicity_T                      = 8;
    sib1.cell_barred                      = LIBLTE_RRC_CELL_NOT_BARRED;
    sib1.intra_freq_reselection           = LIBLTE_RRC_INTRA_FREQ_RESELECTION_ALLOWED;
    sib1.si_window_length                 = LIBLTE_RRC_SI_WINDOW_LENGTH_MS1;
    si_win_len                            = 1;
    sib1.sf_assignment                    = LIBLTE_RRC_SUBFRAME_ASSIGNMENT_0;
    sib1.special_sf_patterns              = LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_0;
    sib1.cell_id                          = 0;
    sib1.csg_id                           = 0;
    sib1.tracking_area_code               = 0;
    sib1.q_rx_lev_min                     = -140;
    sib1.csg_indication                   = 0;
    sib1.q_rx_lev_min_offset              = 1;
    sib1.freq_band_indicator              = 1;
    sib1.system_info_value_tag            = 0;
    sib1.p_max                            = -30;
    sib1.tdd                              = false;
    // SIB2
    sib2.ac_barring_info_present                                                      = false;
    sib2.rr_config_common_sib.rach_cnfg.num_ra_preambles                              = LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N64;
    sib2.rr_config_common_sib.rach_cnfg.preambles_group_a_cnfg.present                = false;
    sib2.rr_config_common_sib.rach_cnfg.pwr_ramping_step                              = LIBLTE_RRC_POWER_RAMPING_STEP_DB6;
    sib2.rr_config_common_sib.rach_cnfg.preamble_init_rx_target_pwr                   = LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N100;
    sib2.rr_config_common_sib.rach_cnfg.preamble_trans_max                            = LIBLTE_RRC_PREAMBLE_TRANS_MAX_N200;
    sib2.rr_config_common_sib.rach_cnfg.ra_resp_win_size                              = LIBLTE_RRC_RA_RESPONSE_WINDOW_SIZE_SF10;
    sib2.rr_config_common_sib.rach_cnfg.mac_con_res_timer                             = LIBLTE_RRC_MAC_CONTENTION_RESOLUTION_TIMER_SF64;
    sib2.rr_config_common_sib.rach_cnfg.max_harq_msg3_tx                              = 1;
    sib2.rr_config_common_sib.bcch_cnfg.modification_period_coeff                     = LIBLTE_RRC_MODIFICATION_PERIOD_COEFF_N2;
    sib2.rr_config_common_sib.pcch_cnfg.default_paging_cycle                          = LIBLTE_RRC_DEFAULT_PAGING_CYCLE_RF256;
    sib2.rr_config_common_sib.pcch_cnfg.nB                                            = LIBLTE_RRC_NB_ONE_T;
    sib2.rr_config_common_sib.prach_cnfg.root_sequence_index                          = 0;
    sib2.rr_config_common_sib.prach_cnfg.prach_cnfg_info.prach_config_index           = 0;
    sib2.rr_config_common_sib.prach_cnfg.prach_cnfg_info.high_speed_flag              = false;
    sib2.rr_config_common_sib.prach_cnfg.prach_cnfg_info.zero_correlation_zone_config = 0;
    sib2.rr_config_common_sib.prach_cnfg.prach_cnfg_info.prach_freq_offset            = 0;
    sib2.rr_config_common_sib.pdsch_cnfg.rs_power                                     = -60;
    sib2.rr_config_common_sib.pdsch_cnfg.p_b                                          = 0;
    sib2.rr_config_common_sib.pusch_cnfg.n_sb                                         = 1;
    sib2.rr_config_common_sib.pusch_cnfg.hopping_mode                                 = LIBLTE_RRC_HOPPING_MODE_INTER_SUBFRAME;
    sib2.rr_config_common_sib.pusch_cnfg.pusch_hopping_offset                         = 0;
    sib2.rr_config_common_sib.pusch_cnfg.enable_64_qam                                = true;
    sib2.rr_config_common_sib.pusch_cnfg.ul_rs.group_hopping_enabled                  = false;
    sib2.rr_config_common_sib.pusch_cnfg.ul_rs.group_assignment_pusch                 = 0;
    sib2.rr_config_common_sib.pusch_cnfg.ul_rs.sequence_hopping_enabled               = false;
    sib2.rr_config_common_sib.pusch_cnfg.ul_rs.cyclic_shift                           = 0;
    sib2.rr_config_common_sib.pucch_cnfg.delta_pucch_shift                            = LIBLTE_RRC_DELTA_PUCCH_SHIFT_DS1;
    sib2.rr_config_common_sib.pucch_cnfg.n_rb_cqi                                     = 0;
    sib2.rr_config_common_sib.pucch_cnfg.n_cs_an                                      = 0;
    sib2.rr_config_common_sib.pucch_cnfg.n1_pucch_an                                  = 0;
    sib2.rr_config_common_sib.srs_ul_cnfg.present                                     = false;
    sib2.rr_config_common_sib.ul_pwr_ctrl.p0_nominal_pusch                            = -70;
    sib2.rr_config_common_sib.ul_pwr_ctrl.alpha                                       = LIBLTE_RRC_UL_POWER_CONTROL_ALPHA_1;
    sib2.rr_config_common_sib.ul_pwr_ctrl.p0_nominal_pucch                            = -96;
    sib2.rr_config_common_sib.ul_pwr_ctrl.delta_flist_pucch.format_1                  = LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_1_0;
    sib2.rr_config_common_sib.ul_pwr_ctrl.delta_flist_pucch.format_1b                 = LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_1B_1;
    sib2.rr_config_common_sib.ul_pwr_ctrl.delta_flist_pucch.format_2                  = LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2_0;
    sib2.rr_config_common_sib.ul_pwr_ctrl.delta_flist_pucch.format_2a                 = LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2A_0;
    sib2.rr_config_common_sib.ul_pwr_ctrl.delta_flist_pucch.format_2b                 = LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2B_0;
    sib2.rr_config_common_sib.ul_pwr_ctrl.delta_preamble_msg3                         = -2;
    sib2.rr_config_common_sib.ul_cp_length                                            = LIBLTE_RRC_UL_CP_LENGTH_1;
    sib2.ue_timers_and_constants.t300                                                 = LIBLTE_RRC_T300_MS1000;
    sib2.ue_timers_and_constants.t301                                                 = LIBLTE_RRC_T301_MS1000;
    sib2.ue_timers_and_constants.t310                                                 = LIBLTE_RRC_T310_MS1000;
    sib2.ue_timers_and_constants.n310                                                 = LIBLTE_RRC_N310_N20;
    sib2.ue_timers_and_constants.t311                                                 = LIBLTE_RRC_T311_MS1000;
    sib2.ue_timers_and_constants.n311                                                 = LIBLTE_RRC_N311_N10;
    sib2.arfcn_value_eutra.present                                                    = false;
    sib2.ul_bw.present                                                                = false;
    sib2.additional_spectrum_emission                                                 = 1;
    sib2.mbsfn_subfr_cnfg_list_size                                                   = 0;
    sib2.time_alignment_timer                                                         = LIBLTE_RRC_TIME_ALIGNMENT_TIMER_SF500;
    // PCFICH
    pcfich.cfi = 2;

    // Initialize the configuration
    need_config = true;

    // Allocate the sample buffer
    i_buf         = (float *)malloc(4*LTE_FDD_DL_FG_SAMP_BUF_SIZE*sizeof(float));
    q_buf         = (float *)malloc(4*LTE_FDD_DL_FG_SAMP_BUF_SIZE*sizeof(float));
    samp_buf_idx  = 0;
    samples_ready = false;
}
LTE_fdd_dl_fg_samp_buf::~LTE_fdd_dl_fg_samp_buf()
{
    // Cleanup the LTE library
    liblte_phy_cleanup(phy_struct);

    // Free the sample buffer
    free(i_buf);
    free(q_buf);
}

int32 LTE_fdd_dl_fg_samp_buf::work(int32                      ninput_items,
                                   gr_vector_const_void_star &input_items,
                                   gr_vector_void_star       &output_items)
{
    LIBLTE_PHY_SUBFRAME_STRUCT  subframe;
    float                       i_samp;
    float                       q_samp;
    uint32                      noutput_items;
    uint32                      i;
    uint32                      j;
    uint32                      k;
    uint32                      p;
    uint32                      N_sfr;
    uint32                      line_size = LINE_MAX;
    int8                       *out       = (int8 *)output_items[0];
    char                       *line;
    bool                        done = false;

    line = (char *)malloc(line_size);
    if(need_config)
    {
        print_config();
    }
    while(need_config)
    {
        getline(&line, &line_size, stdin);
        line[strlen(line)-1] = '\0';
        change_config(line);
    }
    free(line);

    if(false == samples_ready)
    {
        // Generate frame
        if(sfn < N_frames)
        {
            for(N_sfr=0; N_sfr<10; N_sfr++)
            {
                // Initialize the output to all zeros
                for(p=0; p<N_ant; p++)
                {
                    for(j=0; j<16; j++)
                    {
                        for(k=0; k<LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP; k++)
                        {
                            subframe.tx_symb_re[p][j][k] = 0;
                            subframe.tx_symb_im[p][j][k] = 0;
                        }
                    }
                }
                subframe.num = N_sfr;

                // PSS and SSS
                if(subframe.num == 0 ||
                   subframe.num == 5)
                {
                    liblte_phy_map_pss(phy_struct,
                                       &subframe,
                                       N_id_2,
                                       N_ant,
                                       N_rb_dl,
                                       LIBLTE_PHY_N_SC_RB_NORMAL_CP);
                    liblte_phy_map_sss(phy_struct,
                                       &subframe,
                                       N_id_1,
                                       N_id_2,
                                       N_ant,
                                       N_rb_dl,
                                       LIBLTE_PHY_N_SC_RB_NORMAL_CP);
                }

                // CRS
                liblte_phy_map_crs(phy_struct,
                                   &subframe,
                                   FFT_pad_size,
                                   N_rb_dl,
                                   N_id_cell,
                                   N_ant);

                // PBCH
                if(subframe.num == 0)
                {
                    mib.sfn_div_4 = sfn/4;
                    liblte_rrc_pack_bcch_bch_msg(&mib,
                                                 &rrc_msg);
                    liblte_phy_bch_channel_encode(phy_struct,
                                                  rrc_msg.msg,
                                                  rrc_msg.N_bits,
                                                  N_id_cell,
                                                  N_ant,
                                                  N_rb_dl,
                                                  LIBLTE_PHY_N_SC_RB_NORMAL_CP,
                                                  &subframe,
                                                  sfn);
                }

                // PDCCH & PDSCH
                if(subframe.num == 5 &&
                   (sfn % 2)    == 0)
                {
                    // SIB1
                    bcch_dlsch_msg.N_sibs           = 0;
                    bcch_dlsch_msg.sibs[0].sib_type = LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1;
                    memcpy(&bcch_dlsch_msg.sibs[0].sib, &sib1, sizeof(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1_STRUCT));
                    liblte_rrc_pack_bcch_dlsch_msg(&bcch_dlsch_msg,
                                                   &rrc_msg);
                    liblte_phy_get_tbs_mcs_and_n_prb_for_si(rrc_msg.N_bits,
                                                            N_rb_dl,
                                                            &pdcch.alloc[0].tbs,
                                                            &pdcch.alloc[0].mcs,
                                                            &pdcch.alloc[0].N_prb);
                    pdcch.N_alloc                 = 1;
                    pdcch.alloc[0].pre_coder_type = LIBLTE_PHY_PRE_CODER_TYPE_TX_DIVERSITY;
                    pdcch.alloc[0].mod_type       = LIBLTE_PHY_MODULATION_TYPE_QPSK;
                    pdcch.alloc[0].rv_idx         = (uint32)ceilf(1.5 * ((sfn / 2) % 4)) % 4; //36.321 section 5.3.1
                    for(i=0; i<pdcch.alloc[0].N_prb; i++)
                    {
                        pdcch.alloc[0].prb[i] = i; // FIXME: Scheduler
                    }
                    pdcch.alloc[0].N_codewords = 1;
                    pdcch.alloc[0].rnti        = LIBLTE_PHY_SI_RNTI;
                    if(N_ant == 1)
                    {
                        pdcch.alloc[0].tx_mode = 1;
                    }else{
                        pdcch.alloc[0].tx_mode = 2;
                    }
                    liblte_phy_pdcch_channel_encode(phy_struct,
                                                    &pcfich,
                                                    &phich,
                                                    &pdcch,
                                                    N_id_cell,
                                                    N_ant,
                                                    LIBLTE_PHY_N_SC_RB_NORMAL_CP,
                                                    N_rb_dl,
                                                    phich_res,
                                                    mib.phich_config.dur,
                                                    &subframe);
                    liblte_phy_pdsch_channel_encode(phy_struct,
                                                    &pdcch,
                                                    rrc_msg.msg,
                                                    rrc_msg.N_bits,
                                                    N_id_cell,
                                                    N_ant,
                                                    LIBLTE_PHY_N_SC_RB_NORMAL_CP,
                                                    N_rb_dl,
                                                    &subframe);
                }else if(subframe.num             == (0 * si_win_len) &&
                         (sfn % si_periodicity_T) == ((0 * si_win_len)/10)){
                    // SIs in 1st scheduling info list entry
                    bcch_dlsch_msg.N_sibs           = 1;
                    bcch_dlsch_msg.sibs[0].sib_type = LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_2;
                    memcpy(&bcch_dlsch_msg.sibs[0].sib, &sib2, sizeof(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_2_STRUCT));
                    liblte_rrc_pack_bcch_dlsch_msg(&bcch_dlsch_msg,
                                                   &rrc_msg);
                    liblte_phy_get_tbs_mcs_and_n_prb_for_si(rrc_msg.N_bits,
                                                            N_rb_dl,
                                                            &pdcch.alloc[0].tbs,
                                                            &pdcch.alloc[0].mcs,
                                                            &pdcch.alloc[0].N_prb);
                    pdcch.N_alloc                 = 1;
                    pdcch.alloc[0].pre_coder_type = LIBLTE_PHY_PRE_CODER_TYPE_TX_DIVERSITY;
                    pdcch.alloc[0].mod_type       = LIBLTE_PHY_MODULATION_TYPE_QPSK;
                    pdcch.alloc[0].rv_idx         = 0; //36.321 section 5.3.1
                    for(i=0; i<pdcch.alloc[0].N_prb; i++)
                    {
                        pdcch.alloc[0].prb[i] = i; // FIXME: Scheduler
                    }
                    pdcch.alloc[0].N_codewords = 1;
                    pdcch.alloc[0].rnti        = LIBLTE_PHY_SI_RNTI;
                    if(N_ant == 1)
                    {
                        pdcch.alloc[0].tx_mode = 1;
                    }else{
                        pdcch.alloc[0].tx_mode = 2;
                    }
                    liblte_phy_pdcch_channel_encode(phy_struct,
                                                    &pcfich,
                                                    &phich,
                                                    &pdcch,
                                                    N_id_cell,
                                                    N_ant,
                                                    LIBLTE_PHY_N_SC_RB_NORMAL_CP,
                                                    N_rb_dl,
                                                    phich_res,
                                                    mib.phich_config.dur,
                                                    &subframe);
                    liblte_phy_pdsch_channel_encode(phy_struct,
                                                    &pdcch,
                                                    rrc_msg.msg,
                                                    rrc_msg.N_bits,
                                                    N_id_cell,
                                                    N_ant,
                                                    LIBLTE_PHY_N_SC_RB_NORMAL_CP,
                                                    N_rb_dl,
                                                    &subframe);
                }else{
                    pdcch.N_alloc = 0; // Nothing to schedule
                    liblte_phy_pdcch_channel_encode(phy_struct,
                                                    &pcfich,
                                                    &phich,
                                                    &pdcch,
                                                    N_id_cell,
                                                    N_ant,
                                                    LIBLTE_PHY_N_SC_RB_NORMAL_CP,
                                                    N_rb_dl,
                                                    phich_res,
                                                    mib.phich_config.dur,
                                                    &subframe);
                }

                // Construct the output
                for(p=0; p<N_ant; p++)
                {
                    liblte_phy_create_subframe(phy_struct,
                                               &subframe,
                                               FFT_pad_size,
                                               p,
                                               &i_buf[(p*LTE_FDD_DL_FG_SAMP_BUF_SIZE) + (subframe.num*30720)],
                                               &q_buf[(p*LTE_FDD_DL_FG_SAMP_BUF_SIZE) + (subframe.num*30720)]);
                }
            }
        }else{
            done = true;
        }
        sfn++;
        samples_ready = true;
    }

    if(false == done &&
       true  == samples_ready)
    {
        // Determine how many items to write
        if((LTE_FDD_DL_FG_SAMP_BUF_SIZE - samp_buf_idx) < (MAX_NOUTPUT_ITEMS / 2))
        {
            noutput_items = (LTE_FDD_DL_FG_SAMP_BUF_SIZE - samp_buf_idx)*2;
        }else{
            noutput_items = MAX_NOUTPUT_ITEMS;
        }

        // Convert the samples and write them out
        for(i=0; i<noutput_items/2; i++)
        {
            i_samp = 0;
            q_samp = 0;
            for(p=0; p<N_ant; p++)
            {
                i_samp += i_buf[(p*LTE_FDD_DL_FG_SAMP_BUF_SIZE) + samp_buf_idx];
                q_samp += q_buf[(p*LTE_FDD_DL_FG_SAMP_BUF_SIZE) + samp_buf_idx];
            }

            out[i*2+0] = (int8)(i_samp);
            out[i*2+1] = (int8)(q_samp);
            samp_buf_idx++;
        }
        usleep(1); // file_sink was corrupting output file, this fixed it

        // Check to see if we need more samples
        if(samp_buf_idx >= LTE_FDD_DL_FG_SAMP_BUF_SIZE)
        {
            samples_ready = false;
            samp_buf_idx  = 0;
        }
    }else if(true == done){
        // No more samples to write to file, so mark as done
        noutput_items = -1;
    }else{
        // Should never get here
        noutput_items = 0;
        samples_ready = false;
    }

    // Tell runtime system how many input items we consumed
    consume_each(ninput_items);

    // Tell runtime system how many output items we produced
    return(noutput_items);
}

void LTE_fdd_dl_fg_samp_buf::print_config(void)
{
    printf("***System Configuration Parameters***\n");
    printf("\tType 'help' to reprint this menu\n");
    printf("\tHit enter to finish config and generate file\n");
    printf("\tSet parameters using <param>=<value> format\n");

    // BANDWIDTH
    printf("\t%-30s = ",
           BANDWIDTH_PARAM);
    switch(N_rb_dl)
    {
    case LIBLTE_PHY_N_RB_DL_1_4MHZ:
        printf("%10s", "1.4");
        break;
    case LIBLTE_PHY_N_RB_DL_3MHZ:
        printf("%10s", "3");
        break;
    case LIBLTE_PHY_N_RB_DL_5MHZ:
        printf("%10s", "5");
        break;
    case LIBLTE_PHY_N_RB_DL_10MHZ:
        printf("%10s", "10");
        break;
    case LIBLTE_PHY_N_RB_DL_15MHZ:
        printf("%10s", "15");
        break;
    case LIBLTE_PHY_N_RB_DL_20MHZ:
        printf("%10s", "20");
        break;
    }
    printf(", values = [1.4, 3, 5, 10, 15, 20]\n");

    // FREQ_BAND
    printf("\t%-30s = %10u, bounds = [1, 25]\n",
           FREQ_BAND_PARAM,
           sib1.freq_band_indicator);

    // N_frames
    printf("\t%-30s = %10u, bounds = [1, 1000]\n",
           N_FRAMES_PARAM,
           N_frames);

    // N_ant
    printf("\t%-30s = %10u, values = [1, 2, 4]\n",
           N_ANT_PARAM,
           N_ant);

    // N_id_cell
    printf("\t%-30s = %10u, bounds = [0, 503]\n",
           N_ID_CELL_PARAM,
           N_id_cell);

    // MCC
    printf("\t%-30s = %7s%03X, bounds = [000, 999]\n",
           MCC_PARAM, "",
           (sib1.plmn_id[0].id.mcc & 0x0FFF));

    // MNC
    if((sib1.plmn_id[0].id.mnc & 0xFF00) == 0xFF00)
    {
        printf("\t%-30s = %8s%02X, bounds = [00, 999]\n",
               MNC_PARAM, "",
               (sib1.plmn_id[0].id.mnc & 0x00FF));
    }else{
        printf("\t%-30s = %7s%03X, bounds = [00, 999]\n",
               MNC_PARAM, "",
               (sib1.plmn_id[0].id.mnc & 0x0FFF));
    }

    // CELL_ID
    printf("\t%-30s = %10u, bounds = [0, 268435455]\n",
           CELL_ID_PARAM,
           sib1.cell_id);

    // TRACKING_AREA_CODE
    printf("\t%-30s = %10u, bounds = [0, 65535]\n",
           TRACKING_AREA_CODE_PARAM,
           sib1.tracking_area_code);

    // Q_RX_LEV_MIN
    printf("\t%-30s = %10d, bounds = [-140, -44]\n",
           Q_RX_LEV_MIN_PARAM,
           sib1.q_rx_lev_min);

    // P0_NOMINAL_PUSCH
    printf("\t%-30s = %10d, bounds = [-126, 24]\n",
           P0_NOMINAL_PUSCH_PARAM,
           sib2.rr_config_common_sib.ul_pwr_ctrl.p0_nominal_pusch);

    // P0_NOMINAL_PUCCH
    printf("\t%-30s = %10d, bounds = [-127, -96]\n",
           P0_NOMINAL_PUCCH_PARAM,
           sib2.rr_config_common_sib.ul_pwr_ctrl.p0_nominal_pucch);
}

void LTE_fdd_dl_fg_samp_buf::change_config(char *line)
{
    char *param;
    char *value;
    bool  err = false;

    param = strtok(line, "=");
    value = strtok(NULL, "=");

    if(param == NULL)
    {
        need_config = false;
    }else{
        if(!strcasecmp(param, "help"))
        {
            print_config();
        }else if(value != NULL){
            if(!strcasecmp(param, BANDWIDTH_PARAM))
            {
                err = set_bandwidth(value);
            }else if(!strcasecmp(param, FREQ_BAND_PARAM)){
                err = set_param(&sib1.freq_band_indicator, value, 1, 25);
            }else if(!strcasecmp(param, N_FRAMES_PARAM)){
                err = set_param(&N_frames, value, 1, 1000);
            }else if(!strcasecmp(param, N_ANT_PARAM)){
                err = set_n_ant(value);
            }else if(!strcasecmp(param, N_ID_CELL_PARAM)){
                err = set_n_id_cell(value);
            }else if(!strcasecmp(param, MCC_PARAM)){
                err = set_mcc(value);
            }else if(!strcasecmp(param, MNC_PARAM)){
                err = set_mnc(value);
            }else if(!strcasecmp(param, CELL_ID_PARAM)){
                err = set_param(&sib1.cell_id, value, 0, 268435455);
            }else if(!strcasecmp(param, TRACKING_AREA_CODE_PARAM)){
                err = set_param(&sib1.tracking_area_code, value, 0, 65535);
            }else if(!strcasecmp(param, Q_RX_LEV_MIN_PARAM)){
                err = set_param(&sib1.q_rx_lev_min, value, -140, -44);
            }else if(!strcasecmp(param, P0_NOMINAL_PUSCH_PARAM)){
                err = set_param(&sib2.rr_config_common_sib.ul_pwr_ctrl.p0_nominal_pusch, value, -126, 24);
            }else if(!strcasecmp(param, P0_NOMINAL_PUCCH_PARAM)){
                err = set_param(&sib2.rr_config_common_sib.ul_pwr_ctrl.p0_nominal_pucch, value, -127, -96);
            }else{
                printf("Invalid parameter (%s)\n", param);
            }

            if(err)
            {
                printf("Invalid value\n");
            }
        }else{
            printf("Invalid value\n");
        }
    }
}

bool LTE_fdd_dl_fg_samp_buf::set_bandwidth(char *char_value)
{
    bool err = false;

    if(!strcasecmp(char_value, "1.4"))
    {
        bandwidth    = 1.4;
        N_rb_dl      = LIBLTE_PHY_N_RB_DL_1_4MHZ;
        FFT_pad_size = LIBLTE_PHY_FFT_PAD_SIZE_1_4MHZ;
        mib.dl_bw    = LIBLTE_RRC_DL_BANDWIDTH_6;
    }else if(!strcasecmp(char_value, "3")){
        bandwidth    = 3;
        N_rb_dl      = LIBLTE_PHY_N_RB_DL_3MHZ;
        FFT_pad_size = LIBLTE_PHY_FFT_PAD_SIZE_3MHZ;
        mib.dl_bw    = LIBLTE_RRC_DL_BANDWIDTH_15;
    }else if(!strcasecmp(char_value, "5")){
        bandwidth    = 5;
        N_rb_dl      = LIBLTE_PHY_N_RB_DL_5MHZ;
        FFT_pad_size = LIBLTE_PHY_FFT_PAD_SIZE_5MHZ;
        mib.dl_bw    = LIBLTE_RRC_DL_BANDWIDTH_25;
    }else if(!strcasecmp(char_value, "10")){
        bandwidth    = 10;
        N_rb_dl      = LIBLTE_PHY_N_RB_DL_10MHZ;
        FFT_pad_size = LIBLTE_PHY_FFT_PAD_SIZE_10MHZ;
        mib.dl_bw    = LIBLTE_RRC_DL_BANDWIDTH_50;
    }else if(!strcasecmp(char_value, "15")){
        bandwidth    = 15;
        N_rb_dl      = LIBLTE_PHY_N_RB_DL_15MHZ;
        FFT_pad_size = LIBLTE_PHY_FFT_PAD_SIZE_15MHZ;
        mib.dl_bw    = LIBLTE_RRC_DL_BANDWIDTH_75;
    }else if(!strcasecmp(char_value, "20")){
        bandwidth    = 20;
        N_rb_dl      = LIBLTE_PHY_N_RB_DL_20MHZ;
        FFT_pad_size = LIBLTE_PHY_FFT_PAD_SIZE_20MHZ;
        mib.dl_bw    = LIBLTE_RRC_DL_BANDWIDTH_100;
    }else{
        err = true;
    }

    return(err);
}

bool LTE_fdd_dl_fg_samp_buf::set_n_ant(char *char_value)
{
    uint32 value;
    bool   err = true;

    if(false  == char_to_uint32(char_value, &value) &&
       (value == 1                                  ||
        value == 2))// FIXME: 4 antenna support
    {
        N_ant = value;
        err   = false;
    }

    return(err);
}

bool LTE_fdd_dl_fg_samp_buf::set_n_id_cell(char *char_value)
{
    uint32 value;
    bool   err = true;

    if(false == char_to_uint32(char_value, &value) &&
       value <= 503)
    {
        N_id_cell = value;
        N_id_2    = (N_id_cell % 3);
        N_id_1    = (N_id_cell - N_id_2)/3;
        err       = false;
    }

    return(err);
}

bool LTE_fdd_dl_fg_samp_buf::set_mcc(char *char_value)
{
    uint32 i;
    uint32 length = strlen(char_value);
    bool   err    = true;

    if(3 >= length)
    {
        sib1.plmn_id[0].id.mcc = 0xF000;
        for(i=0; i<length; i++)
        {
            sib1.plmn_id[0].id.mcc |= ((char_value[i] & 0x0F) << ((length-i-1)*4));
        }
        err = false;
    }

    return(err);
}

bool LTE_fdd_dl_fg_samp_buf::set_mnc(char *char_value)
{
    uint32 i;
    uint32 length = strlen(char_value);
    bool   err    = true;

    if(3 >= length)
    {
        sib1.plmn_id[0].id.mnc = 0x0000;
        for(i=0; i<length; i++)
        {
            sib1.plmn_id[0].id.mnc |= ((char_value[i] & 0x0F) << ((length-i-1)*4));
        }
        if(2 >= length)
        {
            sib1.plmn_id[0].id.mnc |= 0xFF00;
        }else{
            sib1.plmn_id[0].id.mnc |= 0xF000;
        }
        err = false;
    }

    return(err);
}

bool LTE_fdd_dl_fg_samp_buf::set_param(uint32 *param,
                                       char   *char_value,
                                       uint32  llimit,
                                       uint32  ulimit)
{
    uint32 value;
    bool   err = true;

    if(param != NULL                               &&
       false == char_to_uint32(char_value, &value) &&
       value >= llimit                             &&
       value <= ulimit)
    {
        *param = value;
        err    = false;
    }

    return(err);
}

bool LTE_fdd_dl_fg_samp_buf::set_param(uint16 *param,
                                       char   *char_value,
                                       uint16  llimit,
                                       uint16  ulimit)
{
    uint32 value;
    bool   err = true;

    if(param != NULL                               &&
       false == char_to_uint32(char_value, &value) &&
       value >= llimit                             &&
       value <= ulimit)
    {
        *param = (uint16)value;
        err    = false;
    }

    return(err);
}

bool LTE_fdd_dl_fg_samp_buf::set_param(uint8 *param,
                                       char  *char_value,
                                       uint8  llimit,
                                       uint8  ulimit)
{
    uint32 value;
    bool   err = true;

    if(param != NULL                               &&
       false == char_to_uint32(char_value, &value) &&
       value >= llimit                             &&
       value <= ulimit)
    {
        *param = (uint8)value;
        err    = false;
    }

    return(err);
}

bool LTE_fdd_dl_fg_samp_buf::set_param(int32 *param,
                                       char  *char_value,
                                       int32  llimit,
                                       int32  ulimit)
{
    int32 value;
    bool  err = true;

    if(param != NULL                              &&
       false == char_to_int32(char_value, &value) &&
       value >= llimit                            &&
       value <= ulimit)
    {
        *param = value;
        err    = false;
    }

    return(err);
}

bool LTE_fdd_dl_fg_samp_buf::set_param(int16 *param,
                                       char  *char_value,
                                       int16  llimit,
                                       int16  ulimit)
{
    int32 value;
    bool  err = true;

    if(param != NULL                              &&
       false == char_to_int32(char_value, &value) &&
       value >= llimit                            &&
       value <= ulimit)
    {
        *param = (int16)value;
        err    = false;
    }

    return(err);
}

bool LTE_fdd_dl_fg_samp_buf::set_param(int8 *param,
                                       char *char_value,
                                       int8  llimit,
                                       int8  ulimit)
{
    int32 value;
    bool  err = true;

    if(param != NULL                              &&
       false == char_to_int32(char_value, &value) &&
       value >= llimit                            &&
       value <= ulimit)
    {
        *param = (int8)value;
        err    = false;
    }

    return(err);
}

bool LTE_fdd_dl_fg_samp_buf::char_to_uint32(char   *char_value,
                                            uint32 *uint32_value)
{
    uint32  tmp_value;
    char   *endptr;
    bool    err = true;

    errno     = 0;
    tmp_value = strtoul(char_value, &endptr, 10);

    if(errno          == 0 &&
       strlen(endptr) == 0)
    {
        *uint32_value = tmp_value;
        err           = false;
    }

    return(err);
}

bool LTE_fdd_dl_fg_samp_buf::char_to_int32(char  *char_value,
                                           int32 *int32_value)
{
    int32  tmp_value;
    char  *endptr;
    bool   err = true;

    errno     = 0;
    tmp_value = strtol(char_value, &endptr, 10);

    if(errno          == 0 &&
       strlen(endptr) == 0)
    {
        *int32_value = tmp_value;
        err          = false;
    }

    return(err);
}
