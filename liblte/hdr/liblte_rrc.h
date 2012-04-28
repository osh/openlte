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

    File: liblte_rrc.h

    Description: Contains all the definitions for the LTE Radio Resource
                 Control Layer library.

    Revision History
    ----------    -------------    --------------------------------------------
    03/24/2012    Ben Wojtowicz    Created file.
    04/21/2012    Ben Wojtowicz    Added SIB1 parameters, IEs, and messages

*******************************************************************************/

#ifndef __LIBLTE_RRC_H__
#define __LIBLTE_RRC_H__

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "liblte_common.h"

/*******************************************************************************
                              DEFINES
*******************************************************************************/

// FIXME: These were chosen arbitrarily
#define LIBLTE_RRC_MAX_IE_SIZE  4096
#define LIBLTE_RRC_MAX_MSG_SIZE 4096

/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/

typedef struct{
    uint32 N_bits;
    uint8  msg[LIBLTE_RRC_MAX_MSG_SIZE];
}LIBLTE_RRC_MSG_STRUCT;

/*******************************************************************************
                              INFORMATION ELEMENT DECLARATIONS
*******************************************************************************/

/*********************************************************************
    IE Name: PHICH Config

    Description: Specifies the PHICH configuration

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_RRC_PHICH_DURATION_NORMAL = 0,
    LIBLTE_RRC_PHICH_DURATION_EXTENDED,
}LIBLTE_RRC_PHICH_DURATION_ENUM;
typedef enum{
    LIBLTE_RRC_PHICH_RESOURCE_1_6 = 0,
    LIBLTE_RRC_PHICH_RESOURCE_1_2,
    LIBLTE_RRC_PHICH_RESOURCE_1,
    LIBLTE_RRC_PHICH_RESOURCE_2,
}LIBLTE_RRC_PHICH_RESOURCE_ENUM;
// Structs
typedef struct{
    LIBLTE_RRC_PHICH_DURATION_ENUM dur;
    LIBLTE_RRC_PHICH_RESOURCE_ENUM res;
}LIBLTE_RRC_PHICH_CONFIG_STRUCT;

/*********************************************************************
    IE Name: P Max

    Description: Limits the UE's uplink transmission power on a
                 carrier frequency and is used to calculate the
                 parameter P Compensation

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
// Defines
// Enums
// Structs

/*********************************************************************
    IE Name: TDD Config

    Description: Specifies the TDD specific physical channel
                 configuration

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_RRC_SUBFRAME_ASSIGNMENT_0 = 0,
    LIBLTE_RRC_SUBFRAME_ASSIGNMENT_1,
    LIBLTE_RRC_SUBFRAME_ASSIGNMENT_2,
    LIBLTE_RRC_SUBFRAME_ASSIGNMENT_3,
    LIBLTE_RRC_SUBFRAME_ASSIGNMENT_4,
    LIBLTE_RRC_SUBFRAME_ASSIGNMENT_5,
    LIBLTE_RRC_SUBFRAME_ASSIGNMENT_6,
}LIBLTE_RRC_SUBFRAME_ASSIGNMENT_ENUM;
typedef enum{
    LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_0 = 0,
    LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_1,
    LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_2,
    LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_3,
    LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_4,
    LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_5,
    LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_6,
    LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_7,
    LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_8,
}LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_ENUM;
// Structs

/*********************************************************************
    IE Name: Cell Identity

    Description: Unambiguously identifies a cell within a PLMN

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs

/*********************************************************************
    IE Name: CSG Identity

    Description: Identifies a Closed Subscriber Group

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
#define LIBLTE_RRC_CSG_IDENTITY_NOT_PRESENT 0xFFFFFFFF
// Enums
// Structs

/*********************************************************************
    IE Name: PLMN Identity

    Description: Identifies a Public Land Mobile Network

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
#define LIBLTE_RRC_MCC_NOT_PRESENT 0xFFFF
// Enums
// Structs
typedef struct{
    uint16 mcc;
    uint16 mnc;
}LIBLTE_RRC_PLMN_IDENTITY_STRUCT;

/*********************************************************************
    IE Name: Q Rx Lev Min

    Description: Indicates the required minimum received RSRP level in
                 the (E-UTRA) cell for cell selection/re-selection

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs

/*********************************************************************
    IE Name: Tracking Area Code

    Description: Identifies a tracking area within the scope of a
                 PLMN

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs

/*******************************************************************************
                              MESSAGE DECLARATIONS
*******************************************************************************/

