/*******************************************************************************

    Copyright 2012 Ben Wojtowicz

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

    File: LTE_fdd_dl_fg_samp_buf.h

    Description: Contains all the definitions for the LTE FDD DL File Generator
                 sample buffer block.

    Revision History
    ----------    -------------    --------------------------------------------
    06/15/2012    Ben Wojtowicz    Created file.

*******************************************************************************/
#ifndef __LTE_FDD_DL_FG_SAMP_BUF_H__
#define __LTE_FDD_DL_FG_SAMP_BUF_H__


/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "gr_sync_block.h"
#include "liblte_phy.h"
#include "liblte_rrc.h"

/*******************************************************************************
                              DEFINES
*******************************************************************************/

#define LTE_FDD_DL_FG_SAMP_BUF_SIZE (307200)

/*******************************************************************************
                              FORWARD DECLARATIONS
*******************************************************************************/

class LTE_fdd_dl_fg_samp_buf;

/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/

typedef boost::shared_ptr<LTE_fdd_dl_fg_samp_buf> LTE_fdd_dl_fg_samp_buf_sptr;

/*******************************************************************************
                              CLASS DECLARATIONS
*******************************************************************************/

LTE_fdd_dl_fg_samp_buf_sptr LTE_fdd_dl_fg_make_samp_buf(uint32  _N_frames,
                                                        uint32  _N_ant,
                                                        uint32  _N_id_cell,
                                                        float   _bandwidth,
                                                        char   *_mcc,
                                                        char   *_mnc);
class LTE_fdd_dl_fg_samp_buf : public gr_sync_block
{
public:
    ~LTE_fdd_dl_fg_samp_buf();

    int32 work(int32                      ninput_items,
               gr_vector_const_void_star &input_items,
               gr_vector_void_star       &output_items);

private:
    friend LTE_fdd_dl_fg_samp_buf_sptr LTE_fdd_dl_fg_make_samp_buf(uint32  _N_frames,
                                                                   uint32  _N_ant,
                                                                   uint32  _N_id_cell,
                                                                   float   _bandwidth,
                                                                   char   *_mcc,
                                                                   char   *_mnc);

    LTE_fdd_dl_fg_samp_buf(uint32  _N_frames,
                           uint32  _N_ant,
                           uint32  _N_id_cell,
                           float   _bandwidth,
                           char   *_mcc,
                           char   *_mnc);

    // LTE library
    LIBLTE_PHY_STRUCT     *phy_struct;
    LIBLTE_RRC_MSG_STRUCT  rrc_msg;

    // Sample buffer
    float  *i_buf;
    float  *q_buf;
    uint32  samp_buf_idx;
    bool    samples_ready;

    // LTE parameters
    LIBLTE_RRC_MIB_STRUCT                   mib;
    LIBLTE_RRC_SYS_INFO_BLOCK_TYPE_1_STRUCT sib1;
    LIBLTE_PHY_PCFICH_STRUCT                pcfich;
    LIBLTE_PHY_PHICH_STRUCT                 phich;
    LIBLTE_PHY_PDCCH_STRUCT                 pdcch;
    float                                   phich_res;
    float                                   bandwidth;
    uint32                                  sfn;
    uint32                                  N_frames;
    uint32                                  N_id_cell;
    uint32                                  N_id_1;
    uint32                                  N_id_2;
    uint32                                  N_rb_dl;
    uint32                                  FFT_pad_size;
    uint8                                   N_ant;
};

#endif /* __LTE_FDD_DL_FG_SAMP_BUF_H__ */
