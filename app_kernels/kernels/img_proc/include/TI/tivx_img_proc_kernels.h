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

#ifndef TIVX_IMG_PROC_KERNELS_H_
#define TIVX_IMG_PROC_KERNELS_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>
#include "itidl_ti.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup group_vision_apps_kernels_img_proc TIVX Kernels for Image Pre/Post Processing
 *
 * \brief This section documents the kernels defined for Image Pre/Post Processing
 *
 * \ingroup group_vision_apps_kernels
 */
/*!
 * \file
 * \brief The list of supported kernels in this kernel extension.
 */

/*! \brief OpenVX module name
 * \ingroup group_vision_apps_kernels_img_proc
 */
#define TIVX_MODULE_NAME_IMG_PROC    "img_proc"

/*! \brief Kernel Name: Image Pre Processing
 *  \see group_vision_apps_kernels_img_proc
 */
#define TIVX_KERNEL_IMG_PREPROCESS_NAME     "com.ti.img_proc.img.preprocess"

/*! \brief Kernel Name: Image Pre Processing-4D
 *  \see group_vision_apps_kernels_img_proc
 */
#define TIVX_KERNEL_DL_PRE_PROC_ARMV8_4D_NAME          "com.ti.img_proc.dev.dl.pre.proc.armv8"

/*! \brief Kernel Name: OC Pre Processing
 *  \see group_vision_apps_kernels_img_proc
 */
#define TIVX_KERNEL_OC_PRE_PROC_NAME     "com.ti.img_proc.oc.pre.proc"

/*! \brief Kernel Name: OC Post Processing
 *  \see group_vision_apps_kernels_img_proc
 */
#define TIVX_KERNEL_OC_POST_PROC_NAME     "com.ti.img_proc.oc.post.proc"

/*! \brief Kernel Name: Parking Spot detection Post Processing
 *  \see group_vision_apps_kernels_img_proc
 */
#define TIVX_KERNEL_OD_POSTPROCESS_NAME     "com.ti.img_proc.od.postprocess"

/*! \brief Kernel Name: Dense Optical Flow plane seperation
 *  \see group_vision_apps_kernels_img_proc
 */
#define TIVX_KERNEL_DOF_PLANE_SEPERATION_NAME     "com.ti.img_proc.dof.plane.seperation"

/*! \brief Kernel Name: Pixel visualization
 *  \see group_vision_apps_kernels_img_proc
 */
#define TIVX_KERNEL_PIXEL_VISUALIZATION_NAME     "com.ti.img_proc.pixel.visualization"

/*! \brief Kernel Name: Ego Localization
 *  \see group_vision_apps_kernels_img_proc
 */
#define TIVX_KERNEL_VISUAL_LOCALIZATION_NAME     "com.ti.img_proc.visual.localization"

/*! \brief Kernel Name: Pose Visualization
 *  \see group_vision_apps_kernels_img_proc
 */
#define TIVX_KERNEL_POSE_VISUALIZATION_NAME     "com.ti.img_proc.pose.visualization"

/*! \brief Kernel Name: Image Mosaic
 *  \see group_vision_apps_kernels_img_proc
 */
#define TIVX_KERNEL_IMG_MOSAIC_NAME             "com.ti.img_proc.img.mosaic"


/*! \brief Kernel Name: Draw Keypoint Detections
 *  \see group_vision_apps_kernels_img_proc
 */
#define TIVX_KERNEL_DRAW_KEYPOINT_DETECTIONS_NAME     "com.ti.img_proc.od.draw.keypoint.detections"

/*! \brief Kernel Name: Draw Box Detections
 *  \see group_vision_apps_kernels_img_proc
 */
#define TIVX_KERNEL_DRAW_BOX_DETECTIONS_NAME     "com.ti.img_proc.od.draw.box.detections"

/*! \brief Kernel Name: Draw BEV Box Detections
 *  \see group_vision_apps_kernels_img_proc
 */
#define TIVX_KERNEL_DRAW_BEV_BOX_DETECTIONS_NAME     "com.ti.img_proc.od.draw.bev.box.detections"


/*! \brief Kernel Name: Draw BEV Box Detections
 *  \see group_vision_apps_kernels_img_proc
 */
#define TIVX_KERNEL_DRAW_BEV_CAM_BOX_DETECTIONS_NAME     "com.ti.img_proc.od.draw.cam.bev.box.detections"

/*! \brief Kernel Name: Image histogram
 *  \see group_vision_apps_kernels_img_hist
 */
#define TIVX_KERNEL_IMG_HIST_NAME     "com.ti.img_proc.img.hist"

/*! \brief Kernel Name: Image histogram
 *  \see group_vision_apps_kernels_sfm
 */
#define TIVX_KERNEL_SFM_NAME     "com.ti.img_proc.sfm"

/*! \brief Kernel Name: DL Pre processing
 *  \see group_vision_apps_kernels_dl_pre_proc
 */
#define TIVX_KERNEL_DL_PRE_PROC_NAME     "com.ti.img_proc.dl.pre.proc"

/*! \brief Kernel Name: DL color blend
 *  \see group_vision_apps_kernels_dl_color_blend
 */
#define TIVX_KERNEL_DL_COLOR_BLEND_NAME     "com.ti.img_proc.dl.color.blend"

/*! \brief Kernel Name: DL draw box
 *  \see group_vision_apps_kernels_dl_draw_box
 */
#define TIVX_KERNEL_DL_DRAW_BOX_NAME     "com.ti.img_proc.dl.draw.box"

/*! \brief Kernel Name: DL color convert
 *  \see group_vision_apps_kernels_dl_color_convert
 */
