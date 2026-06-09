/*
*
* Copyright (c) {2015 - 2020} Texas Instruments Incorporated
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
 *  \file tidl_device_functions.h
 *
 *  \brief This file has device specific functions required by tidl_alg
 */

#ifndef _TIDL_DEVICE_FUCNTIONS_H_
#define _TIDL_DEVICE_FUCNTIONS_H_

/*Bit to check whether OTF pad is enabled*/
#define TIDL_COMP_FLAG_BIT (0x200U)
#define TIDL_DEVICE_MASK   ( 0xFF)

#include <stdlib.h>
#include "tidl_bufParams.h"
#include <typeinfo>
#include "tidl_alg_int.h"
#include "perfsim.h"
#include "tidl_commonUtils.h"
#include "tidl_deviceInfo.h"


int32_t TIDL_getBiasParamSize(int32_t deviceName, int32_t numOutChannels);

static inline int32_t TIDL_isWeightCompEnabled(uint32_t deviceName)
{
    int32_t ret_val = -1;
    #ifdef __C7100__
    ret_val = FALSE;
    #else
    /*
      Only the lower 8 bits encode the actual device, the upper bits
      are reserved as control bits to enable/disable features
    */
    if((deviceName & TIDL_COMP_FLAG_BIT) == TIDL_COMP_FLAG_BIT)
    {
        ret_val = TRUE;
    }
    else
    {
        ret_val = FALSE;
    }
    #endif
    return ret_val;
}

static inline uint32_t TIDL_isLegacyKernelEnabled(int32_t deviceName)
{
    uint32_t ret_val = 2;
    #ifdef __C7100__
    ret_val = FALSE;
    #else
    /*
      Only the lower 8 bits encode the actual device, the upper bits
      are reserved as control bits to enable/disable features
    */
    if(((uint32_t)deviceName & (uint32_t)TIDL_DEVICE_MASK) == 0U)
    {
      ret_val = TRUE;
    }
    else
    {
      ret_val = FALSE;
    }
    #endif
    return ret_val;
}

static inline uint32_t TIDL_isBiasHighPrecision(uint32_t deviceName)
{
    uint32_t ret_val = 2;
    #ifdef __C7100__
    ret_val = FALSE;
    #endif
    #if defined (__C7504__) || defined (__C7524__)
    ret_val = TRUE;
    #endif
    /*
      Only the lower 8 bits encode the actual device, the upper bits
      are reserved as control bits to enable/disable features
    */
    #if defined (__C7120__) || defined (__C7604__)
    if((deviceName & TIDL_OTF_FLAG_BIT) == TIDL_OTF_FLAG_BIT)
    {
        ret_val = TRUE;
    }
    else
    {
        ret_val = FALSE;
    }
    #endif
    return ret_val;
}

static inline int32_t TIDL_disablePadOTF(uint32_t deviceName)
{
  int32_t newDeviceName;

  #ifdef __C7100__
  newDeviceName = deviceName;
  #endif

  #ifndef __C7100__
  /*
    Only the lower 8 bits encode the actual device, the upper bits
    are reserved as control bits to enable/disable features
  */
  newDeviceName = deviceName & (~TIDL_OTF_FLAG_BIT);
  #endif

  return newDeviceName;
}

//This function is the overloaded driving function for biasSplit (device-specific)
template <class Tdst, class Tsrc, class Tbias>
void TIDL_conv2dBiasSplit(int32_t deviceName, int32_t dataFlowType, int32_t procElemSize, Tsrc * srcPtr, Tdst * dstPtr, Tbias * biasB, int32_t dataSize, float inScaleFactor, int32_t satLow, int32_t satHigh, int32_t biasBMax , int32_t inFeatSign);
int32_t TIDL_isLastSubMBlockHandleNeeded(int32_t subMChannels, int32_t No, int32_t stridedFlow, int32_t targetDevice);
#endif /* _TIDL_DEVICE_FUCNTIONS_H_*/
