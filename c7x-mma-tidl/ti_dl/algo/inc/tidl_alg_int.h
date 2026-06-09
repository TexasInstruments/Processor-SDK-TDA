/*
*
* Copyright (c) {2015 - 2025} Texas Instruments Incorporated
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

/**
 ----------------------------------------------------------------------------
 @file    tidl_alg_int.h
 @brief   This file defines process function and interface structures for
          convolution layer also some definitions and enums used in other
          layers.
 @version 0.1 (Oct 2016) : Initial version [ADK]
 @version 0.2 (Feb 2017) : Cleaned up [EPR]
 ----------------------------------------------------------------------------
*/

#ifndef ITIDL_ALG_INT_H
#define ITIDL_ALG_INT_H

#pragma CHECK_MISRA ("none")
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include "itidl_ti.h"
#include "tidl_dataflow.h"
#include "tidl_deviceInfo.h"
#include <stdarg.h>
#include "mmalib_cnn.h"
#include "perfsim.h"
#include "tidl_generic_datatypes.h"
#include "tidl_preEmption.h"
#include "dmautils_ext.h"
#include "tidl_scalability_support.h"
#include "tidl_traceUtils.h"
#include "tidl_temp.h"
#include "tidl_forceNegativeTest.h"
#include "tidl_errors.h"
#include "tidl_common_utils_infer_import.h"

using namespace c7x;

#if BUILD_WITH_OPENACC
#define OPENACC_IMPL(x) _Pragma(#x)
#define OPENACC(x) OPENACC_IMPL(acc x)
#else
#define OPENACC(x)
#endif

#ifdef HOST_EMULATION
#define CSPRAGMA(f,s) //
#define DSPRAGMA(f,s) //
#else
#define PRAGMA(x) _Pragma(#x)
#ifdef __cplusplus
#define CSPRAGMA(f,s) PRAGMA(CODE_SECTION(s))
#define DSPRAGMA(f,s) PRAGMA(DATA_SECTION(s))
#else
#define CSPRAGMA(f,s) PRAGMA(CODE_SECTION(f, s))
#define DSPRAGMA(f,s) PRAGMA(DATA_SECTION(f, s))
#endif
#endif

#define TIDL_ALIGN_CEIL(VAL, ALIGN) ((((VAL) + (ALIGN) - 1)/(ALIGN)) * (ALIGN) )
#define TIDL_ALIGN_CLIP(VAL, ALIGN) (((VAL)/(ALIGN)) * (ALIGN) )


#if defined( __C7504__ ) || defined (__C7524__) || defined (__C7604__)
  #define MMA_SIZE_8_BIT_POWER_2 (5)
#else
  #define MMA_SIZE_8_BIT_POWER_2 (6)
#endif

#define TIDL_MIN_ROWFLOW_PROCSIZE (__C7X_VEC_SIZE_BYTES__ << 1U)
#define TIDL_MMA_PANEL_SIZE (__C7X_VEC_SIZE_BYTES__)
/* Maximum number of bias split */
#define TIDL_MAX_NUM_BIAS (1U)

#define TIDL_MSMC_NUM_VIRTUAL_BANKS ( 16U)
#define TIDL_MSMC_VIRTUAL_BANK_SIZE (64U)

#define TIDL_MSMC_NUM_PHY_BANKS ( 4U)
#define TIDL_MSMC_PHY_BANK_SIZE (256U)

#define TIDL_MSMC_BANK_PITCH (TIDL_MSMC_NUM_PHY_BANKS * TIDL_MSMC_PHY_BANK_SIZE)

#define TIDL_L2_NUM_VIRTUAL_BANKS (8U)
#define TIDL_L2_VIRTUAL_BANK_SIZE (64U)

#define TIDL_L2_NUM_PHY_BANKS ( 4U)
#define TIDL_L2_PHY_BANK_SIZE (128U)

#define TIDL_L2_BANK_PITCH (TIDL_L2_NUM_PHY_BANKS * TIDL_L2_PHY_BANK_SIZE)

#define TIDL_MSMC_CACHE_LINE_SIZE (128U)
#define TIDL_L2_CACHE_LINE_SIZE (128U)

#define TIDL_WT_STAGING_MEM ((1024U * 1025U) + (128U * 1024U))

/* This is the worst case memory required in L2 for DMA/compute of the last block */
#define TIDL_CONV2D_EXTRA_L2MEM_FOR_FEAT_REQ  (256U * 1024U)


#define NUM_BMA_PBLOCK        (512)
#define NUM_COEFFS_PER_LOOP   (4U)

#define ALIGN_SIZE(x,y)       ((((x) + ((y)-1U)) / (y)) * (y))
#define MAX_IMAGE_PITCH       (1280)
#define MAX_IMAGE_HEIGHT      (720)
#define MAX_NUM_CONV_GROUPS   (1024)

#define INBUF_INT_MEM_SIZE    (1024U*16U)
#define ACC_INT_MEM_SIZE      (1024*32)
#define DENSE_COEFF_MEM_SIZE  (1024*12)
#define OUTBUF_INT_MEM_SIZE   (INBUF_INT_MEM_SIZE)
#define TIDL_BLOCK_WIDTH			(32U)
#define TIDL_BLOCK_HEIGHT			(32U)

#define TIDL_POOL_BLOCK_CENTER (0U)
#define TIDL_POOL_BLOCK_TOP    (1U)
#define TIDL_POOL_BLOCK_BOTTOM (2U)
#define TIDL_POOL_BLOCK_FULL   (4)


#ifdef CORE_DSP
#define MAX_NUM_PROC_IN_CH      (1024U)
#define TIDL_CONV2D_NUM_AUTODMA (2)
#define NUM_MAX_SCATTER_GATHER_TRANSFERS (32)
#define TIDL_MAX_BLOCK_WIDTH      (128)
#define TIDL_ENABLE_MULTIPLE_COPY
#define TIDL_ALIGN_BUFFER_CONV_KERNEL
#define MAX_NUM_LOCAL_OUT_CH     (32U)
#define TIDL_NUM_LOCAL_IN_CHS    (4)
#define TIDL_PROC_BLOCK_WIDTH    (128U)
#define TIDL_PROC_BLOCK_HEIGHT   (8U)
#define TIDL_MIN_PROC_BLOCK_HEIGHT    (4U)
#define NUM_COFFES_INT_MEM       ((uint32_t)((uint32_t)1U << 11U))
#define NUM_COFFES_INT_MEM_MASK  ((uint32_t)((uint32_t)1U << 12U)-1U)
#define NUM_OUT_CH_INT_MEM       ((uint32_t)((uint32_t)1U << 8))
#define NUM_OUT_CH_INT_MEM_MASK  ((uint32_t)((uint32_t)1U << 9U)-1U)

#else
#define MAX_NUM_PROC_IN_CH        (1024U)
#define TIDL_CONV2D_NUM_AUTODMA   (1)
#define TIDL_MAX_BLOCK_WIDTH      (32)
#define MAX_NUM_LOCAL_OUT_CH      (32U)
#define TIDL_NUM_LOCAL_IN_CHS     (4)
#define TIDL_PROC_BLOCK_WIDTH     (32U)
#define TIDL_PROC_BLOCK_HEIGHT    (32U)
#define TIDL_MIN_PROC_BLOCK_HEIGHT    (16U)
#define NUM_MAX_SCATTER_GATHER_TRANSFERS (32U)
#define NUM_COFFES_INT_MEM        (1UL << 10UL)
#define NUM_COFFES_INT_MEM_MASK   ((1UL << 11UL)-1UL)
#define NUM_OUT_CH_INT_MEM        (1UL << 8UL)
#define NUM_OUT_CH_INT_MEM_MASK   ((1UL << 9UL)-1UL)
#endif

#define MIN_INPUT_BLOCK_WIDTH     (1)
#define MIN_INPUT_BLOCK_HEIGHT    (1)
#define MIN_OUTPUT_BLOCK_WIDTH    (1)
#define MIN_OUTPUT_BLOCK_HEIGHT   (1)
#define DEF_INPUT_BLOCK_WIDTH     (1)
#define DEF_INPUT_BLOCK_HEIGHT    (1)
#define DEF_OUTPUT_BLOCK_WIDTH    (1)
#define DEF_OUTPUT_BLOCK_HEIGHT   (1)

#define DEF_POOL_WIDTH            (2)
#define DEF_POOL_HEIGHT           (2)
#define DEF_POOL_STRIDE_WIDTH     (2)
#define DEF_POOL_STRIDE_HEIGHT    (2)

#define CONV_KER_MIN_WIDTH        (1)
#define CONV_KER_MAX_WIDTH        (9)
#define CONV_KER_MIN_HEIGHT       (1)
#define CONV_KER_MAX_HEIGHT       (9)
#define CONV_STRIDE_MIN_WIDTH     (1)
#define CONV_STRIDE_MAX_WIDTH     (4)
#define CONV_STRIDE_MIN_HEIGHT    (1)
#define CONV_STRIDE_MAX_HEIGHT    (4)

