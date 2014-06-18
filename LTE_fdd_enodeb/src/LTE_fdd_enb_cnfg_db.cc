#line 2 "LTE_fdd_enb_cnfg_db.cc" // Make __FILE__ omit the path
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

    File: LTE_fdd_enb_cnfg_db.cc

    Description: Contains all the implementations for the LTE FDD eNodeB
                 configuration database.

    Revision History
    ----------    -------------    --------------------------------------------
    11/10/2013    Ben Wojtowicz    Created file
    01/18/2014    Ben Wojtowicz    Changed several default values, updating
                                   EARFCNs in the radio, and added set/get for
                                   uint32 values.
    03/26/2014    Ben Wojtowicz    Using the latest LTE library.
    05/04/2014    Ben Wojtowicz    Added PCAP support.
    06/15/2014    Ben Wojtowicz    Omitting path from __FILE__.

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_enb_cnfg_db.h"
#include "LTE_fdd_enb_interface.h"
#include "LTE_fdd_enb_phy.h"
#include "LTE_fdd_enb_mac.h"
#include "LTE_fdd_enb_rlc.h"
#include "LTE_fdd_enb_pdcp.h"
#include "LTE_fdd_enb_rrc.h"
#include "LTE_fdd_enb_mme.h"
#include "liblte_mac.h"
#include "liblte_interface.h"
#include <boost/thread/mutex.hpp>
#include <boost/lexical_cast.hpp>

/*******************************************************************************
                              DEFINES
*******************************************************************************/


/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/


/*******************************************************************************
                              GLOBAL VARIABLES
*******************************************************************************/

LTE_fdd_enb_cnfg_db* LTE_fdd_enb_cnfg_db::instance = NULL;
boost::mutex         cnfg_db_instance_mutex;

/*******************************************************************************
                              CLASS IMPLEMENTATIONS
*******************************************************************************/

/*******************/
/*    Singleton    */
/*******************/
LTE_fdd_enb_cnfg_db* LTE_fdd_enb_cnfg_db::get_instance(void)
{
    boost::mutex::scoped_lock lock(cnfg_db_instance_mutex);

    if(NULL == instance)
    {
        instance = new LTE_fdd_enb_cnfg_db();
    }

    return(instance);
}
void LTE_fdd_enb_cnfg_db::cleanup(void)
{
    boost::mutex::scoped_lock lock(cnfg_db_instance_mutex);

    if(NULL != instance)
    {
        delete instance;
        instance = NULL;
    }
}

/********************************/
/*    Constructor/Destructor    */
/********************************/
LTE_fdd_enb_cnfg_db::LTE_fdd_enb_cnfg_db()
{
    // Parameter initialization
    var_map_double[LTE_FDD_ENB_PARAM_BANDWIDTH]                = 10.0;
    var_map_int64[LTE_FDD_ENB_PARAM_FREQ_BAND]                 = 0;
    var_map_int64[LTE_FDD_ENB_PARAM_DL_EARFCN]                 = liblte_interface_first_dl_earfcn[0];
    var_map_int64[LTE_FDD_ENB_PARAM_UL_EARFCN]                 = liblte_interface_get_corresponding_ul_earfcn(liblte_interface_first_dl_earfcn[0]);
    var_map_int64[LTE_FDD_ENB_PARAM_N_RB_DL]                   = LIBLTE_PHY_N_RB_DL_10MHZ;
    var_map_int64[LTE_FDD_ENB_PARAM_N_RB_UL]                   = LIBLTE_PHY_N_RB_UL_10MHZ;
    var_map_int64[LTE_FDD_ENB_PARAM_DL_BW]                     = LIBLTE_RRC_DL_BANDWIDTH_50;
    var_map_int64[LTE_FDD_ENB_PARAM_N_SC_RB_DL]                = LIBLTE_PHY_N_SC_RB_DL_NORMAL_CP;
    var_map_int64[LTE_FDD_ENB_PARAM_N_SC_RB_UL]                = LIBLTE_PHY_N_SC_RB_UL;
    var_map_int64[LTE_FDD_ENB_PARAM_N_ANT]                     = 1;
    var_map_int64[LTE_FDD_ENB_PARAM_N_ID_CELL]                 = 0;
    var_map_int64[LTE_FDD_ENB_PARAM_N_ID_2]                    = 0;
    var_map_int64[LTE_FDD_ENB_PARAM_N_ID_1]                    = 0;
    var_map_uint32[LTE_FDD_ENB_PARAM_MCC]                      = 0xFFFFF001;
    var_map_uint32[LTE_FDD_ENB_PARAM_MNC]                      = 0xFFFFFF01;
    var_map_int64[LTE_FDD_ENB_PARAM_CELL_ID]                   = 1;
    var_map_int64[LTE_FDD_ENB_PARAM_TRACKING_AREA_CODE]        = 1;
    var_map_int64[LTE_FDD_ENB_PARAM_Q_RX_LEV_MIN]              = -140;
    var_map_int64[LTE_FDD_ENB_PARAM_P0_NOMINAL_PUSCH]          = -70;
    var_map_int64[LTE_FDD_ENB_PARAM_P0_NOMINAL_PUCCH]          = -96;
    var_map_int64[LTE_FDD_ENB_PARAM_SIB3_PRESENT]              = 0;
    var_map_int64[LTE_FDD_ENB_PARAM_Q_HYST]                    = LIBLTE_RRC_Q_HYST_DB_0;
    var_map_int64[LTE_FDD_ENB_PARAM_SIB4_PRESENT]              = 0;
    var_map_int64[LTE_FDD_ENB_PARAM_SIB5_PRESENT]              = 0;
    var_map_int64[LTE_FDD_ENB_PARAM_SIB6_PRESENT]              = 0;
    var_map_int64[LTE_FDD_ENB_PARAM_SIB7_PRESENT]              = 0;
    var_map_int64[LTE_FDD_ENB_PARAM_SIB8_PRESENT]              = 0;
    var_map_int64[LTE_FDD_ENB_PARAM_SEARCH_WIN_SIZE]           = 0;
    var_map_uint32[LTE_FDD_ENB_PARAM_SYSTEM_INFO_VALUE_TAG]    = 1;
    var_map_int64[LTE_FDD_ENB_PARAM_SYSTEM_INFO_WINDOW_LENGTH] = LIBLTE_RRC_SI_WINDOW_LENGTH_MS1;
    var_map_int64[LTE_FDD_ENB_PARAM_PHICH_RESOURCE]            = LIBLTE_RRC_PHICH_RESOURCE_1;
    var_map_int64[LTE_FDD_ENB_PARAM_N_SCHED_INFO]              = 1;
    var_map_int64[LTE_FDD_ENB_PARAM_SYSTEM_INFO_PERIODICITY]   = LIBLTE_RRC_SI_PERIODICITY_RF8;
    var_map_uint32[LTE_FDD_ENB_PARAM_DEBUG_TYPE]               = 0xFFFFFFFF;
    var_map_uint32[LTE_FDD_ENB_PARAM_DEBUG_LEVEL]              = 0xFFFFFFFF;
    var_map_int64[LTE_FDD_ENB_PARAM_ENABLE_PCAP]               = 0;
}
LTE_fdd_enb_cnfg_db::~LTE_fdd_enb_cnfg_db()
{
}

