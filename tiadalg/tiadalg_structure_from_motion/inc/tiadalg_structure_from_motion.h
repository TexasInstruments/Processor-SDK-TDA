/*
* module name : Structure From Motion
*
* module descripton : Generates sparse 3D points from optical flow information for camera captured images
*
*/
/*

Copyright (c) 2009-2021 Texas Instruments Incorporated

All rights reserved not granted herein.

Limited License.

Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive license under copyrights and patents it now or hereafter owns or controls to make, have made, use, import, offer to sell and sell ("Utilize") this software subject to the terms herein.  With respect to the foregoing patent license, such license is granted  solely to the extent that any such patent is necessary to Utilize the software alone.  The patent license shall not apply to any combinations which include this software, other than combinations with devices manufactured by or for TI ("TI Devices").  No hardware patent is licensed hereunder.

Redistributions must preserve existing copyright notices and reproduce this license (including the above copyright notice and the disclaimer and (if applicable) source code license limitations below) in the documentation and/or other materials provided with the distribution

Redistribution and use in binary form, without modification, are permitted provided that the following conditions are met:

*       No reverse engineering, decompilation, or disassembly of this software is permitted with respect to any software provided in binary form.

*       any redistribution and use are licensed by TI for use only with TI Devices.

*       Nothing shall obligate TI to provide you with source code for the software licensed and provided to you in object code.

If software source code is provided to you, modification and redistribution of the source code are permitted provided that the following conditions are met:

*       any redistribution and use of the source code, including any resulting derivative works, are licensed by TI for use only with TI Devices.

*       any redistribution and use of any object code compiled from the source code and any resulting derivative works, are licensed by TI for use only with TI Devices.

Neither the name of Texas Instruments Incorporated nor the names of its suppliers may be used to endorse or promote products derived from this software without specific prior written permission.

DISCLAIMER.

THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
/**
    @file       tiadalg_structure_from_motion.h

    @brief      This file defines ivision interface for Structure From Motion (SFM)
                Algorithm. This module expects optical flow information along with camera
                pose to generates sparse 3D points. Input image data is needed for visualization
                purpose only.

    @version 1.0 (April 2021) : Base version.
 */

/** @ingroup    ti_ivision_TI_SFM*/
/*@{*/

#ifndef ISFM_TI_H
#define ISFM_TI_H

#include <ivision.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Size of the Camera Extrinsic parameters consisting of
 * rotation and translation parameters.
 */
#define SFM_TI_CAMERA_EXTRENSIC_PARAM_SIZE  (12)

/*
 * Size of Camera Intrinsic parameters.
 */
#define SFM_TI_CAMERA_INTRINSIC_PARAM_SIZE  (9)

/*
 * Maximum number of past frames for which a key point is tracked.
*/
#define MAX_NUM_FRAMES_TO_TRACK  (6)

/**
 * @brief The unique constant function table for SFM
 */
extern const IVISION_Fxns SFM_TI_VISION_FXNS;

/**
  @brief User provides most of the information through IVISION buffers
         during process call.
         Below enums define the purpose of each input buffers.

  @param SFM_TI_IN_BUFDESC_IN_DOF_BUFFER: Input optical flow gnerated from DOF accelerator

  @param SFM_TI_IN_BUFDESC_IN_LUMA_IMG_BUFFER: Input image buffer

*/
typedef enum
{
  SFM_TI_IN_BUFDESC_IN_DOF_BUFFER     = 0,
  SFM_TI_IN_BUFDESC_IN_LUMA_IMG_BUFFER     = 1,
  SFM_TI_IN_BUFDESC_TOTAL
} SFM_TI_InBufOrder ;


