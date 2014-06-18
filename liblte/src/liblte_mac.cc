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

    File: liblte_mac.cc

    Description: Contains all the implementations for the LTE Medium Access
                 Control Layer library.

    Revision History
    ----------    -------------    --------------------------------------------
    07/21/2013    Ben Wojtowicz    Created file.
    03/26/2014    Ben Wojtowicz    Added DL-SCH/UL-SCH PDU handling.
    05/04/2014    Ben Wojtowicz    Added control element handling.
    06/15/2014    Ben Wojtowicz    Added support for padding LCIDs and breaking
                                   out max and min buffer sizes for BSRs.

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "liblte_mac.h"

/*******************************************************************************
                              DEFINES
*******************************************************************************/


/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/


/*******************************************************************************
                              GLOBAL VARIABLES
*******************************************************************************/

uint32 truncated_short_bsr_max_buffer_size[64] = {     0,     10,     12,     14,     17,     19,     22,     26,
                                                      31,     36,     42,     49,     57,     67,     78,     91,
                                                     107,    125,    146,    171,    200,    234,    274,    321,
                                                     376,    440,    515,    603,    706,    826,    967,   1132,
                                                    1326,   1552,   1817,   2127,   2490,   2915,   3413,   3995,
                                                    4677,   5476,   6411,   7505,   8787,  10287,  12043,  14099,
                                                   16507,  19325,  22624,  26487,  31009,  36304,  42502,  49759,
                                                   58255,  68201,  79864,  93479, 109439, 128125, 150000, 150000};
uint32 truncated_short_bsr_min_buffer_size[64] = {     0,      0,     10,     12,     14,     17,     19,     22,
                                                      26,     31,     36,     42,     49,     57,     67,     78,
                                                      91,    107,    125,    146,    171,    200,    234,    274,
                                                     321,    376,    440,    515,    603,    706,    826,    967,
                                                    1132,   1326,   1552,   1817,   2127,   2490,   2915,   3413,
                                                    3995,   4677,   5476,   6411,   7505,   8787,  10287,  12043,
                                                   14099,  16507,  19325,  22624,  26487,  31009,  36304,  42502,
                                                   49759,  58255,  68201,  79864,  93479, 109439, 128125, 150000};

/*******************************************************************************
                              LOCAL FUNCTION PROTOTYPES
*******************************************************************************/

/*********************************************************************
    Name: mac_value_2_bits

    Description: Converts a value to a bit string
*********************************************************************/
void mac_value_2_bits(uint32   value,
                      uint8  **bits,
                      uint32   N_bits);

/*********************************************************************
    Name: mac_bits_2_value

    Description: Converts a bit string to a value
*********************************************************************/
uint32 mac_bits_2_value(uint8  **bits,
                        uint32   N_bits);

/*******************************************************************************
                              CONTROL ELEMENT FUNCTIONS
*******************************************************************************/

