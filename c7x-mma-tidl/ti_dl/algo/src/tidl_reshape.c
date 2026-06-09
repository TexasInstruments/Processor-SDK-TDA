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
 *  \file tidl_reshape_ref.c
 *
 *  \brief This file defines kernel functions for reshape layer
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "tidl_reshape.h"
#include "gc.h"
#include "gc_helper.h"

int32_t TIDL_reshapeAllocNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                             const TIDL_NetworkCommonParams *commonParams,
                             int32_t layerIdx,
                             int32_t memorySize[TIDL_LAYER_MEMORY_MAX])
{
  int32_t status = IALG_EOK;

  if (commonParams->gcHelperHandle == NULL)
  {
#ifdef HOST_EMULATION
    status = TIDL_deviceCommonRefAlloc(layerSpecificParams, commonParams, layerIdx, memorySize);
#endif
  }
  return status;
}

int32_t TIDL_reshapeInitNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                            const TIDL_NetworkCommonParams *commonParams,
                            sTIDL_AlgLayer_t *algLayer,
                            int32_t layerIdx,
                            uint8_t *memory[TIDL_LAYER_MEMORY_MAX],
                            int32_t memorySize[TIDL_LAYER_MEMORY_MAX],
                            void **outPtr)
{
  int32_t status = IALG_EOK;
#ifdef HOST_EMULATION
  if (commonParams->gcHelperHandle == NULL)
  {
    status = TIDL_deviceCommonRefInit(layerSpecificParams,
                                      commonParams,
                                      algLayer,
                                      layerIdx,
                                      memory,
                                      memorySize,
                                      outPtr);
  }
#endif
  return status;
}

#ifdef HOST_EMULATION
static int32_t tidl_getInputElemIdxWithPad(sTIDL_DataParams_t *inDataParams, int32_t outElemIdx)
{
  int32_t inElemIdxWithPad = 0, dimIdx = 0;
  uint32_t i0 = 0, i1 = 0;
  int32_t inputDims[TIDL_DIM_MAX];
  int32_t pitchesWithoutPads[TIDL_DIM_MAX];
  int32_t outElemIdxLoc = outElemIdx;

  for (i0 = 0; i0 < TIDL_DIM_MAX; i0++)
  {
    pitchesWithoutPads[i0] = 1;
    for (i1 = TIDL_DIM_WIDTH; i1 > i0; i1--)
    {
      pitchesWithoutPads[i0] *= inDataParams->dimValues[i1];
    }
  }
  pitchesWithoutPads[TIDL_DIM_WIDTH] = 1;

  for (i0 = 0; i0 < TIDL_DIM_MAX; i0++)
  {
    dimIdx = outElemIdxLoc / pitchesWithoutPads[i0];
    inputDims[i0] = dimIdx;
    outElemIdxLoc = (outElemIdxLoc % pitchesWithoutPads[i0]);
  }

  inElemIdxWithPad = (inputDims[TIDL_DIM_BATCH] * inDataParams->pitch[TIDL_ROI_PITCH]) +
                     (inputDims[TIDL_DIM_DIM1] * inDataParams->pitch[TIDL_DIM1_PITCH]) +
                     (inputDims[TIDL_DIM_DIM2] * inDataParams->pitch[TIDL_DIM2_PITCH]) +
                     (inputDims[TIDL_DIM_NUMCH] * inDataParams->pitch[TIDL_CHANNEL_PITCH]) +
                     (inputDims[TIDL_DIM_HEIGHT] * inDataParams->pitch[TIDL_LINE_PITCH]) +
                     inputDims[TIDL_DIM_WIDTH];

  return inElemIdxWithPad;
}
#endif
#ifdef HOST_EMULATION
template<typename Tin>
    int32_t TIDL_reshapeProcess(TIDL_NetworkCommonParams *commonParams,
                                sTIDL_AlgLayer_t *algLayer,
                                sTIDL_Layer_t *tidlLayer,
                                Tin *src,
                                Tin *dst,
                                int32_t layerIdx)
{
  int32_t status = TIDL_SUCCESS;

  /*Copy the data for reference flow if perfsimInfo not available*/
  if (commonParams->gcHelperHandle == NULL)
  {
    int32_t outElemIdxWithPad, outElemIdx, inputElemIdxWithPad, pitchesWithoutPads[TIDL_DIM_MAX];
    int32_t i0, i1, i2, i3, i4, i5;
    sTIDL_Layer_t* layer = &commonParams->createParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[0]];
    sTIDL_DataParams_t *inDataParams   = &layer->outData;
    for (i0 = 0; i0 < TIDL_DIM_MAX; i0++)
    {
      pitchesWithoutPads[i0] = 1;
      for (i1 = TIDL_DIM_WIDTH; i1 > i0; i1--)
      {
        pitchesWithoutPads[i0] *= tidlLayer->outData.dimValues[i1];
      }
    }
    /* Copy the data from input to output element wise */
    for (i0 = 0; i0 < tidlLayer->outData.dimValues[TIDL_DIM_BATCH]; i0++)
    {
      for (i1 = 0; i1 < tidlLayer->outData.dimValues[TIDL_DIM_DIM1]; i1++)
      {
        for (i2 = 0; i2 < tidlLayer->outData.dimValues[TIDL_DIM_DIM2]; i2++)
        {
          for (i3 = 0; i3 < tidlLayer->outData.dimValues[TIDL_DIM_NUMCH]; i3++)
          {
            for (i4 = 0; i4 < tidlLayer->outData.dimValues[TIDL_DIM_HEIGHT]; i4++)
            {
              for (i5 = 0; i5 < tidlLayer->outData.dimValues[TIDL_DIM_WIDTH]; i5++)
              {
                outElemIdxWithPad = i0 * tidlLayer->outData.pitch[TIDL_ROI_PITCH] +
                                    i1 * tidlLayer->outData.pitch[TIDL_DIM1_PITCH] +
                                    i2 * tidlLayer->outData.pitch[TIDL_DIM2_PITCH] +
                                    i3 * tidlLayer->outData.pitch[TIDL_CHANNEL_PITCH] +
                                    i4 * tidlLayer->outData.pitch[TIDL_LINE_PITCH] +
                                    i5;
                outElemIdx = i0 * pitchesWithoutPads[TIDL_DIM_BATCH] +
                             i1 * pitchesWithoutPads[TIDL_DIM_DIM1] +
                             i2 * pitchesWithoutPads[TIDL_DIM_DIM2] +
                             i3 * pitchesWithoutPads[TIDL_DIM_NUMCH] +
                             i4 * pitchesWithoutPads[TIDL_DIM_HEIGHT] +
                             i5 * pitchesWithoutPads[TIDL_DIM_WIDTH];
                inputElemIdxWithPad = tidl_getInputElemIdxWithPad(inDataParams, outElemIdx);
                dst[outElemIdxWithPad] = src[inputElemIdxWithPad];
              }
            }
          }
        }
      }
    }
    TIDL_L1DandL2CacheWbInv();
  }/*With NC, in ref flow memcopy is removed from here, because inptr (not the scratch ptr which is passed in here but the original one) and outptr are same*/
  else
  {
    /* do nothing */
  }

  return status;
}
#endif

