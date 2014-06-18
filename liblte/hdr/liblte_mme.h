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

    File: liblte_mme.h

    Description: Contains all the definitions for the LTE Mobility Management
                 Entity library.

    Revision History
    ----------    -------------    --------------------------------------------
    06/15/2014    Ben Wojtowicz    Created file.

*******************************************************************************/

#ifndef __LIBLTE_MME_H__
#define __LIBLTE_MME_H__

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "liblte_common.h"

/*******************************************************************************
                              DEFINES
*******************************************************************************/


/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/


/*******************************************************************************
                              INFORMATION ELEMENT DECLARATIONS
*******************************************************************************/

/*********************************************************************
    IE Name: Additional Information

    Description: Provides additional information to upper layers in
                 relation to the generic NAS message transport
                 mechanism.

    Document Reference: 24.301 v10.2.0 Section 9.9.2.0
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Device Properties

    Description: Indicates if the UE is configured for NAS signalling
                 low priority.

    Document Reference: 24.301 v10.2.0 Section 9.9.2.0A
                        24.008 v10.2.0 Section 10.5.7.8
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_MME_DEVICE_PROPERTIES_NOT_CONFIGURED_FOR_LOW_PRIORITY = 0,
    LIBLTE_MME_DEVICE_PROPERTIES_CONFIGURED_FOR_LOW_PRIORITY,
    LIBLTE_MME_DEVICE_PROPERTIES_N_ITEMS,
}LIBLTE_MME_DEVICE_PROPERTIES_ENUM;
static const char liblte_mme_device_properties_text[LIBLTE_MME_DEVICE_PROPERTIES_N_ITEMS][50] = {"Not configured for low priority",
                                                                                                 "Configured for low priority"};
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_mme_pack_device_properties_ie(LIBLTE_MME_DEVICE_PROPERTIES_ENUM   device_props,
                                                       uint8                               bit_offset,
                                                       uint8                             **ie_ptr);
LIBLTE_ERROR_ENUM liblte_mme_unpack_device_properties_ie(uint8                             **ie_ptr,
                                                         uint8                               bit_offset,
                                                         LIBLTE_MME_DEVICE_PROPERTIES_ENUM  *device_props);

/*********************************************************************
    IE Name: EPS Bearer Context Status

    Description: Indicates the state of each EPS bearer context that
                 can be identified by an EPS bearer identity.

    Document Reference: 24.301 v10.2.0 Section 9.9.2.1
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Location Area Identification

    Description: Provides an unambiguous identification of location
                 areas within the area covered by the 3GPP system.

    Document Reference: 24.301 v10.2.0 Section 9.9.2.2
                        24.008 v10.2.0 Section 10.5.1.3
*********************************************************************/
// Defines
// Enums
// Structs
typedef struct{
    uint16 mcc;
    uint16 mnc;
    uint16 lac;
}LIBLTE_MME_LOCATION_AREA_ID_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_mme_pack_location_area_id_ie(LIBLTE_MME_LOCATION_AREA_ID_STRUCT  *lai,
                                                      uint8                              **ie_ptr);
LIBLTE_ERROR_ENUM liblte_mme_unpack_location_area_id_ie(uint8                              **ie_ptr,
                                                        LIBLTE_MME_LOCATION_AREA_ID_STRUCT  *lai);

/*********************************************************************
    IE Name: Mobile Identity

    Description: Provides either the IMSI, TMSI/P-TMSI/M-TMSI, IMEI,
                 IMEISV, or TMGI, associated with the optional MBMS
                 session identity.

    Document Reference: 24.301 v10.2.0 Section 9.9.2.3
                        24.008 v10.2.0 Section 10.5.1.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Mobile Station Classmark 2

    Description: Provides the network with information concerning
                 aspects of both high and low priority of the UE.

    Document Reference: 24.301 v10.2.0 Section 9.9.2.4
                        24.008 v10.2.0 Section 10.5.1.6
*********************************************************************/
// Defines
// Enums
// Structs
typedef struct{
    // FIXME
}LIBLTE_MME_MOBILE_STATION_CLASSMARK_2_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_mme_pack_mobile_station_classmark_2_ie(LIBLTE_MME_MOBILE_STATION_CLASSMARK_2_STRUCT  *ms_cm2,
                                                                uint8                                        **ie_ptr);
LIBLTE_ERROR_ENUM liblte_mme_unpack_mobile_station_classmark_2_ie(uint8                                        **ie_ptr,
                                                                  LIBLTE_MME_MOBILE_STATION_CLASSMARK_2_STRUCT  *ms_cm2);

/*********************************************************************
    IE Name: Mobile Station Classmark 3

    Description: Provides the network with information concerning
                 aspects of the UE.

    Document Reference: 24.301 v10.2.0 Section 9.9.2.5
                        24.008 v10.2.0 Section 10.5.1.7
*********************************************************************/
// Defines
// Enums
// Structs
typedef struct{
    // FIXME
}LIBLTE_MME_MOBILE_STATION_CLASSMARK_3_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_mme_pack_mobile_station_classmark_3_ie(LIBLTE_MME_MOBILE_STATION_CLASSMARK_3_STRUCT  *ms_cm3,
                                                                uint8                                        **ie_ptr);
LIBLTE_ERROR_ENUM liblte_mme_unpack_mobile_station_classmark_3_ie(uint8                                        **ie_ptr,
                                                                  LIBLTE_MME_MOBILE_STATION_CLASSMARK_3_STRUCT  *ms_cm3);

/*********************************************************************
    IE Name: NAS Security Parameters From E-UTRA

    Description: Provides the UE with information that enables the UE
                 to create a mapped UMTS security context.

    Document Reference: 24.301 v10.2.0 Section 9.9.2.6
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: NAS Security Parameters To E-UTRA

    Description: Provides the UE with parameters that enables the UE
                 to create a mapped EPS security context and take
                 this context into use after inter-system handover to
                 S1 mode.

    Document Reference: 24.301 v10.2.0 Section 9.9.2.7
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: PLMN List

    Description: Provides a list of PLMN codes to the UE.

    Document Reference: 24.301 v10.2.0 Section 9.9.2.8
                        24.008 v10.2.0 Section 10.5.1.13
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Spare Half Octet

    Description: Used in the description of EMM and ESM messages when
                 an odd number of half octet type 1 information
                 elements are used.

    Document Reference: 24.301 v10.2.0 Section 9.9.2.9
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Supported Codec List

    Description: Provides the network with information about the
                 speech codecs supported by the UE.

    Document Reference: 24.301 v10.2.0 Section 9.9.2.10
                        24.008 v10.2.0 Section 10.5.4.32
*********************************************************************/
// Defines
// Enums
// Structs
typedef struct{
    // FIXME
}LIBLTE_MME_SUPPORTED_CODEC_LIST_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_mme_pack_supported_codec_list_ie(LIBLTE_MME_SUPPORTED_CODEC_LIST_STRUCT  *supported_codec_list,
                                                          uint8                                  **ie_ptr);
LIBLTE_ERROR_ENUM liblte_mme_unpack_supported_codec_list_ie(uint8                                  **ie_ptr,
                                                            LIBLTE_MME_SUPPORTED_CODEC_LIST_STRUCT  *supported_codec_list);