/*********************************************************************
    MAC CE Name: Truncated Buffer Status Report

    Description: CE containing one LCG ID field and one corresponding
                 Buffer Size field

    Document Reference: 36.321 v10.2.0 Section 6.1.3.1
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mac_pack_truncated_bsr_ce(LIBLTE_MAC_TRUNCATED_BSR_CE_STRUCT  *truncated_bsr,
                                                   uint8                              **ce_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;
    uint32            i;

    if(truncated_bsr != NULL &&
       ce_ptr        != NULL)
    {
        mac_value_2_bits(truncated_bsr->lcg_id, ce_ptr, 2);
        for(i=0; i<64; i++)
        {
            if(truncated_bsr->max_buffer_size  > truncated_short_bsr_min_buffer_size[i] &&
               truncated_bsr->max_buffer_size <= truncated_short_bsr_max_buffer_size[i])
            {
                mac_value_2_bits(i, ce_ptr, 6);
                break;
            }
        }
        if(i == 64)
        {
            mac_value_2_bits(63, ce_ptr, 6);
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mac_unpack_truncated_bsr_ce(uint8                              **ce_ptr,
                                                     LIBLTE_MAC_TRUNCATED_BSR_CE_STRUCT  *truncated_bsr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;
    uint8             buffer_size_idx;

    if(ce_ptr        != NULL &&
       truncated_bsr != NULL)
    {
        truncated_bsr->lcg_id          = mac_bits_2_value(ce_ptr, 2);
        buffer_size_idx                = mac_bits_2_value(ce_ptr, 6);
        truncated_bsr->max_buffer_size = truncated_short_bsr_max_buffer_size[buffer_size_idx];
        truncated_bsr->min_buffer_size = truncated_short_bsr_min_buffer_size[buffer_size_idx];

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    MAC CE Name: Short Buffer Status Report

    Description: CE containing one LCG ID field and one corresponding
                 Buffer Size field

    Document Reference: 36.321 v10.2.0 Section 6.1.3.1
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mac_pack_short_bsr_ce(LIBLTE_MAC_TRUNCATED_BSR_CE_STRUCT  *short_bsr,
                                               uint8                              **ce_ptr)
{
    return(liblte_mac_pack_truncated_bsr_ce(short_bsr, ce_ptr));
}
LIBLTE_ERROR_ENUM liblte_mac_unpack_short_bsr_ce(uint8                          **ce_ptr,
                                                 LIBLTE_MAC_SHORT_BSR_CE_STRUCT  *short_bsr)
{
    return(liblte_mac_unpack_truncated_bsr_ce(ce_ptr, short_bsr));
}

/*********************************************************************
    MAC CE Name: Long Buffer Status Report

    Description: CE containing four Buffer Size fields, corresponding
                 to LCG IDs #0 through #3

    Document Reference: 36.321 v10.2.0 Section 6.1.3.1
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mac_pack_long_bsr_ce(LIBLTE_MAC_LONG_BSR_CE_STRUCT  *long_bsr,
                                              uint8                         **ce_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(long_bsr != NULL &&
       ce_ptr   != NULL)
    {
        mac_value_2_bits(long_bsr->buffer_size_0, ce_ptr, 6);
        mac_value_2_bits(long_bsr->buffer_size_1, ce_ptr, 6);
        mac_value_2_bits(long_bsr->buffer_size_2, ce_ptr, 6);
        mac_value_2_bits(long_bsr->buffer_size_3, ce_ptr, 6);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mac_unpack_long_bsr_ce(uint8                         **ce_ptr,
                                                LIBLTE_MAC_LONG_BSR_CE_STRUCT  *long_bsr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ce_ptr   != NULL &&
       long_bsr != NULL)
    {
        long_bsr->buffer_size_0 = mac_bits_2_value(ce_ptr, 6);
        long_bsr->buffer_size_1 = mac_bits_2_value(ce_ptr, 6);
        long_bsr->buffer_size_2 = mac_bits_2_value(ce_ptr, 6);
        long_bsr->buffer_size_3 = mac_bits_2_value(ce_ptr, 6);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    MAC CE Name: C-RNTI

    Description: CE containing a C-RNTI

    Document Reference: 36.321 v10.2.0 Section 6.1.3.2
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mac_pack_c_rnti_ce(LIBLTE_MAC_C_RNTI_CE_STRUCT  *c_rnti,
                                            uint8                       **ce_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(c_rnti != NULL &&
       ce_ptr != NULL)
    {
        mac_value_2_bits(c_rnti->c_rnti, ce_ptr, 16);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mac_unpack_c_rnti_ce(uint8                       **ce_ptr,
                                              LIBLTE_MAC_C_RNTI_CE_STRUCT  *c_rnti)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ce_ptr != NULL &&
       c_rnti != NULL)
    {
        c_rnti->c_rnti = mac_bits_2_value(ce_ptr, 16);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    MAC CE Name: DRX Command

    Description: CE containing nothing

    Document Reference: 36.321 v10.2.0 Section 6.1.3.3
*********************************************************************/

