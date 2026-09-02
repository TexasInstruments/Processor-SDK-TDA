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
@file    tidl_pad_ref.c
@brief   This file defines private functions for pad layer.
@version 0.1 (June 2019) : Initial version [ADK]
----------------------------------------------------------------------------
*/

#include "tidl_alg_int.h"
#include "tidl_pad_ref.h"
#include "tidl_commonUtils.h"

using namespace floating_point::bf16_c7x;

/**
 * @brief This is reference implementation of Pad layer
 *
 * @tparam Tin : template for input data buffers
 * @tparam Tout : template for output data buffers
 * @param pIn : Pointer to input memory
 * @param pOut : Pointer to output memory
 * @param inPtrOffset : offset input buffer pointer
 * @param outPtrOffset : offset output buffer pointer
 * @param outWidth : width of the output buffer
 * @param outHeight : height of the output buffer
 * @param numChs : no of total channels
 * @param numROIs : no of total ROI
 * @param inLinePitch : pitch of the input buffer
 * @param outLinePitch : pitch of the output buffer
 * @param inChPitch : Chnnel pitch of the input buffer
 * @param outChPitch : Chnnel pitch of the output buffer
 * @param inROIPitch : ROI pitch of the input buffer
 * @param outROIPitch : ROI pitch of the output buffer
 * @param padT : Pad amount on the top side
 * @param padB : Pad amount on the bottom side
 * @param padL : Pad amount on the left side
 * @param padR : Pad amount on the right side
 * @param padValue : Pad value to be filled
 * @param padType : type of Padding to be done
 * @param padPerChannelPtr : pointer to per channel buffer
 * @param tensorScaleInput : Scale amount for the input
 */
