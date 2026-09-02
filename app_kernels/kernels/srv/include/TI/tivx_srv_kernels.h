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

#ifndef TIVX_SRV_KERNELS_H_
#define TIVX_SRV_KERNELS_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * \defgroup group_vision_apps_kernels_srv TIVX Kernels for SRV
 *
 * \brief This section documents the kernels defined for Surroundview processing
 * \ingroup group_vision_apps_kernels
 */


/*!
 * \file
 * \brief The list of supported kernels in this kernel extension.
 */

/*! \brief Name for OpenVX Extension kernel module: srv
 * \ingroup group_vision_apps_kernels_srv
 */
#define TIVX_MODULE_NAME_SRV    "srv"

/*! \brief The list of kernels supported in srv module
 *
 * Each kernel listed here can be used with the <tt> vxGetKernelByName</tt> call.
 * When programming the parameters, use
 * \arg <tt> VX_INPUT</tt> for [in]
 * \arg <tt> VX_OUTPUT</tt> for [out]
 * \arg <tt> VX_BIDIRECTIONAL</tt> for [in,out]
 *
 * When programming the parameters, use
 * \arg <tt> VX_TYPE_IMAGE</tt> for a <tt> vx_image</tt> in the size field of <tt> vxGetParameterByIndex</tt> or <tt> vxSetParameterByIndex</tt>
 * \arg <tt> VX_TYPE_ARRAY</tt> for a <tt> vx_array</tt> in the size field of <tt> vxGetParameterByIndex</tt> or <tt> vxSetParameterByIndex</tt>
 * \arg or other appropriate types in  vx_type_e.
 * \ingroup group_vision_apps_kernels_srv
 */

/*! \brief pose_estimation kernel name
 *  \ingroup group_vision_apps_kernels_srv
 */
#define TIVX_KERNEL_POSE_ESTIMATION_NAME     "com.ti.srv.pose_estimation"

/*! \brief point_detect kernel name
 *  \ingroup group_vision_apps_kernels_srv
 */
#define TIVX_KERNEL_POINT_DETECT_NAME     "com.ti.srv.point_detect"

/*! \brief generate_3dbowl kernel name
 *  \ingroup group_vision_apps_kernels_srv
 */
#define TIVX_KERNEL_GENERATE_3DBOWL_NAME     "com.ti.srv.generate_3dbowl"

/*! \brief generate_gpulut kernel name
 *  \ingroup group_vision_apps_kernels_srv
 */
#define TIVX_KERNEL_GENERATE_GPULUT_NAME     "com.ti.srv.generate_gpulut"

/*! \brief gl_srv kernel name
 *  \ingroup group_vision_apps_kernels_srv
 */
#define TIVX_KERNEL_GL_SRV_NAME     "com.ti.srv.gl_srv"

/*!
 * \brief Used for the Application to load the srv kernels into the context.
 * \ingroup group_vision_apps_kernels_srv
 */
void tivxSrvLoadKernels(vx_context context);

/*!
 * \brief Used for the Application to unload the srv kernels from the context.
 * \ingroup group_vision_apps_kernels_srv
 */
void tivxSrvUnLoadKernels(vx_context context);

/*!
 * \brief Used to print the performance of the kernels.
 * \ingroup group_vision_apps_kernels_srv
 */
void tivxSrvPrintPerformance(vx_perf_t performance, uint32_t numPixels, const char* testName);

/*!
 * \brief Function to register SRV Kernels on the C6x Target
 * \ingroup group_vision_apps_kernels_srv
 */
void tivxRegisterSrvTargetC6xKernels(void);

/*!
 * \brief Function to un-register SRV Kernels on the C6x Target
 * \ingroup group_vision_apps_kernels_srv
 */
void tivxUnRegisterSrvTargetC6xKernels(void);

/*!
 * \brief Function to register SRV Kernels on the C7x Target
 * \ingroup group_vision_apps_kernels_srv
 */
void tivxRegisterSrvTargetC7xKernels(void);

/*!
 * \brief Function to un-register SRV Kernels on the C7x Target
 * \ingroup group_vision_apps_kernels_srv
 */
void tivxUnRegisterSrvTargetC7xKernels(void);

/*!
 * \brief Function to register SRV Kernels on the gpu Target
 * \ingroup group_vision_apps_kernels_srv
 */
void tivxRegisterSrvTargetGpuKernels(void);

/*!
 * \brief Function to un-register SRV Kernels on the gpu Target
 * \ingroup group_vision_apps_kernels_srv
 */
void tivxUnRegisterSrvTargetGpuKernels(void);

