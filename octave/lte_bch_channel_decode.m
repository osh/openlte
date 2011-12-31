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
% Function:    lte_bch_channel_decode
% Description: Channel decodes the broadcast channel
% Inputs:      bch_bits - Broadcast channel encoded
%                         bits
% Outputs:     mib      - Master information block bits
%              N_ant    - Number of antenna ports used
% Spec:        3GPP TS 36.212 section 5.3.1 v10.1.0
% Notes:       None
% Rev History: Ben Wojtowicz 11/12/2011 Created
%
function [mib, N_ant] = lte_bch_channel_decode(bch_bits)

    % Rate match to get the d_bits
    d_bits = lte_rate_unmatch(bch_bits);
    d_bits = reshape(d_bits, 1, []);

    % Convert from soft NRZ to hard bits for viterbi decoder
    for(n=1:length(d_bits))
        if(d_bits(n) > 0)
            d_bits(n) = 0;
        else
            d_bits(n) = 1;
        endif
    endfor

    % Viterbi decode the d_bits to get the c_bits
    c_bits = cmn_viterbi_decode(d_bits, 7, 3, [133, 171, 165]);

    % Recover a_bits and p_bits
    a_bits = c_bits(1:24);
    p_bits = c_bits(25:40);

    % Calculate p_bits
    calc_p_bits = lte_calc_crc(a_bits, 16);

    % Try all p_bit masks
    ant_mask_1 = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0];
    ant_mask_2 = [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1];
    ant_mask_4 = [0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1];
    check_1    = 0;
    check_2    = 0;
    check_4    = 0;
    for(n=0:length(calc_p_bits)-1)
        bit     = mod(calc_p_bits(n+1) + ant_mask_1(n+1), 2);
        check_1 = check_1 + abs(bit-p_bits(n+1));
        bit     = mod(calc_p_bits(n+1) + ant_mask_2(n+1), 2);
        check_2 = check_2 + abs(bit-p_bits(n+1));
        bit     = mod(calc_p_bits(n+1) + ant_mask_4(n+1), 2);
        check_4 = check_4 + abs(bit-p_bits(n+1));
    endfor
    if(check_1 == 0)
        N_ant = 1;
        mib   = a_bits;
    elseif(check_2 == 0)
        N_ant = 2;
        mib   = a_bits;
    elseif(check_4 == 0)
        N_ant = 4;
        mib   = a_bits;
    else
        N_ant = 0;
        mib   = 0;
    endif
endfunction

function [d_bits] = lte_rate_unmatch(e_bits)
    % In order to undo bit collection, selection and transmission, a dummy block must be
    % sub-block interleaved to determine where NULL bits are to be inserted
    % Sub block interleaving
    % Step 1: Assign C_cc_sb to 32
    C_cc_sb = 32;

    % Step 2: Determine the number of rows
    R_cc_sb    = 0;
    d_bits_len = 40; % FIXME This is specific for BCH
    while(d_bits_len > (C_cc_sb*R_cc_sb))
        R_cc_sb = R_cc_sb + 1;
    endwhile

    % Inter-column permutation values
    ic_perm = [1, 17, 9, 25, 5, 21, 13, 29, 3, 19, 11, 27, 7, 23, 15, 31, 0, 16, 8, 24, 4, 20, 12, 28, 2, 18, 10, 26, 6, 22, 14, 30] + 1;

    % Steps 3, 4, and 5
    for(x=0:3-1)
        % Step 3: Pack data into matrix and pad with dummy (NULL==10000 for this routine)
        if(d_bits_len < (C_cc_sb*R_cc_sb))
            N_dummy = C_cc_sb*R_cc_sb - d_bits_len;
        else
            N_dummy = 0;
        endif
        tmp = [10000*ones(1, N_dummy), zeros(1,d_bits_len)];
        idx = 0;
        for(n=0:R_cc_sb-1)
            for(m=0:C_cc_sb-1)
                sb_mat(n+1,m+1) = tmp(idx+1);
                idx             = idx + 1;
            endfor
        endfor

        % Step 4: Inter-column permutation
        for(n=0:R_cc_sb-1)
            for(m=0:C_cc_sb-1)
                sb_perm_mat(n+1,m+1) = sb_mat(n+1,ic_perm(m+1));
            endfor
        endfor

        % Step 5: Read out the bits
        idx = 0;
        for(m=0:C_cc_sb-1)
            for(n=0:R_cc_sb-1)
                v(x+1,idx+1) = sb_perm_mat(n+1,m+1);
                idx          = idx + 1;
            endfor
        endfor
    endfor
    
    % Undo bit collection, selection, and transmission by recreating the circular buffer
    K_pi  = R_cc_sb*C_cc_sb;
    K_w   = 3*K_pi;
    w_dum = [v(1,:), v(2,:), v(3,:)];
    w     = 10000*ones(1,K_w);
    E     = 1920;
    k_idx = 0;
    j_idx = 0;
    while(k_idx < E)
        if(w_dum(mod(j_idx, K_w)+1) != 10000)
            % FIXME: soft combine the inputs
            if(w(mod(j_idx, K_w)+1) == 10000)
                w(mod(j_idx, K_w)+1) = e_bits(k_idx+1);
%            elseif(e_bits(k_idx+1) != 10000)
%                w(mod(j_idx, K_w)+1) = w(mod(j_idx, K_w)+1) + e_bits(k_idx+1);
            endif
            k_idx = k_idx + 1;
        endif
        j_idx = j_idx + 1;
    endwhile

    % Recreate sub-block interleaver output
    v(1,:) = w(1:K_pi);
    v(2,:) = w(K_pi+1:2*K_pi);
    v(3,:) = w(2*K_pi+1:end);

    % Sub block deinterleaving
    % Steps 5, 4, and 3
    for(x=0:3-1)
        % Step 5: Load the permuted matrix
        idx = 0;
        for(m=0:C_cc_sb-1)
            for(n=0:R_cc_sb-1)
                sb_perm_mat(n+1,m+1) = v(x+1,idx+1);
                idx                  = idx + 1;
            endfor
        endfor

        % Step 4: Undo permutation
        for(n=0:R_cc_sb-1)
            for(m=0:C_cc_sb-1)
                sb_mat(n+1,ic_perm(m+1)) = sb_perm_mat(n+1,m+1);
            endfor
        endfor

        % Step 3: Unpack the data and remove dummy
        if(d_bits_len < (C_cc_sb*R_cc_sb))
            N_dummy = C_cc_sb*R_cc_sb - d_bits_len;
        else
            N_dummy = 0;
        endif
        idx = 0;
        for(n=0:R_cc_sb-1)
            for(m=0:C_cc_sb-1)
                tmp(idx+1) = sb_mat(n+1,m+1);
                idx        = idx + 1;
            endfor
        endfor
        d_bits(x+1,:) = tmp(N_dummy+1:end);
    endfor
endfunction
