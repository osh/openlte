#line 2 "LTE_fdd_enb_interface.cc" // Make __FILE__ omit the path
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

    File: LTE_fdd_enb_interface.cc

    Description: Contains all the implementations for the LTE FDD eNodeB
                 interface.

    Revision History
    ----------    -------------    --------------------------------------------
    11/09/2013    Ben Wojtowicz    Created file
    01/18/2014    Ben Wojtowicz    Added dynamic variable support, added level
                                   to debug prints, fixed usec time in debug
                                   prints, and added uint32 variables.
    03/26/2014    Ben Wojtowicz    Added message printing.
    05/04/2014    Ben Wojtowicz    Added PCAP support.
    06/15/2014    Ben Wojtowicz    Added  ... support for info messages and
                                   using the latest LTE library.

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_enb_interface.h"
#include "LTE_fdd_enb_cnfg_db.h"
#include "LTE_fdd_enb_hss.h"
#include "LTE_fdd_enb_mme.h"
#include "LTE_fdd_enb_rrc.h"
#include "LTE_fdd_enb_pdcp.h"
#include "LTE_fdd_enb_rlc.h"
#include "LTE_fdd_enb_mac.h"
#include "LTE_fdd_enb_phy.h"
#include "LTE_fdd_enb_radio.h"
#include "liblte_interface.h"
#include <boost/lexical_cast.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <iomanip>
#include <arpa/inet.h>

/*******************************************************************************
                              DEFINES
*******************************************************************************/


/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/


/*******************************************************************************
                              GLOBAL VARIABLES
*******************************************************************************/

LTE_fdd_enb_interface* LTE_fdd_enb_interface::instance = NULL;
boost::mutex           interface_instance_mutex;
boost::mutex           ctrl_connect_mutex;
boost::mutex           debug_connect_mutex;
bool                   LTE_fdd_enb_interface::ctrl_connected  = false;
bool                   LTE_fdd_enb_interface::debug_connected = false;

/*******************************************************************************
                              CLASS IMPLEMENTATIONS
*******************************************************************************/

/*******************/
/*    Singleton    */
/*******************/
LTE_fdd_enb_interface* LTE_fdd_enb_interface::get_instance(void)
{
    boost::mutex::scoped_lock lock(interface_instance_mutex);

    if(NULL == instance)
    {
        instance = new LTE_fdd_enb_interface();
    }

    return(instance);
}
void LTE_fdd_enb_interface::cleanup(void)
{
    boost::mutex::scoped_lock lock(interface_instance_mutex);

    if(NULL != instance)
    {
        delete instance;
        instance = NULL;
    }
}

/********************************/
/*    Constructor/Destructor    */
/********************************/
LTE_fdd_enb_interface::LTE_fdd_enb_interface()
{
    uint32 i;

    // Communication
    ctrl_socket     = NULL;
    debug_socket    = NULL;
    ctrl_port       = LTE_FDD_ENB_DEFAULT_CTRL_PORT;
    debug_port      = ctrl_port + LTE_FDD_ENB_DEBUG_PORT_OFFSET;
    ctrl_connected  = false;
    debug_connected = false;

    // Variables
    var_map[lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_BANDWIDTH]]          = (LTE_FDD_ENB_VAR_STRUCT){LTE_FDD_ENB_VAR_TYPE_DOUBLE, LTE_FDD_ENB_PARAM_BANDWIDTH, 0, 0, 0, 0, true, false};
    var_map[lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_FREQ_BAND]]          = (LTE_FDD_ENB_VAR_STRUCT){LTE_FDD_ENB_VAR_TYPE_INT64, LTE_FDD_ENB_PARAM_FREQ_BAND, 0, 0, 0, 0, true, true};
    var_map[lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_DL_EARFCN]]          = (LTE_FDD_ENB_VAR_STRUCT){LTE_FDD_ENB_VAR_TYPE_INT64, LTE_FDD_ENB_PARAM_DL_EARFCN, 0, 0, 0, 0, true, true};
    var_map[lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_N_ANT]]              = (LTE_FDD_ENB_VAR_STRUCT){LTE_FDD_ENB_VAR_TYPE_INT64, LTE_FDD_ENB_PARAM_N_ANT, 0, 0, 0, 0, true, false};
    var_map[lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_N_ID_CELL]]          = (LTE_FDD_ENB_VAR_STRUCT){LTE_FDD_ENB_VAR_TYPE_INT64, LTE_FDD_ENB_PARAM_N_ID_CELL, 0, 0, 0, 503, false, false};
    var_map[lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_MCC]]                = (LTE_FDD_ENB_VAR_STRUCT){LTE_FDD_ENB_VAR_TYPE_HEX, LTE_FDD_ENB_PARAM_MCC, 0, 0, 0, 0, true, true};
    var_map[lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_MNC]]                = (LTE_FDD_ENB_VAR_STRUCT){LTE_FDD_ENB_VAR_TYPE_HEX, LTE_FDD_ENB_PARAM_MNC, 0, 0, 0, 0, true, true};
    var_map[lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_CELL_ID]]            = (LTE_FDD_ENB_VAR_STRUCT){LTE_FDD_ENB_VAR_TYPE_INT64, LTE_FDD_ENB_PARAM_CELL_ID, 0, 0, 0, 268435455, false, true};
    var_map[lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_TRACKING_AREA_CODE]] = (LTE_FDD_ENB_VAR_STRUCT){LTE_FDD_ENB_VAR_TYPE_INT64, LTE_FDD_ENB_PARAM_TRACKING_AREA_CODE, 0, 0, 0, 65535, false, true};
    var_map[lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_Q_RX_LEV_MIN]]       = (LTE_FDD_ENB_VAR_STRUCT){LTE_FDD_ENB_VAR_TYPE_INT64, LTE_FDD_ENB_PARAM_Q_RX_LEV_MIN, 0, 0, -140, -44, false, true};
    var_map[lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_P0_NOMINAL_PUSCH]]   = (LTE_FDD_ENB_VAR_STRUCT){LTE_FDD_ENB_VAR_TYPE_INT64, LTE_FDD_ENB_PARAM_P0_NOMINAL_PUSCH, 0, 0, -126, 24, false, true};
    var_map[lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_P0_NOMINAL_PUCCH]]   = (LTE_FDD_ENB_VAR_STRUCT){LTE_FDD_ENB_VAR_TYPE_INT64, LTE_FDD_ENB_PARAM_P0_NOMINAL_PUCCH, 0, 0, -127, -96, false, true};
    var_map[lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_SIB3_PRESENT]]       = (LTE_FDD_ENB_VAR_STRUCT){LTE_FDD_ENB_VAR_TYPE_INT64, LTE_FDD_ENB_PARAM_SIB3_PRESENT, 0, 0, 0, 1, false, true};
    var_map[lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_Q_HYST]]             = (LTE_FDD_ENB_VAR_STRUCT){LTE_FDD_ENB_VAR_TYPE_INT64, LTE_FDD_ENB_PARAM_Q_HYST, 0, 0, 0, 0, true, true};
    var_map[lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_SIB4_PRESENT]]       = (LTE_FDD_ENB_VAR_STRUCT){LTE_FDD_ENB_VAR_TYPE_INT64, LTE_FDD_ENB_PARAM_SIB4_PRESENT, 0, 0, 0, 1, false, true};
    var_map[lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_SIB5_PRESENT]]       = (LTE_FDD_ENB_VAR_STRUCT){LTE_FDD_ENB_VAR_TYPE_INT64, LTE_FDD_ENB_PARAM_SIB5_PRESENT, 0, 0, 0, 1, false, true};
    var_map[lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_SIB6_PRESENT]]       = (LTE_FDD_ENB_VAR_STRUCT){LTE_FDD_ENB_VAR_TYPE_INT64, LTE_FDD_ENB_PARAM_SIB6_PRESENT, 0, 0, 0, 1, false, true};
    var_map[lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_SIB7_PRESENT]]       = (LTE_FDD_ENB_VAR_STRUCT){LTE_FDD_ENB_VAR_TYPE_INT64, LTE_FDD_ENB_PARAM_SIB7_PRESENT, 0, 0, 0, 1, false, true};
    var_map[lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_SIB8_PRESENT]]       = (LTE_FDD_ENB_VAR_STRUCT){LTE_FDD_ENB_VAR_TYPE_INT64, LTE_FDD_ENB_PARAM_SIB8_PRESENT, 0, 0, 0, 1, false, true};
    var_map[lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_SEARCH_WIN_SIZE]]    = (LTE_FDD_ENB_VAR_STRUCT){LTE_FDD_ENB_VAR_TYPE_INT64, LTE_FDD_ENB_PARAM_SEARCH_WIN_SIZE, 0, 0, 0, 15, false, true};
    var_map[lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_DEBUG_TYPE]]         = (LTE_FDD_ENB_VAR_STRUCT){LTE_FDD_ENB_VAR_TYPE_UINT32, LTE_FDD_ENB_PARAM_DEBUG_TYPE, 0, 0, 0, 0, true, true};
    var_map[lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_DEBUG_LEVEL]]        = (LTE_FDD_ENB_VAR_STRUCT){LTE_FDD_ENB_VAR_TYPE_UINT32, LTE_FDD_ENB_PARAM_DEBUG_LEVEL, 0, 0, 0, 0, true, true};
    var_map[lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_ENABLE_PCAP]]        = (LTE_FDD_ENB_VAR_STRUCT){LTE_FDD_ENB_VAR_TYPE_INT64, LTE_FDD_ENB_PARAM_ENABLE_PCAP, 0, 0, 0, 1, false, true};

    debug_type_mask = 0;
    for(i=0; i<LTE_FDD_ENB_DEBUG_TYPE_N_ITEMS; i++)
    {
        debug_type_mask |= 1 << i;
    }
    debug_level_mask = 0;
    for(i=0; i<LTE_FDD_ENB_DEBUG_LEVEL_N_ITEMS; i++)
    {
        debug_level_mask |= 1 << i;
    }
    open_pcap_fd();
    shutdown = false;
    started  = false;
}
LTE_fdd_enb_interface::~LTE_fdd_enb_interface()
{
    stop_ports();

    fclose(pcap_fd);
}

