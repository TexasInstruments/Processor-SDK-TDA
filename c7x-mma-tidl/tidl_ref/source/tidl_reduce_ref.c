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
 *  \file tidl_reduce_ref.c
 *
 *  \brief This file defines kernel functions for Batch Norm layer
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include <math.h>
#include <limits>
#include "tidl_reduce_ref.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

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

/**
 * @brief This is main function perform reduce on differnt elementTypes
 *
 * @tparam Tin  : template for input
 * @tparam Tw   : template for weights
 * @tparam Tb   : template for Bias
 * @tparam Tout : template for output
 * @tparam Tsat : template for saturate values
 * @param inPtr : Input pointer on which bacthNorm is applied
 * @param outPtr : Output pointer after reduce opreation
 * @param weightsPtr : Pointer to weights buffer
 * @param slopePtr : Pointer to the Slope buffer
 * @param biasPtr  : Pointer to the Bias values
 * @param intAlgHandle : tidl algorothm handle
 * @param layerIdx :index of the current layer
 * @param params : copy of bacthNorm parameters
 * @param algLayer : Pointer to the layer specific parameters
 * @param inDataParams : parameters of the input data buffer
 * @param outDataParams : parameters of the output data buffer
 * @param satLow : min value for the saturation
 * @param satHigh : max value for the saturation
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
template<class Tin, class Tout> static int32_t TIDL_refReduceCore(Tin *inPtr,
                                                                  Tout *outPtr,
                                                                  TIDL_Handle intAlgHandle,
                                                                  int32_t layerIdx,
                                                                  sTIDL_ReduceParams_t *params,
                                                                  sTIDL_AlgLayer_t *algLayer,
                                                                  const sTIDL_DataParams_t *inDataParams,
                                                                  const sTIDL_DataParams_t *outDataParams)
{
  int32_t status = TIDL_SUCCESS;
  int32_t i0, i1, i2, i3, i4, i5;
  int32_t numInCols = (int32_t)inDataParams->dimValues[TIDL_DIM_WIDTH];
  int32_t numInRows = (int32_t)inDataParams->dimValues[TIDL_DIM_HEIGHT];
  int32_t numInChannels = (int32_t)inDataParams->dimValues[TIDL_DIM_NUMCH];
  int32_t numInDim2 = (int32_t)inDataParams->dimValues[TIDL_DIM_DIM2];
  int32_t numInDim1 = (int32_t)inDataParams->dimValues[TIDL_DIM_DIM1];
  int32_t numInBatches = (int32_t)inDataParams->dimValues[TIDL_DIM_BATCH];
  int32_t inPitch = (int32_t)inDataParams->pitch[TIDL_LINE_PITCH];
  int32_t inChPitch = (int32_t)inDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t inDim2Pitch = (int32_t)inDataParams->pitch[TIDL_DIM2_PITCH];
  int32_t inDim1Pitch = (int32_t)inDataParams->pitch[TIDL_DIM1_PITCH];
  int32_t inBatchPitch = (int32_t)inDataParams->pitch[TIDL_ROI_PITCH];

  int32_t numOutCols = (int32_t)outDataParams->dimValues[TIDL_DIM_WIDTH];
  int32_t numOutRows = (int32_t)outDataParams->dimValues[TIDL_DIM_HEIGHT];
  int32_t numOutChannels = (int32_t)outDataParams->dimValues[TIDL_DIM_NUMCH];
  int32_t numOutDim2 = (int32_t)outDataParams->dimValues[TIDL_DIM_DIM2];
  int32_t numOutDim1 = (int32_t)outDataParams->dimValues[TIDL_DIM_DIM1];
  int32_t numOutBatches = (int32_t)outDataParams->dimValues[TIDL_DIM_BATCH];
  int32_t outPitch = (int32_t)outDataParams->pitch[TIDL_LINE_PITCH];
  int32_t outChPitch = (int32_t)outDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t outDim2Pitch = (int32_t)outDataParams->pitch[TIDL_DIM2_PITCH];
  int32_t outDim1Pitch = (int32_t)outDataParams->pitch[TIDL_DIM1_PITCH];
  int32_t outBatchPitch = (int32_t)outDataParams->pitch[TIDL_ROI_PITCH];

  Tin *inData = (Tin *)inPtr;
  Tout *outData = (Tout *)outPtr;

  Tin targetVal;

  if (params->axis == TIDL_DIM_HEIGHT) /*across height in NCHW format*/
  {
    OPENACC(data copyin(inData[:1 + (inBatchPitch * (numInBatches - 1)) + (inDim1Pitch * (numInDim1 - 1)) + (inDim2Pitch * (numInDim2 - 1)) +
                                (inChPitch * (numInChannels - 1)) + (inPitch * (numInRows - 1)) + (numInCols - 1)])
                copyout(outData[:1 + (outBatchPitch * (numOutBatches - 1)) + (outDim1Pitch * (numOutDim1 - 1)) + (outDim2Pitch * (numOutDim2 - 1)) +
                                 (outChPitch * (numOutChannels - 1)) + (outPitch * (numOutRows - 1)) + (numOutCols - 1)]))
    OPENACC(parallel loop collapse(5))
    for (i0 = 0; i0 < numInBatches; i0++)
    {
      for (i1 = 0; i1 < numInDim1; i1++)
      {
        for (i2 = 0; i2 < numInDim2; i2++)
        {
          for (i3 = 0; i3 < numInChannels; i3++)
          {
            for (i5 = 0; i5 < numInCols; i5++)
            {
              /* find min/max in each row */
              targetVal = inData[(i0 * inBatchPitch) + (i1 * inDim1Pitch) + (i2 * inDim2Pitch) + (i3 * inChPitch) + (0 * inPitch) + i5];
              if (params->ops == (int32_t)TIDL_inReduceOpMin) // reduceMin
              {
                OPENACC(loop)
                for (i4 = 1; i4 < numInRows; i4++)
                {
                  if (inData[(i0 * inBatchPitch) + (i1 * inDim1Pitch) + (i2 * inDim2Pitch) + (i3 * inChPitch) + (i4 * inPitch) + i5] < targetVal)
                  {
                    targetVal = inData[(i0 * inBatchPitch) + (i1 * inDim1Pitch) + (i2 * inDim2Pitch) + (i3 * inChPitch) + (i4 * inPitch) + i5];
                  }
                }
              }
              if (params->ops == (int32_t)TIDL_inReduceOpMax) // reduceMax
              {
                OPENACC(loop)
                for (i4 = 1; i4 < numInRows; i4++)
                {
                  if (inData[(i0 * inBatchPitch) + (i1 * inDim1Pitch) + (i2 * inDim2Pitch) + (i3 * inChPitch) + (i4 * inPitch) + i5] > targetVal)
                  {
                    targetVal = inData[(i0 * inBatchPitch) + (i1 * inDim1Pitch) + (i2 * inDim2Pitch) + (i3 * inChPitch) + (i4 * inPitch) + i5];
                  }
                }
              }
              outData[(i0 * outBatchPitch) + (i1 * outDim1Pitch) + (i2 * outDim2Pitch) + (i3 * outChPitch) + (i5)] = targetVal;
            }
          }
        }
      }
    }
  }
  else
  {
    /* tidl_printf(0,"Unsupported axis for reduce layer exiting...\n    "); */
    TIDL_LOG_ERROR(TIDL_ERROR_GROUP_REDUCE, TIDL_ERROR_REDUCE_UNSUPPORTED_AXIS);
    status = TIDL_ERR_FAILURE;
  }

  return status;
}

