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

    File: LTE_fdd_enb_mac.cc

    Description: Contains all the implementations for the LTE FDD eNodeB
                 medium access control layer.

    Revision History
    ----------    -------------    --------------------------------------------
    11/10/2013    Ben Wojtowicz    Created file
    01/18/2014    Ben Wojtowicz    Cached a copy of the interface class, added
                                   real-time priority to PHY->MAC message queue,
                                   added level to debug prints, and fixed
                                   subframe scheduling.

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_enb_user_mgr.h"
#include "LTE_fdd_enb_mac.h"
#include "LTE_fdd_enb_phy.h"

/*******************************************************************************
                              DEFINES
*******************************************************************************/


/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/


/*******************************************************************************
                              GLOBAL VARIABLES
*******************************************************************************/

LTE_fdd_enb_mac* LTE_fdd_enb_mac::instance = NULL;
boost::mutex     mac_instance_mutex;

/*******************************************************************************
                              CLASS IMPLEMENTATIONS
*******************************************************************************/

/*******************/
/*    Singleton    */
/*******************/
LTE_fdd_enb_mac* LTE_fdd_enb_mac::get_instance(void)
{
    boost::mutex::scoped_lock lock(mac_instance_mutex);

    if(NULL == instance)
    {
        instance = new LTE_fdd_enb_mac();
    }

    return(instance);
}
void LTE_fdd_enb_mac::cleanup(void)
{
    boost::mutex::scoped_lock lock(mac_instance_mutex);

    if(NULL != instance)
    {
        delete instance;
        instance = NULL;
    }
}

/********************************/
/*    Constructor/Destructor    */
/********************************/
LTE_fdd_enb_mac::LTE_fdd_enb_mac()
{
    interface = NULL;
    started   = false;
}
LTE_fdd_enb_mac::~LTE_fdd_enb_mac()
{
    stop();
}

/********************/
/*    Start/Stop    */
/********************/
void LTE_fdd_enb_mac::start(LTE_fdd_enb_interface *iface)
{
    boost::mutex::scoped_lock  lock(start_mutex);
    msgq_cb                    phy_cb(&msgq_cb_wrapper<LTE_fdd_enb_mac, &LTE_fdd_enb_mac::handle_phy_msg>, this);
    msgq_cb                    rlc_cb(&msgq_cb_wrapper<LTE_fdd_enb_mac, &LTE_fdd_enb_mac::handle_rlc_msg>, this);
    LTE_fdd_enb_cnfg_db       *cnfg_db = LTE_fdd_enb_cnfg_db::get_instance();
    uint32                     i;

    if(!started)
    {
        interface     = iface;
        started       = true;
        phy_comm_msgq = new LTE_fdd_enb_msgq("phy_mac_mq",
                                             phy_cb,
                                             90);
        rlc_comm_msgq = new LTE_fdd_enb_msgq("rlc_mac_mq",
                                             rlc_cb);
        mac_phy_mq    = new boost::interprocess::message_queue(boost::interprocess::open_only,
                                                               "mac_phy_mq");
        mac_rlc_mq    = new boost::interprocess::message_queue(boost::interprocess::open_only,
                                                               "mac_rlc_mq");

        // Scheduler
        // FIXME: Check this against PHY and RADIO
        cnfg_db->get_sys_info(sys_info);
        for(i=0; i<10; i++)
        {
            sched_dl_subfr[i].pdcch.N_alloc = 0;
            sched_dl_subfr[i].N_avail_prbs  = sys_info.N_rb_dl - get_n_reserved_prbs(i);
            sched_dl_subfr[i].N_sched_prbs  = 0;
            sched_dl_subfr[i].fn_combo      = i;

            sched_ul_subfr[i].allocations.N_alloc = 0;
            sched_ul_subfr[i].decodes.N_alloc     = 0;
            sched_ul_subfr[i].N_avail_prbs        = sys_info.N_rb_ul;
            sched_ul_subfr[i].N_sched_prbs        = 0;
            sched_ul_subfr[i].fn_combo            = i;
            sched_ul_subfr[i].next_prb            = 0;
        }
        sched_dl_subfr[0].fn_combo = 10;
        sched_dl_subfr[1].fn_combo = 11;
        sched_dl_subfr[2].fn_combo = 12;
        sched_cur_dl_subfn         = 3;
        sched_cur_ul_subfn         = 0;
    }
}
void LTE_fdd_enb_mac::stop(void)
{
    boost::mutex::scoped_lock lock(start_mutex);

    if(started)
    {
        started = false;
        delete phy_comm_msgq;
        delete rlc_comm_msgq;
    }
}

