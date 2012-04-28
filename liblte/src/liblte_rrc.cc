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

    File: liblte_rrc.cc

    Description: Contains all the implementations for the LTE Radio Resource
                 Control Layer library.

    Revision History
    ----------    -------------    --------------------------------------------
    03/24/2012    Ben Wojtowicz    Created file.
    04/25/2012    Ben Wojtowicz    Added SIB1 parameters, IEs, and messages

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "liblte_rrc.h"

/*******************************************************************************
                              DEFINES
*******************************************************************************/


/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/


/*******************************************************************************
                              GLOBAL VARIABLES
*******************************************************************************/

LIBLTE_RRC_MSG_STRUCT global_msg;

/*******************************************************************************
                              LOCAL FUNCTION PROTOTYPES
*******************************************************************************/

/*********************************************************************
    Name: rrc_value_2_bits

    Description: Converts a value to a bit string
*********************************************************************/
void rrc_value_2_bits(void);

/*********************************************************************
    Name: rrc_bits_2_value

    Description: Converts a bit string to a value
*********************************************************************/
uint32 rrc_bits_2_value(uint8  **bits,
                        uint32   N_bits);

/*******************************************************************************
                              INFORMATION ELEMENT FUNCTIONS
*******************************************************************************/

/*********************************************************************
    IE Name: PHICH-Config

    Description: Specifies the PHICH configuration

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
void pack_phich_config_ie(void)
{
    // FIXME
}
void unpack_phich_config_ie(uint8                          **ie_ptr,
                            LIBLTE_RRC_PHICH_CONFIG_STRUCT  *phich_config)
{
    // Duration
    phich_config->dur = (LIBLTE_RRC_PHICH_DURATION_ENUM)rrc_bits_2_value(ie_ptr, 1);

    // Resource
    phich_config->res = (LIBLTE_RRC_PHICH_RESOURCE_ENUM)rrc_bits_2_value(ie_ptr, 2);
}

/*********************************************************************
    IE Name: P Max

    Description: Limits the UE's uplink transmission power on a
                 carrier frequency and is used to calculate the
                 parameter P Compensation

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
void pack_p_max_ie(void)
{
    // FIXME
}
void unpack_p_max_ie(uint8 **ie_ptr,
                     int8   *p_max)
{
    *p_max = (int8)rrc_bits_2_value(ie_ptr, 6) - 30;
}

/*********************************************************************
    IE Name: TDD Config

    Description: Specifies the TDD specific physical channel
                 configuration

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
void pack_tdd_config_ie(void)
{
    // FIXME
}
void unpack_tdd_config_ie(uint8                                     **ie_ptr,
                          LIBLTE_RRC_SUBFRAME_ASSIGNMENT_ENUM        *sa,
                          LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_ENUM  *ssp)
{
    *sa  = (LIBLTE_RRC_SUBFRAME_ASSIGNMENT_ENUM)rrc_bits_2_value(ie_ptr, 3);
    *ssp = (LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_ENUM)rrc_bits_2_value(ie_ptr, 4);
}

/*********************************************************************
    IE Name: Cell Identity

    Description: Unambiguously identifies a cell within a PLMN

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
void pack_cell_identity_ie(void)
{
    // FIXME
}
void unpack_cell_identity_ie(uint8  **ie_ptr,
                             uint32  *cell_id)
{
    *cell_id = rrc_bits_2_value(ie_ptr, 28);
}

/*********************************************************************
    IE Name: CSG Identity

    Description: Identifies a Closed Subscriber Group

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
void pack_csg_identity_ie(void)
{
    // FIXME
}
void unpack_csg_identity_ie(uint8  **ie_ptr,
                            uint32  *csg_id)
{
    *csg_id = rrc_bits_2_value(ie_ptr, 27);
}

/*********************************************************************
    IE Name: PLMN Identity

    Description: Identifies a Public Land Mobile Network

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
void pack_plmn_identity_ie(void)
{
    // FIXME
}
void unpack_plmn_identity_ie(uint8                           **ie_ptr,
                             LIBLTE_RRC_PLMN_IDENTITY_STRUCT  *plmn_id)
{
    uint8 mcc_opt;
    uint8 mnc_size;

    mcc_opt = rrc_bits_2_value(ie_ptr, 1);

    if(true == mcc_opt)
    {
        plmn_id->mcc = rrc_bits_2_value(ie_ptr, 12);
    }else{
        plmn_id->mcc = LIBLTE_RRC_MCC_NOT_PRESENT;
    }

    mnc_size     = (rrc_bits_2_value(ie_ptr, 1) + 2)*4;
    plmn_id->mnc = rrc_bits_2_value(ie_ptr, mnc_size);
    if(8 == mnc_size)
    {
        plmn_id->mnc |= 0xFF00;
    }else{
        plmn_id->mnc |= 0xF000;
    }
}

/*********************************************************************
    IE Name: Q Rx Lev Min

    Description: Indicates the required minimum received RSRP level in
                 the (E-UTRA) cell for cell selection/re-selection

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
void pack_q_rx_lev_min_ie(void)
{
    // FIXME
}
void unpack_q_rx_lev_min_ie(uint8 **ie_ptr,
                            int16  *q_rx_lev_min)
{
    *q_rx_lev_min = ((int16)rrc_bits_2_value(ie_ptr, 6) - 70) * 2;
}

/*********************************************************************
    IE Name: Tracking Area Code

    Description: Identifies a tracking area within the scope of a
                 PLMN

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
void pack_tracking_area_code_ie(void)
{
    // FIXME
}
void unpack_tracking_area_code_ie(uint8  **ie_ptr,
                                  uint16  *tac)
{
    *tac = rrc_bits_2_value(ie_ptr, 16);
}

/*******************************************************************************
                              MESSAGE FUNCTIONS
*******************************************************************************/

