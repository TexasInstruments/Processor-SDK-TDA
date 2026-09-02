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
 *
 */

/**
 *  \file tidl_bfloat16.h
 *
 *  \brief BFloat16 (Brain Floating Point 16) type definition for TIDL
 *
 *  BFloat16 is a 16-bit floating point format with the same exponent range as
 *  IEEE 754 float32 (8 exponent bits) but reduced mantissa precision (7 bits
 *  instead of 23). Layout: [1 sign][8 exponent][7 mantissa].
 *
 *  This type provides implicit conversion to/from float32, enabling transparent
 *  use in C++ templates. All arithmetic is performed by upcasting to float32.
 */

#ifndef TIDL_BFLOAT16_H
#define TIDL_BFLOAT16_H

#include <stdint.h>
#include <limits>
#include <cstring>

#ifndef __C7604__
#  define __C7604__
#  include "bfloat16.h"
#  undef __C7604__
#else
#  include "bfloat16.h"
#endif

#ifdef __cplusplus

namespace floating_point {
    namespace bf16_c7x {
        // operator/ and operator/= are not in bfloat16.h (they require a
        // hardware reciprocal instruction on C7x).  Provide a float-based
        // fallback for host emulation and for builds where the intrinsic is
        // not available.
        inline bfloat16_t operator/(const bfloat16_t& a, const bfloat16_t& b) {
            return bfloat16_t(static_cast<float>(a) / static_cast<float>(b));
        }
        inline bfloat16_t& operator/=(bfloat16_t& a, const bfloat16_t& b) {
            a = operator/(a, b);
            return a;
        }
    } // namespace bf16_c7x
} // namespace floating_point

typedef floating_point::bfloat16_t bfloat16_tidl;

inline void TIDL_convertBF16ToFP32(const bfloat16_tidl *src, float *dst, int32_t numElements)
{
  for (int32_t idx = 0; idx < numElements; idx++)
  {
    dst[idx] = (float)src[idx];
  }
}

inline void TIDL_convertFP32ToBF16(const float *src, bfloat16_tidl *dst, int32_t numElements)
{
  for (int32_t idx = 0; idx < numElements; idx++)
  {
    dst[idx] = bfloat16_tidl(src[idx]);
  }
}

#endif /* __cplusplus */

#endif /* TIDL_BFLOAT16_H */
