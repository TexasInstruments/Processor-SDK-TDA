'''
* Copyright (C) 2017 - 2026 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

# The point detect node takes in as an argument a struct (sv) and an image
# The output is the updated sv struct as well as local buffers which are updated
# The node also utilized multiple scratch buffers - Need to check if the final
# output buffer needs to be separate than the intermediate buffers
# final output will be different, for partial nodes, we can use bi-di buffers

from tiovx import *

#code = KernelExportCode(Module.SURROUNDVIEW, Core.C66, "APP_KERNELS_PATH")
code = KernelExportCode("srv", Core.C66, "APP_KERNELS_PATH")
# module name = srv

kernel = Kernel("generate_3DBowl") # Kernel name

########################################
# Define I/O for the Node

# Input SV Struct // This has all the params (Extend to add new params including inchartPos)
kernel.setParameter(Type.USER_DATA_OBJECT, Direction.INPUT, ParamState.REQUIRED, "IN_CONFIGURATION",       ['svGpuLutGen_t'])
kernel.setParameter(Type.USER_DATA_OBJECT, Direction.INPUT, ParamState.REQUIRED, "IN_CALMAT",              ['svACCalmatStruct_t'])
kernel.setParameter(Type.USER_DATA_OBJECT, Direction.INPUT, ParamState.REQUIRED, "IN_OFFSET",              ['svGeometric_t'])

kernel.setParameter(Type.ARRAY, Direction.OUTPUT, ParamState.REQUIRED, "OUT_LUT3DXYZ",                     ['VX_TYPE_FLOAT32'])
kernel.setParameter(Type.USER_DATA_OBJECT, Direction.OUTPUT, ParamState.REQUIRED, "OUT_CALMAT_SCALED",     ['svACCalmatStruct_t'])
########################################


# These are the attributes to check in validate, typically image size etc, for struct
# other items as described in tivox\tools\PyTIVOX\tiovx\vx_array_attribute_e


########################################
# Add all scratch buffers here
# Size are set based on parameters of IN_CONFIGURATION
# Array pointers

########################################

kernel.setTarget(Target.DSP1)
kernel.setTarget(Target.DSP2)

code.export(kernel)
code.exportDiagram(kernel)

