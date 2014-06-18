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

    File: LTE_fdd_enb_rlc.h

    Description: Contains all the definitions for the LTE FDD eNodeB
                 radio link control layer.

    Revision History
    ----------    -------------    --------------------------------------------
    11/09/2013    Ben Wojtowicz    Created file
    05/04/2014    Ben Wojtowicz    Added communication to MAC and PDCP.
    06/15/2014    Ben Wojtowicz    Using the latest LTE library.

*******************************************************************************/

#ifndef __LTE_FDD_ENB_RLC_H__
#define __LTE_FDD_ENB_RLC_H__

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_enb_cnfg_db.h"
#include "LTE_fdd_enb_msgq.h"
#include <boost/thread/mutex.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>

/*******************************************************************************
                              DEFINES
*******************************************************************************/


/*******************************************************************************
                              FORWARD DECLARATIONS
*******************************************************************************/


/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/


/*******************************************************************************
                              CLASS DECLARATIONS
*******************************************************************************/

class LTE_fdd_enb_rlc
{
public:
    // Singleton
    static LTE_fdd_enb_rlc* get_instance(void);
    static void cleanup(void);

    // Start/Stop
    void start(void);
    void stop(void);

    // External interface
    void update_sys_info(void);

private:
    // Singleton
    static LTE_fdd_enb_rlc *instance;
    LTE_fdd_enb_rlc();
    ~LTE_fdd_enb_rlc();

    // Start/Stop
    boost::mutex start_mutex;
    bool         started;

    // Communication
    void handle_mac_msg(LTE_FDD_ENB_MESSAGE_STRUCT *msg);
    void handle_pdcp_msg(LTE_FDD_ENB_MESSAGE_STRUCT *msg);
    LTE_fdd_enb_msgq                   *mac_comm_msgq;
    LTE_fdd_enb_msgq                   *pdcp_comm_msgq;
    boost::interprocess::message_queue *rlc_mac_mq;
    boost::interprocess::message_queue *rlc_pdcp_mq;

    // MAC Message Handlers
    void handle_pdu_ready(LTE_FDD_ENB_RLC_PDU_READY_MSG_STRUCT *pdu_ready);
    void handle_tm_pdu(LIBLTE_BIT_MSG_STRUCT *pdu, LTE_fdd_enb_user *user, LTE_fdd_enb_rb *rb);
    void handle_um_pdu(LIBLTE_BIT_MSG_STRUCT *pdu, LTE_fdd_enb_user *user, LTE_fdd_enb_rb *rb);
    void handle_am_pdu(LIBLTE_BIT_MSG_STRUCT *pdu, LTE_fdd_enb_user *user, LTE_fdd_enb_rb *rb);

    // PDCP Message Handlers
    void handle_sdu_ready(LTE_FDD_ENB_RLC_SDU_READY_MSG_STRUCT *sdu_ready);
    void handle_tm_sdu(LIBLTE_BIT_MSG_STRUCT *sdu, LTE_fdd_enb_user *user, LTE_fdd_enb_rb *rb);
    void handle_um_sdu(LIBLTE_BIT_MSG_STRUCT *sdu, LTE_fdd_enb_user *user, LTE_fdd_enb_rb *rb);
    void handle_am_sdu(LIBLTE_BIT_MSG_STRUCT *sdu, LTE_fdd_enb_user *user, LTE_fdd_enb_rb *rb);

    // Parameters
    boost::mutex                sys_info_mutex;
    LTE_FDD_ENB_SYS_INFO_STRUCT sys_info;
};

#endif /* __LTE_FDD_ENB_RLC_H__ */
