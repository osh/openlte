/*******************************************************************************

    Copyright 2012-2014 Ben Wojtowicz

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

    File: liblte_common.h

    Description: Contains all the common definitions for the LTE library.

    Revision History
    ----------    -------------    --------------------------------------------
    02/26/2012    Ben Wojtowicz    Created file.
    07/21/2013    Ben Wojtowicz    Added a common message structure.
    06/15/2014    Ben Wojtowicz    Split LIBLTE_MSG_STRUCT into bit and byte
                                   aligned messages.

*******************************************************************************/

#ifndef __LIBLTE_COMMON_H__
#define __LIBLTE_COMMON_H__

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "typedefs.h"
#include <string.h>

/*******************************************************************************
                              DEFINES
*******************************************************************************/

// FIXME: This was chosen arbitrarily
#define LIBLTE_MAX_MSG_SIZE 4096

/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/

typedef enum{
    LIBLTE_SUCCESS = 0,
    LIBLTE_ERROR_INVALID_INPUTS,
    LIBLTE_ERROR_DECODE_FAIL,
    LIBLTE_ERROR_INVALID_CRC,
}LIBLTE_ERROR_ENUM;

typedef struct{
    uint32 N_bits;
    uint8  msg[LIBLTE_MAX_MSG_SIZE];
}LIBLTE_BIT_MSG_STRUCT;

typedef struct{
    uint32 N_bytes;
    uint8  msg[LIBLTE_MAX_MSG_SIZE];
}LIBLTE_BYTE_MSG_STRUCT;

/*******************************************************************************
                              DECLARATIONS
*******************************************************************************/

#endif /* __LIBLTE_COMMON_H__ */
