/*
*
* Copyright (c) {2025} Texas Instruments Incorporated
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
@file      itidl_io.h
brief      This file defines the input/output buffer descriptor for TI deep learning library
*/

/** @ingroup    iTI_DL_IO */
/*@{*/
#ifndef ITIDL_IO_H_
#define ITIDL_IO_H_

#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <float.h>


#ifdef __cplusplus
extern "C" {
#endif

#define TIDL_STRING_SIZE                  ((int32_t) 512)
#define TIDL_MAX_ALG_IN_BUFS              ((int32_t) 32)
#define TIDL_MAX_ALG_OUT_BUFS             ((int32_t) 128)

#define TIDL_IO_MAX_NUM_CORES             (4U)

/* Typedef for 32 bit float */
typedef float float32_tidl;
/* Typedef for 64 bit float */
typedef double float64_tidl;


/**
 *  \anchor eTIDL_ElementType
 *  \name   TIDL Element Type
 *
 *  This group defines the different types of element type used by TIDL library
 *
 *  @{
 */
#define TIDL_UnsignedChar          ((int32_t) 0)
#define TIDL_SignedChar            ((int32_t) 1)
#define TIDL_UnsignedShort         ((int32_t) 2)
#define TIDL_SignedShort           ((int32_t) 3)
#define TIDL_UnsignedWord          ((int32_t) 4)
#define TIDL_SignedWord            ((int32_t) 5)
#define TIDL_SinglePrecFloat       ((int32_t) 6)
#define TIDL_UnsignedDoubleWord    ((int32_t) 7)
#define TIDL_SignedDoubleWord      ((int32_t) 8)
#define TIDL_Bool                  ((int32_t) 9)
/* @} */

/**
 *  \anchor eTIDL_TensorLayout
 *  \name   Tenosr Layout Type
 *
 *  This group defines the indices Tenosr Layout Type suported by TIDL library
 *  TIDL_LT_NCHW --- Channels first, (Number_of_ROIs, Number_Of_Channels, Height , Width)- example (1,3,360,720 )
 *  TIDL_LT_NHWC --- Channels last,  (Number_of_ROIs, Height , Width, Number_Of_Channels)- example (1,360,720,3 )
 *
 *  @{
 */
#define TIDL_LT_NCHW             ((uint32_t) 0)
#define TIDL_LT_NHWC             ((uint32_t) 1)
#define TIDL_LT_MAX              (TIDLRT_LT_NHWC+1)
/* @} */

/**
@enum    eTIDL_inferenceMode
@brief   This enumerator defines the different inference implementation modes supported by TIDL
*/
typedef enum
{
  TIDL_inferenceModeDefault = 0,         /* Inference using single c7x/MMA core */
  TIDL_inferenceModeHighThroughput = 1,  /* Batch processing mode - Multiple frames parallely infer on multiple cores */
  TIDL_inferenceModeLowLatency    = 2    /* Single batch inference using network split on multiple cores */
} eTIDL_inferenceMode;

/**
@enum    eTIDL_inDataFormat
@brief   This enumerator defines the different indata formats
*/
typedef enum
{
  TIDL_inDataFormatBGRPlanar = 0,
  TIDL_inDataFormatRGBPlanar = 1,
  TIDL_inDataFormatRGBMax    = 2
} eTIDL_inDataFormat;

/**
@enum    eTIDL_inResizeType
@brief   This enumerator defines the different indata formats
*/
typedef enum
{
  TIDL_inResizeTypeDefault   = 0,
  TIDL_inResizeTypeKeepAR    = 1,
  TIDL_inResizeTypeMax       = 2
} eTIDL_inResizeType;


/**
@struct  sTIDL_IOBufDesc_t
@brief   This structure defines the Input and output buffer descriptors
         required for a given Layer group
*/
typedef struct
{
  /** Number of Input buffer required by the Layer group  */
  int32_t numInputBuf;
  /** Number of Output buffer required by the Layer group  */
  int32_t numOutputBuf;
  /** Number of cores used for compute */
  int32_t numCores;
  /** Number of virtual cores from application point of view for a given mode of implementation */
  int32_t numVirtualCores;
  /** Number of times batch processing call needs to be invoked in multi core devices (each call can handle numVirtualCores number of batches)  */
  int32_t numSuperBatches;
  /** TIDL inference implementation mode @ref eTIDL_inferenceMode */
  int32_t inferenceMode;
  /** Input Tensor format  @ref eTIDL_inDataFormat */
  int32_t inDataFormat[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_IN_BUFS];
  /** Input Tensor resize  Type @ref eTIDL_inResizeType */
  int32_t inResizeType[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_IN_BUFS];
  /** Resize width for compressed image before crop */
  int32_t resizeWidth[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_IN_BUFS];
  /** Resize Height for compressed image before crop */
  int32_t resizeHeight[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_IN_BUFS];
  /** Feature width of each input buffer */
  int32_t inWidth[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_IN_BUFS];
  /** Feature Height of each input buffer */
  int32_t inHeight[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_IN_BUFS];
  /** Number of channels in each input buffer */
  int32_t inNumChannels[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_IN_BUFS];
  /** DIM2 dimension of each input buffer whose tensor dimesnions are N x Dim1 x Dim2 x C x H x W */
  int32_t inDIM2[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_IN_BUFS];
  /** DIM1 dimension of each input buffer whose tensor dimesnions are N x Dim1 x Dim2 x C x H x W */
  int32_t inDIM1[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_IN_BUFS];
  /** Minimum Channel pitch for the input tensor */
  int32_t inChannelPitch[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_IN_BUFS];
  /** Number of Batches in each input buffer */
  int32_t inNumBatches[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_IN_BUFS];
  /** Left zero padding required for each input buffer */
  int32_t inPadL[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_IN_BUFS];
  /** Top zero padding required for each input buffer */
  int32_t inPadT[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_IN_BUFS];
  /** Right zero padding required for each input buffer */
  int32_t inPadR[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_IN_BUFS];
  /** Bottom zero padding required for each input buffer */
  int32_t inPadB[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_IN_BUFS];
  /** Number of extra channels required in each input buffer*/
  int32_t inPadCh[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_IN_BUFS];
  /** Element type of each input data buffer  \ref eTIDL_ElementType */
  int32_t rawDataInElementType[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_IN_BUFS];
  /** Element type of each input buffer \ref eTIDL_ElementType */
  int32_t inElementType[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_IN_BUFS];
  /** zero Point of each input data buffer */
  int32_t inZeroPoint[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_IN_BUFS];
  /** Data Layout of each input data buffer \ref eTIDL_TensorLayout */
  int32_t inLayout[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_IN_BUFS];
  /** Data ID as per Net structure for each input buffer */
  int32_t inDataId[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_IN_BUFS];
    /** Tensor scale for input data */
  float32_tidl inTensorScale[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_IN_BUFS];
  /** In Tensor name in the original input networks */
  int8_t inDataName[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_IN_BUFS][TIDL_STRING_SIZE];
  /** Expected input buffer size (elements) of each input */
  int32_t inBufSize[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_IN_BUFS];
  /** Feature width of each output buffer */
  int32_t outWidth[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_OUT_BUFS];
  /** Feature Height of each output buffer */
  int32_t outHeight[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_OUT_BUFS];
  /** DIM2 dimension of each output buffer whose tensor dimesnions are N x Dim1 x Dim2 x C x H x W */
  int32_t outDIM2[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_OUT_BUFS];
  /** DIM1 dimension of each output buffer whose tensor dimesnions are N x Dim1 x Dim2 x C x H x W */
  int32_t outDIM1[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_OUT_BUFS];
  /** Number of channels in each output buffer */
  int32_t outNumChannels[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_OUT_BUFS];
  /** Channel pitch for the output tensor */
  int32_t outChannelPitch[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_OUT_BUFS];
  /** Number of Batches in each output buffer */
  int32_t outNumBatches[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_OUT_BUFS];
  /** Left zero padding required for each output buffer */
  int32_t outPadL[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_OUT_BUFS];
  /** top zero padding required for each output buffer */
  int32_t outPadT[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_OUT_BUFS];
  /** Right zero padding required for each output buffer */
  int32_t outPadR[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_OUT_BUFS];
  /** Bottom zero padding required for each output buffer */
  int32_t outPadB[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_OUT_BUFS];
  /** Number of extra channels required in each output buffer*/
  int32_t outPadCh[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_OUT_BUFS];
  /** Element type of each output buffer \ref eTIDL_ElementType */
  int32_t outElementType[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_OUT_BUFS];
  /** Data ID as per Net structure for each output buffer */
  int32_t outDataId[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_OUT_BUFS];
  /** Out Tensor name in the original input networks */
  int8_t outDataName[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_OUT_BUFS][TIDL_STRING_SIZE];
  /** TensorScale of each input data buffer */
  float32_tidl outTensorScale[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_OUT_BUFS];
  /** zero Point of each input data buffer */
  int32_t outZeroPoint[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_OUT_BUFS];
  /** Data Layout of each input data buffer \ref eTIDL_TensorLayout */
  int32_t outLayout[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_OUT_BUFS];
  /** Expected input buffer size (elements) of each output */
  int32_t outBufSize[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_OUT_BUFS];
  /** Number of valid dimensions in the output tensor (ONNX)*/
  int32_t numValidTensorDims[TIDL_IO_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_OUT_BUFS];
} sTIDL_IOBufDesc_t;


#ifdef __cplusplus
}
#endif

/*@}*/
/* iTI_DL_IO */

#endif /*ITIDL_IO_H_ */
/*==========================================================================*/
/*     END of the FILE                                                      */
/*==========================================================================*/
