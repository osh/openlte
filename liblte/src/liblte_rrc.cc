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

LIBLTE_RRC_IE_STRUCT global_ie;

/*******************************************************************************
                              LOCAL FUNCTION PROTOTYPES
*******************************************************************************/

/*********************************************************************
    Name: value_2_bits

    Description: Converts a value to a bit string
*********************************************************************/
void value_2_bits(void);

/*********************************************************************
    Name: bits_2_value

    Description: Converts a bit string to a value
*********************************************************************/
uint32 bits_2_value(uint8  **bits,
                    uint32   num_bits);

/*******************************************************************************
                              INFORMATION ELEMENT FUNCTIONS
*******************************************************************************/

/*********************************************************************
    IE Name: PHICH-Config

    Description: Specifies the PHICH configuration

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_phich_config_ie(LIBLTE_RRC_PHICH_CONFIG_STRUCT *phich_config,
                                                  LIBLTE_RRC_IE_STRUCT           *ie)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    // FIXME

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_phich_config_ie(LIBLTE_RRC_IE_STRUCT           *ie,
                                                    LIBLTE_RRC_PHICH_CONFIG_STRUCT *phich_config,
                                                    uint32                         *num_bits_used)
{
    LIBLTE_ERROR_ENUM  err    = LIBLTE_ERROR_INVALID_INPUTS;
    uint8             *ie_ptr = ie->ie;

    if(ie           != NULL &&
       phich_config != NULL)
    {
        // Duration
        phich_config->dur = (LIBLTE_RRC_PHICH_DURATION_ENUM)bits_2_value(&ie_ptr, 1);

        // Resource
        phich_config->res = (LIBLTE_RRC_PHICH_RESOURCE_ENUM)bits_2_value(&ie_ptr, 2);

        // Report number of bits that were used
        *num_bits_used = 3;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*******************************************************************************
                              MESSAGE FUNCTIONS
*******************************************************************************/

/*********************************************************************
    Message Name: bcch_bch_msg

    Description: Contains the set of RRC messages that may be sent
                 from the E-UTRAN to the UE via BCH on the BCCH
                 logical channel.

    Document Reference: 36.331 v10.0.0 Sections 6.2.1 and 6.2.2
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_bch_bcch_msg(LIBLTE_RRC_MIB_STRUCT *mib,
                                               LIBLTE_RRC_MSG_STRUCT *msg)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    // FIXME

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_bch_bcch_msg(LIBLTE_RRC_MSG_STRUCT *msg,
                                                 LIBLTE_RRC_MIB_STRUCT *mib)
{
    LIBLTE_ERROR_ENUM  err     = LIBLTE_ERROR_INVALID_INPUTS;
    uint8             *msg_ptr = msg->msg;
    uint32             num_bits_used;

    if(msg != NULL &&
       mib != NULL)
    {
        // DL Bandwidth
        mib->dl_bw = (LIBLTE_RRC_DL_BANDWIDTH_ENUM)bits_2_value(&msg_ptr, 3);

        // PHICH CONFIG
        memcpy(global_ie.ie, msg_ptr, msg->num_bits-3);
        global_ie.num_bits = msg->num_bits-3;
        err                = liblte_rrc_unpack_phich_config_ie(&global_ie,
                                                               &mib->phich_config,
                                                               &num_bits_used);
        msg_ptr += num_bits_used;

        // SFN/4
        mib->sfn_div_4 = bits_2_value(&msg_ptr, 8);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*******************************************************************************
                              LOCAL FUNCTIONS
*******************************************************************************/

/*********************************************************************
    Name: value_2_bits

    Description: Converts a value to a bit string
*********************************************************************/
void value_2_bits(void)
{
    // FIXME
}

/*********************************************************************
    Name: bits_2_value

    Description: Converts a bit string to a value
*********************************************************************/
uint32 bits_2_value(uint8  **bits,
                    uint32   num_bits)
{
    uint32 value = 0;
    uint32 i;

    for(i=0; i<num_bits; i++)
    {
        value |= (*bits)[i] << (num_bits-i-1);
    }
    *bits += num_bits;

    return(value);
}