template <class Tin, class Tout>
void TIDL_padRefProcess(
  const Tin*    pIn,
  Tout*   pOut,
  int32_t inPtrOffset,
  int32_t outPtrOffset,
  const sTIDL_DataParams_t *inDataParams,
  const sTIDL_DataParams_t *outDataParams,
  sTIDL_PadLayerParams_t *params,
  int32_t padValue,
  float32_tidl * padPerChannelPtr
  )
{
  int32_t i0, i1, i2, i3, i4, i5;

  int32_t outWidth           = outDataParams->dimValues[TIDL_DIM_WIDTH];
  int32_t inWidth            = inDataParams->dimValues[TIDL_DIM_WIDTH];
  int32_t inHeight           = inDataParams->dimValues[TIDL_DIM_HEIGHT];
  int32_t outHeight          = outDataParams->dimValues[TIDL_DIM_HEIGHT];
  int32_t numChs             = inDataParams->dimValues[TIDL_DIM_NUMCH];
  int32_t outDataDim1        = outDataParams->dimValues[TIDL_DIM_DIM1];
  int32_t outDataDim2        = outDataParams->dimValues[TIDL_DIM_DIM2];
  int32_t numROIs            = inDataParams->dimValues[TIDL_DIM_BATCH]; /*ROI  */
  int32_t inLinePitch        = (int32_t)inDataParams->pitch[TIDL_LINE_PITCH];
  int32_t outLinePitch       = (int32_t)outDataParams->pitch[TIDL_LINE_PITCH];
  int32_t inDim1Pitch        = (int32_t)inDataParams->pitch[TIDL_DIM1_PITCH];
  int32_t inDim2Pitch        = (int32_t)inDataParams->pitch[TIDL_DIM2_PITCH];
  int32_t outDim1Pitch       = (int32_t)outDataParams->pitch[TIDL_DIM1_PITCH];
  int32_t outDim2Pitch       = (int32_t)outDataParams->pitch[TIDL_DIM2_PITCH];
  int32_t inChPitch          = (int32_t)inDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t outChPitch         = (int32_t)outDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t inROIPitch         = (int32_t)inDataParams->pitch[TIDL_ROI_PITCH];
  int32_t outROIPitch        = (int32_t)outDataParams->pitch[TIDL_ROI_PITCH];

  float32_tidl tensorScaleInput = inDataParams->tensorScale;

  int32_t padT  =  params->padT;
  int32_t padB  =  params->padB;
  int32_t padL  =  params->padL;
  int32_t padR  =  params->padR;
  int32_t padType = params->padType;

  Tin padValueFinal;
  padValueFinal = padValue;

  /* OPENACC(data copyin(padPerChannelPtr[0:numChs], pIn[0: 1 + inPtrOffset  + (numROIs*inROIPitch)   + (numChs* inChPitch)   + ((outHeight - padT) * inLinePitch)   + (outWidth - padL)]) \
               copyout(pOut[0: 1 + outPtrOffset + (numROIs*outROIPitch)  + (numChs* outChPitch)  + (outHeight * outLinePitch)  + outWidth])) */
  // OPENACC(parallel loop collapse(2))
  for (i0 = 0; i0 < numROIs; i0++)
  {
    for (i1 = 0; i1 < outDataDim1; i1++)
    {
      for (i2 = 0; i2 < outDataDim2; i2++)
      {
        for (i3 = 0; i3 < numChs; i3++)
        {
          if(padType == (int32_t)TIDL_PadPerChannel)
          {
            padValueFinal = padPerChannelPtr[i3] * tensorScaleInput;
          }
          //OPENACC(loop collapse(2))
          for (i4 = 0; i4 < outHeight; i4++)
          {
            for (i5 = 0; i5 < outWidth; i5++)
            {
              int32_t outOffset = outPtrOffset + (i0*outROIPitch)  + (i1* outDim1Pitch)  + (i2 * outDim2Pitch) + (i3* outChPitch)  + (i4 * outLinePitch)  + i5;

              if (padType == TIDL_PadWrapAround)
              {
                if ((i5 < padL) || (i5 >= (outWidth - padR)))
                {
                  pOut[outOffset] = padValueFinal;
                }
                else if (i4 < padT)
                {
                  int32_t inOffset  = inPtrOffset  + (i0*inROIPitch)   + (i1*inDim1Pitch) + (i2*inDim2Pitch) + (i3* inChPitch)   + ((inHeight - padT + i4) * inLinePitch)   + (i5 - padL);
                  pOut[outOffset] = pIn[inOffset];              
                }
                else if (i4 < (outHeight - padB))
                {
                  int32_t inOffset  = inPtrOffset  + (i0*inROIPitch)   + (i1*inDim1Pitch) + (i2*inDim2Pitch) +  (i3* inChPitch)   + ((i4 - padT) * inLinePitch)   + (i5 - padL);
                  pOut[outOffset] = pIn[inOffset];               
                }
                else 
                {
                  int32_t inOffset  = inPtrOffset  + (i0*inROIPitch)   + (i1*inDim1Pitch) + (i2*inDim2Pitch) +  (i3* inChPitch)   + ((i4 - (outHeight - padB)) * inLinePitch)   + (i5 - padL);
                  pOut[outOffset] = pIn[inOffset];
                }
              }
              else
              {
                if((i5 < padL) || (i5 >= (outWidth - padR)) || (i4 < padT) || (i4 >= (outHeight - padB)))
                {
                  pOut[outOffset] = padValueFinal;
                }
                else
                {
                  int32_t inOffset  = inPtrOffset  + (i0*inROIPitch) + (i1*inDim1Pitch) + (i2*inDim2Pitch) + (i3* inChPitch)   + ((i4 - padT) * inLinePitch)   + (i5 - padL);
                  pOut[outOffset] = pIn[inOffset];
                }
              }
            }
          }
        }
      }
    }
  }
  TIDL_L1DandL2CacheWbInv();
}

