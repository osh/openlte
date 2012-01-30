%
% Copyright 2011-2012 Ben Wojtowicz
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
% Function:    lte_bch_channel_encode
% Description: Channel encodes the broadcast channel
% Inputs:      mib      - Master information block bits
%              N_ant    - Number of antenna ports to use
% Outputs:     bch_bits - Broadcast channel encoded
%                         bits
% Spec:        3GPP TS 36.212 section 5.3.1 v10.1.0
% Notes:       None
% Rev History: Ben Wojtowicz 10/30/2011 Created
%              Ben Wojtowicz 01/29/2012 Fixed license statement
%
function [bch_bits] = lte_bch_channel_encode(mib, N_ant)
    % Check mib
    if(length(mib) ~= 24)
        printf("ERROR: Invalid mib (length is %u, should be 24)\n", length(mib));
        bch_bits = 0;
        return;
    endif

    % Check N_ant
    if(N_ant == 1)
        ant_mask = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0];
    elseif(N_ant == 2)
        ant_mask = [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1];
    elseif(N_ant == 4)
        ant_mask = [0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1];
    else
        printf("ERROR: Invalid N_ant (%u)\n", N_ant);
        bch_bits = 0;
        return;
    endif

    % Define a_bits
    a_bits = mib;

    % Calculate p_bits
    p_bits = lte_calc_crc(a_bits, 16);

    % Mask p_bits
    for(n=0:length(p_bits)-1)
        p_bits(n+1) = mod(p_bits(n+1) + ant_mask(n+1), 2);
    endfor

    % Construct c_bits
    c_bits = [a_bits, p_bits];

    % Determine d_bits
    d_bits = cmn_conv_encode(c_bits, 7, 3, [133, 171, 165], 1);
    d_bits = reshape(d_bits, 3, []);

    % Determine e_bits
    e_bits = lte_rate_match(d_bits);

    % Return the e_bits
    bch_bits = e_bits;
endfunction

function [e_bits] = lte_rate_match(d_bits)
    [this_is_three, d_bits_len] = size(d_bits);

    % Sub block interleaving
    % Step 1: Assign C_cc_sb to 32
    C_cc_sb = 32;

    % Step 2: Determine the number of rows
    R_cc_sb = 0;
    while(d_bits_len > (C_cc_sb*R_cc_sb))
        R_cc_sb = R_cc_sb + 1;
    endwhile

    % Inter-column permutation values
    ic_perm = [1, 17, 9, 25, 5, 21, 13, 29, 3, 19, 11, 27, 7, 23, 15, 31, 0, 16, 8, 24, 4, 20, 12, 28, 2, 18, 10, 26, 6, 22, 14, 30] + 1;

    % Steps 3, 4, and 5
    for(x=0:this_is_three-1)
        % Step 3: Pack data into matrix and pad with dummy (NULL==10000 for this routine)
        if(d_bits_len < (C_cc_sb*R_cc_sb))
            N_dummy = C_cc_sb*R_cc_sb - d_bits_len;
        else
            N_dummy = 0;
        endif
        tmp = [10000*ones(1, N_dummy), d_bits(x+1,:)];
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
    K_pi = R_cc_sb*C_cc_sb;

    % Bit collection, selection and transmission
    % Create circular buffer and define E, the number of output bits
    w   = [v(1,:), v(2,:), v(3,:)];
    K_w = 3*K_pi;
    E   = 1920;
    % Create output
    k_idx = 0;
    j_idx = 0;
    while(k_idx < E)
        if(w(mod(j_idx, K_w)+1) ~= 10000)
            e_bits(k_idx+1) = w(mod(j_idx, K_w)+1);
            k_idx           = k_idx + 1;
        endif
        j_idx = j_idx + 1;
    endwhile
endfunction