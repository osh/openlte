%
% Copyright 2011 Ben Wojtowicz
%
%    This program is free software: you can redistribute it and/or modify
%    it under the terms of the GNU General Public License as published by
%    the Free Software Foundation, either version 3 of the License, or
%    (at your option) any later version.
%
%    This program is distributed in the hope that it will be useful,
%    but WITHOUT ANY WARRANTY; without even the implied warranty of
%    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%    GNU General Public License for more details.
%
%    You should have received a copy of the GNU General Public License
%    along with this program.  If not, see <http://www.gnu.org/licenses/>.
%
% Function:    lte_fdd_dl_receive
% Description: Synchronizes to an FDD LTE signal and
%              decodes control information
% Inputs:      input_samps - LTE signal sampled at 30.72MHz
%                            of instantaneous BW.
% Outputs:     None
% Spec:        N/A
% Notes:       Only supports normal cyclic prefix
% Rev History: Ben Wojtowicz 12/26/2011 Created
%
function [] = lte_fdd_dl_receive(input_samps)
    % DEFINES
    N_sc_rb           = 12; % Only dealing with normal cp at this time
    N_symb_dl         = 7;  % Only dealing with normal cp at this time
    N_slots_per_frame = 20;
    N_cp_l_0          = 160;
    N_cp_l_else       = 144;
    N_rb_dl_max       = 110;

    % Assume bandwidth of 1.4MHz until MIB is decoded
    N_rb_dl      = 6;
    FFT_pad_size = 988; % FFT_size = 2048

    % Check input length
    if(length(input_samps) < 307200)
        printf("ERROR: Not enough samples\n");
        return
    endif

    % Find symbol start locations
    [start_loc_vec, freq_offset] = find_coarse_time_and_freq_offset(input_samps, N_cp_l_else);

    % Remove frequency error
    freq_offset_vec = cos((1:length(input_samps))*(-freq_offset)*2*pi*(0.0005/15360)) + j*sin((1:length(input_samps))*(-freq_offset)*2*pi*(0.0005/15360));
    input_samps     = input_samps ./ freq_offset_vec;

    % Search for PSS and find fine timing offset
    pss_symb = 0;
    [start_loc_vec, N_id_2, pss_symb, pss_thresh] = find_pss_and_fine_timing(input_samps, start_loc_vec);
    if(pss_symb ~= 0)
        printf("Found PSS-%u in %u (%f)\n", N_id_2, pss_symb, pss_thresh);
    else
        printf("ERROR: Didn't find PSS\n");
        return;
    endif

    % Find SSS
    f_start_idx = 0;
    pss_thresh;
    [N_id_1, f_start_idx] = find_sss(input_samps, N_id_2, start_loc_vec, pss_thresh);
    if(f_start_idx ~= 0)
        printf("Found SSS-%u, 0 index is %u, cell_id is %u\n", N_id_1, f_start_idx, 3*N_id_1 + N_id_2);
    else
        printf("ERROR: Didn't find SSS\n");
        return;
    endif

    % Redefine input and start vector to the 0 index
    input_samps   = input_samps(f_start_idx:end);
    f_start_idx   = 0;
    start_loc_vec = [1, 2209, 4401, 6593, 8785, 10977, 13169];

    % Construct N_id_cell and modulos
    N_id_cell       = 3*N_id_1 + N_id_2;
    N_id_cell_mod_3 = mod(N_id_cell, 3);
    N_id_cell_mod_6 = mod(N_id_cell, 6);

    % Decode MIB, trying 1, 2, and 4 antennas
    chan_est  = get_chan_est_for_slot(input_samps, f_start_idx, 1, FFT_pad_size, N_rb_dl, N_id_cell, 4);
    symb(1,:) = samps_to_symbs(input_samps, f_start_idx,  7, FFT_pad_size, 0);
    symb(2,:) = samps_to_symbs(input_samps, f_start_idx,  8, FFT_pad_size, 0);
    symb(3,:) = samps_to_symbs(input_samps, f_start_idx,  9, FFT_pad_size, 0);
    symb(4,:) = samps_to_symbs(input_samps, f_start_idx, 10, FFT_pad_size, 0);
    [N_ant, bw, phich_dur, phich_res, sfn] = decode_mib(chan_est, symb, N_id_cell);
    if(N_ant != 0)
        printf("BCH found, N_ant = %u, N_rb_dl = %u, phich_duration = %s, phich_resource = %u, sfn = %u\n", N_ant, bw, phich_dur, phich_res, sfn);
    else
        printf("ERROR: BCH NOT FOUND\n");
        return;
    endif

    % Redefine N_rb_dl and FFT_pad_size with the actual bandwidth
    N_rb_dl = bw;
    if(N_rb_dl == 6)
        FFT_pad_size = 988; % FFT_size = 2048
    elseif(N_rb_dl == 15)
        FFT_pad_size = 934; % FFT_size = 2048
    elseif(N_rb_dl == 25)
        FFT_pad_size = 874; % FFT_size = 2048
    elseif(N_rb_dl == 50)
        FFT_pad_size = 724; % FFT_size = 2048
    elseif(N_rb_dl == 75)
        FFT_pad_size = 574; % FFT_size = 2048
    else % N_rb_dl == 100
        FFT_pad_size = 424; % FFT_size = 2048
    endif

    % Decode CFI
    chan_est = get_chan_est_for_slot(input_samps, f_start_idx, 0, FFT_pad_size, N_rb_dl, N_id_cell, N_ant);
    symb     = samps_to_symbs(input_samps, f_start_idx, 0, FFT_pad_size, 0);
    [cfi]    = decode_cfi(chan_est, symb, 0, N_ant, N_id_cell, N_sc_rb, N_rb_dl);
    if(cfi != 0)
        printf("CFI found %u\n", cfi);
    else
        printf("ERROR: CFI NOT FOUND\n");
    endif
