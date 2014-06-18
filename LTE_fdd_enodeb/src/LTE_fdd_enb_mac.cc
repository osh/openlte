#line 2 "LTE_fdd_enb_mac.cc" // Make __FILE__ omit the path
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
    03/26/2014    Ben Wojtowicz    Fixed the uplink decode subframe scheduling.
    04/12/2014    Ben Wojtowicz    Using the latest LTE library.
    05/04/2014    Ben Wojtowicz    Added RLC communication handling and UL and
                                   DL CCCH message processing.
    06/15/2014    Ben Wojtowicz    Added uplink scheduling and changed fn_combo
                                   to current_tti.

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_enb_user_mgr.h"
#include "LTE_fdd_enb_timer_mgr.h"
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
    LTE_fdd_enb_msgq_cb        phy_cb(&LTE_fdd_enb_msgq_cb_wrapper<LTE_fdd_enb_mac, &LTE_fdd_enb_mac::handle_phy_msg>, this);
    LTE_fdd_enb_msgq_cb        rlc_cb(&LTE_fdd_enb_msgq_cb_wrapper<LTE_fdd_enb_mac, &LTE_fdd_enb_mac::handle_rlc_msg>, this);
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
        cnfg_db->get_sys_info(sys_info);
        for(i=0; i<10; i++)
        {
            sched_dl_subfr[i].dl_allocations.N_alloc = 0;
            sched_dl_subfr[i].ul_allocations.N_alloc = 0;
            sched_dl_subfr[i].N_avail_prbs           = sys_info.N_rb_dl - get_n_reserved_prbs(i);
            sched_dl_subfr[i].N_sched_prbs           = 0;
            sched_dl_subfr[i].current_tti            = i;

            sched_ul_subfr[i].decodes.N_alloc = 0;
            sched_ul_subfr[i].N_avail_prbs    = sys_info.N_rb_ul;
            sched_ul_subfr[i].N_sched_prbs    = 0;
            sched_ul_subfr[i].current_tti     = i;
            sched_ul_subfr[i].next_prb        = 0;
        }
        sched_dl_subfr[0].current_tti = 10;
        sched_dl_subfr[1].current_tti = 11;
        sched_dl_subfr[2].current_tti = 12;
        sched_cur_dl_subfn            = 3;
        sched_cur_ul_subfn            = 0;
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
                                      "Received invalid PHY message %s",
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
        switch(msg->type)
        {
        case LTE_FDD_ENB_MESSAGE_TYPE_MAC_SDU_READY:
            handle_sdu_ready(&msg->msg.mac_sdu_ready);
            delete msg;
            break;
        default:
            interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                      LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                                      __FILE__,
                                      __LINE__,
                                      "Received invalid RLC message %s",
                                      LTE_fdd_enb_message_type_text[msg->type]);
            delete msg;
            break;
        }
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
void LTE_fdd_enb_mac::sched_ul(LTE_fdd_enb_user *user,
                               uint32            requested_tbs)
{
    LIBLTE_PHY_ALLOCATION_STRUCT alloc;

    alloc.pre_coder_type = LIBLTE_PHY_PRE_CODER_TYPE_TX_DIVERSITY;
    alloc.mod_type       = LIBLTE_PHY_MODULATION_TYPE_QPSK;
    alloc.chan_type      = LIBLTE_PHY_CHAN_TYPE_ULSCH;
    alloc.rv_idx         = 0;
    alloc.N_codewords    = 1;
    alloc.N_layers       = 1;
    alloc.tx_mode        = 1;
    alloc.rnti           = user->get_c_rnti();
    alloc.tpc            = LIBLTE_PHY_TPC_COMMAND_DCI_0_3_4_DB_NEG_1;
    alloc.ndi            = false; // FIXME
    sys_info_mutex.lock();
    liblte_phy_get_tbs_mcs_and_n_prb_for_ul(requested_tbs,
                                            sys_info.N_rb_ul,
                                            &alloc.tbs,
                                            &alloc.mcs,
                                            &alloc.N_prb);
    sys_info_mutex.unlock();

    // Add the allocation to the scheduling queue
    if(LTE_FDD_ENB_ERROR_NONE != add_to_ul_sched_queue((sched_ul_subfr[sched_cur_ul_subfn].current_tti + 4) % (LTE_FDD_ENB_CURRENT_TTI_MAX + 1),
                                                       &alloc))
    {
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                  LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                                  __FILE__,
                                  __LINE__,
                                  "Can't schedule UL");
    }else{
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                  LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                                  __FILE__,
                                  __LINE__,
                                  "UL scheduled for RNTI=%u, UL_QUEUE_SIZE=%u",
                                  alloc.rnti,
                                  ul_sched_queue.size());
    }
}

