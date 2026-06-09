/*
 *
 * Copyright (c) {2015 - 2024} Texas Instruments Incorporated
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
 *  \file tidl_pooling_ref.c
 *
 *  \brief This file defines kernel functions for pooling layer
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include "tidl_alg_utils.h"
#include <limits>
#include <math.h>
#include "tidl_device_functions.h"
#include "tidl_pooling_ref.h"
#include "tidl_forceNegativeTest.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 @struct  TIDL_SpatialPoolingBuffParams_t
 @brief   This structure contains buffer parameters used for transferring input
          from external memory to internal memory for computation and for
          transferring output from internal memory to external memory after
          computation.

 @params  inWidth
          Width of input data pointer in external memory.

 @params  inHeight
          Height of input data pointer in external memory.

 @params  inPitch
          Pitch of input data in external memory.

 @params  outPitch
          Pitch of output data in external memory.

 @params  inChPitch
          Channel Pitch of input data in external memory. It contains the size
          of one complete channel data.

 @params  outChPitch
          Channel Pitch of output data in external memory. It contains the size
          of one complete channel output data.

 @params  scaleValue
          scaleValue used for reciprocal multiplication in Average pooling
          operations.

 @params  roundBits
          Round bits used for final output in pooling operations.

 @params  inElementType
          Indicates the type of input as signed or unsigned.

 @param  scratchSize
         Size of scratch memory.

 @param  scratchMem
         Pointer to the scratch memory.
*/
typedef struct
{
  int32_t inWidth;
  int32_t inHeight;
  int32_t inPitch;
  int32_t outPitch;
  int32_t inChPitch;
  int32_t outChPitch;
  int32_t scaleValue;
  int32_t biasTerm;
  uint8_t mmaShift;
  int32_t roundBits;
  int32_t inElementType;
  int32_t outElementType;
  int32_t scratchSize;
  void *scratchMem;
} TIDL_SpatialPoolingBuffParams_t;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                  Internal/Private Function Declarations                    */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

int32_t TIDL_getOTFOffset(sTIDL_Network_t *net,
                          const sTIDL_AlgLayer_t *algLayer,
                          sTIDL_DataParams_t *inDataParams)
{
  int32_t offsetOTF = 0;
  offsetOTF = ((algLayer->layerParams.poolParams.virtualPad.padT * inDataParams->pitch[TIDL_LINE_PITCH]) + algLayer->layerParams.poolParams.virtualPad.padL);
  return offsetOTF;
}

template<class Tin> void TIDL_adjustOTFinPtr(Tin **pInChannel,
                                             sTIDL_Network_t *net,
                                             const sTIDL_AlgLayer_t *algLayer,
                                             sTIDL_DataParams_t *inDataParams)
{
  if (TIDL_isPadOTF(net->deviceName) == TRUE)
  {
    (*pInChannel) = (*pInChannel) - (((algLayer->layerParams.poolParams.virtualPad.padT * inDataParams->pitch[TIDL_LINE_PITCH]) + algLayer->layerParams.poolParams.virtualPad.padL));
  }
}
#if defined TIDL_COVERAGE_DEAD_CODE
/**
 * @brief  This is the reference implementation for global max pooling
 *
 * @param pInChannel : pointer to the input data buffer
 * @param width  : width of the input data buffer
 * @param height : height of the input data buffer
 * @param inPitch : pitch of the input data buffer
 * @param inChPitch : Channel pitch of the input data buffer
 * @param outChPitch : Channel pitch of the output data buffer
 * @param numOutChannels : no of output channels
 * @param initValue : initial value for Max operation
 * @param pOutChannel : Pointer to output data buffer
 * @param tidlLayer : Pointer to the common layer parameters
 */
template<class Tin> static void TIDL_refGlobalMaxPooling(Tin *pInChannel,
                                                         int32_t width,
                                                         int32_t height,
                                                         int32_t inPitch,
                                                         int32_t inChPitch,
                                                         int32_t outChPitch,
                                                         int32_t numOutChannels,
                                                         Tin *pOutChannel,
                                                         const sTIDL_Layer_t *tidlLayer,
                                                         sTIDL_DataParams_t *inDataParams,
                                                         sTIDL_Network_t *net,
                                                         const sTIDL_AlgLayer_t *algLayer)
{
  Tin input;
  Tin maxValue;
  int32_t i0, i1, i2, i3;
  int32_t numBatches = (int32_t)inDataParams->dimValues[TIDL_DIM_BATCH];
  int32_t inBatchPitch = inDataParams->pitch[TIDL_ROI_PITCH];
  int32_t outBatchPitch = tidlLayer->outData.pitch[TIDL_ROI_PITCH];
  Tin *inData = (Tin *)pInChannel;
  Tin *outData = (Tin *)pOutChannel;
  Tin initValue = std::numeric_limits<Tin>::lowest();
  int32_t offsetOTF = TIDL_getOTFOffset(net, algLayer, inDataParams);
  OPENACC(data copyin(inData [0:1 + offsetOTF + ((numBatches - 1) * inBatchPitch) + ((numOutChannels - 1) * inChPitch) + ((height - 1) * inPitch) + (width - 1)])
              copyout(outData [0:1 + ((numBatches - 1) * outBatchPitch) + (numOutChannels - 1) * outChPitch]))
  {

    TIDL_adjustOTFinPtr(&inData, net, algLayer, inDataParams);

    OPENACC(parallel loop collapse(2))
    for (i0 = 0; i0 < numBatches; i0++)
    {
      for (i1 = 0; i1 < numOutChannels; i1++)
      {
        maxValue = (Tin)initValue;
        OPENACC(loop collapse(2) reduction(max : maxValue))
        for (i2 = 0; i2 < height; i2++)
        {
          for (i3 = 0; i3 < width; i3++)
          {
            input = inData[(i0 * inBatchPitch) + (i1 * inChPitch) + (i2 * inPitch) + i3];
            /* Max pooling operations */
            maxValue = (maxValue > input) ? maxValue : input;
          }
        }
        outData[(i0 * outBatchPitch) + i1 * outChPitch] = maxValue;
      }
    }
  }
}
#endif