/*********************************************************************
    IE Name: Additional Update Result

    Description: Provides additional information about the result of
                 a combined attached procedure or a combined tracking
                 area updating procedure.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.0A
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Additional Update Type

    Description: Provides additional information about the type of
                 request for a combined attach or a combined tracking
                 area updating procedure.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.0B
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_MME_ADDITIONAL_UPDATE_TYPE_NO_ADDITIONAL_INFO = 0,
    LIBLTE_MME_ADDITIONAL_UPDATE_TYPE_SMS_ONLY,
    LIBLTE_MME_ADDITIONAL_UPDATE_TYPE_N_ITEMS,
}LIBLTE_MME_ADDITIONAL_UPDATE_TYPE_ENUM;
static const char liblte_mme_additional_update_type_text[LIBLTE_MME_ADDITIONAL_UPDATE_TYPE_N_ITEMS][20] = {"No additional info",
                                                                                                           "SMS Only"};
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_mme_pack_additional_update_type_ie(LIBLTE_MME_ADDITIONAL_UPDATE_TYPE_ENUM   aut,
                                                            uint8                                    bit_offset,
                                                            uint8                                  **ie_ptr);
LIBLTE_ERROR_ENUM liblte_mme_unpack_additional_update_type_ie(uint8                                  **ie_ptr,
                                                              uint8                                    bit_offset,
                                                              LIBLTE_MME_ADDITIONAL_UPDATE_TYPE_ENUM  *aut);

/*********************************************************************
    IE Name: Authentication Failure Parameter

    Description: Provides the network with the necessary information
                 to begin a re-authentication procedure in the case
                 of a 'Synch failure', following a UMTS or EPS
                 authentication challenge.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.1
                        24.008 v10.2.0 Section 10.5.3.2.2
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Authentication Parameter AUTN

    Description: Provides the UE with a means of authenticating the
                 network.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.2
                        24.008 v10.2.0 Section 10.5.3.1.1
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
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
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Authentication Response Parameter

    Description: Provides the network with the authentication
                 response calculated in the USIM.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Ciphering Key Sequence Number

    Description: Makes it possible for the network to identify the
                 ciphering key Kc which is stored in the UE without
                 invoking the authentication procedure.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.4A
                        24.008 v10.2.0 Section 10.5.1.2
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: CSFB Response

    Description: Indicates whether the UE accepts or rejects a paging
                 for CS fallback.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.5
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Daylight Savings Time

    Description: Encodes the daylight savings time in steps of 1 hour.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.6
                        24.008 v10.2.0 Section 10.5.3.12
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Detach Type

    Description: Indicates the type of detach.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.7
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: DRX Parameter

    Description: Indicates whether the UE uses DRX mode or not.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.8
                        24.008 v10.2.0 Section 10.5.5.6
*********************************************************************/
// Defines
// Enums
// Structs
typedef struct{
    // FIXME
}LIBLTE_MME_DRX_PARAMETER_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_mme_pack_drx_parameter_ie(LIBLTE_MME_DRX_PARAMETER_STRUCT  *drx_param,
                                                   uint8                           **ie_ptr);
LIBLTE_ERROR_ENUM liblte_mme_unpack_drx_parameter_ie(uint8                           **ie_ptr,
                                                     LIBLTE_MME_DRX_PARAMETER_STRUCT  *drx_param);

/*********************************************************************
    IE Name: EMM Cause

    Description: Indicates the reason why an EMM request from the UE
                 is rejected by the network.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.9
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: EPS Attach Result

    Description: Specifies the result of an attach procedure.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.10
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: EPS Attach Type

    Description: Indicates the type of the requested attach.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.11
*********************************************************************/
// Defines
#define LIBLTE_MME_EPS_ATTACH_TYPE_EPS_ATTACH               0x1
#define LIBLTE_MME_EPS_ATTACH_TYPE_COMBINED_EPS_IMSI_ATTACH 0x2
#define LIBLTE_MME_EPS_ATTACH_TYPE_EPS_EMERGENCY_ATTACH     0x6
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_mme_pack_eps_attach_type_ie(uint8   attach_type,
                                                     uint8   bit_offset,
                                                     uint8 **ie_ptr);
LIBLTE_ERROR_ENUM liblte_mme_unpack_eps_attach_type_ie(uint8 **ie_ptr,
                                                       uint8   bit_offset,
                                                       uint8  *attach_type);

/*********************************************************************
    IE Name: EPS Mobile Identity

    Description: Provides either the IMSI, the GUTI, or the IMEI.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.12
*********************************************************************/
// Defines
#define LIBLTE_MME_EPS_MOBILE_ID_TYPE_IMSI 0x1
#define LIBLTE_MME_EPS_MOBILE_ID_TYPE_GUTI 0x6
#define LIBLTE_MME_EPS_MOBILE_ID_TYPE_IMEI 0x3
// Enums
// Structs
typedef struct{
    uint32 m_tmsi;
    uint16 mcc;
    uint16 mnc;
    uint16 mme_group_id;
    uint8  mme_code;
}LIBLTE_MME_EPS_MOBILE_ID_GUTI_STRUCT;
typedef struct{
    LIBLTE_MME_EPS_MOBILE_ID_GUTI_STRUCT guti;
    uint8                                type_of_id;
    uint8                                imsi[15];
    uint8                                imei[15];
}LIBLTE_MME_EPS_MOBILE_ID_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_mme_pack_eps_mobile_id_ie(LIBLTE_MME_EPS_MOBILE_ID_STRUCT  *eps_mobile_id,
                                                   uint8                           **ie_ptr);
LIBLTE_ERROR_ENUM liblte_mme_unpack_eps_mobile_id_ie(uint8                           **ie_ptr,
                                                     LIBLTE_MME_EPS_MOBILE_ID_STRUCT  *eps_mobile_id);

/*********************************************************************
    IE Name: EPS Network Feature Support

    Description: Indicates whether certain features are supported by
                 the network.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.12A
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: EPS Update Result

    Description: Specifies the result of the associated updating
                 procedure.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.13
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: EPS Update Type

    Description: Specifies the area the updating procedure is
                 associated with.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.14
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: ESM Message Container

    Description: Enables piggybacked transfer of a single ESM message
                 within an EMM message.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.15
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_mme_pack_esm_message_container_ie(LIBLTE_BYTE_MSG_STRUCT  *esm_msg,
                                                           uint8                  **ie_ptr);
LIBLTE_ERROR_ENUM liblte_mme_unpack_esm_message_container_ie(uint8                  **ie_ptr,
                                                             LIBLTE_BYTE_MSG_STRUCT  *esm_msg);

/*********************************************************************
    IE Name: GPRS Timer

    Description: Specifies GPRS specific timer values.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.16
                        24.008 v10.2.0 Section 10.5.7.3
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: GPRS Timer 2

    Description: Specifies GPRS specific timer values.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.16A
                        24.008 v10.2.0 Section 10.5.7.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: GPRS Timer 3

    Description: Specifies GPRS specific timer values.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.16B
                        24.008 v10.2.0 Section 10.5.7.4A
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Identity Type 2

    Description: Specifies which identity is requested.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.17
                        24.008 v10.2.0 Section 10.5.5.9
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: IMEISV Request

    Description: Indicates that the IMEISV shall be included by the
                 UE in the authentication and ciphering response
                 message.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.18
                        24.008 v10.2.0 Section 10.5.5.10
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
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
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: MS Network Capability

    Description: Provides the network with information concerning
                 aspects of the UE related to GPRS.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.20
                        24.008 v10.2.0 Section 10.5.5.12
*********************************************************************/
// Defines
// Enums
// Structs
typedef struct{
    // FIXME
}LIBLTE_MME_MS_NETWORK_CAPABILITY_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_mme_pack_ms_network_capability_ie(LIBLTE_MME_MS_NETWORK_CAPABILITY_STRUCT  *ms_network_cap,
                                                           uint8                                   **ie_ptr);
