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
@file    tidl_tile_ref.c
@brief   This file defines private functions for Tile Layer.
----------------------------------------------------------------------------
*/

#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include "tidl_alg_utils.h"
#include "tidl_tile_ref.h"

int32_t TIDL_repeatData(sTIDL_AlgLayer_t     *algLayer,
                        sTIDL_TileParams_t   *params,
                        sTIDL_DataParams_t   *inDataParams,
                        sTIDL_DataParams_t   *outDataParams,
                        void                 *inPtr,
                        void                 *outPtr)
{
  uint32_t   i0, i1, i2, i3, i4, i5;
  int32_t   status = IALG_EOK;

  uint32_t inBatch = inDataParams->dimValues[TIDL_DIM_BATCH];
  uint32_t inBatchPitch = (uint32_t)inDataParams->pitch[TIDL_ROI_PITCH];

  uint8_t *inData = (uint8_t *)inPtr;
  uint8_t *outData = (uint8_t *)outPtr;

  int32_t elementSize = TIDL_getDatElementSize(inDataParams->elementType);

  uint32_t outIdx[TIDL_DIM_MAX] = {0};
  uint8_t done = 0;
  while (!done)
  {
    uint64_t inOffset = 0;
    uint64_t outOffset = 0;

    for (i0 = 0; i0 < TIDL_DIM_MAX; i0++)
    {
      uint32_t inPitch = 1;
      uint32_t outPitch = 1;
      if (i0 != TIDL_DIM_WIDTH)
      {
        inPitch = inDataParams->pitch[i0];
        outPitch = outDataParams->pitch[i0];
      }

      inOffset += (outIdx[i0] % inDataParams->dimValues[i0]) * inPitch;
      outOffset += outIdx[i0] * outPitch;
    }

    outOffset = outOffset * elementSize;
    inOffset = inOffset * elementSize;

    for (i0 = 0; i0 < elementSize; i0++)
    {
      outData[outOffset + i0] = inData[inOffset + i0];
    }

    for (i0 = TIDL_DIM_WIDTH; i0 >= 0; i0--)
    {
      outIdx[i0]++;
      if(outIdx[i0] < outDataParams->dimValues[i0])
      {
        break;
      }
      if (i0 == 0)
      {
        done = 1;
        break;
      }
      outIdx[i0] = 0;
    }
  }
  TIDL_L1DandL2CacheWbInv();
  return status;
}

int32_t TIDL_TileRefProcess(sTIDL_Network_t      *net,
                            sTIDL_AlgLayer_t     *algLayer,
                            sTIDL_Layer_t        *tidlLayer,
                            void                 *inPtrs[],
                            void                 *outPtrs[],
                            int32_t               flowCtrl,
                            int32_t               layerIdx)
{
  int32_t status = IALG_EOK;
  uint8_t (*inPtr)[] = (uint8_t (*)[])(inPtrs[0]);
  uint8_t (*outPtr)[] = (uint8_t (*)[])(outPtrs[0]);

  sTIDL_TileParams_t  *params    = &tidlLayer->layerParams.tileParams;
  sTIDL_DataParams_t  *inDataParams  = &net->TIDLLayers[algLayer->inLayerIdx[0]].outData;
  sTIDL_DataParams_t  *outDataParams = &tidlLayer->outData;

  status = TIDL_repeatData(algLayer,
                             params,
                             inDataParams,
                             outDataParams,
                             inPtr,
                             outPtr);

  return status;
}
