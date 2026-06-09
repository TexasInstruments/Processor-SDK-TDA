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

import sim
import sys
import sim_utils
from sim_utils.sim_print_messages import *
from uart_ctrl import uart_ctrl
import sim_setup
from armsp.sim_shared.plugins import info
import os

###### INIT #######
try:
    rproc_load_env = int(os.environ["QNX_RPROC_BOOT"])
    if rproc_load_env == 0:
        print("QNX_RPROC_BOOT=0. Remote processors will not be loaded.")
        sim.suspend_script()
except:
    print("Environment Variable 'QNX_RPROC_BOOT' is not defined. Remote processors will not be loaded.")
    sim.suspend_script()

if len(sys.argv) < 2:
    ERROR_MSG('Not Enough arguments')
    sys.exit(1)
uart_0=uart_ctrl(sys.argv[1])

###### MAIN ######
print("Waiting for QNX Message prompt 'Welcome to QNX' to boot remoteproc cores")
uart_0.expect("Welcome to QNX",100000,5000)
if uart_0.string_match_detected:
    sim.wait_for((5000,'ms'))
    vpconfig_dir = info.get_vp_config_dir()
    handle_m55 = sim_setup.create_standalone_sc_thread(os.path.join(vpconfig_dir, 'scripts/board-support/init_m55.py'))
    handle_c76_0 = sim_setup.create_standalone_sc_thread(os.path.join(vpconfig_dir, 'scripts/board-support/init_c7x.py'))
    # handle_r52_0 = sim_setup.create_standalone_sc_thread(os.path.join(vpconfig_dir, 'scripts/board-support/init_r52.py'))
else:
    INFO_MSG("Did not detect qnx login prompt")

sim.suspend_script()


