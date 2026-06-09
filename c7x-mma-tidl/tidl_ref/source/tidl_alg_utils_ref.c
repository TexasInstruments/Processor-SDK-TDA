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
@file    tidl_alg_utils_ref.c
@brief   This file defines reference code  specific utility functions
@version 0.1 (Oct 2025) : Initial version, Moved functions from other files [PKS]
----------------------------------------------------------------------------
*/

#include <stdint.h>
#include "tidl_commonUtils.h"
#include "tidl_alg_utils_ref.h"


/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION_SUPPORT
@fn         TIDL_sortLayersByOutVol
@brief      Function to sort output layers by output volume in descending order

@param      params       : Create Parameters
@param      layerIdx     : Current Layer index in Net structure
@param      memRec       : memory table populated by the Algorithm
@remarks    None
@return     IALG_EOK   - Successful
            IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/

static void TIDL_sortLayersByOutVol(const TIDL_CreateParams *params,
int32_t *sortedLayers, int32_t *sortedLayerIdx)
{
  int32_t ii, jj;
  while((*sortedLayerIdx) < params->net->numLayers)
  {
    int64_t maxVol = -1;
    for(ii = 0; ii < params->net->numLayers ; ii++)
    {
      int8_t layerAdded = 0;
      for (jj = 0; jj < (*sortedLayerIdx); jj++)
      {
        if(sortedLayers[jj] == ii)
        {
          layerAdded = 1;
          break;
        }
      }
      if(layerAdded == 1)
      {
        continue;
      }

      const sTIDL_Layer_t *layer = &params->net->TIDLLayers[ii];
      int64_t currVol = (int64_t)(layer->outData.pitch[TIDL_ROI_PITCH] * layer->outData.dimValues[TIDL_DIM_BATCH]);
      if(layer->numOutBufs < 0)
      {
        /*Any layer with no output (Currently only output Data layer) has no volume as it has no output buffer */
        currVol = 0;
      }
      if(currVol > maxVol)
      {
        maxVol = currVol;
      }
    }
    if(maxVol == -1)
    {
      break;
    }
    for(ii = 0; ii < params->net->numLayers ; ii++)
    {
      const sTIDL_Layer_t *layer = &params->net->TIDLLayers[ii];
      int64_t currVol = (int64_t)(layer->outData.pitch[TIDL_ROI_PITCH] * layer->outData.dimValues[TIDL_DIM_BATCH]);
      if(layer->numOutBufs < 0)
      {
        /*Any layer with no output (Currently only output Data layer) has no volume as it has no output buffer */
        currVol = 0;
      }
      if(currVol == maxVol)
      {
        sortedLayers[(*sortedLayerIdx)] = ii;
        *sortedLayerIdx = (*sortedLayerIdx) + 1;
      }
    }
  }
}

/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION_SUPPORT
@fn         TIDL_findFreeOutBuff
@brief      Function to find the free output buffer