endfunction

function [coarse_start, freq_offset] = find_coarse_time_and_freq_offset(in, N_cp_l_else)

    % Decompose input
    in_re = real(in);
    in_im = imag(in);

    % Can only rely on symbols 0 and 4 because of CRS

    % Rough correlation
    abs_corr = zeros(1,15360);
    for(slot=0:10)
        for(n=1:40:15360)
            corr_re = 0;
            corr_im = 0;
            for(z=1:N_cp_l_else)
                index   = (slot*15360) + n-1 + z;
                corr_re = corr_re + in_re(index)*in_re(index+2048) + in_im(index)*in_im(index+2048);
                corr_im = corr_im + in_re(index)*in_im(index+2048) - in_im(index)*in_re(index+2048);
            endfor
            abs_corr(n) = abs_corr(n) + corr_re*corr_re + corr_im*corr_im;
        endfor
    endfor

    % Find first and second max
    abs_corr_idx = zeros(1,2);
    for(m=0:1)
        abs_corr_max = 0;
        for(n=1:7680)
            if(abs_corr((m*7680)+n) > abs_corr_max)
                abs_corr_max      = abs_corr((m*7680)+n);
                abs_corr_idx(m+1) = (m*7680)+n;
            endif
        endfor
    endfor

    % Fine correlation and fraction frequency offset
    abs_corr      = zeros(1,15360);
    corr_freq_err = zeros(1,15360);
    for(slot=1:10)
        for(idx=1:2)
            if((abs_corr_idx(idx) - 40) < 1)
                abs_corr_idx(idx) = 41;
            endif
            for(n=abs_corr_idx(idx)-40:abs_corr_idx(idx)+40)
                corr_re = 0;
                corr_im = 0;
                for(z=1:N_cp_l_else)
                    index = (slot*15360) + n-1 + z;
                    corr_re = corr_re + in_re(index)*in_re(index+2048) + in_im(index)*in_im(index+2048);
                    corr_im = corr_im + in_re(index)*in_im(index+2048) - in_im(index)*in_re(index+2048);
                endfor
                abs_corr(n)      = abs_corr(n) + corr_re*corr_re + corr_im*corr_im;
                corr_freq_err(n) = corr_freq_err(n) + atan2(corr_im, corr_re)/(2048*2*pi*(0.0005/15360));
            endfor
        endfor
    endfor

    % Find first and second max
    abs_corr_idx = zeros(1,2);
    for(m=0:1)
        abs_corr_max = 0;
        for(n=1:7680)
            if(abs_corr((m*7680)+n) > abs_corr_max)
                abs_corr_max      = abs_corr((m*7680)+n);
                abs_corr_idx(m+1) = (m*7680)+n;
            endif
        endfor
    endfor

    % Determine frequency offset FIXME No integer offset is calculated here
    freq_offset = (corr_freq_err(abs_corr_idx(1))/10 + corr_freq_err(abs_corr_idx(2))/10)/2;

    % Determine the symbol start locations from the correlation peaks
    % FIXME Needs some work
    tmp = abs_corr_idx(1);
    while(tmp > 0)
        tmp = tmp - 2192;
    endwhile
    for(n=1:7)
        coarse_start(n) = tmp + (n*2192);
    endfor
