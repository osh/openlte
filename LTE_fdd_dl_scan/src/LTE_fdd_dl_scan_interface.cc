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

    File: LTE_fdd_dl_scan_interface.cc

    Description: Contains all the implementations for the LTE FDD DL Scanner
                 interface.

    Revision History
    ----------    -------------    --------------------------------------------
    02/26/2013    Ben Wojtowicz    Created file

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_dl_scan_interface.h"
#include "LTE_fdd_dl_scan_flowgraph.h"
#include "liblte_mcc_mnc_list.h"
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>
#include <boost/lexical_cast.hpp>

/*******************************************************************************
                              DEFINES
*******************************************************************************/

#define BAND_PARAM           "band"
#define DL_EARFCN_LIST_PARAM "dl_earfcn_list"
#define REPEAT_PARAM         "repeat"

/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/


/*******************************************************************************
                              GLOBAL VARIABLES
*******************************************************************************/

LTE_fdd_dl_scan_interface* LTE_fdd_dl_scan_interface::instance = NULL;
boost::mutex               interface_instance_mutex;
boost::mutex               connect_mutex;
bool                       LTE_fdd_dl_scan_interface::ctrl_connected = false;

/*******************************************************************************
                              CLASS IMPLEMENTATIONS
*******************************************************************************/

// Singleton
LTE_fdd_dl_scan_interface* LTE_fdd_dl_scan_interface::get_instance(void)
{
    boost::mutex::scoped_lock lock(interface_instance_mutex);

    if(NULL == instance)
    {
        instance = new LTE_fdd_dl_scan_interface();
    }

    return(instance);
}
void LTE_fdd_dl_scan_interface::cleanup(void)
{
    boost::mutex::scoped_lock lock(interface_instance_mutex);

    if(NULL != instance)
    {
        delete instance;
        instance = NULL;
    }
}

// Constructor/Destructor
LTE_fdd_dl_scan_interface::LTE_fdd_dl_scan_interface()
{
    uint32 i;

    // Communication
    ctrl_socket    = NULL;
    ctrl_port      = LTE_FDD_DL_SCAN_DEFAULT_CTRL_PORT;
    ctrl_connected = false;

    // Variables
    band                = LIBLTE_INTERFACE_BAND_1;
    dl_earfcn_list_size = liblte_interface_last_dl_earfcn[band] - liblte_interface_first_dl_earfcn[band] + 1;
    dl_earfcn_list_idx  = 0;
    for(i=0; i<dl_earfcn_list_size; i++)
    {
        dl_earfcn_list[i] = liblte_interface_first_dl_earfcn[band] + i;
    }
    current_dl_earfcn = dl_earfcn_list[dl_earfcn_list_idx];
    repeat            = true;
    shutdown          = false;
}
LTE_fdd_dl_scan_interface::~LTE_fdd_dl_scan_interface()
{
    stop_ctrl_port();
}