/**********************/
/*    PHY Handlers    */
/**********************/
void LTE_fdd_enb_mac::handle_ready_to_send(LTE_FDD_ENB_READY_TO_SEND_MSG_STRUCT *rts)
{
    LTE_fdd_enb_timer_mgr *timer_mgr = LTE_fdd_enb_timer_mgr::get_instance();

    // Send tick to timer manager
    // FIXME: Send this through msgq
    timer_mgr->handle_tick();

    interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                              LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                              __FILE__,
                              __LINE__,
                              "RTS DL_CURRENT_TTI:PHY=%u,MAC=%u UL_CURRENT_TTI:PHY=%u,MAC=%u",
                              rts->dl_current_tti,
                              sched_dl_subfr[sched_cur_dl_subfn].current_tti,
                              rts->ul_current_tti,
                              sched_ul_subfr[sched_cur_ul_subfn].current_tti);

    if(rts->dl_current_tti != sched_dl_subfr[sched_cur_dl_subfn].current_tti)
    {
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                  LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                                  __FILE__,
                                  __LINE__,
                                  "RTS dl_current_tti incorrect: RTS %u but currently on %u",
                                  rts->dl_current_tti,
                                  sched_dl_subfr[sched_cur_dl_subfn].current_tti);
        while(rts->dl_current_tti != sched_dl_subfr[sched_cur_dl_subfn].current_tti)
        {
            // Advance the frame number combination
            sched_dl_subfr[sched_cur_dl_subfn].current_tti = (sched_dl_subfr[sched_cur_dl_subfn].current_tti + 10) % (LTE_FDD_ENB_CURRENT_TTI_MAX + 1);

            // Clear the subframe
            sys_info_mutex.lock();
            sched_dl_subfr[sched_cur_dl_subfn].dl_allocations.N_alloc = 0;
            sched_dl_subfr[sched_cur_dl_subfn].ul_allocations.N_alloc = 0;
            sched_dl_subfr[sched_cur_dl_subfn].N_avail_prbs           = sys_info.N_rb_dl - get_n_reserved_prbs(sched_dl_subfr[sched_cur_dl_subfn].current_tti);
            sched_dl_subfr[sched_cur_dl_subfn].N_sched_prbs           = 0;
            sys_info_mutex.unlock();

            // Advance the subframe number
            sched_cur_dl_subfn = (sched_cur_dl_subfn + 1) % 10;
        }
    }
    if(rts->ul_current_tti != sched_ul_subfr[sched_cur_ul_subfn].current_tti)
    {
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                  LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                                  __FILE__,
                                  __LINE__,
                                  "RTS ul_current_tti incorrect: RTS %u but currently on %u",
                                  rts->ul_current_tti,
                                  sched_ul_subfr[sched_cur_ul_subfn].current_tti);
        while(rts->ul_current_tti != sched_ul_subfr[sched_cur_ul_subfn].current_tti)
        {
            // Advance the frame number combination
            sched_ul_subfr[sched_cur_ul_subfn].current_tti = (sched_ul_subfr[sched_cur_ul_subfn].current_tti + 10) % (LTE_FDD_ENB_CURRENT_TTI_MAX + 1);

            // Clear the subframe
            sched_ul_subfr[sched_cur_ul_subfn].decodes.N_alloc = 0;
            sched_ul_subfr[sched_cur_ul_subfn].N_sched_prbs    = 0;
            sched_ul_subfr[sched_cur_ul_subfn].next_prb        = 0;

            // Advance the subframe number
            sched_cur_ul_subfn = (sched_cur_ul_subfn + 1) % 10;
        }
    }

    LTE_fdd_enb_msgq::send(mac_phy_mq,
                           LTE_FDD_ENB_MESSAGE_TYPE_DL_SCHEDULE,
                           LTE_FDD_ENB_DEST_LAYER_PHY,
                           (LTE_FDD_ENB_MESSAGE_UNION *)&sched_dl_subfr[sched_cur_dl_subfn],
                           sizeof(LTE_FDD_ENB_DL_SCHEDULE_MSG_STRUCT));
    LTE_fdd_enb_msgq::send(mac_phy_mq,
                           LTE_FDD_ENB_MESSAGE_TYPE_UL_SCHEDULE,
                           LTE_FDD_ENB_DEST_LAYER_PHY,
                           (LTE_FDD_ENB_MESSAGE_UNION *)&sched_ul_subfr[sched_cur_ul_subfn],
                           sizeof(LTE_FDD_ENB_UL_SCHEDULE_MSG_STRUCT));

    // Advance the frame number combination
    sched_dl_subfr[sched_cur_dl_subfn].current_tti = (sched_dl_subfr[sched_cur_dl_subfn].current_tti + 10) % (LTE_FDD_ENB_CURRENT_TTI_MAX + 1);
    sched_ul_subfr[sched_cur_ul_subfn].current_tti = (sched_ul_subfr[sched_cur_ul_subfn].current_tti + 10) % (LTE_FDD_ENB_CURRENT_TTI_MAX + 1);

    // Clear the subframes
    sys_info_mutex.lock();
    sched_dl_subfr[sched_cur_dl_subfn].dl_allocations.N_alloc = 0;
    sched_dl_subfr[sched_cur_dl_subfn].ul_allocations.N_alloc = 0;
    sched_dl_subfr[sched_cur_dl_subfn].N_avail_prbs           = sys_info.N_rb_dl - get_n_reserved_prbs(sched_dl_subfr[sched_cur_dl_subfn].current_tti);
    sched_dl_subfr[sched_cur_dl_subfn].N_sched_prbs           = 0;
    sched_ul_subfr[sched_cur_ul_subfn].decodes.N_alloc        = 0;
    sched_ul_subfr[sched_cur_ul_subfn].N_sched_prbs           = 0;
    sched_ul_subfr[sched_cur_ul_subfn].next_prb               = 0;
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
                                         prach_decode->current_tti);
    }
}
void LTE_fdd_enb_mac::handle_pucch_decode(LTE_FDD_ENB_PUCCH_DECODE_MSG_STRUCT *pucch_decode)
{
    interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                              LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                              __FILE__,
                              __LINE__,
                              "Not handling PUCCH_DECODE");
}
void LTE_fdd_enb_mac::handle_pusch_decode(LTE_FDD_ENB_PUSCH_DECODE_MSG_STRUCT *pusch_decode)
{
    LTE_fdd_enb_user_mgr *user_mgr = LTE_fdd_enb_user_mgr::get_instance();
    LTE_fdd_enb_user     *user     = NULL;
    uint32                i;

    // Find the user
    if(LTE_FDD_ENB_ERROR_NONE == user_mgr->find_user(pusch_decode->rnti, &user))
    {
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                  LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                                  __FILE__,
                                  __LINE__,
                                  &pusch_decode->msg,
                                  "PUSCH decode for RNTI=%u CURRENT_TTI=%u",
                                  pusch_decode->rnti,
                                  pusch_decode->current_tti);
        interface->send_pcap_msg(LTE_FDD_ENB_PCAP_DIRECTION_UL,
                                 pusch_decode->rnti,
                                 pusch_decode->current_tti,
                                 pusch_decode->msg.msg,
                                 pusch_decode->msg.N_bits);

        // Set the correct channel type
        user->pusch_mac_pdu.chan_type = LIBLTE_MAC_CHAN_TYPE_ULSCH;

        // Unpack MAC PDU
        liblte_mac_unpack_mac_pdu(&pusch_decode->msg,
                                  &user->pusch_mac_pdu);

        for(i=0; i<user->pusch_mac_pdu.N_subheaders; i++)
        {
            if(LIBLTE_MAC_ULSCH_CCCH_LCID == user->pusch_mac_pdu.subheader[i].lcid)
            {
                handle_ulsch_ccch_sdu(user, user->pusch_mac_pdu.subheader[i].lcid, &user->pusch_mac_pdu.subheader[i].payload.sdu);
            }else if(LIBLTE_MAC_ULSCH_DCCH_LCID_BEGIN <= user->pusch_mac_pdu.subheader[i].lcid &&
                     LIBLTE_MAC_ULSCH_DCCH_LCID_END   >= user->pusch_mac_pdu.subheader[i].lcid){
                handle_ulsch_dcch_sdu(user, user->pusch_mac_pdu.subheader[i].lcid, &user->pusch_mac_pdu.subheader[i].payload.sdu);
            }else if(LIBLTE_MAC_ULSCH_EXT_POWER_HEADROOM_REPORT_LCID == user->pusch_mac_pdu.subheader[i].lcid){
                handle_ulsch_ext_power_headroom_report(user, &user->pusch_mac_pdu.subheader[i].payload.ext_power_headroom);
            }else if(LIBLTE_MAC_ULSCH_POWER_HEADROOM_REPORT_LCID == user->pusch_mac_pdu.subheader[i].lcid){
                handle_ulsch_power_headroom_report(user, &user->pusch_mac_pdu.subheader[i].payload.power_headroom);
            }else if(LIBLTE_MAC_ULSCH_C_RNTI_LCID == user->pusch_mac_pdu.subheader[i].lcid){
                handle_ulsch_c_rnti(&user, &user->pusch_mac_pdu.subheader[i].payload.c_rnti);
            }else if(LIBLTE_MAC_ULSCH_TRUNCATED_BSR_LCID == user->pusch_mac_pdu.subheader[i].lcid){
                handle_ulsch_truncated_bsr(user, &user->pusch_mac_pdu.subheader[i].payload.truncated_bsr);
            }else if(LIBLTE_MAC_ULSCH_SHORT_BSR_LCID == user->pusch_mac_pdu.subheader[i].lcid){
                handle_ulsch_short_bsr(user, &user->pusch_mac_pdu.subheader[i].payload.short_bsr);
            }else if(LIBLTE_MAC_ULSCH_LONG_BSR_LCID == user->pusch_mac_pdu.subheader[i].lcid){
                handle_ulsch_long_bsr(user, &user->pusch_mac_pdu.subheader[i].payload.long_bsr);
            }
        }
    }else{
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                  LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                                  __FILE__,
                                  __LINE__,
                                  &pusch_decode->msg,
                                  "PUSCH decode for invalid RNTI (%u)",
                                  pusch_decode->rnti);
    }
}

