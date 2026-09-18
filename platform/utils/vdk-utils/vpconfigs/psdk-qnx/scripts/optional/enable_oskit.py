#############################################################################
# Portions contained herein copyright 1996-2026 Synopsys, Inc.              #
#                                                                           #
# This Synopsys software and all associated documentation are proprietary   #
# to Synopsys, Inc. and may only be used pursuant to the terms and          #
# conditions of a written license agreement with Synopsys, Inc.             #
# Synopsys licensees may modify this code as necessary for use with their   #
# products and/or Synopsys licensed products, and may distribute such       #
# modifications to other Synopsys licensees for the same purpose.           #
# All other use, reproduction, modification, or distribution of the         #
# Synopsys software or the associated documentation is strictly prohibited. #
#############################################################################

from armsp.common.plugins import common_utils
from armsp.sim_shared.plugins import oskit

#hierarchy_name should be modified if the VDK hierarchy changes (for example if there are multiple CPU_SS blocks)
hierarchy_name = None
cpu_name = common_utils.get_cpu_name(hierarchy_name)

# Load the Linux OS Kit to be able to trace Linux activity
oskit.enable_oskit(cpu_name, "ARMv8")


