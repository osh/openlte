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
    05/28/2012    Ben Wojtowicz    Added SIB1 pack functionality
    08/19/2012    Ben Wojtowicz    Added functionality to support SIB2, SIB3,
                                   SIB4, and SIB8 packing and unpacking
    10/06/2012    Ben Wojtowicz    Added more decoding/encoding.
    12/26/2012    Ben Wojtowicz    Added text versions of some enums.

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
    IE Name: MBSFN Notification Config

    Description: Specifies the MBMS notification related configuration
                 parameters, that are applicable for all MBSFN areas

    Document Reference: 36.331 v10.0.0 Section 6.3.7
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_RRC_NOTIFICATION_REPETITION_COEFF_R9_N2 = 0,
    LIBLTE_RRC_NOTIFICATION_REPETITION_COEFF_R9_N4,
}LIBLTE_RRC_NOTIFICATION_REPETITION_COEFF_R9_ENUM;
// Structs
typedef struct{
    LIBLTE_RRC_NOTIFICATION_REPETITION_COEFF_R9_ENUM repetition_coeff;
    uint8                                            offset;
    uint8                                            sf_index;
}LIBLTE_RRC_MBSFN_NOTIFICATION_CONFIG_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_mbsfn_notification_config_ie(LIBLTE_RRC_MBSFN_NOTIFICATION_CONFIG_STRUCT  *mbsfn_notification_cnfg,
                                                               uint8                                       **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_mbsfn_notification_config_ie(uint8                                       **ie_ptr,
                                                                 LIBLTE_RRC_MBSFN_NOTIFICATION_CONFIG_STRUCT  *mbsfn_notification_cnfg);

/*********************************************************************
    IE Name: MBSFN Area Info List

    Description: Contains the information required to acquire the MBMS
                 control information associated with one or more MBSFN
                 areas

    Document Reference: 36.331 v10.0.0 Section 6.3.7
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: MBSFN Subframe Config

    Description: Defines subframes that are reserved for MBSFN in
                 downlink

    Document Reference: 36.331 v10.0.0 Section 6.3.7
*********************************************************************/
// Defines
#define LIBLTE_RRC_MAX_MBSFN_ALLOCATIONS 8
// Enums
typedef enum{
    LIBLTE_RRC_RADIO_FRAME_ALLOCATION_PERIOD_N1 = 0,
    LIBLTE_RRC_RADIO_FRAME_ALLOCATION_PERIOD_N2,
    LIBLTE_RRC_RADIO_FRAME_ALLOCATION_PERIOD_N4,
    LIBLTE_RRC_RADIO_FRAME_ALLOCATION_PERIOD_N8,
    LIBLTE_RRC_RADIO_FRAME_ALLOCATION_PERIOD_N16,
    LIBLTE_RRC_RADIO_FRAME_ALLOCATION_PERIOD_N32,
}LIBLTE_RRC_RADIO_FRAME_ALLOCATION_PERIOD_ENUM;
typedef enum{
    LIBLTE_RRC_SUBFRAME_ALLOCATION_NUM_FRAMES_ONE = 0,
    LIBLTE_RRC_SUBFRAME_ALLOCATION_NUM_FRAMES_FOUR,
}LIBLTE_RRC_SUBFRAME_ALLOCATION_NUM_FRAMES_ENUM;
// Structs
typedef struct{
    LIBLTE_RRC_RADIO_FRAME_ALLOCATION_PERIOD_ENUM  radio_fr_alloc_period;
    LIBLTE_RRC_SUBFRAME_ALLOCATION_NUM_FRAMES_ENUM subfr_alloc_num_frames;
    uint32                                         subfr_alloc;
    uint8                                          radio_fr_alloc_offset;
}LIBLTE_RRC_MBSFN_SUBFRAME_CONFIG_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_mbsfn_subframe_config_ie(LIBLTE_RRC_MBSFN_SUBFRAME_CONFIG_STRUCT  *mbsfn_subfr_cnfg,
                                                           uint8                                   **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_mbsfn_subframe_config_ie(uint8                                   **ie_ptr,
                                                             LIBLTE_RRC_MBSFN_SUBFRAME_CONFIG_STRUCT  *mbsfn_subfr_cnfg);

/*********************************************************************
    IE Name: PMCH Info List

    Description: Specifies configuration of all PMCHs of an MBSFN area

    Document Reference: 36.331 v10.0.0 Section 6.3.7
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: C-RNTI

    Description: Identifies a UE having a RRC connection within a cell

    Document Reference: 36.331 v10.0.0 Section 6.3.6
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_c_rnti_ie(uint16   rnti,
                                            uint8  **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_c_rnti_ie(uint8  **ie_ptr,
                                              uint16  *rnti);

/*********************************************************************
    IE Name: Dedicated Info CDMA2000

    Description: Transfers UE specific CDMA2000 information between
                 the network and the UE

    Document Reference: 36.331 v10.0.0 Section 6.3.6
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_dedicated_info_cdma2000_ie(uint8   *ded_info_cdma2000,
                                                             uint32   length,
                                                             uint8  **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_dedicated_info_cdma2000_ie(uint8  **ie_ptr,
                                                               uint8   *ded_info_cdma2000,
                                                               uint32  *length);

/*********************************************************************
    IE Name: Dedicated Info NAS

    Description: Transfers UE specific NAS layer information between
                 the network and the UE

    Document Reference: 36.331 v10.0.0 Section 6.3.6
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_dedicated_info_nas_ie(uint8   *ded_info_nas,
                                                        uint32   length,
                                                        uint8  **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_dedicated_info_nas_ie(uint8  **ie_ptr,
                                                          uint8   *ded_info_nas,
                                                          uint32  *length);

/*********************************************************************
    IE Name: Filter Coefficient

    Description: Specifies the measurement filtering coefficient

    Document Reference: 36.331 v10.0.0 Section 6.3.6
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_RRC_FILTER_COEFFICIENT_FC0 = 0,
    LIBLTE_RRC_FILTER_COEFFICIENT_FC1,
    LIBLTE_RRC_FILTER_COEFFICIENT_FC2,
    LIBLTE_RRC_FILTER_COEFFICIENT_FC3,
    LIBLTE_RRC_FILTER_COEFFICIENT_FC4,
    LIBLTE_RRC_FILTER_COEFFICIENT_FC5,
    LIBLTE_RRC_FILTER_COEFFICIENT_FC6,
    LIBLTE_RRC_FILTER_COEFFICIENT_FC7,
    LIBLTE_RRC_FILTER_COEFFICIENT_FC8,
    LIBLTE_RRC_FILTER_COEFFICIENT_FC9,
    LIBLTE_RRC_FILTER_COEFFICIENT_FC11,
    LIBLTE_RRC_FILTER_COEFFICIENT_FC13,
    LIBLTE_RRC_FILTER_COEFFICIENT_FC15,
    LIBLTE_RRC_FILTER_COEFFICIENT_FC17,
    LIBLTE_RRC_FILTER_COEFFICIENT_FC19,
    LIBLTE_RRC_FILTER_COEFFICIENT_SPARE1,
}LIBLTE_RRC_FILTER_COEFFICIENT_ENUM;
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_filter_coefficient_ie(LIBLTE_RRC_FILTER_COEFFICIENT_ENUM   filter_coeff,
                                                        uint8                              **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_filter_coefficient_ie(uint8                              **ie_ptr,
                                                          LIBLTE_RRC_FILTER_COEFFICIENT_ENUM  *filter_coeff);

/*********************************************************************
    IE Name: MMEC

    Description: Identifies an MME within the scope of an MME group
                 within a PLMN

    Document Reference: 36.331 v10.0.0 Section 6.3.6
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_mmec_ie(uint8   mmec,
                                          uint8 **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_mmec_ie(uint8 **ie_ptr,
                                            uint8  *mmec);

/*********************************************************************
    IE Name: Neigh Cell Config

    Description: Provides the information related to MBSFN and TDD
                 UL/DL configuration of neighbor cells

    Document Reference: 36.331 v10.0.0 Section 6.3.6
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_neigh_cell_config_ie(uint8   neigh_cell_config,
                                                       uint8 **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_neigh_cell_config_ie(uint8 **ie_ptr,
                                                         uint8  *neigh_cell_config);

/*********************************************************************
    IE Name: Other Config

    Description: Contains configuration related to other configuration

    Document Reference: 36.331 v10.0.0 Section 6.3.6
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_RRC_REPORT_PROXIMITY_INDICATION_EUTRA_R9_ENABLED = 0,
}LIBLTE_RRC_REPORT_PROXIMITY_INDICATION_EUTRA_R9_ENUM;
typedef enum{
    LIBLTE_RRC_REPORT_PROXIMITY_INDICATION_UTRA_R9_ENABLED = 0,
}LIBLTE_RRC_REPORT_PROXIMITY_INDICATION_UTRA_R9_ENUM;
// Structs
typedef struct{
    LIBLTE_RRC_REPORT_PROXIMITY_INDICATION_EUTRA_R9_ENUM report_proximity_ind_eutra;
    LIBLTE_RRC_REPORT_PROXIMITY_INDICATION_UTRA_R9_ENUM  report_proximity_ind_utra;
    bool                                                 report_proximity_ind_eutra_present;
    bool                                                 report_proximity_ind_utra_present;
}LIBLTE_RRC_REPORT_PROXIMITY_CONFIG_R9_STRUCT;
typedef struct{
    LIBLTE_RRC_REPORT_PROXIMITY_CONFIG_R9_STRUCT report_proximity_cnfg;
    bool                                         report_proximity_cnfg_present;
}LIBLTE_RRC_OTHER_CONFIG_R9_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_other_config_ie(LIBLTE_RRC_OTHER_CONFIG_R9_STRUCT  *other_cnfg,
                                                  uint8                             **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_other_config_ie(uint8                             **ie_ptr,
                                                    LIBLTE_RRC_OTHER_CONFIG_R9_STRUCT  *other_cnfg);

/*********************************************************************
    IE Name: RAND CDMA2000 (1xRTT)

    Description: Contains a random value, generated by the eNB, to be
                 passed to the CDMA2000 upper layers

    Document Reference: 36.331 v10.0.0 Section 6.3.6
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_rand_cdma2000_1xrtt_ie(uint32   rand,
                                                         uint8  **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_rand_cdma2000_1xrtt_ie(uint8  **ie_ptr,
                                                           uint32  *rand);

/*********************************************************************
    IE Name: RAT Type

    Description: Indicates the radio access technology (RAT),
                 including E-UTRA, of the requested/transferred UE
                 capabilities

    Document Reference: 36.331 v10.0.0 Section 6.3.6
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_RRC_RAT_TYPE_EUTRA = 0,
    LIBLTE_RRC_RAT_TYPE_UTRA,
    LIBLTE_RRC_RAT_TYPE_GERAN_CS,
    LIBLTE_RRC_RAT_TYPE_GERAN_PS,
    LIBLTE_RRC_RAT_TYPE_CDMA2000_1XRTT,
    LIBLTE_RRC_RAT_TYPE_SPARE_3,
    LIBLTE_RRC_RAT_TYPE_SPARE_2,
    LIBLTE_RRC_RAT_TYPE_SPARE_1,
}LIBLTE_RRC_RAT_TYPE_ENUM;
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_rat_type_ie(LIBLTE_RRC_RAT_TYPE_ENUM   rat_type,
                                              uint8                    **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_rat_type_ie(uint8                    **ie_ptr,
                                                LIBLTE_RRC_RAT_TYPE_ENUM  *rat_type);

/*********************************************************************
    IE Name: RRC Transaction Identifier

    Description: Identifies an RRC procedure along with the message
                 type

    Document Reference: 36.331 v10.0.0 Section 6.3.6
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_rrc_transaction_identifier_ie(uint8   rrc_transaction_id,
                                                                uint8 **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_rrc_transaction_identifier_ie(uint8 **ie_ptr,
                                                                  uint8  *rrc_transaction_id);

/*********************************************************************
    IE Name: S-TMSI

    Description: Contains an S-Temporary Mobile Subscriber Identity,
                 a temporary UE identity provided by the EPC which
                 uniquely identifies the UE within the tracking area

    Document Reference: 36.331 v10.0.0 Section 6.3.6
*********************************************************************/
// Defines
// Enums
// Structs
typedef struct{
    uint32 m_tmsi;
    uint8  mmec;
}LIBLTE_RRC_S_TMSI_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_s_tmsi_ie(LIBLTE_RRC_S_TMSI_STRUCT  *s_tmsi,
                                            uint8                    **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_s_tmsi_ie(uint8                    **ie_ptr,
                                              LIBLTE_RRC_S_TMSI_STRUCT  *s_tmsi);

/*********************************************************************
    IE Name: UE Capability RAT Container List

    Description: Contains list of containers, one for each RAT for
                 which UE capabilities are transferred

    Document Reference: 36.331 v10.0.0 Section 6.3.6
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: UE EUTRA Capability

    Description: Conveys the E-UTRA UE Radio Access Capability
                 Parameters

    Document Reference: 36.331 v10.0.0 Section 6.3.6
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: UE Timers and Constants

    Description: Contains timers and constants used by the UE in
                 either RRC_CONNECTED or RRC_IDLE

    Document Reference: 36.331 v10.0.0 Section 6.3.6
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_RRC_T300_MS100 = 0,
    LIBLTE_RRC_T300_MS200,
    LIBLTE_RRC_T300_MS300,
    LIBLTE_RRC_T300_MS400,
    LIBLTE_RRC_T300_MS600,
    LIBLTE_RRC_T300_MS1000,
    LIBLTE_RRC_T300_MS1500,
    LIBLTE_RRC_T300_MS2000,
}LIBLTE_RRC_T300_ENUM;
typedef enum{
    LIBLTE_RRC_T301_MS100 = 0,
    LIBLTE_RRC_T301_MS200,
    LIBLTE_RRC_T301_MS300,
    LIBLTE_RRC_T301_MS400,
    LIBLTE_RRC_T301_MS600,
    LIBLTE_RRC_T301_MS1000,
    LIBLTE_RRC_T301_MS1500,
    LIBLTE_RRC_T301_MS2000,
}LIBLTE_RRC_T301_ENUM;
typedef enum{
    LIBLTE_RRC_T310_MS0 = 0,
    LIBLTE_RRC_T310_MS50,
    LIBLTE_RRC_T310_MS100,
    LIBLTE_RRC_T310_MS200,
    LIBLTE_RRC_T310_MS500,
    LIBLTE_RRC_T310_MS1000,
    LIBLTE_RRC_T310_MS2000,
}LIBLTE_RRC_T310_ENUM;
typedef enum{
    LIBLTE_RRC_N310_N1 = 0,
    LIBLTE_RRC_N310_N2,
    LIBLTE_RRC_N310_N3,
    LIBLTE_RRC_N310_N4,
    LIBLTE_RRC_N310_N6,
    LIBLTE_RRC_N310_N8,
    LIBLTE_RRC_N310_N10,
    LIBLTE_RRC_N310_N20,
}LIBLTE_RRC_N310_ENUM;
typedef enum{
    LIBLTE_RRC_T311_MS1000 = 0,
    LIBLTE_RRC_T311_MS3000,
    LIBLTE_RRC_T311_MS5000,
    LIBLTE_RRC_T311_MS10000,
    LIBLTE_RRC_T311_MS15000,
    LIBLTE_RRC_T311_MS20000,
    LIBLTE_RRC_T311_MS30000,
}LIBLTE_RRC_T311_ENUM;
typedef enum{
    LIBLTE_RRC_N311_N1 = 0,
    LIBLTE_RRC_N311_N2,
    LIBLTE_RRC_N311_N3,
    LIBLTE_RRC_N311_N4,
    LIBLTE_RRC_N311_N5,
    LIBLTE_RRC_N311_N6,
    LIBLTE_RRC_N311_N8,
    LIBLTE_RRC_N311_N10,
}LIBLTE_RRC_N311_ENUM;
// Structs
typedef struct{
    LIBLTE_RRC_T300_ENUM t300;
    LIBLTE_RRC_T301_ENUM t301;
    LIBLTE_RRC_T310_ENUM t310;
    LIBLTE_RRC_N310_ENUM n310;
    LIBLTE_RRC_T311_ENUM t311;
    LIBLTE_RRC_N311_ENUM n311;
}LIBLTE_RRC_UE_TIMERS_AND_CONSTANTS_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_ue_timers_and_constants_ie(LIBLTE_RRC_UE_TIMERS_AND_CONSTANTS_STRUCT  *ue_timers_and_constants,
                                                             uint8                                     **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_ue_timers_and_constants_ie(uint8                                     **ie_ptr,
                                                               LIBLTE_RRC_UE_TIMERS_AND_CONSTANTS_STRUCT  *ue_timers_and_constants);

/*********************************************************************
    IE Name: Allowed Meas Bandwidth

    Description: Indicates the maximum allowed measurement bandwidth
                 on a carrier frequency as defined by the parameter
                 Transmission Bandwidth Configuration

    Document Reference: 36.331 v10.0.0 Section 6.3.5
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_RRC_ALLOWED_MEAS_BANDWIDTH_MBW6 = 0,
    LIBLTE_RRC_ALLOWED_MEAS_BANDWIDTH_MBW15,
    LIBLTE_RRC_ALLOWED_MEAS_BANDWIDTH_MBW25,
    LIBLTE_RRC_ALLOWED_MEAS_BANDWIDTH_MBW50,
    LIBLTE_RRC_ALLOWED_MEAS_BANDWIDTH_MBW75,
    LIBLTE_RRC_ALLOWED_MEAS_BANDWIDTH_MBW100,
}LIBLTE_RRC_ALLOWED_MEAS_BANDWIDTH_ENUM;
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_allowed_meas_bandwidth_ie(LIBLTE_RRC_ALLOWED_MEAS_BANDWIDTH_ENUM   allowed_meas_bw,
                                                            uint8                                  **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_allowed_meas_bandwidth_ie(uint8                                  **ie_ptr,
                                                              LIBLTE_RRC_ALLOWED_MEAS_BANDWIDTH_ENUM  *allowed_meas_bw);

/*********************************************************************
    IE Name: Hysteresis

    Description: Used within the entry and leave condition of an
                 event triggered reporting condition

    Document Reference: 36.331 v10.0.0 Section 6.3.5
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_hysteresis_ie(uint8   hysteresis,
                                                uint8 **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_hysteresis_ie(uint8 **ie_ptr,
                                                  uint8  *hysteresis);

/*********************************************************************
    IE Name: Location Info

    Description: Transfers location information available at the UE to
                 correlate measurements and UE position information

    Document Reference: 36.331 v10.0.0 Section 6.3.5
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Meas Config

    Description: Specifies measurements to be performed by the UE,
                 and covers intra-frequency, inter-frequency and
                 inter-RAT mobility as well as configuration of
                 measurement gaps

    Document Reference: 36.331 v10.0.0 Section 6.3.5
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Meas ID

    Description: Identifies a measurement configuration, i.e. linking
                 of a measurement object and a reporting configuration

    Document Reference: 36.331 v10.0.0 Section 6.3.5
*********************************************************************/
// Defines
#define LIBLTE_RRC_MAX_MEAS_ID 32
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_meas_id_ie(uint8   meas_id,
                                             uint8 **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_meas_id_ie(uint8 **ie_ptr,
                                               uint8  *meas_id);

/*********************************************************************
    IE Name: Meas Id To Add Mod List

    Description: Concerns a list of measurement identities to add or
                 modify, with for each entry the meas ID, the
                 associated meas object ID and the associated report
                 config ID

    Document Reference: 36.331 v10.0.0 Section 6.3.5
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Meas Object CDMA2000

    Description: Specifies information applicable for inter-RAT
                 CDMA2000 neighboring cells

    Document Reference: 36.331 v10.0.0 Section 6.3.5
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Meas Object EUTRA

    Description: Specifies information applicable for intra-frequency
                 or inter-frequency E-UTRA cells

    Document Reference: 36.331 v10.0.0 Section 6.3.5
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Meas Object GERAN

    Description: Specifies information applicable for inter-RAT
                 GERAN neighboring frequencies

    Document Reference: 36.331 v10.0.0 Section 6.3.5
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Meas Object ID

    Description: Identifies a measurement object configuration

    Document Reference: 36.331 v10.0.0 Section 6.3.5
*********************************************************************/
// Defines
#define LIBLTE_RRC_MAX_OBJECT_ID 32
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_meas_object_id_ie(uint8   meas_object_id,
                                                    uint8 **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_meas_object_id_ie(uint8 **ie_ptr,
                                                      uint8  *meas_object_id);

/*********************************************************************
    IE Name: Meas Object To Add Mod List

    Description: Concerns a list of measurement objects to add or
                 modify

    Document Reference: 36.331 v10.0.0 Section 6.3.5
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Meas Object UTRA

    Description: Specifies information applicable for inter-RAT UTRA
                 neighboring cells

    Document Reference: 36.331 v10.0.0 Section 6.3.5
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Meas Results

    Description: Covers measured results for intra-frequency,
                 inter-frequency and inter-RAT mobility

    Document Reference: 36.331 v10.0.0 Section 6.3.5
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Quantity Config

    Description: Specifies the measurement quantities and layer 3
                 filtering coefficients for E-UTRA and inter-RAT
                 measurements

    Document Reference: 36.331 v10.0.0 Section 6.3.5
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Report Config EUTRA

    Description: Specifies criteria for triggering of an E-UTRA
                 measurement reporting event

    Document Reference: 36.331 v10.0.0 Section 6.3.5
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Report Config ID

    Description: Identifies a measurement reporting configuration

    Document Reference: 36.331 v10.0.0 Section 6.3.5
*********************************************************************/
// Defines
#define LIBLTE_RRC_MAX_REPORT_CONFIG_ID 32
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_report_config_id_ie(uint8   report_cnfg_id,
                                                      uint8 **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_report_config_id_ie(uint8 **ie_ptr,
                                                        uint8  *report_cnfg_id);

/*********************************************************************
    IE Name: Report Config Inter RAT

    Description: Specifies criteria for triggering of an inter-RAT
                 measurement reporting event

    Document Reference: 36.331 v10.0.0 Section 6.3.5
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Report Config To Add Mod List

    Description: Concerns a list of reporting configurations to add
                 or modify

    Document Reference: 36.331 v10.0.0 Section 6.3.5
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Report Interval

    Description: Indicates the interval between periodic reports

    Document Reference: 36.331 v10.0.0 Section 6.3.5
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_RRC_REPORT_INTERVAL_MS120 = 0,
    LIBLTE_RRC_REPORT_INTERVAL_MS240,
    LIBLTE_RRC_REPORT_INTERVAL_MS480,
    LIBLTE_RRC_REPORT_INTERVAL_MS640,
    LIBLTE_RRC_REPORT_INTERVAL_MS1024,
    LIBLTE_RRC_REPORT_INTERVAL_MS2048,
    LIBLTE_RRC_REPORT_INTERVAL_MS5120,
    LIBLTE_RRC_REPORT_INTERVAL_MS10240,
    LIBLTE_RRC_REPORT_INTERVAL_MIN1,
    LIBLTE_RRC_REPORT_INTERVAL_MIN6,
    LIBLTE_RRC_REPORT_INTERVAL_MIN12,
    LIBLTE_RRC_REPORT_INTERVAL_MIN30,
    LIBLTE_RRC_REPORT_INTERVAL_MIN60,
    LIBLTE_RRC_REPORT_INTERVAL_SPARE3,
    LIBLTE_RRC_REPORT_INTERVAL_SPARE2,
    LIBLTE_RRC_REPORT_INTERVAL_SPARE1,
}LIBLTE_RRC_REPORT_INTERVAL_ENUM;
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_report_interval_ie(LIBLTE_RRC_REPORT_INTERVAL_ENUM   report_int,
                                                     uint8                           **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_report_interval_ie(uint8                           **ie_ptr,
                                                       LIBLTE_RRC_REPORT_INTERVAL_ENUM  *report_int);

/*********************************************************************
    IE Name: RSRP Range

    Description: Specifies the value range used in RSRP measurements
                 and thresholds

    Document Reference: 36.331 v10.0.0 Section 6.3.5
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_rsrp_range_ie(uint8   rsrp_range,
                                                uint8 **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_rsrp_range_ie(uint8 **ie_ptr,
                                                  uint8  *rsrp_range);

/*********************************************************************
    IE Name: RSRQ Range

    Description: Specifies the value range used in RSRQ measurements
                 and thresholds

    Document Reference: 36.331 v10.0.0 Section 6.3.5
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_rsrq_range_ie(uint8   rsrq_range,
                                                uint8 **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_rsrq_range_ie(uint8 **ie_ptr,
                                                  uint8  *rsrq_range);

/*********************************************************************
    IE Name: Time To Trigger

    Description: Specifies the value range used for the time to
                 trigger parameter, which concerns the time during
                 which specific criteria for the event needs to be
                 met in order to trigger a measurement report

    Document Reference: 36.331 v10.0.0 Section 6.3.5
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_RRC_TIME_TO_TRIGGER_MS0 = 0,
    LIBLTE_RRC_TIME_TO_TRIGGER_MS40,
    LIBLTE_RRC_TIME_TO_TRIGGER_MS64,
    LIBLTE_RRC_TIME_TO_TRIGGER_MS80,
    LIBLTE_RRC_TIME_TO_TRIGGER_MS100,
    LIBLTE_RRC_TIME_TO_TRIGGER_MS128,
    LIBLTE_RRC_TIME_TO_TRIGGER_MS160,
    LIBLTE_RRC_TIME_TO_TRIGGER_MS256,
    LIBLTE_RRC_TIME_TO_TRIGGER_MS320,
    LIBLTE_RRC_TIME_TO_TRIGGER_MS480,
    LIBLTE_RRC_TIME_TO_TRIGGER_MS512,
    LIBLTE_RRC_TIME_TO_TRIGGER_MS640,
    LIBLTE_RRC_TIME_TO_TRIGGER_MS1024,
    LIBLTE_RRC_TIME_TO_TRIGGER_MS1280,
    LIBLTE_RRC_TIME_TO_TRIGGER_MS2560,
    LIBLTE_RRC_TIME_TO_TRIGGER_MS5120,
}LIBLTE_RRC_TIME_TO_TRIGGER_ENUM;
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_time_to_trigger_ie(LIBLTE_RRC_TIME_TO_TRIGGER_ENUM   time_to_trigger,
                                                     uint8                           **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_time_to_trigger_ie(uint8                           **ie_ptr,
                                                       LIBLTE_RRC_TIME_TO_TRIGGER_ENUM  *time_to_trigger);

/*********************************************************************
    IE Name: Additional Spectrum Emission

    Description: FIXME

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_additional_spectrum_emission_ie(uint8   add_spect_em,
                                                                  uint8 **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_additional_spectrum_emission_ie(uint8 **ie_ptr,
                                                                    uint8  *add_spect_em);

/*********************************************************************
    IE Name: ARFCN value CDMA2000

    Description: Indicates the CDMA2000 carrier frequency within
                 a CDMA2000 band

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_arfcn_value_cdma2000_ie(uint16   arfcn,
                                                          uint8  **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_arfcn_value_cdma2000_ie(uint8  **ie_ptr,
                                                            uint16  *arfcn);

/*********************************************************************
    IE Name: ARFCN value EUTRA

    Description: Indicates the ARFCN applicable for a downlink,
                 uplink, or bi-directional (TDD) E-UTRA carrier
                 frequency

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_arfcn_value_eutra_ie(uint16   arfcn,
                                                       uint8  **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_arfcn_value_eutra_ie(uint8  **ie_ptr,
                                                         uint16  *arfcn);

/*********************************************************************
    IE Name: ARFCN value GERAN

    Description: Specifies the ARFCN value applicable for a GERAN
                 BCCH carrier frequency

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_arfcn_value_geran_ie(uint16   arfcn,
                                                       uint8  **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_arfcn_value_geran_ie(uint8  **ie_ptr,
                                                         uint16  *arfcn);

/*********************************************************************
    IE Name: ARFCN value UTRA

    Description: Indicates the ARFCN applicable for a downlink (Nd,
                 FDD) or bi-directional (Nt, TDD) UTRA carrier
                 frequency

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_arfcn_value_utra_ie(uint16   arfcn,
                                                      uint8  **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_arfcn_value_utra_ie(uint8  **ie_ptr,
                                                        uint16  *arfcn);

/*********************************************************************
    IE Name: Band Class CDMA2000

    Description: Defines the CDMA2000 band in which the CDMA2000
                 carrier frequency can be found

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_RRC_BAND_CLASS_CDMA2000_BC0 = 0,
    LIBLTE_RRC_BAND_CLASS_CDMA2000_BC1,
    LIBLTE_RRC_BAND_CLASS_CDMA2000_BC2,
    LIBLTE_RRC_BAND_CLASS_CDMA2000_BC3,
    LIBLTE_RRC_BAND_CLASS_CDMA2000_BC4,
    LIBLTE_RRC_BAND_CLASS_CDMA2000_BC5,
    LIBLTE_RRC_BAND_CLASS_CDMA2000_BC6,
    LIBLTE_RRC_BAND_CLASS_CDMA2000_BC7,
    LIBLTE_RRC_BAND_CLASS_CDMA2000_BC8,
    LIBLTE_RRC_BAND_CLASS_CDMA2000_BC9,
    LIBLTE_RRC_BAND_CLASS_CDMA2000_BC10,
    LIBLTE_RRC_BAND_CLASS_CDMA2000_BC11,
    LIBLTE_RRC_BAND_CLASS_CDMA2000_BC12,
    LIBLTE_RRC_BAND_CLASS_CDMA2000_BC13,
    LIBLTE_RRC_BAND_CLASS_CDMA2000_BC14,
    LIBLTE_RRC_BAND_CLASS_CDMA2000_BC15,
    LIBLTE_RRC_BAND_CLASS_CDMA2000_BC16,
    LIBLTE_RRC_BAND_CLASS_CDMA2000_BC17,
    LIBLTE_RRC_BAND_CLASS_CDMA2000_SPARE14,
    LIBLTE_RRC_BAND_CLASS_CDMA2000_SPARE13,
    LIBLTE_RRC_BAND_CLASS_CDMA2000_SPARE12,
    LIBLTE_RRC_BAND_CLASS_CDMA2000_SPARE11,
    LIBLTE_RRC_BAND_CLASS_CDMA2000_SPARE10,
    LIBLTE_RRC_BAND_CLASS_CDMA2000_SPARE9,
    LIBLTE_RRC_BAND_CLASS_CDMA2000_SPARE8,
    LIBLTE_RRC_BAND_CLASS_CDMA2000_SPARE7,
    LIBLTE_RRC_BAND_CLASS_CDMA2000_SPARE6,
    LIBLTE_RRC_BAND_CLASS_CDMA2000_SPARE5,
    LIBLTE_RRC_BAND_CLASS_CDMA2000_SPARE4,
    LIBLTE_RRC_BAND_CLASS_CDMA2000_SPARE3,
    LIBLTE_RRC_BAND_CLASS_CDMA2000_SPARE2,
    LIBLTE_RRC_BAND_CLASS_CDMA2000_SPARE1,
}LIBLTE_RRC_BAND_CLASS_CDMA2000_ENUM;
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_band_class_cdma2000_ie(LIBLTE_RRC_BAND_CLASS_CDMA2000_ENUM   bc_cdma2000,
                                                         uint8                               **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_band_class_cdma2000_ie(uint8                               **ie_ptr,
                                                           LIBLTE_RRC_BAND_CLASS_CDMA2000_ENUM  *bc_cdma2000);

/*********************************************************************
    IE Name: Band Indicator GERAN

    Description: Indicates how to interpret an associated GERAN
                 carrier ARFCN

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_RRC_BAND_INDICATOR_GERAN_DCS1800 = 0,
    LIBLTE_RRC_BAND_INDICATOR_GERAN_PCS1900,
}LIBLTE_RRC_BAND_INDICATOR_GERAN_ENUM;
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_band_indicator_geran_ie(LIBLTE_RRC_BAND_INDICATOR_GERAN_ENUM   bi_geran,
                                                          uint8                                **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_band_indicator_geran_ie(uint8                                **ie_ptr,
                                                            LIBLTE_RRC_BAND_INDICATOR_GERAN_ENUM  *bi_geran);

/*********************************************************************
    IE Name: Carrier Freq CDMA2000

    Description: Provides the CDMA2000 carrier information

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
typedef struct{
    LIBLTE_RRC_BAND_CLASS_CDMA2000_ENUM bandclass;
    uint16                              arfcn;
}LIBLTE_RRC_CARRIER_FREQ_CDMA2000_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_carrier_freq_cdma2000_ie(LIBLTE_RRC_CARRIER_FREQ_CDMA2000_STRUCT  *carrier_freq,
                                                           uint8                                   **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_carrier_freq_cdma2000_ie(uint8                                   **ie_ptr,
                                                             LIBLTE_RRC_CARRIER_FREQ_CDMA2000_STRUCT  *carrier_freq);

/*********************************************************************
    IE Name: Carrier Freq GERAN

    Description: Provides an unambiguous carrier frequency description
                 of a GERAN cell

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
typedef struct{
    LIBLTE_RRC_BAND_INDICATOR_GERAN_ENUM band_indicator;
    uint16                               arfcn;
}LIBLTE_RRC_CARRIER_FREQ_GERAN_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_carrier_freq_geran_ie(LIBLTE_RRC_CARRIER_FREQ_GERAN_STRUCT  *carrier_freq,
                                                        uint8                                **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_carrier_freq_geran_ie(uint8                                **ie_ptr,
                                                          LIBLTE_RRC_CARRIER_FREQ_GERAN_STRUCT  *carrier_freq);

/*********************************************************************
    IE Name: Carrier Freqs GERAN

    Description: Provides one or more GERAN ARFCN values, which
                 represent a list of GERAN BCCH carrier frequencies

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: CDMA2000 Type

    Description: Describes the type of CDMA2000 network

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_RRC_CDMA2000_TYPE_1XRTT = 0,
    LIBLTE_RRC_CDMA2000_TYPE_HRPD,
}LIBLTE_RRC_CDMA2000_TYPE_ENUM;
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_cdma2000_type_ie(LIBLTE_RRC_CDMA2000_TYPE_ENUM   cdma2000_type,
                                                   uint8                         **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_cdma2000_type_ie(uint8                         **ie_ptr,
                                                     LIBLTE_RRC_CDMA2000_TYPE_ENUM  *cdma2000_type);

/*********************************************************************
    IE Name: Cell Identity

    Description: Unambiguously identifies a cell within a PLMN

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_cell_identity_ie(uint32   cell_id,
                                                   uint8  **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_cell_identity_ie(uint8  **ie_ptr,
                                                     uint32  *cell_id);

/*********************************************************************
    IE Name: Cell Index List

    Description: Concerns a list of cell indecies, which may be used
                 for different purposes

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Cell Reselection Priority

    Description: Contains the absolute priority of the concerned
                 carrier frequency/set of frequencies (GERAN)/
                 bandclass (CDMA2000), as used by the cell
                 reselection procedure

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_cell_reselection_priority_ie(uint8   cell_resel_prio,
                                                               uint8 **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_cell_reselection_priority_ie(uint8 **ie_ptr,
                                                                 uint8  *cell_resel_prio);

/*********************************************************************
    IE Name: CSFB Registration Param 1xRTT

    Description: Indicates whether or not the UE shall perform a
                 CDMA2000 1xRTT pre-registration if the UE does not
                 have a valid/current pre-registration

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_RRC_POWER_DOWN_REG_R9_TRUE = 0,
}LIBLTE_RRC_POWER_DOWN_REG_R9_ENUM;
// Structs
typedef struct{
    uint16 sid;
    uint16 nid;
    uint16 reg_zone;
    uint8  reg_period;
    uint8  total_zone;
    uint8  zone_timer;
    bool   multiple_sid;
    bool   multiple_nid;
    bool   home_reg;
    bool   foreign_sid_reg;
    bool   foreign_nid_reg;
    bool   param_reg;
    bool   power_up_reg;
}LIBLTE_RRC_CSFB_REGISTRATION_PARAM_1XRTT_STRUCT;
typedef struct{
    LIBLTE_RRC_POWER_DOWN_REG_R9_ENUM power_down_reg;
}LIBLTE_RRC_CSFB_REGISTRATION_PARAM_1XRTT_V920_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_csfb_registration_param_1xrtt_ie(LIBLTE_RRC_CSFB_REGISTRATION_PARAM_1XRTT_STRUCT  *csfb_reg_param,
                                                                   uint8                                           **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_csfb_registration_param_1xrtt_ie(uint8                                           **ie_ptr,
                                                                     LIBLTE_RRC_CSFB_REGISTRATION_PARAM_1XRTT_STRUCT  *csfb_reg_param);
LIBLTE_ERROR_ENUM liblte_rrc_pack_csfb_registration_param_1xrtt_v920_ie(LIBLTE_RRC_CSFB_REGISTRATION_PARAM_1XRTT_V920_STRUCT  *csfb_reg_param,
                                                                        uint8                                                **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_csfb_registration_param_1xrtt_v920_ie(uint8                                                **ie_ptr,
                                                                          LIBLTE_RRC_CSFB_REGISTRATION_PARAM_1XRTT_V920_STRUCT  *csfb_reg_param);

/*********************************************************************
    IE Name: Cell Global ID EUTRA

    Description: Specifies the Evolved Cell Global Identifier (ECGI),
                 the globally unique identity of a cell in E-UTRA

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
// FIXME
typedef struct{
    uint16 mcc;
    uint16 mnc;
}LIBLTE_RRC_PLMN_IDENTITY_STRUCT;
// FIXME END
typedef struct{
    LIBLTE_RRC_PLMN_IDENTITY_STRUCT plmn_id;
    uint32                          cell_id;
}LIBLTE_RRC_CELL_GLOBAL_ID_EUTRA_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_cell_global_id_eutra_ie(LIBLTE_RRC_CELL_GLOBAL_ID_EUTRA_STRUCT  *cell_global_id,
                                                          uint8                                  **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_cell_global_id_eutra_ie(uint8                                  **ie_ptr,
                                                            LIBLTE_RRC_CELL_GLOBAL_ID_EUTRA_STRUCT  *cell_global_id);

/*********************************************************************
    IE Name: Cell Global ID UTRA

    Description: Specifies the global UTRAN Cell Identifier, the
                 globally unique identity of a cell in UTRA

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
typedef struct{
    LIBLTE_RRC_PLMN_IDENTITY_STRUCT plmn_id;
    uint32                          cell_id;
}LIBLTE_RRC_CELL_GLOBAL_ID_UTRA_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_cell_global_id_utra_ie(LIBLTE_RRC_CELL_GLOBAL_ID_UTRA_STRUCT  *cell_global_id,
                                                         uint8                                 **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_cell_global_id_utra_ie(uint8                                 **ie_ptr,
                                                           LIBLTE_RRC_CELL_GLOBAL_ID_UTRA_STRUCT  *cell_global_id);

/*********************************************************************
    IE Name: Cell Global ID GERAN

    Description: Specifies the Cell Global Identity (CGI), the
                 globally unique identity of a cell in GERAN

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
typedef struct{
    LIBLTE_RRC_PLMN_IDENTITY_STRUCT plmn_id;
    uint16                          lac;
    uint16                          cell_id;
}LIBLTE_RRC_CELL_GLOBAL_ID_GERAN_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_cell_global_id_geran_ie(LIBLTE_RRC_CELL_GLOBAL_ID_GERAN_STRUCT  *cell_global_id,
                                                          uint8                                  **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_cell_global_id_geran_ie(uint8                                  **ie_ptr,
                                                            LIBLTE_RRC_CELL_GLOBAL_ID_GERAN_STRUCT  *cell_global_id);

/*********************************************************************
    IE Name: Cell Global ID CDMA2000

    Description: Specifies the Cell Global Identity (CGI), the
                 globally unique identity of a cell in CDMA2000

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
typedef struct{
    uint64 onexrtt;
    uint32 hrpd[4];
}LIBLTE_RRC_CELL_GLOBAL_ID_CDMA2000_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_cell_global_id_cdma2000_ie(LIBLTE_RRC_CELL_GLOBAL_ID_CDMA2000_STRUCT  *cell_global_id,
                                                             uint8                                     **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_cell_global_id_cdma2000_ie(uint8                                     **ie_ptr,
                                                               LIBLTE_RRC_CELL_GLOBAL_ID_CDMA2000_STRUCT  *cell_global_id);

/*********************************************************************
    IE Name: CSG Identity

    Description: Identifies a Closed Subscriber Group

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
#define LIBLTE_RRC_CSG_IDENTITY_NOT_PRESENT 0xFFFFFFFF
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_csg_identity_ie(uint32   csg_id,
                                                  uint8  **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_csg_identity_ie(uint8  **ie_ptr,
                                                    uint32  *csg_id);

/*********************************************************************
    IE Name: Mobility Control Info

    Description: Includes parameters relevant for network controlled
                 mobility to/within E-UTRA

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Mobility Parameters CDMA2000 (1xRTT)

    Description: Contains the parameters provided to the UE for
                 handover and (enhanced) CSFB to 1xRTT support

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Mobility State Parameters

    Description: Contains parameters to determine UE mobility state

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_RRC_T_EVALUATION_S30 = 0,
    LIBLTE_RRC_T_EVALUATION_S60,
    LIBLTE_RRC_T_EVALUATION_S120,
    LIBLTE_RRC_T_EVALUATION_S180,
    LIBLTE_RRC_T_EVALUATION_S240,
    LIBLTE_RRC_T_EVALUATION_SPARE3,
    LIBLTE_RRC_T_EVALUATION_SPARE2,
    LIBLTE_RRC_T_EVALUATION_SPARE1,
}LIBLTE_RRC_T_EVALUATION_ENUM;
typedef enum{
    LIBLTE_RRC_T_HYST_NORMAL_S30 = 0,
    LIBLTE_RRC_T_HYST_NORMAL_S60,
    LIBLTE_RRC_T_HYST_NORMAL_S120,
    LIBLTE_RRC_T_HYST_NORMAL_S180,
    LIBLTE_RRC_T_HYST_NORMAL_S240,
    LIBLTE_RRC_T_HYST_NORMAL_SPARE3,
    LIBLTE_RRC_T_HYST_NORMAL_SPARE2,
    LIBLTE_RRC_T_HYST_NORMAL_SPARE1,
}LIBLTE_RRC_T_HYST_NORMAL_ENUM;
// Structs
typedef struct{
    LIBLTE_RRC_T_EVALUATION_ENUM  t_eval;
    LIBLTE_RRC_T_HYST_NORMAL_ENUM t_hyst_normal;
    uint8                         n_cell_change_medium;
    uint8                         n_cell_change_high;
}LIBLTE_RRC_MOBILITY_STATE_PARAMETERS_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_mobility_state_parameters_ie(LIBLTE_RRC_MOBILITY_STATE_PARAMETERS_STRUCT  *mobility_state_params,
                                                               uint8                                       **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_mobility_state_parameters_ie(uint8                                       **ie_ptr,
                                                                 LIBLTE_RRC_MOBILITY_STATE_PARAMETERS_STRUCT  *mobility_state_params);

/*********************************************************************
    IE Name: Phys Cell ID

    Description: Indicates the physical layer identity of the cell

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_phys_cell_id_ie(uint16   phys_cell_id,
                                                  uint8  **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_phys_cell_id_ie(uint8  **ie_ptr,
                                                    uint16  *phys_cell_id);

/*********************************************************************
    IE Name: Phys Cell ID Range

    Description: Encodes either a single or a range of physical cell
                 identities

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_RRC_PHYS_CELL_ID_RANGE_N4 = 0,
    LIBLTE_RRC_PHYS_CELL_ID_RANGE_N8,
    LIBLTE_RRC_PHYS_CELL_ID_RANGE_N12,
    LIBLTE_RRC_PHYS_CELL_ID_RANGE_N16,
    LIBLTE_RRC_PHYS_CELL_ID_RANGE_N24,
    LIBLTE_RRC_PHYS_CELL_ID_RANGE_N32,
    LIBLTE_RRC_PHYS_CELL_ID_RANGE_N48,
    LIBLTE_RRC_PHYS_CELL_ID_RANGE_N64,
    LIBLTE_RRC_PHYS_CELL_ID_RANGE_N84,
    LIBLTE_RRC_PHYS_CELL_ID_RANGE_N96,
    LIBLTE_RRC_PHYS_CELL_ID_RANGE_N128,
    LIBLTE_RRC_PHYS_CELL_ID_RANGE_N168,
    LIBLTE_RRC_PHYS_CELL_ID_RANGE_N252,
    LIBLTE_RRC_PHYS_CELL_ID_RANGE_N504,
    LIBLTE_RRC_PHYS_CELL_ID_RANGE_SPARE2,
    LIBLTE_RRC_PHYS_CELL_ID_RANGE_SPARE1,
    LIBLTE_RRC_PHYS_CELL_ID_RANGE_N1,
}LIBLTE_RRC_PHYS_CELL_ID_RANGE_ENUM;
// Structs
typedef struct{
    LIBLTE_RRC_PHYS_CELL_ID_RANGE_ENUM range;
    uint16                             start;
}LIBLTE_RRC_PHYS_CELL_ID_RANGE_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_phys_cell_id_range_ie(LIBLTE_RRC_PHYS_CELL_ID_RANGE_STRUCT  *phys_cell_id_range,
                                                        uint8                                **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_phys_cell_id_range_ie(uint8                                **ie_ptr,
                                                          LIBLTE_RRC_PHYS_CELL_ID_RANGE_STRUCT  *phys_cell_id_range);

/*********************************************************************
    IE Name: Phys Cell ID Range UTRA FDD List

    Description: Encodes one or more of Phys Cell ID Range UTRA FDD

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Phys Cell ID CDMA2000

    Description: Identifies the PN offset that represents the
                 "Physical cell identity" in CDMA2000

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
#define LIBLTE_RRC_MAX_PN_OFFSET 511
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_phys_cell_id_cdma2000_ie(uint16   phys_cell_id,
                                                           uint8  **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_phys_cell_id_cdma2000_ie(uint8  **ie_ptr,
                                                             uint16  *phys_cell_id);

/*********************************************************************
    IE Name: Phys Cell ID GERAN

    Description: Contains the Base Station Identity Code (BSIC)

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
typedef struct{
    uint8 ncc;
    uint8 bcc;
}LIBLTE_RRC_PHYS_CELL_ID_GERAN_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_phys_cell_id_geran_ie(LIBLTE_RRC_PHYS_CELL_ID_GERAN_STRUCT  *phys_cell_id,
                                                        uint8                                **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_phys_cell_id_geran_ie(uint8                                **ie_ptr,
                                                          LIBLTE_RRC_PHYS_CELL_ID_GERAN_STRUCT  *phys_cell_id);

/*********************************************************************
    IE Name: Phys Cell ID UTRA FDD

    Description: Indicates the physical layer identity of the cell

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_phys_cell_id_utra_fdd_ie(uint16   phys_cell_id,
                                                           uint8  **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_phys_cell_id_utra_fdd_ie(uint8  **ie_ptr,
                                                             uint16  *phys_cell_id);

/*********************************************************************
    IE Name: Phys Cell ID UTRA TDD

    Description: Indicates the physical layer identity of the cell

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_phys_cell_id_utra_tdd_ie(uint8   phys_cell_id,
                                                           uint8 **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_phys_cell_id_utra_tdd_ie(uint8 **ie_ptr,
                                                             uint8  *phys_cell_id);

/*********************************************************************
    IE Name: PLMN Identity

    Description: Identifies a Public Land Mobile Network

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
#define LIBLTE_RRC_MCC_NOT_PRESENT 0xFFFF
// Enums
// Structs
// FIXME
//typedef struct{
//    uint16 mcc;
//    uint16 mnc;
//}LIBLTE_RRC_PLMN_IDENTITY_STRUCT;
// FIXME END
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_plmn_identity_ie(LIBLTE_RRC_PLMN_IDENTITY_STRUCT  *plmn_id,
                                                   uint8                           **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_plmn_identity_ie(uint8                           **ie_ptr,
                                                     LIBLTE_RRC_PLMN_IDENTITY_STRUCT  *plmn_id);

/*********************************************************************
    IE Name: Pre Registration Info HRPD

    Description: FIXME

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
typedef struct{
    uint8 pre_reg_zone_id;
    uint8 secondary_pre_reg_zone_id_list[2];
    uint8 secondary_pre_reg_zone_id_list_size;
    bool  pre_reg_allowed;
    bool  pre_reg_zone_id_present;
}LIBLTE_RRC_PRE_REGISTRATION_INFO_HRPD_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_pre_registration_info_hrpd_ie(LIBLTE_RRC_PRE_REGISTRATION_INFO_HRPD_STRUCT  *pre_reg_info_hrpd,
                                                                uint8                                        **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_pre_registration_info_hrpd_ie(uint8                                        **ie_ptr,
                                                                  LIBLTE_RRC_PRE_REGISTRATION_INFO_HRPD_STRUCT  *pre_reg_info_hrpd);

/*********************************************************************
    IE Name: Q Qual Min

    Description: Indicates for cell selection/re-selection the
                 required minimum received RSRQ level in the (E-UTRA)
                 cell

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_q_qual_min_ie(int8    q_qual_min,
                                                uint8 **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_q_qual_min_ie(uint8 **ie_ptr,
                                                  int8   *q_qual_min);

/*********************************************************************
    IE Name: Q Rx Lev Min

    Description: Indicates the required minimum received RSRP level in
                 the (E-UTRA) cell for cell selection/re-selection

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_q_rx_lev_min_ie(int16   q_rx_lev_min,
                                                  uint8 **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_q_rx_lev_min_ie(uint8 **ie_ptr,
                                                    int16  *q_rx_lev_min);

/*********************************************************************
    IE Name: Q Offset Range

    Description: Indicates a cell or frequency specific offset to be
                 applied when evaluating candidates for cell
                 reselection or when evaluating triggering conditions
                 for measurement reporting

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_RRC_Q_OFFSET_RANGE_DB_N24 = 0,
    LIBLTE_RRC_Q_OFFSET_RANGE_DB_N22,
    LIBLTE_RRC_Q_OFFSET_RANGE_DB_N20,
    LIBLTE_RRC_Q_OFFSET_RANGE_DB_N18,
    LIBLTE_RRC_Q_OFFSET_RANGE_DB_N16,
    LIBLTE_RRC_Q_OFFSET_RANGE_DB_N14,
    LIBLTE_RRC_Q_OFFSET_RANGE_DB_N12,
    LIBLTE_RRC_Q_OFFSET_RANGE_DB_N10,
    LIBLTE_RRC_Q_OFFSET_RANGE_DB_N8,
    LIBLTE_RRC_Q_OFFSET_RANGE_DB_N6,
    LIBLTE_RRC_Q_OFFSET_RANGE_DB_N5,
    LIBLTE_RRC_Q_OFFSET_RANGE_DB_N4,
    LIBLTE_RRC_Q_OFFSET_RANGE_DB_N3,
    LIBLTE_RRC_Q_OFFSET_RANGE_DB_N2,
    LIBLTE_RRC_Q_OFFSET_RANGE_DB_N1,
    LIBLTE_RRC_Q_OFFSET_RANGE_DB_0,
    LIBLTE_RRC_Q_OFFSET_RANGE_DB_1,
    LIBLTE_RRC_Q_OFFSET_RANGE_DB_2,
    LIBLTE_RRC_Q_OFFSET_RANGE_DB_3,
    LIBLTE_RRC_Q_OFFSET_RANGE_DB_4,
    LIBLTE_RRC_Q_OFFSET_RANGE_DB_5,
    LIBLTE_RRC_Q_OFFSET_RANGE_DB_6,
    LIBLTE_RRC_Q_OFFSET_RANGE_DB_8,
    LIBLTE_RRC_Q_OFFSET_RANGE_DB_10,
    LIBLTE_RRC_Q_OFFSET_RANGE_DB_12,
    LIBLTE_RRC_Q_OFFSET_RANGE_DB_14,
    LIBLTE_RRC_Q_OFFSET_RANGE_DB_16,
    LIBLTE_RRC_Q_OFFSET_RANGE_DB_18,
    LIBLTE_RRC_Q_OFFSET_RANGE_DB_20,
    LIBLTE_RRC_Q_OFFSET_RANGE_DB_22,
    LIBLTE_RRC_Q_OFFSET_RANGE_DB_24,
    LIBLTE_RRC_Q_OFFSET_RANGE_SIZE,
}LIBLTE_RRC_Q_OFFSET_RANGE_ENUM;
static const char liblte_rrc_q_offset_range_text[LIBLTE_RRC_Q_OFFSET_RANGE_SIZE][10] = {"-24", "-22", "-20", "-18",
                                                                                        "-16", "-14", "-12", "-10",
                                                                                         "-8",  "-6",  "-5",  "-4",
                                                                                         "-3",  "-2",  "-1",   "0",
                                                                                          "1",   "2",   "3",   "4",
                                                                                          "5",   "6",   "8",  "10",
                                                                                         "12",  "14",  "16",  "18",
                                                                                         "20",  "22",  "24"};
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_q_offset_range_ie(LIBLTE_RRC_Q_OFFSET_RANGE_ENUM   q_offset_range,
                                                    uint8                          **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_q_offset_range_ie(uint8                          **ie_ptr,
                                                      LIBLTE_RRC_Q_OFFSET_RANGE_ENUM  *q_offset_range);

/*********************************************************************
    IE Name: Q Offset Range Inter RAT

    Description: Indicates a frequency specific offset to be applied
                 when evaluating triggering conditions for
                 measurement reporting

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_q_offset_range_inter_rat_ie(int8    q_offset_range_inter_rat,
                                                              uint8 **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_q_offset_range_inter_rat_ie(uint8 **ie_ptr,
                                                                int8   *q_offset_range_inter_rat);

/*********************************************************************
    IE Name: Reselection Threshold

    Description: Indicates an RX level threshold for cell reselection

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_reselection_threshold_ie(uint8   resel_thresh,
                                                           uint8 **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_reselection_threshold_ie(uint8 **ie_ptr,
                                                             uint8  *resel_thresh);

/*********************************************************************
    IE Name: Reselection Threshold Q

    Description: Indicates a quality level threshold for cell
                 reselection

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_reselection_threshold_q_ie(uint8   resel_thresh_q,
                                                             uint8 **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_reselection_threshold_q_ie(uint8 **ie_ptr,
                                                               uint8  *resel_thresh_q);

/*********************************************************************
    IE Name: S Cell Index

    Description: Contains a short identity, used to identify an
                 SCell

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_s_cell_index_ie(uint8   s_cell_idx,
                                                  uint8 **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_s_cell_index_ie(uint8 **ie_ptr,
                                                    uint8  *s_cell_idx);

/*********************************************************************
    IE Name: Serv Cell Index

    Description: Contains a short identity, used to identify a
                 serving cell

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_serv_cell_index_ie(uint8   serv_cell_idx,
                                                     uint8 **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_serv_cell_index_ie(uint8 **ie_ptr,
                                                       uint8  *serv_cell_idx);

/*********************************************************************
    IE Name: Speed State Scale Factors

    Description: Contains factors, to be applied when the UE is in
                 medium or high speed state, used for scaling a
                 mobility control related parameter

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_RRC_SSSF_MEDIUM_0DOT25 = 0,
    LIBLTE_RRC_SSSF_MEDIUM_0DOT5,
    LIBLTE_RRC_SSSF_MEDIUM_0DOT75,
    LIBLTE_RRC_SSSF_MEDIUM_1DOT0,
}LIBLTE_RRC_SSSF_MEDIUM_ENUM;
typedef enum{
    LIBLTE_RRC_SSSF_HIGH_0DOT25 = 0,
    LIBLTE_RRC_SSSF_HIGH_0DOT5,
    LIBLTE_RRC_SSSF_HIGH_0DOT75,
    LIBLTE_RRC_SSSF_HIGH_1DOT0,
}LIBLTE_RRC_SSSF_HIGH_ENUM;
// Structs
typedef struct{
    LIBLTE_RRC_SSSF_MEDIUM_ENUM sf_medium;
    LIBLTE_RRC_SSSF_HIGH_ENUM   sf_high;
}LIBLTE_RRC_SPEED_STATE_SCALE_FACTORS_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_speed_state_scale_factors_ie(LIBLTE_RRC_SPEED_STATE_SCALE_FACTORS_STRUCT  *speed_state_scale_factors,
                                                               uint8                                       **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_speed_state_scale_factors_ie(uint8                                       **ie_ptr,
                                                                 LIBLTE_RRC_SPEED_STATE_SCALE_FACTORS_STRUCT  *speed_state_scale_factors);

/*********************************************************************
    IE Name: System Info List GERAN

    Description: Contains system information of a GERAN cell

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: System Time Info CDMA2000

    Description: Informs the UE about the absolute time in the current
                 cell

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
typedef struct{
    uint64 system_time;
    bool   system_time_async;
    bool   cdma_eutra_sync;
}LIBLTE_RRC_SYSTEM_TIME_INFO_CDMA2000_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_system_time_info_cdma2000_ie(LIBLTE_RRC_SYSTEM_TIME_INFO_CDMA2000_STRUCT  *sys_time_info_cdma2000,
                                                               uint8                                       **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_system_time_info_cdma2000_ie(uint8                                       **ie_ptr,
                                                                 LIBLTE_RRC_SYSTEM_TIME_INFO_CDMA2000_STRUCT  *sys_time_info_cdma2000);

/*********************************************************************
    IE Name: Tracking Area Code

    Description: Identifies a tracking area within the scope of a
                 PLMN

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_tracking_area_code_ie(uint16   tac,
                                                        uint8  **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_tracking_area_code_ie(uint8  **ie_ptr,
                                                          uint16  *tac);

/*********************************************************************
    IE Name: T Reselection

    Description: Contains the timer T_reselection_rat for E-UTRA,
                 UTRA, GERAN, or CDMA2000

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_t_reselection_ie(uint8   t_resel,
                                                   uint8 **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_t_reselection_ie(uint8 **ie_ptr,
                                                     uint8  *t_resel);

/*********************************************************************
    IE Name: Next Hop Chaining Count

    Description: Updates the Kenb key and corresponds to parameter
                 NCC

    Document Reference: 36.331 v10.0.0 Section 6.3.3
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_next_hop_chaining_count_ie(uint8   next_hop_chaining_count,
                                                             uint8 **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_next_hop_chaining_count_ie(uint8 **ie_ptr,
                                                               uint8  *next_hop_chaining_count);

/*********************************************************************
    IE Name: Security Algorithm Config

    Description: Configures AS integrity protection algorithm (SRBs)
                 and AS ciphering algorithm (SRBs and DRBs)

    Document Reference: 36.331 v10.0.0 Section 6.3.3
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_RRC_CIPHERING_ALGORITHM_EEA0 = 0,
    LIBLTE_RRC_CIPHERING_ALGORITHM_EEA1,
    LIBLTE_RRC_CIPHERING_ALGORITHM_EEA2,
    LIBLTE_RRC_CIPHERING_ALGORITHM_SPARE5,
    LIBLTE_RRC_CIPHERING_ALGORITHM_SPARE4,
    LIBLTE_RRC_CIPHERING_ALGORITHM_SPARE3,
    LIBLTE_RRC_CIPHERING_ALGORITHM_SPARE2,
    LIBLTE_RRC_CIPHERING_ALGORITHM_SPARE1,
}LIBLTE_RRC_CIPHERING_ALGORITHM_ENUM;
typedef enum{
    LIBLTE_RRC_INTEGRITY_PROT_ALGORITHM_EIA0_V920 = 0,
    LIBLTE_RRC_INTEGRITY_PROT_ALGORITHM_EIA1,
    LIBLTE_RRC_INTEGRITY_PROT_ALGORITHM_EIA2,
    LIBLTE_RRC_INTEGRITY_PROT_ALGORITHM_SPARE5,
    LIBLTE_RRC_INTEGRITY_PROT_ALGORITHM_SPARE4,
    LIBLTE_RRC_INTEGRITY_PROT_ALGORITHM_SPARE3,
    LIBLTE_RRC_INTEGRITY_PROT_ALGORITHM_SPARE2,
    LIBLTE_RRC_INTEGRITY_PROT_ALGORITHM_SPARE1,
}LIBLTE_RRC_INTEGRITY_PROT_ALGORITHM_ENUM;
// Structs
typedef struct{
    LIBLTE_RRC_CIPHERING_ALGORITHM_ENUM      cipher_alg;
    LIBLTE_RRC_INTEGRITY_PROT_ALGORITHM_ENUM int_alg;
}LIBLTE_RRC_SECURITY_ALGORITHM_CONFIG_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_security_algorithm_config_ie(LIBLTE_RRC_SECURITY_ALGORITHM_CONFIG_STRUCT  *sec_alg_cnfg,
                                                               uint8                                       **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_security_algorithm_config_ie(uint8                                       **ie_ptr,
                                                                 LIBLTE_RRC_SECURITY_ALGORITHM_CONFIG_STRUCT  *sec_alg_cnfg);

/*********************************************************************
    IE Name: Short MAC I

    Description: Identifies and verifies the UE at RRC connection
                 re-establishment

    Document Reference: 36.331 v10.0.0 Section 6.3.3
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_short_mac_i_ie(uint16   short_mac_i,
                                                 uint8  **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_short_mac_i_ie(uint8  **ie_ptr,
                                                   uint16  *short_mac_i);

/*********************************************************************
    IE Name: Antenna Info

    Description: Specifies the common and the UE specific antenna
                 configuration

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: CQI Report Config

    Description: Specifies the CQI reporting configuration

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Cross Carrier Scheduling Config

    Description: Specifies the configuration when the cross carrier
                 scheduling is used in a cell

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: CSI RS Config

    Description: Specifies the CSI (Channel State Information)
                 reference signal configuration

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: DRB Identity

    Description: Identifies a DRB used by a UE

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_drb_identity_ie(uint8   drb_id,
                                                  uint8 **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_drb_identity_ie(uint8 **ie_ptr,
                                                    uint8  *drb_id);

/*********************************************************************
    IE Name: Logical Channel Config

    Description: Configures the logical channel parameters

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_RRC_PRIORITIZED_BIT_RATE_KBPS_0 = 0,
    LIBLTE_RRC_PRIORITIZED_BIT_RATE_KBPS_8,
    LIBLTE_RRC_PRIORITIZED_BIT_RATE_KBPS_16,
    LIBLTE_RRC_PRIORITIZED_BIT_RATE_KBPS_32,
    LIBLTE_RRC_PRIORITIZED_BIT_RATE_KBPS_64,
    LIBLTE_RRC_PRIORITIZED_BIT_RATE_KBPS_128,
    LIBLTE_RRC_PRIORITIZED_BIT_RATE_KBPS_256,
    LIBLTE_RRC_PRIORITIZED_BIT_RATE_INFINITY,
    LIBLTE_RRC_PRIORITIZED_BIT_RATE_SPARE8,
    LIBLTE_RRC_PRIORITIZED_BIT_RATE_SPARE7,
    LIBLTE_RRC_PRIORITIZED_BIT_RATE_SPARE6,
    LIBLTE_RRC_PRIORITIZED_BIT_RATE_SPARE5,
    LIBLTE_RRC_PRIORITIZED_BIT_RATE_SPARE4,
    LIBLTE_RRC_PRIORITIZED_BIT_RATE_SPARE3,
    LIBLTE_RRC_PRIORITIZED_BIT_RATE_SPARE2,
    LIBLTE_RRC_PRIORITIZED_BIT_RATE_SPARE1,
}LIBLTE_RRC_PRIORITIZED_BIT_RATE_ENUM;
typedef enum{
    LIBLTE_RRC_BUCKET_SIZE_DURATION_MS50 = 0,
    LIBLTE_RRC_BUCKET_SIZE_DURATION_MS100,
    LIBLTE_RRC_BUCKET_SIZE_DURATION_MS150,
    LIBLTE_RRC_BUCKET_SIZE_DURATION_MS300,
    LIBLTE_RRC_BUCKET_SIZE_DURATION_MS500,
    LIBLTE_RRC_BUCKET_SIZE_DURATION_MS1000,
    LIBLTE_RRC_BUCKET_SIZE_DURATION_SPARE2,
    LIBLTE_RRC_BUCKET_SIZE_DURATION_SPARE1,
}LIBLTE_RRC_BUCKET_SIZE_DURATION_ENUM;
typedef enum{
    LIBLTE_RRC_LOGICAL_CHANNEL_SR_MASK_R9_SETUP = 0,
}LIBLTE_RRC_LOGICAL_CHANNEL_SR_MASK_R9_ENUM;
// Structs
typedef struct{
    LIBLTE_RRC_PRIORITIZED_BIT_RATE_ENUM prioritized_bit_rate;
    LIBLTE_RRC_BUCKET_SIZE_DURATION_ENUM bucket_size_duration;
    uint8                                priority;
    uint8                                log_chan_group;
    bool                                 log_chan_group_present;
}LIBLTE_RRC_UL_SPECIFIC_PARAMETERS_STRUCT;
typedef struct{
    LIBLTE_RRC_UL_SPECIFIC_PARAMETERS_STRUCT   ul_specific_params;
    LIBLTE_RRC_LOGICAL_CHANNEL_SR_MASK_R9_ENUM log_chan_sr_mask;
    bool                                       ul_specific_params_present;
    bool                                       log_chan_sr_mask_present;
}LIBLTE_RRC_LOGICAL_CHANNEL_CONFIG_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_logical_channel_config_ie(LIBLTE_RRC_LOGICAL_CHANNEL_CONFIG_STRUCT  *log_chan_cnfg,
                                                            uint8                                    **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_logical_channel_config_ie(uint8                                    **ie_ptr,
                                                              LIBLTE_RRC_LOGICAL_CHANNEL_CONFIG_STRUCT  *log_chan_cnfg);

/*********************************************************************
    IE Name: MAC Main Config

    Description: Specifies the MAC main configuration for signalling
                 and data radio bearers

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: PDCP Config

    Description: Sets the configurable PDCP parameters for data
                 radio bearers

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: PDSCH Config

    Description: Specifies the common and the UE specific PDSCH
                 configuration

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_RRC_PDSCH_CONFIG_P_A_DB_N6 = 0,
    LIBLTE_RRC_PDSCH_CONFIG_P_A_DB_N4_DOT_77,
    LIBLTE_RRC_PDSCH_CONFIG_P_A_DB_N3,
    LIBLTE_RRC_PDSCH_CONFIG_P_A_DB_N1_DOT_77,
    LIBLTE_RRC_PDSCH_CONFIG_P_A_DB_0,
    LIBLTE_RRC_PDSCH_CONFIG_P_A_DB_1,
    LIBLTE_RRC_PDSCH_CONFIG_P_A_DB_2,
    LIBLTE_RRC_PDSCH_CONFIG_P_A_DB_3,
}LIBLTE_RRC_PDSCH_CONFIG_P_A_ENUM;
// Structs
typedef struct{
    uint8 p_b;
    int8  rs_power;
}LIBLTE_RRC_PDSCH_CONFIG_COMMON_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_pdsch_config_common_ie(LIBLTE_RRC_PDSCH_CONFIG_COMMON_STRUCT  *pdsch_config,
                                                         uint8                                 **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_pdsch_config_common_ie(uint8                                 **ie_ptr,
                                                           LIBLTE_RRC_PDSCH_CONFIG_COMMON_STRUCT  *pdsch_config);
LIBLTE_ERROR_ENUM liblte_rrc_pack_pdsch_config_dedicated_ie(LIBLTE_RRC_PDSCH_CONFIG_P_A_ENUM   p_a,
                                                            uint8                            **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_pdsch_config_dedicated_ie(uint8                            **ie_ptr,
                                                              LIBLTE_RRC_PDSCH_CONFIG_P_A_ENUM  *p_a);

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
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_phich_config_ie(LIBLTE_RRC_PHICH_CONFIG_STRUCT  *phich_config,
                                                  uint8                          **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_phich_config_ie(uint8                          **ie_ptr,
                                                    LIBLTE_RRC_PHICH_CONFIG_STRUCT  *phich_config);

/*********************************************************************
    IE Name: Physical Config Dedicated

    Description: Specifies the UE specific physical channel
                 configuration

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

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
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_p_max_ie(int8    p_max,
                                           uint8 **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_p_max_ie(uint8 **ie_ptr,
                                             int8   *p_max);

/*********************************************************************
    IE Name: PRACH Config

    Description: Specifies the PRACH configuration in the system
                 information and in the mobility control information

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
// Defines
// Enums
// Structs
typedef struct{
    uint8 prach_config_index;
    uint8 zero_correlation_zone_config;
    uint8 prach_freq_offset;
    bool  high_speed_flag;
}LIBLTE_RRC_PRACH_CONFIG_INFO_STRUCT;
typedef struct{
    LIBLTE_RRC_PRACH_CONFIG_INFO_STRUCT prach_cnfg_info;
    uint16                              root_sequence_index;
}LIBLTE_RRC_PRACH_CONFIG_SIB_STRUCT;
typedef struct{
    LIBLTE_RRC_PRACH_CONFIG_INFO_STRUCT prach_cnfg_info;
    uint16                              root_sequence_index;
    bool                                prach_cnfg_info_present;
}LIBLTE_RRC_PRACH_CONFIG_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_prach_config_sib_ie(LIBLTE_RRC_PRACH_CONFIG_SIB_STRUCT  *prach_cnfg,
                                                      uint8                              **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_prach_config_sib_ie(uint8                              **ie_ptr,
                                                        LIBLTE_RRC_PRACH_CONFIG_SIB_STRUCT  *prach_cnfg);
LIBLTE_ERROR_ENUM liblte_rrc_pack_prach_config_ie(LIBLTE_RRC_PRACH_CONFIG_STRUCT  *prach_cnfg,
                                                  uint8                          **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_prach_config_ie(uint8                          **ie_ptr,
                                                    LIBLTE_RRC_PRACH_CONFIG_STRUCT  *prach_cnfg);
LIBLTE_ERROR_ENUM liblte_rrc_pack_prach_config_scell_r10_ie(uint8   prach_cnfg_idx,
                                                            uint8 **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_prach_config_scell_r10_ie(uint8 **ie_ptr,
                                                              uint8  *prach_cnfg_idx);

/*********************************************************************
    IE Name: Presence Antenna Port 1

    Description: Indicates whether all the neighboring cells use
                 antenna port 1

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_presence_antenna_port_1_ie(bool    presence_ant_port_1,
                                                             uint8 **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_presence_antenna_port_1_ie(uint8 **ie_ptr,
                                                               bool   *presence_ant_port_1);

/*********************************************************************
    IE Name: PUCCH Config

    Description: Specifies the common and the UE specific PUCCH
                 configuration

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_RRC_DELTA_PUCCH_SHIFT_DS1 = 0,
    LIBLTE_RRC_DELTA_PUCCH_SHIFT_DS2,
    LIBLTE_RRC_DELTA_PUCCH_SHIFT_DS3,
}LIBLTE_RRC_DELTA_PUCCH_SHIFT_ENUM;
// Structs
typedef struct{
    LIBLTE_RRC_DELTA_PUCCH_SHIFT_ENUM delta_pucch_shift;
    uint16                            n1_pucch_an;
    uint8                             n_rb_cqi;
    uint8                             n_cs_an;
}LIBLTE_RRC_PUCCH_CONFIG_COMMON_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_pucch_config_common_ie(LIBLTE_RRC_PUCCH_CONFIG_COMMON_STRUCT  *pucch_cnfg,
                                                         uint8                                 **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_pucch_config_common_ie(uint8                                 **ie_ptr,
                                                           LIBLTE_RRC_PUCCH_CONFIG_COMMON_STRUCT  *pucch_cnfg);
// FIXME

/*********************************************************************
    IE Name: PUSCH Config

    Description: Specifies the common and the UE specific PUSCH
                 configuration and the reference signal configuration
                 for PUSCH and PUCCH

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_RRC_HOPPING_MODE_INTER_SUBFRAME = 0,
    LIBLTE_RRC_HOPPING_MODE_INTRA_AND_INTER_SUBFRAME,
}LIBLTE_RRC_HOPPING_MODE_ENUM;
// Structs
typedef struct{
    uint8 group_assignment_pusch;
    uint8 cyclic_shift;
    bool  group_hopping_enabled;
    bool  sequence_hopping_enabled;
}LIBLTE_RRC_UL_RS_PUSCH_STRUCT;
typedef struct{
    LIBLTE_RRC_UL_RS_PUSCH_STRUCT ul_rs;
    LIBLTE_RRC_HOPPING_MODE_ENUM  hopping_mode;
    uint8                         n_sb;
    uint8                         pusch_hopping_offset;
    bool                          enable_64_qam;
}LIBLTE_RRC_PUSCH_CONFIG_COMMON_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_pusch_config_common_ie(LIBLTE_RRC_PUSCH_CONFIG_COMMON_STRUCT  *pusch_cnfg,
                                                         uint8                                 **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_pusch_config_common_ie(uint8                                 **ie_ptr,
                                                           LIBLTE_RRC_PUSCH_CONFIG_COMMON_STRUCT  *pusch_cnfg);
// FIXME

/*********************************************************************
    IE Name: RACH Config Common

    Description: Specifies the generic random access parameters

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N4 = 0,
    LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N8,
    LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N12,
    LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N16,
    LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N20,
    LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N24,
    LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N28,
    LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N32,
    LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N36,
    LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N40,
    LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N44,
    LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N48,
    LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N52,
    LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N56,
    LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N60,
    LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N64,
}LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_ENUM;
typedef enum{
    LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_N4 = 0,
    LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_N8,
    LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_N12,
    LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_N16,
    LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_N20,
    LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_N24,
    LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_N28,
    LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_N32,
    LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_N36,
    LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_N40,
    LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_N44,
    LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_N48,
    LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_N52,
    LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_N56,
    LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_N60,
}LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_ENUM;
typedef enum{
    LIBLTE_RRC_MESSAGE_SIZE_GROUP_A_B56 = 0,
    LIBLTE_RRC_MESSAGE_SIZE_GROUP_A_B144,
    LIBLTE_RRC_MESSAGE_SIZE_GROUP_A_B208,
    LIBLTE_RRC_MESSAGE_SIZE_GROUP_A_B256,
}LIBLTE_RRC_MESSAGE_SIZE_GROUP_A_ENUM;
typedef enum{
    LIBLTE_RRC_MESSAGE_POWER_OFFSET_GROUP_B_MINUS_INFINITY = 0,
    LIBLTE_RRC_MESSAGE_POWER_OFFSET_GROUP_B_DB0,
    LIBLTE_RRC_MESSAGE_POWER_OFFSET_GROUP_B_DB5,
    LIBLTE_RRC_MESSAGE_POWER_OFFSET_GROUP_B_DB8,
    LIBLTE_RRC_MESSAGE_POWER_OFFSET_GROUP_B_DB10,
    LIBLTE_RRC_MESSAGE_POWER_OFFSET_GROUP_B_DB12,
    LIBLTE_RRC_MESSAGE_POWER_OFFSET_GROUP_B_DB15,
    LIBLTE_RRC_MESSAGE_POWER_OFFSET_GROUP_B_DB18,
}LIBLTE_RRC_MESSAGE_POWER_OFFSET_GROUP_B_ENUM;
typedef enum{
    LIBLTE_RRC_POWER_RAMPING_STEP_DB0 = 0,
    LIBLTE_RRC_POWER_RAMPING_STEP_DB2,
    LIBLTE_RRC_POWER_RAMPING_STEP_DB4,
    LIBLTE_RRC_POWER_RAMPING_STEP_DB6,
}LIBLTE_RRC_POWER_RAMPING_STEP_ENUM;
typedef enum{
    LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N120 = 0,
    LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N118,
    LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N116,
    LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N114,
    LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N112,
    LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N110,
    LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N108,
    LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N106,
    LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N104,
    LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N102,
    LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N100,
    LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N98,
    LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N96,
    LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N94,
    LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N92,
    LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N90,
}LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_ENUM;
typedef enum{
    LIBLTE_RRC_PREAMBLE_TRANS_MAX_N3 = 0,
    LIBLTE_RRC_PREAMBLE_TRANS_MAX_N4,
    LIBLTE_RRC_PREAMBLE_TRANS_MAX_N5,
    LIBLTE_RRC_PREAMBLE_TRANS_MAX_N6,
    LIBLTE_RRC_PREAMBLE_TRANS_MAX_N7,
    LIBLTE_RRC_PREAMBLE_TRANS_MAX_N8,
    LIBLTE_RRC_PREAMBLE_TRANS_MAX_N10,
    LIBLTE_RRC_PREAMBLE_TRANS_MAX_N20,
    LIBLTE_RRC_PREAMBLE_TRANS_MAX_N50,
    LIBLTE_RRC_PREAMBLE_TRANS_MAX_N100,
    LIBLTE_RRC_PREAMBLE_TRANS_MAX_N200,
}LIBLTE_RRC_PREAMBLE_TRANS_MAX_ENUM;
typedef enum{
    LIBLTE_RRC_RA_RESPONSE_WINDOW_SIZE_SF2 = 0,
    LIBLTE_RRC_RA_RESPONSE_WINDOW_SIZE_SF3,
    LIBLTE_RRC_RA_RESPONSE_WINDOW_SIZE_SF4,
    LIBLTE_RRC_RA_RESPONSE_WINDOW_SIZE_SF5,
    LIBLTE_RRC_RA_RESPONSE_WINDOW_SIZE_SF6,
    LIBLTE_RRC_RA_RESPONSE_WINDOW_SIZE_SF7,
    LIBLTE_RRC_RA_RESPONSE_WINDOW_SIZE_SF8,
    LIBLTE_RRC_RA_RESPONSE_WINDOW_SIZE_SF10,
}LIBLTE_RRC_RA_RESPONSE_WINDOW_SIZE_ENUM;
typedef enum{
    LIBLTE_RRC_MAC_CONTENTION_RESOLUTION_TIMER_SF8 = 0,
    LIBLTE_RRC_MAC_CONTENTION_RESOLUTION_TIMER_SF16,
    LIBLTE_RRC_MAC_CONTENTION_RESOLUTION_TIMER_SF24,
    LIBLTE_RRC_MAC_CONTENTION_RESOLUTION_TIMER_SF32,
    LIBLTE_RRC_MAC_CONTENTION_RESOLUTION_TIMER_SF40,
    LIBLTE_RRC_MAC_CONTENTION_RESOLUTION_TIMER_SF48,
    LIBLTE_RRC_MAC_CONTENTION_RESOLUTION_TIMER_SF56,
    LIBLTE_RRC_MAC_CONTENTION_RESOLUTION_TIMER_SF64,
}LIBLTE_RRC_MAC_CONTENTION_RESOLUTION_TIMER_ENUM;
// Structs
typedef struct{
    LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_ENUM size_of_ra;
    LIBLTE_RRC_MESSAGE_SIZE_GROUP_A_ENUM         msg_size;
    LIBLTE_RRC_MESSAGE_POWER_OFFSET_GROUP_B_ENUM msg_pwr_offset_group_b;
    bool                                         present;
}LIBLTE_RRC_PREAMBLES_GROUP_A_STRUCT;
typedef struct{
    LIBLTE_RRC_PREAMBLES_GROUP_A_STRUCT                    preambles_group_a_cnfg;
    LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_ENUM                 num_ra_preambles;
    LIBLTE_RRC_POWER_RAMPING_STEP_ENUM                     pwr_ramping_step;
    LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_ENUM preamble_init_rx_target_pwr;
    LIBLTE_RRC_PREAMBLE_TRANS_MAX_ENUM                     preamble_trans_max;
    LIBLTE_RRC_RA_RESPONSE_WINDOW_SIZE_ENUM                ra_resp_win_size;
    LIBLTE_RRC_MAC_CONTENTION_RESOLUTION_TIMER_ENUM        mac_con_res_timer;
    uint8                                                  max_harq_msg3_tx;
}LIBLTE_RRC_RACH_CONFIG_COMMON_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_rach_config_common_ie(LIBLTE_RRC_RACH_CONFIG_COMMON_STRUCT  *rach_cnfg,
                                                        uint8                                **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_rach_config_common_ie(uint8                                **ie_ptr,
                                                          LIBLTE_RRC_RACH_CONFIG_COMMON_STRUCT  *rach_cnfg);

/*********************************************************************
    IE Name: RACH Config Dedicated

    Description: Specifies the dedicated random access parameters

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
// Defines
// Enums
// Structs
typedef struct{
    uint8 preamble_index;
    uint8 prach_mask_index;
}LIBLTE_RRC_RACH_CONFIG_DEDICATED_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_rach_config_dedicated_ie(LIBLTE_RRC_RACH_CONFIG_DEDICATED_STRUCT  *rach_cnfg,
                                                           uint8                                   **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_rach_config_dedicated_ie(uint8                                   **ie_ptr,
                                                             LIBLTE_RRC_RACH_CONFIG_DEDICATED_STRUCT  *rach_cnfg);

/*********************************************************************
    IE Name: Radio Resource Config Common

    Description: Specifies the common radio resource configurations
                 in the system information and in the mobility control
                 information, including random access parameters
                 and static physical layer parameters

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_RRC_MODIFICATION_PERIOD_COEFF_N2 = 0,
    LIBLTE_RRC_MODIFICATION_PERIOD_COEFF_N4,
    LIBLTE_RRC_MODIFICATION_PERIOD_COEFF_N8,
    LIBLTE_RRC_MODIFICATION_PERIOD_COEFF_N16,
}LIBLTE_RRC_MODIFICATION_PERIOD_COEFF_ENUM;
typedef enum{
    LIBLTE_RRC_DEFAULT_PAGING_CYCLE_RF32 = 0,
    LIBLTE_RRC_DEFAULT_PAGING_CYCLE_RF64,
    LIBLTE_RRC_DEFAULT_PAGING_CYCLE_RF128,
    LIBLTE_RRC_DEFAULT_PAGING_CYCLE_RF256,
}LIBLTE_RRC_DEFAULT_PAGING_CYCLE_ENUM;
typedef enum{
    LIBLTE_RRC_NB_FOUR_T = 0,
    LIBLTE_RRC_NB_TWO_T,
    LIBLTE_RRC_NB_ONE_T,
    LIBLTE_RRC_NB_HALF_T,
    LIBLTE_RRC_NB_QUARTER_T,
    LIBLTE_RRC_NB_ONE_EIGHTH_T,
    LIBLTE_RRC_NB_ONE_SIXTEENTH_T,
    LIBLTE_RRC_NB_ONE_THIRTY_SECOND_T,
}LIBLTE_RRC_NB_ENUM;
typedef enum{
    LIBLTE_RRC_UL_CP_LENGTH_1 = 0,
    LIBLTE_RRC_UL_CP_LENGTH_2,
}LIBLTE_RRC_UL_CP_LENGTH_ENUM;
// FIXME
typedef enum{
    LIBLTE_RRC_SRS_BW_CONFIG_0 = 0,
    LIBLTE_RRC_SRS_BW_CONFIG_1,
    LIBLTE_RRC_SRS_BW_CONFIG_2,
    LIBLTE_RRC_SRS_BW_CONFIG_3,
    LIBLTE_RRC_SRS_BW_CONFIG_4,
    LIBLTE_RRC_SRS_BW_CONFIG_5,
    LIBLTE_RRC_SRS_BW_CONFIG_6,
    LIBLTE_RRC_SRS_BW_CONFIG_7,
}LIBLTE_RRC_SRS_BW_CONFIG_ENUM;
typedef enum{
    LIBLTE_RRC_SRS_SUBFR_CONFIG_0 = 0,
    LIBLTE_RRC_SRS_SUBFR_CONFIG_1,
    LIBLTE_RRC_SRS_SUBFR_CONFIG_2,
    LIBLTE_RRC_SRS_SUBFR_CONFIG_3,
    LIBLTE_RRC_SRS_SUBFR_CONFIG_4,
    LIBLTE_RRC_SRS_SUBFR_CONFIG_5,
    LIBLTE_RRC_SRS_SUBFR_CONFIG_6,
    LIBLTE_RRC_SRS_SUBFR_CONFIG_7,
    LIBLTE_RRC_SRS_SUBFR_CONFIG_8,
    LIBLTE_RRC_SRS_SUBFR_CONFIG_9,
    LIBLTE_RRC_SRS_SUBFR_CONFIG_10,
    LIBLTE_RRC_SRS_SUBFR_CONFIG_11,
    LIBLTE_RRC_SRS_SUBFR_CONFIG_12,
    LIBLTE_RRC_SRS_SUBFR_CONFIG_13,
    LIBLTE_RRC_SRS_SUBFR_CONFIG_14,
    LIBLTE_RRC_SRS_SUBFR_CONFIG_15,
}LIBLTE_RRC_SRS_SUBFR_CONFIG_ENUM;
typedef enum{
    LIBLTE_RRC_UL_POWER_CONTROL_ALPHA_0 = 0,
    LIBLTE_RRC_UL_POWER_CONTROL_ALPHA_04,
    LIBLTE_RRC_UL_POWER_CONTROL_ALPHA_05,
    LIBLTE_RRC_UL_POWER_CONTROL_ALPHA_06,
    LIBLTE_RRC_UL_POWER_CONTROL_ALPHA_07,
    LIBLTE_RRC_UL_POWER_CONTROL_ALPHA_08,
    LIBLTE_RRC_UL_POWER_CONTROL_ALPHA_09,
    LIBLTE_RRC_UL_POWER_CONTROL_ALPHA_1,
}LIBLTE_RRC_UL_POWER_CONTROL_ALPHA_ENUM;
typedef enum{
    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_1_NEG_2 = 0,
    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_1_0,
    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_1_2,
}LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_1_ENUM;
typedef enum{
    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_1B_1 = 0,
    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_1B_3,
    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_1B_5,
}LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_1B_ENUM;
typedef enum{
    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2_NEG_2 = 0,
    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2_0,
    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2_1,
    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2_2,
}LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2_ENUM;
typedef enum{
    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2A_NEG_2 = 0,
    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2A_0,
    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2A_2,
}LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2A_ENUM;
typedef enum{
    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2B_NEG_2 = 0,
    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2B_0,
    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2B_2,
}LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2B_ENUM;
// FIXME END
// Structs
typedef struct{
    LIBLTE_RRC_MODIFICATION_PERIOD_COEFF_ENUM modification_period_coeff;
}LIBLTE_RRC_BCCH_CONFIG_STRUCT;
typedef struct{
    LIBLTE_RRC_DEFAULT_PAGING_CYCLE_ENUM default_paging_cycle;
    LIBLTE_RRC_NB_ENUM                   nB;
}LIBLTE_RRC_PCCH_CONFIG_STRUCT;
// FIXME
typedef struct{
    LIBLTE_RRC_SRS_BW_CONFIG_ENUM    bw_cnfg;
    LIBLTE_RRC_SRS_SUBFR_CONFIG_ENUM subfr_cnfg;
    bool                             ack_nack_simul_tx;
    bool                             max_up_pts;
    bool                             max_up_pts_present;
    bool                             present;
}LIBLTE_RRC_SRS_UL_CONFIG_COMMON_STRUCT;
typedef struct{
    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_1_ENUM  format_1;
    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_1B_ENUM format_1b;
    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2_ENUM  format_2;
    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2A_ENUM format_2a;
    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2B_ENUM format_2b;
}LIBLTE_RRC_DELTA_FLIST_PUCCH_STRUCT;
typedef struct{
    LIBLTE_RRC_DELTA_FLIST_PUCCH_STRUCT    delta_flist_pucch;
    LIBLTE_RRC_UL_POWER_CONTROL_ALPHA_ENUM alpha;
    int8                                   p0_nominal_pusch;
    int8                                   p0_nominal_pucch;
    int8                                   delta_preamble_msg3;
}LIBLTE_RRC_UL_POWER_CONTROL_COMMON_STRUCT;
// FIXME END
typedef struct{
    LIBLTE_RRC_RACH_CONFIG_COMMON_STRUCT      rach_cnfg;
    LIBLTE_RRC_BCCH_CONFIG_STRUCT             bcch_cnfg;
    LIBLTE_RRC_PCCH_CONFIG_STRUCT             pcch_cnfg;
    LIBLTE_RRC_PRACH_CONFIG_SIB_STRUCT        prach_cnfg;
    LIBLTE_RRC_PDSCH_CONFIG_COMMON_STRUCT     pdsch_cnfg;
    LIBLTE_RRC_PUSCH_CONFIG_COMMON_STRUCT     pusch_cnfg;
    LIBLTE_RRC_PUCCH_CONFIG_COMMON_STRUCT     pucch_cnfg;
    LIBLTE_RRC_SRS_UL_CONFIG_COMMON_STRUCT    srs_ul_cnfg;
    LIBLTE_RRC_UL_POWER_CONTROL_COMMON_STRUCT ul_pwr_ctrl;
    LIBLTE_RRC_UL_CP_LENGTH_ENUM              ul_cp_length;
}LIBLTE_RRC_RR_CONFIG_COMMON_SIB_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_rr_config_common_sib_ie(LIBLTE_RRC_RR_CONFIG_COMMON_SIB_STRUCT  *rr_cnfg,
                                                          uint8                                  **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_rr_config_common_sib_ie(uint8                                  **ie_ptr,
                                                            LIBLTE_RRC_RR_CONFIG_COMMON_SIB_STRUCT  *rr_cnfg);
// FIXME

/*********************************************************************
    IE Name: Radio Resource Config Dedicated

    Description: Sets up/Modifies/Releases RBs, modifies the MAC
                 main configuration, modifies the SPS configuration
                 and modifies dedicated physical configuration

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: RLC Config

    Description: Specifies the RLC configuration of SRBs and DRBs

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: RLF Timers and Constants

    Description: Contains UE specific timers and constants applicable
                 for UEs in RRC_CONNECTED

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: RN Subframe Config

    Description: Specifies the subframe configuration for an RN

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Scheduling Request Config

    Description: Specifies the scheduling request related parameters

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Sounding RS UL Config

    Description: Specifies the uplink Sounding RS configuration for
                 periodic and aperiodic sounding

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
// Defines
// Enums
// FIXME
//typedef enum{
//    LIBLTE_RRC_SRS_BW_CONFIG_0 = 0,
//    LIBLTE_RRC_SRS_BW_CONFIG_1,
//    LIBLTE_RRC_SRS_BW_CONFIG_2,
//    LIBLTE_RRC_SRS_BW_CONFIG_3,
//    LIBLTE_RRC_SRS_BW_CONFIG_4,
//    LIBLTE_RRC_SRS_BW_CONFIG_5,
//    LIBLTE_RRC_SRS_BW_CONFIG_6,
//    LIBLTE_RRC_SRS_BW_CONFIG_7,
//}LIBLTE_RRC_SRS_BW_CONFIG_ENUM;
//typedef enum{
//    LIBLTE_RRC_SRS_SUBFR_CONFIG_0 = 0,
//    LIBLTE_RRC_SRS_SUBFR_CONFIG_1,
//    LIBLTE_RRC_SRS_SUBFR_CONFIG_2,
//    LIBLTE_RRC_SRS_SUBFR_CONFIG_3,
//    LIBLTE_RRC_SRS_SUBFR_CONFIG_4,
//    LIBLTE_RRC_SRS_SUBFR_CONFIG_5,
//    LIBLTE_RRC_SRS_SUBFR_CONFIG_6,
//    LIBLTE_RRC_SRS_SUBFR_CONFIG_7,
//    LIBLTE_RRC_SRS_SUBFR_CONFIG_8,
//    LIBLTE_RRC_SRS_SUBFR_CONFIG_9,
//    LIBLTE_RRC_SRS_SUBFR_CONFIG_10,
//    LIBLTE_RRC_SRS_SUBFR_CONFIG_11,
//    LIBLTE_RRC_SRS_SUBFR_CONFIG_12,
//    LIBLTE_RRC_SRS_SUBFR_CONFIG_13,
//    LIBLTE_RRC_SRS_SUBFR_CONFIG_14,
//    LIBLTE_RRC_SRS_SUBFR_CONFIG_15,
//}LIBLTE_RRC_SRS_SUBFR_CONFIG_ENUM;
// FIXME END
// Structs
// FIXME
//typedef struct{
//    LIBLTE_RRC_SRS_BW_CONFIG_ENUM    bw_cnfg;
//    LIBLTE_RRC_SRS_SUBFR_CONFIG_ENUM subfr_cnfg;
//    bool                             ack_nack_simul_tx;
//    bool                             max_up_pts;
//    bool                             max_up_pts_present;
//    bool                             present;
//}LIBLTE_RRC_SRS_UL_CONFIG_COMMON_STRUCT;
// FIXME END
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_srs_ul_config_common_ie(LIBLTE_RRC_SRS_UL_CONFIG_COMMON_STRUCT  *srs_ul_cnfg,
                                                          uint8                                  **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_srs_ul_config_common_ie(uint8                                  **ie_ptr,
                                                            LIBLTE_RRC_SRS_UL_CONFIG_COMMON_STRUCT  *srs_ul_cnfg);
// FIXME

/*********************************************************************
    IE Name: SPS Config

    Description: Specifies the semi-persistent scheduling
                 configuration

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

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
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_tdd_config_ie(LIBLTE_RRC_SUBFRAME_ASSIGNMENT_ENUM         sa,
                                                LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_ENUM   ssp,
                                                uint8                                     **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_tdd_config_ie(uint8                                     **ie_ptr,
                                                  LIBLTE_RRC_SUBFRAME_ASSIGNMENT_ENUM        *sa,
                                                  LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_ENUM  *ssp);

/*********************************************************************
    IE Name: Time Alignment Timer

    Description: Controls how long the UE is considered uplink time
                 aligned

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_RRC_TIME_ALIGNMENT_TIMER_SF500 = 0,
    LIBLTE_RRC_TIME_ALIGNMENT_TIMER_SF750,
    LIBLTE_RRC_TIME_ALIGNMENT_TIMER_SF1280,
    LIBLTE_RRC_TIME_ALIGNMENT_TIMER_SF1920,
    LIBLTE_RRC_TIME_ALIGNMENT_TIMER_SF2560,
    LIBLTE_RRC_TIME_ALIGNMENT_TIMER_SF5120,
    LIBLTE_RRC_TIME_ALIGNMENT_TIMER_SF10240,
    LIBLTE_RRC_TIME_ALIGNMENT_TIMER_INFINITY,
}LIBLTE_RRC_TIME_ALIGNMENT_TIMER_ENUM;
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_time_alignment_timer_ie(LIBLTE_RRC_TIME_ALIGNMENT_TIMER_ENUM   time_alignment_timer,
                                                          uint8                                **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_time_alignment_timer_ie(uint8                                **ie_ptr,
                                                            LIBLTE_RRC_TIME_ALIGNMENT_TIMER_ENUM  *time_alignment_timer);

/*********************************************************************
    IE Name: TPC PDCCH Config

    Description: Specifies the RNTIs and indecies for PUCCH and PUSCH
                 power control

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: UL Antenna Info

    Description: Specifies the UL antenna configuration

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_RRC_UL_TRANSMISSION_MODE_R10_TM1 = 0,
    LIBLTE_RRC_UL_TRANSMISSION_MODE_R10_TM2,
    LIBLTE_RRC_UL_TRANSMISSION_MODE_R10_SPARE6,
    LIBLTE_RRC_UL_TRANSMISSION_MODE_R10_SPARE5,
    LIBLTE_RRC_UL_TRANSMISSION_MODE_R10_SPARE4,
    LIBLTE_RRC_UL_TRANSMISSION_MODE_R10_SPARE3,
    LIBLTE_RRC_UL_TRANSMISSION_MODE_R10_SPARE2,
    LIBLTE_RRC_UL_TRANSMISSION_MODE_R10_SPARE1,
}LIBLTE_RRC_UL_TRANSMISSION_MODE_R10_ENUM;
// Structs
typedef struct{
    LIBLTE_RRC_UL_TRANSMISSION_MODE_R10_ENUM ul_tx_mode;
    bool                                     four_ant_port_activated;
}LIBLTE_RRC_UL_ANTENNA_INFO_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_ul_antenna_info_ie(LIBLTE_RRC_UL_ANTENNA_INFO_STRUCT  *ul_ant_info,
                                                     uint8                             **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_ul_antenna_info_ie(uint8                             **ie_ptr,
                                                       LIBLTE_RRC_UL_ANTENNA_INFO_STRUCT  *ul_ant_info);

/*********************************************************************
    IE Name: Uplink Power Control

    Description: Specifies the parameters for uplink power control in
                 the system information and in the dedicated
                 signalling

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
// Defines
// Enums
// FIXME
//typedef enum{
//    LIBLTE_RRC_UL_POWER_CONTROL_ALPHA_0 = 0,
//    LIBLTE_RRC_UL_POWER_CONTROL_ALPHA_04,
//    LIBLTE_RRC_UL_POWER_CONTROL_ALPHA_05,
//    LIBLTE_RRC_UL_POWER_CONTROL_ALPHA_06,
//    LIBLTE_RRC_UL_POWER_CONTROL_ALPHA_07,
//    LIBLTE_RRC_UL_POWER_CONTROL_ALPHA_08,
//    LIBLTE_RRC_UL_POWER_CONTROL_ALPHA_09,
//    LIBLTE_RRC_UL_POWER_CONTROL_ALPHA_1,
//}LIBLTE_RRC_UL_POWER_CONTROL_ALPHA_ENUM;
//typedef enum{
//    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_1_NEG_2 = 0,
//    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_1_0,
//    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_1_2,
//}LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_1_ENUM;
//typedef enum{
//    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_1B_1 = 0,
//    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_1B_3,
//    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_1B_5,
//}LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_1B_ENUM;
//typedef enum{
//    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2_NEG_2 = 0,
//    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2_0,
//    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2_1,
//    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2_2,
//}LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2_ENUM;
//typedef enum{
//    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2A_NEG_2 = 0,
//    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2A_0,
//    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2A_2,
//}LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2A_ENUM;
//typedef enum{
//    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2B_NEG_2 = 0,
//    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2B_0,
//    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2B_2,
//}LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2B_ENUM;
// FIXME END
// Structs
// FIXME
//typedef struct{
//    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_1_ENUM  format_1;
//    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_1B_ENUM format_1b;
//    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2_ENUM  format_2;
//    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2A_ENUM format_2a;
//    LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2B_ENUM format_2b;
//}LIBLTE_RRC_DELTA_FLIST_PUCCH_STRUCT;
//typedef struct{
//    LIBLTE_RRC_DELTA_FLIST_PUCCH_STRUCT    delta_flist_pucch;
//    LIBLTE_RRC_UL_POWER_CONTROL_ALPHA_ENUM alpha;
//    int8                                   p0_nominal_pusch;
//    int8                                   p0_nominal_pucch;
//    int8                                   delta_preamble_msg3;
//}LIBLTE_RRC_UL_POWER_CONTROL_COMMON_STRUCT;
// FIXME END
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_ul_power_control_common_ie(LIBLTE_RRC_UL_POWER_CONTROL_COMMON_STRUCT  *ul_pwr_ctrl,
                                                             uint8                                     **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_ul_power_control_common_ie(uint8                                     **ie_ptr,
                                                               LIBLTE_RRC_UL_POWER_CONTROL_COMMON_STRUCT  *ul_pwr_ctrl);
// FIXME

/*********************************************************************
    IE Name: System Information Block Type 2

    Description: Contains radio resource configuration that is common
                 for all UEs

    Document Reference: 36.331 v10.0.0 Section 6.3.1
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_RRC_AC_BARRING_FACTOR_P00 = 0,
    LIBLTE_RRC_AC_BARRING_FACTOR_P05,
    LIBLTE_RRC_AC_BARRING_FACTOR_P10,
    LIBLTE_RRC_AC_BARRING_FACTOR_P15,
    LIBLTE_RRC_AC_BARRING_FACTOR_P20,
    LIBLTE_RRC_AC_BARRING_FACTOR_P25,
    LIBLTE_RRC_AC_BARRING_FACTOR_P30,
    LIBLTE_RRC_AC_BARRING_FACTOR_P40,
    LIBLTE_RRC_AC_BARRING_FACTOR_P50,
    LIBLTE_RRC_AC_BARRING_FACTOR_P60,
    LIBLTE_RRC_AC_BARRING_FACTOR_P70,
    LIBLTE_RRC_AC_BARRING_FACTOR_P75,
    LIBLTE_RRC_AC_BARRING_FACTOR_P80,
    LIBLTE_RRC_AC_BARRING_FACTOR_P85,
    LIBLTE_RRC_AC_BARRING_FACTOR_P90,
    LIBLTE_RRC_AC_BARRING_FACTOR_P95,
}LIBLTE_RRC_AC_BARRING_FACTOR_ENUM;
typedef enum{
    LIBLTE_RRC_AC_BARRING_TIME_S4 = 0,
    LIBLTE_RRC_AC_BARRING_TIME_S8,
    LIBLTE_RRC_AC_BARRING_TIME_S16,
    LIBLTE_RRC_AC_BARRING_TIME_S32,
    LIBLTE_RRC_AC_BARRING_TIME_S64,
    LIBLTE_RRC_AC_BARRING_TIME_S128,
    LIBLTE_RRC_AC_BARRING_TIME_S256,
    LIBLTE_RRC_AC_BARRING_TIME_S512,
}LIBLTE_RRC_AC_BARRING_TIME_ENUM;
typedef enum{
    LIBLTE_RRC_UL_BW_N6 = 0,
    LIBLTE_RRC_UL_BW_N15,
    LIBLTE_RRC_UL_BW_N25,
    LIBLTE_RRC_UL_BW_N50,
    LIBLTE_RRC_UL_BW_N75,
    LIBLTE_RRC_UL_BW_N100,
}LIBLTE_RRC_UL_BW_ENUM;
// Structs
typedef struct{
    LIBLTE_RRC_AC_BARRING_FACTOR_ENUM factor;
    LIBLTE_RRC_AC_BARRING_TIME_ENUM   time;
    uint8                             for_special_ac;
    bool                              enabled;
}LIBLTE_RRC_AC_BARRING_CONFIG_STRUCT;
typedef struct{
    uint16 value;
    bool   present;
}LIBLTE_RRC_ARFCN_VALUE_EUTRA_STRUCT;
typedef struct{
    LIBLTE_RRC_UL_BW_ENUM bw;
    bool                  present;
}LIBLTE_RRC_UL_BW_STRUCT;
typedef struct{
    LIBLTE_RRC_AC_BARRING_CONFIG_STRUCT       ac_barring_for_mo_signalling;
    LIBLTE_RRC_AC_BARRING_CONFIG_STRUCT       ac_barring_for_mo_data;
    LIBLTE_RRC_RR_CONFIG_COMMON_SIB_STRUCT    rr_config_common_sib;
    LIBLTE_RRC_UE_TIMERS_AND_CONSTANTS_STRUCT ue_timers_and_constants;
    LIBLTE_RRC_ARFCN_VALUE_EUTRA_STRUCT       arfcn_value_eutra;
    LIBLTE_RRC_UL_BW_STRUCT                   ul_bw;
    LIBLTE_RRC_MBSFN_SUBFRAME_CONFIG_STRUCT   mbsfn_subfr_cnfg[LIBLTE_RRC_MAX_MBSFN_ALLOCATIONS];
    LIBLTE_RRC_TIME_ALIGNMENT_TIMER_ENUM      time_alignment_timer;
    uint32                                    mbsfn_subfr_cnfg_list_size;
    uint8                                     additional_spectrum_emission;
    bool                                      ac_barring_for_emergency;
    bool                                      ac_barring_info_present;
}LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_2_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_sys_info_block_type_2_ie(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_2_STRUCT  *sib2,
                                                           uint8                                   **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_sys_info_block_type_2_ie(uint8                                   **ie_ptr,
                                                             LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_2_STRUCT  *sib2);

/*********************************************************************
    IE Name: System Information Block Type 3

    Description: Contains cell reselection information common for
                 intra-frequency, inter-frequency, and/or inter-RAT
                 cell re-selection as well as intra-frequency cell
                 re-selection information other than neighboring
                 cell related

    Document Reference: 36.331 v10.0.0 Section 6.3.1
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_RRC_Q_HYST_DB_0 = 0,
    LIBLTE_RRC_Q_HYST_DB_1,
    LIBLTE_RRC_Q_HYST_DB_2,
    LIBLTE_RRC_Q_HYST_DB_3,
    LIBLTE_RRC_Q_HYST_DB_4,
    LIBLTE_RRC_Q_HYST_DB_5,
    LIBLTE_RRC_Q_HYST_DB_6,
    LIBLTE_RRC_Q_HYST_DB_8,
    LIBLTE_RRC_Q_HYST_DB_10,
    LIBLTE_RRC_Q_HYST_DB_12,
    LIBLTE_RRC_Q_HYST_DB_14,
    LIBLTE_RRC_Q_HYST_DB_16,
    LIBLTE_RRC_Q_HYST_DB_18,
    LIBLTE_RRC_Q_HYST_DB_20,
    LIBLTE_RRC_Q_HYST_DB_22,
    LIBLTE_RRC_Q_HYST_DB_24,
    LIBLTE_RRC_Q_HYST_SIZE,
}LIBLTE_RRC_Q_HYST_ENUM;
static const char liblte_rrc_q_hyst_text[LIBLTE_RRC_Q_HYST_SIZE][10] = { "0",  "1",  "2",  "3",
                                                                         "4",  "5",  "6",  "8",
                                                                        "10", "12", "14", "16",
                                                                        "18", "20", "22", "24"};
typedef enum{
    LIBLTE_RRC_SF_MEDIUM_DB_N6 = 0,
    LIBLTE_RRC_SF_MEDIUM_DB_N4,
    LIBLTE_RRC_SF_MEDIUM_DB_N2,
    LIBLTE_RRC_SF_MEDIUM_DB_0,
}LIBLTE_RRC_SF_MEDIUM_ENUM;
typedef enum{
    LIBLTE_RRC_SF_HIGH_DB_N6 = 0,
    LIBLTE_RRC_SF_HIGH_DB_N4,
    LIBLTE_RRC_SF_HIGH_DB_N2,
    LIBLTE_RRC_SF_HIGH_DB_0,
}LIBLTE_RRC_SF_HIGH_ENUM;
// Structs
typedef struct{
    LIBLTE_RRC_SF_MEDIUM_ENUM medium;
    LIBLTE_RRC_SF_HIGH_ENUM   high;
}LIBLTE_RRC_Q_HYST_SF_STRUCT;
typedef struct{
    LIBLTE_RRC_MOBILITY_STATE_PARAMETERS_STRUCT mobility_state_params;
    LIBLTE_RRC_Q_HYST_SF_STRUCT                 q_hyst_sf;
    bool                                        present;
}LIBLTE_RRC_SPEED_STATE_RESELECTION_PARS_STRUCT;
typedef struct{
    LIBLTE_RRC_SPEED_STATE_RESELECTION_PARS_STRUCT speed_state_resel_params;
    LIBLTE_RRC_SPEED_STATE_SCALE_FACTORS_STRUCT    t_resel_eutra_sf;
    LIBLTE_RRC_Q_HYST_ENUM                         q_hyst;
    LIBLTE_RRC_ALLOWED_MEAS_BANDWIDTH_ENUM         allowed_meas_bw;
    int16                                          q_rx_lev_min;
    uint8                                          s_non_intra_search;
    uint8                                          thresh_serving_low;
    uint8                                          cell_resel_prio;
    uint8                                          s_intra_search;
    uint8                                          neigh_cell_cnfg;
    uint8                                          t_resel_eutra;
    int8                                           p_max;
    bool                                           s_non_intra_search_present;
    bool                                           presence_ant_port_1;
    bool                                           p_max_present;
    bool                                           s_intra_search_present;
    bool                                           allowed_meas_bw_present;
    bool                                           t_resel_eutra_sf_present;
}LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_3_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_sys_info_block_type_3_ie(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_3_STRUCT  *sib3,
                                                           uint8                                   **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_sys_info_block_type_3_ie(uint8                                   **ie_ptr,
                                                             LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_3_STRUCT  *sib3);

/*********************************************************************
    IE Name: System Information Block Type 4

    Description: Contains the neighboring cell related information
                 relevant only for intra-frequency cell reselection

    Document Reference: 36.331 v10.0.0 Section 6.3.1
*********************************************************************/
// Defines
#define LIBLTE_RRC_MAX_CELL_INTRA 16
#define LIBLTE_RRC_MAX_CELL_BLACK 16
// Enums
// Structs
typedef struct{
    LIBLTE_RRC_Q_OFFSET_RANGE_ENUM q_offset_range;
    uint16                         phys_cell_id;
}LIBLTE_RRC_INTRA_FREQ_NEIGH_CELL_INFO_STRUCT;
typedef struct{
    LIBLTE_RRC_INTRA_FREQ_NEIGH_CELL_INFO_STRUCT intra_freq_neigh_cell_list[LIBLTE_RRC_MAX_CELL_INTRA];
    LIBLTE_RRC_PHYS_CELL_ID_RANGE_STRUCT         intra_freq_black_cell_list[LIBLTE_RRC_MAX_CELL_BLACK];
    LIBLTE_RRC_PHYS_CELL_ID_RANGE_STRUCT         csg_phys_cell_id_range;
    uint32                                       intra_freq_neigh_cell_list_size;
    uint32                                       intra_freq_black_cell_list_size;
    bool                                         csg_phys_cell_id_range_present;
}LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_4_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_sys_info_block_type_4_ie(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_4_STRUCT  *sib4,
                                                           uint8                                   **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_sys_info_block_type_4_ie(uint8                                   **ie_ptr,
                                                             LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_4_STRUCT  *sib4);

/*********************************************************************
    IE Name: System Information Block Type 5

    Description: Contains information relevant only for
                 inter-frequency cell reselection, i.e. information
                 about other E-UTRA frequencies and inter-frequency
                 neighboring cells relevant for cell reselection

    Document Reference: 36.331 v10.0.0 Section 6.3.1
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: System Information Block Type 6

    Description: Contains information relevant only for inter-RAT
                 cell reselection, i.e. information about UTRA
                 frequencies and UTRA neighboring cells relevant for
                 cell reselection

    Document Reference: 36.331 v10.0.0 Section 6.3.1
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: System Information Block Type 7

    Description: Contains information relevant only for inter-RAT
                 cell reselection, i.e. information about GERAN
                 frequencies relevant for cell reselection

    Document Reference: 36.331 v10.0.0 Section 6.3.1
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: System Information Block Type 8

    Description: Contains information relevant only for inter-RAT
                 cell re-selection i.e. information about CDMA2000
                 frequencies and CDMA2000 neighboring cells relevant
                 for cell re-selection

    Document Reference: 36.331 v10.0.0 Section 6.3.1
*********************************************************************/
// Defines
#define LIBLTE_RRC_MAX_CDMA_BAND_CLASS 32
// Enums
// Structs
typedef struct{
    LIBLTE_RRC_BAND_CLASS_CDMA2000_ENUM band_class;
    uint8                               cell_resel_prio;
    uint8                               thresh_x_high;
    uint8                               thresh_x_low;
    bool                                cell_resel_prio_present;
}LIBLTE_RRC_BAND_CLASS_INFO_CDMA2000_STRUCT;
typedef struct{
    uint16 arfcn;
    uint16 phys_cell_id_list[16];
    uint8  phys_cell_id_list_size;
}LIBLTE_RRC_NEIGH_CELLS_PER_BAND_CLASS_CDMA2000_STRUCT;
typedef struct{
    LIBLTE_RRC_BAND_CLASS_CDMA2000_ENUM                   band_class;
    LIBLTE_RRC_NEIGH_CELLS_PER_BAND_CLASS_CDMA2000_STRUCT neigh_cells_per_freq_list[16];
    uint8                                                 neigh_cells_per_freq_list_size;
}LIBLTE_RRC_NEIGH_CELL_CDMA2000_STRUCT;
typedef struct{
    LIBLTE_RRC_BAND_CLASS_INFO_CDMA2000_STRUCT  band_class_list[LIBLTE_RRC_MAX_CDMA_BAND_CLASS];
    LIBLTE_RRC_NEIGH_CELL_CDMA2000_STRUCT       neigh_cell_list[16];
    LIBLTE_RRC_SPEED_STATE_SCALE_FACTORS_STRUCT t_resel_cdma2000_sf;
    uint8                                       band_class_list_size;
    uint8                                       neigh_cell_list_size;
    uint8                                       t_resel_cdma2000;
    bool                                        t_resel_cdma2000_sf_present;
}LIBLTE_RRC_CELL_RESELECTION_PARAMS_CDMA2000_STRUCT;
typedef struct{
    LIBLTE_RRC_SYSTEM_TIME_INFO_CDMA2000_STRUCT        sys_time_info_cdma2000;
    LIBLTE_RRC_PRE_REGISTRATION_INFO_HRPD_STRUCT       pre_reg_info_hrpd;
    LIBLTE_RRC_CELL_RESELECTION_PARAMS_CDMA2000_STRUCT cell_resel_params_hrpd;
    LIBLTE_RRC_CSFB_REGISTRATION_PARAM_1XRTT_STRUCT    csfb_reg_param_1xrtt;
    LIBLTE_RRC_CELL_RESELECTION_PARAMS_CDMA2000_STRUCT cell_resel_params_1xrtt;
    uint64                                             long_code_state_1xrtt;
    uint8                                              search_win_size;
    bool                                               sys_time_info_present;
    bool                                               search_win_size_present;
    bool                                               params_hrpd_present;
    bool                                               cell_resel_params_hrpd_present;
    bool                                               params_1xrtt_present;
    bool                                               csfb_reg_param_1xrtt_present;
    bool                                               long_code_state_1xrtt_present;
    bool                                               cell_resel_params_1xrtt_present;
}LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_8_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_sys_info_block_type_8_ie(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_8_STRUCT  *sib8,
                                                           uint8                                   **ie_ptr);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_sys_info_block_type_8_ie(uint8                                   **ie_ptr,
                                                             LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_8_STRUCT  *sib8);

/*********************************************************************
    IE Name: System Information Block Type 9

    Description: Contains a home eNB name (HNB name)

    Document Reference: 36.331 v10.0.0 Section 6.3.1
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: System Information Block Type 10

    Description: Contains an ETWS primary notification

    Document Reference: 36.331 v10.0.0 Section 6.3.1
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: System Information Block Type 11

    Description: Contains an ETWS secondary notification

    Document Reference: 36.331 v10.0.0 Section 6.3.1
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: System Information Block Type 12

    Description: Contains a CMAS notification

    Document Reference: 36.331 v10.0.0 Section 6.3.1
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: System Information Block Type 13

    Description: Contains the information required to acquire the
                 MBMS control information associated with one or more
                 MBSFN areas

    Document Reference: 36.331 v10.0.0 Section 6.3.1
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

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
    bool                                      p_max_present;
}LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_sys_info_block_type_1_msg(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1_STRUCT *sib1,
                                                            LIBLTE_RRC_MSG_STRUCT                   *msg);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_sys_info_block_type_1_msg(LIBLTE_RRC_MSG_STRUCT                   *msg,
                                                              LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1_STRUCT *sib1,
                                                              uint32                                  *N_bits_used);

/*********************************************************************
    Message Name: System Information

    Description: Conveys one or more System Information Blocks

    Document Reference: 36.331 v10.0.0 Section 6.2.2
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_2 = 0,
    LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_3,
    LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_4,
    LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_5,
    LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_6,
    LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_7,
    LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_8,
    LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_9,
    LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_10,
    LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_11,
    LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_12,
    LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_13,
    LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1, // Intentionally not first
}LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_ENUM;
// Structs
typedef union{
    LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1_STRUCT sib1;
    LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_2_STRUCT sib2;
    LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_3_STRUCT sib3;
    LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_4_STRUCT sib4;
    LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_8_STRUCT sib8;
}LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_UNION;
typedef struct{
    LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_UNION sib;
    LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_ENUM  sib_type;
}LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_STRUCT;
typedef struct{
    LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_STRUCT sibs[LIBLTE_RRC_MAX_SIB];
    uint32                                N_sibs;
}LIBLTE_RRC_SYS_INFO_MSG_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_sys_info_msg(LIBLTE_RRC_SYS_INFO_MSG_STRUCT *sibs,
                                               LIBLTE_RRC_MSG_STRUCT          *msg);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_sys_info_msg(LIBLTE_RRC_MSG_STRUCT          *msg,
                                                 LIBLTE_RRC_SYS_INFO_MSG_STRUCT *sibs);

/*********************************************************************
    Message Name: BCCH BCH Message

    Description: Contains the set of RRC messages that may be sent
                 from the E-UTRAN to the UE via BCH on the BCCH
                 logical channel

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
                 logical channel

    Document Reference: 36.331 v10.0.0 Section 6.2.1
*********************************************************************/
// Defines
// Enums
// Structs
typedef LIBLTE_RRC_SYS_INFO_MSG_STRUCT LIBLTE_RRC_BCCH_DLSCH_MSG_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_rrc_pack_bcch_dlsch_msg(LIBLTE_RRC_BCCH_DLSCH_MSG_STRUCT *bcch_dlsch_msg,
                                                 LIBLTE_RRC_MSG_STRUCT            *msg);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_bcch_dlsch_msg(LIBLTE_RRC_MSG_STRUCT            *msg,
                                                   LIBLTE_RRC_BCCH_DLSCH_MSG_STRUCT *bcch_dlsch_msg);

#endif /* __LIBLTE_RRC_H__ */