/***********************/
/*    Communication    */
/***********************/
void LTE_fdd_enb_interface::set_ctrl_port(int16 port)
{
    boost::mutex::scoped_lock c_lock(ctrl_connect_mutex);
    boost::mutex::scoped_lock d_lock(debug_connect_mutex);

    if(!ctrl_connected)
    {
        ctrl_port = port;
    }
    if(!debug_connected)
    {
        debug_port = ctrl_port + LTE_FDD_ENB_DEBUG_PORT_OFFSET;
    }
}
void LTE_fdd_enb_interface::start_ports(void)
{
    boost::mutex::scoped_lock       c_lock(ctrl_mutex);
    boost::mutex::scoped_lock       d_lock(debug_mutex);
    LIBTOOLS_SOCKET_WRAP_ERROR_ENUM error;

    if(NULL == debug_socket)
    {
        debug_socket = new libtools_socket_wrap(NULL,
                                                debug_port,
                                                LIBTOOLS_SOCKET_WRAP_TYPE_SERVER,
                                                &handle_debug_msg,
                                                &handle_debug_connect,
                                                &handle_debug_disconnect,
                                                &handle_debug_error,
                                                &error);
        if(LIBTOOLS_SOCKET_WRAP_SUCCESS != error)
        {
            printf("Couldn't open debug_socket %s\n", libtools_socket_wrap_error_text[error]);
            debug_socket = NULL;
        }
    }
    if(NULL == ctrl_socket)
    {
        ctrl_socket = new libtools_socket_wrap(NULL,
                                               ctrl_port,
                                               LIBTOOLS_SOCKET_WRAP_TYPE_SERVER,
                                               &handle_ctrl_msg,
                                               &handle_ctrl_connect,
                                               &handle_ctrl_disconnect,
                                               &handle_ctrl_error,
                                               &error);
        if(LIBTOOLS_SOCKET_WRAP_SUCCESS != error)
        {
            send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                           LTE_FDD_ENB_DEBUG_LEVEL_IFACE,
                           __FILE__,
                           __LINE__,
                           "Couldn't open ctrl_socket %s",
                           libtools_socket_wrap_error_text[error]);
            ctrl_socket = NULL;
        }
    }
}
void LTE_fdd_enb_interface::stop_ports(void)
{
    boost::mutex::scoped_lock c_lock(ctrl_mutex);
    boost::mutex::scoped_lock d_lock(debug_mutex);

    if(NULL != ctrl_socket)
    {
        delete ctrl_socket;
        ctrl_socket = NULL;
    }
    if(NULL != debug_socket)
    {
        delete debug_socket;
        debug_socket = NULL;
    }
}
void LTE_fdd_enb_interface::send_ctrl_msg(std::string msg)
{
    boost::mutex::scoped_lock lock(ctrl_connect_mutex);
    std::string               tmp_msg;

    if(ctrl_connected)
    {
        tmp_msg  = msg;
        tmp_msg += "\n";
        ctrl_socket->send(tmp_msg);
    }
}
void LTE_fdd_enb_interface::send_ctrl_info_msg(std::string msg,
                                               ...)
{
    boost::mutex::scoped_lock  lock(ctrl_connect_mutex);
    std::string                tmp_msg;
    va_list                    args;
    char                      *args_msg;

    if(ctrl_connected)
    {
        tmp_msg = "info ";
        va_start(args, msg);
        if(-1 != vasprintf(&args_msg, msg.c_str(), args))
        {
            tmp_msg += args_msg;
        }
        tmp_msg += "\n";

        // Cleanup the variable argument string
        free(args_msg);

        ctrl_socket->send(tmp_msg);
    }
}
void LTE_fdd_enb_interface::send_ctrl_error_msg(LTE_FDD_ENB_ERROR_ENUM error,
                                                std::string            msg)
{
    boost::mutex::scoped_lock lock(ctrl_connect_mutex);
    std::string               tmp_msg;

    if(ctrl_connected)
    {
        if(LTE_FDD_ENB_ERROR_NONE == error)
        {
            tmp_msg = "ok ";
        }else{
            tmp_msg  = "fail \"";
            tmp_msg += LTE_fdd_enb_error_text[error];
            tmp_msg += "\"";
        }
        tmp_msg += msg;
        tmp_msg += "\n";

        ctrl_socket->send(tmp_msg);
    }
}
void LTE_fdd_enb_interface::send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ENUM  type,
                                           LTE_FDD_ENB_DEBUG_LEVEL_ENUM level,
                                           std::string                  file_name,
                                           int32                        line,
                                           std::string                  msg,
                                           ...)
{
    boost::mutex::scoped_lock  lock(debug_connect_mutex);
    std::string                tmp_msg;
    std::stringstream          tmp_ss;
    va_list                    args;
    struct timeval             time;
    struct timezone            time_zone;
    char                      *args_msg;

    if(debug_connected                 &&
       (debug_type_mask & (1 << type)) &&
       (debug_level_mask & (1 << level)))
    {
        // Format the output string
        gettimeofday(&time, &time_zone);
        tmp_msg  = boost::lexical_cast<std::string>(time.tv_sec) + ".";
        tmp_ss  << std::setw(6) << std::setfill('0') << time.tv_usec;
        tmp_msg += tmp_ss.str() + " ";
        tmp_msg += LTE_fdd_enb_debug_type_text[type];
        tmp_msg += " ";
        tmp_msg += LTE_fdd_enb_debug_level_text[level];
        tmp_msg += " ";
        tmp_msg += file_name.c_str();
        tmp_msg += " ";
        tmp_msg += boost::lexical_cast<std::string>(line);
        tmp_msg += " ";
        va_start(args, msg);
        if(-1 != vasprintf(&args_msg, msg.c_str(), args))
        {
            tmp_msg += args_msg;
        }
        tmp_msg += "\n";

        // Cleanup the variable argument string
        free(args_msg);

        debug_socket->send(tmp_msg);
    }
}
void LTE_fdd_enb_interface::send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ENUM   type,
                                           LTE_FDD_ENB_DEBUG_LEVEL_ENUM  level,
                                           std::string                   file_name,
                                           int32                         line,
                                           LIBLTE_BIT_MSG_STRUCT        *lte_msg,
                                           std::string                   msg,
                                           ...)
{
    boost::mutex::scoped_lock  lock(debug_connect_mutex);
    std::string                tmp_msg;
    std::stringstream          tmp_ss;
    va_list                    args;
    struct timeval             time;
    struct timezone            time_zone;
    uint32                     i;
    uint32                     hex_val;
    char                      *args_msg;

    if(debug_connected                 &&
       (debug_type_mask & (1 << type)) &&
       (debug_level_mask & (1 << level)))
    {
        // Format the output string
        gettimeofday(&time, &time_zone);
        tmp_msg  = boost::lexical_cast<std::string>(time.tv_sec) + ".";
        tmp_ss  << std::setw(6) << std::setfill('0') << time.tv_usec;
        tmp_msg += tmp_ss.str() + " ";
        tmp_msg += LTE_fdd_enb_debug_type_text[type];
        tmp_msg += " ";
        tmp_msg += LTE_fdd_enb_debug_level_text[level];
        tmp_msg += " ";
        tmp_msg += file_name.c_str();
        tmp_msg += " ";
        tmp_msg += boost::lexical_cast<std::string>(line);
        tmp_msg += " ";
        va_start(args, msg);
        if(-1 != vasprintf(&args_msg, msg.c_str(), args))
        {
            tmp_msg += args_msg;
        }
        tmp_msg += " ";
        hex_val  = 0;
        for(i=0; i<lte_msg->N_bits; i++)
        {
            hex_val <<= 1;
            hex_val  |= lte_msg->msg[i];
            if((i % 4) == 3)
            {
                if(hex_val < 0xA)
                {
                    tmp_msg += (char)(hex_val + '0');
                }else{
                    tmp_msg += (char)((hex_val-0xA) + 'A');
                }
                hex_val = 0;
            }
        }
        if((lte_msg->N_bits % 4) != 0)
        {
            for(i=0; i<4-(lte_msg->N_bits % 4); i++)
            {
                hex_val <<= 1;
            }
            if(hex_val < 0xA)
            {
                tmp_msg += (char)(hex_val + '0');
            }else{
                tmp_msg += (char)((hex_val-0xA) + 'A');
            }
        }
        tmp_msg += "\n";

        // Cleanup the variable argument string
        free(args_msg);

        debug_socket->send(tmp_msg);
    }
}
void LTE_fdd_enb_interface::send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ENUM   type,
                                           LTE_FDD_ENB_DEBUG_LEVEL_ENUM  level,
                                           std::string                   file_name,
                                           int32                         line,
                                           LIBLTE_BYTE_MSG_STRUCT       *lte_msg,
                                           std::string                   msg,
                                           ...)
{
    boost::mutex::scoped_lock  lock(debug_connect_mutex);
    std::string                tmp_msg;
    std::stringstream          tmp_ss;
    va_list                    args;
    struct timeval             time;
    struct timezone            time_zone;
    uint32                     i;
    uint32                     hex_val;
    char                      *args_msg;

    if(debug_connected                 &&
       (debug_type_mask & (1 << type)) &&
       (debug_level_mask & (1 << level)))
    {
        // Format the output string
        gettimeofday(&time, &time_zone);
        tmp_msg  = boost::lexical_cast<std::string>(time.tv_sec) + ".";
        tmp_ss  << std::setw(6) << std::setfill('0') << time.tv_usec;
        tmp_msg += tmp_ss.str() + " ";
        tmp_msg += LTE_fdd_enb_debug_type_text[type];
        tmp_msg += " ";
        tmp_msg += LTE_fdd_enb_debug_level_text[level];
        tmp_msg += " ";
        tmp_msg += file_name.c_str();
        tmp_msg += " ";
        tmp_msg += boost::lexical_cast<std::string>(line);
        tmp_msg += " ";
        va_start(args, msg);
        if(-1 != vasprintf(&args_msg, msg.c_str(), args))
        {
            tmp_msg += args_msg;
        }
        tmp_msg += " ";
        for(i=0; i<lte_msg->N_bytes; i++)
        {
            hex_val = (lte_msg->msg[i] >> 4) & 0xF;
            if(hex_val < 0xA)
            {
                tmp_msg += (char)(hex_val + '0');
            }else{
                tmp_msg += (char)((hex_val-0xA) + 'A');
            }
            hex_val = lte_msg->msg[i] & 0xF;
            if(hex_val < 0xA)
            {
                tmp_msg += (char)(hex_val + '0');
            }else{
                tmp_msg += (char)((hex_val-0xA) + 'A');
            }
        }
        tmp_msg += "\n";

        // Cleanup the variable argument string
        free(args_msg);

        debug_socket->send(tmp_msg);
    }
}
void LTE_fdd_enb_interface::open_pcap_fd(void)
{
    uint32 magic_number  = 0xa1b2c3d4;
    uint32 timezone      = 0;
    uint32 sigfigs       = 0;
    uint32 snap_len      = (LIBLTE_MAX_MSG_SIZE/4);
    uint32 dlt           = 147;
    uint16 major_version = 2;
    uint16 minor_version = 4;

    pcap_fd = fopen("/tmp/LTE_fdd_enodeb.pcap", "w");

    fwrite(&magic_number,  sizeof(magic_number),  1, pcap_fd);
    fwrite(&major_version, sizeof(major_version), 1, pcap_fd);
    fwrite(&minor_version, sizeof(minor_version), 1, pcap_fd);
    fwrite(&timezone,      sizeof(timezone),      1, pcap_fd);
    fwrite(&sigfigs,       sizeof(sigfigs),       1, pcap_fd);
    fwrite(&snap_len,      sizeof(snap_len),      1, pcap_fd);
    fwrite(&dlt,           sizeof(dlt),           1, pcap_fd);
}
void LTE_fdd_enb_interface::send_pcap_msg(LTE_FDD_ENB_PCAP_DIRECTION_ENUM  dir,
                                          uint32                           rnti,
                                          uint32                           current_tti,
                                          uint8                           *msg,
                                          uint32                           N_bits)
{
    LTE_fdd_enb_cnfg_db *cnfg_db = LTE_fdd_enb_cnfg_db::get_instance();
    struct timeval       time;
    struct timezone      time_zone;
    int64                enable_pcap;
    uint32               i;
    uint32               idx;
    uint32               length;
    uint16               tmp;
    uint8                pcap_c_hdr[15];
    uint8                pcap_msg[LIBLTE_MAX_MSG_SIZE/8];

    cnfg_db->get_param(LTE_FDD_ENB_PARAM_ENABLE_PCAP, enable_pcap);

    if(enable_pcap)
    {
        // Get approximate time stamp
        gettimeofday(&time, &time_zone);

        // Radio Type
        pcap_c_hdr[0] = 1;

        // Direction
        pcap_c_hdr[1] = dir;

        // RNTI Type
        if(0xFFFFFFFF == rnti)
        {
            pcap_c_hdr[2] = 0;
        }else if(LIBLTE_MAC_P_RNTI == rnti){
            pcap_c_hdr[2] = 1;
        }else if(LIBLTE_MAC_RA_RNTI_START <= rnti &&
                 LIBLTE_MAC_RA_RNTI_END   >= rnti){
            pcap_c_hdr[2] = 2;
        }else if(LIBLTE_MAC_SI_RNTI == rnti){
            pcap_c_hdr[2] = 4;
        }else if(LIBLTE_MAC_M_RNTI == rnti){
            pcap_c_hdr[2] = 6;
        }else{
            pcap_c_hdr[2] = 3;
        }

        // RNTI Tag and RNTI
        pcap_c_hdr[3] = 2;
        tmp           = htons((uint16)rnti);
        memcpy(&pcap_c_hdr[4], &tmp, sizeof(uint16));

        // UEID Tag and UEID
        pcap_c_hdr[6] = 3;
        pcap_c_hdr[7] = 0;
        pcap_c_hdr[8] = 0;

        // SUBFN Tag and SUBFN
        pcap_c_hdr[9] = 4;
        tmp           = htons((uint16)(current_tti%10));
        memcpy(&pcap_c_hdr[10], &tmp, sizeof(uint16));

        // CRC Status Tag and CRC Status
        pcap_c_hdr[12] = 7;
        pcap_c_hdr[13] = 1;

        // Payload Tag
        pcap_c_hdr[14] = 1;

        // Payload
        idx           = 0;
        pcap_msg[idx] = 0;
        for(i=0; i<N_bits; i++)
        {
            pcap_msg[idx] <<= 1;
            pcap_msg[idx]  |= msg[i];
            if((i % 8) == 7)
            {
                idx++;
                pcap_msg[idx] = 0;
            }
        }

        // Total Length
        length = 15 + idx;

        // Write Data
        fwrite(&time.tv_sec,  sizeof(uint32), 1,   pcap_fd);
        fwrite(&time.tv_usec, sizeof(uint32), 1,   pcap_fd);
        fwrite(&length,       sizeof(uint32), 1,   pcap_fd);
        fwrite(&length,       sizeof(uint32), 1,   pcap_fd);
        fwrite(pcap_c_hdr,    sizeof(uint8),  15,  pcap_fd);
        fwrite(pcap_msg,      sizeof(uint8),  idx, pcap_fd);
    }
}
void LTE_fdd_enb_interface::handle_ctrl_msg(std::string msg)
{
    LTE_fdd_enb_interface *interface = LTE_fdd_enb_interface::get_instance();
    LTE_fdd_enb_cnfg_db   *cnfg_db   = LTE_fdd_enb_cnfg_db::get_instance();
    LTE_fdd_enb_radio     *radio     = LTE_fdd_enb_radio::get_instance();

    if(std::string::npos != msg.find("read"))
    {
        interface->handle_read(msg.substr(msg.find("read")+sizeof("read"), std::string::npos));
    }else if(std::string::npos != msg.find("write")){
        interface->handle_write(msg.substr(msg.find("write")+sizeof("write"), std::string::npos));
    }else if(std::string::npos != msg.find("start")){
        interface->handle_start();
    }else if(std::string::npos != msg.find("stop")){
        interface->handle_stop();
    }else if(std::string::npos != msg.find("shutdown")){
        interface->shutdown = true;
        if(radio->is_started())
        {
            interface->handle_stop();
        }else{
            interface->send_ctrl_error_msg(LTE_FDD_ENB_ERROR_NONE, "");
        }
    }else if(std::string::npos != msg.find("construct_si")){
        cnfg_db->construct_sys_info();
        interface->send_ctrl_error_msg(LTE_FDD_ENB_ERROR_NONE, "");
    }else if(std::string::npos != msg.find("help")){
        interface->handle_help();
    }else{
        interface->send_ctrl_error_msg(LTE_FDD_ENB_ERROR_INVALID_COMMAND, "");
    }
}
void LTE_fdd_enb_interface::handle_ctrl_connect(void)
{
    ctrl_connect_mutex.lock();
    LTE_fdd_enb_interface::ctrl_connected = true;
    ctrl_connect_mutex.unlock();

    LTE_fdd_enb_interface *interface = LTE_fdd_enb_interface::get_instance();

    interface->send_ctrl_msg("*** LTE FDD ENB ***");
    interface->send_ctrl_msg("Type help to see a list of commands");
}
void LTE_fdd_enb_interface::handle_ctrl_disconnect(void)
{
    boost::mutex::scoped_lock lock(ctrl_connect_mutex);

    LTE_fdd_enb_interface::ctrl_connected = false;
}
void LTE_fdd_enb_interface::handle_ctrl_error(LIBTOOLS_SOCKET_WRAP_ERROR_ENUM err)
{
    LTE_fdd_enb_interface *interface = LTE_fdd_enb_interface::get_instance();

    interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_ERROR,
                              LTE_FDD_ENB_DEBUG_LEVEL_IFACE,
                              __FILE__,
                              __LINE__,
                              "ctrl_socket error %s",
                              libtools_socket_wrap_error_text[err]);
    assert(0);
}
void LTE_fdd_enb_interface::handle_debug_msg(std::string msg)
{
    // No messages to handle
}
void LTE_fdd_enb_interface::handle_debug_connect(void)
{
    debug_connect_mutex.lock();
    LTE_fdd_enb_interface::debug_connected = true;
    debug_connect_mutex.unlock();

    LTE_fdd_enb_interface *interface = LTE_fdd_enb_interface::get_instance();

    interface->send_debug_msg(LTE_FDD_ENB_DEBUG_TYPE_INFO,
                              LTE_FDD_ENB_DEBUG_LEVEL_IFACE,
                              __FILE__,
                              __LINE__,
                              "*** LTE FDD ENB DEBUG INTERFACE ***");
}
void LTE_fdd_enb_interface::handle_debug_disconnect(void)
{
    boost::mutex::scoped_lock lock(debug_connect_mutex);

    LTE_fdd_enb_interface::debug_connected = false;
}
void LTE_fdd_enb_interface::handle_debug_error(LIBTOOLS_SOCKET_WRAP_ERROR_ENUM err)
{
    printf("debug_socket error %s\n", libtools_socket_wrap_error_text[err]);
    assert(0);
}