/**
 * @brief This This function is only used to do non zero constant padding
 *        in NATC mode, tensor data is copied to output using DMA
 *
 * @tparam Tin : template for input data buffers
 * @tparam Tout : template for output data buffers
 * @param pIn : Pointer to input memory
 * @param pOut : Pointer to output memory
 * @param inPtrOffset : offset input buffer pointer
 * @param outPtrOffset : offset output buffer pointer
 * @param outWidth : width of the output buffer
 * @param outHeight : height of the output buffer
 * @param numChs : no of total channels
 * @param numROIs : no of total ROI
 * @param inLinePitch : pitch of the input buffer
 * @param outLinePitch : pitch of the output buffer
 * @param inChPitch : Chnnel pitch of the input buffer
 * @param outChPitch : Chnnel pitch of the output buffer
 * @param inROIPitch : ROI pitch of the input buffer
 * @param outROIPitch : ROI pitch of the output buffer
 * @param padT : Pad amount on the top side
 * @param padB : Pad amount on the bottom side
 * @param padL : Pad amount on the left side
 * @param padR : Pad amount on the right side
 * @param padValue : Pad value to be filled
 * @param padType : type of Padding to be done
 * @param padPerChannelPtr : pointer to per channel buffer
 * @param tensorScaleInput : Scale amount for the input
 */