endfunction

function [fine_start, N_id_2, pss_symb, pss_thresh] = find_pss_and_fine_timing(input, coarse_start)
    % DEFINES Assuming 20MHz
    N_rb_dl      = 100;
    N_sc_rb      = 12; % Only dealing with normal cp at this time
    N_symb_dl    = 7;  % Only dealing with normal cp at this time
    N_cp_l_else  = 144;
    FFT_pad_size = 424;

    % Generate primary synchronization signals
    pss_mod_vec = zeros(3, N_rb_dl*N_sc_rb);
    for(loc_N_id_2=0:2)
        pss = lte_generate_pss(loc_N_id_2);
        for(n=0:61)
            k = n - 31 + (N_rb_dl*N_sc_rb)/2;
            pss_mod_vec(loc_N_id_2+1, k+1) = pss(n+1);
        endfor
    endfor

    % Demod symbols and correlate with primary synchronization signals
    num_slots_to_demod = floor(200000/15360)-1;
    for(n=0:num_slots_to_demod-1)
        for(m=1:N_symb_dl)
            symb = conj(samps_to_symbs(input, coarse_start(m), 7*n, FFT_pad_size, 0)); % FIXME Not sure why conj is needed

            for(loc_N_id_2=0:2)
                pss_corr = 0;
                for(z=1:N_rb_dl*N_sc_rb)
                    pss_corr = pss_corr + symb(z)*pss_mod_vec(loc_N_id_2+1, z);
                endfor
                pss_corr_mat((n*N_symb_dl)+m, loc_N_id_2+1) = abs(pss_corr);
            endfor
        endfor
    endfor

    % Find maximum
    [val, abs_slot_num]  = max(pss_corr_mat);
    [val, N_id_2_plus_1] = max(val);
    pss_symb             = abs_slot_num(N_id_2_plus_1);

    % Find optimal timing
    N_s    = floor((abs_slot_num(N_id_2_plus_1)-1)/7);
    N_symb = mod(abs_slot_num(N_id_2_plus_1)-1, 7)+1;
    N_id_2 = N_id_2_plus_1 - 1;
    for(timing=-40:40)
        symb = conj(samps_to_symbs(input, coarse_start(N_symb)+timing, 7*N_s, FFT_pad_size, 0)); % FIXME Not sure why conj is needed

        pss_corr = 0;
        for(z=1:N_rb_dl*N_sc_rb)
            pss_corr = pss_corr + symb(z)*pss_mod_vec(N_id_2+1, z);
        endfor
        timing_vec(timing+41) = abs(pss_corr);
    endfor
    [pss_thresh, timing_plus_41] = max(timing_vec);

    % Construct fine symbol start locations
    pss_timing_idx = coarse_start(N_symb)+(15360*N_s)+timing_plus_41-41;
    fine_start(1)  = pss_timing_idx + (2048+144)*1 - 15360;
    fine_start(2)  = pss_timing_idx + (2048+144)*1 + 2048+160 - 15360;
    fine_start(3)  = pss_timing_idx + (2048+144)*2 + 2048+160 - 15360;
    fine_start(4)  = pss_timing_idx + (2048+144)*3 + 2048+160 - 15360;
    fine_start(5)  = pss_timing_idx + (2048+144)*4 + 2048+160 - 15360;
    fine_start(6)  = pss_timing_idx + (2048+144)*5 + 2048+160 - 15360;
    fine_start(7)  = pss_timing_idx + (2048+144)*6 + 2048+160 - 15360;
    while(fine_start(1) < 1)
        fine_start = fine_start + 15360;
    endwhile
