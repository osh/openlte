/*******************************************************************************

    Copyright 2013 Ben Wojtowicz

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

    File: liblte_mac.h

    Description: Contains all the definitions for the LTE Medium Access Control
                 Layer library.

    Revision History
    ----------    -------------    --------------------------------------------
    07/21/2013    Ben Wojtowicz    Created file.

*******************************************************************************/

#ifndef __LIBLTE_MAC_H__
#define __LIBLTE_MAC_H__

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "liblte_common.h"

/*******************************************************************************
                              DEFINES
*******************************************************************************/

// RNTIs 36.321 v10.2.0 Section 7.1
#define LIBLTE_MAC_INVALID_RNTI    0x0000
#define LIBLTE_MAC_RA_RNTI_START   0x0001
#define LIBLTE_MAC_RA_RNTI_END     0x003C
#define LIBLTE_MAC_C_RNTI_START    0x003D
#define LIBLTE_MAC_C_RNTI_END      0xFFF3
#define LIBLTE_MAC_RESV_RNTI_START 0xFFF4
#define LIBLTE_MAC_RESV_RNTI_END   0xFFFC
#define LIBLTE_MAC_M_RNTI          0xFFFD
#define LIBLTE_MAC_P_RNTI          0xFFFE
#define LIBLTE_MAC_SI_RNTI         0xFFFF

/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/


/*******************************************************************************
                              PDU DECLARATIONS
*******************************************************************************/

/*********************************************************************
    PDU Name: DL-SCH and UL-SCH

    Description: PDU containing a MAC header, zero or more MAC SDUs,
                 and zero or more MAC control elements

    Document Reference: 36.321 v10.2.0 Section 6.1.2
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    PDU Name: Transparent

    Description: PDU containing a MAC SDU

    Document Reference: 36.321 v10.2.0 Section 6.1.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    PDU Name: Random Access Response

    Description: PDU containing a MAC header and zero or more MAC
                 Random Access Responses

    Document Reference: 36.321 v10.2.0 Section 6.1.5
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_MAC_RAR_HEADER_TYPE_BI = 0,
    LIBLTE_MAC_RAR_HEADER_TYPE_RAPID,
    LIBLTE_MAC_RAR_HEADER_TYPE_N_ITEMS,
}LIBLTE_MAC_RAR_HEADER_TYPE_ENUM;
static const char liblte_mac_rar_header_type_text[LIBLTE_MAC_RAR_HEADER_TYPE_N_ITEMS][20] = {"BI", "RAPID"};
typedef enum{
    LIBLTE_MAC_RAR_HOPPING_DISABLED = 0,
    LIBLTE_MAC_RAR_HOPPING_ENABLED,
    LIBLTE_MAC_RAR_HOPPING_N_ITEMS,
}LIBLTE_MAC_RAR_HOPPING_ENUM;
static const char liblte_mac_rar_hopping_text[LIBLTE_MAC_RAR_HOPPING_N_ITEMS][20] = {"Disabled", "Enabled"};
typedef enum{
    LIBLTE_MAC_RAR_TPC_COMMAND_N6dB = 0,
    LIBLTE_MAC_RAR_TPC_COMMAND_N4dB,
    LIBLTE_MAC_RAR_TPC_COMMAND_N2dB,
    LIBLTE_MAC_RAR_TPC_COMMAND_0dB,
    LIBLTE_MAC_RAR_TPC_COMMAND_2dB,
    LIBLTE_MAC_RAR_TPC_COMMAND_4dB,
    LIBLTE_MAC_RAR_TPC_COMMAND_6dB,
    LIBLTE_MAC_RAR_TPC_COMMAND_8dB,
    LIBLTE_MAC_RAR_TPC_COMMAND_N_ITEMS,
}LIBLTE_MAC_RAR_TPC_COMMAND_ENUM;
static const char liblte_mac_rar_tpc_command_text[LIBLTE_MAC_RAR_TPC_COMMAND_N_ITEMS][20] = {"-6dB", "-4dB", "-2dB",  "0dB",
                                                                                              "2dB",  "4dB",  "6dB",  "8dB"};
static const int8 liblte_mac_rar_tpc_command_num[LIBLTE_MAC_RAR_TPC_COMMAND_N_ITEMS] = {-6, -4, -2, 0, 2, 4, 6, 8};
typedef enum{
    LIBLTE_MAC_RAR_UL_DELAY_DISABLED = 0,
    LIBLTE_MAC_RAR_UL_DELAY_ENABLED,
    LIBLTE_MAC_RAR_UL_DELAY_N_ITEMS,
}LIBLTE_MAC_RAR_UL_DELAY_ENUM;
static const char liblte_mac_rar_ul_delay_text[LIBLTE_MAC_RAR_UL_DELAY_N_ITEMS][20] = {"Disabled", "Enabled"};
typedef enum{
    LIBLTE_MAC_RAR_CSI_REQ_DISABLED = 0,
    LIBLTE_MAC_RAR_CSI_REQ_ENABLED,
    LIBLTE_MAC_RAR_CSI_REQ_N_ITEMS,
}LIBLTE_MAC_RAR_CSI_REQ_ENUM;
static const char liblte_mac_rar_csi_req_text[LIBLTE_MAC_RAR_CSI_REQ_N_ITEMS][20] = {"Disabled", "Enabled"};
// Structs
typedef struct{
    LIBLTE_MAC_RAR_HEADER_TYPE_ENUM hdr_type;
    LIBLTE_MAC_RAR_HOPPING_ENUM     hopping_flag;
    LIBLTE_MAC_RAR_TPC_COMMAND_ENUM tpc_command;
    LIBLTE_MAC_RAR_UL_DELAY_ENUM    ul_delay;
    LIBLTE_MAC_RAR_CSI_REQ_ENUM     csi_req;
    uint16                          rba; // FIXME
    uint16                          timing_adv_cmd;
    uint16                          temp_crnti;
    uint8                           mcs; // FIXME
    uint8                           RAPID;
    uint8                           BI;
}LIBLTE_MAC_RAR_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_mac_pack_random_access_response_pdu(LIBLTE_MAC_RAR_STRUCT *rar,
                                                             LIBLTE_MSG_STRUCT     *pdu);
LIBLTE_ERROR_ENUM liblte_mac_unpack_random_access_response_pdu(LIBLTE_MSG_STRUCT     *pdu,
                                                               LIBLTE_MAC_RAR_STRUCT *rar);

#endif /* __LIBLTE_MAC_H__ */