#define TIVX_KERNEL_DL_COLOR_CONVERT_NAME     "com.ti.img_proc.dl.color.convert"

/*! \brief Kernel Name: DL Pre processing Armv8
 *  \see group_app_tiovx_kernels_dl_pre_proc_armv8
 */
#define TIVX_KERNEL_DL_PRE_PROC_ARMV8_NAME          "com.ti.img_proc.dl.pre.proc.armv8"

/*! \brief Kernel Name: DL post process
 *  \see group_app_tiovx_kernels_dl_post_process
 */
#define TIVX_KERNEL_DL_POST_PROC_NAME               "com.ti.img_proc.dl.post.proc"

/*! \brief Kernel Name: DL color convert Armv8
 *  \see group_vision_apps_kernels_dl_color_convert_armv8
 */
#define TIVX_KERNEL_DL_COLOR_CONVERT_ARMV8_NAME     "com.ti.img_proc.dl.color.convert.armv8"

/** \brief Image Add kernel name */
#define TIVX_C7X_KERNEL_IMG_ADD_NAME                "com.ti.img_proc.c7x.img_add"

/*!
 * \brief
 * \ingroup group_vision_apps_kernels_img_proc
 */
/* Number of maximum output tensor for visualization purpose. motion/semantic/depth*/
#define TIVX_PIXEL_VIZ_MAX_TENSOR  (3U)

/*For semantic segmentation supported maximum class is 256 including background*/
/*For depth tensor output is U08, which makes for each value one different color has to be assigned, so each value is treated as different class*/
#define TIVX_PIXEL_VIZ_MAX_CLASS  (256U)

#if defined(SOC_TDA54)
#define TIVX_IMG_MOSAIC_MAX_INPUTS    (24U)
#elif defined(SOC_J784S4) || defined(SOC_J742S2)
#define TIVX_IMG_MOSAIC_MAX_INPUTS    (16U)
#else
#define TIVX_IMG_MOSAIC_MAX_INPUTS    (8U)
#endif

#define TIVX_IMG_MOSAIC_MAX_CHANNELS  (4U)
#define TIVX_IMG_MOSAIC_MAX_WINDOWS   (TIVX_IMG_MOSAIC_MAX_INPUTS * TIVX_IMG_MOSAIC_MAX_CHANNELS)

/* Maximum number of classes drawn on the image */
#define TIVX_DRAW_KEYPOINT_DETECTIONS_MAX_CLASSES  (16)
#define TIVX_DRAW_BOX_DETECTIONS_MAX_CLASSES       (128)

#define TIVX_OC_MAX_CLASSES  (10)
#define TIVX_MAX_PAST_POSES (16)

/* Send this control command to reset the background image in app_tild_vl */
#define TIVX_IMG_PROC_POSE_VIZ_RESET_BACKGROUND    (0xF00F)
#define TIVX_IMG_PROC_VIZ_LOC_RESET_POSE           (0xFFEF)

/* Supported tensor formats in dl-pre-proc */
#define TIVX_DL_PRE_PROC_TENSOR_FORMAT_RGB   (0)
#define TIVX_DL_PRE_PROC_TENSOR_FORMAT_BGR   (1)

/* Supported channel ordering in dl-pre-proc */
#define TIVX_DL_PRE_PROC_CHANNEL_ORDER_NCHW   (0)
#define TIVX_DL_PRE_PROC_CHANNEL_ORDER_NHWC   (1)

/* Supported crop indexes in dl-pre-proc */
#define TIVX_DL_PRE_PROC_IMAGE_CROP_TOP     (0)
#define TIVX_DL_PRE_PROC_IMAGE_CROP_BOTTOM  (1)
#define TIVX_DL_PRE_PROC_IMAGE_CROP_LEFT    (2)
#define TIVX_DL_PRE_PROC_IMAGE_CROP_RIGHT   (3)

/* Macros to indicate max outputs, classes and colors in dl-draw-box */
#define TIVX_DL_DRAW_BOX_MAX_OUTPUTS     (4U)
#define TIVX_DL_DRAW_BOX_MAX_CLASSES     (256U)
#define TIVX_DL_DRAW_BOX_MAX_COLORS      (3U)
#define TIVX_DL_DRAW_BOX_MAX_CLASS_NAME  (64U)
#define TIVX_DL_DRAW_BOX_MAX_RECTANGLES  (256U)

/* Macros to indicate max outputs, classes and colors in dl-color-blend */
#define TIVX_DL_COLOR_BLEND_MAX_OUTPUTS    (4U)
#define TIVX_DL_COLOR_BLEND_MAX_CLASSES    (256U)
#define TIVX_DL_COLOR_BLEND_MAX_COLORS     (3U)

/* Macros to indicate max outputs, classes and colors in dl-post-proc */
#define TIVX_DL_BEV_POST_PROC_CLASSIFICATION_TASK_TYPE  (0U)
#define TIVX_DL_BEV_POST_PROC_DETECTION_TASK_TYPE       (1U)
#define TIVX_DL_BEV_POST_PROC_SEGMENTATION_TASK_TYPE    (2U)
#define TIVX_DL_BEV_POST_PROC_OC_MAX_CLASSES            (10)
#define TIVX_DL_BEV_POST_PROC_MAX_NUM_CLASSNAMES        (1000)
#define TIVX_DL_BEV_POST_PROC_MAX_SIZE_CLASSNAME        (64)