endfunction

function [N_id_1, f_start_idx] = find_sss(input, N_id_2, start, pss_thresh)
    % DEFINES Assuming 20MHz
    N_rb_dl      = 100;
    N_sc_rb      = 12; % Only dealing with normal cp at this time
    N_symb_dl    = 7;  % Only dealing with normal cp at this time
    N_cp_l_0     = 160;
    N_cp_l_else  = 144;
    FFT_pad_size = 424;

    % Generate secondary synchronization signals
    sss_mod_vec_0 = zeros(167, N_rb_dl*N_sc_rb);
    sss_mod_vec_5 = zeros(167, N_rb_dl*N_sc_rb);
    for(loc_N_id_1=0:167)
        [sss_0, sss_5] = lte_generate_sss(loc_N_id_1, N_id_2);
        for(m=0:61)
            k = m - 31 + (N_rb_dl*N_sc_rb)/2;
            sss_mod_vec_0(loc_N_id_1+1, k+1) = sss_0(m+1);
            sss_mod_vec_5(loc_N_id_1+1, k+1) = sss_5(m+1);
        endfor
    endfor
    sss_thresh = pss_thresh * .9;

    % Demod symbol and search for secondary synchronization signals
    symb = samps_to_symbs(input, start(6), 0, FFT_pad_size, 0);
    for(loc_N_id_1=0:167)
        sss_corr = 0;
        for(m=1:(N_rb_dl*N_sc_rb))
            sss_corr = sss_corr + symb(m)*sss_mod_vec_0(loc_N_id_1+1, m);
        endfor
        if(abs(sss_corr) > sss_thresh)
            N_id_1      = loc_N_id_1;
            f_start_idx = start(6) - ((2048+N_cp_l_else)*4 + 2048+N_cp_l_0);
            break;
        endif

        sss_corr = 0;
        for(m=1:(N_rb_dl*N_sc_rb))
            sss_corr = sss_corr + symb(m)*sss_mod_vec_5(loc_N_id_1+1, m);
        endfor
        if(abs(sss_corr) > sss_thresh)
            N_id_1      = loc_N_id_1;
            f_start_idx = start(6) - ((2048+N_cp_l_else)*4 + 2048+N_cp_l_0);
            f_start_idx = f_start_idx - 15360*10;
            break;
        endif
    endfor
endfunction

