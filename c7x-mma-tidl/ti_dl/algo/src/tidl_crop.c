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
----------------------------------------------------------------------------
@file    tidl_crop.c
@brief   This file defines private functions for crop layer.
@version 0.1 (June 2019) : Initial version [ADK]
----------------------------------------------------------------------------
*/

#include "tidl_alg_int.h"
#include "tidl_crop.h"
#include "tidl_commonUtils.h"
#include "tidl_device_utils.h"
#include "tidl_crop_ref.h"

int32_t TIDL_cropProcessNew(TIDL_NetworkCommonParams *commonParams,
                            sTIDL_AlgLayer_t *algLayer,
                            sTIDL_Layer_t *tidlLayer,
                            void *inPtrs[],
                            void *outPtrs[],
                            int32_t layerIdx)
{
  int32_t status = IALG_EOK;

#ifdef HOST_EMULATION

  int32_t multiCoreMode = (algLayer->workloadUnit != NULL)
                            ? algLayer->workloadUnit->multiCoreMode
                            : TIDL_NOT_MULTI_CORE;

  if (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != 0U)
  {
    sTIDL_DataParams_t *inDataParams = &commonParams->createParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[0]].outData;
    const sTIDL_CropParams_t *params = &tidlLayer->layerParams.cropParams;

    int32_t outWidth = tidlLayer->outData.dimValues[TIDL_DIM_WIDTH];
    int32_t outHeight = tidlLayer->outData.dimValues[TIDL_DIM_HEIGHT];
    int32_t inChs = inDataParams->dimValues[TIDL_DIM_NUMCH];
    int32_t numDim1 = inDataParams->dimValues[TIDL_DIM_DIM1];
    int32_t numDim2 = inDataParams->dimValues[TIDL_DIM_DIM2];
    int32_t numROIs = inDataParams->dimValues[TIDL_DIM_BATCH];
    int32_t inPitch = (int32_t)inDataParams->pitch[TIDL_LINE_PITCH];
    int32_t outPitch = (int32_t)tidlLayer->outData.pitch[TIDL_LINE_PITCH];
    int32_t inChPitch = (int32_t)inDataParams->pitch[TIDL_CHANNEL_PITCH];
    int32_t inDim2Pitch = inChs * inChPitch;
    int32_t inDim1Pitch = numDim2 * inDim2Pitch;
    int32_t inROIPitch = numDim1 * inDim1Pitch;
    int32_t outChPitch = (int32_t)tidlLayer->outData.pitch[TIDL_CHANNEL_PITCH];
    int32_t outDim2Pitch = inChs * outChPitch;
    int32_t outDim1Pitch = numDim2 * outDim2Pitch;
    int32_t outROIPitch = numDim1 * outDim1Pitch;
    int32_t cropOffsetW = params->offsetW;
    int32_t cropOffsetH = params->offsetH;
    int32_t inPtrOffset = 0;
    int32_t outPtrOffset = 0;

    if (commonParams->gcHelperHandle == NULL)
    {
      inPtrOffset = (inPitch * cropOffsetH) + cropOffsetW;
    }
#if TIDL_DEVICE_MULTICORE
    else if ((multiCoreMode == TIDL_MULTI_CORE_SPATIAL_WITH_JOIN))
    {
      TIDL_GetTensorDimensions(commonParams->createParams->net, algLayer->workloadUnit, algLayer->layerIdx, NULL, NULL, NULL, NULL, &outHeight, NULL);
    }
    else if ((multiCoreMode == TIDL_MULTI_CORE_CHANNEL_WITH_JOIN))
    {
      TIDL_GetTensorDimensions(commonParams->createParams->net, algLayer->workloadUnit, algLayer->layerIdx, NULL, NULL, &inChs, NULL, NULL, NULL);
    }
#endif
    else
    {
      /*DO Nothing*/
    }
    if (inDataParams->elementType == TIDL_SinglePrecFloat)
    {
      TIDL_cropRefProcess((float32_tidl *)inPtrs[0], (float32_tidl *)outPtrs[0], inPtrOffset, outPtrOffset,
                          outWidth, outHeight, inChs, numDim2, numDim1, numROIs, inPitch, outPitch, inChPitch, inDim2Pitch, inDim1Pitch, inROIPitch, outChPitch, outDim2Pitch, outDim1Pitch, outROIPitch);
    }
    else if (inDataParams->elementType == TIDL_BFloat16)
    {
      TIDL_cropRefProcess((bfloat16_tidl *)inPtrs[0], (bfloat16_tidl *)outPtrs[0], inPtrOffset, outPtrOffset,
                          outWidth, outHeight, inChs, numDim2, numDim1, numROIs, inPitch, outPitch, inChPitch, inDim2Pitch, inDim1Pitch, inROIPitch, outChPitch, outDim2Pitch, outDim1Pitch, outROIPitch);
    }
    else if ((inDataParams->elementType == TIDL_SignedChar) || (inDataParams->elementType == TIDL_UnsignedChar))
    {
      TIDL_cropRefProcess((uint8_t *)inPtrs[0], (uint8_t *)outPtrs[0], inPtrOffset, outPtrOffset,
                          outWidth, outHeight, inChs, numDim2, numDim1, numROIs, inPitch, outPitch, inChPitch, inDim2Pitch, inDim1Pitch, inROIPitch, outChPitch, outDim2Pitch, outDim1Pitch, outROIPitch);
    }
    else
    {
      TIDL_cropRefProcess((uint16_t *)inPtrs[0], (uint16_t *)outPtrs[0], inPtrOffset, outPtrOffset,
                          outWidth, outHeight, inChs, numDim2, numDim1, numROIs, inPitch, outPitch, inChPitch, inDim2Pitch, inDim1Pitch, inROIPitch, outChPitch, outDim2Pitch, outDim1Pitch, outROIPitch);
    }

    if (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) == TIDL_FLOW_CTRL_REF_STAT)
    {
      TIDL_Obj intAlgObj;
      TIDL_CreateParams createParams;

      (void)memcpy(&createParams, commonParams->createParams, sizeof(TIDL_CreateParams));
      intAlgObj.createParams = (TIDL_CreateParams *)&createParams;

      TIDL_UpdateScaleFactors(&intAlgObj, algLayer->layerIdx, 0, 0, 1);
    }
  }
  else /* if ((intAlgHandle->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != TIDL_FLOW_CTRL_REF_ONLY) */
#endif
  {
    status = TIDL_deviceUtilsCommonProcess(commonParams,
                                           algLayer,
                                           tidlLayer,
                                           inPtrs,
                                           outPtrs,
                                           layerIdx);
  }

  return status;
}