/******************************/
/*    RLC Message Handlers    */
/******************************/
void LTE_fdd_enb_mac::handle_sdu_ready(LTE_FDD_ENB_MAC_SDU_READY_MSG_STRUCT *sdu_ready)
{
    LTE_fdd_enb_user             *user;
    LIBLTE_MAC_PDU_STRUCT         mac_pdu;
    LIBLTE_PHY_ALLOCATION_STRUCT  alloc;
    LIBLTE_BIT_MSG_STRUCT        *sdu;
    uint32                        current_tti;

    if(LTE_FDD_ENB_ERROR_NONE == sdu_ready->rb->get_next_mac_sdu(&sdu))
    {
        user = sdu_ready->user;

        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                  LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                                  __FILE__,
                                  __LINE__,
                                  sdu,
                                  "Received SDU for RNTI=%u and RB=%s",
                                  user->get_c_rnti(),
                                  LTE_fdd_enb_rb_text[sdu_ready->rb->get_rb_id()]);

        // Fill in the allocation
        alloc.pre_coder_type = LIBLTE_PHY_PRE_CODER_TYPE_TX_DIVERSITY;
        alloc.mod_type       = LIBLTE_PHY_MODULATION_TYPE_QPSK;
        alloc.chan_type      = LIBLTE_PHY_CHAN_TYPE_DLSCH;
        alloc.rv_idx         = 0;
        alloc.N_codewords    = 1;
        sys_info_mutex.lock();
        if(1 == sys_info.N_ant)
        {
            alloc.tx_mode = 1;
        }else{
            alloc.tx_mode = 2;
        }
        sys_info_mutex.unlock();
        alloc.rnti = user->get_c_rnti();
        alloc.mcs  = 0;
        alloc.tpc  = LIBLTE_PHY_TPC_COMMAND_DCI_1_1A_1B_1D_2_3_DB_ZERO;
        alloc.ndi  = false;

        // Pack the SDU
        mac_pdu.chan_type         = LIBLTE_MAC_CHAN_TYPE_DLSCH;
        mac_pdu.N_subheaders      = 1;
        mac_pdu.subheader[0].lcid = sdu_ready->rb->get_rb_id();
        memcpy(&mac_pdu.subheader[0].payload.sdu, sdu, sizeof(LIBLTE_BIT_MSG_STRUCT));

        // Determine the current_tti
        current_tti = (sched_dl_subfr[sched_cur_dl_subfn].current_tti + 4) % (LTE_FDD_ENB_CURRENT_TTI_MAX + 1);

        // Add the SDU to the scheduling queue
        if(LTE_FDD_ENB_ERROR_NONE != add_to_dl_sched_queue(current_tti,
                                                           &mac_pdu,
                                                           &alloc))
        {
            interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                      LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                                      __FILE__,
                                      __LINE__,
                                      "Can't schedule SDU");
        }else{
            interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                      LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                                      __FILE__,
                                      __LINE__,
                                      &alloc.msg,
                                      "SDU scheduled for RNTI=%u, DL_QUEUE_SIZE=%u",
                                      alloc.rnti,
                                      dl_sched_queue.size());
        }

        // Delete the SDU
        sdu_ready->rb->delete_next_mac_sdu();
    }else{
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                  LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                                  __FILE__,
                                  __LINE__,
                                  "Received sdu_ready message with no SDU queued");
    }
}