LIBLTE_ERROR_ENUM liblte_mme_unpack_ms_network_capability_ie(uint8                                   **ie_ptr,
                                                             LIBLTE_MME_MS_NETWORK_CAPABILITY_STRUCT  *ms_network_cap);

/*********************************************************************
    IE Name: NAS Key Set Identifier

    Description: Provides the NAS key set identifier that is allocated
                 by the network.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.21
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_MME_TYPE_OF_SECURITY_CONTEXT_FLAG_NATIVE = 0,
    LIBLTE_MME_TYPE_OF_SECURITY_CONTEXT_FLAG_MAPPED,
    LIBLTE_MME_TYPE_OF_SECURITY_CONTEXT_FLAG_N_ITEMS,
}LIBLTE_MME_TYPE_OF_SECURITY_CONTEXT_FLAG_ENUM;
static const char liblte_mme_type_of_security_context_flag_text[LIBLTE_MME_TYPE_OF_SECURITY_CONTEXT_FLAG_N_ITEMS][20] = {"Native",
                                                                                                                         "Mapped"};
// Structs
typedef struct{
    LIBLTE_MME_TYPE_OF_SECURITY_CONTEXT_FLAG_ENUM tsc_flag;
    uint8                                         nas_ksi;
}LIBLTE_MME_NAS_KEY_SET_ID_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_mme_pack_nas_key_set_id_ie(LIBLTE_MME_NAS_KEY_SET_ID_STRUCT  *nas_ksi,
                                                    uint8                              bit_offset,
                                                    uint8                            **ie_ptr);
LIBLTE_ERROR_ENUM liblte_mme_unpack_nas_key_set_id_ie(uint8                            **ie_ptr,
                                                      uint8                              bit_offset,
                                                      LIBLTE_MME_NAS_KEY_SET_ID_STRUCT  *nas_ksi);

/*********************************************************************
    IE Name: NAS Message Container

    Description: Encapsulates the SMS messages transferred between
                 the UE and the network.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.22
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: NAS Security Algorithms

    Description: Indicates the algorithms to be used for ciphering
                 and integrity protection.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.23
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Network Name

    Description: Passes a text string to the UE.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.24
                        24.008 v10.2.0 Section 10.5.3.5A
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Nonce

    Description: Transfers a 32-bit nonce value to support deriving
                 a new mapped EPS security context.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.25
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Paging Identity

    Description: Indicates the identity used for paging for non-EPS
                 services.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.25A
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: P-TMSI Signature

    Description: Identifies a GMM context of a UE.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.26
                        24.008 v10.2.0 Section 10.5.5.8
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_mme_pack_p_tmsi_signature_ie(uint32   p_tmsi_signature,
                                                      uint8  **ie_ptr);
LIBLTE_ERROR_ENUM liblte_mme_unpack_p_tmsi_signature_ie(uint8  **ie_ptr,
                                                        uint32  *p_tmsi_signature);

/*********************************************************************
    IE Name: Service Type

    Description: Specifies the purpose of the service request
                 procedure.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.27
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Short MAC

    Description: Protects the integrity of a SERVICE REQUEST message.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.28
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Time Zone

    Description: Encodes the offset between universal time and local
                 time in steps of 15 minutes.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.29
                        24.008 v10.2.0 Section 10.5.3.8
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
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
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: TMSI Status

    Description: Indicates whether a valid TMSI is available in the
                 UE or not.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.31
                        24.008 v10.2.0 Section 10.5.5.4
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_MME_TMSI_STATUS_NO_VALID_TMSI = 0,
    LIBLTE_MME_TMSI_STATUS_VALID_TMSI,
    LIBLTE_MME_TMSI_STATUS_N_ITEMS,
}LIBLTE_MME_TMSI_STATUS_ENUM;
static const char liblte_mme_tmsi_status_text[LIBLTE_MME_TMSI_STATUS_N_ITEMS][20] = {"No valid TMSI",
                                                                                     "Valid TMSI"};
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_mme_pack_tmsi_status_ie(LIBLTE_MME_TMSI_STATUS_ENUM   tmsi_status,
                                                 uint8                         bit_offset,
                                                 uint8                       **ie_ptr);
LIBLTE_ERROR_ENUM liblte_mme_unpack_tmsi_status_ie(uint8                       **ie_ptr,
                                                   uint8                         bit_offset,
                                                   LIBLTE_MME_TMSI_STATUS_ENUM  *tmsi_status);

/*********************************************************************
    IE Name: Tracking Area Identity

    Description: Provides an unambiguous identification of tracking
                 areas within the area covered by the 3GPP system.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.32
*********************************************************************/
// Defines
// Enums
// Structs
typedef struct{
    uint16 mcc;
    uint16 mnc;
    uint16 tac;
}LIBLTE_MME_TRACKING_AREA_ID_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_mme_pack_tracking_area_id_ie(LIBLTE_MME_TRACKING_AREA_ID_STRUCT  *tai,
                                                      uint8                              **ie_ptr);
LIBLTE_ERROR_ENUM liblte_mme_unpack_tracking_area_id_ie(uint8                              **ie_ptr,
                                                        LIBLTE_MME_TRACKING_AREA_ID_STRUCT  *tai);

/*********************************************************************
    IE Name: Tracking Area Identity List

    Description: Transfers a list of tracking areas from the network
                 to the UE.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.33
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: UE Network Capability

    Description: Provides the network with information concerning
                 aspects of the UE related to EPS or interworking with
                 GPRS.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.34
*********************************************************************/
// Defines
// Enums
// Structs
typedef struct{
    bool eea0;
    bool eea1;
    bool eea2;
    bool eea3;
    bool eea4;
    bool eea5;
    bool eea6;
    bool eea7;
    bool eia0;
    bool eia1;
    bool eia2;
    bool eia3;
    bool eia4;
    bool eia5;
    bool eia6;
    bool eia7;
    bool uea0;
    bool uea1;
    bool uea2;
    bool uea3;
    bool uea4;
    bool uea5;
    bool uea6;
    bool uea7;
    bool ucs2;
    bool uia1;
    bool uia2;
    bool uia3;
    bool uia4;
    bool uia5;
    bool uia6;
    bool uia7;
    bool lpp;
    bool lcs;
    bool onexsrvcc;
    bool nf;
}LIBLTE_MME_UE_NETWORK_CAPABILITY_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_mme_pack_ue_network_capability_ie(LIBLTE_MME_UE_NETWORK_CAPABILITY_STRUCT  *ue_network_cap,
                                                           uint8                                   **ie_ptr);
LIBLTE_ERROR_ENUM liblte_mme_unpack_ue_network_capability_ie(uint8                                   **ie_ptr,
                                                             LIBLTE_MME_UE_NETWORK_CAPABILITY_STRUCT  *ue_network_cap);