#define DECONV_STRIDE_MIN_WIDTH   (2)
#define DECONV_STRIDE_MAX_WIDTH   (2)
#define DECONV_STRIDE_MIN_HEIGHT  (2)
#define DECONV_STRIDE_MAX_HEIGHT  (2)
#define DECONV_KER_MIN_WIDTH      (2)
#define DECONV_KER_MAX_WIDTH      (4)
#define DECONV_KER_MIN_HEIGHT     (2)
#define DECONV_KER_MAX_HEIGHT     (4)


#define TIDL_SAT_LO_UINT8              (0)
#define TIDL_SAT_HI_UINT8              (255)
#define TIDL_SAT_LO_INT8               (-128)
#define TIDL_SAT_HI_INT8               (127)
#define TIDL_SAT_LO_UINT16             (0)
#define TIDL_SAT_HI_UINT16             (65535)
#define TIDL_SAT_LO_INT16              (-32768)
#define TIDL_SAT_HI_INT16              (32767)
#define TIDL_SAT_LO_INT32              (0x80000000U)
#define TIDL_SAT_HI_INT32              (0x7FFFFFFF)

#define TIDL_DEFAULT_L1_MEM_SIZE       (16 * 1024)
#define TIDL_DEFAULT_L2_MEM_SIZE       (4*1024)
#define TIDL_DEFAULT_L3_MEM_SIZE       (L3MEM_SCRATCH_BUF_SIZEKB * 1024U)

#define TIDL_1K_ZERO_VECTOR ( 1024U)
#define TIDL_MEMCPY_TR_MEM  (128U)
/* Define TRUE/FALSE to go with Bool */
#ifndef TRUE

#define TRUE		( 1)
#define FALSE		( 0)

#endif


#define USE_MSMC_STAGE_FOR_WT  (0)

#if USE_MSMC_STAGE_FOR_WT
static const int32_t gTempUseMsmcStageForWt = 1;
#else
static const int32_t gTempUseMsmcStageForWt = 0;
#endif

/**
 *  \anchor TIDL_KernelType
 *  \name   TIDL Kernel Type
 *
 *  This group defines the different types of kernel
 *
 *  @{
 */
#define TIDL_NonLinearKernel     ((int32_t) 0)
#define TIDL_LinearKernel        ((int32_t) 1)
/* @} */

#if defined(HOST_EMULATION)
#define ENABLE_TRACE_PROFILE      (0)
/* To enable the natural C-code for the optimized C66x kernels */
#define ENABLE_CN_CODE            (0)
#else
#if defined(LOKI_BUILD)
#define ENABLE_TRACE_PROFILE      (0)
#define ENABLE_CN_CODE            (0)
#else
#define ENABLE_TRACE_PROFILE      (0)
#define ENABLE_CN_CODE            (0)
#endif

#endif


#define QUANT_DIVISOR_Q (16)
#define IP_QUANT_DIVISOR_Q (16)
#define PRELU_SLOPE_Q (8)

#if (ENABLE_TRACE_PROFILE) && defined(CORE_DSP) && !defined(HOST_EMULATION)
int64_t _TSC_read(void);
#endif

#if (ENABLE_TRACE_PROFILE) && defined(CORE_DSP) && defined(HOST_EMULATION)
#define _TSC_read() (0)
#endif

#define ENABLE_OLD_FLOW (1)

#ifndef __C7100__
#define ENABLE_BACKWARDS_COMPATIBILITY  (0) //Intended only for OTF flow till all layers migrated
#endif

#define TIDL_1913_NOT_FIXED (1)
#define TIDL_2386_NOT_FIXED (1)
#define TIDL_2383_NOT_FIXED (1)
#define TIDL_3350_NOT_FIXED (1)

#define MAX_MULTI_CORE_CTX_COPY_LINK (2)

/**
@struct BBox
@brief  This structure defines the BBox

@param  xmin
indicates the min value in x direction
@param  ymin
indicates the min value in y direction
@param  xmax
indicates the max value in x direction
@param  ymax
indicates the max value in y direction
@param  size
indicates the size of the BBox
*/
typedef struct
{
  float32_tidl xmin;
  float32_tidl ymin;
  float32_tidl xmax;
  float32_tidl ymax;
}BBox;

/**
 @struct  tidlConv2dBuffParams_t

 @brief   This structure contains parameters used for transferring input data
          from external memory to internal memory for computation and for
          transferring output data from internal memory to external memory after
          computation. It is used for block level operations.

 @params  inWidth
          Width of input data in external memory.

 @params  inHeight
          Height of input data in external memory.

 @params  inPitch
          Pitch of input data in external memory.

 @params  outWidth
          Width of output data in external memory.

 @params  outHeight
          Height of output data in external memory.

 @params  outPitch
          Pitch of output data in external memory.

 @params  outPitchPooling
          Pitch of output data used for pooling in external memory.

 @params  inChPitch
          Channel Pitch of input data in external memory. It contains the size
          of one complete channel data in external memory.

 @params  outChPitch
          Channel Pitch of output data in external memory. It contains the size
          of one complete channel data in external memory.

 @params  outChPitchPooling
          Channel Pitch of output data for pooling in external memory.

 @params  inElementType
          Indicates the type of input as signed or unsigned.

 @params  outElementType
          Indicates the type of output as signed or unsigned.

 @params  firstRoundBits
          Round bits used for intermediate sum results in convolution
          operations.

 @params  secondRoundBits
          Round bits used for final output in convolution operations.

 @params  avgPoolingRoundBits
          Round bits used for final output in pooling operations.

 @param   quantizationStyle
          Value to indicate the quantizationStyle used for Convolution

 @param   biasQFact
          Value to indicate Q factor used for Bias values in Convolution

 @param  zeroWeightValue
          value of weights added for dynamic quantSytle

 @param  max
          max value across the all the convolution output values

 @param  min
          min value across the all the convolution output values

 @param  kernelType
          value indicates the type of kernel coeffs used either dense or sparse

 @param  quantFactor
          value indicates the quantization factor used in convolution

 @param  outElementSize
          value indicates the size of the convolution output element

*/
typedef struct {
  int32_t    inWidth;
  int32_t    inHeight;
  int32_t    inPitch;
  int32_t    outWidth;
  int32_t    outHeight;
  int32_t    outPitch;
  int32_t    outPitchPooling;
  int32_t    inChPitch;
  int32_t    outChPitch;
  int32_t    outChPitchPooling;
  uint32_t    inBatchPitch;
  uint32_t    outBatchPitch;
  int32_t     inElementType;
  int32_t     outElementType;
  int32_t     firstRoundBits;
  int32_t     secondRoundBits;
  int32_t     avgPoolingRoundBits;
  int32_t     quantizationStyle;
  int32_t     biasQFact;
  int32_t     zeroWeightValue;
  int32_t     max;
  int32_t     min;
  int32_t     kernelType;
  int32_t     actType;
  uint16_t    quantFactor;
  int32_t     numTotRoi;
  int32_t     scratchSize;
  int32_t     mmaPSATMin;
  int32_t     mmaPSATMax;
  void *      scratchMem;
  void *      outRoundBitsPtr;
  void *      zeroPointsPtr;
  uint8_t*    memcpyTr;
  float32_tidl       inScaleFactor;
}tidlConv2dBuffParams_t;

static inline void TIDL_edmaCopy(void * dst, const void * src, uint16_t size);

#define TIDL_KEEP_NET_FROM_APP_ALIVE (0)

/**
*  @enum   eMemrecs
*  @brief  Memory records for image pyra,id application
*/
typedef enum
{
  /* Memory records for handle */
  ALG_HANDLE_MEMREC,/*0*/
  /**< Space to point to create time paramemetrs, not created instance in handle as
   * handle size is kept small to be able to fit in local memory */
  ALG_CREATE_PARAM_MEMREC, //1
  /**< L1 memory*/
  ALG_SCRATCH_L1_MEM_MEMREC,//2
  /**< L2 memory*/
  ALG_SCRATCH_L2_MEM_MEMREC,//3
  /**< L3 memory*/
  ALG_SCRATCH_L3_MEM_MEMREC,//4
  /**< Space for layer specific infomration - like coeffs buffers,
   * scale memory buffer for pooling,... very specific to network
   * and need to persist - though it needs some corection as below
   * 1. All const data shall be segregated like network parameters
   * 2. All scratch data shall be segragated like scale meomory for pooling
   */
  ALG_LAYERS_PARAMS_BUFF_MEMREC, //5

  ALG_SCRATCH_DATA_BUFF_MEMREC,/*6*/

  /**< External memory for holding intermediate feature maps, ideally it is scratch
  * but during pre-emption, there can be some layer output which is in DDR and due to that
  * we need to make it persistent
  * As of now it also have weights buffer which needs to be removed */
  ALG_SCRATCH_DATA_BUFF_EXT_MEMREC, //7
  ALG_REF_SCRATCH_BUFF_MEMREC,/*8*/
  ALG_REF_OUTPUT_BUFF_MEMREC,/*9*/

  /**< External memory for holding derived information for each layer to allow compute
   * upfront during init and then use during process (Persistent)
   * */
  ALG_LAYERS_MEMREC,/*10*/

  /**< External memory for object detection layers' some intermediate data
   * Only for object Detection layers
  */
  ALG_SCRATCH_DDR_MEM_MEMREC,/*11*/

  /**< External memory to hold the context/state of internal memories if the network
   * is being pre-empted. Possible for this space to be reused across netowrks based upon
   * number of pre-emption levels
   */
  ALG_CONTEXT_MEM_MEMREC,/*12*/

  ALG_PERSIST_DDR_NET_MEMREC , //13

  ALG_PERSIST_SYNC_MEMREC, /*14*/

  /**< External memory for holding GC data */
  ALG_PERSIST_DDR_INIT_DATA , //15

  MAX_NUM_MEMRECS
} eMemrecs;