template<class Tin, class Tout, class Tacc, class Tscale> static int32_t TIDL_refGlobalAvgPoolingv2(sTIDL_Network_t *net,
                                                                                                    Tin *pInChannel,
                                                                                                    int32_t width,
                                                                                                    int32_t height,
                                                                                                    int32_t inPitch,
                                                                                                    int32_t inChPitch,
                                                                                                    int32_t outChPitch,
                                                                                                    int32_t numOutChannels,
                                                                                                    Tout *pOutChannel,
                                                                                                    Tacc *accPtr,
                                                                                                    Tscale scaleValue,
                                                                                                    int32_t biasTerm,
                                                                                                    uint8_t mmaShift,
                                                                                                    const sTIDL_AlgLayer_t *algLayer,
                                                                                                    const sTIDL_Layer_t *tidlLayer,
                                                                                                    sTIDL_DataParams_t *inDataParams)
{
  Tacc inRowCol;
  int32_t i0, i1, i2, i3;
  int32_t numBatches = (int32_t)inDataParams->dimValues[TIDL_DIM_BATCH];
  uint32_t inBatchPitch = (uint32_t)inDataParams->pitch[TIDL_ROI_PITCH];
  uint32_t outBatchPitch = (uint32_t)tidlLayer->outData.pitch[TIDL_ROI_PITCH];
  Tacc sumBlock;
  Tacc result = 0;
  Tin *inData = (Tin *)pInChannel;
  Tout *outData = (Tout *)pOutChannel;

  int32_t status = TIDL_SUCCESS;
  int32_t layerIdx = algLayer->layerIdx;
  uint32_t accumulatorSize = (uint32_t)(sizeof(Tacc));
  int32_t accMemSize = numBatches * outBatchPitch * (int32_t)accumulatorSize;

  Tacc min = 0, max = 0;

  max = std::numeric_limits<Tacc>::lowest();
  min = std::numeric_limits<Tacc>::max();

  if (algLayer->scratchSize >= accMemSize)
  {
    accPtr = (Tacc *)algLayer->scratchMem;
  }
  else
  {
    tidl_printf(0, "Memory for  TIDL_refGlobalAvgPooling accumulator is not sufficient exiting...\n");
    status = TIDL_ERR_FAILURE;
  }

  int32_t offsetOTF = TIDL_getOTFOffset(net, algLayer, inDataParams);

  OPENACC(data copyin(inData [0:1 + offsetOTF + ((numBatches - 1) * inBatchPitch) + ((numOutChannels - 1) * inChPitch) + ((height - 1) * inPitch) + (width - 1)])
              present(accPtr [0:1 + ((numBatches - 1) * outBatchPitch) + ((numOutChannels - 1) * outChPitch)])
                  copyout(outData [0:1 + ((numBatches - 1) * outBatchPitch) + ((numOutChannels - 1) * outChPitch)]))
  {

    TIDL_adjustOTFinPtr(&inData, net, algLayer, inDataParams);

    if (TIDL_SUCCESS == status)
    {
      OPENACC(parallel loop collapse(2))
      for (i0 = 0; i0 < numBatches; i0++)
      {
        for (i1 = 0; i1 < numOutChannels; i1++)
        {
          sumBlock = 0;
          OPENACC(loop collapse(2) reduction(+ : sumBlock))
          for (i2 = 0; i2 < height; i2++)
          {
            for (i3 = 0; i3 < width; i3++)
            {
              inRowCol = (Tacc)inData[(i0 * inBatchPitch) + (i1 * inChPitch) + (i2 * inPitch) + i3];
              sumBlock += inRowCol;
            }
          }
          sumBlock += biasTerm;
          result = sumBlock * scaleValue;
          min = (result < min) ? result : min;
          max = (result > max) ? result : max;
          accPtr[(i0 * outBatchPitch) + (i1 * outChPitch)] = result;
        }
      }

      int32_t satLow;
      int32_t satHigh;
      int32_t mixedPrecision = 0;
      Tout temp = 0;
      temp = std::numeric_limits<Tout>::lowest();
      satLow = (int32_t)temp;
      temp = std::numeric_limits<Tout>::max();
      satHigh = (int32_t)(temp);
      int64_t tempAcc;

      OPENACC(parallel loop collapse(2))
      for (i0 = 0; i0 < numBatches; i0++)
      {
        for (i1 = 0; i1 < numOutChannels; i1++)
        {
          result = accPtr[(i0 * outBatchPitch) + (i1 * outChPitch)];
// if (typeid(Tacc) == typeid(float32_tidl))
//  For global average pool with AsymQuantEnabledTFL has Tacc as int32
#if defined TIDL_COVERAGE_DEAD_CODE
          if (std::is_same<Tacc, float32_tidl>::value)
          {
            OPENACC(routine(TIDL_floatSat))
            result = TIDL_floatSat(result, &net->TIDLLayers[layerIdx]);
          }
          else
#endif
          {
            tempAcc = result;
            /* Rounding and saturation */
            OPENACC(routine(TIDL_roundSat))
            result = (Tacc)TIDL_roundSat((int64_t)tempAcc, mmaShift, (int32_t)satLow, (int32_t)satHigh);
#ifdef GLOBAL_AVG_POOL_MIXED_PRECISION_ENABLE
            if (mixedPrecision == 1)
            {
              result = (uint64_t)result >> 8U;
            }
#endif
          }
          outData[(i0 * outBatchPitch) + (i1 * outChPitch)] = (Tout)result;
        }
      }
    }
  }
  return status;
}

/**
 * @brief  This is the reference implementation for global average pooling
 *
 * @param intAlgHandle : tidl algorothm handle
 * @param pInChannel : pointer to the input data buffer
 * @param width  : width of the input data buffer
 * @param height : height of the input data buffer
 * @param inPitch : pitch of the input data buffer
 * @param inChPitch : Channel pitch of the input data buffer
 * @param outChPitch : Channel pitch of the output data buffer
 * @param numOutChannels : no of output channels
 * @param pOutChannel : Pointer to output data buffer
 * @param scaleValue : the scale value for the input
 * @param satLow : Min value for the saturation
 * @param satHigh : Max value for the saturation
 * @param algLayer : Pointer to the layer specific parameters
 * @param tidlLayer : Pointer to the common layer parameters
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
template<class Tin, class Tout, class Tacc, class Tscale> static int32_t TIDL_refGlobalAvgPooling(sTIDL_Network_t *net,
                                                                                                  Tin *pInChannel,
                                                                                                  int32_t width,
                                                                                                  int32_t height,
                                                                                                  int32_t inPitch,
                                                                                                  int32_t inChPitch,
                                                                                                  int32_t outChPitch,
                                                                                                  int32_t numOutChannels,
                                                                                                  Tout *pOutChannel,
                                                                                                  Tacc *accPtr,
                                                                                                  Tscale scaleValue,
                                                                                                  const sTIDL_AlgLayer_t *algLayer,
                                                                                                  const sTIDL_Layer_t *tidlLayer,
                                                                                                  sTIDL_DataParams_t *inDataParams)
{
  Tacc inRowCol;
  int32_t i0, i1, i2, i3;
  int32_t numBatches = (int32_t)inDataParams->dimValues[TIDL_DIM_BATCH];
  int32_t inBatchPitch = inDataParams->pitch[TIDL_ROI_PITCH];
  int32_t outBatchPitch = tidlLayer->outData.pitch[TIDL_ROI_PITCH];
  Tacc sumBlock;
  Tacc result = 0;
  Tin *inData = (Tin *)pInChannel;
  Tout *outData = (Tout *)pOutChannel;

  int32_t status = TIDL_SUCCESS;
  int32_t layerIdx = algLayer->layerIdx;
  int32_t roundBits;
  uint32_t accumulatorSize = (uint32_t)(sizeof(Tacc));
  int32_t accMemSize = numBatches * outBatchPitch * (int32_t)accumulatorSize;

  Tacc min = 0, max = 0;

  max = std::numeric_limits<Tacc>::lowest();
  min = std::numeric_limits<Tacc>::max();

  if (algLayer->scratchSize >= accMemSize)
  {
    accPtr = (Tacc *)algLayer->scratchMem;
  }
  else
  {
    tidl_printf(0, "Memory for  TIDL_refGlobalAvgPooling accumulator is not sufficient exiting...\n");
    status = TIDL_ERR_FAILURE;
  }

  int32_t offsetOTF = TIDL_getOTFOffset(net, algLayer, inDataParams);

  OPENACC(data copyin(inData [0:1 + offsetOTF + ((numBatches - 1) * inBatchPitch) + ((numOutChannels - 1) * inChPitch) + ((height - 1) * inPitch) + (width - 1)])
              present(accPtr [0:1 + ((numBatches - 1) * outBatchPitch) + ((numOutChannels - 1) * outChPitch)])
                  copyout(outData [0:1 + ((numBatches - 1) * outBatchPitch) + ((numOutChannels - 1) * outChPitch)]))
  {

    TIDL_adjustOTFinPtr(&inData, net, algLayer, inDataParams);

    if (TIDL_SUCCESS == status)
    {
      OPENACC(parallel loop collapse(2) reduction(min : min) reduction(max : max))
      for (i0 = 0; i0 < numBatches; i0++)
      {
        for (i1 = 0; i1 < numOutChannels; i1++)
        {
          sumBlock = 0;
          OPENACC(loop collapse(2) reduction(+ : sumBlock))
          for (i2 = 0; i2 < height; i2++)
          {
            for (i3 = 0; i3 < width; i3++)
            {
              inRowCol = (Tacc)inData[(i0 * inBatchPitch) + (i1 * inChPitch) + (i2 * inPitch) + i3];
              sumBlock += inRowCol;
            }
          }
          result = sumBlock * scaleValue;
          min = (result < min) ? result : min;
          max = (result > max) ? result : max;
          accPtr[(i0 * outBatchPitch) + (i1 * outChPitch)] = result;
        }
      }
    }

    int32_t satLow;
    int32_t satHigh;
    int32_t mixedPrecision = 0;
    Tout temp = 0;

    temp = std::numeric_limits<Tout>::lowest();
    satLow = (int32_t)temp;
    temp = std::numeric_limits<Tout>::max();
    satHigh = (int32_t)(temp);

    roundBits = net->TIDLLayers[layerIdx].outData.roundBits;

#if GLOBAL_AVG_POOL_MIXED_PRECISION_ENABLE
    int32_t procElemSize;
    procElemSize = TIDL_getProcessingElementSizeInBytes(tidlLayer);
    if (sizeof(Tout) != (uint32_t)procElemSize)
    {
      mixedPrecision = 1;
      /* This is to match with target implementation wherin the saturations
      happens after applying outRoundBits - 8 shift and then the right shift
      of 8 is applied */
      roundBits = roundBits - 8;
      if (net->TIDLLayers[layerIdx].outData.elementType == TIDL_SignedChar)
      {
        satLow = std::numeric_limits<int16_t>::lowest();
        satHigh = std::numeric_limits<int16_t>::max();
      }
      else
      {
        satLow = std::numeric_limits<uint16_t>::lowest();
        satHigh = std::numeric_limits<uint16_t>::max();
      }
    }
