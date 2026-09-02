/*
 *
 * Copyright (c) {2015 - 2026} Texas Instruments Incorporated
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
 */

/**
----------------------------------------------------------------------------
@file    tidl_cast_ref.c
@brief   This file defines private functions for cast layer
----------------------------------------------------------------------------
*/

#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include "tidl_cast_ref.h"
#include "tidl_device_utils.h"


/**
 * @brief Reference implementation of Cast layer 
 *
 * @tparam Tin : template for input data buffers
 * @tparam Tout : template for output data buffers
 * @param pIn : Pointer to input memory
 * @param pOut : Pointer to output memory
 * @param inDataParams : Pointer to input data parameters
 * @param tidlLayer : Pointer to layer struct with dimValues and pitch
 */
template<class Tin, class Tout> void TIDL_castRef(const Tin *pIn,
                                                 Tout *pOut,
                                                 sTIDL_DataParams_t *inDataParams,
                                                 sTIDL_Layer_t *tidlLayer)
{

    int32_t batch = tidlLayer->outData.dimValues[TIDL_DIM_BATCH];
    int32_t numCh = tidlLayer->outData.dimValues[TIDL_DIM_NUMCH];
    int32_t height = tidlLayer->outData.dimValues[TIDL_DIM_HEIGHT];
    int32_t width = tidlLayer->outData.dimValues[TIDL_DIM_WIDTH];
    int32_t dim1 = tidlLayer->outData.dimValues[TIDL_DIM_DIM1];
    int32_t dim2 = tidlLayer->outData.dimValues[TIDL_DIM_DIM2];

    int32_t inRoiPitch = inDataParams->pitch[TIDL_ROI_PITCH];
    int32_t inChPitch = inDataParams->pitch[TIDL_CHANNEL_PITCH];
    int32_t inLinePitch = inDataParams->pitch[TIDL_LINE_PITCH];
    int32_t inDim1Pitch = inDataParams->pitch[TIDL_DIM1_PITCH];
    int32_t inDim2Pitch = inDataParams->pitch[TIDL_DIM2_PITCH];

    int32_t outRoiPitch = tidlLayer->outData.pitch[TIDL_ROI_PITCH];
    int32_t outChPitch = tidlLayer->outData.pitch[TIDL_CHANNEL_PITCH];
    int32_t outLinePitch = tidlLayer->outData.pitch[TIDL_LINE_PITCH];
    int32_t outDim1Pitch = tidlLayer->outData.pitch[TIDL_DIM1_PITCH];
    int32_t outDim2Pitch = tidlLayer->outData.pitch[TIDL_DIM2_PITCH];

    for (int32_t b = 0; b < batch; b++) {
      for (int32_t d1 = 0; d1 < dim1; d1++) {
        for (int32_t d2 = 0; d2 < dim2; d2++) {
          for (int32_t c = 0; c < numCh; c++) {
            for (int32_t h = 0; h < height; h++) {
              for (int32_t w = 0; w < width; w++) {
                int32_t inOffset = b * inRoiPitch + c * inChPitch + h * inLinePitch + w + d1 * inDim1Pitch + d2 * inDim2Pitch;
                int32_t outOffset = b * outRoiPitch + c * outChPitch + h * outLinePitch + w + d1 * outDim1Pitch + d2 * outDim2Pitch;
                pOut[outOffset] = static_cast<Tout>(pIn[inOffset]);
              }
            }
          }
        }
      }
    }
    TIDL_L1DandL2CacheWbInv();
}