/*********************************************************************
    MAC CE Name: UE Contention Resolution Identity

    Description: CE containing the contention resolution identity for
                 a UE

    Document Reference: 36.321 v10.2.0 Section 6.1.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mac_pack_ue_contention_resolution_id_ce(LIBLTE_MAC_UE_CONTENTION_RESOLUTION_ID_CE_STRUCT  *ue_con_res_id,
                                                                 uint8                                            **ce_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ue_con_res_id != NULL &&
       ce_ptr        != NULL)
    {
        mac_value_2_bits((uint32)(ue_con_res_id->id >> 32), ce_ptr, 16);
        mac_value_2_bits((uint32)(ue_con_res_id->id),       ce_ptr, 32);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mac_unpack_ue_contention_resolution_id_ce(uint8                                            **ce_ptr,
                                                                   LIBLTE_MAC_UE_CONTENTION_RESOLUTION_ID_CE_STRUCT  *ue_con_res_id)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ce_ptr        != NULL &&
       ue_con_res_id != NULL)
    {
        ue_con_res_id->id  = (uint64)mac_bits_2_value(ce_ptr, 16) << 32;
        ue_con_res_id->id |= (uint64)mac_bits_2_value(ce_ptr, 32);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    MAC CE Name: Timing Advance Command

    Description: CE containing a timing advance command for a UE

    Document Reference: 36.321 v10.2.0 Section 6.1.3.5
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mac_pack_ta_command_ce(LIBLTE_MAC_TA_COMMAND_CE_STRUCT  *ta_command,
                                                uint8                           **ce_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ta_command != NULL &&
       ce_ptr     != NULL)
    {
        mac_value_2_bits(0,              ce_ptr, 1); // R
        mac_value_2_bits(0,              ce_ptr, 1); // R
        mac_value_2_bits(ta_command->ta, ce_ptr, 6);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mac_unpack_ta_command_ce(uint8                           **ce_ptr,
                                                  LIBLTE_MAC_TA_COMMAND_CE_STRUCT  *ta_command)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ce_ptr     != NULL &&
       ta_command != NULL)
    {
        mac_bits_2_value(ce_ptr, 1); // R
        mac_bits_2_value(ce_ptr, 1); // R
        ta_command->ta = mac_bits_2_value(ce_ptr, 6);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    MAC CE Name: Power Headroom

    Description: CE containing the power headroom of the UE

    Document Reference: 36.321 v10.2.0 Section 6.1.3.6
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mac_pack_power_headroom_ce(LIBLTE_MAC_POWER_HEADROOM_CE_STRUCT  *power_headroom,
                                                    uint8                               **ce_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(power_headroom != NULL &&
       ce_ptr         != NULL)
    {
        mac_value_2_bits(0,                  ce_ptr, 1); // R
        mac_value_2_bits(0,                  ce_ptr, 1); // R
        mac_value_2_bits(power_headroom->ph, ce_ptr, 6);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mac_unpack_power_headroom_ce(uint8                               **ce_ptr,
                                                      LIBLTE_MAC_POWER_HEADROOM_CE_STRUCT  *power_headroom)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ce_ptr         != NULL &&
       power_headroom != NULL)
    {
        mac_bits_2_value(ce_ptr, 1); // R
        mac_bits_2_value(ce_ptr, 1); // R
        power_headroom->ph = mac_bits_2_value(ce_ptr, 6);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    MAC CE Name: Extended Power Headroom

    Description: CE containing the power headroom of the UE

    Document Reference: 36.321 v10.2.0 Section 6.1.3.6a
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mac_pack_ext_power_headroom_ce(LIBLTE_MAC_EXT_POWER_HEADROOM_CE_STRUCT  *ext_power_headroom,
                                                        uint8                                   **ce_ptr)
{
    // FIXME
    return(LIBLTE_ERROR_INVALID_INPUTS);
}
LIBLTE_ERROR_ENUM liblte_mac_unpack_ext_power_headroom_ce(uint8                                   **ce_ptr,
                                                          LIBLTE_MAC_EXT_POWER_HEADROOM_CE_STRUCT  *ext_power_headroom)
{
    // FIXME
    return(LIBLTE_ERROR_INVALID_INPUTS);
}

/*********************************************************************
    MAC CE Name: MCH Scheduling Information

    Description: CE containing MTCH stops

    Document Reference: 36.321 v10.2.0 Section 6.1.3.7
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mac_pack_mch_scheduling_information_ce(LIBLTE_MAC_MCH_SCHEDULING_INFORMATION_CE_STRUCT  *mch_sched_info,
                                                                uint8                                           **ce_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;
    uint32            i;

    if(mch_sched_info          != NULL &&
       ce_ptr                  != NULL &&
       mch_sched_info->N_items <= LIBLTE_MAC_MCH_SCHEDULING_INFORMATION_MAX_N_ITEMS)
    {
        for(i=0; i<mch_sched_info->N_items; i++)
        {
            mac_value_2_bits(mch_sched_info->lcid[i],     ce_ptr,  5);
            mac_value_2_bits(mch_sched_info->stop_mch[i], ce_ptr, 11);
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mac_unpack_mch_scheduling_information_ce(uint8                                           **ce_ptr,
                                                                  LIBLTE_MAC_MCH_SCHEDULING_INFORMATION_CE_STRUCT  *mch_sched_info)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;
    uint32            i;

    if(ce_ptr                  != NULL &&
       mch_sched_info          != NULL &&
       mch_sched_info->N_items <= LIBLTE_MAC_MCH_SCHEDULING_INFORMATION_MAX_N_ITEMS)
    {
        for(i=0; i<mch_sched_info->N_items; i++)
        {
            mch_sched_info->lcid[i]     = mac_bits_2_value(ce_ptr,  5);
            mch_sched_info->stop_mch[i] = mac_bits_2_value(ce_ptr, 11);
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    MAC CE Name: Activation Deactivation

    Description: CE containing activation/deactivation of SCells

    Document Reference: 36.321 v10.2.0 Section 6.1.3.8
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mac_pack_activation_deactivation_ce(LIBLTE_MAC_ACTIVATION_DEACTIVATION_CE_STRUCT  *act_deact,
                                                             uint8                                        **ce_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(act_deact != NULL &&
       ce_ptr    != NULL)
    {
        mac_value_2_bits(act_deact->c7, ce_ptr, 1);
        mac_value_2_bits(act_deact->c6, ce_ptr, 1);
        mac_value_2_bits(act_deact->c5, ce_ptr, 1);
        mac_value_2_bits(act_deact->c4, ce_ptr, 1);
        mac_value_2_bits(act_deact->c3, ce_ptr, 1);
        mac_value_2_bits(act_deact->c2, ce_ptr, 1);
        mac_value_2_bits(act_deact->c1, ce_ptr, 1);
        mac_value_2_bits(0,             ce_ptr, 1); // R

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mac_unpack_activation_deactivation_ce(uint8                                        **ce_ptr,
                                                               LIBLTE_MAC_ACTIVATION_DEACTIVATION_CE_STRUCT  *act_deact)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ce_ptr    != NULL &&
       act_deact != NULL)
    {
        act_deact->c7 = mac_bits_2_value(ce_ptr, 1);
        act_deact->c6 = mac_bits_2_value(ce_ptr, 1);
        act_deact->c5 = mac_bits_2_value(ce_ptr, 1);
        act_deact->c4 = mac_bits_2_value(ce_ptr, 1);
        act_deact->c3 = mac_bits_2_value(ce_ptr, 1);
        act_deact->c2 = mac_bits_2_value(ce_ptr, 1);
        act_deact->c1 = mac_bits_2_value(ce_ptr, 1);
        mac_bits_2_value(ce_ptr, 1); // R

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*******************************************************************************
                              PDU FUNCTIONS
*******************************************************************************/

