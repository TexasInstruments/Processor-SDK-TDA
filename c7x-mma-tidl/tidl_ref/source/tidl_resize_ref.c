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
@file    tidl_resize_ref.c
@brief   This file defines private functions for resize layer.
@version 0.1 (Aug 2019) : Initial version
----------------------------------------------------------------------------
*/

#include <math.h>
#include <float.h>
#include "tidl_commonUtils.h"
#include "tidl_priv_algo.h"
#include "gc_helper.h"
#include "tidl_resize_ref.h"
#include "tidl_resize.h"

using namespace floating_point::bf16_c7x;

#ifdef BUILD_WITH_CUDA
#include "tidl_cuda.h"
#endif

template <class Tin>
int32_t TIDL_refResize(
    const Tin *pIn,
    Tin *pOut,
    sTIDL_AlgLayer_t *algLayer,
    const sTIDL_Layer_t *tidlLayer,
    const sTIDL_ResizeLayerParams_t *params,
    const sTIDL_DataParams_t *inDataParams,
    const TIDL_CreateParams *createParams)
{
  int32_t status = TIDL_SUCCESS;
  uint32_t numInChannels        = (uint16_t)inDataParams->dimValues[TIDL_DIM_NUMCH];
  uint32_t inWidth              = (uint16_t)inDataParams->dimValues[TIDL_DIM_WIDTH];
  uint32_t inHeight             = (uint16_t)inDataParams->dimValues[TIDL_DIM_HEIGHT];
  uint32_t inPitch              = (uint16_t)inDataParams->pitch[TIDL_LINE_PITCH];
  uint32_t inChPitch            = (uint32_t)inDataParams->pitch[TIDL_CHANNEL_PITCH];
  uint32_t outPitch             = (uint16_t)tidlLayer->outData.pitch[TIDL_LINE_PITCH];
  uint32_t outChPitch           = (uint32_t)tidlLayer->outData.pitch[TIDL_CHANNEL_PITCH];
  uint32_t outWidth             = (uint16_t)tidlLayer->outData.dimValues[TIDL_DIM_WIDTH];
  uint32_t outHeight            = (uint16_t)tidlLayer->outData.dimValues[TIDL_DIM_HEIGHT];
  uint32_t numBatches              = (int32_t)inDataParams->dimValues[TIDL_DIM_BATCH];
  uint32_t inBatchPitch           = (uint32_t)inDataParams->pitch[TIDL_ROI_PITCH];
  uint32_t outBatchPitch          = (uint32_t)tidlLayer->outData.pitch[TIDL_ROI_PITCH];

  float32_tidl wRatio ;
  float32_tidl hRatio ;


  if (params->resizeRatio[TIDL_DIM_WIDTH] > 0.0f)
  {
    wRatio = 1.0f / params->resizeRatio[TIDL_DIM_WIDTH];
  }
  else
  {
    wRatio = 0;
    status = TIDL_ERR_FAILURE;
  }
  if (status == TIDL_SUCCESS)
  {
  if (params->resizeRatio[TIDL_DIM_HEIGHT] > 0.0f)
  {
    hRatio = 1.0f / params->resizeRatio[TIDL_DIM_HEIGHT];
  }
  else
  {
    hRatio = 0;
      status = TIDL_ERR_FAILURE;
    }
  }

  if(status == TIDL_SUCCESS)
  //OPENACC(data copyin(pIn[0: 1+((numBatches-1) * inBatchPitch + (numInChannels-1) * inChPitch + (inPitch * (inHeight-1) + (inWidth-1)))])
               //copy(pOut[0: 1+((numBatches-1) * outBatchPitch + (numInChannels-1) * outChPitch + (outPitch * (outHeight-1) + (outWidth-1)))]))
{
  #ifdef BUILD_WITH_CUDA_RESIZE
  // call cuda resize wrapper
  uint32_t leftPadResize = (TIDL_isPadOTF(createParams->net->deviceName) == 1U) ? 0 : 1;
  status = TIDL_cudaResize<Tin>(pIn, pOut, numBatches, numInChannels, inHeight, inWidth, outHeight, outWidth, inBatchPitch, inChPitch, inPitch,
                                outBatchPitch, outChPitch, outPitch, params->resizeRatio[TIDL_DIM_HEIGHT], params->resizeRatio[TIDL_DIM_WIDTH], 
                                params->resizePadZeroOffset, params->mode, leftPadResize, 0, 0, 0, 0, tidlLayer->outData.tensorZeroPoint);
  #else
  if (params->mode == TIDL_ResizePadZero)
  {
    // initialize output buffer with zeroes
    memset (pOut, 0, (numBatches * outBatchPitch * sizeof(Tin)) );

    int32_t i1 = 0, i2 = 0, i3 = 0, i4 = 0;

    for (i1 = 0; i1 < (int32_t)numBatches; i1++)
    {
      for (i2 = 0; i2 < (int32_t)numInChannels; i2++)
      {
        for (i3 = 0; i3 < (int32_t)inHeight; i3++)
        {
          for (i4 = 0; i4 < (int32_t)inWidth; i4++)
          {
            int32_t inputOffset = (i1 * inBatchPitch) + (i2 * inChPitch) + (i3 * inPitch) + i4;
            int32_t outputOffset = (i1 * outBatchPitch) + (i2 * outChPitch) + (i3 * (int32_t)params->resizeRatio[TIDL_DIM_HEIGHT] + params->resizePadZeroOffset)* outPitch + (i4 * (int32_t)params->resizeRatio[TIDL_DIM_WIDTH] + params->resizePadZeroOffset);

            pOut[outputOffset] = pIn[inputOffset];
          }
        }
      }
    }
  }
  else if (params->mode == TIDL_ResizeNearest)
  {
    uint32_t i = 0;
    uint32_t j = 0;
    uint32_t k = 0;
    uint32_t l = 0;
    float32_tidl hLoc, wLoc;
    uint32_t hIdx, wIdx;

    //OPENACC(parallel loop collapse(4))
    for(l=0; l<numBatches; l++)
    {
    for (k = 0; k < numInChannels; k++)
    {
      for (i = 0; i < outHeight; i++)
      {
        for (j = 0; j < outWidth; j++)
      {
          hLoc = (hRatio * ((float32_tidl)i + 0.5)) - 0.5;
          hLoc = (hLoc < 0.0f) ? 0.0f : hLoc;
          hIdx = hLoc + 0.5;
              /* LDRA_JUSTIFY_START
              <metric start> statement branch <metric end>
              <justification start>
              Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
              Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
              <justification end> */
          hIdx = (hIdx < inHeight) ? hIdx : (inHeight - 1U);
              /* LDRA_JUSTIFY_END */

          wLoc = (wRatio * ((float32_tidl)j + 0.5)) - 0.5;
          wLoc = (wLoc < 0.0f) ? 0.0f : wLoc;
          wIdx = wLoc + 0.5;
              /* LDRA_JUSTIFY_START
              <metric start> statement branch <metric end>
              <justification start>
              Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
              Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
              <justification end> */
          wIdx = (wIdx < inWidth) ? wIdx : (inWidth - 1U);
              /* LDRA_JUSTIFY_END */
          uint32_t inputOffset = (inPitch * hIdx) + wIdx;
          uint32_t outputOffset = (outPitch * i) + j;

          pOut[(l * outBatchPitch) + (k * outChPitch) + outputOffset] = pIn[(l * inBatchPitch) + (k * inChPitch) + inputOffset];
          //*(pOut + l * outBatchPitch + k * outChPitch + outputOffset) = *(pIn + l * inBatchPitch + k * inChPitch + inputOffset);
        }
      }
    }
    }
  }
    else if (params->mode == TIDL_ResizeBilinear)
  {
    int32_t i = 0;
    int32_t j = 0;
    int32_t k = 0;
    int32_t l = 0;
    float32_tidl hLoc, wLoc;
    int32_t hIdx, wIdx, hNext, wNext;
    float32_tidl w00, w01, w10, w11;

    //OPENACC(parallel loop collapse(4))
    for(l=0; l<(int32_t)numBatches; l++)
    {
    for (k = 0; k < (int32_t)numInChannels; k++)
    {
      for (i = 0; i < (int32_t)outHeight; i++)
      {
        for (j = 0; j < (int32_t)outWidth; j++)
        {

          hLoc = (hRatio * ((float32_tidl)i + 0.5)) - 0.5;
          if(hLoc < 0.0f)
          {
            hIdx = floor(hLoc);
          }
          else
          {
            hIdx = hLoc;
          }
          hNext = 1; /* By default, we are at current line and hNext = 1 corresponds to next line */
          w11 = hLoc - (float32_tidl)hIdx;
          w10 = 1.0f - w11;

          wLoc = (wRatio * ((float32_tidl)j + 0.5)) - 0.5;
          wLoc = (wLoc < 0.0f) ? 0.0f : wLoc;
          wIdx = wLoc;
          w01 = wLoc - (float32_tidl)wIdx;
          w00 = 1.0f - w01;
          int32_t inputOffset = ((int32_t)inPitch * hIdx) + wIdx;
          int32_t inputOffset2 = inputOffset;
          int32_t outputOffset = ((int32_t)outPitch * (i)) + (j);

          wNext = (wIdx < ((int32_t)inWidth - 1)) ? 1 : 0;
          /* Calculate the offset for second line differently when horizontal clipping is disabled */
          if(inputOffset < 0)
          {
            inputOffset2 = wIdx;
            /* pIn corresponds to valid data (i.e. excludes top padding). inputOffset < 0 indicates we are at first valid line and need to start from top padding region
            to generate first output line for resize
            In this context, i00 and i01 calculated below correspond to top padding region. Next line for resize is the first valid line which corresponds to current ptr pIn
            so hNext must be 0 for i10 and i11 calculation */
            hNext = 0;
          }

          Tin i00 = *(pIn + (l * (int32_t)inBatchPitch) + (k * (int32_t)inChPitch) + inputOffset);
          Tin i01 = *(pIn + (l * (int32_t)inBatchPitch) + (k * (int)inChPitch) + inputOffset + wNext);
          Tin i10 = *(pIn + (l * (int32_t)inBatchPitch) + (k * (int)inChPitch) + inputOffset2 + (hNext * (int32_t)inPitch));
          Tin i11 = *(pIn + (l * (int32_t)inBatchPitch) + (k * (int32_t)inChPitch) + inputOffset2 + (hNext * (int32_t)inPitch) + wNext);
#ifdef HOST_EMULATION   
          int32_t widthResizeRatio;
          int32_t heightResizeRatio;
          int32_t inter1, inter2;
          uint8_t w00Int, w01Int;
          uint8_t w10Int, w11Int;
          uint32_t leftshift_operation1 = 0U;
          uint32_t leftshift_operation2 = 0U;

          if ((params->resizeRatio[TIDL_DIM_HEIGHT] == params->resizeRatio[TIDL_DIM_WIDTH]) && ((params->resizeRatio[TIDL_DIM_HEIGHT] == 2.0)|| (params->resizeRatio[TIDL_DIM_HEIGHT] == 4.0)))
          {
            if(params->resizeRatio[TIDL_DIM_HEIGHT] == 4.0f)
            {
              heightResizeRatio = 3;
              widthResizeRatio  = 3;
            }
            else
            {
              heightResizeRatio = 2;
              widthResizeRatio  = 2;
            }
            leftshift_operation1 = ((uint64_t)1 << (uint32_t)widthResizeRatio);
            leftshift_operation2 = ((uint64_t)1 << (uint32_t)heightResizeRatio);
            w00Int = w00 * (float32_tidl)leftshift_operation1;
            w01Int = leftshift_operation1 - w00Int;
            w10Int = w10 * (float32_tidl)leftshift_operation2;
            w11Int = leftshift_operation2 - w10Int;
            inter1 = ((i00 * w00Int) + (i01 * w01Int)) ;
            inter2 = ((i10 * w00Int) + (i11 * w01Int)) ;
            // *(pOut + l * outBatchPitch + k * outChPitch + outputOffset) = (w10Int * inter1 + w11Int * inter2) >> (heightResizeRatio + widthResizeRatio);
            pOut[(l * (int32_t)outBatchPitch) + (k * (int32_t)outChPitch) + outputOffset] = ((w10Int * (uint32_t)inter1) + (w11Int * (uint32_t)inter2)) >> ((uint32_t)heightResizeRatio + (uint32_t)widthResizeRatio);

          }
          else
#endif
          {
            pOut[(l * (int32_t)outBatchPitch) + (k * (int32_t)outChPitch) + outputOffset] = ((w10 * ((i00 * w00) + (i01 * w01))) + (w11 * ((i10 * w00) + (i11 * w01))));
            //*(pOut + l * outBatchPitch + k * outChPitch + outputOffset) = (w10 * (i00 * w00 + i01 * w01) + w11 * (i10 * w00 + i11 * w01));
          }
        }
      }
    }
    }
  }
  else
  {
    tidl_printf(0, "params->mode is  Not supported !!!\n ");
    status = TIDL_ERR_FAILURE;
  }
  #endif
  }
  return status;
}