#endif

    if (TIDL_SUCCESS == status)
    {
      OPENACC(parallel loop collapse(2))
      for (i0 = 0; i0 < numBatches; i0++)
      {
        for (i1 = 0; i1 < numOutChannels; i1++)
        {
          result = accPtr[(i0 * outBatchPitch) + (i1 * outChPitch)];
          if (std::is_same<Tacc, float32_tidl>::value)
          {
            OPENACC(routine(TIDL_floatSat))
            result = TIDL_floatSat(result, &net->TIDLLayers[layerIdx]);
          }
          else
          {
            /* Rounding and saturation */
            OPENACC(routine(TIDL_roundSat))
            result = (Tacc)TIDL_roundSat((int64_t)result, roundBits, (int32_t)satLow, (int32_t)satHigh);
#ifdef GLOBAL_AVG_POOL_MIXED_PRECISION_ENABLE
            if (mixedPrecision == 1)
            {
              result = (uint64_t)result >> 8U;
            }
#endif
          }
          outData[(i0 * outBatchPitch) + (i1 * outChPitch)] = (Tout)result;
        }
      }
    }
  }

  return status;
}

/**
 * @brief Global pooling layer reference implementation
 *
 * @param intAlgHandle : tidl algorothm handle
 * @param tidlLayer : Pointer to the common layer parameters
 * @param algLayer : Pointer to the layer specific parameters
 * @param params : copy of pooling layer parameters
 * @param inPtr : Pointer to input buffers to be processed
 * @param outPtr : Pointer to output buffers to be processed
 * @param poolingBuffParams : parameters of the pooling buffer
 * @param numBatches : number of total batches
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
static int32_t TIDL_refGlobalPoolingProcess(sTIDL_Network_t *net,
                                            sTIDL_Layer_t *tidlLayer,
                                            sTIDL_DataParams_t *inDataParams,
                                            const sTIDL_AlgLayer_t *algLayer,
                                            const sTIDL_PoolingParams_t *params,
                                            void *inPtr,
                                            void *outPtr,
                                            const TIDL_SpatialPoolingBuffParams_t *poolingBuffParams,
                                            int32_t numBatches)
{
  int32_t status = TIDL_SUCCESS;
  int32_t quantizationStyle = net->quantizationStyle;
#if defined TIDL_COVERAGE_DEAD_CODE
  if (TIDL_MaxPooling == params->poolingType)
  {
    if (poolingBuffParams->inElementType == TIDL_UnsignedChar)
    {
      TIDL_refGlobalMaxPooling((uint8_t *)inPtr,
                               poolingBuffParams->inWidth,
                               poolingBuffParams->inHeight,
                               poolingBuffParams->inPitch,
                               poolingBuffParams->inChPitch,
                               poolingBuffParams->outChPitch,
                               params->numChannels,
                               (uint8_t *)outPtr,
                               tidlLayer, inDataParams,
                               net, algLayer);
    }
    else if (poolingBuffParams->inElementType == TIDL_SignedChar)
    {
      TIDL_refGlobalMaxPooling((int8_t *)inPtr,
                               poolingBuffParams->inWidth,
                               poolingBuffParams->inHeight,
                               poolingBuffParams->inPitch,
                               poolingBuffParams->inChPitch,
                               poolingBuffParams->outChPitch,
                               params->numChannels,
                               (int8_t *)outPtr,
                               tidlLayer, inDataParams,
                               net, algLayer);
    }
    else if (poolingBuffParams->inElementType == TIDL_UnsignedShort)
    {
      TIDL_refGlobalMaxPooling((uint16_t *)inPtr,
                               poolingBuffParams->inWidth,
                               poolingBuffParams->inHeight,
                               poolingBuffParams->inPitch,
                               poolingBuffParams->inChPitch,
                               poolingBuffParams->outChPitch,
                               params->numChannels,
                               (uint16_t *)outPtr,
                               tidlLayer, inDataParams,
                               net, algLayer);
    }
    else if (poolingBuffParams->inElementType == TIDL_SignedShort)
    {
      TIDL_refGlobalMaxPooling((int16_t *)inPtr,
                               poolingBuffParams->inWidth,
                               poolingBuffParams->inHeight,
                               poolingBuffParams->inPitch,
                               poolingBuffParams->inChPitch,
                               poolingBuffParams->outChPitch,
                               params->numChannels,
                               (int16_t *)outPtr,
                               tidlLayer, inDataParams,
                               net, algLayer);
    }
    else if (poolingBuffParams->inElementType == TIDL_SinglePrecFloat)
    {
      TIDL_refGlobalMaxPooling((float32_tidl *)inPtr,
                               poolingBuffParams->inWidth,
                               poolingBuffParams->inHeight,
                               poolingBuffParams->inPitch,
                               poolingBuffParams->inChPitch,
                               poolingBuffParams->outChPitch,
                               params->numChannels,
                               (float32_tidl *)outPtr,
                               tidlLayer, inDataParams,
                               net, algLayer);
    }
    else
    {
      status = TIDL_ERR_FAILURE;
    }
  }
  else
#endif
      if (TIDL_AveragePooling == params->poolingType)
  {
    void *accPtr = NULL;
    if (TIDL_isAsymQuantEnabledTFL(quantizationStyle) != FALSE)
    {
      status = TIDL_refGlobalAvgPoolingv2(net,
                                          (int8_t *)inPtr,
                                          poolingBuffParams->inWidth,
                                          poolingBuffParams->inHeight,
                                          poolingBuffParams->inPitch,
                                          poolingBuffParams->inChPitch,
                                          poolingBuffParams->outChPitch,
                                          params->numChannels,
                                          (int8_t *)outPtr,
                                          (int32_t *)accPtr,
                                          (uint8_t)poolingBuffParams->scaleValue,
                                          (int32_t)poolingBuffParams->biasTerm,
                                          (uint8_t)poolingBuffParams->mmaShift,
                                          algLayer,
                                          tidlLayer, inDataParams);
    }
    else
    {
      if (poolingBuffParams->inElementType == TIDL_UnsignedChar)
      {
        if (poolingBuffParams->outElementType == TIDL_UnsignedChar)
        {
          status = TIDL_refGlobalAvgPooling(net,
                                            (uint8_t *)inPtr,
                                            poolingBuffParams->inWidth,
                                            poolingBuffParams->inHeight,
                                            poolingBuffParams->inPitch,
                                            poolingBuffParams->inChPitch,
                                            poolingBuffParams->outChPitch,
                                            params->numChannels,
                                            (uint8_t *)outPtr,
                                            (uint32_t *)accPtr,
                                            (uint32_t)poolingBuffParams->scaleValue,
                                            algLayer,
                                            tidlLayer, inDataParams);
        }
        else if (poolingBuffParams->outElementType == TIDL_UnsignedShort)
        {
          status = TIDL_refGlobalAvgPooling(net,
                                            (uint8_t *)inPtr,
                                            poolingBuffParams->inWidth,
                                            poolingBuffParams->inHeight,
                                            poolingBuffParams->inPitch,
                                            poolingBuffParams->inChPitch,
                                            poolingBuffParams->outChPitch,
                                            params->numChannels,
                                            (uint16_t *)outPtr,
                                            (uint64_t *)accPtr,
                                            (uint64_t)poolingBuffParams->scaleValue,
                                            algLayer,
                                            tidlLayer, inDataParams);
        }
        else
        {
          status = TIDL_ERR_FAILURE;
        }
      }
      else if (poolingBuffParams->inElementType == TIDL_SignedChar)
      {
        if (poolingBuffParams->outElementType == TIDL_SignedChar)
        {
          status = TIDL_refGlobalAvgPooling(net,
                                            (int8_t *)inPtr,
                                            poolingBuffParams->inWidth,
                                            poolingBuffParams->inHeight,
                                            poolingBuffParams->inPitch,
                                            poolingBuffParams->inChPitch,
                                            poolingBuffParams->outChPitch,
                                            params->numChannels,
                                            (int8_t *)outPtr,
                                            (int32_t *)accPtr,
                                            (int32_t)poolingBuffParams->scaleValue,
                                            algLayer,
                                            tidlLayer, inDataParams);
        }
        else if (poolingBuffParams->outElementType == TIDL_SignedShort)
        {
          status = TIDL_refGlobalAvgPooling(net,
                                            (int8_t *)inPtr,
                                            poolingBuffParams->inWidth,
                                            poolingBuffParams->inHeight,
                                            poolingBuffParams->inPitch,
                                            poolingBuffParams->inChPitch,
                                            poolingBuffParams->outChPitch,
                                            params->numChannels,
                                            (int16_t *)outPtr,
                                            (int64_t *)accPtr,
                                            (int64_t)poolingBuffParams->scaleValue,
                                            algLayer,
                                            tidlLayer, inDataParams);
        }
        else
        {
          status = TIDL_ERR_FAILURE;
        }
      }
      else if (poolingBuffParams->inElementType == TIDL_UnsignedShort)
      {
        if (poolingBuffParams->outElementType == TIDL_UnsignedShort)
        {
          status = TIDL_refGlobalAvgPooling(net,
                                            (uint16_t *)inPtr,
                                            poolingBuffParams->inWidth,
                                            poolingBuffParams->inHeight,
                                            poolingBuffParams->inPitch,
                                            poolingBuffParams->inChPitch,
                                            poolingBuffParams->outChPitch,
                                            params->numChannels,
                                            (uint16_t *)outPtr,
                                            (uint64_t *)accPtr,
                                            (uint64_t)poolingBuffParams->scaleValue,
                                            algLayer,
                                            tidlLayer, inDataParams);
        }
        else
        {
          status = TIDL_ERR_FAILURE;
        }
      }
      else if (poolingBuffParams->inElementType == TIDL_SignedShort)
      {
        if (poolingBuffParams->outElementType == TIDL_SignedShort)
        {
          status = TIDL_refGlobalAvgPooling(net,
                                            (int16_t *)inPtr,
                                            poolingBuffParams->inWidth,
                                            poolingBuffParams->inHeight,
                                            poolingBuffParams->inPitch,
                                            poolingBuffParams->inChPitch,
                                            poolingBuffParams->outChPitch,
                                            params->numChannels,
                                            (int16_t *)outPtr,
                                            (int64_t *)accPtr,
                                            (int64_t)poolingBuffParams->scaleValue,
                                            algLayer,
                                            tidlLayer, inDataParams);
        }
        else
        {
          status = TIDL_ERR_FAILURE;
        }
      }
      else if (poolingBuffParams->inElementType == TIDL_SinglePrecFloat)
      {
        float32_tidl scaleValue = (float32_tidl)(1.0) / ((float32_tidl)poolingBuffParams->inWidth * (float32_tidl)poolingBuffParams->inHeight);
        status = TIDL_refGlobalAvgPooling(net,
                                          (float32_tidl *)inPtr,
                                          poolingBuffParams->inWidth,
                                          poolingBuffParams->inHeight,
                                          poolingBuffParams->inPitch,
                                          poolingBuffParams->inChPitch,
                                          poolingBuffParams->outChPitch,
                                          params->numChannels,
                                          (float32_tidl *)outPtr,
                                          (float32_tidl *)accPtr,
                                          scaleValue,
                                          algLayer,
                                          tidlLayer, inDataParams);
      }
      else
      {
        /*do nothing*/
      }
    }
  }
  else
  {
    status = TIDL_ERR_FAILURE;
  }
  TIDL_L1DandL2CacheWbInv();
  return status;
}

