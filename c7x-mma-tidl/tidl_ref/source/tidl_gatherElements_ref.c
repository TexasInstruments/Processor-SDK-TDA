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
 *  \file tidl_gatherElements_ref.c
 *
 *  \brief This file defines kernel functions for GatherElements layer
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include <math.h>
#include "tidl_gatherElements_ref.h"

/**
 * @brief This is main function perform gatherElements on differnt elementTypes
 *
 * @tparam Tin  : template for input
 * @tparam Tw   : template for weights
 * @tparam Tb   : template for Bias
 * @tparam Tout : template for output
 * @tparam Tsat : template for saturate values
 * @param inPtr : Input pointer on which bacthNorm is applied
 * @param outPtr : Output pointer after gatherElements opreation
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

template<class Tin, class Tindx, class Tout> static int32_t TIDL_refGatherElements(Tin *data,
                                                                           Tindx *indices,
                                                                           Tout *outPtr,
                                                                           TIDL_Handle intAlgHandle,
                                                                           int32_t layerIdx,
                                                                           sTIDL_GatherElementsParams_t *params,
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
  int32_t inDataDim2Pitch     = (int32_t)inDataParams->pitch[TIDL_DIM_DIM2];
  int32_t inDataDim1Pitch     = (int32_t)inDataParams->pitch[TIDL_DIM_DIM1];
  
  int32_t inIndicesPitch      = (int32_t)inIndicesParams->pitch[TIDL_LINE_PITCH];
  int32_t inIndicesChPitch    = (int32_t)inIndicesParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t inIndicesRoiPitch   = (int32_t)inIndicesParams->pitch[TIDL_ROI_PITCH];
  int32_t inIndicesDim2Pitch  = (int32_t)inIndicesParams->pitch[TIDL_DIM_DIM2];
  int32_t inIndicesDim1Pitch  = (int32_t)inIndicesParams->pitch[TIDL_DIM_DIM1];

  int32_t outPitch            = (int32_t)outDataParams->pitch[TIDL_LINE_PITCH];
  int32_t outChPitch          = (int32_t)outDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t outRoiPitch         = (int32_t)outDataParams->pitch[TIDL_ROI_PITCH];
  int32_t outDim2Pitch        = (int32_t)outDataParams->pitch[TIDL_DIM_DIM2];
  int32_t outDim1Pitch        = (int32_t)outDataParams->pitch[TIDL_DIM_DIM1];

  data = (Tin *)data + (inDataParams->padH * inDataPitch) + inDataParams->padW;
  indices = (Tindx *)indices + (inIndicesParams->padH * inIndicesPitch) + inIndicesParams->padW;
  Tout *output = (Tout *)outPtr + (outDataParams->padH * outPitch) + outDataParams->padW;

  int32_t axis = params->axis;


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
              for(i5=0; i5<inIndicesNumCols; i5++)
              {
                index = (int32_t)indices[(i0*inIndicesRoiPitch) + (i1*inIndicesDim1Pitch) + (i2*inIndicesDim2Pitch) + (i3*inIndicesChPitch) + (i4*inIndicesPitch) + i5];
                
                if(axis == TIDL_DIM_BATCH)
                {
                  if(index < 0)
                  {
                    index = (int32_t)inDataParams->dimValues[TIDL_DIM_BATCH] + index;
                  }
                  output[(i0*outRoiPitch) + (i1*outDim1Pitch) + (i2*outDim2Pitch) + (i3*outChPitch) + (i4*outPitch) + i5] = data[(index * inDataRoiPitch) + (i1*inDataDim1Pitch) + (i2*inDataDim2Pitch) + (i3*inDataChPitch) + (i4*inDataPitch) + i5];
                }
                else if(axis == TIDL_DIM_DIM1)
                {
                  if(index < 0)
                  {
                    index = (int32_t)inDataParams->dimValues[TIDL_DIM_DIM1] + index;
                  }
                  output[(i0*outRoiPitch) + (i1*outDim1Pitch) + (i2*outDim2Pitch) + (i3*outChPitch) + (i4*outPitch) + i5] = data[(i0*inDataRoiPitch) + (index * inDataDim1Pitch) + (i2*inDataDim2Pitch) + (i3*inDataChPitch) + (i4*inDataPitch) + i5];
                }
                else if(axis == TIDL_DIM_DIM2)
                {
                  if(index < 0)
                  {
                    index = (int32_t)inDataParams->dimValues[TIDL_DIM_DIM2] + index;
                  }
                  output[(i0*outRoiPitch) + (i1*outDim1Pitch) + (i2*outDim2Pitch) + (i3*outChPitch) + (i4*outPitch) + i5] = data[(i0*inDataRoiPitch) + (i1*inDataDim1Pitch) + (index * inDataDim2Pitch) + (i3*inDataChPitch) + (i4*inDataPitch) + i5] ;
                }
                else if(axis == TIDL_DIM_NUMCH)
                {
                  if(index < 0)
                  {
                    index = (int32_t)inDataParams->dimValues[TIDL_DIM_NUMCH] + index;
                  }
                  output[(i0*outRoiPitch) + (i1*outDim1Pitch) + (i2*outDim2Pitch) + (i3*outChPitch) + (i4*outPitch) + i5] = data[(i0*inDataRoiPitch) + (i1*inDataDim1Pitch) + (i2*inDataDim2Pitch) + (index * inDataChPitch) + (i4*inDataPitch) + i5];
                }
                else if(axis == TIDL_DIM_HEIGHT)
                {
                  if(index < 0)
                  {
                    index = (int32_t)inDataParams->dimValues[TIDL_DIM_HEIGHT] + index;
                  }
                  output[(i0*outRoiPitch) + (i1*outDim1Pitch) + (i2*outDim2Pitch) + (i3*outChPitch) + (i4*outPitch) + i5] = data[(i0*inDataRoiPitch) + (i1*inDataDim1Pitch) + (i2*inDataDim2Pitch) + (i3*inDataChPitch) + (index * inDataPitch) + i5];
                }
                else if(axis == TIDL_DIM_WIDTH)
                {
                  if(index < 0)
                  {
                    index = (int32_t)inDataParams->dimValues[TIDL_DIM_WIDTH] + index;
                  }
                  output[(i0*outRoiPitch) + (i1*outDim1Pitch) + (i2*outDim2Pitch) + (i3*outChPitch) + (i4*outPitch) + i5] = data[(i0*inDataRoiPitch) + (i1*inDataDim1Pitch) + (i2*inDataDim2Pitch) + (i3*inDataChPitch) + (i4*inDataPitch) + index];
                }
                else
                {
                  status = TIDL_ERR_FAILURE;
                  break;
                }
              }
            }
          }
        }
      }
    }

  return status;
}

/**
 * @brief gatherElements layer reference implementation
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
int32_t TIDL_gatherElementsRefProcess(TIDL_Handle intAlgHandle,
                              sTIDL_AlgLayer_t *algLayer,
                              const sTIDL_Layer_t *tidlLayer,
                              sTIDL_GatherElementsParams_t *params,
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
      tidl_printf(0, "Indice data type should int32");
      status = TIDL_ERR_FAILURE;
    }
    else
    {
      status = TIDL_refGatherElements((int8_t *)data,
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
      tidl_printf(0, "Indice data type should int32");
      status = TIDL_ERR_FAILURE;
    }
    else
    {
      status = TIDL_refGatherElements((uint8_t *)data,
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
      tidl_printf(0, "Indice data type should int32");
      status = TIDL_ERR_FAILURE;
    }
    else
    {
      status = TIDL_refGatherElements((int16_t *)data,
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
      tidl_printf(0, "Indice data type should int32");
      status = TIDL_ERR_FAILURE;
    }

    else
    {
      status = TIDL_refGatherElements((uint16_t *)data,
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
      tidl_printf(0, "Indice data type should int32");
      status = TIDL_ERR_FAILURE;
    }
    else
    {
      status = TIDL_refGatherElements((int32_t *)data,
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
      tidl_printf(0, "Indice data type should int32");
      status = TIDL_ERR_FAILURE;
    }
    else
    {
      status = TIDL_refGatherElements((uint32_t *)data,
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
      status = TIDL_refGatherElements((float32_tidl *)data,
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
      status = TIDL_refGatherElements((float32_tidl *)data,
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
