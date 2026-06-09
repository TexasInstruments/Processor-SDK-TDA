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
@file    tidl_slice.c
@brief   This file defines private functions for Slice layer.
@version 0.1 (June 2019) : Initial version [ADK]
----------------------------------------------------------------------------
*/
#include "tidl_alg_int.h"
#include "tidl_slice.h"
#include "tidl_commonUtils.h"
#include "tidl_device_utils.h"
#include "tidl_slice_ref.h"

int32_t TIDL_sliceProcessNew(TIDL_NetworkCommonParams *commonParams,
                              sTIDL_AlgLayer_t     * algLayer,
                              sTIDL_Layer_t        * tidlLayer,
                              void                 * inPtrs[],
                              void                 * outPtrs[],
                              int32_t                layerIdx)
{
  int32_t status             = IALG_EOK;

  void    *inPtr = inPtrs[0];
  uint8_t(*outPtr)[] = (uint8_t(*)[])outPtrs[0];
  #ifdef HOST_EMULATION
  if (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != TIDL_FLOW_CTRL_REF_ONLY)
  #endif
  {
    if(inPtr != outPtr)
    {
      status = TIDL_deviceUtilsCommonProcess(commonParams,
                                  algLayer,
                                  tidlLayer,
                                  inPtrs,
                                  outPtrs,
                                  layerIdx);
    }
  }
  #ifdef HOST_EMULATION
  else
  {
    sTIDL_DataParams_t *inDataParams   = &commonParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[0]].outData;
    int32_t outWidth           = tidlLayer->outData.dimValues[TIDL_DIM_WIDTH];
    int32_t outHeight          = tidlLayer->outData.dimValues[TIDL_DIM_HEIGHT];
    int32_t outChs             = tidlLayer->outData.dimValues[TIDL_DIM_NUMCH];
    int32_t outDim1            = tidlLayer->outData.dimValues[TIDL_DIM_DIM1];
    int32_t outDim2            = tidlLayer->outData.dimValues[TIDL_DIM_DIM2];
    int32_t numROIs            =  inDataParams->dimValues[TIDL_DIM_BATCH]; /*ROI  */
    int32_t inPitch            = (int32_t)inDataParams->pitch[TIDL_LINE_PITCH];
    int32_t outPitch           = (int32_t)tidlLayer->outData.pitch[TIDL_LINE_PITCH];
    int32_t inChPitch          = (int32_t)inDataParams->pitch[TIDL_CHANNEL_PITCH];
    int32_t outChPitch         = (int32_t)tidlLayer->outData.pitch[TIDL_CHANNEL_PITCH];
    int32_t inDim1Pitch          = (int32_t)inDataParams->pitch[TIDL_DIM1_PITCH];
    int32_t inDim2Pitch          = (int32_t)inDataParams->pitch[TIDL_DIM2_PITCH];
    int32_t inROIPitch         = (int32_t)inDataParams->pitch[TIDL_ROI_PITCH];
    int32_t outROIPitch        = (int32_t)tidlLayer->outData.pitch[TIDL_ROI_PITCH];
    int32_t outDim1Pitch        = (int32_t)tidlLayer->outData.pitch[TIDL_DIM_DIM1];
    int32_t outDim2Pitch        = (int32_t)tidlLayer->outData.pitch[TIDL_DIM_DIM2];
    const sTIDL_SliceLayerParams_t *params = &tidlLayer->layerParams.sliceParams;
    int32_t slicePoint        = params->slicePoints[0];
    int32_t inPtrOffset        = 0;
    int32_t outPtrOffset       = 0;
    void * refInPtr;
    void * refOutPtr;
    void * targetInPtr;
    
    if (commonParams->gcHelperHandle == NULL)
    {
      if(params->axis == TIDL_DIM_DIM1)
      {
        inPtrOffset       += (slicePoint*inDim1Pitch);
      }
      else if(params->axis == TIDL_DIM_DIM2)
      {
        inPtrOffset       += (slicePoint*inDim2Pitch);
      }
      else 
      if(params->axis == TIDL_DIM_NUMCH)
      {
        inPtrOffset       += (slicePoint*inChPitch);
      }
      else if(params->axis == TIDL_DIM_HEIGHT)
      {
        inPtrOffset       += (slicePoint*inPitch);
      }
      else if(params->axis == TIDL_DIM_WIDTH)
      {
        inPtrOffset       += (slicePoint);
      }
      else
      {
        status = IALG_EFAIL;
      }
    }

    if(status == IALG_EOK)
    {
      targetInPtr = ((int8_t *)inPtr);
      refOutPtr   =(*outPtr);
      refInPtr    = targetInPtr;
      
      if (inDataParams->elementType == TIDL_SinglePrecFloat)
      {
        TIDL_sliceRefProcess((float32_tidl *)refInPtr, (float32_tidl *)refOutPtr, inPtrOffset, outPtrOffset, outWidth, outHeight, outChs, outDim1, outDim2, numROIs, inPitch, outPitch, inChPitch, outChPitch, inDim1Pitch, outDim1Pitch, inDim2Pitch, outDim2Pitch, inROIPitch, outROIPitch);
      }
      else if ((inDataParams->elementType == TIDL_SignedChar) || (inDataParams->elementType == TIDL_UnsignedChar))
      {
        TIDL_sliceRefProcess((uint8_t *)refInPtr, (uint8_t *)refOutPtr, inPtrOffset, outPtrOffset, outWidth, outHeight, outChs, outDim1, outDim2, numROIs, inPitch, outPitch, inChPitch, outChPitch, inDim1Pitch, outDim1Pitch, inDim2Pitch, outDim2Pitch, inROIPitch, outROIPitch);
      }
      else if ((inDataParams->elementType == TIDL_SignedShort) || (inDataParams->elementType == TIDL_UnsignedShort))
      {
        TIDL_sliceRefProcess((uint16_t *)refInPtr, (uint16_t *)refOutPtr, inPtrOffset, outPtrOffset, outWidth, outHeight, outChs, outDim1, outDim2, numROIs, inPitch, outPitch, inChPitch, outChPitch, inDim1Pitch, outDim1Pitch, inDim2Pitch, outDim2Pitch, inROIPitch, outROIPitch);
      }

      /* LDRA_JUSTIFY_START
      <metric start> statement branch <metric end>
      <justification start>  LDRA_JUSTIFICATION_SLICE_SAFETY_CHECK:
      safe programming hard to hit this condition with real world data
      <justification end> */
      else if ((inDataParams->elementType == TIDL_UnsignedWord) || (inDataParams->elementType == TIDL_SignedWord))
      /* LDRA_JUSTIFY_END */
      {
        TIDL_sliceRefProcess((uint32_t *)refInPtr, (uint32_t *)refOutPtr, inPtrOffset, outPtrOffset, outWidth, outHeight, outChs, outDim1, outDim2, numROIs, inPitch, outPitch, inChPitch, outChPitch, inDim1Pitch, outDim1Pitch, inDim2Pitch, outDim2Pitch, inROIPitch, outROIPitch);
      }
      /* LDRA_JUSTIFY_START
      <metric start> statement branch <metric end>
      <justification start>  LDRA_JUSTIFICATION_SLICE_SAFETY_CHECK:
      safe programming hard to hit this condition with real world data
      <justification end> */
      else
      {
        status = TIDL_ERR_INVALID_TYPE;
      }
      /* LDRA_JUSTIFY_END */

      if(((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) == TIDL_FLOW_CTRL_REF_STAT)
      {
        TIDL_Obj intAlgObj;
        TIDL_CreateParams createParams;
        (void)memcpy(&createParams, commonParams->createParams, sizeof(TIDL_CreateParams));
        intAlgObj.createParams = (TIDL_CreateParams *) &createParams;
        TIDL_UpdateScaleFactors(&intAlgObj, algLayer->layerIdx, 0, 0, 1);
      }
    }
  }
  #endif

  return status;
}
