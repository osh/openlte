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

    File: LTE_fdd_enb_mac.h

    Description: Contains all the definitions for the LTE FDD eNodeB
                 medium access control layer.

    Revision History
    ----------    -------------    --------------------------------------------
    11/09/2013    Ben Wojtowicz    Created file
    01/18/2014    Ben Wojtowicz    Cached a copy of the interface class.
    05/04/2014    Ben Wojtowicz    Added ULSCH handling.
    06/15/2014    Ben Wojtowicz    Added uplink scheduling and changed fn_combo
                                   to current_tti.

*******************************************************************************/

#ifndef __LTE_FDD_ENB_MAC_H__
#define __LTE_FDD_ENB_MAC_H__

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_enb_interface.h"
#include "LTE_fdd_enb_cnfg_db.h"
#include "LTE_fdd_enb_msgq.h"
#include "LTE_fdd_enb_user.h"
#include "liblte_mac.h"
#include <boost/thread/mutex.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <list>

/*******************************************************************************
                              DEFINES
*******************************************************************************/


/*******************************************************************************
                              FORWARD DECLARATIONS
*******************************************************************************/


/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/

typedef struct{
    LIBLTE_PHY_ALLOCATION_STRUCT dl_alloc;
    LIBLTE_PHY_ALLOCATION_STRUCT ul_alloc;
    LIBLTE_MAC_RAR_STRUCT        rar;
    uint32                       current_tti;
}LTE_FDD_ENB_RAR_SCHED_QUEUE_STRUCT;

typedef struct{
    LIBLTE_PHY_ALLOCATION_STRUCT alloc;
    LIBLTE_MAC_PDU_STRUCT        mac_pdu;
    uint32                       current_tti;
}LTE_FDD_ENB_DL_SCHED_QUEUE_STRUCT;

typedef struct{
    LIBLTE_PHY_ALLOCATION_STRUCT alloc;
    uint32                       current_tti;
}LTE_FDD_ENB_UL_SCHED_QUEUE_STRUCT;

/*******************************************************************************
                              CLASS DECLARATIONS
*******************************************************************************/

class LTE_fdd_enb_mac
{
public:
    // Singleton
    static LTE_fdd_enb_mac* get_instance(void);
    static void cleanup(void);

    // Start/Stop
    void start(LTE_fdd_enb_interface *iface);
    void stop(void);

    // External interface
    void update_sys_info(void);
    void sched_ul(LTE_fdd_enb_user *user, uint32 requested_tbs);

private:
    // Singleton
    static LTE_fdd_enb_mac *instance;
    LTE_fdd_enb_mac();
    ~LTE_fdd_enb_mac();

    // Start/Stop
    boost::mutex           start_mutex;
    LTE_fdd_enb_interface *interface;
    bool                   started;

    // Communication
    void handle_phy_msg(LTE_FDD_ENB_MESSAGE_STRUCT *msg);
    void handle_rlc_msg(LTE_FDD_ENB_MESSAGE_STRUCT *msg);
    LTE_fdd_enb_msgq                   *phy_comm_msgq;
    LTE_fdd_enb_msgq                   *rlc_comm_msgq;
    boost::interprocess::message_queue *mac_phy_mq;
    boost::interprocess::message_queue *mac_rlc_mq;

    // PHY Message Handlers
    void handle_ready_to_send(LTE_FDD_ENB_READY_TO_SEND_MSG_STRUCT *rts);
    void handle_prach_decode(LTE_FDD_ENB_PRACH_DECODE_MSG_STRUCT *prach_decode);
    void handle_pucch_decode(LTE_FDD_ENB_PUCCH_DECODE_MSG_STRUCT *pucch_decode);
    void handle_pusch_decode(LTE_FDD_ENB_PUSCH_DECODE_MSG_STRUCT *pusch_decode);

    // RLC Message Handlers
    void handle_sdu_ready(LTE_FDD_ENB_MAC_SDU_READY_MSG_STRUCT *sdu_ready);

    // MAC PDU Handlers
    void handle_ulsch_ccch_sdu(LTE_fdd_enb_user *user, uint32 lcid, LIBLTE_BIT_MSG_STRUCT *sdu);
    void handle_ulsch_dcch_sdu(LTE_fdd_enb_user *user, uint32 lcid, LIBLTE_BIT_MSG_STRUCT *sdu);
    void handle_ulsch_ext_power_headroom_report(LTE_fdd_enb_user *user, LIBLTE_MAC_EXT_POWER_HEADROOM_CE_STRUCT *ext_power_headroom);
    void handle_ulsch_power_headroom_report(LTE_fdd_enb_user *user, LIBLTE_MAC_POWER_HEADROOM_CE_STRUCT *power_headroom);
    void handle_ulsch_c_rnti(LTE_fdd_enb_user **user, LIBLTE_MAC_C_RNTI_CE_STRUCT *c_rnti);
    void handle_ulsch_truncated_bsr(LTE_fdd_enb_user *user, LIBLTE_MAC_TRUNCATED_BSR_CE_STRUCT *truncated_bsr);
    void handle_ulsch_short_bsr(LTE_fdd_enb_user *user, LIBLTE_MAC_SHORT_BSR_CE_STRUCT *short_bsr);
    void handle_ulsch_long_bsr(LTE_fdd_enb_user *user, LIBLTE_MAC_LONG_BSR_CE_STRUCT *long_bsr);

    // Data Constructors
    void construct_random_access_response(uint8 preamble, uint16 timing_adv, uint32 current_tti);

    // Scheduler
    void scheduler(void);
    LTE_FDD_ENB_ERROR_ENUM add_to_rar_sched_queue(uint32 current_tti, LIBLTE_PHY_ALLOCATION_STRUCT *dl_alloc, LIBLTE_PHY_ALLOCATION_STRUCT *ul_alloc, LIBLTE_MAC_RAR_STRUCT *rar);
    LTE_FDD_ENB_ERROR_ENUM add_to_dl_sched_queue(uint32 current_tti, LIBLTE_MAC_PDU_STRUCT *mac_pdu, LIBLTE_PHY_ALLOCATION_STRUCT *alloc);
    LTE_FDD_ENB_ERROR_ENUM add_to_ul_sched_queue(uint32 current_tti, LIBLTE_PHY_ALLOCATION_STRUCT *alloc);
    boost::mutex                                   rar_sched_queue_mutex;
    boost::mutex                                   dl_sched_queue_mutex;
    boost::mutex                                   ul_sched_queue_mutex;
    std::list<LTE_FDD_ENB_RAR_SCHED_QUEUE_STRUCT*> rar_sched_queue;
    std::list<LTE_FDD_ENB_DL_SCHED_QUEUE_STRUCT*>  dl_sched_queue;
    std::list<LTE_FDD_ENB_UL_SCHED_QUEUE_STRUCT*>  ul_sched_queue;
    LTE_FDD_ENB_DL_SCHEDULE_MSG_STRUCT             sched_dl_subfr[10];
    LTE_FDD_ENB_UL_SCHEDULE_MSG_STRUCT             sched_ul_subfr[10];
    uint8                                          sched_cur_dl_subfn;
    uint8                                          sched_cur_ul_subfn;

    // Parameters
    boost::mutex                sys_info_mutex;
    LTE_FDD_ENB_SYS_INFO_STRUCT sys_info;

    // Helpers
    uint32 get_n_reserved_prbs(uint32 current_tti);
};

#endif /* __LTE_FDD_ENB_MAC_H__ */
