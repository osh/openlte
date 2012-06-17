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

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_dl_fg_samp_buf.h"
#include "gr_io_signature.h"

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

LTE_fdd_dl_fg_samp_buf_sptr LTE_fdd_dl_fg_make_samp_buf(uint32  _N_frames,
                                                        uint32  _N_ant,
                                                        uint32  _N_id_cell,
                                                        float   _bandwidth,
                                                        char   *_mcc,
                                                        char   *_mnc)
{
    return LTE_fdd_dl_fg_samp_buf_sptr(new LTE_fdd_dl_fg_samp_buf(_N_frames,
                                                                  _N_ant,
                                                                  _N_id_cell,
                                                                  _bandwidth,
                                                                  _mcc,
                                                                  _mnc));
}

LTE_fdd_dl_fg_samp_buf::LTE_fdd_dl_fg_samp_buf(uint32  _N_frames,
                                               uint32  _N_ant,
                                               uint32  _N_id_cell,
                                               float   _bandwidth,
                                               char   *_mcc,
                                               char   *_mnc)
    : gr_sync_block ("samp_buf",
                     gr_make_io_signature(MIN_IN,  MAX_IN,  sizeof(int8)),
                     gr_make_io_signature(MIN_OUT, MAX_OUT, sizeof(int8)))
{
    uint32 i;

    // Initialize the LTE library
    liblte_phy_init(&phy_struct);

    // Initialize the LTE parameters
    bandwidth = _bandwidth;
    sfn       = 0;
    N_frames  = _N_frames;
    N_ant     = (uint8)_N_ant;
    if(N_ant > 2)
    {
        printf("Error: Not supporting more than 2 antennas at this time, using 2\n");
        N_ant = 2;
    }else if(N_ant != 1 && N_ant != 2){
        printf("Error: Invalid N_ant (%u), using 1\n", N_ant);
        N_ant = 1;
    }
    N_id_cell            = _N_id_cell;
    if(N_id_cell > 503)
    {
        printf("Error: Invalid N_id_cell (%u), using 0\n", N_id_cell);
        N_id_cell = 0;
    }
    N_id_2               = (N_id_cell % 3);
    N_id_1               = (N_id_cell - N_id_2)/3;
    mib.phich_config.dur = LIBLTE_RRC_PHICH_DURATION_NORMAL;
    mib.phich_config.res = LIBLTE_RRC_PHICH_RESOURCE_1;
    phich_res            = 1;
    mib.sfn_div_4        = sfn/4;
    if((uint32)(roundf(bandwidth*10)) == 14)
    {
        N_rb_dl      = LIBLTE_PHY_N_RB_DL_1_4MHZ;
        FFT_pad_size = LIBLTE_PHY_FFT_PAD_SIZE_1_4MHZ;
        mib.dl_bw    = LIBLTE_RRC_DL_BANDWIDTH_6;
    }else if(bandwidth == 3){
        N_rb_dl      = LIBLTE_PHY_N_RB_DL_3MHZ;
        FFT_pad_size = LIBLTE_PHY_FFT_PAD_SIZE_3MHZ;
        mib.dl_bw    = LIBLTE_RRC_DL_BANDWIDTH_15;
    }else if(bandwidth == 5){
        N_rb_dl      = LIBLTE_PHY_N_RB_DL_5MHZ;
        FFT_pad_size = LIBLTE_PHY_FFT_PAD_SIZE_5MHZ;
        mib.dl_bw    = LIBLTE_RRC_DL_BANDWIDTH_25;
    }else if(bandwidth == 10){
        N_rb_dl      = LIBLTE_PHY_N_RB_DL_10MHZ;
        FFT_pad_size = LIBLTE_PHY_FFT_PAD_SIZE_10MHZ;
        mib.dl_bw    = LIBLTE_RRC_DL_BANDWIDTH_50;
    }else if(bandwidth == 15){
        N_rb_dl      = LIBLTE_PHY_N_RB_DL_15MHZ;
        FFT_pad_size = LIBLTE_PHY_FFT_PAD_SIZE_15MHZ;
        mib.dl_bw    = LIBLTE_RRC_DL_BANDWIDTH_75;
    }else{ // bandwidth == 20
        N_rb_dl      = LIBLTE_PHY_N_RB_DL_20MHZ;
        FFT_pad_size = LIBLTE_PHY_FFT_PAD_SIZE_20MHZ;
        mib.dl_bw    = LIBLTE_RRC_DL_BANDWIDTH_100;
        if(bandwidth != 20)
        {
            printf("Error: Invalid bandwidth (%f), using 20\n", bandwidth);
        }
    }
    sib1.N_plmn_ids        = 1;
    sib1.plmn_id[0].id.mcc = 0xF000;
    if(strlen(_mcc) < 3)
    {
        for(i=0; i<strlen(_mcc); i++)
        {
            sib1.plmn_id[0].id.mcc |= ((_mcc[i] & 0x0F) << ((strlen(_mcc)-i-1)*4));
        }
        printf("Error: MCC to short (%s), padding with zeros (%03X)\n", _mcc, sib1.plmn_id[0].id.mcc & 0xFFF);
    }else{
        for(i=0; i<3; i++)
        {
            sib1.plmn_id[0].id.mcc |= ((_mcc[i] & 0x0F) << ((3-i-1)*4));
        }
        if(strlen(_mcc) > 3)
        {
            printf("Error: MCC to long (%s), using the first three digits (%03X)\n", _mcc, sib1.plmn_id[0].id.mcc & 0xFFF);
        }
    }
    sib1.plmn_id[0].id.mnc = 0x0000;
    if(strlen(_mnc) <= 2)
    {
        for(i=0; i<strlen(_mnc); i++)
        {
            sib1.plmn_id[0].id.mnc |= ((_mnc[i] & 0x0F) << ((strlen(_mnc)-i-1)*4));
        }
        sib1.plmn_id[0].id.mnc |= 0xFF00;
        if(strlen(_mnc) < 2)
        {
            printf("Error: MNC to short (%s), padding with zeros (%02X)\n", _mnc, sib1.plmn_id[0].id.mnc & 0xFF);
        }
    }else{
        for(i=0; i<3; i++)
        {
            sib1.plmn_id[0].id.mnc |= ((_mnc[i] & 0x0F) << ((3-i-1)*4));
        }
        sib1.plmn_id[0].id.mnc |= 0xF000;
        if(strlen(_mnc) > 3)
        {
            printf("Error: MNC to long (%s), using the first three digits (%03X)\n", _mnc, sib1.plmn_id[0].id.mnc & 0xFFF);
        }
    }
    sib1.plmn_id[0].resv_for_oper         = LIBLTE_RRC_NOT_RESV_FOR_OPER;
    sib1.N_sched_info                     = 1;
    sib1.sched_info[0].N_sib_mapping_info = 0;
    sib1.sched_info[0].si_periodicity     = LIBLTE_RRC_SI_PERIODICITY_RF8;
    sib1.cell_barred                      = LIBLTE_RRC_CELL_NOT_BARRED;
    sib1.intra_freq_reselection           = LIBLTE_RRC_INTRA_FREQ_RESELECTION_ALLOWED;
    sib1.si_window_length                 = LIBLTE_RRC_SI_WINDOW_LENGTH_MS1;
    sib1.sf_assignment                    = LIBLTE_RRC_SUBFRAME_ASSIGNMENT_0;
    sib1.special_sf_patterns              = LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_0;
    sib1.cell_id                          = 1;
    sib1.csg_id                           = 2;
    sib1.tracking_area_code               = 3;
    sib1.q_rx_lev_min                     = -140;
    sib1.csg_indication                   = 0;
    sib1.q_rx_lev_min_offset              = 1;
    sib1.freq_band_indicator              = 1;
    sib1.system_info_value_tag            = 0;
    sib1.p_max                            = -30;
    sib1.tdd                              = false;
    pcfich.cfi                            = 2;

    // Allocate the sample buffer
    i_buf         = (float *)malloc(N_ant*LTE_FDD_DL_FG_SAMP_BUF_SIZE*sizeof(float));
    q_buf         = (float *)malloc(N_ant*LTE_FDD_DL_FG_SAMP_BUF_SIZE*sizeof(float));
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
    int8                       *out  = (int8 *)output_items[0];
    bool                        done = false;

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
                    liblte_rrc_pack_bcch_dlsch_msg(&sib1,
                                                   &rrc_msg);
                    pdcch.N_alloc                 = 1;
                    pdcch.alloc[0].pre_coder_type = LIBLTE_PHY_PRE_CODER_TYPE_TX_DIVERSITY;
                    pdcch.alloc[0].mod_type       = LIBLTE_PHY_MODULATION_TYPE_QPSK;
                    pdcch.alloc[0].rv_idx         = (uint32)ceilf(1.5 * ((sfn / 2) % 4)) % 4;
                    pdcch.alloc[0].N_prb          = 3; // FIXME
                    pdcch.alloc[0].prb[0]         = 0; // FIXME
                    pdcch.alloc[0].prb[1]         = 1; // FIXME
                    pdcch.alloc[0].prb[2]         = 2; // FIXME
                    pdcch.alloc[0].N_codewords    = 1;
                    pdcch.alloc[0].rnti           = LIBLTE_PHY_SI_RNTI;
                    pdcch.alloc[0].mcs            = 2;
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
                i_samp += i_buf[samp_buf_idx];
                q_samp += q_buf[samp_buf_idx];
            }
            i_samp /= N_ant;
            q_samp /= N_ant;

            out[i*2+0] = (int8)(i_samp);
            out[i*2+1] = (int8)(q_samp);
            samp_buf_idx++;
        }

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
