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
@file    tidl_size_ref.c
@brief   This file defines private functions for Size Layer.
----------------------------------------------------------------------------
*/

#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include "tidl_alg_utils.h"
#include "tidl_size_ref.h"

template <class Tout>
int32_t TIDL_computeInputSize(sTIDL_Network_t      *net,
                              sTIDL_Layer_t        *tidlLayer,
                              sTIDL_DataParams_t   *inDataParams,
                              sTIDL_DataParams_t   *outDataParams,
                              Tout                 *outPtr)
{
  int32_t   i;
  int32_t   status = IALG_EOK;
  int64_t   totalElements = 1;

  for (i = 0; i < (int32_t)TIDL_DIM_MAX; i++)
  {
    totalElements *= inDataParams->dimValues[i];
  }
  // outPtr will always be a scalar (single element) for Size operator
  outPtr[0] = (Tout)totalElements;
  TIDL_L1DandL2CacheWbInv();
  return status;
}

int32_t TIDL_SizeRefProcess(sTIDL_Network_t      *net,
                            sTIDL_AlgLayer_t     *algLayer,
                            sTIDL_Layer_t        *tidlLayer,
                            void                 *inPtrs[],
                            void                 *outPtrs[],
                            int32_t               flowCtrl,
                            int32_t               layerIdx)
{
  int32_t status = IALG_EOK;
  sTIDL_DataParams_t  *inDataParams  = &net->TIDLLayers[algLayer->inLayerIdx[0]].outData;
  sTIDL_DataParams_t  *outDataParams = &tidlLayer->outData;

  if (outDataParams->elementType == TIDL_UnsignedChar)
  {
    status = TIDL_computeInputSize(net,
                                   tidlLayer,
                                   inDataParams,
                                   outDataParams,
                                   (uint8_t*) outPtrs[0]);
  }
  else if (outDataParams->elementType == TIDL_SignedChar)
  {
    status = TIDL_computeInputSize(net,
                                   tidlLayer,
                                   inDataParams,
                                   outDataParams,
                                   (int8_t*) outPtrs[0]);
  }
  else if (outDataParams->elementType == TIDL_UnsignedShort)
  {
    status = TIDL_computeInputSize(net,
                                   tidlLayer,
                                   inDataParams,
                                   outDataParams,
                                   (uint16_t*) outPtrs[0]);
  }
  else if (outDataParams->elementType == TIDL_SignedShort)
  {
    status = TIDL_computeInputSize(net,
                                   tidlLayer,
                                   inDataParams,
                                   outDataParams,
                                   (int16_t*) outPtrs[0]);
  }
  else if (outDataParams->elementType == TIDL_UnsignedWord)
  {
    status = TIDL_computeInputSize(net,
                                   tidlLayer,
                                   inDataParams,
                                   outDataParams,
                                   (uint32_t*) outPtrs[0]);
  }
  else if (outDataParams->elementType == TIDL_SignedWord)
  {
    status = TIDL_computeInputSize(net,
                                   tidlLayer,
                                   inDataParams,
                                   outDataParams,
                                   (int32_t*) outPtrs[0]);
  }
  else if (outDataParams->elementType == TIDL_UnsignedDoubleWord)
  {
    status = TIDL_computeInputSize(net,
                                   tidlLayer,
                                   inDataParams,
                                   outDataParams,
                                   (uint64_t*) outPtrs[0]);
  }
  else if (outDataParams->elementType == TIDL_SignedDoubleWord)
  {
    status = TIDL_computeInputSize(net,
                                   tidlLayer,
                                   inDataParams,
                                   outDataParams,
                                   (int64_t*) outPtrs[0]);
  }
  else if (outDataParams->elementType == TIDL_SinglePrecFloat)
  {
    status = TIDL_computeInputSize(net,
                                   tidlLayer,
                                   inDataParams,
                                   outDataParams,
                                   (float*) outPtrs[0]);
  }
  else
  {
    status = TIDL_ERR_FAILURE;
  }

  return status;
}