/*********************************************************************
    IE Name: UE Radio Capability Update Needed

    Description: Indicates whether the MME shall delete the stored
                 UE radio capability information, if any.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.35
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: UE Security Capability

    Description: Indicates which security algorithms are supported by
                 the UE in S1 mode.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.36
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Emergency Number List

    Description: Encodes emergency number(s) for use within the
                 country (as indicated by MCC) where the IE is
                 received.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.37
                        24.008 v10.2.0 Section 10.5.3.13
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: CLI

    Description: Conveys information about the calling line for a
                 terminated call to a CS fallback capable UE.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.38
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: SS Code

    Description: Conveys information related to a network initiated
                 supplementary service request to a CS fallback
                 capable UE.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.39
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: LCS Indicator

    Description: Indicates that the origin of the message is due to a
                 LCS request and the type of this request to a CS
                 fallback capable UE.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.40
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: LCS Client Identity

    Description: Conveys information related to the client of a LCS
                 request for a CS fallback capable UE.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.41
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Generic Message Container Type

    Description: Specifies the type of message contained in the
                 generic message container IE.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.42
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Generic Message Container

    Description: Encapsulates the application message transferred
                 between the UE and the network.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.43
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Voice Domain Preference and UE's Usage Setting

    Description: Provides the network with the UE's usage setting and
                 the voice domain preference for the E-UTRAN.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.44
                        24.008 v10.2.0 Section 10.5.5.28
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_MME_UE_USAGE_SETTING_VOICE_CENTRIC = 0,
    LIBLTE_MME_UE_USAGE_SETTING_DATA_CENTRIC,
    LIBLTE_MME_UE_USAGE_SETTING_N_ITEMS,
}LIBLTE_MME_UE_USAGE_SETTING_ENUM;
static const char liblte_mme_ue_usage_setting_text[LIBLTE_MME_UE_USAGE_SETTING_N_ITEMS][20] = {"Voice Centric",
                                                                                               "Data Centric"};
typedef enum{
    LIBLTE_MME_VOICE_DOMAIN_PREF_CS_ONLY = 0,
    LIBLTE_MME_VOICE_DOMAIN_PREF_PS_ONLY,
    LIBLTE_MME_VOICE_DOMAIN_PREF_CS_PREFFERED,
    LIBLTE_MME_VOICE_DOMAIN_PREF_PS_PREFFERED,
    LIBLTE_MME_VOICE_DOMAIN_PREF_N_ITEMS,
}LIBLTE_MME_VOICE_DOMAIN_PREF_ENUM;
static const char liblte_mme_voice_domain_pref_text[LIBLTE_MME_VOICE_DOMAIN_PREF_N_ITEMS][20] = {"CS Only",
                                                                                                 "PS Only",
                                                                                                 "CS Preffered",
                                                                                                 "PS Preffered"};
// Structs
typedef struct{
    LIBLTE_MME_UE_USAGE_SETTING_ENUM  ue_usage_setting;
    LIBLTE_MME_VOICE_DOMAIN_PREF_ENUM voice_domain_pref;
}LIBLTE_MME_VOICE_DOMAIN_PREF_AND_UE_USAGE_SETTING_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_mme_pack_voice_domain_pref_and_ue_usage_setting_ie(LIBLTE_MME_VOICE_DOMAIN_PREF_AND_UE_USAGE_SETTING_STRUCT  *voice_domain_pref_and_ue_usage_setting,
                                                                            uint8                                                    **ie_ptr);
LIBLTE_ERROR_ENUM liblte_mme_unpack_voice_domain_pref_and_ue_usage_setting_ie(uint8                                                    **ie_ptr,
                                                                              LIBLTE_MME_VOICE_DOMAIN_PREF_AND_UE_USAGE_SETTING_STRUCT  *voice_domain_pref_and_ue_usage_setting);

/*********************************************************************
    IE Name: GUTI Type

    Description: Indicates whether the GUTI included in the same
                 message in an information element of type EPS
                 mobility identity represents a native GUTI or a
                 mapped GUTI.

    Document Reference: 24.301 v10.2.0 Section 9.9.3.45
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_MME_GUTI_TYPE_NATIVE = 0,
    LIBLTE_MME_GUTI_TYPE_MAPPED,
    LIBLTE_MME_GUTI_TYPE_N_ITEMS,
}LIBLTE_MME_GUTI_TYPE_ENUM;
static const char liblte_mme_guti_type_text[LIBLTE_MME_GUTI_TYPE_N_ITEMS][20] = {"Native",
                                                                                 "Mapped"};
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_mme_pack_guti_type_ie(LIBLTE_MME_GUTI_TYPE_ENUM   guti_type,
                                               uint8                       bit_offset,
                                               uint8                     **ie_ptr);
LIBLTE_ERROR_ENUM liblte_mme_unpack_guti_type_ie(uint8                     **ie_ptr,
                                                 uint8                       bit_offset,
                                                 LIBLTE_MME_GUTI_TYPE_ENUM  *guti_type);

/*********************************************************************
    IE Name: Access Point Name

    Description: Identifies the packet data network to which the GPRS
                 user wishes to connect and notifies the access point
                 of the packet data network that wishes to connect to
                 the UE.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.1
                        24.008 v10.2.0 Section 10.5.6.1
*********************************************************************/
// Defines
// Enums
// Structs
typedef struct{
    // FIXME
}LIBLTE_MME_ACCESS_POINT_NAME_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_mme_pack_access_point_name_ie(LIBLTE_MME_ACCESS_POINT_NAME_STRUCT  *apn,
                                                       uint8                               **ie_ptr);
LIBLTE_ERROR_ENUM liblte_mme_unpack_access_point_name_ie(uint8                               **ie_ptr,
                                                         LIBLTE_MME_ACCESS_POINT_NAME_STRUCT  *apn);