// Communication
void LTE_fdd_dl_scan_interface::set_ctrl_port(int16 port)
{
    boost::mutex::scoped_lock lock(connect_mutex);

    if(!ctrl_connected)
    {
        ctrl_port = port;
    }
}
void LTE_fdd_dl_scan_interface::start_ctrl_port(void)
{
    boost::mutex::scoped_lock       lock(ctrl_mutex);
    LIBTOOLS_SOCKET_WRAP_ERROR_ENUM error;

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
            printf("ERROR: Couldn't open ctrl_socket %u\n", error);
            ctrl_socket = NULL;
        }
    }
}
void LTE_fdd_dl_scan_interface::stop_ctrl_port(void)
{
    boost::mutex::scoped_lock lock(ctrl_mutex);

    if(NULL != ctrl_socket)
    {
        delete ctrl_socket;
        ctrl_socket = NULL;
    }
}
void LTE_fdd_dl_scan_interface::send_ctrl_msg(std::string msg)
{
    boost::mutex::scoped_lock lock(connect_mutex);
    std::string               tmp_msg;

    if(ctrl_connected)
    {
        tmp_msg  = msg;
        tmp_msg += "\n";
        ctrl_socket->send(tmp_msg);
    }
}
void LTE_fdd_dl_scan_interface::send_ctrl_info_msg(std::string msg)
{
    boost::mutex::scoped_lock lock(connect_mutex);
    std::string               tmp_msg;

    if(ctrl_connected)
    {
        tmp_msg  = "info ";
        tmp_msg += msg;
        tmp_msg += "\n";
        ctrl_socket->send(tmp_msg);
    }
}
void LTE_fdd_dl_scan_interface::send_ctrl_channel_found_msg(LTE_FDD_DL_SCAN_CHAN_DATA_STRUCT *chan_data)
{
    boost::mutex::scoped_lock lock(connect_mutex);
    std::string               tmp_msg;
    uint32                    i;
    uint32                    j;
    uint16                    mnc;

    if(ctrl_connected)
    {
        tmp_msg = "info channel_found ";
        try
        {
            tmp_msg += "freq=" + boost::lexical_cast<std::string>(liblte_interface_dl_earfcn_to_frequency(current_dl_earfcn)) + " ";
            tmp_msg += "dl_earfcn=" + boost::lexical_cast<std::string>(current_dl_earfcn) + " ";
            tmp_msg += "freq_offset=" + boost::lexical_cast<std::string>(chan_data->freq_offset) + " ";
            tmp_msg += "sfn=" + boost::lexical_cast<std::string>(chan_data->sfn) + " ";
            tmp_msg += "n_ant=" + boost::lexical_cast<std::string>((uint32)chan_data->N_ant) + " ";
            tmp_msg += "phich_dur=" + boost::lexical_cast<std::string>(liblte_rrc_phich_duration_text[chan_data->mib.phich_config.dur]) + " ";
            tmp_msg += "phich_res=" + boost::lexical_cast<std::string>(liblte_rrc_phich_resource_text[chan_data->mib.phich_config.res]) + " ";
            tmp_msg += "bandwidth=" + boost::lexical_cast<std::string>(liblte_rrc_dl_bandwidth_text[chan_data->mib.dl_bw]) + " ";
            tmp_msg += "phys_cell_id=" + boost::lexical_cast<std::string>(chan_data->N_id_cell) + " ";

            tmp_msg += "\n";
        }catch(boost::bad_lexical_cast &){
            tmp_msg += "\n";
        }
        ctrl_socket->send(tmp_msg);
    }
}
void LTE_fdd_dl_scan_interface::send_ctrl_channel_not_found_msg(void)
{
    boost::mutex::scoped_lock lock(connect_mutex);
    std::string               tmp_msg;

    if(ctrl_connected)
    {
        tmp_msg = "info channel_not_found ";
        try
        {
            tmp_msg += "freq=" + boost::lexical_cast<std::string>(liblte_interface_dl_earfcn_to_frequency(current_dl_earfcn)) + " ";
            tmp_msg += "dl_earfcn=" + boost::lexical_cast<std::string>(current_dl_earfcn) + " ";
            tmp_msg += "\n";
        }catch(boost::bad_lexical_cast &){
            tmp_msg += "\n";
        }
        ctrl_socket->send(tmp_msg);
    }
}
void LTE_fdd_dl_scan_interface::send_ctrl_status_msg(LTE_FDD_DL_SCAN_STATUS_ENUM status,
                                                     std::string                 msg)
{
    boost::mutex::scoped_lock lock(connect_mutex);
    std::string               tmp_msg;

    if(ctrl_connected)
    {
        if(LTE_FDD_DL_SCAN_STATUS_OK == status)
        {
            tmp_msg = "ok ";
        }else{
            tmp_msg = "fail ";
        }
        tmp_msg += msg;
        tmp_msg += "\n";

        ctrl_socket->send(tmp_msg);
    }
}
void LTE_fdd_dl_scan_interface::handle_ctrl_msg(std::string msg)
{
    LTE_fdd_dl_scan_interface *interface = LTE_fdd_dl_scan_interface::get_instance();

    if(std::string::npos != msg.find("read"))
    {
        interface->handle_read(msg);
    }else if(std::string::npos != msg.find("write")){
        interface->handle_write(msg);
    }else if(std::string::npos != msg.find("start")){
        interface->handle_start();
    }else if(std::string::npos != msg.find("stop")){
        interface->handle_stop();
    }else if(std::string::npos != msg.find("shutdown")){
        interface->shutdown = true;
        interface->send_ctrl_status_msg(LTE_FDD_DL_SCAN_STATUS_OK, "");
    }else if(std::string::npos != msg.find("help")){
        interface->handle_help();
    }else{
        interface->send_ctrl_status_msg(LTE_FDD_DL_SCAN_STATUS_FAIL, "Invalid param");
    }
}
void LTE_fdd_dl_scan_interface::handle_ctrl_connect(void)
{
    LTE_fdd_dl_scan_interface *interface = LTE_fdd_dl_scan_interface::get_instance();

    connect_mutex.lock();
    LTE_fdd_dl_scan_interface::ctrl_connected = true;
    connect_mutex.unlock();

    interface->send_ctrl_msg("*** LTE FDD DL SCAN ***");
    interface->send_ctrl_msg("Type help to see a list of commands");
}
void LTE_fdd_dl_scan_interface::handle_ctrl_disconnect(void)
{
    boost::mutex::scoped_lock lock(connect_mutex);

    LTE_fdd_dl_scan_interface::ctrl_connected = false;
}
void LTE_fdd_dl_scan_interface::handle_ctrl_error(LIBTOOLS_SOCKET_WRAP_ERROR_ENUM err)
{
    printf("ERROR: ctrl_socket error %u\n", err);
    assert(0);
}

