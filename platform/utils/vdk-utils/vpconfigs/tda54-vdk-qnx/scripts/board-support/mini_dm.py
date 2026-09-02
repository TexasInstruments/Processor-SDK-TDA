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

import sim, sim_setup
import sim_utils
from sim_utils.sim_print_messages import *
from armsp.sim_shared.plugins import info
import os

class DM(object):
    def __init__(self):
        self.vpconfig_dir = info.get_vp_config_dir()
        # Attach callback on ROT_M55 to start u-boot-m55-spl.bin on DM_M55
        self.rot_m55_core = sim.CoreProbe("/TDA5_System/TDA5_SoC/MCU_Domain/Security/ROT/M55_SS/CLUSTER_0/cpu0")
        self.rot_m55_trace_function = "end_of_test_reached"
        self.dm_m55_reset_vector = '0x23e00000'
        self.rot_m55_func_observer = sim_utils.FunctionObserver(self.rot_m55_core, self.rot_m55_trace_function, self.rot_m55_bootcode_release_dm_m55, None)
        # Attach callback on DM_M55 to start ATF on A720_SS
        self.dm_m55_core = sim.CoreProbe("/TDA5_System/TDA5_SoC/MCU_Domain/DM/M55_SS/CLUSTER_0/cpu0")
        self.dm_m55_trace_function = "rproc_load"
        self.a720_ss_reset_vector = '0x23d80000'
        self.dm_m55_func_observer = sim_utils.FunctionObserver(self.dm_m55_core, self.dm_m55_trace_function, None, self.dm_m55_uboot_spl_release_a720)

    def rot_m55_bootcode_release_dm_m55(self, func_name, observer, args):
        # Start u-boot-m55-spl on DM_M55
        INFO_MSG("Core: %s has entered function %s" % (observer.core_probe.instance_name, func_name))
        handle_bootmode = sim_setup.create_standalone_sc_thread(os.path.join(self.vpconfig_dir, 'scripts/board-support/bootmode_selection.py'))
        handle_dm_m55 = sim_setup.create_standalone_sc_thread(os.path.join(self.vpconfig_dir, 'scripts/board-support/init_dm_m55.py'),[self.dm_m55_reset_vector])

    def dm_m55_uboot_spl_release_a720(self, func_name, observer, args):
        # just show an example of extracting some of the elements from the observer
        #if observer::function_state == observer::INSIDE
        # Start ATF on A720_SS
        handle_a720_ss = sim_setup.create_standalone_sc_thread(os.path.join(self.vpconfig_dir, 'scripts/board-support/init_a720_ss.py'),[self.a720_ss_reset_vector])
        INFO_MSG("Core: %s has returned from function %s" % (observer.core_probe.instance_name, func_name))


# Setup all callbacks and pointers
dm_inst = DM()

#nothing else needed
sim.suspend_script()

