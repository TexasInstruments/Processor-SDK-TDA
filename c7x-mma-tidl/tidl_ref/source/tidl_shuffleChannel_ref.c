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
@file    tidl_shuffleChannel_ref.c
@brief   This file defines private functions for shuffle layer.
@version 0.1 (Oct 2016) : Initial version [TVS]
@version 0.2 (Jan 2017) : Cleaned up [TVS]
----------------------------------------------------------------------------
*/

#include <float.h>
#include "tidl_commonUtils.h"
#include "tidl_types.h"
#include "tidl_alg_utils.h"
#include "tidl_shuffleChannel_ref.h"

/**
 * @brief This is reference implementation of shuffle layer
 *
 * @param pIn : Pointer to input memory
 * @param pOut : Pointer to output memory
 * @param inPtrOffset : offset input buffer pointer
 * @param outPtrOffset : offset output buffer pointer
 * @param width : width of the input buffer
 * @param height : height of the input buffer
 * @param numInChs : no of total channels
 * @param numGroups : no of total groups
 * @param numROIs : no of total ROI
 * @param inLinePitch : pitch of the input buffer
 * @param outLinePitch : pitch of the output buffer
 * @param inChPitch : Chnnel pitch of the input buffer
 * @param outChPitch : Chnnel pitch of the output buffer
 * @param inROIPitch : ROI pitch of the input buffer
 * @param outROIPitch : ROI pitch of the output buffer
 */
template<class TinOut> void TIDL_shuffleChannelRefProcess(const TinOut *pIn,
                                                          TinOut *pOut,
                                                          int32_t inPtrOffset,
                                                          int32_t outPtrOffset,
                                                          int32_t width,
                                                          int32_t height,
                                                          int32_t numInChs,
                                                          int32_t numGroups,
                                                          int32_t numROIs,
                                                          int32_t inLinePitch,
                                                          int32_t outLinePitch,
                                                          int32_t inChPitch,
                                                          int32_t outChPitch,
                                                          int32_t inROIPitch,
                                                          int32_t outROIPitch)
{
  int32_t i0, i1, i2, i3;
  int32_t NiPerG = numInChs / numGroups;
  int32_t elemSize;

  elemSize = (int32_t)sizeof(*pIn);

  OPENACC(data copyin(pIn[:1 + inPtrOffset + ((numROIs - 1) * inROIPitch) + ((numGroups - 1) * inChPitch * NiPerG) + ((NiPerG - 1) * inChPitch) + ((height - 1) * inLinePitch)])
              copy(pOut[:1 + outPtrOffset + ((numROIs - 1) * outROIPitch) + ((numGroups - 1) * outChPitch) + ((NiPerG - 1) * outChPitch * numGroups) + ((height - 1) * outLinePitch)]))
  OPENACC(parallel loop collapse(4))
  for (i0 = 0; i0 < numROIs; i0++)
  {
    for (i1 = 0; i1 < numGroups; i1++)
    {
      for (i2 = 0; i2 < NiPerG; i2++)
      {
        for (i3 = 0; i3 < height; i3++)
        {
          int32_t inOffset = inPtrOffset + (i0 * inROIPitch) + (i1 * inChPitch * NiPerG) + (i2 * inChPitch) + (i3 * inLinePitch);
          int32_t outOffset = outPtrOffset + (i0 * outROIPitch) + (i1 * outChPitch) + (i2 * outChPitch * numGroups) + (i3 * outLinePitch);
          (void)memcpy(pOut + outOffset, pIn + inOffset, ((uint64_t)width * (uint64_t)elemSize));
        }
      }
    }
  }
  TIDL_L1DandL2CacheWbInv();
}

template void TIDL_shuffleChannelRefProcess(const float32_tidl *pIn,
                                            float32_tidl *pOut,
                                            int32_t inPtrOffset,
                                            int32_t outPtrOffset,
                                            int32_t width,
                                            int32_t height,
                                            int32_t numInChs,
                                            int32_t numGroups,
                                            int32_t numROIs,
                                            int32_t inLinePitch,
                                            int32_t outLinePitch,
                                            int32_t inChPitch,
                                            int32_t outChPitch,
                                            int32_t inROIPitch,
                                            int32_t outROIPitch);

template void TIDL_shuffleChannelRefProcess(const uint8_t *pIn,
                                            uint8_t *pOut,
                                            int32_t inPtrOffset,
                                            int32_t outPtrOffset,
                                            int32_t width,
                                            int32_t height,
                                            int32_t numInChs,
                                            int32_t numGroups,
                                            int32_t numROIs,
                                            int32_t inLinePitch,
                                            int32_t outLinePitch,
                                            int32_t inChPitch,
                                            int32_t outChPitch,
                                            int32_t inROIPitch,
                                            int32_t outROIPitch);

template void TIDL_shuffleChannelRefProcess(const uint16_t *pIn,
                                            uint16_t *pOut,
                                            int32_t inPtrOffset,
                                            int32_t outPtrOffset,
                                            int32_t width,
                                            int32_t height,
                                            int32_t numInChs,
                                            int32_t numGroups,
                                            int32_t numROIs,
                                            int32_t inLinePitch,
                                            int32_t outLinePitch,
                                            int32_t inChPitch,
                                            int32_t outChPitch,
                                            int32_t inROIPitch,
                                            int32_t outROIPitch);
