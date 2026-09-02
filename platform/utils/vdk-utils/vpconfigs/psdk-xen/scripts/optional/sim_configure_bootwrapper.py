#############################################################################
# Copyright 1996-2026 Synopsys, Inc.                                        #
#                                                                           #
# This Synopsys software and all associated documentation are proprietary   #
# to Synopsys, Inc. and may only be used pursuant to the terms and          #
# conditions of a written license agreement with Synopsys, Inc.             #
# All other use, reproduction, modification, or distribution of the         #
# Synopsys software or the associated documentation is strictly prohibited. #
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



    

