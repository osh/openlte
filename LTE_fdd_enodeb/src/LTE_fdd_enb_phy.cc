/*******************************************************************************

    Copyright 2013-2014 Ben Wojtowicz

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

    File: LTE_fdd_enb_phy.cc

    Description: Contains all the implementations for the LTE FDD eNodeB
                 physical layer.

    Revision History
    ----------    -------------    --------------------------------------------
    11/10/2013    Ben Wojtowicz    Created file
    01/18/2014    Ben Wojtowicz    Cached a copy of the interface class, added
                                   level to debug prints, add the ability to
                                   handle late subframes, fixed a bug with
                                   transmitting SIB2 for 1.4MHz bandwidth, and
                                   added PRACH detection.

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_enb_phy.h"
#include "LTE_fdd_enb_radio.h"

/*******************************************************************************
                              DEFINES
*******************************************************************************/


/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/


/*******************************************************************************
                              GLOBAL VARIABLES
*******************************************************************************/

LTE_fdd_enb_phy* LTE_fdd_enb_phy::instance = NULL;
boost::mutex     phy_instance_mutex;

/*******************************************************************************
                              CLASS IMPLEMENTATIONS
*******************************************************************************/

/*******************/
/*    Singleton    */
/*******************/
LTE_fdd_enb_phy* LTE_fdd_enb_phy::get_instance(void)
{
    boost::mutex::scoped_lock lock(phy_instance_mutex);

    if(NULL == instance)
    {
        instance = new LTE_fdd_enb_phy();
    }

    return(instance);
}
void LTE_fdd_enb_phy::cleanup(void)
{
    boost::mutex::scoped_lock lock(phy_instance_mutex);

    if(NULL != instance)
    {
        delete instance;
        instance = NULL;
    }
}

/********************************/
/*    Constructor/Destructor    */
/********************************/
LTE_fdd_enb_phy::LTE_fdd_enb_phy()
{
    interface = NULL;
    started   = false;
}
LTE_fdd_enb_phy::~LTE_fdd_enb_phy()
{
    stop();
}

