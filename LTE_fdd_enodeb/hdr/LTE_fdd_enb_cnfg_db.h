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

    File: LTE_fdd_enb_cnfg_db.h

    Description: Contains all the definitions for the LTE FDD eNodeB
                 configuration database.

    Revision History
    ----------    -------------    --------------------------------------------
    11/09/2013    Ben Wojtowicz    Created file
    01/18/2014    Ben Wojtowicz    Added set/get routines for uint32 values.
    03/26/2014    Ben Wojtowicz    Using the latest LTE library.

*******************************************************************************/

#ifndef __LTE_FDD_ENB_CNFG_DB_H__
#define __LTE_FDD_ENB_CNFG_DB_H__

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_enb_interface.h"
#include "liblte_rrc.h"
#include "liblte_phy.h"
#include <string>
#include <map>

/*******************************************************************************
                              DEFINES
*******************************************************************************/


/*******************************************************************************
                              FORWARD DECLARATIONS
*******************************************************************************/


/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/

typedef struct{
    LIBLTE_RRC_MIB_STRUCT                   mib;
    LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1_STRUCT sib1;
    LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_2_STRUCT sib2;
    LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_3_STRUCT sib3;
    LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_4_STRUCT sib4;
    LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_5_STRUCT sib5;
    LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_6_STRUCT sib6;
    LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_7_STRUCT sib7;
    LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_8_STRUCT sib8;
    LIBLTE_PHY_ALLOCATION_STRUCT            sib1_alloc;
    LIBLTE_PHY_ALLOCATION_STRUCT            sib_alloc[4];
    uint32                                  N_ant;
    uint32                                  N_id_cell;
    uint32                                  N_id_1;
    uint32                                  N_id_2;
    uint32                                  N_rb_dl;
    uint32                                  N_rb_ul;
    uint32                                  N_sc_rb_dl;
    uint32                                  N_sc_rb_ul;
    uint32                                  si_periodicity_T;
    uint32                                  si_win_len;
    bool                                    sib3_present;
    bool                                    sib4_present;
    bool                                    sib5_present;
    bool                                    sib6_present;
    bool                                    sib7_present;
    bool                                    sib8_present;
}LTE_FDD_ENB_SYS_INFO_STRUCT;

/*******************************************************************************
                              CLASS DECLARATIONS
*******************************************************************************/

class LTE_fdd_enb_cnfg_db
{
public:
    // Singleton
    static LTE_fdd_enb_cnfg_db* get_instance(void);
    static void cleanup(void);

    // Parameter Gets/Sets
    LTE_FDD_ENB_ERROR_ENUM set_param(LTE_FDD_ENB_PARAM_ENUM param, int64 value);
    LTE_FDD_ENB_ERROR_ENUM set_param(LTE_FDD_ENB_PARAM_ENUM param, double value);
    LTE_FDD_ENB_ERROR_ENUM set_param(LTE_FDD_ENB_PARAM_ENUM param, std::string value);
    LTE_FDD_ENB_ERROR_ENUM set_param(LTE_FDD_ENB_PARAM_ENUM param, uint32 value);
    LTE_FDD_ENB_ERROR_ENUM get_param(LTE_FDD_ENB_PARAM_ENUM param, int64 &value);
    LTE_FDD_ENB_ERROR_ENUM get_param(LTE_FDD_ENB_PARAM_ENUM param, double &value);
    LTE_FDD_ENB_ERROR_ENUM get_param(LTE_FDD_ENB_PARAM_ENUM param, std::string &value);
    LTE_FDD_ENB_ERROR_ENUM get_param(LTE_FDD_ENB_PARAM_ENUM param, uint32 &value);

    // MIB/SIB Construction
    void construct_sys_info(void);
    void get_sys_info(LTE_FDD_ENB_SYS_INFO_STRUCT &_sys_info);

private:
    // Singleton
    static LTE_fdd_enb_cnfg_db *instance;
    LTE_fdd_enb_cnfg_db();
    ~LTE_fdd_enb_cnfg_db();

    // Parameters
    std::map<LTE_FDD_ENB_PARAM_ENUM, double> var_map_double;
    std::map<LTE_FDD_ENB_PARAM_ENUM, int64>  var_map_int64;
    std::map<LTE_FDD_ENB_PARAM_ENUM, uint32> var_map_uint32;

    // System information
    LTE_FDD_ENB_SYS_INFO_STRUCT sys_info;
};

#endif /* __LTE_FDD_ENB_CNFG_DB_H__ */