#define SKIP                            4       /*Subsample ratio */
/* Note: skip is hard-coded here to 4 for pipeline efficiency ... this value should match sv->subsampleratio */
#define LDC_LIB_DATA_TYPE               0       /* 0: float, 1:double */
#define LDC_U2D_TABLE_MAX_LENGTH        (1024)  /* maximum u2d table length allowed */
#define LDC_D2U_TABLE_MAX_LENGTH        (1024)  /* maximum d2d table length allowed */
#define LDC_MAX_NUM_CAMERAS             (6)     /* maximum number of cameras allowed */
#define FP_TO_DETECT                 2
#define MAX_INPUT_CAMERAS            4

/*data type*/
#if LDC_LIB_DATA_TYPE==2
typedef int32_t dtype;
#elif LDC_LIB_DATA_TYPE==1
typedef double dtype;
#elif LDC_LIB_DATA_TYPE==0
typedef float dtype;
#endif

/*!
 * \brief The configuration data structure  for representing lens model of a signle camera
 *
 * \ingroup group_vision_apps_kernels_srv
 */
typedef struct {
    dtype distCenterX;         /*!< X coordinate of distortion center */
    dtype distCenterY;         /*!< Y coordinate of distortion center */
    dtype distFocalLength;     /*!< Focal length */
    dtype distFocalLengthInv;  /*!< Inverse Focal length */
    dtype lut_d2u[LDC_D2U_TABLE_MAX_LENGTH]; /*!< Lookup Table from distorted to undisotrted domain */
    int32_t lut_d2u_indMax;    /*!< Maximum index of the d2u look up table */
    dtype lut_d2u_step;        /*!< Step size of the distorted to undistorted lut */
    dtype lut_d2u_stepInv;     /*!< Inverse Step size of the distorted to undistorted lut */
    dtype lut_u2d[LDC_U2D_TABLE_MAX_LENGTH] ; /*!< Lookup Table from undistorted to disotrted domain */
    int32_t lut_u2d_indMax;    /*!< Maximum index of the u2d look up table */
    dtype lut_u2d_step;        /*!< Step size of the undistorted to distorted lut */
    dtype lut_u2d_stepInv;     /*!< Inverse Step size of the undistorted to distorted lut */
} LensDistortionCorrection;


/*!
 * \brief The point detect structure is the configuration data structure used by the TIVX_KERNEL_POINT_DETECT kernel.
 *
 * \ingroup group_vision_apps_kernels_srv
 */
typedef struct  {
    vx_uint8          enableSV_ACDetect; /*!< Dilate / Erode function; 0: Disabled (Default); 1: Enabled         */
    vx_uint8          thresholdMode;     /*!< Bias for binarization, [Range (0 - 4)] ; Default =0                */
    vx_uint8          windowMode;        /*!< Window offset selection, 0:Default offset; 1 ; custom offset       */
    vx_uint8          Ransac;            /*!< 0: Disable Ransac (Default); 1: Enable Ransac                      */

    vx_int16          SVROIWidth;           /*!< Width of the ROI for point detect */
    vx_int16          SVROIHeight;          /*!< Height of the ROI for point detect */
    vx_int16          binarizeOffset;       /*!< binarize offset  for point detect */
    vx_int16          borderOffset;         /*!< border offset  for point detect */
    vx_int16          smallestCenter;       /*!< Size of smallest center to be detected */
    vx_int16          largestCenter;        /*!< Size of largest center to be detected */
    vx_int16          maxWinWidth;          /*!< Maximum window width  */
    vx_int16          maxWinHeight;         /*!< Maximum window height  */
    vx_int16          maxBandLen;           /*!< Maximum band length  */
    vx_int16          minBandLen;           /*!< Miinimum band length  */
    vx_int16          minSampleInCluster;   /*!< Miinimum sample cluser size  */
    vx_int16          firstROITop;          /*!< Top position of first ROI  */
    vx_int16          firstROIBottom;       /*!< Bottom position of first ROI  */
    vx_int16          firstROILeft;         /*!< Left position of first ROI  */
    vx_int16          firstROIRight;        /*!< Right position of first ROI  */
    vx_int16          secondROITop;         /*!< Top position of second ROI  */
    vx_int16          secondROIBottom;      /*!< Bottom position of second ROI  */
    vx_int16          secondROILeft;        /*!< Left position of second ROI  */
    vx_int16          secondROIRight;       /*!< Right position of second ROI  */

    vx_int16          camera_id;            /*!< Camera Id 0:Front; 1:Right; 2:Back; 3:Left  */


}  svPointDetect_t;

/*!
 * \brief The svLdcLut_t structure is the configuration data structure for lens model  used by the followig kernel(s)
 *  TIVX_KERNEL_POINT_DETECT, TIVX_KERNEL_POSE_ESTIMATION, TIVX_KERNEL_GENERATE_GPULUT
 *
 * \ingroup group_vision_apps_kernels_srv
 */
