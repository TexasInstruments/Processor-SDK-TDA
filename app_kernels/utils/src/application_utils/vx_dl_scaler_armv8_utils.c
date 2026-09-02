/*
 *
 * Copyright (c) 2023-2026 Texas Instruments Incorporated
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

#include <vx_dl_scaler_armv8_utils.h>
#include <vx_arm_neon_utils.h>
#include <stddef.h>
#if defined(A72) || defined(A53) || defined(A720)
#include <arm_neon.h>
#endif

void scaleNNNV12
(
    uint8_t* src_y,
    bufParams2D_t * src_y_prms,
    uint8_t* src_uv,
    bufParams2D_t * src_uv_prms,
    uint8_t* dst_y,
    bufParams2D_t * dst_y_prms,
    uint8_t* dst_uv,
    bufParams2D_t * dst_uv_prms
)

{
    register int sw = src_y_prms->dim_x;
    register int sh = src_y_prms->dim_y;
    register int dw = dst_y_prms->dim_x;
    register int dh = dst_y_prms->dim_y;
    register int src_stride_y = src_y_prms->stride_y;
    register int src_stride_uv = src_uv_prms->stride_y;
    register int dst_stride_y = dst_y_prms->stride_y;
    register int dst_stride_uv = dst_uv_prms->stride_y;
    register int y = 0, x = 0;

    unsigned long int xrIntFloat_16 = (sw << 16) / (dw + 1);
    unsigned long int yrIntFloat_16 = (sh << 16) / (dh + 1);

    unsigned long int offset_y;
    uint8_t* dst_y_yScanline = dst_y;
    uint8_t* src_y_yScanline = src_y;
    uint8_t* dst_uv_yScanline = dst_uv;
    uint8_t* src_uv_yScanline = src_uv;
    uint8_t* sp = NULL;
    uint8_t* dp = NULL;

#if defined(A72) || defined(A53) || defined(A720)
    register int i = 0;
    uint16_t offset_x[16];
    uint8x16_t Y = {};
    uint8x16_t UV = {};
#endif

    for (y = 0; y < dh; y++)
    {
#if defined(A72) || defined(A53) || defined(A720)
        for(x = 0; x < (dw >> 4) << 4 ; x += 16)
        {
            for (i = 0 ; i < 16; i++)
            {
                offset_x[i] = ((x+i) * xrIntFloat_16) >> 16;
                Y[i] =  src_y_yScanline[offset_x[i]];
            }
            vst1q_u8(dst_y_yScanline + x, Y);

            if(0 == (y & 1)) //y is even
            {
                UV[0] = src_uv_yScanline[offset_x[0]/2 * 2];
                for (i = 1 ; i < 16; i++)
                {
                    if (0 == (i & 1))
                        UV[i] = src_uv_yScanline[offset_x[i]/2 * 2];
                    else
                        UV[i] = src_uv_yScanline[1 + (offset_x[i-1]/2 * 2)];
                }
                vst1q_u8(dst_uv_yScanline + x, UV);
            }
        }
#endif

        // Process remaining pixel
        for(; x < dw; x ++)
        {
            dst_y_yScanline[x] = src_y_yScanline[(x * xrIntFloat_16) >> 16];
            if((y & 1) == 0 && (x & 1) == 0)
            {
                dp = dst_uv_yScanline + x;
                sp = src_uv_yScanline + ((x * xrIntFloat_16) >> 17) / 2;
                *((uint16_t*)dp) = (*(sp+1) << 8) | *(sp);
            }
        }

        offset_y = (y * yrIntFloat_16) >> 16;
        src_y_yScanline = src_y + (offset_y * src_stride_y);
        dst_y_yScanline = dst_y_yScanline + dst_stride_y;
        if((y & 1) == 0)
        {
            src_uv_yScanline = src_uv + ((offset_y / 2) * src_stride_uv);
            dst_uv_yScanline = dst_uv_yScanline + dst_stride_uv;
        }
    }
}

void scaleBLNV12
(
    uint8_t* src_y,
    bufParams2D_t * src_y_prms,
    uint8_t* src_uv,
    bufParams2D_t * src_uv_prms,
    uint8_t* dst_y,
    bufParams2D_t * dst_y_prms,
    uint8_t* dst_uv,
    bufParams2D_t * dst_uv_prms
)

{
    const int sw = src_y_prms->dim_x;
    const int sh = src_y_prms->dim_y;
    const int dw = dst_y_prms->dim_x;
    const int dh = dst_y_prms->dim_y;
    const int src_stride_y = src_y_prms->stride_y;
    const int src_stride_uv = src_uv_prms->stride_y;
    const int dst_stride_y = dst_y_prms->stride_y;
    const int dst_stride_uv = dst_uv_prms->stride_y;
    int j = 0, i = 0;

    const int xratio = (sw << 8) / dw;
    const int yratio = (sh << 8) / dh;

    uint8_t *src_y_row, *src_uv_row, *src_y_row_next;
    uint8_t *dst_y_row, *dst_uv_row;

    int ox,oy,x,y,tmpX,tmpY;
    int w0,w1,w2,w3;

    uint8_t y00,y01,y10,y11;
    uint8_t uv0,uv1;

#if defined(A72) || defined(A53) || defined(A720)
    int p = 0;
    uint8x8_t y_final = {};
    uint8x8_t uv_final = {};
#endif

    tmpY = 0;
    for (j = 0; j < dh; j++)
    {
        oy = tmpY >> 8;
        y = tmpY & 0xFF;

        src_y_row = src_y + (oy * src_stride_y);
        src_uv_row = src_uv + ((oy/2) * src_stride_uv);
        src_y_row_next = src_y_row + src_stride_y;

        dst_y_row = dst_y + (j * dst_stride_y);
        dst_uv_row = dst_uv + ((j/2) * dst_stride_uv);

        tmpX = 0;
#if defined(A72) || defined(A53) || defined(A720)
        for(i = 0; i < (dw >> 3) << 3; i += 8)
        {
            for (p = 0 ; p < 8; p++)
            {
                ox = tmpX >> 8;
                x  = tmpX & 0xFF;
                y00 = src_y_row[ox];
                y01 = src_y_row[ox+1];
                y10 = src_y_row_next[ox];
                y11 = src_y_row_next[ox+1];

                w0  = (0x100 - x) * (0x100 - y);
                w1  = (x) * (0x100 - y);
                w2  = (0x100 - x) * (y);
                w3  = (x) * (y);

                y_final[p] =   (w0 * y00 +
                                w1 * y01 +
                                w2 * y10 +
                                w3 * y11 ) >> 16;

                tmpX += xratio;
            }
            vst1_u8(dst_y_row + i, y_final);

            if ( 0 == (j & 1))
            {
                for (p = 0 ; p < 8; p++)
                {
                    if (0 == (p & 1))
                    {
                        uv0 = src_uv_row[ox & ~1];
                        uv1 = src_uv_row[(ox & ~1) + 2];
                    }
                    else
                    {
                        uv0 = src_uv_row[ox | 1];
                        uv1 = src_uv_row[(ox | 1) + 2];
                    }
                    uv_final[p] = ( w0 * uv0 +
                                    w1 * uv1 +
                                    w2 * uv0 +
                                    w3 * uv1) >> 16;
                }
                vst1_u8(dst_uv_row + i, uv_final);
            }
        }
#endif

        for(; i < dw; i++)
        {
            ox = tmpX >> 8;

            y00 = src_y_row[ox];
            y01 = src_y_row[ox+1];
            y10 = src_y_row_next[ox];
            y11 = src_y_row_next[ox+1];

            x = tmpX & 0xFF;

            w0 = (0x100 - x) * (0x100 - y);
            w1 = (x) * (0x100 - y);
            w2 = (0x100 - x) * (y);
            w3 = (x) * (y);

            uint8_t y_final =   (w0 * y00 +
                         w1 * y01 +
                         w2 * y10 +
                         w3 * y11 ) >> 16;

            dst_y_row[i] = y_final;

            if ((j & 1) == 0 && (i & 1) == 0)
            {
                uv0 = src_uv_row[ox & ~1];
                uv1 = src_uv_row[(ox & ~1) + 2];
                uint8_t u_final = (w0 * uv0 +
                             w1 * uv1 +
                             w2 * uv0 +
                             w3 * uv1 ) >> 16;

                uv0 = src_uv_row[ox | 1];
                uv1 = src_uv_row[(ox | 1) + 2];

                uint8_t v_final = (w0 * uv0 +
                             w1 * uv1 +
                             w2 * uv0 +
                             w3 * uv1) >> 16;

                dst_uv_row[i] = u_final;
                dst_uv_row[i+1] = v_final;
            }
            tmpX += xratio;
        }
        tmpY += yratio;
    }
}
