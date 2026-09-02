'''
* Copyright (C) 2019 - 2026 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from tiovx import *

code = KernelExportCode(Module.SRV, Core.GPU, "APP_KERNELS_PATH")

kernel = Kernel("gl_srv")

kernel.setParameter(Type.USER_DATA_OBJECT, Direction.INPUT, ParamState.REQUIRED, "CONFIGURATION", ['tivx_srv_params_t'])
kernel.setParameter(Type.OBJECT_ARRAY, Direction.INPUT, ParamState.REQUIRED, "INPUT", ['VX_TYPE_UINT16'])
kernel.setParameter(Type.OBJECT_ARRAY, Direction.INPUT, ParamState.OPTIONAL, "SRV_VIEWS", ['tivx_srv_coords_t'])
kernel.setParameter(Type.ARRAY, Direction.INPUT, ParamState.OPTIONAL, "GALIGN_LUT")
kernel.setParameter(Type.IMAGE, Direction.OUTPUT, ParamState.REQUIRED, "OUTPUT", ['VX_DF_IMAGE_RGBX'])

kernel.setTarget(Target.MPU_0)

code.export(kernel)
code.exportDiagram(kernel)
