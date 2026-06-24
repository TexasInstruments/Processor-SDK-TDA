/*
 *
 * Copyright (c) 2022 Texas Instruments Incorporated
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

#ifndef TIVX_EDGEAI_IMG_PROC_KERNELS_H_
#define TIVX_EDGEAI_IMG_PROC_KERNELS_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>
#include <itidl_ti.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup group_edgeai_tiovx_kernels_img_proc TIVX Kernels for Image Pre/Post Processing
 *
 * \brief This section documents the kernels defined for Image Pre/Post Processing
 *
 * \ingroup group_edgeai_tiovx_kernels
 */
/*!
 * \file
 * \brief The list of supported kernels in this kernel extension.
 */

/*! \brief OpenVX module name
 * \ingroup group_edgeai_tiovx_kernels_img_proc
 */
#define TIVX_MODULE_NAME_EDGEAI_IMG_PROC            "edgeai_img_proc"

/*! \brief Kernel Name: DL Pre processing Armv8
 *  \see group_edgeai_tiovx_kernels_dl_pre_proc_armv8
 */
#define TIVX_KERNEL_DL_PRE_PROC_ARMV8_NAME          "com.ti.img_proc.dl.pre.proc.armv8"

/*! \brief Kernel Name: DL post process
 *  \see group_edgeai_tiovx_kernels_dl_post_process
 */
#define TIVX_KERNEL_DL_POST_PROC_NAME               "com.ti.img_proc.dl.post.proc"

/*! \brief Kernel Name: DL color convert Armv8
 *  \see group_vision_apps_kernels_dl_color_convert_armv8
 */
#define TIVX_KERNEL_DL_COLOR_CONVERT_ARMV8_NAME     "com.ti.img_proc.dl.color.convert.armv8"

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

/*!
 * \brief DL Pre processing to be used with DL-RT
 * \ingroup group_edgeai_tiovx_kernels_img_proc
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

  /* Channel pitch in bytes from TIDL IO buf desc (inChannelPitch), used for
  * 4D batch mode to match the stride TIDL kernel expects between channels */
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

}
oc_params;

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

}
od_params;

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
ss_params;

/*!
 * \brief DL Post Process to be used with DL-RT
 * \ingroup group_edgeai_tiovx_kernels_img_proc
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

/*********************************
 *      Functions
 *********************************/

/*!
 * \brief Used for the Application to load the img_proc kernels into the context.
 * \ingroup group_edgeai_tiovx_kernels_img_proc
 */
void tivxEdgeaiImgProcLoadKernels(vx_context context);

/*!
 * \brief Used for the Application to unload the img_proc kernels from the context.
 * \ingroup group_edgeai_tiovx_kernels_img_proc
 */
void tivxEdgeaiImgProcUnLoadKernels(vx_context context);

/*!
 * \brief Function to register IMG_PROC Kernels on the Armv8 Target
 * \ingroup group_edgeai_tiovx_kernels_img_proc
 */
void tivxRegisterEdgeaiImgProcTargetArmv8Kernels(void);

/*!
 * \brief Function to un-register IMG_PROC Kernels on the Armv8 Target
 * \ingroup group_edgeai_tiovx_kernels_img_proc
 */
void tivxUnRegisterEdgeaiImgProcTargetArmv8Kernels(void);

/*!
 * \brief Used by the application to create the dl post proc kernel from the context.
 * \ingroup group_edgeai_tiovx_kernels_img_proc
 */
vx_kernel tivxAddKernelDLPostProc(vx_context context, vx_int32 num_input_tensors);

/*!
 * \brief Used by the application to remove the dl post proc kernel from the context.
 * \ingroup group_edgeai_tiovx_kernels_img_proc
 */
vx_status tivxRemoveKernelDLPostProc(vx_context context);

#ifdef __cplusplus
}
#endif

#endif /* TIVX_EDGEAI_IMG_PROC_KERNELS_H_ */
