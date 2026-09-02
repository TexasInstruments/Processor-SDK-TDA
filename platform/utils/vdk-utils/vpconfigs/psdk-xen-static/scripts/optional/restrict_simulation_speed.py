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

import sim

#Restrict the rate at which SystemC time advances when the CPUs are in idle state
simSpeedLimiter = sim.get_full_model_name("SimulationSpeedLimiter")
sim.CommandProcessorProbe(simSpeedLimiter).execute_command("set_max_rate", ["1"])