/***********************/
/*    Communication    */
/***********************/
void LTE_fdd_enb_mac::handle_phy_msg(LTE_FDD_ENB_MESSAGE_STRUCT *msg)
{
    if(LTE_FDD_ENB_DEST_LAYER_MAC == msg->dest_layer ||
       LTE_FDD_ENB_DEST_LAYER_ANY == msg->dest_layer)
    {
        switch(msg->type)
        {
        case LTE_FDD_ENB_MESSAGE_TYPE_READY_TO_SEND:
            handle_ready_to_send(&msg->msg.ready_to_send);
            delete msg;
            break;
        case LTE_FDD_ENB_MESSAGE_TYPE_PRACH_DECODE:
            handle_prach_decode(&msg->msg.prach_decode);
            delete msg;
            break;
        case LTE_FDD_ENB_MESSAGE_TYPE_PUCCH_DECODE:
            handle_pucch_decode(&msg->msg.pucch_decode);
            delete msg;
            break;
        case LTE_FDD_ENB_MESSAGE_TYPE_PUSCH_DECODE:
            handle_pusch_decode(&msg->msg.pusch_decode);
            delete msg;
            break;
        default:
            interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_WARNING,
                                      LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                                      __FILE__,
                                      __LINE__,
                                      "Received invalid message %s",
                                      LTE_fdd_enb_message_type_text[msg->type]);
            delete msg;
            break;
        }
    }else{
        // Forward message to RLC
        mac_rlc_mq->send(&msg, sizeof(msg), 0);
    }
}
void LTE_fdd_enb_mac::handle_rlc_msg(LTE_FDD_ENB_MESSAGE_STRUCT *msg)
{
    if(LTE_FDD_ENB_DEST_LAYER_MAC == msg->dest_layer ||
       LTE_FDD_ENB_DEST_LAYER_ANY == msg->dest_layer)
    {
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                  LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                                  __FILE__,
                                  __LINE__,
                                  "Received RLC message %s",
                                  LTE_fdd_enb_message_type_text[msg->type]);
        delete msg;
    }else{
        // Forward message to PHY
        mac_phy_mq->send(&msg, sizeof(msg), 0);
    }
}

/****************************/
/*    External Interface    */
/****************************/
void LTE_fdd_enb_mac::update_sys_info(void)
{
    LTE_fdd_enb_cnfg_db *cnfg_db = LTE_fdd_enb_cnfg_db::get_instance();

    sys_info_mutex.lock();
    cnfg_db->get_sys_info(sys_info);
    sys_info_mutex.unlock();
}