template <class Tin, class Tout>
void TIDL_padNATCProcess(
  const Tin*    pIn,
  Tout*   pOut,
  int32_t inPtrOffset,
  int32_t outPtrOffset,
  const sTIDL_DataParams_t *inDataParams,
  const sTIDL_DataParams_t *outDataParams,
  sTIDL_PadLayerParams_t *params,
  int32_t padValue,
  float32_tidl * padPerChannelPtr
  )
{
  int32_t i0, i1, i2, i3, i4, i5;

  int32_t outWidth           = outDataParams->dimValues[TIDL_DIM_WIDTH];
  int32_t inWidth            = inDataParams->dimValues[TIDL_DIM_WIDTH];
  int32_t inHeight           = inDataParams->dimValues[TIDL_DIM_HEIGHT];
  int32_t outHeight          = outDataParams->dimValues[TIDL_DIM_HEIGHT];
  int32_t numChs             = inDataParams->dimValues[TIDL_DIM_NUMCH];
  int32_t outDataDim1        = outDataParams->dimValues[TIDL_DIM_DIM1];
  int32_t outDataDim2        = outDataParams->dimValues[TIDL_DIM_DIM2];
  int32_t numROIs            = inDataParams->dimValues[TIDL_DIM_BATCH]; /*ROI  */
  int32_t inLinePitch        = (int32_t)inDataParams->pitch[TIDL_LINE_PITCH];
  int32_t outLinePitch       = (int32_t)outDataParams->pitch[TIDL_LINE_PITCH];
  int32_t inDim1Pitch        = (int32_t)inDataParams->pitch[TIDL_DIM1_PITCH];
  int32_t inDim2Pitch        = (int32_t)inDataParams->pitch[TIDL_DIM2_PITCH];
  int32_t outDim1Pitch       = (int32_t)outDataParams->pitch[TIDL_DIM1_PITCH];
  int32_t outDim2Pitch       = (int32_t)outDataParams->pitch[TIDL_DIM2_PITCH];
  int32_t inChPitch          = (int32_t)inDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t outChPitch         = (int32_t)outDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t inROIPitch         = (int32_t)inDataParams->pitch[TIDL_ROI_PITCH];
  int32_t outROIPitch        = (int32_t)outDataParams->pitch[TIDL_ROI_PITCH];
  
  float32_tidl tensorScaleInput = inDataParams->tensorScale;

  int32_t padT  =  params->padT;
  int32_t padB  =  params->padB;
  int32_t padL  =  params->padL;
  int32_t padR  =  params->padR;
  int32_t padType = params->padType;

  Tin padValueFinal;
  padValueFinal = padValue;

  /* OPENACC(data copyin(padPerChannelPtr[0:numChs]) \
               copyout(pOut[0:1 + outPtrOffset + (numROIs*outROIPitch)  + (numChs* outChPitch)  + (outHeight * outLinePitch)  + outWidth])) */
  // OPENACC(parallel loop collapse(2))
  for (i0 = 0; i0 < numROIs; i0++)
  {
    for (i1 = 0; i1 < outDataDim1; i1++)
    {
      for (i2 = 0; i2 < outDataDim2; i2++)
      {
        for (i3 = 0; i3 < numChs; i3++)
        {
          /* LDRA_JUSTIFY_START
          <metric start> statement branch <metric end>
          <justification start>
          Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
          This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
          TIDL_LDRA_TAG : TIDL_LDRA_TAG_PAD_PRIOR_CHECK_001
          Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
          This does not affect runtime behavior or safety.
          <justification end> */
          if(padType == (int32_t)TIDL_PadPerChannel)
          /* LDRA_JUSTIFY_END */
          {
            padValueFinal = padPerChannelPtr[i3] * tensorScaleInput;
          }
          //OPENACC(loop collapse(2))
          for (i4 = 0; i4 < outHeight; i4++)
          {
            for (i5 = 0; i5 < outWidth; i5++)
            {
              int32_t outOffset = outPtrOffset + (i0*outROIPitch)  + (i1* outDim1Pitch)  + (i2 * outDim2Pitch) + (i3* outChPitch)  + (i4 * outLinePitch)  + i5;

              if (padType == TIDL_PadWrapAround)
              {
                if ((i5 < padL) || (i5 >= (outWidth - padR)))
                {
                  pOut[outOffset] = padValueFinal;
                }
                else if (i4 < padT)
                {
                  int32_t inOffset  = inPtrOffset  + (i0*inROIPitch)   + (i1*inDim1Pitch) + (i2*inDim2Pitch) + (i3* inChPitch)   + ((inHeight - padT + i4) * inLinePitch)   + (i5 - padL);
                  pOut[outOffset] = pIn[inOffset];              
                }
                else if (i4 < (outHeight - padB))
                {
                  int32_t inOffset  = inPtrOffset  + (i0*inROIPitch)   + (i1*inDim1Pitch) + (i2*inDim2Pitch) +  (i3* inChPitch)   + ((i4 - padT) * inLinePitch)   + (i5 - padL);
                  pOut[outOffset] = pIn[inOffset];               
                }
                else 
                {
                  int32_t inOffset  = inPtrOffset  + (i0*inROIPitch)   + (i1*inDim1Pitch) + (i2*inDim2Pitch) +  (i3* inChPitch)   + ((i4 - (outHeight - padB)) * inLinePitch)   + (i5 - padL);
                  pOut[outOffset] = pIn[inOffset];
                }
              }
              else
              {
                if((i5 < padL) || (i5 >= (outWidth - padR)) || (i4 < padT) || (i4 >= (outHeight - padB)))
                {
                  pOut[outOffset] = padValueFinal;
                }
                else
                {
                  int32_t inOffset  = inPtrOffset  + (i0*inROIPitch) + (i1*inDim1Pitch) + (i2*inDim2Pitch) + (i3* inChPitch)   + ((i4 - padT) * inLinePitch)   + (i5 - padL);
                  pOut[outOffset] = pIn[inOffset];
                }
              }
            }
          }
        }
      }
    }
  }
}



template void TIDL_padRefProcess(const float32_tidl*    pIn, float32_tidl*   pOut, int32_t inPtrOffset, int32_t outPtrOffset,
  const sTIDL_DataParams_t *inDataParams,
  const sTIDL_DataParams_t *outDataParams,            
  sTIDL_PadLayerParams_t *params,
  int32_t padValue,
  float32_tidl * padPerChannelPtr
  );

