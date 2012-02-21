%
% Copyright 2012 Ben Wojtowicz
%
%    This program is free software: you can redistribute it and/or modify
%    it under the terms of the GNU Affero General Public License as published by
%    the Free Software Foundation, either version 3 of the License, or
%    (at your option) any later version.
%
%    This program is distributed in the hope that it will be useful,
%    but WITHOUT ANY WARRANTY; without even the implied warranty of
%    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%    GNU Affero General Public License for more details.
%
%    You should have received a copy of the GNU Affero General Public License
%    along with this program.  If not, see <http://www.gnu.org/licenses/>.
%
% Function:    lte_sib1_unpack
% Description: Unpacks all of the fields from a System Information Block
%              Type 1 message.
% Inputs:      sib1_msg - Packed System Information Type 1 message
% Outputs:     sib_s    - Structure containing all of the SIB1 fields
%                         (plmn_ids, tac, cell_id, cell_barred, q_rx_lev_min,
%                         band, sched_info_list)
% Spec:        3GPP TS 36.331 section 6.2.2 v10.0.0
% Notes:       None
% Rev History: Ben Wojtowicz 02/11/2012 Created
%
function [sib1_s] = lte_sib1_unpack(sib1_msg)
    % Unpack the message
    idx = 0;

    % Optional field indicators
    p_max_opt        = sib1_msg(idx+1);
    idx              = idx + 1;
    tdd_config_opt   = sib1_msg(idx+1);
    idx              = idx + 1;
    non_crit_ext_opt = sib1_msg(idx+1);
    idx              = idx + 1;

    % Cell Access Related Info
    csg_id_opt           = sib1_msg(idx+1);
    idx                  = idx + 1;
    plmn_id_list.num_ids = cmn_bin2dec(sib1_msg(idx+1:idx+3), 3)+1;
    idx                  = idx + 3;
    for(n=0:plmn_id_list.num_ids-1)
        mcc_opt = sib1_msg(idx+1);
        idx     = idx + 1;
        if(mcc_opt == 1)
            id.mcc = 0;
            for(m=0:2)
                tmp    = cmn_bin2dec(sib1_msg(idx+1:idx+4), 4);
                id.mcc = id.mcc + tmp*10^(2-m);
                idx    = idx + 4;
            endfor
        else
            id.mcc = plmn_id_list.id(n).mcc;
        endif
        id.mnc_size = sib1_msg(idx+1)+2;
        idx         = idx + 1;
        id.mnc      = 0;
        for(m=0:id.mnc_size-1)
            tmp    = cmn_bin2dec(sib1_msg(idx+1:idx+4), 4);
            id.mnc = id.mnc + tmp*10^(id.mnc_size-1-m);
            idx    = idx + 4;
        endfor
        id.crou              = sib1_msg(idx+1);
        idx                  = idx + 1;
        plmn_id_list.id(n+1) = id;
    endfor
    tac              = cmn_bin2dec(sib1_msg(idx+1:idx+16), 16);
    idx              = idx + 16;
    cell_id          = cmn_bin2dec(sib1_msg(idx+1:idx+28), 28);
    idx              = idx + 28;
    cell_barred      = sib1_msg(idx+1);
    idx              = idx + 1;
    intra_freq_resel = sib1_msg(idx+1);
    idx              = idx + 1;
    csg_indication   = sib1_msg(idx+1);
    idx              = idx + 1;
    if(csg_id_opt == 1)
        csg_id = cmn_bin2dec(sib1_msg(idx+1:idx+27), 27);
        idx    = idx + 27;
    endif

    % Cell Selection Info
    q_rx_lev_min_offset_opt = sib1_msg(idx+1);
    idx                     = idx + 1;
    q_rx_lev_min            = cmn_bin2dec(sib1_msg(idx+1:idx+6), 6) - 70;
    idx                     = idx + 6;
    if(q_rx_lev_min_offset_opt == 1)
        q_rx_lev_min_offset = cmn_bin2dec(sib1_msg(idx+1:idx+3), 3) + 1;
        idx                 = idx + 3;
    endif

    % P_max
    if(p_max_opt == 1)
        p_max = cmn_bin2dec(sib1_msg(idx+1:idx+7), 7) - 30;
        idx   = idx + 7;
    endif

    % Frequency band indicator
    f_band_ind = cmn_bin2dec(sib1_msg(idx+1:idx+6), 6) + 1;
    idx        = idx + 6;

    % Scheduling Info List
    sched_info_list.size = cmn_bin2dec(sib1_msg(idx+1:idx+5), 5) + 1;
    idx                  = idx + 5;
    for(n=0:sched_info_list.size-1)
        si_periodicity = cmn_bin2dec(sib1_msg(idx+1:idx+3), 3);
        idx            = idx + 3;
        if(si_periodicity == 0)
            tmp.si_periodicity = 8;
        elseif(si_periodicity == 1)
            tmp.si_periodicity = 16;
        elseif(si_periodicity == 2)
            tmp.si_periodicity = 32;
        elseif(si_periodicity == 3)
            tmp.si_periodicity = 64;
        elseif(si_periodicity == 4)
            tmp.si_periodicity = 128;
        elseif(si_periodicity == 5)
            tmp.si_periodicity = 256;
        else
            tmp.si_periodicity = 512;
        endif
        tmp.sib_map_size   = cmn_bin2dec(sib1_msg(idx+1:idx+5), 5);
        idx                = idx + 5;
        for(m=0:tmp.sib_map_size-1)
            tmp.sib_map(m+1).sib_type_ext = sib1_msg(idx+1);
            idx                           = idx + 1;
            tmp.sib_map(m+1).sib_type     = cmn_bin2dec(sib1_msg(idx+1:idx+4), 4);
            idx                           = idx + 4;
        endfor
        sched_info_list.sched_info(n+1) = tmp;
    endfor

    % TDD Config
    if(tdd_config_opt == 1)
        sf_assign           = cmn_bin2dec(sib1_msg(idx+1:idx+3), 3);
        idx                 = idx + 3;
        special_sf_patterns = cmn_bin2dec(sib1_msg(idx+1,idx+4), 4);
        idx                 = idx + 4;
    endif

    % SI Window Length
    si_win_len = cmn_bin2dec(sib1_msg(idx+1:idx+3), 3);
    idx        = idx + 3;
    if(si_win_len == 0)
        si_window_len = 1;
    elseif(si_win_len == 1)
        si_window_len = 2;
    elseif(si_win_len == 2)
        si_window_len = 5;
    elseif(si_win_len == 3)
        si_window_len = 10;
    elseif(si_win_len == 4)
        si_window_len = 15;
    elseif(si_win_len == 5)
        si_window_len = 20;
    else
        si_window_len = 40;
    endif

    % System Info Value Tag
    si_value_tag = cmn_bin2dec(sib1_msg(idx+1:idx+5), 5);
    idx          = idx + 5;

    % Non Critical Extension
    if(non_crit_ext_opt == 1)
        non_crit_ext = sib1_msg(idx+1,end)
    endif

    % Fill the output structure
    sib1_s.plmn_ids        = plmn_id_list;
    sib1_s.tac             = tac;
    sib1_s.cell_id         = cell_id;
    sib1_s.cell_barred     = cell_barred;
    sib1_s.q_rx_lev_min    = q_rx_lev_min;
    sib1_s.band            = f_band_ind;
    sib1_s.sched_info_list = sched_info_list;
    sib1_s.si_window_len   = si_window_len;
endfunction