/*********************************************************************
    PDU Name: DL-SCH and UL-SCH MAC PDU

    Description: PDU containing a MAC header, zero or more MAC SDUs,
                 and zero or more MAC control elements

    Document Reference: 36.321 v10.2.0 Section 6.1.2
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mac_pack_mac_pdu(LIBLTE_MAC_PDU_STRUCT *pdu,
                                          LIBLTE_BIT_MSG_STRUCT *msg)
{
    LIBLTE_ERROR_ENUM  err     = LIBLTE_ERROR_INVALID_INPUTS;
    uint8             *msg_ptr = msg->msg;
    uint32             i;

    if(pdu != NULL &&
       msg != NULL)
    {
        // Pack the subheaders
        for(i=0; i<pdu->N_subheaders; i++)
        {
            mac_value_2_bits(0, &msg_ptr, 1); // R
            mac_value_2_bits(0, &msg_ptr, 1); // R
            if(i != pdu->N_subheaders-1)
            {
                mac_value_2_bits(1, &msg_ptr, 1); // E
            }else{
                mac_value_2_bits(0, &msg_ptr, 1); // E
            }
            mac_value_2_bits(pdu->subheader[i].lcid, &msg_ptr, 5);
            if(LIBLTE_MAC_CHAN_TYPE_DLSCH == pdu->chan_type)
            {
                if(!(LIBLTE_MAC_DLSCH_ACTIVATION_DEACTIVATION_LCID     == pdu->subheader[i].lcid ||
                     LIBLTE_MAC_DLSCH_UE_CONTENTION_RESOLUTION_ID_LCID == pdu->subheader[i].lcid ||
                     LIBLTE_MAC_DLSCH_TA_COMMAND_LCID                  == pdu->subheader[i].lcid ||
                     LIBLTE_MAC_DLSCH_DRX_COMMAND_LCID                 == pdu->subheader[i].lcid ||
                     LIBLTE_MAC_DLSCH_PADDING_LCID                     == pdu->subheader[i].lcid))
                {
                    if(i != (pdu->N_subheaders-1))
                    {
                        if((pdu->subheader[i].payload.sdu.N_bits/8) < 128)
                        {
                            mac_value_2_bits(0,                                      &msg_ptr, 1); // F
                            mac_value_2_bits(pdu->subheader[i].payload.sdu.N_bits/8, &msg_ptr, 7);
                        }else{
                            mac_value_2_bits(1,                                      &msg_ptr,  1); // F
                            mac_value_2_bits(pdu->subheader[i].payload.sdu.N_bits/8, &msg_ptr, 15);
                        }
                    }
                }
            }else if(LIBLTE_MAC_CHAN_TYPE_ULSCH == pdu->chan_type){
                if(LIBLTE_MAC_ULSCH_EXT_POWER_HEADROOM_REPORT_LCID == pdu->subheader[i].lcid)
                {
                    // FIXME
                }else if(!(LIBLTE_MAC_ULSCH_POWER_HEADROOM_REPORT_LCID == pdu->subheader[i].lcid ||
                           LIBLTE_MAC_ULSCH_C_RNTI_LCID                == pdu->subheader[i].lcid ||
                           LIBLTE_MAC_ULSCH_TRUNCATED_BSR_LCID         == pdu->subheader[i].lcid ||
                           LIBLTE_MAC_ULSCH_SHORT_BSR_LCID             == pdu->subheader[i].lcid ||
                           LIBLTE_MAC_ULSCH_LONG_BSR_LCID              == pdu->subheader[i].lcid ||
                           LIBLTE_MAC_ULSCH_PADDING_LCID               == pdu->subheader[i].lcid)){
                    if(i != (pdu->N_subheaders-1))
                    {
                        if((pdu->subheader[i].payload.sdu.N_bits/8) < 128)
                        {
                            mac_value_2_bits(0,                                      &msg_ptr, 1); // F
                            mac_value_2_bits(pdu->subheader[i].payload.sdu.N_bits/8, &msg_ptr, 7);
                        }else{
                            mac_value_2_bits(1,                                      &msg_ptr,  1); // F
                            mac_value_2_bits(pdu->subheader[i].payload.sdu.N_bits/8, &msg_ptr, 15);
                        }
                    }
                }
            }else{ // LIBLTE_MAC_CHAN_TYPE_MCH == mac_pdu->chan_type
                if(LIBLTE_MAC_MCH_SCHEDULING_INFORMATION_LCID == pdu->subheader[i].lcid)
                {
                    if(i != (pdu->N_subheaders-1))
                    {
                        if((pdu->subheader[i].payload.mch_sched_info.N_items*2) < 128)
                        {
                            mac_value_2_bits(0,                                                  &msg_ptr, 1); // F
                            mac_value_2_bits(pdu->subheader[i].payload.mch_sched_info.N_items*2, &msg_ptr, 7);
                        }else{
                            mac_value_2_bits(1,                                                  &msg_ptr,  1); // F
                            mac_value_2_bits(pdu->subheader[i].payload.mch_sched_info.N_items*2, &msg_ptr, 15);
                        }
                    }
                }else{
                    if(i != (pdu->N_subheaders-1))
                    {
                        if((pdu->subheader[i].payload.sdu.N_bits/8) < 128)
                        {
                            mac_value_2_bits(0,                                      &msg_ptr, 1); // F
                            mac_value_2_bits(pdu->subheader[i].payload.sdu.N_bits/8, &msg_ptr, 7);
                        }else{
                            mac_value_2_bits(1,                                      &msg_ptr,  1); // F
                            mac_value_2_bits(pdu->subheader[i].payload.sdu.N_bits/8, &msg_ptr, 15);
                        }
                    }
                }
            }
        }

        // Pack the control elements and SDUs
        for(i=0; i<pdu->N_subheaders; i++)
        {
            if(LIBLTE_MAC_CHAN_TYPE_DLSCH == pdu->chan_type)
            {
                if(LIBLTE_MAC_DLSCH_ACTIVATION_DEACTIVATION_LCID == pdu->subheader[i].lcid)
                {
                    liblte_mac_pack_activation_deactivation_ce(&pdu->subheader[i].payload.act_deact, &msg_ptr);
                }else if(LIBLTE_MAC_DLSCH_UE_CONTENTION_RESOLUTION_ID_LCID == pdu->subheader[i].lcid){
                    liblte_mac_pack_ue_contention_resolution_id_ce(&pdu->subheader[i].payload.ue_con_res_id, &msg_ptr);
                }else if(LIBLTE_MAC_DLSCH_TA_COMMAND_LCID == pdu->subheader[i].lcid){
                    liblte_mac_pack_ta_command_ce(&pdu->subheader[i].payload.ta_command, &msg_ptr);
                }else if(LIBLTE_MAC_DLSCH_DRX_COMMAND_LCID == pdu->subheader[i].lcid){
                    // No content for DRX Command CE
                }else{ // SDU
                    memcpy(msg_ptr, pdu->subheader[i].payload.sdu.msg, pdu->subheader[i].payload.sdu.N_bits);
                    msg_ptr += pdu->subheader[i].payload.sdu.N_bits;
                }
            }else if(LIBLTE_MAC_CHAN_TYPE_ULSCH == pdu->chan_type){
                if(LIBLTE_MAC_ULSCH_EXT_POWER_HEADROOM_REPORT_LCID == pdu->subheader[i].lcid)
                {
                    liblte_mac_pack_ext_power_headroom_ce(&pdu->subheader[i].payload.ext_power_headroom, &msg_ptr);
                }else if(LIBLTE_MAC_ULSCH_POWER_HEADROOM_REPORT_LCID == pdu->subheader[i].lcid){
                    liblte_mac_pack_power_headroom_ce(&pdu->subheader[i].payload.power_headroom, &msg_ptr);
                }else if(LIBLTE_MAC_ULSCH_C_RNTI_LCID == pdu->subheader[i].lcid){
                    liblte_mac_pack_c_rnti_ce(&pdu->subheader[i].payload.c_rnti, &msg_ptr);
                }else if(LIBLTE_MAC_ULSCH_TRUNCATED_BSR_LCID == pdu->subheader[i].lcid){
                    liblte_mac_pack_truncated_bsr_ce(&pdu->subheader[i].payload.truncated_bsr, &msg_ptr);
                }else if(LIBLTE_MAC_ULSCH_SHORT_BSR_LCID == pdu->subheader[i].lcid){
                    liblte_mac_pack_short_bsr_ce(&pdu->subheader[i].payload.short_bsr, &msg_ptr);
                }else if(LIBLTE_MAC_ULSCH_LONG_BSR_LCID == pdu->subheader[i].lcid){
                    liblte_mac_pack_long_bsr_ce(&pdu->subheader[i].payload.long_bsr, &msg_ptr);
                }else{ // SDU
                    memcpy(msg_ptr, pdu->subheader[i].payload.sdu.msg, pdu->subheader[i].payload.sdu.N_bits);
                    msg_ptr += pdu->subheader[i].payload.sdu.N_bits;
                }
            }else{ // LIBLTE_MAC_CHAN_TYPE_MCH == mac_pdu->chan_type
                if(LIBLTE_MAC_MCH_SCHEDULING_INFORMATION_LCID == pdu->subheader[i].lcid)
                {
                    liblte_mac_pack_mch_scheduling_information_ce(&pdu->subheader[i].payload.mch_sched_info, &msg_ptr);
                }else{ // SDU
                    memcpy(msg_ptr, pdu->subheader[i].payload.sdu.msg, pdu->subheader[i].payload.sdu.N_bits);
                    msg_ptr += pdu->subheader[i].payload.sdu.N_bits;
                }
            }
        }

        msg->N_bits = msg_ptr - msg->msg;
        err         = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mac_unpack_mac_pdu(LIBLTE_BIT_MSG_STRUCT *msg,
                                            LIBLTE_MAC_PDU_STRUCT *pdu)
{
    LIBLTE_ERROR_ENUM  err     = LIBLTE_ERROR_INVALID_INPUTS;
    uint8             *msg_ptr = msg->msg;
    uint32             i;
    uint8              e_bit = 1;

    if(msg != NULL &&
       pdu != NULL)
    {
        // Unpack the subheaders
        pdu->N_subheaders = 0;
        while(e_bit)
        {
            mac_bits_2_value(&msg_ptr, 2); // R
            e_bit = mac_bits_2_value(&msg_ptr, 1);
            pdu->subheader[pdu->N_subheaders].lcid = mac_bits_2_value(&msg_ptr, 5);

            if(LIBLTE_MAC_CHAN_TYPE_DLSCH == pdu->chan_type)
            {
                if(!(LIBLTE_MAC_DLSCH_ACTIVATION_DEACTIVATION_LCID     == pdu->subheader[pdu->N_subheaders].lcid ||
                     LIBLTE_MAC_DLSCH_UE_CONTENTION_RESOLUTION_ID_LCID == pdu->subheader[pdu->N_subheaders].lcid ||
                     LIBLTE_MAC_DLSCH_TA_COMMAND_LCID                  == pdu->subheader[pdu->N_subheaders].lcid ||
                     LIBLTE_MAC_DLSCH_DRX_COMMAND_LCID                 == pdu->subheader[pdu->N_subheaders].lcid ||
                     LIBLTE_MAC_DLSCH_PADDING_LCID                     == pdu->subheader[pdu->N_subheaders].lcid))
                {
                    if(e_bit)
                    {
                        if(mac_bits_2_value(&msg_ptr, 1)) // F
                        {
                            pdu->subheader[pdu->N_subheaders].payload.sdu.N_bits = mac_bits_2_value(&msg_ptr, 15) * 8;
                        }else{
                            pdu->subheader[pdu->N_subheaders].payload.sdu.N_bits = mac_bits_2_value(&msg_ptr, 7) * 8;
                        }
                    }else{
                        pdu->subheader[pdu->N_subheaders].payload.sdu.N_bits = 0;
                    }
                }
            }else if(LIBLTE_MAC_CHAN_TYPE_ULSCH == pdu->chan_type){
                if(LIBLTE_MAC_ULSCH_EXT_POWER_HEADROOM_REPORT_LCID == pdu->subheader[pdu->N_subheaders].lcid)
                {
                    // FIXME
                }else if(!(LIBLTE_MAC_ULSCH_POWER_HEADROOM_REPORT_LCID == pdu->subheader[pdu->N_subheaders].lcid ||
                           LIBLTE_MAC_ULSCH_C_RNTI_LCID                == pdu->subheader[pdu->N_subheaders].lcid ||
                           LIBLTE_MAC_ULSCH_TRUNCATED_BSR_LCID         == pdu->subheader[pdu->N_subheaders].lcid ||
                           LIBLTE_MAC_ULSCH_SHORT_BSR_LCID             == pdu->subheader[pdu->N_subheaders].lcid ||
                           LIBLTE_MAC_ULSCH_LONG_BSR_LCID              == pdu->subheader[pdu->N_subheaders].lcid ||
                           LIBLTE_MAC_ULSCH_PADDING_LCID               == pdu->subheader[pdu->N_subheaders].lcid)){
                    if(e_bit)
                    {
                        if(mac_bits_2_value(&msg_ptr, 1)) // F
                        {
                            pdu->subheader[pdu->N_subheaders].payload.sdu.N_bits = mac_bits_2_value(&msg_ptr, 15) * 8;
                        }else{
                            pdu->subheader[pdu->N_subheaders].payload.sdu.N_bits = mac_bits_2_value(&msg_ptr, 7) * 8;
                        }
                    }else{
                        pdu->subheader[pdu->N_subheaders].payload.sdu.N_bits = 0;
                    }
                }
            }else{ // LIBLTE_MAC_CHAN_TYPE_MCH == mac_pdu->subheader[i].lcid
                if(LIBLTE_MAC_MCH_SCHEDULING_INFORMATION_LCID == pdu->subheader[pdu->N_subheaders].lcid)
                {
                    if(e_bit)
                    {
                        if(mac_bits_2_value(&msg_ptr, 1)) // F
                        {
                            pdu->subheader[pdu->N_subheaders].payload.mch_sched_info.N_items = mac_bits_2_value(&msg_ptr, 15) / 2;
                        }else{
                            pdu->subheader[pdu->N_subheaders].payload.mch_sched_info.N_items = mac_bits_2_value(&msg_ptr, 7) / 2;
                        }
                    }else{
                        pdu->subheader[pdu->N_subheaders].payload.mch_sched_info.N_items = 0;
                    }
                }else{
                    if(e_bit)
                    {
                        if(mac_bits_2_value(&msg_ptr, 1)) // F
                        {
                            pdu->subheader[pdu->N_subheaders].payload.sdu.N_bits = mac_bits_2_value(&msg_ptr, 15) * 8;
                        }else{
                            pdu->subheader[pdu->N_subheaders].payload.sdu.N_bits = mac_bits_2_value(&msg_ptr, 7) * 8;
                        }
                    }else{
                        pdu->subheader[pdu->N_subheaders].payload.sdu.N_bits = 0;
                    }
                }
            }
            pdu->N_subheaders++;
        }

        // Unpack the control elements and SDUs
        for(i=0; i<pdu->N_subheaders; i++)
        {
            if(LIBLTE_MAC_CHAN_TYPE_DLSCH == pdu->chan_type)
            {
                if(LIBLTE_MAC_DLSCH_ACTIVATION_DEACTIVATION_LCID == pdu->subheader[i].lcid)
                {
                    liblte_mac_unpack_activation_deactivation_ce(&msg_ptr, &pdu->subheader[i].payload.act_deact);
                }else if(LIBLTE_MAC_DLSCH_UE_CONTENTION_RESOLUTION_ID_LCID == pdu->subheader[i].lcid){
                    liblte_mac_unpack_ue_contention_resolution_id_ce(&msg_ptr, &pdu->subheader[i].payload.ue_con_res_id);
                }else if(LIBLTE_MAC_DLSCH_TA_COMMAND_LCID == pdu->subheader[i].lcid){
                    liblte_mac_unpack_ta_command_ce(&msg_ptr, &pdu->subheader[i].payload.ta_command);
                }else if(LIBLTE_MAC_DLSCH_DRX_COMMAND_LCID == pdu->subheader[i].lcid){
                    // No content for DRX Command CE
                }else if(LIBLTE_MAC_DLSCH_PADDING_LCID == pdu->subheader[i].lcid){
                    // No content for PADDING CE
                }else{ // SDU
                    if(pdu->subheader[i].payload.sdu.N_bits == 0)
                    {
                        pdu->subheader[i].payload.sdu.N_bits = msg->N_bits - (msg_ptr - msg->msg);
                    }
                    memcpy(pdu->subheader[i].payload.sdu.msg, msg_ptr, pdu->subheader[i].payload.sdu.N_bits);
                    msg_ptr += pdu->subheader[i].payload.sdu.N_bits;
                }
            }else if(LIBLTE_MAC_CHAN_TYPE_ULSCH == pdu->chan_type){
                if(LIBLTE_MAC_ULSCH_EXT_POWER_HEADROOM_REPORT_LCID == pdu->subheader[i].lcid)
                {
                    liblte_mac_unpack_ext_power_headroom_ce(&msg_ptr, &pdu->subheader[i].payload.ext_power_headroom);
                }else if(LIBLTE_MAC_ULSCH_POWER_HEADROOM_REPORT_LCID == pdu->subheader[i].lcid){
                    liblte_mac_unpack_power_headroom_ce(&msg_ptr, &pdu->subheader[i].payload.power_headroom);
                }else if(LIBLTE_MAC_ULSCH_C_RNTI_LCID == pdu->subheader[i].lcid){
                    liblte_mac_unpack_c_rnti_ce(&msg_ptr, &pdu->subheader[i].payload.c_rnti);
                }else if(LIBLTE_MAC_ULSCH_TRUNCATED_BSR_LCID == pdu->subheader[i].lcid){
                    liblte_mac_unpack_truncated_bsr_ce(&msg_ptr, &pdu->subheader[i].payload.truncated_bsr);
                }else if(LIBLTE_MAC_ULSCH_SHORT_BSR_LCID == pdu->subheader[i].lcid){
                    liblte_mac_unpack_short_bsr_ce(&msg_ptr, &pdu->subheader[i].payload.short_bsr);
                }else if(LIBLTE_MAC_ULSCH_LONG_BSR_LCID == pdu->subheader[i].lcid){
                    liblte_mac_unpack_long_bsr_ce(&msg_ptr, &pdu->subheader[i].payload.long_bsr);
                }else if(LIBLTE_MAC_ULSCH_PADDING_LCID == pdu->subheader[i].lcid){
                    // No content for PADDING CE
                }else{ // SDU
                    if(pdu->subheader[i].payload.sdu.N_bits == 0)
                    {
                        pdu->subheader[i].payload.sdu.N_bits = msg->N_bits - (msg_ptr - msg->msg);
                    }
                    memcpy(pdu->subheader[i].payload.sdu.msg, msg_ptr, pdu->subheader[i].payload.sdu.N_bits);
                    msg_ptr += pdu->subheader[i].payload.sdu.N_bits;
                }
            }else{ // LIBLTE_MAC_CHAN_TYPE_MCH == mac_pdu->chan_type
                if(LIBLTE_MAC_MCH_SCHEDULING_INFORMATION_LCID == pdu->subheader[i].lcid)
                {
                    if(pdu->subheader[i].payload.mch_sched_info.N_items == 0)
                    {
                        pdu->subheader[i].payload.mch_sched_info.N_items = ((msg->N_bits - (msg_ptr - msg->msg)) / 8) / 2;
                    }
                    liblte_mac_unpack_mch_scheduling_information_ce(&msg_ptr, &pdu->subheader[i].payload.mch_sched_info);
                }else if(LIBLTE_MAC_MCH_PADDING_LCID == pdu->subheader[i].lcid){
                    // No content for PADDING CE
                }else{ // SDU
                    if(pdu->subheader[i].payload.sdu.N_bits == 0)
                    {
                        pdu->subheader[i].payload.sdu.N_bits = msg->N_bits - (msg_ptr - msg->msg);
                    }
                    memcpy(pdu->subheader[i].payload.sdu.msg, msg_ptr, pdu->subheader[i].payload.sdu.N_bits);
                    msg_ptr += pdu->subheader[i].payload.sdu.N_bits;
                }
            }
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    PDU Name: Transparent

    Description: PDU containing a MAC SDU

    Document Reference: 36.321 v10.2.0 Section 6.1.4
*********************************************************************/
// FIXME