/********************/
/*    Start/Stop    */
/********************/
void LTE_fdd_enb_phy::start(LTE_fdd_enb_interface *iface)
{
    LTE_fdd_enb_radio         *radio = LTE_fdd_enb_radio::get_instance();
    msgq_cb                    cb(&msgq_cb_wrapper<LTE_fdd_enb_phy, &LTE_fdd_enb_phy::handle_mac_msg>, this);
    LIBLTE_PHY_FS_ENUM         fs;
    uint32                     i;
    uint32                     samp_rate;
    uint8                      prach_cnfg_idx;

    if(!started)
    {
        // Get the latest sys info
        update_sys_info();

        // Initialize phy
        samp_rate = radio->get_sample_rate();
        if(30720000 == samp_rate)
        {
            fs = LIBLTE_PHY_FS_30_72MHZ;
        }else if(15360000 == samp_rate){
            fs = LIBLTE_PHY_FS_15_36MHZ;
        }else if(7680000 == samp_rate){
            fs = LIBLTE_PHY_FS_7_68MHZ;
        }else if(3840000 == samp_rate){
            fs = LIBLTE_PHY_FS_3_84MHZ;
        }else if(1920000 == samp_rate){
            fs = LIBLTE_PHY_FS_1_92MHZ;
        }else{
            iface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                  LTE_FDD_ENB_DEBUG_LEVEL_PHY,
                                  __FILE__,
                                  __LINE__,
                                  "Invalid sample rate %u",
                                  samp_rate);
        }
        liblte_phy_init(&phy_struct,
                        fs,
                        sys_info.N_id_cell,
                        sys_info.N_ant,
                        sys_info.N_rb_dl,
                        sys_info.N_sc_rb,
                        liblte_rrc_phich_resource_num[sys_info.mib.phich_config.res],
                        sys_info.sib2.rr_config_common_sib.prach_cnfg.root_sequence_index,
                        sys_info.sib2.rr_config_common_sib.prach_cnfg.prach_cnfg_info.prach_config_index>>4,
                        sys_info.sib2.rr_config_common_sib.prach_cnfg.prach_cnfg_info.zero_correlation_zone_config,
                        sys_info.sib2.rr_config_common_sib.prach_cnfg.prach_cnfg_info.high_speed_flag);

        // Downlink
        // FIXME: Check this against MAC and RADIO
        for(i=0; i<10; i++)
        {
            mac_pdsch_schedule[i].fn_combo            = i;
            mac_pdsch_schedule[i].pdcch.N_alloc       = 0;
            mac_pusch_schedule[i].fn_combo            = i;
            mac_pusch_schedule[i].allocations.N_alloc = 0;
            mac_pusch_schedule[i].decodes.N_alloc     = 0;
        }
        pcfich.cfi        = 2; // FIXME: Make this dynamic every subfr
        pdcch.N_alloc     = 0;
        pdcch.N_symbs     = 2; // FIXME: Make this dynamic every subfr
        dl_subframe.num   = 0;
        dl_fn_combo       = 0;
        last_rts_fn_combo = 0;
        late_subfr        = false;

        // Uplink
        ul_fn_combo    = (LTE_FDD_ENB_FN_COMBO_MAX + 1) - 2;
        prach_cnfg_idx = sys_info.sib2.rr_config_common_sib.prach_cnfg.prach_cnfg_info.prach_config_index;
        if(prach_cnfg_idx ==  0 ||
           prach_cnfg_idx ==  1 ||
           prach_cnfg_idx ==  2 ||
           prach_cnfg_idx == 15 ||
           prach_cnfg_idx == 16 ||
           prach_cnfg_idx == 17 ||
           prach_cnfg_idx == 18 ||
           prach_cnfg_idx == 31 ||
           prach_cnfg_idx == 32 ||
           prach_cnfg_idx == 33 ||
           prach_cnfg_idx == 34 ||
           prach_cnfg_idx == 47 ||
           prach_cnfg_idx == 48 ||
           prach_cnfg_idx == 49 ||
           prach_cnfg_idx == 50 ||
           prach_cnfg_idx == 63)
        {
            prach_sfn_mod = 2;
        }else{
            prach_sfn_mod = 1;
        }
        prach_subfn_zero_allowed = true;
        switch(prach_cnfg_idx % 16)
        {
        case 0:
            // Intentional fall through
        case 3:
            prach_subfn_mod   = 10;
            prach_subfn_check = 1;
            break;
        case 1:
            // Intentional fall through
        case 4:
            prach_subfn_mod   = 10;
            prach_subfn_check = 4;
            break;
        case 2:
            // Intentional fall through
        case 5:
            prach_subfn_mod   = 10;
            prach_subfn_check = 7;
            break;
        case 6:
            prach_subfn_mod   = 5;
            prach_subfn_check = 1;
            break;
        case 7:
            prach_subfn_mod   = 5;
            prach_subfn_check = 2;
            break;
        case 8:
            prach_subfn_mod   = 5;
            prach_subfn_check = 3;
            break;
        case 9:
            prach_subfn_mod   = 3;
            prach_subfn_check = 1;
            break;
        case 10:
            prach_subfn_mod   = 3;
            prach_subfn_check = 2;
            break;
        case 11:
            prach_subfn_mod          = 3;
            prach_subfn_check        = 0;
            prach_subfn_zero_allowed = false;
            break;
        case 12:
            prach_subfn_mod   = 2;
            prach_subfn_check = 0;
            break;
        case 13:
            prach_subfn_mod   = 2;
            prach_subfn_check = 1;
            break;
        case 14:
            prach_subfn_mod   = 1;
            prach_subfn_check = 0;
            break;
        case 15:
            prach_subfn_mod   = 10;
            prach_subfn_check = 9;
            break;
        }

        // Communication
        mac_comm_msgq = new LTE_fdd_enb_msgq("mac_phy_mq",
                                             cb);
        phy_mac_mq    = new boost::interprocess::message_queue(boost::interprocess::open_only,
                                                               "phy_mac_mq");

        interface = iface;
        started   = true;
    }
}
void LTE_fdd_enb_phy::stop(void)
{
    if(started)
    {
        started = false;

        delete mac_comm_msgq;
    }
}

