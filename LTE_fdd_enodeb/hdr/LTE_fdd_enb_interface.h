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

    File: LTE_fdd_enb_interface.h

    Description: Contains all the definitions for the LTE FDD eNodeB interface.

    Revision History
    ----------    -------------    --------------------------------------------
    11/09/2013    Ben Wojtowicz    Created file
    01/18/2014    Ben Wojtowicz    Added dynamic variables and added level to
                                   debug prints.

*******************************************************************************/

#ifndef __LTE_FDD_ENB_INTERFACE_H__
#define __LTE_FDD_ENB_INTERFACE_H__

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "liblte_common.h"
#include "libtools_socket_wrap.h"
#include <boost/thread/mutex.hpp>
#include <string>

/*******************************************************************************
                              DEFINES
*******************************************************************************/

#define LTE_FDD_ENB_DEFAULT_CTRL_PORT 30000
#define LTE_FDD_ENB_DEBUG_PORT_OFFSET 1

/*******************************************************************************
                              FORWARD DECLARATIONS
*******************************************************************************/


/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/

typedef enum{
    LTE_FDD_ENB_ERROR_NONE = 0,
    LTE_FDD_ENB_ERROR_INVALID_COMMAND,
    LTE_FDD_ENB_ERROR_INVALID_PARAM,
    LTE_FDD_ENB_ERROR_OUT_OF_BOUNDS,
    LTE_FDD_ENB_ERROR_EXCEPTION,
    LTE_FDD_ENB_ERROR_ALREADY_STARTED,
    LTE_FDD_ENB_ERROR_ALREADY_STOPPED,
    LTE_FDD_ENB_ERROR_CANT_START,
    LTE_FDD_ENB_ERROR_CANT_STOP,
    LTE_FDD_ENB_ERROR_BAD_ALLOC,
    LTE_FDD_ENB_ERROR_USER_NOT_FOUND,
    LTE_FDD_ENB_ERROR_NO_FREE_C_RNTI,
    LTE_FDD_ENB_ERROR_C_RNTI_NOT_FOUND,
    LTE_FDD_ENB_ERROR_CANT_SCHEDULE,
    LTE_FDD_ENB_ERROR_VARIABLE_NOT_DYNAMIC,
    LTE_FDD_ENB_ERROR_N_ITEMS,
}LTE_FDD_ENB_ERROR_ENUM;
static const char LTE_fdd_enb_error_text[LTE_FDD_ENB_ERROR_N_ITEMS][100] = {"none",
                                                                            "invalid command",
                                                                            "invalid parameter",
                                                                            "out of bounds",
                                                                            "exception",
                                                                            "already started",
                                                                            "already stopped",
                                                                            "cant start",
                                                                            "cant stop",
                                                                            "bad alloc",
                                                                            "user not found",
                                                                            "no free C-RNTI",
                                                                            "C-RNTI not found",
                                                                            "cant schedule",
                                                                            "variable not dynamic"};

typedef enum{
    LTE_FDD_ENB_DEBUG_TYPE_ERROR = 0,
    LTE_FDD_ENB_DEBUG_TYPE_WARNING,
    LTE_FDD_ENB_DEBUG_TYPE_INFO,
    LTE_FDD_ENB_DEBUG_TYPE_DEBUG,
    LTE_FDD_ENB_DEBUG_TYPE_N_ITEMS,
}LTE_FDD_ENB_DEBUG_TYPE_ENUM;
static const char LTE_fdd_enb_debug_type_text[LTE_FDD_ENB_DEBUG_TYPE_N_ITEMS][100] = {"error",
                                                                                      "warning",
                                                                                      "info",
                                                                                      "debug"};

typedef enum{
    LTE_FDD_ENB_DEBUG_LEVEL_RADIO = 0,
    LTE_FDD_ENB_DEBUG_LEVEL_PHY,
    LTE_FDD_ENB_DEBUG_LEVEL_MAC,
    LTE_FDD_ENB_DEBUG_LEVEL_RLC,
    LTE_FDD_ENB_DEBUG_LEVEL_PDCP,
    LTE_FDD_ENB_DEBUG_LEVEL_RRC,
    LTE_FDD_ENB_DEBUG_LEVEL_MME,
    LTE_FDD_ENB_DEBUG_LEVEL_USER,
    LTE_FDD_ENB_DEBUG_LEVEL_IFACE,
    LTE_FDD_ENB_DEBUG_LEVEL_N_ITEMS,
}LTE_FDD_ENB_DEBUG_LEVEL_ENUM;
static const char LTE_fdd_enb_debug_level_text[LTE_FDD_ENB_DEBUG_LEVEL_N_ITEMS][100] = {"radio",
                                                                                        "phy",
                                                                                        "mac",
                                                                                        "rlc",
                                                                                        "pdcp",
                                                                                        "rrc",
                                                                                        "mme",
                                                                                        "user",
                                                                                        "iface"};

