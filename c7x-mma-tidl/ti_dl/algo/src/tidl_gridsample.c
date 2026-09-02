/*
 *
 * Copyright (c) {2022 - 2023} Texas Instruments Incorporated
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
@file    tidl_gridsample.c
@brief   This file defines private functions for gridsample layer.
@version 0.1 (June 2023) : Initial version [ADK]
----------------------------------------------------------------------------
*/

#include "tidl_alg_int.h"
#include "tidl_gridsample.h"
#include "tidl_commonUtils.h"
#include "tidl_forceNegativeTest.h"
#ifdef BUILD_WITH_CUDA
#include "tidl_cuda.h"
#endif

using namespace floating_point::bf16_c7x;

#ifdef HOST_EMULATION
/**
 * @brief This is reference implementation of floating point GridSample layer
 *
 * @tparam Tin : template for input data buffers
 * @tparam Tout : template for output data buffers
 * @param  pIn : Pointer to input memory
 * @param  pOut : Pointer to output memory
 * @param inPtrOffset  : offset for the input buffer for gridsample
 * @param outPtrOffset : offset for the input buffer for gridsample
 * @param outWidth : width of the output buffer
 * @param outHeight : height of the output buffer
 * @param numChs : no of channels
 * @param numROIs : no of ROIs
 * @param inLinePitch : pitch of the input buffer
 * @param outLinePitch : pitch of the output buffer
 * @param inChPitch : Chnnel pitch of the input buffer
 * @param outChPitch : Chnnel pitch of the output buffer
 * @param inROIPitch : ROI pitch of the input buffer
 * @param outROIPitch : ROI pitch of the output buffer
 */
