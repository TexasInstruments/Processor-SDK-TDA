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
#include <iostream>

#include <TI/tivx.h>
#include <stdint.h>
#include <unistd.h>

#include "../inc/itidl_rt.h"
#include "tidl_rt_ovx_utils.h"

#include <stdio.h>
#include <limits>

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> static Tdst tidl_rt_sat.* <function end>
<justification start> NOT_IN_SCOPE : This file is used in context of open source run time usages such as ONNX-RT or TFLite-RT. 
Not in scope of TIDL-RT safety use case. Hence this file is justified not to include in tidl-runtime coverage build
<justification end> */
template <typename Tdst>
static Tdst tidl_rt_sat(float val)
{
    float out;
    float max = std::numeric_limits<Tdst>::max();
    float min = std::numeric_limits<Tdst>::lowest();
    out = (val<min) ? min : val;
    out = (out>max) ? max : out;
    return (Tdst)out;
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> void copy_asis_linear.* <function end>
<justification start> NOT_IN_SCOPE : This file is used in context of open source run time usages such as ONNX-RT or TFLite-RT. 
Not in scope of TIDL-RT safety use case. Hence this file is justified not to include in tidl-runtime coverage build
<justification end> */
template <typename Tsrc, typename Tdst>
void copy_asis_linear(Tdst *dst, Tsrc *src,
                      int32_t src_bp, int32_t src_cp, int32_t src_lp, int32_t src_offset,
                      int32_t dst_bp, int32_t dst_cp, int32_t dst_lp, int32_t dst_offset,
                      int32_t nb, int32_t nc, int32_t nl, int32_t np)
{
    int32_t i0, i1, i2, i3;
    Tdst *d0 = dst + dst_offset, *d1, *d2, *d3;
    Tsrc *s0 = src + src_offset, *s1, *s2, *s3;
    float data;

    for (i0 = 0; i0 < nb; i0++)
    {
        d1 = d0;
        s1 = s0;
        for (i1 = 0; i1 < nc; i1++)
        {
            d2 = d1;
            s2 = s1;
            for (i2 = 0; i2 < nl; i2++)
            {
                d3 = d2;
                s3 = s2;
                for (i3 = 0; i3 < np; i3++)
                {
                    data = *s3++;
                    *d3++ = (Tdst)data;
                }
                d2 += dst_lp;
                s2 += src_lp;
            }
            d1 += dst_cp;
            s1 += src_cp;
        }
        d0 += dst_bp;
        s0 += src_bp;
    }
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> void copy_asis_transpose.* <function end>
<justification start> NOT_IN_SCOPE : This file is used in context of open source run time usages such as ONNX-RT or TFLite-RT. 
Not in scope of TIDL-RT safety use case. Hence this file is justified not to include in tidl-runtime coverage build
<justification end> */
template <typename Tsrc, typename Tdst>
void copy_asis_transpose(Tdst *dst, Tsrc *src,
                         int32_t src_bp, int32_t src_cp, int32_t src_lp, int32_t src_pp, int32_t src_offset,
                         int32_t dst_bp, int32_t dst_cp, int32_t dst_lp, int32_t dst_pp, int32_t dst_offset,
                         int32_t nb, int32_t nc, int32_t nl, int32_t np)
{
    int32_t i0, i1, i2, i3;
    Tdst *d0 = dst + dst_offset, *d1, *d2, *d3;
    Tsrc *s0 = src + src_offset, *s1, *s2, *s3;
    float data;

    for (i0 = 0; i0 < nb; i0++)
    {
        d1 = d0;
        s1 = s0;
        for (i1 = 0; i1 < nc; i1++)
        {
            d2 = d1;
            s2 = s1;
            for (i2 = 0; i2 < nl; i2++)
            {
                d3 = d2;
                s3 = s2;
                for (i3 = 0; i3 < np; i3++)
                {
                    data = *s3;
                    *d3 = (Tdst)data;
                    s3 += src_pp;
                    d3 += dst_pp;
                }
                d2 += dst_lp;
                s2 += src_lp;
            }
            d1 += dst_cp;
            s1 += src_cp;
        }
        d0 += dst_bp;
        s0 += src_bp;
    }
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> void copy_zf_linear.* <function end>
<justification start> NOT_IN_SCOPE : This file is used in context of open source run time usages such as ONNX-RT or TFLite-RT. 
Not in scope of TIDL-RT safety use case. Hence this file is justified not to include in tidl-runtime coverage build
<justification end> */
template <typename Tsrc, typename Tdst>
void copy_zf_linear(Tdst *dst, Tsrc *src,
                    int32_t src_bp, int32_t src_cp, int32_t src_lp, int32_t src_offset,
                    int32_t dst_bp, int32_t dst_cp, int32_t dst_lp, int32_t dst_offset,
                    int32_t nb, int32_t nc, int32_t nl, int32_t np,
                    float zf)
{
    int32_t i0, i1, i2, i3;
    Tdst *d0 = dst + dst_offset, *d1, *d2, *d3;
    Tsrc *s0 = src + src_offset, *s1, *s2, *s3;
    float data;

    for (i0 = 0; i0 < nb; i0++)
    {
        d1 = d0;
        s1 = s0;
        for (i1 = 0; i1 < nc; i1++)
        {
            d2 = d1;
            s2 = s1;
            for (i2 = 0; i2 < nl; i2++)
            {
                d3 = d2;
                s3 = s2;
                for (i3 = 0; i3 < np; i3++)
                {
                    data = *s3++;
                    *d3++  = tidl_rt_sat<Tdst>(data - zf);
                }
                d2 += dst_lp;
                s2 += src_lp;
            }
            d1 += dst_cp;
            s1 += src_cp;
        }
        d0 += dst_bp;
        s0 += src_bp;
    }
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> void copy_zf_transpose.* <function end>
<justification start> NOT_IN_SCOPE : This file is used in context of open source run time usages such as ONNX-RT or TFLite-RT. 
Not in scope of TIDL-RT safety use case. Hence this file is justified not to include in tidl-runtime coverage build
<justification end> */
template <typename Tsrc, typename Tdst>
void copy_zf_transpose(Tdst *dst, Tsrc *src,
                    int32_t src_bp, int32_t src_cp, int32_t src_lp, int32_t src_pp, int32_t src_offset,
                    int32_t dst_bp, int32_t dst_cp, int32_t dst_lp, int32_t dst_pp, int32_t dst_offset,
                    int32_t nb, int32_t nc, int32_t nl, int32_t np,
                    float zf)
{
    int32_t i0, i1, i2, i3;
    Tdst *d0 = dst + dst_offset, *d1, *d2, *d3;
    Tsrc *s0 = src + src_offset, *s1, *s2, *s3;
    float data;

    for (i0 = 0; i0 < nb; i0++)
    {
        d1 = d0;
        s1 = s0;
        for (i1 = 0; i1 < nc; i1++)
        {
            d2 = d1;
            s2 = s1;
            for (i2 = 0; i2 < nl; i2++)
            {
                d3 = d2;
                s3 = s2;
                for (i3 = 0; i3 < np; i3++)
                {
                    data = *s3;
                    *d3 = tidl_rt_sat<Tdst>(data - zf);
                    s3 += src_pp;
                    d3 += dst_pp;
                }
                d2 += dst_lp;
                s2 += src_lp;
            }
            d1 += dst_cp;
            s1 += src_cp;
        }
        d0 += dst_bp;
        s0 += src_bp;
    }
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> void copy_scale_linear.* <function end>
<justification start> NOT_IN_SCOPE : This file is used in context of open source run time usages such as ONNX-RT or TFLite-RT. 
Not in scope of TIDL-RT safety use case. Hence this file is justified not to include in tidl-runtime coverage build
<justification end> */
template <typename Tsrc, typename Tdst>
void copy_scale_linear(Tdst *dst, Tsrc *src,
                       int32_t src_bp, int32_t src_cp, int32_t src_lp, int32_t src_offset,
                       int32_t dst_bp, int32_t dst_cp, int32_t dst_lp, int32_t dst_offset,
                       int32_t nb, int32_t nc, int32_t nl, int32_t np,
                       float in_scale, float out_scale)
{
    int32_t i0, i1, i2, i3;
    Tdst *d0 = dst + dst_offset, *d1, *d2, *d3;
    Tsrc *s0 = src + src_offset, *s1, *s2, *s3;
    float data, scale = out_scale / in_scale;

    for (i0 = 0; i0 < nb; i0++)
    {
        d1 = d0;
        s1 = s0;
        for (i1 = 0; i1 < nc; i1++)
        {
            d2 = d1;
            s2 = s1;
            for (i2 = 0; i2 < nl; i2++)
            {
                d3 = d2;
                s3 = s2;
                for (i3 = 0; i3 < np; i3++)
                {
                    data = *s3++;
                    *d3++ = tidl_rt_sat<Tdst>(data * scale);
                }
                d2 += dst_lp;
                s2 += src_lp;
            }
            d1 += dst_cp;
            s1 += src_cp;
        }
        d0 += dst_bp;
        s0 += src_bp;
    }
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> void copy_scale_transpose.* <function end>
<justification start> NOT_IN_SCOPE : This file is used in context of open source run time usages such as ONNX-RT or TFLite-RT. 
Not in scope of TIDL-RT safety use case. Hence this file is justified not to include in tidl-runtime coverage build
<justification end> */
template <typename Tsrc, typename Tdst>
void copy_scale_transpose(Tdst *dst, Tsrc *src,
                       int32_t src_bp, int32_t src_cp, int32_t src_lp, int32_t src_pp, int32_t src_offset,
                       int32_t dst_bp, int32_t dst_cp, int32_t dst_lp, int32_t dst_pp, int32_t dst_offset,
                       int32_t nb, int32_t nc, int32_t nl, int32_t np,
                       float in_scale, float out_scale)
{
    int32_t i0, i1, i2, i3;
    Tdst *d0 = dst + dst_offset, *d1, *d2, *d3;
    Tsrc *s0 = src + src_offset, *s1, *s2, *s3;
    float data, scale = out_scale / in_scale;

    for (i0 = 0; i0 < nb; i0++)
    {
        d1 = d0;
        s1 = s0;
        for (i1 = 0; i1 < nc; i1++)
        {
            d2 = d1;
            s2 = s1;
            for (i2 = 0; i2 < nl; i2++)
            {
                d3 = d2;
                s3 = s2;
                for (i3 = 0; i3 < np; i3++)
                {
                    data = *s3;
                    *d3 = tidl_rt_sat<Tdst>(data * scale);

                    s3 += src_pp;
                    d3 += dst_pp;
                }
                d2 += dst_lp;
                s2 += src_lp;
            }
            d1 += dst_cp;
            s1 += src_cp;
        }
        d0 += dst_bp;
        s0 += src_bp;
    }
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> void copy_zfscale_linear.* <function end>
<justification start> NOT_IN_SCOPE : This file is used in context of open source run time usages such as ONNX-RT or TFLite-RT. 
Not in scope of TIDL-RT safety use case. Hence this file is justified not to include in tidl-runtime coverage build
<justification end> */
template <typename Tsrc, typename Tdst>
void copy_zfscale_linear(Tdst *dst, Tsrc *src,
                         int32_t src_bp, int32_t src_cp, int32_t src_lp, int32_t src_offset,
                         int32_t dst_bp, int32_t dst_cp, int32_t dst_lp, int32_t dst_offset,
                         int32_t nb, int32_t nc, int32_t nl, int32_t np,
                         float zf, float in_scale, float out_scale)
{
    int32_t i0, i1, i2, i3;
    Tdst *d0 = dst + dst_offset, *d1, *d2, *d3;
    Tsrc *s0 = src + src_offset, *s1, *s2, *s3;
    float data, scale = out_scale / in_scale;

    for (i0 = 0; i0 < nb; i0++)
    {
        d1 = d0;
        s1 = s0;
        for (i1 = 0; i1 < nc; i1++)
        {
            d2 = d1;
            s2 = s1;
            for (i2 = 0; i2 < nl; i2++)
            {
                d3 = d2;
                s3 = s2;
                for (i3 = 0; i3 < np; i3++)
                {
                    data = *s3++;
                    *d3++ = tidl_rt_sat<Tdst>((data - zf) * scale);
                }
                d2 += dst_lp;
                s2 += src_lp;
            }
            d1 += dst_cp;
            s1 += src_cp;
        }
        d0 += dst_bp;
        s0 += src_bp;
    }
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> void copy_zfscale_transpose.* <function end>
<justification start> NOT_IN_SCOPE : This file is used in context of open source run time usages such as ONNX-RT or TFLite-RT. 
Not in scope of TIDL-RT safety use case. Hence this file is justified not to include in tidl-runtime coverage build
<justification end> */
template <typename Tsrc, typename Tdst>
void copy_zfscale_transpose(Tdst *dst, Tsrc *src,
                         int32_t src_bp, int32_t src_cp, int32_t src_lp, int32_t src_pp, int32_t src_offset,
                         int32_t dst_bp, int32_t dst_cp, int32_t dst_lp, int32_t dst_pp, int32_t dst_offset,
                         int32_t nb, int32_t nc, int32_t nl, int32_t np,
                         float zf, float in_scale, float out_scale)
{
    int32_t i0, i1, i2, i3;
    Tdst *d0 = dst + dst_offset, *d1, *d2, *d3;
    Tsrc *s0 = src + src_offset, *s1, *s2, *s3;
    float data, scale = out_scale / in_scale;

    for (i0 = 0; i0 < nb; i0++)
    {
        d1 = d0;
        s1 = s0;
        for (i1 = 0; i1 < nc; i1++)
        {
            d2 = d1;
            s2 = s1;
            for (i2 = 0; i2 < nl; i2++)
            {
                d3 = d2;
                s3 = s2;
                for (i3 = 0; i3 < np; i3++)
                {
                    data = *s3;
                    *d3 = tidl_rt_sat<Tdst>((data - zf) * scale);
                    s3 += src_pp;
                    d3 += dst_pp;
                }
                d2 += dst_lp;
                s2 += src_lp;
            }
            d1 += dst_cp;
            s1 += src_cp;
        }
        d0 += dst_bp;
        s0 += src_bp;
    }
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> void copy_asis.* <function end>
<justification start> NOT_IN_SCOPE : This file is used in context of open source run time usages such as ONNX-RT or TFLite-RT. 
Not in scope of TIDL-RT safety use case. Hence this file is justified not to include in tidl-runtime coverage build
<justification end> */
template<typename Tsrc, typename Tdst>
void copy_asis(Tdst *dst, Tsrc *src,
        int32_t src_bp, int32_t src_cp, int32_t src_lp, int32_t src_pp, int32_t src_offset,
        int32_t dst_bp, int32_t dst_cp, int32_t dst_lp, int32_t dst_pp, int32_t dst_offset,
        int32_t nb, int32_t nc, int32_t nl, int32_t np)
{
    if((src_pp == 1) && (dst_pp == 1))
    {
      copy_asis_linear(dst, src, src_bp, src_cp, src_lp, src_offset, dst_bp, dst_cp, dst_lp, dst_offset, nb, nc, nl, np);
    }
    else
    {
      copy_asis_transpose(dst, src, src_bp, src_cp, src_lp, src_pp, src_offset, dst_bp, dst_cp, dst_lp, dst_pp, dst_offset, nb, nc, nl, np);
    }
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> void copy_zf.* <function end>
<justification start> NOT_IN_SCOPE : This file is used in context of open source run time usages such as ONNX-RT or TFLite-RT. 
Not in scope of TIDL-RT safety use case. Hence this file is justified not to include in tidl-runtime coverage build
<justification end> */
template<typename Tsrc, typename Tdst>
void copy_zf(Tdst *dst, Tsrc *src,
        int32_t src_bp, int32_t src_cp, int32_t src_lp, int32_t src_pp, int32_t src_offset,
        int32_t dst_bp, int32_t dst_cp, int32_t dst_lp, int32_t dst_pp, int32_t dst_offset,
        int32_t nb, int32_t nc, int32_t nl, int32_t np,
        float zf)
{
    if((src_pp == 1) && (dst_pp == 1))
    {
      copy_zf_linear(dst, src, src_bp, src_cp, src_lp, src_offset, dst_bp, dst_cp, dst_lp, dst_offset, nb, nc, nl, np, zf);
    }
    else
    {
      copy_zf_transpose(dst, src, src_bp, src_cp, src_lp, src_pp, src_offset, dst_bp, dst_cp, dst_lp, dst_pp, dst_offset, nb, nc, nl, np, zf);
    }
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> void copy_scale.* <function end>
<justification start> NOT_IN_SCOPE : This file is used in context of open source run time usages such as ONNX-RT or TFLite-RT. 
Not in scope of TIDL-RT safety use case. Hence this file is justified not to include in tidl-runtime coverage build
<justification end> */
template<typename Tsrc, typename Tdst>
void copy_scale(Tdst *dst, Tsrc *src,
        int32_t src_bp, int32_t src_cp, int32_t src_lp, int32_t src_pp, int32_t src_offset,
        int32_t dst_bp, int32_t dst_cp, int32_t dst_lp, int32_t dst_pp, int32_t dst_offset,
        int32_t nb, int32_t nc, int32_t nl, int32_t np,
        float in_scale, float out_scale)
{
    if((src_pp == 1) && (dst_pp == 1))
    {
      copy_scale_linear(dst, src, src_bp, src_cp, src_lp, src_offset, dst_bp, dst_cp, dst_lp, dst_offset, nb, nc, nl, np, in_scale, out_scale);
    }
    else
    {
      copy_scale_transpose(dst, src, src_bp, src_cp, src_lp, src_pp, src_offset, dst_bp, dst_cp, dst_lp, dst_pp, dst_offset, nb, nc, nl, np, in_scale, out_scale);
    }
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> void copy_zfscale.* <function end>
<justification start> NOT_IN_SCOPE : This file is used in context of open source run time usages such as ONNX-RT or TFLite-RT. 
Not in scope of TIDL-RT safety use case. Hence this file is justified not to include in tidl-runtime coverage build
<justification end> */
template<typename Tsrc, typename Tdst>
void copy_zfscale(Tdst *dst, Tsrc *src,
        int32_t src_bp, int32_t src_cp, int32_t src_lp, int32_t src_pp, int32_t src_offset,
        int32_t dst_bp, int32_t dst_cp, int32_t dst_lp, int32_t dst_pp, int32_t dst_offset,
        int32_t nb, int32_t nc, int32_t nl, int32_t np,
        float zf, float in_scale, float out_scale)
{
    if((src_pp == 1) && (dst_pp == 1))
    {
      copy_zfscale_linear(dst, src, src_bp, src_cp, src_lp, src_offset, dst_bp, dst_cp, dst_lp, dst_offset, nb, nc, nl, np, zf, in_scale, out_scale);
    }
    else
    {
      copy_zfscale_transpose(dst, src, src_bp, src_cp, src_lp, src_pp, src_offset, dst_bp, dst_cp, dst_lp, dst_pp, dst_offset, nb, nc, nl, np, zf, in_scale, out_scale);
    }
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> static int32_t tidlrt_getDatElementSize.* <function end>
<justification start> NOT_IN_SCOPE : This file is used in context of open source run time usages such as ONNX-RT or TFLite-RT. 
Not in scope of TIDL-RT safety use case. Hence this file is justified not to include in tidl-runtime coverage build
<justification end> */
static int32_t tidlrt_getDatElementSize(int32_t elementType)
{
  if ((elementType == TIDL_SignedChar) || (elementType == TIDL_UnsignedChar))
  {
    return sizeof(int8_t);
  }
  else if ((elementType == TIDL_SignedShort) || (elementType == TIDL_UnsignedShort))
  {
    return sizeof(int16_t);
  }
  else if ((elementType == TIDL_SinglePrecFloat) || (elementType == TIDL_UnsignedWord) || (elementType == TIDL_SignedWord))
  {
    return sizeof(int32_t);
  }
  else if ((elementType == TIDL_UnsignedDoubleWord) || (elementType == TIDL_SignedDoubleWord))
  {
    return sizeof(int64_t);
  }
  else
  {
    return 1;
  }
}

extern "C" {
int tidlrt_printf(const char *fmt, ...);
/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> vx_status is_tidlrt_in_tensor_same.* <function end>
<justification start> NOT_IN_SCOPE : This file is used in context of open source run time usages such as ONNX-RT or TFLite-RT. 
Not in scope of TIDL-RT safety use case. Hence this file is justified not to include in tidl-runtime coverage build
<justification end> */
vx_status is_tidlrt_in_tensor_same( sTIDL_IOBufDesc_t *ioBufDesc, sTIDLRT_Tensor_t *in[], uint32_t id, uint32_t tidlrt_id)
{
    if(in[tidlrt_id]->elementType != ioBufDesc->inElementType[id])
    {
        tidlrt_printf("TIDL_RT_OVX : ElementType - %d, %d \n", ioBufDesc->inElementType[id], in[tidlrt_id]->elementType);        
        return 0;
    }
    int32_t chPitch = in[tidlrt_id]->pitch[TIDL_CHANNEL_PITCH];
    if(chPitch == -1)
    {
        chPitch = ioBufDesc->inWidth[id]*ioBufDesc->inHeight[id];
    }
    if((chPitch != ioBufDesc->inChannelPitch[id]) && (ioBufDesc->inNumChannels[id] > 1) && (in[tidlrt_id]->layout == TIDLRT_LT_NCHW))
    {
       tidlrt_printf("TIDL_RT_OVX : CP - %d, %d\n", ioBufDesc->inChannelPitch[id], chPitch);        
       return 0;
    }
    if((in[tidlrt_id]->layout != (uint32_t)ioBufDesc->inLayout[id]) && (ioBufDesc->inNumChannels[id] > 1))
    {
       tidlrt_printf("TIDL_RT_OVX : Layout - %d, %d\n", ioBufDesc->inLayout[id], in[tidlrt_id]->layout);        
       return 0;
    }
    if(in[tidlrt_id]->zeroPoint != ioBufDesc->inZeroPoint[id])
    {
       tidlrt_printf("TIDL_RT_OVX : Zp - %d, %d \n", ioBufDesc->inZeroPoint[id], in[tidlrt_id]->zeroPoint);        
       return 0;
    }
    if(in[tidlrt_id]->scale != ioBufDesc->inTensorScale[id])
    {
       tidlrt_printf("TIDL_RT_OVX : Scale - %f, %f \n", ioBufDesc->inTensorScale[id], in[tidlrt_id]->scale);        
       return 0;
    }
    if(in[tidlrt_id]->padValues[0] != ioBufDesc->inPadL[id])
    {
       tidlrt_printf("TIDL_RT_OVX : PadL - %f, %f \n", ioBufDesc->inPadL[id], in[tidlrt_id]->padValues[0]);
       return 0;
    }
    if(in[tidlrt_id]->padValues[1] != ioBufDesc->inPadR[id])
    {
       tidlrt_printf("TIDL_RT_OVX : PadR - %f, %f \n", ioBufDesc->inPadR[id], in[tidlrt_id]->padValues[1]);
       return 0;
    }
    if(in[tidlrt_id]->padValues[2] != ioBufDesc->inPadT[id])
    {
       tidlrt_printf("TIDL_RT_OVX : PadT - %f, %f \n", ioBufDesc->inPadT[id], in[tidlrt_id]->padValues[2]);
       return 0;
    }
    int32_t bufferSize = in[tidlrt_id]->bufferSize;
    if(bufferSize != -1)
    {
        /* bufferSize = -1 indicates buffer size is not set. Buffer size is always set in OSRT, so this condition is always checked for OSRT
         In standalone TIDL-RT, upto customer to set this variable. Even if not set, we assume customer has set buffer size based on iobufdesc buffer size 
         so no copy should be needed and it is ok not to check this condition */
        if(in[tidlrt_id]->bufferSize < ioBufDesc->inBufSize[id])
        {
            /* This condition should never be observed in TIDL-RT since exact size required by ioBufDesc->inBufSize is allocated
            Can be seen in OSRT if TIDL expects larger input buffer size (TIDL-4466 as an example) than actual tensor size in model */
            tidlrt_printf("TIDL_RT_OVX : TIDL required buffer size - %d, buffer size allocated by application %d \n", ioBufDesc->inBufSize[id], in[tidlrt_id]->bufferSize);        
            return 0;
        }
    }
    return 1;
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> vx_status is_tidlrt_out_tensor_same.* <function end>
<justification start> NOT_IN_SCOPE : This file is used in context of open source run time usages such as ONNX-RT or TFLite-RT. 
Not in scope of TIDL-RT safety use case. Hence this file is justified not to include in tidl-runtime coverage build
<justification end> */
vx_status is_tidlrt_out_tensor_same( sTIDL_IOBufDesc_t *ioBufDesc, sTIDLRT_Tensor_t *in[], uint32_t id, uint32_t tidlrt_id)
{
    if (in[tidlrt_id]->elementType != ioBufDesc->outElementType[id])
    {
        tidlrt_printf("TIDL_RT_OVX : ElementType - %d, %d \n", ioBufDesc->outElementType[id], in[tidlrt_id]->elementType);        
        return 0;
    }
    int32_t chPitch = in[tidlrt_id]->pitch[TIDL_CHANNEL_PITCH];
    if(chPitch == -1)
    {
        chPitch = ioBufDesc->outWidth[id]*ioBufDesc->outHeight[id];
    }
    if((chPitch != ioBufDesc->outChannelPitch[id]) && (ioBufDesc->outNumChannels[id] > 1) && (in[tidlrt_id]->layout == TIDLRT_LT_NCHW))
    {
        tidlrt_printf("TIDL_RT_OVX : CP - %d, %d\n", ioBufDesc->outChannelPitch[id], chPitch);        
        return 0;
    }
    if((in[tidlrt_id]->layout != (uint32_t)ioBufDesc->outLayout[id]) && (ioBufDesc->outNumChannels[id] > 1))
    {
        tidlrt_printf("TIDL_RT_OVX : Layout - %d, %d\n", ioBufDesc->outLayout[id], in[tidlrt_id]->layout);        
        return 0;
    }
    if(in[tidlrt_id]->zeroPoint != ioBufDesc->outZeroPoint[id])
    {
       tidlrt_printf("TIDL_RT_OVX : Zp - %d, %d \n", ioBufDesc->outZeroPoint[id], in[tidlrt_id]->zeroPoint);        
       return 0;
    }
    if(in[tidlrt_id]->scale != ioBufDesc->outTensorScale[id])
    {
        tidlrt_printf("TIDL_RT_OVX : Scale - %f, %f \n", ioBufDesc->outTensorScale[id], in[tidlrt_id]->scale);        
        return 0;
    }
    if(in[tidlrt_id]->padValues[0] != ioBufDesc->outPadL[id])
    {
       tidlrt_printf("TIDL_RT_OVX : PadL - %f, %f \n", ioBufDesc->outPadL[id], in[tidlrt_id]->padValues[0]);
       return 0;
    }
    if(in[tidlrt_id]->padValues[1] != ioBufDesc->outPadR[id])
    {
       tidlrt_printf("TIDL_RT_OVX : PadR - %f, %f \n", ioBufDesc->outPadR[id], in[tidlrt_id]->padValues[1]);
       return 0;
    }
    if(in[tidlrt_id]->padValues[2] != ioBufDesc->outPadT[id])
    {
       tidlrt_printf("TIDL_RT_OVX : PadT - %f, %f \n", ioBufDesc->outPadT[id], in[tidlrt_id]->padValues[2]);
       return 0;
    }
    return 1;
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> vx_status cp_data_in_tidlrt_tensor_tiovx.* <function end>
<justification start> NOT_IN_SCOPE : This file is used in context of open source run time usages such as ONNX-RT or TFLite-RT. 
Not in scope of TIDL-RT safety use case. Hence this file is justified not to include in tidl-runtime coverage build
<justification end> */
vx_status cp_data_in_tidlrt_tensor_tiovx(AppObj *obj, sTIDLRT_Tensor_t *in[], void *input_buffer, uint32_t id, uint32_t tidlrt_id)
{
    sTIDL_IOBufDesc_t *ioBufDesc = &obj->ioBufDesc;

    if(is_tidlrt_in_tensor_same(ioBufDesc, in, id, tidlrt_id))
    {
        if(in[tidlrt_id]->memType != TIDLRT_MEM_SHARED)
        {
            int32_t c = ioBufDesc->inNumChannels[id] * ioBufDesc->inDIM1[id] * ioBufDesc->inDIM2[id];
            int32_t n = ioBufDesc->inNumBatches[id];
            void *rtPtr  = in[tidlrt_id]->ptr;
            void * ivPtr = input_buffer;
            int32_t elementSizeBytes  = tidlrt_getDatElementSize(in[tidlrt_id]->elementType);
            int32_t chPitch = in[tidlrt_id]->pitch[TIDL_CHANNEL_PITCH];
            int32_t memcpSize;

            if(chPitch == -1)
            {
                chPitch = ioBufDesc->inWidth[id]*ioBufDesc->inHeight[id];
            }

            memcpSize = n * c * elementSizeBytes * chPitch;
            memcpy(ivPtr, rtPtr, memcpSize);
            tidlrt_printf("TIDL_RT_OVX : Memcpy Input Buffer \n"); 
        }
        else
        {
            tidlrt_printf("TIDL_RT_OVX : Direclty using User Buffer in C7x for input \n"); 
        }
    }
    else
    {
        tidlrt_printf("TIDL_RT_OVX : Reformat input Buffer \n"); 
  
        int32_t c, h, w, b, lp, cp, pp = 1, bp;
        int32_t offset;
        
        void *rtPtr  = in[tidlrt_id]->ptr;
        void *ivPtr = input_buffer;

        if (in[tidlrt_id]->dimValues[TIDLRT_DIM_BATCH] > 0)
        {
            b =  in[tidlrt_id]->dimValues[TIDLRT_DIM_BATCH];
        }
        else
        {
            b = ioBufDesc->inNumBatches[id];
        }

        if (in[tidlrt_id]->dimValues[TIDLRT_DIM_NUMCH] > 0)
        {
            c =  in[tidlrt_id]->dimValues[TIDLRT_DIM_NUMCH] * in[tidlrt_id]->dimValues[TIDLRT_DIM_DIM2] * in[tidlrt_id]->dimValues[TIDLRT_DIM_DIM1];
        }
        else
        {
            c = ioBufDesc->inNumChannels[id] * ioBufDesc->inDIM1[id] * ioBufDesc->inDIM2[id];
        }

        if (in[tidlrt_id]->dimValues[TIDLRT_DIM_HEIGHT] > 0)
        {
            h =  in[tidlrt_id]->dimValues[TIDLRT_DIM_HEIGHT];
        }
        else
        {
            h = ioBufDesc->inHeight[id];
        }

        if (in[tidlrt_id]->dimValues[TIDLRT_DIM_WIDTH] > 0)
        {
            w =  in[tidlrt_id]->dimValues[TIDLRT_DIM_WIDTH];
        }
        else
        {
            w = ioBufDesc->inWidth[id];
        }

        if (in[tidlrt_id]->pitch[TIDLRT_CHANNEL_PITCH] > 0)
        {
            cp = in[tidlrt_id]->pitch[TIDLRT_CHANNEL_PITCH];
        }
        else
        {
            cp = ioBufDesc->inChannelPitch[id];
        }

        bp = c * cp;
        lp = w + in[tidlrt_id]->padValues[0] + in[tidlrt_id]->padValues[1];
        offset = lp * in[tidlrt_id]->padValues[2] + in[tidlrt_id]->padValues[0];

        float scale = ioBufDesc->inTensorScale[id];
        float inScale = in[tidlrt_id]->scale;
        int32_t zp = in[tidlrt_id]->zeroPoint;
        int32_t zp_iobuf = ioBufDesc->inZeroPoint[id];

        if(in[tidlrt_id]->elementType == TIDL_SinglePrecFloat)
        {
            inScale = 1.0;
            zp = 0;
        }
        if(ioBufDesc->inElementType[id]== TIDL_SinglePrecFloat)
        {
            scale = 1.0;
        }
        int32_t src_bp, src_cp, src_lp, src_pp;
        if(in[tidlrt_id]->layout == TIDLRT_LT_NCHW)
        {
            src_bp = c * h * w;
            src_cp = h * w;
            src_lp = w;
            src_pp = 1;
        }
        else /* TIDLRT_LT_NHWC */
        {
            src_bp = c * h * w;
            src_cp = 1;
            src_lp = c * w;
            src_pp = c;
        }

        if(ioBufDesc->inLayout[id]== TIDLRT_LT_NHWC)
        {
            cp = 1;
            lp = c * w;
            pp = c;
            bp = c * h * w;
        }

        tidlrt_printf("TIDL_RT_OVX : ElementType - %d, %d | Scale - %f, %f | Zp - %d, %d | Layout - %d, %d | CP - %d, %d\n", in[tidlrt_id]->elementType, ioBufDesc->inElementType[id], inScale, scale, zp, ioBufDesc->inZeroPoint[id], in[tidlrt_id]->layout, ioBufDesc->inLayout[id], ioBufDesc->inChannelPitch[id], src_cp);        

        if(zp == zp_iobuf && inScale == scale) {
            switch(in[tidlrt_id]->elementType) {
                case TIDLRT_Uint8:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_asis(( uint8_t*)ivPtr, ( uint8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_asis((  int8_t*)ivPtr, ( uint8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_asis((uint16_t*)ivPtr, ( uint8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_asis(( int16_t*)ivPtr, ( uint8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_asis((   float*)ivPtr, ( uint8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else
                        return VX_FAILURE;
                    break;
                case TIDLRT_Int8:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_asis(( uint8_t*)ivPtr, (  int8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_asis((  int8_t*)ivPtr, (  int8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_asis((uint16_t*)ivPtr, (  int8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_asis(( int16_t*)ivPtr, (  int8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_asis((   float*)ivPtr, (  int8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else
                        return VX_FAILURE;
                    break;
                case TIDLRT_Uint16:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_asis(( uint8_t*)ivPtr, (uint16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_asis((  int8_t*)ivPtr, (uint16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_asis((uint16_t*)ivPtr, (uint16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_asis(( int16_t*)ivPtr, (uint16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_asis((   float*)ivPtr, (uint16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else
                        return VX_FAILURE;
                    break;
                case TIDLRT_Int16:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_asis(( uint8_t*)ivPtr, ( int16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_asis((  int8_t*)ivPtr, ( int16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_asis((uint16_t*)ivPtr, ( int16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_asis(( int16_t*)ivPtr, ( int16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_asis((   float*)ivPtr, ( int16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else
                        return VX_FAILURE;
                    break;
                case TIDLRT_Uint32:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_asis(( uint8_t*)ivPtr, (uint32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_asis((  int8_t*)ivPtr, (uint32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_asis((uint16_t*)ivPtr, (uint32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_asis(( int16_t*)ivPtr, (uint32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_asis((   float*)ivPtr, (uint32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else
                        return VX_FAILURE;
                    break;
                case TIDLRT_Int32:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_asis(( uint8_t*)ivPtr, ( int32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_asis((  int8_t*)ivPtr, ( int32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_asis((uint16_t*)ivPtr, ( int32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_asis(( int16_t*)ivPtr, ( int32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] == TIDL_SignedWord)
                        copy_asis(( int32_t*)ivPtr, ( int32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] == TIDL_UnsignedWord)
                        copy_asis(( uint32_t*)ivPtr, ( int32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_asis((   float*)ivPtr, ( int32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else
                        return VX_FAILURE;
                    break;
                case TIDLRT_Float32:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_asis(( uint8_t*)ivPtr, (   float*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_asis((  int8_t*)ivPtr, (   float*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_asis((uint16_t*)ivPtr, (   float*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_asis(( int16_t*)ivPtr, (   float*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_asis((   float*)ivPtr, (   float*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else
                        return VX_FAILURE;
                    break;
                case TIDLRT_Uint64:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_asis(( uint8_t*)ivPtr, (uint64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_asis((  int8_t*)ivPtr, (uint64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_asis((uint16_t*)ivPtr, (uint64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_asis(( int16_t*)ivPtr, (uint64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_asis((   float*)ivPtr, (uint64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else
                        return VX_FAILURE;
                    break;
                case TIDLRT_Int64:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_asis(( uint8_t*)ivPtr, ( int64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_asis((  int8_t*)ivPtr, ( int64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_asis((uint16_t*)ivPtr, ( int64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_asis(( int16_t*)ivPtr, ( int64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedDoubleWord)
                        copy_asis(( uint64_t*)ivPtr, ( int64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedDoubleWord)
                        copy_asis(( int64_t*)ivPtr, ( int64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedWord)
                        copy_asis(( int32_t*)ivPtr, ( int64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_asis((   float*)ivPtr, ( int64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w);
                    else
                        return VX_FAILURE;
                    break;
                default:
                    return VX_FAILURE;
            }
        } else if(zp == zp_iobuf) {
            switch(in[tidlrt_id]->elementType) {
                case TIDLRT_Uint8:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_scale(( uint8_t*)ivPtr, ( uint8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_scale((  int8_t*)ivPtr, ( uint8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_scale((uint16_t*)ivPtr, ( uint8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_scale(( int16_t*)ivPtr, ( uint8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_scale((   float*)ivPtr, ( uint8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else
                        return VX_FAILURE;
                    break;
                case TIDLRT_Int8:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_scale(( uint8_t*)ivPtr, (  int8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_scale((  int8_t*)ivPtr, (  int8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_scale((uint16_t*)ivPtr, (  int8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_scale(( int16_t*)ivPtr, (  int8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_scale((   float*)ivPtr, (  int8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else
                        return VX_FAILURE;
                    break;
                case TIDLRT_Uint16:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_scale(( uint8_t*)ivPtr, (uint16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_scale((  int8_t*)ivPtr, (uint16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_scale((uint16_t*)ivPtr, (uint16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_scale(( int16_t*)ivPtr, (uint16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_scale((   float*)ivPtr, (uint16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else
                        return VX_FAILURE;
                    break;
                case TIDLRT_Int16:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_scale(( uint8_t*)ivPtr, ( int16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_scale((  int8_t*)ivPtr, ( int16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_scale((uint16_t*)ivPtr, ( int16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_scale(( int16_t*)ivPtr, ( int16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_scale((   float*)ivPtr, ( int16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else
                        return VX_FAILURE;
                    break;
                case TIDLRT_Uint32:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_scale(( uint8_t*)ivPtr, (uint32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_scale((  int8_t*)ivPtr, (uint32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_scale((uint16_t*)ivPtr, (uint32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_scale(( int16_t*)ivPtr, (uint32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_scale((   float*)ivPtr, (uint32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else
                        return VX_FAILURE;
                    break;
                case TIDLRT_Int32:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_scale(( uint8_t*)ivPtr, ( int32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_scale((  int8_t*)ivPtr, ( int32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_scale((uint16_t*)ivPtr, ( int32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_scale(( int16_t*)ivPtr, ( int32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_scale((   float*)ivPtr, ( int32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else
                        return VX_FAILURE;
                    break;
                case TIDLRT_Float32:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_scale(( uint8_t*)ivPtr, (   float*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_scale((  int8_t*)ivPtr, (   float*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_scale((uint16_t*)ivPtr, (   float*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_scale(( int16_t*)ivPtr, (   float*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_scale((   float*)ivPtr, (   float*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else
                        return VX_FAILURE;
                    break;
                case TIDLRT_Uint64:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_scale(( uint8_t*)ivPtr, (uint64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_scale((  int8_t*)ivPtr, (uint64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_scale((uint16_t*)ivPtr, (uint64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_scale(( int16_t*)ivPtr, (uint64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_scale((   float*)ivPtr, (uint64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else
                        return VX_FAILURE;
                    break;
                case TIDLRT_Int64:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_scale(( uint8_t*)ivPtr, ( int64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_scale((  int8_t*)ivPtr, ( int64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_scale((uint16_t*)ivPtr, ( int64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_scale(( int16_t*)ivPtr, ( int64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_scale((   float*)ivPtr, ( int64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, inScale, scale);
                    else
                        return VX_FAILURE;
                    break;
                default:
                    return VX_FAILURE;
            }
        } else if(inScale == scale) {
            switch(in[tidlrt_id]->elementType) {
                case TIDLRT_Uint8:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_zf(( uint8_t*)ivPtr, ( uint8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_zf((  int8_t*)ivPtr, ( uint8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_zf((uint16_t*)ivPtr, ( uint8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_zf(( int16_t*)ivPtr, ( uint8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_zf((   float*)ivPtr, ( uint8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else
                        return VX_FAILURE;
                    break;
                case TIDLRT_Int8:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_zf(( uint8_t*)ivPtr, (  int8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_zf((  int8_t*)ivPtr, (  int8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_zf((uint16_t*)ivPtr, (  int8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_zf(( int16_t*)ivPtr, (  int8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_zf((   float*)ivPtr, (  int8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else
                        return VX_FAILURE;
                    break;
                case TIDLRT_Uint16:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_zf(( uint8_t*)ivPtr, (uint16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_zf((  int8_t*)ivPtr, (uint16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_zf((uint16_t*)ivPtr, (uint16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_zf(( int16_t*)ivPtr, (uint16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_zf((   float*)ivPtr, (uint16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else
                        return VX_FAILURE;
                    break;
                case TIDLRT_Int16:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_zf(( uint8_t*)ivPtr, ( int16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_zf((  int8_t*)ivPtr, ( int16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_zf((uint16_t*)ivPtr, ( int16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_zf(( int16_t*)ivPtr, ( int16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_zf((   float*)ivPtr, ( int16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else
                        return VX_FAILURE;
                    break;
                case TIDLRT_Uint32:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_zf(( uint8_t*)ivPtr, (uint32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_zf((  int8_t*)ivPtr, (uint32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_zf((uint16_t*)ivPtr, (uint32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_zf(( int16_t*)ivPtr, (uint32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_zf((   float*)ivPtr, (uint32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else
                        return VX_FAILURE;
                    break;
                case TIDLRT_Int32:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_zf(( uint8_t*)ivPtr, ( int32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_zf((  int8_t*)ivPtr, ( int32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_zf((uint16_t*)ivPtr, ( int32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_zf(( int16_t*)ivPtr, ( int32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_zf((   float*)ivPtr, ( int32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else
                        return VX_FAILURE;
                    break;
                case TIDLRT_Float32:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_zf(( uint8_t*)ivPtr, (   float*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_zf((  int8_t*)ivPtr, (   float*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_zf((uint16_t*)ivPtr, (   float*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_zf(( int16_t*)ivPtr, (   float*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_zf((   float*)ivPtr, (   float*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else
                        return VX_FAILURE;
                    break;
                case TIDLRT_Uint64:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_zf(( uint8_t*)ivPtr, (uint64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_zf((  int8_t*)ivPtr, (uint64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_zf((uint16_t*)ivPtr, (uint64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_zf(( int16_t*)ivPtr, (uint64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_zf((   float*)ivPtr, (uint64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else
                        return VX_FAILURE;
                    break;
                case TIDLRT_Int64:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_zf(( uint8_t*)ivPtr, ( int64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_zf((  int8_t*)ivPtr, ( int64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_zf((uint16_t*)ivPtr, ( int64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_zf(( int16_t*)ivPtr, ( int64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_zf((   float*)ivPtr, ( int64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp);
                    else
                        return VX_FAILURE;
                    break;
                default:
                    return VX_FAILURE;
            }
        } else {
            switch(in[tidlrt_id]->elementType) {
                case TIDLRT_Uint8:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_zfscale(( uint8_t*)ivPtr, ( uint8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_zfscale((  int8_t*)ivPtr, ( uint8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_zfscale((uint16_t*)ivPtr, ( uint8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_zfscale(( int16_t*)ivPtr, ( uint8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_zfscale((   float*)ivPtr, ( uint8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else
                        return VX_FAILURE;
                    break;
                case TIDLRT_Int8:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_zfscale(( uint8_t*)ivPtr, (  int8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_zfscale((  int8_t*)ivPtr, (  int8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_zfscale((uint16_t*)ivPtr, (  int8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_zfscale(( int16_t*)ivPtr, (  int8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_zfscale((   float*)ivPtr, (  int8_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else
                        return VX_FAILURE;
                    break;
                case TIDLRT_Uint16:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_zfscale(( uint8_t*)ivPtr, (uint16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_zfscale((  int8_t*)ivPtr, (uint16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_zfscale((uint16_t*)ivPtr, (uint16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_zfscale(( int16_t*)ivPtr, (uint16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_zfscale((   float*)ivPtr, (uint16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else
                        return VX_FAILURE;
                    break;
                case TIDLRT_Int16:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_zfscale(( uint8_t*)ivPtr, ( int16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_zfscale((  int8_t*)ivPtr, ( int16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_zfscale((uint16_t*)ivPtr, ( int16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_zfscale(( int16_t*)ivPtr, ( int16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_zfscale((   float*)ivPtr, ( int16_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else
                        return VX_FAILURE;
                    break;
                case TIDLRT_Uint32:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_zfscale(( uint8_t*)ivPtr, (uint32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_zfscale((  int8_t*)ivPtr, (uint32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_zfscale((uint16_t*)ivPtr, (uint32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_zfscale(( int16_t*)ivPtr, (uint32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_zfscale((   float*)ivPtr, (uint32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else
                        return VX_FAILURE;
                    break;
                case TIDLRT_Int32:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_zfscale(( uint8_t*)ivPtr, ( int32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_zfscale((  int8_t*)ivPtr, ( int32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_zfscale((uint16_t*)ivPtr, ( int32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_zfscale(( int16_t*)ivPtr, ( int32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_zfscale((   float*)ivPtr, ( int32_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else
                        return VX_FAILURE;
                    break;
                case TIDLRT_Float32:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_zfscale(( uint8_t*)ivPtr, (   float*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_zfscale((  int8_t*)ivPtr, (   float*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_zfscale((uint16_t*)ivPtr, (   float*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_zfscale(( int16_t*)ivPtr, (   float*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_zfscale((   float*)ivPtr, (   float*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else
                        return VX_FAILURE;
                    break;
                case TIDLRT_Uint64:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_zfscale(( uint8_t*)ivPtr, (uint64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_zfscale((  int8_t*)ivPtr, (uint64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_zfscale((uint16_t*)ivPtr, (uint64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_zfscale(( int16_t*)ivPtr, (uint64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_zfscale((   float*)ivPtr, (uint64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else
                        return VX_FAILURE;
                    break;
                case TIDLRT_Int64:
                    if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedChar)
                        copy_zfscale(( uint8_t*)ivPtr, ( int64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedChar)
                        copy_zfscale((  int8_t*)ivPtr, ( int64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_UnsignedShort)
                        copy_zfscale((uint16_t*)ivPtr, ( int64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] ==  TIDL_SignedShort)
                        copy_zfscale(( int16_t*)ivPtr, ( int64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else if(ioBufDesc->inElementType[id] == TIDL_SinglePrecFloat)
                        copy_zfscale((   float*)ivPtr, ( int64_t*)rtPtr, src_bp, src_cp, src_lp, src_pp, 0, bp, cp, lp, pp, offset, b, c, h, w, zp, inScale, scale);
                    else
                        return VX_FAILURE;
                    break;
                default:
                    return VX_FAILURE;
            }
        }
    }
    return VX_SUCCESS;
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> vx_status cp_data_out_tensor_tidlrt_tiovx.* <function end>
<justification start> NOT_IN_SCOPE : This file is used in context of open source run time usages such as ONNX-RT or TFLite-RT. 
Not in scope of TIDL-RT safety use case. Hence this file is justified not to include in tidl-runtime coverage build
<justification end> */
vx_status cp_data_out_tensor_tidlrt_tiovx(AppObj *obj, sTIDLRT_Tensor_t *out[], void *output_buffer, uint32_t id, uint32_t elementSize, vx_size data_type, float scale, uint32_t tidlrt_id)
{
    sTIDL_IOBufDesc_t *ioBufDesc = &obj->ioBufDesc;
    scale = ioBufDesc->outTensorScale[id]; /* Scale should be used from ioBufDesc */

    if(is_tidlrt_out_tensor_same(ioBufDesc, out, id, tidlrt_id))
    {
        if(out[tidlrt_id]->memType != TIDLRT_MEM_SHARED)
        {
            int32_t c = ioBufDesc->outNumChannels[id] * ioBufDesc->outDIM1[id] * ioBufDesc->outDIM2[id];
            int32_t n = ioBufDesc->outNumBatches[id];
            void * rtPtr = out[tidlrt_id]->ptr;
            void *ivPtr  = output_buffer;
            int32_t elementSizeBytes  = tidlrt_getDatElementSize(out[tidlrt_id]->elementType);
            int32_t memcpSize;
            int32_t chPitch = out[tidlrt_id]->pitch[TIDL_CHANNEL_PITCH];
            if(chPitch == -1)
            {
                chPitch = ioBufDesc->outWidth[id]*ioBufDesc->outHeight[id];
            }
            memcpSize = n * c * elementSizeBytes * chPitch;
            memcpy(rtPtr, ivPtr, memcpSize);
            tidlrt_printf("TIDL_RT_OVX : Memcpy output Buffer %p %d %s %s\n", rtPtr, memcpSize, ioBufDesc->outDataName[id], out[tidlrt_id]->name); 
        }
        else
        {
            tidlrt_printf("TIDL_RT_OVX : Direclty using User Buffer in C7x for output \n"); 
        }


    }
    else
    {
        tidlrt_printf("TIDL_RT_OVX : Reformat output Buffer \n"); 
        int32_t b, c, h, w, bp, lp, cp, pp = 1;
        int32_t offset, zp, zp_iobuf;
        float outScale;
        void * rtPtr = out[tidlrt_id]->ptr;
        void *ivPtr  = output_buffer;

        /*
         * Just like cp_data_in_tensor_tidlrt_tiovx update for supporting
         * runtime changing shapes, we should we picking these values from
         * out tensor instead of ioBufDesc. But currently the way output
         * buffer is allocated to onnxruntime is based on ioBufDesc. So
         * the output buffer from onnxruntime is always asme size as decided in
         * ioBufDesc and not actual model. Based on that case this logic still
         * works. Correct fix is to not allocate based on ioBufDesc but actual
         * output shape as calculated and updated in TIDL_OutArgs after
         * TIDL_Process. This requires change in onnxruntime. Post that this
         * logic should also be updated. It is a TODO 
         */ 

        b = ioBufDesc->outNumBatches[id];
        c = ioBufDesc->outNumChannels[id] * ioBufDesc->outDIM1[id] * ioBufDesc->outDIM2[id];
        w = ioBufDesc->outWidth[id];
        h = ioBufDesc->outHeight[id];
        lp = w + ioBufDesc->outPadL[id] + ioBufDesc->outPadR[id];
        cp = ioBufDesc->outChannelPitch[id];
        bp = c * cp;
        offset = lp * ioBufDesc->outPadT[id] + ioBufDesc->outPadL[id];

        if(out[tidlrt_id]->elementType == TIDLRT_Float32)
        {
            out[tidlrt_id]->scale = 1.0;
            out[tidlrt_id]->zeroPoint = 0;
        }

        if(out[tidlrt_id]->scale == -1)
        {
            out[tidlrt_id]->scale = scale;
        }

        outScale = out[tidlrt_id]->scale;
        zp = out[tidlrt_id]->zeroPoint;
        zp_iobuf = ioBufDesc->outZeroPoint[id];

        int32_t dst_bp, dst_cp, dst_lp, dst_pp;

        if(out[tidlrt_id]->layout == TIDLRT_LT_NCHW)
        {
            dst_bp = c * h * w;
            dst_cp = h * w;
            dst_lp = w;
            dst_pp = 1;
        }
        else /* TIDLRT_LT_NHWC */
        {
            dst_cp = 1;
            dst_lp = c * w;
            dst_pp = c;
            dst_bp = c * h * w;
        }
        if(ioBufDesc->outLayout[id]== TIDLRT_LT_NHWC)
        {
            cp = 1;
            lp = c * w;
            pp = c;
            bp = c * h * w;
        }

        tidlrt_printf("TIDL_RT_OVX : ElementType - %d, %d | Scale - %f, %f | Zp - %d, %d | Layout - %d, %d | CP - %d, %d\n", ioBufDesc->outElementType[id], out[tidlrt_id]->elementType, ioBufDesc->outTensorScale[id], out[tidlrt_id]->scale , ioBufDesc->outZeroPoint[id], out[tidlrt_id]->zeroPoint, ioBufDesc->outLayout[id], out[tidlrt_id]->layout , ioBufDesc->outChannelPitch[id], dst_cp);
        if(zp == zp_iobuf && outScale == scale) {
            switch(ioBufDesc->outElementType[id]) {
                case TIDL_UnsignedChar:
                    if(out[tidlrt_id]->elementType == TIDLRT_Uint8)
                        copy_asis(( uint8_t*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int8)
                        copy_asis((  int8_t*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint16)
                        copy_asis((uint16_t*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int16)
                        copy_asis(( int16_t*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint32)
                        copy_asis((uint32_t*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int32)
                        copy_asis(( int32_t*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Float32)
                        copy_asis((   float*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint64)
                        copy_asis(( uint64_t*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int64)
                        copy_asis(( int64_t*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else
                        return VX_FAILURE;
                    break;
                case TIDL_SignedChar:
                    if(out[tidlrt_id]->elementType == TIDLRT_Uint8)
                        copy_asis(( uint8_t*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int8)
                        copy_asis((  int8_t*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint16)
                        copy_asis((uint16_t*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int16)
                        copy_asis(( int16_t*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint32)
                        copy_asis((uint32_t*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int32)
                        copy_asis(( int32_t*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Float32)
                        copy_asis((   float*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint64)
                        copy_asis((uint64_t*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int64)
                        copy_asis(( int64_t*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else
                        return VX_FAILURE;
                    break;
                case TIDL_UnsignedShort:
                    if(out[tidlrt_id]->elementType == TIDLRT_Uint8)
                        copy_asis(( uint8_t*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int8)
                        copy_asis((  int8_t*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint16)
                        copy_asis((uint16_t*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int16)
                        copy_asis(( int16_t*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint32)
                        copy_asis((uint32_t*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int32)
                        copy_asis(( int32_t*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Float32)
                        copy_asis((   float*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint64)
                        copy_asis((uint64_t*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int64)
                        copy_asis(( int64_t*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else
                        return VX_FAILURE;
                    break;
                case TIDL_SignedShort:
                    if(out[tidlrt_id]->elementType == TIDLRT_Uint8)
                        copy_asis(( uint8_t*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int8)
                        copy_asis((  int8_t*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint16)
                        copy_asis((uint16_t*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int16)
                        copy_asis(( int16_t*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint32)
                        copy_asis((uint32_t*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int32)
                        copy_asis(( int32_t*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Float32)
                        copy_asis((   float*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint64)
                        copy_asis((uint64_t*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int64)
                        copy_asis(( int64_t*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else
                        return VX_FAILURE;
                    break;
                case TIDL_SinglePrecFloat:
                    if(out[tidlrt_id]->elementType == TIDLRT_Uint8)
                        copy_asis(( uint8_t*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int8)
                        copy_asis((  int8_t*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint16)
                        copy_asis((uint16_t*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int16)
                        copy_asis(( int16_t*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint32)
                        copy_asis((uint32_t*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int32)
                        copy_asis(( int32_t*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Float32)
                        copy_asis((   float*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint64)
                        copy_asis((uint64_t*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int64)
                        copy_asis(( int64_t*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w);
                    else
                        return VX_FAILURE;
                    break;
                default:
                    return VX_FAILURE;
            }
        } else if(zp == zp_iobuf) {
            switch(ioBufDesc->outElementType[id]) {
                case TIDL_UnsignedChar:
                    if(out[tidlrt_id]->elementType == TIDLRT_Uint8)
                        copy_scale(( uint8_t*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int8)
                        copy_scale((  int8_t*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint16)
                        copy_scale((uint16_t*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int16)
                        copy_scale(( int16_t*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint32)
                        copy_scale((uint32_t*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int32)
                        copy_scale(( int32_t*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Float32)
                        copy_scale((   float*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint64)
                        copy_scale((uint64_t*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int64)
                        copy_scale(( int64_t*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else
                        return VX_FAILURE;
                    break;
                case TIDL_SignedChar:
                    if(out[tidlrt_id]->elementType == TIDLRT_Uint8)
                        copy_scale(( uint8_t*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int8)
                        copy_scale((  int8_t*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint16)
                        copy_scale((uint16_t*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int16)
                        copy_scale(( int16_t*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint32)
                        copy_scale((uint32_t*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int32)
                        copy_scale(( int32_t*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Float32)
                        copy_scale((   float*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint64)
                        copy_scale((uint64_t*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int64)
                        copy_scale(( int64_t*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else
                        return VX_FAILURE;
                    break;
                case TIDL_UnsignedShort:
                    if(out[tidlrt_id]->elementType == TIDLRT_Uint8)
                        copy_scale(( uint8_t*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int8)
                        copy_scale((  int8_t*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint16)
                        copy_scale((uint16_t*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int16)
                        copy_scale(( int16_t*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint32)
                        copy_scale((uint32_t*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int32)
                        copy_scale(( int32_t*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Float32)
                        copy_scale((   float*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint64)
                        copy_scale((uint64_t*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int64)
                        copy_scale(( int64_t*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else
                        return VX_FAILURE;
                    break;
                case TIDL_SignedShort:
                    if(out[tidlrt_id]->elementType == TIDLRT_Uint8)
                        copy_scale(( uint8_t*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int8)
                        copy_scale((  int8_t*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint16)
                        copy_scale((uint16_t*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int16)
                        copy_scale(( int16_t*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint32)
                        copy_scale((uint32_t*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int32)
                        copy_scale(( int32_t*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Float32)
                        copy_scale((   float*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint64)
                        copy_scale((uint64_t*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int64)
                        copy_scale(( int64_t*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else
                        return VX_FAILURE;
                    break;
                case TIDL_SinglePrecFloat:
                    if(out[tidlrt_id]->elementType == TIDLRT_Uint8)
                        copy_scale(( uint8_t*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int8)
                        copy_scale((  int8_t*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint16)
                        copy_scale((uint16_t*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int16)
                        copy_scale(( int16_t*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint32)
                        copy_scale((uint32_t*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int32)
                        copy_scale(( int32_t*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Float32)
                        copy_scale((   float*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint64)
                        copy_scale((uint64_t*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int64)
                        copy_scale(( int64_t*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, scale, outScale);
                    else
                        return VX_FAILURE;
                    break;
                default:
                    return VX_FAILURE;
            }
        } else if(outScale == scale) {
            switch(ioBufDesc->outElementType[id]) {
                case TIDL_UnsignedChar:
                    if(out[tidlrt_id]->elementType == TIDLRT_Uint8)
                        copy_zf(( uint8_t*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int8)
                        copy_zf((  int8_t*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint16)
                        copy_zf((uint16_t*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int16)
                        copy_zf(( int16_t*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint32)
                        copy_zf((uint32_t*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int32)
                        copy_zf(( int32_t*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Float32)
                        copy_zf((   float*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint64)
                        copy_zf((uint64_t*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int64)
                        copy_zf(( int64_t*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else
                        return VX_FAILURE;
                    break;
                case TIDL_SignedChar:
                    if(out[tidlrt_id]->elementType == TIDLRT_Uint8)
                        copy_zf(( uint8_t*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int8)
                        copy_zf((  int8_t*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint16)
                        copy_zf((uint16_t*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int16)
                        copy_zf(( int16_t*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint32)
                        copy_zf((uint32_t*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int32)
                        copy_zf(( int32_t*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Float32)
                        copy_zf((   float*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint64)
                        copy_zf((uint64_t*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int64)
                        copy_zf(( int64_t*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else
                        return VX_FAILURE;
                    break;
                case TIDL_UnsignedShort:
                    if(out[tidlrt_id]->elementType == TIDLRT_Uint8)
                        copy_zf(( uint8_t*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int8)
                        copy_zf((  int8_t*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint16)
                        copy_zf((uint16_t*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int16)
                        copy_zf(( int16_t*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint32)
                        copy_zf((uint32_t*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int32)
                        copy_zf(( int32_t*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Float32)
                        copy_zf((   float*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint64)
                        copy_zf((uint64_t*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int64)
                        copy_zf(( int64_t*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else
                        return VX_FAILURE;
                    break;
                case TIDL_SignedShort:
                    if(out[tidlrt_id]->elementType == TIDLRT_Uint8)
                        copy_zf(( uint8_t*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int8)
                        copy_zf((  int8_t*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint16)
                        copy_zf((uint16_t*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int16)
                        copy_zf(( int16_t*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint32)
                        copy_zf((uint32_t*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int32)
                        copy_zf(( int32_t*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Float32)
                        copy_zf((   float*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint64)
                        copy_zf((uint64_t*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int64)
                        copy_zf(( int64_t*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else
                        return VX_FAILURE;
                    break;
                case TIDL_SinglePrecFloat:
                    if(out[tidlrt_id]->elementType == TIDLRT_Uint8)
                        copy_zf(( uint8_t*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int8)
                        copy_zf((  int8_t*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint16)
                        copy_zf((uint16_t*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int16)
                        copy_zf(( int16_t*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint32)
                        copy_zf((uint32_t*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int32)
                        copy_zf(( int32_t*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Float32)
                        copy_zf((   float*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint64)
                        copy_zf((uint64_t*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int64)
                        copy_zf(( int64_t*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp);
                    else
                        return VX_FAILURE;
                    break;
                default:
                    return VX_FAILURE;
            }
        } else {
            switch(ioBufDesc->outElementType[id]) {
                case TIDL_UnsignedChar:
                    if(out[tidlrt_id]->elementType == TIDLRT_Uint8)
                        copy_zfscale(( uint8_t*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int8)
                        copy_zfscale((  int8_t*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint16)
                        copy_zfscale((uint16_t*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int16)
                        copy_zfscale(( int16_t*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint32)
                        copy_zfscale((uint32_t*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int32)
                        copy_zfscale(( int32_t*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Float32)
                        copy_zfscale((   float*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint64)
                        copy_zfscale((uint64_t*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int64)
                        copy_zfscale(( int64_t*)rtPtr, ( uint8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else
                        return VX_FAILURE;
                    break;
                case TIDL_SignedChar:
                    if(out[tidlrt_id]->elementType == TIDLRT_Uint8)
                        copy_zfscale(( uint8_t*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int8)
                        copy_zfscale((  int8_t*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint16)
                        copy_zfscale((uint16_t*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int16)
                        copy_zfscale(( int16_t*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint32)
                        copy_zfscale((uint32_t*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int32)
                        copy_zfscale(( int32_t*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Float32)
                        copy_zfscale((   float*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint64)
                        copy_zfscale((uint64_t*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int64)
                        copy_zfscale(( int64_t*)rtPtr, (  int8_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else
                        return VX_FAILURE;
                    break;
                case TIDL_UnsignedShort:
                    if(out[tidlrt_id]->elementType == TIDLRT_Uint8)
                        copy_zfscale(( uint8_t*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int8)
                        copy_zfscale((  int8_t*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint16)
                        copy_zfscale((uint16_t*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int16)
                        copy_zfscale(( int16_t*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint32)
                        copy_zfscale((uint32_t*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int32)
                        copy_zfscale(( int32_t*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Float32)
                        copy_zfscale((   float*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint64)
                        copy_zfscale((uint64_t*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int64)
                        copy_zfscale(( int64_t*)rtPtr, (uint16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else
                        return VX_FAILURE;
                    break;
                case TIDL_SignedShort:
                    if(out[tidlrt_id]->elementType == TIDLRT_Uint8)
                        copy_zfscale(( uint8_t*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int8)
                        copy_zfscale((  int8_t*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint16)
                        copy_zfscale((uint16_t*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int16)
                        copy_zfscale(( int16_t*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint32)
                        copy_zfscale((uint32_t*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int32)
                        copy_zfscale(( int32_t*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Float32)
                        copy_zfscale((   float*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint64)
                        copy_zfscale((uint64_t*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int64)
                        copy_zfscale(( int64_t*)rtPtr, ( int16_t*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else
                        return VX_FAILURE;
                    break;
                case TIDL_SinglePrecFloat:
                    if(out[tidlrt_id]->elementType == TIDLRT_Uint8)
                        copy_zfscale(( uint8_t*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int8)
                        copy_zfscale((  int8_t*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint16)
                        copy_zfscale((uint16_t*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int16)
                        copy_zfscale(( int16_t*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint32)
                        copy_zfscale((uint32_t*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int32)
                        copy_zfscale(( int32_t*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Float32)
                        copy_zfscale((   float*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Uint64)
                        copy_zfscale((uint64_t*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else if(out[tidlrt_id]->elementType == TIDLRT_Int64)
                        copy_zfscale(( int64_t*)rtPtr, (   float*)ivPtr, bp, cp, lp, pp, offset, dst_bp, dst_cp, dst_lp, dst_pp, 0, b, c, h, w, -zp * scale / outScale, scale, outScale);
                    else
                        return VX_FAILURE;
                    break;
                default:
                    return VX_FAILURE;
            }
        }
    }
    return VX_SUCCESS;
}
}
