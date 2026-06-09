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
@file    tidl_batchReshape_ref.c
@brief   This file defines private functions for batchReshape layer.
@version 0.1 (Jun 2021) : Initial version [EPR]
----------------------------------------------------------------------------
*/

#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include "tidl_batchReshape_ref.h"

/**
 * @brief This is reference implementation of batchReshape layer
 *
 * @tparam Tin : template for input data buffers
 * @tparam Tout : template for output data buffers
 * @param  pIn : Pointer to input memory
 * @param  pOut : Pointer to output memory
 * @param inPtrOffset  : offset for the input buffer for batchReshape
 * @param outPtrOffset : offset for the input buffer for batchReshape
 * @param width : width of the output buffer
 * @param height : height of the output buffer
 * @param numChs : no of channels
 * @param numBatches : no of Batches to process
 * @param inLinePitch : pitch of the input buffer
 * @param outLinePitch : pitch of the output buffer
 * @param inChPitch : Chnnel pitch of the input buffer
 * @param outChPitch : Chnnel pitch of the output buffer
 * @param inBatchPitch : Pitch b/w Batches in the input buffer
 * @param outBatchPitch : Pitch b/w Batches in the output buffer
 */
template<class Tin, class Tout> void TIDL_refBatchReshape(
    const Tin *pIn,
    Tout *pOut,
    int32_t inPtrOffset,
    int32_t outPtrOffset,
    int32_t width,
    int32_t height,
    int32_t numChs,
    int32_t numBatches,
    int32_t inLinePitch,
    int32_t outLinePitch,
    int32_t inChPitch,
    int32_t outChPitch,
    int32_t inBatchPitch,
    int32_t outBatchPitch)
{
  int32_t i1, i2, i3, i4;

  OPENACC(data copyin(pIn[:1 + inPtrOffset + ((numBatches - 1) * inBatchPitch) + ((numChs - 1) * inChPitch) + ((height - 1) * inLinePitch) + (width - 1)])
              copy(pOut[:1 + outPtrOffset + ((numBatches - 1) * outBatchPitch) + ((numChs - 1) * outChPitch) + ((height - 1) * outLinePitch) + (width - 1)]))
  OPENACC(parallel loop collapse(4))
  for (i1 = 0; i1 < numBatches; i1++)
  {
    for (i2 = 0; i2 < numChs; i2++)
    {
      for (i3 = 0; i3 < height; i3++)
      {
        for (i4 = 0; i4 < width; i4++)
        {
          int32_t inOffset = inPtrOffset + (i1 * inBatchPitch) + (i2 * inChPitch) + (i3 * inLinePitch) + i4;
          int32_t outOffset = outPtrOffset + (i1 * outBatchPitch) + (i2 * outChPitch) + (i3 * outLinePitch) + i4;
          pOut[outOffset] = pIn[inOffset];
        }
      }
    }
  }
}
/**
 * @brief This is reference implementation of batchReshape layer
 *
 * @param pIn : Pointer to input memory
 * @param pOut : Pointer to output memory
 * @param inPtrOffset  : offset for the input buffer for batchReshape
 * @param outPtrOffset : offset for the input buffer for batchReshape
 * @param width : width of the output buffer
 * @param height : height of the output buffer
 * @param numChs : no of channels
 * @param numBatches : no of Batches to process
 * @param inLinePitch : pitch of the input buffer
 * @param outLinePitch : pitch of the output buffer
 * @param inChPitch : Chnnel pitch of the input buffer
 * @param outChPitch : Chnnel pitch of the output buffer
 * @param inBatchPitch : Pitch b/w Batches in the input buffer
 * @param outBatchPitch : Pitch b/w Batches in the output buffer
 * @param elementType : elementType of the output buffer
 */
int32_t TIDL_refBatchReshapeiX(
    void *pIn,
    void *pOut,
    int32_t inPtrOffset,
    int32_t outPtrOffset,
    int32_t width,
    int32_t height,
    int32_t numChs,
    int32_t numBatches,
    int32_t inLinePitch,
    int32_t outLinePitch,
    int32_t inChPitch,
    int32_t outChPitch,
    int32_t inBatchPitch,
    int32_t outBatchPitch,
    int32_t elementType)
{
  int32_t status = TIDL_SUCCESS;
  if (elementType == TIDL_SinglePrecFloat)
  {
    TIDL_refBatchReshape((float32_tidl *)pIn, (float32_tidl *)pOut, inPtrOffset, outPtrOffset, width, height, numChs,
                         numBatches, inLinePitch, outLinePitch, inChPitch, outChPitch, inBatchPitch, outBatchPitch);
  }
  else if ((elementType == TIDL_SignedChar) || (elementType == TIDL_UnsignedChar))
  {
    TIDL_refBatchReshape((uint8_t *)pIn, (uint8_t *)pOut, inPtrOffset, outPtrOffset, width, height, numChs,
                         numBatches, inLinePitch, outLinePitch, inChPitch, outChPitch, inBatchPitch, outBatchPitch);
  }
  else if ((elementType == TIDL_SignedShort) || (elementType == TIDL_UnsignedShort))
  {
    TIDL_refBatchReshape((uint16_t *)pIn, (uint16_t *)pOut, inPtrOffset, outPtrOffset, width, height, numChs,
                         numBatches, inLinePitch, outLinePitch, inChPitch, outChPitch, inBatchPitch, outBatchPitch);
  }
  else
  {
    /* tidl_printf(0, " Un Supported elementType , TIDL_ERROR_BATCHRESHAPE_UNSUPPORTED_DATA_TYPE, Exiting from %s, %d\n", __FILE__, __LINE__);*/
    TIDL_LOG_ERROR(TIDL_ERROR_GROUP_BATCHRESHAPE, TIDL_ERROR_BATCHRESHAPE_UNSUPPORTED_DATA_TYPE);
    status = TIDL_ERR_FAILURE;
  }
  if (status == TIDL_SUCCESS)
  {
    TIDL_L1DandL2CacheWbInv();
  }
  return status;
}

