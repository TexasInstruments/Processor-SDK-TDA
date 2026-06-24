/*
 *
 * Copyright (c) {2022 - 2026} Texas Instruments Incorporated
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
 *  \file tidl_gatherND_ref.c
 *
 *  \brief This file defines kernel functions for GatherND layer
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include <math.h>
#include "tidl_gatherNDLayer_ref.h"

/**
 * @brief This function populates the indices tuple for gatherND operation
 *
 * @param indices : pointer to the indices buffer
 * @param indicesTuple : pointer to the indices tuple
 * @param inIndicesParams : parameters of the input indices buffer
 * @param i0, i1, i2, i3, i4 : index values for each dimension
 *
 * @return 0 on success, -1 on failure
 */
template<class Tind>
    int32_t tidl_gatherNDPopulateIndicesTuple(
        Tind *indices,
        int32_t *indicesTuple,
        const sTIDL_DataParams_t *inIndicesParams,
        int32_t i0, int32_t i1, int32_t i2, int32_t i3, int32_t i4)
{
  int32_t index;
  int32_t indDim = inIndicesParams->dimValues[TIDL_DIM_WIDTH];
  memset(indicesTuple, 0, sizeof(int32_t) * TIDL_DIM_MAX);
  index = i0 * inIndicesParams->pitch[TIDL_ROI_PITCH] +
          i1 * inIndicesParams->pitch[TIDL_DIM1_PITCH] +
          i2 * inIndicesParams->pitch[TIDL_DIM2_PITCH] +
          i3 * inIndicesParams->pitch[TIDL_CHANNEL_PITCH] +
          i4 * inIndicesParams->pitch[TIDL_LINE_PITCH];
  for (int32_t i = TIDL_DIM_WIDTH, j = indDim - 1; j > -1 && i > -1; i--, j--)
  {
    indicesTuple[i] = (int32_t)indices[index + j];
  }
  return 0;
}

/**
 * @brief Helper function to calculate output base offset for GatherND operation
 *
 * @param outDataParams : parameters of the output data buffer
 * @param remainingDims : number of remaining dimensions to process
 * @param i0, i1, i2, i3, i4 : index values for each dimension
 *
 * @return outputBaseOffset : calculated output base offset
 */
int32_t tidl_gatherNDCalculateOutputOffset(
    const sTIDL_DataParams_t *outDataParams,
    int32_t remainingDims,
    int32_t i0, int32_t i1, int32_t i2, int32_t i3, int32_t i4)
{
  int32_t outRoiPitch = (int32_t)outDataParams->pitch[TIDL_ROI_PITCH];
  int32_t outDim1Pitch = (int32_t)outDataParams->pitch[TIDL_DIM1_PITCH];
  int32_t outDim2Pitch = (int32_t)outDataParams->pitch[TIDL_DIM2_PITCH];
  int32_t outChPitch = (int32_t)outDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t outPitch = (int32_t)outDataParams->pitch[TIDL_LINE_PITCH];
  
  int32_t outputBaseOffset = 0;
  
  if(remainingDims == 5)
  {
    outputBaseOffset = i4 * outRoiPitch;
  }
  else if(remainingDims == 4)
  {
    outputBaseOffset = (i3 * outRoiPitch) + (i4 * outDim1Pitch);
  }
  else if(remainingDims == 3)
  {
    outputBaseOffset = (i2 * outRoiPitch) + (i3 * outDim1Pitch) + (i4 * outDim2Pitch);
  }
  else if(remainingDims == 2)
  {
    outputBaseOffset = (i1 * outRoiPitch) + (i2 * outDim1Pitch) + (i3 * outDim2Pitch) + (i4 * outChPitch);
  }
  else if(remainingDims == 1)
  {
    outputBaseOffset = (i0 * outRoiPitch) + (i1 * outDim1Pitch) + (i2 * outDim2Pitch) + (i3 * outChPitch) + (i4 * outPitch);
  }
  else if(remainingDims == 0)
  {
    outputBaseOffset = (i0 * outDim1Pitch) + (i1 * outDim2Pitch) + (i2 * outChPitch) + (i3 * outPitch) + i4;
  }
  
  return outputBaseOffset;
}