/**
*  @enum   eAlgState
*  @brief  State of application
*/
typedef enum
{
  ALG_NOT_ACTIVE,
  ALG_ACTIVE
} eAlgState;

/* @} */

/**
 *  \anchor TIDL_DmaChannels
 *  \name   TIDL DMA Channels
 *
 *  DMA Channels used for TIDL
 *
 *  @{
 */
#define TIDL_DMA_CHANNEL_DIRECT_TR_INFEATMAP      ((int32_t) 0)
#define TIDL_DMA_CHANNEL_DIRECT_TR_INFEATMAP_2    ((int32_t) 1)
#define TIDL_DMA_CHANNEL_DIRECT_TR_INFEATMAP_3    ((int32_t) 2)
#define TIDL_DMA_CHANNEL_DIRECT_TR_INFEATMAP_4    ((int32_t) 3)
#define TIDL_DMA_CHANNEL_DIRECT_TR_WTMAP          ((int32_t) 4)
#define TIDL_DMA_CHANNEL_DIRECT_TR_OUTFEATMAP     ((int32_t) 5)
#define TIDL_DMA_CHANNEL_DIRECT_TR_OUTFEATMAP_2   ((int32_t) 6)
#define TIDL_DMA_CHANNEL_DIRECT_TR_OUTFEATMAP_3   ((int32_t) 8)
#define TIDL_DMA_CHANNEL_DIRECT_TR_OUTFEATMAP_4   ((int32_t) 7)
#define TIDL_DMA_CHANNEL_DIRECT_TR_GENERIC_1      ((int32_t) 9)
#define TIDL_DMA_CHANNEL_DIRECT_TR_GENERIC_2      ((int32_t) 10)
#define TIDL_DMA_CHANNEL_WT_STAGING               ((int32_t) 11) /*Till this all are direct TR channels*/
#define TIDL_DMA_CHANNEL_ST                       ((int32_t) 12) /*Till this all are direct TR channels*/
#define TIDL_DMA_CHANNEL_MEMCPY                   ((int32_t) 13) /*Till this all are direct TR channels*/
#define TIDL_DMA_CHANNEL_MAX                      ((int32_t) 14)


/* @} */


/**
 *  \anchor TIDL_BuildType
 *  \name   TIDL Build Type
 *
 *  This group defines the different build types supported by TIDL
 *
 *  @{
 */
#define TIDL_BUILD_TYPE_RUN_REF_ONLY            (0x00000001U)
#define TIDL_BUILD_TYPE_NATC_MMALIB             (0x00000002U)
#define TIDL_BUILD_TYPE_NATC_DSP                (0x00000004U)
#define TIDL_BUILD_TYPE_REF_COMP                (0x00000008U)
#define TIDL_BUILD_TYPE_WITH_DMA                (0x00000020U)

#define TIDL_REF_CODE_FOR_IP                    (0x00000001U)
#define TIDL_HIGH_ACCURACY_SIGMOID              (0x00000002U)
#define TIDL_NATC_CODE_SIGNED_MAXPOOL           (0x00000008U)
/* @} */


#define TIDL_PADDING_TYPE_TOP                   ((int32_t)0)
#define TIDL_PADDING_TYPE_BOTTOM                ((int32_t)1)
#define TIDL_PADDING_TYPE_BOTH                  ((int32_t)2)
#define TIDL_PADDING_TYPE_TOP_LEFT              ((int32_t)3)
#define TIDL_PADDING_TYPE_ST_TOP_LEFT           ((int32_t)4)
#define TIDL_PADDING_TYPE_PAD_LAYER             ((int32_t)5)
#define TIDL_PADDING_TYPE_PAD_LAYER_BOTH        ((int32_t)6)
#define TIDL_PADDING_TYPE_PAD_LAYER_TOP         ((int32_t)7)
#define TIDL_PADDING_TYPE_PAD_LAYER_BOTTOM      ((int32_t)8)

#define  TIDL_MMA_HANDLE_MAX                    (3)
#define  TIDL_ST_HANDLE_MAX                     (3U)

#define TIDL_OBJ_DET_MAX_HEADS (16)

/**
@struct sTIDL_compressionDimensions
@brief  This structure defines the dimensions of a compression based transfer
@param  sbIcnt0
Innermost superblock count
@param  sbIcnt1
Second superblock dimension
@param  sbDim1
Superblock internal dim
@param  sIcnt0
Number of Superblocks
@param  sIcnt0
Number of Superblocks
@param  sDim0
Jump between Superblocks
@param  algorithm
Compression Algorithm
@param bias
Compression Bias
*/
typedef struct
{
  int32_t sbIcnt0;
  int32_t sbIcnt1;
  int32_t sbDim1;
  int32_t sIcnt0;
  int32_t sDim0;
  int32_t algorithm;
  int32_t bias;
  uint8_t* secTrPtr;
  uint8_t* cdbTablePtr;
  uint8_t* compBitStreamPtr;
}sTIDL_compressionDimensions;

/*Virtual Padding:*/
typedef struct
{
  int32_t padT;
  int32_t padL;
  int32_t padR;
  int32_t padB;
}sTIDL_virtualPad;


/**
@struct sTIDL_ALgDetectOutputParams_t
@brief  This structure defines the buffers used in the
Detecttion Output Layer

@param  all_loc_preds
Scratch buffer used to store predictions of
prior boxes
@param  all_decode_bboxes
Buffer to store all the decoded Bboxs
@param  all_decode_kps
Buffer to store all the decoded Key points
@param  all_conf_scores
Buffer to store the scores and indexs after
threshold calculation for all the classes
@param  all_conf_scores
Buffer to store the scores and indexs after
threshold calculation for all the classes
@param  indices
Buffer to store the scores and indexs after
NMS and overlap for each classes
@param  score
Buffer to store the sorting scores and indexs
for all classes to select top k objects
@param  index
Buffer to store size of each class or number of
objects in each class after NMS
@param  labels
Buffer to store labels for all classes
@param  scoreLut
Buffer to store Sigmoid Score
*/
typedef struct {

  void                     *inLocDataList[TIDL_OBJ_DET_MAX_HEADS];
  float32_tidl             inLocdataQList[TIDL_OBJ_DET_MAX_HEADS];
  float32_tidl             inLocdataZPList[TIDL_OBJ_DET_MAX_HEADS];
  void                     *inConfDataList[TIDL_OBJ_DET_MAX_HEADS];
  float32_tidl             inConfdataQList[TIDL_OBJ_DET_MAX_HEADS];
  float32_tidl             inConfdataZPList[TIDL_OBJ_DET_MAX_HEADS];
  void                     *inExtraDataList[TIDL_OBJ_DET_MAX_HEADS];
  float32_tidl             inExtradataQList[TIDL_OBJ_DET_MAX_HEADS];
  float32_tidl             inExtradataZPList[TIDL_OBJ_DET_MAX_HEADS];
  int32_t                  locHeadPitchList[TIDL_OBJ_DET_MAX_HEADS][TIDL_PITCH_MAX];
  int32_t                  confHeadPitchList[TIDL_OBJ_DET_MAX_HEADS][TIDL_PITCH_MAX];
  int32_t                  extraHeadPitchList[TIDL_OBJ_DET_MAX_HEADS][TIDL_PITCH_MAX];
  float32_tidl             confHeadInvPitchList[TIDL_OBJ_DET_MAX_HEADS][TIDL_PITCH_MAX];
  int32_t                  locDataOffset[TIDL_OBJ_DET_MAX_HEADS];
  int32_t                  confDataOffset[TIDL_OBJ_DET_MAX_HEADS];
  int32_t                  extraDataOffset[TIDL_OBJ_DET_MAX_HEADS];
  int32_t                  numAnchors[TIDL_OBJ_DET_MAX_HEADS];

  void *priorBoxPtr ;

  /*Needed in valid point score caluclation API dependent on M. allocated in L2*/
  uint16_t * topMScore;
  uint16_t * topMScoreSorted; // needed in softmax only
  float32_tidl    * tempScore; // number of elements equal to number of classes, needed in softmax only
  int32_t  * topMIndices;
  int32_t  * topMIndicesSorted;
  /*------------------------------------------*/ /*allocated in L1D*/
  int32_t  * countMList; //
  int32_t  * countMListAcc;
  int32_t  * topKIndices;
  void     * topKLoc;
  BBox     * topKBbox; // equal to topKLoc
  uint16_t * topKScore;
  int32_t  * nmsKeptIndices; // Needed in NMS depdent on K only
  /*--------------------------------------------*/
  /*temprorary buffer for breaking the software pipelined loop. Allocated in L3*/
  // buffer dependent on feature map resolution
  int64_t  * pred;
  void       * featMaxMinVal;
  /*-------------------------------------------*/
  /*Needed in valid point score caluclation API dependent on M when executing from DDR flow*/
  uint16_t * topMScoreDdr;
  uint16_t * topMScoreSortedDdr;
  int32_t  * topMIndicesDdr;
  int32_t  * topMIndicesSortedDdr;
  void     * scratchPtr;
  /*-------------------------------------------*/
  int32_t elementType;
  int32_t elmSize;
  int32_t topM;
  int32_t topMDdr;
  int32_t scratchDDRConsumed;
  int32_t topMAllClasses;
  int32_t isBckClsAvailable;
  int32_t boxParamsSize;
  int32_t extraParamSize;
  int32_t numSubCodeElements; //Store number of element for a given subCodeType
  int32_t numInElementPerKeypoint; //store number of elements per keypoint as output of the n/w
  int32_t numOutElementPerKeypoint; //store number of elements per keypoint as part of outObjData
  int32_t numOutElementPer_2dBox; //store number of elements per 2D box as part of outObjData
  int32_t numOutElementPer_3dBox; //store number of elements per 3D box as part of outObjData
}sTIDL_ALgDetectOutputParams_t;

