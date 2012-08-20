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
    06/02/2012    Ben Wojtowicz    Added message and IE packing
    06/09/2012    Ben Wojtowicz    Added the number of bits used to SIB1 pack
    08/19/2012    Ben Wojtowicz    Added functionality to support SIB2, SIB3,
                                   SIB4, and SIB8 packing and unpacking

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
void rrc_value_2_bits(uint32   value,
                      uint8  **bits,
                      uint32   N_bits);

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
    IE Name: MBSFN Subframe Config

    Description: Defines subframes that are reserved for MBSFN in
                 downlink.

    Document Reference: 36.331 v10.0.0 Section 6.3.7
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_mbsfn_subframe_config_ie(LIBLTE_RRC_MBSFN_SUBFRAME_CONFIG_STRUCT  *mbsfn_subfr_cnfg,
                                                           uint8                                   **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(mbsfn_subfr_cnfg != NULL &&
       ie_ptr           != NULL)
    {
        rrc_value_2_bits(mbsfn_subfr_cnfg->radio_fr_alloc_period,  ie_ptr, 3);
        rrc_value_2_bits(mbsfn_subfr_cnfg->radio_fr_alloc_offset,  ie_ptr, 3);
        rrc_value_2_bits(mbsfn_subfr_cnfg->subfr_alloc_num_frames, ie_ptr, 1);
        if(LIBLTE_RRC_SUBFRAME_ALLOCATION_NUM_FRAMES_ONE == mbsfn_subfr_cnfg->subfr_alloc_num_frames)
        {
            rrc_value_2_bits(mbsfn_subfr_cnfg->subfr_alloc, ie_ptr, 6);
        }else{
            rrc_value_2_bits(mbsfn_subfr_cnfg->subfr_alloc, ie_ptr, 24);
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_mbsfn_subframe_config_ie(uint8                                   **ie_ptr,
                                                             LIBLTE_RRC_MBSFN_SUBFRAME_CONFIG_STRUCT  *mbsfn_subfr_cnfg)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr           != NULL &&
       mbsfn_subfr_cnfg != NULL)
    {
        mbsfn_subfr_cnfg->radio_fr_alloc_period  = (LIBLTE_RRC_RADIO_FRAME_ALLOCATION_PERIOD_ENUM)rrc_bits_2_value(ie_ptr, 3);
        mbsfn_subfr_cnfg->radio_fr_alloc_offset  = rrc_bits_2_value(ie_ptr, 3);
        mbsfn_subfr_cnfg->subfr_alloc_num_frames = (LIBLTE_RRC_SUBFRAME_ALLOCATION_NUM_FRAMES_ENUM)rrc_bits_2_value(ie_ptr, 1);
        if(LIBLTE_RRC_SUBFRAME_ALLOCATION_NUM_FRAMES_ONE == mbsfn_subfr_cnfg->subfr_alloc_num_frames)
        {
            mbsfn_subfr_cnfg->subfr_alloc = rrc_bits_2_value(ie_ptr, 6);
        }else{
            mbsfn_subfr_cnfg->subfr_alloc = rrc_bits_2_value(ie_ptr, 24);
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Filter Coefficient

    Description: Specifies the measurement filtering coefficient.

    Document Reference: 36.331 v10.0.0 Section 6.3.6
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_filter_coefficient_ie(LIBLTE_RRC_FILTER_COEFFICIENT_ENUM   filter_coeff,
                                                        uint8                              **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        // Extension indicator
        rrc_value_2_bits(0, ie_ptr, 1);

        rrc_value_2_bits(filter_coeff, ie_ptr, 4);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_filter_coefficient_ie(uint8                              **ie_ptr,
                                                          LIBLTE_RRC_FILTER_COEFFICIENT_ENUM  *filter_coeff)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr       != NULL &&
       filter_coeff != NULL)
    {
        // Extension indicator
        rrc_bits_2_value(ie_ptr, 1);

        *filter_coeff = (LIBLTE_RRC_FILTER_COEFFICIENT_ENUM)rrc_bits_2_value(ie_ptr, 4);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: MMEC

    Description: Identifies an MME within the scope of an MME group
                 within a PLMN.

    Document Reference: 36.331 v10.0.0 Section 6.3.6
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_mmec_ie(uint8   mmec,
                                          uint8 **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(mmec, ie_ptr, 8);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_mmec_ie(uint8 **ie_ptr,
                                            uint8  *mmec)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL &&
       mmec   != NULL)
    {
        *mmec = rrc_bits_2_value(ie_ptr, 8);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Neigh Cell Config

    Description: Provides the information related to MBSFN and TDD
                 UL/DL configuration of neighbor cells.

    Document Reference: 36.331 v10.0.0 Section 6.3.6
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_neigh_cell_config_ie(uint8   neigh_cell_config,
                                                       uint8 **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(neigh_cell_config, ie_ptr, 2);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_neigh_cell_config_ie(uint8 **ie_ptr,
                                                         uint8  *neigh_cell_config)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr            != NULL &&
       neigh_cell_config != NULL)
    {
        *neigh_cell_config = rrc_bits_2_value(ie_ptr, 2);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: UE Timers and Constants

    Description: Contains timers and constants used by the UE in
                 either RRC_CONNECTED or RRC_IDLE.

    Document Reference: 36.331 v10.0.0 Section 6.3.6
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_ue_timers_and_constants_ie(LIBLTE_RRC_UE_TIMERS_AND_CONSTANTS_STRUCT  *ue_timers_and_constants,
                                                             uint8                                     **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ue_timers_and_constants != NULL &&
       ie_ptr                  != NULL)
    {
        // Extension indicator
        rrc_value_2_bits(0, ie_ptr, 1);

        rrc_value_2_bits(ue_timers_and_constants->t300, ie_ptr, 3);
        rrc_value_2_bits(ue_timers_and_constants->t301, ie_ptr, 3);
        rrc_value_2_bits(ue_timers_and_constants->t310, ie_ptr, 3);
        rrc_value_2_bits(ue_timers_and_constants->n310, ie_ptr, 3);
        rrc_value_2_bits(ue_timers_and_constants->t311, ie_ptr, 3);
        rrc_value_2_bits(ue_timers_and_constants->n311, ie_ptr, 3);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_ue_timers_and_constants_ie(uint8                                     **ie_ptr,
                                                               LIBLTE_RRC_UE_TIMERS_AND_CONSTANTS_STRUCT  *ue_timers_and_constants)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr                  != NULL &&
       ue_timers_and_constants != NULL)
    {
        // Extension indicator
        rrc_bits_2_value(ie_ptr, 1);

        ue_timers_and_constants->t300 = (LIBLTE_RRC_T300_ENUM)rrc_bits_2_value(ie_ptr, 3);
        ue_timers_and_constants->t301 = (LIBLTE_RRC_T301_ENUM)rrc_bits_2_value(ie_ptr, 3);
        ue_timers_and_constants->t310 = (LIBLTE_RRC_T310_ENUM)rrc_bits_2_value(ie_ptr, 3);
        ue_timers_and_constants->n310 = (LIBLTE_RRC_N310_ENUM)rrc_bits_2_value(ie_ptr, 3);
        ue_timers_and_constants->t311 = (LIBLTE_RRC_T311_ENUM)rrc_bits_2_value(ie_ptr, 3);
        ue_timers_and_constants->n311 = (LIBLTE_RRC_N311_ENUM)rrc_bits_2_value(ie_ptr, 3);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Allowed Meas Bandwidth

    Description: Indicates the maximum allowed measurement bandwidth
                 on a carrier frequency as defined by the parameter
                 Transmission Bandwidth Configuration.

    Document Reference: 36.331 v10.0.0 Section 6.3.5
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_allowed_meas_bandwidth_ie(LIBLTE_RRC_ALLOWED_MEAS_BANDWIDTH_ENUM   allowed_meas_bw,
                                                            uint8                                  **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(allowed_meas_bw, ie_ptr, 3);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_allowed_meas_bandwidth_ie(uint8                                  **ie_ptr,
                                                              LIBLTE_RRC_ALLOWED_MEAS_BANDWIDTH_ENUM  *allowed_meas_bw)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr          != NULL &&
       allowed_meas_bw != NULL)
    {
        *allowed_meas_bw = (LIBLTE_RRC_ALLOWED_MEAS_BANDWIDTH_ENUM)rrc_bits_2_value(ie_ptr, 3);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Hysteresis

    Description: Used within the entry and leave condition of an
                 event triggered reporting condition.

    Document Reference: 36.331 v10.0.0 Section 6.3.5
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_hysteresis_ie(uint8   hysteresis,
                                                uint8 **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(hysteresis, ie_ptr, 5);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_hysteresis_ie(uint8 **ie_ptr,
                                                  uint8  *hysteresis)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr     != NULL &&
       hysteresis != NULL)
    {
        *hysteresis = rrc_bits_2_value(ie_ptr, 5);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Additional Spectrum Emission

    Description: FIXME

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_additional_spectrum_emission_ie(uint8   add_spect_em,
                                                                  uint8 **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(add_spect_em - 1, ie_ptr, 5);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_additional_spectrum_emission_ie(uint8 **ie_ptr,
                                                                    uint8  *add_spect_em)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr       != NULL &&
       add_spect_em != NULL)
    {
        *add_spect_em = rrc_bits_2_value(ie_ptr, 5) + 1;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: ARFCN value CDMA2000

    Description: Indicates the CDMA2000 carrier frequency within
                 a CDMA2000 band.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_arfcn_value_cdma2000_ie(uint16   arfcn,
                                                          uint8  **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(arfcn, ie_ptr, 11);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_arfcn_value_cdma2000_ie(uint8  **ie_ptr,
                                                            uint16  *arfcn)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL &&
       arfcn  != NULL)
    {
        *arfcn = rrc_bits_2_value(ie_ptr, 11);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: ARFCN value EUTRA

    Description: Indicates the ARFCN applicable for a downlink,
                 uplink, or bi-directional (TDD) E-UTRA carrier
                 frequency.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_arfcn_value_eutra_ie(uint16   arfcn,
                                                       uint8  **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(arfcn, ie_ptr, 16);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_arfcn_value_eutra_ie(uint8  **ie_ptr,
                                                         uint16  *arfcn)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL &&
       arfcn  != NULL)
    {
        *arfcn = rrc_bits_2_value(ie_ptr, 16);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: ARFCN value GERAN

    Description: Specifies the ARFCN value applicable for a GERAN
                 BCCH carrier frequency.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_arfcn_value_geran_ie(uint16   arfcn,
                                                       uint8  **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(arfcn, ie_ptr, 10);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_arfcn_value_geran_ie(uint8  **ie_ptr,
                                                         uint16  *arfcn)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL &&
       arfcn  != NULL)
    {
        *arfcn = rrc_bits_2_value(ie_ptr, 10);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: ARFCN value UTRA

    Description: Indicates the ARFCN applicable for a downlink (Nd,
                 FDD) or bi-directional (Nt, TDD) UTRA carrier
                 frequency.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_arfcn_value_utra_ie(uint16   arfcn,
                                                      uint8  **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(arfcn, ie_ptr, 16);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_arfcn_value_utra_ie(uint8  **ie_ptr,
                                                        uint16  *arfcn)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL &&
       arfcn  != NULL)
    {
        *arfcn = rrc_bits_2_value(ie_ptr, 16);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Band Class CDMA2000

    Description: Defines the CDMA2000 band in which the CDMA2000
                 carrier frequency can be found.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_band_class_cdma2000_ie(LIBLTE_RRC_BAND_CLASS_CDMA2000_ENUM   bc_cdma2000,
                                                         uint8                               **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        // Extension indicator
        rrc_value_2_bits(0, ie_ptr, 1);

        rrc_value_2_bits(bc_cdma2000, ie_ptr, 5);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_band_class_cdma2000_ie(uint8                               **ie_ptr,
                                                           LIBLTE_RRC_BAND_CLASS_CDMA2000_ENUM  *bc_cdma2000)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr      != NULL &&
       bc_cdma2000 != NULL)
    {
        // Extension indicator
        rrc_bits_2_value(ie_ptr, 1);

        *bc_cdma2000 = (LIBLTE_RRC_BAND_CLASS_CDMA2000_ENUM)rrc_bits_2_value(ie_ptr, 5);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Band Indicator GERAN

    Description: Indicates how to interpret an associated GERAN
                 carrier ARFCN.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_band_indicator_geran_ie(LIBLTE_RRC_BAND_INDICATOR_GERAN_ENUM   bi_geran,
                                                          uint8                                **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(bi_geran, ie_ptr, 1);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_band_indicator_geran_ie(uint8                                **ie_ptr,
                                                            LIBLTE_RRC_BAND_INDICATOR_GERAN_ENUM  *bi_geran)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr   != NULL &&
       bi_geran != NULL)
    {
        *bi_geran = (LIBLTE_RRC_BAND_INDICATOR_GERAN_ENUM)rrc_bits_2_value(ie_ptr, 1);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Carrier Freq CDMA2000

    Description: Provides the CDMA2000 carrier information.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_carrier_freq_cdma2000_ie(LIBLTE_RRC_CARRIER_FREQ_CDMA2000_STRUCT  *carrier_freq,
                                                           uint8                                   **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(carrier_freq != NULL &&
       ie_ptr       != NULL)
    {
        liblte_rrc_pack_band_class_cdma2000_ie(carrier_freq->bandclass, ie_ptr);
        liblte_rrc_pack_arfcn_value_cdma2000_ie(carrier_freq->arfcn, ie_ptr);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_carrier_freq_cdma2000_ie(uint8                                   **ie_ptr,
                                                             LIBLTE_RRC_CARRIER_FREQ_CDMA2000_STRUCT  *carrier_freq)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr       != NULL &&
       carrier_freq != NULL)
    {
        liblte_rrc_unpack_band_class_cdma2000_ie(ie_ptr, &carrier_freq->bandclass);
        liblte_rrc_unpack_arfcn_value_cdma2000_ie(ie_ptr, &carrier_freq->arfcn);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Carrier Freq GERAN

    Description: Provides an unambiguous carrier frequency description
                 of a GERAN cell.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_carrier_freq_geran_ie(LIBLTE_RRC_CARRIER_FREQ_GERAN_STRUCT  *carrier_freq,
                                                        uint8                                **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(carrier_freq != NULL &&
       ie_ptr       != NULL)
    {
        liblte_rrc_pack_arfcn_value_geran_ie(carrier_freq->arfcn, ie_ptr);
        liblte_rrc_pack_band_indicator_geran_ie(carrier_freq->band_indicator, ie_ptr);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_carrier_freq_geran_ie(uint8                                **ie_ptr,
                                                          LIBLTE_RRC_CARRIER_FREQ_GERAN_STRUCT  *carrier_freq)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr       != NULL &&
       carrier_freq != NULL)
    {
        liblte_rrc_unpack_arfcn_value_geran_ie(ie_ptr, &carrier_freq->arfcn);
        liblte_rrc_unpack_band_indicator_geran_ie(ie_ptr, &carrier_freq->band_indicator);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: CDMA2000 Type

    Description: Describes the type of CDMA2000 network.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_cdma2000_type_ie(LIBLTE_RRC_CDMA2000_TYPE_ENUM   cdma2000_type,
                                                   uint8                         **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(cdma2000_type, ie_ptr, 1);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_cdma2000_type_ie(uint8                         **ie_ptr,
                                                     LIBLTE_RRC_CDMA2000_TYPE_ENUM  *cdma2000_type)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr        != NULL &&
       cdma2000_type != NULL)
    {
        *cdma2000_type = (LIBLTE_RRC_CDMA2000_TYPE_ENUM)rrc_bits_2_value(ie_ptr, 1);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Cell Identity

    Description: Unambiguously identifies a cell within a PLMN.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_cell_identity_ie(uint32   cell_id,
                                                   uint8  **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(cell_id, ie_ptr, 28);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_cell_identity_ie(uint8  **ie_ptr,
                                                     uint32  *cell_id)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr  != NULL &&
       cell_id != NULL)
    {
        *cell_id = rrc_bits_2_value(ie_ptr, 28);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Cell Reselection Priority

    Description: Contains the absolute priority of the concerned
                 carrier frequency/set of frequencies (GERAN)/
                 bandclass (CDMA2000), as used by the cell
                 reselection procedure.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_cell_reselection_priority_ie(uint8   cell_resel_prio,
                                                               uint8 **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(cell_resel_prio, ie_ptr, 3);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_cell_reselection_priority_ie(uint8 **ie_ptr,
                                                                 uint8  *cell_resel_prio)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr          != NULL &&
       cell_resel_prio != NULL)
    {
        *cell_resel_prio = rrc_bits_2_value(ie_ptr, 3);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: CSFB Registration Param 1xRTT

    Description: Indicates whether or not the UE shall perform a
                 CDMA2000 1xRTT pre-registration if the UE does not
                 have a valid/current pre-registration.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_csfb_registration_param_1xrtt_ie(LIBLTE_RRC_CSFB_REGISTRATION_PARAM_1XRTT_STRUCT  *csfb_reg_param,
                                                                   uint8                                           **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(csfb_reg_param != NULL &&
       ie_ptr         != NULL)
    {
        rrc_value_2_bits(csfb_reg_param->sid,             ie_ptr, 15);
        rrc_value_2_bits(csfb_reg_param->nid,             ie_ptr, 16);
        rrc_value_2_bits(csfb_reg_param->multiple_sid,    ie_ptr,  1);
        rrc_value_2_bits(csfb_reg_param->multiple_nid,    ie_ptr,  1);
        rrc_value_2_bits(csfb_reg_param->home_reg,        ie_ptr,  1);
        rrc_value_2_bits(csfb_reg_param->foreign_sid_reg, ie_ptr,  1);
        rrc_value_2_bits(csfb_reg_param->foreign_nid_reg, ie_ptr,  1);
        rrc_value_2_bits(csfb_reg_param->param_reg,       ie_ptr,  1);
        rrc_value_2_bits(csfb_reg_param->power_up_reg,    ie_ptr,  1);
        rrc_value_2_bits(csfb_reg_param->reg_period,      ie_ptr,  7);
        rrc_value_2_bits(csfb_reg_param->reg_zone,        ie_ptr, 12);
        rrc_value_2_bits(csfb_reg_param->total_zone,      ie_ptr,  3);
        rrc_value_2_bits(csfb_reg_param->zone_timer,      ie_ptr,  3);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_csfb_registration_param_1xrtt_ie(uint8                                           **ie_ptr,
                                                                     LIBLTE_RRC_CSFB_REGISTRATION_PARAM_1XRTT_STRUCT  *csfb_reg_param)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr         != NULL &&
       csfb_reg_param != NULL)
    {
        csfb_reg_param->sid             = rrc_bits_2_value(ie_ptr, 15);
        csfb_reg_param->nid             = rrc_bits_2_value(ie_ptr, 16);
        csfb_reg_param->multiple_sid    = rrc_bits_2_value(ie_ptr,  1);
        csfb_reg_param->multiple_nid    = rrc_bits_2_value(ie_ptr,  1);
        csfb_reg_param->home_reg        = rrc_bits_2_value(ie_ptr,  1);
        csfb_reg_param->foreign_sid_reg = rrc_bits_2_value(ie_ptr,  1);
        csfb_reg_param->foreign_nid_reg = rrc_bits_2_value(ie_ptr,  1);
        csfb_reg_param->param_reg       = rrc_bits_2_value(ie_ptr,  1);
        csfb_reg_param->power_up_reg    = rrc_bits_2_value(ie_ptr,  1);
        csfb_reg_param->reg_period      = rrc_bits_2_value(ie_ptr,  7);
        csfb_reg_param->reg_zone        = rrc_bits_2_value(ie_ptr, 12);
        csfb_reg_param->total_zone      = rrc_bits_2_value(ie_ptr,  3);
        csfb_reg_param->zone_timer      = rrc_bits_2_value(ie_ptr,  3);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_pack_csfb_registration_param_1xrtt_v920_ie(LIBLTE_RRC_CSFB_REGISTRATION_PARAM_1XRTT_V920_STRUCT  *csfb_reg_param,
                                                                        uint8                                                **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(csfb_reg_param != NULL &&
       ie_ptr         != NULL)
    {
        rrc_value_2_bits(csfb_reg_param->power_down_reg, ie_ptr, 1);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_csfb_registration_param_1xrtt_v920_ie(uint8                                                **ie_ptr,
                                                                          LIBLTE_RRC_CSFB_REGISTRATION_PARAM_1XRTT_V920_STRUCT  *csfb_reg_param)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr         != NULL &&
       csfb_reg_param != NULL)
    {
        csfb_reg_param->power_down_reg = (LIBLTE_RRC_POWER_DOWN_REG_R9_ENUM)rrc_bits_2_value(ie_ptr, 1);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Cell Global ID EUTRA

    Description: Specifies the Evolved Cell Global Identifier (ECGI),
                 the globally unique identity of a cell in E-UTRA.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_cell_global_id_eutra_ie(LIBLTE_RRC_CELL_GLOBAL_ID_EUTRA_STRUCT  *cell_global_id,
                                                          uint8                                  **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(cell_global_id != NULL &&
       ie_ptr         != NULL)
    {
        liblte_rrc_pack_plmn_identity_ie(&cell_global_id->plmn_id, ie_ptr);
        liblte_rrc_pack_cell_identity_ie(cell_global_id->cell_id, ie_ptr);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_cell_global_id_eutra_ie(uint8                                  **ie_ptr,
                                                            LIBLTE_RRC_CELL_GLOBAL_ID_EUTRA_STRUCT  *cell_global_id)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr         != NULL &&
       cell_global_id != NULL)
    {
        liblte_rrc_unpack_plmn_identity_ie(ie_ptr, &cell_global_id->plmn_id);
        liblte_rrc_unpack_cell_identity_ie(ie_ptr, &cell_global_id->cell_id);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Cell Global ID UTRA

    Description: Specifies the global UTRAN Cell Identifier, the
                 globally unique identity of a cell in UTRA.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_cell_global_id_utra_ie(LIBLTE_RRC_CELL_GLOBAL_ID_UTRA_STRUCT  *cell_global_id,
                                                         uint8                                 **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(cell_global_id != NULL &&
       ie_ptr         != NULL)
    {
        liblte_rrc_pack_plmn_identity_ie(&cell_global_id->plmn_id, ie_ptr);
        rrc_value_2_bits(cell_global_id->cell_id, ie_ptr, 28);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_cell_global_id_utra_ie(uint8                                 **ie_ptr,
                                                           LIBLTE_RRC_CELL_GLOBAL_ID_UTRA_STRUCT  *cell_global_id)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr         != NULL &&
       cell_global_id != NULL)
    {
        liblte_rrc_unpack_plmn_identity_ie(ie_ptr, &cell_global_id->plmn_id);
        cell_global_id->cell_id = rrc_bits_2_value(ie_ptr, 28);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Cell Global ID GERAN

    Description: Specifies the Cell Global Identity (CGI), the
                 globally unique identity of a cell in GERAN.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_cell_global_id_geran_ie(LIBLTE_RRC_CELL_GLOBAL_ID_GERAN_STRUCT  *cell_global_id,
                                                          uint8                                  **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(cell_global_id != NULL &&
       ie_ptr         != NULL)
    {
        liblte_rrc_pack_plmn_identity_ie(&cell_global_id->plmn_id, ie_ptr);
        rrc_value_2_bits(cell_global_id->lac,     ie_ptr, 16);
        rrc_value_2_bits(cell_global_id->cell_id, ie_ptr, 16);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_cell_global_id_geran_ie(uint8                                  **ie_ptr,
                                                            LIBLTE_RRC_CELL_GLOBAL_ID_GERAN_STRUCT  *cell_global_id)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr         != NULL &&
       cell_global_id != NULL)
    {
        liblte_rrc_unpack_plmn_identity_ie(ie_ptr, &cell_global_id->plmn_id);
        cell_global_id->lac     = rrc_bits_2_value(ie_ptr, 16);
        cell_global_id->cell_id = rrc_bits_2_value(ie_ptr, 16);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Cell Global ID CDMA2000

    Description: Specifies the Cell Global Identity (CGI), the
                 globally unique identity of a cell in CDMA2000.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_cell_global_id_cdma2000_ie(LIBLTE_RRC_CELL_GLOBAL_ID_CDMA2000_STRUCT  *cell_global_id,
                                                             uint8                                     **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(cell_global_id != NULL &&
       ie_ptr         != NULL)
    {
        rrc_value_2_bits((uint32)(cell_global_id->onexrtt >> 15),       ie_ptr, 32);
        rrc_value_2_bits((uint32)(cell_global_id->onexrtt & 0x7FFFULL), ie_ptr, 15);
        rrc_value_2_bits(cell_global_id->hrpd[0],                       ie_ptr, 32);
        rrc_value_2_bits(cell_global_id->hrpd[1],                       ie_ptr, 32);
        rrc_value_2_bits(cell_global_id->hrpd[2],                       ie_ptr, 32);
        rrc_value_2_bits(cell_global_id->hrpd[3],                       ie_ptr, 32);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_cell_global_id_cdma2000_ie(uint8                                     **ie_ptr,
                                                               LIBLTE_RRC_CELL_GLOBAL_ID_CDMA2000_STRUCT  *cell_global_id)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr         != NULL &&
       cell_global_id != NULL)
    {
        cell_global_id->onexrtt  = (uint64)rrc_bits_2_value(ie_ptr, 32) << 15;
        cell_global_id->onexrtt |= (uint64)rrc_bits_2_value(ie_ptr, 15);
        cell_global_id->hrpd[0]  = rrc_bits_2_value(ie_ptr, 32);
        cell_global_id->hrpd[1]  = rrc_bits_2_value(ie_ptr, 32);
        cell_global_id->hrpd[2]  = rrc_bits_2_value(ie_ptr, 32);
        cell_global_id->hrpd[3]  = rrc_bits_2_value(ie_ptr, 32);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: CSG Identity

    Description: Identifies a Closed Subscriber Group

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_csg_identity_ie(uint32   csg_id,
                                                  uint8  **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(csg_id, ie_ptr, 27);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_csg_identity_ie(uint8  **ie_ptr,
                                                    uint32  *csg_id)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL &&
       csg_id != NULL)
    {
        *csg_id = rrc_bits_2_value(ie_ptr, 27);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Mobility State Parameters

    Description: Contains parameters to determine UE mobility state.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_mobility_state_parameters_ie(LIBLTE_RRC_MOBILITY_STATE_PARAMETERS_STRUCT  *mobility_state_params,
                                                               uint8                                       **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(mobility_state_params != NULL &&
       ie_ptr                != NULL)
    {
        rrc_value_2_bits(mobility_state_params->t_eval,                   ie_ptr, 3);
        rrc_value_2_bits(mobility_state_params->t_hyst_normal,            ie_ptr, 3);
        rrc_value_2_bits(mobility_state_params->n_cell_change_medium - 1, ie_ptr, 4);
        rrc_value_2_bits(mobility_state_params->n_cell_change_high - 1,   ie_ptr, 4);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_mobility_state_parameters_ie(uint8                                       **ie_ptr,
                                                                 LIBLTE_RRC_MOBILITY_STATE_PARAMETERS_STRUCT  *mobility_state_params)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr                != NULL &&
       mobility_state_params != NULL)
    {
        mobility_state_params->t_eval               = (LIBLTE_RRC_T_EVALUATION_ENUM)rrc_bits_2_value(ie_ptr, 3);
        mobility_state_params->t_hyst_normal        = (LIBLTE_RRC_T_HYST_NORMAL_ENUM)rrc_bits_2_value(ie_ptr, 3);
        mobility_state_params->n_cell_change_medium = rrc_bits_2_value(ie_ptr, 4) + 1;
        mobility_state_params->n_cell_change_high   = rrc_bits_2_value(ie_ptr, 4) + 1;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Phys Cell ID

    Description: Indicates the physical layer identity of the cell.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_phys_cell_id_ie(uint16   phys_cell_id,
                                                  uint8  **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(phys_cell_id, ie_ptr, 9);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_phys_cell_id_ie(uint8  **ie_ptr,
                                                    uint16  *phys_cell_id)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr       != NULL &&
       phys_cell_id != NULL)
    {
        *phys_cell_id = rrc_bits_2_value(ie_ptr, 9);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Phys Cell ID Range

    Description: Encodes either a single or a range of physical cell
                 identities.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_phys_cell_id_range_ie(LIBLTE_RRC_PHYS_CELL_ID_RANGE_STRUCT  *phys_cell_id_range,
                                                        uint8                                **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(phys_cell_id_range != NULL &&
       ie_ptr             != NULL)
    {
        liblte_rrc_pack_phys_cell_id_ie(phys_cell_id_range->start, ie_ptr);

        if(LIBLTE_RRC_PHYS_CELL_ID_RANGE_N1 != phys_cell_id_range->range)
        {
            rrc_value_2_bits(1,                         ie_ptr, 1);
            rrc_value_2_bits(phys_cell_id_range->range, ie_ptr, 4);
        }else{
            rrc_value_2_bits(0, ie_ptr, 1);
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_phys_cell_id_range_ie(uint8                                **ie_ptr,
                                                          LIBLTE_RRC_PHYS_CELL_ID_RANGE_STRUCT  *phys_cell_id_range)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;
    bool              opt;

    if(ie_ptr             != NULL &&
       phys_cell_id_range != NULL)
    {
        liblte_rrc_unpack_phys_cell_id_ie(ie_ptr, &phys_cell_id_range->start);

        opt = rrc_bits_2_value(ie_ptr, 1);
        if(true == opt)
        {
            phys_cell_id_range->range = (LIBLTE_RRC_PHYS_CELL_ID_RANGE_ENUM)rrc_bits_2_value(ie_ptr, 4);
        }else{
            phys_cell_id_range->range = LIBLTE_RRC_PHYS_CELL_ID_RANGE_N1;
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Phys Cell ID CDMA2000

    Description: Identifies the PN offset that represents the
                 "Physical cell identity" in CDMA2000.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_phys_cell_id_cdma2000_ie(uint16   phys_cell_id,
                                                           uint8  **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(phys_cell_id, ie_ptr, 9);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_phys_cell_id_cdma2000_ie(uint8  **ie_ptr,
                                                             uint16  *phys_cell_id)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr       != NULL &&
       phys_cell_id != NULL)
    {
        *phys_cell_id = rrc_bits_2_value(ie_ptr, 9);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Phys Cell ID GERAN

    Description: Contains the Base Station Identity Code (BSIC).

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_phys_cell_id_geran_ie(LIBLTE_RRC_PHYS_CELL_ID_GERAN_STRUCT  *phys_cell_id,
                                                        uint8                                **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(phys_cell_id != NULL &&
       ie_ptr       != NULL)
    {
        rrc_value_2_bits(phys_cell_id->ncc, ie_ptr, 3);
        rrc_value_2_bits(phys_cell_id->bcc, ie_ptr, 3);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_phys_cell_id_geran_ie(uint8                                **ie_ptr,
                                                          LIBLTE_RRC_PHYS_CELL_ID_GERAN_STRUCT  *phys_cell_id)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr       != NULL &&
       phys_cell_id != NULL)
    {
        phys_cell_id->ncc = rrc_bits_2_value(ie_ptr, 3);
        phys_cell_id->bcc = rrc_bits_2_value(ie_ptr, 3);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Phys Cell ID UTRA FDD

    Description: Indicates the physical layer identity of the cell.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_phys_cell_id_utra_fdd_ie(uint16   phys_cell_id,
                                                           uint8  **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(phys_cell_id, ie_ptr, 9);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_phys_cell_id_utra_fdd_ie(uint8  **ie_ptr,
                                                             uint16  *phys_cell_id)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr       != NULL &&
       phys_cell_id != NULL)
    {
        *phys_cell_id = rrc_bits_2_value(ie_ptr, 9);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Phys Cell ID UTRA TDD

    Description: Indicates the physical layer identity of the cell.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_phys_cell_id_utra_tdd_ie(uint8   phys_cell_id,
                                                           uint8 **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(phys_cell_id, ie_ptr, 7);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_phys_cell_id_utra_tdd_ie(uint8 **ie_ptr,
                                                             uint8  *phys_cell_id)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr       != NULL &&
       phys_cell_id != NULL)
    {
        *phys_cell_id = rrc_bits_2_value(ie_ptr, 7);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: PLMN Identity

    Description: Identifies a Public Land Mobile Network.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_plmn_identity_ie(LIBLTE_RRC_PLMN_IDENTITY_STRUCT  *plmn_id,
                                                   uint8                           **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;
    uint16            mnc;
    uint8             mcc_opt = true;
    uint8             mnc_size;

    if(plmn_id != NULL &&
       ie_ptr  != NULL)
    {
        rrc_value_2_bits(mcc_opt, ie_ptr, 1);

        if(true == mcc_opt)
        {
            rrc_value_2_bits(plmn_id->mcc, ie_ptr, 12);
        }

        if((plmn_id->mnc & 0xFF00) == 0xFF00)
        {
            mnc_size = 8;
            mnc      = plmn_id->mnc & 0x00FF;
        }else{
            mnc_size = 12;
            mnc      = plmn_id->mnc & 0x0FFF;
        }
        rrc_value_2_bits((mnc_size/4)-2, ie_ptr, 1);
        rrc_value_2_bits(mnc,            ie_ptr, mnc_size);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_plmn_identity_ie(uint8                           **ie_ptr,
                                                     LIBLTE_RRC_PLMN_IDENTITY_STRUCT  *plmn_id)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;
    uint8             mcc_opt;
    uint8             mnc_size;

    if(ie_ptr  != NULL &&
       plmn_id != NULL)
    {
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

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Pre Registration Info HRPD

    Description: FIXME

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_pre_registration_info_hrpd_ie(LIBLTE_RRC_PRE_REGISTRATION_INFO_HRPD_STRUCT  *pre_reg_info_hrpd,
                                                                uint8                                        **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;
    uint32            i;

    if(pre_reg_info_hrpd != NULL &&
       ie_ptr            != NULL)
    {
        // Optional indicators
        rrc_value_2_bits(pre_reg_info_hrpd->pre_reg_zone_id_present, ie_ptr, 1);
        if(0 != pre_reg_info_hrpd->secondary_pre_reg_zone_id_list_size)
        {
            rrc_value_2_bits(1, ie_ptr, 1);
        }else{
            rrc_value_2_bits(0, ie_ptr, 1);
        }

        rrc_value_2_bits(pre_reg_info_hrpd->pre_reg_allowed, ie_ptr, 1);

        if(true == pre_reg_info_hrpd->pre_reg_zone_id_present)
        {
            rrc_value_2_bits(pre_reg_info_hrpd->pre_reg_zone_id, ie_ptr, 8);
        }

        if(0 != pre_reg_info_hrpd->secondary_pre_reg_zone_id_list_size)
        {
            rrc_value_2_bits(pre_reg_info_hrpd->secondary_pre_reg_zone_id_list_size - 1, ie_ptr, 1);
            for(i=0; i<pre_reg_info_hrpd->secondary_pre_reg_zone_id_list_size; i++)
            {
                rrc_value_2_bits(pre_reg_info_hrpd->secondary_pre_reg_zone_id_list[i], ie_ptr, 8);
            }
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_pre_registration_info_hrpd_ie(uint8                                        **ie_ptr,
                                                                  LIBLTE_RRC_PRE_REGISTRATION_INFO_HRPD_STRUCT  *pre_reg_info_hrpd)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;
    uint32            i;
    bool              secondary_pre_reg_zone_id_opt;

    if(ie_ptr            != NULL &&
       pre_reg_info_hrpd != NULL)
    {
        // Optional indicators
        pre_reg_info_hrpd->pre_reg_zone_id_present = rrc_bits_2_value(ie_ptr, 1);
        secondary_pre_reg_zone_id_opt              = rrc_bits_2_value(ie_ptr, 1);

        pre_reg_info_hrpd->pre_reg_allowed = rrc_bits_2_value(ie_ptr, 1);

        if(true == pre_reg_info_hrpd->pre_reg_zone_id_present)
        {
            pre_reg_info_hrpd->pre_reg_zone_id = rrc_bits_2_value(ie_ptr, 8);
        }

        if(true == secondary_pre_reg_zone_id_opt)
        {
            pre_reg_info_hrpd->secondary_pre_reg_zone_id_list_size = rrc_bits_2_value(ie_ptr, 1) + 1;
            for(i=0; i<pre_reg_info_hrpd->secondary_pre_reg_zone_id_list_size; i++)
            {
                pre_reg_info_hrpd->secondary_pre_reg_zone_id_list[i] = rrc_bits_2_value(ie_ptr, 8);
            }
        }else{
            pre_reg_info_hrpd->secondary_pre_reg_zone_id_list_size = 0;
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Q Qual Min

    Description: Indicates for cell selection/re-selection the
                 required minimum received RSRQ level in the (E-UTRA)
                 cell.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_q_qual_min_ie(int8    q_qual_min,
                                                uint8 **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(q_qual_min + 34, ie_ptr, 5);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_q_qual_min_ie(uint8 **ie_ptr,
                                                  int8   *q_qual_min)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr     != NULL &&
       q_qual_min != NULL)
    {
        *q_qual_min = (int8)rrc_bits_2_value(ie_ptr, 5) - 34;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Q Rx Lev Min

    Description: Indicates the required minimum received RSRP level in
                 the (E-UTRA) cell for cell selection/re-selection.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_q_rx_lev_min_ie(int16   q_rx_lev_min,
                                                  uint8 **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits((q_rx_lev_min / 2) + 70, ie_ptr, 6);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_q_rx_lev_min_ie(uint8 **ie_ptr,
                                                    int16  *q_rx_lev_min)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr       != NULL &&
       q_rx_lev_min != NULL)
    {
        *q_rx_lev_min = ((int16)rrc_bits_2_value(ie_ptr, 6) - 70) * 2;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Q Offset Range

    Description: Indicates a cell or frequency specific offset to be
                 applied when evaluating candidates for cell
                 reselection or when evaluating triggering conditions
                 for measurement reporting.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_q_offset_range_ie(LIBLTE_RRC_Q_OFFSET_RANGE_ENUM   q_offset_range,
                                                    uint8                          **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(q_offset_range, ie_ptr, 5);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_q_offset_range_ie(uint8                          **ie_ptr,
                                                      LIBLTE_RRC_Q_OFFSET_RANGE_ENUM  *q_offset_range)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr         != NULL &&
       q_offset_range != NULL)
    {
        *q_offset_range = (LIBLTE_RRC_Q_OFFSET_RANGE_ENUM)rrc_bits_2_value(ie_ptr, 5);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Q Offset Range Inter RAT

    Description: Indicates a frequency specific offset to be applied
                 when evaluating triggering conditions for
                 measurement reporting.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_q_offset_range_inter_rat_ie(int8    q_offset_range_inter_rat,
                                                              uint8 **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(q_offset_range_inter_rat + 15, ie_ptr, 5);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_q_offset_range_inter_rat_ie(uint8 **ie_ptr,
                                                                int8   *q_offset_range_inter_rat)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr                   != NULL &&
       q_offset_range_inter_rat != NULL)
    {
        *q_offset_range_inter_rat = (int8)(rrc_bits_2_value(ie_ptr, 5)) - 15;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Reselection Threshold

    Description: Indicates an RX level threshold for cell reselection.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_reselection_threshold_ie(uint8   resel_thresh,
                                                           uint8 **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(resel_thresh / 2, ie_ptr, 5);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_reselection_threshold_ie(uint8 **ie_ptr,
                                                             uint8  *resel_thresh)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr       != NULL &&
       resel_thresh != NULL)
    {
        *resel_thresh = rrc_bits_2_value(ie_ptr, 5) * 2;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Reselection Threshold Q

    Description: Indicates a quality level threshold for cell
                 reselection.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_reselection_threshold_q_ie(uint8   resel_thresh_q,
                                                             uint8 **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(resel_thresh_q, ie_ptr, 5);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_reselection_threshold_q_ie(uint8 **ie_ptr,
                                                               uint8  *resel_thresh_q)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr         != NULL &&
       resel_thresh_q != NULL)
    {
        *resel_thresh_q = rrc_bits_2_value(ie_ptr, 5);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: S Cell Index

    Description: Contains a short identity, used to identify an
                 SCell.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_s_cell_index_ie(uint8   s_cell_idx,
                                                  uint8 **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(s_cell_idx - 1, ie_ptr, 3);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_s_cell_index_ie(uint8 **ie_ptr,
                                                    uint8  *s_cell_idx)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr     != NULL &&
       s_cell_idx != NULL)
    {
        *s_cell_idx = rrc_bits_2_value(ie_ptr, 3) + 1;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Serv Cell Index

    Description: Contains a short identity, used to identify a
                 serving cell.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_serv_cell_index_ie(uint8   serv_cell_idx,
                                                     uint8 **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(serv_cell_idx, ie_ptr, 3);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_serv_cell_index_ie(uint8 **ie_ptr,
                                                       uint8  *serv_cell_idx)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr        != NULL &&
       serv_cell_idx != NULL)
    {
        *serv_cell_idx = rrc_bits_2_value(ie_ptr, 3);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Speed State Scale Factors

    Description: Contains factors, to be applied when the UE is in
                 medium or high speed state, used for scaling a
                 mobility control related parameter.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_speed_state_scale_factors_ie(LIBLTE_RRC_SPEED_STATE_SCALE_FACTORS_STRUCT  *speed_state_scale_factors,
                                                               uint8                                       **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(speed_state_scale_factors != NULL &&
       ie_ptr                    != NULL)
    {
        rrc_value_2_bits(speed_state_scale_factors->sf_medium, ie_ptr, 2);
        rrc_value_2_bits(speed_state_scale_factors->sf_high,   ie_ptr, 2);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_speed_state_scale_factors_ie(uint8                                       **ie_ptr,
                                                                 LIBLTE_RRC_SPEED_STATE_SCALE_FACTORS_STRUCT  *speed_state_scale_factors)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr                    != NULL &&
       speed_state_scale_factors != NULL)
    {
        speed_state_scale_factors->sf_medium = (LIBLTE_RRC_SSSF_MEDIUM_ENUM)rrc_bits_2_value(ie_ptr, 2);
        speed_state_scale_factors->sf_high   = (LIBLTE_RRC_SSSF_HIGH_ENUM)rrc_bits_2_value(ie_ptr, 2);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: System Time Info CDMA2000

    Description: Informs the UE about the absolute time in the current
                 cell.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_system_time_info_cdma2000_ie(LIBLTE_RRC_SYSTEM_TIME_INFO_CDMA2000_STRUCT  *sys_time_info_cdma2000,
                                                               uint8                                       **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(sys_time_info_cdma2000 != NULL &&
       ie_ptr                 != NULL)
    {
        rrc_value_2_bits(sys_time_info_cdma2000->cdma_eutra_sync,   ie_ptr, 1);
        rrc_value_2_bits(sys_time_info_cdma2000->system_time_async, ie_ptr, 1);
        if(true == sys_time_info_cdma2000->system_time_async)
        {
            rrc_value_2_bits((uint32)(sys_time_info_cdma2000->system_time >> 17),     ie_ptr, 32);
            rrc_value_2_bits((uint32)(sys_time_info_cdma2000->system_time & 0x1FFFF), ie_ptr, 17);
        }else{
            rrc_value_2_bits((uint32)(sys_time_info_cdma2000->system_time >> 7),   ie_ptr, 32);
            rrc_value_2_bits((uint32)(sys_time_info_cdma2000->system_time & 0x7F), ie_ptr,  7);
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_system_time_info_cdma2000_ie(uint8                                       **ie_ptr,
                                                                 LIBLTE_RRC_SYSTEM_TIME_INFO_CDMA2000_STRUCT  *sys_time_info_cdma2000)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr                 != NULL &&
       sys_time_info_cdma2000 != NULL)
    {
        sys_time_info_cdma2000->cdma_eutra_sync   = rrc_bits_2_value(ie_ptr, 1);
        sys_time_info_cdma2000->system_time_async = rrc_bits_2_value(ie_ptr, 1);
        if(true == sys_time_info_cdma2000->system_time_async)
        {
            sys_time_info_cdma2000->system_time  = (uint64)rrc_bits_2_value(ie_ptr, 32) << 17;
            sys_time_info_cdma2000->system_time |= (uint64)rrc_bits_2_value(ie_ptr, 17);
        }else{
            sys_time_info_cdma2000->system_time  = (uint64)rrc_bits_2_value(ie_ptr, 32) << 7;
            sys_time_info_cdma2000->system_time |= (uint64)rrc_bits_2_value(ie_ptr,  7);
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Tracking Area Code

    Description: Identifies a tracking area within the scope of a
                 PLMN.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_tracking_area_code_ie(uint16   tac,
                                                        uint8  **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(tac, ie_ptr, 16);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_tracking_area_code_ie(uint8  **ie_ptr,
                                                          uint16  *tac)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL &&
       tac    != NULL)
    {
        *tac = rrc_bits_2_value(ie_ptr, 16);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: T Reselection

    Description: Contains the timer T_reselection_rat for E-UTRA,
                 UTRA, GERAN, or CDMA2000.

    Document Reference: 36.331 v10.0.0 Section 6.3.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_t_reselection_ie(uint8   t_resel,
                                                   uint8 **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(t_resel, ie_ptr, 3);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_t_reselection_ie(uint8 **ie_ptr,
                                                     uint8  *t_resel)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr  != NULL &&
       t_resel != NULL)
    {
        *t_resel = rrc_bits_2_value(ie_ptr, 3);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Next Hop Chaining Count

    Description: Updates the Kenb key and corresponds to parameter
                 NCC.

    Document Reference: 36.331 v10.0.0 Section 6.3.3
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_next_hop_chaining_count_ie(uint8   next_hop_chaining_count,
                                                             uint8 **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(next_hop_chaining_count, ie_ptr, 3);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_next_hop_chaining_count_ie(uint8 **ie_ptr,
                                                               uint8  *next_hop_chaining_count)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr                  != NULL &&
       next_hop_chaining_count != NULL)
    {
        *next_hop_chaining_count = rrc_bits_2_value(ie_ptr, 3);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Security Algorithm Config

    Description: Configures AS integrity protection algorithm (SRBs)
                 and AS ciphering algorithm (SRBs and DRBs).

    Document Reference: 36.331 v10.0.0 Section 6.3.3
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_security_algorithm_config_ie(LIBLTE_RRC_SECURITY_ALGORITHM_CONFIG_STRUCT  *sec_alg_cnfg,
                                                               uint8                                       **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(sec_alg_cnfg != NULL &&
       ie_ptr       != NULL)
    {
        // Extension indicator
        rrc_value_2_bits(0, ie_ptr, 1);

        rrc_value_2_bits(sec_alg_cnfg->cipher_alg, ie_ptr, 3);

        // Extension indicator
        rrc_value_2_bits(0, ie_ptr, 1);

        rrc_value_2_bits(sec_alg_cnfg->int_alg, ie_ptr, 3);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_security_algorithm_config_ie(uint8                                       **ie_ptr,
                                                                 LIBLTE_RRC_SECURITY_ALGORITHM_CONFIG_STRUCT  *sec_alg_cnfg)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr       != NULL &&
       sec_alg_cnfg != NULL)
    {
        // Extension indicator
        rrc_bits_2_value(ie_ptr, 1);

        sec_alg_cnfg->cipher_alg = (LIBLTE_RRC_CIPHERING_ALGORITHM_ENUM)rrc_bits_2_value(ie_ptr, 3);

        // Extension indicator
        rrc_bits_2_value(ie_ptr, 1);

        sec_alg_cnfg->int_alg = (LIBLTE_RRC_INTEGRITY_PROT_ALGORITHM_ENUM)rrc_bits_2_value(ie_ptr, 3);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Short MAC I

    Description: Identifies and verifies the UE at RRC connection
                 re-establishment.

    Document Reference: 36.331 v10.0.0 Section 6.3.3
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_short_mac_i_ie(uint16   short_mac_i,
                                                 uint8  **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(short_mac_i, ie_ptr, 16);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_short_mac_i_ie(uint8  **ie_ptr,
                                                   uint16  *short_mac_i)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr      != NULL &&
       short_mac_i != NULL)
    {
        *short_mac_i = rrc_bits_2_value(ie_ptr, 16);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: PDSCH Config Common

    Description: Specifies the common PDSCH configuration.

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_pdsch_config_common_ie(LIBLTE_RRC_PDSCH_CONFIG_COMMON_STRUCT  *pdsch_config,
                                                         uint8                                 **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(pdsch_config != NULL &&
       ie_ptr       != NULL)
    {
        rrc_value_2_bits(pdsch_config->rs_power + 60, ie_ptr, 7);
        rrc_value_2_bits(pdsch_config->p_b,           ie_ptr, 2);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_pdsch_config_common_ie(uint8                                 **ie_ptr,
                                                           LIBLTE_RRC_PDSCH_CONFIG_COMMON_STRUCT  *pdsch_config)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr       != NULL &&
       pdsch_config != NULL)
    {
        pdsch_config->rs_power = rrc_bits_2_value(ie_ptr, 7) - 60;
        pdsch_config->p_b      = rrc_bits_2_value(ie_ptr, 2);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: PHICH Config

    Description: Specifies the PHICH configuration.

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_phich_config_ie(LIBLTE_RRC_PHICH_CONFIG_STRUCT  *phich_config,
                                                  uint8                          **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(phich_config != NULL &&
       ie_ptr       != NULL)
    {
        rrc_value_2_bits(phich_config->dur, ie_ptr, 1);
        rrc_value_2_bits(phich_config->res, ie_ptr, 2);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_phich_config_ie(uint8                          **ie_ptr,
                                                    LIBLTE_RRC_PHICH_CONFIG_STRUCT  *phich_config)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr       != NULL &&
       phich_config != NULL)
    {
        phich_config->dur = (LIBLTE_RRC_PHICH_DURATION_ENUM)rrc_bits_2_value(ie_ptr, 1);
        phich_config->res = (LIBLTE_RRC_PHICH_RESOURCE_ENUM)rrc_bits_2_value(ie_ptr, 2);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: P Max

    Description: Limits the UE's uplink transmission power on a
                 carrier frequency and is used to calculate the
                 parameter P Compensation.

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_p_max_ie(int8    p_max,
                                           uint8 **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(p_max + 30, ie_ptr, 6);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_p_max_ie(uint8 **ie_ptr,
                                             int8   *p_max)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL &&
       p_max  != NULL)
    {
        *p_max = (int8)rrc_bits_2_value(ie_ptr, 6) - 30;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: PRACH Config SIB

    Description: Specifies the PRACH configuration for system
                 information.

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_prach_config_sib_ie(LIBLTE_RRC_PRACH_CONFIG_SIB_STRUCT  *prach_cnfg,
                                                      uint8                              **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(prach_cnfg != NULL &&
       ie_ptr     != NULL)
    {
        rrc_value_2_bits(prach_cnfg->root_sequence_index,                          ie_ptr, 10);
        rrc_value_2_bits(prach_cnfg->prach_cnfg_info.prach_config_index,           ie_ptr,  6);
        rrc_value_2_bits(prach_cnfg->prach_cnfg_info.high_speed_flag,              ie_ptr,  1);
        rrc_value_2_bits(prach_cnfg->prach_cnfg_info.zero_correlation_zone_config, ie_ptr,  4);
        rrc_value_2_bits(prach_cnfg->prach_cnfg_info.prach_freq_offset,            ie_ptr,  7);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_prach_config_sib_ie(uint8                              **ie_ptr,
                                                        LIBLTE_RRC_PRACH_CONFIG_SIB_STRUCT  *prach_cnfg)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr     != NULL &&
       prach_cnfg != NULL)
    {
        prach_cnfg->root_sequence_index                          = rrc_bits_2_value(ie_ptr, 10);
        prach_cnfg->prach_cnfg_info.prach_config_index           = rrc_bits_2_value(ie_ptr,  6);
        prach_cnfg->prach_cnfg_info.high_speed_flag              = rrc_bits_2_value(ie_ptr,  1);
        prach_cnfg->prach_cnfg_info.zero_correlation_zone_config = rrc_bits_2_value(ie_ptr,  4);
        prach_cnfg->prach_cnfg_info.prach_freq_offset            = rrc_bits_2_value(ie_ptr,  7);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Presence Antenna Port 1

    Description: Indicates whether all the neighboring cells use
                 antenna port 1.

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_presence_antenna_port_1_ie(bool    presence_ant_port_1,
                                                             uint8 **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(presence_ant_port_1, ie_ptr, 1);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_presence_antenna_port_1_ie(uint8 **ie_ptr,
                                                               bool   *presence_ant_port_1)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr              != NULL &&
       presence_ant_port_1 != NULL)
    {
        *presence_ant_port_1 = rrc_bits_2_value(ie_ptr, 1);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: PUCCH Config Common

    Description: Specifies the common PUCCH configuration.

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_pucch_config_common_ie(LIBLTE_RRC_PUCCH_CONFIG_COMMON_STRUCT  *pucch_cnfg,
                                                         uint8                                 **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(pucch_cnfg != NULL &&
       ie_ptr     != NULL)
    {
        rrc_value_2_bits(pucch_cnfg->delta_pucch_shift, ie_ptr,  2);
        rrc_value_2_bits(pucch_cnfg->n_rb_cqi,          ie_ptr,  7);
        rrc_value_2_bits(pucch_cnfg->n_cs_an,           ie_ptr,  3);
        rrc_value_2_bits(pucch_cnfg->n1_pucch_an,       ie_ptr, 11);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_pucch_config_common_ie(uint8                                 **ie_ptr,
                                                           LIBLTE_RRC_PUCCH_CONFIG_COMMON_STRUCT  *pucch_cnfg)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr     != NULL &&
       pucch_cnfg != NULL)
    {
        pucch_cnfg->delta_pucch_shift = (LIBLTE_RRC_DELTA_PUCCH_SHIFT_ENUM)rrc_bits_2_value(ie_ptr, 2);
        pucch_cnfg->n_rb_cqi          = rrc_bits_2_value(ie_ptr,  7);
        pucch_cnfg->n_cs_an           = rrc_bits_2_value(ie_ptr,  3);
        pucch_cnfg->n1_pucch_an       = rrc_bits_2_value(ie_ptr, 11);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: PUSCH Config Common

    Description: Specifies the common PUSCH configuration and the
                 reference signal configuration for PUSCH and PUCCH.

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_pusch_config_common_ie(LIBLTE_RRC_PUSCH_CONFIG_COMMON_STRUCT  *pusch_cnfg,
                                                         uint8                                 **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(pusch_cnfg != NULL &&
       ie_ptr     != NULL)
    {
        // PUSCH Config Basic
        rrc_value_2_bits(pusch_cnfg->n_sb - 1,             ie_ptr, 2);
        rrc_value_2_bits(pusch_cnfg->hopping_mode,         ie_ptr, 1);
        rrc_value_2_bits(pusch_cnfg->pusch_hopping_offset, ie_ptr, 7);
        rrc_value_2_bits(pusch_cnfg->enable_64_qam,        ie_ptr, 1);

        // UL Reference Signals PUSCH
        rrc_value_2_bits(pusch_cnfg->ul_rs.group_hopping_enabled,    ie_ptr, 1);
        rrc_value_2_bits(pusch_cnfg->ul_rs.group_assignment_pusch,   ie_ptr, 5);
        rrc_value_2_bits(pusch_cnfg->ul_rs.sequence_hopping_enabled, ie_ptr, 1);
        rrc_value_2_bits(pusch_cnfg->ul_rs.cyclic_shift,             ie_ptr, 3);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_pusch_config_common_ie(uint8                                 **ie_ptr,
                                                           LIBLTE_RRC_PUSCH_CONFIG_COMMON_STRUCT  *pusch_cnfg)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr     != NULL &&
       pusch_cnfg != NULL)
    {
        // PUSCH Config Basic
        pusch_cnfg->n_sb                 = rrc_bits_2_value(ie_ptr, 2) + 1;
        pusch_cnfg->hopping_mode         = (LIBLTE_RRC_HOPPING_MODE_ENUM)rrc_bits_2_value(ie_ptr, 1);
        pusch_cnfg->pusch_hopping_offset = rrc_bits_2_value(ie_ptr, 7);
        pusch_cnfg->enable_64_qam        = rrc_bits_2_value(ie_ptr, 1);

        // UL Reference Signals PUSCH
        pusch_cnfg->ul_rs.group_hopping_enabled    = rrc_bits_2_value(ie_ptr, 1);
        pusch_cnfg->ul_rs.group_assignment_pusch   = rrc_bits_2_value(ie_ptr, 5);
        pusch_cnfg->ul_rs.sequence_hopping_enabled = rrc_bits_2_value(ie_ptr, 1);
        pusch_cnfg->ul_rs.cyclic_shift             = rrc_bits_2_value(ie_ptr, 3);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: RACH Config Common

    Description: Specifies the generic random access parameters.

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_rach_config_common_ie(LIBLTE_RRC_RACH_CONFIG_COMMON_STRUCT  *rach_cnfg,
                                                        uint8                                **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(rach_cnfg != NULL &&
       ie_ptr    != NULL)
    {
        // Extension indicator
        rrc_value_2_bits(0, ie_ptr, 1);

        // Preamble Info
        rrc_value_2_bits(rach_cnfg->preambles_group_a_cnfg.present, ie_ptr, 1);
        rrc_value_2_bits(rach_cnfg->num_ra_preambles,               ie_ptr, 4);
        if(true == rach_cnfg->preambles_group_a_cnfg.present)
        {
            // Extension indicator
            rrc_value_2_bits(0, ie_ptr, 1);

            rrc_value_2_bits(rach_cnfg->preambles_group_a_cnfg.size_of_ra,             ie_ptr, 4);
            rrc_value_2_bits(rach_cnfg->preambles_group_a_cnfg.msg_size,               ie_ptr, 2);
            rrc_value_2_bits(rach_cnfg->preambles_group_a_cnfg.msg_pwr_offset_group_b, ie_ptr, 3);
        }

        // Power Ramping Parameters
        rrc_value_2_bits(rach_cnfg->pwr_ramping_step,            ie_ptr, 2);
        rrc_value_2_bits(rach_cnfg->preamble_init_rx_target_pwr, ie_ptr, 4);

        // RA Supervision Info
        rrc_value_2_bits(rach_cnfg->preamble_trans_max, ie_ptr, 4);
        rrc_value_2_bits(rach_cnfg->ra_resp_win_size,   ie_ptr, 3);
        rrc_value_2_bits(rach_cnfg->mac_con_res_timer,  ie_ptr, 3);

        rrc_value_2_bits(rach_cnfg->max_harq_msg3_tx - 1, ie_ptr, 3);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_rach_config_common_ie(uint8                                **ie_ptr,
                                                          LIBLTE_RRC_RACH_CONFIG_COMMON_STRUCT  *rach_cnfg)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr    != NULL &&
       rach_cnfg != NULL)
    {
        // Extension indicator
        rrc_bits_2_value(ie_ptr, 1);

        // Preamble Info
        rach_cnfg->preambles_group_a_cnfg.present = rrc_bits_2_value(ie_ptr, 1);
        rach_cnfg->num_ra_preambles               = (LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_ENUM)rrc_bits_2_value(ie_ptr, 4);
        if(true == rach_cnfg->preambles_group_a_cnfg.present)
        {
            // Extension indicator
            rrc_bits_2_value(ie_ptr, 1);

            rach_cnfg->preambles_group_a_cnfg.size_of_ra             = (LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_ENUM)rrc_bits_2_value(ie_ptr, 4);
            rach_cnfg->preambles_group_a_cnfg.msg_size               = (LIBLTE_RRC_MESSAGE_SIZE_GROUP_A_ENUM)rrc_bits_2_value(ie_ptr, 2);
            rach_cnfg->preambles_group_a_cnfg.msg_pwr_offset_group_b = (LIBLTE_RRC_MESSAGE_POWER_OFFSET_GROUP_B_ENUM)rrc_bits_2_value(ie_ptr, 3);
        }else{
            rach_cnfg->preambles_group_a_cnfg.size_of_ra = (LIBLTE_RRC_SIZE_OF_RA_PREAMBLES_GROUP_A_ENUM)rach_cnfg->num_ra_preambles;
        }

        // Power Ramping Parameters
        rach_cnfg->pwr_ramping_step            = (LIBLTE_RRC_POWER_RAMPING_STEP_ENUM)rrc_bits_2_value(ie_ptr, 2);
        rach_cnfg->preamble_init_rx_target_pwr = (LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_ENUM)rrc_bits_2_value(ie_ptr, 4);

        // RA Supervision Info
        rach_cnfg->preamble_trans_max = (LIBLTE_RRC_PREAMBLE_TRANS_MAX_ENUM)rrc_bits_2_value(ie_ptr, 4);
        rach_cnfg->ra_resp_win_size   = (LIBLTE_RRC_RA_RESPONSE_WINDOW_SIZE_ENUM)rrc_bits_2_value(ie_ptr, 3);
        rach_cnfg->mac_con_res_timer  = (LIBLTE_RRC_MAC_CONTENTION_RESOLUTION_TIMER_ENUM)rrc_bits_2_value(ie_ptr, 3);

        rach_cnfg->max_harq_msg3_tx = rrc_bits_2_value(ie_ptr, 3) + 1;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: RACH Config Dedicated

    Description: Specifies the dedicated random access parameters.

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_rach_config_dedicated_ie(LIBLTE_RRC_RACH_CONFIG_DEDICATED_STRUCT  *rach_cnfg,
                                                           uint8                                   **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(rach_cnfg != NULL &&
       ie_ptr    != NULL)
    {
        rrc_value_2_bits(rach_cnfg->preamble_index,   ie_ptr, 6);
        rrc_value_2_bits(rach_cnfg->prach_mask_index, ie_ptr, 4);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_rach_config_dedicated_ie(uint8                                   **ie_ptr,
                                                             LIBLTE_RRC_RACH_CONFIG_DEDICATED_STRUCT  *rach_cnfg)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr    != NULL &&
       rach_cnfg != NULL)
    {
        rach_cnfg->preamble_index   = rrc_bits_2_value(ie_ptr, 6);
        rach_cnfg->prach_mask_index = rrc_bits_2_value(ie_ptr, 4);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Radio Resource Config Common SIB

    Description: Specifies the common radio resource configurations
                 for system information, including random access
                 parameters and static physical layer parameters.

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_rr_config_common_sib_ie(LIBLTE_RRC_RR_CONFIG_COMMON_SIB_STRUCT  *rr_cnfg,
                                                          uint8                                  **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(rr_cnfg != NULL &&
       ie_ptr  != NULL)
    {
        // Extension indicator
        rrc_value_2_bits(0, ie_ptr, 1);

        liblte_rrc_pack_rach_config_common_ie(&rr_cnfg->rach_cnfg, ie_ptr);

        // BCCH Config
        rrc_value_2_bits(rr_cnfg->bcch_cnfg.modification_period_coeff, ie_ptr, 2);

        // PCCH Config
        rrc_value_2_bits(rr_cnfg->pcch_cnfg.default_paging_cycle, ie_ptr, 2);
        rrc_value_2_bits(rr_cnfg->pcch_cnfg.nB,                   ie_ptr, 3);

        liblte_rrc_pack_prach_config_sib_ie(&rr_cnfg->prach_cnfg,         ie_ptr);
        liblte_rrc_pack_pdsch_config_common_ie(&rr_cnfg->pdsch_cnfg,      ie_ptr);
        liblte_rrc_pack_pusch_config_common_ie(&rr_cnfg->pusch_cnfg,      ie_ptr);
        liblte_rrc_pack_pucch_config_common_ie(&rr_cnfg->pucch_cnfg,      ie_ptr);
        liblte_rrc_pack_srs_ul_config_common_ie(&rr_cnfg->srs_ul_cnfg,    ie_ptr);
        liblte_rrc_pack_ul_power_control_common_ie(&rr_cnfg->ul_pwr_ctrl, ie_ptr);

        // UL CP Length
        rrc_value_2_bits(rr_cnfg->ul_cp_length, ie_ptr, 1);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_rr_config_common_sib_ie(uint8                                  **ie_ptr,
                                                            LIBLTE_RRC_RR_CONFIG_COMMON_SIB_STRUCT  *rr_cnfg)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr  != NULL &&
       rr_cnfg != NULL)
    {
        // Extension indicator
        rrc_bits_2_value(ie_ptr, 1);

        liblte_rrc_unpack_rach_config_common_ie(ie_ptr, &rr_cnfg->rach_cnfg);

        // BCCH Config
        rr_cnfg->bcch_cnfg.modification_period_coeff = (LIBLTE_RRC_MODIFICATION_PERIOD_COEFF_ENUM)rrc_bits_2_value(ie_ptr, 2);

        // PCCH Config
        rr_cnfg->pcch_cnfg.default_paging_cycle = (LIBLTE_RRC_DEFAULT_PAGING_CYCLE_ENUM)rrc_bits_2_value(ie_ptr, 2);
        rr_cnfg->pcch_cnfg.nB                   = (LIBLTE_RRC_NB_ENUM)rrc_bits_2_value(ie_ptr, 3);

        liblte_rrc_unpack_prach_config_sib_ie(ie_ptr,        &rr_cnfg->prach_cnfg);
        liblte_rrc_unpack_pdsch_config_common_ie(ie_ptr,     &rr_cnfg->pdsch_cnfg);
        liblte_rrc_unpack_pusch_config_common_ie(ie_ptr,     &rr_cnfg->pusch_cnfg);
        liblte_rrc_unpack_pucch_config_common_ie(ie_ptr,     &rr_cnfg->pucch_cnfg);
        liblte_rrc_unpack_srs_ul_config_common_ie(ie_ptr,    &rr_cnfg->srs_ul_cnfg);
        liblte_rrc_unpack_ul_power_control_common_ie(ie_ptr, &rr_cnfg->ul_pwr_ctrl);

        // UL CP Length
        rr_cnfg->ul_cp_length = (LIBLTE_RRC_UL_CP_LENGTH_ENUM)rrc_bits_2_value(ie_ptr, 1);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Sounding RS UL Config Common

    Description: Specifies the uplink Sounding RS configuration for
                 periodic and aperiodic sounding.

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_srs_ul_config_common_ie(LIBLTE_RRC_SRS_UL_CONFIG_COMMON_STRUCT  *srs_ul_cnfg,
                                                          uint8                                  **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(srs_ul_cnfg != NULL &&
       ie_ptr      != NULL)
    {
        rrc_value_2_bits(srs_ul_cnfg->present, ie_ptr, 1);

        if(true == srs_ul_cnfg->present)
        {
            rrc_value_2_bits(srs_ul_cnfg->max_up_pts_present, ie_ptr, 1);

            rrc_value_2_bits(srs_ul_cnfg->bw_cnfg,           ie_ptr, 3);
            rrc_value_2_bits(srs_ul_cnfg->subfr_cnfg,        ie_ptr, 4);
            rrc_value_2_bits(srs_ul_cnfg->ack_nack_simul_tx, ie_ptr, 1);

            if(true == srs_ul_cnfg->max_up_pts_present)
            {
                rrc_value_2_bits(srs_ul_cnfg->max_up_pts, ie_ptr, 1);
            }
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_srs_ul_config_common_ie(uint8                                  **ie_ptr,
                                                            LIBLTE_RRC_SRS_UL_CONFIG_COMMON_STRUCT  *srs_ul_cnfg)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr      != NULL &&
       srs_ul_cnfg != NULL)
    {
        srs_ul_cnfg->present = rrc_bits_2_value(ie_ptr, 1);

        if(true == srs_ul_cnfg->present)
        {
            srs_ul_cnfg->max_up_pts_present = rrc_bits_2_value(ie_ptr, 1);

            srs_ul_cnfg->bw_cnfg           = (LIBLTE_RRC_SRS_BW_CONFIG_ENUM)rrc_bits_2_value(ie_ptr, 3);
            srs_ul_cnfg->subfr_cnfg        = (LIBLTE_RRC_SRS_SUBFR_CONFIG_ENUM)rrc_bits_2_value(ie_ptr, 4);
            srs_ul_cnfg->ack_nack_simul_tx = rrc_bits_2_value(ie_ptr, 1);

            if(true == srs_ul_cnfg->max_up_pts_present)
            {
                srs_ul_cnfg->max_up_pts = rrc_bits_2_value(ie_ptr, 1);
            }
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: TDD Config

    Description: Specifies the TDD specific physical channel
                 configuration.

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_tdd_config_ie(LIBLTE_RRC_SUBFRAME_ASSIGNMENT_ENUM         sa,
                                                LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_ENUM   ssp,
                                                uint8                                     **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(sa,  ie_ptr, 3);
        rrc_value_2_bits(ssp, ie_ptr, 4);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_tdd_config_ie(uint8                                     **ie_ptr,
                                                  LIBLTE_RRC_SUBFRAME_ASSIGNMENT_ENUM        *sa,
                                                  LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_ENUM  *ssp)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL &&
       sa     != NULL &&
       ssp    != NULL)
    {
        *sa  = (LIBLTE_RRC_SUBFRAME_ASSIGNMENT_ENUM)rrc_bits_2_value(ie_ptr, 3);
        *ssp = (LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_ENUM)rrc_bits_2_value(ie_ptr, 4);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Time Alignment Timer

    Description: Controls how long the UE is considered uplink time
                 aligned.

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_time_alignment_timer_ie(LIBLTE_RRC_TIME_ALIGNMENT_TIMER_ENUM   time_alignment_timer,
                                                          uint8                                **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        rrc_value_2_bits(time_alignment_timer, ie_ptr, 3);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_time_alignment_timer_ie(uint8                                **ie_ptr,
                                                            LIBLTE_RRC_TIME_ALIGNMENT_TIMER_ENUM  *time_alignment_timer)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr               != NULL &&
       time_alignment_timer != NULL)
    {
        *time_alignment_timer = (LIBLTE_RRC_TIME_ALIGNMENT_TIMER_ENUM)rrc_bits_2_value(ie_ptr, 3);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Uplink Power Control Common

    Description: Specifies the parameters for uplink power control for
                 system information.

    Document Reference: 36.331 v10.0.0 Section 6.3.2
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_ul_power_control_common_ie(LIBLTE_RRC_UL_POWER_CONTROL_COMMON_STRUCT  *ul_pwr_ctrl,
                                                             uint8                                     **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ul_pwr_ctrl != NULL &&
       ie_ptr      != NULL)
    {
        rrc_value_2_bits(ul_pwr_ctrl->p0_nominal_pusch + 126, ie_ptr, 8);
        rrc_value_2_bits(ul_pwr_ctrl->alpha,                  ie_ptr, 3);
        rrc_value_2_bits(ul_pwr_ctrl->p0_nominal_pucch + 127, ie_ptr, 5);

        // Delta F List
        rrc_value_2_bits(ul_pwr_ctrl->delta_flist_pucch.format_1,  ie_ptr, 2);
        rrc_value_2_bits(ul_pwr_ctrl->delta_flist_pucch.format_1b, ie_ptr, 2);
        rrc_value_2_bits(ul_pwr_ctrl->delta_flist_pucch.format_2,  ie_ptr, 2);
        rrc_value_2_bits(ul_pwr_ctrl->delta_flist_pucch.format_2a, ie_ptr, 2);
        rrc_value_2_bits(ul_pwr_ctrl->delta_flist_pucch.format_2b, ie_ptr, 2);

        rrc_value_2_bits((ul_pwr_ctrl->delta_preamble_msg3 / 2) + 1, ie_ptr, 3);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_ul_power_control_common_ie(uint8                                     **ie_ptr,
                                                               LIBLTE_RRC_UL_POWER_CONTROL_COMMON_STRUCT  *ul_pwr_ctrl)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr      != NULL &&
       ul_pwr_ctrl != NULL)
    {
        ul_pwr_ctrl->p0_nominal_pusch = rrc_bits_2_value(ie_ptr, 8) - 126;
        ul_pwr_ctrl->alpha            = (LIBLTE_RRC_UL_POWER_CONTROL_ALPHA_ENUM)rrc_bits_2_value(ie_ptr, 3);
        ul_pwr_ctrl->p0_nominal_pucch = rrc_bits_2_value(ie_ptr, 5) - 127;

        // Delta F List
        ul_pwr_ctrl->delta_flist_pucch.format_1  = (LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_1_ENUM)rrc_bits_2_value(ie_ptr, 2);
        ul_pwr_ctrl->delta_flist_pucch.format_1b = (LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_1B_ENUM)rrc_bits_2_value(ie_ptr, 2);
        ul_pwr_ctrl->delta_flist_pucch.format_2  = (LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2_ENUM)rrc_bits_2_value(ie_ptr, 2);
        ul_pwr_ctrl->delta_flist_pucch.format_2a = (LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2A_ENUM)rrc_bits_2_value(ie_ptr, 2);
        ul_pwr_ctrl->delta_flist_pucch.format_2b = (LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2B_ENUM)rrc_bits_2_value(ie_ptr, 2);

        ul_pwr_ctrl->delta_preamble_msg3 = (rrc_bits_2_value(ie_ptr, 3) - 1) * 2;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: System Information Block Type 2

    Description: Contains radio resource configuration that is common
                 for all UEs.

    Document Reference: 36.331 v10.0.0 Section 6.3.1
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_sys_info_block_type_2_ie(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_2_STRUCT  *sib2,
                                                           uint8                                   **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;
    uint32            i;
    bool              mbsfn_subfr_cnfg_list_opt;

    if(sib2   != NULL &&
       ie_ptr != NULL)
    {
        // Extension indicator
        rrc_value_2_bits(0, ie_ptr, 1);

        // Optional indicators
        rrc_value_2_bits(sib2->ac_barring_info_present, ie_ptr, 1);
        if(0 != sib2->mbsfn_subfr_cnfg_list_size)
        {
            rrc_value_2_bits(1, ie_ptr, 1);
            mbsfn_subfr_cnfg_list_opt = true;
        }else{
            rrc_value_2_bits(0, ie_ptr, 1);
            mbsfn_subfr_cnfg_list_opt = false;
        }

        // AC Barring
        if(true == sib2->ac_barring_info_present)
        {
            rrc_value_2_bits(sib2->ac_barring_for_mo_signalling.enabled, ie_ptr, 1);
            rrc_value_2_bits(sib2->ac_barring_for_mo_data.enabled,       ie_ptr, 1);

            // AC Barring for emergency
            rrc_value_2_bits(sib2->ac_barring_for_emergency, ie_ptr, 1);

            // AC Barring for MO signalling
            if(true == sib2->ac_barring_for_mo_signalling.enabled)
            {
                rrc_value_2_bits(sib2->ac_barring_for_mo_signalling.factor,         ie_ptr, 4);
                rrc_value_2_bits(sib2->ac_barring_for_mo_signalling.time,           ie_ptr, 3);
                rrc_value_2_bits(sib2->ac_barring_for_mo_signalling.for_special_ac, ie_ptr, 5);
            }

            // AC Barring for MO data
            if(true == sib2->ac_barring_for_mo_data.enabled)
            {
                rrc_value_2_bits(sib2->ac_barring_for_mo_data.factor,         ie_ptr, 4);
                rrc_value_2_bits(sib2->ac_barring_for_mo_data.time,           ie_ptr, 3);
                rrc_value_2_bits(sib2->ac_barring_for_mo_data.for_special_ac, ie_ptr, 5);
            }
        }

        // Radio Resource Config Common
        liblte_rrc_pack_rr_config_common_sib_ie(&sib2->rr_config_common_sib, ie_ptr);

        // UE Timers and Constants
        liblte_rrc_pack_ue_timers_and_constants_ie(&sib2->ue_timers_and_constants, ie_ptr);

        // Frequency information
        {
            // Optional indicators
            rrc_value_2_bits(sib2->arfcn_value_eutra.present, ie_ptr, 1);
            rrc_value_2_bits(sib2->ul_bw.present,             ie_ptr, 1);

            // UL Carrier Frequency
            if(true == sib2->arfcn_value_eutra.present)
            {
                liblte_rrc_pack_arfcn_value_eutra_ie(sib2->arfcn_value_eutra.value, ie_ptr);
            }

            // UL Bandwidth
            if(true == sib2->ul_bw.present)
            {
                rrc_value_2_bits(sib2->ul_bw.bw, ie_ptr, 3);
            }

            // Additional Spectrum Emission
            liblte_rrc_pack_additional_spectrum_emission_ie(sib2->additional_spectrum_emission,
                                                            ie_ptr);
        }

        // MBSFN Subframe Config List
        if(true == mbsfn_subfr_cnfg_list_opt)
        {
            rrc_value_2_bits(sib2->mbsfn_subfr_cnfg_list_size - 1, ie_ptr, 3);
            for(i=0; i<sib2->mbsfn_subfr_cnfg_list_size; i++)
            {
                liblte_rrc_pack_mbsfn_subframe_config_ie(&sib2->mbsfn_subfr_cnfg[i], ie_ptr);
            }
        }

        // Time Alignment Timer Common
        liblte_rrc_pack_time_alignment_timer_ie(sib2->time_alignment_timer, ie_ptr);

        // FIXME: Not handling extensions

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_sys_info_block_type_2_ie(uint8                                   **ie_ptr,
                                                             LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_2_STRUCT  *sib2)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;
    uint32            i;
    uint8             ext_ind;
    bool              mbsfn_subfr_cnfg_list_opt;

    if(ie_ptr != NULL &&
       sib2   != NULL)
    {
        // Extension indicator
        ext_ind = rrc_bits_2_value(ie_ptr, 1);

        // Optional indicators
        sib2->ac_barring_info_present = rrc_bits_2_value(ie_ptr, 1);
        mbsfn_subfr_cnfg_list_opt     = rrc_bits_2_value(ie_ptr, 1);

        // AC Barring
        if(true == sib2->ac_barring_info_present)
        {
            // Optional indicators
            sib2->ac_barring_for_mo_signalling.enabled = rrc_bits_2_value(ie_ptr, 1);
            sib2->ac_barring_for_mo_data.enabled       = rrc_bits_2_value(ie_ptr, 1);

            // AC Barring for emergency
            sib2->ac_barring_for_emergency = rrc_bits_2_value(ie_ptr, 1);

            // AC Barring for MO signalling
            if(true == sib2->ac_barring_for_mo_signalling.enabled)
            {
                sib2->ac_barring_for_mo_signalling.factor         = (LIBLTE_RRC_AC_BARRING_FACTOR_ENUM)rrc_bits_2_value(ie_ptr, 4);
                sib2->ac_barring_for_mo_signalling.time           = (LIBLTE_RRC_AC_BARRING_TIME_ENUM)rrc_bits_2_value(ie_ptr, 3);
                sib2->ac_barring_for_mo_signalling.for_special_ac = rrc_bits_2_value(ie_ptr, 5);
            }

            // AC Barring for MO data
            if(true == sib2->ac_barring_for_mo_data.enabled)
            {
                sib2->ac_barring_for_mo_data.factor         = (LIBLTE_RRC_AC_BARRING_FACTOR_ENUM)rrc_bits_2_value(ie_ptr, 4);
                sib2->ac_barring_for_mo_data.time           = (LIBLTE_RRC_AC_BARRING_TIME_ENUM)rrc_bits_2_value(ie_ptr, 3);
                sib2->ac_barring_for_mo_data.for_special_ac = rrc_bits_2_value(ie_ptr, 5);
            }
        }else{
            sib2->ac_barring_for_emergency             = false;
            sib2->ac_barring_for_mo_signalling.enabled = false;
            sib2->ac_barring_for_mo_data.enabled       = false;
        }

        // Radio Resource Config Common
        liblte_rrc_unpack_rr_config_common_sib_ie(ie_ptr, &sib2->rr_config_common_sib);

        // UE Timers and Constants
        liblte_rrc_unpack_ue_timers_and_constants_ie(ie_ptr, &sib2->ue_timers_and_constants);

        // Frequency information
        {
            // Optional indicators
            sib2->arfcn_value_eutra.present = rrc_bits_2_value(ie_ptr, 1);
            sib2->ul_bw.present             = rrc_bits_2_value(ie_ptr, 1);

            // UL Carrier Frequency
            if(true == sib2->arfcn_value_eutra.present)
            {
                liblte_rrc_unpack_arfcn_value_eutra_ie(ie_ptr, &sib2->arfcn_value_eutra.value);
            }

            // UL Bandwidth
            if(true == sib2->ul_bw.present)
            {
                sib2->ul_bw.bw = (LIBLTE_RRC_UL_BW_ENUM)rrc_bits_2_value(ie_ptr, 3);
            }

            // Additional Spectrum Emission
            liblte_rrc_unpack_additional_spectrum_emission_ie(ie_ptr,
                                                              &sib2->additional_spectrum_emission);
        }

        // MBSFN Subframe Config List
        if(true == mbsfn_subfr_cnfg_list_opt)
        {
            sib2->mbsfn_subfr_cnfg_list_size = rrc_bits_2_value(ie_ptr, 3) + 1;
            for(i=0; i<sib2->mbsfn_subfr_cnfg_list_size; i++)
            {
                liblte_rrc_unpack_mbsfn_subframe_config_ie(ie_ptr, &sib2->mbsfn_subfr_cnfg[i]);
            }
        }else{
            sib2->mbsfn_subfr_cnfg_list_size = 0;
        }

        // Time Alignment Timer Common
        liblte_rrc_unpack_time_alignment_timer_ie(ie_ptr, &sib2->time_alignment_timer);

        // Extensions
        if(true == ext_ind)
        {
            // FIXME
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: System Information Block Type 3

    Description: Contains cell reselection information common for
                 intra-frequency, inter-frequency, and/or inter-RAT
                 cell re-selection as well as intra-frequency cell
                 re-selection information other than neighboring
                 cell related.

    Document Reference: 36.331 v10.0.0 Section 6.3.1
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_sys_info_block_type_3_ie(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_3_STRUCT  *sib3,
                                                           uint8                                   **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(sib3   != NULL &&
       ie_ptr != NULL)
    {
        // Extension indicator
        rrc_value_2_bits(0, ie_ptr, 1);

        // Cell reselection info common
        {
            // Optional indicator
            rrc_value_2_bits(sib3->speed_state_resel_params.present, ie_ptr, 1);

            rrc_value_2_bits(sib3->q_hyst, ie_ptr, 4);

            // Speed state reselection parameters
            if(true == sib3->speed_state_resel_params.present)
            {
                liblte_rrc_pack_mobility_state_parameters_ie(&sib3->speed_state_resel_params.mobility_state_params, ie_ptr);

                rrc_value_2_bits(sib3->speed_state_resel_params.q_hyst_sf.medium, ie_ptr, 2);
                rrc_value_2_bits(sib3->speed_state_resel_params.q_hyst_sf.high,   ie_ptr, 2);
            }
        }

        // Cell reselection serving frequency information
        {
            // Optional indicators
            rrc_value_2_bits(sib3->s_non_intra_search_present, ie_ptr, 1);

            if(true == sib3->s_non_intra_search_present)
            {
                liblte_rrc_pack_reselection_threshold_ie(sib3->s_non_intra_search, ie_ptr);
            }

            liblte_rrc_pack_reselection_threshold_ie(sib3->thresh_serving_low, ie_ptr);

            liblte_rrc_pack_cell_reselection_priority_ie(sib3->cell_resel_prio, ie_ptr);
        }

        // Intra frequency cell reselection information
        {
            // Optional indicators
            rrc_value_2_bits(sib3->p_max_present,            ie_ptr, 1);
            rrc_value_2_bits(sib3->s_intra_search_present,   ie_ptr, 1);
            rrc_value_2_bits(sib3->allowed_meas_bw_present,  ie_ptr, 1);
            rrc_value_2_bits(sib3->t_resel_eutra_sf_present, ie_ptr, 1);

            liblte_rrc_pack_q_rx_lev_min_ie(sib3->q_rx_lev_min, ie_ptr);

            if(true == sib3->p_max_present)
            {
                liblte_rrc_pack_p_max_ie(sib3->p_max, ie_ptr);
            }

            if(true == sib3->s_intra_search_present)
            {
                liblte_rrc_pack_reselection_threshold_ie(sib3->s_intra_search, ie_ptr);
            }

            if(true == sib3->allowed_meas_bw_present)
            {
                liblte_rrc_pack_allowed_meas_bandwidth_ie(sib3->allowed_meas_bw, ie_ptr);
            }

            liblte_rrc_pack_presence_antenna_port_1_ie(sib3->presence_ant_port_1, ie_ptr);

            liblte_rrc_pack_neigh_cell_config_ie(sib3->neigh_cell_cnfg, ie_ptr);

            liblte_rrc_pack_t_reselection_ie(sib3->t_resel_eutra, ie_ptr);

            if(true == sib3->t_resel_eutra_sf_present)
            {
                liblte_rrc_pack_speed_state_scale_factors_ie(&sib3->t_resel_eutra_sf, ie_ptr);
            }
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_sys_info_block_type_3_ie(uint8                                   **ie_ptr,
                                                             LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_3_STRUCT  *sib3)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;
    bool              ext_ind;

    if(ie_ptr != NULL &&
       sib3   != NULL)
    {
        // Extension indicator
        ext_ind = rrc_bits_2_value(ie_ptr, 1);

        // Cell reselection info common
        {
            // Optional indicator
            sib3->speed_state_resel_params.present = rrc_bits_2_value(ie_ptr, 1);

            sib3->q_hyst = (LIBLTE_RRC_Q_HYST_ENUM)rrc_bits_2_value(ie_ptr, 4);

            // Speed state reselection parameters
            if(true == sib3->speed_state_resel_params.present)
            {
                liblte_rrc_unpack_mobility_state_parameters_ie(ie_ptr, &sib3->speed_state_resel_params.mobility_state_params);

                sib3->speed_state_resel_params.q_hyst_sf.medium = (LIBLTE_RRC_SF_MEDIUM_ENUM)rrc_bits_2_value(ie_ptr, 2);
                sib3->speed_state_resel_params.q_hyst_sf.high   = (LIBLTE_RRC_SF_HIGH_ENUM)rrc_bits_2_value(ie_ptr, 2);
            }
        }

        // Cell reselection serving frequency information
        {
            // Optional indicators
            sib3->s_non_intra_search_present = rrc_bits_2_value(ie_ptr, 1);

            if(true == sib3->s_non_intra_search_present)
            {
                liblte_rrc_unpack_reselection_threshold_ie(ie_ptr, &sib3->s_non_intra_search);
            }

            liblte_rrc_unpack_reselection_threshold_ie(ie_ptr, &sib3->thresh_serving_low);

            liblte_rrc_unpack_cell_reselection_priority_ie(ie_ptr, &sib3->cell_resel_prio);
        }

        // Intra frequency cell reselection information
        {
            // Optional indicators
            sib3->p_max_present            = rrc_bits_2_value(ie_ptr, 1);
            sib3->s_intra_search_present   = rrc_bits_2_value(ie_ptr, 1);
            sib3->allowed_meas_bw_present  = rrc_bits_2_value(ie_ptr, 1);
            sib3->t_resel_eutra_sf_present = rrc_bits_2_value(ie_ptr, 1);

            liblte_rrc_unpack_q_rx_lev_min_ie(ie_ptr, &sib3->q_rx_lev_min);

            if(true == sib3->p_max_present)
            {
                liblte_rrc_unpack_p_max_ie(ie_ptr, &sib3->p_max);
            }

            if(true == sib3->s_intra_search_present)
            {
                liblte_rrc_unpack_reselection_threshold_ie(ie_ptr, &sib3->s_intra_search);
            }

            if(true == sib3->allowed_meas_bw_present)
            {
                liblte_rrc_unpack_allowed_meas_bandwidth_ie(ie_ptr, &sib3->allowed_meas_bw);
            }

            liblte_rrc_unpack_presence_antenna_port_1_ie(ie_ptr, &sib3->presence_ant_port_1);

            liblte_rrc_unpack_neigh_cell_config_ie(ie_ptr, &sib3->neigh_cell_cnfg);

            liblte_rrc_unpack_t_reselection_ie(ie_ptr, &sib3->t_resel_eutra);

            if(true == sib3->t_resel_eutra_sf_present)
            {
                liblte_rrc_unpack_speed_state_scale_factors_ie(ie_ptr, &sib3->t_resel_eutra_sf);
            }
        }

        // Extensions
        if(true == ext_ind)
        {
            // FIXME
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: System Information Block Type 4

    Description: Contains the neighboring cell related information
                 relevant only for intra-frequency cell reselection.

    Document Reference: 36.331 v10.0.0 Section 6.3.1
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_sys_info_block_type_4_ie(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_4_STRUCT  *sib4,
                                                           uint8                                   **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;
    uint32            i;

    if(sib4   != NULL &&
       ie_ptr != NULL)
    {
        // Extension indicator
        rrc_value_2_bits(0, ie_ptr, 1);

        // Optional indicators
        if(0 != sib4->intra_freq_neigh_cell_list_size)
        {
            rrc_value_2_bits(1, ie_ptr, 1);
        }else{
            rrc_value_2_bits(0, ie_ptr, 1);
        }
        if(0 != sib4->intra_freq_black_cell_list_size)
        {
            rrc_value_2_bits(1, ie_ptr, 1);
        }else{
            rrc_value_2_bits(0, ie_ptr, 1);
        }
        rrc_value_2_bits(sib4->csg_phys_cell_id_range_present, ie_ptr, 1);

        if(0 != sib4->intra_freq_neigh_cell_list_size)
        {
            rrc_value_2_bits(sib4->intra_freq_neigh_cell_list_size - 1, ie_ptr, 4);
            for(i=0; i<sib4->intra_freq_neigh_cell_list_size; i++)
            {
                // Extension indicator
                rrc_value_2_bits(0, ie_ptr, 1);

                liblte_rrc_pack_phys_cell_id_ie(sib4->intra_freq_neigh_cell_list[i].phys_cell_id, ie_ptr);
                liblte_rrc_pack_q_offset_range_ie(sib4->intra_freq_neigh_cell_list[i].q_offset_range, ie_ptr);
            }
        }

        if(0 != sib4->intra_freq_black_cell_list_size)
        {
            rrc_value_2_bits(sib4->intra_freq_black_cell_list_size - 1, ie_ptr, 4);
            for(i=0; i<sib4->intra_freq_black_cell_list_size; i++)
            {
                liblte_rrc_pack_phys_cell_id_range_ie(&sib4->intra_freq_black_cell_list[i], ie_ptr);
            }
        }

        if(true == sib4->csg_phys_cell_id_range_present)
        {
            liblte_rrc_pack_phys_cell_id_range_ie(&sib4->csg_phys_cell_id_range, ie_ptr);
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_sys_info_block_type_4_ie(uint8                                   **ie_ptr,
                                                             LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_4_STRUCT  *sib4)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;
    uint32            i;
    bool              ext_ind;
    bool              intra_freq_neigh_cell_list_opt;
    bool              intra_freq_black_cell_list_opt;

    if(ie_ptr != NULL &&
       sib4   != NULL)
    {
        // Extension indicator
        ext_ind = rrc_bits_2_value(ie_ptr, 1);

        // Optional indicators
        intra_freq_neigh_cell_list_opt       = rrc_bits_2_value(ie_ptr, 1);
        intra_freq_black_cell_list_opt       = rrc_bits_2_value(ie_ptr, 1);
        sib4->csg_phys_cell_id_range_present = rrc_bits_2_value(ie_ptr, 1);

        if(true == intra_freq_neigh_cell_list_opt)
        {
            sib4->intra_freq_neigh_cell_list_size = rrc_bits_2_value(ie_ptr, 4) + 1;
            for(i=0; i<sib4->intra_freq_neigh_cell_list_size; i++)
            {
                // Extension indicator
                rrc_bits_2_value(ie_ptr, 1);

                liblte_rrc_unpack_phys_cell_id_ie(ie_ptr, &sib4->intra_freq_neigh_cell_list[i].phys_cell_id);
                liblte_rrc_unpack_q_offset_range_ie(ie_ptr, &sib4->intra_freq_neigh_cell_list[i].q_offset_range);
            }
        }else{
            sib4->intra_freq_neigh_cell_list_size = 0;
        }

        if(true == intra_freq_black_cell_list_opt)
        {
            sib4->intra_freq_black_cell_list_size = rrc_bits_2_value(ie_ptr, 4) + 1;
            for(i=0; i<sib4->intra_freq_black_cell_list_size; i++)
            {
                liblte_rrc_unpack_phys_cell_id_range_ie(ie_ptr, &sib4->intra_freq_black_cell_list[i]);
            }
        }else{
            sib4->intra_freq_black_cell_list_size = 0;
        }

        if(true == sib4->csg_phys_cell_id_range_present)
        {
            liblte_rrc_unpack_phys_cell_id_range_ie(ie_ptr, &sib4->csg_phys_cell_id_range);
        }

        // Extension
        if(true == ext_ind)
        {
            // FIXME
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: System Information Block Type 8

    Description: Contains information relevant only for inter-RAT
                 cell re-selection i.e. information about CDMA2000
                 frequencies and CDMA2000 neighboring cells relevant
                 for cell re-selection.

    Document Reference: 36.331 v10.0.0 Section 6.3.1
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_sys_info_block_type_8_ie(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_8_STRUCT  *sib8,
                                                           uint8                                   **ie_ptr)
{
    LIBLTE_RRC_NEIGH_CELL_CDMA2000_STRUCT *neigh_cell_list;
    LIBLTE_ERROR_ENUM                      err = LIBLTE_ERROR_INVALID_INPUTS;
    uint32                                 i;
    uint32                                 j;
    uint32                                 k;

    if(sib8   != NULL &&
       ie_ptr != NULL)
    {
        // Extension indicator
        rrc_value_2_bits(0, ie_ptr, 1);

        // Optional indicators
        rrc_value_2_bits(sib8->sys_time_info_present,   ie_ptr, 1);
        rrc_value_2_bits(sib8->search_win_size_present, ie_ptr, 1);
        rrc_value_2_bits(sib8->params_hrpd_present,     ie_ptr, 1);
        rrc_value_2_bits(sib8->params_1xrtt_present,    ie_ptr, 1);

        if(true == sib8->sys_time_info_present)
        {
            liblte_rrc_pack_system_time_info_cdma2000_ie(&sib8->sys_time_info_cdma2000, ie_ptr);
        }

        if(true == sib8->search_win_size_present)
        {
            rrc_value_2_bits(sib8->search_win_size, ie_ptr, 4);
        }

        if(true == sib8->params_hrpd_present)
        {
            // Optional indicator
            rrc_value_2_bits(sib8->cell_resel_params_hrpd_present, ie_ptr, 1);

            liblte_rrc_pack_pre_registration_info_hrpd_ie(&sib8->pre_reg_info_hrpd, ie_ptr);

            if(true == sib8->cell_resel_params_hrpd_present)
            {
                // Optional indicator
                rrc_value_2_bits(sib8->cell_resel_params_hrpd.t_resel_cdma2000_sf_present, ie_ptr, 1);

                rrc_value_2_bits(sib8->cell_resel_params_hrpd.band_class_list_size - 1, ie_ptr, 5);
                for(i=0; i<sib8->cell_resel_params_hrpd.band_class_list_size; i++)
                {
                    // Extension indicator
                    rrc_value_2_bits(0, ie_ptr, 1);

                    // Optional indicator
                    rrc_value_2_bits(sib8->cell_resel_params_hrpd.band_class_list[i].cell_resel_prio_present, ie_ptr, 1);

                    liblte_rrc_pack_band_class_cdma2000_ie(sib8->cell_resel_params_hrpd.band_class_list[i].band_class, ie_ptr);
                    if(true == sib8->cell_resel_params_hrpd.band_class_list[i].cell_resel_prio_present)
                    {
                        liblte_rrc_pack_cell_reselection_priority_ie(sib8->cell_resel_params_hrpd.band_class_list[i].cell_resel_prio, ie_ptr);
                    }
                    rrc_value_2_bits(sib8->cell_resel_params_hrpd.band_class_list[i].thresh_x_high, ie_ptr, 6);
                    rrc_value_2_bits(sib8->cell_resel_params_hrpd.band_class_list[i].thresh_x_low,  ie_ptr, 6);
                }

                rrc_value_2_bits(sib8->cell_resel_params_hrpd.neigh_cell_list_size - 1, ie_ptr, 4);
                for(i=0; i<sib8->cell_resel_params_hrpd.neigh_cell_list_size; i++)
                {
                    neigh_cell_list = &sib8->cell_resel_params_hrpd.neigh_cell_list[i];
                    liblte_rrc_pack_band_class_cdma2000_ie(neigh_cell_list->band_class, ie_ptr);
                    rrc_value_2_bits(neigh_cell_list->neigh_cells_per_freq_list_size - 1, ie_ptr, 4);
                    for(j=0; j<neigh_cell_list->neigh_cells_per_freq_list_size; j++)
                    {
                        liblte_rrc_pack_arfcn_value_cdma2000_ie(neigh_cell_list->neigh_cells_per_freq_list[j].arfcn, ie_ptr);
                        rrc_value_2_bits(neigh_cell_list->neigh_cells_per_freq_list[j].phys_cell_id_list_size - 1, ie_ptr, 4);
                        for(k=0; k<neigh_cell_list->neigh_cells_per_freq_list[j].phys_cell_id_list_size; k++)
                        {
                            liblte_rrc_pack_phys_cell_id_cdma2000_ie(neigh_cell_list->neigh_cells_per_freq_list[j].phys_cell_id_list[k], ie_ptr);
                        }
                    }
                }
                liblte_rrc_pack_t_reselection_ie(sib8->cell_resel_params_hrpd.t_resel_cdma2000, ie_ptr);

                if(true == sib8->cell_resel_params_hrpd.t_resel_cdma2000_sf_present)
                {
                    liblte_rrc_pack_speed_state_scale_factors_ie(&sib8->cell_resel_params_hrpd.t_resel_cdma2000_sf, ie_ptr);
                }
            }
        }

        if(true == sib8->params_1xrtt_present)
        {
            // Optional indicators
            rrc_value_2_bits(sib8->csfb_reg_param_1xrtt_present,    ie_ptr, 1);
            rrc_value_2_bits(sib8->long_code_state_1xrtt_present,   ie_ptr, 1);
            rrc_value_2_bits(sib8->cell_resel_params_1xrtt_present, ie_ptr, 1);

            if(true == sib8->csfb_reg_param_1xrtt_present)
            {
                liblte_rrc_pack_csfb_registration_param_1xrtt_ie(&sib8->csfb_reg_param_1xrtt, ie_ptr);
            }

            if(true == sib8->long_code_state_1xrtt_present)
            {
                rrc_value_2_bits((uint32)(sib8->long_code_state_1xrtt >> 10),   ie_ptr, 32);
                rrc_value_2_bits((uint32)(sib8->long_code_state_1xrtt & 0x3FF), ie_ptr, 10);
            }

            if(true == sib8->cell_resel_params_1xrtt_present)
            {
                // Optional indicator
                rrc_value_2_bits(sib8->cell_resel_params_1xrtt.t_resel_cdma2000_sf_present, ie_ptr, 1);

                rrc_value_2_bits(sib8->cell_resel_params_1xrtt.band_class_list_size - 1, ie_ptr, 5);
                for(i=0; i<sib8->cell_resel_params_1xrtt.band_class_list_size; i++)
                {
                    // Extension indicator
                    rrc_value_2_bits(0, ie_ptr, 1);

                    // Optional indicator
                    rrc_value_2_bits(sib8->cell_resel_params_1xrtt.band_class_list[i].cell_resel_prio_present, ie_ptr, 1);

                    liblte_rrc_pack_band_class_cdma2000_ie(sib8->cell_resel_params_1xrtt.band_class_list[i].band_class, ie_ptr);
                    if(true == sib8->cell_resel_params_1xrtt.band_class_list[i].cell_resel_prio_present)
                    {
                        liblte_rrc_pack_cell_reselection_priority_ie(sib8->cell_resel_params_1xrtt.band_class_list[i].cell_resel_prio, ie_ptr);
                    }
                    rrc_value_2_bits(sib8->cell_resel_params_1xrtt.band_class_list[i].thresh_x_high, ie_ptr, 6);
                    rrc_value_2_bits(sib8->cell_resel_params_1xrtt.band_class_list[i].thresh_x_low,  ie_ptr, 6);
                }

                rrc_value_2_bits(sib8->cell_resel_params_1xrtt.neigh_cell_list_size - 1, ie_ptr, 4);
                for(i=0; i<sib8->cell_resel_params_1xrtt.neigh_cell_list_size; i++)
                {
                    neigh_cell_list = &sib8->cell_resel_params_1xrtt.neigh_cell_list[i];
                    liblte_rrc_pack_band_class_cdma2000_ie(neigh_cell_list->band_class, ie_ptr);
                    rrc_value_2_bits(neigh_cell_list->neigh_cells_per_freq_list_size - 1, ie_ptr, 4);
                    for(j=0; j<neigh_cell_list->neigh_cells_per_freq_list_size; j++)
                    {
                        liblte_rrc_pack_arfcn_value_cdma2000_ie(neigh_cell_list->neigh_cells_per_freq_list[j].arfcn, ie_ptr);
                        rrc_value_2_bits(neigh_cell_list->neigh_cells_per_freq_list[j].phys_cell_id_list_size - 1, ie_ptr, 4);
                        for(k=0; k<neigh_cell_list->neigh_cells_per_freq_list[j].phys_cell_id_list_size; k++)
                        {
                            liblte_rrc_pack_phys_cell_id_cdma2000_ie(neigh_cell_list->neigh_cells_per_freq_list[j].phys_cell_id_list[k], ie_ptr);
                        }
                    }
                }
                liblte_rrc_pack_t_reselection_ie(sib8->cell_resel_params_1xrtt.t_resel_cdma2000, ie_ptr);

                if(true == sib8->cell_resel_params_1xrtt.t_resel_cdma2000_sf_present)
                {
                    liblte_rrc_pack_speed_state_scale_factors_ie(&sib8->cell_resel_params_1xrtt.t_resel_cdma2000_sf, ie_ptr);
                }
            }
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_sys_info_block_type_8_ie(uint8                                   **ie_ptr,
                                                             LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_8_STRUCT  *sib8)
{
    LIBLTE_RRC_NEIGH_CELL_CDMA2000_STRUCT *neigh_cell_list;
    LIBLTE_ERROR_ENUM                      err = LIBLTE_ERROR_INVALID_INPUTS;
    uint32                                 i;
    uint32                                 j;
    uint32                                 k;
    bool                                   ext_ind;

    if(ie_ptr != NULL &&
       sib8   != NULL)
    {
        // Extension indicator
        ext_ind = rrc_bits_2_value(ie_ptr, 1);

        // Optional indicators
        sib8->sys_time_info_present   = rrc_bits_2_value(ie_ptr, 1);
        sib8->search_win_size_present = rrc_bits_2_value(ie_ptr, 1);
        sib8->params_hrpd_present     = rrc_bits_2_value(ie_ptr, 1);
        sib8->params_1xrtt_present    = rrc_bits_2_value(ie_ptr, 1);

        if(true == sib8->sys_time_info_present)
        {
            liblte_rrc_unpack_system_time_info_cdma2000_ie(ie_ptr, &sib8->sys_time_info_cdma2000);
        }

        if(true == sib8->search_win_size_present)
        {
            sib8->search_win_size = rrc_bits_2_value(ie_ptr, 4);
        }

        if(true == sib8->params_hrpd_present)
        {
            // Optional indicator
            sib8->cell_resel_params_hrpd_present = rrc_bits_2_value(ie_ptr, 1);

            liblte_rrc_unpack_pre_registration_info_hrpd_ie(ie_ptr, &sib8->pre_reg_info_hrpd);

            if(true == sib8->cell_resel_params_hrpd_present)
            {
                // Optional indicator
                sib8->cell_resel_params_hrpd.t_resel_cdma2000_sf_present = rrc_bits_2_value(ie_ptr, 1);

                sib8->cell_resel_params_hrpd.band_class_list_size = rrc_bits_2_value(ie_ptr, 5) + 1;
                for(i=0; i<sib8->cell_resel_params_hrpd.band_class_list_size; i++)
                {
                    // Extension indicator
                    rrc_bits_2_value(ie_ptr, 1);

                    // Optional indicator
                    sib8->cell_resel_params_hrpd.band_class_list[i].cell_resel_prio_present = rrc_bits_2_value(ie_ptr, 1);

                    liblte_rrc_unpack_band_class_cdma2000_ie(ie_ptr, &sib8->cell_resel_params_hrpd.band_class_list[i].band_class);
                    if(true == sib8->cell_resel_params_hrpd.band_class_list[i].cell_resel_prio_present)
                    {
                        liblte_rrc_unpack_cell_reselection_priority_ie(ie_ptr, &sib8->cell_resel_params_hrpd.band_class_list[i].cell_resel_prio);
                    }
                    sib8->cell_resel_params_hrpd.band_class_list[i].thresh_x_high = rrc_bits_2_value(ie_ptr, 6);
                    sib8->cell_resel_params_hrpd.band_class_list[i].thresh_x_low  = rrc_bits_2_value(ie_ptr, 6);
                }

                sib8->cell_resel_params_hrpd.neigh_cell_list_size = rrc_bits_2_value(ie_ptr, 4) + 1;
                for(i=0; i<sib8->cell_resel_params_hrpd.neigh_cell_list_size; i++)
                {
                    neigh_cell_list = &sib8->cell_resel_params_hrpd.neigh_cell_list[i];
                    liblte_rrc_unpack_band_class_cdma2000_ie(ie_ptr, &neigh_cell_list->band_class);
                    neigh_cell_list->neigh_cells_per_freq_list_size = rrc_bits_2_value(ie_ptr, 4) + 1;
                    for(j=0; j<neigh_cell_list->neigh_cells_per_freq_list_size; j++)
                    {
                        liblte_rrc_unpack_arfcn_value_cdma2000_ie(ie_ptr, &neigh_cell_list->neigh_cells_per_freq_list[j].arfcn);
                        neigh_cell_list->neigh_cells_per_freq_list[j].phys_cell_id_list_size = rrc_bits_2_value(ie_ptr, 4) + 1;
                        for(k=0; k<neigh_cell_list->neigh_cells_per_freq_list[j].phys_cell_id_list_size; k++)
                        {
                            liblte_rrc_unpack_phys_cell_id_cdma2000_ie(ie_ptr, &neigh_cell_list->neigh_cells_per_freq_list[j].phys_cell_id_list[k]);
                        }
                    }
                }
                liblte_rrc_unpack_t_reselection_ie(ie_ptr, &sib8->cell_resel_params_hrpd.t_resel_cdma2000);

                if(true == sib8->cell_resel_params_hrpd.t_resel_cdma2000_sf_present)
                {
                    liblte_rrc_unpack_speed_state_scale_factors_ie(ie_ptr, &sib8->cell_resel_params_hrpd.t_resel_cdma2000_sf);
                }
            }
        }else{
            sib8->cell_resel_params_hrpd_present = false;
        }

        if(true == sib8->params_1xrtt_present)
        {
            // Optional indicators
            sib8->csfb_reg_param_1xrtt_present    = rrc_bits_2_value(ie_ptr, 1);
            sib8->long_code_state_1xrtt_present   = rrc_bits_2_value(ie_ptr, 1);
            sib8->cell_resel_params_1xrtt_present = rrc_bits_2_value(ie_ptr, 1);

            if(true == sib8->csfb_reg_param_1xrtt_present)
            {
                liblte_rrc_unpack_csfb_registration_param_1xrtt_ie(ie_ptr, &sib8->csfb_reg_param_1xrtt);
            }

            if(true == sib8->long_code_state_1xrtt_present)
            {
                sib8->long_code_state_1xrtt  = (uint64)rrc_bits_2_value(ie_ptr, 32) << 10;
                sib8->long_code_state_1xrtt |= (uint64)rrc_bits_2_value(ie_ptr, 10);
            }

            if(true == sib8->cell_resel_params_1xrtt_present)
            {
                // Optional indicator
                sib8->cell_resel_params_1xrtt.t_resel_cdma2000_sf_present = rrc_bits_2_value(ie_ptr, 1);

                sib8->cell_resel_params_1xrtt.band_class_list_size = rrc_bits_2_value(ie_ptr, 5) + 1;
                for(i=0; i<sib8->cell_resel_params_1xrtt.band_class_list_size; i++)
                {
                    // Extension indicator
                    rrc_bits_2_value(ie_ptr, 1);

                    // Optional indicator
                    sib8->cell_resel_params_1xrtt.band_class_list[i].cell_resel_prio_present = rrc_bits_2_value(ie_ptr, 1);

                    liblte_rrc_unpack_band_class_cdma2000_ie(ie_ptr, &sib8->cell_resel_params_1xrtt.band_class_list[i].band_class);
                    if(true == sib8->cell_resel_params_1xrtt.band_class_list[i].cell_resel_prio_present)
                    {
                        liblte_rrc_unpack_cell_reselection_priority_ie(ie_ptr, &sib8->cell_resel_params_1xrtt.band_class_list[i].cell_resel_prio);
                    }
                    sib8->cell_resel_params_1xrtt.band_class_list[i].thresh_x_high = rrc_bits_2_value(ie_ptr, 6);
                    sib8->cell_resel_params_1xrtt.band_class_list[i].thresh_x_low  = rrc_bits_2_value(ie_ptr, 6);
                }

                sib8->cell_resel_params_1xrtt.neigh_cell_list_size = rrc_bits_2_value(ie_ptr, 4) + 1;
                for(i=0; i<sib8->cell_resel_params_1xrtt.neigh_cell_list_size; i++)
                {
                    neigh_cell_list = &sib8->cell_resel_params_1xrtt.neigh_cell_list[i];
                    liblte_rrc_unpack_band_class_cdma2000_ie(ie_ptr, &neigh_cell_list->band_class);
                    neigh_cell_list->neigh_cells_per_freq_list_size = rrc_bits_2_value(ie_ptr, 4) + 1;
                    for(j=0; j<neigh_cell_list->neigh_cells_per_freq_list_size; j++)
                    {
                        liblte_rrc_unpack_arfcn_value_cdma2000_ie(ie_ptr, &neigh_cell_list->neigh_cells_per_freq_list[j].arfcn);
                        neigh_cell_list->neigh_cells_per_freq_list[j].phys_cell_id_list_size = rrc_bits_2_value(ie_ptr, 4) + 1;
                        for(k=0; k<neigh_cell_list->neigh_cells_per_freq_list[j].phys_cell_id_list_size; k++)
                        {
                            liblte_rrc_unpack_phys_cell_id_cdma2000_ie(ie_ptr, &neigh_cell_list->neigh_cells_per_freq_list[j].phys_cell_id_list[k]);
                        }
                    }
                }
                liblte_rrc_unpack_t_reselection_ie(ie_ptr, &sib8->cell_resel_params_1xrtt.t_resel_cdma2000);

                if(true == sib8->cell_resel_params_1xrtt.t_resel_cdma2000_sf_present)
                {
                    liblte_rrc_unpack_speed_state_scale_factors_ie(ie_ptr, &sib8->cell_resel_params_1xrtt.t_resel_cdma2000_sf);
                }
            }
        }else{
            sib8->csfb_reg_param_1xrtt_present    = false;
            sib8->long_code_state_1xrtt_present   = false;
            sib8->cell_resel_params_1xrtt_present = false;
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
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
    LIBLTE_ERROR_ENUM  err     = LIBLTE_ERROR_INVALID_INPUTS;
    uint8             *msg_ptr = msg->msg;
    uint32             i;
    uint32             j;
    uint8              non_crit_ext_opt        = false;
    uint8              csg_id_opt              = false;
    uint8              q_rx_lev_min_offset_opt = false;
    uint8              extension               = false;

    if(sib1 != NULL &&
       msg  != NULL)
    {
        // Optional indicators
        rrc_value_2_bits(sib1->p_max_present, &msg_ptr, 1);
        rrc_value_2_bits(sib1->tdd,           &msg_ptr, 1);
        rrc_value_2_bits(non_crit_ext_opt,    &msg_ptr, 1);

        // Cell Access Related Info
        rrc_value_2_bits(csg_id_opt,           &msg_ptr, 1);
        rrc_value_2_bits(sib1->N_plmn_ids - 1, &msg_ptr, 3);
        for(i=0; i<sib1->N_plmn_ids; i++)
        {
            liblte_rrc_pack_plmn_identity_ie(&sib1->plmn_id[i].id, &msg_ptr);
            rrc_value_2_bits(sib1->plmn_id[i].resv_for_oper, &msg_ptr, 1);
        }
        liblte_rrc_pack_tracking_area_code_ie(sib1->tracking_area_code, &msg_ptr);
        liblte_rrc_pack_cell_identity_ie(sib1->cell_id, &msg_ptr);
        rrc_value_2_bits(sib1->cell_barred,            &msg_ptr, 1);
        rrc_value_2_bits(sib1->intra_freq_reselection, &msg_ptr, 1);
        rrc_value_2_bits(sib1->csg_indication,         &msg_ptr, 1);
        if(true == csg_id_opt)
        {
            liblte_rrc_pack_csg_identity_ie(sib1->csg_id, &msg_ptr);
        }

        // Cell Selection Info
        rrc_value_2_bits(q_rx_lev_min_offset_opt, &msg_ptr, 1);
        liblte_rrc_pack_q_rx_lev_min_ie(sib1->q_rx_lev_min, &msg_ptr);
        if(true == q_rx_lev_min_offset_opt)
        {
            rrc_value_2_bits((sib1->q_rx_lev_min_offset / 2) - 1, &msg_ptr, 3);
        }

        // P Max
        if(true == sib1->p_max_present)
        {
            liblte_rrc_pack_p_max_ie(sib1->p_max, &msg_ptr);
        }

        // Freq Band Indicator
        rrc_value_2_bits(sib1->freq_band_indicator - 1, &msg_ptr, 6);

        // Scheduling Info List
        rrc_value_2_bits(sib1->N_sched_info - 1, &msg_ptr, 5);
        for(i=0; i<sib1->N_sched_info; i++)
        {
            rrc_value_2_bits(sib1->sched_info[i].si_periodicity,     &msg_ptr, 3);
            rrc_value_2_bits(sib1->sched_info[i].N_sib_mapping_info, &msg_ptr, 5);
            for(j=0; j<sib1->sched_info[i].N_sib_mapping_info; j++)
            {
                rrc_value_2_bits(extension,                                        &msg_ptr, 1);
                rrc_value_2_bits(sib1->sched_info[i].sib_mapping_info[j].sib_type, &msg_ptr, 4);
            }
        }

        // TDD Config
        if(true == sib1->tdd)
        {
            liblte_rrc_pack_tdd_config_ie(sib1->sf_assignment, sib1->special_sf_patterns, &msg_ptr);
        }

        // SI Window Length
        rrc_value_2_bits(sib1->si_window_length, &msg_ptr, 3);

        // System Info Value Tag
        rrc_value_2_bits(sib1->system_info_value_tag, &msg_ptr, 5);

        // Non Critical Extension
        // FIXME

        // Fill in the number of bits used
        msg->N_bits = msg_ptr - msg->msg;

        err = LIBLTE_SUCCESS;
    }

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
    bool               tdd_config_opt;
    bool               non_crit_ext_opt;
    bool               csg_id_opt;
    bool               q_rx_lev_min_offset_opt;
    bool               extension;

    if(msg         != NULL &&
       sib1        != NULL &&
       N_bits_used != NULL)
    {
        // Optional indicators
        sib1->p_max_present = rrc_bits_2_value(&msg_ptr, 1);
        tdd_config_opt      = rrc_bits_2_value(&msg_ptr, 1);
        non_crit_ext_opt    = rrc_bits_2_value(&msg_ptr, 1);

        // Cell Access Related Info
        csg_id_opt       = rrc_bits_2_value(&msg_ptr, 1);
        sib1->N_plmn_ids = rrc_bits_2_value(&msg_ptr, 3) + 1;
        for(i=0; i<sib1->N_plmn_ids; i++)
        {
            liblte_rrc_unpack_plmn_identity_ie(&msg_ptr, &sib1->plmn_id[i].id);
            if(LIBLTE_RRC_MCC_NOT_PRESENT == sib1->plmn_id[i].id.mcc &&
               0                          != i)
            {
                sib1->plmn_id[i].id.mcc = sib1->plmn_id[i-1].id.mcc;
            }
            sib1->plmn_id[i].resv_for_oper = (LIBLTE_RRC_RESV_FOR_OPER_ENUM)rrc_bits_2_value(&msg_ptr, 1);
        }
        liblte_rrc_unpack_tracking_area_code_ie(&msg_ptr, &sib1->tracking_area_code);
        liblte_rrc_unpack_cell_identity_ie(&msg_ptr, &sib1->cell_id);
        sib1->cell_barred            = (LIBLTE_RRC_CELL_BARRED_ENUM)rrc_bits_2_value(&msg_ptr, 1);
        sib1->intra_freq_reselection = (LIBLTE_RRC_INTRA_FREQ_RESELECTION_ENUM)rrc_bits_2_value(&msg_ptr, 1);
        sib1->csg_indication         = rrc_bits_2_value(&msg_ptr, 1);
        if(true == csg_id_opt)
        {
            liblte_rrc_unpack_csg_identity_ie(&msg_ptr, &sib1->csg_id);
        }else{
            sib1->csg_id = LIBLTE_RRC_CSG_IDENTITY_NOT_PRESENT;
        }

        // Cell Selection Info
        q_rx_lev_min_offset_opt = rrc_bits_2_value(&msg_ptr, 1);
        liblte_rrc_unpack_q_rx_lev_min_ie(&msg_ptr, &sib1->q_rx_lev_min);
        if(true == q_rx_lev_min_offset_opt)
        {
            sib1->q_rx_lev_min_offset = (rrc_bits_2_value(&msg_ptr, 3) + 1) * 2;
        }else{
            sib1->q_rx_lev_min_offset = 0;
        }

        // P Max
        if(true == sib1->p_max_present)
        {
            liblte_rrc_unpack_p_max_ie(&msg_ptr, &sib1->p_max);
        }

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
            liblte_rrc_unpack_tdd_config_ie(&msg_ptr, &sib1->sf_assignment, &sib1->special_sf_patterns);
        }else{
            sib1->tdd = false;
        }

        // SI Window Length
        sib1->si_window_length = (LIBLTE_RRC_SI_WINDOW_LENGTH_ENUM)rrc_bits_2_value(&msg_ptr, 3);

        // System Info Value Tag
        sib1->system_info_value_tag = rrc_bits_2_value(&msg_ptr, 5);

        // Non Critical Extension
        if(true == non_crit_ext_opt)
        {
            // FIXME
        }

        // N_bits_used
        *N_bits_used = msg_ptr - (msg->msg);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    Message Name: System Information

    Description: Conveys one or more System Information Blocks.

    Document Reference: 36.331 v10.0.0 Section 6.2.2
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_rrc_pack_sys_info_msg(LIBLTE_RRC_SYS_INFO_MSG_STRUCT *sibs,
                                               LIBLTE_RRC_MSG_STRUCT          *msg)
{
    LIBLTE_ERROR_ENUM  err     = LIBLTE_ERROR_INVALID_INPUTS;
    uint8             *msg_ptr = msg->msg;
    uint32             i;

    if(sibs != NULL &&
       msg  != NULL)
    {
        // Critical extensions choice
        rrc_value_2_bits(0, &msg_ptr, 1);

        // Optional indicator
        rrc_value_2_bits(0, &msg_ptr, 1);

        // Number of SIBs present
        rrc_value_2_bits(sibs->N_sibs - 1, &msg_ptr, 5);

        for(i=0; i<sibs->N_sibs; i++)
        {
            // Extension indicator
            rrc_value_2_bits(0, &msg_ptr, 1);

            rrc_value_2_bits(sibs->sibs[i].sib_type, &msg_ptr, 4);
            switch(sibs->sibs[i].sib_type)
            {
            case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_2:
                err = liblte_rrc_pack_sys_info_block_type_2_ie((LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_2_STRUCT *)&sibs->sibs[i].sib,
                                                               &msg_ptr);
                break;
            case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_3:
                err = liblte_rrc_pack_sys_info_block_type_3_ie((LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_3_STRUCT *)&sibs->sibs[i].sib,
                                                               &msg_ptr);
                break;
            case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_4:
                err = liblte_rrc_pack_sys_info_block_type_4_ie((LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_4_STRUCT *)&sibs->sibs[i].sib,
                                                               &msg_ptr);
                break;
            case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_8:
                err = liblte_rrc_pack_sys_info_block_type_8_ie((LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_8_STRUCT *)&sibs->sibs[i].sib,
                                                               &msg_ptr);
                break;
            case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_5:
            case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_6:
            case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_7:
            case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_9:
            case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_10:
            case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_11:
            default:
                printf("ERROR: Not handling sib type %u\n", sibs->sibs[i].sib_type);
                err = LIBLTE_ERROR_INVALID_INPUTS;
                break;
            }

            if(LIBLTE_SUCCESS != err)
            {
                break;
            }
        }
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_sys_info_msg(LIBLTE_RRC_MSG_STRUCT          *msg,
                                                 LIBLTE_RRC_SYS_INFO_MSG_STRUCT *sibs)
{
    LIBLTE_ERROR_ENUM  err     = LIBLTE_ERROR_INVALID_INPUTS;
    uint8             *msg_ptr = msg->msg;
    uint32             i;
    uint8              non_crit_ext_opt;

    if(msg  != NULL &&
       sibs != NULL)
    {
        // Critical extensions choice
        if(0 == rrc_bits_2_value(&msg_ptr, 1))
        {
            // Optional indicator
            non_crit_ext_opt = rrc_bits_2_value(&msg_ptr, 1);

            // Number of SIBs present
            sibs->N_sibs = rrc_bits_2_value(&msg_ptr, 5) + 1;

            for(i=0; i<sibs->N_sibs; i++)
            {
                // Extension indicator
                if(0 == rrc_bits_2_value(&msg_ptr, 1))
                {
                    sibs->sibs[i].sib_type = (LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_ENUM)rrc_bits_2_value(&msg_ptr, 4);
                    switch(sibs->sibs[i].sib_type)
                    {
                    case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_2:
                        err = liblte_rrc_unpack_sys_info_block_type_2_ie(&msg_ptr,
                                                                         (LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_2_STRUCT *)&sibs->sibs[i].sib);
                        break;
                    case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_3:
                        err = liblte_rrc_unpack_sys_info_block_type_3_ie(&msg_ptr,
                                                                         (LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_3_STRUCT *)&sibs->sibs[i].sib);
                        break;
                    case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_4:
                        err = liblte_rrc_unpack_sys_info_block_type_4_ie(&msg_ptr,
                                                                         (LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_4_STRUCT *)&sibs->sibs[i].sib);
                        break;
                    case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_8:
                        err = liblte_rrc_unpack_sys_info_block_type_8_ie(&msg_ptr,
                                                                         (LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_8_STRUCT *)&sibs->sibs[i].sib);
                        break;
                    case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_5:
                    case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_6:
                    case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_7:
                    case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_9:
                    case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_10:
                    case LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_11:
                    default:
                        printf("ERROR: Not handling sib type %u\n", sibs->sibs[i].sib_type);
                        err = LIBLTE_ERROR_INVALID_INPUTS;
                        break;
                    }
                }else{
                    printf("ERROR: Not handling extended SIB type and info\n");
                    err = LIBLTE_ERROR_INVALID_INPUTS;
                }

                if(LIBLTE_SUCCESS != err)
                {
                    break;
                }
            }
        }else{
            printf("ERROR: Not handling critical extensions in system information message\n");
        }
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
    LIBLTE_ERROR_ENUM  err     = LIBLTE_ERROR_INVALID_INPUTS;
    uint8             *msg_ptr = msg->msg;

    if(mib != NULL &&
       msg != NULL)
    {
        // DL Bandwidth
        rrc_value_2_bits(mib->dl_bw, &msg_ptr, 3);

        // PHICH Config
        liblte_rrc_pack_phich_config_ie(&mib->phich_config, &msg_ptr);

        // SFN/4
        rrc_value_2_bits(mib->sfn_div_4, &msg_ptr, 8);

        // Spare
        rrc_value_2_bits(0, &msg_ptr, 10);

        // Fill in the number of bits used
        msg->N_bits = msg_ptr - msg->msg;

        err = LIBLTE_SUCCESS;
    }

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

        // PHICH Config
        liblte_rrc_unpack_phich_config_ie(&msg_ptr, &mib->phich_config);

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
LIBLTE_ERROR_ENUM liblte_rrc_pack_bcch_dlsch_msg(LIBLTE_RRC_BCCH_DLSCH_MSG_STRUCT *bcch_dlsch_msg,
                                                 LIBLTE_RRC_MSG_STRUCT            *msg)
{
    LIBLTE_ERROR_ENUM  err     = LIBLTE_ERROR_INVALID_INPUTS;
    uint8             *msg_ptr = msg->msg;
    uint8              ext     = false;

    if(bcch_dlsch_msg != NULL &&
       msg            != NULL)
    {
        // Extension indicator
        rrc_value_2_bits(ext, &msg_ptr, 1);

        if(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1 == bcch_dlsch_msg->sibs[0].sib_type)
        {
            // SIB1 Choice
            rrc_value_2_bits(1, &msg_ptr, 1);

            liblte_rrc_pack_sys_info_block_type_1_msg((LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1_STRUCT *)&bcch_dlsch_msg->sibs[0].sib,
                                                      &global_msg);
            memcpy(msg_ptr, global_msg.msg, global_msg.N_bits);
            msg->N_bits = global_msg.N_bits + 2;
        }else{
            // SIB1 Choice
            rrc_value_2_bits(0, &msg_ptr, 1);

            liblte_rrc_pack_sys_info_msg(bcch_dlsch_msg,
                                         &global_msg);
            memcpy(msg_ptr, global_msg.msg, global_msg.N_bits);
            msg->N_bits = global_msg.N_bits + 2;
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_rrc_unpack_bcch_dlsch_msg(LIBLTE_RRC_MSG_STRUCT            *msg,
                                                   LIBLTE_RRC_BCCH_DLSCH_MSG_STRUCT *bcch_dlsch_msg)
{
    LIBLTE_ERROR_ENUM  err     = LIBLTE_ERROR_INVALID_INPUTS;
    uint8             *msg_ptr = msg->msg;
    uint32             N_bits_used;
    uint8              ext;

    if(msg            != NULL &&
       bcch_dlsch_msg != NULL)
    {
        // Extension indicator
        ext = rrc_bits_2_value(&msg_ptr, 1);

        // SIB1 Choice
        if(true == rrc_bits_2_value(&msg_ptr, 1))
        {
            bcch_dlsch_msg->N_sibs           = 1;
            bcch_dlsch_msg->sibs[0].sib_type = LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1;
            memcpy(global_msg.msg, msg_ptr, msg->N_bits-(msg_ptr-msg->msg));
            global_msg.N_bits = msg->N_bits-(msg_ptr-msg->msg);
            err               = liblte_rrc_unpack_sys_info_block_type_1_msg(&global_msg,
                                                                            (LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1_STRUCT *)&bcch_dlsch_msg->sibs[0].sib,
                                                                            &N_bits_used);
            msg_ptr += N_bits_used;
        }else{
            memcpy(global_msg.msg, msg_ptr, msg->N_bits-(msg_ptr-msg->msg));
            err = liblte_rrc_unpack_sys_info_msg(&global_msg,
                                                 bcch_dlsch_msg);
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
void rrc_value_2_bits(uint32   value,
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