/**
 * @brief Helper function to copy multi-dimensional data blocks
 *
 * @param data : source data pointer
 * @param output : destination data pointer
 * @param dataOffset : base offset in source data
 * @param outputBaseOffset : base offset in output data
 * @param inDataParams : input data parameters
 * @param outDataParams : output data parameters
 * @param remainingDims : number of remaining dimensions to copy
 */
template<class Tin, class Tout>
void tidl_gatherNDCopyDataBlock(
    Tin *data,
    Tout *output,
    int32_t dataOffset,
    int32_t outputBaseOffset,
    const sTIDL_DataParams_t *inDataParams,
    const sTIDL_DataParams_t *outDataParams,
    int32_t remainingDims)
{
  int32_t inDataPitch = (int32_t)inDataParams->pitch[TIDL_LINE_PITCH];
  int32_t inDataChPitch = (int32_t)inDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t inDataDim2Pitch = (int32_t)inDataParams->pitch[TIDL_DIM2_PITCH];
  int32_t inDataDim1Pitch = (int32_t)inDataParams->pitch[TIDL_DIM1_PITCH];
  
  int32_t outPitch = (int32_t)outDataParams->pitch[TIDL_LINE_PITCH];
  int32_t outChPitch = (int32_t)outDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t outDim2Pitch = (int32_t)outDataParams->pitch[TIDL_DIM2_PITCH];
  int32_t outDim1Pitch = (int32_t)outDataParams->pitch[TIDL_DIM1_PITCH];
  
  int32_t srcOffset, dstOffset;
  
  if(remainingDims == 5)
  {
    // Copy 5D block: DIM1 x DIM2 x NUMCH x HEIGHT x WIDTH
    for(int32_t d1 = 0; d1 < (int32_t)outDataParams->dimValues[TIDL_DIM_DIM1]; d1++)
    {
      for(int32_t d2 = 0; d2 < (int32_t)outDataParams->dimValues[TIDL_DIM_DIM2]; d2++)
      {
        for(int32_t c = 0; c < (int32_t)outDataParams->dimValues[TIDL_DIM_NUMCH]; c++)
        {
          for(int32_t h = 0; h < (int32_t)outDataParams->dimValues[TIDL_DIM_HEIGHT]; h++)
          {
            for(int32_t w = 0; w < (int32_t)outDataParams->dimValues[TIDL_DIM_WIDTH]; w++)
            {
              srcOffset = dataOffset + d1*inDataDim1Pitch + d2*inDataDim2Pitch + c*inDataChPitch + (h * inDataPitch) + w;
              dstOffset = outputBaseOffset + d1*outDim1Pitch + d2*outDim2Pitch + c*outChPitch + (h * outPitch) + w;
              output[dstOffset] = data[srcOffset];
            }
          }
        }
      }
    }
  }
  else if(remainingDims == 4)
  {
    // Copy 4D block: DIM2 x NUMCH x HEIGHT x WIDTH
    for(int32_t d2 = 0; d2 < (int32_t)outDataParams->dimValues[TIDL_DIM_DIM2]; d2++)
    {
      for(int32_t c = 0; c < (int32_t)outDataParams->dimValues[TIDL_DIM_NUMCH]; c++)
      {
        for(int32_t h = 0; h < (int32_t)outDataParams->dimValues[TIDL_DIM_HEIGHT]; h++)
        {
          for(int32_t w = 0; w < (int32_t)outDataParams->dimValues[TIDL_DIM_WIDTH]; w++)
          {
            srcOffset = dataOffset + d2*inDataDim2Pitch + c*inDataChPitch + (h * inDataPitch) + w;
            dstOffset = outputBaseOffset + d2*outDim2Pitch + c*outChPitch + (h * outPitch) + w;
            output[dstOffset] = data[srcOffset];
          }
        }
      }
    }
  }
  else if(remainingDims == 3)
  {
    // Copy 3D block: NUMCH x HEIGHT x WIDTH
    for(int32_t c = 0; c < (int32_t)outDataParams->dimValues[TIDL_DIM_NUMCH]; c++)
    {
      for(int32_t h = 0; h < (int32_t)outDataParams->dimValues[TIDL_DIM_HEIGHT]; h++)
      {
        for(int32_t w = 0; w < (int32_t)outDataParams->dimValues[TIDL_DIM_WIDTH]; w++)
        {
          srcOffset = dataOffset + c*inDataChPitch + (h * inDataPitch) + w;
          dstOffset = outputBaseOffset + c*outChPitch + (h * outPitch) + w;
          output[dstOffset] = data[srcOffset];
        }
      }
    }
  }
  else if(remainingDims == 2)
  {
    // Copy 2D block: HEIGHT x WIDTH
    for(int32_t h = 0; h < (int32_t)outDataParams->dimValues[TIDL_DIM_HEIGHT]; h++)
    {
      for(int32_t w = 0; w < (int32_t)outDataParams->dimValues[TIDL_DIM_WIDTH]; w++)
      {
        srcOffset = dataOffset + (h * inDataPitch) + w;
        dstOffset = outputBaseOffset + (h * outPitch) + w;
        output[dstOffset] = data[srcOffset];
      }
    }
  }
  else if(remainingDims == 1)
  {
    // Copy 1D block: WIDTH
    for(int32_t w = 0; w < (int32_t)outDataParams->dimValues[TIDL_DIM_WIDTH]; w++)
    {
      srcOffset = dataOffset + w;
      dstOffset = outputBaseOffset + w;
      output[dstOffset] = data[srcOffset];
    }
  }
  else if(remainingDims == 0)
  {
    // Copy single element
    output[outputBaseOffset] = data[dataOffset];
  }
}

