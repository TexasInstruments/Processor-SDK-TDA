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

import sys
import os
from armsp.sim_shared.plugins import info

script_dir = os.path.join(info.get_vp_config_dir(), 'scripts/board-support')
if script_dir not in sys.path:
    sys.path.append(script_dir)

from dm_utils import *

print('init_c7x.py started')

c7x_0_reset_vector = 0xb1200000
c7x_1_reset_vector = 0xb3800000
c7x_2_reset_vector = 0xb5800000
c7x_3_reset_vector = 0xb7800000

# Due to a VDK bug, IPC with C7x can only work with one instance at a time

# Default = Option 1: C7x_0
# Set reset vectors
# write_mem_reg_with_val(mem_CCC_DMSC_BOOT, AW0_C76_INTVECT, c7x_0_reset_vector)
# Configure cores out of reset for each PSC instance
# program_lpsc(mem_MAIN_PSC_0_mReg, [LPSC_MAIN_AW0_C7_IDX])

# Option 2: C7x_1 (Uncomment below and comment every other options)
# Set reset vectors
#write_mem_reg_with_val(mem_CCC_DMSC_BOOT, AW1_C76_INTVECT, c7x_1_reset_vector)
# Configure cores out of reset for each PSC instance
#program_lpsc(mem_MAIN_PSC_0_mReg, [LPSC_MAIN_AW1_C7_IDX])

# Option 3: C7x_2 (Uncomment below and comment every other options)
# Set reset vectors
#write_mem_reg_with_val(mem_CCC_DMSC_BOOT, AW2_C76_INTVECT, c7x_2_reset_vector)
# Configure cores out of reset for each PSC instance
#program_lpsc(mem_MAIN_PSC_0_mReg, [LPSC_MAIN_AW2_C7_IDX])

# Option 4: C7x_3 (Uncomment below and comment every other options)
# Set reset vectors
#write_mem_reg_with_val(mem_CCC_DMSC_BOOT, AW3_C76_INTVECT, c7x_3_reset_vector)
# Configure cores out of reset for each PSC instance
#program_lpsc(mem_MAIN_PSC_0_mReg, [LPSC_MAIN_AW3_C7_IDX])

print('init_c7x.py Done.')
sim.suspend_script()
