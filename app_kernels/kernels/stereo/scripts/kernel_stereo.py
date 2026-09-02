'''
* Copyright (C) 2017 - 2026 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from tiovx import *

code = KernelExportCode(Module.STEREO, "target", "CUSTOM_APPLICATION_PATH")

# point colud creation node
code.setCoreDirectory("target")
kernel = Kernel("point_cloud_creation")

kernel.setParameter(Type.USER_DATA_OBJECT, Direction.INPUT,  ParamState.REQUIRED, "configuration",              ['tivx_ss_sde_point_cloud_params_t'])
kernel.setParameter(Type.IMAGE,            Direction.INPUT,  ParamState.REQUIRED, "input_image",                ['VX_DF_IMAGE_U8'])
kernel.setParameter(Type.IMAGE,            Direction.INPUT,  ParamState.REQUIRED, "input_sdeDisparity",         ['VX_DF_IMAGE_S16'])
kernel.setParameter(Type.TENSOR,           Direction.INPUT,  ParamState.REQUIRED, "input_tensor",               ['VX_TYPE_UINT8'])
kernel.setParameter(Type.USER_DATA_OBJECT, Direction.OUTPUT, ParamState.REQUIRED, "point_cloud_out",            ['PTK_PointCloud'])

kernel.setTarget(Target.DSP1)
kernel.setTarget(Target.DSP2)
kernel.setTarget(Target.MPU_0)
kernel.setTarget(Target.MPU_1)
kernel.setTarget(Target.MPU_2)
kernel.setTarget(Target.MPU_3)
code.export(kernel)

# occupancy grid detection node
code.setCoreDirectory("target")
kernel = Kernel("occupancy_grid_detection")

kernel.setParameter(Type.USER_DATA_OBJECT, Direction.INPUT,  ParamState.REQUIRED, "configuration",              ['tivx_ss_sde_og_detection_params_t'])
kernel.setParameter(Type.USER_DATA_OBJECT, Direction.INPUT,  ParamState.REQUIRED, "point_cloud_in",             ['PTK_PointCloud'])
kernel.setParameter(Type.USER_DATA_OBJECT, Direction.OUTPUT, ParamState.REQUIRED, "bound_box_3d_out",           ['tivx_ss_sde_obs_3d_bound_box_t'])

kernel.setTarget(Target.DSP1)
kernel.setTarget(Target.DSP2)
kernel.setTarget(Target.MPU_0)
kernel.setTarget(Target.MPU_1)
kernel.setTarget(Target.MPU_2)
kernel.setTarget(Target.MPU_3)
code.export(kernel)


# disparity merge node
code.setCoreDirectory("target")
kernel = Kernel("disparity_merge")

kernel.setParameter(Type.USER_DATA_OBJECT, Direction.INPUT,  ParamState.REQUIRED, "configuration",              ['tivx_disparity_merge_params_t'])
kernel.setParameter(Type.IMAGE,            Direction.INPUT,  ParamState.REQUIRED, "low_input_disparity",        ['VX_DF_IMAGE_S16'])
kernel.setParameter(Type.IMAGE,            Direction.INPUT,  ParamState.REQUIRED, "high_input_disparity",       ['VX_DF_IMAGE_S16'])
kernel.setParameter(Type.IMAGE,            Direction.OUTPUT, ParamState.REQUIRED, "output_disparity",           ['VX_DF_IMAGE_S16'])
       
kernel.setTarget(Target.DSP1)
kernel.setTarget(Target.DSP2)
kernel.setTarget(Target.MPU_0)
code.export(kernel)

# median filter node
code.setCoreDirectory("target")
kernel = Kernel("median_filter")

kernel.setParameter(Type.USER_DATA_OBJECT, Direction.INPUT,  ParamState.REQUIRED, "configuration",              ['tivx_median_filter_params_t'])
kernel.setParameter(Type.IMAGE,            Direction.INPUT,  ParamState.REQUIRED, "input_disparity",            ['VX_DF_IMAGE_S16'])
kernel.setParameter(Type.IMAGE,            Direction.OUTPUT, ParamState.REQUIRED, "output_disparity",           ['VX_DF_IMAGE_S16'])
       
kernel.setTarget(Target.DSP1)
kernel.setTarget(Target.DSP2)
kernel.setTarget(Target.MPU_0)
code.export(kernel)


# hole filling node
code.setCoreDirectory("target")
kernel = Kernel("hole_filling")

kernel.setParameter(Type.USER_DATA_OBJECT, Direction.INPUT,  ParamState.REQUIRED, "configuration",              ['tivx_hole_filling_params_t'])
kernel.setParameter(Type.IMAGE,            Direction.INPUT,  ParamState.REQUIRED, "input_disparity",            ['VX_DF_IMAGE_S16'])
       
kernel.setTarget(Target.DSP1)
kernel.setTarget(Target.DSP2)
kernel.setTarget(Target.MPU_0)
code.export(kernel)


# disparity confidence extraction node
code.setCoreDirectory("target")
kernel = Kernel("extract_disparity_confidence")

kernel.setParameter(Type.IMAGE,            Direction.INPUT,  ParamState.REQUIRED, "input_sdeDisparity",         ['VX_DF_IMAGE_S16'])
kernel.setParameter(Type.ARRAY,            Direction.OUTPUT, ParamState.REQUIRED, "output_disparity",           ['float'])
kernel.setParameter(Type.ARRAY,            Direction.OUTPUT, ParamState.REQUIRED, "output_confidence",          ['uint8_t'])
       
kernel.setTarget(Target.DSP1)
kernel.setTarget(Target.DSP2)
kernel.setTarget(Target.MPU_0)
code.export(kernel)


# Ground plane estimation node
code.setCoreDirectory("target")
kernel = Kernel("ground_estimation")

kernel.setParameter(Type.USER_DATA_OBJECT, Direction.INPUT,  ParamState.REQUIRED, "configuration",              ['tivx_ground_estimation_params_t'])
kernel.setParameter(Type.IMAGE,            Direction.INPUT,  ParamState.REQUIRED, "input_image",                ['VX_DF_IMAGE_U8'])
kernel.setParameter(Type.IMAGE,            Direction.INPUT,  ParamState.REQUIRED, "input_sdeDisparity",         ['VX_DF_IMAGE_S16'])
kernel.setParameter(Type.IMAGE,            Direction.OUTPUT, ParamState.REQUIRED, "output_disparity",           ['VX_DF_IMAGE_S16'])
kernel.setParameter(Type.USER_DATA_OBJECT, Direction.OUTPUT, ParamState.REQUIRED, "output_ground_model",        ['tivx_ground_model_params_t'])
       
kernel.setTarget(Target.DSP1)
kernel.setTarget(Target.DSP2)
kernel.setTarget(Target.MPU_0)
code.export(kernel)


# Obstacle Detection Node
code.setCoreDirectory("target")
kernel = Kernel("obstacle_detection")

kernel.setParameter(Type.USER_DATA_OBJECT, Direction.INPUT,  ParamState.REQUIRED, "configuration",              ['tivx_obstacle_detection_params_t'])
kernel.setParameter(Type.IMAGE,            Direction.INPUT,  ParamState.REQUIRED, "input_image",                ['VX_DF_IMAGE_U8'])
kernel.setParameter(Type.IMAGE,            Direction.INPUT,  ParamState.REQUIRED, "input_disparity",            ['VX_DF_IMAGE_S16'])
kernel.setParameter(Type.USER_DATA_OBJECT, Direction.INPUT,  ParamState.REQUIRED, "input_ground_model",         ['tivx_ground_model_params_t'])
kernel.setParameter(Type.ARRAY,            Direction.OUTPUT, ParamState.REQUIRED, "output_obstacle_pos",        ['tivx_obstacle_pos_t'])
kernel.setParameter(Type.SCALAR,           Direction.OUTPUT, ParamState.REQUIRED, "output_num_obstacles",       ['VX_SCALAR_TYPE'])
kernel.setParameter(Type.ARRAY,            Direction.OUTPUT, ParamState.REQUIRED, "output_freespace_boundary",  ['int32_t'])
kernel.setParameter(Type.USER_DATA_OBJECT, Direction.OUTPUT, ParamState.REQUIRED, "output_drivable_space",      ['tivx_drivable_space_t'])

kernel.setTarget(Target.DSP1)
kernel.setTarget(Target.DSP2)
kernel.setTarget(Target.MPU_0)
code.export(kernel)


# SDE disparity visualize
code.setCoreDirectory("target")
kernel = Kernel("sde_disparity_visualize")

kernel.setParameter(Type.USER_DATA_OBJECT, Direction.INPUT,  ParamState.REQUIRED, "configuration",              ['tivx_sde_disparity_vis_params_t'])
kernel.setParameter(Type.IMAGE,            Direction.INPUT,  ParamState.REQUIRED, "disparity",                  ['VX_DF_IMAGE_S16'])
kernel.setParameter(Type.IMAGE,            Direction.OUTPUT, ParamState.REQUIRED, "disparity_rgb",              ['VX_DF_IMAGE_RGB'])

kernel.setTarget(Target.DSP1)
kernel.setTarget(Target.DSP2)
kernel.setTarget(Target.MPU_0)
code.export(kernel)