typedef struct {
  void * pCoeffs[TIDL_MMA_HANDLE_MAX];
  void * pCoeffStagePtr;
  void * tempOutPr;
  void * biasParamMem;
  /* This is applicable only if channelwise quantization is enabled */
  void * outRoundBitsPtr;
  /* This is applicable only if channelwise quantization is enabled */
  void * biasBPtr;
  /* Scales and shifts (Asymmetric only)*/
  void *mmaScalesPtr;
  void *mmaShiftsPtr;
  int32_t quantizationStyle;
  int32_t startRowNumberInTensor;
  int32_t orgInTensorHeight;
  void * mmaHandleArgsMem[TIDL_MMA_HANDLE_MAX];
  uint32_t dataFlow;
  // sDataFlowInfo_t * dataFlowInfo;
  int32_t biasParamSize;
  int32_t mmaHandleArgsSize[TIDL_MMA_HANDLE_MAX];
  uint32_t totalNumCoeffs[MAX_NUM_CONV_GROUPS];
  uint32_t totalActualCoeffs[MAX_NUM_CONV_GROUPS];
  /* Size of processing */
  int32_t  procElemSize;
  /*Flag to indicate if compression is enabled for weight buffers*/
  int32_t isWTCompressed;
  sTIDL_compressionDimensions compProperties;
  /* To be used only when rowflow grouped conv has to be split across multiple workloads */
  int32_t coeffRefExtraOffset;
  /* To be used only when conv is split across multiple cores channel-wise */
  int32_t coeffMultiCoreExtraOffset;
  int32_t biasRefExtraOffset;
  /* To be used only when conv is split across multiple cores channel-wise */
  int32_t biasMultiCoreExtraOffset;
  sTIDL_virtualPad virtualPad;
}sTIDL_ALgConvParams_t;

typedef struct {
  void * biasParamMem;
  void * wtTranformMem;
  void * accMem;
  void * matmulScaleMem;
  void * matmulShiftMem;
  int32_t wtTranformSize;
  int32_t biasParamSize;
  int32_t accMemSize;
  uint8_t mmaScale;
  uint8_t mmaShift;
  int32_t matmulScaleMemSize;
  int32_t matmulShiftMemSize;
  int32_t wtTransformBatchPitch;
  int32_t biasBatchPitch;
  int32_t maxWorkloadLoopIter;
}sTIDL_ALgInnerProductParams_t;

typedef struct {
  void* reserved;
}sTIDL_ALgReLULayer_t;

typedef struct {
  void* reserved;
}sTIDL_ALgArgOpLayer_t;

typedef struct {
  int32_t isOptResizeAvail;
  int32_t scratchSize;
  int32_t isMmaCiAvailable;
  int32_t inNumBytes;
  __HWA_CONFIG_REG_v1 mma_config_reg;
  void* reserved;
  int32_t inputPadRefillZeros;
  void* permutePatternBuf;
  void* permutePatternBufInt;
}sTIDL_ALgResizeLayer_t;

typedef struct {
  void* reserved;
}sTIDL_ALgColorConversionParams_t;

/**<
 *******************************************************************************
  @struct sTIDL_ALgEltWiseParams_t
  @brief  This structure used to store Eltwise layer parameters to use in the Process call

  @param whichInputIsSigned  : To indicate which one input is signed out of 2 inputs
  @param buffInputBlockOffset: Offset of the second input buffer from first input address
  @param inputScale          : input scales of the two inputs
  @param isMmaCiAvailable    : To indicate in the process call that MMA kernel is available
  @param inNumBytes          : Input elementSize in bytes, this required in MMA HW init in Process call
  @param mma_config_reg      : MMA config register required in MMA HW init in Process call
  @param mma_offset_reg      : MMA offset register required in MMA HW init in Process call
 *******************************************************************************
*/
typedef struct {
  int32_t whichInputIsSigned;
  int64_t buffInputBlockOffset[TIDL_ST_HANDLE_MAX];
  uint16_t inputScale[TIDL_SIMD_WIDTH>>1];
  int32_t isMmaCiAvailable;
  int32_t inNumBytes;
  __HWA_CONFIG_REG_v1 mma_config_reg;
  __HWA_OFFSET_REG mma_offset_reg;
}sTIDL_ALgEltWiseParams_t;

typedef struct {
  int32_t predicateBufSize;
  uint32_t scaleMemSize[TIDL_ST_HANDLE_MAX];
  void *scalePtr[TIDL_ST_HANDLE_MAX];
  void *scalePtrInt[TIDL_ST_HANDLE_MAX];
  /** For average pooling, Q value of the weight used to replace division */
  int32_t internalPoolingWeightQ;
  int32_t nextOutOffset; /* Denotes offset where the kernel will write data during next call */
  int32_t startRowNumberInTensor; /*Denotes the offset in row number of Tensor */
  int32_t orgInTensorHeight; /*Denotes the  Height of orignal input tensor - Used in case of Tiled processing*/
  sTIDL_virtualPad virtualPad;
}sTIDL_ALgPoolingParams_t;

typedef struct {
  int32_t outOffsetIncPerTensor;
  int32_t isMmaCiAvailable;
  int32_t inNumBytes;
  __HWA_CONFIG_REG_v1 mma_config_reg;
  __HWA_OFFSET_REG mma_offset_reg;
}sTIDL_ALgConcatParams_t;

typedef struct {
  void* reserved;
  int32_t outOffsetIncPerTensor;
  int32_t ops;
  void *ptrToScratch;
}sTIDL_AlgReduceParams_t;

typedef struct {
  int32_t outOffsetIncPerTensor;
  int32_t isMmaCiAvailable;
  void *accPtr;    /* Pointer to scratch memory needed for accumulate(twice of output data)[ScatterND] */
  __HWA_CONFIG_REG_v1 mma_config_reg;
  __HWA_OFFSET_REG mma_offset_reg;
}sTIDL_AlgScatterElementsParams_t;

typedef struct{
  int32_t factorB, factorC;
  void* lutParamMem;
  int32_t lutParamSize;
  int32_t nonLinearActMethod;
} sTIDL_AlgNonLinearActivation_LUT_Params_t ;

typedef struct {
  void *slopeFactMem;
  int32_t slopeFactSize;
  int32_t *slopeFact;
  void * biasParamMem;
  void * weightParamMem;
  int32_t biasParamSize;
  int32_t weightParamSize;
  int32_t biasB;
  int32_t isMmaCiAvailable;
  int32_t isLUTenabled;
  int32_t inNumBytes;
  uint32_t swLutScratchSize[TIDL_ST_HANDLE_MAX];
  void *swLutScratchPtr[TIDL_ST_HANDLE_MAX];
  void *lutScratchInt[TIDL_ST_HANDLE_MAX];
  sTIDL_AlgNonLinearActivation_LUT_Params_t lutParams;
}sTIDL_ALgBatchNormParams_t;

