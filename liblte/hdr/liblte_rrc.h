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
    uint32 num_bits;
    uint8  ie[LIBLTE_RRC_MAX_IE_SIZE];
}LIBLTE_RRC_IE_STRUCT;
typedef struct{
    uint32 num_bits;
    uint8  msg[LIBLTE_RRC_MAX_MSG_SIZE];
}LIBLTE_RRC_MSG_STRUCT;

/*******************************************************************************
                              INFORMATION ELEMENT DECLARATIONS
*******************************************************************************/

/*********************************************************************
    IE Name: PHICH-Config

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
LIBLTE_ERROR_ENUM liblte_rrc_pack_phich_config_ie(LIBLTE_RRC_PHICH_CONFIG_STRUCT *phich_config,
                                                  LIBLTE_RRC_IE_STRUCT           *ie);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_phich_config_ie(LIBLTE_RRC_IE_STRUCT           *ie,
                                                    LIBLTE_RRC_PHICH_CONFIG_STRUCT *phich_config,
                                                    uint32                         *num_bits_used);

/*******************************************************************************
                              MESSAGE DECLARATIONS
*******************************************************************************/

/*********************************************************************
    Message Name: bcch_bch_msg

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
LIBLTE_ERROR_ENUM liblte_rrc_pack_bch_bcch_msg(LIBLTE_RRC_MIB_STRUCT *mib,
                                               LIBLTE_RRC_MSG_STRUCT *msg);
LIBLTE_ERROR_ENUM liblte_rrc_unpack_bch_bcch_msg(LIBLTE_RRC_MSG_STRUCT *msg,
                                                 LIBLTE_RRC_MIB_STRUCT *mib);

#endif /* __LIBLTE_RRC_H__ */