/* Supported crop indexes in dl-pre-proc */
#define TIVX_DL_PRE_PROC_ARMV8_IMAGE_CROP_TOP     (0)
#define TIVX_DL_PRE_PROC_ARMV8_IMAGE_CROP_BOTTOM  (1)
#define TIVX_DL_PRE_PROC_ARMV8_IMAGE_CROP_LEFT    (2)
#define TIVX_DL_PRE_PROC_ARMV8_IMAGE_CROP_RIGHT   (3)

/* Macros to indicate max outputs, classes and colors in dl-post-proc */
#define TIVX_DL_POST_PROC_CLASSIFICATION_TASK_TYPE  (0U)
#define TIVX_DL_POST_PROC_DETECTION_TASK_TYPE       (1U)
#define TIVX_DL_POST_PROC_SEGMENTATION_TASK_TYPE    (2U)
#define TIVX_DL_POST_PROC_OC_MAX_CLASSES            (10)
#define TIVX_DL_POST_PROC_MAX_NUM_CLASSNAMES        (1000)
#define TIVX_DL_POST_PROC_MAX_SIZE_CLASSNAME        (64)

/** \brief Index of input image 1 in parameter list */
#define APP_C7X_IMG_ADD_IN0_IMG_IDX   (0u)
/** \brief Index of input image 2 in parameter list */
#define APP_C7X_IMG_ADD_IN1_IMG_IDX   (1u)
/** \brief Index of output image in parameter list */
#define APP_C7X_IMG_ADD_OUT0_IMG_IDX  (2u)
/** \brief Total number of parameters for this function */
#define APP_C7X_IMG_ADD_MAX_PARAMS    (3u)

/*!
 * \brief
 * \ingroup group_vision_apps_kernels_img_proc
 */

typedef struct{
  /**Mean pixel for each output plane which needs to be subtracted*/
  vx_float32 mean_pixel[3];

  /**Scale value for each output plane which needs to be multiplied*/
  vx_float32 scale_val[3];

  /**Pad value for output planes for each side of image*/
  vx_int32 pad_pixel[4];

  /** To enable 8 bit or 16 bit input/output tensor for tidl. 0 means 8 bit, 1 means 16 bit*/
  vx_int32 tidl_8bit_16bit_flag;

  /** Falg to indicate input is RGB or YUV. 0 means RGB otherwise YUV*/
  vx_int32 ip_rgb_or_yuv;

  /** color conversion flag for seperating the plane as peer the model requirement*/
  vx_int32 color_conv_flag;

  /** Number of times to clear the output buffer*/
  vx_uint32 clear_count;

  /** Skip processing */
  vx_int32 skip_flag;

}tivxImgPreProcParams;

typedef struct {

  /** Skip processing */
  vx_int32 skip_flag;

  /* Scale values to be applied per channel in the range of 0.0 to 1.0 */
  vx_float32 scale[3];

  /* Mean value per channel to be subtracted, range depends on channel bit-depth */
  vx_float32 mean[3];

  /* Channel ordering, 0-NCHW, 1-NHWC */
  vx_int32 channel_order;

  /* Tensor format, 0-RGB, 1-BGR */
  vx_int32 tensor_format;

  /* Crop values to be applied, 0-Top, 1-Bottom, 2-Right, 3-Left */
  vx_int32 crop[4];

  vx_int32 channel_pitch;

}tivxDLPreProcArmv8Params4D;
/*!
 * \brief DL Pre processing to be used with DL-RT
 * \ingroup group_app_tiovx_kernels_img_proc
 */
typedef struct {

  /** Skip processing */
  vx_int32 skip_flag;

  /* Scale values to be applied per channel in the range of 0.0 to 1.0 */
  vx_float32 scale[3];

  /* Mean value per channel to be subtracted, range depends on channel bit-depth */
  vx_float32 mean[3];

  /* Channel ordering, 0-NCHW, 1-NHWC */
  vx_int32 channel_order;

  /* Tensor format, 0-RGB, 1-BGR */
  vx_int32 tensor_format;

  /* Crop values to be applied, 0-Top, 1-Bottom, 2-Right, 3-Left */
  vx_int32 crop[4];

  vx_uint32 channel_pitch;

}tivxDLPreProcArmv8Params;

typedef struct{

    /** Classification tensor I/O params */
    sTIDL_IOBufDesc_t *ioBufDesc;

    /** Number of top results to consider */
    vx_int32 num_top_results;

    /** Extract labelOffset from param.yaml */
    vx_int32 labelOffset;

    /** Classnames corresponding to class indexs */
    char (*classnames)[TIVX_DL_POST_PROC_MAX_SIZE_CLASSNAME];

}oc_params;

typedef struct{

    /** Detection tensor I/O params */
    sTIDL_IOBufDesc_t *ioBufDesc;

    /** User config visualization threshold */
    vx_float32 viz_th;

    /** Amount by which index is offset */
    vx_int32 labelIndexOffset;

    /** Extract labelOffset from param.yaml using labelIndexOffset */
    vx_int32 *labelOffset;

    /** Classnames corresponding to class indexs */
    char (*classnames)[TIVX_DL_POST_PROC_MAX_SIZE_CLASSNAME];

    /** formatter */
    vx_int32 formatter[6];

    /** scaleX */
    vx_float32 scaleX;

    /** scaleY */
    vx_float32 scaleY;

}od_params;

typedef struct{

    /** Detection tensor I/O params */
    sTIDL_IOBufDesc_t *ioBufDesc;

    /** Input width for tensor, extract it from tensor format, NCHW or NHWC */
    vx_int32 inDataWidth;

    /** Input height for tensor */
    vx_int32 inDataHeight;

    /** Alpha determines the extent of blending, this is extracted from params.yaml of model */
    vx_float32 alpha;

    /** YUV Color Map based on class is. */
    uint8_t **YUVColorMap;

    /** Max number of classes the color map can support. If class if
      * is more than max supported class, black is overlayed by default.
      */
    uint8_t MaxColorClass;

}ss_params;

