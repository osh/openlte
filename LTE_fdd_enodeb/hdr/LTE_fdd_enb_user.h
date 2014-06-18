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

    File: LTE_fdd_enb_user.h

    Description: Contains all the definitions for the LTE FDD eNodeB
                 user class.

    Revision History
    ----------    -------------    --------------------------------------------
    11/09/2013    Ben Wojtowicz    Created file
    05/04/2014    Ben Wojtowicz    Added radio bearer support.
    06/15/2014    Ben Wojtowicz    Added initialize routine.

*******************************************************************************/

#ifndef __LTE_FDD_ENB_USER_H__
#define __LTE_FDD_ENB_USER_H__

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_enb_interface.h"
#include "LTE_fdd_enb_rb.h"
#include "liblte_mac.h"
#include "typedefs.h"
#include <string>

/*******************************************************************************
                              DEFINES
*******************************************************************************/


/*******************************************************************************
                              FORWARD DECLARATIONS
*******************************************************************************/


/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/


/*******************************************************************************
                              CLASS DECLARATIONS
*******************************************************************************/

class LTE_fdd_enb_user
{
public:
    // Constructor/Destructor
    LTE_fdd_enb_user(std::string _imsi);
    ~LTE_fdd_enb_user();

    // Initialize
    void init(void);

    // Identity
    std::string get_imsi(void);
    void set_c_rnti(uint16 _c_rnti);
    uint16 get_c_rnti(void);

    // Radio Bearers
    void get_srb0(LTE_fdd_enb_rb **rb);
    LTE_FDD_ENB_ERROR_ENUM setup_srb1(LTE_fdd_enb_rb **rb);
    LTE_FDD_ENB_ERROR_ENUM teardown_srb1(void);
    LTE_FDD_ENB_ERROR_ENUM get_srb1(LTE_fdd_enb_rb **rb);
    LTE_FDD_ENB_ERROR_ENUM setup_srb2(LTE_fdd_enb_rb **rb);
    LTE_FDD_ENB_ERROR_ENUM teardown_srb2(void);
    LTE_FDD_ENB_ERROR_ENUM get_srb2(LTE_fdd_enb_rb **rb);

    // MAC
    LIBLTE_MAC_PDU_STRUCT pusch_mac_pdu;

private:
    // Identity
    std::string imsi;
    uint32      c_rnti;

    // Radio Bearers
    LTE_fdd_enb_rb *srb0;
    LTE_fdd_enb_rb *srb1;
    LTE_fdd_enb_rb *srb2;
    LTE_fdd_enb_rb *drb[8];
};

#endif /* __LTE_FDD_ENB_USER_H__ */