function [chan_est] = get_chan_est_for_slot(input_samps, start_idx, N_s, FFT_pad_size, N_rb_dl, N_id_cell, N_ant)
    N_sc_rb        = 12; % Only dealing with normal cp at this time
    N_rb_dl_max    = 110;
    v_shift        = mod(N_id_cell, 6);
    slot_start_idx = start_idx + N_s*15360;
    crs0           = lte_generate_crs(mod(N_s, 20), 0, N_id_cell);
    crs1           = lte_generate_crs(mod(N_s, 20), 1, N_id_cell);
    crs4           = lte_generate_crs(mod(N_s, 20), 4, N_id_cell);
    symb0          = samps_to_symbs(input_samps, slot_start_idx, 0, FFT_pad_size, 0);
    symb1          = samps_to_symbs(input_samps, slot_start_idx, 1, FFT_pad_size, 0);
    symb4          = samps_to_symbs(input_samps, slot_start_idx, 4, FFT_pad_size, 0);
    symb7          = samps_to_symbs(input_samps, slot_start_idx, 7, FFT_pad_size, 0);
    symb8          = samps_to_symbs(input_samps, slot_start_idx, 8, FFT_pad_size, 0);

    for(p=1:N_ant)
        % Define v and symb
        if(p == 1)
            v       = 0;
            crs     = [crs0; crs4; crs0];
            symb    = [symb0; symb4; symb7];
            N_symbs = 3;
        elseif(p == 2)
            v       = 3;
            crs     = [crs0; crs4; crs0];
            symb    = [symb0; symb4; symb7];
            N_symbs = 3;
        elseif(p == 3)
            v       = 3;
            crs     = [crs1; crs1];
            symb    = [symb1; symb8];
            N_symbs = 2;
        else
            v       = 6;
            crs     = [crs1; crs1];
            symb    = [symb1; symb8];
            N_symbs = 2;
        endif

        for(n=1:N_symbs)
            for(m=0:2*N_rb_dl-1)
                k          = 6*m + mod((v + v_shift), 6);
                m_prime    = m + N_rb_dl_max - N_rb_dl;
                tmp        = symb(n,k+1)/crs(n,m_prime+1);
                mag(n,k+1) = abs(tmp);
                ang(n,k+1) = angle(tmp);

                % Unwrap phase
                if(m > 0)
                    while((ang(n,k+1) - ang(n,k-6+1)) > pi)
                        ang(n,k+1) = ang(n,k+1) - 2*pi;
                    endwhile
                    while((ang(n,k+1) - ang(n,k-6+1)) < -pi)
                        ang(n,k+1) = ang(n,k+1) + 2*pi;
                    endwhile
                endif

                % Interpolate between CRSs (simple linear interpolation)
                if(m > 0)
                    frac_mag = (mag(n,k+1) - mag(n,k-6+1))/6;
                    frac_ang = (ang(n,k+1) - ang(n,k-6+1))/6;
                    for(o=1:5)
                        mag(n,k-o+1) = mag(n,k-(o-1)+1) - frac_mag;
                        ang(n,k-o+1) = ang(n,k-(o-1)+1) - frac_ang;
                    endfor
                endif

                % Interpolate before 1st CRS
                if(m == 1)
                    for(o=1:mod(v + v_shift, 6))
                        mag(n,k-6-o+1) = mag(n,k-6-(o-1)+1) - frac_mag;
                        ang(n,k-6-o+1) = ang(n,k-6-(o-1)+1) - frac_ang;
                    endfor
                endif
            endfor

            % Interpolate after last CRS
            for(o=1:(5-mod(v + v_shift, 6)))
                mag(n,k+o+1) = mag(n,k+(o-1)+1) - frac_mag;
                ang(n,k+o+1) = ang(n,k+(o-1)+1) - frac_ang;
            endfor
        endfor

        % Interpolate between symbols and construct channel estimates
        if(N_symbs == 2)
            for(n=1:N_sc_rb*N_rb_dl)
                % Construct symbol 1 channel estimate directly
                chan_est(p,1+1,n) = mag(1,n)*(cos(ang(1,n)) + j*sin(ang(1,n)));

                % Interpolate for symbol 2, 3, 4, 5, and 6 channel estimates
                frac_mag = (mag(2,n) - mag(1,n))/7;
                frac_ang = (ang(2,n) - ang(1,n))/7;
                prev_mag = mag(2,n) - frac_mag;
                prev_ang = ang(2,n) - frac_ang;
                for(o=6:-1:2)
                    ce_mag            = prev_mag - frac_mag;
                    ce_ang            = prev_ang - frac_ang;
                    chan_est(p,o+1,n) = ce_mag*(cos(ce_ang) + j*sin(ce_ang));
                    prev_mag          = ce_mag;
                    prev_ang          = ce_ang;
                endfor

                % Interpolate for symbol 0 channel estimate
                % FIXME: Use previous slot to do this correctly
                ce_mag            = mag(1,n) - frac_mag;
                ce_ang            = ang(1,n) - frac_ang;
                chan_est(p,0+1,n) = ce_mag*(cos(ce_ang) + j*sin(ce_ang));
            endfor
        else
            for(n=1:N_sc_rb*N_rb_dl)
                % Construct symbol 0 and 4 channel estimates directly
                chan_est(p,0+1,n) = mag(1,n)*(cos(ang(1,n)) + j*sin(ang(1,n)));
                chan_est(p,4+1,n) = mag(2,n)*(cos(ang(2,n)) + j*sin(ang(2,n)));

                % Interpolate for symbol 1, 2, and 3 channel estimates
                frac_mag = (mag(2,n) - mag(1,n))/4;
                frac_ang = (ang(2,n) - ang(1,n))/4;
                prev_mag = mag(2,n);
                prev_ang = ang(2,n);
                for(o=3:-1:1)
                    ce_mag            = prev_mag - frac_mag;
                    ce_ang            = prev_ang - frac_ang;
                    chan_est(p,o+1,n) = ce_mag*(cos(ce_ang) + j*sin(ce_ang));
                    prev_mag          = ce_mag;
                    prev_ang          = ce_ang;
                endfor

                % Interpolate for symbols 5 and 6 channel estimates
                frac_mag = (mag(3,n) - mag(2,n))/3;
                frac_ang = (ang(3,n) - ang(2,n))/3;
                prev_mag = mag(3,n);
                prev_ang = ang(3,n);
                for(o=6:-1:5)
                    ce_mag            = prev_mag - frac_mag;
                    ce_ang            = prev_ang - frac_ang;
                    chan_est(p,o+1,n) = ce_mag*(cos(ce_ang) + j*sin(ce_ang));
                    prev_mag          = ce_mag;
                    prev_ang          = ce_ang;
                endfor
            endfor
        endif
    endfor