typedef enum{
    LTE_FDD_ENB_VAR_TYPE_DOUBLE = 0,
    LTE_FDD_ENB_VAR_TYPE_INT64,
    LTE_FDD_ENB_VAR_TYPE_HEX,
    LTE_FDD_ENB_VAR_TYPE_UINT32,
}LTE_FDD_ENB_VAR_TYPE_ENUM;

typedef enum{
    // System parameters managed by LTE_fdd_enb_cnfg_db
    LTE_FDD_ENB_PARAM_BANDWIDTH = 0,
    LTE_FDD_ENB_PARAM_FREQ_BAND,
    LTE_FDD_ENB_PARAM_DL_EARFCN,
    LTE_FDD_ENB_PARAM_UL_EARFCN,
    LTE_FDD_ENB_PARAM_N_RB_DL,
    LTE_FDD_ENB_PARAM_N_RB_UL,
    LTE_FDD_ENB_PARAM_DL_BW,
    LTE_FDD_ENB_PARAM_N_SC_RB,
    LTE_FDD_ENB_PARAM_N_ANT,
    LTE_FDD_ENB_PARAM_N_ID_CELL,
    LTE_FDD_ENB_PARAM_N_ID_2,
    LTE_FDD_ENB_PARAM_N_ID_1,
    LTE_FDD_ENB_PARAM_MCC,
    LTE_FDD_ENB_PARAM_MNC,
    LTE_FDD_ENB_PARAM_CELL_ID,
    LTE_FDD_ENB_PARAM_TRACKING_AREA_CODE,
    LTE_FDD_ENB_PARAM_Q_RX_LEV_MIN,
    LTE_FDD_ENB_PARAM_P0_NOMINAL_PUSCH,
    LTE_FDD_ENB_PARAM_P0_NOMINAL_PUCCH,
    LTE_FDD_ENB_PARAM_SIB3_PRESENT,
    LTE_FDD_ENB_PARAM_Q_HYST,
    LTE_FDD_ENB_PARAM_SIB4_PRESENT,
    LTE_FDD_ENB_PARAM_SIB5_PRESENT,
    LTE_FDD_ENB_PARAM_SIB6_PRESENT,
    LTE_FDD_ENB_PARAM_SIB7_PRESENT,
    LTE_FDD_ENB_PARAM_SIB8_PRESENT,
    LTE_FDD_ENB_PARAM_SEARCH_WIN_SIZE,
    LTE_FDD_ENB_PARAM_SYSTEM_INFO_VALUE_TAG,
    LTE_FDD_ENB_PARAM_SYSTEM_INFO_WINDOW_LENGTH,
    LTE_FDD_ENB_PARAM_PHICH_RESOURCE,
    LTE_FDD_ENB_PARAM_N_SCHED_INFO,
    LTE_FDD_ENB_PARAM_SYSTEM_INFO_PERIODICITY,
    LTE_FDD_ENB_PARAM_DEBUG_TYPE,
    LTE_FDD_ENB_PARAM_DEBUG_LEVEL,

    // Radio parameters managed by LTE_fdd_enb_radio
    LTE_FDD_ENB_PARAM_AVAILABLE_RADIOS,
    LTE_FDD_ENB_PARAM_SELECTED_RADIO_NAME,
    LTE_FDD_ENB_PARAM_SELECTED_RADIO_IDX,
    LTE_FDD_ENB_PARAM_TX_GAIN,
    LTE_FDD_ENB_PARAM_RX_GAIN,

    LTE_FDD_ENB_PARAM_N_ITEMS,
}LTE_FDD_ENB_PARAM_ENUM;
static const char lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_N_ITEMS][100] = {"bandwidth",
                                                                            "band",
                                                                            "dl_earfcn",
                                                                            "ul_earfcn",
                                                                            "n_rb_dl",
                                                                            "n_rb_ul",
                                                                            "dl_bw",
                                                                            "n_sc_rb",
                                                                            "n_ant",
                                                                            "n_id_cell",
                                                                            "n_id_2",
                                                                            "n_id_1",
                                                                            "mcc",
                                                                            "mnc",
                                                                            "cell_id",
                                                                            "tracking_area_code",
                                                                            "q_rx_lev_min",
                                                                            "p0_nominal_pusch",
                                                                            "p0_nominal_pucch",
                                                                            "sib3_present",
                                                                            "q_hyst",
                                                                            "sib4_present",
                                                                            "sib5_present",
                                                                            "sib6_present",
                                                                            "sib7_present",
                                                                            "sib8_present",
                                                                            "search_win_size",
                                                                            "system_info_value_tag",
                                                                            "system_info_window_length",
                                                                            "phich_resource",
                                                                            "n_sched_info",
                                                                            "system_info_periodicity",
                                                                            "debug_type",
                                                                            "debug_level",
                                                                            "available_radios",
                                                                            "selected_radio_name",
                                                                            "selected_radio_idx",
                                                                            "tx_gain",
                                                                            "rx_gain"};

