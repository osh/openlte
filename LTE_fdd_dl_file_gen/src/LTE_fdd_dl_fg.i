/* -*- c++ -*- */

%include "gnuradio.i"			// the common stuff

%{
#include "LTE_fdd_dl_fg_samp_buf.h"
%}

// ----------------------------------------------------------------

/*
 * First arg is the package prefix.
 * Second arg is the name of the class minus the prefix.
 *
 * This does some behind-the-scenes magic so we can
 * access LTE_fdd_dl_fg_samp_buf from python as LTE_fdd_dl_fg.samp_buf
 */
GR_SWIG_BLOCK_MAGIC(LTE_fdd_dl_fg,samp_buf);

LTE_fdd_dl_fg_samp_buf_sptr LTE_fdd_dl_fg_make_samp_buf(unsigned int  _N_frames,
                                                        unsigned int  _N_ant,
                                                        unsigned int  _N_id_cell,
                                                        float         _bandwidth,
                                                        char         *_mcc,
                                                        char         *_mnc);

class LTE_fdd_dl_fg_samp_buf : public gr_block
{
private:
    LTE_fdd_dl_fg_samp_buf(uint32  _N_frames,
                           uint32  _N_ant,
                           uint32  _N_id_cell,
                           float   _bandwidth,
                           char   *_mcc,
                           char   *_mnc);
};
