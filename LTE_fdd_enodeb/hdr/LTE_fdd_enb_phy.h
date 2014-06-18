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

    File: LTE_fdd_enb_phy.h

    Description: Contains all the definitions for the LTE FDD eNodeB
                 physical layer.

    Revision History
    ----------    -------------    --------------------------------------------
    11/09/2013    Ben Wojtowicz    Created file
    01/18/2014    Ben Wojtowicz    Cached a copy of the interface class and
                                   added the ability to handle late subframes.
    05/04/2014    Ben Wojtowicz    Added PHICH support.
    06/15/2014    Ben Wojtowicz    Changed fn_combo to current_tti.

*******************************************************************************/

#ifndef __LTE_FDD_ENB_PHY_H__
#define __LTE_FDD_ENB_PHY_H__

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_enb_interface.h"
#include "LTE_fdd_enb_cnfg_db.h"
#include "LTE_fdd_enb_msgq.h"
#include "LTE_fdd_enb_radio.h"
#include "liblte_phy.h"
#include <boost/thread/mutex.hpp>

/*******************************************************************************
                              DEFINES
*******************************************************************************/

#define LTE_FDD_ENB_CURRENT_TTI_MAX (LIBLTE_PHY_SFN_MAX*10 + 9)

/*******************************************************************************
                              FORWARD DECLARATIONS
*******************************************************************************/


/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/


/*******************************************************************************
                              CLASS DECLARATIONS
*******************************************************************************/

class LTE_fdd_enb_phy
{
public:
    // Singleton
    static LTE_fdd_enb_phy* get_instance(void);
    static void cleanup(void);

    // Start/Stop
    void start(LTE_fdd_enb_interface *iface);
    void stop(void);

    // External interface
    void update_sys_info(void);
    uint32 get_n_cce(void);

    // Radio interface
    void radio_interface(LTE_FDD_ENB_RADIO_TX_BUF_STRUCT *tx_buf, LTE_FDD_ENB_RADIO_RX_BUF_STRUCT *rx_buf);
    void radio_interface(LTE_FDD_ENB_RADIO_TX_BUF_STRUCT *tx_buf);

private:
    // Singleton
    static LTE_fdd_enb_phy *instance;
    LTE_fdd_enb_phy();
    ~LTE_fdd_enb_phy();

    // Start/Stop
    LTE_fdd_enb_interface *interface;
    bool                   started;

    // Communication
    void handle_mac_msg(LTE_FDD_ENB_MESSAGE_STRUCT *msg);
    LTE_fdd_enb_msgq                   *mac_comm_msgq;
    boost::interprocess::message_queue *phy_mac_mq;

    // Generic parameters
    LIBLTE_PHY_STRUCT *phy_struct;

    // Downlink
    void handle_dl_schedule(LTE_FDD_ENB_DL_SCHEDULE_MSG_STRUCT *dl_sched);
    void handle_ul_schedule(LTE_FDD_ENB_UL_SCHEDULE_MSG_STRUCT *ul_sched);
    void process_dl(LTE_FDD_ENB_RADIO_TX_BUF_STRUCT *tx_buf);
    boost::mutex                       sys_info_mutex;
    boost::mutex                       dl_sched_mutex;
    boost::mutex                       ul_sched_mutex;
    LTE_FDD_ENB_SYS_INFO_STRUCT        sys_info;
    LTE_FDD_ENB_DL_SCHEDULE_MSG_STRUCT dl_schedule[10];
    LTE_FDD_ENB_UL_SCHEDULE_MSG_STRUCT ul_schedule[10];
    LIBLTE_PHY_PCFICH_STRUCT           pcfich;
    LIBLTE_PHY_PHICH_STRUCT            phich[10];
    LIBLTE_PHY_PDCCH_STRUCT            pdcch;
    LIBLTE_PHY_SUBFRAME_STRUCT         dl_subframe;
    LIBLTE_BIT_MSG_STRUCT              dl_rrc_msg;
    uint32                             dl_current_tti;
    uint32                             last_rts_current_tti;
    bool                               late_subfr;

    // Uplink
    void process_ul(LTE_FDD_ENB_RADIO_RX_BUF_STRUCT *rx_buf);
    LTE_FDD_ENB_PRACH_DECODE_MSG_STRUCT prach_decode;
    LTE_FDD_ENB_PUCCH_DECODE_MSG_STRUCT pucch_decode;
    LTE_FDD_ENB_PUSCH_DECODE_MSG_STRUCT pusch_decode;
    LIBLTE_PHY_SUBFRAME_STRUCT          ul_subframe;
    uint32                              ul_current_tti;
    uint32                              prach_sfn_mod;
    uint32                              prach_subfn_mod;
    uint32                              prach_subfn_check;
    bool                                prach_subfn_zero_allowed;
};

#endif /* __LTE_FDD_ENB_PHY_H__ */