/****************************/
/*    External Interface    */
/****************************/
void LTE_fdd_enb_phy::update_sys_info(void)
{
    LTE_fdd_enb_cnfg_db *cnfg_db = LTE_fdd_enb_cnfg_db::get_instance();

    sys_info_mutex.lock();
    cnfg_db->get_sys_info(sys_info);
    sys_info_mutex.unlock();
}
uint32 LTE_fdd_enb_phy::get_n_cce(void)
{
    boost::mutex::scoped_lock lock(sys_info_mutex);
    uint32                    N_cce;

    liblte_phy_get_n_cce(phy_struct,
                         liblte_rrc_phich_resource_num[sys_info.mib.phich_config.res],
                         pdcch.N_symbs,
                         sys_info.N_ant,
                         &N_cce);

    return(N_cce);
}

/***********************/
/*    Communication    */
/***********************/
void LTE_fdd_enb_phy::handle_mac_msg(LTE_FDD_ENB_MESSAGE_STRUCT *msg)
{
    if(LTE_FDD_ENB_DEST_LAYER_PHY == msg->dest_layer ||
       LTE_FDD_ENB_DEST_LAYER_ANY == msg->dest_layer)
    {
        switch(msg->type)
        {
        case LTE_FDD_ENB_MESSAGE_TYPE_PDSCH_SCHEDULE:
            handle_pdsch_schedule(&msg->msg.pdsch_schedule);
            delete msg;
            break;
        case LTE_FDD_ENB_MESSAGE_TYPE_PUSCH_SCHEDULE:
            handle_pusch_schedule(&msg->msg.pusch_schedule);
            delete msg;
            break;
        default:
            interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_WARNING,
                                      LTE_FDD_ENB_DEBUG_LEVEL_PHY,
                                      __FILE__,
                                      __LINE__,
                                      "Received invalid message %s",
                                      LTE_fdd_enb_message_type_text[msg->type]);
            delete msg;
            break;
        }
    }else{
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                  LTE_FDD_ENB_DEBUG_LEVEL_PHY,
                                  __FILE__,
                                  __LINE__,
                                  "Received message for invalid layer %s",
                                  LTE_fdd_enb_dest_layer_text[msg->dest_layer]);
        delete msg;
    }
}

/*************************/
/*    Radio Interface    */
/*************************/
void LTE_fdd_enb_phy::radio_interface(LTE_FDD_ENB_RADIO_TX_BUF_STRUCT *tx_buf,
                                      LTE_FDD_ENB_RADIO_RX_BUF_STRUCT *rx_buf)
{
    if(started)
    {
        // Once started, this routine gets called every millisecond to:
        //     1) process the new uplink subframe
        //     2) generate the next downlink subframe
        process_ul(rx_buf);
        process_dl(tx_buf);
    }
}
void LTE_fdd_enb_phy::radio_interface(LTE_FDD_ENB_RADIO_TX_BUF_STRUCT *tx_buf)
{
    // This routine gets called once to generate the first downlink subframe
    process_dl(tx_buf);
}