int32_t TIDL_reshapeProcessNew(TIDL_NetworkCommonParams *commonParams,
                               sTIDL_AlgLayer_t *algLayer,
                               sTIDL_Layer_t *tidlLayer,
                               void *inPtrs[],
                               void *outPtrs[],
                               int32_t layerIdx)
{
  int32_t status = TIDL_SUCCESS;

#ifdef HOST_EMULATION // body of TIDL_reshapeProcess() is under HOST_EMULATION protection, moving calling func as well
  if (commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_RESHAPE_FORCE_ELEMENTTYPE_MAX)
  {
    tidlLayer->outData.elementType = TIDL_SignedDoubleWord + 1;
  }

  if (tidlLayer->outData.elementType == TIDL_SinglePrecFloat)
  {
    status = TIDL_reshapeProcess(commonParams,
                                 algLayer,
                                 tidlLayer,
                                 (float32_tidl *)inPtrs[0],
                                 (float32_tidl *)outPtrs[0],
                                 layerIdx);
  }
  else if ((tidlLayer->outData.elementType == TIDL_SignedChar) || (tidlLayer->outData.elementType == TIDL_UnsignedChar))
  {
    status = TIDL_reshapeProcess(commonParams,
                                 algLayer,
                                 tidlLayer,
                                 (uint8_t *)inPtrs[0],
                                 (uint8_t *)outPtrs[0],
                                 layerIdx);
  }
  else if ((tidlLayer->outData.elementType == TIDL_SignedShort) || (tidlLayer->outData.elementType == TIDL_UnsignedShort))
  {
    status = TIDL_reshapeProcess(commonParams,
                                 algLayer,
                                 tidlLayer,
                                 (uint16_t *)inPtrs[0],
                                 (uint16_t *)outPtrs[0],
                                 layerIdx);
  }
  else if ((tidlLayer->outData.elementType == TIDL_SignedWord) || (tidlLayer->outData.elementType == TIDL_UnsignedWord))
  {
    status = TIDL_reshapeProcess(commonParams,
                                 algLayer,
                                 tidlLayer,
                                 (uint32_t *)inPtrs[0],
                                 (uint32_t *)outPtrs[0],
                                 layerIdx);
  }
#ifdef TIDL_COVERAGE_DEAD_CODE_NO_TEST
  else if ((tidlLayer->outData.elementType == TIDL_SignedDoubleWord) || (tidlLayer->outData.elementType == TIDL_UnsignedDoubleWord))
  {
    status = TIDL_reshapeProcess(commonParams,
                                 algLayer,
                                 tidlLayer,
                                 (uint64_t *)inPtrs[0],
                                 (uint64_t *)outPtrs[0],
                                 layerIdx);
  }
#endif
  else
  {
    status = IALG_EFAIL;
  }
#endif
  return status;
}