/*!
 * \brief
 * \ingroup group_vision_apps_kernels_img_proc
 */

typedef struct{

  /** Detection tensor I/O params */
  sTIDL_IOBufDesc_t ioBufDesc;

}tivxOCPreProcParams;


/*!
 * \brief
 * \ingroup group_vision_apps_kernels_img_proc
 */

typedef struct{

  /** Detection tensor I/O params */
  sTIDL_IOBufDesc_t ioBufDesc;
  /** Number of top results to consider */
  vx_int32 num_top_results;

}tivxOCPostProcParams;

/*!
 * \brief DL Post Process to be used with DL-RT
 * \ingroup group_app_tiovx_kernels_img_proc
 */
typedef struct {

    /** Task type : Classification, Detection, Segmentation */
    vx_int32 task_type;

    /* Number of input tensors of Post Proc Kernel */
    vx_uint32 num_input_tensors;

    /* Classification params */
    oc_params oc_prms;

    /* Detection params */
    od_params od_prms;

    /* Segmentation params */
    ss_params ss_prms;

}tivxDLPostProcParams;

/*!
 * \brief
 * \ingroup group_vision_apps_kernels_img_proc
 */

typedef struct{

  /** List of class ID's in descending order */
  vx_uint32 class_id[TIVX_OC_MAX_CLASSES];
  /** Number of top results to consider */
  vx_int32 num_top_results;

}tivxOCPostProcOutput;

/*!
 * \brief
 * \ingroup group_vision_apps_kernels_img_proc
 */
typedef struct {
  /** focal length of camera*/
  vx_float32 focal_length;

  /** Optical center x of the input image*/
  vx_int32 center_x;

  /** Optical center y of the input image*/
  vx_int32 center_y;

  /** Optical center x multiplication factor of the output image. output image optical center x = inter_center_x_fact*center_x */
  vx_int32 inter_center_x_fact;

  /** Optical center y multiplication factor of the output image. output image optical center y = inter_center_y_fact*center_y */
  vx_int32 inter_center_y_fact;

  /** Number of table entries provided for each angular value for transformation */
  vx_int32 num_table_rows;

  /** Maximum nuber of detection */
  vx_int32 num_max_det;

  /** Width of the image */
  vx_int32 width;

  /** Height of the image */
  vx_int32 height;

  /** Width of the image given to deep learning algorithm*/
  vx_int32 dl_width;

  /** Height of the image given to deep learning algorithm*/
  vx_int32 dl_height;

  /** Points per line to be interpolated for visualization purpose*/
  vx_int32 points_per_line;

  /** Number of keypoints to be detected by algorithm for each parking spot*/
  vx_int32 num_keypoints;

  /** Offset of the object information by the deep learning algorithm in output buffer*/
  vx_int32 output_buffer_offset;

  /** Skip processing */
  vx_int32 skip_flag;

}tivxODPostProcParams;

/*!
 * \brief
 * \ingroup group_vision_apps_kernels_img_proc
 */

typedef struct{
  /** width of the input dof plane */
  vx_int32 width;

  /** height of the input dof plane */
  vx_int32 height;

  /**Pad value for output planes for each side of dof plane*/
  vx_int32 pad_pixel[4];

  /** To enable 8 bit or 16 bit input/output tensor for tidl. 0 means 8 bit, 1 means 16 bit*/
  vx_int32 tidl_8bit_16bit_flag;

  /** Skip processing */
  vx_int32 skip_flag;

}tivxDofPlaneSepParams;

/*!
 * \brief
 * \ingroup group_vision_apps_kernels_img_proc
 */

typedef struct{
  /*voxel size in meters for x,y,z directionin meters*/
  vx_int32 voxel_size[3];
  /*map range minimum and maximum for x,y,z direction in meters[x/y/z][min/max]*/
  vx_int32 map_range[3][2];
  /*total number of voxels in image*/
  vx_int32 num_voxels;
  /*total number of features in map*/
  vx_int32 num_map_feat;
  /*Maximum number of match between current frame and map descriptors*/
  vx_int32 max_feat_match;
  /*Maximum number of features accounted in given frame of estimation*/
  vx_int32 max_map_feat;
  /*Is feature points calculated inside the algorithm or provided from outside. 0->inside,1-outside*/
  vx_int32 is_feat_comp_ext;
  /*max number of features possible in a frame*/
  vx_int32 max_frame_feat;
  /*initial estimate for location used in sampling the map data*/
  vx_float32 init_est[3];
  /*Input image width for depp leraning module */
  vx_int32 dl_width;
  /*Input image height for deep learning module */
  vx_int32 dl_height;
  /*Input image width in original resolution */
  vx_int32 img_width;
  /*Input image height in original resolution */
  vx_int32 img_height;
  /*Flag to tell input image is in fish eye domain, or rectilinear domain*/
  vx_int32 is_ip_fe;
  /*Input size for feature descriptor plane produced by TIDL including padding */
  vx_int32 desc_plane_size;
  /*Input descriptor (at lower resolution) scale in power of 2. Will come from TIDL outArgs */
  vx_int32 lo_res_desc_scale_pw2;
  /*upsampling filter scale in power of 2. Will come from external binary file of filter coefficients */
  vx_int32 filter_scale_pw2;
  /*upsampling filter bias scale in power of 2. Derived quantity*/
  vx_int32 bias_scale_pw2;
  /*Output descriptor (at original resolution) scale in power of 2. Has to be sync with MAP descriptor scale*/
  vx_int32 hi_res_desc_scale_pw2;
  /*Input key point score (at original resolution) scale in power of 2. Will come from TIDL outArgs */
  vx_int32 score_scale_pw2;
  /*TIDL layer Id for low resolution descriptor head*/
  vx_int32 lo_res_desc_lyr_id;
  /*TIDL layer Id for original resolution score head*/
  vx_int32 score_lyr_id;
  /*TIDL layer Id for low resolution descriptor head element type*/
  vx_int32 lo_res_desc_elm_type;
  /*TIDL layer Id for original resolution score head element type*/
  vx_int32 score_lyr_elm_type;
  /* NMS score threshold for feature point filtering */
  vx_int32 score_th;
  /* Start location for tidl tensor */
  vx_int32 tidl_tensor_startx[2];
  /* pitch for tidl tensor */
  vx_int32 tidl_tensor_pitch[2];
  /* Lens fish eye distortion correction table size*/
  vx_int32 lens_dist_table_size;
  /*focal length of camera in x direction*/
  vx_float32 fx;
  /*focal length of camera in y direction*/
  vx_float32 fy;
  /*principal axis x location*/
  vx_float32 cx;
  /*principal axis y location*/
  vx_float32 cy;
  /** Skip processing */
  vx_int32 skip_flag;

}tivxVisualLocalizationParams;

