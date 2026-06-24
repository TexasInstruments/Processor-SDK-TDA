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
----------------------------------------------------------------------------
@file    tidl_activation_optimizations.h
@brief   This file contains TIDL activation optimization utilities
@version 0.1 (May 2026) : Initial version [NK]
----------------------------------------------------------------------------
*/

#ifndef ITIDL_ACTIVATION_OPTIMIZATIONS_H
#define ITIDL_ACTIVATION_OPTIMIZATIONS_H

#include "itidl_ti.h"

#include <unordered_map>
#include <math.h>

namespace { /* TU-local linkage to avoid duplicate-definition / double-destruction across DSOs */

/** tidl_activationProducerRange defines the utilizable input range for each activation function
 *
 * To determine the utilizable input range for an activation function, compute the inverse of the activation function f^-1(x), 
 * and identify the input values corresponding to the minimum and maximum output values of the original function.
 * 
 * For example, for the Sigmoid activation function, the inverse is the Logit function. 
 * The output range of Sigmoid, (0, 1), is quantized into 65,536 bins to support 16-bit inference precision. 
 * Each bin represents a step size of 1 / 65535 = 1.525902189669642e-5, with the minimum quantization error at the midpoint = 7.629510948348211e-6.
 * The utilizable input range is determined by evaluating the inverse function at the output values Logit( 7.629510948348211e-6 ) and Logit( 1 - 7.629510948348211e-6 )
 */
std::unordered_map<int32_t, std::pair<float32_tidl, float32_tidl>> tidl_activationProducerRange = {
  {TIDL_Sigmoid, {-11.783479181074f, 11.783479181074f}},
  {TIDL_Tanh,    {-5.8917f, 5.8917f}},
  {TIDL_HardSigmoid, {-3.0f, 3.0f}},
  {TIDL_ELU,    {-4.0f, FLT_MAX}},
  {TIDL_GELU,  {-5.0f, FLT_MAX}}, // GeLU@-5.0 = -1.43325786e-06
  {TIDL_Asin,  {-1.0f, 1.0f}},
  {TIDL_HardSwish, {-3.0f, FLT_MAX}},
  {TIDL_Logit, {0.0f, 1.0f}},
  {TIDL_Sign,  {-0.1f, 0.1f}}
};

std::unordered_map<int32_t, std::pair<float32_tidl, float32_tidl>> tidl_activationOutputRange = {
  {TIDL_Sin,  {-1.0f, 1.0f}},
  {TIDL_Cos,  {-1.0f, 1.0f}},
  {TIDL_Tanh,    {-1.0f, 1.0f}},
  {TIDL_Sign,  {-1.0f, 1.0f}},
  {TIDL_Sigmoid, {0.0f, 1.0f}},
  {TIDL_HardSigmoid, {0.0f, 1.0f}},
  {TIDL_Asin, {-M_PI/2, M_PI/2}},
  {TIDL_Acos, {0.0f, M_PI}},
  {TIDL_Atan, {-M_PI/2, M_PI/2}}
};

void TIDL_getActivationOptimizedInputRange(sTIDL_Layer_t *layer, float32_tidl *minVal, float32_tidl *maxVal) {
  if(tidl_activationProducerRange.find(layer->actParams.actType) != tidl_activationProducerRange.end()) {
    *minVal = tidl_activationProducerRange[layer->actParams.actType].first;
    *maxVal = tidl_activationProducerRange[layer->actParams.actType].second;
  }
}

bool TIDL_isLayerRangeOptimizedActivation(sTIDL_Layer_t *layer) {
  return tidl_activationProducerRange.find(layer->actParams.actType) != tidl_activationProducerRange.end();
}

} /* anonymous namespace */

#endif /*ITIDL_ACTIVATION_OPTIMIZATIONS_H */