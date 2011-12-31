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
% Function:    lte_fdd_dl_transmit
% Description: Creates an FDD LTE signal.
% Inputs:      bandwidth    - Desired bandwidth
%              N_frames     - Desired number of frames
%              N_id_2       - Physical layer identity
%              N_id_1       - Physical layer cell identity group 
%              N_ant        - Number of antenna ports
% Outputs:     output_samps - LTE signal sampled at 30.72MHz
%                             30.72MHz instantaneous BW.
% Spec:        N/A
% Notes:       Only supports normal cyclic prefix
% Rev History: Ben Wojtowicz 12/26/2011 Created
%
function [output_samps] = lte_fdd_dl_transmit(bandwidth, N_frames, N_id_2, N_id_1, N_ant)
    % DEFINES
    N_sc_rb           = 12; % Only dealing with normal cp at this time
    N_symb_dl         = 7;  % Only dealing with normal cp at this time
    N_slots_per_frame = 20;
    N_cp_l_0          = 160;
    N_cp_l_else       = 144;
    N_rb_dl_max       = 110;
    N_id_cell         = 3*N_id_1 + N_id_2;

    % Check bandwidth and set N_rb_dl: 36.104 section 5.6 v10.1.0
    if(bandwidth == 1.4)
        N_rb_dl      = 6;
        FFT_pad_size = 988; % FFT_size = 2048
    elseif(bandwidth == 3)
        N_rb_dl      = 15;
        FFT_pad_size = 934; % FFT_size = 2048
    elseif(bandwidth == 5)
        N_rb_dl      = 25;
        FFT_pad_size = 874; % FFT_size = 2048
    elseif(bandwidth == 10)
        N_rb_dl      = 50;
        FFT_pad_size = 724; % FFT_size = 2048
    elseif(bandwidth == 15)
        N_rb_dl      = 75;
        FFT_pad_size = 574; % FFT_size = 2048
    else
        N_rb_dl      = 100;
        FFT_pad_size = 424; % FFT_size = 2048
        if(bandwidth ~= 20)
            printf("WARNING: Invalid bandwidth (%u), using 20\n", bandwidth);
        endif
    endif

    % Check N_ant
    if(N_ant > 2)
        printf("ERROR: Only supporting 1 or 2 antennas at this time\n");
        return;
    endif

    % Generate syncronization signals: 36.211 section 6.11 v10.1.0
    pss_d_u                = lte_generate_pss(N_id_2);
    [sss_d_u_0, sss_d_u_5] = lte_generate_sss(N_id_1, N_id_2);

    % Generate reference signals: 36.211 section 6.10 v10.1.0
    for(n=0:N_slots_per_frame-1)
        crs_r_0(n+1,:) = lte_generate_crs(n, 0, N_id_cell);
        crs_r_4(n+1,:) = lte_generate_crs(n, 4, N_id_cell);
    endfor

    % Generate symbols
    frame_num = 0;
    slot_num  = 0;
    symb_num  = 0;
    last_idx  = 1;
    while(frame_num < N_frames)
        % Preload mod_vec and dirty_vec with zeros
        mod_vec   = zeros(N_ant, N_rb_dl*N_sc_rb);
        dirty_vec = zeros(N_ant, N_rb_dl*N_sc_rb);

        % Map PSS to grid
        if(slot_num == 0 || slot_num == 10)
            if(symb_num == (N_symb_dl-1))
                for(n=0:61)
                    k                = n - 31 + (N_rb_dl*N_sc_rb)/2;
                    mod_vec(1,k+1)   = pss_d_u(n+1);
                    dirty_vec(1,k+1) = 1;
                endfor
            endif
        endif

        % Map SSS to grid
        if(slot_num == 0)
            if(symb_num == (N_symb_dl-2))
                for(n=0:61)
                    k                = n - 31 + (N_rb_dl*N_sc_rb)/2;
                    mod_vec(1,k+1)   = sss_d_u_0(n+1);
                    dirty_vec(1,k+1) = 1;
                endfor
            endif
        elseif(slot_num == 10)
            if(symb_num == (N_symb_dl-2))
                for(n=0:61)
                    k                = n - 31 + (N_rb_dl*N_sc_rb)/2;
                    mod_vec(1,k+1)   = sss_d_u_5(n+1);
                    dirty_vec(1,k+1) = 1;
                endfor
            endif
        endif

        % Map CRS to grid
        if(symb_num == 0 || symb_num == (N_symb_dl-3))
            v_shift = mod(N_id_cell, 6);
            for(p=1:N_ant)
                if(symb_num == 0)
                    crs = crs_r_0;
                    if(p == 1)
                        v = 0;
                    else
                        v = 3;
                    endif
                else
                    crs = crs_r_4;
                    if(p == 1)
                        v = 3;
                    else
                        v = 0;
                    endif
                endif
                for(m=0:2*N_rb_dl-1)
                    k                = 6*m + mod((v + v_shift), 6);
                    m_prime          = m + N_rb_dl_max - N_rb_dl;
                    mod_vec(p,k+1)   = crs(slot_num+1,m_prime+1);
                    dirty_vec(:,k+1) = 1;
                endfor
            endfor
        endif

        % PBCH
        if(slot_num == 1)
            % Generate PBCH
            if(symb_num == 0 && mod(frame_num, 4) == 0)
                pbch_mib   = lte_mib_pack(N_rb_dl, "n", 1, frame_num);
                pbch_bits  = lte_bch_channel_encode(pbch_mib, N_ant);
                pbch_c     = lte_generate_prs_c(N_id_cell, 1920);
                pbch_bits  = mod(pbch_bits+pbch_c, 2);
                pbch_symbs = lte_modulation_mapper(pbch_bits, "qpsk");
                pbch_x     = lte_layer_mapper(pbch_symbs, N_ant, "tx_diversity");
                pbch_y     = lte_pre_coder(pbch_x, N_ant, "tx_diversity");
            endif

            % Dirty resource elements assuming 4 antennas
            saved_dirty_vec = dirty_vec;
            if(symb_num == 0 || symb_num == 1)
                for(p=1:N_ant)
                    for(m=0:4*N_rb_dl-1)
                        k = 3*m + mod(N_id_cell, 3);
                        dirty_vec(p,k+1) = 1;
                    endfor
                endfor
            endif

            % Map to resource elements
            if(symb_num == 0 || symb_num == 1 || symb_num == 2 || symb_num == 3)
                for(p=1:N_ant)
                    idx = 0;
                    for(k_prime=0:71)
                        k = (N_rb_dl*N_sc_rb)/2 - 36 + k_prime;
                        if(dirty_vec(p,k+1) == 0)
                            mod_vec(p,k+1)   = pbch_y(p,idx+1);
                            dirty_vec(p,k+1) = 1;
                            idx              = idx + 1;
                        endif
                    endfor
                endfor
                pbch_y = pbch_y(:,idx+1:end);
            endif
            dirty_vec = saved_dirty_vec;
        endif

        % CFICH
        if(mod(slot_num, 2) == 0 && symb_num == 0)
            cfi_bits   = lte_cfi_channel_encode(1);
            cfi_c_init = (floor(slot_num/2) + 1)*(2*N_id_cell + 1)*2^9 + N_id_cell;
            cfi_c      = lte_generate_prs_c(cfi_c_init, 32);
            cfi_bits   = mod(cfi_bits+cfi_c, 2);
            cfi_symbs  = lte_modulation_mapper(cfi_bits, "qpsk");
            cfi_x      = lte_layer_mapper(cfi_symbs, N_ant, "tx_diversity");
            cfi_y      = lte_pre_coder(cfi_x, N_ant, "tx_diversity");
            k_hat      = (N_sc_rb/2)*mod(N_id_cell, 2*N_rb_dl);
            k_1        = k_hat;
            k_2        = mod(k_hat + floor(N_rb_dl/2)*N_sc_rb/2, N_rb_dl*N_sc_rb);
            k_3        = mod(k_hat + floor(2*N_rb_dl/2)*N_sc_rb/2, N_rb_dl*N_sc_rb);
            k_4        = mod(k_hat + floor(3*N_rb_dl/2)*N_sc_rb/2, N_rb_dl*N_sc_rb);
            for(p=1:N_ant)
                idx = 0;
                for(n=0:5)
                    if(mod(N_id_cell,3) != mod(n,3))
                        mod_vec(p,k_1+n+1)   = cfi_y(p,idx+1);
                        mod_vec(p,k_2+n+1)   = cfi_y(p,idx+4+1);
                        mod_vec(p,k_3+n+1)   = cfi_y(p,idx+8+1);
                        mod_vec(p,k_4+n+1)   = cfi_y(p,idx+12+1);
                        dirty_vec(p,k_1+n+1) = 1;
                        dirty_vec(p,k_2+n+1) = 1;
                        dirty_vec(p,k_3+n+1) = 1;
                        dirty_vec(p,k_4+n+1) = 1;
                        idx                  = idx + 1;
                    endif
                endfor
            endfor
        endif

        for(p=1:N_ant)
            % Pad to the next largest FFT size and insert the DC 0
            ifft_input_vec = [zeros(1,FFT_pad_size), mod_vec(p,1:N_rb_dl*N_sc_rb/2), 0, mod_vec(p,(N_rb_dl*N_sc_rb/2)+1:end), zeros(1,FFT_pad_size-1)];

            % Take IFFT to get output samples
            ifft_output_vec = ifft(fftshift(ifft_input_vec));

            % Add cyclic prefix
            if(symb_num == 0)
                cp_output = [ifft_output_vec(length(ifft_output_vec)-N_cp_l_0+1:length(ifft_output_vec)), ifft_output_vec];
            else
                cp_output = [ifft_output_vec(length(ifft_output_vec)-N_cp_l_else+1:length(ifft_output_vec)), ifft_output_vec];
            endif

            % Concatenate output
            if(p == 1)
                final_out_0(last_idx:last_idx+length(cp_output)-1) = cp_output;
            else
                final_out_1(last_idx:last_idx+length(cp_output)-1) = cp_output;
            endif
        endfor
        last_idx = last_idx + length(cp_output);

        % Increment symbol, slot, and frame numbers
        symb_num = symb_num + 1;
        if(symb_num == N_symb_dl)
            symb_num = 0;
            slot_num = slot_num + 1;
            if(slot_num == N_slots_per_frame)
                slot_num  = 0;
                frame_num = frame_num + 1;
            endif
        endif
    endwhile
    if(N_ant == 1)
        output_samps = final_out_0;
    else
        output_samps = [final_out_0; final_out_1];
    endif

endfunction