typedef struct{
  /*Image width for visualization*/
  vx_int32 img_width;
  /*Image height for visualization*/
  vx_int32 img_height;
  /*Image planes*/
  vx_int32 img_num_planes;
  /*Color counter*/
  vx_int32 frame_cnt;
  /*Matrix to convert 3D world location on top view image*/
  vx_float32 projMat[3][4];
  /*focal length of camera in x direction*/
  vx_float32 fx;
  /*focal length of camera in y direction*/
  vx_float32 fy;
  /*principal axis x location*/
  vx_float32 cx;
  /*principal axis y location*/
  vx_float32 cy;
  /*past values of x y locations to be drawn*/
  vx_int16 past_xy[TIVX_MAX_PAST_POSES][2];
  /** Skip processing */
  vx_int32 skip_flag;
  /** Maximum number of times background image needs to be copied */
  vx_int32 max_background_image_copy;

}tivxPoseVizParams;

/*!
 * \brief
 * \ingroup group_vision_apps_kernels_img_proc
 */
typedef struct {

  /** Number of input tensor*/
  vx_int32 num_input_tensors;

  /** Number of output tensor. Most likely this will be equal to number of input tensors*/
  vx_int32 num_output_tensors;

  /** Number of classes for each input tensor for visualization*/
  vx_int32 num_classes[TIVX_PIXEL_VIZ_MAX_TENSOR];

  /** Valid ROI for each class for visualiization. [TL_x, TL_y, BR_x, BR_y] */
  vx_int32 valid_region[TIVX_PIXEL_VIZ_MAX_CLASS][4];

  /** max value for each input tensor*/
  vx_int32 max_value[TIVX_PIXEL_VIZ_MAX_TENSOR];

  /** Number of classes for each output tensor*/
  vx_uint8 color_map[TIVX_PIXEL_VIZ_MAX_TENSOR][TIVX_PIXEL_VIZ_MAX_CLASS][3];

  /** output image width for each output*/
  vx_int32 output_width[TIVX_PIXEL_VIZ_MAX_TENSOR];

  /** output image height for each output*/
  vx_int32 output_height[TIVX_PIXEL_VIZ_MAX_TENSOR];

  /** offset for first pixel in each input tensor given out by TIDL*/
  vx_int32 output_buffer_offset[TIVX_PIXEL_VIZ_MAX_TENSOR];

  /** Pitch for one output row for each input tensor given out by TIDL*/
  vx_int32 output_buffer_pitch[TIVX_PIXEL_VIZ_MAX_TENSOR];

  /** offset for first pixel in input image tensor*/
  vx_int32 input_img_offset;

  /** Pitch for input image tensor*/
  vx_int32 input_img_pitch;

  /** Pitch for input image each plane pitch*/
  vx_int32 input_img_plane_pitch;

  /** Flag to generate raw tidl output*/
  vx_int32 raw_tidl_op;

  /** To enable 8 bit or 16 bit input/output tensor for tidl. 0 means 8 bit, 1 means 16 bit*/
  vx_int32 tidl_8bit_16bit_flag;

  /** To enable RGB output or yuv output. 0 - RGB, non-zero - YUV*/
  vx_int32 op_rgb_or_yuv;

  /** Falg to indicate input is RGB or YUV. 0 means RGB otherwise YUV*/
  vx_int32 ip_rgb_or_yuv;

  /** Skip processing */
  vx_int32 skip_flag;

}tivxPixelVizParams;

typedef struct {
  /** Start postion of image along X direction (horizontal) */
  vx_uint32 startX;

  /** Start postion of image along Y direction (vertical) */
  vx_uint32 startY;

  /** Width of the image */
  vx_uint32 width;

  /** Height of the image */
  vx_uint32 height;

  /** Input selector */
  vx_uint32 input_select;

  /** Channel selector */
  vx_uint32 channel_select;

  /** 1 - Enable ROI cropping, 0 - disable ROI cropping (default) */
  vx_uint32 enable_roi;

  /** When enable_roi is 1, indicates the starting X position of ROI */
  vx_uint32 roiStartX;

  /** When enable_roi is 1, indicates the starting Y position of ROI */
  vx_uint32 roiStartY;

  /** When enable_roi is 1, indicates the ROI width */
  vx_uint32 roiWidth;

  /** When enable_roi is 1, indicates the ROI height */
  vx_uint32 roiHeight;

}tivxImgMosaicWindow;

