#line 2 "LTE_fdd_enb_user.cc" // Make __FILE__ omit the path
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

    File: LTE_fdd_enb_user.cc

    Description: Contains all the implementations for the LTE FDD eNodeB
                 user class.

    Revision History
    ----------    -------------    --------------------------------------------
    11/10/2013    Ben Wojtowicz    Created file
    05/04/2014    Ben Wojtowicz    Added radio bearer support.
    06/15/2014    Ben Wojtowicz    Added initialize routine.

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_enb_user.h"

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
                              CLASS IMPLEMENTATIONS
*******************************************************************************/

/********************************/
/*    Constructor/Destructor    */
/********************************/
LTE_fdd_enb_user::LTE_fdd_enb_user(std::string _imsi)
{
    uint32 i;

    // Identity
    imsi   = _imsi;
    c_rnti = LIBLTE_MAC_INVALID_RNTI;

    // Radio Bearers
    srb0 = new LTE_fdd_enb_rb(LTE_FDD_ENB_RB_SRB0, this);
    srb1 = NULL;
    srb2 = NULL;
    for(i=0; i<8; i++)
    {
        drb[i] = NULL;
    }
}
LTE_fdd_enb_user::~LTE_fdd_enb_user()
{
    uint32 i;

    // Radio Bearers
    for(i=0; i<8; i++)
    {
        delete drb[i];
    }
    delete srb2;
    delete srb1;
    delete srb0;
}

/********************/
/*    Initialize    */
/********************/
void LTE_fdd_enb_user::init(void)
{
    uint32 i;

    // Radio Bearers
    for(i=0; i<8; i++)
    {
        delete drb[i];
    }
    delete srb2;
    delete srb1;
    srb0->set_rrc_procedure(LTE_FDD_ENB_RRC_PROC_IDLE);
    srb0->set_rrc_state(LTE_FDD_ENB_RRC_STATE_IDLE);
}

/******************/
/*    Identity    */
/******************/
std::string LTE_fdd_enb_user::get_imsi(void)
{
    return(imsi);
}
void LTE_fdd_enb_user::set_c_rnti(uint16 _c_rnti)
{
    c_rnti = _c_rnti;
}
uint16 LTE_fdd_enb_user::get_c_rnti(void)
{
    return(c_rnti);
}

/***********************/
/*    Radio Bearers    */
/***********************/
void LTE_fdd_enb_user::get_srb0(LTE_fdd_enb_rb **rb)
{
    *rb = srb0;
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_user::setup_srb1(LTE_fdd_enb_rb **rb)
{
    LTE_FDD_ENB_ERROR_ENUM err = LTE_FDD_ENB_ERROR_RB_ALREADY_SETUP;

    if(NULL == srb1)
    {
        srb1 = new LTE_fdd_enb_rb(LTE_FDD_ENB_RB_SRB1, this);
        err  = LTE_FDD_ENB_ERROR_NONE;
    }
    *rb = srb1;

    return(err);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_user::teardown_srb1(void)
{
    LTE_FDD_ENB_ERROR_ENUM err = LTE_FDD_ENB_ERROR_RB_NOT_SETUP;

    if(NULL != srb1)
    {
        delete srb1;
        err = LTE_FDD_ENB_ERROR_NONE;
    }

    return(err);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_user::get_srb1(LTE_fdd_enb_rb **rb)
{
    LTE_FDD_ENB_ERROR_ENUM err = LTE_FDD_ENB_ERROR_RB_NOT_SETUP;

    if(NULL != srb1)
    {
        err = LTE_FDD_ENB_ERROR_NONE;
    }
    *rb = srb1;

    return(err);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_user::setup_srb2(LTE_fdd_enb_rb **rb)
{
    LTE_FDD_ENB_ERROR_ENUM err = LTE_FDD_ENB_ERROR_RB_ALREADY_SETUP;

    if(NULL == srb2)
    {
        srb2 = new LTE_fdd_enb_rb(LTE_FDD_ENB_RB_SRB2, this);
        err  = LTE_FDD_ENB_ERROR_NONE;
    }
    *rb = srb2;

    return(err);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_user::teardown_srb2(void)
{
    LTE_FDD_ENB_ERROR_ENUM err = LTE_FDD_ENB_ERROR_RB_NOT_SETUP;

    if(NULL != srb2)
    {
        delete srb2;
        err = LTE_FDD_ENB_ERROR_NONE;
    }

    return(err);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_user::get_srb2(LTE_fdd_enb_rb **rb)
{
    LTE_FDD_ENB_ERROR_ENUM err = LTE_FDD_ENB_ERROR_RB_NOT_SETUP;

    if(NULL != srb2)
    {
        err = LTE_FDD_ENB_ERROR_NONE;
    }
    *rb = srb2;

    return(err);
}
