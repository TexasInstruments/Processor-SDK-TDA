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

kernel = Kernel("pose_estimation") # Kernel name

########################################
# Define I/O for the Node
# Input corners detected from four instances of point_detect  
#kernel.setParameter(Type.USER_DATA_OBJECT, Direction.INPUT, ParamState.REQUIRED,  "IN_CORNER_POINTS",      ['svACDetectStructFourCameraCorner_t'])

# Input SV Struct // This has all the params (Extend to add new params including inchartPos)
kernel.setParameter(Type.USER_DATA_OBJECT, Direction.INPUT, ParamState.REQUIRED,  "IN_CONFIGURATION",      ['svPoseEstimation_t'])

kernel.setParameter(Type.USER_DATA_OBJECT, Direction.INPUT, ParamState.REQUIRED,  "IN_LDCLUT",               ['svLdcLut_t'])

kernel.setParameter(Type.OBJECT_ARRAY, Direction.INPUT, ParamState.REQUIRED,  "IN_CORNER_POINTS",      ['svACDetectStructFinalCorner_t'])

kernel.setParameter(Type.USER_DATA_OBJECT, Direction.OUTPUT, ParamState.REQUIRED, "OUT_CALMAT",      ['svACCalmatStruct_t'])
########################################


# These are the attributes to check in validate, typically image size etc, for struct
# other items as described in tivox\tools\PyTIVOX\tiovx\vx_array_attribute_e


########################################
# Add all scratch buffers here
# Size are set based on parameters of IN_CONFIGURATION
# Array pointers
kernel.allocateLocalMemory("buf_cip", ["sizeof(CameraIntrinsicParams)"])
kernel.allocateLocalMemory("buf_chartPoints", ["sizeof(Point2D_f) * NUM_CHART_CORNERS"])
kernel.allocateLocalMemory("buf_cornerPoints", ["sizeof(Point2D_f) * NUM_CHART_CORNERS"])
kernel.allocateLocalMemory("buf_baseChartPoints", ["sizeof(Point2D_f) * NUM_CORNERS_PER_CHART"])



# Non Array pointers
kernel.allocateLocalMemory("buf_inConerPoints", [ "sizeof(vx_int32) * ((MAX_INPUT_CAMERAS*8*FP_TO_DETECT) +1)"])
kernel.allocateLocalMemory("buf_H_cg", ["sizeof(Matrix3D_f) * MAX_INPUT_CAMERAS"])
kernel.allocateLocalMemory("buf_R_cg", ["sizeof(Matrix3D_f) * MAX_INPUT_CAMERAS"])
kernel.allocateLocalMemory("buf_R_gc", ["sizeof(Matrix3D_f) * MAX_INPUT_CAMERAS"])
kernel.allocateLocalMemory("buf_t_cg", ["sizeof(Matrix3D_f) * MAX_INPUT_CAMERAS"])
kernel.allocateLocalMemory("buf_t_gc", ["sizeof(Matrix3D_f) * MAX_INPUT_CAMERAS"])
kernel.allocateLocalMemory("buf_normCornerPoint", ["sizeof(Point2D_f) * NUM_CHART_CORNERS"])
kernel.allocateLocalMemory("buf_points1norm", ["sizeof(Point2D_f) * NUM_CHART_CORNERS"])
kernel.allocateLocalMemory("buf_points2norm", ["sizeof(Point2D_f) * NUM_CHART_CORNERS"])
kernel.allocateLocalMemory("buf_xvec", ["sizeof(Flouble) * (NUM_CHART_CORNERS*2 +3)"])
kernel.allocateLocalMemory("buf_fvec", ["sizeof(Flouble) * (NUM_CHART_CORNERS*2 +3)"])
kernel.allocateLocalMemory("buf_bvec", ["sizeof(Flouble) * A_NCOLS"])
kernel.allocateLocalMemory("buf_tempvec", ["sizeof(Flouble) * (NUM_CHART_CORNERS*2 +3)"])
kernel.allocateLocalMemory("buf_deltavec", ["sizeof(Flouble) * A_NCOLS"])
kernel.allocateLocalMemory("buf_Jacob", ["sizeof(Flouble) * (NUM_CHART_CORNERS*2 +3) * A_NCOLS"])
kernel.allocateLocalMemory("buf_JacobT", ["sizeof(Flouble) * (NUM_CHART_CORNERS*2 +3)* A_NCOLS"])
kernel.allocateLocalMemory("buf_gammaIdentity", ["sizeof(Flouble) * A_NCOLS"])
kernel.allocateLocalMemory("buf_A", ["sizeof(Flouble) * A_SIZE"])
kernel.allocateLocalMemory("buf_U", ["sizeof(Flouble) * U_SIZE"])
kernel.allocateLocalMemory("buf_U1", ["sizeof(Flouble) * U_SIZE"])
kernel.allocateLocalMemory("buf_V", ["sizeof(Flouble) * V_SIZE"])
kernel.allocateLocalMemory("buf_Diag", ["sizeof(Flouble) * A_NROWS"])
kernel.allocateLocalMemory("buf_superDiag", ["sizeof(Flouble) * A_NROWS"])

kernel.allocateLocalMemory("buf_AInv", ["sizeof(Flouble) * A_NCOLS * A_NCOLS"])
kernel.allocateLocalMemory("buf_Q",    ["sizeof(Flouble) * A_NCOLS * A_NCOLS"])
kernel.allocateLocalMemory("buf_R",    ["sizeof(Flouble) * A_NCOLS * A_NCOLS"])
kernel.allocateLocalMemory("buf_yvec", ["sizeof(Flouble) * A_NCOLS"])
kernel.allocateLocalMemory("buf_h", ["sizeof(Flouble) * A_NCOLS"])
kernel.allocateLocalMemory("buf_R_DLT", ["sizeof(Flouble) * 9"])
kernel.allocateLocalMemory("buf_t_DLT", ["sizeof(Flouble) * 3"])
kernel.allocateLocalMemory("buf_in_corners", ["sizeof(svACDetectStructFourCameraCorner_t)"])



########################################

kernel.setTarget(Target.DSP1)
kernel.setTarget(Target.DSP2)

code.export(kernel)
code.exportDiagram(kernel)