/*!
 * \brief
 * \ingroup group_vision_apps_kernels_img_proc
 */
typedef struct {

  /** Array of windows */
  tivxImgMosaicWindow windows[TIVX_IMG_MOSAIC_MAX_WINDOWS];

  /** Number of windows participating in mosiac operation */
  vx_uint32 num_windows;

  /** Number of times to clear the output buffer */
  vx_uint32 clear_count;

  /** Number of MSC instances to use */
  vx_uint32 num_msc_instances;

  /** MSC instance to use when num_msc_instances is 1 */
  vx_uint32 msc_instance;

}tivxImgMosaicParams;

/*!
 * \brief
 * \ingroup group_vision_apps_kernels_img_proc
 */
typedef struct {

  /** Detection tensor I/O params */
  sTIDL_IOBufDesc_t ioBufDesc;

  /** User config visualization threshold */
  vx_float32 viz_th;

  /** Maximum nuber of detection */
  vx_int32 num_max_det;

  /** Original Width of the image */
  vx_int32 width;

  /** Original Height of the image */
  vx_int32 height;

  /** DL Width of the image */
  vx_int32 dl_width;

  /** DL Height of the image */
  vx_int32 dl_height;

  /** Points per line to be interpolated for visualization purpose*/
  vx_int32 points_per_line;

  /** Number of keypoints to be detected by algorithm for each parking spot*/
  vx_int32 num_keypoints;

  /** List of valid class IDs to draw*/
  vx_int32 class_id[TIVX_DRAW_KEYPOINT_DETECTIONS_MAX_CLASSES];

  /** Number of valid classes to draw*/
  vx_int32 num_classes;

  /** Skip processing */
  vx_int32 skip_flag;

}tivxDrawKeypointDetectionsParams;

/*!
 * \brief
 * \ingroup group_vision_apps_kernels_img_proc
 */
typedef struct {
  /** focal length of camera*/
  vx_float32 focal_length;

  /** Optical center x of the input image*/
  vx_int32 center_x;

  /** Optical center y of the input image*/
  vx_int32 center_y;

  /** Optical center x multiplication factor of the output image. output image optical center x = inter_center_x_fact*center_x */
  vx_int32 inter_center_x_fact;

  /** Optical center y multiplication factor of the output image. output image optical center y = inter_center_y_fact*center_y */
  vx_int32 inter_center_y_fact;

  /** Number of table entries provided for each angular value for transformation */
  vx_int32 num_table_rows;

  /** Maximum nuber of detection */
  vx_int32 num_max_det;

  /** Width of the image */
  vx_int32 width;

  /** Height of the image */
  vx_int32 height;

  /** Width of the image given to deep learning algorithm*/
  vx_int32 dl_width;

  /** Height of the image given to deep learning algorithm*/
  vx_int32 dl_height;

  /** Points per line to be interpolated for visualization purpose*/
  vx_int32 points_per_line;

  /** Number of keypoints to be detected by algorithm for each parking spot*/
  vx_int32 num_keypoints;

  /** Offset of the object information by the deep learning algorithm in output buffer*/
  vx_int32 output_buffer_offset;

  /** Skip processing */
  vx_int32 skip_flag;

}tivx3DODPostProcParams;

/*!
 * \brief
 * \ingroup group_vision_apps_kernels_img_proc
 */
typedef struct {

  /** Detection tensor I/O params */
  sTIDL_IOBufDesc_t ioBufDesc;

  /** User config visualization threshold */
  vx_float32 viz_th;

  /** width of the image */
  vx_int32 width;

  /** height of the image */
  vx_int32 height;

  /** Number of classes for each output tensor*/
  vx_uint8 color_map[TIVX_DRAW_BOX_DETECTIONS_MAX_CLASSES][3];

  /** Number of valid classes to draw*/
  vx_int32 num_classes;

  /** Skip processing */
  vx_int32 skip_flag;

}tivxDrawBoxDetectionsParams;


typedef struct{

    /** Classification tensor I/O params */
    sTIDL_IOBufDesc_t *ioBufDesc;

    /** Number of top results to consider */
    vx_int32 num_top_results;

    /** Extract labelOffset from param.yaml */
    vx_int32 labelOffset;

    /** Classnames corresponding to class indexs */
    char (*classnames)[TIVX_DL_BEV_POST_PROC_MAX_SIZE_CLASSNAME];

}
oc_bev_params;

typedef struct{

    /** Detection tensor I/O params */
    sTIDL_IOBufDesc_t *ioBufDesc;

    /** User config visualization threshold */
    vx_float32 viz_th;

    /** Amount by which index is offset */
    vx_int32 labelIndexOffset;

    /** Extract labelOffset from param.yaml using labelIndexOffset */
    vx_int32 *labelOffset;

    /** Classnames corresponding to class indexs */
    char (*classnames)[TIVX_DL_BEV_POST_PROC_MAX_SIZE_CLASSNAME];

    /** formatter */
    vx_int32 formatter[6];

    /** scaleX */
    vx_float32 scaleX;

    /** scaleY */
    vx_float32 scaleY;

}
od_bev_params;