template<class Tin, class Tgrid, class Tout>
    int32_t TIDL_refGridSampleFloat(
        const Tin *pIn,
        const Tgrid *pGrid,
        Tout *pOut,
        const sTIDL_DataParams_t *inDataParams,
        const sTIDL_DataParams_t *gridParams,
        const sTIDL_Layer_t *tidlLayer)
{
  Tin *data;
  Tgrid *grid;
  int32_t status = IALG_EOK;
  const sTIDL_GridSampleParams_t *params = &tidlLayer->layerParams.gridSampleParams;
  int32_t align_corners = params->align_corners;

  int32_t data_c = (int32_t)inDataParams->dimValues[TIDL_DIM_WIDTH];
  int32_t data_h = (int32_t)inDataParams->dimValues[TIDL_DIM_NUMCH];
  int32_t data_w = (int32_t)inDataParams->dimValues[TIDL_DIM_HEIGHT];

  int32_t inBatchPitch = (int32_t)inDataParams->pitch[TIDL_ROI_PITCH];
  int32_t inDim1Pitch = (int32_t)inDataParams->pitch[TIDL_DIM1_PITCH];
  int32_t inDim2Pitch = (int32_t)inDataParams->pitch[TIDL_DIM2_PITCH];
  int32_t inChannelPitch = (int32_t)inDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t inLinePitch = (int32_t)inDataParams->pitch[TIDL_LINE_PITCH];

  int32_t outBatchPitch = (int32_t)tidlLayer->outData.pitch[TIDL_ROI_PITCH];
  int32_t outDim1Pitch = (int32_t)tidlLayer->outData.pitch[TIDL_DIM1_PITCH];
  int32_t outDim2Pitch = (int32_t)tidlLayer->outData.pitch[TIDL_DIM2_PITCH];
  int32_t outChannelPitch = (int32_t)tidlLayer->outData.pitch[TIDL_CHANNEL_PITCH];
  int32_t outLinePitch = (int32_t)tidlLayer->outData.pitch[TIDL_LINE_PITCH];

  int32_t out_n = (int32_t)tidlLayer->outData.dimValues[TIDL_DIM_BATCH];
  int32_t out_dim1 = (int32_t)tidlLayer->outData.dimValues[TIDL_DIM_DIM1];
  int32_t out_dim2 = (int32_t)tidlLayer->outData.dimValues[TIDL_DIM_DIM2];
  int32_t out_h = (int32_t)tidlLayer->outData.dimValues[TIDL_DIM_NUMCH];
  int32_t out_w = (int32_t)tidlLayer->outData.dimValues[TIDL_DIM_HEIGHT];

  int32_t inDataPitch = (int32_t)inDataParams->pitch[TIDL_LINE_PITCH];
  int32_t gridBatchPitch = (int32_t)gridParams->pitch[TIDL_ROI_PITCH];
  int32_t gridDim1Pitch = (int32_t)gridParams->pitch[TIDL_DIM1_PITCH];
  int32_t gridDim2Pitch = (int32_t)gridParams->pitch[TIDL_DIM2_PITCH];
  int32_t gridChannelPitch = (int32_t)gridParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t gridLinePitch = (int32_t)gridParams->pitch[TIDL_LINE_PITCH];
  int32_t outPitch = (int32_t)tidlLayer->outData.pitch[TIDL_LINE_PITCH];

  data = (Tin *)pIn + (inDataParams->padH * inDataPitch) + inDataParams->padW;
  grid = (Tgrid *)pGrid + (gridParams->padH * gridLinePitch) + gridParams->padW;

  Tout *out = (Tout *)pOut + (tidlLayer->outData.padH * outPitch) + tidlLayer->outData.padW;

  #ifdef BUILD_WITH_CUDA_GRIDSAMPLE
  // Use CUDA wrapper with extracted parameters
  status = TIDL_cudaGridSampleFloat<Tin, Tgrid, Tout>(
    pIn, pGrid, pOut,
    params->mode, (align_corners == 1),
    out_n, out_dim1, out_dim2, data_c, data_h, data_w,
    out_h, out_w,
    inBatchPitch, inDim1Pitch, inDim2Pitch, inChannelPitch, inLinePitch,
    gridBatchPitch, gridDim1Pitch, gridDim2Pitch, gridChannelPitch, gridLinePitch,
    outBatchPitch, outDim1Pitch, outDim2Pitch, outChannelPitch, outLinePitch,
    inDataParams->padH, inDataParams->padW,
    gridParams->padH, gridParams->padW,
    tidlLayer->outData.padH, tidlLayer->outData.padW,
    (int)inDataParams->tensorScale, (int)gridParams->tensorScale, (int)tidlLayer->outData.tensorScale,
    (int)params->is_grid_precomputed
  );
  #else

  /* bL: bottomLeft point
     bR: bottomRight point
     tL: topLeft point
     tR: topRight point
  */
  float32_tidl x, y, bL, bR, tL, tR, out_val;
  Tin bL_val, bR_val, tL_val, tR_val;
  int32_t x_bL, y_bL, x_bR, y_bR, x_tR, y_tR, x_tL, y_tL;

  for (int32_t i0 = 0; i0 < out_n; i0++)
  {
    for (int32_t i1 = 0; i1 < out_dim1; i1++)
    {
      for (int32_t i2 = 0; i2 < out_dim2; i2++)
      {
        for (int32_t i3 = 0; i3 < out_h; i3++)
        {
          for (int32_t i4 = 0; i4 < out_w; i4++)
          {
            /* find current point (x,y) in grid, also changing them back to float for mapping to corresponding point in input data */
            int32_t grid_offset = (i0 * gridBatchPitch) + (i1 * gridDim1Pitch) + (i2 * gridDim2Pitch) + (i3 * gridChannelPitch) + (i4 * gridLinePitch);

            x = (float32_tidl)(grid[grid_offset] / gridParams->tensorScale);
            y = (float32_tidl)(grid[grid_offset + 1] / gridParams->tensorScale);

            /* map (x,y) to actual point in input data based on align_corners using input widht and height */
            if (params->is_grid_precomputed == false)
            {
              if (align_corners == true)
              {
                x = ((x + 1.0F) / 2.0F) * ((float32_tidl)data_w - 1.0F);
                y = ((y + 1.0F) / 2.0F) * ((float32_tidl)data_h - 1.0F);
              }
              else
              {
                x = (((x + 1.0F) * ((float32_tidl)data_w)) - 1.0F) / 2.0F;
                y = (((y + 1.0F) * ((float32_tidl)data_h)) - 1.0F) / 2.0F;
              }
            }

            if (params->mode == (int32_t)TIDL_ModeBilinear)
            {
              x_bL = floor(x);
              y_bL = floor(y);
              x_bR = x_bL + 1;
              y_bR = y_bL;
              x_tL = x_bL;
              y_tL = y_bL + 1;
              x_tR = x_bL + 1;
              y_tR = y_bL + 1;

              /* weights of four points */
              bL = (((float32_tidl)x_tR) - x) * (((float32_tidl)y_tR) - y);
              bR = (x - ((float32_tidl)x_tL)) * (((float32_tidl)y_tL) - y);
              tL = (((float32_tidl)x_bR) - x) * (y - ((float32_tidl)y_bR));
              tR = (x - ((float32_tidl)x_bL)) * (y - ((float32_tidl)y_bL));

              for (int32_t c = 0; c < data_c; c++)
              {
                /* current channel data present at four points, if indices are going out of bound then use 0
                for 'zeros' padding mode (udpate this when we add support for other padding modes) */
                bR_val = ((y_bR >= 0) && (y_bR < data_h) && (x_bR >= 0) && (x_bR < data_w)) ? data[(i0 * inBatchPitch) + (i1 * inDim1Pitch) + (i2 * inDim2Pitch) + (c * 1) + (y_bR * inChannelPitch) + (x_bR * inLinePitch)] : (Tin)0;
                bL_val = ((y_bL >= 0) && (y_bL < data_h) && (x_bL >= 0) && (x_bL < data_w)) ? data[(i0 * inBatchPitch) + (i1 * inDim1Pitch) + (i2 * inDim2Pitch) + (c * 1) + (y_bL * inChannelPitch) + (x_bL * inLinePitch)] : (Tin)0;
                tL_val = ((y_tR >= 0) && (y_tR < data_h) && (x_tR >= 0) && (x_tR < data_w)) ? data[(i0 * inBatchPitch) + (i1 * inDim1Pitch) + (i2 * inDim2Pitch) + (c * 1) + (y_tR * inChannelPitch) + (x_tR * inLinePitch)] : (Tin)0;
                tR_val = ((y_tL >= 0) && (y_tL < data_h) && (x_tL >= 0) && (x_tL < data_w)) ? data[(i0 * inBatchPitch) + (i1 * inDim1Pitch) + (i2 * inDim2Pitch) + (c * 1) + (y_tL * inChannelPitch) + (x_tL * inLinePitch)] : (Tin)0;

                out_val = ((((float32_tidl)bR_val) * bR) + (((float32_tidl)bL_val) * bL) + (((float32_tidl)tL_val) * tR) + (((float32_tidl)tR_val) * tL)) * (tidlLayer->outData.tensorScale / inDataParams->tensorScale);
                out[(i0 * outBatchPitch) + (i1 * outDim1Pitch) + (i2 * outDim2Pitch) + c + (i3 * outChannelPitch) + (i4 * outLinePitch)] = (Tout)tidl_sat<Tout>(out_val);
              }
            }
            else if (params->mode == (int32_t)TIDL_ModeNearest)
            {
              /*Round grid values and access:*/
              for (int32_t c = 0; c < data_c; c++)
              {
                int32_t xint_round, yint_round;
                /* If the grid buffer holds pre-computed integer pixel
                   indices (written at import time by TIDL_convertWeightsAndSlopesToBF16 for
                   BF16 networks with a const-data nearest-mode grid), read the values directly
                   and skip the un-normalization that was already applied at import time. */
                if (params->is_grid_precomputed == true)
                {
                  xint_round = (int32_t)grid[grid_offset];
                  yint_round = (int32_t)grid[grid_offset + 1];
                }
                else
                {
                  /* round function in c rounds away from zero in case of 0.5, Ex: 1.5 -> 2, 2.5 -> 3
                  Onnxruntime uses round to even number which is nearbyintf function in c, it will round 1.5 to 2, 2.5 to 2,
                  hence using nearintf in place of round */
                  int32_t xint_round = nearbyintf(x);
                  int32_t yint_round = nearbyintf(y);
                }
                /*Bounds check for pad to get outVal:*/
                out_val = ((yint_round >= 0) && (yint_round < data_h) && (xint_round >= 0) && (xint_round < data_w)) ? (float32_tidl)data[(i0 * inBatchPitch) + (i1 * inDim1Pitch) + (i2 * inDim2Pitch) + (c * 1) + (yint_round * inChannelPitch) + (xint_round * inLinePitch)] : 0.0F;
                out[(i0 * outBatchPitch) + (i1 * outDim1Pitch) + (i2 * outDim2Pitch) + c + (i3 * outChannelPitch) + (i4 * outLinePitch)] = (Tout)out_val;
              }
            }
            else
            {
              /*Unsupported*/
              tidl_printf(0, "TIDL_ERROR_GRIDSAMPLE_NOT_IMPLEMENTED Error \n ");
              status = IALG_EFAIL;
              break;
            }
          }
        }
      }
    }
  }
  #endif
  TIDL_L1DandL2CacheWbInv();
  return status;
}

