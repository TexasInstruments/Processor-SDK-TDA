/*
 *
 * Copyright (c) 2019-2026 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 *
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 * license under copyrights and patents it now or hereafter owns or controls to make,
 * have made, use, import, offer to sell and sell ("Utilize") this software subject to the
 * terms herein.  With respect to the foregoing patent license, such license is granted
 * solely to the extent that any such patent is necessary to Utilize the software alone.
 * The patent license shall not apply to any combinations which include this software,
 * other than combinations with devices manufactured by or for TI ("TI Devices").
 * No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this license
 * (including the above copyright notice and the disclaimer and (if applicable) source
 * code license limitations below) in the documentation and/or other materials provided
 * with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted provided
 * that the following conditions are met:
 *
 * *       No reverse engineering, decompilation, or disassembly of this software is
 * permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the software
 * licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the
 * source code are permitted provided that the following conditions are met:
 *
 * *       any redistribution and use of the source code, including any resulting derivative
 * works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 * and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers
 *
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef TIVX_STEREO_KERNELS_H_
#define TIVX_STEREO_KERNELS_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>
#if defined(PC)
#include <perception/perception.h>
#endif

#include <perception/algos.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup group_vision_apps_kernels_stereo TIVX Kernels for Stereo
 *
 * \brief This section documents the kernels defined for Stereo.
 *
 * The DMPAC SDE node itself is available as part of TIOVX.
 *
 * \ingroup group_vision_apps_kernels
 */

/*!
 * \file
 * \brief The list of supported kernels in this kernel extension.
 */

/*! \brief OpenVX module name
 * \ingroup group_vision_apps_kernels_stereo
 */
#define TIVX_MODULE_NAME_STEREO    "stereo"


/*! \brief ss_sde_pointcloud kernel name
 *  \see group_vision_apps_kernels_stereo
 */
#define TIVX_KERNEL_POINT_CLOUD_CREATION_NAME          "com.ti.stereo.point_cloud_creation"

/*! \brief ss_sde_og_detection kernel name
 *  \see group_vision_apps_kernels_stereo
 */
#define TIVX_KERNEL_OCCUPANCY_GRID_DETECTION_NAME          "com.ti.stereo.occupancy_grid_detection"

/*! \brief Kernel Name: Stereo Confidence Histogram Visualization
 * \ingroup group_vision_apps_kernels_stereo
 */
#define TIVX_KERNEL_SDE_HISTOGRAM_VISUALIZE_NAME     "com.ti.stereo.sde_histogram_visualize"

/*! \brief Kernel Name: Stereo Triangulation
 * \ingroup group_vision_apps_kernels_stereo
 */
#define TIVX_KERNEL_SDE_TRIANGULATION_NAME     "com.ti.stereo.sde_triangulation"

/*! \brief disparity merge kernel name
 *  \see group_vision_apps_kernels_stereo
 */
#define TIVX_KERNEL_DISPARITY_MERGE_NAME     "com.ti.stereo.disparity_merge"

/*! \brief median_filter kernel name
 *  \see group_vision_apps_kernels_stereo
 */
#define TIVX_KERNEL_MEDIAN_FILTER_NAME     "com.ti.stereo.median_filter"

/*! \brief hole_filling kernel name
 *  \see group_vision_apps_kernels_stereo
 */
#define TIVX_KERNEL_HOLE_FILLING_NAME     "com.ti.stereo.hole_filling"

/*! \brief extract_disparity_confidence kernel name
 *  \see group_vision_apps_kernels_stereo
 */
#define TIVX_KERNEL_EXTRACT_DISPARITY_CONFIDENCE_NAME     "com.ti.stereo.extract_disparity_confidence"

/*! \brief ground_estimation kernel name
 *  \see group_vision_apps_kernels_stereo
 */
#define TIVX_KERNEL_GROUND_ESTIMATION_NAME     "com.ti.stereo.ground_estimation"

/*! \brief obstacle_detection kernel name
 *  \see group_vision_apps_kernels_stereo
 */
#define TIVX_KERNEL_OBSTACLE_DETECTION_NAME     "com.ti.stereo.obstacle_detection"

/*! \brief sde_disparity_visualize kernel name
 *  \see group_vision_apps_kernels_stereo
 */
#define TIVX_KERNEL_SDE_DISPARITY_VISUALIZE_NAME     "com.ti.stereo.sde_disparity_visualize"


/*! \brief Control command for resetting the ground estimation library.
 *  \see group_vision_apps_kernels_stereo
 */
#define TIVX_KERNEL_GROUND_ESTIMATION_RESET          (0U)

/*! \brief Control command for resetting the occupancy grid detection library.
 *  \see group_vision_apps_kernels_stereo
 */
#define TIVX_KERNEL_OCCUPANCY_GRID_DETECTION_RESET   (1U)


/*!
 * \brief Stereo camera parameters
 *
 * Used by \ref tivxSdeDisparityVisualizeNode, \ref tivxSdeTriangulationNode
 *
 * \ingroup group_vision_apps_kernels_stereo
 */
typedef struct {
    vx_float32 baseline;      /**< baseline between left and right */
    vx_float32 scale_x;       /**< horizontal scaling factor when input image and output disparity size are differernt */
    vx_float32 scale_y;       /**< vertical scaling factor when input image and output disparity size are differernt   */
    vx_float32 focal_length;  /**< camera focal length */
    vx_uint16  dist_center_x; /**< distortion center x */
    vx_uint16  dist_center_y; /**< distortion center y */
} tivx_stereo_cam_params_t;


/*!
 * \brief Stereo 3D point cloud parameters
 *
 * Used by \ref tivxSdeDisparityVisualizeNode, \ref tivxSdeTriangulationNode
 *
 * \ingroup group_vision_apps_kernels_stereo
 */