/**
 * @brief  This is the reference implementation for spatial max pooling
 *
 * @param net : the network structure
 * @param pInChannel : pointer to the input data buffer
 * @param width  : width of the input data buffer
 * @param height : height of the input data buffer
 * @param inPitch : pitch of the input data buffer
 * @param inChPitch : Channel pitch of the input data buffer
 * @param outChPitch : Channel pitch of the output data buffer
 * @param numOutChannels : no of output channels
 * @param kernelW : width of the kernel
 * @param kernelH : height of the kernel
 * @param strideW : stride in width dimension
 * @param strideH : stride in height dimension
 * @param padW : Pad in width dimension
 * @param padH : Pad in height dimension
 * @param initValue : initial value for Max operation
 * @param pOutChannel : Pointer to output data buffer
 * @param tidlLayer : Pointer to the common layer parameters
 */
template<class Tin> static void TIDL_refSpatialMaxPooling(sTIDL_Network_t *net,
                                                          Tin *pInChannel,
                                                          int32_t startRowNumberInTensor,
                                                          int32_t orgInTensorHeight,
                                                          int32_t width,
                                                          int32_t height,
                                                          int32_t inPitch,
                                                          int32_t inChPitch,
                                                          int32_t outChPitch,
                                                          int32_t numOutChannels,
                                                          int32_t kernelW,
                                                          int32_t kernelH,
                                                          int32_t strideW,
                                                          int32_t strideH,
                                                          int32_t padW,
                                                          int32_t padH,
                                                          Tin *pOutChannel,
                                                          const sTIDL_Layer_t *tidlLayer,
                                                          sTIDL_DataParams_t *inDataParams,
                                                          const sTIDL_AlgLayer_t *algLayer)
{
  Tin inRowCol;
  int32_t i0, i1, i2, i3, i4, i5;
  Tin maxValue = 0;
  int32_t outPitch = tidlLayer->outData.pitch[TIDL_LINE_PITCH];
  int32_t numBatches = (int32_t)inDataParams->dimValues[TIDL_DIM_BATCH];
  uint32_t inBatchPitch = (uint32_t)inDataParams->pitch[TIDL_ROI_PITCH];
  uint32_t outBatchPitch = (uint32_t)tidlLayer->outData.pitch[TIDL_ROI_PITCH];
  Tin *inData = (Tin *)pInChannel;
  Tin *outData = (Tin *)pOutChannel;

  Tin initValue = 0;
  initValue = std::numeric_limits<Tin>::lowest();

  int32_t isBorderPixel;
  int32_t validPosXMin, validPosXMax, validPosYMin, validPosYMax;
  int32_t batchOffset, channelOffset, spatialOffsetY, spatialOffsetX;

  // For float pass we are making assumptions of padded buffer whereas fixed pass uses OTF pad.
  if (inDataParams->elementType == TIDL_SinglePrecFloat)
  {
    validPosXMin = tidlLayer->layerParams.poolParams.padL;
    validPosXMax = tidlLayer->layerParams.poolParams.padL + width;
    validPosYMin = tidlLayer->layerParams.poolParams.padT;
    validPosYMax = tidlLayer->layerParams.poolParams.padT + orgInTensorHeight;
  }
  else
  {
    validPosXMin = padW;
    validPosXMax = padW + width;
    validPosYMin = padH;
    validPosYMax = padH + orgInTensorHeight;
  }

  int32_t numRows, numCols;
  numRows = tidlLayer->outData.dimValues[TIDL_DIM_HEIGHT];
  numCols = tidlLayer->outData.dimValues[TIDL_DIM_WIDTH];

  int32_t offsetOTF = TIDL_getOTFOffset(net, algLayer, inDataParams);

  //OPENACC(data copyin(inData[0:1 + offsetOTF + (numBatches-1)*inBatchPitch + (numOutChannels-1)*inChPitch + ((numRows-1)*strideH) + (kernelH-1) + ((numCols-1)*strideW) + (kernelW-1)]) \
    //           copy(outData[0:1 + ((numBatches-1)*outBatchPitch) + ((numOutChannels-1)*outChPitch) + ((numRows-1) * outPitch) + (numCols-1)]))
  {

    if (TIDL_isPadOTF(net->deviceName) == TRUE)
    {
      /* TIDL_adjustOTFinPtr(&inData, net, algLayer, inDataParams);
      Adjust input data pointer for OTF padding
      not calling adjustOTFinPtr because we have already checked the condition for isPadOTF and don't want to check it again inside adjustOTFinPtr function.
      */
      inData = inData - (((algLayer->layerParams.poolParams.virtualPad.padT * inDataParams->pitch[TIDL_LINE_PITCH]) + algLayer->layerParams.poolParams.virtualPad.padL));
    }
    else
    {
      /*
     input is padded with padH and padW, for the case where padT and padL are zero we have to move input pointer to valid position.

     input data padded with padH and padW                input ptr position is determined by the padT and padL not by padH and padW,
                                                         we have to move input ptr to the correct position, so we have to jump (padH-padT)
                                                         amount on height and (padW-padL) amount width wise.

     ex: padH = 2, padW = 2                               padT = 1, padL = 1

           inPtr-> ptr 0  0  0  0  0  0                           0   0   0  0  0  0  0
                   0   0  0  0  0  0  0                           0   ptr 0  0  0  0  0
                   0   0  x  x  x  x  x                           0   0   x  x  x  x  x
                   0   0  x  x  x  x  x                           0   0   x  x  x  x  x
                   0   0  x  x  x  x  x                           0   0   x  x  x  x  x

     */
      // move this function into adjustOTFFinPtr function after testing average poolqq
      inData = inData + (((padH - tidlLayer->layerParams.poolParams.padT) * inDataParams->pitch[TIDL_LINE_PITCH]) + (padW - tidlLayer->layerParams.poolParams.padL));
    }

    // OPENACC(parallel loop collapse(4))
    for (i0 = 0; i0 < numBatches; i0++)
    {
      for (i1 = 0; i1 < numOutChannels; i1++)
      {
        for (i2 = 0; i2 < numRows; i2++)
        {
          for (i3 = 0; i3 < numCols; i3++)
          {
            maxValue = initValue;

            // OPENACC(loop seq)
            for (i4 = 0; i4 < kernelH; i4++)
            {
              // OPENACC(loop seq)
              for (i5 = 0; i5 < kernelW; i5++)
              {
                // For better readability, we segregate the below formula of inRowCol calculation into respestive spatial dimensions
                // channelOffset - for pixels in Channel dimesion
                // spatialOffsetY - for offset in height dimension,this is not multiplying with inPitch.
                // spatialOffsetX - for offset in widhth dimension
                // inRowCol = inData[(i1*inChPitch) + (((i2*strideH) + i4) * inPitch) + (i3*strideW) + i5];
                batchOffset = i0 * inBatchPitch;
                channelOffset = i1 * inChPitch;
                spatialOffsetY = (i2 * strideH) + i4;
                spatialOffsetX = (i3 * strideW) + i5;
                // OPENACC(routine(TIDL_checkPixelInPadRegion))
                // checking if the current pixels falls in borders
                isBorderPixel = TIDL_checkPixelInPadRegion((spatialOffsetY + startRowNumberInTensor), spatialOffsetX, validPosXMin, validPosXMax, validPosYMin, validPosYMax);
                /*Over write the boarder pixel to not influence the operation */
                if (isBorderPixel != 0)
                {
                  inRowCol = initValue;
                }
                else
                {

                  inRowCol = inData[batchOffset + channelOffset + (spatialOffsetY * inPitch) + spatialOffsetX];
                }
                /* Max pooling operation */
                maxValue = (maxValue > inRowCol) ? maxValue : inRowCol;
              }
            }
            outData[(i0 * outBatchPitch) + (i1 * outChPitch) + (i2 * outPitch) + i3] = (Tin)maxValue;
          }
        }
      }
    }
  }
}