endfunction

function [N_ant, bw, phich_dur, phich_res, sfn] = decode_mib(ce_slot, symb, N_id_cell)
    % Unmap PBCH and channel estimate from resource elements
    N_id_cell_mod_3 = mod(N_id_cell, 3);
    idx             = 0;
    for(n=0:71)
        if(N_id_cell_mod_3 != mod(n,3))
            y_est(idx+1)    = symb(1,n+1);
            y_est(idx+48+1) = symb(2,n+1);
            for(m=0:3)
                ce(m+1,idx+1)    = ce_slot(m+1,1,n+1);
                ce(m+1,idx+48+1) = ce_slot(m+1,2,n+1);
            endfor
            idx = idx + 1;
        endif
        y_est(n+96+1)  = symb(3,n+1);
        y_est(n+168+1) = symb(4,n+1);
        for(m=0:3)
            ce(m+1,n+96+1)  = ce_slot(m+1,3,n+1);
            ce(m+1,n+168+1) = ce_slot(m+1,4,n+1);
        endfor
    endfor

    % Generate the scrambling sequence in NRZ
    c = 1 - 2*lte_generate_prs_c(N_id_cell, 1920);

    % Try decoding with 1, 2, and 4 antennas
    for(n=[1,2,4])
        x    = lte_pre_decoder_and_matched_filter(y_est, ce(1:n,:), "tx_diversity");
        d    = lte_layer_demapper(x, 1, "tx_diversity");
        bits = lte_modulation_demapper(d, "qpsk");

        % Try decoding at each mod 4 offset
        [mib, N_ant] = lte_bch_channel_decode([bits.*c(1:480), 10000*ones(1,1440)]);
        if(N_ant != 0)
            % Unpack the mib and convert sfn_mod_4 to sfn
            [bw, phich_dur, phich_res, sfn_mod_4] = lte_mib_unpack(mib);
            sfn                                   = sfn_mod_4;
            break;
        endif
        [mib, N_ant] = lte_bch_channel_decode([10000*ones(1,480), bits.*c(481:960), 10000*ones(1,960)]);
        if(N_ant != 0)
            % Unpack the mib and convert sfn_mod_4 to sfn
            [bw, phich_dur, phich_res, sfn_mod_4] = lte_mib_unpack(mib);
            sfn                                   = sfn_mod_4 + 1;
            break;
        endif
        [mib, N_ant] = lte_bch_channel_decode([10000*ones(1,960), bits.*c(961:1440), 10000*ones(1,480)]);
        if(N_ant != 0)
            % Unpack the mib and convert sfn_mod_4 to sfn
            [bw, phich_dur, phich_res, sfn_mod_4] = lte_mib_unpack(mib);
            sfn                                   = sfn_mod_4 + 2;
            break;
        endif
        [mib, N_ant] = lte_bch_channel_decode([10000*ones(1,1440), bits.*c(1441:end)]);
        if(N_ant != 0)
            % Unpack the mib and convert sfn_mod_4 to sfn
            [bw, phich_dur, phich_res, sfn_mod_4] = lte_mib_unpack(mib);
            sfn                                   = sfn_mod_4 + 3;
            break;
        endif
    endfor
