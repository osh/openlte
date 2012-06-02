#!/usr/bin/env python

from gnuradio import gr
from gnuradio.eng_option import eng_option
from optparse import OptionParser
import LTE_fdd_dl_fs
import sys

class LTE_fdd_dl_file_scan (gr.top_block):

    def __init__(self):
        gr.top_block.__init__(self)

        usage = "usage: %prog [options] file"
        parser=OptionParser(option_class=eng_option, usage=usage)
        # Add options here
        (options, args) = parser.parse_args()
        if len(args) != 1:
            parser.print_help()
            sys.exit(1)

        input_filename = args[0]

        # Build flow graph
        self.fsrc = gr.file_source(gr.sizeof_char, input_filename, False)
        self.samp_buf = LTE_fdd_dl_fs.samp_buf()
        self.connect(self.fsrc, self.samp_buf)

if __name__ == '__main__':
    tb = LTE_fdd_dl_file_scan()
    try:
        tb.run()
    except KeyboardInterrupt:
        pass