#ifdef HOST_EMULATION
/**
 * @brief This is reference implementation of Resize layer
 *
 * @param tidlLayer : Pointer to the common layer parameters
 * @param inPtrs : Pointer to input memory
 * @param outPtr : Pointer to output memory
 * @param inDataParams : parameters of the input data buffer
 * @param outDataParams : parameters of the output data buffer
 * @param basePrmPtr : Copy of Resize layer parameters
 */
template <class Tin, class Tout>
int32_t TIDL_resizeProcessSP(
    sTIDL_Layer_t *tidlLayer,
    void *inPtrs[],
    void *outPtr,
    sTIDL_DataParams_t *inDataParams[],
    sTIDL_DataParams_t *outDataParams,
    uint8_t *basePrmPtr)
{
  int32_t status = TIDL_SUCCESS;
  sTIDL_ResizeLayerParams_t *params = &tidlLayer->layerParams.resizeParams;
  Tin *in = (Tin *)inPtrs[0];
  Tout *out = (Tout *)outPtr;

  uint32_t numInChannels = (uint16_t)inDataParams[0]->dimValues[TIDL_DIM_NUMCH];
  uint32_t inWidth = (uint16_t)inDataParams[0]->dimValues[TIDL_DIM_WIDTH];
  uint32_t inHeight = (uint16_t)inDataParams[0]->dimValues[TIDL_DIM_HEIGHT];
  uint32_t inPitch = (uint16_t)inDataParams[0]->pitch[TIDL_LINE_PITCH];
  uint32_t inChPitch = (uint32_t)inDataParams[0]->pitch[TIDL_CHANNEL_PITCH];
  uint32_t outPitch = (uint16_t)tidlLayer->outData.pitch[TIDL_LINE_PITCH];
  uint32_t outChPitch = (uint32_t)tidlLayer->outData.pitch[TIDL_CHANNEL_PITCH];
  uint32_t outWidth = (uint16_t)tidlLayer->outData.dimValues[TIDL_DIM_WIDTH];
  uint32_t outHeight = (uint16_t)tidlLayer->outData.dimValues[TIDL_DIM_HEIGHT];
  int32_t numBatches = (int32_t)inDataParams[0]->dimValues[TIDL_DIM_BATCH];
  uint32_t inBatchPitch = (uint32_t)inDataParams[0]->pitch[TIDL_ROI_PITCH];
  uint32_t outBatchPitch = (uint32_t)tidlLayer->outData.pitch[TIDL_ROI_PITCH];

  uint32_t inOffset = ((1U) * inPitch) + 1U;
  uint32_t outOffset = 0;

  float32_tidl wRatio;
  float32_tidl hRatio;

  if (params->resizeRatio[TIDL_DIM_WIDTH] > 0.0f)
  {
    wRatio = 1.0f / params->resizeRatio[TIDL_DIM_WIDTH];
  }
  else
  {
    wRatio = 0.0f;
    status = TIDL_ERR_FAILURE;
  }
  if (status == TIDL_SUCCESS)
  {
    if (params->resizeRatio[TIDL_DIM_HEIGHT] > 0.0f)
    {
      hRatio = 1.0f / params->resizeRatio[TIDL_DIM_HEIGHT];
    }
    else
    {
      hRatio = 0.0f;
      status = TIDL_ERR_FAILURE;
    }
  }

  if (status == TIDL_SUCCESS)
  //OPENACC(data copyin(in[0: 1+((numBatches-1) * inBatchPitch + (numInChannels-1) * inChPitch + (inPitch * (inHeight-1) + (inWidth-1)))])
               //copyout(out[0: 1+((numBatches-1) * outBatchPitch + (numInChannels-1) * outChPitch + (outPitch * (outHeight-1) + (outWidth-1)))]))
{
  #ifdef BUILD_WITH_CUDA_RESIZE
  // call cuda resize wrapper
  status = TIDL_cudaResize<Tin>(in, out, numBatches, numInChannels, inHeight, inWidth, outHeight, outWidth, inBatchPitch, inChPitch, inPitch,
                                outBatchPitch, outChPitch, outPitch, params->resizeRatio[TIDL_DIM_HEIGHT], params->resizeRatio[TIDL_DIM_WIDTH], 
                                params->resizePadZeroOffset, params->mode, 0, inDataParams[0]->padH, inDataParams[0]->padW, inOffset, outOffset, tidlLayer->outData.tensorZeroPoint);
  #else
  if (params->mode == TIDL_ResizePadZero)
  {
    // initialize output buffer with zeroes
    memset (out, 0, (numBatches * outBatchPitch * sizeof (Tout)) );

    in               = in + (inDataParams[0]->padH*inPitch) + inDataParams[0]->padW;
    // out              = out + (tidlLayer->outData.padH*outPitch) + tidlLayer->outData.padW;

    int32_t i1 = 0, i2 = 0, i3 = 0, i4 = 0;

    for (i1 = 0; i1 < (int32_t)numBatches; i1++)
    {
      for (i2 = 0; i2 < (int32_t)numInChannels; i2++)
      {
        for (i3 = 0; i3 < (int32_t)inHeight; i3++)
        {
          for (i4 = 0; i4 < (int32_t)inWidth; i4++)
          {
            int32_t inputOffset = (i1 * inBatchPitch) + (i2 * inChPitch) + (i3 * inPitch) + i4;
            int32_t outputOffset = (i1 * outBatchPitch) + (i2 * outChPitch) + (i3 * (int32_t)params->resizeRatio[TIDL_DIM_HEIGHT] + params->resizePadZeroOffset)* outPitch + (i4 * (int32_t)params->resizeRatio[TIDL_DIM_WIDTH] + params->resizePadZeroOffset);

            out[outputOffset] = in[inputOffset];
          }
        }
      }
    }
  }
  else if (params->mode == TIDL_ResizeNearest)
  {
    uint32_t i = 0;
    uint32_t j = 0;
    uint32_t k = 0;
    uint32_t l = 0;
    float32_tidl hLoc, wLoc;
    uint32_t hIdx, wIdx;

    //OPENACC(parallel loop collapse(4))
    for (l = 0; l < (uint32_t)numBatches; l++)
    {
    for (i = 0; i < outHeight; i++)
    {
      for (j = 0; j < outWidth; j++)
      {
        for (k = 0; k < numInChannels; k++)
      {
        hLoc = (hRatio * ((float32_tidl)i + 0.5f)) - 0.5f;
        hLoc = (hLoc < 0.0f) ? 0.0f : hLoc;
        hIdx = hLoc+0.5f;
              /* LDRA_JUSTIFY_START
              <metric start> statement branch <metric end>
              <justification start>
              Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
              Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
              <justification end> */
        hIdx = (hIdx < inHeight) ? hIdx : (inHeight-1U);
              /* LDRA_JUSTIFY_END */

        wLoc = (wRatio * ((float32_tidl)j + 0.5f)) - 0.5f;
        wLoc = (wLoc < 0.0f) ? 0.0f : wLoc;
        wIdx = wLoc+0.5f;
              /* LDRA_JUSTIFY_START
              <metric start> statement branch <metric end>
              <justification start>
              Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
              Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
              <justification end> */
        wIdx = (wIdx < inWidth) ? wIdx : (inWidth-1U);
              /* LDRA_JUSTIFY_END */
        uint32_t inputOffset = inOffset + (inPitch  * hIdx) + wIdx;
        uint32_t outputOffset = outOffset + (outPitch * i) + j;

        out[(l*outBatchPitch) + (k*outChPitch) + outputOffset] = in[(l*inBatchPitch) + (k*inChPitch) + inputOffset];
        //*(out + l*outBatchPitch + k*outChPitch + outputOffset) = *(in + l*inBatchPitch + k*inChPitch + inputOffset);
        }
      }
    }
    }
  }
    else if (params->mode == TIDL_ResizeBilinear)
  {
    uint32_t i = 0;
    uint32_t j = 0;
    uint32_t k = 0;
    uint32_t l = 0;
    float32_tidl hLoc, wLoc;
    uint32_t hIdx, wIdx, hNext, wNext;
    Tin w00, w01, w10, w11;

    //OPENACC(parallel loop collapse(4))
    for (l = 0; l < (uint32_t)numBatches; l++)
    {
    for (i = 0; i < outHeight; i++)
    {
      for (j = 0; j < outWidth; j++)
      {
        for (k = 0; k < numInChannels; k++)
      {
        hLoc = (hRatio * ((float32_tidl)i + 0.5f)) - 0.5f;
        hLoc = (hLoc < 0.0f) ? 0.0f : hLoc;
        hIdx = hLoc;
        hNext = (hIdx < (inHeight-1U)) ? 1U : 0U;
        w11 = hLoc - (float32_tidl)hIdx;
        w10 = (Tin)1.0f - w11;

        wLoc = (wRatio * ((float32_tidl)j + 0.5f)) - 0.5f;
        wLoc = (wLoc < 0.0f) ? 0.0f : wLoc;
        wIdx = wLoc;
        w01 = wLoc - (float32_tidl)wIdx;
        w00 = (Tin)1.0f - w01;
        int32_t inputOffset = inOffset+ (inPitch  * hIdx) + wIdx;
        int32_t outputOffset = outOffset + (outPitch * (i)) + (j);
        wNext = (wIdx < (inWidth-1U)) ? 1U : 0U;

        float32_tidl i00 = *(in + (l*inBatchPitch) + (k*inChPitch) + inputOffset);
        float32_tidl i01 = *(in + (l*inBatchPitch) + (k*inChPitch) + inputOffset + wNext);
        float32_tidl i10 = *(in + (l*inBatchPitch) + (k*inChPitch) + inputOffset + (hNext*inPitch));
        float32_tidl i11 = *(in + (l*inBatchPitch) + (k*inChPitch) + inputOffset + (hNext*inPitch) + wNext);

        out[(l*outBatchPitch) + (k*outChPitch) + (uint32_t)outputOffset] = (((float32_tidl)w10*((i00* (float32_tidl)w00) +  (i01* (float32_tidl)w01))) +
                                                                           ((float32_tidl)w11*((i10* (float32_tidl)w00) +  (i11* (float32_tidl)w01))));
        //*(out + l*outBatchPitch + k*outChPitch + outputOffset) = (w10*(i00* w00 +  i01* w01) + w11*(i10* w00 +  i11* w01));
        }
      }
    }
    }
  }
  else
  {
    tidl_printf(0,"params->mode is  Not supported !!!\n ");
    status = TIDL_ERR_FAILURE;
  }
  #endif
}
  return status;
}
#endif

