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

    File: liblte_interface.h

    Description: Contains all the definitions for the LTE interface library.

    Revision History
    ----------    -------------    --------------------------------------------
    02/23/2013    Ben Wojtowicz    Created file
    11/13/2013    Ben Wojtowicz    Added functions for getting corresponding
                                   EARFCNs for FDD configuration

*******************************************************************************/

#ifndef __LIBLTE_INTERFACE_H__
#define __LIBLTE_INTERFACE_H__

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "typedefs.h"

/*******************************************************************************
                              DEFINES
*******************************************************************************/


/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/


/*******************************************************************************
                              DECLARATIONS
*******************************************************************************/

/*********************************************************************
    Parameter Name: Band

    Description: Defines the operating frequency bands.

    Document Reference: 36.101 v10.4.0 Section 5.5
*********************************************************************/
// Defines
// Enums
typedef enum{
    LIBLTE_INTERFACE_BAND_1 = 0,
    LIBLTE_INTERFACE_BAND_2,
    LIBLTE_INTERFACE_BAND_3,
    LIBLTE_INTERFACE_BAND_4,
    LIBLTE_INTERFACE_BAND_5,
    LIBLTE_INTERFACE_BAND_6,
    LIBLTE_INTERFACE_BAND_7,
    LIBLTE_INTERFACE_BAND_8,
    LIBLTE_INTERFACE_BAND_9,
    LIBLTE_INTERFACE_BAND_10,
    LIBLTE_INTERFACE_BAND_11,
    LIBLTE_INTERFACE_BAND_12,
    LIBLTE_INTERFACE_BAND_13,
    LIBLTE_INTERFACE_BAND_14,
    LIBLTE_INTERFACE_BAND_17,
    LIBLTE_INTERFACE_BAND_18,
    LIBLTE_INTERFACE_BAND_19,
    LIBLTE_INTERFACE_BAND_20,
    LIBLTE_INTERFACE_BAND_21,
    LIBLTE_INTERFACE_BAND_22,
    LIBLTE_INTERFACE_BAND_23,
    LIBLTE_INTERFACE_BAND_24,
    LIBLTE_INTERFACE_BAND_25,
    LIBLTE_INTERFACE_BAND_33,
    LIBLTE_INTERFACE_BAND_34,
    LIBLTE_INTERFACE_BAND_35,
    LIBLTE_INTERFACE_BAND_36,
    LIBLTE_INTERFACE_BAND_37,
    LIBLTE_INTERFACE_BAND_38,
    LIBLTE_INTERFACE_BAND_39,
    LIBLTE_INTERFACE_BAND_40,
    LIBLTE_INTERFACE_BAND_41,
    LIBLTE_INTERFACE_BAND_42,
    LIBLTE_INTERFACE_BAND_43,
    LIBLTE_INTERFACE_BAND_N_ITEMS,
}LIBLTE_INTERFACE_BAND_ENUM;
static const char liblte_interface_band_text[LIBLTE_INTERFACE_BAND_N_ITEMS][20] = { "1",  "2",  "3",  "4",
                                                                                    "5",  "6",  "7",  "8",
                                                                                    "9", "10", "11", "12",
                                                                                   "13", "14", "17", "18",
                                                                                   "19", "20", "21", "22",
                                                                                   "23", "24", "25", "33",
                                                                                   "34", "35", "36", "37",
                                                                                   "38", "39", "40", "41",
                                                                                   "42", "43"};
static const uint8 liblte_interface_band_num[LIBLTE_INTERFACE_BAND_N_ITEMS] = { 1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 17, 18, 19,
                                                                               20, 21, 22, 23, 24, 25, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43};
// Structs
// Functions

/*********************************************************************
    Parameter Name: DL_EARFCN

    Description: Defines the downlink E-UTRA Absolute Radio Frequency
                 Channel Number.

    Document Reference: 36.101 v10.4.0 Section 5.7.3
*********************************************************************/
// Defines
#define LIBLTE_INTERFACE_DL_EARFCN_INVALID 65535
static const uint16 liblte_interface_first_dl_earfcn[LIBLTE_INTERFACE_BAND_N_ITEMS] = {   25,   607,  1207,  1957,  2407,  2675,  2775,  3457,
                                                                                        3825,  4175,  4775,  5017,  5205,  5305,  5755,  5875,
                                                                                        6025,  6175,  6475,  6625,  7507,  7725,  8047, 36025,
                                                                                       36225, 36357, 36957, 37575, 37775, 38275, 38675, 39675,
                                                                                       41615, 43615};
static const uint16 liblte_interface_last_dl_earfcn[LIBLTE_INTERFACE_BAND_N_ITEMS]  = {  575,  1193,  1943,  2393,  2643,  2725,  3425,  3793,
                                                                                        4125,  4725,  4925,  5173,  5255,  5355,  5825,  5975,
                                                                                        6125,  6425,  6575,  7375,  7693,  8015,  8683, 36175,
                                                                                       36325, 36943, 37543, 37725, 38225, 38625, 39625, 41565,
                                                                                       43565, 45565};
// Enums
// Structs
// Functions
uint32 liblte_interface_dl_earfcn_to_frequency(uint16 dl_earfcn);
uint16 liblte_interface_get_corresponding_dl_earfcn(uint16 ul_earfcn);

/*********************************************************************
    Parameter Name: UL_EARFCN

    Description: Defines the uplink E-UTRA Absolute Radio Frequency
                 Channel Number.

    Document Reference: 36.101 v10.4.0 Section 5.7.3
*********************************************************************/
// Defines
#define LIBLTE_INTERFACE_UL_EARFCN_INVALID 65535
static const uint16 liblte_interface_first_ul_earfcn[LIBLTE_INTERFACE_BAND_N_ITEMS] = {18025, 18607, 19207, 19957, 20407, 20675, 20775, 21457,
                                                                                       21825, 22175, 22775, 23017, 23205, 23305, 23755, 23875,
                                                                                       24025, 24175, 24475, 24625, 25507, 25725, 26047, 36025,
                                                                                       36225, 36357, 36957, 37575, 37775, 38275, 38675, 39675,
                                                                                       41615, 43615};
static const uint16 liblte_interface_last_ul_earfcn[LIBLTE_INTERFACE_BAND_N_ITEMS]  = {18575, 19193, 19943, 20393, 20643, 20725, 21425, 21793,
                                                                                       22125, 22725, 22925, 23173, 23255, 23355, 23825, 23975,
                                                                                       24125, 24425, 24575, 25375, 25693, 26015, 26683, 36175,
                                                                                       36325, 36943, 37543, 37725, 38225, 38625, 39625, 41565,
                                                                                       43565, 45565};
// Enums
// Structs
// Functions
uint32 liblte_interface_ul_earfcn_to_frequency(uint16 ul_earfcn);
uint16 liblte_interface_get_corresponding_ul_earfcn(uint16 dl_earfcn);

#endif /* __LIBLTE_INTERFACE_H__ */