typedef struct{

    /** Detection tensor I/O params */
    sTIDL_IOBufDesc_t *ioBufDesc;

    /** Input width for tensor, extract it from tensor format, NCHW or NHWC */
    vx_int32 inDataWidth;

    /** Input height for tensor */
    vx_int32 inDataHeight;

    /** Alpha determines the extent of blending, this is extracted from params.yaml of model */
    vx_float32 alpha;

    /** YUV Color Map based on class is. */
    uint8_t **YUVColorMap;

    /** Max number of classes the color map can support. If class if
      * is more than max supported class, black is overlayed by default.
      */
    uint8_t MaxColorClass;

}
ss_bev_params;


/*!
 * \brief DL Post Process to be used with DL-RT
 * \ingroup group_app_tiovx_kernels_img_proc
 */
typedef struct {

    /** Task type : Classification, Detection, Segmentation */
    vx_int32 task_type;

    /* Number of input tensors of Post Proc Kernel */
    vx_uint32 num_input_tensors;

    /* Classification params */
    oc_bev_params oc_prms;

    /* Detection params */
    od_bev_params od_prms;

    /* Segmentation params */
    ss_bev_params ss_prms;

}tivxDLBEVPostProcParams;

/*!
 * \brief
 * \ingroup group_vision_apps_kernels_img_proc
 */
typedef struct {

  /** Detection tensor I/O params */
  sTIDL_IOBufDesc_t ioBufDesc;

  /** User config visualization threshold */
  vx_float32 viz_th;

  /** width of the image */
  vx_int32 width;

  /** height of the image */
  vx_int32 height;

  /** Number of classes for each output tensor*/
  vx_uint8 color_map[TIVX_DRAW_BOX_DETECTIONS_MAX_CLASSES][3];

  /** Number of valid classes to draw*/
  vx_int32 num_classes;

  /** Skip processing */
  vx_int32 skip_flag;

}tivxDraw3DBoxDetectionsParams;

/*!
 * \brief SFM frame level control parameter
 * \ingroup group_vision_apps_kernels_img_proc
 */
typedef struct {

  /** Skip processing */
  vx_int32 skip_flag;
  /** flow confidence threshold*/
  vx_int32 flowConfThr;
  /** Enable point cloud visualization */
  vx_int32 enAlgPtCldVis;
  /** Enable Occupancy grid visualization */
  vx_int32 enAlgOGVis;
  /** 4x4 matrix of camera pose information */
  vx_float32 camera_pose[16];

}tivxSFMParams;

/*!
 * \brief DL Pre processing to be used with DL-RT
 * \ingroup group_vision_apps_kernels_img_proc
 */
typedef struct {

  /** Skip processing */
  vx_int32 skip_flag;

  /* Scale values to be applied per channel in the range of 0.0 to 1.0 */
  vx_float32 scale[3];

  /* Mean value per channel to be subtracted, range depends on channel bit-depth */
  vx_float32 mean[3];

  /* Channel ordering, 0-NCHW, 1-NHWC */
  vx_int32 channel_order;

  /* Tensor format, 0-RGB, 1-BGR */
  vx_int32 tensor_format;

  /* Crop values to be applied, 0-Top, 1-Bottom, 2-Right, 3-Left */
  vx_int32 crop[4];

}tivxDLPreProcParams;

/*!
 * \brief DL color blend to be used with DL-RT
 * \ingroup group_vision_apps_kernels_img_proc
 */
typedef struct {

    /** Skip processing */
    vx_int32 skip_flag;

    /** Flag to indicate, if 1 - use color map, 0 - auto assign colors to each class */
    vx_int32 use_color_map;

    /** Number of classes per output not to exceed TIVX_DL_COLOR_BLEND_MAX_CLASSES*/
    vx_int32 num_classes;

    /** Color map for each output, number of colors not to exceed TIVX_DL_COLOR_BLEND_MAX_COLORS */
    vx_uint8 color_map[TIVX_DL_COLOR_BLEND_MAX_CLASSES][TIVX_DL_COLOR_BLEND_MAX_COLORS];

}tivxDLColorBlendParams;

/*!
 * \brief DL rectangles to be used with DL Draw Box
 * \ingroup group_vision_apps_kernels_img_proc
 */
typedef struct {

    /** Array of 4 values, which could be either
     *  output_type = 0 -> [xmin, ymin], [xmax, ymax]
     *  output_type = 1 -> [x, y], [width height]
     *  output_type is defined in tivxDLDrawBoxParams
     */
    vx_int32 pos[4];

    /** Confidence score in % value [0.0f - 100.0f]*/
    vx_float32 score;

    /** class_id of detected object which indexes into
     * array of class_names defined in tivxDLDrawBoxParams
    */
    vx_int32 class_id;

}tivxDLRectangle;

/*!
 * \brief DL draw box to be used with DL-RT
 * \ingroup group_vision_apps_kernels_img_proc
 */
typedef struct {

    /** Skip processing */
    vx_int32 skip_flag;

    /** Flag to indicate, if 1 - use color map, 0 - auto assign colors to each class */
    vx_int32 use_color_map;

    /** Number of classes per output not to exceed TIVX_DL_DRAW_BOX_MAX_CLASSES*/
    vx_int32 num_classes;

    /** Color map for each output, number of colors not to exceed TIVX_DL_DRAW_BOX_MAX_COLORS */
    vx_uint8 color_map[TIVX_DL_DRAW_BOX_MAX_CLASSES][TIVX_DL_DRAW_BOX_MAX_COLORS];

    /** Dictionary of class names not to exceed TIVX_DL_DRAW_BOX_MAX_CLASSES
     * and each class name not to exceed TIVX_DL_DRAW_BOX_MAX_CLASS_NAME
     */
    vx_uint8 class_names[TIVX_DL_DRAW_BOX_MAX_CLASSES][TIVX_DL_DRAW_BOX_MAX_CLASS_NAME];

    /** Array of tivxDLRectangle not to exceed TIVX_DL_DRAW_BOX_MAX_CLASSES
     * and number of rectangles per class not to exceed TIVX_DL_DRAW_BOX_MAX_RECTANGLES
     */
    tivxDLRectangle rectangle[TIVX_DL_DRAW_BOX_MAX_CLASSES][TIVX_DL_DRAW_BOX_MAX_RECTANGLES];

}tivxDLDrawBoxParams;

