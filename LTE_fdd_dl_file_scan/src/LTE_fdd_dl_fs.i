/* -*- c++ -*- */

%include "gnuradio.i"			// the common stuff

%{
#include "LTE_fdd_dl_fs_samp_buf.h"
%}

// ----------------------------------------------------------------

/*
 * First arg is the package prefix.
 * Second arg is the name of the class minus the prefix.
 *
 * This does some behind-the-scenes magic so we can
 * access LTE_fdd_dl_fs_samp_buf from python as LTE_fdd_dl_fs.samp_buf
 */
GR_SWIG_BLOCK_MAGIC(LTE_fdd_dl_fs,samp_buf);

LTE_fdd_dl_fs_samp_buf_sptr LTE_fdd_dl_fs_make_samp_buf ();

class LTE_fdd_dl_fs_samp_buf : public gr_block
{
private:
    LTE_fdd_dl_fs_samp_buf ();
};