// Handlers
void LTE_fdd_dl_scan_interface::handle_read(std::string msg)
{
    if(std::string::npos != msg.find(BAND_PARAM))
    {
        read_band();
    }else if(std::string::npos != msg.find(DL_EARFCN_LIST_PARAM)){
        read_dl_earfcn_list();
    }else if(std::string::npos != msg.find(REPEAT_PARAM)){
        read_repeat();
    }else{
        send_ctrl_status_msg(LTE_FDD_DL_SCAN_STATUS_FAIL, "Invalid read");
    }
}
void LTE_fdd_dl_scan_interface::handle_write(std::string msg)
{
    if(std::string::npos != msg.find(BAND_PARAM))
    {
        write_band(msg.substr(msg.find(BAND_PARAM)+sizeof(BAND_PARAM), std::string::npos).c_str());
    }else if(std::string::npos != msg.find(DL_EARFCN_LIST_PARAM)){
        write_dl_earfcn_list(msg.substr(msg.find(DL_EARFCN_LIST_PARAM)+sizeof(DL_EARFCN_LIST_PARAM), std::string::npos).c_str());
    }else if(std::string::npos != msg.find(REPEAT_PARAM)){
        write_repeat(msg.substr(msg.find(REPEAT_PARAM)+sizeof(REPEAT_PARAM), std::string::npos).c_str());
    }else{
        send_ctrl_status_msg(LTE_FDD_DL_SCAN_STATUS_FAIL, "Invalid write");
    }
}
void LTE_fdd_dl_scan_interface::handle_start(void)
{
    boost::mutex::scoped_lock  lock(dl_earfcn_list_mutex);
    LTE_fdd_dl_scan_flowgraph *flowgraph = LTE_fdd_dl_scan_flowgraph::get_instance();

    if(!flowgraph->is_started())
    {
        dl_earfcn_list_idx = 0;
        current_dl_earfcn  = dl_earfcn_list[dl_earfcn_list_idx];
        if(LTE_FDD_DL_SCAN_STATUS_OK == flowgraph->start(current_dl_earfcn))
        {
            send_ctrl_status_msg(LTE_FDD_DL_SCAN_STATUS_OK, "");
        }else{
            send_ctrl_status_msg(LTE_FDD_DL_SCAN_STATUS_FAIL, "Start fail, likely there is no hardware connected");
        }
    }else{
        send_ctrl_status_msg(LTE_FDD_DL_SCAN_STATUS_FAIL, "Flowgraph already started");
    }
}
void LTE_fdd_dl_scan_interface::handle_stop(void)
{
    LTE_fdd_dl_scan_flowgraph *flowgraph = LTE_fdd_dl_scan_flowgraph::get_instance();

    if(flowgraph->is_started())
    {
        if(LTE_FDD_DL_SCAN_STATUS_OK == flowgraph->stop())
        {
            send_ctrl_status_msg(LTE_FDD_DL_SCAN_STATUS_OK, "");
        }else{
            send_ctrl_status_msg(LTE_FDD_DL_SCAN_STATUS_FAIL, "Stop fail");
        }
    }else{
        send_ctrl_status_msg(LTE_FDD_DL_SCAN_STATUS_FAIL, "Flowgraph not started");
    }
}
void LTE_fdd_dl_scan_interface::handle_help(void)
{
    boost::mutex::scoped_lock lock(dl_earfcn_list_mutex);
    std::string               tmp_str;
    uint32                    i;

    send_ctrl_msg("***System Configuration Parameters***");
    send_ctrl_msg("\tRead parameters using read <param> format");
    send_ctrl_msg("\tSet parameters using write <param> <value> format");
    send_ctrl_msg("\tCommands:");
    send_ctrl_msg("\t\tstart    - Starts scanning the dl_earfcn_list");
    send_ctrl_msg("\t\tstop     - Stops the scan");
    send_ctrl_msg("\t\tshutdown - Stops the scan and exits");
    send_ctrl_msg("\t\thelp     - Prints this screen");
    send_ctrl_msg("\tParameters:");

    // Band
    tmp_str  = "\t\t";
    tmp_str += BAND_PARAM;
    tmp_str += " = ";
    tmp_str += liblte_interface_band_text[band];
    send_ctrl_msg(tmp_str);

    // DL EARFCN list
    tmp_str  = "\t\t";
    tmp_str += DL_EARFCN_LIST_PARAM;
    tmp_str += " = ";
    try
    {
        for(i=0; i<dl_earfcn_list_size; i++)
        {
            tmp_str += boost::lexical_cast<std::string>(dl_earfcn_list[i]);
            if(i != dl_earfcn_list_size-1)
            {
                tmp_str += ",";
            }
        }
    }catch(boost::bad_lexical_cast &){
        // Intentionally do nothing
    }
    send_ctrl_msg(tmp_str);

    // Repeat
    tmp_str  = "\t\t";
    tmp_str += REPEAT_PARAM;
    tmp_str += " = ";
    if(true == repeat)
    {
        tmp_str += "on";
    }else{
        tmp_str += "off";
    }
    send_ctrl_msg(tmp_str);
}