template void TIDL_padRefProcess(const bfloat16_tidl*    pIn, bfloat16_tidl*   pOut, int32_t inPtrOffset, int32_t outPtrOffset,
  const sTIDL_DataParams_t *inDataParams,
  const sTIDL_DataParams_t *outDataParams,            
  sTIDL_PadLayerParams_t *params,
  int32_t padValue,
  float32_tidl * padPerChannelPtr
  );

template void TIDL_padRefProcess(const int8_t*    pIn, int8_t*   pOut, int32_t inPtrOffset, int32_t outPtrOffset,
  const sTIDL_DataParams_t *inDataParams,
  const sTIDL_DataParams_t *outDataParams,            
  sTIDL_PadLayerParams_t *params,
  int32_t padValue,
  float32_tidl * padPerChannelPtr
  );

template void TIDL_padRefProcess(const uint8_t*    pIn, uint8_t*   pOut, int32_t inPtrOffset, int32_t outPtrOffset,
  const sTIDL_DataParams_t *inDataParams,
  const sTIDL_DataParams_t *outDataParams,            
  sTIDL_PadLayerParams_t *params,
  int32_t padValue,
  float32_tidl * padPerChannelPtr
  );

template void TIDL_padRefProcess(const int16_t*    pIn, int16_t*   pOut, int32_t inPtrOffset, int32_t outPtrOffset,
  const sTIDL_DataParams_t *inDataParams,
  const sTIDL_DataParams_t *outDataParams,            
  sTIDL_PadLayerParams_t *params,
  int32_t padValue,
  float32_tidl * padPerChannelPtr
  );

template void TIDL_padRefProcess(const uint16_t*    pIn, uint16_t*   pOut, int32_t inPtrOffset, int32_t outPtrOffset,
  const sTIDL_DataParams_t *inDataParams,
  const sTIDL_DataParams_t *outDataParams,            
  sTIDL_PadLayerParams_t *params,
  int32_t padValue,
  float32_tidl * padPerChannelPtr
  );

template void TIDL_padNATCProcess(const float32_tidl*    pIn, float32_tidl*   pOut, int32_t inPtrOffset, int32_t outPtrOffset,
  const sTIDL_DataParams_t *inDataParams,
  const sTIDL_DataParams_t *outDataParams,            
  sTIDL_PadLayerParams_t *params,
  int32_t padValue,
  float32_tidl * padPerChannelPtr
  );

template void TIDL_padNATCProcess(const int8_t*    pIn, int8_t*   pOut, int32_t inPtrOffset, int32_t outPtrOffset,
  const sTIDL_DataParams_t *inDataParams,
  const sTIDL_DataParams_t *outDataParams,            
  sTIDL_PadLayerParams_t *params,
  int32_t padValue,
  float32_tidl * padPerChannelPtr
  );

template void TIDL_padNATCProcess(const uint8_t*    pIn, uint8_t*   pOut, int32_t inPtrOffset, int32_t outPtrOffset,
  const sTIDL_DataParams_t *inDataParams,
  const sTIDL_DataParams_t *outDataParams,            
  sTIDL_PadLayerParams_t *params,
  int32_t padValue,
  float32_tidl * padPerChannelPtr
  );

template void TIDL_padNATCProcess(const int16_t*    pIn, int16_t*   pOut, int32_t inPtrOffset, int32_t outPtrOffset,
  const sTIDL_DataParams_t *inDataParams,
  const sTIDL_DataParams_t *outDataParams,            
  sTIDL_PadLayerParams_t *params,
  int32_t padValue,
  float32_tidl * padPerChannelPtr
  );

template void TIDL_padNATCProcess(const uint16_t*    pIn, uint16_t*   pOut, int32_t inPtrOffset, int32_t outPtrOffset,
  const sTIDL_DataParams_t *inDataParams,
  const sTIDL_DataParams_t *outDataParams,            
  sTIDL_PadLayerParams_t *params,
  int32_t padValue,
  float32_tidl * padPerChannelPtr
  );