typedef struct{
    LTE_FDD_ENB_VAR_TYPE_ENUM var_type;
    LTE_FDD_ENB_PARAM_ENUM    param;
    double                    double_l_bound;
    double                    double_u_bound;
    int64                     int64_l_bound;
    int64                     int64_u_bound;
    bool                      special_bounds;
    bool                      dynamic;
}LTE_FDD_ENB_VAR_STRUCT;

/*******************************************************************************
                              CLASS DECLARATIONS
*******************************************************************************/

class LTE_fdd_enb_interface
{
public:
    // Singleton
    static LTE_fdd_enb_interface* get_instance(void);
    static void cleanup(void);

    // Communication
    void set_ctrl_port(int16 port);
    void start_ports(void);
    void stop_ports(void);
    void send_ctrl_msg(std::string msg);
    void send_ctrl_info_msg(std::string msg);
    void send_ctrl_error_msg(LTE_FDD_ENB_ERROR_ENUM error, std::string msg);
    void send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ENUM type, LTE_FDD_ENB_DEBUG_LEVEL_ENUM level, std::string file_name, int32 line, std::string msg, ...);
    void send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ENUM type, LTE_FDD_ENB_DEBUG_LEVEL_ENUM level, std::string file_name, int32 line, LIBLTE_MSG_STRUCT *lte_msg, std::string msg, ...);
    static void handle_ctrl_msg(std::string msg);
    static void handle_ctrl_connect(void);
    static void handle_ctrl_disconnect(void);
    static void handle_ctrl_error(LIBTOOLS_SOCKET_WRAP_ERROR_ENUM err);
    static void handle_debug_msg(std::string msg);
    static void handle_debug_connect(void);
    static void handle_debug_disconnect(void);
    static void handle_debug_error(LIBTOOLS_SOCKET_WRAP_ERROR_ENUM err);
    boost::mutex          ctrl_mutex;
    boost::mutex          debug_mutex;
    libtools_socket_wrap *ctrl_socket;
    libtools_socket_wrap *debug_socket;
    int16                 ctrl_port;
    int16                 debug_port;
    static bool           ctrl_connected;
    static bool           debug_connected;

    // Get/Set
    bool get_shutdown(void);
    bool app_is_started(void);

private:
    // Singleton
    static LTE_fdd_enb_interface *instance;
    LTE_fdd_enb_interface();
    ~LTE_fdd_enb_interface();

    // Handlers
    void handle_read(std::string msg);
    void handle_write(std::string msg);
    void handle_start(void);
    void handle_stop(void);
    void handle_help(void);

    // Variables
    std::map<std::string, LTE_FDD_ENB_VAR_STRUCT> var_map;
    boost::mutex                                  start_mutex;
    uint32                                        debug_type_mask;
    uint32                                        debug_level_mask;
    bool                                          shutdown;
    bool                                          started;

    // Helpers
    LTE_FDD_ENB_ERROR_ENUM write_value(LTE_FDD_ENB_VAR_STRUCT *var, double value);
    LTE_FDD_ENB_ERROR_ENUM write_value(LTE_FDD_ENB_VAR_STRUCT *var, int64 value);
    LTE_FDD_ENB_ERROR_ENUM write_value(LTE_FDD_ENB_VAR_STRUCT *var, std::string value);
    LTE_FDD_ENB_ERROR_ENUM write_value(LTE_FDD_ENB_VAR_STRUCT *var, uint32 value);
};

#endif /* __LTE_FDD_ENB_INTERFACE_H__ */