/**
  @brief Process call provides most of the information through IVISION buffers
         during process call. Below enums define the purpose of each out buffer

  @param SFM_TI_OUT_BUFDESC_FEATURE_PLANES: This  buffer is filled up by SFM algorithm
         with a list of estimated 3D points location and their corresponding image pixel location.
         Output buffer is treated as continuous array of objects of the structure /c SFM_TI_output.
         Where each object corresponds to information related one reconstructed 3-D point from one
         track information /c SFM_TI_trackInfo. Output buffer is not image buffer, it is set
         of objects of 'SFM_TI_output' placed in linear memory. Since number of output buffer
         is not known at the time of process call, hence out buf should be allocated assuming maximum
         possible number of 3D output points. Maximum number of out 3D points can be equal to maxNumTracks*6.
         Hence user should set outBufs :: bufDesc[SFM_TI_OUT_BUFDESC_FEATURE_PLANES] :: bufPlanes[0]
         ::width greater than or equal to (maxNumTracks*6). maxNumTracks has to be set at the time of
         create API call.

   @param SFM_TI_OUT_BUFDESC_LUMA_PTCLD_BUFFER: Luma output buffer with point cloud visualization

   @param SFM_TI_OUT_BUFDESC_CHROMA_PTCLD_BUFFER: Chroma output buffer with point cloud visualization

   @param SFM_TI_OUT_BUFDESC_LUMA_OCPGD_BUFFER: Luma output buffer for occupancy grid visualization

   @param SFM_TI_OUT_BUFDESC_CHROMA_OCPGD_BUFFER: Chroma output buffer for occupancy grid visualization
  */

typedef enum
{
  SFM_TI_OUT_BUFDESC_FEATURE_PLANES    = 0,
  SFM_TI_OUT_BUFDESC_LUMA_PTCLD_BUFFER   = 1,
  SFM_TI_OUT_BUFDESC_CHROMA_PTCLD_BUFFER = 2,
  SFM_TI_OUT_BUFDESC_LUMA_OCPGD_BUFFER   = 3,
  SFM_TI_OUT_BUFDESC_CHROMA_OCPGD_BUFFER = 4,
  SFM_TI_OUT_BUFDESC_TOTAL
} SFM_TI_OutBufOrder ;


/**
  @brief Defines the method to calculate/estimate Fundamental Matrix. It is recommended
         to use SFM_TI_FMAT_FROM_RT_PARAMS to save computation cycle.

  SFM_TI_FMAT_8POINT_RANSAC  : Estimate using 8 point algorithm with RANSAC iteration.
                               Not varified in this version.

  SFM_TI_FMAT_FROM_RT_PARAMS : Calculate it using available rotation and translation
                               parameters provided through extrinsic parameters
                               SFM_TI_InArgs :: camExtPrm

*/

typedef enum
{

  SFM_TI_FMAT_8POINT_RANSAC           = 0,
  SFM_TI_FMAT_FROM_RT_PARAMS          = 1,
  SFM_TI_FMAT_EXT_PRM_NORM_DEFAULT    = SFM_TI_FMAT_FROM_RT_PARAMS

} SFM_TI_FmatrixCalcType;