/**
 * @brief Reduce layer reference implementation
 *
 * @param intAlgHandle : tidl algorothm handle
 * @param algLayer : Pointer to the layer specific parameters
 * @param tidlLayer : Pointer to the common layer parameters
 * @param params : copy of batch norm layer parameters
 * @param inPtr : Pointer to input buffers to be processed
 * @param outPtr : Pointer to output buffers to be processed
 * @param inDataParams : pointer to input data parameters
 * @param outDataParams : pointer to output data parameters
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
int32_t TIDL_reduceRefProcess(TIDL_Handle intAlgHandle,
                              sTIDL_AlgLayer_t *algLayer,
                              const sTIDL_Layer_t *tidlLayer,
                              sTIDL_ReduceParams_t *params,
                              void *inPtr,
                              void *outPtr,
                              const sTIDL_DataParams_t *inDataParams,
                              const sTIDL_DataParams_t *outDataParams)
{
  int32_t status = TIDL_SUCCESS;
  int32_t layerIdx = algLayer->layerIdx;

  if (TIDL_SignedChar == ((int32_t)inDataParams->elementType))
  {
    status = TIDL_refReduceCore((int8_t *)inPtr,
                                (int8_t *)outPtr,
                                intAlgHandle,
                                layerIdx,
                                params,
                                algLayer,
                                inDataParams,
                                outDataParams);
  }
  else if (TIDL_UnsignedChar == ((int32_t)inDataParams->elementType))
  {
    status = TIDL_refReduceCore((uint8_t *)inPtr,
                                (uint8_t *)outPtr,
                                intAlgHandle,
                                layerIdx,
                                params,
                                algLayer,
                                inDataParams,
                                outDataParams);
  }
  else if (TIDL_SignedShort == ((int32_t)inDataParams->elementType))
  {
    status = TIDL_refReduceCore((int16_t *)inPtr,
                                (int16_t *)outPtr,
                                intAlgHandle,
                                layerIdx,
                                params,
                                algLayer,
                                inDataParams,
                                outDataParams);
  }
  else if (TIDL_UnsignedShort == ((int32_t)inDataParams->elementType))
  {
    status = TIDL_refReduceCore((uint16_t *)inPtr,
                                (uint16_t *)outPtr,
                                intAlgHandle,
                                layerIdx,
                                params,
                                algLayer,
                                inDataParams,
                                outDataParams);
  }
  else if (TIDL_SinglePrecFloat == ((int32_t)inDataParams->elementType))
  {
    status = TIDL_refReduceCore((float32_tidl *)inPtr,
                                (float32_tidl *)outPtr,
                                intAlgHandle,
                                layerIdx,
                                params,
                                algLayer,
                                inDataParams,
                                outDataParams);
  }
  else
  {
    status = TIDL_ERR_FAILURE;
  }
  TIDL_L1DandL2CacheWbInv();
  return status;
}
