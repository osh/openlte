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

    File: liblte_mme.cc

    Description: Contains all the implementations for the LTE Mobility
                 Management Entity library.

    Revision History
    ----------    -------------    --------------------------------------------
    06/15/2014    Ben Wojtowicz    Created file.

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "liblte_mme.h"

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


/*******************************************************************************
                              INFORMATION ELEMENT FUNCTIONS
*******************************************************************************/

/*********************************************************************
    IE Name: Additional Information

    Description: Provides additional information to upper layers in
                 relation to the generic NAS message transport
                 mechanism.

    Document Reference: 24.301 v10.2.0 Section 9.9.2.0
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: Device Properties

    Description: Indicates if the UE is configured for NAS signalling
                 low priority.

    Document Reference: 24.301 v10.2.0 Section 9.9.2.0A
                        24.008 v10.2.0 Section 10.5.7.8
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mme_pack_device_properties_ie(LIBLTE_MME_DEVICE_PROPERTIES_ENUM   device_props,
                                                       uint8                               bit_offset,
                                                       uint8                             **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        **ie_ptr |= device_props << bit_offset;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mme_unpack_device_properties_ie(uint8                             **ie_ptr,
                                                         uint8                               bit_offset,
                                                         LIBLTE_MME_DEVICE_PROPERTIES_ENUM  *device_props)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr       != NULL &&
       device_props != NULL)
    {
        *device_props = (LIBLTE_MME_DEVICE_PROPERTIES_ENUM)((**ie_ptr >> bit_offset) & 0x01);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: EPS Bearer Context Status

    Description: Indicates the state of each EPS bearer context that
                 can be identified by an EPS bearer identity.

    Document Reference: 24.301 v10.2.0 Section 9.9.2.1
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: Location Area Identification

    Description: Provides an unambiguous identification of location
                 areas within the area covered by the 3GPP system.

    Document Reference: 24.301 v10.2.0 Section 9.9.2.2
                        24.008 v10.2.0 Section 10.5.1.3
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mme_pack_location_area_id_ie(LIBLTE_MME_LOCATION_AREA_ID_STRUCT  *lai,
                                                      uint8                              **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(lai    != NULL &&
       ie_ptr != NULL)
    {
        **ie_ptr  = ((lai->mcc/10) % 10) | ((lai->mcc/100) % 10);
        *ie_ptr  += 1;
        if(lai->mnc > 100)
        {
            **ie_ptr  = 0xF | (lai->mcc % 10);
            *ie_ptr  += 1;
            **ie_ptr  = (lai->mnc % 10) | ((lai->mnc/10) % 10);
            *ie_ptr  += 1;
        }else{
            **ie_ptr  = (lai->mnc % 10) | (lai->mcc % 10);
            *ie_ptr  += 1;
            **ie_ptr  = ((lai->mnc/10) % 10) | ((lai->mnc/100) % 10);
            *ie_ptr  += 1;
        }
        **ie_ptr  = (lai->lac >> 8) & 0xFF;
        *ie_ptr  += 1;
        **ie_ptr  = lai->lac & 0xFF;
        *ie_ptr  += 1;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mme_unpack_location_area_id_ie(uint8                              **ie_ptr,
                                                        LIBLTE_MME_LOCATION_AREA_ID_STRUCT  *lai)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL &&
       lai    != NULL)
    {
        lai->mcc  = (**ie_ptr & 0x0F)*100;
        lai->mcc += ((**ie_ptr >> 4) & 0x0F)*10;
        *ie_ptr  += 1;
        lai->mcc += **ie_ptr & 0x0F;
        if(((**ie_ptr >> 4) & 0x0F) == 0x0F)
        {
            *ie_ptr  += 1;
            lai->mnc  = (**ie_ptr & 0x0F)*10;
            lai->mnc += (**ie_ptr >> 4) & 0x0F;
            *ie_ptr  += 1;
        }else{
            lai->mnc  = (**ie_ptr >> 4) & 0x0F;
            *ie_ptr  += 1;
            lai->mnc += (**ie_ptr & 0x0F)*100;
            lai->mnc += ((**ie_ptr >> 4) & 0x0F)*10;
            *ie_ptr  += 1;
        }
        lai->lac  = **ie_ptr << 8;
        *ie_ptr  += 1;
        lai->lac |= **ie_ptr;
        *ie_ptr  += 1;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Mobile Identity

    Description: Provides either the IMSI, TMSI/P-TMSI/M-TMSI, IMEI,
                 IMEISV, or TMGI, associated with the optional MBMS
                 session identity.

    Document Reference: 24.301 v10.2.0 Section 9.9.2.3
                        24.008 v10.2.0 Section 10.5.1.4
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: Mobile Station Classmark 2

    Description: Provides the network with information concerning
                 aspects of both high and low priority of the UE.

    Document Reference: 24.301 v10.2.0 Section 9.9.2.4
                        24.008 v10.2.0 Section 10.5.1.6
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mme_pack_mobile_station_classmark_2_ie(LIBLTE_MME_MOBILE_STATION_CLASSMARK_2_STRUCT  *ms_cm2,
                                                                uint8                                        **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ms_cm2 != NULL &&
       ie_ptr != NULL)
    {
        // FIXME
        **ie_ptr  = 3;
        *ie_ptr  += 1;
        *ie_ptr  += 3;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mme_unpack_mobile_station_classmark_2_ie(uint8                                        **ie_ptr,
                                                                  LIBLTE_MME_MOBILE_STATION_CLASSMARK_2_STRUCT  *ms_cm2)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL &&
       ms_cm2 != NULL)
    {
        // FIXME
        *ie_ptr += 4;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Mobile Station Classmark 3

    Description: Provides the network with information concerning
                 aspects of the UE.

    Document Reference: 24.301 v10.2.0 Section 9.9.2.5
                        24.008 v10.2.0 Section 10.5.1.7
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mme_pack_mobile_station_classmark_3_ie(LIBLTE_MME_MOBILE_STATION_CLASSMARK_3_STRUCT  *ms_cm3,
                                                                uint8                                        **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ms_cm3 != NULL &&
       ie_ptr != NULL)
    {
        // FIXME

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mme_unpack_mobile_station_classmark_3_ie(uint8                                        **ie_ptr,
                                                                  LIBLTE_MME_MOBILE_STATION_CLASSMARK_3_STRUCT  *ms_cm3)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL &&
       ms_cm3 != NULL)
    {
        // FIXME

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: NAS Security Parameters From E-UTRA

    Description: Provides the UE with information that enables the UE
                 to create a mapped UMTS security context.

    Document Reference: 24.301 v10.2.0 Section 9.9.2.6
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: NAS Security Parameters To E-UTRA

    Description: Provides the UE with parameters that enables the UE
                 to create a mapped EPS security context and take
                 this context into use after inter-system handover to
                 S1 mode.

    Document Reference: 24.301 v10.2.0 Section 9.9.2.7
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: PLMN List

    Description: Provides a list of PLMN codes to the UE.

    Document Reference: 24.301 v10.2.0 Section 9.9.2.8
                        24.008 v10.2.0 Section 10.5.1.13
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: Spare Half Octet

    Description: Used in the description of EMM and ESM messages when
                 an odd number of half octet type 1 information
                 elements are used.

    Document Reference: 24.301 v10.2.0 Section 9.9.2.9
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: Supported Codec List

    Description: Provides the network with information about the
                 speech codecs supported by the UE.

    Document Reference: 24.301 v10.2.0 Section 9.9.2.10
                        24.008 v10.2.0 Section 10.5.4.32
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mme_pack_supported_codec_list_ie(LIBLTE_MME_SUPPORTED_CODEC_LIST_STRUCT  *supported_codec_list,
                                                          uint8                                  **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(supported_codec_list != NULL &&
       ie_ptr               != NULL)
    {
        // FIXME
        **ie_ptr  = 0;
        *ie_ptr  += 1;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mme_unpack_supported_codec_list_ie(uint8                                  **ie_ptr,
                                                            LIBLTE_MME_SUPPORTED_CODEC_LIST_STRUCT  *supported_codec_list)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr               != NULL &&
       supported_codec_list != NULL)
    {
        // FIXME
        *ie_ptr += **ie_ptr;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Additional Update Result

    Description: Provides additional information about the result of
                 a combined attached procedure or a combined tracking
                 area updating procedure.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.0A
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: Additional Update Type

    Description: Provides additional information about the type of
                 request for a combined attach or a combined tracking
                 area updating procedure.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.0B
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mme_pack_additional_update_type_ie(LIBLTE_MME_ADDITIONAL_UPDATE_TYPE_ENUM   aut,
                                                            uint8                                    bit_offset,
                                                            uint8                                  **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        **ie_ptr |= aut << bit_offset;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mme_unpack_additional_update_type_ie(uint8                                  **ie_ptr,
                                                              uint8                                    bit_offset,
                                                              LIBLTE_MME_ADDITIONAL_UPDATE_TYPE_ENUM  *aut)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL &&
       aut    != NULL)
    {
        *aut = (LIBLTE_MME_ADDITIONAL_UPDATE_TYPE_ENUM)((**ie_ptr >> bit_offset) & 0x01);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Authentication Failure Parameter

    Description: Provides the network with the necessary information
                 to begin a re-authentication procedure in the case
                 of a 'Synch failure', following a UMTS or EPS
                 authentication challenge.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.1
                        24.008 v10.2.0 Section 10.5.3.2.2
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: Authentication Parameter AUTN

    Description: Provides the UE with a means of authenticating the
                 network.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.2
                        24.008 v10.2.0 Section 10.5.3.1.1
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: Authentication Parameter RAND

    Description: Provides the UE with a non-predictable number to be
                 used to calculate the authentication signature SRES
                 and the ciphering key Kc (for a GSM authentication
                 challenge), or the response RES and both the
                 ciphering key CK and the integrity key IK (for a
                 UMTS authentication challenge).

    Document Reference: 24.301 v10.2.0 Section 9.9.3.3
                        24.008 v10.2.0 Section 10.5.3.1
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: Authentication Response Parameter

    Description: Provides the network with the authentication
                 response calculated in the USIM.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.4
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: Ciphering Key Sequence Number

    Description: Makes it possible for the network to identify the
                 ciphering key Kc which is stored in the UE without
                 invoking the authentication procedure.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.4A
                        24.008 v10.2.0 Section 10.5.1.2
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: CSFB Response

    Description: Indicates whether the UE accepts or rejects a paging
                 for CS fallback.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.5
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: Daylight Savings Time

    Description: Encodes the daylight savings time in steps of 1 hour.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.6
                        24.008 v10.2.0 Section 10.5.3.12
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: Detach Type

    Description: Indicates the type of detach.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.7
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: DRX Parameter

    Description: Indicates whether the UE uses DRX mode or not.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.8
                        24.008 v10.2.0 Section 10.5.5.6
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mme_pack_drx_parameter_ie(LIBLTE_MME_DRX_PARAMETER_STRUCT  *drx_param,
                                                   uint8                           **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(drx_param != NULL &&
       ie_ptr    != NULL)
    {
        // FIXME
        *ie_ptr += 2;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mme_unpack_drx_parameter_ie(uint8                           **ie_ptr,
                                                     LIBLTE_MME_DRX_PARAMETER_STRUCT  *drx_param)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr    != NULL &&
       drx_param != NULL)
    {
        // FIXME
        *ie_ptr += 2;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: EMM Cause

    Description: Indicates the reason why an EMM request from the UE
                 is rejected by the network.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.9
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: EPS Attach Result

    Description: Specifies the result of an attach procedure.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.10
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: EPS Attach Type

    Description: Indicates the type of the requested attach.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.11
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mme_pack_eps_attach_type_ie(uint8   attach_type,
                                                     uint8   bit_offset,
                                                     uint8 **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        **ie_ptr |= attach_type << bit_offset;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mme_unpack_eps_attach_type_ie(uint8 **ie_ptr,
                                                       uint8   bit_offset,
                                                       uint8  *attach_type)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr      != NULL &&
       attach_type != NULL)
    {
        *attach_type = (**ie_ptr >> bit_offset) & 0x07;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: EPS Mobile Identity

    Description: Provides either the IMSI, the GUTI, or the IMEI.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.12
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mme_pack_eps_mobile_id_ie(LIBLTE_MME_EPS_MOBILE_ID_STRUCT  *eps_mobile_id,
                                                   uint8                           **ie_ptr)
{
    LIBLTE_ERROR_ENUM  err = LIBLTE_ERROR_INVALID_INPUTS;
    uint8             *id;
    uint32             i;

    if(eps_mobile_id != NULL &&
       ie_ptr        != NULL)
    {
        if(LIBLTE_MME_EPS_MOBILE_ID_TYPE_GUTI == eps_mobile_id->type_of_id)
        {
            **ie_ptr  = 11;
            *ie_ptr  += 1;
            **ie_ptr  = 0xF0 | eps_mobile_id->type_of_id;
            *ie_ptr  += 1;
            **ie_ptr  = ((eps_mobile_id->guti.mcc/10) % 10) | ((eps_mobile_id->guti.mcc/100) % 10);
            *ie_ptr  += 1;
            if(eps_mobile_id->guti.mcc > 100)
            {
                **ie_ptr  = 0xF | (eps_mobile_id->guti.mcc % 10);
                *ie_ptr  += 1;
                **ie_ptr  = (eps_mobile_id->guti.mnc % 10) | ((eps_mobile_id->guti.mnc/10) % 10);
                *ie_ptr  += 1;
            }else{
                **ie_ptr  = (eps_mobile_id->guti.mnc % 10) | (eps_mobile_id->guti.mcc % 10);
                *ie_ptr  += 1;
                **ie_ptr  = ((eps_mobile_id->guti.mnc/10) % 10) | ((eps_mobile_id->guti.mnc/100) % 10);
                *ie_ptr  += 1;
            }
            **ie_ptr  = (eps_mobile_id->guti.mme_group_id >> 8) & 0x0F;
            *ie_ptr  += 1;
            **ie_ptr  = eps_mobile_id->guti.mme_group_id & 0x0F;
            *ie_ptr  += 1;
            **ie_ptr  = eps_mobile_id->guti.mme_code;
            *ie_ptr  += 1;
            **ie_ptr  = (eps_mobile_id->guti.m_tmsi >> 24) & 0xFF;
            *ie_ptr  += 1;
            **ie_ptr  = (eps_mobile_id->guti.m_tmsi >> 16) & 0xFF;
            *ie_ptr  += 1;
            **ie_ptr  = (eps_mobile_id->guti.m_tmsi >> 8) & 0xFF;
            *ie_ptr  += 1;
            **ie_ptr  = eps_mobile_id->guti.m_tmsi & 0xFF;
            *ie_ptr  += 1;
        }else{
            if(LIBLTE_MME_EPS_MOBILE_ID_TYPE_IMSI == eps_mobile_id->type_of_id)
            {
                id = eps_mobile_id->imsi;
            }else{
                id = eps_mobile_id->imei;
            }

            **ie_ptr  = (id[0] << 4) | (1 << 3) | eps_mobile_id->type_of_id;
            *ie_ptr  += 1;
            for(i=0; i<7; i++)
            {
                **ie_ptr  = (id[i*2+1] << 4) | id[i*2+2];
                *ie_ptr  += 1;
            }
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mme_unpack_eps_mobile_id_ie(uint8                           **ie_ptr,
                                                     LIBLTE_MME_EPS_MOBILE_ID_STRUCT  *eps_mobile_id)
{
    LIBLTE_ERROR_ENUM  err = LIBLTE_ERROR_INVALID_INPUTS;
    uint8             *id;
    uint32             length;
    uint32             i;

    if(ie_ptr        != NULL &&
       eps_mobile_id != NULL)
    {
        length   = **ie_ptr;
        *ie_ptr += 1;

        eps_mobile_id->type_of_id = **ie_ptr & 0x07;

        if(LIBLTE_MME_EPS_MOBILE_ID_TYPE_GUTI == eps_mobile_id->type_of_id)
        {
            *ie_ptr                 += 1;
            eps_mobile_id->guti.mcc  = (**ie_ptr & 0x0F)*100;
            eps_mobile_id->guti.mcc += ((**ie_ptr >> 4) & 0x0F)*10;
            *ie_ptr                 += 1;
            eps_mobile_id->guti.mcc += **ie_ptr & 0x0F;
            if(((**ie_ptr >> 4) & 0x0F) == 0x0F)
            {
                *ie_ptr                 += 1;
                eps_mobile_id->guti.mnc  = (**ie_ptr & 0x0F)*10;
                eps_mobile_id->guti.mnc += (**ie_ptr >> 4) & 0x0F;
                *ie_ptr                 += 1;
            }else{
                eps_mobile_id->guti.mnc  = (**ie_ptr >> 4) & 0x0F;
                *ie_ptr                 += 1;
                eps_mobile_id->guti.mnc += (**ie_ptr & 0x0F)*100;
                eps_mobile_id->guti.mnc += ((**ie_ptr >> 4) & 0x0F)*10;
                *ie_ptr                 += 1;
            }
            eps_mobile_id->guti.mme_group_id  = **ie_ptr << 8;
            *ie_ptr                          += 1;
            eps_mobile_id->guti.mme_group_id |= **ie_ptr;
            *ie_ptr                          += 1;
            eps_mobile_id->guti.mme_code      = **ie_ptr;
            *ie_ptr                          += 1;
            eps_mobile_id->guti.m_tmsi        = **ie_ptr << 24;
            *ie_ptr                          += 1;
            eps_mobile_id->guti.m_tmsi       |= **ie_ptr << 16;
            *ie_ptr                          += 1;
            eps_mobile_id->guti.m_tmsi       |= **ie_ptr << 8;
            *ie_ptr                          += 1;
            eps_mobile_id->guti.m_tmsi       |= **ie_ptr;
            *ie_ptr                          += 1;
        }else{
            if(LIBLTE_MME_EPS_MOBILE_ID_TYPE_IMSI == eps_mobile_id->type_of_id)
            {
                id = eps_mobile_id->imsi;
            }else{
                id = eps_mobile_id->imei;
            }

            id[0]    = **ie_ptr >> 4;
            *ie_ptr += 1;
            for(i=0; i<7; i++)
            {
                id[i*2+1]  = **ie_ptr >> 4;
                id[i*2+2]  = **ie_ptr & 0x0F;
                *ie_ptr   += 1;
            }
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: EPS Network Feature Support

    Description: Indicates whether certain features are supported by
                 the network.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.12A
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: EPS Update Result

    Description: Specifies the result of the associated updating
                 procedure.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.13
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: EPS Update Type

    Description: Specifies the area the updating procedure is
                 associated with.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.14
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: ESM Message Container

    Description: Enables piggybacked transfer of a single ESM message
                 within an EMM message.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.15
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mme_pack_esm_message_container_ie(LIBLTE_BYTE_MSG_STRUCT  *esm_msg,
                                                           uint8                  **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;
    uint32            i;

    if(esm_msg != NULL &&
       ie_ptr  != NULL)
    {
        **ie_ptr  = esm_msg->N_bytes >> 8;
        *ie_ptr  += 1;
        **ie_ptr  = esm_msg->N_bytes & 0xFF;
        *ie_ptr  += 1;
        for(i=0; i<esm_msg->N_bytes; i++)
        {
            **ie_ptr  = esm_msg->msg[i];
            *ie_ptr  += 1;
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mme_unpack_esm_message_container_ie(uint8                  **ie_ptr,
                                                             LIBLTE_BYTE_MSG_STRUCT  *esm_msg)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;
    uint32            i;

    if(ie_ptr  != NULL &&
       esm_msg != NULL)
    {
        esm_msg->N_bytes  = **ie_ptr << 8;
        *ie_ptr          += 1;
        esm_msg->N_bytes |= **ie_ptr;
        *ie_ptr          += 1;
        for(i=0; i<esm_msg->N_bytes; i++)
        {
            esm_msg->msg[i]  = **ie_ptr;
            *ie_ptr         += 1;
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: GPRS Timer

    Description: Specifies GPRS specific timer values.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.16
                        24.008 v10.2.0 Section 10.5.7.3
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: GPRS Timer 2

    Description: Specifies GPRS specific timer values.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.16A
                        24.008 v10.2.0 Section 10.5.7.4
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: GPRS Timer 3

    Description: Specifies GPRS specific timer values.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.16B
                        24.008 v10.2.0 Section 10.5.7.4A
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: Identity Type 2

    Description: Specifies which identity is requested.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.17
                        24.008 v10.2.0 Section 10.5.5.9
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: IMEISV Request

    Description: Indicates that the IMEISV shall be included by the
                 UE in the authentication and ciphering response
                 message.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.18
                        24.008 v10.2.0 Section 10.5.5.10
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: KSI And Sequence Number

    Description: Provides the network with the key set identifier
                 (KSI) value of the current EPS security context and
                 the 5 least significant bits of the NAS COUNT value
                 applicable for the message including this information
                 element.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.19
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: MS Network Capability

    Description: Provides the network with information concerning
                 aspects of the UE related to GPRS.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.20
                        24.008 v10.2.0 Section 10.5.5.12
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mme_pack_ms_network_capability_ie(LIBLTE_MME_MS_NETWORK_CAPABILITY_STRUCT  *ms_network_cap,
                                                           uint8                                   **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ms_network_cap != NULL &&
       ie_ptr         != NULL)
    {
        // FIXME
        **ie_ptr  = 0;
        *ie_ptr  += 1;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mme_unpack_ms_network_capability_ie(uint8                                   **ie_ptr,
                                                             LIBLTE_MME_MS_NETWORK_CAPABILITY_STRUCT  *ms_network_cap)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr         != NULL &&
       ms_network_cap != NULL)
    {
        // FIXME
        *ie_ptr += **ie_ptr + 1;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: NAS Key Set Identifier

    Description: Provides the NAS key set identifier that is allocated
                 by the network.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.21
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mme_pack_nas_key_set_id_ie(LIBLTE_MME_NAS_KEY_SET_ID_STRUCT  *nas_ksi,
                                                    uint8                              bit_offset,
                                                    uint8                            **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(nas_ksi != NULL &&
       ie_ptr  != NULL)
    {
        **ie_ptr |= nas_ksi->tsc_flag << (bit_offset + 3);
        **ie_ptr |= nas_ksi->nas_ksi  << bit_offset;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mme_unpack_nas_key_set_id_ie(uint8                            **ie_ptr,
                                                      uint8                              bit_offset,
                                                      LIBLTE_MME_NAS_KEY_SET_ID_STRUCT  *nas_ksi)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr  != NULL &&
       nas_ksi != NULL)
    {
        nas_ksi->tsc_flag = (LIBLTE_MME_TYPE_OF_SECURITY_CONTEXT_FLAG_ENUM)((**ie_ptr >> (bit_offset + 3)) & 0x01);
        nas_ksi->nas_ksi  = (**ie_ptr >> bit_offset) & 0x07;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: NAS Message Container

    Description: Encapsulates the SMS messages transferred between
                 the UE and the network.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.22
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: NAS Security Algorithms

    Description: Indicates the algorithms to be used for ciphering
                 and integrity protection.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.23
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: Network Name

    Description: Passes a text string to the UE.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.24
                        24.008 v10.2.0 Section 10.5.3.5A
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: Nonce

    Description: Transfers a 32-bit nonce value to support deriving
                 a new mapped EPS security context.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.25
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: Paging Identity

    Description: Indicates the identity used for paging for non-EPS
                 services.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.25A
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: P-TMSI Signature

    Description: Identifies a GMM context of a UE.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.26
                        24.008 v10.2.0 Section 10.5.5.8
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mme_pack_p_tmsi_signature_ie(uint32   p_tmsi_signature,
                                                      uint8  **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        **ie_ptr  = (p_tmsi_signature >> 24) & 0xFF;
        *ie_ptr  += 1;
        **ie_ptr  = (p_tmsi_signature >> 16) & 0xFF;
        *ie_ptr  += 1;
        **ie_ptr  = (p_tmsi_signature >> 8) & 0xFF;
        *ie_ptr  += 1;
        **ie_ptr  = p_tmsi_signature & 0xFF;
        *ie_ptr  += 1;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mme_unpack_p_tmsi_signature_ie(uint8  **ie_ptr,
                                                        uint32  *p_tmsi_signature)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr           != NULL &&
       p_tmsi_signature != NULL)
    {
        *p_tmsi_signature  = (**ie_ptr << 24);
        *ie_ptr           += 1;
        *p_tmsi_signature |= (**ie_ptr << 16);
        *ie_ptr           += 1;
        *p_tmsi_signature |= (**ie_ptr << 8);
        *ie_ptr           += 1;
        *p_tmsi_signature |= **ie_ptr;
        *ie_ptr           += 1;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Service Type

    Description: Specifies the purpose of the service request
                 procedure.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.27
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: Short MAC

    Description: Protects the integrity of a SERVICE REQUEST message.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.28
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: Time Zone

    Description: Encodes the offset between universal time and local
                 time in steps of 15 minutes.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.29
                        24.008 v10.2.0 Section 10.5.3.8
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: Time Zone And Time

    Description: Encodes the offset between universal time and local
                 time in steps of 15 minutes and encodes the universal
                 time at which the IE may have been sent by the
                 network.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.30
                        24.008 v10.2.0 Section 10.5.3.9
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: TMSI Status

    Description: Indicates whether a valid TMSI is available in the
                 UE or not.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.31
                        24.008 v10.2.0 Section 10.5.5.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mme_pack_tmsi_status_ie(LIBLTE_MME_TMSI_STATUS_ENUM   tmsi_status,
                                                 uint8                         bit_offset,
                                                 uint8                       **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        **ie_ptr |= tmsi_status << bit_offset;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mme_unpack_tmsi_status_ie(uint8                       **ie_ptr,
                                                   uint8                         bit_offset,
                                                   LIBLTE_MME_TMSI_STATUS_ENUM  *tmsi_status)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr      != NULL &&
       tmsi_status != NULL)
    {
        *tmsi_status = (LIBLTE_MME_TMSI_STATUS_ENUM)((**ie_ptr >> bit_offset) & 0x01);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Tracking Area Identity

    Description: Provides an unambiguous identification of tracking
                 areas within the area covered by the 3GPP system.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.32
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mme_pack_tracking_area_id_ie(LIBLTE_MME_TRACKING_AREA_ID_STRUCT  *tai,
                                                      uint8                              **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(tai    != NULL &&
       ie_ptr != NULL)
    {
        **ie_ptr  = ((tai->mcc/10) % 10) | ((tai->mcc/100) % 10);
        *ie_ptr  += 1;
        if(tai->mnc > 100)
        {
            **ie_ptr  = 0xF | (tai->mcc % 10);
            *ie_ptr  += 1;
            **ie_ptr  = (tai->mnc % 10) | ((tai->mnc/10) % 10);
            *ie_ptr  += 1;
        }else{
            **ie_ptr  = (tai->mnc % 10) | (tai->mcc % 10);
            *ie_ptr  += 1;
            **ie_ptr  = ((tai->mnc/10) % 10) | ((tai->mnc/100) % 10);
            *ie_ptr  += 1;
        }
        **ie_ptr  = (tai->tac >> 8) & 0xFF;
        *ie_ptr  += 1;
        **ie_ptr  = tai->tac & 0xFF;
        *ie_ptr  += 1;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mme_unpack_tracking_area_id_ie(uint8                              **ie_ptr,
                                                        LIBLTE_MME_TRACKING_AREA_ID_STRUCT  *tai)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL &&
       tai    != NULL)
    {
        tai->mcc  = (**ie_ptr & 0x0F)*100;
        tai->mcc += ((**ie_ptr >> 4) & 0x0F)*10;
        *ie_ptr  += 1;
        tai->mcc += **ie_ptr & 0x0F;
        if(((**ie_ptr >> 4) & 0x0F) == 0x0F)
        {
            *ie_ptr  += 1;
            tai->mnc  = (**ie_ptr & 0x0F)*10;
            tai->mnc += (**ie_ptr >> 4) & 0x0F;
            *ie_ptr  += 1;
        }else{
            tai->mnc  = (**ie_ptr >> 4) & 0x0F;
            *ie_ptr  += 1;
            tai->mnc += (**ie_ptr & 0x0F)*100;
            tai->mnc += ((**ie_ptr >> 4) & 0x0F)*10;
            *ie_ptr  += 1;
        }
        tai->tac  = **ie_ptr << 8;
        *ie_ptr  += 1;
        tai->tac |= **ie_ptr;
        *ie_ptr  += 1;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Tracking Area Identity List

    Description: Transfers a list of tracking areas from the network
                 to the UE.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.33
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: UE Network Capability

    Description: Provides the network with information concerning
                 aspects of the UE related to EPS or interworking with
                 GPRS.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.34
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mme_pack_ue_network_capability_ie(LIBLTE_MME_UE_NETWORK_CAPABILITY_STRUCT  *ue_network_cap,
                                                           uint8                                   **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ue_network_cap != NULL &&
       ie_ptr         != NULL)
    {
        **ie_ptr  = 5;
        *ie_ptr  += 1;
        **ie_ptr  = ue_network_cap->eea0 << 7;
        **ie_ptr |= ue_network_cap->eea1 << 6;
        **ie_ptr |= ue_network_cap->eea2 << 5;
        **ie_ptr |= ue_network_cap->eea3 << 4;
        **ie_ptr |= ue_network_cap->eea4 << 3;
        **ie_ptr |= ue_network_cap->eea5 << 2;
        **ie_ptr |= ue_network_cap->eea6 << 1;
        **ie_ptr |= ue_network_cap->eea7;
        *ie_ptr  += 1;
        **ie_ptr  = ue_network_cap->eia0 << 7;
        **ie_ptr |= ue_network_cap->eia1 << 6;
        **ie_ptr |= ue_network_cap->eia2 << 5;
        **ie_ptr |= ue_network_cap->eia3 << 4;
        **ie_ptr |= ue_network_cap->eia4 << 3;
        **ie_ptr |= ue_network_cap->eia5 << 2;
        **ie_ptr |= ue_network_cap->eia6 << 1;
        **ie_ptr |= ue_network_cap->eia7;
        *ie_ptr  += 1;
        **ie_ptr  = ue_network_cap->uea0 << 7;
        **ie_ptr |= ue_network_cap->uea1 << 6;
        **ie_ptr |= ue_network_cap->uea2 << 5;
        **ie_ptr |= ue_network_cap->uea3 << 4;
        **ie_ptr |= ue_network_cap->uea4 << 3;
        **ie_ptr |= ue_network_cap->uea5 << 2;
        **ie_ptr |= ue_network_cap->uea6 << 1;
        **ie_ptr |= ue_network_cap->uea7;
        *ie_ptr  += 1;
        **ie_ptr  = ue_network_cap->ucs2 << 7;
        **ie_ptr |= ue_network_cap->uia1 << 6;
        **ie_ptr |= ue_network_cap->uia2 << 5;
        **ie_ptr |= ue_network_cap->uia3 << 4;
        **ie_ptr |= ue_network_cap->uia4 << 3;
        **ie_ptr |= ue_network_cap->uia5 << 2;
        **ie_ptr |= ue_network_cap->uia6 << 1;
        **ie_ptr |= ue_network_cap->uia7;
        *ie_ptr  += 1;
        **ie_ptr  = ue_network_cap->lpp << 3;
        **ie_ptr |= ue_network_cap->lcs << 2;
        **ie_ptr |= ue_network_cap->onexsrvcc << 1;
        **ie_ptr |= ue_network_cap->nf;
        *ie_ptr  += 1;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mme_unpack_ue_network_capability_ie(uint8                                   **ie_ptr,
                                                             LIBLTE_MME_UE_NETWORK_CAPABILITY_STRUCT  *ue_network_cap)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;
    uint32            length;

    if(ie_ptr         != NULL &&
       ue_network_cap != NULL)
    {
        length                = **ie_ptr;
        *ie_ptr              += 1;
        ue_network_cap->eea0  = (**ie_ptr >> 7) & 0x01;
        ue_network_cap->eea1  = (**ie_ptr >> 6) & 0x01;
        ue_network_cap->eea2  = (**ie_ptr >> 5) & 0x01;
        ue_network_cap->eea3  = (**ie_ptr >> 4) & 0x01;
        ue_network_cap->eea4  = (**ie_ptr >> 3) & 0x01;
        ue_network_cap->eea5  = (**ie_ptr >> 2) & 0x01;
        ue_network_cap->eea6  = (**ie_ptr >> 1) & 0x01;
        ue_network_cap->eea7  = **ie_ptr & 0x01;
        *ie_ptr              += 1;
        ue_network_cap->eia0  = (**ie_ptr >> 7) & 0x01;
        ue_network_cap->eia1  = (**ie_ptr >> 6) & 0x01;
        ue_network_cap->eia2  = (**ie_ptr >> 5) & 0x01;
        ue_network_cap->eia3  = (**ie_ptr >> 4) & 0x01;
        ue_network_cap->eia4  = (**ie_ptr >> 3) & 0x01;
        ue_network_cap->eia5  = (**ie_ptr >> 2) & 0x01;
        ue_network_cap->eia6  = (**ie_ptr >> 1) & 0x01;
        ue_network_cap->eia7  = **ie_ptr & 0x01;
        *ie_ptr              += 1;
        if(length > 2)
        {
            ue_network_cap->uea0  = (**ie_ptr >> 7) & 0x01;
            ue_network_cap->uea1  = (**ie_ptr >> 6) & 0x01;
            ue_network_cap->uea2  = (**ie_ptr >> 5) & 0x01;
            ue_network_cap->uea3  = (**ie_ptr >> 4) & 0x01;
            ue_network_cap->uea4  = (**ie_ptr >> 3) & 0x01;
            ue_network_cap->uea5  = (**ie_ptr >> 2) & 0x01;
            ue_network_cap->uea6  = (**ie_ptr >> 1) & 0x01;
            ue_network_cap->uea7  = **ie_ptr & 0x01;
            *ie_ptr              += 1;
        }
        if(length > 3)
        {
            ue_network_cap->ucs2  = (**ie_ptr >> 7) & 0x01;
            ue_network_cap->uia1  = (**ie_ptr >> 6) & 0x01;
            ue_network_cap->uia2  = (**ie_ptr >> 5) & 0x01;
            ue_network_cap->uia3  = (**ie_ptr >> 4) & 0x01;
            ue_network_cap->uia4  = (**ie_ptr >> 3) & 0x01;
            ue_network_cap->uia5  = (**ie_ptr >> 2) & 0x01;
            ue_network_cap->uia6  = (**ie_ptr >> 1) & 0x01;
            ue_network_cap->uia7  = **ie_ptr & 0x01;
            *ie_ptr              += 1;
        }
        if(length > 4)
        {
            ue_network_cap->lpp        = (**ie_ptr >> 3) & 0x01;
            ue_network_cap->lcs        = (**ie_ptr >> 2) & 0x01;
            ue_network_cap->onexsrvcc  = (**ie_ptr >> 1) & 0x01;
            ue_network_cap->nf         = **ie_ptr >> 1;
            *ie_ptr                   += 1;
        }
        if(length > 5)
        {
            *ie_ptr += length-5;
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: UE Radio Capability Update Needed

    Description: Indicates whether the MME shall delete the stored
                 UE radio capability information, if any.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.35
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: UE Security Capability

    Description: Indicates which security algorithms are supported by
                 the UE in S1 mode.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.36
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: Emergency Number List

    Description: Encodes emergency number(s) for use within the
                 country (as indicated by MCC) where the IE is
                 received.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.37
                        24.008 v10.2.0 Section 10.5.3.13
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: CLI

    Description: Conveys information about the calling line for a
                 terminated call to a CS fallback capable UE.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.38
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: SS Code

    Description: Conveys information related to a network initiated
                 supplementary service request to a CS fallback
                 capable UE.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.39
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: LCS Indicator

    Description: Indicates that the origin of the message is due to a
                 LCS request and the type of this request to a CS
                 fallback capable UE.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.40
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: LCS Client Identity

    Description: Conveys information related to the client of a LCS
                 request for a CS fallback capable UE.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.41
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: Generic Message Container Type

    Description: Specifies the type of message contained in the
                 generic message container IE.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.42
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: Generic Message Container

    Description: Encapsulates the application message transferred
                 between the UE and the network.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.43
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: Voice Domain Preference and UE's Usage Setting

    Description: Provides the network with the UE's usage setting and
                 the voice domain preference for the E-UTRAN.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.44
                        24.008 v10.2.0 Section 10.5.5.28
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mme_pack_voice_domain_pref_and_ue_usage_setting_ie(LIBLTE_MME_VOICE_DOMAIN_PREF_AND_UE_USAGE_SETTING_STRUCT  *voice_domain_pref_and_ue_usage_setting,
                                                                            uint8                                                    **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(voice_domain_pref_and_ue_usage_setting != NULL &&
       ie_ptr                                 != NULL)
    {
        **ie_ptr  = 1;
        *ie_ptr  += 1;
        **ie_ptr  = voice_domain_pref_and_ue_usage_setting->ue_usage_setting << 2;
        **ie_ptr |= voice_domain_pref_and_ue_usage_setting->voice_domain_pref;
        *ie_ptr  += 1;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mme_unpack_voice_domain_pref_and_ue_usage_setting_ie(uint8                                                    **ie_ptr,
                                                                              LIBLTE_MME_VOICE_DOMAIN_PREF_AND_UE_USAGE_SETTING_STRUCT  *voice_domain_pref_and_ue_usage_setting)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr                                 != NULL &&
       voice_domain_pref_and_ue_usage_setting != NULL)
    {
        *ie_ptr                                                   += 1;
        voice_domain_pref_and_ue_usage_setting->ue_usage_setting   = (LIBLTE_MME_UE_USAGE_SETTING_ENUM)((**ie_ptr >> 2) & 0x01);
        voice_domain_pref_and_ue_usage_setting->voice_domain_pref  = (LIBLTE_MME_VOICE_DOMAIN_PREF_ENUM)(**ie_ptr & 0x03);
        *ie_ptr                                                   += 1;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: GUTI Type

    Description: Indicates whether the GUTI included in the same
                 message in an information element of type EPS
                 mobility identity represents a native GUTI or a
                 mapped GUTI.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.45
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mme_pack_guti_type_ie(LIBLTE_MME_GUTI_TYPE_ENUM   guti_type,
                                               uint8                       bit_offset,
                                               uint8                     **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        **ie_ptr |= guti_type << bit_offset;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mme_unpack_guti_type_ie(uint8                     **ie_ptr,
                                                 uint8                       bit_offset,
                                                 LIBLTE_MME_GUTI_TYPE_ENUM  *guti_type)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr    != NULL &&
       guti_type != NULL)
    {
        *guti_type = (LIBLTE_MME_GUTI_TYPE_ENUM)((**ie_ptr >> bit_offset) & 0x01);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Access Point Name

    Description: Identifies the packet data network to which the GPRS
                 user wishes to connect and notifies the access point
                 of the packet data network that wishes to connect to
                 the UE.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.1
                        24.008 v10.2.0 Section 10.5.6.1
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mme_pack_access_point_name_ie(LIBLTE_MME_ACCESS_POINT_NAME_STRUCT  *apn,
                                                       uint8                               **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(apn    != NULL &&
       ie_ptr != NULL)
    {
        // FIXME
        **ie_ptr  = 0;
        *ie_ptr  += 1;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mme_unpack_access_point_name_ie(uint8                               **ie_ptr,
                                                         LIBLTE_MME_ACCESS_POINT_NAME_STRUCT  *apn)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL &&
       apn    != NULL)
    {
        // FIXME
        *ie_ptr += **ie_ptr + 1;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: APN Aggregate Maximum Bit Rate

    Description: Indicates the initial subscribed APN-AMBR when the
                 UE establishes a PDN connection or indicates the new
                 APN-AMBR if it is changed by the network.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.2
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: Connectivity Type

    Description: Specifies the type of connectivity selected by the
                 network for the PDN connection.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.2A
                        24.008 v10.2.0 Section 10.5.6.19
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: EPS Quality Of Service

    Description: Specifies the QoS parameters for an EPS bearer
                 context.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.3
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: ESM Cause

    Description: Indicates the reason why a session management request
                 is rejected.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.4
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: ESM Information Transfer Flag

    Description: Indicates whether ESM information, i.e. protocol
                 configuration options or APN or both, is to be
                 transferred security protected.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.5
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mme_pack_esm_info_transfer_flag_ie(LIBLTE_MME_ESM_INFO_TRANSFER_FLAG_ENUM   esm_info_transfer_flag,
                                                            uint8                                    bit_offset,
                                                            uint8                                  **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        **ie_ptr |= esm_info_transfer_flag << bit_offset;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mme_unpack_esm_info_transfer_flag_ie(uint8                                  **ie_ptr,
                                                              uint8                                    bit_offset,
                                                              LIBLTE_MME_ESM_INFO_TRANSFER_FLAG_ENUM  *esm_info_transfer_flag)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr                 != NULL &&
       esm_info_transfer_flag != NULL)
    {
        *esm_info_transfer_flag = (LIBLTE_MME_ESM_INFO_TRANSFER_FLAG_ENUM)((**ie_ptr >> bit_offset) & 0x01);

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Linked EPS Bearer Identity

    Description: Identifies the default bearer that is associated
                 with a dedicated EPS bearer or identifies the EPS
                 bearer (default or dedicated) with which one or more
                 packet filters specified in a traffic flow aggregate
                 are associated.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.6
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: LLC Service Access Point Identifier

    Description: Identifies the service access point that is used for
                 the GPRS data transfer at LLC layer.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.7
                        24.008 v10.2.0 Section 10.5.6.9
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: Notification Indicator

    Description: Informs the UE about an event which is relevant for
                 the upper layer using an EPS bearer context or
                 having requested a procedure transaction.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.7A
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: Packet Flow Identifier

    Description: Indicates the packet flow identifier for a packet
                 flow context.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.8
                        24.008 v10.2.0 Section 10.5.6.11
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: PDN Address

    Description: Assigns an IPv4 address to the UE associated with a
                 packet data network and provides the UE with an
                 interface identifier to be used to build the IPv6
                 link local address.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.9
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: PDN Type

    Description: Indicates the IP version capability of the IP stack
                 associated with the UE.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.10
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mme_pack_pdn_type_ie(uint8   pdn_type,
                                              uint8   bit_offset,
                                              uint8 **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        **ie_ptr |= pdn_type << bit_offset;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mme_unpack_pdn_type_ie(uint8 **ie_ptr,
                                                uint8   bit_offset,
                                                uint8  *pdn_type)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr   != NULL &&
       pdn_type != NULL)
    {
        *pdn_type = (**ie_ptr >> bit_offset) & 0x07;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Protocol Configuration Options

    Description: Transfers external network protocol options
                 associated with a PDP context activation and
                 transfers additional (protocol) data (e.g.
                 configuration parameters, error codes or messages/
                 events) associated with an external protocol or an
                 application.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.11
                        24.008 v10.2.0 Section 10.5.6.3
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mme_pack_protocol_config_options_ie(LIBLTE_MME_PROTOCOL_CONFIG_OPTIONS_STRUCT  *protocol_cnfg_opts,
                                                             uint8                                     **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(protocol_cnfg_opts != NULL &&
       ie_ptr             != NULL)
    {
        // FIXME
        **ie_ptr  = 0;
        *ie_ptr  += 1;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mme_unpack_protocol_config_options_ie(uint8                                     **ie_ptr,
                                                               LIBLTE_MME_PROTOCOL_CONFIG_OPTIONS_STRUCT  *protocol_cnfg_opts)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr             != NULL &&
       protocol_cnfg_opts != NULL)
    {
        // FIXME
        *ie_ptr += **ie_ptr + 1;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Quality Of Service

    Description: Specifies the QoS parameters for a PDP context.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.12
                        24.008 v10.2.0 Section 10.5.6.5
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: Radio Priority

    Description: Specifies the priority level the UE shall use at the
                 lower layers for transmission of data related to a
                 PDP context or for mobile originated SMS
                 transmission.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.13
                        24.008 v10.2.0 Section 10.5.7.2
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: Request Type

    Description: Indicates whether the UE requests to establish a new
                 connectivity to a PDN or keep the connection(s) to
                 which it has connected via non-3GPP access.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.14
                        24.008 v10.2.0 Section 10.5.6.17
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mme_pack_request_type_ie(uint8   req_type,
                                                  uint8   bit_offset,
                                                  uint8 **ie_ptr)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr != NULL)
    {
        **ie_ptr |= req_type << bit_offset;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mme_unpack_request_type_ie(uint8 **ie_ptr,
                                                    uint8   bit_offset,
                                                    uint8  *req_type)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(ie_ptr   != NULL &&
       req_type != NULL)
    {
        *req_type = (**ie_ptr >> bit_offset) & 0x07;

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    IE Name: Traffic Flow Aggregate Description

    Description: Specifies the aggregate of one or more packet filters
                 and their related parameters and operations.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.15
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: Traffic Flow Template

    Description: Specifies the TFT parameters and operations for a
                 PDP context.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.16
                        24.008 v10.2.0 Section 10.5.6.12
*********************************************************************/
// FIXME

