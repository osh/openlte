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
% Function:    lte_mib_pack
% Description: Packs all of the fields into the Master
%              Information Block
% Inputs:      bw        - System bandwidth (in number of RBs)
%              phich_dur - PHICH Duration (normal or extended)
%              phich_res - Number of PHICH groups (1/6, 1/2, 1, 2)
%              sfn       - System frame number
% Outputs:     mib       - Packed master information block
% Spec:        3GPP TS 36.331 section 6.2.2 v10.0.0
% Notes:       None
% Rev History: Ben Wojtowicz 10/30/2011 Created
%
function [mib] = lte_mib_pack(bw, phich_dur, phich_res, sfn)
    % Check bandwidth
    if(bw == 6)
        act_bw = [0,0,0];
    elseif(bw == 15)
        act_bw = [0,0,1];
    elseif(bw == 25)
        act_bw = [0,1,0];
    elseif(bw == 50)
        act_bw = [0,1,1];
    elseif(bw == 75)
        act_bw = [1,0,0];
    elseif(bw == 100)
        act_bw = [1,0,1];
    else
        printf("ERROR: Invalid bw (%u)\n", bw);
        mib = 0;
        return;
    endif

    % Check phich_dur
    if(phich_dur(1) == "n")
        act_phich_dur = 0;
    elseif(phich_dur(1) == "e")
        act_phich_dur = 1;
    else
        printf("ERROR: Invalid phich_dur (%s)\n", phich_dur);
        mib = 0;
        return;
    endif

    % Check phich_res
    if(phich_res == 1/6)
        act_phich_res = [0,0];
    elseif(phich_res == 1/2)
        act_phich_res = [0,1];
    elseif(phich_res == 1)
        act_phich_res = [1,0];
    elseif(phich_res == 2)
        act_phich_res = [1,1];
    else
        printf("ERROR: Invalid phich_res (%f)\n", phich_res);
        mib = 0;
        return;
    endif

    % Check SFN
    if(sfn >= 0 && sfn <= 1023)
        tmp = sfn;
        for(n=0:7)
            act_sfn(n+1) = floor(tmp/2^(9-n));
            tmp          = tmp - floor(tmp/2^(9-n))*2^(9-n);
        endfor
    else
        printf("ERROR: Invalid sfn (%u)\n", sfn);
        mib = 0;
        return;
    endif

    % Pack MIB
    mib = [act_bw, act_phich_dur, act_phich_res, act_sfn, zeros(1,10)];
endfunction