/*********************************************************************
    Message Name: System Information Block Type 1

    Description: Contains information relevant when evaluating if a
                 UE is allowed to access a cell and defines the
                 scheduling of other system information

    Document Reference: 36.331 v10.0.0 Section 6.2.2 
*********************************************************************/
// Defines
#define LIBLTE_RRC_MAX_N_PLMN_IDENTITIES 6
#define LIBLTE_RRC_MAX_SIB               32
#define LIBLTE_RRC_MAX_SI_MESSAGE        32
// Enums
typedef enum{
    LIBLTE_RRC_CELL_BARRED = 0,
    LIBLTE_RRC_CELL_NOT_BARRED,
}LIBLTE_RRC_CELL_BARRED_ENUM;
typedef enum{
    LIBLTE_RRC_INTRA_FREQ_RESELECTION_ALLOWED = 0,
    LIBLTE_RRC_INTRA_FREQ_RESELECTION_NOT_ALLOWED,
}LIBLTE_RRC_INTRA_FREQ_RESELECTION_ENUM;
typedef enum{
    LIBLTE_RRC_SI_WINDOW_LENGTH_MS1 = 0,
    LIBLTE_RRC_SI_WINDOW_LENGTH_MS2,
    LIBLTE_RRC_SI_WINDOW_LENGTH_MS5,
    LIBLTE_RRC_SI_WINDOW_LENGTH_MS10,
    LIBLTE_RRC_SI_WINDOW_LENGTH_MS15,
    LIBLTE_RRC_SI_WINDOW_LENGTH_MS20,
    LIBLTE_RRC_SI_WINDOW_LENGTH_MS40,
}LIBLTE_RRC_SI_WINDOW_LENGTH_ENUM;
typedef enum{
    LIBLTE_RRC_RESV_FOR_OPER = 0,
    LIBLTE_RRC_NOT_RESV_FOR_OPER,
}LIBLTE_RRC_RESV_FOR_OPER_ENUM;
typedef enum{
    LIBLTE_RRC_SI_PERIODICITY_RF8 = 0,
    LIBLTE_RRC_SI_PERIODICITY_RF16,
    LIBLTE_RRC_SI_PERIODICITY_RF32,
    LIBLTE_RRC_SI_PERIODICITY_RF64,
    LIBLTE_RRC_SI_PERIODICITY_RF128,
    LIBLTE_RRC_SI_PERIODICITY_RF256,
    LIBLTE_RRC_SI_PERIODICITY_RF512,
}LIBLTE_RRC_SI_PERIODICITY_ENUM;
typedef enum{
    LIBLTE_RRC_SIB_TYPE_3 = 0,
    LIBLTE_RRC_SIB_TYPE_4,
    LIBLTE_RRC_SIB_TYPE_5,
    LIBLTE_RRC_SIB_TYPE_6,
    LIBLTE_RRC_SIB_TYPE_7,
    LIBLTE_RRC_SIB_TYPE_8,
    LIBLTE_RRC_SIB_TYPE_9,
    LIBLTE_RRC_SIB_TYPE_10,
    LIBLTE_RRC_SIB_TYPE_11,
    LIBLTE_RRC_SIB_TYPE_12_v920,
    LIBLTE_RRC_SIB_TYPE_13_v920,
    LIBLTE_RRC_SIB_TYPE_SPARE_5,
    LIBLTE_RRC_SIB_TYPE_SPARE_4,
    LIBLTE_RRC_SIB_TYPE_SPARE_3,
    LIBLTE_RRC_SIB_TYPE_SPARE_2,
    LIBLTE_RRC_SIB_TYPE_SPARE_1,
}LIBLTE_RRC_SIB_TYPE_ENUM;
// Structs
typedef struct{
    LIBLTE_RRC_PLMN_IDENTITY_STRUCT id;
    LIBLTE_RRC_RESV_FOR_OPER_ENUM   resv_for_oper;
}LIBLTE_RRC_PLMN_IDENTITY_LIST_STRUCT;
typedef struct{
    LIBLTE_RRC_SIB_TYPE_ENUM sib_type;
}LIBLTE_RRC_SIB_MAPPING_INFO_STRUCT;
typedef struct{
    LIBLTE_RRC_SIB_MAPPING_INFO_STRUCT sib_mapping_info[LIBLTE_RRC_MAX_SIB];
    LIBLTE_RRC_SI_PERIODICITY_ENUM     si_periodicity;
    uint32                             N_sib_mapping_info;
}LIBLTE_RRC_SCHEDULING_INFO_STRUCT;
typedef struct{
    LIBLTE_RRC_PLMN_IDENTITY_LIST_STRUCT      plmn_id[LIBLTE_RRC_MAX_N_PLMN_IDENTITIES];
    LIBLTE_RRC_SCHEDULING_INFO_STRUCT         sched_info[LIBLTE_RRC_MAX_SI_MESSAGE];
    LIBLTE_RRC_CELL_BARRED_ENUM               cell_barred;
    LIBLTE_RRC_INTRA_FREQ_RESELECTION_ENUM    intra_freq_reselection;
    LIBLTE_RRC_SI_WINDOW_LENGTH_ENUM          si_window_length;
    LIBLTE_RRC_SUBFRAME_ASSIGNMENT_ENUM       sf_assignment;
    LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_ENUM special_sf_patterns;
    uint32                                    cell_id;
    uint32                                    csg_id;
    uint32                                    N_plmn_ids;
    uint32                                    N_sched_info;
    uint16                                    tracking_area_code;
    int16                                     q_rx_lev_min;
    uint8                                     csg_indication;
    uint8                                     q_rx_lev_min_offset;
    uint8                                     freq_band_indicator;
    uint8                                     system_info_value_tag;
    int8                                      p_max;
    bool                                      tdd;
}LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_sys_info_block_type_1_msg(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1_STRUCT *sib1,
                                                            LIBLTE_RRC_MSG_STRUCT                   *msg);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_sys_info_block_type_1_msg(LIBLTE_RRC_MSG_STRUCT                   *msg,
                                                              LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1_STRUCT *sib1,
                                                              uint32                                  *N_bits_used);

