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

    File: LTE_fdd_dl_fs_samp_buf.h

    Description: Contains all the definitions for the LTE FDD DL File Scanner
                 sample buffer block.

    Revision History
    ----------    -------------    --------------------------------------------
    03/23/2012    Ben Wojtowicz    Created file.

*******************************************************************************/
#ifndef __LTE_FDD_DL_FS_SAMP_BUF_H__
#define __LTE_FDD_DL_FS_SAMP_BUF_H__


/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "gr_sync_block.h"
#include "liblte_phy.h"
#include "liblte_rrc.h"

/*******************************************************************************
                              DEFINES
*******************************************************************************/

#define LTE_FDD_DL_FS_SAMP_BUF_SIZE (307200*10)


/*******************************************************************************
                              FORWARD DECLARATIONS
*******************************************************************************/

class LTE_fdd_dl_fs_samp_buf;

/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/

typedef boost::shared_ptr<LTE_fdd_dl_fs_samp_buf> LTE_fdd_dl_fs_samp_buf_sptr;

/*******************************************************************************
                              CLASS DECLARATIONS
*******************************************************************************/

LTE_fdd_dl_fs_samp_buf_sptr LTE_fdd_dl_fs_make_samp_buf ();
class LTE_fdd_dl_fs_samp_buf : public gr_sync_block
{
public:
    ~LTE_fdd_dl_fs_samp_buf();

    int32 work(int32                      ninput_items,
               gr_vector_const_void_star &input_items,
               gr_vector_void_star       &output_items);

private:
    friend LTE_fdd_dl_fs_samp_buf_sptr LTE_fdd_dl_fs_make_samp_buf();

    LTE_fdd_dl_fs_samp_buf();

    // LTE library
    LIBLTE_PHY_STRUCT     *phy_struct;
    LIBLTE_RRC_MSG_STRUCT  rrc_msg;

    // Sample buffer
    float  *i_buf;
    float  *q_buf;
    uint32  samp_buf_idx;
    bool    last_samp_was_i;
};

#endif /* __LTE_FDD_DL_FS_SAMP_BUF_H__ */