/**
 * @brief  This is the reference implementation for spatial average pooling
 *
 * @param intAlgHandle : tidl algorothm handle
 * @param pInChannel : pointer to the input data buffer
 * @param width  : width of the input data buffer
 * @param height : height of the input data buffer
 * @param inPitch : pitch of the input data buffer
 * @param inChPitch : Channel pitch of the input data buffer
 * @param outChPitch : Channel pitch of the output data buffer
 * @param numOutChannels : no of output channels
 * @param kernelW : width of the kernel
 * @param kernelH : height of the kernel
 * @param strideW : stride in width dimension
 * @param strideH : stride in height dimension
 * @param padW : Pad in width dimension
 * @param padH : Pad in height dimension
 * @param pOutChannel : Pointer to output data buffer
 * @param satLow : Min value for the saturation
 * @param satHigh : Max value for the saturation
 * @param algLayer : Pointer to the layer specific parameters
 * @param tidlLayer : Pointer to the common layer parameters
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
template<class Tin, class Tout, class Tacc> static int32_t TIDL_refSpatialAvgPooling(sTIDL_Network_t *net,
                                                                                     Tin *pInChannel,
                                                                                     int32_t width,
                                                                                     int32_t height,
                                                                                     int32_t inPitch,
                                                                                     int32_t inChPitch,
                                                                                     int32_t outChPitch,
                                                                                     int32_t numOutChannels,
                                                                                     int32_t kernelW,
                                                                                     int32_t kernelH,
                                                                                     int32_t strideW,
                                                                                     int32_t strideH,
                                                                                     int32_t padW,
                                                                                     int32_t padH,
                                                                                     Tout *pOutChannel,
                                                                                     Tacc *accPtr,
                                                                                     const sTIDL_AlgLayer_t *algLayer,
                                                                                     const sTIDL_Layer_t *tidlLayer,
                                                                                     sTIDL_DataParams_t *inDataParams)
{
  Tacc inRowCol;
  int32_t i0, i1, i2, i3, i4, i5;
  Tacc sumBlock;
  Tacc result = 0;
  Tacc scaleValue;
  int32_t outPitch = tidlLayer->outData.pitch[TIDL_LINE_PITCH];
  int32_t numBatches = (int32_t)inDataParams->dimValues[TIDL_DIM_BATCH];
  uint32_t inBatchPitch = (uint32_t)inDataParams->pitch[TIDL_ROI_PITCH];
  uint32_t outBatchPitch = (uint32_t)tidlLayer->outData.pitch[TIDL_ROI_PITCH];
  Tin *inData = (Tin *)pInChannel;
  Tout *outData = (Tout *)pOutChannel;
  int32_t numRows, numCols, roundVal;

  int32_t isOTFpad = 0;
  int32_t startRowNumberInTensor = algLayer->layerParams.poolParams.startRowNumberInTensor;

  if (TIDL_isPadOTF(net->deviceName) != 0U)
  {
    isOTFpad = 1;
  }

  int32_t isBorderPixel;
  int32_t validPosXMin, validPosXMax, validPosYMin, validPosYMax;
  int32_t spatialOffsetY, spatialOffsetX;

  // For float pass we are making assumptions of padded buffer whereas fixed pass uses OTF pad.
  if (inDataParams->elementType == TIDL_SinglePrecFloat)
  {
    validPosXMin = tidlLayer->layerParams.poolParams.padL;
    validPosXMax = tidlLayer->layerParams.poolParams.padL + width;
    validPosYMin = tidlLayer->layerParams.poolParams.padT;
    validPosYMax = tidlLayer->layerParams.poolParams.padT + algLayer->layerParams.poolParams.orgInTensorHeight;
  }
  else
  {
    validPosXMin = padW;
    validPosXMax = padW + width;
    validPosYMin = padH;
    validPosYMax = padH + algLayer->layerParams.poolParams.orgInTensorHeight;
  }

  int32_t status = TIDL_SUCCESS;
  int32_t layerIdx = algLayer->layerIdx;

  int32_t roundBits;
  uint32_t accumulatorSize = (uint32_t)(sizeof(Tacc));
  int32_t accMemSize = numBatches * outBatchPitch * (int32_t)accumulatorSize;

  Tacc min = 0, max = 0;

  max = std::numeric_limits<Tacc>::lowest();
  min = std::numeric_limits<Tacc>::max();

  if (algLayer->scratchSize >= accMemSize)
  {
    accPtr = (Tacc *)algLayer->scratchMem;
  }
  else
  {
    tidl_printf(0, "Memory for  TIDL_refSpatialAvgPooling accumulator is not sufficient exiting...\n");
    status = TIDL_ERR_FAILURE;
  }

  numRows = tidlLayer->outData.dimValues[TIDL_DIM_HEIGHT];
  numCols = tidlLayer->outData.dimValues[TIDL_DIM_WIDTH];

  roundVal = algLayer->layerParams.poolParams.internalPoolingWeightQ;

#ifdef BUILD_WITH_OPENACC
  // NOTE: helper variable to get the size of data in advanced
  const int32_t &tmp_startH = numRows * strideH;
  const int32_t &tmp_startW = numCols * strideW;

  const int32_t &tmp_hEndComp = tmp_startH + kernelH;
  const int32_t &tmp_hEndBound = height + padH;
  const int32_t &tmp_endH = (tmp_hEndComp < tmp_hEndBound) ? tmp_hEndComp : tmp_hEndBound;

  const int32_t &tmp_wEndComp = tmp_startW + kernelW;
  const int32_t &tmp_wEndBound = width + padW;
  const int32_t &tmp_endW = (tmp_wEndComp < tmp_wEndBound) ? tmp_wEndComp : tmp_wEndBound;
  int32_t offsetOTF = TIDL_getOTFOffset(net, algLayer, inDataParams);
  volatile int32_t inData_size = 1 + ((numBatches - 1) * inBatchPitch) + ((numOutChannels - 1) * inChPitch) + ((tmp_endH - 1) * inPitch) + (tmp_endW - 1);
  inData_size += offsetOTF;
  volatile int32_t accPtr_size = 1 + ((numBatches - 1) * outBatchPitch) + ((numOutChannels - 1) * outChPitch) + ((numRows - 1) * outPitch) + (numCols - 1);
#endif

  OPENACC(data copyin(inData[:inData_size])
              present(accPtr[:1 + ((numBatches - 1) * outBatchPitch) + ((numOutChannels - 1) * outChPitch) + ((numRows - 1) * outPitch) + (numCols - 1)])
                  copyout(outData[:1 + ((numBatches - 1) * outBatchPitch) + ((numOutChannels - 1) * outChPitch) + ((numRows - 1) * outPitch) + (numCols - 1)]))
  {

    if (TIDL_isPadOTF(net->deviceName) == TRUE)
    {
      /* TIDL_adjustOTFinPtr(&inData, net, algLayer, inDataParams);
      Adjust input data pointer for OTF padding
      not calling adjustOTFinPtr because we have already checked the condition for isPadOTF and don't want to check it again inside adjustOTFinPtr function.
      */
      inData = inData - (((algLayer->layerParams.poolParams.virtualPad.padT * inDataParams->pitch[TIDL_LINE_PITCH]) + algLayer->layerParams.poolParams.virtualPad.padL));
    }
    else
    {
      /*
      input is padded with padH and padW, for the case where padT and padL are zero we have to move input pointer to valid position.

      input data padded with padH and padW                input ptr position is determined by the padT and padL not by padH and padW,
                                                          we have to move input ptr to the correct position, so we have to jump (padH-padT)
                                                          amount on height and (padW-padL) amount width wise.

      ex: padH = 2, padW = 2                               padT = 1, padL = 1

            inPtr-> ptr 0  0  0  0  0  0                           0   0   0  0  0  0  0
                    0   0  0  0  0  0  0                           0   ptr 0  0  0  0  0
                    0   0  x  x  x  x  x                           0   0   x  x  x  x  x
                    0   0  x  x  x  x  x                           0   0   x  x  x  x  x
                    0   0  x  x  x  x  x                           0   0   x  x  x  x  x

      */
      // move this function into adjustOTFFinPtr function after testing average poolqq
      inData = inData + (((padH - tidlLayer->layerParams.poolParams.padT) * inDataParams->pitch[TIDL_LINE_PITCH]) + (padW - tidlLayer->layerParams.poolParams.padL));
    }

    if (TIDL_SUCCESS == status)
    {
      OPENACC(parallel loop collapse(4) reduction(min : min) reduction(max : max))
      for (i0 = 0; i0 < numBatches; i0++)
      {
        for (i1 = 0; i1 < numOutChannels; i1++)
        {
          for (i2 = 0; i2 < numRows; i2++)
          {
            for (i3 = 0; i3 < numCols; i3++)
            {
              /*  BVLC caffe doesn't assume the values of invalid inputs as zero whereas this reference implementation assumes that invalid pixels
               *  belonging to the border have been pre-initialized to 0 before this function is called.
               *  While computing average pooling when the frame boundary is reached the average is computed for only the values inside the frame.
               *  Example: If the pooling size is 9 and at the boundary we only have 6 valid values then
               *  the average computed will be the sum of the 6 valid values divided by the the pooling size which will be 6.
               *  https://github.com/BVLC/caffe/blob/master/src/caffe/layers/pooling_layer.cpp
               */
              int32_t startH = i2 * strideH;
              int32_t startW = i3 * strideW;

              int32_t hEndComp = startH + kernelH;
              int32_t hEndBound = height + padH;
              int32_t endH = (hEndComp < hEndBound) ? hEndComp : hEndBound;

              int32_t wEndComp = startW + kernelW;
              int32_t wEndBound = width + padW;
              int32_t endW = (wEndComp < wEndBound) ? wEndComp : wEndBound;

              /* The values startH, endH, startW, endW contain the boundaries for valid indices.
               * In the following code please note that we compute the average for only the valid indices.
               */
              int32_t poolSize = (endH - startH) * (endW - startW);

              /* This check Ensure that the last pooling starts inside the image (at least 1 pixel)
               * Onnxruntime == 1.21 has fixed for this issue by decrementing output shape(we are currently at onnruntime-tidl==1.15)
                 this check can be removed after onnxruntime-tidl version moved to 1.21
                *https://github.com/microsoft/onnxruntime/blob/main/onnxruntime/core/providers/cpu/nn/pool_attributes.h
              */
              if (poolSize == 0)
              {
                continue;
              }

              sumBlock = 0;

              OPENACC(loop seq)
              for (i4 = startH; i4 < endH; i4++)
              {
                OPENACC(loop seq)
                for (i5 = startW; i5 < endW; i5++)
                {
                  spatialOffsetY = i4;
                  spatialOffsetX = i5;
                  OPENACC(routine(TIDL_checkPixelInPadRegion))
                  isBorderPixel = TIDL_checkPixelInPadRegion(spatialOffsetY + startRowNumberInTensor, spatialOffsetX, validPosXMin, validPosXMax, validPosYMin, validPosYMax);
                  if (((uint32_t)isBorderPixel & (uint32_t)isOTFpad) != 0U)
                  {
                    inRowCol = 0;
                  }
                  else
                  {
                    inRowCol = (Tacc)inData[(i0 * inBatchPitch) + (i1 * inChPitch) + (i4 * inPitch) + i5];
                  }
                  sumBlock += inRowCol;
                }
              }
              if (std::is_same<Tacc, float32_tidl>::value)
              {
                scaleValue = 1.0f / (float)poolSize;
              }
              else
              {
                scaleValue = ((uint32_t)1 << (uint32_t)roundVal) / (uint32_t)(poolSize);
              }
              result = sumBlock * scaleValue;
              min = (result < min) ? result : min;
              max = (result > max) ? result : max;

              accPtr[(i0 * outBatchPitch) + (i1 * outChPitch) + (i2 * outPitch) + i3] = result;
            }
          }
        }
      }
    }

    roundBits = net->TIDLLayers[layerIdx].outData.roundBits;

    int32_t satLow;
    int32_t satHigh;
    int32_t mixedPrecision = 0;
    int32_t procElemSize;
    Tout temp = 0;
    temp = std::numeric_limits<Tout>::lowest();
    satLow = (int32_t)temp;
    temp = std::numeric_limits<Tout>::max();
    satHigh = (int32_t)(temp);

    procElemSize = TIDL_getProcessingElementSizeInBytes(tidlLayer);

    if ((int64_t)sizeof(Tout) != procElemSize)
    {
      mixedPrecision = 1;
      /* This is to match with target implementation wherin the saturations
      happens after applying outRoundBits - 8 shift and then the right shift
      of 8 is applied */
      roundBits = roundBits - 8;

      satLow = std::numeric_limits<uint16_t>::lowest();
      satHigh = std::numeric_limits<uint16_t>::max();
    }

    if (TIDL_SUCCESS == status)
    {
      OPENACC(parallel loop collapse(4) reduction(min : min) reduction(max : max))
      for (i0 = 0; i0 < numBatches; i0++)
      {
        for (i1 = 0; i1 < numOutChannels; i1++)
        {
          for (i2 = 0; i2 < numRows; i2++)
          {
            for (i3 = 0; i3 < numCols; i3++)
            {
              result = accPtr[(i0 * outBatchPitch) + (i1 * outChPitch) + (i2 * outPitch) + i3];
              if (std::is_same<Tacc, float32_tidl>::value)
              {
                OPENACC(routine(TIDL_floatSat))
                result = TIDL_floatSat(result, &net->TIDLLayers[layerIdx]);
              }
              else
              {
                result = (Tacc)TIDL_roundSat((int64_t)result, roundBits, (int32_t)satLow, (int32_t)satHigh);
                if (mixedPrecision == 1)
                {
                  result = (uint64_t)result >> 8U;
                }
              }
              outData[(i0 * outBatchPitch) + (i1 * outChPitch) + (i2 * outPitch) + i3] = (Tout)result;
            }
          }
        }
      }
    }
  }

  return status;
}