typedef struct {
  void* perChannelPadConstTensorPtr;
  int32_t perChannelPadConstTensorSize;
}sTIDL_AlgPadParams_t;

typedef struct {
  void* paramPtr;
  int32_t paramSize;
}sTIDL_AlgCustomParams_t;

typedef struct {
  void* paramPtr;
  int32_t paramSize;
}sTIDL_AlgConstDataParams_t;



/* CHECK_MISRA("-18.4")  -> Disable rule 18.4  */
typedef union {
  sTIDL_ALgConvParams_t              convParams;
  sTIDL_ALgEltWiseParams_t           eltWiseParams;
  sTIDL_ALgPoolingParams_t           poolParams;
  sTIDL_ALgArgOpLayer_t              argOpParams;
  sTIDL_ALgResizeLayer_t             resizeParams;
  sTIDL_ALgInnerProductParams_t      innerProductParams;
  sTIDL_ALgDetectOutputParams_t      detectionOutputParams;
  sTIDL_ALgConcatParams_t            concatParams;
  sTIDL_ALgBatchNormParams_t         batchNormParams;
  sTIDL_ALgColorConversionParams_t   colorConversionParams;
  sTIDL_AlgReduceParams_t            reduceParams;
  sTIDL_AlgScatterElementsParams_t   scatterElementsParams;
  sTIDL_AlgPadParams_t               padParams;
  sTIDL_AlgCustomParams_t            customParams;
  sTIDL_AlgConstDataParams_t         constDataParams;
}sTIDL_AlgLayerParams_t;
/*RESET_MISRA("18.4")  -> Reset rule 18.4 */


/* Generic Flow related structures */
/**
 *  @enum   eTensorType
 *  @brief  Enumeration for different tensor types
 *
 */
typedef enum
{
  /**< This value indicates tensor tpye as input data */
  TIDL_INPUT_TENSOR       = 0,
  /**< This value indicates tensor tpye as output data */
  TIDL_OUTPUT_TENSOR      = 1,
  /**< This value indicates tensor tpye as parameters(weights) */
  TIDL_PARAMETER_TENSOR   = 2,
  /**< This value indicates different tensor tpyes */
  TIDL_MAX_TENSOR_TYPES   = 3
} eTensorType ;

/**<
 *******************************************************************************
  @struct TIDL_TensorTransferInfo
  @brief  This structure used to store tensor parameters for each tesnor and
             to use them in the process call for configuration of DMA

  @param tensorType        : Different tensor types supported, refer to eTensorType
  @param transferSize      : Size of data to be transfered,
                             In case of input/params it is from DDR/MSMC to L2
                             In case of output it is from MSMC to DDR
  @param elementSizeinBytes: Size of elements of the tensor in Bytes,
  @param dmaNumChs         : Number of DMA channels required to transfer each tensor data
  @param dmaStartChIndex   : Indicates the DMA start channel Index for each tensor
  @param perChTrSize       : Size of data to be transfered to L2 for each channel
  @param lastChTrSize      : Size of data to be transfered to L2 for last channel
  @param srcChOffset       : OffsetInBytes in the src buffer for the next DMA channel
  @param chPitchL3InBytes  : chPitch in src for input read and in dst for output write,
                             In case of IN: srcPitch : Pitch of buffer in source being in MSMC/DDR
                             In case of OUT: If final destination is MSMC then this pitch is not used
                             In case of OUT: If final destination is DDR then pitch of the buffer in DDR
 *******************************************************************************
*/
typedef struct
{
  eTensorType tensorType;
  int32_t transferSize;
  int32_t elementSizeinBytes;
  int32_t dmaNumChs;
  int32_t dmaStartChIndex;
  int32_t perChTrSize;
  int32_t lastChTrSize;
  int32_t srcChOffset;
  int32_t chPitchL3InBytes;
  int32_t lfmJump;
}TIDL_TensorTransferInfo;


/* Each tensor is DMAed with different DMA channel so the TIDL_MAX_IN_TENSORS
   is limited by Max number of DMA channels available for generic flow */
#define TIDL_MAX_IN_TENSORS      (8U)
#define TIDL_MAX_OUT_TENSORS     (1U)
#define TIDL_MAX_PARAM_BUFFERS   (1U)
#define TIDL_TOTAL_ALL_TENSORS   (TIDL_MAX_IN_TENSORS + TIDL_MAX_OUT_TENSORS + TIDL_MAX_PARAM_BUFFERS)
/* 64 is DmaUtilsAutoInc3d_getTrMemReq(1) */
#define DMAUTILS_AUTOINC_3D_TR_MEM_SIZE (64)

/**<
 *******************************************************************************
  @struct TIDL_GenericHandle
  @brief  This structure is the main handle for GenericFlow process call
  @param numberTensors      : Total number of tensors requires generic data flow,
                              Multiple input and parameter tensors and single
                              output Tensor is supported
  @param disableMSMCstaging : Used indicate for the GenericFlow that MSMC staging is disabled
  @param inBlkProcessingJump: Processing jump used for ping/pong buffers at blk boundary on the Input side
  @param inChProcessingJump : Processing jump used for ping/pong buffers at channel boundary on the Input side
  @param outBlkProcessingJump: Processing jump used for ping/pong buffers at blk boundary on the Output side
  @param outChProcessingJump : Processing jump used for ping/pong buffers at channel boundary on the Output side
  @param isCPUcopyNeeded     : Flag indicates if someBytes to be filled as zeros in the L2 before DMA transfer
                               This is required when procsize > activeBufWidthDst from DataFlowInfo
  @param inDMAChList       : List holds the channel ID's of DMA for input tensors
  @param outDMAChList      : List holds the channel ID's of DMA for output tensor
  @param paramDMAChList    : List holds the channel ID's of DMA for weight tensors
  @param tensorTransferInfo : Holds the tensor level info for all tensors, refer to TIDL_TensorTransferInfo
  @param trMemTr            : Memory used to store trasfer parameters to prepare a TR
  @param execInArgs      : Used to send process level parameters to the kernel
  @param execOutArgs     : Used to read process level parameters from the kernel
  @param functionPtr       : Function pointer for the layer execution function
 *******************************************************************************
*/
typedef struct
{
  int32_t numberTensors;
  int32_t disableMSMCstaging;
  int32_t inBlkProcessingJump;
  int32_t inChProcessingJump;
  int32_t outBlkProcessingJump;
  int32_t outChProcessingJump;
  int32_t outBatchProcessingJump;
  int32_t numSplitsPerBatch;
  int32_t isCPUcopyNeeded;
  int32_t mixPrecisionWithSignedInput;
  int32_t mixPrecisionEnabledForOutput;
  int32_t inDMAChList[TIDL_DMA_CHANNEL_MAX];
  int32_t outDMAChList[TIDL_DMA_CHANNEL_MAX];
  int32_t paramDMAChList[TIDL_DMA_CHANNEL_MAX];
  TIDL_TensorTransferInfo tensorTransferInfo[TIDL_TOTAL_ALL_TENSORS];
  uint8_t trMemTr[TIDL_DMA_CHANNEL_MAX][DMAUTILS_AUTOINC_3D_TR_MEM_SIZE];
  TIDL_ExecInArgs execInArgs;
  TIDL_ExecOutArgs execOutArgs;
  TIDL_genericExecFuncPtr functionPtr;
  TIDL_genericUpdateInArgsPtr updateFunctionPtr;
}TIDL_GenericHandle;

typedef struct {
  sTIDL_AlgLayerParams_t layerParams;
  TIDL_LayerMetaData metaData;
  int32_t layerIdx; // Data ID
  int32_t execLayerNum ;
  int32_t isOptimalForPreemption;
  int32_t wlRepeatIter;
  int32_t procType;
  int16_t inLayerIdx[TIDL_NUM_IN_BUFS];
  int16_t isInData[TIDL_NUM_IN_BUFS];
  int16_t isInOutData[TIDL_NUM_IN_BUFS];
  int16_t isOutData[TIDL_NUM_OUT_BUFS];
  int16_t inWorkloadIdx[TIDL_NUM_IN_BUFS];
  int32_t outBufIdxWl;
  void  * scratchMem;
  int32_t scratchSize;
  /* New Generic flow related */
  const sWorkloadUnit_t         *workloadUnit;
  const sGCHelperHandle         *gcHelperHandle;
  TIDL_GenericHandle * TIDL_GenericFlowHandle[TIDL_MMA_HANDLE_MAX][TIDL_MAX_IN_TENSORS];
  void* kernelHandle[TIDL_MMA_HANDLE_MAX][TIDL_MAX_IN_TENSORS];
  int32_t kerHandleSize;
  int32_t totalL1Size;
  uint8_t *dataFlowMemPtr;
  int32_t dataFlowMemSize;
  void    *memcpyTr;
  void    *workloadHandle;
  /*Used to Store Indexes of the Multi-Core Context Transfer Links*/
  int32_t multiCoreCtxtLinkIndex[MAX_MULTI_CORE_CTX_COPY_LINK];
  WorkloadUnitExec_padParams wlPadParams;
}sTIDL_AlgLayer_t;