/******************/
/*    Handlers    */
/******************/
void LTE_fdd_enb_interface::handle_read(std::string msg)
{
    LTE_fdd_enb_cnfg_db                                     *cnfg_db = LTE_fdd_enb_cnfg_db::get_instance();
    LTE_fdd_enb_radio                                       *radio   = LTE_fdd_enb_radio::get_instance();
    std::map<std::string, LTE_FDD_ENB_VAR_STRUCT>::iterator  iter    = var_map.find(msg);
    std::string                                              tmp_str;
    std::string                                              s_value;
    LTE_FDD_ENB_AVAILABLE_RADIOS_STRUCT                      avail_radios   = radio->get_available_radios();
    LTE_FDD_ENB_RADIO_STRUCT                                 selected_radio = radio->get_selected_radio();
    double                                                   d_value;
    int64                                                    i_value;
    uint32                                                   u_value;
    uint32                                                   i;

    try
    {
        if(var_map.end() != iter)
        {
            // Handle all system parameters
            switch((*iter).second.var_type)
            {
            case LTE_FDD_ENB_VAR_TYPE_DOUBLE:
                cnfg_db->get_param((*iter).second.param, d_value);
                send_ctrl_error_msg(LTE_FDD_ENB_ERROR_NONE, boost::lexical_cast<std::string>(d_value));
                break;
            case LTE_FDD_ENB_VAR_TYPE_INT64:
                cnfg_db->get_param((*iter).second.param, i_value);
                if(LTE_FDD_ENB_PARAM_FREQ_BAND == (*iter).second.param)
                {
                    send_ctrl_error_msg(LTE_FDD_ENB_ERROR_NONE, liblte_interface_band_text[i_value]);
                }else{
                    send_ctrl_error_msg(LTE_FDD_ENB_ERROR_NONE, boost::lexical_cast<std::string>(i_value));
                }
                break;
            case LTE_FDD_ENB_VAR_TYPE_HEX:
                cnfg_db->get_param((*iter).second.param, s_value);
                send_ctrl_error_msg(LTE_FDD_ENB_ERROR_NONE, s_value);
                break;
            case LTE_FDD_ENB_VAR_TYPE_UINT32:
                cnfg_db->get_param((*iter).second.param, u_value);
                if(LTE_FDD_ENB_PARAM_DEBUG_TYPE == (*iter).second.param)
                {
                    for(i=0; i<LTE_FDD_ENB_DEBUG_TYPE_N_ITEMS; i++)
                    {
                        if((u_value & (1 << i)))
                        {
                            tmp_str += LTE_fdd_enb_debug_type_text[i];
                            tmp_str += " ";
                        }
                    }
                    send_ctrl_error_msg(LTE_FDD_ENB_ERROR_NONE, tmp_str);
                }else if(LTE_FDD_ENB_PARAM_DEBUG_LEVEL == (*iter).second.param){
                    for(i=0; i<LTE_FDD_ENB_DEBUG_LEVEL_N_ITEMS; i++)
                    {
                        if((u_value & (1 << i)))
                        {
                            tmp_str += LTE_fdd_enb_debug_level_text[i];
                            tmp_str += " ";
                        }
                    }
                    send_ctrl_error_msg(LTE_FDD_ENB_ERROR_NONE, tmp_str);
                }else{
                    send_ctrl_error_msg(LTE_FDD_ENB_ERROR_NONE, boost::lexical_cast<std::string>(u_value));
                }
                break;
            default:
                send_ctrl_error_msg(LTE_FDD_ENB_ERROR_INVALID_PARAM, "");
                break;
            }
        }else{
            // Handle all radio parameters
            if(std::string::npos != msg.find(lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_AVAILABLE_RADIOS]))
            {
                send_ctrl_error_msg(LTE_FDD_ENB_ERROR_NONE, boost::lexical_cast<std::string>(avail_radios.num_radios));
                for(i=0; i<avail_radios.num_radios; i++)
                {
                    tmp_str  = boost::lexical_cast<std::string>(i);
                    tmp_str += ":";
                    tmp_str += avail_radios.radio[i].name;
                    send_ctrl_msg(tmp_str);
                }
            }else if(std::string::npos != msg.find(lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_SELECTED_RADIO_NAME])){
                send_ctrl_error_msg(LTE_FDD_ENB_ERROR_NONE, selected_radio.name);
            }else if(std::string::npos != msg.find(lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_SELECTED_RADIO_IDX])){
                send_ctrl_error_msg(LTE_FDD_ENB_ERROR_NONE, boost::lexical_cast<std::string>(radio->get_selected_radio_idx()));
            }else if(std::string::npos != msg.find(lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_TX_GAIN])){
                send_ctrl_error_msg(LTE_FDD_ENB_ERROR_NONE, boost::lexical_cast<std::string>(radio->get_tx_gain()));
            }else if(std::string::npos != msg.find(lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_RX_GAIN])){
                send_ctrl_error_msg(LTE_FDD_ENB_ERROR_NONE, boost::lexical_cast<std::string>(radio->get_rx_gain()));
            }else{
                send_ctrl_error_msg(LTE_FDD_ENB_ERROR_INVALID_PARAM, "");
            }
        }
    }catch(...){
        send_ctrl_error_msg(LTE_FDD_ENB_ERROR_EXCEPTION, "");
    }
}
void LTE_fdd_enb_interface::handle_write(std::string msg)
{
    LTE_fdd_enb_radio                                       *radio   = LTE_fdd_enb_radio::get_instance();
    std::map<std::string, LTE_FDD_ENB_VAR_STRUCT>::iterator  iter    = var_map.find(msg.substr(0, msg.find(" ")));
    LTE_FDD_ENB_ERROR_ENUM                                   err;
    double                                                   d_value;
    int64                                                    i_value;
    uint32                                                   u_value;
    uint32                                                   i;

    try
    {
        if(var_map.end() != iter)
        {
            // Handle all system parameters
            switch((*iter).second.var_type)
            {
            case LTE_FDD_ENB_VAR_TYPE_DOUBLE:
                d_value = boost::lexical_cast<double>(msg.substr(msg.find(" ")+1, std::string::npos));
                err     = write_value(&(*iter).second, d_value);
                break;
            case LTE_FDD_ENB_VAR_TYPE_INT64:
                i_value = boost::lexical_cast<int64>(msg.substr(msg.find(" ")+1, std::string::npos));
                err     = write_value(&(*iter).second, i_value);
                break;
            case LTE_FDD_ENB_VAR_TYPE_HEX:
                err = write_value(&(*iter).second, msg.substr(msg.find(" ")+1, std::string::npos));
                break;
            case LTE_FDD_ENB_VAR_TYPE_UINT32:
                if(LTE_FDD_ENB_PARAM_DEBUG_TYPE == (*iter).second.param)
                {
                    u_value = 0;
                    for(i=0; i<LTE_FDD_ENB_DEBUG_TYPE_N_ITEMS; i++)
                    {
                        if(std::string::npos != msg.substr(msg.find(" ")+1, std::string::npos).find(LTE_fdd_enb_debug_type_text[i]))
                        {
                            u_value |= 1 << i;
                        }
                    }
                    debug_type_mask = u_value;
                    err             = write_value(&(*iter).second, u_value);
                }else if(LTE_FDD_ENB_PARAM_DEBUG_LEVEL == (*iter).second.param){
                    u_value = 0;
                    for(i=0; i<LTE_FDD_ENB_DEBUG_LEVEL_N_ITEMS; i++)
                    {
                        if(std::string::npos != msg.substr(msg.find(" ")+1, std::string::npos).find(LTE_fdd_enb_debug_level_text[i]))
                        {
                            u_value |= 1 << i;
                        }
                    }
                    debug_level_mask = u_value;
                    err              = write_value(&(*iter).second, u_value);
                }else{
                    u_value = boost::lexical_cast<uint32>(msg.substr(msg.find(" ")+1, std::string::npos));
                    err     = write_value(&(*iter).second, u_value);
                }
                break;
            default:
                send_ctrl_error_msg(LTE_FDD_ENB_ERROR_INVALID_PARAM, "");
                break;
            }

            if(LTE_FDD_ENB_ERROR_NONE == err)
            {
                send_ctrl_error_msg(err, "");
            }else{
                send_ctrl_error_msg(err, "");
            }
        }else{
            // Handle all radio parameters
            if(std::string::npos != msg.find(lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_SELECTED_RADIO_IDX]))
            {
                u_value = boost::lexical_cast<uint32>(msg.substr(msg.find(" ")+1, std::string::npos));
                send_ctrl_error_msg(radio->set_selected_radio_idx(u_value), "");
            }else if(std::string::npos != msg.find(lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_TX_GAIN])){
                u_value = boost::lexical_cast<uint32>(msg.substr(msg.find(" ")+1, std::string::npos));
                send_ctrl_error_msg(radio->set_tx_gain(u_value), "");
            }else if(std::string::npos != msg.find(lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_RX_GAIN])){
                u_value = boost::lexical_cast<uint32>(msg.substr(msg.find(" ")+1, std::string::npos));
                send_ctrl_error_msg(radio->set_rx_gain(u_value), "");
            }else{
                send_ctrl_error_msg(LTE_FDD_ENB_ERROR_INVALID_PARAM, "");
            }
        }
    }catch(...){
        send_ctrl_error_msg(LTE_FDD_ENB_ERROR_EXCEPTION, "");
    }
}
void LTE_fdd_enb_interface::handle_start(void)
{
    boost::mutex::scoped_lock  lock(start_mutex);
    LTE_fdd_enb_cnfg_db       *cnfg_db = LTE_fdd_enb_cnfg_db::get_instance();
    LTE_fdd_enb_mac           *mac     = LTE_fdd_enb_mac::get_instance();
    LTE_fdd_enb_rlc           *rlc     = LTE_fdd_enb_rlc::get_instance();
    LTE_fdd_enb_pdcp          *pdcp    = LTE_fdd_enb_pdcp::get_instance();
    LTE_fdd_enb_rrc           *rrc     = LTE_fdd_enb_rrc::get_instance();
    LTE_fdd_enb_mme           *mme     = LTE_fdd_enb_mme::get_instance();
    LTE_fdd_enb_phy           *phy     = LTE_fdd_enb_phy::get_instance();
    LTE_fdd_enb_radio         *radio   = LTE_fdd_enb_radio::get_instance();
    LTE_FDD_ENB_ERROR_ENUM     err;

    if(!started)
    {
        started = true;
        start_mutex.unlock();

        // Construct the system information
        cnfg_db->construct_sys_info();

        // Initialize message queues for inter-layer communication
        boost::interprocess::message_queue::remove("phy_mac_mq");
        boost::interprocess::message_queue::remove("mac_phy_mq");
        boost::interprocess::message_queue::remove("mac_rlc_mq");
        boost::interprocess::message_queue::remove("rlc_mac_mq");
        boost::interprocess::message_queue::remove("rlc_pdcp_mq");
        boost::interprocess::message_queue::remove("pdcp_rlc_mq");
        boost::interprocess::message_queue::remove("pdcp_rrc_mq");
        boost::interprocess::message_queue::remove("rrc_pdcp_mq");
        boost::interprocess::message_queue::remove("rrc_mme_mq");
        boost::interprocess::message_queue::remove("mme_rrc_mq");
        boost::interprocess::message_queue phy_mac_mq(boost::interprocess::create_only,
                                                      "phy_mac_mq",
                                                      100,
                                                      sizeof(LTE_FDD_ENB_MESSAGE_STRUCT *));
        boost::interprocess::message_queue mac_phy_mq(boost::interprocess::create_only,
                                                      "mac_phy_mq",
                                                      100,
                                                      sizeof(LTE_FDD_ENB_MESSAGE_STRUCT *));
        boost::interprocess::message_queue mac_rlc_mq(boost::interprocess::create_only,
                                                      "mac_rlc_mq",
                                                      100,
                                                      sizeof(LTE_FDD_ENB_MESSAGE_STRUCT *));
        boost::interprocess::message_queue rlc_mac_mq(boost::interprocess::create_only,
                                                      "rlc_mac_mq",
                                                      100,
                                                      sizeof(LTE_FDD_ENB_MESSAGE_STRUCT *));
        boost::interprocess::message_queue rlc_pdcp_mq(boost::interprocess::create_only,
                                                       "rlc_pdcp_mq",
                                                       100,
                                                       sizeof(LTE_FDD_ENB_MESSAGE_STRUCT *));
        boost::interprocess::message_queue pdcp_rlc_mq(boost::interprocess::create_only,
                                                       "pdcp_rlc_mq",
                                                       100,
                                                       sizeof(LTE_FDD_ENB_MESSAGE_STRUCT *));
        boost::interprocess::message_queue pdcp_rrc_mq(boost::interprocess::create_only,
                                                       "pdcp_rrc_mq",
                                                       100,
                                                       sizeof(LTE_FDD_ENB_MESSAGE_STRUCT *));
        boost::interprocess::message_queue rrc_pdcp_mq(boost::interprocess::create_only,
                                                       "rrc_pdcp_mq",
                                                       100,
                                                       sizeof(LTE_FDD_ENB_MESSAGE_STRUCT *));
        boost::interprocess::message_queue rrc_mme_mq(boost::interprocess::create_only,
                                                      "rrc_mme_mq",
                                                      100,
                                                      sizeof(LTE_FDD_ENB_MESSAGE_STRUCT *));
        boost::interprocess::message_queue mme_rrc_mq(boost::interprocess::create_only,
                                                      "mme_rrc_mq",
                                                      100,
                                                      sizeof(LTE_FDD_ENB_MESSAGE_STRUCT *));

        // Start layers
        phy->start(this);
        mac->start(this);
        rlc->start();
        pdcp->start();
        rrc->start();
        mme->start();
        err = radio->start();
        if(LTE_FDD_ENB_ERROR_NONE == err)
        {
            send_ctrl_error_msg(err, "");
        }else{
            start_mutex.lock();
            started = false;
            start_mutex.unlock();

            phy->stop();
            mac->stop();
            rlc->stop();
            pdcp->stop();
            rrc->stop();
            mme->stop();

            send_ctrl_error_msg(err, "");
        }
    }else{
        send_ctrl_error_msg(LTE_FDD_ENB_ERROR_ALREADY_STARTED, "");
    }
}
void LTE_fdd_enb_interface::handle_stop(void)
{
    boost::mutex::scoped_lock  lock(start_mutex);
    LTE_fdd_enb_radio         *radio = LTE_fdd_enb_radio::get_instance();
    LTE_fdd_enb_phy           *phy   = LTE_fdd_enb_phy::get_instance();
    LTE_fdd_enb_mac           *mac   = LTE_fdd_enb_mac::get_instance();
    LTE_fdd_enb_rlc           *rlc   = LTE_fdd_enb_rlc::get_instance();
    LTE_fdd_enb_pdcp          *pdcp  = LTE_fdd_enb_pdcp::get_instance();
    LTE_fdd_enb_rrc           *rrc   = LTE_fdd_enb_rrc::get_instance();
    LTE_fdd_enb_mme           *mme   = LTE_fdd_enb_mme::get_instance();
    LTE_FDD_ENB_ERROR_ENUM     err;

    if(started)
    {
        started = false;
        start_mutex.unlock();

        // Stop all layers
        err = radio->stop();
        if(LTE_FDD_ENB_ERROR_NONE == err)
        {
            phy->stop();
            mac->stop();
            rlc->stop();
            pdcp->stop();
            rrc->stop();
            mme->stop();

            // Send a message to all inter-layer message_queues to unblock receive
            LTE_fdd_enb_msgq::send("phy_mac_mq",
                                   LTE_FDD_ENB_MESSAGE_TYPE_KILL,
                                   LTE_FDD_ENB_DEST_LAYER_ANY,
                                   NULL,
                                   0);
            LTE_fdd_enb_msgq::send("mac_phy_mq",
                                   LTE_FDD_ENB_MESSAGE_TYPE_KILL,
                                   LTE_FDD_ENB_DEST_LAYER_ANY,
                                   NULL,
                                   0);
            LTE_fdd_enb_msgq::send("mac_rlc_mq",
                                   LTE_FDD_ENB_MESSAGE_TYPE_KILL,
                                   LTE_FDD_ENB_DEST_LAYER_ANY,
                                   NULL,
                                   0);
            LTE_fdd_enb_msgq::send("rlc_mac_mq",
                                   LTE_FDD_ENB_MESSAGE_TYPE_KILL,
                                   LTE_FDD_ENB_DEST_LAYER_ANY,
                                   NULL,
                                   0);
            LTE_fdd_enb_msgq::send("rlc_pdcp_mq",
                                   LTE_FDD_ENB_MESSAGE_TYPE_KILL,
                                   LTE_FDD_ENB_DEST_LAYER_ANY,
                                   NULL,
                                   0);
            LTE_fdd_enb_msgq::send("pdcp_rlc_mq",
                                   LTE_FDD_ENB_MESSAGE_TYPE_KILL,
                                   LTE_FDD_ENB_DEST_LAYER_ANY,
                                   NULL,
                                   0);
            LTE_fdd_enb_msgq::send("pdcp_rrc_mq",
                                   LTE_FDD_ENB_MESSAGE_TYPE_KILL,
                                   LTE_FDD_ENB_DEST_LAYER_ANY,
                                   NULL,
                                   0);
            LTE_fdd_enb_msgq::send("rrc_pdcp_mq",
                                   LTE_FDD_ENB_MESSAGE_TYPE_KILL,
                                   LTE_FDD_ENB_DEST_LAYER_ANY,
                                   NULL,
                                   0);
            LTE_fdd_enb_msgq::send("rrc_mme_mq",
                                   LTE_FDD_ENB_MESSAGE_TYPE_KILL,
                                   LTE_FDD_ENB_DEST_LAYER_ANY,
                                   NULL,
                                   0);
            LTE_fdd_enb_msgq::send("mme_rrc_mq",
                                   LTE_FDD_ENB_MESSAGE_TYPE_KILL,
                                   LTE_FDD_ENB_DEST_LAYER_ANY,
                                   NULL,
                                   0);
            sleep(1);

            boost::interprocess::message_queue::remove("phy_mac_mq");
            boost::interprocess::message_queue::remove("mac_phy_mq");
            boost::interprocess::message_queue::remove("mac_rlc_mq");
            boost::interprocess::message_queue::remove("rlc_mac_mq");
            boost::interprocess::message_queue::remove("rlc_pdcp_mq");
            boost::interprocess::message_queue::remove("pdcp_rlc_mq");
            boost::interprocess::message_queue::remove("pdcp_rrc_mq");
            boost::interprocess::message_queue::remove("rrc_pdcp_mq");
            boost::interprocess::message_queue::remove("rrc_mme_mq");
            boost::interprocess::message_queue::remove("mme_rrc_mq");

            // Cleanup all layers
            LTE_fdd_enb_radio::cleanup();
            LTE_fdd_enb_phy::cleanup();
            LTE_fdd_enb_mac::cleanup();
            LTE_fdd_enb_rlc::cleanup();
            LTE_fdd_enb_pdcp::cleanup();
            LTE_fdd_enb_rrc::cleanup();
            LTE_fdd_enb_mme::cleanup();

            send_ctrl_error_msg(LTE_FDD_ENB_ERROR_NONE, "");
        }else{
            send_ctrl_error_msg(err, "");
        }
    }else{
        send_ctrl_error_msg(LTE_FDD_ENB_ERROR_ALREADY_STOPPED, "");
    }
}
void LTE_fdd_enb_interface::handle_help(void)
{
    LTE_fdd_enb_cnfg_db                                     *cnfg_db = LTE_fdd_enb_cnfg_db::get_instance();
    LTE_fdd_enb_radio                                       *radio   = LTE_fdd_enb_radio::get_instance();
    std::map<std::string, LTE_FDD_ENB_VAR_STRUCT>::iterator  iter;
    std::string                                              tmp_str;
    std::string                                              s_value;
    LTE_FDD_ENB_AVAILABLE_RADIOS_STRUCT                      avail_radios   = radio->get_available_radios();
    LTE_FDD_ENB_RADIO_STRUCT                                 selected_radio = radio->get_selected_radio();
    double                                                   d_value;
    int64                                                    i_value;
    uint32                                                   u_value;
    uint32                                                   i;

    send_ctrl_msg("***System Configuration Parameters***");
    send_ctrl_msg("\tRead parameters using read <param> format");
    send_ctrl_msg("\tSet parameters using write <param> <value> format");
    // Commands
    send_ctrl_msg("\tCommands:");
    send_ctrl_msg("\t\tstart        - Constructs the system information and starts the eNB");
    send_ctrl_msg("\t\tstop         - Stops the eNB");
    send_ctrl_msg("\t\tshutdown     - Stops the eNB and exits");
    send_ctrl_msg("\t\tconstruct_si - Constructs the new system information");
    send_ctrl_msg("\t\thelp         - Prints this screen");

    // Radio Parameters
    send_ctrl_msg("\tRadio Parameters:");
    tmp_str  = "\t\t";
    tmp_str += lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_AVAILABLE_RADIOS];
    tmp_str += ": (read-only)";
    send_ctrl_msg(tmp_str);
    for(i=0; i<avail_radios.num_radios; i++)
    {
        try
        {
            tmp_str  = "\t\t\t";
            tmp_str += boost::lexical_cast<std::string>(i);
            tmp_str += ": ";
            tmp_str += avail_radios.radio[i].name;
        }catch(...){
            // Intentionally do nothing
        }
        send_ctrl_msg(tmp_str);
    }
    tmp_str  = "\t\t";
    tmp_str += lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_SELECTED_RADIO_NAME];
    tmp_str += " (read-only) = ";
    tmp_str += selected_radio.name;
    send_ctrl_msg(tmp_str);
    tmp_str  = "\t\t";
    tmp_str += lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_SELECTED_RADIO_IDX];
    tmp_str += " = ";
    try
    {
        tmp_str += boost::lexical_cast<std::string>(radio->get_selected_radio_idx());
    }catch(...){
        // Intentionally do nothing
    }
    send_ctrl_msg(tmp_str);
    tmp_str  = "\t\t";
    tmp_str += lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_TX_GAIN];
    tmp_str += " = ";
    try
    {
        tmp_str += boost::lexical_cast<std::string>(radio->get_tx_gain());
    }catch(...){
        // Intentionally do nothing
    }
    send_ctrl_msg(tmp_str);
    tmp_str  = "\t\t";
    tmp_str += lte_fdd_enb_param_text[LTE_FDD_ENB_PARAM_RX_GAIN];
    tmp_str += " = ";
    try
    {
        tmp_str += boost::lexical_cast<std::string>(radio->get_rx_gain());
    }catch(...){
        // Intentionally do nothing
    }
    send_ctrl_msg(tmp_str);

    // System Parameters
    send_ctrl_msg("\tSystem Parameters:");
    for(iter=var_map.begin(); iter!=var_map.end(); iter++)
    {
        tmp_str  = "\t\t";
        tmp_str += lte_fdd_enb_param_text[(*iter).second.param];
        tmp_str += " = ";
        try
        {
            switch((*iter).second.var_type)
            {
            case LTE_FDD_ENB_VAR_TYPE_DOUBLE:
                cnfg_db->get_param((*iter).second.param, d_value);
                tmp_str += boost::lexical_cast<std::string>(d_value);
                break;
            case LTE_FDD_ENB_VAR_TYPE_INT64:
                cnfg_db->get_param((*iter).second.param, i_value);
                if(LTE_FDD_ENB_PARAM_FREQ_BAND == (*iter).second.param)
                {
                    tmp_str += liblte_interface_band_text[i_value];
                }else{
                    tmp_str += boost::lexical_cast<std::string>(i_value);
                }
                break;
            case LTE_FDD_ENB_VAR_TYPE_HEX:
                s_value.clear();
                cnfg_db->get_param((*iter).second.param, s_value);
                tmp_str += s_value;
                break;
            case LTE_FDD_ENB_VAR_TYPE_UINT32:
                cnfg_db->get_param((*iter).second.param, u_value);
                if(LTE_FDD_ENB_PARAM_DEBUG_TYPE == (*iter).second.param)
                {
                    for(i=0; i<LTE_FDD_ENB_DEBUG_TYPE_N_ITEMS; i++)
                    {
                        if((u_value & (1 << i)))
                        {
                            tmp_str += LTE_fdd_enb_debug_type_text[i];
                            tmp_str += " ";
                        }
                    }
                }else if(LTE_FDD_ENB_PARAM_DEBUG_LEVEL == (*iter).second.param){
                    for(i=0; i<LTE_FDD_ENB_DEBUG_LEVEL_N_ITEMS; i++)
                    {
                        if((u_value & (1 << i)))
                        {
                            tmp_str += LTE_fdd_enb_debug_level_text[i];
                            tmp_str += " ";
                        }
                    }
                }else{
                    tmp_str += boost::lexical_cast<std::string>(u_value);
                }
                break;
            }
        }catch(...){
            // Intentionally do nothing
        }
        send_ctrl_msg(tmp_str);
    }
}