typedef struct  {
    LensDistortionCorrection   ldc[LDC_MAX_NUM_CAMERAS];  /*!< Lens parameter/camera for all the cameras in the system  */
}  svLdcLut_t;



/*!
 * \brief The svACDetectStructFinalCorner_t structure is the configuration data structure containing detected point coordinates   used by the followig kernel(s)
 *  TIVX_KERNEL_POINT_DETECT, TIVX_KERNEL_POSE_ESTIMATION
 *
 * \ingroup group_vision_apps_kernels_srv
 */

/* Each camera used FP_TO_DETECT * 8 entries                 */
/* The 8 entries are stored in y,x format for each 4 corners */
/* Starting from the top left corner                         */
typedef struct  {
    vx_int32 finalCorners [FP_TO_DETECT][8];   /*!< Cordinates of detected corners, 4 points/chart, 2 coordinates (x,y) / point           */
    vx_int16 numFPDetected;                    /*!< Number of points detected                                                             */

}  svACDetectStructFinalCorner_t;



/*!
 * \brief The svPoseEstimation_t structure is the configuration data structure used by the TIVX_KERNEL_POSE_ESTIMATION kernel.
 *
 * \ingroup group_vision_apps_kernels_srv
 */
typedef struct  {
    vx_uint8          Ransac;                  /*!< 0: Disable Ransac (Default); 1: Enable Ransac                      */
    vx_uint8          SingleChartPose;         /*!< 0: Create Pose from both squares (Default) ; 1:  Create pose from one square */
    vx_uint8          numCameras;              /*!< Number of cameras in the system, typically 4   */
    vx_uint8          rsv[1];                  /*!< rsv mem for alignment */
    vx_int8           inChartPos [164];        /*!< Physical distance of calibration charts        */

}  svPoseEstimation_t;


/*!
 * \brief The svACCalmatStruct_t structure is the structure containing calibration data used by the following kernel(s)
 *  TIVX_KERNEL_POSE_ESTIMATION, TIVX_KERNEL_GENERATE_3DBOWL, TIVX_KERNEL_GENERATE_GPULUT
 *
 * \ingroup group_vision_apps_kernels_srv
 */
typedef struct
{

    vx_int32     outcalmat[12*MAX_INPUT_CAMERAS];            /*!< Calibration data (Pose) in fixed point notation                    */
    vx_float32   scaled_outcalmat[12*MAX_INPUT_CAMERAS];     /*!< Calibration data (Pose) in floating point notation after scaling   */

}  svACCalmatStruct_t;


/*!
 * \brief The svGeometric_t structure is the configuration data structure used by the TIVX_KERNEL_GENERATE_3DBOWL kernel.
 *
 * \ingroup group_vision_apps_kernels_srv
 */
typedef struct  {
    vx_int32          offsetXleft;         /*!< Bowl offset for left surface                     */
    vx_int32          offsetXright;        /*!< Bowl offset for right surface                    */
    vx_int32          offsetYfront;        /*!< Bowl offset for front surface                    */
    vx_int32          offsetYback;         /*!< Bowl offset for back  surface                    */

}  svGeometric_t;


/*!
 * \brief The svGpuLutGen_t structure is the configuration structure used by the following kernel(s)
 *  TIVX_KERNEL_GENERATE_3DBOWL, TIVX_KERNEL_GENERATE_GPULUT
 *
 * \ingroup group_vision_apps_kernels_srv
 */
typedef struct  {
    vx_int16          SVInCamFrmHeight;          /*!< Height of input image          */
    vx_int16          SVInCamFrmWidth;           /*!< Width of input image           */
    vx_int16          SVOutDisplayHeight;        /*!< Height of Output image         */
    vx_int16          SVOutDisplayWidth;         /*!< Width of Output image          */
    vx_uint8          numCameras;                /*!< Number of cameras in the system, typically 4  */
    vx_int8           subsampleratio;            /*!< Mesh subsampling factor, typically set to 4  */
    vx_int8           useWideBowl;               /*!< 0:Use default bowl  1: Use flat bowl         */

}  svGpuLutGen_t;

/*********************************
 *      GPU SRV STRUCTURES
 *********************************/
/*!
 * \brief The data structure used by the SGX SRV kernel for SRV creation.
 *
 * \ingroup group_vision_apps_kernels_srv
 */
typedef struct
{
    uint32_t cam_bpp;          /*!< Flag for cam_bpp */

} tivx_srv_params_t;

#ifdef __cplusplus
}
#endif

#endif /* TIVX_SRV_KERNELS_H_ */