// Gets/Sets
bool LTE_fdd_dl_scan_interface::get_shutdown(void)
{
    return(shutdown);
}

// Reads/Writes
void LTE_fdd_dl_scan_interface::read_band(void)
{
    send_ctrl_status_msg(LTE_FDD_DL_SCAN_STATUS_OK, liblte_interface_band_text[band]);
}
void LTE_fdd_dl_scan_interface::write_band(std::string band_str)
{
    boost::mutex::scoped_lock lock(dl_earfcn_list_mutex);
    uint32                    i;

    for(i=0; i<LIBLTE_INTERFACE_BAND_N_ITEMS; i++)
    {
        if(band_str == liblte_interface_band_text[i])
        {
            band = (LIBLTE_INTERFACE_BAND_ENUM)i;
            break;
        }
    }

    if(LIBLTE_INTERFACE_BAND_N_ITEMS == i)
    {
        send_ctrl_status_msg(LTE_FDD_DL_SCAN_STATUS_FAIL, "Invalid Band");
    }else{
        dl_earfcn_list_size = liblte_interface_last_dl_earfcn[band] - liblte_interface_first_dl_earfcn[band] + 1;
        for(i=0; i<dl_earfcn_list_size; i++)
        {
            dl_earfcn_list[i] = liblte_interface_first_dl_earfcn[band] + i;
        }
        send_ctrl_status_msg(LTE_FDD_DL_SCAN_STATUS_OK, "");
    }
}
void LTE_fdd_dl_scan_interface::read_dl_earfcn_list(void)
{
    boost::mutex::scoped_lock lock(dl_earfcn_list_mutex);
    std::string               tmp_str;
    uint32                    i;

    try
    {
        for(i=0; i<dl_earfcn_list_size; i++)
        {
            tmp_str += boost::lexical_cast<std::string>(dl_earfcn_list[i]);
            if(i != dl_earfcn_list_size-1)
            {
                tmp_str += ",";
            }
        }
        send_ctrl_status_msg(LTE_FDD_DL_SCAN_STATUS_OK, tmp_str);
    }catch(boost::bad_lexical_cast &){
        send_ctrl_status_msg(LTE_FDD_DL_SCAN_STATUS_FAIL, "bad dl_earfcn_list");
    }
}
void LTE_fdd_dl_scan_interface::write_dl_earfcn_list(std::string dl_earfcn_list_str)
{
    boost::mutex::scoped_lock   lock(dl_earfcn_list_mutex);
    LTE_FDD_DL_SCAN_STATUS_ENUM stat = LTE_FDD_DL_SCAN_STATUS_OK;
    uint32                      i;
    uint16                      tmp_list[65535];
    uint16                      tmp_list_size = 0;

    while(std::string::npos != dl_earfcn_list_str.find(","))
    {
        try
        {
            tmp_list[tmp_list_size] = boost::lexical_cast<uint16>(dl_earfcn_list_str.substr(0, dl_earfcn_list_str.find(",")));
        }catch(boost::bad_lexical_cast &){
            stat = LTE_FDD_DL_SCAN_STATUS_FAIL;
            break;
        }
        if(tmp_list[tmp_list_size] >= liblte_interface_first_dl_earfcn[band] &&
           tmp_list[tmp_list_size] <= liblte_interface_last_dl_earfcn[band])
        {
            tmp_list_size++;
        }else{
            stat = LTE_FDD_DL_SCAN_STATUS_FAIL;
            break;
        }
        dl_earfcn_list_str = dl_earfcn_list_str.substr(dl_earfcn_list_str.find(",")+1, std::string::npos);
    }
    try
    {
        tmp_list[tmp_list_size] = boost::lexical_cast<uint16>(dl_earfcn_list_str);
    }catch(boost::bad_lexical_cast &){
        stat = LTE_FDD_DL_SCAN_STATUS_FAIL;
    }
    if(tmp_list[tmp_list_size] >= liblte_interface_first_dl_earfcn[band] &&
       tmp_list[tmp_list_size] <= liblte_interface_last_dl_earfcn[band])
    {
        tmp_list_size++;
    }else{
        stat = LTE_FDD_DL_SCAN_STATUS_FAIL;
    }

    if(LTE_FDD_DL_SCAN_STATUS_FAIL == stat)
    {
        send_ctrl_status_msg(stat, "Invalid dl_earfcn_list");
    }else{
        for(i=0; i<tmp_list_size; i++)
        {
            dl_earfcn_list[i] = tmp_list[i];
        }
        dl_earfcn_list_size = tmp_list_size;
        send_ctrl_status_msg(stat, "");
    }
}
void LTE_fdd_dl_scan_interface::read_repeat(void)
{
    if(true == repeat)
    {
        send_ctrl_status_msg(LTE_FDD_DL_SCAN_STATUS_OK, "on");
    }else{
        send_ctrl_status_msg(LTE_FDD_DL_SCAN_STATUS_OK, "off");
    }
}
void LTE_fdd_dl_scan_interface::write_repeat(std::string repeat_str)
{
    if(repeat_str == "on")
    {
        repeat = true;
        send_ctrl_status_msg(LTE_FDD_DL_SCAN_STATUS_OK, "");
    }else if(repeat_str == "off"){
        repeat = false;
        send_ctrl_status_msg(LTE_FDD_DL_SCAN_STATUS_OK, "");
    }else{
        send_ctrl_status_msg(LTE_FDD_DL_SCAN_STATUS_FAIL, "Invalid Repeat");
    }
}