/**********************/
/*    PHY Handlers    */
/**********************/
void LTE_fdd_enb_mac::handle_ready_to_send(LTE_FDD_ENB_READY_TO_SEND_MSG_STRUCT *rts)
{
    interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                              LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                              __FILE__,
                              __LINE__,
                              "RTS %u:%u %u:%u",
                              rts->dl_fn_combo,
                              sched_dl_subfr[sched_cur_dl_subfn].fn_combo,
                              rts->ul_fn_combo,
                              sched_ul_subfr[sched_cur_ul_subfn].fn_combo);

    if(rts->dl_fn_combo != sched_dl_subfr[sched_cur_dl_subfn].fn_combo)
    {
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                  LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                                  __FILE__,
                                  __LINE__,
                                  "RTS dl_fn_combo incorrect: RTS %u but currently on %u",
                                  rts->dl_fn_combo,
                                  sched_dl_subfr[sched_cur_dl_subfn].fn_combo);
        while(rts->dl_fn_combo != sched_dl_subfr[sched_cur_dl_subfn].fn_combo)
        {
            // Advance the frame number combination
            sched_dl_subfr[sched_cur_dl_subfn].fn_combo = (sched_dl_subfr[sched_cur_dl_subfn].fn_combo + 10) % (LTE_FDD_ENB_FN_COMBO_MAX + 1);

            // Clear the subframe
            sys_info_mutex.lock();
            sched_dl_subfr[sched_cur_dl_subfn].pdcch.N_alloc = 0;
            sched_dl_subfr[sched_cur_dl_subfn].N_avail_prbs  = sys_info.N_rb_dl - get_n_reserved_prbs(sched_dl_subfr[sched_cur_dl_subfn].fn_combo);
            sched_dl_subfr[sched_cur_dl_subfn].N_sched_prbs  = 0;
            sys_info_mutex.unlock();

            // Advance the subframe number
            sched_cur_dl_subfn = (sched_cur_dl_subfn + 1) % 10;
        }
    }
    if(rts->ul_fn_combo != sched_ul_subfr[sched_cur_ul_subfn].fn_combo)
    {
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                  LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                                  __FILE__,
                                  __LINE__,
                                  "RTS ul_fn_combo incorrect: RTS %u but currently on %u",
                                  rts->ul_fn_combo,
                                  sched_ul_subfr[sched_cur_ul_subfn].fn_combo);
        while(rts->ul_fn_combo != sched_ul_subfr[sched_cur_ul_subfn].fn_combo)
        {
            // Advance the frame number combination
            sched_ul_subfr[sched_cur_ul_subfn].fn_combo = (sched_ul_subfr[sched_cur_ul_subfn].fn_combo + 10) % (LTE_FDD_ENB_FN_COMBO_MAX + 1);

            // Clear the subframe
            sched_ul_subfr[sched_cur_ul_subfn].allocations.N_alloc = 0;
            sched_ul_subfr[sched_cur_ul_subfn].decodes.N_alloc     = 0;
            sched_ul_subfr[sched_cur_ul_subfn].N_sched_prbs        = 0;
            sched_ul_subfr[sched_cur_ul_subfn].next_prb            = 0;

            // Advance the subframe number
            sched_cur_ul_subfn = (sched_cur_ul_subfn + 1) % 10;
        }
    }

    LTE_fdd_enb_msgq::send(mac_phy_mq,
                           LTE_FDD_ENB_MESSAGE_TYPE_PDSCH_SCHEDULE,
                           LTE_FDD_ENB_DEST_LAYER_PHY,
                           (LTE_FDD_ENB_MESSAGE_UNION *)&sched_dl_subfr[sched_cur_dl_subfn],
                           sizeof(LTE_FDD_ENB_PDSCH_SCHEDULE_MSG_STRUCT));
    LTE_fdd_enb_msgq::send(mac_phy_mq,
                           LTE_FDD_ENB_MESSAGE_TYPE_PUSCH_SCHEDULE,
                           LTE_FDD_ENB_DEST_LAYER_PHY,
                           (LTE_FDD_ENB_MESSAGE_UNION *)&sched_ul_subfr[sched_cur_ul_subfn],
                           sizeof(LTE_FDD_ENB_PUSCH_SCHEDULE_MSG_STRUCT));

    // Advance the frame number combination
    sched_dl_subfr[sched_cur_dl_subfn].fn_combo = (sched_dl_subfr[sched_cur_dl_subfn].fn_combo + 10) % (LTE_FDD_ENB_FN_COMBO_MAX + 1);
    sched_ul_subfr[sched_cur_ul_subfn].fn_combo = (sched_ul_subfr[sched_cur_ul_subfn].fn_combo + 10) % (LTE_FDD_ENB_FN_COMBO_MAX + 1);

    // Clear the subframes
    sys_info_mutex.lock();
    sched_dl_subfr[sched_cur_dl_subfn].pdcch.N_alloc       = 0;
    sched_dl_subfr[sched_cur_dl_subfn].N_avail_prbs        = sys_info.N_rb_dl - get_n_reserved_prbs(sched_dl_subfr[sched_cur_dl_subfn].fn_combo);
    sched_dl_subfr[sched_cur_dl_subfn].N_sched_prbs        = 0;
    sched_ul_subfr[sched_cur_ul_subfn].allocations.N_alloc = 0;
    sched_ul_subfr[sched_cur_ul_subfn].decodes.N_alloc     = 0;
    sched_ul_subfr[sched_cur_ul_subfn].N_sched_prbs        = 0;
    sched_ul_subfr[sched_cur_ul_subfn].next_prb            = 0;
    sys_info_mutex.unlock();

    // Advance the subframe numbers
    sched_cur_dl_subfn = (sched_cur_dl_subfn + 1) % 10;
    sched_cur_ul_subfn = (sched_cur_ul_subfn + 1) % 10;

    scheduler();
}
void LTE_fdd_enb_mac::handle_prach_decode(LTE_FDD_ENB_PRACH_DECODE_MSG_STRUCT *prach_decode)
{
    uint32 i;

    // Construct a random access response for each preamble
    for(i=0; i<prach_decode->num_preambles; i++)
    {
        construct_random_access_response(prach_decode->preamble[i],
                                         prach_decode->timing_adv[i],
                                         prach_decode->fn_combo);
    }
}
void LTE_fdd_enb_mac::handle_pucch_decode(LTE_FDD_ENB_PUCCH_DECODE_MSG_STRUCT *pucch_decode)
{
    // FIXME
}
void LTE_fdd_enb_mac::handle_pusch_decode(LTE_FDD_ENB_PUSCH_DECODE_MSG_STRUCT *pusch_decode)
{
    // FIXME
}