/**
 * @brief This is reference implementation of BFloat16 for GridSample layer
 *
 * @tparam Tin : template for input data buffers
 * @tparam Tout : template for output data buffers
 * @param  pIn : Pointer to input memory
 * @param  pOut : Pointer to output memory
 * @param inPtrOffset  : offset for the input buffer for gridsample
 * @param outPtrOffset : offset for the input buffer for gridsample
 * @param outWidth : width of the output buffer
 * @param outHeight : height of the output buffer
 * @param numChs : no of channels
 * @param numROIs : no of ROIs
 * @param inLinePitch : pitch of the input buffer
 * @param outLinePitch : pitch of the output buffer
 * @param inChPitch : Chnnel pitch of the input buffer
 * @param outChPitch : Chnnel pitch of the output buffer
 * @param inROIPitch : ROI pitch of the input buffer
 * @param outROIPitch : ROI pitch of the output buffer
 */
template<class Tin, class Tgrid, class Tout>
    int32_t TIDL_refGridSampleBFloat16(
        const Tin *pIn,
        const Tgrid *pGrid,
        Tout *pOut,
        const sTIDL_DataParams_t *inDataParams,
        const sTIDL_DataParams_t *gridParams,
        const sTIDL_Layer_t *tidlLayer)
{
  Tin *data;
  Tgrid *grid;
  int32_t status = IALG_EOK;
  const sTIDL_GridSampleParams_t *params = &tidlLayer->layerParams.gridSampleParams;
  int32_t align_corners = params->align_corners;

  int32_t data_c = (int32_t)inDataParams->dimValues[TIDL_DIM_WIDTH];
  int32_t data_h = (int32_t)inDataParams->dimValues[TIDL_DIM_NUMCH];
  int32_t data_w = (int32_t)inDataParams->dimValues[TIDL_DIM_HEIGHT];

  int32_t inBatchPitch = (int32_t)inDataParams->pitch[TIDL_ROI_PITCH];
  int32_t inDim1Pitch = (int32_t)inDataParams->pitch[TIDL_DIM1_PITCH];
  int32_t inDim2Pitch = (int32_t)inDataParams->pitch[TIDL_DIM2_PITCH];
  int32_t inChannelPitch = (int32_t)inDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t inLinePitch = (int32_t)inDataParams->pitch[TIDL_LINE_PITCH];

  int32_t outBatchPitch = (int32_t)tidlLayer->outData.pitch[TIDL_ROI_PITCH];
  int32_t outDim1Pitch = (int32_t)tidlLayer->outData.pitch[TIDL_DIM1_PITCH];
  int32_t outDim2Pitch = (int32_t)tidlLayer->outData.pitch[TIDL_DIM2_PITCH];
  int32_t outChannelPitch = (int32_t)tidlLayer->outData.pitch[TIDL_CHANNEL_PITCH];
  int32_t outLinePitch = (int32_t)tidlLayer->outData.pitch[TIDL_LINE_PITCH];

  int32_t out_n = (int32_t)tidlLayer->outData.dimValues[TIDL_DIM_BATCH];
  int32_t out_dim1 = (int32_t)tidlLayer->outData.dimValues[TIDL_DIM_DIM1];
  int32_t out_dim2 = (int32_t)tidlLayer->outData.dimValues[TIDL_DIM_DIM2];
  int32_t out_h = (int32_t)tidlLayer->outData.dimValues[TIDL_DIM_NUMCH];
  int32_t out_w = (int32_t)tidlLayer->outData.dimValues[TIDL_DIM_HEIGHT];

  int32_t inDataPitch = (int32_t)inDataParams->pitch[TIDL_LINE_PITCH];
  int32_t gridBatchPitch = (int32_t)gridParams->pitch[TIDL_ROI_PITCH];
  int32_t gridDim1Pitch = (int32_t)gridParams->pitch[TIDL_DIM1_PITCH];
  int32_t gridDim2Pitch = (int32_t)gridParams->pitch[TIDL_DIM2_PITCH];
  int32_t gridChannelPitch = (int32_t)gridParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t gridLinePitch = (int32_t)gridParams->pitch[TIDL_LINE_PITCH];
  int32_t outPitch = (int32_t)tidlLayer->outData.pitch[TIDL_LINE_PITCH];

  data = (Tin *)pIn + (inDataParams->padH * inDataPitch) + inDataParams->padW;
  grid = (Tgrid *)pGrid + (gridParams->padH * gridLinePitch) + gridParams->padW;

  Tout *out = (Tout *)pOut + (tidlLayer->outData.padH * outPitch) + tidlLayer->outData.padW;

  #ifdef BUILD_WITH_CUDA_GRIDSAMPLE
  // Use CUDA wrapper with extracted parameters
  status = TIDL_cudaGridSampleFloat<Tin, Tgrid, Tout>(
    pIn, pGrid, pOut,
    params->mode, (align_corners == 1),
    out_n, out_dim1, out_dim2, data_c, data_h, data_w,
    out_h, out_w,
    inBatchPitch, inDim1Pitch, inDim2Pitch, inChannelPitch, inLinePitch,
    gridBatchPitch, gridDim1Pitch, gridDim2Pitch, gridChannelPitch, gridLinePitch,
    outBatchPitch, outDim1Pitch, outDim2Pitch, outChannelPitch, outLinePitch,
    inDataParams->padH, inDataParams->padW,
    gridParams->padH, gridParams->padW,
    tidlLayer->outData.padH, tidlLayer->outData.padW,
    (int)inDataParams->tensorScale, (int)gridParams->tensorScale, (int)tidlLayer->outData.tensorScale, 
    (int)params->is_grid_precomputed
  );
  #else
  /* bL: bottomLeft point
     bR: bottomRight point
     tL: topLeft point
     tR: topRight point
  */
  bfloat16_tidl x, y, bL, bR, tL, tR, out_val;
  Tin bL_val, bR_val, tL_val, tR_val;
  int32_t x_bL, y_bL, x_bR, y_bR, x_tR, y_tR, x_tL, y_tL;

  for (int32_t i0 = 0; i0 < out_n; i0++)
  {
    for (int32_t i1 = 0; i1 < out_dim1; i1++)
    {
      for (int32_t i2 = 0; i2 < out_dim2; i2++)
      {
        for (int32_t i3 = 0; i3 < out_h; i3++)
        {
          for (int32_t i4 = 0; i4 < out_w; i4++)
          {
            /* find current point (x,y) in grid, also changing them back to float for mapping to corresponding point in input data */
            int32_t grid_offset = (i0 * gridBatchPitch) + (i1 * gridDim1Pitch) + (i2 * gridDim2Pitch) + (i3 * gridChannelPitch) + (i4 * gridLinePitch);

            x = grid[grid_offset];
            y = grid[grid_offset + 1];

            if (params->is_grid_precomputed == false)
            {
              /* map (x,y) to actual point in input data based on align_corners using input widht and height */
              if (align_corners == true)
              {
                x = ((x + (bfloat16_tidl)1.0F) * (bfloat16_tidl)0.5f) * ((bfloat16_tidl)data_w - (bfloat16_tidl)1.0F);
                y = ((y + (bfloat16_tidl)1.0F) * (bfloat16_tidl)0.5f) * ((bfloat16_tidl)data_h - (bfloat16_tidl)1.0F);
              }
              else
              {
                x = (((x + (bfloat16_tidl)1.0F) * ((bfloat16_tidl)data_w)) - (bfloat16_tidl)1.0F) * (bfloat16_tidl)0.5f;
                y = (((y + (bfloat16_tidl)1.0F) * ((bfloat16_tidl)data_h)) - (bfloat16_tidl)1.0F) * (bfloat16_tidl)0.5f;
              }
            }

            if (params->mode == (int32_t)TIDL_ModeBilinear)
            {
              x_bL = floor(x);
              y_bL = floor(y);
              x_bR = x_bL + 1;
              y_bR = y_bL;
              x_tL = x_bL;
              y_tL = y_bL + 1;
              x_tR = x_bL + 1;
              y_tR = y_bL + 1;

              /* weights of four points */
              bL = (((bfloat16_tidl)x_tR) - x) * (((bfloat16_tidl)y_tR) - y);
              bR = (x - ((bfloat16_tidl)x_tL)) * (((bfloat16_tidl)y_tL) - y);
              tL = (((bfloat16_tidl)x_bR) - x) * (y - ((bfloat16_tidl)y_bR));
              tR = (x - ((bfloat16_tidl)x_bL)) * (y - ((bfloat16_tidl)y_bL));

              for (int32_t c = 0; c < data_c; c++)
              {
                /* current channel data present at four points, if indices are going out of bound then use 0
                for 'zeros' padding mode (udpate this when we add support for other padding modes) */
                bR_val = ((y_bR >= 0) && (y_bR < data_h) && (x_bR >= 0) && (x_bR < data_w)) ? data[(i0 * inBatchPitch) + (i1 * inDim1Pitch) + (i2 * inDim2Pitch) + (c * 1) + (y_bR * inChannelPitch) + (x_bR * inLinePitch)] : (Tin)0.0f;
                bL_val = ((y_bL >= 0) && (y_bL < data_h) && (x_bL >= 0) && (x_bL < data_w)) ? data[(i0 * inBatchPitch) + (i1 * inDim1Pitch) + (i2 * inDim2Pitch) + (c * 1) + (y_bL * inChannelPitch) + (x_bL * inLinePitch)] : (Tin)0.0f;
                tL_val = ((y_tR >= 0) && (y_tR < data_h) && (x_tR >= 0) && (x_tR < data_w)) ? data[(i0 * inBatchPitch) + (i1 * inDim1Pitch) + (i2 * inDim2Pitch) + (c * 1) + (y_tR * inChannelPitch) + (x_tR * inLinePitch)] : (Tin)0.0f;
                tR_val = ((y_tL >= 0) && (y_tL < data_h) && (x_tL >= 0) && (x_tL < data_w)) ? data[(i0 * inBatchPitch) + (i1 * inDim1Pitch) + (i2 * inDim2Pitch) + (c * 1) + (y_tL * inChannelPitch) + (x_tL * inLinePitch)] : (Tin)0.0f;

                out_val = ((((float32_tidl)bR_val) * (float32_tidl)bR) + 
                           (((float32_tidl)bL_val) * (float32_tidl)bL) + 
                           (((float32_tidl)tL_val) * (float32_tidl)tR) + 
                           (((float32_tidl)tR_val) * (float32_tidl)tL));
                out[(i0 * outBatchPitch) + (i1 * outDim1Pitch) + (i2 * outDim2Pitch) + c + (i3 * outChannelPitch) + (i4 * outLinePitch)] = tidl_sat_bf16(out_val);
              }
            }
            else if (params->mode == (int32_t)TIDL_ModeNearest)
            {
              /*Round grid values and access:*/
              for (int32_t c = 0; c < data_c; c++)
              {
                int32_t xint_round, yint_round;
                /* If the grid buffer holds pre-computed integer pixel
                   indices (written at import time by TIDL_convertWeightsAndSlopesToBF16 for
                   BF16 networks with a const-data nearest-mode grid), read the values directly
                   and skip the un-normalization that was already applied at import time. */
                if (params->is_grid_precomputed == true)
                {
                  xint_round = (int32_t)grid[grid_offset];
                  yint_round = (int32_t)grid[grid_offset + 1];
                }
                else
                {
                  /* round function in c rounds away from zero in case of 0.5, Ex: 1.5 -> 2, 2.5 -> 3
                    Onnxruntime uses round to even number which is nearbyintf function in c, it will round 1.5 to 2, 2.5 to 2,
                    hence using nearbyintf in place of round */
                  /*nearbyintf expects input in float32. bfloat16 value internally typecasted to float32 before sending to the function. 
                  The output of the function is not affected by bf16->fp32 cast, so no need for bf16 native implementation of nearbyintf() */
                  xint_round = nearbyintf(x);
                  yint_round = nearbyintf(y);
                }
                /*Bounds check for pad to get outVal:*/
                out_val = ((yint_round >= 0) && (yint_round < data_h) && (xint_round >= 0) && (xint_round < data_w)) ? data[(i0 * inBatchPitch) + (i1 * inDim1Pitch) + (i2 * inDim2Pitch) + (c * 1) + (yint_round * inChannelPitch) + (xint_round * inLinePitch)] : (bfloat16_tidl)0.0F;
                out[(i0 * outBatchPitch) + (i1 * outDim1Pitch) + (i2 * outDim2Pitch) + c + (i3 * outChannelPitch) + (i4 * outLinePitch)] = (Tout)out_val;
              }
            }
            else
            {
              /*Unsupported*/
              tidl_printf(0, "TIDL_ERROR_GRIDSAMPLE_NOT_IMPLEMENTED Error \n ");
              status = IALG_EFAIL;
              break;
            }
          }
        }
      }
    }
  }
  #endif
  TIDL_L1DandL2CacheWbInv();
  return status;
}