/*********************************
 *      Functions
 *********************************/

/*!
 * \brief Used for the Application to load the img_proc kernels into the context.
 * \ingroup group_vision_apps_kernels_img_proc
 */
void tivxImgProcLoadKernels(vx_context context);

/*!
 * \brief Used for the Application to unload the img_proc kernels from the context.
 * \ingroup group_vision_apps_kernels_img_proc
 */
void tivxImgProcUnLoadKernels(vx_context context);

/*!
 * \brief Function to register IMG_PROC Kernels on the C6x Target
 * \ingroup group_tivx_ext
 */
void tivxRegisterImgProcTargetC6xKernels(void);

/*!
 * \brief Function to un-register IMG_PROC Kernels on the C6x Target
 * \ingroup group_tivx_ext
 */
void tivxUnRegisterImgProcTargetC6xKernels(void);

/*!
 * \brief Function to register IMG_PROC Kernels on the C7x Target
 * \ingroup group_tivx_ext
 */
void tivxRegisterImgProcTargetC7xKernels(void);

/*!
 * \brief Function to un-register IMG_PROC Kernels on the C7x Target
 * \ingroup group_tivx_ext
 */
void tivxUnRegisterImgProcTargetC7xKernels(void);

/*!
 * \brief Function to register ADD IMG_PROC Kernels on the C7x Target
 * \ingroup group_tivx_ext
 */
void tivxRegisterImgProcTargetAddImageC7xKernels(void);

/*!
 * \brief Function to un-register ADD IMG_PROC Kernels on the C7x Target
 * \ingroup group_tivx_ext
 */
void tivxUnRegisterImgProcTargetAddImageC7xKernels(void);

/*!
 * \brief Function to register IMG_PROC Kernels on the R5F Target
 * \ingroup group_tivx_ext
 */
void tivxRegisterImgProcTargetMcuKernels(void);

/*!
 * \brief Function to un-register IMG_PROC Kernels on the R5F Target
 * \ingroup group_tivx_ext
 */
void tivxUnRegisterImgProcTargetMcuKernels(void);

/*!
 * \brief Function to register IMG_PROC Kernels on the A72 Target
 * \ingroup group_tivx_ext
 */
void tivxRegisterImgProcTargetMpuKernels(void);

/*!
 * \brief Function to un-register IMG_PROC Kernels on the A72 Target
 * \ingroup group_tivx_ext
 */
void tivxUnRegisterImgProcTargetMpuKernels(void);

/*!
 * \brief Used for the Application to create the pix viz kernel from the context.
 * \ingroup group_vision_apps_kernels_img_proc
 */
vx_kernel tivxAddKernelPixelViz(vx_context context, vx_int32 num_output_tensors);

/*!
 * \brief Used by the application to create the mosaic kernel from the context.
 * \ingroup group_vision_apps_kernels_img_proc
 */
vx_kernel tivxAddKernelImgMosaic(vx_context context, vx_int32 num_inputs);

/*!
 * \brief Set default parameters for tivxImgMosaicParams
 * \ingroup group_vision_apps_kernels_img_proc
 */
void tivxImgMosaicParamsSetDefaults(tivxImgMosaicParams *prms);

/*!
 * \brief Used by the application to create the dl post proc kernel from the context.
 * \ingroup group_app_tiovx_kernels_img_proc
 */
vx_kernel tivxAddKernelDLBEVPostProc(vx_context context, vx_int32 num_input_tensors);

/*!
 * \brief Used by the application to remove the dl post proc kernel from the context.
 * \ingroup group_app_tiovx_kernels_img_proc
 */
vx_status tivxRemoveKernelDLBEVPostProc(vx_context context);

/*!
 * \brief Used by the application to create the dl post proc kernel from the context.
 * \ingroup group_app_tiovx_kernels_img_proc
 */
vx_kernel tivxAddKernelDLBEVCAMPostProc(vx_context context, vx_int32 num_input_tensors);

/*!
 * \brief Used by the application to remove the dl post proc kernel from the context.
 * \ingroup group_app_tiovx_kernels_img_proc
 */
vx_status tivxRemoveKernelDLBEVCAMPostProc(vx_context context);

/*!
 * \brief Used by the application to create the dl post proc kernel from the context.
 * \ingroup group_app_tiovx_kernels_img_proc
 */
vx_kernel tivxAddKernelDLPostProc(vx_context context, vx_int32 num_input_tensors);

/*!
 * \brief Used by the application to remove the dl post proc kernel from the context.
 * \ingroup group_app_tiovx_kernels_img_proc
 */
vx_status tivxRemoveKernelDLPostProc(vx_context context);

/*!
 * \brief Used by the application to create add_c7x kernel from the context.
 * \ingroup group_app_tiovx_kernels_img_proc
 */
vx_status tivxAddKernelImgAdd(vx_context context);

/*!
 * \brief Used by the application to remove the add c7x kernel from the context.
 * \ingroup group_app_tiovx_kernels_img_proc
 */
vx_status tivxRemoveKernelAddImg(vx_context context);

#ifdef __cplusplus
}
#endif

#endif /* TIVX_IMG_PROC_KERNELS_H_ */
