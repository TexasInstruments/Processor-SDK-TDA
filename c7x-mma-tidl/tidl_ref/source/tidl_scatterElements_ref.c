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
 *  \file tidl_scatterElements_ref.c
 *
 *  \brief This file defines kernel functions for Batch Norm layer
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include <math.h>
#include <limits>
#include "tidl_scatterElements_ref.h"

// #define TIDL_REF_BATCH_NORM_DEBUG
/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define TIDL_SCATTERND_DST_TO_SRC_TABLE_SIZE_DDR (200U * 1024U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                  Internal/Private Function Declarations                    */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */
template<class Tin, class Tacc> void TIDL_refScatterElementsOutputUpdation(Tacc *accPtr, Tin *update, int32_t targetIndex, int32_t updateIndex, int32_t reduction, Tacc minValue, Tacc maxValue)
{
  if (reduction == (int32_t)TIDL_ScatterElementsAdd)
  {
    if(std::is_same<Tacc, float32_tidl>::value)
    {
      float32_tidl sum = accPtr[targetIndex] + (Tacc)update[updateIndex];
      sum = (sum >= maxValue) ? (Tacc)maxValue : sum;
      sum = (sum <= minValue) ? (Tacc)minValue : sum; 
      accPtr[targetIndex] = sum;
    }
    else
    {
      int64_t sum = accPtr[targetIndex] + (Tacc)update[updateIndex];
      sum = (sum >= maxValue) ? (Tacc)maxValue : sum;
      sum = (sum <= minValue) ? (Tacc)minValue : sum; 
      accPtr[targetIndex] = sum;
    }
  }
  else if (reduction == (int32_t)TIDL_ScatterElementsMax)
  {
    accPtr[targetIndex] = (accPtr[targetIndex] > (Tacc)update[updateIndex]) ? accPtr[targetIndex] : (Tacc)update[updateIndex];
  }
  else if (reduction == (int32_t)TIDL_ScatterElementsMin)
  {
    accPtr[targetIndex] = (accPtr[targetIndex] < (Tacc)update[updateIndex]) ? accPtr[targetIndex] : (Tacc)update[updateIndex];
  }
  else
  {
    accPtr[targetIndex] = (Tacc)update[updateIndex];
  }
}

/**
 * @brief This function populates the indices tuple for scatterND operation
 *
 * @param indices : pointer to the indices buffer
 * @param indicesTuple : pointer to the indices tuple
 * @param inIndicesParams : parameters of the input indices buffer
 * @param i0, i1, i2, i3, i4 : index values for each dimension
 *
 * @return 0 on success, -1 on failure
 */
template<class Tind>
    int32_t tidl_scatterElementsPopulateIndicesTuple(
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

template<class Tacc, class Tin>
    int32_t tidl_scatterNDcopyInputToAccumulator(
        Tacc *accPtr,
        Tin *data,
        const sTIDL_DataParams_t *inDataParams)
{
  int32_t i1, i2, i3, i4, i5, i0;
  /**
   * Element type of input data & accumulator can be different
   * Hence doing an element wise copy
   */
  for (i0 = 0; i0 < inDataParams->dimValues[TIDL_DIM_BATCH]; i0++)
  {
    for (i1 = 0; i1 < inDataParams->dimValues[TIDL_DIM_DIM1]; i1++)
    {
      for (i2 = 0; i2 < inDataParams->dimValues[TIDL_DIM_DIM2]; i2++)
      {
        for (i3 = 0; i3 < inDataParams->dimValues[TIDL_DIM_NUMCH]; i3++)
        {
          for (i4 = 0; i4 < inDataParams->dimValues[TIDL_DIM_HEIGHT]; i4++)
          {
            for (i5 = 0; i5 < inDataParams->dimValues[TIDL_DIM_WIDTH]; i5++)
            {
              int32_t index = (i0 * inDataParams->pitch[TIDL_ROI_PITCH]) +
                              (i1 * inDataParams->pitch[TIDL_DIM1_PITCH]) +
                              (i2 * inDataParams->pitch[TIDL_DIM2_PITCH]) +
                              (i3 * inDataParams->pitch[TIDL_CHANNEL_PITCH]) +
                              (i4 * inDataParams->pitch[TIDL_LINE_PITCH]) +
                              (i5);
              accPtr[index] = (Tacc)(data[index]);
            }
          }
        }
      }
    }
  }

  return 0;
}

/**
 * @brief This function returns the target index for scatterElements operation
 *
 * @param inDataParams : parameters of the input data buffer
 * @param indicesTuple : pointer to the indices tuple
 *
 * @return targetIndex : index of the target element
 */
int32_t tidl_scatterElementsgetTargetIndex(
    const sTIDL_DataParams_t *inDataParams,
    int32_t *indicesTuple)
{
  int32_t targetIndex = 0;
  targetIndex = indicesTuple[TIDL_DIM_BATCH] * inDataParams->pitch[TIDL_ROI_PITCH] +
                indicesTuple[TIDL_DIM_DIM1] * inDataParams->pitch[TIDL_DIM1_PITCH] +
                indicesTuple[TIDL_DIM_DIM2] * inDataParams->pitch[TIDL_DIM2_PITCH] +
                indicesTuple[TIDL_DIM_NUMCH] * inDataParams->pitch[TIDL_CHANNEL_PITCH] +
                indicesTuple[TIDL_DIM_HEIGHT] * inDataParams->pitch[TIDL_LINE_PITCH] +
                indicesTuple[TIDL_DIM_WIDTH];
  return targetIndex;
}

int32_t tidl_scatterElementsgetUpdateIndex(
    const sTIDL_DataParams_t *inUpdateParams,
    int32_t i0, int32_t i1, int32_t i2, int32_t i3, int32_t i4,
    int32_t updatePitch)
{
  int32_t updateIndex = 0;
  updateIndex = i0 * inUpdateParams->pitch[TIDL_DIM1_PITCH] +
                i1 * inUpdateParams->pitch[TIDL_DIM2_PITCH] +
                i2 * inUpdateParams->pitch[TIDL_CHANNEL_PITCH] +
                i3 * inUpdateParams->pitch[TIDL_LINE_PITCH] +
                i4 * updatePitch;
  return updateIndex;
}

/**
 * @brief This is main function perform scatterElements on differnt elementTypes
 *
 * @tparam Tin  : template for input
 * @tparam Tw   : template for weights
 * @tparam Tb   : template for Bias
 * @tparam Tout : template for output
 * @tparam Tsat : template for saturate values
 * @param inPtr : Input pointer on which bacthNorm is applied
 * @param outPtr : Output pointer after scatterElements opreation
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

template <class Tin, class Tindx, class Tout, class Tacc>
static int32_t TIDL_refScatterElements(Tin  *data,
                                     Tindx* indices,
                                     Tin  *update,
                                     Tout *outPtr,
                                     Tacc *accPtr,
                                     TIDL_Handle intAlgHandle,
                                     int32_t layerIdx,
                                     sTIDL_ScatterElementsParams_t *params,
                                     sTIDL_AlgLayer_t *algLayer,
                                     const sTIDL_Layer_t  *tidlLayer,
                                     const sTIDL_DataParams_t *inDataParams,
                                     const sTIDL_DataParams_t *inIndicesParams, 
                                     const sTIDL_DataParams_t *inUpdateParams,
                                     const sTIDL_DataParams_t *outDataParams)
{
  int32_t status = TIDL_SUCCESS;
  int32_t i0, i1, i2, i3, i4, i5, i6, i7;

  int32_t inIndicesNumCols    = (int32_t)inIndicesParams->dimValues[TIDL_DIM_WIDTH];
  int32_t inIndicesNumRows    = (int32_t)inIndicesParams->dimValues[TIDL_DIM_HEIGHT];
  int32_t inIndicesNumChs     = (int32_t)inIndicesParams->dimValues[TIDL_DIM_NUMCH];
  int32_t inIndicesDim2       = (int32_t)inIndicesParams->dimValues[TIDL_DIM_DIM2];
  int32_t inIndicesDim1       = (int32_t)inIndicesParams->dimValues[TIDL_DIM_DIM1];
  int32_t inIndicesBatches    = (int32_t)inIndicesParams->dimValues[TIDL_DIM_BATCH];

  int32_t outNumCols          = (int32_t)outDataParams->dimValues[TIDL_DIM_WIDTH];
  int32_t outNumRows          = (int32_t)outDataParams->dimValues[TIDL_DIM_HEIGHT];
  int32_t outChs              = (int32_t)outDataParams->dimValues[TIDL_DIM_NUMCH];
  int32_t outBatches          = (int32_t)outDataParams->dimValues[TIDL_DIM_BATCH];
  int32_t outDataDim2         = (int32_t)outDataParams->dimValues[TIDL_DIM_DIM2];
  int32_t outDataDim1         = (int32_t)outDataParams->dimValues[TIDL_DIM_DIM1];

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
  
  int32_t inUpdatePitch       = (int32_t)inUpdateParams->pitch[TIDL_LINE_PITCH];
  int32_t inUpdateChPitch     = (int32_t)inUpdateParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t inUpdateRoiPitch    = (int32_t)inUpdateParams->pitch[TIDL_ROI_PITCH];
  int32_t inUpdateDim2Pitch   = (int32_t)inUpdateParams->pitch[TIDL_DIM_DIM2];
  int32_t inUpdateDim1Pitch   = (int32_t)inUpdateParams->pitch[TIDL_DIM_DIM1];

  int32_t outPitch            = (int32_t)outDataParams->pitch[TIDL_LINE_PITCH];
  int32_t outChPitch          = (int32_t)outDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t outRoiPitch         = (int32_t)outDataParams->pitch[TIDL_ROI_PITCH];
  int32_t outDim2Pitch        = (int32_t)outDataParams->pitch[TIDL_DIM_DIM2];
  int32_t outDim1Pitch        = (int32_t)outDataParams->pitch[TIDL_DIM_DIM1];

  data = (Tin *)data + (inDataParams->padH * inDataPitch) + inDataParams->padW;
  indices = (Tindx *)indices + (inIndicesParams->padH * inIndicesPitch) + inIndicesParams->padW;
  update = (Tin *)update + (inUpdateParams->padH * inUpdatePitch) + inUpdateParams->padW;

  Tout *output = (Tout *)outPtr + (outDataParams->padH * outPitch) + outDataParams->padW;

  int32_t numTotRoi        = (int32_t)inDataParams->dimValues[TIDL_DIM_BATCH];
  int32_t inDataDim2       = (int32_t)inDataParams->dimValues[TIDL_DIM_DIM2];
  int32_t inDataDim1       = (int32_t)inDataParams->dimValues[TIDL_DIM_DIM1];

  int32_t numInChannels = inDataParams->dimValues[TIDL_DIM_NUMCH];
  int32_t numOutChannels = outDataParams->dimValues[TIDL_DIM_NUMCH];
  int32_t axis = params->axis;

  int32_t index, badIndex = 0;
  Tin updateVal;

  int32_t isScatterND = (params->axis == -1) ? 1 : 0;
  int32_t indicesTuple[TIDL_DIM_MAX] = {0}, updatePitch = 0;
  int32_t n = 0, c = 0, h = 0, w, targetIndex, updateIndex;

  tidl_scatterNDcopyInputToAccumulator(accPtr, data, inDataParams);

  if (isScatterND != 0)
  {
    int32_t dataDim = outDataParams->numDim;
    int32_t indDim = inIndicesParams->dimValues[TIDL_DIM_WIDTH];

    int32_t outSize = outChs * outChPitch;
    Tacc minValueAcc = 0;
    minValueAcc = std::numeric_limits<Tacc>::lowest();
    Tacc maxValueAcc = 0;
    maxValueAcc = std::numeric_limits<Tacc>::max();

    if (dataDim == indDim)
    {
      // openacc(data copyin(indices[:(inIndicesNumRows-1)*inIndicesNumCols + 4]))
      // openacc(parallel loop)
      // element update
      updatePitch = 1;
      for (i0 = 0; i0 < inIndicesBatches; i0++)
      {
        for (i1 = 0; i1 < inIndicesDim1; i1++)
        {
          for (i2 = 0; i2 < inIndicesDim2; i2++)
          {
            for (i3 = 0; i3 < inIndicesNumChs; i3++)
            {
              for (i4 = 0; i4 < inIndicesNumRows; i4++)
              {

                tidl_scatterElementsPopulateIndicesTuple(
                    indices,
                    indicesTuple,
                    inIndicesParams,
                    i0, i1, i2, i3, i4);

                for (int32_t i5 = TIDL_DIM_WIDTH, i6 = indDim - 1; i5 > -1 && i6 > -1; i5--, i6--)
                {
                  badIndex = false;
                  if (indicesTuple[i5] >= inDataParams->dimValues[i5])
                  {
                    badIndex = true;
                    break;
                  }
                }
                if (badIndex == true)
                {
                  // #ifndef _//openacc
                  tidl_printf(0, "Index out of bounds! skipping the update");
                  continue;
                  // #endif
                }
                targetIndex = tidl_scatterElementsgetTargetIndex(inDataParams, indicesTuple);
                updateIndex = tidl_scatterElementsgetUpdateIndex(inUpdateParams, i0, i1, i2, i3, i4, updatePitch);
                TIDL_refScatterElementsOutputUpdation(accPtr, update, targetIndex, updateIndex, params->reduction, minValueAcc, maxValueAcc);
              }
            }
          }
        }
      }
    }
    else if ((dataDim - indDim) == 1)
    {
      //openacc(data copyin(indices[:(inIndicesNumRows-1)*inIndicesNumCols + 3]))
      //openacc(parallel loop)
      for (i0 = 0; i0 < inIndicesBatches; i0++)
      {
        for (i1 = 0; i1 < inIndicesDim1; i1++)
        {
          for (i2 = 0; i2 < inIndicesDim2; i2++)
          {
            for (i3 = 0; i3 < inIndicesNumChs; i3++)
            {
              for (i4 = 0; i4 < inIndicesNumRows; i4++)
              {
                index = i0 * inIndicesParams->pitch[TIDL_ROI_PITCH] +
                        i1 * inIndicesParams->pitch[TIDL_DIM1_PITCH] +
                        i2 * inIndicesParams->pitch[TIDL_DIM2_PITCH] +
                        i3 * inIndicesParams->pitch[TIDL_CHANNEL_PITCH];
                if ((dataDim == 4) && (indDim == 3))
                {
                  n = (int32_t)indices[index + (i4*inIndicesParams->pitch[TIDL_LINE_PITCH]) + 0];
                  c = (int32_t)indices[index + (i4*inIndicesParams->pitch[TIDL_LINE_PITCH]) + 1];
                  h = (int32_t)indices[(index + i4*inIndicesParams->pitch[TIDL_LINE_PITCH]) + 2];
                }
                else if ((dataDim == 3) && (indDim == 2))
                {
                  n = 0;
                  c = (int32_t)indices[index + (i4*inIndicesParams->pitch[TIDL_LINE_PITCH]) + 0];
                  h = (int32_t)indices[index + (i4*inIndicesParams->pitch[TIDL_LINE_PITCH]) + 1];
                }
                else if ((dataDim == 2) && (indDim == 1))
                {
                  n = 0;
                  c = 0;
                  h = (int32_t)indices[index + (i4*inIndicesParams->pitch[TIDL_LINE_PITCH]) + 0];
                }
                else
                {
                  /*do nothing*/
                }

                if((n >= outDataDim2 || n < ((-1)*(outDataDim2))) || (c >= outChs || c < ((-1)*(outChs))) || (h >= outNumRows || h < ((-1)*(outNumRows))))
                {
                  continue;
                }
                /* Wrap around if negative */
                n = (n < 0) ? (n + outDataDim2) : n;
                c = (c < 0) ? (c + outChs) : c;
                h = (h < 0) ? (h + outNumRows) : h;

                targetIndex = (n * inDataDim2Pitch) + (c * inDataChPitch) + (h * inDataPitch);
                updateIndex = i0 * inUpdateParams->pitch[TIDL_ROI_PITCH] +
                              i1 * inUpdateParams->pitch[TIDL_DIM1_PITCH] +
                              i2 * inUpdateParams->pitch[TIDL_DIM2_PITCH] +
                              i3 * inUpdateParams->pitch[TIDL_CHANNEL_PITCH] +
                              i4 * inUpdateParams->pitch[TIDL_LINE_PITCH];
                //openacc(loop)
                for (i5=0;i5<outDataParams->dimValues[TIDL_DIM_WIDTH];i5++)
                {
                  TIDL_refScatterElementsOutputUpdation(accPtr, update, targetIndex, updateIndex, params->reduction, minValueAcc, maxValueAcc);
                  targetIndex++;
                  updateIndex++;
                }
              }
            }
          }
        }
      }
    }
    else if ((dataDim - indDim) == 2)
    {
      //openacc(data copyin(indices[:(inIndicesNumRows-1)*inIndicesNumCols+2]))
      //openacc(parallel loop)
      //channel update
      int32_t totalIndices = inIndicesBatches * inIndicesDim1 * inIndicesDim2 * inIndicesNumChs * inIndicesNumRows;
      for (i0=0; i0 < totalIndices; i0++)
      {
        n = (int32_t)indices[i0 * inIndicesNumCols + 0];
        c = (int32_t)indices[i0 *inIndicesNumCols + 1];

        if((n >= outDataDim2 || n < ((-1)*(outDataDim2))) || (c >= outChs || c < ((-1)*(outChs))))
        {
          // #ifndef _//openacc
          // tidl_printf(0, "Index out of bounds! skipping the update");
          // #endif
          continue;
        }
        /* Wrap around if negative */
        n = (n < 0) ? (n + outDataDim2) : n;
        c = (c < 0) ? (c + outChs) : c;

        targetIndex = (n * inDataDim2Pitch) + (c * inDataChPitch);
        updateIndex = i0*inUpdateChPitch;
        //openacc(loop collapse(2))
        for (i1=0; i1<outDataParams->dimValues[TIDL_DIM_HEIGHT]; i1++)
        {
          for (i2=0; i2<outDataParams->dimValues[TIDL_DIM_WIDTH]; i2++)
          {
            TIDL_refScatterElementsOutputUpdation(accPtr, update, targetIndex, updateIndex, params->reduction, minValueAcc, maxValueAcc);
            targetIndex++;
            updateIndex++;
          }
        }
      }
    }
    else
    {
      for (i0 = 0; i0 < inIndicesBatches; i0++)
      {
        for (i1 = 0; i1 < inIndicesDim1; i1++)
        {
          for (i2 = 0; i2 < inIndicesDim2; i2++)
          {
            for (i3 = 0; i3 < inIndicesNumChs; i3++)
            {
              for (i4 = 0; i4 < inIndicesNumRows; i4++)
              {
                index = i0 * inIndicesParams->pitch[TIDL_ROI_PITCH] +
                        i1 * inIndicesParams->pitch[TIDL_DIM1_PITCH] +
                        i2 * inIndicesParams->pitch[TIDL_DIM2_PITCH] +
                        i3 * inIndicesParams->pitch[TIDL_CHANNEL_PITCH];
                n = (int32_t)indices[index + i4];
                if((n >= outDataDim2 || n < ((-1)*(outDataDim2))))
                {
                  continue;
                }
                /* Wrap around if negative */
                n = (n < 0) ? (n + outDataDim2) : n;
                
                //openacc(loop collapse(2))
                for (i5=0; i5<outDataParams->dimValues[TIDL_DIM_NUMCH]; i5++)
                {
                  updateIndex = i2 * inUpdateParams->pitch[TIDL_ROI_PITCH] +
                              i3 * inUpdateParams->pitch[TIDL_DIM1_PITCH] +
                              i4 * inUpdateParams->pitch[TIDL_DIM2_PITCH] + 
                              i5 * inUpdateParams->pitch[TIDL_CHANNEL_PITCH];

                  targetIndex = (n * inDataDim2Pitch) + i5*inDataChPitch;
                  for (i6=0; i6<outDataParams->dimValues[TIDL_DIM_HEIGHT]; i6++)
                  {
                    for (i7=0; i7<outDataParams->dimValues[TIDL_DIM_WIDTH]; i7++)
                    {
                      TIDL_refScatterElementsOutputUpdation(accPtr, update, targetIndex, updateIndex, params->reduction, minValueAcc, maxValueAcc);
                      targetIndex++;
                      updateIndex++;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }

    // openacc(data copyin(accPtr[:outSize]) copyout(output[:outSize]))
    {
      if (params->reduction == (int32_t)TIDL_ScatterElementsAdd)
      {
        Tout temp = 0;
        Tacc outAcc;
        temp = std::numeric_limits<Tout>::lowest();
        int32_t minValueOutput = (int32_t)temp;
        temp = std::numeric_limits<Tout>::max();
        int32_t maxValueOutput = (int32_t)temp;

        uint8_t cScale, shift;
        float32_tidl fVal = (outDataParams->tensorScale / inUpdateParams->tensorScale);
        TIDL_getMMAv2_ScaleAndShift(fVal, &cScale, &shift);

      /* Prepare output from accPtr */
      //openacc(parallel loop)
      for(int32_t k = 0; k < outRoiPitch * outBatches; k++)
      {
        outAcc = accPtr[k];
        if (outDataParams->elementType == TIDL_SinglePrecFloat)
        {
          //openacc(routine(TIDL_floatSat))
          outAcc = TIDL_floatSat(outAcc, (sTIDL_Layer_t  *)tidlLayer);
        }
        else
        {
          int64_t tempAcc;
          tempAcc = outAcc * cScale;
          outAcc = (Tacc)TIDL_roundSat((int64_t)tempAcc, shift, minValueOutput, maxValueOutput);
        }
        output[k] = outAcc;
      }
    }
    else
    {
      //openacc(parallel loop)
      for (i0 = 0; i0 < inDataParams->dimValues[TIDL_DIM_BATCH]; i0++)
      {
        for (i1 = 0; i1 < inDataParams->dimValues[TIDL_DIM_DIM1]; i1++)
        {
          for (i2 = 0; i2 < inDataParams->dimValues[TIDL_DIM_DIM2]; i2++)
          {
            for (i3 = 0; i3 < inDataParams->dimValues[TIDL_DIM_NUMCH]; i3++)
            {
              for (i4 = 0; i4 < inDataParams->dimValues[TIDL_DIM_HEIGHT]; i4++)
              {
                for (i5 = 0; i5 < inDataParams->dimValues[TIDL_DIM_WIDTH]; i5++)
                {
                  int32_t inputIdx = (i0 * inDataParams->pitch[TIDL_ROI_PITCH]) +
                             (i1 * inDataParams->pitch[TIDL_DIM1_PITCH]) +
                             (i2 * inDataParams->pitch[TIDL_DIM2_PITCH]) +
                             (i3 * inDataParams->pitch[TIDL_CHANNEL_PITCH]) +
                             (i4 * inDataParams->pitch[TIDL_LINE_PITCH]) + i5;
                  int32_t outputIdx = (i0 * outDataParams->pitch[TIDL_ROI_PITCH]) +
                             (i1 * outDataParams->pitch[TIDL_DIM1_PITCH]) +
                             (i2 * outDataParams->pitch[TIDL_DIM2_PITCH]) +
                             (i3 * outDataParams->pitch[TIDL_CHANNEL_PITCH]) +
                             (i4 * outDataParams->pitch[TIDL_LINE_PITCH]) + i5;
                  output[outputIdx] = accPtr[inputIdx];
                }
              }
            }
          }
        }
      }
    }
    }
  }

  else
  {
    /**
     * First tensor, data : initialization data. NxCxW*H
     * Second tensor, Indices : Location at which update has to happen. NxCxP
     * Third tensor, update : values to be scattered.NxCxP
     * Values are taken from update and placed in data buffer at indices location.
     * Final data buffer is copied into output buffer output of shape NxCxW*H
     * output[index[i][j][k]][j][k] = update[i][j][k]  # if axis == 0
     * output[i][index[i][j][k]][k] = update[i][j][k]  # if axis == 1
     * output[i][j][index[i][j][k]] = update[i][j][k]  # if axis == 2
     */
    //openacc(data copyin(indices[: 1 + (numTotRoi-1)*inIndicesChPitch*numInChannels + (inIndicesNumRows-1)*inIndicesChPitch + (inIndicesNumRows-1)*inIndicesPitch + (inIndicesNumCols-1)],
                //         update[: 1 + (numTotRoi-1)*inUpdateChPitch*numInChannels + (numInChannels-1)*inUpdateChPitch + (inIndicesNumRows-1)*inUpdatePitch + (inIndicesNumCols-1)])
                //  copyout(data[: 1 + (numTotRoi-1)*inDataChPitch*numOutChannels + (numInChannels-1)*inDataChPitch + (inIndicesNumRows-1)*inDataPitch + (inIndicesNumCols-1)]))
    //openacc(parallel loop collapse(4))
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
                /*indices and updates should be of same dimension*/
                index = (int32_t)indices[(i0*inIndicesRoiPitch) + (i1*inIndicesDim1Pitch) + (i2*inIndicesDim2Pitch) + (i3*inIndicesChPitch) + (i4*inIndicesPitch) + i5];
                updateVal = update[(i0*inUpdateRoiPitch) + (i1*inUpdateDim1Pitch) + (i2*inUpdateDim2Pitch) + (i3*inUpdateChPitch) + (i4*inUpdatePitch) + i5];

                if(index >= inDataPitch)
                {
                  // #ifndef _//openacc
                  // tidl_printf(0,"something is wrong as index is higher than the allowed \n");
                  // #endif
                  continue;
                }
                
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start> FUTURE_USE: This condition is present to support future testing scenarios and it is retained for robustness and exception handling.
                <justification end> */
                if(axis == TIDL_DIM_BATCH)
                {
                  if(index < 0)
                  {
                    index = (int32_t)inDataParams->dimValues[TIDL_DIM_BATCH] + index;
                  }
                  accPtr[index + (i1*inDataDim1Pitch) + (i2*inDataDim2Pitch) + (i3*inDataChPitch) + (i4*inDataPitch) + i5] = updateVal;
                }
                else if(axis == TIDL_DIM_DIM1)
                {
                  if(index < 0)
                  {
                    index = (int32_t)inDataParams->dimValues[TIDL_DIM_DIM1] + index;
                  }
                  accPtr[(i0*inDataRoiPitch) + index + (i2*inDataDim2Pitch) + (i3*inDataChPitch) + (i4*inDataPitch) + i5] = updateVal;
                }
                else if(axis == TIDL_DIM_DIM2)
                {
                  if(index < 0)
                  {
                    index = (int32_t)inDataParams->dimValues[TIDL_DIM_DIM2] + index;
                  }
                  accPtr[(i0*inDataRoiPitch) + (i1*inDataDim1Pitch) + index + (i3*inDataChPitch) + (i4*inDataPitch) + i5] = updateVal;
                }
                else if(axis == TIDL_DIM_NUMCH)
                {
                  if(index < 0)
                  {
                    index = (int32_t)inDataParams->dimValues[TIDL_DIM_NUMCH] + index;
                  }
                  accPtr[(i0*inDataRoiPitch) + (i1*inDataDim1Pitch) + (i2*inDataDim2Pitch) + index + (i4*inDataPitch) + i5] = updateVal;
                }
                else if(axis == TIDL_DIM_HEIGHT)
                {
                  if(index < 0)
                  {
                    index = (int32_t)inDataParams->dimValues[TIDL_DIM_HEIGHT] + index;
                  }
                  accPtr[(i0*inDataRoiPitch) + (i1*inDataDim1Pitch) + (i2*inDataDim2Pitch) + (i3*inDataChPitch) + index + i5] = updateVal;
                }
                else if(axis == TIDL_DIM_WIDTH)
                /* LDRA_JUSTIFY_END */
                {
                  if(index < 0)
                  {
                    index = (int32_t)inDataParams->dimValues[TIDL_DIM_WIDTH] + index;
                  }
                  accPtr[(i0*inDataRoiPitch) + (i1*inDataDim1Pitch) + (i2*inDataDim2Pitch) + (i3*inDataChPitch) + (i4*inDataPitch) + index] = updateVal;
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

    //openacc(accPtr copyout(output[:(numTotRoi-1)*outChPitch*numOutChannels + (numOutChannels-1)*outChPitch + (outNumRows-1)*outPitch + 1])
                 //copyin(accPtr[:(numTotRoi-1)*outNumCols*outNumRows*numOutChannels + (numOutChannels-1)*outNumCols*outNumRows + (outNumRows-1)*outNumCols + 1]))
    //openacc(parallel loop collapse(3))
    /*reshape of accPtr also happens here as in accPtr is assumed to be linear*/
     //openacc(parallel loop)
    for (i0 = 0; i0 < inDataParams->dimValues[TIDL_DIM_BATCH]; i0++)
    {
      for (i1 = 0; i1 < inDataParams->dimValues[TIDL_DIM_DIM1]; i1++)
      {
        for (i2 = 0; i2 < inDataParams->dimValues[TIDL_DIM_DIM2]; i2++)
        {
          for (i3 = 0; i3 < inDataParams->dimValues[TIDL_DIM_NUMCH]; i3++)
          {
            for (i4 = 0; i4 < inDataParams->dimValues[TIDL_DIM_HEIGHT]; i4++)
            {
              for (i5 = 0; i5 < inDataParams->dimValues[TIDL_DIM_WIDTH]; i5++)
              {
                int32_t inputIdx = (i0 * inDataParams->pitch[TIDL_ROI_PITCH]) +
                            (i1 * inDataParams->pitch[TIDL_DIM1_PITCH]) +
                            (i2 * inDataParams->pitch[TIDL_DIM2_PITCH]) +
                            (i3 * inDataParams->pitch[TIDL_CHANNEL_PITCH]) +
                            (i4 * inDataParams->pitch[TIDL_LINE_PITCH]) + i5;
                int32_t outputIdx = (i0 * outDataParams->pitch[TIDL_ROI_PITCH]) +
                            (i1 * outDataParams->pitch[TIDL_DIM1_PITCH]) +
                            (i2 * outDataParams->pitch[TIDL_DIM2_PITCH]) +
                            (i3 * outDataParams->pitch[TIDL_CHANNEL_PITCH]) +
                            (i4 * outDataParams->pitch[TIDL_LINE_PITCH]) + i5;
                output[outputIdx] = accPtr[inputIdx];
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
 * @brief scatterElements layer reference implementation
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
int32_t TIDL_scatterElementsRefProcess(TIDL_Handle intAlgHandle,
                                       sTIDL_AlgLayer_t *algLayer,
                                       const sTIDL_Layer_t *tidlLayer,
                                       sTIDL_ScatterElementsParams_t *params,
                                       void *data,
                                       void *indices,
                                       void *update,
                                       void *outPtr,
                                       const sTIDL_DataParams_t *inDataParams,
                                       const sTIDL_DataParams_t *inIndicesParams,
                                       const sTIDL_DataParams_t *inUpdateParams,
                                       const sTIDL_DataParams_t *outDataParams)
{
  int32_t status = TIDL_SUCCESS;
  int32_t layerIdx = algLayer->layerIdx;
  void *accPtr = algLayer->layerParams.scatterElementsParams.accPtr;

  if(TIDL_SignedChar == ((int32_t)inDataParams->elementType))
  {

    if (TIDL_SignedWord != ((int32_t)inIndicesParams->elementType))
    {
      tidl_printf(0, "Indice data type should int32");
      status = TIDL_ERR_FAILURE;
    }
    else if (params->reduction == (int32_t)TIDL_ScatterElementsAdd)
    {
      status = TIDL_refScatterElements((int8_t *)data,
                                       (int32_t *)indices,
                                       (int8_t *)update,
                                       (int8_t *)outPtr,
                                       (int16_t *)accPtr,
                                       intAlgHandle,
                                       layerIdx,
                                       params,
                                       algLayer,
                                       tidlLayer,
                                       inDataParams,
                                       inIndicesParams,
                                       inUpdateParams,
                                       outDataParams);
    }
    else
    {
      status = TIDL_refScatterElements((int8_t *)data,
                                       (int32_t *)indices,
                                       (int8_t *)update,
                                       (int8_t *)outPtr,
                                       (int8_t *)accPtr,
                                       intAlgHandle,
                                       layerIdx,
                                       params,
                                       algLayer,
                                       tidlLayer,
                                       inDataParams,
                                       inIndicesParams,
                                       inUpdateParams,
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

    else if (params->reduction == (int32_t)TIDL_ScatterElementsAdd)
    {
      status = TIDL_refScatterElements((uint8_t *)data,
                                       (int32_t *)indices,
                                       (uint8_t *)update,
                                       (uint8_t *)outPtr,
                                       (uint16_t *)accPtr,
                                       intAlgHandle,
                                       layerIdx,
                                       params,
                                       algLayer,
                                       tidlLayer,
                                       inDataParams,
                                       inIndicesParams,
                                       inUpdateParams,
                                       outDataParams);
    }
    else
    {
      status = TIDL_refScatterElements((uint8_t *)data,
                                       (int32_t *)indices,
                                       (uint8_t *)update,
                                       (uint8_t *)outPtr,
                                       (uint8_t *)accPtr,
                                       intAlgHandle,
                                       layerIdx,
                                       params,
                                       algLayer,
                                       tidlLayer,
                                       inDataParams,
                                       inIndicesParams,
                                       inUpdateParams,
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

    else if (params->reduction == (int32_t)TIDL_ScatterElementsAdd)
    {
      status = TIDL_refScatterElements((int16_t *)data,
                                       (int32_t *)indices,
                                       (int16_t *)update,
                                       (int16_t *)outPtr,
                                       (int32_t *)accPtr,
                                       intAlgHandle,
                                       layerIdx,
                                       params,
                                       algLayer,
                                       tidlLayer,
                                       inDataParams,
                                       inIndicesParams,
                                       inUpdateParams,
                                       outDataParams);
    }
    else
    {
      status = TIDL_refScatterElements((int16_t *)data,
                                       (int32_t *)indices,
                                       (int16_t *)update,
                                       (int16_t *)outPtr,
                                       (int16_t *)accPtr,
                                       intAlgHandle,
                                       layerIdx,
                                       params,
                                       algLayer,
                                       tidlLayer,
                                       inDataParams,
                                       inIndicesParams,
                                       inUpdateParams,
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
    else if (params->reduction == (int32_t)TIDL_ScatterElementsAdd)
    {
      status = TIDL_refScatterElements((uint16_t *)data,
                                       (int32_t *)indices,
                                       (uint16_t *)update,
                                       (uint16_t *)outPtr,
                                       (uint32_t *)accPtr,
                                       intAlgHandle,
                                       layerIdx,
                                       params,
                                       algLayer,
                                       tidlLayer,
                                       inDataParams,
                                       inIndicesParams,
                                       inUpdateParams,
                                       outDataParams);
    }
    else
    {
      status = TIDL_refScatterElements((uint16_t *)data,
                                       (int32_t *)indices,
                                       (uint16_t *)update,
                                       (uint16_t *)outPtr,
                                       (uint16_t *)accPtr,
                                       intAlgHandle,
                                       layerIdx,
                                       params,
                                       algLayer,
                                       tidlLayer,
                                       inDataParams,
                                       inIndicesParams,
                                       inUpdateParams,
                                       outDataParams);
    }
  }
  else if (TIDL_SinglePrecFloat == ((int32_t)inDataParams->elementType))
  {
    /*In stat collection , all input tensor is assumed as float.
      hence making indices as float in this flow
    */
    if ((TIDL_SinglePrecFloat != ((int32_t)inIndicesParams->elementType)) &&
        (TIDL_SignedWord != ((int32_t)inIndicesParams->elementType)))
    {
      tidl_printf(0, "Indice data type should float32");
      status = TIDL_ERR_FAILURE;
    }

    else if ((TIDL_SinglePrecFloat == ((int32_t)inIndicesParams->elementType)))
    {
      status = TIDL_refScatterElements((float32_tidl *)data,
                                       (float32_tidl *)indices,
                                       (float32_tidl *)update,
                                       (float32_tidl *)outPtr,
                                       (float32_tidl *)accPtr,
                                       intAlgHandle,
                                       layerIdx,
                                       params,
                                       algLayer,
                                       tidlLayer,
                                       inDataParams,
                                       inIndicesParams,
                                       inUpdateParams,
                                       outDataParams);
    }
    else
    {
      /*do nothing*/
    }

    if ((TIDL_SignedWord == ((int32_t)inIndicesParams->elementType)))
    {
      status = TIDL_refScatterElements((float32_tidl *)data,
                                       (int32_t *)indices,
                                       (float32_tidl *)update,
                                       (float32_tidl *)outPtr,
                                       (float32_tidl *)accPtr,
                                       intAlgHandle,
                                       layerIdx,
                                       params,
                                       algLayer,
                                       tidlLayer,
                                       inDataParams,
                                       inIndicesParams,
                                       inUpdateParams,
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

/* Added new Functions */

int32_t TIDL_scatterElementsRefInit(const TIDL_LayerSpecificParams *layerSpecificParams,
                                    const TIDL_NetworkCommonParams *commonParams,
                                    sTIDL_AlgLayer_t *algLayer,
                                    int32_t layerIdx,
                                    uint8_t *memory[TIDL_LAYER_MEMORY_MAX],
                                    int32_t memorySize[TIDL_LAYER_MEMORY_MAX],
                                    void **outPtr)
{
  int32_t status = IALG_EOK;
  int32_t currOffset = 0;
  int32_t scratchDataSize = 0;
  int32_t outDataSize;
  int32_t paramMemSize = 0;

  outDataSize = TIDL_refGetOutDataSize(commonParams, layerSpecificParams, layerIdx);

  sTIDL_Layer_t *tidlLayer = &commonParams->createParams->net->TIDLLayers[layerIdx];
  int32_t outRoiPitch = (int32_t)tidlLayer->outData.pitch[TIDL_ROI_PITCH];
  int32_t outBatches = (int32_t)tidlLayer->outData.dimValues[TIDL_DIM_BATCH];
  /* memory needed for accumulate( twice of output data)[ScatterND]*/
  size_t size = TIDL_getDatElementSize(tidlLayer->outData.elementType);
  int32_t accSize = 2U * sizeof(size);
  scratchDataSize = outBatches * outRoiPitch * accSize;

  TIDL_AllocatePtr((intptr_t)memory[TIDL_LAYER_MEMORY_SCRATCH],
                   &currOffset,
                   scratchDataSize,
                   128,
                   &algLayer->layerParams.scatterElementsParams.accPtr);

  algLayer->scratchSize = scratchDataSize;
  currOffset = 0;
  if (outPtr != NULL)
  {
    if (TIDL_isOutDataBuff(commonParams->net,
                           commonParams->net->TIDLLayers[layerIdx].outData.dataId,
                           commonParams->createParams->currLayersGroupId) == 1)
    {
      *outPtr = NULL;
    }
    else if (outDataSize != 0)
    {
      TIDL_AllocatePtr((intptr_t)memory[TIDL_LAYER_MEMORY_OUTPUT],
                       &currOffset,
                       outDataSize,
                       128,
                       outPtr);
    }
    else
    {
      *outPtr = NULL;
    }
  }
  else
  {
    status = TIDL_ERR_NULL_POINTER;
  }

  memorySize[TIDL_LAYER_MEMORY_SCRATCH] = scratchDataSize + TIDL_ALIGNMENT_SIZE;
  memorySize[TIDL_LAYER_MEMORY_PERSISTENT] = paramMemSize + TIDL_ALIGNMENT_SIZE;
  memorySize[TIDL_LAYER_MEMORY_OUTPUT] = outDataSize + TIDL_ALIGNMENT_SIZE;

  return status;
}