/**
  @struct SFM_TI_CreateParams
  @brief  This structure contains all the parameters which controls
          SFM algorithm at create time.

  @param  visionParams: Common parameters for all ivison based modules.

  @param  imgWidth: Image width in pixels. Should be inlined with input and output buffers.

  @param  imgHeight: Image height in pixels. Should be inlined with input and output buffers.

  @param  maxNumTracks: Maximum Number of tracks allowed.

  @param  cameraIntParams
          Camera Intrinsic Parameters. Format of intrinsic parameter
          is [ax 0.0 x0 0 ay y0 0 0 1.0]. Where ax and ay are
          standard scaling parameter in x and y direction. Whereas
          (x0, y0) is the principal point , where optic axis
          intersects the image plane.

  @param  udmaDrvObj: Udma driver handle. All the addresses given to algorithms are assumed to be virtual addresses.
          If udma driver is initialized with virtual to physical function pointer initPrms.virtToPhyFxn
          then the same function will be used for this conversion in algorithm whereever it is needed.

  @param  flowCtrl: Control for selecting different flows. Applicable only for advanced user. 0--> enables
          optimized flow, 1 enabled natural c flow.

  @param  enAlgPtCldVis: Enables point cloud visualization.

  @param  ocpGrdSizeInPixels: Occupancy grid size in pixels.

  @param  enAlgOcpGrdVis: Enables occupancy grid visualization.

  @param  imgColorFmt: Input image format. Not used in current version, all input and output
          images are assumed in YUV 420 nv12 format.

  @param  rsvd1: For future expansion. Should be set as zero

  @param  rsvd2: For future expansion. Should be set as zero


*/
typedef struct
{
  IVISION_Params visionParams;

  uint16_t imgWidth;
  uint16_t imgHeight;
  uint16_t keyPointStep;

  uint16_t maxNumKeyPoints   ;
  uint16_t maxNumTracks      ;

  float    camIntPrm[SFM_TI_CAMERA_INTRINSIC_PARAM_SIZE];

  /* Note : .*/
  void * udmaDrvObj          ;

  int32_t flowCtrl;

  /*Enables dumping out of visualization buffer (SFM_TI_OUT_BUFDESC_LUMA_PTCLD_BUFFER) by algorithm itself. */
  int32_t enAlgPtCldVis      ;

  /*Sqaure occupancy grid width or height in pixels*/
  int32_t ocpGrdSizeInPixels ;

  /*Enables dumping out of visualization buffer (SFM_TI_OUT_BUFDESC_LUMA_OCPGRD_BUFFER) by algorithm itself. */
  int32_t enAlgOcpGrdVis     ;

  /*input and output image color format. 0 for yuv nv12, and 1 for BGR */
  int32_t imgColorFmt        ;

  uint16_t rsvd1             ;
  uint16_t rsvd2             ;
} SFM_TI_CreateParams;

/**

  @struct SFM_TI_InArgs
  @brief  This structure contains all the parameters which are given as an
          input to OD algorithm at frame level

  @param  iVisionInArgs: Common InArgs  for all ivison based modules

  @param  flowConfThr: Optical flow confidence threshold

  @param  flowInvalidPadX: Number of pixels to be ignored in optical buffer in x direction

  @param  flowInvalidPadY: Number of pixels to be ignored in optical buffer in y direction

  @param  camExtPrm : Camera Extrinsic Parameters. Format of camera extrinsic parameters is [ r00
          r01 r02 t0 r10 r11 r12 t1 r20 r21 r22 t2], where rij are the rotation parameter,
          and ti are the translation parameter.
          If this pointer is provided as NULL, then these parameters are computed internally.

  @param  fMatCalcMethod: This controls how Fundamental Matrix is estimated/calculated.
          There is possibility of estimating Fundamental matrix using
          feature point correspondence between two frames, whereas there
          could be another method to calculate using rotation and translation
          parameters provided.
          Refer @SFM_TI_FmatrixCalcType for various supported values and
          default values.

  @param  fMatrixPrunEn: Control to enable or disable Fundamental matrix based pruning of
          the input tracks. User should set this as '1' to enable this
          pruning.

  @param  fMatrixInTh: Inlier threshold used in Fundamental matrix based pruning.

  @param  maxRansacItr:
          Maximum number of RANSAC iteration in F matrix iteration. Its
          Supported value is [0, 200]. User should set default value
          for this as 90. When value zero is set then algorithm does
          optimal number of iterations.

  @param  maxTriangItr:
          Maximum number of iteration in triangulation. Supported value for
          this is [1,10]. User should set default value which is 4.

  @param  pointPruneAngle:
          Angle threshold used for angle based pruning. Supported Values is
          [0.0 10.0]

  @param  staticFrmTh:
          Threshold to determine wheather current frame is static or not. Norm of
          the current frame camera translation  with respect to previous frame should
          be less than this threshold to flag current frame as static frame. Not used in
          current version.

  @param  reset:
          resets the algorithm from this frame onward. Past tracks are ignored and
          it will get built up again from this frame onward.

  @param  reserved0
          Reserved 32-bit field. Must be set to 0 for normal operation.

  @param  reserved1
          Reserved 32-bit field. Must be set to 0 for normal operation.

*/
typedef struct
{
  IVISION_InArgs   iVisionInArgs      ;

  int32_t          flowConfThr        ;
  int32_t          flowInvalidPadX    ;
  int32_t          flowInvalidPadY    ;

  float            camExtPrm[SFM_TI_CAMERA_EXTRENSIC_PARAM_SIZE];
  uint32_t         fMatrixCalcMethod  ;
  uint32_t         fMatrixPrunEn      ;
  uint32_t         fMatrixInTh        ;
  uint32_t         maxRansacItr       ;
  uint32_t         maxTriangItr       ;
  float            pointPruneAngle    ;
  float            staticFrmTh        ;
  int32_t          reset              ;
  uint32_t         reserved0          ;
  uint32_t         reserved1          ;
} SFM_TI_InArgs;

