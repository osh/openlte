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

    File: LTE_fdd_enb_rrc.h

    Description: Contains all the definitions for the LTE FDD eNodeB
                 radio resource control layer.

    Revision History
    ----------    -------------    --------------------------------------------
    11/09/2013    Ben Wojtowicz    Created file
    05/04/2014    Ben Wojtowicz    Added PDCP communication and UL CCCH state
                                   machine.
    06/15/2014    Ben Wojtowicz    Added UL DCCH message handling and MME NAS
                                   message handling.

*******************************************************************************/

#ifndef __LTE_FDD_ENB_RRC_H__
#define __LTE_FDD_ENB_RRC_H__

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_enb_cnfg_db.h"
#include "LTE_fdd_enb_user.h"
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

class LTE_fdd_enb_rrc
{
public:
    // Singleton
    static LTE_fdd_enb_rrc* get_instance(void);
    static void cleanup(void);

    // Start/Stop
    void start(void);
    void stop(void);

    // External interface
    void update_sys_info(void);

private:
    // Singleton
    static LTE_fdd_enb_rrc *instance;
    LTE_fdd_enb_rrc();
    ~LTE_fdd_enb_rrc();

    // Start/Stop
    boost::mutex start_mutex;
    bool         started;

    // Communication
    void handle_pdcp_msg(LTE_FDD_ENB_MESSAGE_STRUCT *msg);
    void handle_mme_msg(LTE_FDD_ENB_MESSAGE_STRUCT *msg);
    LTE_fdd_enb_msgq                   *pdcp_comm_msgq;
    LTE_fdd_enb_msgq                   *mme_comm_msgq;
    boost::interprocess::message_queue *rrc_pdcp_mq;
    boost::interprocess::message_queue *rrc_mme_mq;

    // PDCP Message Handlers
    void handle_pdu_ready(LTE_FDD_ENB_RRC_PDU_READY_MSG_STRUCT *pdu_ready);

    // MME Message Handlers
    void handle_nas_msg(LTE_FDD_ENB_RRC_NAS_MSG_READY_MSG_STRUCT *nas_msg);

    // State Machines
    void ccch_sm(LIBLTE_BIT_MSG_STRUCT *msg, LTE_fdd_enb_user *user, LTE_fdd_enb_rb *rb);
    void dcch_sm(LIBLTE_BIT_MSG_STRUCT *msg, LTE_fdd_enb_user *user, LTE_fdd_enb_rb *rb);

    // Message Parsers
    void parse_ul_ccch_message(LIBLTE_BIT_MSG_STRUCT *msg, LTE_fdd_enb_user *user, LTE_fdd_enb_rb *rb);
    void parse_ul_dcch_message(LIBLTE_BIT_MSG_STRUCT *msg, LTE_fdd_enb_user *user, LTE_fdd_enb_rb *rb);

    // Message Senders
    void send_rrc_con_setup(LTE_fdd_enb_user *user, LTE_fdd_enb_rb *rb);

    // Parameters
    boost::mutex                sys_info_mutex;
    LTE_FDD_ENB_SYS_INFO_STRUCT sys_info;
};

#endif /* __LTE_FDD_ENB_RRC_H__ */
