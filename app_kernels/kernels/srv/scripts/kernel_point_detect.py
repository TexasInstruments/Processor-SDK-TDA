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

code = KernelExportCode("srv", Core.C66, "APP_KERNELS_PATH")
# module name = srv

kernel = Kernel("point_detect") # Kernel name


# Input SV Struct
kernel.setParameter(Type.USER_DATA_OBJECT, Direction.INPUT, ParamState.REQUIRED,  "IN_CONFIGURATION",        ['svPointDetect_t'])
kernel.setParameter(Type.USER_DATA_OBJECT, Direction.INPUT, ParamState.REQUIRED,  "IN_LDCLUT",               ['svLdcLut_t'])

# Input YUV Image  
kernel.setParameter(Type.IMAGE, Direction.INPUT, ParamState.REQUIRED,  "IN",                      ['VX_DF_IMAGE_U8'])

# Output SV Struct
kernel.setParameter(Type.USER_DATA_OBJECT, Direction.OUTPUT, ParamState.REQUIRED, "OUT_CONFIGURATION",       ['svACDetectStructFinalCorner_t'])

########################################
kernel.setParameter(Type.IMAGE, Direction.OUTPUT, ParamState.OPTIONAL, "BUF_BWLUMA_FRAME",        ['VX_DF_IMAGE_U8'])
# This is defined as output but will later become scratch
########################################


# These are the attributes to check in validate, typically image size etc, for struct
# other items as described in tivox\tools\PyTIVOX\tiovx\vx_array_attribute_e


# Add all scratch buffers here
# Size are set based on parameters of IN_CONFIGURATION
# The sizes are set manually in the code
kernel.allocateLocalMemory("buf_IntegralImg", ["TBD_SIZE_1"])
kernel.allocateLocalMemory("buf_IntegralRows", ["TBD_SIZE_1"])
kernel.allocateLocalMemory("buf_grayLumaFrame", ["TBD_SIZE_1"])

kernel.allocateLocalMemory("buf_bwLumaFrame", ["TBD_SIZE_1"])
kernel.allocateLocalMemory("buf_candFPId", ["TBD_SIZE_1"])
kernel.allocateLocalMemory("buf_candFPx", ["TBD_SIZE_1"])
kernel.allocateLocalMemory("buf_candFPy", ["TBD_SIZE_1"])
kernel.allocateLocalMemory("buf_svFPBoundaryPosStruct", ["TBD_SIZE_1"])
kernel.allocateLocalMemory("buf_intOutCenter", ["TBD_SIZE_1"])
kernel.allocateLocalMemory("buf_outCenterNum", ["TBD_SIZE_1"])
kernel.allocateLocalMemory("buf_outCenter", ["TBD_SIZE_1"])
kernel.allocateLocalMemory("buf_candidCenter", ["TBD_SIZE_1"])
kernel.allocateLocalMemory("buf_boundX", ["TBD_SIZE_1"])
kernel.allocateLocalMemory("buf_boundY", ["TBD_SIZE_1"])
kernel.allocateLocalMemory("buf_tempBoundX", ["TBD_SIZE_1"])
kernel.allocateLocalMemory("buf_tempBoundY", ["TBD_SIZE_1"])
kernel.allocateLocalMemory("buf_fcBoundX", ["TBD_SIZE_1"])
kernel.allocateLocalMemory("buf_fcBoundY", ["TBD_SIZE_1"])
kernel.allocateLocalMemory("buf_tempCorner", ["TBD_SIZE_1"])
kernel.allocateLocalMemory("buf_tempCorner1", ["TBD_SIZE_1"])

kernel.allocateLocalMemory("buf_tempCorner2", ["TBD_SIZE_1"])
kernel.allocateLocalMemory("buf_line1PtsX", ["TBD_SIZE_1"])
kernel.allocateLocalMemory("buf_line1PtsY", ["TBD_SIZE_1"])
kernel.allocateLocalMemory("buf_line2PtsX", ["TBD_SIZE_1"])
kernel.allocateLocalMemory("buf_line2PtsY", ["TBD_SIZE_1"])
kernel.allocateLocalMemory("buf_candidCorners", ["TBD_SIZE_1"])
kernel.allocateLocalMemory("buf_finalCorners", ["TBD_SIZE_1"])
kernel.allocateLocalMemory("buf_matchScore", ["TBD_SIZE_1"])

kernel.setTarget(Target.DSP1)
kernel.setTarget(Target.DSP2)

code.export(kernel)
code.exportDiagram(kernel)

