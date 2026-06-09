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
 *  \file tidl_colorConversion_ref.c
 *
 *  \brief This file defines kernel functions for color conversion layer
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include "tidl_alg_utils.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

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

template<class T> static void TIDL_refUV420Conv(const T *pIn,
                                                T *pOut,
                                                sTIDL_DataParams_t *inDataParams,
                                                sTIDL_DataParams_t *outDataParams);

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */
/**
 * @brief ColorConversion refernece implementation
 *
 * @tparam T  : template for input
 * @param pIn : Pointer to the input data
 * @param pOut : Pointer to the output data
 * @param inDataParams : pointer to input data parameters
 * @param outDataParams : pointer to output data parameters
 */
template<class T> static void TIDL_refUV420Conv(const T *pIn,
                                                T *pOut,
                                                sTIDL_DataParams_t *inDataParams,
                                                sTIDL_DataParams_t *outDataParams)
{
  int32_t i0, i1, i2;

  int32_t inWidth = inDataParams->dimValues[TIDL_DIM_WIDTH];
  int32_t inHeight = inDataParams->dimValues[TIDL_DIM_HEIGHT];
  int32_t inLinePitch = inDataParams->pitch[TIDL_LINE_PITCH];
  int32_t outLinePitch = outDataParams->pitch[TIDL_LINE_PITCH];
  int32_t outChPitch = outDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t numTotBatches = (int32_t)inDataParams->dimValues[TIDL_DIM_BATCH];
  uint32_t inBatchPitch = (uint32_t)inDataParams->pitch[TIDL_ROI_PITCH];
  uint32_t outBatchPitch = (uint32_t)outDataParams->pitch[TIDL_ROI_PITCH];

  for (i0 = 0; i0 < numTotBatches; i0++)
  {
    for (i1 = 0; i1 < inHeight; i1++)
    {
      for (i2 = 0; i2 < inWidth; i2++)
      {
        int32_t inOffset = (i0 * inBatchPitch) + (i1 * inLinePitch) + i2;
        int32_t outChOffset = (0 == (inOffset % 2)) ? 0 : outChPitch;
        int32_t outOffset1 = outChOffset + (i0 * outBatchPitch) + (outLinePitch * (2 * i1)) + (2 * (i2 / 2));
        int32_t outOffset2 = outChOffset + (i0 * outBatchPitch) + (outLinePitch * (2 * i1)) + (2 * (i2 / 2)) + 1;
        int32_t outOffset3 = outChOffset + (i0 * outBatchPitch) + (outLinePitch * ((2 * i1) + 1)) + (2 * (i2 / 2));
        int32_t outOffset4 = outChOffset + (i0 * outBatchPitch) + (outLinePitch * ((2 * i1) + 1)) + (2 * (i2 / 2)) + 1;

        pOut[outOffset1] = pIn[inOffset];
        pOut[outOffset2] = pIn[inOffset];
        pOut[outOffset3] = pIn[inOffset];
        pOut[outOffset4] = pIn[inOffset];
      }
    }
  }
}
/**
 * @brief ColorConversion refernece implementation for different bit-depths
 *
 * @param pIn : Pointer to the input data
 * @param pOut : Pointer to the output data
 * @param inDataParams : pointer to input data parameters
 * @param outDataParams : pointer to output data parameters
 * @param elementType : Element type of the input data
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
int32_t TIDL_colorConversionRefProcess(void *pIn,
                                       void *pOut,
                                       sTIDL_DataParams_t *inDataParams,
                                       sTIDL_DataParams_t *outDataParams,
                                       int32_t elementType)
{
  int32_t status = TIDL_SUCCESS;

  if (elementType == TIDL_SinglePrecFloat)
  {
    TIDL_refUV420Conv((float32_tidl *)pIn, (float32_tidl *)pOut, inDataParams, outDataParams);
  }
  else if (elementType == TIDL_UnsignedChar)
  {
    TIDL_refUV420Conv((uint8_t *)pIn, (uint8_t *)pOut, inDataParams, outDataParams);
  }
  else if (elementType == TIDL_UnsignedShort)
  {
    TIDL_refUV420Conv((uint16_t *)pIn, (uint16_t *)pOut, inDataParams, outDataParams);
  }
  else
  {
    tidl_printf(0, " Unsupported elementType for for UV420 Conversion, Exiting from %s, %d\n", __FILE__, __LINE__);
    status = TIDL_ERR_INVALID_TYPE;
  }
  if (status == TIDL_SUCCESS)
  {
    TIDL_L1DandL2CacheWbInv();
  }
  return status;
}

/* ========================================================================== */
/*                          Function Definitions                             */
/* ========================================================================== */