/*****************************/
/*    Parameter Gets/Sets    */
/*****************************/
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_cnfg_db::set_param(LTE_FDD_ENB_PARAM_ENUM param,
                                                      int64                  value)
{
    LTE_fdd_enb_radio                                 *radio = LTE_fdd_enb_radio::get_instance();
    std::map<LTE_FDD_ENB_PARAM_ENUM, int64>::iterator  iter  = var_map_int64.find(param);
    LTE_FDD_ENB_ERROR_ENUM                             err   = LTE_FDD_ENB_ERROR_INVALID_PARAM;

    if(var_map_int64.end() != iter)
    {
        (*iter).second = value;
        err            = LTE_FDD_ENB_ERROR_NONE;

        // Set any related parameters
        if(LTE_FDD_ENB_PARAM_N_ID_CELL == param)
        {
            set_param(LTE_FDD_ENB_PARAM_N_ID_2, value % 3);
            set_param(LTE_FDD_ENB_PARAM_N_ID_1, (value - (value % 3))/3);
        }else if(LTE_FDD_ENB_PARAM_DL_EARFCN == param){
            set_param(LTE_FDD_ENB_PARAM_UL_EARFCN, (int64)liblte_interface_get_corresponding_ul_earfcn(value));
            radio->set_earfcns(value, (int64)liblte_interface_get_corresponding_ul_earfcn(value));
        }
    }

    return(err);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_cnfg_db::set_param(LTE_FDD_ENB_PARAM_ENUM param,
                                                      double                 value)
{
    std::map<LTE_FDD_ENB_PARAM_ENUM, double>::iterator iter = var_map_double.find(param);
    LTE_FDD_ENB_ERROR_ENUM                             err  = LTE_FDD_ENB_ERROR_INVALID_PARAM;

    if(var_map_double.end() != iter)
    {
        (*iter).second = value;
        err            = LTE_FDD_ENB_ERROR_NONE;

        // Set any related parameters
        if(LTE_FDD_ENB_PARAM_BANDWIDTH == param)
        {
            if(value == 20)
            {
                set_param(LTE_FDD_ENB_PARAM_N_RB_DL, (int64)LIBLTE_PHY_N_RB_DL_20MHZ);
                set_param(LTE_FDD_ENB_PARAM_N_RB_UL, (int64)LIBLTE_PHY_N_RB_UL_20MHZ);
                set_param(LTE_FDD_ENB_PARAM_DL_BW,   (int64)LIBLTE_RRC_DL_BANDWIDTH_100);
            }else if(value == 15){
                set_param(LTE_FDD_ENB_PARAM_N_RB_DL, (int64)LIBLTE_PHY_N_RB_DL_15MHZ);
                set_param(LTE_FDD_ENB_PARAM_N_RB_UL, (int64)LIBLTE_PHY_N_RB_UL_15MHZ);
                set_param(LTE_FDD_ENB_PARAM_DL_BW,   (int64)LIBLTE_RRC_DL_BANDWIDTH_75);
            }else if(value == 10){
                set_param(LTE_FDD_ENB_PARAM_N_RB_DL, (int64)LIBLTE_PHY_N_RB_DL_10MHZ);
                set_param(LTE_FDD_ENB_PARAM_N_RB_UL, (int64)LIBLTE_PHY_N_RB_UL_10MHZ);
                set_param(LTE_FDD_ENB_PARAM_DL_BW,   (int64)LIBLTE_RRC_DL_BANDWIDTH_50);
            }else if(value == 5){
                set_param(LTE_FDD_ENB_PARAM_N_RB_DL, (int64)LIBLTE_PHY_N_RB_DL_5MHZ);
                set_param(LTE_FDD_ENB_PARAM_N_RB_UL, (int64)LIBLTE_PHY_N_RB_UL_5MHZ);
                set_param(LTE_FDD_ENB_PARAM_DL_BW,   (int64)LIBLTE_RRC_DL_BANDWIDTH_25);
            }else if(value == 3){
                set_param(LTE_FDD_ENB_PARAM_N_RB_DL, (int64)LIBLTE_PHY_N_RB_DL_3MHZ);
                set_param(LTE_FDD_ENB_PARAM_N_RB_UL, (int64)LIBLTE_PHY_N_RB_UL_3MHZ);
                set_param(LTE_FDD_ENB_PARAM_DL_BW,   (int64)LIBLTE_RRC_DL_BANDWIDTH_15);
            }else{
                set_param(LTE_FDD_ENB_PARAM_N_RB_DL, (int64)LIBLTE_PHY_N_RB_DL_1_4MHZ);
                set_param(LTE_FDD_ENB_PARAM_N_RB_UL, (int64)LIBLTE_PHY_N_RB_UL_1_4MHZ);
                set_param(LTE_FDD_ENB_PARAM_DL_BW,   (int64)LIBLTE_RRC_DL_BANDWIDTH_6);
            }
        }
    }

    return(err);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_cnfg_db::set_param(LTE_FDD_ENB_PARAM_ENUM param,
                                                      std::string            value)
{
    std::map<LTE_FDD_ENB_PARAM_ENUM, uint32>::iterator iter = var_map_uint32.find(param);
    LTE_FDD_ENB_ERROR_ENUM                             err  = LTE_FDD_ENB_ERROR_INVALID_PARAM;
    uint32                                             i;

    if(var_map_uint32.end() != iter)
    {
        (*iter).second = 0xFFFFFFFF;
        for(i=0; i<value.length(); i++)
        {
            (*iter).second <<= 4;
            (*iter).second  |= (value[i] & 0x0F);
        }
        err = LTE_FDD_ENB_ERROR_NONE;
    }

    return(err);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_cnfg_db::set_param(LTE_FDD_ENB_PARAM_ENUM param,
                                                      uint32                 value)
{
    std::map<LTE_FDD_ENB_PARAM_ENUM, uint32>::iterator iter = var_map_uint32.find(param);
    LTE_FDD_ENB_ERROR_ENUM                             err  = LTE_FDD_ENB_ERROR_INVALID_PARAM;

    if(var_map_uint32.end() != iter)
    {
        (*iter).second = value;
        err            = LTE_FDD_ENB_ERROR_NONE;
    }

    return(err);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_cnfg_db::get_param(LTE_FDD_ENB_PARAM_ENUM  param,
                                                      int64                  &value)
{
    std::map<LTE_FDD_ENB_PARAM_ENUM, int64>::iterator iter = var_map_int64.find(param);
    LTE_FDD_ENB_ERROR_ENUM                            err  = LTE_FDD_ENB_ERROR_INVALID_PARAM;

    if(var_map_int64.end() != iter)
    {
        value = (*iter).second;
        err   = LTE_FDD_ENB_ERROR_NONE;
    }

    return(err);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_cnfg_db::get_param(LTE_FDD_ENB_PARAM_ENUM  param,
                                                      double                 &value)
{
    std::map<LTE_FDD_ENB_PARAM_ENUM, double>::iterator iter = var_map_double.find(param);
    LTE_FDD_ENB_ERROR_ENUM                             err  = LTE_FDD_ENB_ERROR_INVALID_PARAM;

    if(var_map_double.end() != iter)
    {
        value = (*iter).second;
        err   = LTE_FDD_ENB_ERROR_NONE;
    }

    return(err);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_cnfg_db::get_param(LTE_FDD_ENB_PARAM_ENUM  param,
                                                      std::string            &value)
{
    std::map<LTE_FDD_ENB_PARAM_ENUM, uint32>::iterator iter = var_map_uint32.find(param);
    LTE_FDD_ENB_ERROR_ENUM                             err  = LTE_FDD_ENB_ERROR_INVALID_PARAM;
    uint32                                             i;

    if(var_map_uint32.end() != iter)
    {
        try
        {
            for(i=0; i<8; i++)
            {
                if((((*iter).second >> (7-i)*4) & 0x0F) != 0xF)
                {
                    value += boost::lexical_cast<std::string>(((*iter).second >> (7-i)*4) & 0x0F);
                }
            }
            err = LTE_FDD_ENB_ERROR_NONE;
        }catch(...){
            // Intentionally do nothing
        }
    }

    return(err);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_cnfg_db::get_param(LTE_FDD_ENB_PARAM_ENUM  param,
                                                      uint32                 &value)
{
    std::map<LTE_FDD_ENB_PARAM_ENUM, uint32>::iterator iter = var_map_uint32.find(param);
    LTE_FDD_ENB_ERROR_ENUM                             err  = LTE_FDD_ENB_ERROR_INVALID_PARAM;

    if(var_map_uint32.end() != iter)
    {
        value = (*iter).second;
        err   = LTE_FDD_ENB_ERROR_NONE;
    }

    return(err);
}

/******************************/
/*    MIB/SIB Construction    */
/******************************/
void LTE_fdd_enb_cnfg_db::construct_sys_info(void)
{
    LTE_fdd_enb_phy                                    *phy  = LTE_fdd_enb_phy::get_instance();
    LTE_fdd_enb_mac                                    *mac  = LTE_fdd_enb_mac::get_instance();
    LTE_fdd_enb_rlc                                    *rlc  = LTE_fdd_enb_rlc::get_instance();
    LTE_fdd_enb_pdcp                                   *pdcp = LTE_fdd_enb_pdcp::get_instance();
    LTE_fdd_enb_rrc                                    *rrc  = LTE_fdd_enb_rrc::get_instance();
    LTE_fdd_enb_mme                                    *mme  = LTE_fdd_enb_mme::get_instance();
    std::map<LTE_FDD_ENB_PARAM_ENUM, double>::iterator  double_iter;
    std::map<LTE_FDD_ENB_PARAM_ENUM, int64>::iterator   int64_iter;
    std::map<LTE_FDD_ENB_PARAM_ENUM, uint32>::iterator  uint32_iter;
    LIBLTE_RRC_SIB_TYPE_ENUM                            sib_array[6];
    LIBLTE_RRC_BCCH_DLSCH_MSG_STRUCT                    bcch_dlsch_msg;
    uint32                                              num_sibs      = 0;
    uint32                                              sib_idx       = 0;
    uint32                                              N_sibs_to_map = 0;
    uint32                                              i;
    uint32                                              j;

    // MIB
    double_iter = var_map_double.find(LTE_FDD_ENB_PARAM_BANDWIDTH);
    if(var_map_double.end() != double_iter)
    {
        for(i=0; i<LIBLTE_RRC_DL_BANDWIDTH_N_ITEMS; i++)
        {
            if((*double_iter).second == liblte_rrc_dl_bandwidth_num[i])
            {
                sys_info.mib.dl_bw = (LIBLTE_RRC_DL_BANDWIDTH_ENUM)i;
                break;
            }
        }
    }
    sys_info.mib.phich_config.dur = LIBLTE_RRC_PHICH_DURATION_NORMAL;
    sys_info.mib.phich_config.res = LIBLTE_RRC_PHICH_RESOURCE_1;

    // Determine which SIBs need to be mapped
    int64_iter = var_map_int64.find(LTE_FDD_ENB_PARAM_SIB3_PRESENT);
    if(var_map_int64.end() != int64_iter &&
       1                   == (*int64_iter).second)
    {
        sib_array[num_sibs++] = LIBLTE_RRC_SIB_TYPE_3;
    }
    int64_iter = var_map_int64.find(LTE_FDD_ENB_PARAM_SIB4_PRESENT);
    if(var_map_int64.end() != int64_iter &&
       1                   == (*int64_iter).second)
    {
        sib_array[num_sibs++] = LIBLTE_RRC_SIB_TYPE_4;
    }
    int64_iter = var_map_int64.find(LTE_FDD_ENB_PARAM_SIB5_PRESENT);
    if(var_map_int64.end() != int64_iter &&
       1                   == (*int64_iter).second)
    {
        sib_array[num_sibs++] = LIBLTE_RRC_SIB_TYPE_5;
    }
    int64_iter = var_map_int64.find(LTE_FDD_ENB_PARAM_SIB6_PRESENT);
    if(var_map_int64.end() != int64_iter &&
       1                   == (*int64_iter).second)
    {
        sib_array[num_sibs++] = LIBLTE_RRC_SIB_TYPE_6;
    }
    int64_iter = var_map_int64.find(LTE_FDD_ENB_PARAM_SIB7_PRESENT);
    if(var_map_int64.end() != int64_iter &&
       1                   == (*int64_iter).second)
    {
        sib_array[num_sibs++] = LIBLTE_RRC_SIB_TYPE_7;
    }
    int64_iter = var_map_int64.find(LTE_FDD_ENB_PARAM_SIB8_PRESENT);
    if(var_map_int64.end() != int64_iter &&
       1                   == (*int64_iter).second)
    {
        sib_array[num_sibs++] = LIBLTE_RRC_SIB_TYPE_8;
    }

    // Initialize the scheduling info
    sys_info.sib1.N_sched_info                     = 1;
    sys_info.sib1.sched_info[0].N_sib_mapping_info = 0;

    // Map the SIBs
    while(num_sibs > 0)
    {
        // Determine how many SIBs can be mapped to this scheduling info
        if(1 == sys_info.sib1.N_sched_info)
        {
            if(0                         == sys_info.sib1.sched_info[0].N_sib_mapping_info &&
               LIBLTE_RRC_DL_BANDWIDTH_6 != sys_info.mib.dl_bw)
            {
                N_sibs_to_map = 1;
            }else{
                N_sibs_to_map                                                           = 2;
                sys_info.sib1.sched_info[sys_info.sib1.N_sched_info].N_sib_mapping_info = 0;
                sys_info.sib1.sched_info[sys_info.sib1.N_sched_info].si_periodicity     = LIBLTE_RRC_SI_PERIODICITY_RF8;
                sys_info.sib1.N_sched_info++;
            }
        }else{
            if(2 > sys_info.sib1.sched_info[sys_info.sib1.N_sched_info-1].N_sib_mapping_info)
            {
                N_sibs_to_map = 2 - sys_info.sib1.sched_info[sys_info.sib1.N_sched_info-1].N_sib_mapping_info;
            }else{
                N_sibs_to_map                                                           = 2;
                sys_info.sib1.sched_info[sys_info.sib1.N_sched_info].N_sib_mapping_info = 0;
                sys_info.sib1.sched_info[sys_info.sib1.N_sched_info].si_periodicity     = LIBLTE_RRC_SI_PERIODICITY_RF8;
                sys_info.sib1.N_sched_info++;
            }
        }

        // Map the SIBs for this scheduling info
        for(i=0; i<N_sibs_to_map; i++)
        {
            sys_info.sib1.sched_info[sys_info.sib1.N_sched_info-1].sib_mapping_info[sys_info.sib1.sched_info[sys_info.sib1.N_sched_info-1].N_sib_mapping_info].sib_type = sib_array[sib_idx++];
            sys_info.sib1.sched_info[sys_info.sib1.N_sched_info-1].N_sib_mapping_info++;
            num_sibs--;

            if(0 == num_sibs)
            {
                break;
            }
        }
    }

    // SIB1
    sys_info.sib1.N_plmn_ids = 1;
    uint32_iter = var_map_uint32.find(LTE_FDD_ENB_PARAM_MCC);
    if(var_map_uint32.end() != uint32_iter)
    {
        sys_info.sib1.plmn_id[0].id.mcc = ((*uint32_iter).second) & 0xFFFF;
    }
    uint32_iter = var_map_uint32.find(LTE_FDD_ENB_PARAM_MNC);
    if(var_map_uint32.end() != uint32_iter)
    {
        sys_info.sib1.plmn_id[0].id.mnc = ((*uint32_iter).second) & 0xFFFF;
    }
    sys_info.sib1.plmn_id[0].resv_for_oper         = LIBLTE_RRC_NOT_RESV_FOR_OPER;
    sys_info.sib1.cell_barred                      = LIBLTE_RRC_CELL_NOT_BARRED;
    sys_info.sib1.intra_freq_reselection           = LIBLTE_RRC_INTRA_FREQ_RESELECTION_ALLOWED;
    sys_info.sib1.si_window_length                 = LIBLTE_RRC_SI_WINDOW_LENGTH_MS2;
    sys_info.sib1.sf_assignment                    = LIBLTE_RRC_SUBFRAME_ASSIGNMENT_0;
    sys_info.sib1.special_sf_patterns              = LIBLTE_RRC_SPECIAL_SUBFRAME_PATTERNS_0;
    int64_iter                                     = var_map_int64.find(LTE_FDD_ENB_PARAM_CELL_ID);
    if(var_map_int64.end() != int64_iter)
    {
        sys_info.sib1.cell_id = (*int64_iter).second;
    }
    sys_info.sib1.csg_id = 0;
    int64_iter           = var_map_int64.find(LTE_FDD_ENB_PARAM_TRACKING_AREA_CODE);
    if(var_map_int64.end() != int64_iter)
    {
        sys_info.sib1.tracking_area_code = (*int64_iter).second;
    }
    int64_iter = var_map_int64.find(LTE_FDD_ENB_PARAM_Q_RX_LEV_MIN);
    if(var_map_int64.end() != int64_iter)
    {
        sys_info.sib1.q_rx_lev_min = (*int64_iter).second;
    }
    sys_info.sib1.csg_indication      = 0;
    sys_info.sib1.q_rx_lev_min_offset = 1;
    int64_iter                        = var_map_int64.find(LTE_FDD_ENB_PARAM_FREQ_BAND);
    if(var_map_int64.end() != int64_iter)
    {
        sys_info.sib1.freq_band_indicator = liblte_interface_band_num[(*int64_iter).second];
    }
    uint32_iter = var_map_uint32.find(LTE_FDD_ENB_PARAM_SYSTEM_INFO_VALUE_TAG);
    if(var_map_uint32.end() != uint32_iter)
    {
        sys_info.sib1.system_info_value_tag = (*uint32_iter).second++;
    }
    sys_info.sib1.p_max_present = true;
    sys_info.sib1.p_max         = 23;
    sys_info.sib1.tdd           = false;

    // SIB2
    sys_info.sib2.ac_barring_info_present                                                      = false;
    sys_info.sib2.rr_config_common_sib.rach_cnfg.num_ra_preambles                              = LIBLTE_RRC_NUMBER_OF_RA_PREAMBLES_N4;
    sys_info.sib2.rr_config_common_sib.rach_cnfg.preambles_group_a_cnfg.present                = false;
    sys_info.sib2.rr_config_common_sib.rach_cnfg.pwr_ramping_step                              = LIBLTE_RRC_POWER_RAMPING_STEP_DB6;
    sys_info.sib2.rr_config_common_sib.rach_cnfg.preamble_init_rx_target_pwr                   = LIBLTE_RRC_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER_DBM_N90;
    sys_info.sib2.rr_config_common_sib.rach_cnfg.preamble_trans_max                            = LIBLTE_RRC_PREAMBLE_TRANS_MAX_N200;
    sys_info.sib2.rr_config_common_sib.rach_cnfg.ra_resp_win_size                              = LIBLTE_RRC_RA_RESPONSE_WINDOW_SIZE_SF7;
    sys_info.sib2.rr_config_common_sib.rach_cnfg.mac_con_res_timer                             = LIBLTE_RRC_MAC_CONTENTION_RESOLUTION_TIMER_SF64;
    sys_info.sib2.rr_config_common_sib.rach_cnfg.max_harq_msg3_tx                              = 1;
    sys_info.sib2.rr_config_common_sib.bcch_cnfg.modification_period_coeff                     = LIBLTE_RRC_MODIFICATION_PERIOD_COEFF_N2;
    sys_info.sib2.rr_config_common_sib.pcch_cnfg.default_paging_cycle                          = LIBLTE_RRC_DEFAULT_PAGING_CYCLE_RF256;
    sys_info.sib2.rr_config_common_sib.pcch_cnfg.nB                                            = LIBLTE_RRC_NB_ONE_T;
    sys_info.sib2.rr_config_common_sib.prach_cnfg.root_sequence_index                          = 0;
    sys_info.sib2.rr_config_common_sib.prach_cnfg.prach_cnfg_info.prach_config_index           = 0;
    sys_info.sib2.rr_config_common_sib.prach_cnfg.prach_cnfg_info.high_speed_flag              = false;
    sys_info.sib2.rr_config_common_sib.prach_cnfg.prach_cnfg_info.zero_correlation_zone_config = 1;
    sys_info.sib2.rr_config_common_sib.prach_cnfg.prach_cnfg_info.prach_freq_offset            = 0;
    sys_info.sib2.rr_config_common_sib.pdsch_cnfg.rs_power                                     = 0;
    sys_info.sib2.rr_config_common_sib.pdsch_cnfg.p_b                                          = 0;
    sys_info.sib2.rr_config_common_sib.pusch_cnfg.n_sb                                         = 1;
    sys_info.sib2.rr_config_common_sib.pusch_cnfg.hopping_mode                                 = LIBLTE_RRC_HOPPING_MODE_INTER_SUBFRAME;
    sys_info.sib2.rr_config_common_sib.pusch_cnfg.pusch_hopping_offset                         = 0;
    sys_info.sib2.rr_config_common_sib.pusch_cnfg.enable_64_qam                                = true;
    sys_info.sib2.rr_config_common_sib.pusch_cnfg.ul_rs.group_hopping_enabled                  = false;
    sys_info.sib2.rr_config_common_sib.pusch_cnfg.ul_rs.group_assignment_pusch                 = 0;
    sys_info.sib2.rr_config_common_sib.pusch_cnfg.ul_rs.sequence_hopping_enabled               = false;
    sys_info.sib2.rr_config_common_sib.pusch_cnfg.ul_rs.cyclic_shift                           = 0;
    sys_info.sib2.rr_config_common_sib.pucch_cnfg.delta_pucch_shift                            = LIBLTE_RRC_DELTA_PUCCH_SHIFT_DS1;
    sys_info.sib2.rr_config_common_sib.pucch_cnfg.n_rb_cqi                                     = 0;
    sys_info.sib2.rr_config_common_sib.pucch_cnfg.n_cs_an                                      = 0;
    sys_info.sib2.rr_config_common_sib.pucch_cnfg.n1_pucch_an                                  = 0;
    sys_info.sib2.rr_config_common_sib.srs_ul_cnfg.present                                     = false;
    int64_iter                                                                                 = var_map_int64.find(LTE_FDD_ENB_PARAM_P0_NOMINAL_PUSCH);
    if(var_map_int64.end() != int64_iter)
    {
        sys_info.sib2.rr_config_common_sib.ul_pwr_ctrl.p0_nominal_pusch = (*int64_iter).second;
    }
    sys_info.sib2.rr_config_common_sib.ul_pwr_ctrl.alpha = LIBLTE_RRC_UL_POWER_CONTROL_ALPHA_1;
    int64_iter                                           = var_map_int64.find(LTE_FDD_ENB_PARAM_P0_NOMINAL_PUCCH);
    if(var_map_int64.end() != int64_iter)
    {
        sys_info.sib2.rr_config_common_sib.ul_pwr_ctrl.p0_nominal_pucch = (*int64_iter).second;
    }
    sys_info.sib2.rr_config_common_sib.ul_pwr_ctrl.delta_flist_pucch.format_1  = LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_1_0;
    sys_info.sib2.rr_config_common_sib.ul_pwr_ctrl.delta_flist_pucch.format_1b = LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_1B_1;
    sys_info.sib2.rr_config_common_sib.ul_pwr_ctrl.delta_flist_pucch.format_2  = LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2_0;
    sys_info.sib2.rr_config_common_sib.ul_pwr_ctrl.delta_flist_pucch.format_2a = LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2A_0;
    sys_info.sib2.rr_config_common_sib.ul_pwr_ctrl.delta_flist_pucch.format_2b = LIBLTE_RRC_DELTA_F_PUCCH_FORMAT_2B_0;
    sys_info.sib2.rr_config_common_sib.ul_pwr_ctrl.delta_preamble_msg3         = -2;
    sys_info.sib2.rr_config_common_sib.ul_cp_length                            = LIBLTE_RRC_UL_CP_LENGTH_1;
    sys_info.sib2.ue_timers_and_constants.t300                                 = LIBLTE_RRC_T300_MS1000;
    sys_info.sib2.ue_timers_and_constants.t301                                 = LIBLTE_RRC_T301_MS1000;
    sys_info.sib2.ue_timers_and_constants.t310                                 = LIBLTE_RRC_T310_MS1000;
    sys_info.sib2.ue_timers_and_constants.n310                                 = LIBLTE_RRC_N310_N20;
    sys_info.sib2.ue_timers_and_constants.t311                                 = LIBLTE_RRC_T311_MS1000;
    sys_info.sib2.ue_timers_and_constants.n311                                 = LIBLTE_RRC_N311_N10;
    sys_info.sib2.arfcn_value_eutra.present                                    = false;
    sys_info.sib2.ul_bw.present                                                = false;
    sys_info.sib2.additional_spectrum_emission                                 = 1;
    sys_info.sib2.mbsfn_subfr_cnfg_list_size                                   = 0;
    sys_info.sib2.time_alignment_timer                                         = LIBLTE_RRC_TIME_ALIGNMENT_TIMER_SF500;

    // SIB3
    sys_info.sib3_present = false;
    int64_iter            = var_map_int64.find(LTE_FDD_ENB_PARAM_SIB3_PRESENT);
    if(var_map_int64.end() != int64_iter &&
       1                   == (*int64_iter).second)
    {
        sys_info.sib3_present = true;
        int64_iter            = var_map_int64.find(LTE_FDD_ENB_PARAM_Q_HYST);
        if(var_map_int64.end() != int64_iter)
        {
            sys_info.sib3.q_hyst = (LIBLTE_RRC_Q_HYST_ENUM)(*int64_iter).second;
        }
        sys_info.sib3.speed_state_resel_params.present = false;
        sys_info.sib3.s_non_intra_search_present       = false;
        sys_info.sib3.thresh_serving_low               = 0;
        sys_info.sib3.cell_resel_prio                  = 0;
        sys_info.sib3.q_rx_lev_min                     = sys_info.sib1.q_rx_lev_min;
        sys_info.sib3.p_max_present                    = true;
        sys_info.sib3.p_max                            = sys_info.sib1.p_max;
        sys_info.sib3.s_intra_search_present           = false;
        sys_info.sib3.allowed_meas_bw_present          = false;
        sys_info.sib3.presence_ant_port_1              = false;
        sys_info.sib3.neigh_cell_cnfg                  = 0;
        sys_info.sib3.t_resel_eutra                    = 0;
        sys_info.sib3.t_resel_eutra_sf_present         = false;
    }

    // SIB4
    sys_info.sib4_present = false;
    int64_iter            = var_map_int64.find(LTE_FDD_ENB_PARAM_SIB4_PRESENT);
    if(var_map_int64.end() != int64_iter &&
       1                   == (*int64_iter).second)
    {
        sys_info.sib4_present                         = true;
        sys_info.sib4.intra_freq_neigh_cell_list_size = 0;
        sys_info.sib4.intra_freq_black_cell_list_size = 0;
        sys_info.sib4.csg_phys_cell_id_range_present  = false;
    }

    // SIB5
    sys_info.sib5_present = false;
    int64_iter            = var_map_int64.find(LTE_FDD_ENB_PARAM_SIB5_PRESENT);
    if(var_map_int64.end() != int64_iter &&
       1                   == (*int64_iter).second)
    {
        sys_info.sib5_present                           = true;
        sys_info.sib5.inter_freq_carrier_freq_list_size = 0;
    }

    // SIB6
    sys_info.sib6_present = false;
    int64_iter            = var_map_int64.find(LTE_FDD_ENB_PARAM_SIB6_PRESENT);
    if(var_map_int64.end() != int64_iter &&
       1                   == (*int64_iter).second)
    {
        sys_info.sib6_present                         = true;
        sys_info.sib6.carrier_freq_list_utra_fdd_size = 0;
        sys_info.sib6.carrier_freq_list_utra_tdd_size = 0;
        sys_info.sib6.t_resel_utra                    = 1;
        sys_info.sib6.t_resel_utra_sf_present         = false;
    }

    // SIB7
    sys_info.sib7_present = false;
    int64_iter            = var_map_int64.find(LTE_FDD_ENB_PARAM_SIB7_PRESENT);
    if(var_map_int64.end() != int64_iter &&
       1                   == (*int64_iter).second)
    {
        sys_info.sib7_present                      = true;
        sys_info.sib7.t_resel_geran                = 1;
        sys_info.sib7.t_resel_geran_sf_present     = false;
        sys_info.sib7.carrier_freqs_info_list_size = 0;
    }

    // SIB8
    sys_info.sib8_present = false;
    int64_iter            = var_map_int64.find(LTE_FDD_ENB_PARAM_SIB8_PRESENT);
    if(var_map_int64.end() != int64_iter &&
       1                   == (*int64_iter).second)
    {
        sys_info.sib8_present                 = true;
        sys_info.sib8.sys_time_info_present   = false;
        sys_info.sib8.search_win_size_present = true;
        int64_iter                            = var_map_int64.find(LTE_FDD_ENB_PARAM_SEARCH_WIN_SIZE);
        if(var_map_int64.end() != int64_iter)
        {
            sys_info.sib8.search_win_size = (*int64_iter).second;
        }
        sys_info.sib8.params_hrpd_present  = false;
        sys_info.sib8.params_1xrtt_present = false;
    }

    // Pack SIB1
    bcch_dlsch_msg.N_sibs           = 0;
    bcch_dlsch_msg.sibs[0].sib_type = LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1;
    memcpy(&bcch_dlsch_msg.sibs[0].sib, &sys_info.sib1, sizeof(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1_STRUCT));
    liblte_rrc_pack_bcch_dlsch_msg(&bcch_dlsch_msg,
                                   &sys_info.sib1_alloc.msg);
    sys_info.sib1_alloc.pre_coder_type = LIBLTE_PHY_PRE_CODER_TYPE_TX_DIVERSITY;
    sys_info.sib1_alloc.mod_type       = LIBLTE_PHY_MODULATION_TYPE_QPSK;
    sys_info.sib1_alloc.rv_idx         = 0; // 36.321 section 5.3.1
    sys_info.sib1_alloc.N_codewords    = 1;
    sys_info.sib1_alloc.rnti           = LIBLTE_MAC_SI_RNTI;
    sys_info.sib1_alloc.tx_mode        = 1;

    // Pack additional SIBs
    bcch_dlsch_msg.N_sibs           = 1;
    bcch_dlsch_msg.sibs[0].sib_type = LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_2;
    memcpy(&bcch_dlsch_msg.sibs[0].sib, &sys_info.sib2, sizeof(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_2_STRUCT));
    if(0 != sys_info.sib1.sched_info[0].N_sib_mapping_info)
    {
        switch(sys_info.sib1.sched_info[0].sib_mapping_info[0].sib_type)
        {
        case LIBLTE_RRC_SIB_TYPE_3:
            bcch_dlsch_msg.N_sibs++;
            bcch_dlsch_msg.sibs[1].sib_type = LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_3;
            memcpy(&bcch_dlsch_msg.sibs[1].sib, &sys_info.sib3, sizeof(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_3_STRUCT));
            break;
        case LIBLTE_RRC_SIB_TYPE_4:
            bcch_dlsch_msg.N_sibs++;
            bcch_dlsch_msg.sibs[1].sib_type = LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_4;
            memcpy(&bcch_dlsch_msg.sibs[1].sib, &sys_info.sib4, sizeof(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_4_STRUCT));
            break;
        case LIBLTE_RRC_SIB_TYPE_5:
            bcch_dlsch_msg.N_sibs++;
            bcch_dlsch_msg.sibs[1].sib_type = LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_5;
            memcpy(&bcch_dlsch_msg.sibs[1].sib, &sys_info.sib5, sizeof(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_5_STRUCT));
            break;
        case LIBLTE_RRC_SIB_TYPE_6:
            bcch_dlsch_msg.N_sibs++;
            bcch_dlsch_msg.sibs[1].sib_type = LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_6;
            memcpy(&bcch_dlsch_msg.sibs[1].sib, &sys_info.sib6, sizeof(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_6_STRUCT));
            break;
        case LIBLTE_RRC_SIB_TYPE_7:
            bcch_dlsch_msg.N_sibs++;
            bcch_dlsch_msg.sibs[1].sib_type = LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_7;
            memcpy(&bcch_dlsch_msg.sibs[1].sib, &sys_info.sib7, sizeof(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_7_STRUCT));
            break;
        case LIBLTE_RRC_SIB_TYPE_8:
            bcch_dlsch_msg.N_sibs++;
            bcch_dlsch_msg.sibs[1].sib_type = LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_8;
            memcpy(&bcch_dlsch_msg.sibs[1].sib, &sys_info.sib8, sizeof(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_8_STRUCT));
            break;
        default:
            break;
        }
    }
    liblte_rrc_pack_bcch_dlsch_msg(&bcch_dlsch_msg,
                                   &sys_info.sib_alloc[0].msg);
    sys_info.sib_alloc[0].pre_coder_type = LIBLTE_PHY_PRE_CODER_TYPE_TX_DIVERSITY;
    sys_info.sib_alloc[0].mod_type       = LIBLTE_PHY_MODULATION_TYPE_QPSK;
    sys_info.sib_alloc[0].rv_idx         = 0; // 36.321 section 5.3.1
    sys_info.sib_alloc[0].N_codewords    = 1;
    sys_info.sib_alloc[0].rnti           = LIBLTE_MAC_SI_RNTI;
    sys_info.sib_alloc[0].tx_mode        = 1;
    for(i=1; i<sys_info.sib1.N_sched_info; i++)
    {
        bcch_dlsch_msg.N_sibs = sys_info.sib1.sched_info[i].N_sib_mapping_info;
        for(j=0; j<bcch_dlsch_msg.N_sibs; j++)
        {
            switch(sys_info.sib1.sched_info[i].sib_mapping_info[j].sib_type)
            {
            case LIBLTE_RRC_SIB_TYPE_3:
                bcch_dlsch_msg.sibs[j].sib_type = LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_3;
                memcpy(&bcch_dlsch_msg.sibs[j].sib, &sys_info.sib3, sizeof(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_3_STRUCT));
                break;
            case LIBLTE_RRC_SIB_TYPE_4:
                bcch_dlsch_msg.sibs[j].sib_type = LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_4;
                memcpy(&bcch_dlsch_msg.sibs[j].sib, &sys_info.sib4, sizeof(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_4_STRUCT));
                break;
            case LIBLTE_RRC_SIB_TYPE_5:
                bcch_dlsch_msg.sibs[j].sib_type = LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_5;
                memcpy(&bcch_dlsch_msg.sibs[j].sib, &sys_info.sib5, sizeof(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_5_STRUCT));
                break;
            case LIBLTE_RRC_SIB_TYPE_6:
                bcch_dlsch_msg.sibs[j].sib_type = LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_6;
                memcpy(&bcch_dlsch_msg.sibs[j].sib, &sys_info.sib6, sizeof(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_6_STRUCT));
                break;
            case LIBLTE_RRC_SIB_TYPE_7:
                bcch_dlsch_msg.sibs[j].sib_type = LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_7;
                memcpy(&bcch_dlsch_msg.sibs[j].sib, &sys_info.sib7, sizeof(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_7_STRUCT));
                break;
            case LIBLTE_RRC_SIB_TYPE_8:
                bcch_dlsch_msg.sibs[j].sib_type = LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_8;
                memcpy(&bcch_dlsch_msg.sibs[j].sib, &sys_info.sib8, sizeof(LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_8_STRUCT));
                break;
            default:
                break;
            }
        }
        liblte_rrc_pack_bcch_dlsch_msg(&bcch_dlsch_msg,
                                       &sys_info.sib_alloc[i].msg);
        sys_info.sib_alloc[i].pre_coder_type = LIBLTE_PHY_PRE_CODER_TYPE_TX_DIVERSITY;
        sys_info.sib_alloc[i].mod_type       = LIBLTE_PHY_MODULATION_TYPE_QPSK;
        sys_info.sib_alloc[i].rv_idx         = 0; // 36.321 section 5.3.1
        sys_info.sib_alloc[i].N_codewords    = 1;
        sys_info.sib_alloc[i].rnti           = LIBLTE_MAC_SI_RNTI;
        sys_info.sib_alloc[i].tx_mode        = 1;
    }

    // Generic parameters
    int64_iter = var_map_int64.find(LTE_FDD_ENB_PARAM_N_ANT);
    if(var_map_int64.end() != int64_iter)
    {
        sys_info.N_ant = (*int64_iter).second;
    }
    int64_iter = var_map_int64.find(LTE_FDD_ENB_PARAM_N_ID_CELL);
    if(var_map_int64.end() != int64_iter)
    {
        sys_info.N_id_cell = (*int64_iter).second;
    }
    int64_iter = var_map_int64.find(LTE_FDD_ENB_PARAM_N_ID_1);
    if(var_map_int64.end() != int64_iter)
    {
        sys_info.N_id_1 = (*int64_iter).second;
    }
    int64_iter = var_map_int64.find(LTE_FDD_ENB_PARAM_N_ID_2);
    if(var_map_int64.end() != int64_iter)
    {
        sys_info.N_id_2 = (*int64_iter).second;
    }
    int64_iter = var_map_int64.find(LTE_FDD_ENB_PARAM_N_RB_DL);
    if(var_map_int64.end() != int64_iter)
    {
        sys_info.N_rb_dl = (*int64_iter).second;
    }
    int64_iter = var_map_int64.find(LTE_FDD_ENB_PARAM_N_RB_UL);
    if(var_map_int64.end() != int64_iter)
    {
        sys_info.N_rb_ul = (*int64_iter).second;
    }
    int64_iter = var_map_int64.find(LTE_FDD_ENB_PARAM_N_SC_RB_DL);
    if(var_map_int64.end() != int64_iter)
    {
        sys_info.N_sc_rb_dl = (*int64_iter).second;
    }
    int64_iter = var_map_int64.find(LTE_FDD_ENB_PARAM_N_SC_RB_UL);
    if(var_map_int64.end() != int64_iter)
    {
        sys_info.N_sc_rb_ul = (*int64_iter).second;
    }
    sys_info.si_periodicity_T = liblte_rrc_si_periodicity_num[sys_info.sib1.sched_info[0].si_periodicity];
    sys_info.si_win_len       = liblte_rrc_si_window_length_num[sys_info.sib1.si_window_length];

    // Update all layers
    phy->update_sys_info();
    mac->update_sys_info();
    rlc->update_sys_info();
    pdcp->update_sys_info();
    rrc->update_sys_info();
    mme->update_sys_info();
}
void LTE_fdd_enb_cnfg_db::get_sys_info(LTE_FDD_ENB_SYS_INFO_STRUCT &_sys_info)
{
    memcpy(&_sys_info, &sys_info, sizeof(sys_info));
}