typedef struct {
  uint32_t    isWtDmaPending;
  uint8_t      *wtMemBasePtr;
  uint32_t     currPtrOffset;
  uint32_t     lastPtrOffset;
  uint32_t     totStageMemAvail;
  uint8_t      trMem[64];
}sTIDL_weightStagingSync;

typedef struct {
  int32_t    isWtDmaPending;
  uint8_t      *wtMemBasePtr;
  int32_t     currPtrOffset;
  int32_t     lastPtrOffset;
  int32_t     totStageMemAvail;
  uint8_t      trMem[64];
  }sTIDL_acrossLayerShare;

typedef struct
{
  uint8_t  *trMem;
  uint32_t trMemSize;
  uint32_t numDruChannel;
}TIDL_DmaRetParams;

typedef struct{
  int32_t i_OD;
  sTIDL_AlgLayer_t *algLayer_OD ;
}sODLayerInfo_t ;

/**
@struct TIDL_Obj
@brief  This structure is the main handle of
applet.

@param  ivision    All public function pointers
@param  algState   State of algorithm to indicate
@param  numMemRecs Number of memory records
@param  memRec     Array of memory records
@param  privContext Pointer to the conctext memory of Private algo
@param  isPreEmptionEnable Is 1 indicates that pre-emption is enabled
@param  preEmptHandle Pointer to the preemption handle, refer @TIDL_preEmptionObj
@param  preemptPerfHW Indicate cycles from HW need to consider for preemption logic
@param  preEmptContextInfo Structure holds the context memory info for pre-emption
*/
typedef struct
{
  const IVISION_Fxns*   ivision;
  uint8_t               algState;
  uint32_t              numMemRecs;
  IALG_MemRec           memRec[MAX_NUM_MEMRECS];
  TIDL_CreateParams    * createParams ;
  void                 * pNetParams ;
  void *                dataBuf[TIDL_MAX_DATA_BUFS];
  sTIDL_sysMemHandle_t sysMems[TIDL_SYSMEM_MAX];
  sTIDL_AlgLayer_t *    alglayerParams;
  uint32_t              procCallCounter;
  sTIDL_LayerBuf_t *    TIDLLayersBuf;
  uint8_t                 * refScratchBuf;
  int32_t                   refScratchBufSize;
  int32_t              * activationHistPtr;
  float32_tidl         * activationRangePtr;
  TIDL_sysScratchPtr  sysScratchPtr;
  sPerfSim_t             *perfSimOutput;
  sWorkloadSuperGroup_t  *wlSuperGrp;
  sGCHelperHandle         gcHelperHandleObj;
  sGCHelperHandle         *gcHelperHandle;
  uint8_t * zeroVector1k;
  uint8_t                   * intAlgHandle;
  uint32_t                    memAvailForFeat;
  uint8_t                      isPadDmaPending;
  void                      * udmaDrvObj;
  void                      * dmaUtilsContext;
  void                      * privContext;
  TIDL_dmaChannelAllocContext dmaChannelAllocContext;
  sTIDL_acrossLayerShare layerShare;
  sTIDL_weightStagingSync weightStageSync;
  /* This is the pointer to the location where TR for TIDL_memcpy2D is stored */
  uint8_t *memcpyTr;
  uint8_t *stagingMemPtr;
  int32_t currAlgLayer;
  int32_t isPreEmptionEnable;
  int32_t preemptPerfHW;
  float32_tidl ddrBytesPerCPUCycle;
  void    * preEmptHandle;
  TIDL_preEmptContextObj  preEmptContextInfo;
  sODLayerInfo_t odLayerInfo[TIDL_OBJ_DET_MAX_HEADS];
  int32_t numODLayer ;
  int32_t groupId;
  TIDL_controlSetArgs controlArgs;
  void* cudaMemManager;  /**< Pointer to TIDL_CudaMemManager structure (opaque pointer) */
} TIDL_Obj;

typedef TIDL_Obj * TIDL_Handle;

/*--------------------------------------------------------*/
/* IALG functions                                         */
/* Refer XDAIS ialg.h file for details on these functions */
/*--------------------------------------------------------*/
int32_t TIDL_numAlloc(void);
int32_t TIDL_alloc(const IALG_Params *params,
                       IALG_Fxns **parentFxns, IALG_MemRec *memRec);
int32_t TIDL_init(IALG_Handle handle,
                      const IALG_MemRec *memRec, IALG_Handle parent,
                      const IALG_Params *params);
void  TIDL_activate(IALG_Handle handle);
void  TIDL_deactivate(IALG_Handle handle);
int32_t TIDL_free(IALG_Handle handle, IALG_MemRec *memRec);

int32_t TIDL_control(IVISION_Handle Handle, IALG_Cmd cmd,
                         const IALG_Params *inParams, IALG_Params *outParams);

int32_t TIDL_process(IVISION_Handle Handle, IVISION_InBufs *inBufs,
                         IVISION_OutBufs *outBufs, IVISION_InArgs *inArgs,
                         IVISION_OutArgs *outArgs);

/* TIDL_odOutputReformat_dummyFunc is a dummy function to avoid unused function warning */
void TIDL_odOutputReformat_dummyFunc(void);

#if defined TIDL_COVERAGE_DEAD_CODE
/**
----------------------------------------------------------------------------
@ingroup    TIDL_UTILS
@fn         TIDL_getSysScratchPtr
@brief      Function gives you a pointer to system memory used to store networks
            intermediate output
@param      bufInfo : Pointer to buffer info. This will come from dataflow
@param      sysScratchPtr    : Pointer which stores the base pointer for all system scratch
@param      baseAccessFlag   : Set it to one to get base pointer of feature map. Set it to zero to
                             go to the location where DMA/CPU will READ/WRITE.

@remarks    None
@return     Pointer to the memory. NULL if  buffers accessor is NONE
----------------------------------------------------------------------------
*/
void * TIDL_getSysScratchPtr(const sBufferInfo_t * bufInfo, const TIDL_sysScratchPtr *sysScratchPtr, uint32_t baseAccessFlag);
#endif

void * TIDL_getMemoryChunkFromSysmem(sTIDL_sysMemHandle_t sysMems[TIDL_SYSMEM_MAX],
  uint32_t size, uint32_t alignment, uint32_t space, uint32_t attribute);

void TIDL_resetSysmem(sTIDL_sysMemHandle_t sysMems[TIDL_SYSMEM_MAX]);
void TIDL_reset_OneMem(sTIDL_sysMemHandle_t sysMems[TIDL_SYSMEM_MAX], int32_t i);


/* If it is inter Buff Allocate buffer or Initilize it NULL */
/* At start of process call, all the null Ptrs as to be initlized
by alg using the in and out buffers provided by user */
/* At the end of processing the these has be again initialized with NULL*/
int32_t TIDL_IsInterBuffer(sTIDL_Network_t * net, int32_t buffId,
int32_t currLayersGroupId);

/* Initilze dataMemTabID as zero if it is not allocated by this core */
/* If any of the previusly allocated buffer can be
re-used (layer IDx is less than the current layer) then gwt the buf ID.*/
/* Update the memeTab to max of prev and current values */
int32_t TIDL_getReuseBufferId(int32_t buffId);


int32_t TIDL_conv2dParamsInit(sTIDL_AlgLayer_t * algLayer,
        const sTIDL_ConvParams_t * params,uint16_t imHeight,uint16_t imWidth,
        uint32_t l2ScratchSize, uint32_t l3ScratchSize, int32_t	quantizationStyle,
        uint16_t interElementSize, int32_t numCoeffsBytes);

sTIDL_DataParams_t * TIDL_getDataParams(
sTIDL_Network_t * pTIDLNetStructure,
int32_t dataId);

int32_t isPerChannelQuantizationEnabled(const sTIDL_ConvParams_t * convParams,
                                                 int32_t calibrationOption);
static  uint32_t TIDL_findHCF(uint32_t num1,uint32_t num2);



/** ======================= ============================
*  @name   TIDLCONV_dataFlowType
*
*  @desc   Describes the various data flow used in convolution
*
*  @field  TIDLCONV_DATAFLOW_ROW
*             Row flow
*  @field  TIDLCONV_DATAFLOW_COL
*             Column flow
*  ===============================================================
*/

#define  TIDLCONV_DATAFLOW_ROW (0)
#define  TIDLCONV_DATAFLOW_COL (1)



