/*
 *  Copyright (C) 2022-2026 Texas Instruments Incorporated - http://www.ti.com/
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Module headers. */
#include <vx_arm_neon_utils.h>

#if defined(A72) || defined(A53) || defined(A720)
#include <arm_neon.h>
#endif
#include <string.h>

void memcpy_neon (void *dest, const void *src, size_t len)
{
#if defined(A72) || defined(A53) || defined(A720)
    uint64x2_t reg;
    size_t len_16 = len/16;

    for (size_t i = 0; i < len_16; i++)
    {
        reg = vld1q_u64 ((uint64_t *)src + 2*i);
        vst1q_u64 ((uint64_t *)dest + 2*i, reg);
    }

    if (len % 16 != 0)
    {
        memcpy((uint8_t *)dest + len_16 * 16,
                (uint8_t *)src + len_16 * 16, len - len_16 * 16);
    }
#else
    memcpy(dest, src, len);
#endif
}

void memset_neon (void *dest, int s, size_t len)
{
#if defined(A72) || defined(A53) || defined(A720)
    int32x4_t reg;

    s = s + (s << 8) + (s << 16) + (s << 24);
    reg = vdupq_n_s32(s);
    size_t len_16 = len/16;

    for (size_t i = 0; i < len_16; i++)
    {
        vst1q_s32 ((int32_t *)dest + 4*i, reg);
    }

    if (len % 16 != 0)
    {
        memset((uint8_t *)dest + len_16 * 16, s, len - len_16 * 16);
    }
#else
    memset(dest, s, len);
#endif
}