/**
 * @brief Spatial pooling layer reference implementation
 *
 * @param intAlgHandle : tidl algorothm handle
 * @param tidlLayer : Pointer to the common layer parameters
 * @param algLayer : Pointer to the layer specific parameters
 * @param params : copy of pooling layer parameters
 * @param inPtr : Pointer to input buffers to be processed
 * @param outPtr : Pointer to output buffers to be processed
 * @param poolingBuffParams : parameters of the pooling buffer
 * @param numBatches : number of total batches
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
static int32_t TIDL_refSpatialPoolingProcess(sTIDL_Network_t *net,
                                             sTIDL_Layer_t *tidlLayer,
                                             sTIDL_DataParams_t *inDataParams,
                                             const sTIDL_AlgLayer_t *algLayer,
                                             const sTIDL_PoolingParams_t *params,
                                             void *inPtr,
                                             void *outPtr,
                                             const TIDL_SpatialPoolingBuffParams_t *poolingBuffParams,
                                             int32_t numBatches)
{
  int32_t status = TIDL_SUCCESS;
  int32_t padW;
  int32_t padH;
  if (TIDL_isPadOTF(net->deviceName) != 0U)
  {
    padW = algLayer->layerParams.poolParams.virtualPad.padL;
    padH = algLayer->layerParams.poolParams.virtualPad.padT;
  }
  else
  {
    padW = params->padW;
    padH = params->padH;
  }

  if (TIDL_MaxPooling == params->poolingType)
  {
    if (poolingBuffParams->inElementType == TIDL_UnsignedChar)
    {
      TIDL_refSpatialMaxPooling(net,
                                (uint8_t *)inPtr,
                                algLayer->layerParams.poolParams.startRowNumberInTensor,
                                algLayer->layerParams.poolParams.orgInTensorHeight,
                                poolingBuffParams->inWidth,
                                poolingBuffParams->inHeight,
                                poolingBuffParams->inPitch,
                                poolingBuffParams->inChPitch,
                                poolingBuffParams->outChPitch,
                                params->numChannels,
                                params->kernelW,
                                params->kernelH,
                                params->strideW,
                                params->strideH,
                                padW,
                                padH,
                                (uint8_t *)outPtr,
                                tidlLayer, inDataParams,
                                algLayer);
    }
    else if (poolingBuffParams->inElementType == TIDL_SignedChar)
    {
      TIDL_refSpatialMaxPooling(net,
                                (int8_t *)inPtr,
                                algLayer->layerParams.poolParams.startRowNumberInTensor,
                                algLayer->layerParams.poolParams.orgInTensorHeight,
                                poolingBuffParams->inWidth,
                                poolingBuffParams->inHeight,
                                poolingBuffParams->inPitch,
                                poolingBuffParams->inChPitch,
                                poolingBuffParams->outChPitch,
                                params->numChannels,
                                params->kernelW,
                                params->kernelH,
                                params->strideW,
                                params->strideH,
                                padW,
                                padH,
                                (int8_t *)outPtr,
                                tidlLayer, inDataParams, algLayer);
    }
    else if (poolingBuffParams->inElementType == TIDL_UnsignedShort)
    {
      TIDL_refSpatialMaxPooling(net,
                                (uint16_t *)inPtr,
                                algLayer->layerParams.poolParams.startRowNumberInTensor,
                                algLayer->layerParams.poolParams.orgInTensorHeight,
                                poolingBuffParams->inWidth,
                                poolingBuffParams->inHeight,
                                poolingBuffParams->inPitch,
                                poolingBuffParams->inChPitch,
                                poolingBuffParams->outChPitch,
                                params->numChannels,
                                params->kernelW,
                                params->kernelH,
                                params->strideW,
                                params->strideH,
                                padW,
                                padH,
                                (uint16_t *)outPtr,
                                tidlLayer, inDataParams, algLayer);
    }
    else if (poolingBuffParams->inElementType == TIDL_SignedShort)
    {
      TIDL_refSpatialMaxPooling(net,
                                (int16_t *)inPtr,
                                algLayer->layerParams.poolParams.startRowNumberInTensor,
                                algLayer->layerParams.poolParams.orgInTensorHeight,
                                poolingBuffParams->inWidth,
                                poolingBuffParams->inHeight,
                                poolingBuffParams->inPitch,
                                poolingBuffParams->inChPitch,
                                poolingBuffParams->outChPitch,
                                params->numChannels,
                                params->kernelW,
                                params->kernelH,
                                params->strideW,
                                params->strideH,
                                padW,
                                padH,
                                (int16_t *)outPtr,
                                tidlLayer, inDataParams, algLayer);
    }
    else if (poolingBuffParams->inElementType == TIDL_SinglePrecFloat)
    {
      TIDL_refSpatialMaxPooling(net,
                                (float32_tidl *)inPtr,
                                algLayer->layerParams.poolParams.startRowNumberInTensor,
                                algLayer->layerParams.poolParams.orgInTensorHeight,
                                poolingBuffParams->inWidth,
                                poolingBuffParams->inHeight,
                                poolingBuffParams->inPitch,
                                poolingBuffParams->inChPitch,
                                poolingBuffParams->outChPitch,
                                params->numChannels,
                                params->kernelW,
                                params->kernelH,
                                params->strideW,
                                params->strideH,
                                padW,
                                padH,
                                (float32_tidl *)outPtr,
                                tidlLayer, inDataParams, algLayer);
    }
    else
    {
      status = TIDL_ERR_FAILURE;
    }
  }
  else if (TIDL_AveragePooling == params->poolingType)
  {
    void *accPtr = NULL;

    if (poolingBuffParams->inElementType == TIDL_UnsignedChar)
    {
      if (poolingBuffParams->outElementType == TIDL_UnsignedChar)
      {
        status = TIDL_refSpatialAvgPooling(net,
                                           (uint8_t *)inPtr,
                                           poolingBuffParams->inWidth,
                                           poolingBuffParams->inHeight,
                                           poolingBuffParams->inPitch,
                                           poolingBuffParams->inChPitch,
                                           poolingBuffParams->outChPitch,
                                           params->numChannels,
                                           params->kernelW,
                                           params->kernelH,
                                           params->strideW,
                                           params->strideH,
                                           padW,
                                           padH,
                                           (uint8_t *)outPtr,
                                           (int32_t *)accPtr,
                                           algLayer,
                                           tidlLayer, inDataParams);
      }
      else if (poolingBuffParams->outElementType == TIDL_UnsignedShort)
      {
        status = TIDL_refSpatialAvgPooling(net,
                                           (uint8_t *)inPtr,
                                           poolingBuffParams->inWidth,
                                           poolingBuffParams->inHeight,
                                           poolingBuffParams->inPitch,
                                           poolingBuffParams->inChPitch,
                                           poolingBuffParams->outChPitch,
                                           params->numChannels,
                                           params->kernelW,
                                           params->kernelH,
                                           params->strideW,
                                           params->strideH,
                                           padW,
                                           padH,
                                           (uint16_t *)outPtr,
                                           (int64_t *)accPtr,
                                           algLayer,
                                           tidlLayer, inDataParams);
      }
      else
      {
        status = TIDL_ERR_FAILURE;
      }
    }
    else if (poolingBuffParams->inElementType == TIDL_SignedChar)
    {
      if (poolingBuffParams->outElementType == TIDL_SignedChar)
      {
        status = TIDL_refSpatialAvgPooling(net,
                                           (int8_t *)inPtr,
                                           poolingBuffParams->inWidth,
                                           poolingBuffParams->inHeight,
                                           poolingBuffParams->inPitch,
                                           poolingBuffParams->inChPitch,
                                           poolingBuffParams->outChPitch,
                                           params->numChannels,
                                           params->kernelW,
                                           params->kernelH,
                                           params->strideW,
                                           params->strideH,
                                           padW,
                                           padH,
                                           (int8_t *)outPtr,
                                           (int32_t *)accPtr,
                                           algLayer,
                                           tidlLayer, inDataParams);
      }
      else
      {
        status = TIDL_ERR_FAILURE;
      }
    }
    else if (poolingBuffParams->inElementType == TIDL_UnsignedShort)
    {
      if (poolingBuffParams->outElementType == TIDL_UnsignedChar)
      {
        status = TIDL_refSpatialAvgPooling(net,
                                           (uint16_t *)inPtr,
                                           poolingBuffParams->inWidth,
                                           poolingBuffParams->inHeight,
                                           poolingBuffParams->inPitch,
                                           poolingBuffParams->inChPitch,
                                           poolingBuffParams->outChPitch,
                                           params->numChannels,
                                           params->kernelW,
                                           params->kernelH,
                                           params->strideW,
                                           params->strideH,
                                           padW,
                                           padH,
                                           (uint8_t *)outPtr,
                                           (int32_t *)accPtr,
                                           algLayer,
                                           tidlLayer, inDataParams);
      }
      else if (poolingBuffParams->outElementType == TIDL_UnsignedShort)
      {
        status = TIDL_refSpatialAvgPooling(net,
                                           (uint16_t *)inPtr,
                                           poolingBuffParams->inWidth,
                                           poolingBuffParams->inHeight,
                                           poolingBuffParams->inPitch,
                                           poolingBuffParams->inChPitch,
                                           poolingBuffParams->outChPitch,
                                           params->numChannels,
                                           params->kernelW,
                                           params->kernelH,
                                           params->strideW,
                                           params->strideH,
                                           padW,
                                           padH,
                                           (uint16_t *)outPtr,
                                           (int64_t *)accPtr,
                                           algLayer,
                                           tidlLayer, inDataParams);
      }
      else
      {
        status = TIDL_ERR_FAILURE;
      }
    }
    else if (poolingBuffParams->inElementType == TIDL_SignedShort)
    {
      if (poolingBuffParams->outElementType == TIDL_SignedShort)
      {
        status = TIDL_refSpatialAvgPooling(net,
                                           (int16_t *)inPtr,
                                           poolingBuffParams->inWidth,
                                           poolingBuffParams->inHeight,
                                           poolingBuffParams->inPitch,
                                           poolingBuffParams->inChPitch,
                                           poolingBuffParams->outChPitch,
                                           params->numChannels,
                                           params->kernelW,
                                           params->kernelH,
                                           params->strideW,
                                           params->strideH,
                                           padW,
                                           padH,
                                           (int16_t *)outPtr,
                                           (int64_t *)accPtr,
                                           algLayer,
                                           tidlLayer, inDataParams);
      }
      else
      {
        status = TIDL_ERR_FAILURE;
      }
    }
    else if (poolingBuffParams->inElementType == TIDL_SinglePrecFloat)
    {
      status = TIDL_refSpatialAvgPooling(net,
                                         (float32_tidl *)inPtr,
                                         poolingBuffParams->inWidth,
                                         poolingBuffParams->inHeight,
                                         poolingBuffParams->inPitch,
                                         poolingBuffParams->inChPitch,
                                         poolingBuffParams->outChPitch,
                                         params->numChannels,
                                         params->kernelW,
                                         params->kernelH,
                                         params->strideW,
                                         params->strideH,
                                         padW,
                                         padH,
                                         (float32_tidl *)outPtr,
                                         (float32_tidl *)accPtr,
                                         algLayer,
                                         tidlLayer, inDataParams);
    }
    else
    {
      status = TIDL_ERR_FAILURE;
    }
  }
  else
  {
    status = TIDL_ERR_FAILURE;
  }
  TIDL_L1DandL2CacheWbInv();
  return status;
}

int32_t TIDL_poolingRefProcess(TIDL_NetworkCommonParams *commonParams,
                               sTIDL_AlgLayer_t *algLayer,
                               sTIDL_Layer_t *tidlLayer,
                               void *inPtrs[],
                               void *outPtrs[],
                               int32_t layerIdx)
{
  int32_t status = IALG_EOK;

  sTIDL_PoolingParams_t *params = &tidlLayer->layerParams.poolParams;
  int32_t isGlobalPooling = 0;
  uint8_t (*inPtr)[] = (uint8_t (*)[])(inPtrs[0]);
  uint8_t (*outPtr)[] = (uint8_t (*)[])(outPtrs[0]);
  int32_t quantizationStyle = commonParams->net->quantizationStyle;

  if (commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_POOLING_FORCE_SCRATCH_SIZE)
  {
    algLayer->scratchSize = 0;
  }
  else if (commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_POOLING_FORCE_POOL_TYPE_ERROR)
  {
    params->poolingType = -1;
  }
  if (commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_POOLING_FORCE_ACT_TYPE_ERROR)
  {
    tidlLayer->actParams.actType = TIDL_Tanh;
  }

  if ((params->kernelW == 0) && (params->kernelH == 0))
  {
    isGlobalPooling = 1;
  }
  if (isGlobalPooling == 0)
  {
    sTIDL_DataParams_t *inDataParams;
    inDataParams = TIDL_getDataParams(commonParams->createParams->net, tidlLayer->inData[0]);
    sTIDL_DataParams_t *outDataParams = &tidlLayer->outData;
    TIDL_SpatialPoolingBuffParams_t poolingBuffParams;
    poolingBuffParams.inWidth = inDataParams->dimValues[TIDL_DIM_WIDTH];
    poolingBuffParams.inHeight = inDataParams->dimValues[TIDL_DIM_HEIGHT];
    poolingBuffParams.inPitch = inDataParams->pitch[TIDL_LINE_PITCH];
    poolingBuffParams.inChPitch = inDataParams->pitch[TIDL_CHANNEL_PITCH];
    poolingBuffParams.outPitch = outDataParams->pitch[TIDL_LINE_PITCH];
    poolingBuffParams.outChPitch = outDataParams->pitch[TIDL_CHANNEL_PITCH];
    poolingBuffParams.inElementType = inDataParams->elementType;
    poolingBuffParams.outElementType = outDataParams->elementType;
    poolingBuffParams.scaleValue = 1;
    poolingBuffParams.scratchSize = algLayer->scratchSize;
    poolingBuffParams.scratchMem = algLayer->scratchMem;
    params->numChannels = inDataParams->dimValues[TIDL_DIM_NUMCH];
    int32_t numBatches = inDataParams->dimValues[TIDL_DIM_BATCH];

    status = TIDL_refSpatialPoolingProcess(commonParams->net,
                                           tidlLayer,
                                           inDataParams,
                                           algLayer,
                                           params,
                                           (int8_t *)inPtr,
                                           (int8_t *)outPtr,
                                           &poolingBuffParams,
                                           numBatches);
  }
  else /* (isGlobalPooling == TRUE)  */
  {
    sTIDL_DataParams_t *inDataParams;
    inDataParams = TIDL_getDataParams(commonParams->net, tidlLayer->inData[0]);
    sTIDL_DataParams_t *outDataParams = &tidlLayer->outData;
    TIDL_SpatialPoolingBuffParams_t poolingBuffParams;
    poolingBuffParams.inWidth = inDataParams->dimValues[TIDL_DIM_WIDTH];
    poolingBuffParams.inHeight = inDataParams->dimValues[TIDL_DIM_HEIGHT];
    poolingBuffParams.inPitch = inDataParams->pitch[TIDL_LINE_PITCH];
    poolingBuffParams.inChPitch = inDataParams->pitch[TIDL_CHANNEL_PITCH];
    poolingBuffParams.outPitch = outDataParams->pitch[TIDL_LINE_PITCH];
    poolingBuffParams.outChPitch = outDataParams->pitch[TIDL_CHANNEL_PITCH];
    poolingBuffParams.inElementType = inDataParams->elementType;
    poolingBuffParams.outElementType = outDataParams->elementType;
    poolingBuffParams.scaleValue = 1;
    poolingBuffParams.scratchSize = algLayer->scratchSize;
    poolingBuffParams.scratchMem = algLayer->scratchMem;
    params->numChannels = inDataParams->dimValues[TIDL_DIM_NUMCH];

    /* If pad=0, we treat the output as flattened, i.e, as 1x1x1xC instead of 1xCx1x1 */
    if ((outDataParams->padW == 0) && (outDataParams->padH == 0))
    {
      poolingBuffParams.outChPitch = 1;
    }
    int32_t scaleValueNumInInt = ((uint32_t)1 << (uint32_t)algLayer->layerParams.poolParams.internalPoolingWeightQ); /* Numerator */
    int32_t scaleValueDenInInt = inDataParams->dimValues[TIDL_DIM_WIDTH] * inDataParams->dimValues[TIDL_DIM_HEIGHT]; /* Denominator */
    float32_tidl scaleValueInFloat = (1.0f * (float32_tidl)scaleValueNumInInt) / (float32_tidl)scaleValueDenInInt;
    scaleValueInFloat += 0.5f;
    poolingBuffParams.scaleValue = (int32_t)scaleValueInFloat;

    int32_t numBatches = inDataParams->dimValues[TIDL_DIM_BATCH];

    if (TIDL_QuantStyleAsymNP2_TFL == quantizationStyle)
    {
      float32_tidl scaleRatio = inDataParams->tensorScale / (tidlLayer->outData.tensorScale * (float32_tidl)inDataParams->dimValues[TIDL_DIM_WIDTH] * (float32_tidl)inDataParams->dimValues[TIDL_DIM_HEIGHT]);
      uint8_t mmaScale;
      uint8_t mmaShift;
      TIDL_getMMAv2_ScaleAndShift(scaleRatio, &mmaScale, &mmaShift);
      float32_tidl biasTerm = ((float32_tidl)tidlLayer->outData.tensorZeroPoint / scaleRatio) - ((float32_tidl)inDataParams->tensorZeroPoint * (float32_tidl)inDataParams->dimValues[TIDL_DIM_WIDTH] * (float32_tidl)inDataParams->dimValues[TIDL_DIM_HEIGHT]);
      poolingBuffParams.biasTerm = round(biasTerm);
      poolingBuffParams.mmaShift = mmaShift;
      poolingBuffParams.scaleValue = mmaScale;
    }

    status = TIDL_refGlobalPoolingProcess(commonParams->net,
                                          tidlLayer,
                                          inDataParams,
                                          algLayer,
                                          params,
                                          (int8_t *)inPtr,
                                          (int8_t *)outPtr,
                                          &poolingBuffParams,
                                          numBatches);
  }

  return status;
}
