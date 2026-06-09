#############################################################################
# Copyright 2026 Synopsys, Inc.                                             #
#                                                                           #
# Permission is hereby granted, free of charge, to any person obtaining a   #
# copy of this software and associated documentation files (the             #
# "Software"), to deal in the Software without restriction, including       #
# without limitation the rights to use, copy, modify, merge, publish,       #
# distribute, sublicense, and/or sell copies of the Software, and to        #
# permit persons to whom the Software is furnished to do so, subject to     #
# the following conditions:                                                 #
#                                                                           #
# The above copyright notice and this permission notice shall be included   #
# in all copies or substantial portions of the Software.                    #
#                                                                           #
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS   #
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF                #
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.    #
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY      #
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,      #
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE         #
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                    #
############################################################################# 

import sim
from armsp.sim_shared.plugins import info

#hierarchy_name should be modified if the VDK hierarchy changes (for example if there are multiple CPU_SS blocks)
hierarchy_name = None
full_hierarchy_name = info.get_full_hierarchy_name(hierarchy_name)
dts_offset_addr = 0x10000000

# On Reset, reload the image files   
system_reset_name = sim.get_full_model_name(full_hierarchy_name + "Periphs.SYSREGS.nSYSTEM_RESET")
rst_signal = sim.BoolProbe(system_reset_name)    #handle to reset signal
rst_level = 1                                                           #current level of reset signal
                                                                        #init to 1 to avoid loading images at startup
def reset_cb(probe):
    global rst_level
    #Reset signal is active low. Only load the images when signal is de-asserted
    # 0->1 transition
    new_level = int(rst_signal.get_value())
    #print("new_level=%d, rst_level=%d"%(new_level, rst_level))
    if new_level == 1 and rst_level == 0:
        #0->1 transition
        print("Reloading image files")
        #Can't reload images through the CPU because at the time
        #of this breakpoint the internal state of the CPU has not been reset.
        dram_name = sim.get_full_model_name(full_hierarchy_name + "DRAM")
        dram = sim.ObjectInfo(dram_name)
        dram.load_vpconfig_images()
        #If Device tree is not loaded by vpconfig it can be done manually by uncommenting below
        #ram_name = sim.get_full_model_name(full_hierarchy_name + 'RAM')
        #res = sim.CommandProcessorProbe(ram_name).execute_command('mem_load', ["device_tree_output/device_tree_output.dtb", str(dts_offset_addr)])
    rst_level = new_level
    rst_signal.callback_on_any_value_change()

rst_signal.set_callback(reset_cb)
rst_signal.callback_on_any_value_change()