int32_t TIDL_resizeRefProcess(const TIDL_CreateParams *createParams,
                                     sTIDL_AlgLayer_t *algLayer,
                                     sTIDL_Layer_t *tidlLayer,
                                     void *inPtrs[],
                                     void *outPtrs[],
                                     int32_t layerIdx)
{
  int32_t status = TIDL_SUCCESS;
  sTIDL_DataParams_t *inDataParams;
  inDataParams = &createParams->net->TIDLLayers[algLayer->inLayerIdx[0]].outData;
  sTIDL_ResizeLayerParams_t *params = &tidlLayer->layerParams.resizeParams;

  uint32_t inElmtSize     = TIDL_getDatElementSize(inDataParams->elementType);

  uint16_t inPitch        = (uint16_t)inDataParams->pitch[TIDL_LINE_PITCH];
  uint16_t inPitchBytes   = inPitch * inElmtSize;
  uint32_t leftPadResize = (TIDL_isPadOTF(createParams->net->deviceName) == 1U) ? 0 : 1;

  /* Output parameters */
  uint32_t resizeNumChannels       = tidlLayer->outData.dimValues[TIDL_DIM_NUMCH];

  /* Input parameters */
  uint32_t resizeInChPitch    = inDataParams->pitch[TIDL_CHANNEL_PITCH];
  uint32_t resizeInHeight     = inDataParams->dimValues[TIDL_DIM_HEIGHT];
  uint32_t resizeInWidth      = inDataParams->dimValues[TIDL_DIM_WIDTH];
  uint32_t resizeInWidthBytes = resizeInWidth * inElmtSize;
  uint32_t resizeInChPitchBytes= resizeInChPitch * inElmtSize;

  uint8_t *outPtr         = ((uint8_t *)outPtrs[0]);
  uint8_t *inPtrOrig = ((uint8_t *)inPtrs[0]);
  uint8_t *inPtr     = ((uint8_t *)inPtrs[0]) + ((((1U)*inPitch) + leftPadResize) * inElmtSize);
  int32_t copyTopLine = 0, copyBottomLine = 0;
  uint32_t inputHeight = 0;
  if((createParams->forceNegativeTest == TIDL_SAFETY_FLAG_RESIZE_FORCE_PROCESSNEW_WIDTHRATIO_ZERO) || (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_RESIZE_FORCE_PROCESSSP_WIDTHRATIO_ZERO))
  {
    params->resizeRatio[TIDL_DIM_WIDTH] = 0;
  }
  if((createParams->forceNegativeTest == TIDL_SAFETY_FLAG_RESIZE_FORCE_PROCESSNEW_HEIGHTRATIO_ZERO) || (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_RESIZE_FORCE_PROCESSSP_HEIGHTRATIO_ZERO))
  {
    params->resizeRatio[TIDL_DIM_HEIGHT] = 0;
  }
#ifdef HOST_EMULATION
  if(createParams->net->weightsElementSize == 4)
  {
    sTIDL_DataParams_t * inDataPrms[TIDL_NUM_IN_BUFS] = {0};
    for(int j=0; j < tidlLayer->numInBufs; j++)
    {
      inDataPrms[j]  = &createParams->net->TIDLLayers[algLayer->inLayerIdx[j]].outData;
    }
    status = TIDL_resizeProcessSP<float32_tidl, float32_tidl>(tidlLayer, inPtrs, outPtrs[0], inDataPrms, &tidlLayer->outData, (uint8_t*)createParams->net);
    return (status);
  }
  else if(inDataParams->elementType == TIDL_BFloat16)
  {
    sTIDL_DataParams_t * inDataPrms[TIDL_NUM_IN_BUFS] = {0};
    for(int j=0; j < tidlLayer->numInBufs; j++)
    {
      inDataPrms[j]  = &createParams->net->TIDLLayers[algLayer->inLayerIdx[j]].outData;
    }
    status = TIDL_resizeProcessSP<bfloat16_tidl, bfloat16_tidl>(tidlLayer, inPtrs, outPtrs[0], inDataPrms, &tidlLayer->outData, (uint8_t*)createParams->net);
    return (status);
  }
#endif
  if (params->mode == TIDL_ResizeBilinear)
  {
    // inPtr     = ((uint8_t *)inPtrs[0]) +  ((1)*inPitch + 1) * inElmtSize;
    // if(TIDL_privGetProcType(localDataFlowPtr) == 0)
    // {
    sTIDL_ResizeInfo_t resizeInfo;
    getResizeInfo(&resizeInfo, algLayer, tidlLayer, createParams, resizeInHeight);

    inputHeight = resizeInfo.inputHeight;

    copyTopLine = ( (resizeInfo.copyLineInfo & TIDL_COPY_TOP_LINE) == TIDL_COPY_TOP_LINE);
    copyBottomLine = ( (resizeInfo.copyLineInfo & TIDL_COPY_BOTTOM_LINE) == TIDL_COPY_BOTTOM_LINE);


    uint32_t numBatches     = (int32_t)inDataParams->dimValues[TIDL_DIM_BATCH];
    uint32_t inBatchPitch  = (uint32_t)inDataParams->pitch[TIDL_ROI_PITCH];

    //OPENACC(data copy(inPtrOrig[0:1+(numBatches-1) * inBatchPitch + (resizeNumChannels-1) * resizeInChPitchBytes + (inElmtSize*leftPadResize) + (inputHeight + 1) * inPitchBytes + (resizeInWidthBytes-1)]))
    // OPENACC(data copyin(inPtr[0:1+(numBatches-1) * inBatchPitch + (resizeNumChannels-1) * resizeInChPitchBytes + (inputHeight + 1) * inPitchBytes + (resizeInWidthBytes-1)]))
    //OPENACC(parallel loop collapse(3))
    for (uint32_t k = 0; k < numBatches; k++)
    {
      for (uint32_t c = 0; c < resizeNumChannels; c++)
      {
        for (uint32_t j = 0; j < resizeInWidthBytes; j++)
        {
          /* LDRA_JUSTIFY_START
          <metric start> branch <metric end>
          <justification start>
          Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
          Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
          <justification end> */
          if(1 == copyTopLine)
          {
            /* LDRA_JUSTIFY_END */
            // *(inPtrOrig + k * inBatchPitch + c * resizeInChPitchBytes + (inElmtSize*leftPadResize) + j) = *(inPtr + k * inBatchPitch + c * resizeInChPitchBytes + j);
            inPtrOrig[(k * inBatchPitch) + (c * resizeInChPitchBytes) + (inElmtSize*leftPadResize) + j] = inPtrOrig[(k * inBatchPitch) + (c * resizeInChPitchBytes) + j + ((((1U)*inPitch) + leftPadResize) * inElmtSize)];
          }
          /* LDRA_JUSTIFY_START
          <metric start> branch <metric end>
          <justification start>
          Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
          Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
          <justification end> */
          if(1 == copyBottomLine)
          {
            /* LDRA_JUSTIFY_END */
            // *(inPtrOrig + k * inBatchPitch + c * resizeInChPitchBytes + (inElmtSize*leftPadResize) + (inputHeight + 1) * inPitchBytes + j) =
                //     *(inPtr + k * inBatchPitch + c * resizeInChPitchBytes + (inputHeight - 1) * inPitchBytes + j);
            inPtrOrig[(k * inBatchPitch) + (c * resizeInChPitchBytes) + (inElmtSize*leftPadResize) + ((inputHeight + 1U) * inPitchBytes) + j] =
                inPtrOrig[(k * inBatchPitch) + (c * resizeInChPitchBytes) + ((inputHeight - 1U) * inPitchBytes) + j + ((((1U)*inPitch) + leftPadResize) * inElmtSize)];
          }
        }
      }
    }
  }
  if (inDataParams->elementType == TIDL_SignedChar)
  {
    status = TIDL_refResize((int8_t *)inPtr, (int8_t *)outPtr, algLayer, tidlLayer, params, inDataParams, createParams);
  }
  else if (inDataParams->elementType == TIDL_UnsignedChar)
  {
    status = TIDL_refResize((uint8_t *)inPtr, (uint8_t *)outPtr, algLayer, tidlLayer, params, inDataParams, createParams);
  }
  else if (inDataParams->elementType == TIDL_SignedShort)
  {
    status = TIDL_refResize((int16_t *)inPtr, (int16_t *)outPtr, algLayer, tidlLayer, params, inDataParams, createParams);
  }
  else if (inDataParams->elementType == TIDL_UnsignedShort)
  {
    status = TIDL_refResize((uint16_t *)inPtr, (uint16_t *)outPtr, algLayer, tidlLayer, params, inDataParams, createParams);
  }
  else
  {
    /* tidl_printf(0, "TIDL_ResizeProcess in elementType is  Not supported !!!\n "); */
    TIDL_LOG_ERROR(TIDL_ERROR_GROUP_RESIZE, TIDL_ERROR_RESIZE_UNSUPPORTED_ELEM_TYPE);
    status = IALG_EFAIL;
  }
  if(status ==  TIDL_SUCCESS)
  {
  /* Update the inPtrOrig again if it was changed to copy lines */
  if (params->mode == TIDL_ResizeBilinear)
  {
    sTIDL_ResizeInfo_t resizeInfo;
    getResizeInfo(&resizeInfo, algLayer, tidlLayer, createParams, resizeInHeight);

    inputHeight = resizeInfo.inputHeight;

    copyTopLine = ( (resizeInfo.copyLineInfo & TIDL_COPY_TOP_LINE) == TIDL_COPY_TOP_LINE);
    copyBottomLine = ( (resizeInfo.copyLineInfo & TIDL_COPY_BOTTOM_LINE) == TIDL_COPY_BOTTOM_LINE);

    uint32_t numBatches     = (int32_t)inDataParams->dimValues[TIDL_DIM_BATCH];
    uint32_t inBatchPitch  = (uint32_t)inDataParams->pitch[TIDL_ROI_PITCH];

    for (uint32_t k = 0; k < numBatches; k++)
    {
      for (uint32_t c = 0; c < resizeNumChannels; c++)
      {
        for (uint32_t j = 0; j < resizeInWidthBytes; j++)
        {
          /* LDRA_JUSTIFY_START
          <metric start> branch <metric end>
          <justification start>
          Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
          Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
          <justification end> */
          if(1 == copyTopLine)
          {
            /* LDRA_JUSTIFY_END */
            inPtrOrig[(k * inBatchPitch) + (c * resizeInChPitchBytes) + (inElmtSize*leftPadResize) + j] = 0;
          }
          /* LDRA_JUSTIFY_START
          <metric start> branch <metric end>
          <justification start>
          Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
          Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
          <justification end> */
          if(1 == copyBottomLine)
          {
            /* LDRA_JUSTIFY_END */
            inPtrOrig[(k * inBatchPitch) + (c * resizeInChPitchBytes) + (inElmtSize*leftPadResize) + ((inputHeight + 1U) * inPitchBytes) + j] = 0;
          }
        }
      }
    }
  }
#ifdef HOST_EMULATION
  if ((createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) == TIDL_FLOW_CTRL_REF_STAT)
  {
    TIDL_Obj intAlgObj;
    TIDL_CreateParams createParamsCpy;

    (void)memcpy(&createParamsCpy, createParams, sizeof(TIDL_CreateParams));
    intAlgObj.createParams = (TIDL_CreateParams *) &createParamsCpy;

    TIDL_UpdateScaleFactors(&intAlgObj, layerIdx, 1, 0, 0);
  }
#endif
  }

  TIDL_L1DandL2CacheWbInv();

  return status;
}