/**
 * @brief This is main function perform gatherND on differnt elementTypes
 *
 * @tparam Tin  : template for input
 * @tparam Tw   : template for weights
 * @tparam Tb   : template for Bias
 * @tparam Tout : template for output
 * @tparam Tsat : template for saturate values
 * @param inPtr : Input pointer on which bacthNorm is applied
 * @param outPtr : Output pointer after gatherND opreation
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

template<class Tin, class Tindx, class Tout> static int32_t TIDL_refGatherND(Tin *data,
                                                                           Tindx *indices,
                                                                           Tout *outPtr,
                                                                           TIDL_Handle intAlgHandle,
                                                                           int32_t layerIdx,
                                                                           sTIDL_GatherNDLayerParams_t *params,
                                                                           sTIDL_AlgLayer_t *algLayer,
                                                                           const sTIDL_DataParams_t *inDataParams,
                                                                           const sTIDL_DataParams_t *inIndicesParams,
                                                                           const sTIDL_DataParams_t *outDataParams)
{
  int32_t status = TIDL_SUCCESS;
  int32_t i0, i1, i2, i3, i4, i5;
  int32_t index;

  int32_t inIndicesNumCols    = (int32_t)inIndicesParams->dimValues[TIDL_DIM_WIDTH];
  int32_t inIndicesNumRows    = (int32_t)inIndicesParams->dimValues[TIDL_DIM_HEIGHT];
  int32_t inIndicesNumChs     = (int32_t)inIndicesParams->dimValues[TIDL_DIM_NUMCH];
  int32_t inIndicesDim2       = (int32_t)inIndicesParams->dimValues[TIDL_DIM_DIM2];
  int32_t inIndicesDim1       = (int32_t)inIndicesParams->dimValues[TIDL_DIM_DIM1];
  int32_t inIndicesBatches    = (int32_t)inIndicesParams->dimValues[TIDL_DIM_BATCH];

  int32_t inDataPitch         = (int32_t)inDataParams->pitch[TIDL_LINE_PITCH];
  int32_t inDataChPitch       = (int32_t)inDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t inDataRoiPitch      = (int32_t)inDataParams->pitch[TIDL_ROI_PITCH];
  int32_t inDataDim2Pitch     = (int32_t)inDataParams->pitch[TIDL_DIM2_PITCH];
  int32_t inDataDim1Pitch     = (int32_t)inDataParams->pitch[TIDL_DIM1_PITCH];
  
  int32_t inIndicesPitch      = (int32_t)inIndicesParams->pitch[TIDL_LINE_PITCH];
  int32_t inIndicesChPitch    = (int32_t)inIndicesParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t inIndicesRoiPitch   = (int32_t)inIndicesParams->pitch[TIDL_ROI_PITCH];
  int32_t inIndicesDim2Pitch  = (int32_t)inIndicesParams->pitch[TIDL_DIM2_PITCH];
  int32_t inIndicesDim1Pitch  = (int32_t)inIndicesParams->pitch[TIDL_DIM1_PITCH];

  int32_t outPitch            = (int32_t)outDataParams->pitch[TIDL_LINE_PITCH];
  int32_t outChPitch          = (int32_t)outDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t outRoiPitch         = (int32_t)outDataParams->pitch[TIDL_ROI_PITCH];
  int32_t outDim2Pitch        = (int32_t)outDataParams->pitch[TIDL_DIM2_PITCH];
  int32_t outDim1Pitch        = (int32_t)outDataParams->pitch[TIDL_DIM1_PITCH];

  data = (Tin *)data + (inDataParams->padH * inDataPitch) + inDataParams->padW;
  indices = (Tindx *)indices + (inIndicesParams->padH * inIndicesPitch) + inIndicesParams->padW;
  Tout *output = (Tout *)outPtr + (outDataParams->padH * outPitch) + outDataParams->padW;

  int32_t indicesTuple[TIDL_DIM_MAX] = {0}, updatePitch = 0;
  int32_t dataOffset = 0, dataBatchOffset = 0, outputBaseOffset = 0;
  int32_t srcOffset = 0, dstOffset = 0;

  int32_t batchDims = params->batchDims;
  int32_t indDim = inIndicesParams->dimValues[TIDL_DIM_WIDTH];
  int32_t numDims = params->dataDimCount;
  int32_t dataBatchStart = TIDL_DIM_MAX - params->dataDimCount;
  int32_t indicesBatchStart = TIDL_DIM_MAX - params->indicesDimCount;
    
    // Process each set of indices
  for(i0=0; i0<inIndicesBatches; i0++)
  {
    for(i1=0; i1<inIndicesDim1; i1++)
    {
      for(i2=0; i2<inIndicesDim2; i2++)
      {
        for(i3=0; i3<inIndicesNumChs; i3++)
        {
          for(i4=0; i4<inIndicesNumRows; i4++)
          {
            // Populate indices tuple for current position
            tidl_gatherNDPopulateIndicesTuple(
            indices, indicesTuple, inIndicesParams, i0, i1, i2, i3, i4);

            if(batchDims > 0)
            {
              dataBatchOffset = 0;
              int32_t indexValues[5] = {i0, i1, i2, i3, i4};
              if(batchDims >= 1)
              {
                dataBatchOffset += indexValues[indicesBatchStart] * (int32_t)inDataParams->pitch[dataBatchStart];
              }
              if(batchDims >= 2)
              {
                dataBatchOffset += indexValues[indicesBatchStart + 1] * (int32_t)inDataParams->pitch[dataBatchStart + 1];
              }
              if(batchDims >= 3)
              {
                dataBatchOffset += indexValues[indicesBatchStart + 2] * (int32_t)inDataParams->pitch[dataBatchStart + 2];
              }
              if(batchDims >= 4)
              {
                dataBatchOffset += indexValues[indicesBatchStart + 3] * (int32_t)inDataParams->pitch[dataBatchStart + 3];
              }
              if(batchDims >= 5)
              {
                dataBatchOffset += indexValues[indicesBatchStart + 4] * (int32_t)inDataParams->pitch[dataBatchStart + 4];
              }
            }
            int32_t remainingDims = numDims - indDim - batchDims;
              
            // Calculate data offset based on remaining dimensions
            if(remainingDims == 5)
            {
                dataOffset = dataBatchOffset + (indicesTuple[TIDL_DIM_WIDTH] * inDataRoiPitch);
            }
            else if(remainingDims == 4)
            {
                dataOffset = dataBatchOffset + (indicesTuple[TIDL_DIM_HEIGHT] * inDataRoiPitch) +
                           (indicesTuple[TIDL_DIM_WIDTH] * inDataDim1Pitch);
            }
            else if(remainingDims == 3)
            {
                dataOffset = dataBatchOffset + (indicesTuple[TIDL_DIM_NUMCH] * inDataRoiPitch) + 
                             (indicesTuple[TIDL_DIM_HEIGHT] * inDataDim1Pitch) +
                             (indicesTuple[TIDL_DIM_WIDTH] * inDataDim2Pitch);
            }
            else if(remainingDims == 2)
            { 
                dataOffset = dataBatchOffset + (indicesTuple[TIDL_DIM_DIM2] * inDataRoiPitch) +
                             (indicesTuple[TIDL_DIM_NUMCH] * inDataDim1Pitch) + 
                             (indicesTuple[TIDL_DIM_HEIGHT] * inDataDim2Pitch) +
                             (indicesTuple[TIDL_DIM_WIDTH] * inDataChPitch);
            }
            else if(remainingDims == 1)
            {
                dataOffset = dataBatchOffset + (indicesTuple[TIDL_DIM_DIM1] * inDataRoiPitch) +
                             (indicesTuple[TIDL_DIM_DIM2] * inDataDim1Pitch) +
                             (indicesTuple[TIDL_DIM_NUMCH] * inDataDim2Pitch) + 
                             (indicesTuple[TIDL_DIM_HEIGHT] * inDataChPitch) +
                             (indicesTuple[TIDL_DIM_WIDTH] * inDataPitch);
            }
            else if(remainingDims == 0)
            {
                dataOffset = dataBatchOffset + (indicesTuple[TIDL_DIM_BATCH] * inDataRoiPitch) +
                             (indicesTuple[TIDL_DIM_DIM1] * inDataDim1Pitch) +
                             (indicesTuple[TIDL_DIM_DIM2] * inDataDim2Pitch) +
                             (indicesTuple[TIDL_DIM_NUMCH] * inDataChPitch) + 
                             (indicesTuple[TIDL_DIM_HEIGHT] * inDataPitch) + 
                             indicesTuple[TIDL_DIM_WIDTH];
            }
              
            // Calculate output base offset
            outputBaseOffset = tidl_gatherNDCalculateOutputOffset(outDataParams, remainingDims, i0, i1, i2, i3, i4);
              
            // Copy data block using helper function
            tidl_gatherNDCopyDataBlock(data, output, dataOffset, outputBaseOffset, inDataParams, outDataParams, remainingDims);
          }
        }
      }
    }
  }

  return status;
}

/**
 * @brief gatherND layer reference implementation
 *
 * @param intAlgHandle : tidl algorithm handle
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
int32_t TIDL_gatherNDRefProcess(TIDL_Handle intAlgHandle,
                              sTIDL_AlgLayer_t *algLayer,
                              const sTIDL_Layer_t *tidlLayer,
                              sTIDL_GatherNDLayerParams_t *params,
                              void *data,
                              void *indices,
                              void *outPtr,
                              const sTIDL_DataParams_t *inDataParams,
                              const sTIDL_DataParams_t *inIndicesParams,
                              const sTIDL_DataParams_t *outDataParams)
{
  int32_t status = TIDL_SUCCESS;
  int32_t layerIdx = algLayer->layerIdx;

  if (TIDL_SignedChar == ((int32_t)inDataParams->elementType))
  {
    if (TIDL_SignedWord != ((int32_t)inIndicesParams->elementType))
    {
      tidl_printf(0, "Indice data type should be int32");
      status = TIDL_ERR_FAILURE;
    }
    else
    {
      status = TIDL_refGatherND((int8_t *)data,
                              (int32_t *)indices,
                              (int8_t *)outPtr,
                              intAlgHandle,
                              layerIdx,
                              params,
                              algLayer,
                              inDataParams,
                              inIndicesParams,
                              outDataParams);
    }
  }
  else if (TIDL_UnsignedChar == ((int32_t)inDataParams->elementType))
  {
    if (TIDL_SignedWord != ((int32_t)inIndicesParams->elementType))
    {
      tidl_printf(0, "Indice data type should be int32");
      status = TIDL_ERR_FAILURE;
    }
    else
    {
      status = TIDL_refGatherND((uint8_t *)data,
                              (int32_t *)indices,
                              (uint8_t *)outPtr,
                              intAlgHandle,
                              layerIdx,
                              params,
                              algLayer,
                              inDataParams,
                              inIndicesParams,
                              outDataParams);
    }
  }
  else if (TIDL_SignedShort == ((int32_t)inDataParams->elementType))
  {
    if (TIDL_SignedWord != ((int32_t)inIndicesParams->elementType))
    {
      tidl_printf(0, "Indice data type should be int32");
      status = TIDL_ERR_FAILURE;
    }
    else
    {
      status = TIDL_refGatherND((int16_t *)data,
                              (int32_t *)indices,
                              (int16_t *)outPtr,
                              intAlgHandle,
                              layerIdx,
                              params,
                              algLayer,
                              inDataParams,
                              inIndicesParams,
                              outDataParams);
    }
  }
  else if (TIDL_UnsignedShort == ((int32_t)inDataParams->elementType))
  {

    if (TIDL_SignedWord != ((int32_t)inIndicesParams->elementType))
    {
      tidl_printf(0, "Indice data type should be int32");
      status = TIDL_ERR_FAILURE;
    }

    else
    {
      status = TIDL_refGatherND((uint16_t *)data,
                              (int32_t *)indices,
                              (uint16_t *)outPtr,
                              intAlgHandle,
                              layerIdx,
                              params,
                              algLayer,
                              inDataParams,
                              inIndicesParams,
                              outDataParams);
    }
  }
  else if (TIDL_SignedWord == ((int32_t)inDataParams->elementType))
  {
    if (TIDL_SignedWord != ((int32_t)inIndicesParams->elementType))
    {
      tidl_printf(0, "Indice data type should be int32");
      status = TIDL_ERR_FAILURE;
    }
    else
    {
      status = TIDL_refGatherND((int32_t *)data,
                              (int32_t *)indices,
                              (int32_t *)outPtr,
                              intAlgHandle,
                              layerIdx,
                              params,
                              algLayer,
                              inDataParams,
                              inIndicesParams,
                              outDataParams);
    }
  }
  else if (TIDL_UnsignedWord == ((int32_t)inDataParams->elementType))
  {
    if (TIDL_SignedWord != ((int32_t)inIndicesParams->elementType))
    {
      tidl_printf(0, "Indice data type should be int32");
      status = TIDL_ERR_FAILURE;
    }
    else
    {
      status = TIDL_refGatherND((uint32_t *)data,
                              (int32_t *)indices,
                              (uint32_t *)outPtr,
                              intAlgHandle,
                              layerIdx,
                              params,
                              algLayer,
                              inDataParams,
                              inIndicesParams,
                              outDataParams);
    }
  }
  else if (TIDL_SinglePrecFloat == ((int32_t)inDataParams->elementType))
  {
    /*In stat collection , all input tensor is assumed as float.
      hence making indices as float in this flow
    */
    if (TIDL_SignedWord == ((int32_t)inIndicesParams->elementType))
    {
      status = TIDL_refGatherND((float32_tidl *)data,
                              (int32_t *)indices,
                              (float32_tidl *)outPtr,
                              intAlgHandle,
                              layerIdx,
                              params,
                              algLayer,
                              inDataParams,
                              inIndicesParams,
                              outDataParams);
    }
    else
    {
      status = TIDL_refGatherND((float32_tidl *)data,
                              (float32_tidl *)indices,
                              (float32_tidl *)outPtr,
                              intAlgHandle,
                              layerIdx,
                              params,
                              algLayer,
                              inDataParams,
                              inIndicesParams,
                              outDataParams);
    }
  }
  else
  {
    status = TIDL_ERR_FAILURE;
  }
  TIDL_L1DandL2CacheWbInv();
  return status;
}