/******************/
/*    Downlink    */
/******************/
void LTE_fdd_enb_phy::handle_pdsch_schedule(LTE_FDD_ENB_PDSCH_SCHEDULE_MSG_STRUCT *pdsch_schedule)
{
    boost::mutex::scoped_lock lock(pdsch_mutex);

    if(pdsch_schedule->fn_combo                 < dl_fn_combo &&
       (dl_fn_combo - pdsch_schedule->fn_combo) < (LTE_FDD_ENB_FN_COMBO_MAX/2))
    {
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                  LTE_FDD_ENB_DEBUG_LEVEL_PHY,
                                  __FILE__,
                                  __LINE__,
                                  "Late subframe from MAC:%u, PHY is currently on %u",
                                  pdsch_schedule->fn_combo,
                                  dl_fn_combo);

        late_subfr = true;
        if(pdsch_schedule->fn_combo == last_rts_fn_combo)
        {
            late_subfr = false;
        }
    }else{
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                  LTE_FDD_ENB_DEBUG_LEVEL_PHY,
                                  __FILE__,
                                  __LINE__,
                                  "Received PDSCH schedule from MAC %u:%u",
                                  pdsch_schedule->fn_combo,
                                  mac_pdsch_schedule[pdsch_schedule->fn_combo%10].fn_combo);

        memcpy(&mac_pdsch_schedule[pdsch_schedule->fn_combo%10], pdsch_schedule, sizeof(LTE_FDD_ENB_PDSCH_SCHEDULE_MSG_STRUCT));

        late_subfr = false;
    }
}
void LTE_fdd_enb_phy::handle_pusch_schedule(LTE_FDD_ENB_PUSCH_SCHEDULE_MSG_STRUCT *pusch_schedule)
{
    boost::mutex::scoped_lock lock(pusch_mutex);

    interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                              LTE_FDD_ENB_DEBUG_LEVEL_PHY,
                              __FILE__,
                              __LINE__,
                              "Received PUSCH schedule from MAC %u:%u",
                              pusch_schedule->fn_combo,
                              mac_pusch_schedule[pusch_schedule->fn_combo%10].fn_combo);

    if(mac_pusch_schedule[pusch_schedule->fn_combo%10].fn_combo == pusch_schedule->fn_combo)
    {
        memcpy(&mac_pusch_schedule[pusch_schedule->fn_combo%10], pusch_schedule, sizeof(LTE_FDD_ENB_PUSCH_SCHEDULE_MSG_STRUCT));
    }
}
void LTE_fdd_enb_phy::process_dl(LTE_FDD_ENB_RADIO_TX_BUF_STRUCT *tx_buf)
{
    LTE_fdd_enb_radio                    *radio = LTE_fdd_enb_radio::get_instance();
    boost::mutex::scoped_lock             lock(sys_info_mutex);
    LTE_FDD_ENB_READY_TO_SEND_MSG_STRUCT  rts;
    uint32                                p;
    uint32                                i;
    uint32                                j;
    uint32                                last_prb = 0;
    uint32                                act_noutput_items;
    uint32                                sfn   = dl_fn_combo/10;
    uint32                                subfn = dl_fn_combo%10;

    // Initialize the output to all zeros
    for(p=0; p<sys_info.N_ant; p++)
    {
        for(i=0; i<14; i++)
        {
            for(j=0; j<LIBLTE_PHY_N_RB_DL_20MHZ*LIBLTE_PHY_N_SC_RB_NORMAL_CP; j++)
            {
                dl_subframe.tx_symb_re[p][i][j] = 0;
                dl_subframe.tx_symb_im[p][i][j] = 0;
            }
        }
    }
    dl_subframe.num = subfn;

    // Handle PSS and SSS
    if(0 == dl_subframe.num ||
       5 == dl_subframe.num)
    {
        liblte_phy_map_pss(phy_struct,
                           &dl_subframe,
                           sys_info.N_id_2,
                           sys_info.N_ant);
        liblte_phy_map_sss(phy_struct,
                           &dl_subframe,
                           sys_info.N_id_1,
                           sys_info.N_id_2,
                           sys_info.N_ant);
    }

    // Handle CRS
    liblte_phy_map_crs(phy_struct,
                       &dl_subframe,
                       sys_info.N_id_cell,
                       sys_info.N_ant);

    // Handle PBCH
    if(0 == dl_subframe.num)
    {
        sys_info.mib.sfn_div_4 = sfn/4;
        liblte_rrc_pack_bcch_bch_msg(&sys_info.mib,
                                     &dl_rrc_msg);
        liblte_phy_bch_channel_encode(phy_struct,
                                      dl_rrc_msg.msg,
                                      dl_rrc_msg.N_bits,
                                      sys_info.N_id_cell,
                                      sys_info.N_ant,
                                      &dl_subframe,
                                      sfn);
    }

    // Handle SIB data
    pdcch.N_alloc = 0;
    if(5 == dl_subframe.num &&
       0 == (sfn % 2))
    {
        // SIB1
        memcpy(&pdcch.alloc[pdcch.N_alloc], &sys_info.sib1_alloc, sizeof(LIBLTE_PHY_ALLOCATION_STRUCT));
        liblte_phy_get_tbs_mcs_and_n_prb_for_dl(pdcch.alloc[pdcch.N_alloc].msg.N_bits,
                                                dl_subframe.num,
                                                sys_info.N_rb_dl,
                                                pdcch.alloc[pdcch.N_alloc].rnti,
                                                &pdcch.alloc[pdcch.N_alloc].tbs,
                                                &pdcch.alloc[pdcch.N_alloc].mcs,
                                                &pdcch.alloc[pdcch.N_alloc].N_prb);
        pdcch.alloc[pdcch.N_alloc].rv_idx = (uint32)ceilf(1.5 * ((sfn / 2) % 4)) % 4; //36.321 section 5.3.1
        pdcch.N_alloc++;
    }
    if((0 * sys_info.si_win_len)%10   <= dl_subframe.num &&
       (1 * sys_info.si_win_len)%10   >  dl_subframe.num &&
       ((0 * sys_info.si_win_len)/10) == (sfn % sys_info.si_periodicity_T))
    {
        // SIs in 1st scheduling info list entry
        memcpy(&pdcch.alloc[pdcch.N_alloc], &sys_info.sib_alloc[0], sizeof(LIBLTE_PHY_ALLOCATION_STRUCT));
        // FIXME: This was a hack to allow SIB2 decoding with 1.4MHz BW due to overlap with MIB
        if(LIBLTE_SUCCESS == liblte_phy_get_tbs_mcs_and_n_prb_for_dl(pdcch.alloc[pdcch.N_alloc].msg.N_bits,
                                                                     dl_subframe.num,
                                                                     sys_info.N_rb_dl,
                                                                     pdcch.alloc[pdcch.N_alloc].rnti,
                                                                     &pdcch.alloc[pdcch.N_alloc].tbs,
                                                                     &pdcch.alloc[pdcch.N_alloc].mcs,
                                                                     &pdcch.alloc[pdcch.N_alloc].N_prb))
        {
            pdcch.N_alloc++;
        }
    }
    for(i=1; i<sys_info.sib1.N_sched_info; i++)
    {
        if(0                              != sys_info.sib_alloc[i].msg.N_bits &&
           (i * sys_info.si_win_len)%10   == dl_subframe.num                  &&
           ((i * sys_info.si_win_len)/10) == (sfn % sys_info.si_periodicity_T))
        {
            memcpy(&pdcch.alloc[pdcch.N_alloc], &sys_info.sib_alloc[i], sizeof(LIBLTE_PHY_ALLOCATION_STRUCT));
            liblte_phy_get_tbs_mcs_and_n_prb_for_dl(pdcch.alloc[pdcch.N_alloc].msg.N_bits,
                                                    dl_subframe.num,
                                                    sys_info.N_rb_dl,
                                                    pdcch.alloc[pdcch.N_alloc].rnti,
                                                    &pdcch.alloc[pdcch.N_alloc].tbs,
                                                    &pdcch.alloc[pdcch.N_alloc].mcs,
                                                    &pdcch.alloc[pdcch.N_alloc].N_prb);
            pdcch.N_alloc++;
        }
    }

    // Handle user data
    pdsch_mutex.lock();
    if(mac_pdsch_schedule[dl_fn_combo%10].fn_combo == dl_fn_combo)
    {
        for(i=0; i<mac_pdsch_schedule[subfn].pdcch.N_alloc; i++)
        {
            memcpy(&pdcch.alloc[pdcch.N_alloc], &mac_pdsch_schedule[subfn].pdcch.alloc[i], sizeof(LIBLTE_PHY_ALLOCATION_STRUCT));
            pdcch.N_alloc++;
        }
    }else{
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                  LTE_FDD_ENB_DEBUG_LEVEL_PHY,
                                  __FILE__,
                                  __LINE__,
                                  "PDSCH fn_combo from MAC (%u) does not match PHY (%u)",
                                  mac_pdsch_schedule[subfn].fn_combo,
                                  dl_fn_combo);
    }
    pdsch_mutex.unlock();

    // Handle PDCCH and PDSCH
    for(i=0; i<pdcch.N_alloc; i++)
    {
        for(j=0; j<pdcch.alloc[i].N_prb; j++)
        {
            pdcch.alloc[i].prb[j] = last_prb++;
        }
    }
    if(last_prb > phy_struct->N_rb_dl)
    {
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                  LTE_FDD_ENB_DEBUG_LEVEL_PHY,
                                  __FILE__,
                                  __LINE__,
                                  "More PRBs allocated than are available");
    }else{
        if(0 != pdcch.N_alloc)
        {
            liblte_phy_pdcch_channel_encode(phy_struct,
                                            &pcfich,
                                            &phich,
                                            &pdcch,
                                            sys_info.N_id_cell,
                                            sys_info.N_ant,
                                            liblte_rrc_phich_resource_num[sys_info.mib.phich_config.res],
                                            sys_info.mib.phich_config.dur,
                                            &dl_subframe);
            liblte_phy_pdsch_channel_encode(phy_struct,
                                            &pdcch,
                                            sys_info.N_id_cell,
                                            sys_info.N_ant,
                                            &dl_subframe);
        }
    }

    for(p=0; p<sys_info.N_ant; p++)
    {
        liblte_phy_create_dl_subframe(phy_struct,
                                      &dl_subframe,
                                      p,
                                      &tx_buf->i_buf[p][0],
                                      &tx_buf->q_buf[p][0]);
    }
    tx_buf->fn_combo = dl_fn_combo;

    // Update counters
    dl_fn_combo = (dl_fn_combo + 1) % (LTE_FDD_ENB_FN_COMBO_MAX + 1);

    // Send READY TO SEND message to MAC
    if(!late_subfr)
    {
        rts.dl_fn_combo   = (dl_fn_combo + 2) % (LTE_FDD_ENB_FN_COMBO_MAX + 1);
        rts.ul_fn_combo   = (ul_fn_combo + 2) % (LTE_FDD_ENB_FN_COMBO_MAX + 1);
        last_rts_fn_combo = rts.dl_fn_combo;
        LTE_fdd_enb_msgq::send(phy_mac_mq,
                               LTE_FDD_ENB_MESSAGE_TYPE_READY_TO_SEND,
                               LTE_FDD_ENB_DEST_LAYER_MAC,
                               (LTE_FDD_ENB_MESSAGE_UNION *)&rts,
                               sizeof(rts));
    }

    // Send samples to radio
    radio->send(tx_buf);
}