/*********************************************************************
    Message Name: System Information Block Type 1

    Description: Contains information relevant when evaluating if a
                 UE is allowed to access a cell and defines the
                 scheduling of other system information.

    Document Reference: 36.331 v10.0.0 Section 6.2.2
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_sys_info_block_type_1_msg(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1_STRUCT *sib1,
                                                            LIBLTE_RRC_MSG_STRUCT                   *msg)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    // FIXME

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_sys_info_block_type_1_msg(LIBLTE_RRC_MSG_STRUCT                   *msg,
                                                              LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1_STRUCT *sib1,
                                                              uint32                                  *N_bits_used)
{
    LIBLTE_ERROR_ENUM  err     = LIBLTE_ERROR_INVALID_INPUTS;
    uint8             *msg_ptr = msg->msg;
    uint32             i;
    uint32             j;
    uint8              p_max_opt;
    uint8              tdd_config_opt;
    uint8              non_crit_ext_opt;
    uint8              csg_id_opt;
    uint8              q_rx_lev_min_offset_opt;
    uint8              extension;

    if(msg         != NULL &&
       N_bits_used != NULL)
    {
        // Optional field indicators
        p_max_opt        = rrc_bits_2_value(&msg_ptr, 1);
        tdd_config_opt   = rrc_bits_2_value(&msg_ptr, 1);
        non_crit_ext_opt = rrc_bits_2_value(&msg_ptr, 1);

        // Cell Access Related Info
        csg_id_opt       = rrc_bits_2_value(&msg_ptr, 1);
        sib1->N_plmn_ids = rrc_bits_2_value(&msg_ptr, 3) + 1;
        for(i=0; i<sib1->N_plmn_ids; i++)
        {
            unpack_plmn_identity_ie(&msg_ptr, &sib1->plmn_id[i].id);
            if(LIBLTE_RRC_MCC_NOT_PRESENT == sib1->plmn_id[i].id.mcc &&
               0                          != i)
            {
                sib1->plmn_id[i].id.mcc = sib1->plmn_id[i-1].id.mcc;
            }
            sib1->plmn_id[i].resv_for_oper = (LIBLTE_RRC_RESV_FOR_OPER_ENUM)rrc_bits_2_value(&msg_ptr, 1);
        }
        unpack_tracking_area_code_ie(&msg_ptr, &sib1->tracking_area_code);
        unpack_cell_identity_ie(&msg_ptr, &sib1->cell_id);
        sib1->cell_barred            = (LIBLTE_RRC_CELL_BARRED_ENUM)rrc_bits_2_value(&msg_ptr, 1);
        sib1->intra_freq_reselection = (LIBLTE_RRC_INTRA_FREQ_RESELECTION_ENUM)rrc_bits_2_value(&msg_ptr, 1);
        sib1->csg_indication         = rrc_bits_2_value(&msg_ptr, 1);
        if(true == csg_id_opt)
        {
            unpack_csg_identity_ie(&msg_ptr, &sib1->csg_id);
        }else{
            sib1->csg_id = LIBLTE_RRC_CSG_IDENTITY_NOT_PRESENT;
        }

        // Cell Selection Info
        q_rx_lev_min_offset_opt = rrc_bits_2_value(&msg_ptr, 1);
        unpack_q_rx_lev_min_ie(&msg_ptr, &sib1->q_rx_lev_min);
        if(true == q_rx_lev_min_offset_opt)
        {
            sib1->q_rx_lev_min_offset = (rrc_bits_2_value(&msg_ptr, 3) + 1)*2;
        }else{
            sib1->q_rx_lev_min_offset = 0;
        }

        // P Max
        unpack_p_max_ie(&msg_ptr, &sib1->p_max);

        // Freq Band Indicator
        sib1->freq_band_indicator = rrc_bits_2_value(&msg_ptr, 6) + 1;

        // Scheduling Info List
        sib1->N_sched_info = rrc_bits_2_value(&msg_ptr, 5) + 1;
        for(i=0; i<sib1->N_sched_info; i++)
        {
            sib1->sched_info[i].si_periodicity     = (LIBLTE_RRC_SI_PERIODICITY_ENUM)rrc_bits_2_value(&msg_ptr, 3);
            sib1->sched_info[i].N_sib_mapping_info = rrc_bits_2_value(&msg_ptr, 5);
            for(j=0; j<sib1->sched_info[i].N_sib_mapping_info; j++)
            {
                extension                                        = rrc_bits_2_value(&msg_ptr, 1);
                sib1->sched_info[i].sib_mapping_info[j].sib_type = (LIBLTE_RRC_SIB_TYPE_ENUM)rrc_bits_2_value(&msg_ptr, 4);
            }
        }

        // TDD Config
        if(true == tdd_config_opt)
        {
            sib1->tdd = true;
            unpack_tdd_config_ie(&msg_ptr, &sib1->sf_assignment, &sib1->special_sf_patterns);
        }else{
            sib1->tdd = false;
        }

        // SI Window Length
        sib1->si_window_length = (LIBLTE_RRC_SI_WINDOW_LENGTH_ENUM)rrc_bits_2_value(&msg_ptr, 3);

        // System Info Value Tag
        sib1->system_info_value_tag = rrc_bits_2_value(&msg_ptr, 5);

        // Non Critical Extension
        // FIXME

        // N_bits_used
        *N_bits_used = msg_ptr - (msg->msg);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    Message Name: bcch_bch_msg

    Description: Contains the set of RRC messages that may be sent
                 from the E-UTRAN to the UE via BCH on the BCCH
                 logical channel.

    Document Reference: 36.331 v10.0.0 Sections 6.2.1 and 6.2.2
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_bcch_bch_msg(LIBLTE_RRC_MIB_STRUCT *mib,
                                               LIBLTE_RRC_MSG_STRUCT *msg)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    // FIXME

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_bcch_bch_msg(LIBLTE_RRC_MSG_STRUCT *msg,
                                                 LIBLTE_RRC_MIB_STRUCT *mib)
{
    LIBLTE_ERROR_ENUM  err     = LIBLTE_ERROR_INVALID_INPUTS;
    uint8             *msg_ptr = msg->msg;

    if(msg != NULL &&
       mib != NULL)
    {
        // DL Bandwidth
        mib->dl_bw = (LIBLTE_RRC_DL_BANDWIDTH_ENUM)rrc_bits_2_value(&msg_ptr, 3);

        // PHICH CONFIG
        unpack_phich_config_ie(&msg_ptr, &mib->phich_config);


        // SFN/4
        mib->sfn_div_4 = rrc_bits_2_value(&msg_ptr, 8);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    Message Name: bcch_dlsch_msg

    Description: Contains the set of RRC messages that may be sent
                 from the E-UTRAN to the UE via DLSCH on the BCCH
                 logical channel.

    Document Reference: 36.331 v10.0.0 Section 6.2.1
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_bcch_dlsch_msg(LIBLTE_RRC_MSG_STRUCT *msg)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    // FIXME

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_bcch_dlsch_msg(LIBLTE_RRC_MSG_STRUCT                   *msg,
                                                   LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1_STRUCT *sib1)
{
    LIBLTE_ERROR_ENUM  err     = LIBLTE_ERROR_INVALID_INPUTS;
    uint8             *msg_ptr = msg->msg;
    uint32             N_bits_used;
    uint8              ext;
    uint8              sib1_choice;

    if(msg  != NULL &&
       sib1 != NULL)
    {
        // Extension bit
        ext = rrc_bits_2_value(&msg_ptr, 1);

        // SIB1 Choice bit
        sib1_choice = rrc_bits_2_value(&msg_ptr, 1);

        if(1 == sib1_choice)
        {
            memcpy(global_msg.msg, msg_ptr, msg->N_bits-(msg_ptr-msg->msg));
            global_msg.N_bits = msg->N_bits-(msg_ptr-msg->msg);
            err               = liblte_rrc_unpack_sys_info_block_type_1_msg(&global_msg,
                                                                            sib1,
                                                                            &N_bits_used);
            msg_ptr += N_bits_used;
        }else{
            printf("ERROR: Not handling non-SIB1 BCCH DLSCH message\n");
        }
    }

    return(err);
}

/*******************************************************************************
                              LOCAL FUNCTIONS
*******************************************************************************/

/*********************************************************************
    Name: rrc_value_2_bits

    Description: Converts a value to a bit string
*********************************************************************/
void rrc_value_2_bits(void)
{
    // FIXME
}

/*********************************************************************
    Name: rrc_bits_2_value

    Description: Converts a bit string to a value
*********************************************************************/
uint32 rrc_bits_2_value(uint8  **bits,
                        uint32   N_bits)
{
    uint32 value = 0;
    uint32 i;

    for(i=0; i<N_bits; i++)
    {
        value |= (*bits)[i] << (N_bits-i-1);
    }
    *bits += N_bits;

    return(value);
}