// Helpers
LTE_FDD_DL_SCAN_STATUS_ENUM LTE_fdd_dl_scan_interface::switch_to_next_freq(void)
{
    boost::mutex::scoped_lock    lock(dl_earfcn_list_mutex);
    LTE_fdd_dl_scan_flowgraph   *flowgraph = LTE_fdd_dl_scan_flowgraph::get_instance();
    LTE_FDD_DL_SCAN_STATUS_ENUM  stat      = LTE_FDD_DL_SCAN_STATUS_FAIL;

    if(repeat)
    {
        dl_earfcn_list_idx++;
        if(dl_earfcn_list_idx >= dl_earfcn_list_size)
        {
            dl_earfcn_list_idx = 0;
        }
        current_dl_earfcn = dl_earfcn_list[dl_earfcn_list_idx];
        flowgraph->update_center_freq(current_dl_earfcn);
        stat = LTE_FDD_DL_SCAN_STATUS_OK;
    }else{
        dl_earfcn_list_idx++;
        if(dl_earfcn_list_idx < dl_earfcn_list_size)
        {
            current_dl_earfcn = dl_earfcn_list[dl_earfcn_list_idx];
            flowgraph->update_center_freq(current_dl_earfcn);
            stat = LTE_FDD_DL_SCAN_STATUS_OK;
        }
    }

    return(stat);
}
