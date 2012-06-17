#!/usr/bin/env python

from gnuradio import gr
from gnuradio.eng_option import eng_option
from optparse import OptionParser
import LTE_fdd_dl_fg
import sys

class LTE_fdd_dl_file_gen (gr.top_block):

    def __init__(self):
        gr.top_block.__init__(self)

        usage = "usage: %prog [options] file N_frames N_ant N_id_cell bandwidth mcc mnc"
        parser=OptionParser(option_class=eng_option, usage=usage)
        # Add options here
        (options, args) = parser.parse_args()
        if len(args) != 7:
            parser.print_help()
            sys.exit(1)

        output_filename = args[0]
        N_frames = args[1]
        N_ant = args[2]
        N_id_cell = args[3]
        bandwidth = args[4]
        mcc = args[5]
        mnc = args[6]

        # Build flow graph
        self.samp_buf = LTE_fdd_dl_fg.samp_buf(int(N_frames), int(N_ant), int(N_id_cell), float(bandwidth), mcc, mnc)
        self.fsink = gr.file_sink(gr.sizeof_char, output_filename)
        self.connect(self.samp_buf, self.fsink)

if __name__ == '__main__':
    tb = LTE_fdd_dl_file_gen()
    try:
        tb.run()
    except KeyboardInterrupt:
        pass