endfunction

function [cfi] = decode_cfi(ce_slot, symb, N_s, N_ant, N_id_cell, N_sc_rb, N_rb_dl)
    % Determine resource element groups
    k_hat = (N_sc_rb/2)*mod(N_id_cell, 2*N_rb_dl);
    k_1   = k_hat;
    k_2   = mod(k_hat + floor(N_rb_dl/2)*N_sc_rb/2, N_rb_dl*N_sc_rb);
    k_3   = mod(k_hat + floor(2*N_rb_dl/2)*N_sc_rb/2, N_rb_dl*N_sc_rb);
    k_4   = mod(k_hat + floor(3*N_rb_dl/2)*N_sc_rb/2, N_rb_dl*N_sc_rb);

    % Unmap CFICH and channel estimate from resource elements
    N_id_cell_mod_3 = mod(N_id_cell, 3);
    idx             = 0;
    for(n=0:5)
        if(N_id_cell_mod_3 != mod(n,3))
            y_est(idx+1)    = symb(k_1+n+1);
            y_est(idx+4+1)  = symb(k_2+n+1);
            y_est(idx+8+1)  = symb(k_3+n+1);
            y_est(idx+12+1) = symb(k_4+n+1);
            for(m=0:N_ant-1)
                ce(m+1,idx+1)    = ce_slot(m+1,1,k_1+n+1);
                ce(m+1,idx+4+1)  = ce_slot(m+1,1,k_2+n+1);
                ce(m+1,idx+8+1)  = ce_slot(m+1,1,k_3+n+1);
                ce(m+1,idx+12+1) = ce_slot(m+1,1,k_4+n+1);
            endfor
            idx = idx + 1;
        endif
    endfor

    % Generate the scrambling sequence
    c_init = (floor(N_s/2) + 1)*(2*N_id_cell + 1)*2^9 + N_id_cell;
    c      = lte_generate_prs_c(c_init, 32);

    % Try decoding
    x    = lte_pre_decoder_and_matched_filter(y_est, ce, "tx_diversity");
    d    = lte_layer_demapper(x, 1, "tx_diversity");
    bits = (-1 + sign(lte_modulation_demapper(d, "qpsk")))/(-2);
    cfi  = lte_cfi_channel_decode(mod(bits+c, 2));
endfunction

function [symbs] = samps_to_symbs(samps, f_start_idx, symb_offset, FFT_pad_size, scale)
    % Calculate index and CP length
    if(mod(symb_offset, 7) == 0)
        CP_len = 160;
    else
        CP_len = 144;
    endif
    if(symb_offset > 0)
        num_160 = 1 + floor((symb_offset-1)/7);
    else
        num_160 = 0;
    endif
    index = f_start_idx + 2048*symb_offset;
    index = index + 160*num_160 + 144*(symb_offset-num_160);

    % Take FFT
    tmp = fftshift(fft(samps(index+CP_len:index+CP_len+2047)));

    % Remove DC subcarrier
    tmp_symbs = [tmp(FFT_pad_size+1:1024), tmp(1026:2048-(FFT_pad_size-1))];

    if(scale == 0)
        symbs = tmp_symbs;
    else
        for(n=1:length(tmp_symbs))
            symbs(n) = cos(angle(tmp_symbs(n))) + j*sin(angle(tmp_symbs(n)));
        endfor
    endif
endfunction