@param      params       : Create Parameters
@param      layerIdx     : Current Layer index in Net structure
@param      outDataSize  : outDataSize required for this layer
@param      TIDLLayersBuf: Temp structure to store outDataSize and Data Id's
@remarks    None
@return     outDataSize  : outDataSize required for this layer
----------------------------------------------------------------------------
*/
int32_t TIDL_findFreeOutBuff(const TIDL_CreateParams *params,
int32_t layerIdx, int32_t outDataSize, sTIDL_LayerBuf_t *TIDLLayersBufPtr)
{
  /*
   * This function is called to find output buffer for a layer and return its
   * size. If this layer uses free output buffer from previous layer, the
   * size returned is 0
   */

  /*
   * Whenever this function is called for the first time, no buffer is
   * allocated for any of the layer. Hence we need to decide the allocation for
   * all the layers.
   *
   * For minimum allocation, for a layer always try to use buffer from previous
   * layers if possible i.e if previous buffer size is enough and that buffer
   * is not used by any future layers.
   *
   * For optimal overlap, we will always prioritize overlapping buffers with
   * bigger size first
   */

  int32_t ii, jj, kk;
  int8_t isFirstTime = 1;

  /*
   * Check if this function is called for the first time, i.e no output
   * allocation has been decided yet. If yes, then decide optimal
   * output allocation for all layers
   */
  for(ii = 0; ii < params->net->numLayers; ii++)
  {
    if(TIDLLayersBufPtr->outDataSize[ii] != 0)
    {
      isFirstTime = 0;
      break;
    }
  }

  if (isFirstTime == 1)
  {
    // Get layers in sorted order of output volume with largets volume first
    int32_t sortedLayers[TIDL_NUM_MAX_LAYERS];
    int32_t sortedLayerIdx = 0;
    for(ii = 0; ii < TIDL_NUM_MAX_LAYERS; ii++)
    {
      sortedLayers[ii] = -1;
    }
    TIDL_sortLayersByOutVol(params, sortedLayers, &sortedLayerIdx);
    if(sortedLayerIdx != params->net->numLayers)
    {
      tidl_printf(0, "Could not sort layers by output volume size.\n");
    //  return -1; //commented to avoid MISRA-C voilations, needs to add proper error code
    }
    /*
     * TODO: Might need to remove layers from sorted layer is not allocation is
     * defined for the layer. See gTIDLCoreFunctionMapping and wherever
     * TIDL_findFreeOutBuff is called for reference
     */

    // Loop over sorted layers and decide allocation or overlap
    for(int32_t i = 0; i < sortedLayerIdx; i++)
    {
      int32_t index = sortedLayers[i];
      const sTIDL_Layer_t *TIDLLayer = &params->net->TIDLLayers[index];
      int32_t outDataId = TIDLLayer->outData.dataId;
      int32_t curOutDataId = outDataId;
      int32_t skipLoop;

      // Skip if output buffer
      if((TIDL_isOutDataBuff(params->net, outDataId, params->currLayersGroupId) == 0))
      {
        int32_t numBatch = TIDLLayer->outData.dimValues[TIDL_DIM_BATCH];
        int32_t roiPitch = TIDLLayer->outData.pitch[TIDL_ROI_PITCH];
        int32_t elementSize = TIDL_getDatElementSize(TIDLLayer->outData.elementType);
        int64_t layerVol = (int64_t)(numBatch * roiPitch * elementSize);
        int32_t layerPadW = TIDLLayer->outData.padW;
        int32_t layerPadH = TIDLLayer->outData.padH;

        // Check if any output buffer from previous layer can be used
        for(ii = (index - 1); ii > 0; ii--)
        {
          const sTIDL_Layer_t *TIDLLayerCur = &params->net->TIDLLayers[ii];
          /*
           * If the layer under consideration is input to the layer we are
           * deciding allocation for, we cannot use its buffer.
           */
          int8_t isInput = 0;
          for(int32_t idx = 0; idx < TIDLLayer->numInBufs; idx++)
          {
            if(TIDLLayer->inData[idx] == TIDLLayerCur->outData.dataId)
            {
              isInput = 1;
              break;
            }
          }
          if(isInput == 1)
          {
            continue;
          }

          /* Check whether output memory is allocated to this layer
           * TIDLLayersBufPtr->outDataSize will be 0 for layers which re-uses
           * output buffer from previous layer.
           */
          if(TIDLLayersBufPtr->outDataSize[ii] != 0)
          {
            /*
             * For layer to re-use the out buffer from previous layer, following
             * conditions should met:
             * - Previous layer cannot be Data or ConstData layer
             * - Total buffer size of layer should be less than or equal to
             *   buffer size of previous layer
             * - Pad should be equal
             */
            if ((TIDLLayerCur->layerType != TIDL_DataLayer) && (TIDLLayerCur->layerType != TIDL_ConstDataLayer))
            {
              int32_t curNumBatch = TIDLLayerCur->outData.dimValues[TIDL_DIM_BATCH];
              int32_t curRoiPitch = TIDLLayerCur->outData.pitch[TIDL_ROI_PITCH];
              int32_t curElementSize = TIDL_getDatElementSize(TIDLLayerCur->outData.elementType);
              int64_t curLayerVol = (int64_t)(curNumBatch * curRoiPitch * curElementSize);
              int32_t curLayerPadW = TIDLLayerCur->outData.padW;
              int32_t curLayerPadH = TIDLLayerCur->outData.padH;
              int8_t  allow = 0;
              #ifdef TIDL_COVERAGE_DEAD_CODE_NO_TEST /* else is in dead code so 'if' check is not needed*/
              if(params->optimiseExtMem != TIDL_OptimiseExtMemL2)
              #endif
              {
                if((curLayerVol >= layerVol) &&
                   (layerPadW == curLayerPadW) &&
                   (layerPadH == curLayerPadH))
                  {
                    allow = 1;
                  }
              }
              #ifdef TIDL_COVERAGE_DEAD_CODE_NO_TEST
              else
              {
                if(layerVol <= TIDLLayersBufPtr->outDataSize[ii])
                {
                  allow = 1;
                }
              }
              #endif

              if(allow == 1)
              {
                // Found an output buffer of required size in previous layers
                curOutDataId = params->net->TIDLLayers[ii].outData.dataId;

                /*
                  * Check whether the layer we found layer is used as input
                  * to any future layers, in that case we cannot overwrite its
                  * data
                  */
                skipLoop = 0;
                for(kk = index; kk < params->net->numLayers; kk++)
                {
                  const sTIDL_Layer_t *TIDLLayerCurFuture = &params->net->TIDLLayers[kk];
                  for(jj = 0; jj < TIDLLayerCurFuture->numInBufs; jj++)
                  {
                    int32_t inDataId  =  (int32_t)TIDLLayersBufPtr->newInDataId[kk][jj];
                    /*
                      * The output buffer is still in use for future layer,
                      * hence we cannot use this buffer
                      */
                    if((inDataId == curOutDataId) && (skipLoop == 0))
                    {
                      curOutDataId = outDataId;
                      skipLoop = 1;
                    }
                  }
                }
                // Free output buffer found, so break from for loop
                if(curOutDataId != outDataId)
                {
                  break;
                }
              }
            }
          }
        }

        /*
         * Special condition: We cannot reuse the output buffer if it is used
         * as input to Slice Layer. But why?
         */
        skipLoop = 0;
        if(curOutDataId != outDataId)
        {
          for(kk = 0; kk < index; kk++)
          {
            const sTIDL_Layer_t *TIDLLayerCur = &params->net->TIDLLayers[kk];
            for(jj = 0; jj < TIDLLayerCur->numInBufs; jj++)
            {
              int32_t inDataId  =  (int32_t)TIDLLayersBufPtr->newInDataId[kk][jj];
              if((inDataId == curOutDataId) && (skipLoop == 0))
              {
                if(TIDLLayerCur->layerType == TIDL_SliceLayer)
                {
                  curOutDataId = outDataId;
                  skipLoop = 1;
                }
              }
            }
          }
        }

        /*
         * If free output buffer found in previous layer, we can re-use it.
         * Update the input data-id to point to the new found output buffer
         * Also make outDataSize = 0, to not allocate the new output memory for
         * this layer.
         */
        if(curOutDataId != outDataId)
        {
          for(kk = index; kk < params->net->numLayers; kk++)
          {
            const sTIDL_Layer_t *TIDLLayerCur = &params->net->TIDLLayers[kk];
            for(jj = 0; jj < TIDLLayerCur->numInBufs; jj++)
            {
              int32_t inDataId  =  (int32_t)TIDLLayersBufPtr->newInDataId[kk][jj];
              if(inDataId == outDataId)
              {
                TIDLLayersBufPtr->newInDataId[kk][jj] = curOutDataId;
              }
            }
          }
          TIDLLayersBufPtr->newOutDataId[index][0] = curOutDataId;
          TIDLLayersBufPtr->outDataSize[index] = 0;
        }
        else
        {
          TIDLLayersBufPtr->outDataSize[index] = layerVol;
        }
      }
    }
  }

  // Return output data size for layerId
  const sTIDL_Layer_t *TIDLLayerNew = &params->net->TIDLLayers[layerIdx];// renamed variable to avoid conflict and MISRA violation
  int32_t outDataIdNew = TIDLLayerNew->outData.dataId;// renamed variable to avoid conflict and MISRA violation
  int32_t currDataSize = outDataSize;
  if((TIDL_isOutDataBuff(params->net, outDataIdNew, params->currLayersGroupId) == 0))
  {
    /*
     * TIDLLayersBufPtr->newOutDataId[layerIdx] indicates if it uses buffer
     * from previous layer. If set to 0, it does not use buffer from previous
     * layer. If not 0, it uses buffer from previous layer and the value
     * indicates which previous layer it uses the buffer from.
     */
    if(TIDLLayersBufPtr->newOutDataId[layerIdx][0] != 0)
    {
      currDataSize = 0;
    }
  }
  else
  {
    currDataSize = 0;
  }
  return currDataSize;
}