int32_t TIDL_conv2DCoeffMemRequired(const sTIDL_ConvParams_t * conv2dparams,
                                                            uint32_t weightsElementSizeInBits,
                                                            uint32_t dataFlowType,
                                                            uint32_t * coeffBufPitch);


int32_t TIDL_innerProductCoeffMemRequired(sTIDL_Layer_t *tidlLayer);

typedef struct {
  int32_t imWidth;
  int32_t imHeight;
  int32_t pad;
  int32_t mmaPanelWidth;
  int32_t NoPerG;
  int32_t isStridedFlow;
  // const sDataFlowInfo_t * dataFlowInfo;
  const sTIDL_ConvParams_t * convParams;
} TIDL_conv2dRowFlow_mmalibHandleArgs;


template <class typeInitArgs, class TypeSrc0Buf>
int32_t TIDL_conv2dColFlow_getLayerSetupSize(void);

/*Overloaded wrapper*/
int32_t TIDL_conv2dColFlow_getLayerSetupSize(int32_t targetDevice, int32_t quantizationStyle);

static int32_t TIDL_ABS(int32_t x);

/* Get Pointer helper functions */
static inline int64_t * get_int64_t_pointer(int64_t arr[], int32_t offset);

static inline uint64_t * get_uint64_t_pointer(uint64_t arr[], int32_t offset);

static inline int32_t * get_int32_t_pointer(int32_t arr[], int32_t offset);

static inline uint32_t * get_uint32_t_pointer(uint32_t arr[], int32_t offset);

static inline uint8_t * get_uint8_t_pointer(uint8_t arr[], int32_t offset);

static inline uint16_t * get_uint16_t_pointer(uint16_t arr[], int32_t offset);

static inline int16_t * get_int16_t_pointer(int16_t arr[], int32_t offset);

static inline sTIDL_AlgLayer_t * get_AlgLayer_t_pointer(sTIDL_AlgLayer_t arr[], int32_t offset);

static inline int32_t getAlgLyrIdx (sTIDL_AlgLayer_t arr[], int32_t layerIdx);

static inline int32_t getAlgLyrIdxFromWL (sWLDataBase_t *pWLDB, const sWorkloadUnit_t  *workloadUnit);

static inline float32_tidl * get_float32_t_pointer(float32_tidl arr[], int32_t offset);

static inline BBox * get_Bbox_pointer(BBox arr[], int32_t offset);


/* CHECK_MISRA("-8.5")  -> Disable rule 8.5  */

/* MISRA.ONEDEFRULE.FUNC
   MISRAC_2004 Rule_8.5
   MISRAC_WAIVER:
   This is flagged in a single file called: tidl_alg_int.h.
   This function needs to be an inline function, and it is
   declared and defined in the header file. Also, it is
   downgraded to advisory in MISRA-C 2012
*/

static inline void TIDL_edmaCopy(void * dst, const void * src, uint16_t size)
{
#if 0
  EDMA_UTILS_memcpy2D((void *)       dst,
    (const void  *)		 src,
    (uint16_t)     size,
    (uint16_t)     1,
    (int16_t)      0,
    (int16_t)      0);
#endif
}

template <typename typeScalerIn>
static inline void TIDL_memset(typeScalerIn *ptr, typeScalerIn val, int32_t elements){
  for(int32_t offset = 0; offset < elements ; offset++)
  {
    ptr[offset] = val ;
  }
  return ;
}

static int32_t TIDL_ABS(int32_t x)
{
	int32_t y;
	if(x < 0)
	{
		y = (-x);
	}
	else
	{
	  y = (x);
	}
	return (y);
}

static float32_tidl TIDL_ABS_FLOAT(float32_tidl x)
{
  float32_tidl y;
  if (x < 0.0)
  {
    y = (-x);
  }
  else
  {
    y = (x);
  }
  return (y);
}

static uint32_t TIDL_findHCF(uint32_t num1,uint32_t num2)
{
  uint32_t min, i;
  uint32_t hcf = 1U;
  min=(num1>num2)? num2 : num1;
  for(i=min;((i>=1U) && (hcf==1U));--i)
  {
    if(((num1%i)==0U) && ((num2%i)==0U))
    {
      hcf = i;
    }
  }
  return hcf;
}


#if BUILD_WITH_OPENACC
OPENACC(routine seq)
template<typename T>
inline T TIDL_openaccShiftRightImpl(T a, int32_t b) {
    return a >> (uint32_t)b;
}
OPENACC(routine seq)
template<typename T>
inline T TIDL_openaccShiftRight(T a, int32_t b) {
    return TIDL_openaccShiftRightImpl<T>(a, b);
}
#endif

static inline float32_tidl exp_taylor_sigmoid(float32_tidl x)
{
  float32_tidl twoPwF, ePwX;

  float32_tidl ln2      = 0.693147180559945f;
  // float32_tidl oneByLn2 = 1.44269504090f;
  float32_tidl oneBy6   = 0.1666667f;
  // float32_tidl oneBy24  = 0.0416667f;
  // float32_tidl y        = oneByLn2 * x;
  int32_t yI            = (int32_t)x;
  float32_tidl yf       = x - (float32_tidl)yI;
  float32_tidl pkdOneBy65356 = (float32_tidl)0.0000152587890625f;
  float32_tidl floatRes = yf * ln2;

  float32_tidl floatRes2 = floatRes * floatRes;
  float32_tidl floatRes3 = floatRes2 * floatRes;
  // float32_tidl floatRes4 = floatRes2 * floatRes2;
  twoPwF = 1.0f + floatRes + (floatRes2 * 0.5f);
  twoPwF = twoPwF + (floatRes3 * oneBy6);
  // twoPwF = twoPwF + (floatRes4 * oneBy24);

  /*pre left shift of 16 is done to generate the result multiple of (1<<16). Later it is divided
    by (1<<16) to get accurate result.
   */
  __vpred vp =  __cmp_gt_pred(yI, 0);
  int32_t shift_val = __min((int32_t)14, __abs(yI));
  int32_t tempShiftL = __shift_left((1 << 16), shift_val);
  int32_t tempShiftR = __shift_right((1 << 16), shift_val);

  tempShiftL = __select(vp, tempShiftL, tempShiftR);
  ePwX = twoPwF * (float32_tidl)(tempShiftL);

  ePwX = ePwX * pkdOneBy65356;

  return ePwX;
}

static inline float32_tidl exp_taylor(float32_tidl x)
{
  float32_tidl twoPwF, ePwX;
  float32_tidl ln2      = 0.693147180559945f;
  float32_tidl oneByLn2 = 1.44269504090f;
  float32_tidl oneBy6   = 0.1666667f;
  float32_tidl oneBy24  = 0.0416667f;
  float32_tidl y        = oneByLn2 * x;
  int32_t yI            = (int32_t)y;
  float32_tidl yf       = y - (float32_tidl)yI;
  float32_tidl oneBy65356 = 0.0000152587890625f;

  float32_tidl floatRes = yf * ln2;

  float32_tidl floatRes2 = floatRes * floatRes;
  float32_tidl floatRes3 = floatRes2 * floatRes;
  float32_tidl floatRes4 = floatRes2 * floatRes2;
  twoPwF = 1.0f + floatRes + (floatRes2 * 0.5f);
  twoPwF = twoPwF + (floatRes3 * oneBy6);
  twoPwF = twoPwF + (floatRes4 * oneBy24);

  /*pre left shift of 16 is done to generate the result multiple of (1<<16). Later it is divided
    by (1<<16) to get accurate result.
   */
  int32_t tempShiftL = __shift_left((1 << 16), yI);
  int32_t tempShiftR = __shift_right((1 << 16), -yI);

  if(yI > 0)
  {
    tempShiftL = tempShiftL;
  }
  else
  {
    tempShiftL = tempShiftR;
  }

  ePwX = twoPwF * (float32_tidl)(tempShiftL);

  ePwX = ePwX * oneBy65356;

  if((int32_t)-16 > yI)
  {
    ePwX=0.0f;
  }
  

  /*Natural C code has 46 in place of 14. this is done to avoid long8 processing for tempShiftL/R
    hopefully clipping the large value should not affect the algorithm accuracy
  */

  if(yI > (int32_t)14)
  {
    ePwX = FLT_MAX;
  }

  return ePwX;
}