/*********************************************************************
    Message Name: BCCH BCH Message

    Description: Contains the set of RRC messages that may be sent
                 from the E-UTRAN to the UE via BCH on the BCCH
                 logical channel.

    Document Reference: 36.331 v10.0.0 Sections 6.2.1 and 6.2.2
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_RRC_DL_BANDWIDTH_6 = 0,
    LIBLTE_RRC_DL_BANDWIDTH_15,
    LIBLTE_RRC_DL_BANDWIDTH_25,
    LIBLTE_RRC_DL_BANDWIDTH_50,
    LIBLTE_RRC_DL_BANDWIDTH_75,
    LIBLTE_RRC_DL_BANDWIDTH_100,
}LIBLTE_RRC_DL_BANDWIDTH_ENUM;
// Structs
typedef struct{
    LIBLTE_RRC_PHICH_CONFIG_STRUCT phich_config;
    LIBLTE_RRC_DL_BANDWIDTH_ENUM   dl_bw;
    uint8                          sfn_div_4;
}LIBLTE_RRC_MIB_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_bcch_bch_msg(LIBLTE_RRC_MIB_STRUCT *mib,
                                               LIBLTE_RRC_MSG_STRUCT *msg);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_bcch_bch_msg(LIBLTE_RRC_MSG_STRUCT *msg,
                                                 LIBLTE_RRC_MIB_STRUCT *mib);

/*********************************************************************
    Message Name: BCCH DLSCH Message

    Description: Contains the set of RRC messages that may be sent
                 from the E-UTRAN to the UE via DLSCH on the BCCH
                 logical channel.

    Document Reference: 36.331 v10.0.0 Section 6.2.1
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_bcch_dlsch_msg(LIBLTE_RRC_MSG_STRUCT *msg);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_bcch_dlsch_msg(LIBLTE_RRC_MSG_STRUCT                   *msg,
                                                   LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1_STRUCT *sib1);

#endif /* __LIBLTE_RRC_H__ */
