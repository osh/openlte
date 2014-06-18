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

    File: LTE_fdd_dl_scan_interface.cc

    Description: Contains all the implementations for the LTE FDD DL Scanner
                 interface.

    Revision History
    ----------    -------------    --------------------------------------------
    02/26/2013    Ben Wojtowicz    Created file
    07/21/2013    Ben Wojtowicz    Added support for decoding SIBs.
    06/15/2014    Ben Wojtowicz    Added PCAP support.

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
#include <arpa/inet.h>

/*******************************************************************************
                              DEFINES
*******************************************************************************/

#define BAND_PARAM           "band"
#define DL_EARFCN_LIST_PARAM "dl_earfcn_list"
#define REPEAT_PARAM         "repeat"
#define ENABLE_PCAP_PARAM    "enable_pcap"

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
    enable_pcap       = false;
    shutdown          = false;

    open_pcap_fd();
}
LTE_fdd_dl_scan_interface::~LTE_fdd_dl_scan_interface()
{
    stop_ctrl_port();

    fclose(pcap_fd);
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
void LTE_fdd_dl_scan_interface::send_ctrl_channel_found_begin_msg(LTE_FDD_DL_SCAN_CHAN_DATA_STRUCT *chan_data,
                                                                  LIBLTE_RRC_MIB_STRUCT            *mib,
                                                                  uint32                            sfn,
                                                                  uint8                             N_ant)
{
    boost::mutex::scoped_lock lock(connect_mutex);
    std::string               tmp_msg;

    if(ctrl_connected)
    {
        tmp_msg = "info channel_found_begin ";
        try
        {
            tmp_msg += "freq=" + boost::lexical_cast<std::string>(liblte_interface_dl_earfcn_to_frequency(current_dl_earfcn)) + " ";
            tmp_msg += "dl_earfcn=" + boost::lexical_cast<std::string>(current_dl_earfcn) + " ";
            tmp_msg += "freq_offset=" + boost::lexical_cast<std::string>(chan_data->freq_offset) + " ";
            tmp_msg += "phys_cell_id=" + boost::lexical_cast<std::string>(chan_data->N_id_cell) + " ";
            tmp_msg += "sfn=" + boost::lexical_cast<std::string>(sfn) + " ";

            tmp_msg += "n_ant=" + boost::lexical_cast<std::string>((uint32)N_ant) + " ";
            tmp_msg += "phich_dur=" + boost::lexical_cast<std::string>(liblte_rrc_phich_duration_text[mib->phich_config.dur]) + " ";
            tmp_msg += "phich_res=" + boost::lexical_cast<std::string>(liblte_rrc_phich_resource_text[mib->phich_config.res]) + " ";
            tmp_msg += "bandwidth=" + boost::lexical_cast<std::string>(liblte_rrc_dl_bandwidth_text[mib->dl_bw]) + " ";
            tmp_msg += "\n";
        }catch(boost::bad_lexical_cast &){
            tmp_msg += "\n";
        }
        ctrl_socket->send(tmp_msg);
    }
}
void LTE_fdd_dl_scan_interface::send_ctrl_sib1_decoded_msg(LTE_FDD_DL_SCAN_CHAN_DATA_STRUCT        *chan_data,
                                                           LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1_STRUCT *sib1,
                                                           uint32                                   sfn)
{
    boost::mutex::scoped_lock lock(connect_mutex);
    std::string               tmp_msg;
    uint32                    i;
    uint32                    j;
    uint16                    mnc;

    if(ctrl_connected)
    {
        tmp_msg = "info sib1_decoded ";
        try
        {
            tmp_msg += "freq=" + boost::lexical_cast<std::string>(liblte_interface_dl_earfcn_to_frequency(current_dl_earfcn)) + " ";
            tmp_msg += "dl_earfcn=" + boost::lexical_cast<std::string>(current_dl_earfcn) + " ";
            tmp_msg += "freq_offset=" + boost::lexical_cast<std::string>(chan_data->freq_offset) + " ";
            tmp_msg += "phys_cell_id=" + boost::lexical_cast<std::string>(chan_data->N_id_cell) + " ";
            tmp_msg += "sfn=" + boost::lexical_cast<std::string>(sfn) + " ";

            for(i=0; i<sib1->N_plmn_ids; i++)
            {
                tmp_msg += "mcc[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>((sib1->plmn_id[i].id.mcc & 0x0F00) >> 8);
                tmp_msg += boost::lexical_cast<std::string>((sib1->plmn_id[i].id.mcc & 0x00F0) >> 4);
                tmp_msg += boost::lexical_cast<std::string>((sib1->plmn_id[i].id.mcc & 0x000F)) + " ";
                tmp_msg += "mnc[" + boost::lexical_cast<std::string>(i) + "]=";
                if((sib1->plmn_id[i].id.mnc & 0xFF00) == 0xFF00)
                {
                    mnc      = sib1->plmn_id[i].id.mnc & 0x00FF;
                    tmp_msg += boost::lexical_cast<std::string>((sib1->plmn_id[i].id.mnc & 0x00F0) >> 4);
                    tmp_msg += boost::lexical_cast<std::string>((sib1->plmn_id[i].id.mnc & 0x000F)) + " ";
                }else{
                    mnc      = sib1->plmn_id[i].id.mnc & 0x0FFF;
                    tmp_msg += boost::lexical_cast<std::string>((sib1->plmn_id[i].id.mnc & 0x0F00) >> 8);
                    tmp_msg += boost::lexical_cast<std::string>((sib1->plmn_id[i].id.mnc & 0x00F0) >> 4);
                    tmp_msg += boost::lexical_cast<std::string>((sib1->plmn_id[i].id.mnc & 0x000F)) + " ";
                }
                for(j=0; j<LIBLTE_MCC_MNC_LIST_N_ITEMS; j++)
                {
                    if(liblte_mcc_mnc_list[j].mcc == (sib1->plmn_id[i].id.mcc & 0x0FFF) &&
                       liblte_mcc_mnc_list[j].mnc == mnc)
                    {
                        tmp_msg += "network[" + boost::lexical_cast<std::string>(i) + "]=";
                        tmp_msg += boost::lexical_cast<std::string>(liblte_mcc_mnc_list[j].net_name) + " ";
                        break;
                    }
                }
                if(LIBLTE_RRC_RESV_FOR_OPER == sib1->plmn_id[i].resv_for_oper)
                {
                    tmp_msg += "resv_for_oper[" + boost::lexical_cast<std::string>(i) + "]=true ";
                }else{
                    tmp_msg += "resv_for_oper[" + boost::lexical_cast<std::string>(i) + "]=false ";
                }
            }
            tmp_msg += "tac=" + boost::lexical_cast<std::string>((uint32)sib1->tracking_area_code) + " ";
            tmp_msg += "cell_id=" + boost::lexical_cast<std::string>(sib1->cell_id) + " ";
            if(LIBLTE_RRC_CELL_BARRED == sib1->cell_barred)
            {
                tmp_msg += "cell_barred=true ";
            }else{
                tmp_msg += "cell_barred=false ";
            }
            if(LIBLTE_RRC_INTRA_FREQ_RESELECTION_ALLOWED == sib1->intra_freq_reselection)
            {
                tmp_msg += "intra_freq_resel=allowed ";
            }else{
                tmp_msg += "intra_freq_resel=not_allowed ";
            }
            if(LIBLTE_RRC_CSG_IDENTITY_NOT_PRESENT != sib1->csg_id)
            {
                tmp_msg += "csg_id=" + boost::lexical_cast<std::string>(sib1->csg_id) + " ";
            }
            tmp_msg += "q_rx_lev_min=" + boost::lexical_cast<std::string>((int32)sib1->q_rx_lev_min) + " ";
            tmp_msg += "q_rx_lev_min_offset=" + boost::lexical_cast<std::string>((uint32)sib1->q_rx_lev_min_offset) + " ";
            if(true == sib1->p_max_present)
            {
                tmp_msg += "p_max=" + boost::lexical_cast<std::string>((int32)sib1->p_max) + " ";
            }
            tmp_msg += "band=" + boost::lexical_cast<std::string>((uint32)sib1->freq_band_indicator) + " ";
            tmp_msg += "si_win_len=" + boost::lexical_cast<std::string>(liblte_rrc_si_window_length_text[sib1->si_window_length]) + " ";
            for(i=0; i<sib1->N_sched_info; i++)
            {
                tmp_msg += "si_periodicity[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>(liblte_rrc_si_periodicity_text[sib1->sched_info[i].si_periodicity]) + " ";
                tmp_msg += "sib_mapping_info[" + boost::lexical_cast<std::string>(i) + "]=";
                if(0 == i)
                {
                    tmp_msg += "2";
                }
                for(j=0; j<sib1->sched_info[i].N_sib_mapping_info; j++)
                {
                    if(j > 0 || (i == 0 && j == 0))
                    {
                        tmp_msg += ",";
                    }
                    tmp_msg += boost::lexical_cast<std::string>(liblte_rrc_sib_type_text[sib1->sched_info[i].sib_mapping_info[j].sib_type]);
                }
                tmp_msg += " ";
            }
            if(false == sib1->tdd)
            {
                tmp_msg += "duplex_mode=fdd ";
            }else{
                tmp_msg += "duplex_mode=tdd ";
                tmp_msg += "subfr_assignment=" + boost::lexical_cast<std::string>(liblte_rrc_subframe_assignment_text[sib1->sf_assignment]) + " ";
                tmp_msg += "special_subfr_patterns=" + boost::lexical_cast<std::string>(liblte_rrc_special_subframe_patterns_text[sib1->special_sf_patterns]) + " ";
            }
            tmp_msg += "si_value_tag=" + boost::lexical_cast<std::string>((uint32)sib1->system_info_value_tag) + " ";

            tmp_msg += "\n";
        }catch(boost::bad_lexical_cast &){
            tmp_msg += "\n";
        }
        ctrl_socket->send(tmp_msg);
    }
}
void LTE_fdd_dl_scan_interface::send_ctrl_sib2_decoded_msg(LTE_FDD_DL_SCAN_CHAN_DATA_STRUCT        *chan_data,
                                                           LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_2_STRUCT *sib2,
                                                           uint32                                   sfn)
{
    boost::mutex::scoped_lock lock(connect_mutex);
    std::string               tmp_msg;
    uint32                    coeff;
    uint32                    T;
    uint32                    i;

    if(ctrl_connected)
    {
        tmp_msg = "info sib2_decoded ";
        try
        {
            tmp_msg += "freq=" + boost::lexical_cast<std::string>(liblte_interface_dl_earfcn_to_frequency(current_dl_earfcn)) + " ";
            tmp_msg += "dl_earfcn=" + boost::lexical_cast<std::string>(current_dl_earfcn) + " ";
            tmp_msg += "freq_offset=" + boost::lexical_cast<std::string>(chan_data->freq_offset) + " ";
            tmp_msg += "phys_cell_id=" + boost::lexical_cast<std::string>(chan_data->N_id_cell) + " ";
            tmp_msg += "sfn=" + boost::lexical_cast<std::string>(sfn) + " ";

            if(true == sib2->ac_barring_for_emergency)
            {
                tmp_msg += "emergency_barring=enabled ";
            }else{
                tmp_msg += "emergency_barring=disabled ";
            }
            if(true == sib2->ac_barring_for_mo_signalling.enabled)
            {
                tmp_msg += "mo_signalling_barring=enabled ";
                tmp_msg += "mo_signalling_barring_factor=" + boost::lexical_cast<std::string>(liblte_rrc_ac_barring_factor_text[sib2->ac_barring_for_mo_signalling.factor]) + " ";
                tmp_msg += "mo_signalling_barring_time=" + boost::lexical_cast<std::string>(liblte_rrc_ac_barring_time_text[sib2->ac_barring_for_mo_signalling.time]) + " ";
                tmp_msg += "mo_signalling_barring_special_ac=" + boost::lexical_cast<std::string>((uint32)sib2->ac_barring_for_mo_signalling.for_special_ac) + " ";
            }else{
                tmp_msg += "mo_signalling_barring=disabled ";
            }
            if(true == sib2->ac_barring_for_mo_data.enabled)
            {
                tmp_msg += "mo_data_barring=enabled ";
                tmp_msg += "mo_data_barring_factor=" + boost::lexical_cast<std::string>(liblte_rrc_ac_barring_factor_text[sib2->ac_barring_for_mo_data.factor]) + " ";
                tmp_msg += "mo_data_barring_time=" + boost::lexical_cast<std::string>(liblte_rrc_ac_barring_time_text[sib2->ac_barring_for_mo_data.time]) + " ";
                tmp_msg += "mo_data_barring_special_ac=" + boost::lexical_cast<std::string>((uint32)sib2->ac_barring_for_mo_data.for_special_ac) + " ";
            }else{
                tmp_msg += "mo_data_barring=disabled ";
            }
            tmp_msg += "num_rach_preambles=" + boost::lexical_cast<std::string>(liblte_rrc_number_of_ra_preambles_text[sib2->rr_config_common_sib.rach_cnfg.num_ra_preambles]) + " ";
            if(true == sib2->rr_config_common_sib.rach_cnfg.preambles_group_a_cnfg.present)
            {
                tmp_msg += "size_of_rach_preambles_group_a=" + boost::lexical_cast<std::string>(liblte_rrc_size_of_ra_preambles_group_a_text[sib2->rr_config_common_sib.rach_cnfg.preambles_group_a_cnfg.size_of_ra]) + " ";
                tmp_msg += "rach_msg_size_group_a=" + boost::lexical_cast<std::string>(liblte_rrc_message_size_group_a_text[sib2->rr_config_common_sib.rach_cnfg.preambles_group_a_cnfg.msg_size]) + " ";
                tmp_msg += "rach_msg_power_offset_group_b=" + boost::lexical_cast<std::string>(liblte_rrc_message_power_offset_group_b_text[sib2->rr_config_common_sib.rach_cnfg.preambles_group_a_cnfg.msg_pwr_offset_group_b]) + " ";
            }
            tmp_msg += "power_ramping_step=" + boost::lexical_cast<std::string>(liblte_rrc_power_ramping_step_text[sib2->rr_config_common_sib.rach_cnfg.pwr_ramping_step]) + " ";
            tmp_msg += "preamble_init_target_rx_power=" + boost::lexical_cast<std::string>(liblte_rrc_preamble_initial_received_target_power_text[sib2->rr_config_common_sib.rach_cnfg.preamble_init_rx_target_pwr]) + " ";
            tmp_msg += "preamble_trans_max=" + boost::lexical_cast<std::string>(liblte_rrc_preamble_trans_max_text[sib2->rr_config_common_sib.rach_cnfg.preamble_trans_max]) + " ";
            tmp_msg += "ra_response_window_size=" + boost::lexical_cast<std::string>(liblte_rrc_ra_response_window_size_text[sib2->rr_config_common_sib.rach_cnfg.ra_resp_win_size]) + " ";
            tmp_msg += "mac_contention_resolution_timer=" + boost::lexical_cast<std::string>(liblte_rrc_mac_contention_resolution_timer_text[sib2->rr_config_common_sib.rach_cnfg.mac_con_res_timer]) + " ";
            tmp_msg += "max_num_harq_tx_for_msg_3=" + boost::lexical_cast<std::string>((uint32)sib2->rr_config_common_sib.rach_cnfg.max_harq_msg3_tx) + " ";
            tmp_msg += "modification_period_coeff=" + boost::lexical_cast<std::string>(liblte_rrc_modification_period_coeff_text[sib2->rr_config_common_sib.bcch_cnfg.modification_period_coeff]) + " ";
            coeff    = liblte_rrc_modification_period_coeff_num[sib2->rr_config_common_sib.bcch_cnfg.modification_period_coeff];
            tmp_msg += "default_paging_cycle=" + boost::lexical_cast<std::string>(liblte_rrc_default_paging_cycle_text[sib2->rr_config_common_sib.pcch_cnfg.default_paging_cycle]) + " ";
            T        = liblte_rrc_default_paging_cycle_num[sib2->rr_config_common_sib.pcch_cnfg.default_paging_cycle];
            tmp_msg += "modification_period=" + boost::lexical_cast<std::string>(coeff * T) + " ";
            tmp_msg += "n_b=" + boost::lexical_cast<std::string>((uint32)(T * liblte_rrc_nb_num[sib2->rr_config_common_sib.pcch_cnfg.nB])) + " ";
            tmp_msg += "root_sequence_index=" + boost::lexical_cast<std::string>((uint32)sib2->rr_config_common_sib.prach_cnfg.root_sequence_index) + " ";
            tmp_msg += "prach_config_index=" + boost::lexical_cast<std::string>((uint32)sib2->rr_config_common_sib.prach_cnfg.prach_cnfg_info.prach_config_index) + " ";
            switch(sib2->rr_config_common_sib.prach_cnfg.prach_cnfg_info.prach_config_index)
            {
            case 0:
                tmp_msg += "preamble_format=0 rach_sfn=even rach_subframe_num=1 ";
                break;
            case 1:
                tmp_msg += "preamble_format=0 rach_sfn=even rach_subframe_num=4 ";
                break;
            case 2:
                tmp_msg += "preamble_format=0 rach_sfn=even rach_subframe_num=7 ";
                break;
            case 3:
                tmp_msg += "preamble_format=0 rach_sfn=any rach_subframe_num=1 ";
                break;
            case 4:
                tmp_msg += "preamble_format=0 rach_sfn=any rach_subframe_num=4 ";
                break;
            case 5:
                tmp_msg += "preamble_format=0 rach_sfn=any rach_subframe_num=7 ";
                break;
            case 6:
                tmp_msg += "preamble_format=0 rach_sfn=any rach_subframe_num=1,6 ";
                break;
            case 7:
                tmp_msg += "preamble_format=0 rach_sfn=any rach_subframe_num=2,7 ";
                break;
            case 8:
                tmp_msg += "preamble_format=0 rach_sfn=any rach_subframe_num=3,8 ";
                break;
            case 9:
                tmp_msg += "preamble_format=0 rach_sfn=any rach_subframe_num=1,4,7 ";
                break;
            case 10:
                tmp_msg += "preamble_format=0 rach_sfn=any rach_subframe_num=2,5,8 ";
                break;
            case 11:
                tmp_msg += "preamble_format=0 rach_sfn=any rach_subframe_num=3,6,9 ";
                break;
            case 12:
                tmp_msg += "preamble_format=0 rach_sfn=any rach_subframe_num=0,2,4,6,8 ";
                break;
            case 13:
                tmp_msg += "preamble_format=0 rach_sfn=any rach_subframe_num=1,3,5,7,9 ";
                break;
            case 14:
                tmp_msg += "preamble_format=0 rach_sfn=any rach_subframe_num=0,1,2,3,4,5,6,7,8,9 ";
                break;
            case 15:
                tmp_msg += "preamble_format=0 rach_sfn=even rach_subframe_num=9 ";
                break;
            case 16:
                tmp_msg += "preamble_format=1 rach_sfn=even rach_subframe_num=1 ";
                break;
            case 17:
                tmp_msg += "preamble_format=1 rach_sfn=even rach_subframe_num=4 ";
                break;
            case 18:
                tmp_msg += "preamble_format=1 rach_sfn=even rach_subframe_num=7 ";
                break;
            case 19:
                tmp_msg += "preamble_format=1 rach_sfn=any rach_subframe_num=1 ";
                break;
            case 20:
                tmp_msg += "preamble_format=1 rach_sfn=any rach_subframe_num=4 ";
                break;
            case 21:
                tmp_msg += "preamble_format=1 rach_sfn=any rach_subframe_num=7 ";
                break;
            case 22:
                tmp_msg += "preamble_format=1 rach_sfn=any rach_subframe_num=1,6 ";
                break;
            case 23:
                tmp_msg += "preamble_format=1 rach_sfn=any rach_subframe_num=2,7 ";
                break;
            case 24:
                tmp_msg += "preamble_format=1 rach_sfn=any rach_subframe_num=3,8 ";
                break;
            case 25:
                tmp_msg += "preamble_format=1 rach_sfn=any rach_subframe_num=1,4,7 ";
                break;
            case 26:
                tmp_msg += "preamble_format=1 rach_sfn=any rach_subframe_num=2,5,8 ";
                break;
            case 27:
                tmp_msg += "preamble_format=1 rach_sfn=any rach_subframe_num=3,6,9 ";
                break;
            case 28:
                tmp_msg += "preamble_format=1 rach_sfn=any rach_subframe_num=0,2,4,6,8 ";
                break;
            case 29:
                tmp_msg += "preamble_format=1 rach_sfn=any rach_subframe_num=1,3,5,7,9 ";
                break;
            case 30:
                tmp_msg += "preamble_format=n/a rach_sfn=n/a rach_subframe_num=n/a ";
                break;
            case 31:
                tmp_msg += "preamble_format=1 rach_sfn=even rach_subframe_num=9 ";
                break;
            case 32:
                tmp_msg += "preamble_format=2 rach_sfn=even rach_subframe_num=1 ";
                break;
            case 33:
                tmp_msg += "preamble_format=2 rach_sfn=even rach_subframe_num=4 ";
                break;
            case 34:
                tmp_msg += "preamble_format=2 rach_sfn=even rach_subframe_num=7 ";
                break;
            case 35:
                tmp_msg += "preamble_format=2 rach_sfn=any rach_subframe_num=1 ";
                break;
            case 36:
                tmp_msg += "preamble_format=2 rach_sfn=any rach_subframe_num=4 ";
                break;
            case 37:
                tmp_msg += "preamble_format=2 rach_sfn=any rach_subframe_num=7 ";
                break;
            case 38:
                tmp_msg += "preamble_format=2 rach_sfn=any rach_subframe_num=1,6 ";
                break;
            case 39:
                tmp_msg += "preamble_format=2 rach_sfn=any rach_subframe_num=2,7 ";
                break;
            case 40:
                tmp_msg += "preamble_format=2 rach_sfn=any rach_subframe_num=3,8 ";
                break;
            case 41:
                tmp_msg += "preamble_format=2 rach_sfn=any rach_subframe_num=1,4,7 ";
                break;
            case 42:
                tmp_msg += "preamble_format=2 rach_sfn=any rach_subframe_num=2,5,8 ";
                break;
            case 43:
                tmp_msg += "preamble_format=2 rach_sfn=any rach_subframe_num=3,6,9 ";
                break;
            case 44:
                tmp_msg += "preamble_format=2 rach_sfn=any rach_subframe_num=0,2,4,6,8 ";
                break;
            case 45:
                tmp_msg += "preamble_format=2 rach_sfn=any rach_subframe_num=1,3,5,7,9 ";
                break;
            case 46:
                tmp_msg += "preamble_format=n/a rach_sfn=n/a rach_subframe_num=n/a ";
                break;
            case 47:
                tmp_msg += "preamble_format=2 rach_sfn=even rach_subframe_num=9 ";
                break;
            case 48:
                tmp_msg += "preamble_format=3 rach_sfn=even rach_subframe_num=1 ";
                break;
            case 49:
                tmp_msg += "preamble_format=3 rach_sfn=even rach_subframe_num=4 ";
                break;
            case 50:
                tmp_msg += "preamble_format=3 rach_sfn=even rach_subframe_num=7 ";
                break;
            case 51:
                tmp_msg += "preamble_format=3 rach_sfn=any rach_subframe_num=1 ";
                break;
            case 52:
                tmp_msg += "preamble_format=3 rach_sfn=any rach_subframe_num=4 ";
                break;
            case 53:
                tmp_msg += "preamble_format=3 rach_sfn=any rach_subframe_num=7 ";
                break;
            case 54:
                tmp_msg += "preamble_format=3 rach_sfn=any rach_subframe_num=1,6 ";
                break;
            case 55:
                tmp_msg += "preamble_format=3 rach_sfn=any rach_subframe_num=2,7 ";
                break;
            case 56:
                tmp_msg += "preamble_format=3 rach_sfn=any rach_subframe_num=3,8 ";
                break;
            case 57:
                tmp_msg += "preamble_format=3 rach_sfn=any rach_subframe_num=1,4,7 ";
                break;
            case 58:
                tmp_msg += "preamble_format=3 rach_sfn=any rach_subframe_num=2,5,8 ";
                break;
            case 59:
                tmp_msg += "preamble_format=3 rach_sfn=any rach_subframe_num=3,6,9 ";
                break;
            case 60:
                tmp_msg += "preamble_format=n/a rach_sfn=n/a rach_subframe_num=n/a ";
                break;
            case 61:
                tmp_msg += "preamble_format=n/a rach_sfn=n/a rach_subframe_num=n/a ";
                break;
            case 62:
                tmp_msg += "preamble_format=n/a rach_sfn=n/a rach_subframe_num=n/a ";
                break;
            case 63:
                tmp_msg += "preamble_format=3 rach_sfn=even rach_subframe_num=9 ";
                break;
            }
            if(true == sib2->rr_config_common_sib.prach_cnfg.prach_cnfg_info.high_speed_flag)
            {
                tmp_msg += "high_speed_flag=restricted_set ";
            }else{
                tmp_msg += "high_speed_flag=unrestricted_set ";
            }
            tmp_msg += "n_cs_config=" + boost::lexical_cast<std::string>((uint32)sib2->rr_config_common_sib.prach_cnfg.prach_cnfg_info.zero_correlation_zone_config) + " ";
            tmp_msg += "prach_freq_offset=" + boost::lexical_cast<std::string>((uint32)sib2->rr_config_common_sib.prach_cnfg.prach_cnfg_info.prach_freq_offset) + " ";
            tmp_msg += "reference_signal_power=" + boost::lexical_cast<std::string>((int32)sib2->rr_config_common_sib.pdsch_cnfg.rs_power) + " ";
            tmp_msg += "p_b=" + boost::lexical_cast<std::string>((uint32)sib2->rr_config_common_sib.pdsch_cnfg.p_b) + " ";
            tmp_msg += "n_sb=" + boost::lexical_cast<std::string>((uint32)sib2->rr_config_common_sib.pusch_cnfg.n_sb) + " ";
            switch(sib2->rr_config_common_sib.pusch_cnfg.hopping_mode)
            {
            case LIBLTE_RRC_HOPPING_MODE_INTER_SUBFRAME:
                tmp_msg += "hopping_mode=inter_subframe ";
                break;
            case LIBLTE_RRC_HOPPING_MODE_INTRA_AND_INTER_SUBFRAME:
                tmp_msg += "hopping_mode=intra_and_inter_subframe ";
                break;
            }
            tmp_msg += "pusch_n_rb_hopping_offset=" + boost::lexical_cast<std::string>((uint32)sib2->rr_config_common_sib.pusch_cnfg.pusch_hopping_offset) + " ";
            if(true == sib2->rr_config_common_sib.pusch_cnfg.enable_64_qam)
            {
                tmp_msg += "64_qam=allowed ";
            }else{
                tmp_msg += "64_qam=not_allowed ";
            }
            if(true == sib2->rr_config_common_sib.pusch_cnfg.ul_rs.group_hopping_enabled)
            {
                tmp_msg += "group_hopping=enabled ";
            }else{
                tmp_msg += "group_hopping=disabled ";
            }
            tmp_msg += "group_assignment_pusch=" + boost::lexical_cast<std::string>((uint32)sib2->rr_config_common_sib.pusch_cnfg.ul_rs.group_assignment_pusch) + " ";
            if(true == sib2->rr_config_common_sib.pusch_cnfg.ul_rs.sequence_hopping_enabled)
            {
                tmp_msg += "sequence_hopping=enabled ";
            }else{
                tmp_msg += "sequence_hopping=disabled ";
            }
            tmp_msg += "cyclic_shift=" + boost::lexical_cast<std::string>((uint32)sib2->rr_config_common_sib.pusch_cnfg.ul_rs.cyclic_shift) + " ";
            tmp_msg += "delta_pucch_shift=" + boost::lexical_cast<std::string>(liblte_rrc_delta_pucch_shift_text[sib2->rr_config_common_sib.pucch_cnfg.delta_pucch_shift]) + " ";
            tmp_msg += "n_rb_cqi=" + boost::lexical_cast<std::string>((uint32)sib2->rr_config_common_sib.pucch_cnfg.n_rb_cqi) + " ";
            tmp_msg += "n_cs_an=" + boost::lexical_cast<std::string>((uint32)sib2->rr_config_common_sib.pucch_cnfg.n_cs_an) + " ";
            tmp_msg += "n1_pucch_an=" + boost::lexical_cast<std::string>((uint32)sib2->rr_config_common_sib.pucch_cnfg.n1_pucch_an) + " ";
            if(true == sib2->rr_config_common_sib.srs_ul_cnfg.present)
            {
                tmp_msg += "srs_bw_cnfg=" + boost::lexical_cast<std::string>(liblte_rrc_srs_bw_config_text[sib2->rr_config_common_sib.srs_ul_cnfg.bw_cnfg]) + " ";
                tmp_msg += "srs_subframe_cnfg=" + boost::lexical_cast<std::string>(liblte_rrc_srs_subfr_config_text[sib2->rr_config_common_sib.srs_ul_cnfg.subfr_cnfg]) + " ";
                if(true == sib2->rr_config_common_sib.srs_ul_cnfg.ack_nack_simul_tx)
                {
                    tmp_msg += "simultaneous_an_and_srs=true ";
                }else{
                    tmp_msg += "simultaneous_an_and_srs=false ";
                }
                if(true == sib2->rr_config_common_sib.srs_ul_cnfg.max_up_pts_present)
                {
                    tmp_msg += "srs_max_up_pts=true ";
                }else{
                    tmp_msg += "srs_max_up_pts=false ";
                }
            }
            tmp_msg += "p0_nominal_pusch=" + boost::lexical_cast<std::string>((int32)sib2->rr_config_common_sib.ul_pwr_ctrl.p0_nominal_pusch) + " ";
            tmp_msg += "ul_power_ctrl_alpha=" + boost::lexical_cast<std::string>(liblte_rrc_ul_power_control_alpha_text[sib2->rr_config_common_sib.ul_pwr_ctrl.alpha]) + " ";
            tmp_msg += "p0_nominal_pucch=" + boost::lexical_cast<std::string>((int32)sib2->rr_config_common_sib.ul_pwr_ctrl.p0_nominal_pucch) + " ";
            tmp_msg += "delta_f_pucch_format_1=" + boost::lexical_cast<std::string>(liblte_rrc_delta_f_pucch_format_1_text[sib2->rr_config_common_sib.ul_pwr_ctrl.delta_flist_pucch.format_1]) + " ";
            tmp_msg += "delta_f_pucch_format_1b=" + boost::lexical_cast<std::string>(liblte_rrc_delta_f_pucch_format_1b_text[sib2->rr_config_common_sib.ul_pwr_ctrl.delta_flist_pucch.format_1b]) + " ";
            tmp_msg += "delta_f_pucch_format_2=" + boost::lexical_cast<std::string>(liblte_rrc_delta_f_pucch_format_2_text[sib2->rr_config_common_sib.ul_pwr_ctrl.delta_flist_pucch.format_2]) + " ";
            tmp_msg += "delta_f_pucch_format_2a=" + boost::lexical_cast<std::string>(liblte_rrc_delta_f_pucch_format_2a_text[sib2->rr_config_common_sib.ul_pwr_ctrl.delta_flist_pucch.format_2a]) + " ";
            tmp_msg += "delta_f_pucch_format_2b=" + boost::lexical_cast<std::string>(liblte_rrc_delta_f_pucch_format_2b_text[sib2->rr_config_common_sib.ul_pwr_ctrl.delta_flist_pucch.format_2b]) + " ";
            tmp_msg += "delta_preamble_msg_3=" + boost::lexical_cast<std::string>((int32)sib2->rr_config_common_sib.ul_pwr_ctrl.delta_preamble_msg3) + " ";
            switch(sib2->rr_config_common_sib.ul_cp_length)
            {
            case LIBLTE_RRC_UL_CP_LENGTH_1:
                tmp_msg += "ul_cp_length=normal ";
                break;
            case LIBLTE_RRC_UL_CP_LENGTH_2:
                tmp_msg += "ul_cp_length=extended ";
                break;
            }
            tmp_msg += "t300=" + boost::lexical_cast<std::string>(liblte_rrc_t300_text[sib2->ue_timers_and_constants.t300]) + " ";
            tmp_msg += "t301=" + boost::lexical_cast<std::string>(liblte_rrc_t301_text[sib2->ue_timers_and_constants.t301]) + " ";
            tmp_msg += "t310=" + boost::lexical_cast<std::string>(liblte_rrc_t310_text[sib2->ue_timers_and_constants.t310]) + " ";
            tmp_msg += "n310=" + boost::lexical_cast<std::string>(liblte_rrc_n310_text[sib2->ue_timers_and_constants.n310]) + " ";
            tmp_msg += "t311=" + boost::lexical_cast<std::string>(liblte_rrc_t311_text[sib2->ue_timers_and_constants.t311]) + " ";
            tmp_msg += "n311=" + boost::lexical_cast<std::string>(liblte_rrc_n311_text[sib2->ue_timers_and_constants.n311]) + " ";
            if(true == sib2->arfcn_value_eutra.present)
            {
                tmp_msg += "ul_earfcn=" + boost::lexical_cast<std::string>((uint32)sib2->arfcn_value_eutra.value) + " ";
            }
            if(true == sib2->ul_bw.present)
            {
                tmp_msg += "ul_bw=" + boost::lexical_cast<std::string>(liblte_rrc_ul_bw_text[sib2->ul_bw.bw]) + " ";
            }
            tmp_msg += "additional_spectrum_emission=" + boost::lexical_cast<std::string>((uint32)sib2->additional_spectrum_emission) + " ";
            for(i=0; i<sib2->mbsfn_subfr_cnfg_list_size; i++)
            {
                tmp_msg += "radio_frame_alloc_period[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>(liblte_rrc_radio_frame_allocation_period_text[sib2->mbsfn_subfr_cnfg[i].radio_fr_alloc_period]) + " ";
                tmp_msg += "radio_frame_alloc_offset[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>(sib2->mbsfn_subfr_cnfg[i].subfr_alloc) + " ";
                tmp_msg += "subframe_alloc_num_frames[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>(liblte_rrc_subframe_allocation_num_frames_text[sib2->mbsfn_subfr_cnfg[i].subfr_alloc_num_frames]) + " ";
                tmp_msg += "subframe_alloc[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>(sib2->mbsfn_subfr_cnfg[i].subfr_alloc) + " ";
            }
            tmp_msg += "time_alignment_timer=" + boost::lexical_cast<std::string>(liblte_rrc_time_alignment_timer_text[sib2->time_alignment_timer]) + " ";

            tmp_msg += "\n";
        }catch(boost::bad_lexical_cast &){
            tmp_msg += "\n";
        }
        ctrl_socket->send(tmp_msg);
    }
}
void LTE_fdd_dl_scan_interface::send_ctrl_sib3_decoded_msg(LTE_FDD_DL_SCAN_CHAN_DATA_STRUCT        *chan_data,
                                                           LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_3_STRUCT *sib3,
                                                           uint32                                   sfn)
{
    boost::mutex::scoped_lock lock(connect_mutex);
    std::string               tmp_msg;

    if(ctrl_connected)
    {
        tmp_msg = "info sib3_decoded ";
        try
        {
            tmp_msg += "freq=" + boost::lexical_cast<std::string>(liblte_interface_dl_earfcn_to_frequency(current_dl_earfcn)) + " ";
            tmp_msg += "dl_earfcn=" + boost::lexical_cast<std::string>(current_dl_earfcn) + " ";
            tmp_msg += "freq_offset=" + boost::lexical_cast<std::string>(chan_data->freq_offset) + " ";
            tmp_msg += "phys_cell_id=" + boost::lexical_cast<std::string>(chan_data->N_id_cell) + " ";
            tmp_msg += "sfn=" + boost::lexical_cast<std::string>(sfn) + " ";

            tmp_msg += "q_hyst=" + boost::lexical_cast<std::string>(liblte_rrc_q_hyst_text[sib3->q_hyst]) + " ";
            if(true == sib3->speed_state_resel_params.present)
            {
                tmp_msg += "t_evaluation=" + boost::lexical_cast<std::string>(liblte_rrc_t_evaluation_text[sib3->speed_state_resel_params.mobility_state_params.t_eval]) + " ";
                tmp_msg += "t_hyst_normal=" + boost::lexical_cast<std::string>(liblte_rrc_t_hyst_normal_text[sib3->speed_state_resel_params.mobility_state_params.t_hyst_normal]) + " ";
                tmp_msg += "n_cell_change_medium=" + boost::lexical_cast<std::string>((uint32)sib3->speed_state_resel_params.mobility_state_params.n_cell_change_medium) + " ";
                tmp_msg += "n_cell_change_high=" + boost::lexical_cast<std::string>((uint32)sib3->speed_state_resel_params.mobility_state_params.n_cell_change_high) + " ";
                tmp_msg += "q_hyst_sf_medium=" + boost::lexical_cast<std::string>(liblte_rrc_sf_medium_text[sib3->speed_state_resel_params.q_hyst_sf.medium]) + " ";
                tmp_msg += "q_hyst_sf_high=" + boost::lexical_cast<std::string>(liblte_rrc_sf_high_text[sib3->speed_state_resel_params.q_hyst_sf.high]) + " ";
            }
            if(true == sib3->s_non_intra_search_present)
            {
                tmp_msg += "s_non_intra_search=" + boost::lexical_cast<std::string>((uint32)sib3->s_non_intra_search) + " ";
            }
            tmp_msg += "threshold_serving_low=" + boost::lexical_cast<std::string>((uint32)sib3->thresh_serving_low) + " ";
            tmp_msg += "cell_resel_priority=" + boost::lexical_cast<std::string>((uint32)sib3->cell_resel_prio) + " ";
            tmp_msg += "q_rx_lev_min=" + boost::lexical_cast<std::string>((int32)sib3->q_rx_lev_min) + " ";
            if(true == sib3->p_max_present)
            {
                tmp_msg += "p_max=" + boost::lexical_cast<std::string>((int32)sib3->p_max) + " ";
            }
            if(true == sib3->s_intra_search_present)
            {
                tmp_msg += "s_intra_search=" + boost::lexical_cast<std::string>((uint32)sib3->s_intra_search) + " ";
            }
            if(true == sib3->allowed_meas_bw_present)
            {
                tmp_msg += "allowed_meas_bw=" + boost::lexical_cast<std::string>(liblte_rrc_allowed_meas_bandwidth_text[sib3->allowed_meas_bw]) + " ";
            }
            if(true == sib3->presence_ant_port_1)
            {
                tmp_msg += "presence_ant_port_1=true ";
            }else{
                tmp_msg += "presence_ant_port_1=false ";
            }
            tmp_msg += "neigh_cell_cnfg=" + boost::lexical_cast<std::string>((uint32)sib3->neigh_cell_cnfg) + " ";
            tmp_msg += "t_resel_eutra=" + boost::lexical_cast<std::string>((uint32)sib3->t_resel_eutra) + " ";
            if(true == sib3->t_resel_eutra_sf_present)
            {
                tmp_msg += "t_resel_eutra_sf_medium=" + boost::lexical_cast<std::string>(liblte_rrc_sssf_medium_text[sib3->t_resel_eutra_sf.sf_medium]) + " ";
                tmp_msg += "t_resel_eutra_sf_high=" + boost::lexical_cast<std::string>(liblte_rrc_sssf_high_text[sib3->t_resel_eutra_sf.sf_high]) + " ";
            }

            tmp_msg += "\n";
        }catch(boost::bad_lexical_cast &){
            tmp_msg += "\n";
        }
        ctrl_socket->send(tmp_msg);
    }
}
void LTE_fdd_dl_scan_interface::send_ctrl_sib4_decoded_msg(LTE_FDD_DL_SCAN_CHAN_DATA_STRUCT        *chan_data,
                                                           LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_4_STRUCT *sib4,
                                                           uint32                                   sfn)
{
    boost::mutex::scoped_lock lock(connect_mutex);
    std::string               tmp_msg;
    uint32                    i;

    if(ctrl_connected)
    {
        tmp_msg = "info sib4_decoded ";
        try
        {
            tmp_msg += "freq=" + boost::lexical_cast<std::string>(liblte_interface_dl_earfcn_to_frequency(current_dl_earfcn)) + " ";
            tmp_msg += "dl_earfcn=" + boost::lexical_cast<std::string>(current_dl_earfcn) + " ";
            tmp_msg += "freq_offset=" + boost::lexical_cast<std::string>(chan_data->freq_offset) + " ";
            tmp_msg += "phys_cell_id=" + boost::lexical_cast<std::string>(chan_data->N_id_cell) + " ";
            tmp_msg += "sfn=" + boost::lexical_cast<std::string>(sfn) + " ";

            for(i=0; i<sib4->intra_freq_neigh_cell_list_size; i++)
            {
                tmp_msg += "neigh_phys_cell_id[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>((uint32)sib4->intra_freq_neigh_cell_list[i].phys_cell_id) + " ";
                tmp_msg += "neigh_q_offset_range[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>(liblte_rrc_q_offset_range_text[sib4->intra_freq_neigh_cell_list[i].q_offset_range]) + " ";
            }
            for(i=0; i<sib4->intra_freq_black_cell_list_size; i++)
            {
                tmp_msg += "blacklist_phys_cell_id_begin[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>((uint32)sib4->intra_freq_black_cell_list[i].start) + " ";
                tmp_msg += "blacklist_phys_cell_id_end[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>((uint32)sib4->intra_freq_black_cell_list[i].start + liblte_rrc_phys_cell_id_range_num[sib4->intra_freq_black_cell_list[i].range]) + " ";
            }
            if(true == sib4->csg_phys_cell_id_range_present)
            {
                tmp_msg += "csg_phys_cell_id_begin=" + boost::lexical_cast<std::string>((uint32)sib4->csg_phys_cell_id_range.start) + " ";
                tmp_msg += "csg_phys_cell_id_end=" + boost::lexical_cast<std::string>((uint32)sib4->csg_phys_cell_id_range.start + liblte_rrc_phys_cell_id_range_num[sib4->csg_phys_cell_id_range.range]) + " ";
            }

            tmp_msg += "\n";
        }catch(boost::bad_lexical_cast &){
            tmp_msg += "\n";
        }
        ctrl_socket->send(tmp_msg);
    }
}
void LTE_fdd_dl_scan_interface::send_ctrl_sib5_decoded_msg(LTE_FDD_DL_SCAN_CHAN_DATA_STRUCT        *chan_data,
                                                           LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_5_STRUCT *sib5,
                                                           uint32                                   sfn)
{
    boost::mutex::scoped_lock lock(connect_mutex);
    std::string               tmp_msg;
    uint32                    i;
    uint32                    j;

    if(ctrl_connected)
    {
        tmp_msg = "info sib5_decoded ";
        try
        {
            tmp_msg += "freq=" + boost::lexical_cast<std::string>(liblte_interface_dl_earfcn_to_frequency(current_dl_earfcn)) + " ";
            tmp_msg += "dl_earfcn=" + boost::lexical_cast<std::string>(current_dl_earfcn) + " ";
            tmp_msg += "freq_offset=" + boost::lexical_cast<std::string>(chan_data->freq_offset) + " ";
            tmp_msg += "phys_cell_id=" + boost::lexical_cast<std::string>(chan_data->N_id_cell) + " ";
            tmp_msg += "sfn=" + boost::lexical_cast<std::string>(sfn) + " ";

            for(i=0; i<sib5->inter_freq_carrier_freq_list_size; i++)
            {
                tmp_msg += "earfcn[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>((uint32)sib5->inter_freq_carrier_freq_list[i].dl_carrier_freq) + " ";
                tmp_msg += "q_rx_lev_min[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>((int32)sib5->inter_freq_carrier_freq_list[i].q_rx_lev_min) + " ";
                if(true == sib5->inter_freq_carrier_freq_list[i].p_max_present)
                {
                    tmp_msg += "p_max[" + boost::lexical_cast<std::string>(i) + "]=";
                    tmp_msg += boost::lexical_cast<std::string>((int32)sib5->inter_freq_carrier_freq_list[i].p_max) + " ";
                }
                tmp_msg += "t_resel_eutra[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>((uint32)sib5->inter_freq_carrier_freq_list[i].t_resel_eutra) + " ";
                if(true == sib5->inter_freq_carrier_freq_list[i].t_resel_eutra_sf_present)
                {
                    tmp_msg += "t_resel_eutra_sf_medium[" + boost::lexical_cast<std::string>(i) + "]=";
                    tmp_msg += boost::lexical_cast<std::string>(liblte_rrc_sssf_medium_text[sib5->inter_freq_carrier_freq_list[i].t_resel_eutra_sf.sf_medium]) + " ";
                    tmp_msg += "t_resel_eutra_sf_high[" + boost::lexical_cast<std::string>(i) + "]=";
                    tmp_msg += boost::lexical_cast<std::string>(liblte_rrc_sssf_high_text[sib5->inter_freq_carrier_freq_list[i].t_resel_eutra_sf.sf_high]) + " ";
                }
                tmp_msg += "thresh_x_high[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>((uint32)sib5->inter_freq_carrier_freq_list[i].threshx_high) + " ";
                tmp_msg += "thresh_x_low[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>((uint32)sib5->inter_freq_carrier_freq_list[i].threshx_low) + " ";
                tmp_msg += "allowed_meas_bw[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>(liblte_rrc_allowed_meas_bandwidth_text[sib5->inter_freq_carrier_freq_list[i].allowed_meas_bw]) + " ";
                if(true == sib5->inter_freq_carrier_freq_list[i].presence_ant_port_1)
                {
                    tmp_msg += "presence_ant_port_1[" + boost::lexical_cast<std::string>(i) + "]=true ";
                }else{
                    tmp_msg += "presence_ant_port_1[" + boost::lexical_cast<std::string>(i) + "]=false ";
                }
                if(true == sib5->inter_freq_carrier_freq_list[i].cell_resel_prio_present)
                {
                    tmp_msg += "cell_resel_prio[" + boost::lexical_cast<std::string>(i) + "]=";
                    tmp_msg += boost::lexical_cast<std::string>((uint32)sib5->inter_freq_carrier_freq_list[i].cell_resel_prio) + " ";
                }
                tmp_msg += "neigh_cell_cnfg[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>((uint32)sib5->inter_freq_carrier_freq_list[i].neigh_cell_cnfg) + " ";
                tmp_msg += "q_offset_freq[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>(liblte_rrc_q_offset_range_text[sib5->inter_freq_carrier_freq_list[i].q_offset_freq]) + " ";
                if(0 != sib5->inter_freq_carrier_freq_list[i].inter_freq_neigh_cell_list_size)
                {
                    for(j=0; j<sib5->inter_freq_carrier_freq_list[i].inter_freq_neigh_cell_list_size; j++)
                    {
                        tmp_msg += "phys_cell_id[" + boost::lexical_cast<std::string>(i) + "][" + boost::lexical_cast<std::string>(j) + "]=";
                        tmp_msg += boost::lexical_cast<std::string>((uint32)sib5->inter_freq_carrier_freq_list[i].inter_freq_neigh_cell_list[j].phys_cell_id) + " ";
                        tmp_msg += "q_offset_cell[" + boost::lexical_cast<std::string>(i) + "][" + boost::lexical_cast<std::string>(j) + "]=";
                        tmp_msg += boost::lexical_cast<std::string>(liblte_rrc_q_offset_range_text[sib5->inter_freq_carrier_freq_list[i].inter_freq_neigh_cell_list[j].q_offset_cell]) + " ";
                    }
                }
                if(0 != sib5->inter_freq_carrier_freq_list[i].inter_freq_black_cell_list_size)
                {
                    for(j=0; j<sib5->inter_freq_carrier_freq_list[i].inter_freq_black_cell_list_size; j++)
                    {
                        tmp_msg += "blacklisted_earfcn_start[" + boost::lexical_cast<std::string>(i) + "][" + boost::lexical_cast<std::string>(j) + "]=";
                        tmp_msg += boost::lexical_cast<std::string>((uint32)sib5->inter_freq_carrier_freq_list[i].inter_freq_black_cell_list[j].start) + " ";
                        tmp_msg += "blacklisted_earfcn_end[" + boost::lexical_cast<std::string>(i) + "][" + boost::lexical_cast<std::string>(j) + "]=";
                        tmp_msg += boost::lexical_cast<std::string>((uint32)sib5->inter_freq_carrier_freq_list[i].inter_freq_black_cell_list[j].start + liblte_rrc_phys_cell_id_range_num[sib5->inter_freq_carrier_freq_list[i].inter_freq_black_cell_list[j].range]) + " ";
                    }
                }
            }

            tmp_msg += "\n";
        }catch(boost::bad_lexical_cast &){
            tmp_msg += "\n";
        }
        ctrl_socket->send(tmp_msg);
    }
}
void LTE_fdd_dl_scan_interface::send_ctrl_sib6_decoded_msg(LTE_FDD_DL_SCAN_CHAN_DATA_STRUCT        *chan_data,
                                                           LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_6_STRUCT *sib6,
                                                           uint32                                   sfn)
{
    boost::mutex::scoped_lock lock(connect_mutex);
    std::string               tmp_msg;
    uint32                    i;

    if(ctrl_connected)
    {
        tmp_msg = "info sib6_decoded ";
        try
        {
            tmp_msg += "freq=" + boost::lexical_cast<std::string>(liblte_interface_dl_earfcn_to_frequency(current_dl_earfcn)) + " ";
            tmp_msg += "dl_earfcn=" + boost::lexical_cast<std::string>(current_dl_earfcn) + " ";
            tmp_msg += "freq_offset=" + boost::lexical_cast<std::string>(chan_data->freq_offset) + " ";
            tmp_msg += "phys_cell_id=" + boost::lexical_cast<std::string>(chan_data->N_id_cell) + " ";
            tmp_msg += "sfn=" + boost::lexical_cast<std::string>(sfn) + " ";

            for(i=0; i<sib6->carrier_freq_list_utra_fdd_size; i++)
            {
                tmp_msg += "uarfcn[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>((uint32)sib6->carrier_freq_list_utra_fdd[i].carrier_freq) + " ";
                if(true == sib6->carrier_freq_list_utra_fdd[i].cell_resel_prio_present)
                {
                    tmp_msg += "cell_resel_prio[" + boost::lexical_cast<std::string>(i) + "]=";
                    tmp_msg += boost::lexical_cast<std::string>((uint32)sib6->carrier_freq_list_utra_fdd[i].cell_resel_prio) + " ";
                }
                tmp_msg += "thresh_x_high[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>((uint32)sib6->carrier_freq_list_utra_fdd[i].threshx_high) + " ";
                tmp_msg += "thresh_x_low[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>((uint32)sib6->carrier_freq_list_utra_fdd[i].threshx_low) + " ";
                tmp_msg += "q_rx_lev_min[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>((int32)sib6->carrier_freq_list_utra_fdd[i].q_rx_lev_min) + " ";
                tmp_msg += "p_max_utra[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>((int32)sib6->carrier_freq_list_utra_fdd[i].p_max_utra) + " ";
                tmp_msg += "q_qual_min[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>((int32)sib6->carrier_freq_list_utra_fdd[i].q_qual_min) + " ";
            }
            for(i=0; i<sib6->carrier_freq_list_utra_tdd_size; i++)
            {
                tmp_msg += "uarfcn[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>((uint32)sib6->carrier_freq_list_utra_tdd[i].carrier_freq) + " ";
                if(true == sib6->carrier_freq_list_utra_tdd[i].cell_resel_prio_present)
                {
                    tmp_msg += "cell_resel_prio[" + boost::lexical_cast<std::string>(i) + "]=";
                    tmp_msg += boost::lexical_cast<std::string>((uint32)sib6->carrier_freq_list_utra_tdd[i].cell_resel_prio) + " ";
                }
                tmp_msg += "thresh_x_high[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>((uint32)sib6->carrier_freq_list_utra_tdd[i].threshx_high) + " ";
                tmp_msg += "thresh_x_low[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>((uint32)sib6->carrier_freq_list_utra_tdd[i].threshx_low) + " ";
                tmp_msg += "q_rx_lev_min[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>((int32)sib6->carrier_freq_list_utra_tdd[i].q_rx_lev_min) + " ";
                tmp_msg += "p_max_utra[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>((int32)sib6->carrier_freq_list_utra_tdd[i].p_max_utra) + " ";
            }
            tmp_msg += "t_resel_utra=" + boost::lexical_cast<std::string>((uint32)sib6->t_resel_utra) + " ";
            if(true == sib6->t_resel_utra_sf_present)
            {
                tmp_msg += "t_resel_utra_sf_medium=" + boost::lexical_cast<std::string>(liblte_rrc_sssf_medium_text[sib6->t_resel_utra_sf.sf_medium]) + " ";
                tmp_msg += "t_resel_utra_sf_high=" + boost::lexical_cast<std::string>(liblte_rrc_sssf_high_text[sib6->t_resel_utra_sf.sf_high]) + " ";
            }

            tmp_msg += "\n";
        }catch(boost::bad_lexical_cast &){
            tmp_msg += "\n";
        }
        ctrl_socket->send(tmp_msg);
    }
}
void LTE_fdd_dl_scan_interface::send_ctrl_sib7_decoded_msg(LTE_FDD_DL_SCAN_CHAN_DATA_STRUCT        *chan_data,
                                                           LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_7_STRUCT *sib7,
                                                           uint32                                   sfn)
{
    boost::mutex::scoped_lock lock(connect_mutex);
    std::string               tmp_msg;
    uint32                    i;
    uint32                    j;

    if(ctrl_connected)
    {
        tmp_msg = "info sib7_decoded ";
        try
        {
            tmp_msg += "freq=" + boost::lexical_cast<std::string>(liblte_interface_dl_earfcn_to_frequency(current_dl_earfcn)) + " ";
            tmp_msg += "dl_earfcn=" + boost::lexical_cast<std::string>(current_dl_earfcn) + " ";
            tmp_msg += "freq_offset=" + boost::lexical_cast<std::string>(chan_data->freq_offset) + " ";
            tmp_msg += "phys_cell_id=" + boost::lexical_cast<std::string>(chan_data->N_id_cell) + " ";
            tmp_msg += "sfn=" + boost::lexical_cast<std::string>(sfn) + " ";

            tmp_msg += "t_resel_geran=" + boost::lexical_cast<std::string>((uint32)sib7->t_resel_geran) + " ";
            if(true == sib7->t_resel_geran_sf_present)
            {
                tmp_msg += "t_resel_geran_sf_medium=" + boost::lexical_cast<std::string>(liblte_rrc_sssf_medium_text[sib7->t_resel_geran_sf.sf_medium]) + " ";
                tmp_msg += "t_resel_geran_sf_high=" + boost::lexical_cast<std::string>(liblte_rrc_sssf_high_text[sib7->t_resel_geran_sf.sf_high]) + " ";
            }
            for(i=0; i<sib7->carrier_freqs_info_list_size; i++)
            {
                tmp_msg += "geran_neigh_starting_arfcn[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>((uint32)sib7->carrier_freqs_info_list[i].carrier_freqs.starting_arfcn) + " ";
                tmp_msg += "geran_neigh_band_ind[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>(liblte_rrc_band_indicator_geran_text[sib7->carrier_freqs_info_list[i].carrier_freqs.band_indicator]) + " ";
                if(LIBLTE_RRC_FOLLOWING_ARFCNS_EXPLICIT_LIST == sib7->carrier_freqs_info_list[i].carrier_freqs.following_arfcns)
                {
                    for(j=0; j<sib7->carrier_freqs_info_list[i].carrier_freqs.explicit_list_of_arfcns_size; j++)
                    {
                        tmp_msg += "geran_neigh_arfcns[" + boost::lexical_cast<std::string>(i) + "][" + boost::lexical_cast<std::string>(j) + "]=";
                        tmp_msg += boost::lexical_cast<std::string>((uint32)sib7->carrier_freqs_info_list[i].carrier_freqs.explicit_list_of_arfcns[j]) + " ";
                    }
                }else if(LIBLTE_RRC_FOLLOWING_ARFCNS_EQUALLY_SPACED == sib7->carrier_freqs_info_list[i].carrier_freqs.following_arfcns){
                    tmp_msg += "geran_neigh_arfcn_spacing[" + boost::lexical_cast<std::string>(i) + "]=";
                    tmp_msg += boost::lexical_cast<std::string>((uint32)sib7->carrier_freqs_info_list[i].carrier_freqs.equally_spaced_arfcns.arfcn_spacing) + " ";
                    tmp_msg += "geran_neigh_num_arfcns[" + boost::lexical_cast<std::string>(i) + "]=";
                    tmp_msg += boost::lexical_cast<std::string>((uint32)sib7->carrier_freqs_info_list[i].carrier_freqs.equally_spaced_arfcns.number_of_arfcns) + " ";
                }else{
                    tmp_msg += "geran_neigh_arfcn_bitmap[" + boost::lexical_cast<std::string>(i) + "]=";
                    tmp_msg += boost::lexical_cast<std::string>((uint32)sib7->carrier_freqs_info_list[i].carrier_freqs.variable_bit_map_of_arfcns);
                }
                if(true == sib7->carrier_freqs_info_list[i].cell_resel_prio_present)
                {
                    tmp_msg += "cell_resel_prio[" + boost::lexical_cast<std::string>(i) + "]=";
                    tmp_msg += boost::lexical_cast<std::string>((uint32)sib7->carrier_freqs_info_list[i].cell_resel_prio) + " ";
                }
                tmp_msg += "ncc_permitted[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>((uint32)sib7->carrier_freqs_info_list[i].ncc_permitted);
                tmp_msg += "q_rx_lev_min[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>(sib7->carrier_freqs_info_list[i].q_rx_lev_min) + " ";
                if(true == sib7->carrier_freqs_info_list[i].p_max_geran_present)
                {
                    tmp_msg += "p_max_geran[" + boost::lexical_cast<std::string>(i) + "]=";
                    tmp_msg += boost::lexical_cast<std::string>((uint32)sib7->carrier_freqs_info_list[i].p_max_geran) + " ";
                }
                tmp_msg += "thresh_x_high[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>((uint32)sib7->carrier_freqs_info_list[i].threshx_high) + " ";
                tmp_msg += "thresh_x_low[" + boost::lexical_cast<std::string>(i) + "]=";
                tmp_msg += boost::lexical_cast<std::string>((uint32)sib7->carrier_freqs_info_list[i].threshx_low) + " ";
            }

            tmp_msg += "\n";
        }catch(boost::bad_lexical_cast &){
            tmp_msg += "\n";
        }
        ctrl_socket->send(tmp_msg);
    }
}
void LTE_fdd_dl_scan_interface::send_ctrl_sib8_decoded_msg(LTE_FDD_DL_SCAN_CHAN_DATA_STRUCT        *chan_data,
                                                           LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_8_STRUCT *sib8,
                                                           uint32                                   sfn)
{
    boost::mutex::scoped_lock lock(connect_mutex);
    std::string               tmp_msg;
    uint32                    i;
    uint32                    j;
    uint32                    k;

    if(ctrl_connected)
    {
        tmp_msg = "info sib8_decoded ";
        try
        {
            tmp_msg += "freq=" + boost::lexical_cast<std::string>(liblte_interface_dl_earfcn_to_frequency(current_dl_earfcn)) + " ";
            tmp_msg += "dl_earfcn=" + boost::lexical_cast<std::string>(current_dl_earfcn) + " ";
            tmp_msg += "freq_offset=" + boost::lexical_cast<std::string>(chan_data->freq_offset) + " ";
            tmp_msg += "phys_cell_id=" + boost::lexical_cast<std::string>(chan_data->N_id_cell) + " ";
            tmp_msg += "sfn=" + boost::lexical_cast<std::string>(sfn) + " ";

            if(true == sib8->sys_time_info_present)
            {
                if(true == sib8->sys_time_info_cdma2000.cdma_eutra_sync)
                {
                    tmp_msg += "cdma_eutra_sync=true ";
                }else{
                    tmp_msg += "cdma_eutra_sync=false ";
                }
                if(true == sib8->sys_time_info_cdma2000.system_time_async)
                {
                    tmp_msg += "system_time=" + boost::lexical_cast<std::string>(sib8->sys_time_info_cdma2000.system_time * 8) + " ";
                }else{
                    tmp_msg += "system_time=" + boost::lexical_cast<std::string>(sib8->sys_time_info_cdma2000.system_time * 10) + " ";
                }
            }
            if(true == sib8->search_win_size_present)
            {
                tmp_msg += "search_win_size=" + boost::lexical_cast<std::string>((uint32)sib8->search_win_size) + " ";
            }
            if(true == sib8->params_hrpd_present)
            {
                if(true == sib8->pre_reg_info_hrpd.pre_reg_allowed)
                {
                    tmp_msg += "pre_registration=allowed ";
                }else{
                    tmp_msg += "pre_registration=not_allowed ";
                }
                if(true == sib8->pre_reg_info_hrpd.pre_reg_zone_id_present)
                {
                    tmp_msg += "pre_reg_zone_id=" + boost::lexical_cast<std::string>((uint32)sib8->pre_reg_info_hrpd.pre_reg_zone_id) + " ";
                }
                for(i=0; i<sib8->pre_reg_info_hrpd.secondary_pre_reg_zone_id_list_size; i++)
                {
                    tmp_msg += "secondary_pre_reg_zone_id[" + boost::lexical_cast<std::string>(i) + "]=";
                    tmp_msg += boost::lexical_cast<std::string>((uint32)sib8->pre_reg_info_hrpd.secondary_pre_reg_zone_id_list[i]) + " ";
                }
                if(true == sib8->cell_resel_params_hrpd_present)
                {
                    for(i=0; i<sib8->cell_resel_params_hrpd.band_class_list_size; i++)
                    {
                        tmp_msg += "band_class[" + boost::lexical_cast<std::string>(i) + "]=";
                        tmp_msg += boost::lexical_cast<std::string>(liblte_rrc_band_class_cdma2000_text[sib8->cell_resel_params_hrpd.band_class_list[i].band_class]) + " ";
                        if(true == sib8->cell_resel_params_hrpd.band_class_list[i].cell_resel_prio_present)
                        {
                            tmp_msg += "cell_resel_prio[" + boost::lexical_cast<std::string>(i) + "]=";
                            tmp_msg += boost::lexical_cast<std::string>((uint32)sib8->cell_resel_params_hrpd.band_class_list[i].cell_resel_prio) + " ";
                        }
                        tmp_msg += "thresh_x_high[" + boost::lexical_cast<std::string>(i) + "]=";
                        tmp_msg += boost::lexical_cast<std::string>((uint32)sib8->cell_resel_params_hrpd.band_class_list[i].thresh_x_high) + " ";
                        tmp_msg += "thresh_x_low[" + boost::lexical_cast<std::string>(i) + "]=";
                        tmp_msg += boost::lexical_cast<std::string>((uint32)sib8->cell_resel_params_hrpd.band_class_list[i].thresh_x_low) + " ";
                    }
                    for(i=0; i<sib8->cell_resel_params_hrpd.neigh_cell_list_size; i++)
                    {
                        tmp_msg += "neigh_band_class[" + boost::lexical_cast<std::string>(i) + "]=";
                        tmp_msg += boost::lexical_cast<std::string>(liblte_rrc_band_class_cdma2000_text[sib8->cell_resel_params_hrpd.neigh_cell_list[i].band_class]) + " ";
                        for(j=0; j<sib8->cell_resel_params_hrpd.neigh_cell_list[i].neigh_cells_per_freq_list_size; j++)
                        {
                            tmp_msg += "neigh_cell_arfcn[" + boost::lexical_cast<std::string>(i) + "][" + boost::lexical_cast<std::string>(j) + "]=";
                            tmp_msg += boost::lexical_cast<std::string>((uint32)sib8->cell_resel_params_hrpd.neigh_cell_list[i].neigh_cells_per_freq_list[j].arfcn) + " ";
                            for(k=0; k<sib8->cell_resel_params_hrpd.neigh_cell_list[i].neigh_cells_per_freq_list[j].phys_cell_id_list_size; k++)
                            {
                                tmp_msg += "neigh_phys_cell_id[" + boost::lexical_cast<std::string>(i) + "][" + boost::lexical_cast<std::string>(j) + "][" + boost::lexical_cast<std::string>(k);
                                tmp_msg += "]=" + boost::lexical_cast<std::string>((uint32)sib8->cell_resel_params_hrpd.neigh_cell_list[i].neigh_cells_per_freq_list[j].phys_cell_id_list[k]) + " ";
                            }
                        }
                    }
                    tmp_msg += "t_resel=" + boost::lexical_cast<std::string>((uint32)sib8->cell_resel_params_hrpd.t_resel_cdma2000) + " ";
                    if(true == sib8->cell_resel_params_hrpd.t_resel_cdma2000_sf_present)
                    {
                        tmp_msg += "t_resel_sf_medium=" + boost::lexical_cast<std::string>(liblte_rrc_sssf_medium_text[sib8->cell_resel_params_hrpd.t_resel_cdma2000_sf.sf_medium]) + " ";
                        tmp_msg += "t_resel_sf_high=" + boost::lexical_cast<std::string>(liblte_rrc_sssf_high_text[sib8->cell_resel_params_hrpd.t_resel_cdma2000_sf.sf_high]) + " ";
                    }
                }
            }
            if(true == sib8->params_1xrtt_present)
            {
                if(true == sib8->csfb_reg_param_1xrtt_present)
                {
                    tmp_msg += "csfb_sid=" + boost::lexical_cast<std::string>((uint32)sib8->csfb_reg_param_1xrtt.sid) + " ";
                    tmp_msg += "csfb_nid=" + boost::lexical_cast<std::string>((uint32)sib8->csfb_reg_param_1xrtt.nid) + " ";
                    if(true == sib8->csfb_reg_param_1xrtt.multiple_sid)
                    {
                        tmp_msg += "csfb_multiple_sids=true ";
                    }else{
                        tmp_msg += "csfb_multiple_sids=false ";
                    }
                    if(true == sib8->csfb_reg_param_1xrtt.multiple_nid)
                    {
                        tmp_msg += "csfb_multiple_nids=true ";
                    }else{
                        tmp_msg += "csfb_multiple_nids=false ";
                    }
                    if(true == sib8->csfb_reg_param_1xrtt.home_reg)
                    {
                        tmp_msg += "csfb_home_reg=true ";
                    }else{
                        tmp_msg += "csfb_home_reg=false ";
                    }
                    if(true == sib8->csfb_reg_param_1xrtt.foreign_sid_reg)
                    {
                        tmp_msg += "csfb_foreign_sid_reg=true ";
                    }else{
                        tmp_msg += "csfb_foreign_sid_reg=false ";
                    }
                    if(true == sib8->csfb_reg_param_1xrtt.foreign_nid_reg)
                    {
                        tmp_msg += "csfb_foreign_nid_reg=true ";
                    }else{
                        tmp_msg += "csfb_foreign_nid_reg=false ";
                    }
                    if(true == sib8->csfb_reg_param_1xrtt.param_reg)
                    {
                        tmp_msg += "csfb_param_reg=true ";
                    }else{
                        tmp_msg += "csfb_param_reg=false ";
                    }
                    if(true == sib8->csfb_reg_param_1xrtt.power_up_reg)
                    {
                        tmp_msg += "csfb_power_up_reg=true ";
                    }else{
                        tmp_msg += "csfb_power_up_reg=false ";
                    }
                    tmp_msg += "csfb_reg_period=" + boost::lexical_cast<std::string>((uint32)sib8->csfb_reg_param_1xrtt.reg_period) + " ";
                    tmp_msg += "csfb_reg_zone=" + boost::lexical_cast<std::string>((uint32)sib8->csfb_reg_param_1xrtt.reg_zone) + " ";
                    tmp_msg += "csfb_total_zones=" + boost::lexical_cast<std::string>((uint32)sib8->csfb_reg_param_1xrtt.total_zone) + " ";
                    tmp_msg += "csfb_zone_timer=" + boost::lexical_cast<std::string>((uint32)sib8->csfb_reg_param_1xrtt.zone_timer) + " ";
                }
                if(true == sib8->long_code_state_1xrtt_present)
                {
                    tmp_msg += "long_code_state=" + boost::lexical_cast<std::string>(sib8->long_code_state_1xrtt) + " ";
                }
                if(true == sib8->cell_resel_params_1xrtt_present)
                {
                    for(i=0; i<sib8->cell_resel_params_1xrtt.band_class_list_size; i++)
                    {
                        tmp_msg += "band_class[" + boost::lexical_cast<std::string>(i) + "]=";
                        tmp_msg += boost::lexical_cast<std::string>(liblte_rrc_band_class_cdma2000_text[sib8->cell_resel_params_1xrtt.band_class_list[i].band_class]);
                        if(true == sib8->cell_resel_params_1xrtt.band_class_list[i].cell_resel_prio_present)
                        {
                            tmp_msg += "cell_resel_prio[" + boost::lexical_cast<std::string>(i) + "]=";
                            tmp_msg += boost::lexical_cast<std::string>((uint32)sib8->cell_resel_params_1xrtt.band_class_list[i].cell_resel_prio) + " ";
                        }
                        tmp_msg += "thresh_x_high[" + boost::lexical_cast<std::string>(i) + "]=";
                        tmp_msg += boost::lexical_cast<std::string>((uint32)sib8->cell_resel_params_1xrtt.band_class_list[i].thresh_x_high) + " ";
                        tmp_msg += "thresh_x_low[" + boost::lexical_cast<std::string>(i) + "]=";
                        tmp_msg += boost::lexical_cast<std::string>((uint32)sib8->cell_resel_params_1xrtt.band_class_list[i].thresh_x_low) + " ";
                    }
                    for(i=0; i<sib8->cell_resel_params_1xrtt.neigh_cell_list_size; i++)
                    {
                        tmp_msg += "neigh_band_class[" + boost::lexical_cast<std::string>(i) + "]=";
                        tmp_msg += boost::lexical_cast<std::string>(liblte_rrc_band_class_cdma2000_text[sib8->cell_resel_params_1xrtt.neigh_cell_list[i].band_class]) + " ";
                        for(j=0; j<sib8->cell_resel_params_1xrtt.neigh_cell_list[i].neigh_cells_per_freq_list_size; j++)
                        {
                            tmp_msg += "neigh_arfcn[" + boost::lexical_cast<std::string>(i) + "][" + boost::lexical_cast<std::string>(j) + "]=";
                            tmp_msg += boost::lexical_cast<std::string>((uint32)sib8->cell_resel_params_1xrtt.neigh_cell_list[i].neigh_cells_per_freq_list[j].arfcn) + " ";
                            for(k=0; k<sib8->cell_resel_params_1xrtt.neigh_cell_list[i].neigh_cells_per_freq_list[j].phys_cell_id_list_size; k++)
                            {
                                tmp_msg += "neigh_phys_cell_id[" + boost::lexical_cast<std::string>(i) + "][" + boost::lexical_cast<std::string>(j) + "][" + boost::lexical_cast<std::string>(k);
                                tmp_msg += "]=" + boost::lexical_cast<std::string>((uint32)sib8->cell_resel_params_1xrtt.neigh_cell_list[i].neigh_cells_per_freq_list[j].phys_cell_id_list[k]) + " ";
                            }
                        }
                    }
                    tmp_msg += "t_resel=" + boost::lexical_cast<std::string>((uint32)sib8->cell_resel_params_1xrtt.t_resel_cdma2000) + " ";
                    if(true == sib8->cell_resel_params_1xrtt.t_resel_cdma2000_sf_present)
                    {
                        tmp_msg += "t_resel_sf_medium=" + boost::lexical_cast<std::string>(liblte_rrc_sssf_medium_text[sib8->cell_resel_params_1xrtt.t_resel_cdma2000_sf.sf_medium]);
                        tmp_msg += "t_resel_sf_high=" + boost::lexical_cast<std::string>(liblte_rrc_sssf_high_text[sib8->cell_resel_params_1xrtt.t_resel_cdma2000_sf.sf_high]);
                    }
                }
            }

            tmp_msg += "\n";
        }catch(boost::bad_lexical_cast &){
            tmp_msg += "\n";
        }
        ctrl_socket->send(tmp_msg);
    }
}
void LTE_fdd_dl_scan_interface::send_ctrl_channel_found_end_msg(LTE_FDD_DL_SCAN_CHAN_DATA_STRUCT *chan_data)
{
    boost::mutex::scoped_lock lock(connect_mutex);
    std::string               tmp_msg;

    if(ctrl_connected)
    {
        tmp_msg = "info channel_found_end ";
        try
        {
            tmp_msg += "freq=" + boost::lexical_cast<std::string>(liblte_interface_dl_earfcn_to_frequency(current_dl_earfcn)) + " ";
            tmp_msg += "dl_earfcn=" + boost::lexical_cast<std::string>(current_dl_earfcn) + " ";
            tmp_msg += "freq_offset=" + boost::lexical_cast<std::string>(chan_data->freq_offset) + " ";
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
void LTE_fdd_dl_scan_interface::open_pcap_fd(void)
{
    uint32 magic_number  = 0xa1b2c3d4;
    uint32 timezone      = 0;
    uint32 sigfigs       = 0;
    uint32 snap_len      = (LIBLTE_MAX_MSG_SIZE/4);
    uint32 dlt           = 147;
    uint16 major_version = 2;
    uint16 minor_version = 4;

    pcap_fd = fopen("/tmp/LTE_fdd_dl_scan.pcap", "w");

    fwrite(&magic_number,  sizeof(magic_number),  1, pcap_fd);
    fwrite(&major_version, sizeof(major_version), 1, pcap_fd);
    fwrite(&minor_version, sizeof(minor_version), 1, pcap_fd);
    fwrite(&timezone,      sizeof(timezone),      1, pcap_fd);
    fwrite(&sigfigs,       sizeof(sigfigs),       1, pcap_fd);
    fwrite(&snap_len,      sizeof(snap_len),      1, pcap_fd);
    fwrite(&dlt,           sizeof(dlt),           1, pcap_fd);
}
void LTE_fdd_dl_scan_interface::send_pcap_msg(uint32                 rnti,
                                              uint32                 current_tti,
                                              LIBLTE_BIT_MSG_STRUCT *msg)
{
    struct timeval  time;
    struct timezone time_zone;
    uint32          i;
    uint32          idx;
    uint32          length;
    uint16          tmp;
    uint8           pcap_c_hdr[15];
    uint8           pcap_msg[LIBLTE_MAX_MSG_SIZE/8];

    if(enable_pcap)
    {
        // Get approximate time stamp
        gettimeofday(&time, &time_zone);

        // Radio Type
        pcap_c_hdr[0] = 1;

        // Direction
        pcap_c_hdr[1] = 1; // DL only for now

        // RNTI Type
        if(0xFFFFFFFF == rnti)
        {
            pcap_c_hdr[2] = 0;
        }else{
            pcap_c_hdr[2] = 4;
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
        for(i=0; i<msg->N_bits; i++)
        {
            pcap_msg[idx] <<= 1;
            pcap_msg[idx]  |= msg->msg[i];
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
    }else if(std::string::npos != msg.find(ENABLE_PCAP_PARAM)){
        read_enable_pcap();
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
    }else if(std::string::npos != msg.find(ENABLE_PCAP_PARAM)){
        write_enable_pcap(msg.substr(msg.find(ENABLE_PCAP_PARAM)+sizeof(ENABLE_PCAP_PARAM), std::string::npos).c_str());
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
void LTE_fdd_dl_scan_interface::read_enable_pcap(void)
{
    if(true == enable_pcap)
    {
        send_ctrl_status_msg(LTE_FDD_DL_SCAN_STATUS_OK, "on");
    }else{
        send_ctrl_status_msg(LTE_FDD_DL_SCAN_STATUS_OK, "off");
    }
}
void LTE_fdd_dl_scan_interface::write_enable_pcap(std::string enable_pcap_str)
{
    if(enable_pcap_str == "on")
    {
        enable_pcap = true;
        send_ctrl_status_msg(LTE_FDD_DL_SCAN_STATUS_OK, "");
    }else if(enable_pcap_str == "off"){
        enable_pcap = false;
        send_ctrl_status_msg(LTE_FDD_DL_SCAN_STATUS_OK, "");
    }else{
        send_ctrl_status_msg(LTE_FDD_DL_SCAN_STATUS_FAIL, "Invalid enable_pcap");
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
