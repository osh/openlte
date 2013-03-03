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

    File: LTE_fdd_dl_scan_state_machine.h

    Description: Contains all the definitions for the LTE FDD DL Scanner
                 state machine block.

    Revision History
    ----------    -------------    --------------------------------------------
    02/26/2013    Ben Wojtowicz    Created file

*******************************************************************************/

#ifndef __LTE_FDD_DL_SCAN_STATE_MACHINE_H__
#define __LTE_FDD_DL_SCAN_STATE_MACHINE_H__

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_dl_scan_state_machine_api.h"
#include "LTE_fdd_dl_scan_interface.h"
#include "gr_sync_block.h"
#include "liblte_phy.h"
#include "liblte_rrc.h"

/*******************************************************************************
                              DEFINES
*******************************************************************************/

#define LTE_FDD_DL_SCAN_STATE_MACHINE_N_DECODED_CHANS_MAX 10

/*******************************************************************************
                              FORWARD DECLARATIONS
*******************************************************************************/

class LTE_fdd_dl_scan_state_machine;

/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/

typedef boost::shared_ptr<LTE_fdd_dl_scan_state_machine> LTE_fdd_dl_scan_state_machine_sptr;

typedef enum{
    LTE_FDD_DL_SCAN_STATE_MACHINE_STATE_COARSE_TIMING_SEARCH = 0,
    LTE_FDD_DL_SCAN_STATE_MACHINE_STATE_PSS_AND_FINE_TIMING_SEARCH,
    LTE_FDD_DL_SCAN_STATE_MACHINE_STATE_SSS_SEARCH,
    LTE_FDD_DL_SCAN_STATE_MACHINE_STATE_BCH_DECODE,
    LTE_FDD_DL_SCAN_STATE_MACHINE_STATE_PDSCH_DECODE_SIB1,
    LTE_FDD_DL_SCAN_STATE_MACHINE_STATE_PDSCH_DECODE_SI_GENERIC,
}LTE_FDD_DL_SCAN_STATE_MACHINE_STATE_ENUM;

/*******************************************************************************
                              CLASS DECLARATIONS
*******************************************************************************/

LTE_FDD_DL_SCAN_STATE_MACHINE_API LTE_fdd_dl_scan_state_machine_sptr LTE_fdd_dl_scan_make_state_machine ();
class LTE_FDD_DL_SCAN_STATE_MACHINE_API LTE_fdd_dl_scan_state_machine : public gr_sync_block
{
public:
    ~LTE_fdd_dl_scan_state_machine();

    int32 work(int32                      ninput_items,
               gr_vector_const_void_star &input_items,
               gr_vector_void_star       &output_items);

private:
    friend LTE_FDD_DL_SCAN_STATE_MACHINE_API LTE_fdd_dl_scan_state_machine_sptr LTE_fdd_dl_scan_make_state_machine();

    LTE_fdd_dl_scan_state_machine();

    // LTE library
    LIBLTE_PHY_STRUCT                *phy_struct;
    LIBLTE_PHY_COARSE_TIMING_STRUCT   timing_struct;
    LIBLTE_RRC_MSG_STRUCT             rrc_msg;
    LIBLTE_RRC_MIB_STRUCT             mib;
    LIBLTE_RRC_BCCH_DLSCH_MSG_STRUCT  bcch_dlsch_msg;

    // Sample buffer
    float  *i_buf;
    float  *q_buf;
    uint32  samp_buf_w_idx;
    uint32  samp_buf_r_idx;

    // Variables
    LTE_FDD_DL_SCAN_CHAN_DATA_STRUCT         chan_data;
    LTE_FDD_DL_SCAN_STATE_MACHINE_STATE_ENUM state;
    float                                    phich_res;
    uint32                                   N_sfr;
    uint32                                   N_id_1;
    uint32                                   N_id_2;
    uint32                                   corr_peak_idx;
    uint32                                   decoded_chans[LTE_FDD_DL_SCAN_STATE_MACHINE_N_DECODED_CHANS_MAX];
    uint32                                   N_decoded_chans;
    uint32                                   N_attempts;
    uint32                                   N_bch_attempts;
    uint32                                   N_samps_needed;
    uint32                                   freq_change_wait_cnt;
    bool                                     freq_change_wait_done;
    bool                                     mib_rxed;
    bool                                     sib1_rxed;
    bool                                     sib2_rxed;
    bool                                     sib3_rxed;
    bool                                     sib4_rxed;
    bool                                     sib5_rxed;
    bool                                     sib6_rxed;
    bool                                     sib7_rxed;
    bool                                     sib8_rxed;
    bool                                     send_cnf;

    // Helpers
    void init(void);
    void copy_input_to_samp_buf(const gr_complex *in, int32 ninput_items);
    void freq_shift(uint32 start_idx, uint32 num_samps, float freq_offset);
    void save_mib(LIBLTE_RRC_MIB_STRUCT *mib);
    void save_sib1(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1_STRUCT *sib1);
    void save_sib2(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_2_STRUCT *sib2);
    void save_sib3(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_3_STRUCT *sib3);
    void save_sib4(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_4_STRUCT *sib4);
    void save_sib5(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_5_STRUCT *sib5);
    void save_sib6(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_6_STRUCT *sib6);
    void save_sib7(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_7_STRUCT *sib7);
    void save_sib8(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_8_STRUCT *sib8);
    void channel_found(bool &switch_freq, int32 &done_flag);
    void channel_not_found(bool &switch_freq, int32 &done_flag);
};

#endif /* __LTE_FDD_DL_SCAN_STATE_MACHINE_H__ */
