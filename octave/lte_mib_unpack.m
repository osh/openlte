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
% Function:    lte_mib_unpack
% Description: Unpacks all of the fiels from the Master
%              Information Block
% Inputs:      bw - System bandwidth (in number of RBs)
%              phich_dur - PHICH Duration (normal or extended)
%              phich_res - Number of PHICH groups (1/6, 1/2, 1, 2)
%              sfn       - System frame number
% Outputs:     mib       - Packed master information block
% Spec:        3GPP TS 36.331 section 6.2.2 v10.0.0
% Notes:       None
% Rev History: Ben Wojtowicz 10/30/2011 Created
%              Ben Wojtowicz 01/29/2012 Fixed license statement
%
function [bw, phich_dur, phich_res, sfn] = lte_mib_unpack(mib)
    % Check mib
    if(length(mib) ~= 24)
        printf("ERROR: Invalid mib (length is %u, should be 24)\n", length(mib));
        bw        = 0;
        phich_dur = 0;
        phich_res = 0;
        sfn       = 0;
        return;
    endif

    % Unpack MIB
    act_bw        = mib(1:3);
    act_phich_dur = mib(4);
    act_phich_res = mib(5:6);
    act_sfn       = mib(7:14);

    % Construct bandwidth
    act_bw = 4*act_bw(1) + 2*act_bw(2) + act_bw(3);
    if(act_bw == 0)
        bw = 6;
    elseif(act_bw == 1)
        bw = 15;
    elseif(act_bw == 2)
        bw = 25;
    elseif(act_bw == 3)
        bw = 50;
    elseif(act_bw == 4)
        bw = 75;
    elseif(act_bw == 5)
        bw = 100;
    else
        printf("ERROR: Invalid act_bw (%u)\n", act_bw);
        bw = 0;
    endif

    % Construct phich_dur
    if(act_phich_dur == 0)
        phich_dur = "normal";
    else
        phich_dur = "extended";
    endif

    % Construct phich_res
    act_phich_res = 2*act_phich_res(1) + act_phich_res(2);
    if(act_phich_res == 0)
        phich_res = 1/6;
    elseif(act_phich_res == 1)
        phich_res = 1/2;
    elseif(act_phich_res == 2)
        phich_res = 1;
    else
        phich_res = 2;
    endif

    % Construct SFN
    sfn = 0;
    for(n=0:7)
        sfn = sfn + act_sfn(n+1)*2^(9-n);
    endfor
endfunction