/**********************/
/*    RLC Handlers    */
/**********************/

/*************************/
/*    Message Parsers    */
/*************************/

/************************/
/*    State Machines    */
/************************/

/******************************/
/*    Message Constructors    */
/******************************/
void LTE_fdd_enb_mac::construct_random_access_response(uint8  preamble,
                                                       uint16 timing_adv,
                                                       uint32 fn_combo)
{
    LTE_fdd_enb_user_mgr         *user_mgr  = LTE_fdd_enb_user_mgr::get_instance();
    LIBLTE_MAC_RAR_STRUCT         rar;
    LIBLTE_PHY_ALLOCATION_STRUCT  dl_alloc;
    LIBLTE_PHY_ALLOCATION_STRUCT  ul_alloc;

    // Allocate a C-RNTI and a user
    if(LTE_FDD_ENB_ERROR_NONE == user_mgr->get_free_c_rnti(rar.temp_crnti) &&
       LTE_FDD_ENB_ERROR_NONE == user_mgr->add_user(rar.temp_crnti))
    {
        // Fill in the DL allocation
        dl_alloc.pre_coder_type = LIBLTE_PHY_PRE_CODER_TYPE_TX_DIVERSITY;
        dl_alloc.mod_type       = LIBLTE_PHY_MODULATION_TYPE_QPSK;
        dl_alloc.rv_idx         = 0;
        dl_alloc.N_codewords    = 1;
        dl_alloc.tx_mode        = 1; // From 36.213 v10.3.0 section 7.1
        dl_alloc.rnti           = 1 + fn_combo%10 + 0*10; // FIXME: What is f_id

        // Fill in the UL allocation
        ul_alloc.pre_coder_type = LIBLTE_PHY_PRE_CODER_TYPE_TX_DIVERSITY;
        ul_alloc.mod_type       = LIBLTE_PHY_MODULATION_TYPE_QPSK;
        ul_alloc.rv_idx         = 0; // From 36.213 v10.3.0 section 8.6.1
        ul_alloc.N_codewords    = 1;
        ul_alloc.tx_mode        = 1; // From 36.213 v10.3.0 section 7.1
        ul_alloc.rnti           = rar.temp_crnti;
        sys_info_mutex.lock();
        liblte_phy_get_tbs_mcs_and_n_prb_for_ul(56,
                                                sys_info.N_rb_ul,
                                                &ul_alloc.tbs,
                                                &ul_alloc.mcs,
                                                &ul_alloc.N_prb);
        sys_info_mutex.unlock();

        // Fill in the RAR
        rar.hdr_type       = LIBLTE_MAC_RAR_HEADER_TYPE_RAPID;
        rar.hopping_flag   = LIBLTE_MAC_RAR_HOPPING_DISABLED;
        rar.tpc_command    = LIBLTE_MAC_RAR_TPC_COMMAND_0dB;
        rar.ul_delay       = LIBLTE_MAC_RAR_UL_DELAY_DISABLED;
        rar.csi_req        = LIBLTE_MAC_RAR_CSI_REQ_DISABLED;
        rar.timing_adv_cmd = timing_adv;
        rar.mcs            = ul_alloc.mcs;
        rar.RAPID          = preamble;

        // Temporarily pack the RAR to determine the number of bits needed
        liblte_mac_pack_random_access_response_pdu(&rar,
                                                   &dl_alloc.msg);

        // Add the RAR to the scheduling queue
        if(LTE_FDD_ENB_ERROR_NONE != add_to_rar_sched_queue(fn_combo,
                                                            &dl_alloc,
                                                            &ul_alloc,
                                                            &rar))
        {
            interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                      LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                                      __FILE__,
                                      __LINE__,
                                      "Can't schedule RAR");
        }else{
            interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                      LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                                      __FILE__,
                                      __LINE__,
                                      "RAR scheduled %u",
                                      rar_sched_queue.size());
        }
    }else{
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                  LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                                  __FILE__,
                                  __LINE__,
                                  "No free C-RNTI or add_user fail");
    }
}