/**

  @struct SFM_TI_Stats

  @brief  This structure reports statistics of running SFM

  @param  numIterFMat
          Number of actual RANSAC iteration in F matrix estimation

  @param  numInlierFmat
          Number of inlier determined by the pruning based
          on F matrix

  @param  numCur3DPnts
          Number of 3D points generated from all the track info-
          rmation provided in current process call. This may not
          be the actual number of output 3D points.

  @param  isStatic
          if this is '1' then current frame has been declared as static
          frame.

*/

typedef struct
{
  uint32_t numIterFMat        ;
  uint32_t numInlierFmat      ;
  uint32_t numCur3DPnts       ;
  uint8_t  isStatic           ;
  size_t rsvd1                ;
  size_t rsvd2                ;
} SFM_TI_Stats;


/**
  @struct SFM_TI_OutArgs

  @brief  This structure contains all the parameters
          which are given as output.

  @param  iVisionOutArgs
          Common outArgs for all ivison based modules

  @param  sfmStats
          Stats structure of type SFM_TI_Stats for SFM

  @param  outNumPoints
          Total number of output points. should be used to indicate
      maximum number of valid output entries in out buffer /c SFM_TI_OUT_BUFDESC_FEATURE_PLANES.
*/
typedef struct
{
  IVISION_OutArgs    iVisionOutArgs;
  SFM_TI_Stats       sfmStats      ;
  uint32_t           outNumPoints  ;
} SFM_TI_OutArgs;

/**
  @struct SFM_TI_output

  @brief  This structure defines the format of output buffer
          given out from algorithm.

  @param  point2dX
          x co-ordinate of 2d point in image plane

  @param  point2dY
          y co-ordinate of 2d point in image plane

  @param  point3dX
          x co-ordinate of 3d point in current frame camera
          coordinate system

  @param  point3dY
          y co-ordinate of 3d point in current frame camera
          coordinate system

  @param  point3dZ
          z co-ordinate of 3d point in current frame camera
          coordinate system

  @param  frmId
          inArgs input buffer Id of the frame from which this 3D
          point was generated.

  @param  trackAge
          Number of key points in the track from which this output
          point got reconstructed

  @param  trackId
          Id of the track in the frame SFM_TI_output :: frmId from
          which this 3D point got generated.

*/
typedef struct
{
  float    point2dX;
  float    point2dY;
  float    point3dX;
  float    point3dY;
  float    point3dZ;
  uint16_t trackAge;
  uint16_t trackId;

} SFM_TI_output;

/*@}*/
/* ti_ivision_TI_SFM */

#ifdef __cplusplus
}
#endif

#endif /* ISFM_TI_H */
/*==========================================================================*/
/*     END of the FILE                                                      */
/*==========================================================================*/