/*******************/
/*    Gets/Sets    */
/*******************/
bool LTE_fdd_enb_interface::get_shutdown(void)
{
    return(shutdown);
}
bool LTE_fdd_enb_interface::app_is_started(void)
{
    boost::mutex::scoped_lock lock(start_mutex);
    return(started);
}

/*****************/
/*    Helpers    */
/*****************/
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_interface::write_value(LTE_FDD_ENB_VAR_STRUCT *var,
                                                          double                  value)
{
    LTE_fdd_enb_cnfg_db    *cnfg_db = LTE_fdd_enb_cnfg_db::get_instance();
    LTE_FDD_ENB_ERROR_ENUM  err     = LTE_FDD_ENB_ERROR_OUT_OF_BOUNDS;

    if(started && !var->dynamic)
    {
        err = LTE_FDD_ENB_ERROR_VARIABLE_NOT_DYNAMIC;
    }else{
        if(!var->special_bounds)
        {
            if(value >= var->double_l_bound &&
               value <= var->double_u_bound)
            {
                err = cnfg_db->set_param(var->param, value);
            }
        }else{
            if(LTE_FDD_ENB_PARAM_BANDWIDTH == var->param &&
               (value                      == 1.4        ||
                value                      == 3          ||
                value                      == 5          ||
                value                      == 10         ||
                value                      == 15         ||
                value                      == 20))
            {
                err = cnfg_db->set_param(var->param, value);
            }else{
                err = LTE_FDD_ENB_ERROR_INVALID_PARAM;
            }
        }
    }

    return(err);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_interface::write_value(LTE_FDD_ENB_VAR_STRUCT *var,
                                                          int64                   value)
{
    LTE_fdd_enb_cnfg_db    *cnfg_db = LTE_fdd_enb_cnfg_db::get_instance();
    LTE_FDD_ENB_ERROR_ENUM  err     = LTE_FDD_ENB_ERROR_OUT_OF_BOUNDS;
    int64                   band    = LIBLTE_INTERFACE_BAND_N_ITEMS;
    uint32                  i;

    if(started && !var->dynamic)
    {
        err = LTE_FDD_ENB_ERROR_VARIABLE_NOT_DYNAMIC;
    }else{
        if(!var->special_bounds)
        {
            if(value >= var->int64_l_bound &&
               value <= var->int64_u_bound)
            {
                err = cnfg_db->set_param(var->param, value);
            }
        }else{
            if(LTE_FDD_ENB_PARAM_N_ANT == var->param &&
               (value                  == 1          ||
                value                  == 2          ||
                value                  == 4))
            {
                err = cnfg_db->set_param(var->param, value);
            }else if(LTE_FDD_ENB_PARAM_Q_HYST == var->param){
                for(i=0; i<LIBLTE_RRC_Q_HYST_N_ITEMS; i++)
                {
                    if(value == liblte_rrc_q_hyst_num[i])
                    {
                        err = cnfg_db->set_param(var->param, value);
                        break;
                    }
                }
            }else if(LTE_FDD_ENB_PARAM_FREQ_BAND == var->param){
                for(i=0; i<LIBLTE_INTERFACE_BAND_N_ITEMS; i++)
                {
                    if(value == liblte_interface_band_num[i])
                    {
                        err = cnfg_db->set_param(var->param, (int64)i);
                        err = cnfg_db->set_param(LTE_FDD_ENB_PARAM_DL_EARFCN, (int64)liblte_interface_first_dl_earfcn[i]);
                        cnfg_db->construct_sys_info();
                        break;
                    }
                }
            }else if(LTE_FDD_ENB_PARAM_DL_EARFCN == var->param){
                cnfg_db->get_param(LTE_FDD_ENB_PARAM_FREQ_BAND, band);
                if(value >= liblte_interface_first_dl_earfcn[band] &&
                   value <= liblte_interface_last_dl_earfcn[band])
                {
                    err = cnfg_db->set_param(var->param, value);
                }
            }else{
                err = LTE_FDD_ENB_ERROR_INVALID_PARAM;
            }
        }
    }

    return(err);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_interface::write_value(LTE_FDD_ENB_VAR_STRUCT *var,
                                                          std::string             value)
{
    LTE_fdd_enb_cnfg_db    *cnfg_db = LTE_fdd_enb_cnfg_db::get_instance();
    LTE_FDD_ENB_ERROR_ENUM  err     = LTE_FDD_ENB_ERROR_OUT_OF_BOUNDS;
    uint32                  i;

    if(started && !var->dynamic)
    {
        err = LTE_FDD_ENB_ERROR_VARIABLE_NOT_DYNAMIC;
    }else{
        if((LTE_FDD_ENB_PARAM_MCC == var->param &&
            value.length()        == 3)         ||
           (LTE_FDD_ENB_PARAM_MNC == var->param &&
            (value.length()       == 2          ||
             value.length()       == 3)))
        {
            for(i=0; i<value.length(); i++)
            {
                if((value[i] & 0x0F) >= 0x0A)
                {
                    break;
                }
            }
            if(i == value.length())
            {
                err = cnfg_db->set_param(var->param, value);
            }
        }else{
            err = LTE_FDD_ENB_ERROR_INVALID_PARAM;
        }
    }

    return(err);
}
LTE_FDD_ENB_ERROR_ENUM LTE_fdd_enb_interface::write_value(LTE_FDD_ENB_VAR_STRUCT *var,
                                                          uint32                  value)
{
    LTE_fdd_enb_cnfg_db    *cnfg_db = LTE_fdd_enb_cnfg_db::get_instance();
    LTE_FDD_ENB_ERROR_ENUM  err     = LTE_FDD_ENB_ERROR_OUT_OF_BOUNDS;

    if(started && !var->dynamic)
    {
        err = LTE_FDD_ENB_ERROR_VARIABLE_NOT_DYNAMIC;
    }else{
        if(!var->special_bounds)
        {
            if(value >= var->int64_l_bound &&
               value <= var->int64_u_bound)
            {
                err = cnfg_db->set_param(var->param, value);
            }
        }else{
            err = cnfg_db->set_param(var->param, value);
        }
    }

    return(err);
}
