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

from builtins import str
from armsp.sim_shared.plugins import armv8a_ss
from armsp.sim_shared.plugins import memory
from armsp.sim_shared.plugins import utils
from armsp.sim_shared.plugins import info
from armsp.common.plugins import common_utils
import sim
import os

#'hierarchy_name' should be modified if the VDK hierachy changes (for example if there are multiple CPU_SS blocks)
hierarchy_name = None

cpuSubsystem = common_utils.get_cpu_name(hierarchy_name)

memory.write64(utils.get_first_core(cpuSubsystem), "jump_to_kernel", 0)
memory.write64(utils.get_first_core(cpuSubsystem), "jump_to_uboot", 0)
if "kernel" == str(os.environ["BOOTWRAPPER_JUMP_TO"]):
    memory.write64(utils.get_first_core(cpuSubsystem), "jump_to_kernel", 1)
elif "u-boot" == str(os.environ["BOOTWRAPPER_JUMP_TO"]):
    memory.write64(utils.get_first_core(cpuSubsystem), "jump_to_uboot", 1)
else:
    memory.write64(utils.get_first_core(cpuSubsystem), "jump_to_kernel", 1)



    