int32_t TIDL_batchReshapeRefProcess(TIDL_NetworkCommonParams *commonParams,
                                    sTIDL_AlgLayer_t *algLayer,
                                    sTIDL_Layer_t *tidlLayer,
                                    void *inPtrs[],
                                    void *outPtrs[],
                                    int32_t layerIdx)
{
  int32_t status = TIDL_SUCCESS;
  sTIDL_DataParams_t *inDataParams = TIDL_getDataParams(commonParams->createParams->net, tidlLayer->inData[0]);
  sTIDL_DataParams_t *outDataParams = &tidlLayer->outData;
  int32_t inWidth;  //= (int32_t)inDataParams->dimValues[TIDL_DIM_WIDTH];
  int32_t inHeight; //= (int32_t)inDataParams->dimValues[TIDL_DIM_HEIGHT];
  int32_t inNumChs; //= (int32_t)inDataParams->dimValues[TIDL_DIM_NUMCH];
  int32_t inPitch = (int32_t)inDataParams->pitch[TIDL_LINE_PITCH];
  int32_t outPitch = (int32_t)tidlLayer->outData.pitch[TIDL_LINE_PITCH];
  int32_t inChPitch = (int32_t)inDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t outChPitch = (int32_t)tidlLayer->outData.pitch[TIDL_CHANNEL_PITCH];
  int32_t inBatchPitch = (int32_t)inDataParams->pitch[TIDL_ROI_PITCH];
  int32_t outBatchPitch = (int32_t)outDataParams->pitch[TIDL_ROI_PITCH];
  int32_t inPtrOffset = 0;
  int32_t outPtrOffset = 0;
  int32_t numBatchs = inDataParams->numBatchW * inDataParams->dimValues[TIDL_DIM_BATCH];
  void *refInPtr;
  void *refOutPtr;
  void *targetInPtr;
  void *inPtr = inPtrs[0];
  uint8_t (*outPtr)[] = (uint8_t (*)[])outPtrs[0];

  int32_t inBatch = inDataParams->dimValues[TIDL_DIM_BATCH];
  int32_t outBatch = tidlLayer->outData.dimValues[TIDL_DIM_BATCH];

  if ((uint32_t)commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_BATCHRESHAPE_FORCE_INBATCH_OUTBATCH_NULL)
  {
    inBatch = NULL;
    outBatch = NULL;
  }

  // TODO: Need to review this after migrating to using gc info
  TIDL_GetTensorDimensions(commonParams->net,
                           algLayer->workloadUnit,
                           layerIdx,
                           &inWidth,
                           &inHeight,
                           &inNumChs,
                           NULL,
                           NULL,
                           NULL);

  TIDL_GetTensorDimensionsBatchReshape(commonParams->net,
                                       algLayer->workloadUnit,
                                       layerIdx,
                                       &inBatch,
                                       &outBatch);
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  This is a safety check to ensure that the batch reshape layer can handle the case where the inBatch/outBatch is less/greater than the numBatches.
  <justification end> */
  numBatchs = MAX(inBatch, numBatchs);
  numBatchs = MAX(outBatch, numBatchs);
  /* LDRA_JUSTIFY_END */
  /**
   * Looks like the input width for batch is the total width of all tensors in the batch stitched together along the
   * width axis. The workload flow is expected to give the width of the individual tensor and not of the whole stitched
   * batch.
   *
   *  <-----------Input  Width----------->
   *  <----B1----><----B2----><----B3---->
   *  ####################################
   *  ###@@@@@@@@@###@@@@@@@@@###@@@@@@@@@        @: Valid Data
   *  ###@@@@@@@@@###@@@@@@@@@###@@@@@@@@@        #: Pad pixel
   *  ###@@@@@@@@@###@@@@@@@@@###@@@@@@@@@
   *  ####################################
   */
  int32_t orgWidth = (inWidth - (inDataParams->batchPadW * (numBatchs - 1))) / numBatchs;
  int32_t orgHeight = inHeight;

  if ((inBatch > 1) && (outBatch == 1)) // Stitched layer
  {
    orgWidth = inWidth;
    outBatchPitch = orgWidth + tidlLayer->outData.batchPadW;
  }
  else if ((outBatch > 1) && (inBatch == 1)) // UnStitch layer
  {
    inBatchPitch = orgWidth + inDataParams->batchPadW;
  }
  else
  {
    inBatchPitch = orgWidth + inDataParams->batchPadW;
    outBatchPitch = orgWidth + tidlLayer->outData.batchPadW;
  }

  if (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) == TIDL_FLOW_CTRL_REF_STAT)
  {
    TIDL_Obj intAlgObj;
    TIDL_CreateParams createParams;
    (void)memcpy(&createParams, commonParams->createParams, sizeof(TIDL_CreateParams));
    intAlgObj.createParams = (TIDL_CreateParams *)&createParams;

    TIDL_UpdateScaleFactors(&intAlgObj, layerIdx, 0, 0, 1);
  }

  targetInPtr = ((int8_t *)inPtr);
  refOutPtr = (*outPtr);
  refInPtr = targetInPtr;

  status = TIDL_refBatchReshapeiX((void *)refInPtr, (void *)refOutPtr, inPtrOffset, outPtrOffset, orgWidth, orgHeight,
                                  inNumChs, numBatchs, inPitch, outPitch, inChPitch, outChPitch, inBatchPitch, outBatchPitch,
                                  tidlLayer->outData.elementType);

  return status;
}