/*********************************************************************
    IE Name: APN Aggregate Maximum Bit Rate

    Description: Indicates the initial subscribed APN-AMBR when the
                 UE establishes a PDN connection or indicates the new
                 APN-AMBR if it is changed by the network.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.2
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Connectivity Type

    Description: Specifies the type of connectivity selected by the
                 network for the PDN connection.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.2A
                        24.008 v10.2.0 Section 10.5.6.19
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: EPS Quality Of Service

    Description: Specifies the QoS parameters for an EPS bearer
                 context.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.3
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: ESM Cause

    Description: Indicates the reason why a session management request
                 is rejected.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: ESM Information Transfer Flag

    Description: Indicates whether ESM information, i.e. protocol
                 configuration options or APN or both, is to be
                 transferred security protected.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.5
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_MME_ESM_INFO_TRANSFER_FLAG_NOT_REQUIRED = 0,
    LIBLTE_MME_ESM_INFO_TRANSFER_FLAG_REQUIRED,
    LIBLTE_MME_ESM_INFO_TRANSFER_FLAG_N_ITEMS,
}LIBLTE_MME_ESM_INFO_TRANSFER_FLAG_ENUM;
static const char liblte_mme_esm_info_transfer_flag_text[LIBLTE_MME_ESM_INFO_TRANSFER_FLAG_N_ITEMS][20] = {"Not Required",
                                                                                                           "Required"};
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_mme_pack_esm_info_transfer_flag_ie(LIBLTE_MME_ESM_INFO_TRANSFER_FLAG_ENUM   esm_info_transfer_flag,
                                                            uint8                                    bit_offset,
                                                            uint8                                  **ie_ptr);
LIBLTE_ERROR_ENUM liblte_mme_unpack_esm_info_transfer_flag_ie(uint8                                  **ie_ptr,
                                                              uint8                                    bit_offset,
                                                              LIBLTE_MME_ESM_INFO_TRANSFER_FLAG_ENUM  *esm_info_transfer_flag);

/*********************************************************************
    IE Name: Linked EPS Bearer Identity

    Description: Identifies the default bearer that is associated
                 with a dedicated EPS bearer or identifies the EPS
                 bearer (default or dedicated) with which one or more
                 packet filters specified in a traffic flow aggregate
                 are associated.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.6
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: LLC Service Access Point Identifier

    Description: Identifies the service access point that is used for
                 the GPRS data transfer at LLC layer.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.7
                        24.008 v10.2.0 Section 10.5.6.9
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Notification Indicator

    Description: Informs the UE about an event which is relevant for
                 the upper layer using an EPS bearer context or
                 having requested a procedure transaction.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.7A
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Packet Flow Identifier

    Description: Indicates the packet flow identifier for a packet
                 flow context.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.8
                        24.008 v10.2.0 Section 10.5.6.11
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: PDN Address

    Description: Assigns an IPv4 address to the UE associated with a
                 packet data network and provides the UE with an
                 interface identifier to be used to build the IPv6
                 link local address.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.9
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: PDN Type

    Description: Indicates the IP version capability of the IP stack
                 associated with the UE.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.10
*********************************************************************/
// Defines
#define LIBLTE_MME_PDN_TYPE_IPV4   0x1
#define LIBLTE_MME_PDN_TYPE_IPV6   0x2
#define LIBLTE_MME_PDN_TYPE_IPV4V6 0x3
#define LIBLTE_MME_PDN_TYPE_UNUSED 0x4
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_mme_pack_pdn_type_ie(uint8   pdn_type,
                                              uint8   bit_offset,
                                              uint8 **ie_ptr);
LIBLTE_ERROR_ENUM liblte_mme_unpack_pdn_type_ie(uint8 **ie_ptr,
                                                uint8   bit_offset,
                                                uint8  *pdn_type);

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
// Defines
// Enums
// Structs
typedef struct{
    // FIXME
}LIBLTE_MME_PROTOCOL_CONFIG_OPTIONS_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_mme_pack_protocol_config_options_ie(LIBLTE_MME_PROTOCOL_CONFIG_OPTIONS_STRUCT  *protocol_cnfg_opts,
                                                             uint8                                     **ie_ptr);
LIBLTE_ERROR_ENUM liblte_mme_unpack_protocol_config_options_ie(uint8                                     **ie_ptr,
                                                               LIBLTE_MME_PROTOCOL_CONFIG_OPTIONS_STRUCT  *protocol_cnfg_opts);

/*********************************************************************
    IE Name: Quality Of Service

    Description: Specifies the QoS parameters for a PDP context.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.12
                        24.008 v10.2.0 Section 10.5.6.5
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
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
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Request Type

    Description: Indicates whether the UE requests to establish a new
                 connectivity to a PDN or keep the connection(s) to
                 which it has connected via non-3GPP access.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.14
                        24.008 v10.2.0 Section 10.5.6.17
*********************************************************************/
// Defines
#define LIBLTE_MME_REQUEST_TYPE_INITIAL_REQUEST 0x1
#define LIBLTE_MME_REQUEST_TYPE_HANDOVER        0x2
#define LIBLTE_MME_REQUEST_TYPE_UNUSED          0x3
#define LIBLTE_MME_REQUEST_TYPE_EMERGENCY       0x4
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_mme_pack_request_type_ie(uint8   req_type,
                                                  uint8   bit_offset,
                                                  uint8 **ie_ptr);
LIBLTE_ERROR_ENUM liblte_mme_unpack_request_type_ie(uint8 **ie_ptr,
                                                    uint8   bit_offset,
                                                    uint8  *req_type);