/*******************/
/*    Scheduler    */
/*******************/
void LTE_fdd_enb_mac::scheduler(void)
{
    boost::mutex::scoped_lock           lock(sys_info_mutex);
    LTE_fdd_enb_phy                    *phy = LTE_fdd_enb_phy::get_instance();
    LTE_FDD_ENB_RAR_SCHED_QUEUE_STRUCT *rar_sched;
    LTE_FDD_ENB_DL_SCHED_QUEUE_STRUCT  *dl_sched;
    LTE_FDD_ENB_UL_SCHED_QUEUE_STRUCT  *ul_sched;
    uint32                              N_cce;
    uint32                              resp_win_start;
    uint32                              resp_win_stop;
    uint32                              i;
    uint32                              rb_start;
    uint32                              riv;
    int32                               N_avail_dl_prbs;
    int32                               N_avail_ul_prbs;
    int32                               N_avail_dcis;
    bool                                sched_out_of_headroom;

    // Get the number of CCEs for the next subframe
    N_cce = phy->get_n_cce();

    // Schedule RAR for the next subframe
    rar_sched_queue_mutex.lock();
    sched_out_of_headroom = false;
    while(0     != rar_sched_queue.size() &&
          false == sched_out_of_headroom)
    {
        rar_sched = rar_sched_queue.front();

        // Determine when the response window starts
        resp_win_start = (rar_sched->fn_combo + 3) % (LTE_FDD_ENB_FN_COMBO_MAX + 1);

        // Determine when the response window stops
        resp_win_stop = (resp_win_start + liblte_rrc_ra_response_window_size_num[sys_info.sib2.rr_config_common_sib.rach_cnfg.ra_resp_win_size]) % (LTE_FDD_ENB_FN_COMBO_MAX + 1);

        // Take into account the SFN wrap
        // FIXME: Test this
        if(resp_win_start                              <  LIBLTE_PHY_SFN_MAX*10 &&
           sched_dl_subfr[sched_cur_dl_subfn].fn_combo >= LIBLTE_PHY_SFN_MAX*10)
        {
            resp_win_start += LIBLTE_PHY_SFN_MAX*10;
        }
        if(resp_win_stop                               <  LIBLTE_PHY_SFN_MAX*10 &&
           sched_dl_subfr[sched_cur_dl_subfn].fn_combo >= LIBLTE_PHY_SFN_MAX*10)
        {
            resp_win_stop += LIBLTE_PHY_SFN_MAX*10;
        }

        // Check to see if this fn_combo falls in the response window
        if(resp_win_start <= sched_dl_subfr[sched_cur_dl_subfn].fn_combo &&
           resp_win_stop  >= sched_dl_subfr[sched_cur_dl_subfn].fn_combo)
        {
            // Determine how many PRBs are needed for the DL allocation, if using this subframe
            liblte_phy_get_tbs_mcs_and_n_prb_for_dl(rar_sched->dl_alloc.msg.N_bits,
                                                    sched_cur_dl_subfn,
                                                    sys_info.N_rb_dl,
                                                    rar_sched->dl_alloc.rnti,
                                                    &rar_sched->dl_alloc.tbs,
                                                    &rar_sched->dl_alloc.mcs,
                                                    &rar_sched->dl_alloc.N_prb);

            // Determine how many PRBs and DCIs are available in this subframe
            N_avail_dl_prbs = sched_dl_subfr[sched_cur_dl_subfn].N_avail_prbs - sched_dl_subfr[sched_cur_dl_subfn].N_sched_prbs;
            N_avail_ul_prbs = sched_ul_subfr[(sched_cur_ul_subfn+6)%10].N_avail_prbs - sched_ul_subfr[(sched_cur_ul_subfn+6)%10].N_sched_prbs;
            N_avail_dcis    = N_cce - (sched_dl_subfr[sched_cur_dl_subfn].pdcch.N_alloc + sched_ul_subfr[sched_cur_ul_subfn].allocations.N_alloc);

            if(rar_sched->dl_alloc.N_prb <= N_avail_dl_prbs &&
               rar_sched->ul_alloc.N_prb <= N_avail_ul_prbs &&
               1                         <= N_avail_dcis)
            {
                // Determine the RB start for the UL allocation
                rb_start                                            = sched_ul_subfr[(sched_cur_ul_subfn+6)%10].next_prb;
                sched_ul_subfr[(sched_cur_ul_subfn+6)%10].next_prb += rar_sched->ul_alloc.N_prb;

                // Fill in the PRBs for the UL allocation
                for(i=0; i<rar_sched->ul_alloc.N_prb; i++)
                {
                    rar_sched->ul_alloc.prb[i] = rb_start+i;
                }

                // Determine the RIV for the UL and re-pack the RAR
                if((rar_sched->ul_alloc.N_prb-1) <= (sys_info.N_rb_ul/2))
                {
                    riv = sys_info.N_rb_ul*(rar_sched->ul_alloc.N_prb - 1) + rb_start;
                }else{
                    riv = sys_info.N_rb_ul*(sys_info.N_rb_ul - rar_sched->ul_alloc.N_prb + 1) + (sys_info.N_rb_ul - 1 - rb_start);
                }

                // Fill in the RBA for the UL allocation
                // FIXME: implement 36.213 v10.3.0 section 6.2
                rar_sched->rar.rba = riv & 0x3FF;

                // Re-pack the RAR
                liblte_mac_pack_random_access_response_pdu(&rar_sched->rar,
                                                           &rar_sched->dl_alloc.msg);

                interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                          LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                                          __FILE__,
                                          __LINE__,
                                          "RAR sent %u %u %u",
                                          resp_win_start,
                                          resp_win_stop,
                                          sched_dl_subfr[sched_cur_dl_subfn].fn_combo);

                // Schedule DL
                memcpy(&sched_dl_subfr[sched_cur_dl_subfn].pdcch.alloc[sched_dl_subfr[sched_cur_dl_subfn].pdcch.N_alloc],
                       &rar_sched->dl_alloc,
                       sizeof(LIBLTE_PHY_ALLOCATION_STRUCT));
                sched_dl_subfr[sched_cur_dl_subfn].pdcch.N_alloc++;
                // Schedule UL decode 6 subframes from now
                memcpy(&sched_ul_subfr[(sched_cur_ul_subfn+6)%10].decodes.alloc[sched_ul_subfr[(sched_cur_ul_subfn+6)%10].decodes.N_alloc],
                       &rar_sched->ul_alloc,
                       sizeof(LIBLTE_PHY_ALLOCATION_STRUCT));
                sched_ul_subfr[(sched_cur_ul_subfn+6)%10].decodes.N_alloc++;

                // Remove RAR from queue
                rar_sched_queue.pop_front();
                delete rar_sched;
            }else{
                sched_out_of_headroom = true;
            }
        }else if(resp_win_stop < sched_dl_subfr[sched_cur_dl_subfn].fn_combo){ // Check to see if the response window has passed
            // Response window has passed, remove from queue
            interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                      LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                                      __FILE__,
                                      __LINE__,
                                      "RAR outside of resp win %u %u",
                                      resp_win_stop,
                                      sched_dl_subfr[sched_cur_dl_subfn].fn_combo);
            rar_sched_queue.pop_front();
            delete rar_sched;
        }else{
            break;
        }
    }
    rar_sched_queue_mutex.unlock();

    // Schedule DL for the next subframe
    dl_sched_queue_mutex.lock();
    sched_out_of_headroom = false;
    while(0     != dl_sched_queue.size() &&
          false == sched_out_of_headroom)
    {
        dl_sched = dl_sched_queue.front();

        // Determine how many PRBs are needed for the allocation, if using this subframe
        liblte_phy_get_tbs_and_n_prb_for_dl(dl_sched->alloc.msg.N_bits,
                                            sys_info.N_rb_dl,
                                            dl_sched->alloc.mcs,
                                            &dl_sched->alloc.tbs,
                                            &dl_sched->alloc.N_prb);

        // Determine how many PRBs and DCIs are available in this subframe
        N_avail_dl_prbs = sched_dl_subfr[sched_cur_dl_subfn].N_avail_prbs - sched_dl_subfr[sched_cur_dl_subfn].N_sched_prbs;
        N_avail_dcis    = N_cce - (sched_dl_subfr[sched_cur_dl_subfn].pdcch.N_alloc + sched_ul_subfr[sched_cur_ul_subfn].allocations.N_alloc);

        if(dl_sched->alloc.N_prb <= N_avail_dl_prbs &&
           1                     <= N_avail_dcis)
        {
            // Schedule DL
            memcpy(&sched_dl_subfr[sched_cur_dl_subfn].pdcch.alloc[sched_dl_subfr[sched_cur_dl_subfn].pdcch.N_alloc],
                   &dl_sched->alloc,
                   sizeof(LIBLTE_PHY_ALLOCATION_STRUCT));
            sched_dl_subfr[sched_cur_dl_subfn].pdcch.N_alloc++;

            // Remove DL schedule from queue
            dl_sched_queue.pop_front();
            delete dl_sched;
        }else{
            sched_out_of_headroom = true;
        }
    }
    dl_sched_queue_mutex.unlock();

    // Schedule UL for the next subframe
    ul_sched_queue_mutex.lock();
    sched_out_of_headroom = false;
    while(0     != ul_sched_queue.size() &&
          false == sched_out_of_headroom)
    {
        ul_sched = ul_sched_queue.front();

        // Determine how many PRBs and DCIs are available in this subframe
        N_avail_ul_prbs = sched_ul_subfr[(sched_cur_ul_subfn+4)%10].N_avail_prbs - sched_ul_subfr[(sched_cur_ul_subfn+4)%10].N_sched_prbs;
        N_avail_dcis    = N_cce - (sched_dl_subfr[sched_cur_dl_subfn].pdcch.N_alloc + sched_ul_subfr[sched_cur_ul_subfn].allocations.N_alloc);

        if(ul_sched->alloc.N_prb <= N_avail_ul_prbs &&
           1                     <= N_avail_dcis)
        {
            // Schedule UL decode 4 subframes from now
            memcpy(&sched_ul_subfr[(sched_cur_ul_subfn+4)%10].decodes.alloc[sched_ul_subfr[(sched_cur_ul_subfn+4)%10].decodes.N_alloc],
                   &ul_sched->alloc,
                   sizeof(LIBLTE_PHY_ALLOCATION_STRUCT));
            sched_ul_subfr[(sched_cur_ul_subfn+4)%10].decodes.N_alloc++;
            // Schedule UL allocation
            memcpy(&sched_ul_subfr[sched_cur_ul_subfn].allocations.alloc[sched_ul_subfr[sched_cur_ul_subfn].allocations.N_alloc],
                   &ul_sched->alloc,
                   sizeof(LIBLTE_PHY_ALLOCATION_STRUCT));
            sched_ul_subfr[sched_cur_ul_subfn].allocations.N_alloc++;

            // Remove UL schedule from queue
            ul_sched_queue.pop_front();
            delete ul_sched;
        }else{
            sched_out_of_headroom = true;
        }
    }
    ul_sched_queue_mutex.unlock();
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_mac::add_to_rar_sched_queue(uint32                        fn_combo,
                                                               LIBLTE_PHY_ALLOCATION_STRUCT *dl_alloc,
                                                               LIBLTE_PHY_ALLOCATION_STRUCT *ul_alloc,
                                                               LIBLTE_MAC_RAR_STRUCT        *rar)
{
    LTE_FDD_ENB_RAR_SCHED_QUEUE_STRUCT *rar_sched = NULL;
    LTE_FDD_ENB_ERROR_ENUM              err       = LTE_FDD_ENB_ERROR_CANT_SCHEDULE;

    rar_sched = new LTE_FDD_ENB_RAR_SCHED_QUEUE_STRUCT;

    if(NULL != rar_sched)
    {
        rar_sched->fn_combo = fn_combo;
        memcpy(&rar_sched->dl_alloc, dl_alloc, sizeof(LIBLTE_PHY_ALLOCATION_STRUCT));
        memcpy(&rar_sched->ul_alloc, ul_alloc, sizeof(LIBLTE_PHY_ALLOCATION_STRUCT));
        memcpy(&rar_sched->rar, rar, sizeof(LIBLTE_MAC_RAR_STRUCT));

        rar_sched_queue_mutex.lock();
        rar_sched_queue.push_back(rar_sched);
        rar_sched_queue_mutex.unlock();

        err = LTE_FDD_ENB_ERROR_NONE;
    }

    return(err);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_mac::add_to_dl_sched_queue(uint32                        fn_combo,
                                                              LIBLTE_PHY_ALLOCATION_STRUCT *alloc)
{
    LTE_FDD_ENB_DL_SCHED_QUEUE_STRUCT *dl_sched = NULL;
    LTE_FDD_ENB_ERROR_ENUM             err      = LTE_FDD_ENB_ERROR_CANT_SCHEDULE;

    dl_sched = new LTE_FDD_ENB_DL_SCHED_QUEUE_STRUCT;

    if(NULL != dl_sched)
    {
        dl_sched->fn_combo = fn_combo;
        memcpy(&dl_sched->alloc, alloc, sizeof(LIBLTE_PHY_ALLOCATION_STRUCT));

        dl_sched_queue_mutex.lock();
        dl_sched_queue.push_back(dl_sched);
        dl_sched_queue_mutex.unlock();

        err = LTE_FDD_ENB_ERROR_NONE;
    }

    return(err);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_mac::add_to_ul_sched_queue(uint32                        fn_combo,
                                                              LIBLTE_PHY_ALLOCATION_STRUCT *alloc)
{
    LTE_FDD_ENB_UL_SCHED_QUEUE_STRUCT *ul_sched = NULL;
    LTE_FDD_ENB_ERROR_ENUM             err      = LTE_FDD_ENB_ERROR_CANT_SCHEDULE;

    ul_sched = new LTE_FDD_ENB_UL_SCHED_QUEUE_STRUCT;

    if(NULL != ul_sched)
    {
        ul_sched->fn_combo = fn_combo;
        memcpy(&ul_sched->alloc, alloc, sizeof(LIBLTE_PHY_ALLOCATION_STRUCT));

        ul_sched_queue_mutex.lock();
        ul_sched_queue.push_back(ul_sched);
        ul_sched_queue_mutex.unlock();

        err = LTE_FDD_ENB_ERROR_NONE;
    }

    return(err);
}

/*****************/
/*    Helpers    */
/*****************/
uint32 LTE_fdd_enb_mac::get_n_reserved_prbs(uint32 fn_combo)
{
    uint32 N_reserved_prbs = 0;
    uint32 i;

    // Reserve PRBs for the MIB
    if(0 == (fn_combo % 10))
    {
        N_reserved_prbs += 6;
    }

    // Reserve PRBs for SIB1
    if(5 == (fn_combo % 10) &&
       0 == ((fn_combo / 10) % 2))
    {
        N_reserved_prbs += sys_info.sib1_alloc.N_prb;
    }

    // Reserve PRBs for all other SIBs
    for(i=0; i<sys_info.sib1.N_sched_info; i++)
    {
        if(0                             != sys_info.sib_alloc[i].msg.N_bits &&
           (i * sys_info.si_win_len)%10  == (fn_combo % 10)                  &&
           ((i * sys_info.si_win_len)/10 == ((fn_combo / 10) % sys_info.si_periodicity_T)))
        {
            N_reserved_prbs += sys_info.sib_alloc[i].N_prb;
        }
    }

    return(N_reserved_prbs);
}
