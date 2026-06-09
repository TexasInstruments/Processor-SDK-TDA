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
@file    tidl_squeeze_ref.c
@brief   This file defines private functions for squeeze layer.
@version 0.1 (November 2024) : Initial version
----------------------------------------------------------------------------
*/
#if defined TIDL_COVERAGE_DEAD_CODE

#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include "tidl_squeeze_ref.h"

/**
 * @brief This is the reference implementation of the Squeeze layer
 *
 * @param commonParams : Network commonParameters
 * @param algLayer : Pointer to the layer specific parameters
 * @param tidlLayer : Pointer to the common layer parameters
 * @param inPtrs : Pointer to input buffers to be processed
 * @param outPtrs : Pointer to output buffers to be processed
 * @param layerIdx : Index of the layer
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */

int32_t TIDL_squeezeRefProcess(TIDL_NetworkCommonParams *commonParams,
                               sTIDL_AlgLayer_t *algLayer,
                               sTIDL_Layer_t *tidlLayer,
                               void *inPtrs[],
                               void *outPtrs[],
                               int32_t layerIdx)
{
  int32_t status = TIDL_SUCCESS;
  int32_t batch, ch, i, dim1, dim2;
  sTIDL_DataParams_t *inDataParams;
  inDataParams = &commonParams->net->TIDLLayers[algLayer->inLayerIdx[0]].outData;
  int32_t elm_size = TIDL_getDatElementSize(inDataParams->elementType);

  if (inPtrs[0] != outPtrs[0])
  {
    for (batch = 0; batch < inDataParams->dimValues[TIDL_DIM_BATCH]; batch++)
    {
      for (dim1 = 0; dim1 < inDataParams->dimValues[TIDL_DIM_DIM1]; dim1++)
      {
        for (dim2 = 0; dim2 < inDataParams->dimValues[TIDL_DIM_DIM2]; dim2++)
        {
          for (ch = 0; ch < inDataParams->dimValues[TIDL_DIM_NUMCH]; ch++)
          {
            for (i = 0; i < inDataParams->dimValues[TIDL_DIM_HEIGHT]; i++)
            {
              (void)memcpy((void *)((size_t)outPtrs[0] + (((batch * inDataParams->pitch[TIDL_ROI_PITCH]) + (dim1 * inDataParams->pitch[TIDL_DIM1_PITCH]) + (dim2 * inDataParams->pitch[TIDL_DIM2_PITCH]) + (ch * inDataParams->pitch[TIDL_CHANNEL_PITCH]) + (i * inDataParams->pitch[TIDL_LINE_PITCH])) * elm_size)),
                           (void *)((size_t)inPtrs[0] + (((batch * inDataParams->pitch[TIDL_ROI_PITCH]) + (dim1 * inDataParams->pitch[TIDL_DIM1_PITCH]) + (dim2 * inDataParams->pitch[TIDL_DIM2_PITCH]) + (ch * inDataParams->pitch[TIDL_CHANNEL_PITCH]) + (i * inDataParams->pitch[TIDL_LINE_PITCH])) * elm_size)),
                           inDataParams->pitch[TIDL_LINE_PITCH] * elm_size);
            }
          }
        }
      }
    }
  }

  if ((commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) == TIDL_FLOW_CTRL_REF_STAT)
  {
    TIDL_Obj intAlgObj;
    TIDL_CreateParams createParams;

    (void)memcpy(&createParams, commonParams->createParams, sizeof(TIDL_CreateParams));
    intAlgObj.createParams = (TIDL_CreateParams *)&createParams;
    TIDL_UpdateScaleFactors(&intAlgObj, layerIdx, 0, 0, 1);
  }
  TIDL_L1DandL2CacheWbInv();

  return status;
}
#endif