/*********************************************************************
    IE Name: Traffic Flow Aggregate Description

    Description: Specifies the aggregate of one or more packet filters
                 and their related parameters and operations.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.15
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Traffic Flow Template

    Description: Specifies the TFT parameters and operations for a
                 PDP context.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.16
                        24.008 v10.2.0 Section 10.5.6.12
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    IE Name: Transaction Identifier

    Description: Represents the corresponding PDP context in A/Gb
                 mode or Iu mode which is mapped from the EPS bearer
                 context.

    Document Reference: 24.301 v10.2.0 Section 9.9.4.17
                        24.008 v10.2.0 Section 10.5.6.7
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
    Message Name: Message Header (Plain NAS Message)

    Description: Message header for plain NAS messages.

    Document Reference: 24.301 v10.2.0 Section 9.1
*********************************************************************/
// Defines
#define LIBLTE_MME_PD_EPS_SESSION_MANAGEMENT                                              0x2
#define LIBLTE_MME_PD_EPS_MOBILITY_MANAGEMENT                                             0x7
#define LIBLTE_MME_SECURITY_HDR_TYPE_PLAIN_NAS                                            0x0
#define LIBLTE_MME_SECURITY_HDR_TYPE_INTEGRITY                                            0x1
#define LIBLTE_MME_SECURITY_HDR_TYPE_INTEGRITY_AND_CIPHERED                               0x2
#define LIBLTE_MME_SECURITY_HDR_TYPE_INTEGRITY_WITH_NEW_EPS_SECURITY_CONTEXT              0x3
#define LIBLTE_MME_SECURITY_HDR_TYPE_INTEGRITY_AND_CIPHERED_WITH_NEW_EPS_SECURITY_CONTEXT 0x4
#define LIBLTE_MME_SECURITY_HDR_TYPE_SERVICE_REQUEST                                      0xC
#define LIBLTE_MME_MSG_TYPE_ATTACH_REQUEST                                                0x41
#define LIBLTE_MME_MSG_TYPE_ATTACH_ACCEPT                                                 0x42
#define LIBLTE_MME_MSG_TYPE_ATTACH_COMPLETE                                               0x43
#define LIBLTE_MME_MSG_TYPE_ATTACH_REJECT                                                 0x44
#define LIBLTE_MME_MSG_TYPE_DETACH_REQUEST                                                0x45
#define LIBLTE_MME_MSG_TYPE_DETACH_ACCEPT                                                 0x46
#define LIBLTE_MME_MSG_TYPE_TRACKING_AREA_UPDATE_REQUEST                                  0x48
#define LIBLTE_MME_MSG_TYPE_TRACKING_AREA_UPDATE_ACCEPT                                   0x49
#define LIBLTE_MME_MSG_TYPE_TRACKING_AREA_UPDATE_COMPLETE                                 0x4A
#define LIBLTE_MME_MSG_TYPE_TRACKING_AREA_UPDATE_REJECT                                   0x4B
#define LIBLTE_MME_MSG_TYPE_EXTENDED_SERVICE_REQUEST                                      0x4C
#define LIBLTE_MME_MSG_TYPE_SERVICE_REJECT                                                0x4E
#define LIBLTE_MME_MSG_TYPE_GUTI_REALLOCATION_COMMAND                                     0x50
#define LIBLTE_MME_MSG_TYPE_GUTI_REALLOCATION_COMPLETE                                    0x51
#define LIBLTE_MME_MSG_TYPE_AUTHENTICATION_REQUEST                                        0x52
#define LIBLTE_MME_MSG_TYPE_AUTHENTICATION_RESPONSE                                       0x53
#define LIBLTE_MME_MSG_TYPE_AUTHENTICATION_REJECT                                         0x54
#define LIBLTE_MME_MSG_TYPE_AUTHENTICATION_FAILURE                                        0x5C
#define LIBLTE_MME_MSG_TYPE_IDENTITY_REQUEST                                              0x55
#define LIBLTE_MME_MSG_TYPE_IDENTITY_RESPONSE                                             0x56
#define LIBLTE_MME_MSG_TYPE_SECURITY_MODE_COMMAND                                         0x5D
#define LIBLTE_MME_MSG_TYPE_SECURITY_MODE_COMPLETE                                        0x5E
#define LIBLTE_MME_MSG_TYPE_SECURITY_MODE_REJECT                                          0x5F
#define LIBLTE_MME_MSG_TYPE_EMM_STATUS                                                    0x60
#define LIBLTE_MME_MSG_TYPE_EMM_INFORMATION                                               0x61
#define LIBLTE_MME_MSG_TYPE_DOWNLINK_NAS_TRANSPORT                                        0x62
#define LIBLTE_MME_MSG_TYPE_UPLINK_NAS_TRANSPORT                                          0x63
#define LIBLTE_MME_MSG_TYPE_CS_SERVICE_NOTIFICATION                                       0x64
#define LIBLTE_MME_MSG_TYPE_DOWNLINK_GENERIC_NAS_TRANSPORT                                0x68
#define LIBLTE_MME_MSG_TYPE_UPLINK_GENERIC_NAS_TRANSPORT                                  0x69
#define LIBLTE_MME_MSG_TYPE_ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST                   0xC1
#define LIBLTE_MME_MSG_TYPE_ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_ACCEPT                    0xC2
#define LIBLTE_MME_MSG_TYPE_ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REJECT                    0xC3
#define LIBLTE_MME_MSG_TYPE_ACTIVATE_DEDICATED_EPS_BEARER_CONTEXT_REQUEST                 0xC5
#define LIBLTE_MME_MSG_TYPE_ACTIVATE_DEDICATED_EPS_BEARER_CONTEXT_ACCEPT                  0xC6
#define LIBLTE_MME_MSG_TYPE_ACTIVATE_DEDICATED_EPS_BEARER_CONTEXT_REJECT                  0xC7
#define LIBLTE_MME_MSG_TYPE_MODIFY_EPS_BEARER_CONTEXT_REQUEST                             0xC9
#define LIBLTE_MME_MSG_TYPE_MODIFY_EPS_BEARER_CONTEXT_ACCEPT                              0xCA
#define LIBLTE_MME_MSG_TYPE_MODIFY_EPS_BEARER_CONTEXT_REJECT                              0xCB
#define LIBLTE_MME_MSG_TYPE_DEACTIVATE_EPS_BEARER_CONTEXT_REQUEST                         0xCD
#define LIBLTE_MME_MSG_TYPE_DEACTIVATE_EPS_BEARER_CONTEXT_ACCEPT                          0xCE
#define LIBLTE_MME_MSG_TYPE_PDN_CONNECTIVITY_REQUEST                                      0xD0
#define LIBLTE_MME_MSG_TYPE_PDN_CONNECTIVITY_REJECT                                       0xD1
#define LIBLTE_MME_MSG_TYPE_PDN_DISCONNECT_REQUEST                                        0xD2
#define LIBLTE_MME_MSG_TYPE_PDN_DISCONNECT_REJECT                                         0xD3
#define LIBLTE_MME_MSG_TYPE_BEARER_RESOURCE_ALLOCATION_REQUEST                            0xD4
#define LIBLTE_MME_MSG_TYPE_BEARER_RESOURCE_ALLOCATION_REJECT                             0xD5
#define LIBLTE_MME_MSG_TYPE_BEARER_RESOURCE_MODIFICATION_REQUEST                          0xD6
#define LIBLTE_MME_MSG_TYPE_BEARER_RESOURCE_MODIFICATION_REJECT                           0xD7
#define LIBLTE_MME_MSG_TYPE_ESM_INFORMATION_REQUEST                                       0xD9
#define LIBLTE_MME_MSG_TYPE_ESM_INFORMATION_RESPONSE                                      0xDA
#define LIBLTE_MME_MSG_TYPE_NOTIFICATION                                                  0xDB
#define LIBLTE_MME_MSG_TYPE_ESM_STATUS                                                    0xE8
// Enums
// Structs
// Functions
LIBLTE_ERROR_ENUM liblte_mme_parse_msg_header(LIBLTE_BYTE_MSG_STRUCT *msg,
                                              uint8                  *pd,
                                              uint8                  *msg_type);

/*********************************************************************
    Message Name: Attach Accept

    Description: Sent by the network to the UE to indicate that the
                 corresponding attach request has been accepted.

    Document Reference: 24.301 v10.2.0 Section 8.2.1
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Attach Complete

    Description: Sent by the UE to the network in response to an
                 ATTACH ACCEPT message.

    Document Reference: 24.301 v10.2.0 Section 8.2.2
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Attach Reject

    Description: Sent by the network to the UE to indicate that the
                 corresponding attach request has been rejected.

    Document Reference: 24.301 v10.2.0 Section 8.2.3
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Attach Request

    Description: Sent by the UE to the network to perform an attach
                 procedure.

    Document Reference: 24.301 v10.2.0 Section 8.2.4
*********************************************************************/
// Defines
#define LIBLTE_MME_P_TMSI_SIGNATURE_IEI                       0x19
#define LIBLTE_MME_ADDITIONAL_GUTI_IEI                        0x50
#define LIBLTE_MME_LAST_VISITED_REGISTERED_TAI_IEI            0x52
#define LIBLTE_MME_DRX_PARAMETER_IEI                          0x5C
#define LIBLTE_MME_MS_NETWORK_CAPABILITY_IEI                  0x31
#define LIBLTE_MME_LOCATION_AREA_ID_IEI                       0x13
#define LIBLTE_MME_TMSI_STATUS_IEI                            0x9
#define LIBLTE_MME_MS_CLASSMARK_2_IEI                         0x11
#define LIBLTE_MME_MS_CLASSMARK_3_IEI                         0x20
#define LIBLTE_MME_SUPPORTED_CODEC_LIST_IEI                   0x40
#define LIBLTE_MME_ADDITIONAL_UPDATE_TYPE_IEI                 0xF
#define LIBLTE_MME_VOICE_DOMAIN_PREF_AND_UE_USAGE_SETTING_IEI 0x5D
#define LIBLTE_MME_ATTACH_REQUEST_DEVICE_PROPERTIES_IEI       0xD
#define LIBLTE_MME_GUTI_TYPE_IEI                              0xE
// Enums
// Structs
typedef struct{
    LIBLTE_MME_NAS_KEY_SET_ID_STRUCT                         nas_ksi;
    LIBLTE_MME_EPS_MOBILE_ID_STRUCT                          eps_mobile_id;
    LIBLTE_MME_UE_NETWORK_CAPABILITY_STRUCT                  ue_network_cap;
    LIBLTE_BYTE_MSG_STRUCT                                   esm_msg;
    LIBLTE_MME_EPS_MOBILE_ID_STRUCT                          additional_guti;
    LIBLTE_MME_TRACKING_AREA_ID_STRUCT                       last_visited_registered_tai;
    LIBLTE_MME_DRX_PARAMETER_STRUCT                          drx_param;
    LIBLTE_MME_MS_NETWORK_CAPABILITY_STRUCT                  ms_network_cap;
    LIBLTE_MME_LOCATION_AREA_ID_STRUCT                       old_lai;
    LIBLTE_MME_MOBILE_STATION_CLASSMARK_2_STRUCT             ms_cm2;
    LIBLTE_MME_MOBILE_STATION_CLASSMARK_3_STRUCT             ms_cm3;
    LIBLTE_MME_SUPPORTED_CODEC_LIST_STRUCT                   supported_codecs;
    LIBLTE_MME_VOICE_DOMAIN_PREF_AND_UE_USAGE_SETTING_STRUCT voice_domain_pref_and_ue_usage_setting;
    LIBLTE_MME_TMSI_STATUS_ENUM                              tmsi_status;
    LIBLTE_MME_ADDITIONAL_UPDATE_TYPE_ENUM                   additional_update_type;
    LIBLTE_MME_DEVICE_PROPERTIES_ENUM                        device_properties;
    LIBLTE_MME_GUTI_TYPE_ENUM                                old_guti_type;
    uint32                                                   old_p_tmsi_signature;
    uint8                                                    eps_type_result;
    bool                                                     old_p_tmsi_signature_present;
    bool                                                     additional_guti_present;
    bool                                                     last_visited_registered_tai_present;
    bool                                                     drx_param_present;
    bool                                                     ms_network_cap_present;
    bool                                                     old_lai_present;
    bool                                                     tmsi_status_present;
    bool                                                     ms_cm2_present;
    bool                                                     ms_cm3_present;
    bool                                                     supported_codecs_present;
    bool                                                     additional_update_type_present;
    bool                                                     voice_domain_pref_and_ue_usage_setting_present;
    bool                                                     device_properties_present;
    bool                                                     old_guti_type_present;
}LIBLTE_MME_ATTACH_REQUEST_MSG_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_mme_pack_attach_request_msg(LIBLTE_MME_ATTACH_REQUEST_MSG_STRUCT *attach_req,
                                                     LIBLTE_BYTE_MSG_STRUCT               *msg);
