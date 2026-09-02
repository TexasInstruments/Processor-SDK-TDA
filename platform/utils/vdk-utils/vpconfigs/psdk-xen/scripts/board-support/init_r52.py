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


print('init_mcu_r5.py started')

r50_0_reset_vector = 0x670004c0
r51_0_reset_vector = 0x670204c0
r52_0_reset_vector = 0x670404c0

# Due to a VDK bug, IPC with R52s can only work in one of the two options:
#     a. R52_0_0
#     b. R52_1_0 & R52_2_0

# Option a (Default)
#Configure core reset vector
write_mem_reg_with_val(mem_MCU_SEC_MMR_0_cfg0, RT_CLSTR0_CORE0_INTVEC, r50_0_reset_vector)
# Configure cores out of reset for each PSC instance
program_lpsc(mem_MCU_PSC_1_mReg, [LPSC_MCU_RT_CLSTR0_CORE0_IDX])

# Option b (Uncomment this block and comment option a to work)
#Configure core reset vector
#write_mem_reg_with_val(mem_MCU_SEC_MMR_0_cfg0, RT_CLSTR1_CORE0_INTVEC, r51_0_reset_vector)
#write_mem_reg_with_val(mem_MCU_SEC_MMR_0_cfg0, RT_CLSTR2_CORE0_INTVEC, r52_0_reset_vector)
# Configure cores out of reset for each PSC instance
#program_lpsc(mem_MCU_PSC_1_mReg, [LPSC_MCU_RT_CLSTR1_CORE0_IDX, LPSC_MCU_RT_CLSTR2_CORE0_IDX])

print('init_mcu_r5.py Done.')
sim.suspend_script()
