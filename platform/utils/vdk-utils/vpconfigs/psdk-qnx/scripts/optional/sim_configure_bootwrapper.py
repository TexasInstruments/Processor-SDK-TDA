# This distribution contains contributions or derivatives under copyright
# as follows:
#
# Copyright (c) 2026, Texas Instruments Incorporated
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# - Redistributions of source code must retain the above copyright notice,
#   this list of conditions and the following disclaimer.
# - Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# - Neither the name of Texas Instruments nor the names of its
#   contributors may be used to endorse or promote products derived
#   from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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



    