/**************************/
/*    MAC PDU Handlers    */
/**************************/
void LTE_fdd_enb_mac::handle_ulsch_ccch_sdu(LTE_fdd_enb_user      *user,
                                            uint32                 lcid,
                                            LIBLTE_BIT_MSG_STRUCT *sdu)
{
    LTE_fdd_enb_rb                       *rb = NULL;
    LTE_FDD_ENB_RLC_PDU_READY_MSG_STRUCT  rlc_pdu_ready;
    LIBLTE_MAC_PDU_STRUCT                 mac_pdu;
    LIBLTE_PHY_ALLOCATION_STRUCT          alloc;
    uint32                                current_tti;
    uint32                                i;

    if(LIBLTE_MAC_ULSCH_CCCH_LCID == lcid)
    {
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                  LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                                  __FILE__,
                                  __LINE__,
                                  sdu,
                                  "Handling ULSCH SDU for RNTI=%u, LCID=%u",
                                  user->get_c_rnti(),
                                  lcid);

        // Get SRB0
        user->get_srb0(&rb);

        // Fill in the contention resolution allocation
        alloc.pre_coder_type = LIBLTE_PHY_PRE_CODER_TYPE_TX_DIVERSITY;
        alloc.mod_type       = LIBLTE_PHY_MODULATION_TYPE_QPSK;
        alloc.chan_type      = LIBLTE_PHY_CHAN_TYPE_DLSCH;
        alloc.rv_idx         = 0;
        alloc.N_codewords    = 1;
        sys_info_mutex.lock();
        if(1 == sys_info.N_ant)
        {
            alloc.tx_mode = 1;
        }else{
            alloc.tx_mode = 2;
        }
        sys_info_mutex.unlock();
        alloc.rnti = user->get_c_rnti();
        alloc.mcs  = 0;
        alloc.tpc  = LIBLTE_PHY_TPC_COMMAND_DCI_1_1A_1B_1D_2_3_DB_ZERO;
        alloc.ndi  = false;

        // Pack the contention resolution PDU
        mac_pdu.chan_type                             = LIBLTE_MAC_CHAN_TYPE_DLSCH;
        mac_pdu.N_subheaders                          = 1;
        mac_pdu.subheader[0].lcid                     = LIBLTE_MAC_DLSCH_UE_CONTENTION_RESOLUTION_ID_LCID;
        mac_pdu.subheader[0].payload.ue_con_res_id.id = 0;
        for(i=0; i<sdu->N_bits; i++)
        {
            mac_pdu.subheader[0].payload.ue_con_res_id.id <<= 1;
            mac_pdu.subheader[0].payload.ue_con_res_id.id  |= sdu->msg[i];
        }

        // Determine the contention resolution current_tti
        current_tti = (sched_dl_subfr[sched_cur_dl_subfn].current_tti + 4) % (LTE_FDD_ENB_CURRENT_TTI_MAX + 1);

        // Add the contention resolution PDU to the scheduling queue
        if(LTE_FDD_ENB_ERROR_NONE != add_to_dl_sched_queue(current_tti,
                                                           &mac_pdu,
                                                           &alloc))
        {
            interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                      LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                                      __FILE__,
                                      __LINE__,
                                      "Can't schedule PDU");
        }else{
            interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                      LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                                      __FILE__,
                                      __LINE__,
                                      &alloc.msg,
                                      "PDU scheduled for RNTI=%u, DL_QUEUE_SIZE=%u",
                                      alloc.rnti,
                                      dl_sched_queue.size());
        }

        // Queue the SDU for RLC
        rb->queue_rlc_pdu(sdu);

        // Signal RLC
        rlc_pdu_ready.user = user;
        rlc_pdu_ready.rb   = rb;
        LTE_fdd_enb_msgq::send(mac_rlc_mq,
                               LTE_FDD_ENB_MESSAGE_TYPE_RLC_PDU_READY,
                               LTE_FDD_ENB_DEST_LAYER_RLC,
                               (LTE_FDD_ENB_MESSAGE_UNION *)&rlc_pdu_ready,
                               sizeof(LTE_FDD_ENB_RLC_PDU_READY_MSG_STRUCT));
    }else{
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                  LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                                  __FILE__,
                                  __LINE__,
                                  sdu,
                                  "Not handling ULSCH SDU for RNTI=%u, LCID=%u",
                                  user->get_c_rnti(),
                                  lcid);
    }
}
void LTE_fdd_enb_mac::handle_ulsch_dcch_sdu(LTE_fdd_enb_user      *user,
                                            uint32                 lcid,
                                            LIBLTE_BIT_MSG_STRUCT *sdu)
{
    LTE_fdd_enb_rb                       *rb = NULL;
    LTE_FDD_ENB_RLC_PDU_READY_MSG_STRUCT  rlc_pdu_ready;

    if(LIBLTE_MAC_ULSCH_DCCH_LCID_BEGIN <= lcid &&
       LIBLTE_MAC_ULSCH_DCCH_LCID_END   >= lcid)
    {
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                  LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                                  __FILE__,
                                  __LINE__,
                                  sdu,
                                  "Handling ULSCH DCCH_SDU for RNTI=%u, LCID=%u",
                                  user->get_c_rnti(),
                                  lcid);

        // Get RB
        if(LTE_FDD_ENB_RB_SRB1 == lcid)
        {
            user->get_srb1(&rb);
        }else if(LTE_FDD_ENB_RB_SRB2 == lcid){
            user->get_srb2(&rb);
        }else{
            interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                                      LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                                      __FILE__,
                                      __LINE__,
                                      "Not handling ULSCH DCCH_SDU for RNTI=%u, LCID=%u",
                                      user->get_c_rnti(),
                                      lcid);
        }

        // Queue the SDU for RLC
        if(NULL != rb)
        {
            rb->queue_rlc_pdu(sdu);

            // Signal RLC
            rlc_pdu_ready.user = user;
            rlc_pdu_ready.rb   = rb;
            LTE_fdd_enb_msgq::send(mac_rlc_mq,
                                   LTE_FDD_ENB_MESSAGE_TYPE_RLC_PDU_READY,
                                   LTE_FDD_ENB_DEST_LAYER_RLC,
                                   (LTE_FDD_ENB_MESSAGE_UNION *)&rlc_pdu_ready,
                                   sizeof(LTE_FDD_ENB_RLC_PDU_READY_MSG_STRUCT));
        }
    }else{
        interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                  LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                                  __FILE__,
                                  __LINE__,
                                  sdu,
                                  "Not handling ULSCH SDU for RNTI=%u, LCID=%u",
                                  user->get_c_rnti(),
                                  lcid);
    }
}
void LTE_fdd_enb_mac::handle_ulsch_ext_power_headroom_report(LTE_fdd_enb_user                        *user,
                                                             LIBLTE_MAC_EXT_POWER_HEADROOM_CE_STRUCT *ext_power_headroom)
{
    interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                              LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                              __FILE__,
                              __LINE__,
                              "Not handling ULSCH EXTENDED_POWER_HEADROOM_REPORT");
}
void LTE_fdd_enb_mac::handle_ulsch_power_headroom_report(LTE_fdd_enb_user                    *user,
                                                         LIBLTE_MAC_POWER_HEADROOM_CE_STRUCT *power_headroom)
{
    interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                              LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                              __FILE__,
                              __LINE__,
                              "Not handling ULSCH POWER_HEADROOM_REPORT");
}
void LTE_fdd_enb_mac::handle_ulsch_c_rnti(LTE_fdd_enb_user            **user,
                                          LIBLTE_MAC_C_RNTI_CE_STRUCT  *c_rnti)
{
    LTE_fdd_enb_user_mgr *user_mgr   = LTE_fdd_enb_user_mgr::get_instance();
    uint16                old_c_rnti = (*user)->get_c_rnti();

    interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                              LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                              __FILE__,
                              __LINE__,
                              "Received C_RNTI=%u for C_RNTI=%u",
                              c_rnti->c_rnti,
                              (*user)->get_c_rnti());

    if(c_rnti->c_rnti != (*user)->get_c_rnti())
    {
        user_mgr->find_user(c_rnti->c_rnti, user);
        user_mgr->del_user(old_c_rnti);
    }
}
void LTE_fdd_enb_mac::handle_ulsch_truncated_bsr(LTE_fdd_enb_user                   *user,
                                                 LIBLTE_MAC_TRUNCATED_BSR_CE_STRUCT *truncated_bsr)
{
    interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                              LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                              __FILE__,
                              __LINE__,
                              "Not handling ULSCH TRUNCATED_BSR");
}
void LTE_fdd_enb_mac::handle_ulsch_short_bsr(LTE_fdd_enb_user               *user,
                                             LIBLTE_MAC_SHORT_BSR_CE_STRUCT *short_bsr)
{
    interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                              LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                              __FILE__,
                              __LINE__,
                              "Not handling ULSCH SHORT_BSR");
}
void LTE_fdd_enb_mac::handle_ulsch_long_bsr(LTE_fdd_enb_user              *user,
                                            LIBLTE_MAC_LONG_BSR_CE_STRUCT *long_bsr)
{
    interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                              LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                              __FILE__,
                              __LINE__,
                              "Not handling ULSCH LONG_BSR %u %u %u %u",
                              long_bsr->buffer_size_0,
                              long_bsr->buffer_size_1,
                              long_bsr->buffer_size_2,
                              long_bsr->buffer_size_3);
}