/****************/
/*    Uplink    */
/****************/
void LTE_fdd_enb_phy::process_ul(LTE_FDD_ENB_RADIO_RX_BUF_STRUCT *rx_buf)
{
    uint32 N_skipped_subfrs = 0;
    uint32 sfn;
    uint32 subfn;

    // Check the received fn_combo
    if(rx_buf->fn_combo != ul_fn_combo)
    {
        if(rx_buf->fn_combo > ul_fn_combo)
        {
            N_skipped_subfrs = rx_buf->fn_combo - ul_fn_combo;
        }else{
            N_skipped_subfrs = (rx_buf->fn_combo + LTE_FDD_ENB_FN_COMBO_MAX + 1) - ul_fn_combo;
        }

        // Jump the DL and UL fn_combos
        dl_fn_combo = (dl_fn_combo + N_skipped_subfrs) % (LTE_FDD_ENB_FN_COMBO_MAX + 1);
        ul_fn_combo = (ul_fn_combo + N_skipped_subfrs) % (LTE_FDD_ENB_FN_COMBO_MAX + 1);
    }
    sfn   = ul_fn_combo/10;
    subfn = ul_fn_combo%10;

    // Handle PRACH
    if((sfn % prach_sfn_mod) == 0)
    {
        if((subfn % prach_subfn_mod) == prach_subfn_check)
        {
            if(subfn != 0 ||
               true  == prach_subfn_zero_allowed)
            {
                prach_decode.fn_combo = ul_fn_combo;
                liblte_phy_detect_prach(phy_struct,
                                        rx_buf->i_buf,
                                        rx_buf->q_buf,
                                        sys_info.sib2.rr_config_common_sib.prach_cnfg.prach_cnfg_info.prach_freq_offset,
                                        &prach_decode.num_preambles,
                                        prach_decode.preamble,
                                        prach_decode.timing_adv);

                LTE_fdd_enb_msgq::send(phy_mac_mq,
                                       LTE_FDD_ENB_MESSAGE_TYPE_PRACH_DECODE,
                                       LTE_FDD_ENB_DEST_LAYER_MAC,
                                       (LTE_FDD_ENB_MESSAGE_UNION *)&prach_decode,
                                       sizeof(LTE_FDD_ENB_PRACH_DECODE_MSG_STRUCT));
            }
        }
    }

    // Update counters
    ul_fn_combo = (ul_fn_combo + 1) % (LTE_FDD_ENB_FN_COMBO_MAX + 1);
}