/**
 * @brief This is reference implementation of GridSample layer
 *
 * @tparam Tin : template for input data buffers
 * @tparam Tout : template for output data buffers
 * @param  pIn : Pointer to input memory
 * @param  pOut : Pointer to output memory
 * @param inPtrOffset  : offset for the input buffer for gridsample
 * @param outPtrOffset : offset for the input buffer for gridsample
 * @param outWidth : width of the output buffer
 * @param outHeight : height of the output buffer
 * @param numChs : no of channels
 * @param numROIs : no of ROIs
 * @param inLinePitch : pitch of the input buffer
 * @param outLinePitch : pitch of the output buffer
 * @param inChPitch : Chnnel pitch of the input buffer
 * @param outChPitch : Chnnel pitch of the output buffer
 * @param inROIPitch : ROI pitch of the input buffer
 * @param outROIPitch : ROI pitch of the output buffer
 */

template<class Tin, class Tgrid, class Tacc, class TgridDeNorm, class TgridWeight, class Tout>
    int32_t TIDL_refGridSample(
        const Tin *pIn,
        const Tgrid *pGrid,
        Tout *pOut,
        sTIDL_DataParams_t *inDataParams,
        const sTIDL_DataParams_t *gridParams,
        const sTIDL_Layer_t *tidlLayer)
{
  Tin *data;
  Tgrid *grid;
  Tgrid xint_floor, xint_ceil, yint_floor, yint_ceil, xint_round = 0, yint_round = 0;
  Tgrid x, y;
  TgridDeNorm x_1, y_1;
  Tout out_val = 0;
  Tgrid internalGridScale;
  TgridWeight w00, w01, w10, w11;
  Tacc accumulatorValue;
  Tin x00, x01, x10, x11;
  int32_t status = IALG_EOK;

  const sTIDL_GridSampleParams_t *params = &tidlLayer->layerParams.gridSampleParams;
  int32_t align_corners = params->align_corners;

  int32_t data_c = (int32_t)inDataParams->dimValues[TIDL_DIM_WIDTH];
  int32_t data_h = (int32_t)inDataParams->dimValues[TIDL_DIM_NUMCH];
  int32_t data_w = (int32_t)inDataParams->dimValues[TIDL_DIM_HEIGHT];

  int32_t inBatchPitch = (int32_t)inDataParams->pitch[TIDL_ROI_PITCH];
  int32_t inDim1Pitch = (int32_t)inDataParams->pitch[TIDL_DIM1_PITCH];
  int32_t inDim2Pitch = (int32_t)inDataParams->pitch[TIDL_DIM2_PITCH];
  int32_t inChannelPitch = (int32_t)inDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t inLinePitch = (int32_t)inDataParams->pitch[TIDL_LINE_PITCH];

  int32_t outBatchPitch = (int32_t)tidlLayer->outData.pitch[TIDL_ROI_PITCH];
  int32_t outDim1Pitch = (int32_t)tidlLayer->outData.pitch[TIDL_DIM1_PITCH];
  int32_t outDim2Pitch = (int32_t)tidlLayer->outData.pitch[TIDL_DIM2_PITCH];
  int32_t outChannelPitch = (int32_t)tidlLayer->outData.pitch[TIDL_CHANNEL_PITCH];
  int32_t outLinePitch = (int32_t)tidlLayer->outData.pitch[TIDL_LINE_PITCH];

  int32_t out_n = (int32_t)tidlLayer->outData.dimValues[TIDL_DIM_BATCH];
  int32_t out_dim1 = (int32_t)tidlLayer->outData.dimValues[TIDL_DIM_DIM1];
  int32_t out_dim2 = (int32_t)tidlLayer->outData.dimValues[TIDL_DIM_DIM2];
  int32_t out_h = (int32_t)tidlLayer->outData.dimValues[TIDL_DIM_NUMCH];
  int32_t out_w = (int32_t)tidlLayer->outData.dimValues[TIDL_DIM_HEIGHT];

  int32_t inDataPitch = (int32_t)inDataParams->pitch[TIDL_LINE_PITCH];
  int32_t gridBatchPitch = (int32_t)gridParams->pitch[TIDL_ROI_PITCH];
  int32_t gridDim1Pitch = (int32_t)gridParams->pitch[TIDL_DIM1_PITCH];
  int32_t gridDim2Pitch = (int32_t)gridParams->pitch[TIDL_DIM2_PITCH];
  int32_t gridChannelPitch = (int32_t)gridParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t gridLinePitch = (int32_t)gridParams->pitch[TIDL_LINE_PITCH];
  int32_t outPitch = (int32_t)tidlLayer->outData.pitch[TIDL_LINE_PITCH];

  data = (Tin *)pIn + (inDataParams->padH * inDataPitch) + inDataParams->padW;
  grid = (Tgrid *)pGrid + (gridParams->padH * gridLinePitch) + gridParams->padW;

  Tout *out = (Tout *)pOut + (tidlLayer->outData.padH * outPitch) + tidlLayer->outData.padW;

  /*
    Bilinear sampling points:
    x00 ---- x01
     |        |
    x10 -----x11
  */
  /*Only 16-bit grids are supported with gridsample:*/
  // assert(typeid(Tgrid) == typeid(int16_t));
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start> FUTURE_USE: This condition is present to support future testing scenarios and it is retained for robustness and exception handling.
  <justification end> */
  if (typeid(Tgrid) != typeid(int16_t))
  {
    /*Unsupported*/
    tidl_printf(0, "TIDL_ERROR_GRIDSAMPLE_NOT_IMPLEMENTED Error: Only 16-bit grids are supported with gridsample \n ");
    status = IALG_EFAIL;
  }
  /* LDRA_JUSTIFY_END */

  // if(status == IALG_EOK) // status variable is not getting modified
  {
    /*Get the number of bits to shift*/
    int32_t temp_var = 0;
    uint32_t internalPrecisionBits = 0;
    int32_t rescaleFactor = 1U;
    if ((typeid(Tin) == typeid(int8_t)) || (typeid(Tin) == typeid(uint8_t)))
    {
      rescaleFactor = gridParams->tensorScale / (float)TIDL_GRID_INTERNAL_SCALE_8BIT;
      temp_var = (log(TIDL_GRID_INTERNAL_SCALE_8BIT) / log(2.0));
      internalPrecisionBits = (int32_t)temp_var;
      internalGridScale = TIDL_GRID_INTERNAL_SCALE_8BIT;
    }
    else
    {
      rescaleFactor = gridParams->tensorScale / (float)TIDL_GRID_INTERNAL_SCALE_16BIT;
      temp_var = (log(TIDL_GRID_INTERNAL_SCALE_16BIT) / log(2.0));
      internalPrecisionBits = (int32_t)temp_var;
      internalGridScale = TIDL_GRID_INTERNAL_SCALE_16BIT;
    }
    uint32_t rescaleShift = (log(rescaleFactor) / log(2.0));
    float32_tidl gridScale = gridParams->tensorScale;
    int32_t gridScaleFixed = (int32_t)gridScale;

    #ifdef BUILD_WITH_CUDA_GRIDSAMPLE
    // Use CUDA wrapper with extracted parameters
    status = TIDL_cudaGridSample<Tin, Tgrid, Tacc, TgridDeNorm, TgridWeight, Tout>(
      pIn, pGrid, pOut,
      params->mode, (align_corners == 1),
      out_n, out_dim1, out_dim2, data_c, data_h, data_w,
      out_h, out_w,
      inBatchPitch, inDim1Pitch, inDim2Pitch, inChannelPitch, inLinePitch,
      gridBatchPitch, gridDim1Pitch, gridDim2Pitch, gridChannelPitch, gridLinePitch,
      outBatchPitch, outDim1Pitch, outDim2Pitch, outChannelPitch, outLinePitch,
      inDataParams->padH, inDataParams->padW,
      gridParams->padH, gridParams->padW,
      tidlLayer->outData.padH, tidlLayer->outData.padW,
      (int)inDataParams->tensorScale, (int)gridParams->tensorScale, (int)tidlLayer->outData.tensorScale
    );
    #else

    for (int32_t i0=0; i0 < out_n; i0++)
    {
      for (int32_t i1 = 0; i1 < out_dim1; i1++)
      {
        for (int32_t i2 = 0; i2 < out_dim2; i2++)
        {
          for (int32_t i3 = 0; i3 < out_h; i3++)
          {
            for (int32_t i4 = 0; i4 < out_w; i4++)
            {
              /*Get the fixed point grid offsets*/
              int32_t grid_offset = (i0 * gridBatchPitch) + (i1 * gridDim1Pitch) + (i2 * gridDim2Pitch) + (i3 * gridChannelPitch) + (i4 * gridLinePitch);

              x = (Tgrid)(grid[grid_offset]);
              y = (Tgrid)(grid[grid_offset + 1]);

              /* Map to the actual position & rescale inside a new container with 2xTin width & data is in internalScale now*/
              /*Note - Check if we can do round & sat here:*/
              if (align_corners == true)
              {
                x_1 = (((x + gridScaleFixed) >> 1) * (data_w - 1)) >> rescaleShift;
                y_1 = (((y + gridScaleFixed) >> 1) * (data_h - 1)) >> rescaleShift;
              }
              else
              {
                x_1 = (((x + gridScaleFixed) * data_w) - gridScaleFixed) >> (1U + rescaleShift);
                y_1 = (((y + gridScaleFixed) * data_h) - gridScaleFixed) >> (1U + rescaleShift);
              }

              if (params->mode == (int32_t)TIDL_ModeBilinear)
              {
                /*Integer value components for bilinear:*/
                xint_floor = x_1 >> internalPrecisionBits;
                yint_floor = y_1 >> internalPrecisionBits;
                xint_ceil = xint_floor + 1;
                yint_ceil = yint_floor + 1;

                /*Get fractional parts: (3 or 7 bit)*/
                Tin fractionalMask = (1U << internalPrecisionBits) - 1U;
                Tin x_frac = (Tin)(x_1 & fractionalMask);
                Tin y_frac = (Tin)(y_1 & fractionalMask);

                /* Weights of four points (in gridScale^2)
                  Assumption here is that on device they'll not be premultiplied and pushed to gridScale^2*/
                w00 = (internalGridScale - x_frac) * (internalGridScale - y_frac);
                if ((x_frac == 0) && (y_frac == 0))
                {
                  /*Saturate w00*/
                  w00 = std::numeric_limits<TgridWeight>::max();
                }
                w01 = (x_frac) * (internalGridScale - y_frac);
                w10 = (internalGridScale - x_frac) * (y_frac);
                w11 = (x_frac) * (y_frac);

                for (int32_t c = 0; c < data_c; c++)
                {
                  /* current channel data present at four points, if indices are going out of bound then use 0
                  for 'zeros' padding mode (update this when we add support for other padding modes) */
                  x00 = ((yint_floor >= 0) && (yint_floor < data_h) && (xint_floor >= 0) && (xint_floor < data_w)) ? data[(i0 * inBatchPitch) + (i1 * inDim1Pitch) + (i2 * inDim2Pitch) + (c * 1) + (yint_floor * inChannelPitch) + (xint_floor * inLinePitch)] : (Tin)inDataParams->tensorZeroPoint;
                  x01 = ((yint_floor >= 0) && (yint_floor < data_h) && (xint_ceil >= 0) && (xint_ceil < data_w)) ? data[(i0 * inBatchPitch) + (i1 * inDim1Pitch) + (i2 * inDim2Pitch) + (c * 1) + (yint_floor * inChannelPitch) + (xint_ceil * inLinePitch)] : (Tin)inDataParams->tensorZeroPoint;
                  x10 = ((yint_ceil >= 0) && (yint_ceil < data_h) && (xint_floor >= 0) && (xint_floor < data_w)) ? data[(i0 * inBatchPitch) + (i1 * inDim1Pitch) + (i2 * inDim2Pitch) + (c * 1) + (yint_ceil * inChannelPitch) + (xint_floor * inLinePitch)] : (Tin)inDataParams->tensorZeroPoint;
                  x11 = ((yint_ceil >= 0) && (yint_ceil < data_h) && (xint_ceil >= 0) && (xint_ceil < data_w)) ? data[(i0 * inBatchPitch) + (i1 * inDim1Pitch) + (i2 * inDim2Pitch) + (c * 1) + (yint_ceil * inChannelPitch) + (xint_ceil * inLinePitch)] : (Tin)inDataParams->tensorZeroPoint;
                  /*Product in the MMA with Tacc precision*/
                  accumulatorValue = ((x00 * w00) + (x01 * w01) + (x10 * w10) + (x11 * w11));
                  /*Output is in the same scale as input, shifted by precisionBits * 2 to bring it back to outScale:*/
                  out_val = TIDL_roundSatMMA(accumulatorValue, internalPrecisionBits * 2U, std::numeric_limits<Tout>::lowest(), std::numeric_limits<Tout>::max());
                  out[(i0 * outBatchPitch) + (i1 * outDim1Pitch) + (i2 * outDim2Pitch) + c + (i3 * outChannelPitch) + (i4 * outLinePitch)] = out_val;
                }
              }
              else if (params->mode == (int32_t)TIDL_ModeNearest)
              {
                /*Round grid values and access:*/
                for (int32_t c = 0; c < data_c; c++)
                {
                  xint_round = TIDL_roundSat(x_1, internalPrecisionBits, std::numeric_limits<TgridDeNorm>::lowest(), std::numeric_limits<TgridDeNorm>::max());
                  yint_round = TIDL_roundSat(y_1, internalPrecisionBits, std::numeric_limits<TgridDeNorm>::lowest(), std::numeric_limits<TgridDeNorm>::max());
                  /*Bounds check for pad to get outVal:*/
                  out_val = ((yint_round >= 0) && (yint_round < data_h) && (xint_round >= 0) && (xint_round < data_w)) ? data[(i0 * inBatchPitch) + (i1 * inDim1Pitch) + (i2 * inDim2Pitch) + (c * 1) + (yint_round * inChannelPitch) + (xint_round * inLinePitch)] : (Tin)inDataParams->tensorZeroPoint;
                  out[(i0 * outBatchPitch) + (i1 * outDim1Pitch) + (i2 * outDim2Pitch) + c + (i3 * outChannelPitch) + (i4 * outLinePitch)] = out_val;
                }
              }
              else
              {
                /*Unsupported*/
                tidl_printf(0, "TIDL_ERROR_GRIDSAMPLE_NOT_IMPLEMENTED Error \n ");
                status = IALG_EFAIL;
                break;
              }
            }
          }
        }
      }
    }
    #endif
  }

  TIDL_L1DandL2CacheWbInv();
  return status;
}
#endif
/**
 ----------------------------------------------------------------------------
 @ingroup    TIDL_GridSample
 @fn         TIDL_gridsampleProcess
 @brief      GridSample Layer process function

 @param      intAlgHandle : tidl algorothm handle
 @param      algLayer : Ptr to alg layer parameter used in gridsample layer
 @param      tidlLayer: Ptr to tidl layer parameter used in gridsample layer
 @param      inPtrs: Ptrs to input buffers to be processed
 @param      outPtrs: Ptrs to output buffers to be processed
 @param      sysMems: Ptr to memory related buffers used in gridsample layer
 @remarks    None
 @return     IALG_EOK   - Successful
             IALG_EFAIL - Unspecified error
 ----------------------------------------------------------------------------
*/
int32_t TIDL_gridsampleProcess(TIDL_NetworkCommonParams *commonParams,
                               sTIDL_AlgLayer_t *algLayer,
                               sTIDL_Layer_t *tidlLayer,
                               void *inPtrs[],
                               void *outPtrs[],
                               int32_t layerIdx)
{

  int32_t status = IALG_EOK;

#ifdef HOST_EMULATION
  sTIDL_DataParams_t *inDataParams = &commonParams->createParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[0]].outData;
  sTIDL_DataParams_t *gridParams = &commonParams->createParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[1]].outData;

  TIDL_CreateParams createParams;
  (void)memcpy(&createParams, commonParams->createParams, sizeof(TIDL_CreateParams));

  if (commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_GRIDSAMPLE_FORCE_DATATYPE_ERROR)
  {
    /*Set to an unsupported input datatype*/
    inDataParams->elementType = TIDL_SignedWord;
  }

  if (commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_GRIDSAMPLE_FORCE_UNSUPPORTED_MODE)
  {
    /*Set to an unsupported mode*/
    tidlLayer->layerParams.gridSampleParams.mode = (int32_t)TIDL_ModeUnsupported;
  }

  if ((commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == TIDL_FLOW_CTRL_REF_ONLY)
  {
    if (inDataParams->elementType == TIDL_SinglePrecFloat)
    {
      status = TIDL_refGridSampleFloat<float32_tidl, float32_tidl, float32_tidl>((float32_tidl *)inPtrs[0], (float32_tidl *)inPtrs[1], (float32_tidl *)outPtrs[0], inDataParams, gridParams, tidlLayer);
    }
    else if (inDataParams->elementType == TIDL_BFloat16)
    {
      status = TIDL_refGridSampleBFloat16<bfloat16_tidl, bfloat16_tidl, bfloat16_tidl>(
          (bfloat16_tidl *)inPtrs[0],
          (bfloat16_tidl *)inPtrs[1],
          (bfloat16_tidl *)outPtrs[0],
          inDataParams, gridParams, tidlLayer);
    }
    else if (inDataParams->elementType == TIDL_UnsignedChar)
    {
      status = TIDL_refGridSample<uint8_t, int16_t, uint32_t, int16_t, uint8_t, uint8_t>((uint8_t *)inPtrs[0], (int16_t *)inPtrs[1], (uint8_t *)outPtrs[0], inDataParams, gridParams, tidlLayer);
    }
    else if (inDataParams->elementType == TIDL_SignedChar)
    {
      status = TIDL_refGridSample<int8_t, int16_t, int32_t, int16_t, uint8_t, int8_t>((int8_t *)inPtrs[0], (int16_t *)inPtrs[1], (int8_t *)outPtrs[0], inDataParams, gridParams, tidlLayer);
    }
    else if (inDataParams->elementType == TIDL_UnsignedShort)
    {
      status = TIDL_refGridSample<uint16_t, int16_t, uint64_t, int32_t, uint16_t, uint16_t>((uint16_t *)inPtrs[0], (int16_t *)inPtrs[1], (uint16_t *)outPtrs[0], inDataParams, gridParams, tidlLayer);
    }
    else if (inDataParams->elementType == TIDL_SignedShort)
    {
      status = TIDL_refGridSample<int16_t, int16_t, int64_t, int32_t, uint16_t, int16_t>((int16_t *)inPtrs[0], (int16_t *)inPtrs[1], (int16_t *)outPtrs[0], inDataParams, gridParams, tidlLayer);
    }
    else
    {
      TIDL_LOG_ERROR(TIDL_ERROR_GROUP_GRIDSAMPLE, TIDL_ERROR_GRIDSAMPLE_UNSUPPORTED_ELEM_TYPE);
      status = IALG_EFAIL;
    }
  }
  else
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

int32_t TIDL_gridsampleAlloc(const TIDL_LayerSpecificParams *layerSpecificParams,
                             const TIDL_NetworkCommonParams *commonParams,
                             int32_t layerIdx,
                             int32_t memorySize[TIDL_LAYER_MEMORY_MAX])
{
  int32_t status = IALG_EOK;

#ifdef HOST_EMULATION
  if (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != 0U)
  {
    status = TIDL_deviceCommonRefAlloc(layerSpecificParams, commonParams, layerIdx, memorySize);
  }
  else
#endif
  {
    status = TIDL_gridSampleDspAlloc(layerSpecificParams, commonParams, layerIdx, memorySize);
  }

  return status;
}

int32_t TIDL_gridsampleInit(const TIDL_LayerSpecificParams *layerSpecificParams,
                            const TIDL_NetworkCommonParams *commonParams,
                            sTIDL_AlgLayer_t *algLayer,
                            int32_t layerIdx,
                            uint8_t *memory[TIDL_LAYER_MEMORY_MAX],
                            int32_t memorySize[TIDL_LAYER_MEMORY_MAX],
                            void **outPtr)
{
  int32_t status = IALG_EOK;

#ifdef HOST_EMULATION
  if (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != 0U)
  {
    status = TIDL_deviceCommonRefInit(layerSpecificParams,
                                      commonParams,
                                      algLayer,
                                      layerIdx,
                                      memory,
                                      memorySize,
                                      outPtr);
  }
  else
#endif
  {
    status = TIDL_deviceUtilsCommonInit(layerSpecificParams,
                                        commonParams,
                                        algLayer,
                                        layerIdx,
                                        memory,
                                        memorySize,
                                        outPtr);
  }

  return status;
}