/***************************/
/*    Data Constructors    */
/***************************/
void LTE_fdd_enb_mac::construct_random_access_response(uint8  preamble,
                                                       uint16 timing_adv,
                                                       uint32 current_tti)
{
    LTE_fdd_enb_user_mgr         *user_mgr  = LTE_fdd_enb_user_mgr::get_instance();
    LIBLTE_MAC_RAR_STRUCT         rar;
    LIBLTE_PHY_ALLOCATION_STRUCT  dl_alloc;
    LIBLTE_PHY_ALLOCATION_STRUCT  ul_alloc;

    // Allocate a C-RNTI and a user
    if(LTE_FDD_ENB_ERROR_NONE == user_mgr->get_free_c_rnti(&rar.temp_c_rnti) &&
       LTE_FDD_ENB_ERROR_NONE == user_mgr->add_user(rar.temp_c_rnti))
    {
        // Fill in the DL allocation
        dl_alloc.pre_coder_type = LIBLTE_PHY_PRE_CODER_TYPE_TX_DIVERSITY;
        dl_alloc.mod_type       = LIBLTE_PHY_MODULATION_TYPE_QPSK;
        dl_alloc.chan_type      = LIBLTE_PHY_CHAN_TYPE_DLSCH;
        dl_alloc.rv_idx         = 0;
        dl_alloc.N_codewords    = 1;
        dl_alloc.tx_mode        = 1; // From 36.213 v10.3.0 section 7.1
        dl_alloc.rnti           = 1 + current_tti%10 + 0*10; // FIXME: What is f_id

        // Fill in the UL allocation
        ul_alloc.pre_coder_type = LIBLTE_PHY_PRE_CODER_TYPE_TX_DIVERSITY;
        ul_alloc.mod_type       = LIBLTE_PHY_MODULATION_TYPE_QPSK;
        ul_alloc.chan_type      = LIBLTE_PHY_CHAN_TYPE_ULSCH;
        ul_alloc.rv_idx         = 0; // From 36.213 v10.3.0 section 8.6.1
        ul_alloc.N_codewords    = 1;
        ul_alloc.N_layers       = 1;
        ul_alloc.tx_mode        = 1; // From 36.213 v10.3.0 section 7.1
        ul_alloc.rnti           = rar.temp_c_rnti;
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
        if(LTE_FDD_ENB_ERROR_NONE != add_to_rar_sched_queue(current_tti,
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
    uint32                              N_pad;
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
        resp_win_start = (rar_sched->current_tti + 3) % (LTE_FDD_ENB_CURRENT_TTI_MAX + 1);

        // Determine when the response window stops
        resp_win_stop = (resp_win_start + liblte_rrc_ra_response_window_size_num[sys_info.sib2.rr_config_common_sib.rach_cnfg.ra_resp_win_size]) % (LTE_FDD_ENB_CURRENT_TTI_MAX + 1);

        // Take into account the SFN wrap
        // FIXME: Test this
        if(resp_win_start                                 <  LIBLTE_PHY_SFN_MAX*10 &&
           sched_dl_subfr[sched_cur_dl_subfn].current_tti >= LIBLTE_PHY_SFN_MAX*10)
        {
            resp_win_start += LIBLTE_PHY_SFN_MAX*10;
        }
        if(resp_win_stop                                  <  LIBLTE_PHY_SFN_MAX*10 &&
           sched_dl_subfr[sched_cur_dl_subfn].current_tti >= LIBLTE_PHY_SFN_MAX*10)
        {
            resp_win_stop += LIBLTE_PHY_SFN_MAX*10;
        }

        // Check to see if this current_tti falls in the response window
        if(resp_win_start <= sched_dl_subfr[sched_cur_dl_subfn].current_tti &&
           resp_win_stop  >= sched_dl_subfr[sched_cur_dl_subfn].current_tti)
        {
            // Determine how many PRBs are needed for the DL allocation, if using this subframe
            interface->send_pcap_msg(LTE_FDD_ENB_PCAP_DIRECTION_DL,
                                     rar_sched->dl_alloc.rnti,
                                     sched_dl_subfr[sched_cur_dl_subfn].current_tti,
                                     rar_sched->dl_alloc.msg.msg,
                                     rar_sched->dl_alloc.msg.N_bits);
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
            N_avail_dcis    = N_cce - (sched_dl_subfr[sched_cur_dl_subfn].dl_allocations.N_alloc + sched_dl_subfr[sched_cur_dl_subfn].ul_allocations.N_alloc);

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
                    rar_sched->ul_alloc.prb[0][i] = rb_start+i;
                    rar_sched->ul_alloc.prb[1][i] = rb_start+i;
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
                                          &rar_sched->dl_alloc.msg,
                                          "RAR sent %u %u %u %u",
                                          resp_win_start,
                                          resp_win_stop,
                                          sched_dl_subfr[sched_cur_dl_subfn].current_tti,
                                          sched_ul_subfr[(sched_cur_dl_subfn+6)%10].current_tti);

                // Schedule DL
                memcpy(&sched_dl_subfr[sched_cur_dl_subfn].dl_allocations.alloc[sched_dl_subfr[sched_cur_dl_subfn].dl_allocations.N_alloc],
                       &rar_sched->dl_alloc,
                       sizeof(LIBLTE_PHY_ALLOCATION_STRUCT));
                sched_dl_subfr[sched_cur_dl_subfn].dl_allocations.N_alloc++;
                // Schedule UL decode 6 subframes from now
                memcpy(&sched_ul_subfr[(sched_cur_dl_subfn+6)%10].decodes.alloc[sched_ul_subfr[(sched_cur_dl_subfn+6)%10].decodes.N_alloc],
                       &rar_sched->ul_alloc,
                       sizeof(LIBLTE_PHY_ALLOCATION_STRUCT));
                sched_ul_subfr[(sched_cur_dl_subfn+6)%10].decodes.N_alloc++;

                // Remove RAR from queue
                rar_sched_queue.pop_front();
                delete rar_sched;
            }else{
                sched_out_of_headroom = true;
            }
        }else if(resp_win_stop < sched_dl_subfr[sched_cur_dl_subfn].current_tti){ // Check to see if the response window has passed
            // Response window has passed, remove from queue
            interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                      LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                                      __FILE__,
                                      __LINE__,
                                      "RAR outside of resp win %u %u",
                                      resp_win_stop,
                                      sched_dl_subfr[sched_cur_dl_subfn].current_tti);
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

        if(dl_sched->current_tti == sched_dl_subfr[sched_cur_dl_subfn].current_tti)
        {
            // Pack the message and determine TBS
            liblte_mac_pack_mac_pdu(&dl_sched->mac_pdu,
                                    &dl_sched->alloc.msg);
            liblte_phy_get_tbs_and_n_prb_for_dl(dl_sched->alloc.msg.N_bits,
                                                sys_info.N_rb_dl,
                                                dl_sched->alloc.mcs,
                                                &dl_sched->alloc.tbs,
                                                &dl_sched->alloc.N_prb);

            // Pad and repack if needed
            if(dl_sched->alloc.tbs > dl_sched->alloc.msg.N_bits)
            {
                N_pad = (dl_sched->alloc.tbs - dl_sched->alloc.msg.N_bits)/8;

                if(1 == N_pad)
                {
                    for(i=0; i<dl_sched->mac_pdu.N_subheaders; i++)
                    {
                        memcpy(&dl_sched->mac_pdu.subheader[dl_sched->mac_pdu.N_subheaders-i], &dl_sched->mac_pdu.subheader[dl_sched->mac_pdu.N_subheaders-i-1], sizeof(LIBLTE_MAC_PDU_SUBHEADER_STRUCT));
                    }
                    dl_sched->mac_pdu.subheader[0].lcid = LIBLTE_MAC_DLSCH_PADDING_LCID;
                    dl_sched->mac_pdu.N_subheaders++;
                }else if(2 == N_pad){
                    for(i=0; i<dl_sched->mac_pdu.N_subheaders; i++)
                    {
                        memcpy(&dl_sched->mac_pdu.subheader[dl_sched->mac_pdu.N_subheaders-i+1], &dl_sched->mac_pdu.subheader[dl_sched->mac_pdu.N_subheaders-i-1], sizeof(LIBLTE_MAC_PDU_SUBHEADER_STRUCT));
                    }
                    dl_sched->mac_pdu.subheader[0].lcid  = LIBLTE_MAC_DLSCH_PADDING_LCID;
                    dl_sched->mac_pdu.subheader[1].lcid  = LIBLTE_MAC_DLSCH_PADDING_LCID;
                    dl_sched->mac_pdu.N_subheaders      += 2;
                }else{
                    dl_sched->mac_pdu.subheader[dl_sched->mac_pdu.N_subheaders].lcid = LIBLTE_MAC_DLSCH_PADDING_LCID;
                    dl_sched->mac_pdu.N_subheaders++;
                }

                liblte_mac_pack_mac_pdu(&dl_sched->mac_pdu,
                                        &dl_sched->alloc.msg);
            }

            // Send a PCAP message
            interface->send_pcap_msg(LTE_FDD_ENB_PCAP_DIRECTION_DL,
                                     dl_sched->alloc.rnti,
                                     sched_dl_subfr[sched_cur_dl_subfn].current_tti,
                                     dl_sched->alloc.msg.msg,
                                     dl_sched->alloc.tbs);

            // Determine how many PRBs and DCIs are available in this subframe
            N_avail_dl_prbs = sched_dl_subfr[sched_cur_dl_subfn].N_avail_prbs - sched_dl_subfr[sched_cur_dl_subfn].N_sched_prbs;
            N_avail_dcis    = N_cce - (sched_dl_subfr[sched_cur_dl_subfn].dl_allocations.N_alloc + sched_dl_subfr[sched_cur_dl_subfn].ul_allocations.N_alloc);

            if(dl_sched->alloc.N_prb <= N_avail_dl_prbs &&
               1                     <= N_avail_dcis)
            {
                interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                          LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                                          __FILE__,
                                          __LINE__,
                                          &dl_sched->alloc.msg,
                                          "DL allocation sent for RNTI=%u CURRENT_TTI=%u",
                                          dl_sched->alloc.rnti,
                                          sched_dl_subfr[sched_cur_dl_subfn].current_tti);

                // Schedule DL
                memcpy(&sched_dl_subfr[sched_cur_dl_subfn].dl_allocations.alloc[sched_dl_subfr[sched_cur_dl_subfn].dl_allocations.N_alloc],
                       &dl_sched->alloc,
                       sizeof(LIBLTE_PHY_ALLOCATION_STRUCT));
                sched_dl_subfr[sched_cur_dl_subfn].dl_allocations.N_alloc++;

                // Remove DL schedule from queue
                dl_sched_queue.pop_front();
                delete dl_sched;
            }
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
        N_avail_ul_prbs = sched_ul_subfr[(sched_cur_dl_subfn+4)%10].N_avail_prbs - sched_ul_subfr[(sched_cur_dl_subfn+4)%10].N_sched_prbs;
        N_avail_dcis    = N_cce - (sched_dl_subfr[sched_cur_dl_subfn].dl_allocations.N_alloc + sched_dl_subfr[sched_cur_dl_subfn].ul_allocations.N_alloc);

        if(ul_sched->alloc.N_prb <= N_avail_ul_prbs &&
           1                     <= N_avail_dcis)
        {
            // Determine the RB start
            rb_start                                            = sched_ul_subfr[(sched_cur_dl_subfn+4)%10].next_prb;
            sched_ul_subfr[(sched_cur_dl_subfn+4)%10].next_prb += ul_sched->alloc.N_prb;

            // Fill in the PRBs
            for(i=0; i<ul_sched->alloc.N_prb; i++)
            {
                ul_sched->alloc.prb[0][i] = rb_start+i;
                ul_sched->alloc.prb[1][i] = rb_start+i;
            }

            interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                                      LTE_FDD_ENB_DEBUG_LEVEL_MAC,
                                      __FILE__,
                                      __LINE__,
                                      "UL allocation sent for RNTI=%u CURRENT_TTI=%u",
                                      ul_sched->alloc.rnti,
                                      sched_ul_subfr[(sched_cur_dl_subfn+4)%10].current_tti);

            // Schedule UL decode 4 subframes from now
            memcpy(&sched_ul_subfr[(sched_cur_dl_subfn+4)%10].decodes.alloc[sched_ul_subfr[(sched_cur_dl_subfn+4)%10].decodes.N_alloc],
                   &ul_sched->alloc,
                   sizeof(LIBLTE_PHY_ALLOCATION_STRUCT));
            sched_ul_subfr[(sched_cur_dl_subfn+4)%10].decodes.N_alloc++;
            // Schedule UL allocation
            memcpy(&sched_dl_subfr[sched_cur_dl_subfn].ul_allocations.alloc[sched_dl_subfr[sched_cur_dl_subfn].ul_allocations.N_alloc],
                   &ul_sched->alloc,
                   sizeof(LIBLTE_PHY_ALLOCATION_STRUCT));
            sched_dl_subfr[sched_cur_dl_subfn].ul_allocations.N_alloc++;

            // Remove UL schedule from queue
            ul_sched_queue.pop_front();
            delete ul_sched;
        }else{
            sched_out_of_headroom = true;
        }
    }
    ul_sched_queue_mutex.unlock();
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_mac::add_to_rar_sched_queue(uint32                        current_tti,
                                                               LIBLTE_PHY_ALLOCATION_STRUCT *dl_alloc,
                                                               LIBLTE_PHY_ALLOCATION_STRUCT *ul_alloc,
                                                               LIBLTE_MAC_RAR_STRUCT        *rar)
{
    LTE_FDD_ENB_RAR_SCHED_QUEUE_STRUCT *rar_sched = NULL;
    LTE_FDD_ENB_ERROR_ENUM              err       = LTE_FDD_ENB_ERROR_CANT_SCHEDULE;

    rar_sched = new LTE_FDD_ENB_RAR_SCHED_QUEUE_STRUCT;

    if(NULL != rar_sched)
    {
        rar_sched->current_tti = current_tti;
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
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_mac::add_to_dl_sched_queue(uint32                        current_tti,
                                                              LIBLTE_MAC_PDU_STRUCT        *mac_pdu,
                                                              LIBLTE_PHY_ALLOCATION_STRUCT *alloc)
{
    std::list<LTE_FDD_ENB_DL_SCHED_QUEUE_STRUCT *>::iterator  iter;
    LTE_FDD_ENB_DL_SCHED_QUEUE_STRUCT                        *dl_sched = NULL;
    LTE_FDD_ENB_ERROR_ENUM                                    err      = LTE_FDD_ENB_ERROR_CANT_SCHEDULE;

    dl_sched = new LTE_FDD_ENB_DL_SCHED_QUEUE_STRUCT;

    if(NULL != dl_sched)
    {
        dl_sched->current_tti = current_tti;
        memcpy(&dl_sched->mac_pdu, mac_pdu, sizeof(LIBLTE_MAC_PDU_STRUCT));
        memcpy(&dl_sched->alloc, alloc, sizeof(LIBLTE_PHY_ALLOCATION_STRUCT));

        dl_sched_queue_mutex.lock();
        for(iter=dl_sched_queue.begin(); iter!=dl_sched_queue.end(); iter++)
        {
            if((*iter)->alloc.rnti  == alloc->rnti &&
               (*iter)->current_tti == dl_sched->current_tti)
            {
                dl_sched->current_tti = (dl_sched->current_tti + 1) % (LTE_FDD_ENB_CURRENT_TTI_MAX + 1);
            }
        }
        dl_sched_queue.push_back(dl_sched);
        dl_sched_queue_mutex.unlock();

        err = LTE_FDD_ENB_ERROR_NONE;
    }

    return(err);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_mac::add_to_ul_sched_queue(uint32                        current_tti,
                                                              LIBLTE_PHY_ALLOCATION_STRUCT *alloc)
{
    std::list<LTE_FDD_ENB_UL_SCHED_QUEUE_STRUCT *>::iterator  iter;
    LTE_FDD_ENB_UL_SCHED_QUEUE_STRUCT                        *ul_sched = NULL;
    LTE_FDD_ENB_ERROR_ENUM                                    err      = LTE_FDD_ENB_ERROR_CANT_SCHEDULE;

    ul_sched = new LTE_FDD_ENB_UL_SCHED_QUEUE_STRUCT;

    if(NULL != ul_sched)
    {
        ul_sched->current_tti = current_tti;
        memcpy(&ul_sched->alloc, alloc, sizeof(LIBLTE_PHY_ALLOCATION_STRUCT));

        ul_sched_queue_mutex.lock();
        for(iter=ul_sched_queue.begin(); iter!=ul_sched_queue.end(); iter++)
        {
            if((*iter)->alloc.rnti  == alloc->rnti &&
               (*iter)->current_tti == ul_sched->current_tti)
            {
                ul_sched->current_tti = (ul_sched->current_tti + 1) % (LTE_FDD_ENB_CURRENT_TTI_MAX + 1);
            }
        }
        ul_sched_queue.push_back(ul_sched);
        ul_sched_queue_mutex.unlock();

        err = LTE_FDD_ENB_ERROR_NONE;
    }

    return(err);
}

/*****************/
/*    Helpers    */
/*****************/
uint32 LTE_fdd_enb_mac::get_n_reserved_prbs(uint32 current_tti)
{
    uint32 N_reserved_prbs = 0;
    uint32 i;

    // Reserve PRBs for the MIB
    if(0 == (current_tti % 10))
    {
        N_reserved_prbs += 6;
    }

    // Reserve PRBs for SIB1
    if(5 == (current_tti % 10) &&
       0 == ((current_tti / 10) % 2))
    {
        N_reserved_prbs += sys_info.sib1_alloc.N_prb;
    }

    // Reserve PRBs for all other SIBs
    for(i=0; i<sys_info.sib1.N_sched_info; i++)
    {
        if(0                             != sys_info.sib_alloc[i].msg.N_bits &&
           (i * sys_info.si_win_len)%10  == (current_tti % 10)               &&
           ((i * sys_info.si_win_len)/10 == ((current_tti / 10) % sys_info.si_periodicity_T)))
        {
            N_reserved_prbs += sys_info.sib_alloc[i].N_prb;
        }
    }

    return(N_reserved_prbs);
}