/*********************************************************************
    IE Name: Transaction Identifier

    Description: Represents the corresponding PDP context in A/Gb
                 mode or Iu mode which is mapped from the EPS bearer
                 context.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.17
                        24.008 v10.2.0 Section 10.5.6.7
*********************************************************************/
// FIXME

/*******************************************************************************
                              MESSAGE FUNCTIONS
*******************************************************************************/

/*********************************************************************
    Message Name: Message Header (Plain NAS Message)

    Description: Message header for plain NAS messages.

    Document Reference: 24.301 v10.2.0 Section 9.1
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mme_parse_msg_header(LIBLTE_BYTE_MSG_STRUCT *msg,
                                              uint8                  *pd,
                                              uint8                  *msg_type)
{
    LIBLTE_ERROR_ENUM err = LIBLTE_ERROR_INVALID_INPUTS;

    if(msg      != NULL &&
       pd       != NULL &&
       msg_type != NULL)
    {
        // Protocol Discriminator
        *pd = msg->msg[0] & 0x0F;

        if(LIBLTE_MME_PD_EPS_SESSION_MANAGEMENT == *pd)
        {
            // Message Type
            *msg_type = msg->msg[2];
        }else{
            // Message Type
            *msg_type = msg->msg[1];
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    Message Name: Attach Accept

    Description: Sent by the network to the UE to indicate that the
                 corresponding attach request has been accepted.

    Document Reference: 24.301 v10.2.0 Section 8.2.1
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Attach Complete

    Description: Sent by the UE to the network in response to an
                 ATTACH ACCEPT message.

    Document Reference: 24.301 v10.2.0 Section 8.2.2
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Attach Reject

    Description: Sent by the network to the UE to indicate that the
                 corresponding attach request has been rejected.

    Document Reference: 24.301 v10.2.0 Section 8.2.3
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Attach Request

    Description: Sent by the UE to the network to perform an attach
                 procedure.

    Document Reference: 24.301 v10.2.0 Section 8.2.4
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mme_pack_attach_request_msg(LIBLTE_MME_ATTACH_REQUEST_MSG_STRUCT *attach_req,
                                                     LIBLTE_BYTE_MSG_STRUCT               *msg)
{
    LIBLTE_ERROR_ENUM  err     = LIBLTE_ERROR_INVALID_INPUTS;
    uint8             *msg_ptr = msg->msg;

    if(attach_req != NULL &&
       msg        != NULL)
    {
        // Protocol Discriminator and Security Header Type
        *msg_ptr = (LIBLTE_MME_SECURITY_HDR_TYPE_PLAIN_NAS << 4) | (LIBLTE_MME_PD_EPS_MOBILITY_MANAGEMENT);
        msg_ptr++;

        // Message Type
        *msg_ptr = LIBLTE_MME_MSG_TYPE_ATTACH_REQUEST;
        msg_ptr++;

        // EPS Attach Type & NAS Key Set Identifier
        *msg_ptr = 0;
        liblte_mme_pack_eps_attach_type_ie(attach_req->eps_type_result, 0, &msg_ptr);
        liblte_mme_pack_nas_key_set_id_ie(&attach_req->nas_ksi, 4, &msg_ptr);
        msg_ptr++;

        // EPS Mobile ID
        liblte_mme_pack_eps_mobile_id_ie(&attach_req->eps_mobile_id, &msg_ptr);

        // UE Network Capability
        liblte_mme_pack_ue_network_capability_ie(&attach_req->ue_network_cap, &msg_ptr);

        // ESM Message Container
        liblte_mme_pack_esm_message_container_ie(&attach_req->esm_msg, &msg_ptr);

        // Old P-TMSI Signature
        if(attach_req->old_p_tmsi_signature_present)
        {
            *msg_ptr = LIBLTE_MME_P_TMSI_SIGNATURE_IEI;
            msg_ptr++;
            liblte_mme_pack_p_tmsi_signature_ie(attach_req->old_p_tmsi_signature, &msg_ptr);
        }

        // Additional GUTI
        if(attach_req->additional_guti_present)
        {
            *msg_ptr = LIBLTE_MME_ADDITIONAL_GUTI_IEI;
            msg_ptr++;
            liblte_mme_pack_eps_mobile_id_ie(&attach_req->additional_guti, &msg_ptr);
        }

        // Last Visited Registered TAI
        if(attach_req->last_visited_registered_tai_present)
        {
            *msg_ptr = LIBLTE_MME_LAST_VISITED_REGISTERED_TAI_IEI;
            msg_ptr++;
            liblte_mme_pack_tracking_area_id_ie(&attach_req->last_visited_registered_tai, &msg_ptr);
        }

        // DRX Parameter
        if(attach_req->drx_param_present)
        {
            *msg_ptr = LIBLTE_MME_DRX_PARAMETER_IEI;
            msg_ptr++;
            liblte_mme_pack_drx_parameter_ie(&attach_req->drx_param, &msg_ptr);
        }

        // MS Network Capability
        if(attach_req->ms_network_cap_present)
        {
            *msg_ptr = LIBLTE_MME_MS_NETWORK_CAPABILITY_IEI;
            msg_ptr++;
            liblte_mme_pack_ms_network_capability_ie(&attach_req->ms_network_cap, &msg_ptr);
        }

        // Old Location Area ID
        if(attach_req->old_lai_present)
        {
            *msg_ptr = LIBLTE_MME_LOCATION_AREA_ID_IEI;
            msg_ptr++;
            liblte_mme_pack_location_area_id_ie(&attach_req->old_lai, &msg_ptr);
        }

        // TMSI Status
        if(attach_req->tmsi_status_present)
        {
            *msg_ptr = LIBLTE_MME_TMSI_STATUS_IEI << 4;
            liblte_mme_pack_tmsi_status_ie(attach_req->tmsi_status, 0, &msg_ptr);
            msg_ptr++;
        }

        // Mobile Station Classmark 2
        if(attach_req->ms_cm2_present)
        {
            *msg_ptr = LIBLTE_MME_MS_CLASSMARK_2_IEI;
            msg_ptr++;
            liblte_mme_pack_mobile_station_classmark_2_ie(&attach_req->ms_cm2, &msg_ptr);
        }

        // Mobile Station Classmark 3
        if(attach_req->ms_cm3_present)
        {
            *msg_ptr = LIBLTE_MME_MS_CLASSMARK_3_IEI;
            msg_ptr++;
            liblte_mme_pack_mobile_station_classmark_3_ie(&attach_req->ms_cm3, &msg_ptr);
        }

        // Supported Codecs
        if(attach_req->supported_codecs_present)
        {
            *msg_ptr = LIBLTE_MME_SUPPORTED_CODEC_LIST_IEI;
            msg_ptr++;
            liblte_mme_pack_supported_codec_list_ie(&attach_req->supported_codecs, &msg_ptr);
        }

        // Additional Update Type
        if(attach_req->additional_update_type_present)
        {
            *msg_ptr = LIBLTE_MME_ADDITIONAL_UPDATE_TYPE_IEI << 4;
            liblte_mme_pack_additional_update_type_ie(attach_req->additional_update_type, 0, &msg_ptr);
            msg_ptr++;
        }

        // Voice Domain Preference and UE's Usage Setting
        if(attach_req->voice_domain_pref_and_ue_usage_setting_present)
        {
            *msg_ptr = LIBLTE_MME_VOICE_DOMAIN_PREF_AND_UE_USAGE_SETTING_IEI;
            msg_ptr++;
            liblte_mme_pack_voice_domain_pref_and_ue_usage_setting_ie(&attach_req->voice_domain_pref_and_ue_usage_setting, &msg_ptr);
        }

        // Device Properties
        if(attach_req->device_properties_present)
        {
            *msg_ptr = LIBLTE_MME_ATTACH_REQUEST_DEVICE_PROPERTIES_IEI << 4;
            liblte_mme_pack_device_properties_ie(attach_req->device_properties, 0, &msg_ptr);
            msg_ptr++;
        }

        // Old GUTI Type
        if(attach_req->old_guti_type_present)
        {
            *msg_ptr = LIBLTE_MME_GUTI_TYPE_IEI << 4;
            liblte_mme_pack_guti_type_ie(attach_req->old_guti_type, 0, &msg_ptr);
            msg_ptr++;
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mme_unpack_attach_request_msg(LIBLTE_BYTE_MSG_STRUCT               *msg,
                                                       LIBLTE_MME_ATTACH_REQUEST_MSG_STRUCT *attach_req)
{
    LIBLTE_ERROR_ENUM  err     = LIBLTE_ERROR_INVALID_INPUTS;
    uint8             *msg_ptr = msg->msg;

    if(msg        != NULL &&
       attach_req != NULL)
    {
        // Skip Protocol Discriminator and Security Header Type
        msg_ptr++;

        // Skip Message Type
        msg_ptr++;

        // EPS Attach Type & NAS Key Set Identifier
        liblte_mme_unpack_eps_attach_type_ie(&msg_ptr, 0, &attach_req->eps_type_result);
        liblte_mme_unpack_nas_key_set_id_ie(&msg_ptr, 4, &attach_req->nas_ksi);
        msg_ptr++;

        // EPS Mobile ID
        liblte_mme_unpack_eps_mobile_id_ie(&msg_ptr, &attach_req->eps_mobile_id);

        // UE Network Capability
        liblte_mme_unpack_ue_network_capability_ie(&msg_ptr, &attach_req->ue_network_cap);

        // ESM Message Container
        liblte_mme_unpack_esm_message_container_ie(&msg_ptr, &attach_req->esm_msg);

        // Old P-TMSI Signature
        if(LIBLTE_MME_P_TMSI_SIGNATURE_IEI == *msg_ptr)
        {
            msg_ptr++;
            liblte_mme_unpack_p_tmsi_signature_ie(&msg_ptr, &attach_req->old_p_tmsi_signature);
            attach_req->old_p_tmsi_signature_present = true;
        }else{
            attach_req->old_p_tmsi_signature_present = false;
        }

        // Additional GUTI
        if(LIBLTE_MME_ADDITIONAL_GUTI_IEI == *msg_ptr)
        {
            msg_ptr++;
            liblte_mme_unpack_eps_mobile_id_ie(&msg_ptr, &attach_req->additional_guti);
            attach_req->additional_guti_present = true;
        }else{
            attach_req->additional_guti_present = false;
        }

        // Last Visited Registered TAI
        if(LIBLTE_MME_LAST_VISITED_REGISTERED_TAI_IEI == *msg_ptr)
        {
            msg_ptr++;
            liblte_mme_unpack_tracking_area_id_ie(&msg_ptr, &attach_req->last_visited_registered_tai);
            attach_req->last_visited_registered_tai_present = true;
        }else{
            attach_req->last_visited_registered_tai_present = false;
        }

        // DRX Parameter
        if(LIBLTE_MME_DRX_PARAMETER_IEI == *msg_ptr)
        {
            msg_ptr++;
            liblte_mme_unpack_drx_parameter_ie(&msg_ptr, &attach_req->drx_param);
            attach_req->drx_param_present = true;
        }else{
            attach_req->drx_param_present = false;
        }

        // MS Network Capability
        if(LIBLTE_MME_MS_NETWORK_CAPABILITY_IEI == *msg_ptr)
        {
            msg_ptr++;
            liblte_mme_unpack_ms_network_capability_ie(&msg_ptr, &attach_req->ms_network_cap);
            attach_req->ms_network_cap_present = true;
        }else{
            attach_req->ms_network_cap_present = false;
        }

        // Old Location Area ID
        if(LIBLTE_MME_LOCATION_AREA_ID_IEI == *msg_ptr)
        {
            msg_ptr++;
            liblte_mme_unpack_location_area_id_ie(&msg_ptr, &attach_req->old_lai);
            attach_req->old_lai_present = true;
        }else{
            attach_req->old_lai_present = false;
        }

        // TMSI Status
        if((LIBLTE_MME_TMSI_STATUS_IEI << 4) == (*msg_ptr & 0xF0))
        {
            liblte_mme_unpack_tmsi_status_ie(&msg_ptr, 0, &attach_req->tmsi_status);
            msg_ptr++;
            attach_req->tmsi_status_present = true;
        }else{
            attach_req->tmsi_status_present = false;
        }

        // Mobile Station Classmark 2
        if(LIBLTE_MME_MS_CLASSMARK_2_IEI == *msg_ptr)
        {
            msg_ptr++;
            liblte_mme_unpack_mobile_station_classmark_2_ie(&msg_ptr, &attach_req->ms_cm2);
            attach_req->ms_cm2_present = true;
        }else{
            attach_req->ms_cm2_present = false;
        }

        // Mobile Station Classmark 3
        if(LIBLTE_MME_MS_CLASSMARK_3_IEI == *msg_ptr)
        {
            msg_ptr++;
            liblte_mme_unpack_mobile_station_classmark_3_ie(&msg_ptr, &attach_req->ms_cm3);
            attach_req->ms_cm3_present = true;
        }else{
            attach_req->ms_cm3_present = false;
        }

        // Supported Codecs
        if(LIBLTE_MME_SUPPORTED_CODEC_LIST_IEI == *msg_ptr)
        {
            msg_ptr++;
            liblte_mme_unpack_supported_codec_list_ie(&msg_ptr, &attach_req->supported_codecs);
            attach_req->supported_codecs_present = true;
        }else{
            attach_req->supported_codecs_present = false;
        }

        // Additional Update Type
        if((LIBLTE_MME_ADDITIONAL_UPDATE_TYPE_IEI << 4) == (*msg_ptr & 0xF0))
        {
            liblte_mme_unpack_additional_update_type_ie(&msg_ptr, 0, &attach_req->additional_update_type);
            msg_ptr++;
            attach_req->additional_update_type_present = true;
        }else{
            attach_req->additional_update_type_present = false;
        }

        // Voice Domain Preference and UE's Usage Setting
        if(LIBLTE_MME_VOICE_DOMAIN_PREF_AND_UE_USAGE_SETTING_IEI == *msg_ptr)
        {
            msg_ptr++;
            liblte_mme_unpack_voice_domain_pref_and_ue_usage_setting_ie(&msg_ptr, &attach_req->voice_domain_pref_and_ue_usage_setting);
            attach_req->voice_domain_pref_and_ue_usage_setting_present = true;
        }else{
            attach_req->voice_domain_pref_and_ue_usage_setting_present = false;
        }

        // Device Properties
        if((LIBLTE_MME_ATTACH_REQUEST_DEVICE_PROPERTIES_IEI << 4) == (*msg_ptr & 0xF0))
        {
            liblte_mme_unpack_device_properties_ie(&msg_ptr, 0, &attach_req->device_properties);
            msg_ptr++;
            attach_req->device_properties_present = true;
        }else{
            attach_req->device_properties_present = false;
        }

        // Old GUTI Type
        if((LIBLTE_MME_GUTI_TYPE_IEI << 4) == (*msg_ptr & 0xF0))
        {
            liblte_mme_unpack_guti_type_ie(&msg_ptr, 0, &attach_req->old_guti_type);
            msg_ptr++;
            attach_req->old_guti_type_present = true;
        }else{
            attach_req->old_guti_type_present = false;
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    Message Name: Authentication Failure

    Description: Sent by the UE to the network to indicate that
                 authentication of the network has failed.

    Document Reference: 24.301 v10.2.0 Section 8.2.5
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Authentication Reject

    Description: Sent by the network to the UE to indicate that the
                 authentication procedure has failed and that the UE
                 shall abort all activities.

    Document Reference: 24.301 v10.2.0 Section 8.2.6
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Authentication Request

    Description: Sent by the network to the UE to initiate
                 authentication of the UE identity.

    Document Reference: 24.301 v10.2.0 Section 8.2.7
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Authentication Response

    Description: Sent by the UE to the network to deliver a calculated
                 authentication response to the network.

    Document Reference: 24.301 v10.2.0 Section 8.2.8
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: CS Service Notification

    Description: Sent by the network when a paging request with CS
                 call indicator was received via SGs for a UE, and a
                 NAS signalling connection is already established for
                 the UE.

    Document Reference: 24.301 v10.2.0 Section 8.2.9
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Detach Accept

    Description: Sent by the network to indicate that the detach
                 procedure has been completed.

    Document Reference: 24.301 v10.2.0 Section 8.2.10
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Detach Request

    Description: Sent by the UE to request the release of an EMM
                 context.

    Document Reference: 24.301 v10.2.0 Section 8.2.11
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Downlink NAS Transport

    Description: Sent by the network to the UE in order to carry an
                 SMS message in encapsulated format.

    Document Reference: 24.301 v10.2.0 Section 8.2.12
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: EMM Information

    Description: Sent by the network at any time during EMM context is
                 established to send certain information to the UE.

    Document Reference: 24.301 v10.2.0 Section 8.2.13
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: EMM Status

    Description: Sent by the UE or by the network at any time to
                 report certain error conditions.

    Document Reference: 24.301 v10.2.0 Section 8.2.14
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Extended Service Request

    Description: Sent by the UE to the network to initiate a CS
                 fallback or 1xCS fallback call or respond to a mobile
                 terminated CS fallback or 1xCS fallback request from
                 the network or to request the establishment of a NAS
                 signalling connection and of the radio and S1 bearers
                 for packet services, if the UE needs to provide
                 additional information that cannot be provided via a
                 SERVICE REQUEST message.

    Document Reference: 24.301 v10.2.0 Section 8.2.15
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: GUTI Reallocation Command

    Description: Sent by the network to the UE to reallocate a GUTI
                 and optionally provide a new TAI list.

    Document Reference: 24.301 v10.2.0 Section 8.2.16
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: GUTI Reallocation Complete

    Description: Sent by the UE to the network to indicate that
                 reallocation of a GUTI has taken place.

    Document Reference: 24.301 v10.2.0 Section 8.2.17
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Identity Request

    Description: Sent by the network to the UE to request the UE to
                 provide the specified identity.

    Document Reference: 24.301 v10.2.0 Section 8.2.18
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Identity Response

    Description: Sent by the UE to the network in response to an
                 IDENTITY REQUEST message and provides the requested
                 identity.

    Document Reference: 24.301 v10.2.0 Section 8.2.19
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Security Mode Command

    Description: Sent by the network to the UE to establish NAS
                 signalling security.

    Document Reference: 24.301 v10.2.0 Section 8.2.20
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Security Mode Complete

    Description: Sent by the UE to the network in response to a
                 SECURITY MODE COMMAND message.

    Document Reference: 24.301 v10.2.0 Section 8.2.21
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Security Mode Reject

    Description: Sent by the UE to the network to indicate that the
                 corresponding security mode command has been
                 rejected.

    Document Reference: 24.301 v10.2.0 Section 8.2.22
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Security Protected NAS Message

    Description: Sent by the UE or the network to transfer a NAS
                 message together with the sequence number and the
                 message authentication code protecting the message.

    Document Reference: 24.301 v10.2.0 Section 8.2.23
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Service Reject

    Description: Sent by the network to the UE in order to reject the
                 service request procedure.

    Document Reference: 24.301 v10.2.0 Section 8.2.24
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Service Request

    Description: Sent by the UE to the network to request the
                 establishment of a NAS signalling connection and of
                 the radio and S1 bearers.

    Document Reference: 24.301 v10.2.0 Section 8.2.25
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Tracking Area Update Accept

    Description: Sent by the network to the UE to provide the UE with
                 EPS mobility management related data in response to
                 a tracking area update request message.

    Document Reference: 24.301 v10.2.0 Section 8.2.26
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Tracking Area Update Complete

    Description: Sent by the UE to the network in response to a
                 tracking area update accept message if a GUTI has
                 been changed or a new TMSI has been assigned.

    Document Reference: 24.301 v10.2.0 Section 8.2.27
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Tracking Area Update Reject

    Description: Sent by the network to the UE in order to reject the
                 tracking area updating procedure.

    Document Reference: 24.301 v10.2.0 Section 8.2.28
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Tracking Area Update Request

    Description: Sent by the UE to the network to initiate a tracking
                 area updating procedure.

    Document Reference: 24.301 v10.2.0 Section 8.2.29
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Uplink NAS Transport

    Description: Sent by the UE to the network in order to carry an
                 SMS message in encapsulated format.

    Document Reference: 24.301 v10.2.0 Section 8.2.30
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Downlink Generic NAS Transport

    Description: Sent by the network to the UE in order to carry an
                 application message in encapsulated format.

    Document Reference: 24.301 v10.2.0 Section 8.2.31
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Uplink Generic NAS Transport

    Description: Sent by the UE to the network in order to carry an
                 application protocol message in encapsulated format.

    Document Reference: 24.301 v10.2.0 Section 8.2.32
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Activate Dedicated EPS Bearer Context Accept

    Description: Sent by the UE to the network to acknowledge
                 activation of a dedicated EPS bearer context
                 associated with the same PDN address(es) and APN as
                 an already active EPS bearer context.

    Document Reference: 24.301 v10.2.0 Section 8.3.1
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Activate Dedicated EPS Bearer Context Reject

    Description: Sent by the UE to the network to reject activation
                 of a dedicated EPS bearer context.

    Document Reference: 24.301 v10.2.0 Section 8.3.2
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Activate Dedicated EPS Bearer Context Request

    Description: Sent by the network to the UE to request activation
                 of a dedicated EPS bearer context associated with
                 the same PDN address(es) and APN as an already
                 active default EPS bearer context.

    Document Reference: 24.301 v10.2.0 Section 8.3.3
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Activate Default EPS Bearer Context Accept

    Description: Sent by the UE to the network to acknowledge
                 activation of a default EPS bearer context.

    Document Reference: 24.301 v10.2.0 Section 8.3.4
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Activate Default EPS Bearer Context Reject

    Description: Sent by the UE to the network to reject activation
                 of a default EPS bearer context.

    Document Reference: 24.301 v10.2.0 Section 8.3.5
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Activate Default EPS Bearer Context Request

    Description: Sent by the network to the UE to request activation
                 of a default EPS bearer context.

    Document Reference: 24.301 v10.2.0 Section 8.3.6
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Bearer Resource Allocation Reject

    Description: Sent by the network to the UE to reject the
                 allocation of a dedicated bearer resource.

    Document Reference: 24.301 v10.2.0 Section 8.3.7
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Bearer Resource Allocation Request

    Description: Sent by the UE to the network to request the
                 allocation of a dedicated bearer resource.

    Document Reference: 24.301 v10.2.0 Section 8.3.8
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Bearer Resource Modification Reject

    Description: Sent by the network to the UE to reject the
                 modification of a dedicated bearer resource.

    Document Reference: 24.301 v10.2.0 Section 8.3.9
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Bearer Resource Modification Request

    Description: Sent by the UE to the network to request the
                 modification of a dedicated bearer resource.

    Document Reference: 24.301 v10.2.0 Section 8.3.10
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Deactivate EPS Bearer Context Accept

    Description: Sent by the UE to acknowledge deactivation of the
                 EPS bearer context requested in the corresponding
                 deactivate EPS bearer context request message.

    Document Reference: 24.301 v10.2.0 Section 8.3.11
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Deactivate EPS Bearer Context Request

    Description: Sent by the network to request deactivation of an
                 EPS bearer context.

    Document Reference: 24.301 v10.2.0 Section 8.3.12
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: ESM Information Request

    Description: Sent by the network to the UE to request the UE to
                 provide ESM information, i.e. protocol configuration
                 options or APN or both.

    Document Reference: 24.301 v10.2.0 Section 8.3.13
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: ESM Information Response

    Description: Sent by the UE to the network in response to an ESM
                 INFORMATION REQUEST message and provides the
                 requested ESM information.

    Document Reference: 24.301 v10.2.0 Section 8.3.14
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: ESM Status

    Description: Sent by the network or the UE to pass information on
                 the status of the indicated EPS bearer context and
                 report certain error conditions.

    Document Reference: 24.301 v10.2.0 Section 8.3.15
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Modify EPS Bearer Context Accept

    Description: Sent by the UE to the network to acknowledge the
                 modification of an active EPS bearer context.

    Document Reference: 24.301 v10.2.0 Section 8.3.16
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Modify EPS Bearer Context Reject

    Description: Sent by the UE or the network to reject a
                 modification of an active EPS bearer context.

    Document Reference: 24.301 v10.2.0 Section 8.3.17
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Modify EPS Bearer Context Request

    Description: Sent by the network to the UE to request modification
                 of an active EPS bearer context.

    Document Reference: 24.301 v10.2.0 Section 8.3.18
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: Notification

    Description: Sent by the network to inform the UE about events
                 which are relevant for the upper layer using an EPS
                 bearer context or having requested a procedure
                 transaction.

    Document Reference: 24.301 v10.2.0 Section 8.3.18A
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: PDN Connectivity Reject

    Description: Sent by the network to the UE to reject establishment
                 of a PDN connection.

    Document Reference: 24.301 v10.2.0 Section 8.3.19
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: PDN Connectivity Request

    Description: Sent by the UE to the network to initiate
                 establishment of a PDN connection.

    Document Reference: 24.301 v10.2.0 Section 8.3.20
*********************************************************************/
LIBLTE_ERROR_ENUM liblte_mme_pack_pdn_connectivity_request_msg(LIBLTE_MME_PDN_CONNECTIVITY_REQUEST_MSG_STRUCT *pdn_con_req,
                                                               LIBLTE_BYTE_MSG_STRUCT                         *msg)
{
    LIBLTE_ERROR_ENUM  err     = LIBLTE_ERROR_INVALID_INPUTS;
    uint8             *msg_ptr = msg->msg;

    if(pdn_con_req != NULL &&
       msg         != NULL)
    {
        // Protocol Discriminator and EPS Bearer ID
        *msg_ptr = (pdn_con_req->eps_bearer_id << 4) | (LIBLTE_MME_PD_EPS_SESSION_MANAGEMENT);
        msg_ptr++;

        // Procedure Transaction ID
        *msg_ptr = pdn_con_req->proc_transaction_id;
        msg_ptr++;

        // Message Type
        *msg_ptr = LIBLTE_MME_MSG_TYPE_PDN_CONNECTIVITY_REQUEST;
        msg_ptr++;

        // Request Type & PDN Type
        *msg_ptr = 0;
        liblte_mme_pack_request_type_ie(pdn_con_req->request_type, 0, &msg_ptr);
        liblte_mme_pack_pdn_type_ie(pdn_con_req->pdn_type, 4, &msg_ptr);
        msg_ptr++;

        // ESM Information Transfer Flag
        if(pdn_con_req->esm_info_transfer_flag_present)
        {
            *msg_ptr = LIBLTE_MME_ESM_INFO_TRANSFER_FLAG_IEI << 4;
            liblte_mme_pack_esm_info_transfer_flag_ie(pdn_con_req->esm_info_transfer_flag, 0, &msg_ptr);
            msg_ptr++;
        }

        // Access Point Name
        if(pdn_con_req->apn_present)
        {
            *msg_ptr = LIBLTE_MME_ACCESS_POINT_NAME_IEI;
            msg_ptr++;
            liblte_mme_pack_access_point_name_ie(&pdn_con_req->apn, &msg_ptr);
        }

        // Protocol Configuration Options
        if(pdn_con_req->protocol_cnfg_opts_present)
        {
            *msg_ptr = LIBLTE_MME_PROTOCOL_CONFIG_OPTIONS_IEI;
            msg_ptr++;
            liblte_mme_pack_protocol_config_options_ie(&pdn_con_req->protocol_cnfg_opts, &msg_ptr);
        }

        // Device Properties
        if(pdn_con_req->device_properties_present)
        {
            *msg_ptr = LIBLTE_MME_PDN_CONNECTIVITY_REQUEST_DEVICE_PROPERTIES_IEI << 4;
            liblte_mme_pack_device_properties_ie(pdn_con_req->device_properties, 0, &msg_ptr);
            msg_ptr++;
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}
LIBLTE_ERROR_ENUM liblte_mme_unpack_pdn_connectivity_request_msg(LIBLTE_BYTE_MSG_STRUCT                         *msg,
                                                                 LIBLTE_MME_PDN_CONNECTIVITY_REQUEST_MSG_STRUCT *pdn_con_req)
{
    LIBLTE_ERROR_ENUM  err     = LIBLTE_ERROR_INVALID_INPUTS;
    uint8             *msg_ptr = msg->msg;

    if(msg         != NULL &&
       pdn_con_req != NULL)
    {
        // EPS Bearer ID
        pdn_con_req->eps_bearer_id = (*msg_ptr >> 4);
        msg_ptr++;

        // Procedure Transaction ID
        pdn_con_req->proc_transaction_id = *msg_ptr;
        msg_ptr++;

        // Skip Message Type
        msg_ptr++;

        // Request Type & PDN Type
        liblte_mme_unpack_request_type_ie(&msg_ptr, 0, &pdn_con_req->request_type);
        liblte_mme_unpack_pdn_type_ie(&msg_ptr, 4, &pdn_con_req->pdn_type);
        msg_ptr++;

        // ESM Information Transfer Flag
        if((LIBLTE_MME_ESM_INFO_TRANSFER_FLAG_IEI << 4) == (*msg_ptr & 0xF0))
        {
            liblte_mme_unpack_esm_info_transfer_flag_ie(&msg_ptr, 0, &pdn_con_req->esm_info_transfer_flag);
            msg_ptr++;
            pdn_con_req->esm_info_transfer_flag_present = true;
        }else{
            pdn_con_req->esm_info_transfer_flag_present = false;
        }

        // Access Point Name
        if(LIBLTE_MME_ACCESS_POINT_NAME_IEI == *msg_ptr)
        {
            msg_ptr++;
            liblte_mme_unpack_access_point_name_ie(&msg_ptr, &pdn_con_req->apn);
            pdn_con_req->apn_present = true;
        }else{
            pdn_con_req->apn_present = false;
        }

        // Protocol Configuration Options
        if(LIBLTE_MME_PROTOCOL_CONFIG_OPTIONS_IEI == *msg_ptr)
        {
            msg_ptr++;
            liblte_mme_unpack_protocol_config_options_ie(&msg_ptr, &pdn_con_req->protocol_cnfg_opts);
            pdn_con_req->protocol_cnfg_opts_present = true;
        }else{
            pdn_con_req->protocol_cnfg_opts_present = false;
        }

        // Device Properties
        if((LIBLTE_MME_PDN_CONNECTIVITY_REQUEST_DEVICE_PROPERTIES_IEI << 4) == (*msg_ptr & 0xF0))
        {
            liblte_mme_unpack_device_properties_ie(&msg_ptr, 0, &pdn_con_req->device_properties);
            msg_ptr++;
            pdn_con_req->device_properties_present = true;
        }else{
            pdn_con_req->device_properties_present = false;
        }

        err = LIBLTE_SUCCESS;
    }

    return(err);
}

/*********************************************************************
    Message Name: PDN Disconnect Reject

    Description: Sent by the network to the UE to reject release of a
                 PDN connection.

    Document Reference: 24.301 v10.2.0 Section 8.3.21
*********************************************************************/
// FIXME

/*********************************************************************
    Message Name: PDN Disconnect Request

    Description: Sent by the UE to the network to initiate release of
                 a PDN connection.

    Document Reference: 24.301 v10.2.0 Section 8.3.22
*********************************************************************/
// FIXME

/*******************************************************************************
                              LOCAL FUNCTIONS
*******************************************************************************/
