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
@file    tidl_depthToSpace_ref.c
@brief   This file defines private functions for depthToSpace layer.
@version 0.1 (Aug 2020) : Initial version [ADK]
----------------------------------------------------------------------------
*/

#if defined TIDL_COVERAGE_DEAD_CODE

#include "tidl_alg_int.h"
#include "tidl_depthToSpace_ref.h"
#include "tidl_commonUtils.h"

/**
 * @brief This is reference implementation of Crop layer
 *
 * @tparam Tin : template for input data buffers
 * @tparam Tout : template for output data buffers
 * @param  pIn : Pointer to input memory
 * @param  pOut : Pointer to output memory
 * @param inPtrOffset  : offset for the input buffer for crop
 * @param outPtrOffset : offset for the input buffer for crop
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
 * @param blockSize : blockSize/upsampling for D2S layer
 */
template<class Tin, class Tout> void TIDL_refDepthToSpace(
    const Tin *pIn,
    Tout *pOut,
    int32_t inPtrOffset,
    int32_t outPtrOffset,
    int32_t outWidth,
    int32_t outHeight,
    int32_t numChs,
    int32_t numROIs,
    int32_t inLinePitch,
    int32_t outLinePitch,
    int32_t inChPitch,
    int32_t outChPitch,
    int32_t inROIPitch,
    int32_t outROIPitch,
    int32_t blockSize)
{
  int32_t i0, i1, i2, i3, i4, i5;
  int32_t numOutChs = numChs / (blockSize * blockSize);
  for (i0 = 0; i0 < numROIs; i0++)
  {
    for (i1 = 0; i1 < numOutChs; i1++)
    {
      for (i2 = 0; i2 < (outHeight / blockSize); i2++)
      {
        for (i3 = 0; i3 < (outWidth / blockSize); i3++)
        {
          for (i4 = 0; i4 < blockSize; i4++)
          {
            for (i5 = 0; i5 < blockSize; i5++)
            {
              int32_t inOffset = inPtrOffset + (i0 * inROIPitch) + (i1 * inChPitch * blockSize * blockSize) + (i4 * blockSize * inChPitch) + +(i5 * inChPitch) + +(i2 * inLinePitch) + i3;
              int32_t outOffset = outPtrOffset + (i0 * outROIPitch) + (i1 * outChPitch) + (i2 * blockSize * outLinePitch) + (i3 * blockSize) + (i4 * outLinePitch) + i5;
              pOut[outOffset] = pIn[inOffset];
            }
          }
        }
      }
    }
  }
}

/**
 * @brief This is reference implementation of D2S layer
 *
 * @param  pIn : Pointer to input memory
 * @param  pOut : Pointer to output memory
 * @param inPtrOffset  : offset for the input buffer for D2S
 * @param outPtrOffset : offset for the input buffer for D2S
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
 * @param blockSize : blockSize/upsampling for D2S layer
 * @param elementType : elementType of the input buffer
 */
void TIDL_depthToSpaceRefProcess(
    void *pIn,
    void *pOut,
    int32_t inPtrOffset,
    int32_t outPtrOffset,
    int32_t outWidth,
    int32_t outHeight,
    int32_t numChs,
    int32_t numROIs,
    int32_t inLinePitch,
    int32_t outLinePitch,
    int32_t inChPitch,
    int32_t outChPitch,
    int32_t inROIPitch,
    int32_t outROIPitch,
    int32_t blockSize,
    int32_t elementType)
{

  if (elementType == TIDL_SinglePrecFloat)
  {
    TIDL_refDepthToSpace((float32_tidl *)pIn, (float32_tidl *)pOut, inPtrOffset, outPtrOffset, outWidth, outHeight, numChs, numROIs, inLinePitch, outLinePitch, inChPitch, outChPitch, inROIPitch, outROIPitch, blockSize);
  }
  else if ((elementType == TIDL_SignedChar) || (elementType == TIDL_UnsignedChar))
  {
    TIDL_refDepthToSpace((uint8_t *)pIn, (uint8_t *)pOut, inPtrOffset, outPtrOffset, outWidth, outHeight, numChs, numROIs, inLinePitch, outLinePitch, inChPitch, outChPitch, inROIPitch, outROIPitch, blockSize);
  }
  else if ((elementType == TIDL_SignedShort) || (elementType == TIDL_UnsignedShort))
  {
    TIDL_refDepthToSpace((uint16_t *)pIn, (uint16_t *)pOut, inPtrOffset, outPtrOffset, outWidth, outHeight, numChs, numROIs, inLinePitch, outLinePitch, inChPitch, outChPitch, inROIPitch, outROIPitch, blockSize);
  }
  else
  {
    tidl_printf(0, " Un Supported elementType , Exiting from %s, %d\n", __FILE__, __LINE__);
  }
  TIDL_L1DandL2CacheWbInv();
  return;
}
#endif