int32_t TIDL_castRefProcess(sTIDL_DataParams_t *inDataParams,
                            sTIDL_Layer_t *tidlLayer,
                            void *inPtrs[],
                            void *outPtrs[])
{
  int32_t status = IALG_EOK;
  int inType = inDataParams->elementType;
  int outType = tidlLayer->outData.elementType;

  if (inType == TIDL_SignedChar) {
    if (outType == TIDL_SignedChar) {
      TIDL_castRef((const int8_t *)inPtrs[0], (int8_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedChar) {
      TIDL_castRef((const int8_t *)inPtrs[0], (uint8_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedShort) {
      TIDL_castRef((const int8_t *)inPtrs[0], (int16_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedShort) {
      TIDL_castRef((const int8_t *)inPtrs[0], (uint16_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SinglePrecFloat) {
      TIDL_castRef((const int8_t *)inPtrs[0], (float32_tidl *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedWord) {
      TIDL_castRef((const int8_t *)inPtrs[0], (int32_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedWord) {
      TIDL_castRef((const int8_t *)inPtrs[0], (uint32_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedDoubleWord) {
      TIDL_castRef((const int8_t *)inPtrs[0], (int64_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedDoubleWord) {
      TIDL_castRef((const int8_t *)inPtrs[0], (uint64_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_Bool) {
      TIDL_castRef((const int8_t *)inPtrs[0], (bool *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_BFloat16) {
      TIDL_castRef((const int8_t *)inPtrs[0], (bfloat16_tidl *)outPtrs[0], inDataParams, tidlLayer);
    } else {
      tidl_printf(0,"Unsupported output elementType in %s File, %d Line \n  ", __FILE__, __LINE__);
      status = IALG_EFAIL;
    }
  } else if (inType == TIDL_UnsignedChar) {
    if (outType == TIDL_SignedChar) {
      TIDL_castRef((const uint8_t *)inPtrs[0], (int8_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedChar) {
      TIDL_castRef((const uint8_t *)inPtrs[0], (uint8_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedShort) {
      TIDL_castRef((const uint8_t *)inPtrs[0], (int16_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedShort) {
      TIDL_castRef((const uint8_t *)inPtrs[0], (uint16_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SinglePrecFloat) {
      TIDL_castRef((const uint8_t *)inPtrs[0], (float32_tidl *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedWord) {
      TIDL_castRef((const uint8_t *)inPtrs[0], (int32_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedWord) {
      TIDL_castRef((const uint8_t *)inPtrs[0], (uint32_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedDoubleWord) {
      TIDL_castRef((const uint8_t *)inPtrs[0], (int64_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedDoubleWord) {
      TIDL_castRef((const uint8_t *)inPtrs[0], (uint64_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_Bool) {
      TIDL_castRef((const uint8_t *)inPtrs[0], (bool *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_BFloat16) {
      TIDL_castRef((const uint8_t *)inPtrs[0], (bfloat16_tidl *)outPtrs[0], inDataParams, tidlLayer);
    } else {
      tidl_printf(0,"Unsupported output elementType in %s File, %d Line \n  ", __FILE__, __LINE__);
      status = IALG_EFAIL;
    }
  } else if (inType == TIDL_SignedShort) {
    if (outType == TIDL_SignedChar) {
      TIDL_castRef((const int16_t *)inPtrs[0], (int8_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedChar) {
      TIDL_castRef((const int16_t *)inPtrs[0], (uint8_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedShort) {
      TIDL_castRef((const int16_t *)inPtrs[0], (int16_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedShort) {
      TIDL_castRef((const int16_t *)inPtrs[0], (uint16_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SinglePrecFloat) {
      TIDL_castRef((const int16_t *)inPtrs[0], (float32_tidl *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedWord) {
      TIDL_castRef((const int16_t *)inPtrs[0], (int32_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedWord) {
      TIDL_castRef((const int16_t *)inPtrs[0], (uint32_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedDoubleWord) {
      TIDL_castRef((const int16_t *)inPtrs[0], (int64_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedDoubleWord) {
      TIDL_castRef((const int16_t *)inPtrs[0], (uint64_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_Bool) {
      TIDL_castRef((const int16_t *)inPtrs[0], (bool *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_BFloat16) {
      TIDL_castRef((const int16_t *)inPtrs[0], (bfloat16_tidl *)outPtrs[0], inDataParams, tidlLayer);
    } else {
      tidl_printf(0,"Unsupported output elementType in %s File, %d Line \n  ", __FILE__, __LINE__);
      status = IALG_EFAIL;
    }
  } else if (inType == TIDL_UnsignedShort) {
    if (outType == TIDL_SignedChar) {
      TIDL_castRef((const uint16_t *)inPtrs[0], (int8_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedChar) {
      TIDL_castRef((const uint16_t *)inPtrs[0], (uint8_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedShort) {
      TIDL_castRef((const uint16_t *)inPtrs[0], (int16_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedShort) {
      TIDL_castRef((const uint16_t *)inPtrs[0], (uint16_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SinglePrecFloat) {
      TIDL_castRef((const uint16_t *)inPtrs[0], (float32_tidl *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedWord) {
      TIDL_castRef((const uint16_t *)inPtrs[0], (int32_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedWord) {
      TIDL_castRef((const uint16_t *)inPtrs[0], (uint32_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedDoubleWord) {
      TIDL_castRef((const uint16_t *)inPtrs[0], (int64_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedDoubleWord) {
      TIDL_castRef((const uint16_t *)inPtrs[0], (uint64_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_Bool) {
      TIDL_castRef((const uint16_t *)inPtrs[0], (bool *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_BFloat16) {
      TIDL_castRef((const uint16_t *)inPtrs[0], (bfloat16_tidl *)outPtrs[0], inDataParams, tidlLayer);
    } else {
      tidl_printf(0,"Unsupported output elementType in %s File, %d Line \n  ", __FILE__, __LINE__);
      status = IALG_EFAIL;
    }
  } else if (inType == TIDL_SinglePrecFloat) {
    if (outType == TIDL_SignedChar) {
      TIDL_castRef((const float32_tidl *)inPtrs[0], (int8_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedChar) {
      TIDL_castRef((const float32_tidl *)inPtrs[0], (uint8_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedShort) {
      TIDL_castRef((const float32_tidl *)inPtrs[0], (int16_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedShort) {
      TIDL_castRef((const float32_tidl *)inPtrs[0], (uint16_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SinglePrecFloat) {
      TIDL_castRef((const float32_tidl *)inPtrs[0], (float32_tidl *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedWord) {
      TIDL_castRef((const float32_tidl *)inPtrs[0], (int32_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedWord) {
      TIDL_castRef((const float32_tidl *)inPtrs[0], (uint32_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedDoubleWord) {
      TIDL_castRef((const float32_tidl *)inPtrs[0], (int64_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedDoubleWord) {
      TIDL_castRef((const float32_tidl *)inPtrs[0], (uint64_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_Bool) {
      TIDL_castRef((const float32_tidl *)inPtrs[0], (bool *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_BFloat16) {
      TIDL_castRef((const float32_tidl *)inPtrs[0], (bfloat16_tidl *)outPtrs[0], inDataParams, tidlLayer);
    } else {
      tidl_printf(0,"Unsupported output elementType in %s File, %d Line \n  ", __FILE__, __LINE__);
      status = IALG_EFAIL;
    }
  } else if (inType == TIDL_SignedWord) {
    if (outType == TIDL_SignedChar) {
      TIDL_castRef((const int32_t *)inPtrs[0], (int8_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedChar) {
      TIDL_castRef((const int32_t *)inPtrs[0], (uint8_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedShort) {
      TIDL_castRef((const int32_t *)inPtrs[0], (int16_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedShort) {
      TIDL_castRef((const int32_t *)inPtrs[0], (uint16_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SinglePrecFloat) {
      TIDL_castRef((const int32_t *)inPtrs[0], (float32_tidl *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedWord) {
      TIDL_castRef((const int32_t *)inPtrs[0], (int32_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedWord) {
      TIDL_castRef((const int32_t *)inPtrs[0], (uint32_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedDoubleWord) {
      TIDL_castRef((const int32_t *)inPtrs[0], (int64_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedDoubleWord) {
      TIDL_castRef((const int32_t *)inPtrs[0], (uint64_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_Bool) {
      TIDL_castRef((const int32_t *)inPtrs[0], (bool *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_BFloat16) {
      TIDL_castRef((const int32_t *)inPtrs[0], (bfloat16_tidl *)outPtrs[0], inDataParams, tidlLayer);
    } else {
      tidl_printf(0,"Unsupported output elementType in %s File, %d Line \n  ", __FILE__, __LINE__);
      status = IALG_EFAIL;
    }
  } else if (inType == TIDL_UnsignedWord) {
    if (outType == TIDL_SignedChar) {
      TIDL_castRef((const uint32_t *)inPtrs[0], (int8_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedChar) {
      TIDL_castRef((const uint32_t *)inPtrs[0], (uint8_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedShort) {
      TIDL_castRef((const uint32_t *)inPtrs[0], (int16_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedShort) {
      TIDL_castRef((const uint32_t *)inPtrs[0], (uint16_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SinglePrecFloat) {
      TIDL_castRef((const uint32_t *)inPtrs[0], (float32_tidl *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedWord) {
      TIDL_castRef((const uint32_t *)inPtrs[0], (int32_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedWord) {
      TIDL_castRef((const uint32_t *)inPtrs[0], (uint32_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedDoubleWord) {
      TIDL_castRef((const uint32_t *)inPtrs[0], (int64_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedDoubleWord) {
      TIDL_castRef((const uint32_t *)inPtrs[0], (uint64_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_Bool) {
      TIDL_castRef((const uint32_t *)inPtrs[0], (bool *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_BFloat16) {
      TIDL_castRef((const uint32_t *)inPtrs[0], (bfloat16_tidl *)outPtrs[0], inDataParams, tidlLayer);
    } else {
      tidl_printf(0,"Unsupported output elementType in %s File, %d Line \n  ", __FILE__, __LINE__);
      status = IALG_EFAIL;
    }
  } else if (inType == TIDL_SignedDoubleWord) {
    if (outType == TIDL_SignedChar) {
      TIDL_castRef((const int64_t *)inPtrs[0], (int8_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedChar) {
      TIDL_castRef((const int64_t *)inPtrs[0], (uint8_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedShort) {
      TIDL_castRef((const int64_t *)inPtrs[0], (int16_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedShort) {
      TIDL_castRef((const int64_t *)inPtrs[0], (uint16_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SinglePrecFloat) {
      TIDL_castRef((const int64_t *)inPtrs[0], (float32_tidl *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedWord) {
      TIDL_castRef((const int64_t *)inPtrs[0], (int32_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedWord) {
      TIDL_castRef((const int64_t *)inPtrs[0], (uint32_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedDoubleWord) {
      TIDL_castRef((const int64_t *)inPtrs[0], (int64_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedDoubleWord) {
      TIDL_castRef((const int64_t *)inPtrs[0], (uint64_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_Bool) {
      TIDL_castRef((const int64_t *)inPtrs[0], (bool *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_BFloat16) {
      TIDL_castRef((const int64_t *)inPtrs[0], (bfloat16_tidl *)outPtrs[0], inDataParams, tidlLayer);
    } else {
      tidl_printf(0,"Unsupported output elementType in %s File, %d Line \n  ", __FILE__, __LINE__);
      status = IALG_EFAIL;
    }
  } else if (inType == TIDL_UnsignedDoubleWord) {
    if (outType == TIDL_SignedChar) {
      TIDL_castRef((const uint64_t *)inPtrs[0], (int8_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedChar) {
      TIDL_castRef((const uint64_t *)inPtrs[0], (uint8_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedShort) {
      TIDL_castRef((const uint64_t *)inPtrs[0], (int16_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedShort) {
      TIDL_castRef((const uint64_t *)inPtrs[0], (uint16_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SinglePrecFloat) {
      TIDL_castRef((const uint64_t *)inPtrs[0], (float32_tidl *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedWord) {
      TIDL_castRef((const uint64_t *)inPtrs[0], (int32_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedWord) {
      TIDL_castRef((const uint64_t *)inPtrs[0], (uint32_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedDoubleWord) {
      TIDL_castRef((const uint64_t *)inPtrs[0], (int64_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedDoubleWord) {
      TIDL_castRef((const uint64_t *)inPtrs[0], (uint64_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_Bool) {
      TIDL_castRef((const uint64_t *)inPtrs[0], (bool *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_BFloat16) {
      TIDL_castRef((const uint64_t *)inPtrs[0], (bfloat16_tidl *)outPtrs[0], inDataParams, tidlLayer);
    } else {
      tidl_printf(0,"Unsupported output elementType in %s File, %d Line \n  ", __FILE__, __LINE__);
      status = IALG_EFAIL;
    }
  } else if (inType == TIDL_Bool) {
    if (outType == TIDL_SignedChar) {
      TIDL_castRef((const bool *)inPtrs[0], (int8_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedChar) {
      TIDL_castRef((const bool *)inPtrs[0], (uint8_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedShort) {
      TIDL_castRef((const bool *)inPtrs[0], (int16_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedShort) {
      TIDL_castRef((const bool *)inPtrs[0], (uint16_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SinglePrecFloat) {
      TIDL_castRef((const bool *)inPtrs[0], (float32_tidl *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedWord) {
      TIDL_castRef((const bool *)inPtrs[0], (int32_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedWord) {
      TIDL_castRef((const bool *)inPtrs[0], (uint32_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedDoubleWord) {
      TIDL_castRef((const bool *)inPtrs[0], (int64_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedDoubleWord) {
      TIDL_castRef((const bool *)inPtrs[0], (uint64_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_Bool) {
      TIDL_castRef((const bool *)inPtrs[0], (bool *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_BFloat16) {
      TIDL_castRef((const bool *)inPtrs[0], (bfloat16_tidl *)outPtrs[0], inDataParams, tidlLayer);
    } else {
      tidl_printf(0,"Unsupported output elementType in %s File, %d Line \n  ", __FILE__, __LINE__);
      status = IALG_EFAIL;
    }
  } else if (inType == TIDL_BFloat16) {
    if (outType == TIDL_SignedChar) {
      TIDL_castRef((const bfloat16_tidl *)inPtrs[0], (int8_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedChar) {
      TIDL_castRef((const bfloat16_tidl *)inPtrs[0], (uint8_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedShort) {
      TIDL_castRef((const bfloat16_tidl *)inPtrs[0], (int16_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedShort) {
      TIDL_castRef((const bfloat16_tidl *)inPtrs[0], (uint16_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SinglePrecFloat) {
      TIDL_castRef((const bfloat16_tidl *)inPtrs[0], (float32_tidl *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedWord) {
      TIDL_castRef((const bfloat16_tidl *)inPtrs[0], (int32_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedWord) {
      TIDL_castRef((const bfloat16_tidl *)inPtrs[0], (uint32_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_SignedDoubleWord) {
      TIDL_castRef((const bfloat16_tidl *)inPtrs[0], (int64_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_UnsignedDoubleWord) {
      TIDL_castRef((const bfloat16_tidl *)inPtrs[0], (uint64_t *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_Bool) {
      TIDL_castRef((const bfloat16_tidl *)inPtrs[0], (bool *)outPtrs[0], inDataParams, tidlLayer);
    } else if (outType == TIDL_BFloat16) {
      TIDL_castRef((const bfloat16_tidl *)inPtrs[0], (bfloat16_tidl *)outPtrs[0], inDataParams, tidlLayer);
    } else {
      tidl_printf(0,"Unsupported output elementType in %s File, %d Line \n  ", __FILE__, __LINE__);
      status = IALG_EFAIL;
    }
  } else {
    tidl_printf(0,"Unsupported input elementType in %s File, %d Line \n  ", __FILE__, __LINE__);
    status = IALG_EFAIL;
  }

  return status;
}

// Template instantiations
template void TIDL_castRef(const signed char *pIn,
                          signed char *pOut,
                          sTIDL_DataParams_t *inDataParams,
                          sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const signed char *pIn,
                                 unsigned char *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const signed char *pIn,
                                 short *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const signed char *pIn,
                                 unsigned short *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const signed char *pIn,
                                 float *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const unsigned char *pIn,
                                 signed char *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const unsigned char *pIn,
                                 unsigned char *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const unsigned char *pIn,
                                 short *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const unsigned char *pIn,
                                 unsigned short *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const unsigned char *pIn,
                                 float *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const short *pIn,
                                 signed char *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const short *pIn,
                                 unsigned char *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const short *pIn,
                                 short *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const short *pIn,
                                 unsigned short *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const short *pIn,
                                 float *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const unsigned short *pIn,
                                 signed char *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const unsigned short *pIn,
                                 unsigned char *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const unsigned short *pIn,
                                 short *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const unsigned short *pIn,
                                 unsigned short *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const unsigned short *pIn,
                                 float *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const float *pIn,
                                 signed char *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const float *pIn,
                                 unsigned char *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const float *pIn,
                                 short *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const float *pIn,
                                 unsigned short *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const float *pIn,
                                 float *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const signed char *pIn,
                                 int32_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const signed char *pIn,
                                 uint32_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const signed char *pIn,
                                 int64_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const signed char *pIn,
                                 uint64_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const unsigned char *pIn,
                                 int32_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const unsigned char *pIn,
                                 uint32_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const unsigned char *pIn,
                                 int64_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const unsigned char *pIn,
                                 uint64_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const short *pIn,
                                 int32_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const short *pIn,
                                 uint32_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const short *pIn,
                                 int64_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const short *pIn,
                                 uint64_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const unsigned short *pIn,
                                 int32_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const unsigned short *pIn,
                                 uint32_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const unsigned short *pIn,
                                 int64_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const unsigned short *pIn,
                                 uint64_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const float *pIn,
                                 int32_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const float *pIn,
                                 uint32_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const float *pIn,
                                 int64_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const float *pIn,
                                 uint64_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const int32_t *pIn,
                                 signed char *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const int32_t *pIn,
                                 unsigned char *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const int32_t *pIn,
                                 short *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const int32_t *pIn,
                                 unsigned short *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const int32_t *pIn,
                                 float *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const int32_t *pIn,
                                 int32_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const int32_t *pIn,
                                 uint32_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const int32_t *pIn,
                                 int64_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const int32_t *pIn,
                                 uint64_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const uint32_t *pIn,
                                 signed char *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const uint32_t *pIn,
                                 unsigned char *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const uint32_t *pIn,
                                 short *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const uint32_t *pIn,
                                 unsigned short *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const uint32_t *pIn,
                                 float *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const uint32_t *pIn,
                                 int32_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const uint32_t *pIn,
                                 uint32_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const uint32_t *pIn,
                                 int64_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const uint32_t *pIn,
                                 uint64_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const int64_t *pIn,
                                 signed char *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const int64_t *pIn,
                                 unsigned char *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const int64_t *pIn,
                                 short *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const int64_t *pIn,
                                 unsigned short *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const int64_t *pIn,
                                 float *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const int64_t *pIn,
                                 int32_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const int64_t *pIn,
                                 uint32_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const int64_t *pIn,
                                 int64_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const int64_t *pIn,
                                 uint64_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const uint64_t *pIn,
                                 signed char *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const uint64_t *pIn,
                                 unsigned char *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const uint64_t *pIn,
                                 short *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const uint64_t *pIn,
                                 unsigned short *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const uint64_t *pIn,
                                 float *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const uint64_t *pIn,
                                 int32_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const uint64_t *pIn,
                                 uint32_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const uint64_t *pIn,
                                 int64_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const uint64_t *pIn,
                                 uint64_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const bfloat16_tidl *pIn,
                                 signed char *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const bfloat16_tidl *pIn,
                                 unsigned char *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const bfloat16_tidl *pIn,
                                 short *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const bfloat16_tidl *pIn,
                                 unsigned short *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const bfloat16_tidl *pIn,
                                 float *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const bfloat16_tidl *pIn,
                                 int32_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const bfloat16_tidl *pIn,
                                 uint32_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const bfloat16_tidl *pIn,
                                 int64_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);

template void TIDL_castRef(const bfloat16_tidl *pIn,
                                 uint64_t *pOut,
                                 sTIDL_DataParams_t *inDataParams,
                                 sTIDL_Layer_t *tidlLayer);