static inline float_vec exp_taylor_f16(float_vec x)
{
  float_vec twoPwF, ePwX = 0.0f;

  float_vec ln2           = (float_vec)0.693147180559945f;
  float_vec oneByLn2      = (float_vec)1.44269504090f;
  float_vec oneBy6        = (float_vec)0.1666667f;
  float_vec oneBy24       = (float_vec)0.0416667f;
  float_vec y             = oneByLn2 * x;
  int_vec yI              = (int_vec)(0);
  yI                      = (c7x::convert_int_vec(y));
  float_vec yf            = y - __int_to_float(yI);
  float_vec pkdOneBy65356 = (float_vec)0.0000152587890625f;

  float_vec floatRes = yf * ln2;

  float_vec floatRes2 = floatRes * floatRes;
  float_vec floatRes3 = floatRes2 * floatRes;
  float_vec floatRes4 = floatRes2 * floatRes2;
  twoPwF = ((float_vec)1.0f) + floatRes + (floatRes2 * ((float_vec)0.5f));
  twoPwF = twoPwF + (floatRes3 * oneBy6);
  twoPwF = twoPwF + (floatRes4 * oneBy24);

  /*pre left shift of 16 is done to generate the result multiple of (1<<16). Later it is divided
    by (1<<16) to get accurate result.
   */
  __vpred vp =  __cmp_gt_pred(yI, (int_vec)0);

  int_vec tempShiftL = __shift_left((int_vec)(1 << 16), yI);
  int_vec tempShiftR = __shift_right((int_vec)(1 << 16), -yI);
  tempShiftL = __select(vp, tempShiftL, tempShiftR);

  ePwX = twoPwF * (c7x::convert_float_vec(tempShiftL));

  ePwX = ePwX * pkdOneBy65356; // in optimized code this can be replaced by multiplication

  vp   =  __cmp_gt_pred((int_vec)-16, yI);
  ePwX = __select(vp, (float_vec)0.0f, ePwX);

  /*Natural C code has 46 in place of 14. this is done to avoid long8 processing for tempShiftL/R
    hopefully clipping the large value should not affect the algorithm accuracy
  */
  vp   =  __cmp_gt_pred(yI, (int_vec)14);
  ePwX = __select(vp, (float_vec)FLT_MAX, ePwX);


  return ePwX;
}

static inline float32_tidl taylor_erf(float32_tidl x){
  const float32_tidl twoByPI = (float32_tidl)1.1283791670955f;
  const float32_tidl c3 = (float32_tidl)0.3333333333333f;
  const float32_tidl c5 = (float32_tidl)0.1000000f;
  const float32_tidl c7 = (float32_tidl)0.0238095238095f;
  const float32_tidl c9 = (float32_tidl)0.0046296296296f;

  float32_tidl x3 = x * x;
  float32_tidl x5 = x3 * x3;
  float32_tidl x7 = x5 * x3;
  float32_tidl x9 = x7 * x3;

  float32_tidl res = 1.0f - (c3*x3) + (c5*x5) - (c7*x7) + (c9*x9);
  res = res * twoByPI * x;
  return res;
}

static inline float32_tidl div_sp(float32_tidl x, float32_tidl y)
{
  float32_tidl recp_y = __recip(y);

  float32_tidl TWO  = 2.0f;
  float32_tidl result;

  recp_y = recp_y*(TWO - (y*recp_y));
  result = x*recp_y;

  return result;
}

static inline int64_t * get_int64_t_pointer(int64_t arr[], int32_t offset)
{
    return &arr[offset];
}

static inline uint64_t * get_uint64_t_pointer(uint64_t arr[], int32_t offset)
{
    return &arr[offset];
}

static inline int32_t * get_int32_t_pointer(int32_t arr[], int32_t offset)
{
    return &arr[offset];
}

static inline uint32_t * get_uint32_t_pointer(uint32_t arr[], int32_t offset)
{
    return &arr[offset];
}

static inline uint8_t * get_uint8_t_pointer(uint8_t arr[], int32_t offset)
{
    return &arr[offset];
}

static inline uint16_t * get_uint16_t_pointer(uint16_t arr[], int32_t offset)
{
    return &arr[offset];
}

static inline int16_t * get_int16_t_pointer(int16_t arr[], int32_t offset)
{
    return &arr[offset];
}

static inline sTIDL_AlgLayer_t * get_AlgLayer_t_pointer(sTIDL_AlgLayer_t arr[], int32_t offset)
{
    return &arr[offset];
}

static inline int32_t getAlgLyrIdx (sTIDL_AlgLayer_t arr[], int32_t layerIdx)
{
  int32_t i=0, matched=0;
  do
  {
    if (arr[i].layerIdx == layerIdx)
    {
      matched = 1;
    }
    i++;
  } while (matched != 1);

  return i-1;
}


static inline float32_tidl * get_float32_t_pointer(float32_tidl arr[], int32_t offset)
{
  return &arr[offset];
}

static inline BBox * get_Bbox_pointer(BBox arr[], int32_t offset)
{
  return &arr[offset];
}

int32_t TIDL_getLayerNum(const sTIDL_Network_t * pTIDLNetStructure, int32_t dataId);
int32_t TIDL_getDatElementSign(int32_t elementType);
int32_t TIDL_layerProcess(
  TIDL_Handle          intAlgHandle,//:TODO: Ideally should not pass handle
  sTIDL_AlgLayer_t     * algLayer,
  sTIDL_Layer_t        * TIDLLayer,
  void                 * inPtrs[],
  void                 * outPtrs[],
  sTIDL_sysMemHandle_t * sysMems,
  int32_t i,
  TIDL_NetworkCommonParams   *commonParams);


int32_t TIDL_FillPaddedCols(
  uint8_t *ptr,
  const sBufferInfo_t *bufInfo,
  int32_t width,
  int32_t height,
  int32_t numBatches,
  int32_t linePitch,
  int32_t channelPitch,
  int32_t batchPitch,
  int32_t elementType,
  int32_t flowCtrl,
  int32_t paddingType,
  void * padLayerParams,
  int32_t outDataPadH,
  int32_t outDataPadW,
  int32_t padFillValue,
  void * perChannelPadValPtr,
  int32_t inHeight);

int32_t  TIDL_DspFillPaddedBatches(
    uint8_t * ptr,
    int32_t ptrOffset,
    int32_t batchPadW,
    int32_t height,
    int32_t numChs,
    int32_t numBatches,
    int32_t linePitch,
    int32_t channelPitch,
    uint32_t batchPitch);

void TIDL_getMMAv2_EltwiseScalesAndShift(float32_tidl scale1, float32_tidl scale2, uint8_t *intScale1, uint8_t *intScale2, uint8_t *cScale, uint8_t *shift);
void TIDL_getMMAv2_ScaleAndShift(float32_tidl scaleRatio, uint8_t *scale, uint8_t *shift);


/*
  pNet has below structure
  sTIDLNetwork (only for the valid layers, not whole array of [TIDL_MAX_LAYERS]) followed by
  weight BUffer followed by
  data Flow Info

  curretly Net structure is required to be alive even after network creation. Among the three properties
  weight buffer is something which is not required to be different for each network of same group, so
  we can avoid it. Also making sure that pNet is alive ( a pointer provided during create time) is not
  a good assumption from algorithm vendor. So will avoid this assumption and do as below:
  Have 3 different MemTab requesting these buffers : pNet, pWeightBuffer, pDataFlowInfo
  each of them is persistent but pWeightBuffer space shall be reused and it shall be requested as zero size
  for the models which are already part of them same modelGroupId

  information about weight buffer is distributed across multiple layers,
  so need to be careful, its in each layer
  //Offset to the kernel parameters
  int32_t   weights;
  //Offset to the bias parameters
  int32_t   bias;

  There are couple of meomry record which can manage from a common pool of memory
  (A) Context memory for pre-emption (ALG_CONTEXT_MEM_MEMREC), this is variable size so
      how to manage if first is large and then second one is
  (B) Output feature map memory (ALG_SCRATCH_DATA_BUFF_EXT_MEMREC)
    - ideally it is scratch but with pre-emption it becomes persistent
  (C) Buffers for weight - ALG_LAYERS_PARAMS_BUFF_MEMREC, if no reordering,
      if so then request only druing first model of same GroupId and then keep using it
*/

uint32_t TIDL_getSizeNet(const TIDL_CreateParams *pCreateParams);

int32_t TIDL_MoveNetToPrivateMemory(TIDL_Handle algHandle,
  const IALG_MemRec memRec[], int32_t coreId);
void * TIDL_CopyAndReturnGCPrivateMemPtr(TIDL_Handle algHandle, const IALG_MemRec memRec[], int32_t isReferenceFlow);
void * TIDL_getNetObjBufPtr( sTIDL_Network_t *pNet, const IALG_MemRec memRec[], uint32_t netMember);
int32_t TIDL_referencFlow(const TIDL_CreateParams *pCreateParams);

void TIDL_UpdateScaleFactors(TIDL_Handle intAlgHandle, int32_t i, int32_t updateStats, int64_t accMin, int64_t accMax);
void TIDL_syncWl(TIDL_sharedBufferProperties * ctrlSetArgs, int32_t wlToPoll[], int32_t syncPoint, int32_t wlId,
                  uint32_t coreId, int32_t numCores);
int32_t float_to_int_c7x(float32_tidl x);
/* RESET_MISRA("8.5")  -> Reset rule 8.5 */
#endif /*ITIDL_ALG_INT_H */