LIBLTE_ERROR_ENUM liblte_mme_unpack_attach_request_msg(LIBLTE_BYTE_MSG_STRUCT               *msg,
                                                       LIBLTE_MME_ATTACH_REQUEST_MSG_STRUCT *attach_req);

/*********************************************************************
    Message Name: Authentication Failure

    Description: Sent by the UE to the network to indicate that
                 authentication of the network has failed.

    Document Reference: 24.301 v10.2.0 Section 8.2.5
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Authentication Reject

    Description: Sent by the network to the UE to indicate that the
                 authentication procedure has failed and that the UE
                 shall abort all activities.

    Document Reference: 24.301 v10.2.0 Section 8.2.6
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Authentication Request

    Description: Sent by the network to the UE to initiate
                 authentication of the UE identity.

    Document Reference: 24.301 v10.2.0 Section 8.2.7
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Authentication Response

    Description: Sent by the UE to the network to deliver a calculated
                 authentication response to the network.

    Document Reference: 24.301 v10.2.0 Section 8.2.8
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: CS Service Notification

    Description: Sent by the network when a paging request with CS
                 call indicator was received via SGs for a UE, and a
                 NAS signalling connection is already established for
                 the UE.

    Document Reference: 24.301 v10.2.0 Section 8.2.9
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Detach Accept

    Description: Sent by the network to indicate that the detach
                 procedure has been completed.

    Document Reference: 24.301 v10.2.0 Section 8.2.10
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Detach Request

    Description: Sent by the UE to request the release of an EMM
                 context.

    Document Reference: 24.301 v10.2.0 Section 8.2.11
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Downlink NAS Transport

    Description: Sent by the network to the UE in order to carry an
                 SMS message in encapsulated format.

    Document Reference: 24.301 v10.2.0 Section 8.2.12
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: EMM Information

    Description: Sent by the network at any time during EMM context is
                 established to send certain information to the UE.

    Document Reference: 24.301 v10.2.0 Section 8.2.13
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: EMM Status

    Description: Sent by the UE or by the network at any time to
                 report certain error conditions.

    Document Reference: 24.301 v10.2.0 Section 8.2.14
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
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
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: GUTI Reallocation Command

    Description: Sent by the network to the UE to reallocate a GUTI
                 and optionally provide a new TAI list.

    Document Reference: 24.301 v10.2.0 Section 8.2.16
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: GUTI Reallocation Complete

    Description: Sent by the UE to the network to indicate that
                 reallocation of a GUTI has taken place.

    Document Reference: 24.301 v10.2.0 Section 8.2.17
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Identity Request

    Description: Sent by the network to the UE to request the UE to
                 provide the specified identity.

    Document Reference: 24.301 v10.2.0 Section 8.2.18
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Identity Response

    Description: Sent by the UE to the network in response to an
                 IDENTITY REQUEST message and provides the requested
                 identity.

    Document Reference: 24.301 v10.2.0 Section 8.2.19
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Security Mode Command

    Description: Sent by the network to the UE to establish NAS
                 signalling security.

    Document Reference: 24.301 v10.2.0 Section 8.2.20
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Security Mode Complete

    Description: Sent by the UE to the network in response to a
                 SECURITY MODE COMMAND message.

    Document Reference: 24.301 v10.2.0 Section 8.2.21
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Security Mode Reject

    Description: Sent by the UE to the network to indicate that the
                 corresponding security mode command has been
                 rejected.

    Document Reference: 24.301 v10.2.0 Section 8.2.22
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Security Protected NAS Message

    Description: Sent by the UE or the network to transfer a NAS
                 message together with the sequence number and the
                 message authentication code protecting the message.

    Document Reference: 24.301 v10.2.0 Section 8.2.23
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Service Reject

    Description: Sent by the network to the UE in order to reject the
                 service request procedure.

    Document Reference: 24.301 v10.2.0 Section 8.2.24
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Service Request

    Description: Sent by the UE to the network to request the
                 establishment of a NAS signalling connection and of
                 the radio and S1 bearers.

    Document Reference: 24.301 v10.2.0 Section 8.2.25
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Tracking Area Update Accept

    Description: Sent by the network to the UE to provide the UE with
                 EPS mobility management related data in response to
                 a tracking area update request message.

    Document Reference: 24.301 v10.2.0 Section 8.2.26
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Tracking Area Update Complete

    Description: Sent by the UE to the network in response to a
                 tracking area update accept message if a GUTI has
                 been changed or a new TMSI has been assigned.

    Document Reference: 24.301 v10.2.0 Section 8.2.27
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Tracking Area Update Reject

    Description: Sent by the network to the UE in order to reject the
                 tracking area updating procedure.

    Document Reference: 24.301 v10.2.0 Section 8.2.28
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Tracking Area Update Request

    Description: Sent by the UE to the network to initiate a tracking
                 area updating procedure.

    Document Reference: 24.301 v10.2.0 Section 8.2.29
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Uplink NAS Transport

    Description: Sent by the UE to the network in order to carry an
                 SMS message in encapsulated format.

    Document Reference: 24.301 v10.2.0 Section 8.2.30
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Downlink Generic NAS Transport

    Description: Sent by the network to the UE in order to carry an
                 application message in encapsulated format.

    Document Reference: 24.301 v10.2.0 Section 8.2.31
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Uplink Generic NAS Transport

    Description: Sent by the UE to the network in order to carry an
                 application protocol message in encapsulated format.

    Document Reference: 24.301 v10.2.0 Section 8.2.32
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Activate Dedicated EPS Bearer Context Accept

    Description: Sent by the UE to the network to acknowledge
                 activation of a dedicated EPS bearer context
                 associated with the same PDN address(es) and APN as
                 an already active EPS bearer context.

    Document Reference: 24.301 v10.2.0 Section 8.3.1
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Activate Dedicated EPS Bearer Context Reject

    Description: Sent by the UE to the network to reject activation
                 of a dedicated EPS bearer context.

    Document Reference: 24.301 v10.2.0 Section 8.3.2
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Activate Dedicated EPS Bearer Context Request

    Description: Sent by the network to the UE to request activation
                 of a dedicated EPS bearer context associated with
                 the same PDN address(es) and APN as an already
                 active default EPS bearer context.

    Document Reference: 24.301 v10.2.0 Section 8.3.3
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Activate Default EPS Bearer Context Accept

    Description: Sent by the UE to the network to acknowledge
                 activation of a default EPS bearer context.

    Document Reference: 24.301 v10.2.0 Section 8.3.4
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Activate Default EPS Bearer Context Reject

    Description: Sent by the UE to the network to reject activation
                 of a default EPS bearer context.

    Document Reference: 24.301 v10.2.0 Section 8.3.5
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Activate Default EPS Bearer Context Request

    Description: Sent by the network to the UE to request activation
                 of a default EPS bearer context.

    Document Reference: 24.301 v10.2.0 Section 8.3.6
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Bearer Resource Allocation Reject

    Description: Sent by the network to the UE to reject the
                 allocation of a dedicated bearer resource.

    Document Reference: 24.301 v10.2.0 Section 8.3.7
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Bearer Resource Allocation Request

    Description: Sent by the UE to the network to request the
                 allocation of a dedicated bearer resource.

    Document Reference: 24.301 v10.2.0 Section 8.3.8
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Bearer Resource Modification Reject

    Description: Sent by the network to the UE to reject the
                 modification of a dedicated bearer resource.

    Document Reference: 24.301 v10.2.0 Section 8.3.9
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Bearer Resource Modification Request

    Description: Sent by the UE to the network to request the
                 modification of a dedicated bearer resource.

    Document Reference: 24.301 v10.2.0 Section 8.3.10
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Deactivate EPS Bearer Context Accept

    Description: Sent by the UE to acknowledge deactivation of the
                 EPS bearer context requested in the corresponding
                 deactivate EPS bearer context request message.

    Document Reference: 24.301 v10.2.0 Section 8.3.11
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Deactivate EPS Bearer Context Request

    Description: Sent by the network to request deactivation of an
                 EPS bearer context.

    Document Reference: 24.301 v10.2.0 Section 8.3.12
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: ESM Information Request

    Description: Sent by the network to the UE to request the UE to
                 provide ESM information, i.e. protocol configuration
                 options or APN or both.

    Document Reference: 24.301 v10.2.0 Section 8.3.13
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: ESM Information Response

    Description: Sent by the UE to the network in response to an ESM
                 INFORMATION REQUEST message and provides the
                 requested ESM information.

    Document Reference: 24.301 v10.2.0 Section 8.3.14
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: ESM Status

    Description: Sent by the network or the UE to pass information on
                 the status of the indicated EPS bearer context and
                 report certain error conditions.

    Document Reference: 24.301 v10.2.0 Section 8.3.15
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Modify EPS Bearer Context Accept

    Description: Sent by the UE to the network to acknowledge the
                 modification of an active EPS bearer context.

    Document Reference: 24.301 v10.2.0 Section 8.3.16
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Modify EPS Bearer Context Reject

    Description: Sent by the UE or the network to reject a
                 modification of an active EPS bearer context.

    Document Reference: 24.301 v10.2.0 Section 8.3.17
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Modify EPS Bearer Context Request

    Description: Sent by the network to the UE to request modification
                 of an active EPS bearer context.

    Document Reference: 24.301 v10.2.0 Section 8.3.18
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: Notification

    Description: Sent by the network to inform the UE about events
                 which are relevant for the upper layer using an EPS
                 bearer context or having requested a procedure
                 transaction.

    Document Reference: 24.301 v10.2.0 Section 8.3.18A
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: PDN Connectivity Reject

    Description: Sent by the network to the UE to reject establishment
                 of a PDN connection.

    Document Reference: 24.301 v10.2.0 Section 8.3.19
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: PDN Connectivity Request

    Description: Sent by the UE to the network to initiate
                 establishment of a PDN connection.

    Document Reference: 24.301 v10.2.0 Section 8.3.20
*********************************************************************/
// Defines
#define LIBLTE_MME_ESM_INFO_TRANSFER_FLAG_IEI                     0xD
#define LIBLTE_MME_ACCESS_POINT_NAME_IEI                          0x28
#define LIBLTE_MME_PROTOCOL_CONFIG_OPTIONS_IEI                    0x27
#define LIBLTE_MME_PDN_CONNECTIVITY_REQUEST_DEVICE_PROPERTIES_IEI 0xC
// Enums
// Structs
typedef struct{
    LIBLTE_MME_ACCESS_POINT_NAME_STRUCT       apn;
    LIBLTE_MME_PROTOCOL_CONFIG_OPTIONS_STRUCT protocol_cnfg_opts;
    LIBLTE_MME_ESM_INFO_TRANSFER_FLAG_ENUM    esm_info_transfer_flag;
    LIBLTE_MME_DEVICE_PROPERTIES_ENUM         device_properties;
    uint8                                     eps_bearer_id;
    uint8                                     proc_transaction_id;
    uint8                                     pdn_type;
    uint8                                     request_type;
    bool                                      esm_info_transfer_flag_present;
    bool                                      apn_present;
    bool                                      protocol_cnfg_opts_present;
    bool                                      device_properties_present;
}LIBLTE_MME_PDN_CONNECTIVITY_REQUEST_MSG_STRUCT;
// Functions
LIBLTE_ERROR_ENUM liblte_mme_pack_pdn_connectivity_request_msg(LIBLTE_MME_PDN_CONNECTIVITY_REQUEST_MSG_STRUCT *pdn_con_req,
                                                               LIBLTE_BYTE_MSG_STRUCT                         *msg);
LIBLTE_ERROR_ENUM liblte_mme_unpack_pdn_connectivity_request_msg(LIBLTE_BYTE_MSG_STRUCT                         *msg,
                                                                 LIBLTE_MME_PDN_CONNECTIVITY_REQUEST_MSG_STRUCT *pdn_con_req);

/*********************************************************************
    Message Name: PDN Disconnect Reject

    Description: Sent by the network to the UE to reject release of a
                 PDN connection.

    Document Reference: 24.301 v10.2.0 Section 8.3.21
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

/*********************************************************************
    Message Name: PDN Disconnect Request

    Description: Sent by the UE to the network to initiate release of
                 a PDN connection.

    Document Reference: 24.301 v10.2.0 Section 8.3.22
*********************************************************************/
// Defines
// Enums
// Structs
// Functions
// FIXME

#endif /* __LIBLTE_MME_H__ */