/*********************************************************************
    PDU Name: Random Access Response

    Description: PDU containing a MAC header and zero or more MAC
                 Random Access Responses

    Document Reference: 36.321 v10.2.0 Section 6.1.5

    Notes: Currently only supports 1 RAR per PDU
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mac_pack_random_access_response_pdu(LIBLTE_MAC_RAR_STRUCT *rar,
                                                             LIBLTE_BIT_MSG_STRUCT *pdu)
{
    LIBLTE_ERROR_ENUM  err     = LIBLTE_ERROR_INVALID_INPUTS;
    uint8             *pdu_ptr = pdu->msg;

    if(rar != NULL &&
       pdu != NULL)
    {
        if(LIBLTE_MAC_RAR_HEADER_TYPE_BI == rar->hdr_type)
        {
            // Pack Header
            mac_value_2_bits(0,             &pdu_ptr, 1); // E
            mac_value_2_bits(rar->hdr_type, &pdu_ptr, 1);
            mac_value_2_bits(0,             &pdu_ptr, 2); // R
            mac_value_2_bits(rar->BI,       &pdu_ptr, 4);

            err = LIBLTE_SUCCESS;
        }else if(LIBLTE_MAC_RAR_HEADER_TYPE_RAPID == rar->hdr_type){
            // Pack Header
            mac_value_2_bits(0,             &pdu_ptr, 1); // E
            mac_value_2_bits(rar->hdr_type, &pdu_ptr, 1);
            mac_value_2_bits(rar->RAPID,    &pdu_ptr, 6);

            // Pack RAR
            mac_value_2_bits(0,                   &pdu_ptr, 1); // R
            mac_value_2_bits(rar->timing_adv_cmd, &pdu_ptr, 11);
            mac_value_2_bits(rar->hopping_flag,   &pdu_ptr, 1);
            mac_value_2_bits(rar->rba,            &pdu_ptr, 10); // FIXME
            mac_value_2_bits(rar->mcs,            &pdu_ptr, 4); // FIXME
            mac_value_2_bits(rar->tpc_command,    &pdu_ptr, 3);
            mac_value_2_bits(rar->ul_delay,       &pdu_ptr, 1);
            mac_value_2_bits(rar->csi_req,        &pdu_ptr, 1);
            mac_value_2_bits(rar->temp_c_rnti,    &pdu_ptr, 16);

            err = LIBLTE_SUCCESS;
        }

        pdu->N_bits = pdu_ptr - pdu->msg;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mac_unpack_random_access_response_pdu(LIBLTE_BIT_MSG_STRUCT *pdu,
                                                               LIBLTE_MAC_RAR_STRUCT *rar)
{
    LIBLTE_ERROR_ENUM  err     = LIBLTE_ERROR_INVALID_INPUTS;
    uint8             *pdu_ptr = pdu->msg;

    if(pdu != NULL &&
       rar != NULL)
    {
        mac_bits_2_value(&pdu_ptr, 1); // E
        rar->hdr_type = (LIBLTE_MAC_RAR_HEADER_TYPE_ENUM)mac_bits_2_value(&pdu_ptr, 1);
        if(LIBLTE_MAC_RAR_HEADER_TYPE_BI == rar->hdr_type)
        {
            mac_bits_2_value(&pdu_ptr, 2); // R
            rar->BI = mac_bits_2_value(&pdu_ptr, 4);

            err = LIBLTE_SUCCESS;
        }else if(LIBLTE_MAC_RAR_HEADER_TYPE_RAPID == rar->hdr_type){
            // Unpack header
            rar->RAPID = mac_bits_2_value(&pdu_ptr, 6);

            // Unpack RAR
            mac_bits_2_value(&pdu_ptr, 1); // R
            rar->timing_adv_cmd = mac_bits_2_value(&pdu_ptr, 11);
            rar->hopping_flag   = (LIBLTE_MAC_RAR_HOPPING_ENUM)mac_bits_2_value(&pdu_ptr, 1);
            rar->rba            = mac_bits_2_value(&pdu_ptr, 10); // FIXME
            rar->mcs            = mac_bits_2_value(&pdu_ptr, 4); // FIXME
            rar->tpc_command    = (LIBLTE_MAC_RAR_TPC_COMMAND_ENUM)mac_bits_2_value(&pdu_ptr, 3);
            rar->ul_delay       = (LIBLTE_MAC_RAR_UL_DELAY_ENUM)mac_bits_2_value(&pdu_ptr, 1);
            rar->csi_req        = (LIBLTE_MAC_RAR_CSI_REQ_ENUM)mac_bits_2_value(&pdu_ptr, 1);
            rar->temp_c_rnti    = mac_bits_2_value(&pdu_ptr, 16);

            err = LIBLTE_SUCCESS;
        }
    }

    return(err);
}

/*******************************************************************************
                              LOCAL FUNCTIONS
*******************************************************************************/

/*********************************************************************
    Name: mac_value_2_bits

    Description: Converts a value to a bit string
*********************************************************************/
void mac_value_2_bits(uint32   value,
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
    Name: mac_bits_2_value

    Description: Converts a bit string to a value
*********************************************************************/
uint32 mac_bits_2_value(uint8  **bits,
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