typedef struct {
    vx_uint8   usePCConfig;   /**< whether to use the below configs */
    vx_uint8   subSampleRatio;/**< sub-sampling ratio */
    vx_uint8   thConfidence;  /**< minimum threshold to render */
    vx_float32 lowLimitX;     /**< minimum X pos to render */
    vx_float32 highLimitX;    /**< maximum X pos to render */
    vx_float32 lowLimitY;     /**< minimum Y pos to render */
    vx_float32 highLimitY;    /**< maximum Y pos to render */ 
    vx_float32 lowLimitZ;     /**< minimum Z pos to render */
    vx_float32 highLimitZ;    /**< maximum Z pos to render */
} tivx_stereo_pointcloud_params_t;

/*!
 * \brief Stereo disparity visualization parameters
 *
 * Used by \ref tivxSdeDisparityVisualizeNode
 *
 * \ingroup group_vision_apps_kernels_stereo
 */
typedef struct {
    vx_int16   disparity_min;
    vx_int16   disparity_max;
    vx_int8    disparity_only;
    vx_uint8   vis_confidence;
} tivx_sde_disparity_vis_params_t;

/*!
 * \brief Stereo based obstacle detection parameters
 *
 * Used by \ref tivxObstacleDetectionNode
 *
 * \ingroup group_vision_apps_kernels_stereo
 */
typedef PTK_Alg_StereoAlgo_ObjectDetect_allParams     tivx_obstacle_detection_params_t;


/*!
 * \brief Stereo based grond plane estimation parameters
 *
 * Used by \ref tivxGroundEstimationNode
 *
 * \ingroup group_vision_apps_kernels_stereo
 */
typedef PTK_Alg_StereoAlgo_GroundEstimation_allParams tivx_ground_estimation_params_t;

/*!
 * \brief Disparity hole filling algorithm parameters
 *
 * Used by \ref tivxHoleFillingNode
 *
 * \ingroup group_vision_apps_kernels_stereo
 */
typedef PTK_Alg_StereoPP_HoleFilling_configParams     tivx_hole_filling_params_t;

/*!
 * \brief Disparity median filtering algorithm parameters
 *
 * Used by \ref tivxMedianFilterNode
 *
 * \ingroup group_vision_apps_kernels_stereo
 */
typedef PTK_Alg_StereoPP_MedianFilter_configParams    tivx_median_filter_params_t;

/*!
 * \brief Disparity merge algorithm parameters
 *
 * Used by \ref tivxDisparityMergeNode
 *
 * \ingroup group_vision_apps_kernels_stereo
 */
typedef PTK_Alg_StereoPP_DisparityMerge_configParams  tivx_disparity_merge_params_t;


/*!
 * \brief Ground plane model structure
 *
 * Used by \ref tivxGroundEstimationNode
 *
 * \ingroup group_vision_apps_kernels_stereo
 */
typedef PTK_Alg_StereoAlgo_groundModelParams_Array    tivx_ground_model_params_t;


/*!
 * \brief 2D bounding box structure
 *
 * Used by \ref tivxObstacleDetectionNode
 *
 * \ingroup group_vision_apps_kernels_stereo
 */
typedef PTK_Alg_StereoAlgo_obsBox                     tivx_obstacle_pos_t;

/*!
 * \brief Drivable free space structure
 *
 * Used by \ref tivxObstacleDetectionNode
 *
 * \ingroup group_vision_apps_kernels_stereo
 */
typedef PTK_Alg_StereoAlgo_driveSpace                 tivx_drivable_space_t;


/*!
 * \brief Point cloud creation parameters for OG based obstacle detection
 *
 * Used by \ref tivxPointCloudCreationNode
 *
 * \ingroup group_vision_apps_kernels_stereo
 */
typedef PTK_Alg_StereoOG_CreatePCAllParams           tivx_ss_sde_point_cloud_params_t;

/*!
 * \brief Occupancy Grid based detection configuration parameters
 *
 * Used by \ref tivxOccupancyGridDetectionNode
 *
 * \ingroup group_vision_apps_kernels_stereo
 */
typedef PTK_Alg_StereoOG_CreateOGAllParams           tivx_ss_sde_og_detection_params_t;

/*!
 * \brief 3D obstacle bounding box structure
 *
 * Used by \ref tivxOccupancyGridDetectionNode
 *
 * \ingroup group_vision_apps_kernels_stereo
 */
typedef PTK_Alg_StereoOG_obs3DBox                     tivx_ss_sde_obs_3d_bound_box_t;


/*********************************
 *      Function Prototypes
 *********************************/

/*!
 * \brief Used for the application to load the stereo kernels into the context.
 * \ingroup group_vision_apps_kernels_stereo
 */
void tivxStereoLoadKernels(vx_context context);

/*!
 * \brief Used for the application to unload the stereo kernels from the context.
 * \ingroup group_vision_apps_kernels_stereo
 */
void tivxStereoUnLoadKernels(vx_context context);

/*!
 * \brief Function to register STEREO Kernels on the Target
 * \ingroup group_tivx_ext
 */
void tivxRegisterStereoTargetKernels(void);

/*!
 * \brief Function to un-register STEREO Kernels on the Target
 * \ingroup group_tivx_ext
 */
void tivxUnRegisterStereoTargetKernels(void);


/*!
 * \brief Used to print the performance of the kernels.
 * \ingroup group_kernel
 */
void tivxStereoPrintPerformance(vx_perf_t performance, uint32_t numPixels, const char* testName);

#ifdef __cplusplus
}
#endif

#endif /* TIVX_STEREO_KERNELS_H_ */


