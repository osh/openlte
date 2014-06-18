/*******************************************************************************

    Copyright 2014 Ben Wojtowicz

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

    File: liblte_rlc.cc

    Description: Contains all the implementations for the LTE Radio Link
                 Control Layer library.

    Revision History
    ----------    -------------    --------------------------------------------
    06/15/2014    Ben Wojtowicz    Created file.

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "liblte_rlc.h"

/*******************************************************************************
                              DEFINES
*******************************************************************************/


/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/


/*******************************************************************************
                              GLOBAL VARIABLES
*******************************************************************************/


/*******************************************************************************
                              LOCAL FUNCTION PROTOTYPES
*******************************************************************************/

/*********************************************************************
    Name: rlc_value_2_bits

    Description: Converts a value to a bit string
*********************************************************************/
void rlc_value_2_bits(uint32   value,
                      uint8  **bits,
                      uint32   N_bits);

/*********************************************************************
    Name: rlc_bits_2_value

    Description: Converts a bit string to a value
*********************************************************************/
uint32 rlc_bits_2_value(uint8  **bits,
                        uint32   N_bits);

/*******************************************************************************
                              PDU FUNCTIONS
*******************************************************************************/

/*********************************************************************
    PDU Type: Unacknowledged Mode Data PDU

    Document Reference: 36.322 v10.0.0 Section 6.2.1.3
*********************************************************************/
// FIXME

/*********************************************************************
    PDU Type: Acknowledged Mode Data PDU

    Document Reference: 36.322 v10.0.0 Sections 6.2.1.4 & 6.2.1.5
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rlc_pack_amd_pdu(LIBLTE_RLC_AMD_PDU_STRUCT *amd,
                                          LIBLTE_BIT_MSG_STRUCT     *pdu)
{
    // FIXME
}
LIBLTE_ERROR_ENUM liblte_rlc_unpack_amd_pdu(LIBLTE_BIT_MSG_STRUCT     *pdu,
                                            LIBLTE_RLC_AMD_PDU_STRUCT *amd)
{
    LIBLTE_ERROR_ENUM         err     = LIBLTE_ERROR_INVALID_INPUTS;
    uint8                    *pdu_ptr = pdu->msg;
    LIBLTE_RLC_DC_FIELD_ENUM  dc;
    LIBLTE_RLC_E_FIELD_ENUM   e;

    if(pdu != NULL &&
       amd != NULL)
    {
        // Header
        dc = (LIBLTE_RLC_DC_FIELD_ENUM)rlc_bits_2_value(&pdu_ptr, 1);

        if(LIBLTE_RLC_DC_FIELD_DATA_PDU == dc)
        {
            // Header
            amd->hdr.rf = (LIBLTE_RLC_RF_FIELD_ENUM)rlc_bits_2_value(&pdu_ptr, 1);
            amd->hdr.p  = (LIBLTE_RLC_P_FIELD_ENUM)rlc_bits_2_value(&pdu_ptr, 1);
            amd->hdr.fi = (LIBLTE_RLC_FI_FIELD_ENUM)rlc_bits_2_value(&pdu_ptr, 2);
            e           = (LIBLTE_RLC_E_FIELD_ENUM)rlc_bits_2_value(&pdu_ptr, 1);
            amd->hdr.sn = rlc_bits_2_value(&pdu_ptr, 10);

            if(LIBLTE_RLC_RF_FIELD_AMD_PDU_SEGMENT == amd->hdr.rf)
            {
                // FIXME
                printf("Not handling AMD PDU SEGMENTS\n");
            }

            if(LIBLTE_RLC_E_FIELD_HEADER_EXTENDED == e)
            {
                // FIXME
                printf("Not handling HEADER EXTENSION\n");
            }

            // Data
            amd->data.N_bits = pdu->N_bits - (pdu_ptr - pdu->msg);
            memcpy(amd->data.msg, pdu_ptr, amd->data.N_bits);

            err = LIBLTE_SUCCESS;
        }
    }

    return(err);
}

/*********************************************************************
    PDU Type: Status PDU

    Document Reference: 36.322 v10.0.0 Section 6.2.1.6
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rlc_pack_status_pdu(LIBLTE_RLC_STATUS_PDU_STRUCT *status,
                                             LIBLTE_BIT_MSG_STRUCT        *pdu)
{
    LIBLTE_ERROR_ENUM  err     = LIBLTE_ERROR_INVALID_INPUTS;
    uint8             *pdu_ptr = pdu->msg;

    if(status != NULL &&
       pdu    != NULL)
    {
        // D/C Field
        rlc_value_2_bits(LIBLTE_RLC_DC_FIELD_CONTROL_PDU, &pdu_ptr, 1);

        // CPT Field
        rlc_value_2_bits(LIBLTE_RLC_CPT_FIELD_STATUS_PDU, &pdu_ptr, 3);

        // ACK SN
        rlc_value_2_bits(status->ack_sn, &pdu_ptr, 10);

        // E1
        rlc_value_2_bits(LIBLTE_RLC_E1_FIELD_NOT_EXTENDED, &pdu_ptr, 1);

        // Padding
        rlc_value_2_bits(0, &pdu_ptr, 1);

        pdu->N_bits = pdu_ptr - pdu->msg;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rlc_unpack_status_pdu(LIBLTE_BIT_MSG_STRUCT        *pdu,
                                               LIBLTE_RLC_STATUS_PDU_STRUCT *status)
{
    // FIXME
}

/*******************************************************************************
                              LOCAL FUNCTIONS
*******************************************************************************/

/*********************************************************************
    Name: rlc_value_2_bits

    Description: Converts a value to a bit string
*********************************************************************/
void rlc_value_2_bits(uint32   value,
                      uint8  **bits,
                      uint32   N_bits)
{
    uint32 i;

    for(i=0; i<N_bits; i++)
    {
        (*bits)[i] = (value >> (N_bits-i-1)) & 0x1;
    }
    *bits += N_bits;
}

/*********************************************************************
    Name: rlc_bits_2_value

    Description: Converts a bit string to a value
*********************************************************************/
uint32 rlc_bits_2_value(uint8  **bits,
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
