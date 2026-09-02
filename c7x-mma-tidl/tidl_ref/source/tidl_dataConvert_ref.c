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
@file    tidl_dataConvert_ref.c
@brief   This file defines functions for Data Convert reference layer.
@version 0.1 (May 2021) : Initial version [ADK]
----------------------------------------------------------------------------
*/

#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include <stdio.h>
#include "tidl_dataConvert_ref.h"
#include "tidl_dataConvert.h"



template <typename Tsrc, typename Tdst, typename Tscale, typename Tbias>
void TIDL_refDataConvertHighPrecision (
    Tsrc *src, Tdst *dst, int32_t src_offset, int32_t dst_offset,
    int32_t nb, int32_t nd1, int32_t nd2, int32_t nc, int32_t nl, int32_t np,
    int32_t src_bp, int32_t dst_bp, int32_t src_d1p, int32_t dst_d1p, int32_t src_d2p, int32_t dst_d2p, int32_t src_cp, int32_t dst_cp, int32_t src_lp, int32_t dst_lp, int32_t src_pp, int32_t dst_pp,
    Tscale scale, uint8_t shift, Tbias bias)
{
  // OPENACC(parallel loop)
  int32_t i0, i1, i2, i3, i4, i5;
  Tdst *d0 = dst + dst_offset, *d1, *d2, *d3, *d4, *d5;
  Tsrc *s0 = src + src_offset, *s1, *s2, *s3, *s4, *s5;
  Tdst data;
  Tbias acc;
  Tdst min = Tdst(), max = Tdst();
  max = std::numeric_limits<Tdst>::max();
  min = std::numeric_limits<Tdst>::lowest();

  for (i0 = 0; i0 < nb; i0++)
  {
    d1 = d0;
    s1 = s0;
    // OPENACC(loop seq)
    for (i1 = 0; i1 < nd1; i1++)
    {
      d2 = d1;
      s2 = s1;
      // OPENACC(loop seq)
      for (i2 = 0; i2 < nd2; i2++)
      {
        d3 = d2;
        s3 = s2;
        // OPENACC(loop seq)
        for (i3 = 0; i3 < nc; i3++)
        {
          d4 = d3;
          s4 = s3;
          // OPENACC(loop seq)
          for (i4 = 0; i4 < nl; i4++)
          {
            d5 = d4;
            s5 = s4;
            // OPENACC(loop seq)
            for (i5 = 0; i5 < np; i5++)
            {
              acc = (Tbias)(*s5);
              acc = acc * scale + bias;
              /*Input float & Output float*/
              if((typeid (Tdst) == typeid (float32_tidl)))
              {

                *d5 = tidl_sat<Tdst>(acc);
              }
              else if (typeid (Tsrc) == typeid (float32_tidl))
              {
                if (acc > 0)
                {
                  *d5 = tidl_sat<Tdst>(acc + 0.5);
                }
                else
                {
                  *d5 = tidl_sat<Tdst>(acc - 0.5);
                }
              }
              else 
              {
                *d5 = TIDL_roundSat(acc, shift, min, max);
              }
              s5 += src_pp;
              d5 += dst_pp;
            }
            s4 += src_lp;
            d4 += dst_lp;
          }
          s3 += src_cp;
          d3 += dst_cp;
        }
        s2 += src_d2p;
        d2 += dst_d2p;
      }
      s1 += src_d1p;
      d1 += dst_d1p;
    }
    s0 += src_bp;
    d0 += dst_bp;
  }
}

/**
 * @brief This is reference implementation of Data Convert layer
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
 * @param inLinePitch : pitch of the input buffer
 * @param outLinePitch : pitch of the output buffer
 * @param inChPitch : Chnnel pitch of the input buffer
 * @param outChPitch : Chnnel pitch of the output buffer
 */

template<typename Tsrc, typename Tdst, typename Tacc, typename Tscale>
    //: TODO:  Keep bias container different from accumulator
    void TIDL_refDataConvertNew(Tsrc *src, Tdst *dst, int32_t src_offset, int32_t dst_offset,
                                int32_t nb, int32_t nd1, int32_t nd2, int32_t nc, int32_t nl, int32_t np,
                                int32_t src_bp, int32_t dst_bp, int32_t src_d1p, int32_t dst_d1p, int32_t src_d2p, int32_t dst_d2p, int32_t src_cp, int32_t dst_cp, int32_t src_lp, int32_t dst_lp, int32_t src_pp, int32_t dst_pp,
                                Tacc bias, Tscale scale, int32_t shift)
{

  int32_t i0, i1, i2, i3, i4, i5;
  Tdst *d0 = dst + dst_offset, *d1, *d2, *d3, *d4, *d5;
  Tsrc *s0 = src + src_offset, *s1, *s2, *s3, *s4, *s5;
  Tacc data, val;
  Tdst min = Tdst(), max = Tdst();
  max = std::numeric_limits<Tdst>::max();
  min = std::numeric_limits<Tdst>::lowest();

  if (typeid(Tacc) == typeid(float32_tidl))
  {
    float32_tidl q1, q2;
    (void)TIDL_dataConvertSplitBias((float32_tidl)bias,
                                    &q1,
                                    &q2);
    bias = q1 * q2;
  }
  else if (typeid(Tacc) == typeid(int64_t)) /* 16 bit case */
  {
    int16_t q1; /* Keep type of q1 and scale as signed in order to handle
     negative  value of zero point */
    int16_t q2; /* Type of q2 is determined by source */

    if ((typeid(Tsrc) == typeid(int16_t)) ||
        (typeid(Tsrc) == typeid(int8_t)))
    {
      (void)TIDL_dataConvertSplitBias((int64_t)bias,
                                      &q1,
                                      (int16_t *)&q2);
    }
    else
    {
      (void)TIDL_dataConvertSplitBias((int64_t)bias,
                                      &q1,
                                      (uint16_t *)&q2);
    }
    bias = q1 * q2;
  }
  else /* 8 bit case */
  {
    int8_t q1; /* Keep type of q1 and scale as signed in order to handle
     negative  value of zero point */
    Tsrc q2;   /* Type of q2 is determined by source */

    if (typeid(Tsrc) == typeid(int8_t))
    {
      (void)TIDL_dataConvertSplitBias((int32_t)bias,
                                      &q1,
                                      (int8_t *)&q2);
    }
    else
    {
      (void)TIDL_dataConvertSplitBias((int32_t)bias,
                                      &q1,
                                      (uint8_t *)&q2);
    }
    bias = q1 * q2;
  }

  // OPENACC(parallel loop)
  for (i0 = 0; i0 < nb; i0++)
  {
    d1 = d0;
    s1 = s0;
    // OPENACC(loop seq)
    for (i1 = 0; i1 < nd1; i1++)
    {
      d2 = d1;
      s2 = s1;
      // OPENACC(loop seq)
      for (i2 = 0; i2 < nd2; i2++)
      {
        d3 = d2;
        s3 = s2;
        // OPENACC(loop seq)
        for (i3 = 0; i3 < nc; i3++)
        {
          d4 = d3;
          s4 = s3;
          // OPENACC(loop seq)
          for (i4 = 0; i4 < nl; i4++)
          {
            d5 = d4;
            s5 = s4;
            // OPENACC(loop seq)
            for (i5 = 0; i5 < np; i5++)
            {
              data = (Tacc)*s5;
              val = ((data * scale) + bias);
              if (typeid(Tacc) == typeid(float32_tidl))
              {
                if (typeid(Tdst) != typeid(float32_tidl))
                {
                  if (val > 0)
                  {
                    *d5 = tidl_sat<Tdst>(val + 0.5);
                  }
                  else
                  {
                    *d5 = tidl_sat<Tdst>(val - 0.5);
                  }
                }
                else
                {
                  *d5 = tidl_sat<Tdst>(val);
                }
              }
              else
              {
                *d5 = TIDL_roundSat((int64_t)val, shift, (int32_t)min, (int32_t)max);
              }
              s5 += src_pp;
              d5 += dst_pp;
            }
            d4 += dst_lp;
            s4 += src_lp;
          }
          d3 += dst_cp;
          s3 += src_cp;
        }
        d2 += dst_d2p;
        s2 += src_d2p;
      }
      d1 += dst_d1p;
      s1 += src_d1p;
    }
    d0 += dst_bp;
    s0 += src_bp;
  }
}

template<typename Tsrc, typename Tdst>
    int32_t TIDL_refDataConvertTemp(Tsrc *src, Tdst *dst, int32_t src_offset, int32_t dst_offset,
                                    int32_t nb, int32_t nd1, int32_t nd2, int32_t nc, int32_t nl, int32_t np,
                                    int32_t src_bp, int32_t dst_bp, int32_t src_d1p, int32_t dst_d1p, int32_t src_d2p, int32_t dst_d2p, int32_t src_cp, int32_t dst_cp, int32_t src_lp, int32_t dst_lp, int32_t src_pp, int32_t dst_pp,
                                    float in_zf, float out_zf, float in_scale, float out_scale, int32_t shift, 
                                    int32_t isKernelHighPrecision, int32_t type)
{
  float32_tidl scale, bias;
  int64_t scaleFixed, biasFixed;
  int32_t status = TIDL_SUCCESS;

  if (isKernelHighPrecision == TRUE &&
      (!((typeid(Tsrc) == typeid(float32_tidl)) &&
         (typeid(Tdst) == typeid(float32_tidl)))) &&
      (type == TIDL_DC_TYPE_OUTPUT ||
      type == TIDL_DC_TYPE_PTQ_TO_NATIVE   ||
      type == TIDL_DC_TYPE_NATIVE_TO_PTQ ))
  {
    if ((typeid(Tsrc) == typeid(float32_tidl)) ||
        (typeid(Tdst) == typeid(float32_tidl)))
    {
      float32_tidl scale, bias;
      uint8_t shift = 0;
      scale = out_scale/in_scale;
      shift = 0;
      bias = out_zf - (scale * in_zf);
      TIDL_refDataConvertHighPrecision (
          src, dst, src_offset, dst_offset,
          nb, nd1, nd2, nc, nl, np, src_bp, dst_bp, src_d1p, dst_d1p, src_d2p, dst_d2p, src_cp, dst_cp, src_lp, dst_lp, src_pp, dst_pp,
          (float32_tidl)scale, shift, (float32_tidl)bias);
    }
    else
    {
      int64_t bias = 0;
      float32_tidl scaleRatio = out_scale/in_scale;
      
      if ((sizeof(Tsrc) == 2U) ||
              (sizeof(Tdst) == 2U))
      {
        int16_t scale = 0;
        uint8_t shift = 0;
        TIDL_getMMAv2_ScaleShiftAndError(scaleRatio, &scale, &shift, (int32_t)(sizeof(Tsrc)*8U), (float32_tidl)255.0f, true);
        float32_tidl biasFloat = scale * (out_zf / scaleRatio - in_zf);
        bias = round(biasFloat);
        TIDL_refDataConvertHighPrecision (
            src, dst, src_offset, dst_offset,
            nb, nd1, nd2, nc, nl, np, src_bp, dst_bp, src_d1p, dst_d1p, src_d2p, dst_d2p, src_cp, dst_cp, src_lp, dst_lp, src_pp, dst_pp,
            (int16_t)scale, shift, (int64_t)bias);
      }
      else /* 8 bit case */
      {
        int8_t scale = 0;
        uint8_t shift = 0;
        TIDL_getMMAv2_ScaleShiftAndError(scaleRatio, &scale, &shift, (int32_t)(sizeof(Tsrc)*8U), (float32_tidl)255.0f, true);
        float32_tidl biasFloat = scale * (out_zf / scaleRatio - in_zf);
        bias = round(biasFloat);
        TIDL_refDataConvertHighPrecision (
            src, dst, src_offset, dst_offset,
            nb, nd1, nd2, nc, nl, np, src_bp, dst_bp, src_d1p, dst_d1p, src_d2p, dst_d2p, src_cp, dst_cp, src_lp, dst_lp, src_pp, dst_pp,
            (int8_t)scale, shift, (int32_t)bias);
      }
    } 
    
  }
  else 
  {
    TIDL_dataConvertGetScaleAndBias(in_scale,
                                    out_scale,
                                    in_zf,
                                    out_zf,
                                    shift,
                                    &scale,
                                    &bias,
                                    &scaleFixed,
                                    &biasFixed);

    if ((typeid(Tsrc) == typeid(float32_tidl)) ||
        (typeid(Tdst) == typeid(float32_tidl)))
    {
      TIDL_refDataConvertNew(
          src, dst, src_offset, dst_offset,
          nb, nd1, nd2, nc, nl, np, src_bp, dst_bp, src_d1p, dst_d1p, src_d2p, dst_d2p, src_cp, dst_cp, src_lp, dst_lp, src_pp, dst_pp,
          bias, scale, 0);
    }
    else if ((sizeof(Tsrc) == 2U) ||
            (sizeof(Tdst) == 2U))
    {
      int16_t max = 0;
      max = std::numeric_limits<int16_t>::max();
      int16_t min = 0;
      min = std::numeric_limits<int16_t>::lowest();
      scaleFixed = TIDL_roundSat((int64_t)scaleFixed, 0, (int32_t)min, (int32_t)max);
      TIDL_refDataConvertNew(
          src, dst, src_offset, dst_offset,
          nb, nd1, nd2, nc, nl, np, src_bp, dst_bp, src_d1p, dst_d1p, src_d2p, dst_d2p, src_cp, dst_cp, src_lp, dst_lp, src_pp, dst_pp,
          (int64_t)biasFixed, (int16_t)scaleFixed, shift);
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
    This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
    TIDL_LDRA_TAG : TIDL_LDRA_TAG_DATACONVERT_PRIOR_CHECK_001
    Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
    This does not affect runtime behavior or safety.
    <justification end> */
    else if ((sizeof(Tsrc) == (uint32_t)1) ||
            (sizeof(Tdst) == (uint32_t)1))
    {
      /* LDRA_JUSTIFY_END */
      int8_t max = 0;
      max = std::numeric_limits<int8_t>::max();
      int8_t min = 0;
      min = std::numeric_limits<int8_t>::lowest();
      scaleFixed = TIDL_roundSat((int64_t)scaleFixed, 0, (int32_t)min, (int32_t)max);
      TIDL_refDataConvertNew(
          src, dst, src_offset, dst_offset,
          nb, nd1, nd2, nc, nl, np, src_bp, dst_bp, src_d1p, dst_d1p, src_d2p, dst_d2p, src_cp, dst_cp, src_lp, dst_lp, src_pp, dst_pp,
          (int32_t)biasFixed, (int8_t)scaleFixed, shift);
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
    This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
    TIDL_LDRA_TAG : TIDL_LDRA_TAG_DATACONVERT_PRIOR_CHECK_001
    Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
    This does not affect runtime behavior or safety.
    <justification end> */
    else
    {
      status = TIDL_ERR_INVALID_TYPE;
      tidl_printf(0, " Un-supported Data type in %s %d \n", __FILE__, __LINE__);
  #if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
      /* Word and Double word data type, expected to only
      handle Argmax layer where scale is expected to be 1 and bias 0*/
      TIDL_refDataConvertNew(
          src, dst, src_offset, dst_offset,
          nb, nd1, nd2, nc, nl, np, src_bp, dst_bp, src_d1p, dst_d1p, src_d2p, dst_d2p, src_cp, dst_cp, src_lp, dst_lp, src_pp, dst_pp,
          (int32_t)0, (int16_t)1, 0);
  #endif
    }
  }

  /* LDRA_JUSTIFY_END */
  return status;
}

template<typename Tsrc, typename Tdst>
    int32_t TIDL_refDataConvert(Tsrc *src, Tdst *dst, int32_t src_offset, int32_t dst_offset,
                                int32_t nb, int32_t nd1, int32_t nd2, int32_t nc, int32_t nl, int32_t np,
                                int32_t src_bp, int32_t dst_bp, int32_t src_d1p, int32_t dst_d1p, int32_t src_d2p, int32_t dst_d2p, int32_t src_cp, int32_t dst_cp, int32_t src_lp, int32_t dst_lp, int32_t src_pp, int32_t dst_pp,
                                float in_zf, float out_zf, float in_scale, float out_scale)
{
  int32_t status = TIDL_SUCCESS;
  int32_t i0, i1, i2, i3, i4;
  Tdst *d0 = dst + dst_offset, *d1, *d2, *d3, *d4;
  Tsrc *s0 = src + src_offset, *s1, *s2, *s3, *s4;
  float data, scale = out_scale / in_scale;

  // OPENACC(parallel loop)
  for (i0 = 0; i0 < nd1; i0++)
  {
    d1 = d0;
    s1 = s0;
    // OPENACC(loop seq)
    for (i1 = 0; i1 < nd2; i1++)
    {
      d2 = d1;
      s2 = s1;
      // OPENACC(loop seq)
      for (i2 = 0; i2 < nc; i2++)
      {
        d3 = d2;
        s3 = s2;
        // OPENACC(loop seq)
        for (i3 = 0; i3 < nl; i3++)
        {
          d4 = d3;
          s4 = s3;
          // OPENACC(loop seq)
          for (i4 = 0; i4 < np; i4++)
          {
            data = *s4;
            *d4 = tidl_sat<Tdst>(((data - in_zf) * scale) + out_zf);
            s4 += src_pp;
            d4 += dst_pp;
          }
          s3 += src_lp;
          d3 += dst_lp;
        }
        s2 += src_cp;
        d2 += dst_cp;
      }
      d1 += dst_d2p;
      s1 += src_d2p;
    }
    d0 += dst_d1p;
    s0 += src_d1p;
  }
  return status;
}

template<typename Tsrc>
    int32_t TIDL_refDataConvertVarOutType(Tsrc *src, void *dst, int32_t src_offset, int32_t dst_offset,
                                          int32_t nb, int32_t nd1, int32_t nd2, int32_t nc, int32_t nl, int32_t np,
                                          int32_t src_bp, int32_t dst_bp, int32_t src_d1p, int32_t dst_d1p, int32_t src_d2p, int32_t dst_d2p, int32_t src_cp, int32_t dst_cp, int32_t src_lp, int32_t dst_lp, int32_t src_pp, int32_t dst_pp,
                                          float in_zf, float out_zf, float in_scale, float out_scale, int32_t outElemenType, int32_t shift,
                                          int32_t isKernelHighPrecision, int32_t type)
{
  int32_t status = TIDL_SUCCESS;
  if (outElemenType == TIDL_SinglePrecFloat)
  {
    status = TIDL_refDataConvertTemp(
        src, (float32_tidl *)dst, src_offset, dst_offset,
        nb, nd1, nd2, nc, nl, np, src_bp, dst_bp, src_d1p, dst_d1p, src_d2p, dst_d2p, src_cp, dst_cp, src_lp, dst_lp, src_pp, dst_pp,
        in_zf, out_zf, in_scale, out_scale, shift, isKernelHighPrecision, type);
  }
  else if (outElemenType == TIDL_SignedChar)
  {
    status = TIDL_refDataConvertTemp(
        src, (int8_t *)dst, src_offset, dst_offset,
        nb, nd1, nd2, nc, nl, np, src_bp, dst_bp, src_d1p, dst_d1p, src_d2p, dst_d2p, src_cp, dst_cp, src_lp, dst_lp, src_pp, dst_pp,
        in_zf, out_zf, in_scale, out_scale, shift, isKernelHighPrecision, type);
  }
  else if (outElemenType == TIDL_SignedShort)
  {
    status = TIDL_refDataConvertTemp(
        src, (int16_t *)dst, src_offset, dst_offset,
        nb, nd1, nd2, nc, nl, np, src_bp, dst_bp, src_d1p, dst_d1p, src_d2p, dst_d2p, src_cp, dst_cp, src_lp, dst_lp, src_pp, dst_pp,
        in_zf, out_zf, in_scale, out_scale, shift, isKernelHighPrecision, type);
  }
  else if (outElemenType == TIDL_SignedWord)
  {
    status = TIDL_refDataConvertTemp(
        src, (int32_t *)dst, src_offset, dst_offset,
        nb, nd1, nd2, nc, nl, np, src_bp, dst_bp, src_d1p, dst_d1p, src_d2p, dst_d2p, src_cp, dst_cp, src_lp, dst_lp, src_pp, dst_pp,
        in_zf, out_zf, in_scale, out_scale, shift, isKernelHighPrecision, type);
  }
  else if (outElemenType == TIDL_UnsignedChar)
  {
    status = TIDL_refDataConvertTemp(
        src, (uint8_t *)dst, src_offset, dst_offset,
        nb, nd1, nd2, nc, nl, np, src_bp, dst_bp, src_d1p, dst_d1p, src_d2p, dst_d2p, src_cp, dst_cp, src_lp, dst_lp, src_pp, dst_pp,
        in_zf, out_zf, in_scale, out_scale, shift, isKernelHighPrecision, type);
  }
  else if (outElemenType == TIDL_UnsignedShort)
  {
    status = TIDL_refDataConvertTemp(
        src, (uint16_t *)dst, src_offset, dst_offset,
        nb, nd1, nd2, nc, nl, np, src_bp, dst_bp, src_d1p, dst_d1p, src_d2p, dst_d2p, src_cp, dst_cp, src_lp, dst_lp, src_pp, dst_pp,
        in_zf, out_zf, in_scale, out_scale, shift, isKernelHighPrecision, type);
  }
  else if (outElemenType == TIDL_UnsignedWord)
  {
    status = TIDL_refDataConvertTemp(
        src, (uint32_t *)dst, src_offset, dst_offset,
        nb, nd1, nd2, nc, nl, np, src_bp, dst_bp, src_d1p, dst_d1p, src_d2p, dst_d2p, src_cp, dst_cp, src_lp, dst_lp, src_pp, dst_pp,
        in_zf, out_zf, in_scale, out_scale, shift, isKernelHighPrecision, type);
  }
  else if (outElemenType == TIDL_SignedDoubleWord)
  {
    status = TIDL_refDataConvert(
        src, (int64_t *)dst, src_offset, dst_offset,
        nb, nd1, nd2, nc, nl, np, src_bp, dst_bp, src_d1p, dst_d1p, src_d2p, dst_d2p, src_cp, dst_cp, src_lp, dst_lp, src_pp, dst_pp,
        in_zf, out_zf, in_scale, out_scale);
  }
  else
  {
    tidl_printf(0, " Un-supported Data type in %s %d \n", __FILE__, __LINE__);
    status = TIDL_ERR_INVALID_TYPE;
  }
  return status;
}

/**
 * @brief To find float min and max in the tensor
 *
 * @param ptr : pointer to tensor data
 * @param dataPrms : parametes of the data buffer
 * @param min : to store the min value
 * @param max : to store the max value
 */
template<class Tsrc> void TIDL_TensorMinMaxinFloat(const Tsrc *ptr, const sTIDL_DataParams_t *dataPrms, float in_zf, float in_scale, float *min, float *max)
{
  int32_t padOffset = dataPrms->padW + (dataPrms->padH * dataPrms->pitch[TIDL_LINE_PITCH]);
  int32_t i0, i1, i2, i3, i4, i5;
  float val;
  *min = std::numeric_limits<float>::max();
  *max = -1 * std::numeric_limits<float>::max();

  OPENACC(data copyin(ptr[:1 + padOffset + ((dataPrms->dimValues[TIDL_DIM_BATCH] - 1) * dataPrms->pitch[TIDL_ROI_PITCH]) + ((dataPrms->dimValues[TIDL_DIM_DIM1] - 1) * dataPrms->pitch[TIDL_DIM1_PITCH]) +
                           ((dataPrms->dimValues[TIDL_DIM_DIM2] - 1) * dataPrms->pitch[TIDL_DIM2_PITCH]) + ((dataPrms->dimValues[TIDL_DIM_NUMCH] - 1) * dataPrms->pitch[TIDL_CHANNEL_PITCH]) +
                           ((dataPrms->dimValues[TIDL_DIM_HEIGHT] - 1) * dataPrms->pitch[TIDL_LINE_PITCH]) + (dataPrms->dimValues[TIDL_DIM_WIDTH] - 1)]))
  OPENACC(parallel loop collapse(6))
  for (i0 = 0; i0 < dataPrms->dimValues[TIDL_DIM_BATCH]; i0++)
  {
    for (i1 = 0; i1 < dataPrms->dimValues[TIDL_DIM_DIM1]; i1++)
    {
      for (i2 = 0; i2 < dataPrms->dimValues[TIDL_DIM_DIM2]; i2++)
      {
        for (i3 = 0; i3 < dataPrms->dimValues[TIDL_DIM_NUMCH]; i3++)
        {
          for (i4 = 0; i4 < dataPrms->dimValues[TIDL_DIM_HEIGHT]; i4++)
          {
            for (i5 = 0; i5 < dataPrms->dimValues[TIDL_DIM_WIDTH]; i5++)
            {
              val = ptr[padOffset + (i0 * dataPrms->pitch[TIDL_ROI_PITCH]) + (i1 * dataPrms->pitch[TIDL_DIM1_PITCH]) + (i2 * dataPrms->pitch[TIDL_DIM2_PITCH]) + (i3 * dataPrms->pitch[TIDL_CHANNEL_PITCH]) + (i4 * dataPrms->pitch[TIDL_LINE_PITCH]) + i5];
              val = val - in_zf;
              val = val / in_scale;
              *min = (val < *min) ? val : *min;
              *max = (val > *max) ? val : *max;
            }
          }
        }
      }
    }
  }
}
static int32_t TIDL_bf16DataConvertRefProcess(
    int32_t inElemType, int32_t outElemType, uint32_t dcType,
    int32_t nb, int32_t nd1, int32_t nd2, int32_t nc, int32_t nl, int32_t np,
    int32_t src_bp, int32_t dst_bp,
    int32_t src_d1p, int32_t dst_d1p,
    int32_t src_d2p, int32_t dst_d2p,
    int32_t src_cp, int32_t dst_cp,
    int32_t src_lp, int32_t dst_lp,
    int32_t src_pp, int32_t dst_pp,
    void *inPtrs[], void *outPtrs[])
{
  int32_t src_elem_size = 1;
  if ((inElemType == TIDL_SinglePrecFloat) || (inElemType == TIDL_SignedWord) ||
      (inElemType == TIDL_UnsignedWord))
  {
    src_elem_size = 4;
  }
  else if ((inElemType == TIDL_BFloat16) || (inElemType == TIDL_SignedShort) ||
           (inElemType == TIDL_UnsignedShort))
  {
    src_elem_size = 2;
  }
  else if ((inElemType == TIDL_SignedDoubleWord) || (inElemType == TIDL_UnsignedDoubleWord))
  {
    src_elem_size = 8;
  }
  else
  {
    src_elem_size = 1;
  }

  int32_t dst_elem_size = 2; /* BFloat16, SignedShort, UnsignedShort */
  if ((outElemType == TIDL_SinglePrecFloat) ||
      (outElemType == TIDL_SignedWord)      ||
      (outElemType == TIDL_UnsignedWord))
  {
    dst_elem_size = 4;
  }
  else if ((outElemType == TIDL_SignedDoubleWord) || (outElemType == TIDL_UnsignedDoubleWord))
  {
    dst_elem_size = 8;
  }
  else if ((outElemType == TIDL_SignedChar) || (outElemType == TIDL_UnsignedChar))
  {
    dst_elem_size = 1;
  }

  /* Convert element-unit pitches (already NHWC-adjusted by caller) to byte-unit pitches */
  int32_t src_bp_b  = src_bp  * src_elem_size;
  int32_t dst_bp_b  = dst_bp  * dst_elem_size;
  int32_t src_d1p_b = src_d1p * src_elem_size;
  int32_t dst_d1p_b = dst_d1p * dst_elem_size;
  int32_t src_d2p_b = src_d2p * src_elem_size;
  int32_t dst_d2p_b = dst_d2p * dst_elem_size;
  int32_t src_cp_b  = src_cp  * src_elem_size;
  int32_t dst_cp_b  = dst_cp  * dst_elem_size;
  int32_t src_lp_b  = src_lp  * src_elem_size;
  int32_t dst_lp_b  = dst_lp  * dst_elem_size;
  int32_t src_pp_b  = src_pp  * src_elem_size;
  int32_t dst_pp_b  = dst_pp  * dst_elem_size;

  int32_t i0, i1, i2, i3, i4, i5;
  uint8_t *s0 = (uint8_t *)inPtrs[0],  *s1, *s2, *s3, *s4, *s5;
  uint8_t *d0 = (uint8_t *)outPtrs[0], *d1, *d2, *d3, *d4, *d5;

  for (i0 = 0; i0 < nb; i0++)
  {
    d1 = d0;
    s1 = s0;
    for (i1 = 0; i1 < nd1; i1++)
    {
      d2 = d1;
      s2 = s1;
      for (i2 = 0; i2 < nd2; i2++)
      {
        d3 = d2;
        s3 = s2;
        for (i3 = 0; i3 < nc; i3++)
        {
          d4 = d3;
          s4 = s3;
          for (i4 = 0; i4 < nl; i4++)
          {
            d5 = d4;
            s5 = s4;
            for (i5 = 0; i5 < np; i5++)
            {
              if (dcType == TIDL_DC_TYPE_INPUT)
              {
                /* Any type -> BFloat16 */
                float32_tidl src_val = 0.0f;
                if (inElemType == TIDL_SinglePrecFloat)
                {
                  src_val = *(float32_tidl *)s5;
                }
                else if (inElemType == TIDL_BFloat16)
                {
                  src_val = (float32_tidl)(*(bfloat16_tidl *)s5);
                }
                else if (inElemType == TIDL_SignedChar)
                {
                  src_val = (float32_tidl)(*(int8_t *)s5);
                }
                else if (inElemType == TIDL_UnsignedChar)
                {
                  src_val = (float32_tidl)(*(uint8_t *)s5);
                }
                else if (inElemType == TIDL_SignedShort)
                {
                  src_val = (float32_tidl)(*(int16_t *)s5);
                }
                else if (inElemType == TIDL_UnsignedShort)
                {
                  src_val = (float32_tidl)(*(uint16_t *)s5);
                }
                else if (inElemType == TIDL_SignedWord)
                {
                  src_val = (float32_tidl)(*(int32_t *)s5);
                }
                else if (inElemType == TIDL_UnsignedWord)
                {
                  src_val = (float32_tidl)(*(uint32_t *)s5);
                }
                else if (inElemType == TIDL_SignedDoubleWord)
                {
                  src_val = (float32_tidl)(*(int64_t *)s5);
                }
                else if (inElemType == TIDL_UnsignedDoubleWord)
                {
                  src_val = (float32_tidl)(*(uint64_t *)s5);
                }
                *(bfloat16_tidl *)d5 = tidl_sat_bf16(bfloat16_tidl(src_val));
              }
              else if (dcType == TIDL_DC_TYPE_OUTPUT)
              {
                /* BFloat16 -> any type */
                float32_tidl val = (float32_tidl)(*(bfloat16_tidl *)s5);
                if (outElemType == TIDL_SinglePrecFloat)
                {
                  *(float32_tidl *)d5 = val;
                }
                else if (outElemType == TIDL_BFloat16)
                {
                  *(bfloat16_tidl *)d5 = bfloat16_tidl(val);
                }
                else if (outElemType == TIDL_SignedChar)
                {
                  *(int8_t *)d5 = tidl_sat<int8_t>(val > 0 ? val + 0.5f : val - 0.5f);
                }
                else if (outElemType == TIDL_UnsignedChar)
                {
                  *(uint8_t *)d5 = tidl_sat<uint8_t>(val > 0 ? val + 0.5f : val - 0.5f);
                }
                else if (outElemType == TIDL_SignedShort)
                {
                  *(int16_t *)d5 = tidl_sat<int16_t>(val > 0 ? val + 0.5f : val - 0.5f);
                }
                else if (outElemType == TIDL_UnsignedShort)
                {
                  *(uint16_t *)d5 = tidl_sat<uint16_t>(val > 0 ? val + 0.5f : val - 0.5f);
                }
                else if (outElemType == TIDL_SignedWord)
                {
                  *(int32_t *)d5 = tidl_sat<int32_t>(val > 0 ? val + 0.5f : val - 0.5f);
                }
                else if (outElemType == TIDL_UnsignedWord)
                {
                  *(uint32_t *)d5 = tidl_sat<uint32_t>(val > 0 ? val + 0.5f : val - 0.5f);
                }
                else if (outElemType == TIDL_SignedDoubleWord)
                {
                  *(int64_t *)d5 = (int64_t)val;
                }
                else if (outElemType == TIDL_UnsignedDoubleWord)
                {
                  *(uint64_t *)d5 = (uint64_t)val;
                }
                else
                {
                  *(float32_tidl *)d5 = val;
                }
              }
              else
              {
                /* Intermediate: TIDL_DC_TYPE_PTQ_TO_NATIVE, TIDL_DC_TYPE_NATIVE_TO_PTQ,
                 * or layout conversion (NCHW <-> NHWC).
                 * Dispatch on which side is BFloat16. */
                if (inElemType == TIDL_BFloat16 && outElemType == TIDL_BFloat16)
                {
                  /* BF16 -> BF16: layout conversion or saturation-only pass */
                  *(bfloat16_tidl *)d5 = tidl_sat_bf16(*(bfloat16_tidl *)s5);
                }
                else if (outElemType == TIDL_BFloat16)
                {
                  /* Any -> BF16: PTQ_TO_NATIVE (quantized -> network-native BF16) */
                  float32_tidl src_val = 0.0f;
                  if (inElemType == TIDL_SinglePrecFloat)
                  {
                    src_val = *(float32_tidl *)s5;
                  }
                  else if (inElemType == TIDL_SignedChar)
                  {
                    src_val = (float32_tidl)(*(int8_t *)s5);
                  }
                  else if (inElemType == TIDL_UnsignedChar)
                  {
                    src_val = (float32_tidl)(*(uint8_t *)s5);
                  }
                  else if (inElemType == TIDL_SignedShort)
                  {
                    src_val = (float32_tidl)(*(int16_t *)s5);
                  }
                  else if (inElemType == TIDL_UnsignedShort)
                  {
                    src_val = (float32_tidl)(*(uint16_t *)s5);
                  }
                  else if (inElemType == TIDL_SignedWord)
                  {
                    src_val = (float32_tidl)(*(int32_t *)s5);
                  }
                  else if (inElemType == TIDL_UnsignedWord)
                  {
                    src_val = (float32_tidl)(*(uint32_t *)s5);
                  }
                  else if (inElemType == TIDL_SignedDoubleWord)
                  {
                    src_val = (float32_tidl)(*(int64_t *)s5);
                  }
                  else if (inElemType == TIDL_UnsignedDoubleWord)
                  {
                    src_val = (float32_tidl)(*(uint64_t *)s5);
                  }
                  *(bfloat16_tidl *)d5 = tidl_sat_bf16(bfloat16_tidl(src_val));
                }
                else
                {
                  /* BF16 -> Any: NATIVE_TO_PTQ (network-native BF16 -> quantized) */
                  float32_tidl val = (float32_tidl)(*(bfloat16_tidl *)s5);
                  if (outElemType == TIDL_SinglePrecFloat)
                  {
                    *(float32_tidl *)d5 = val;
                  }
                  else if (outElemType == TIDL_SignedChar)
                  {
                    *(int8_t *)d5 = tidl_sat<int8_t>(val > 0 ? val + 0.5f : val - 0.5f);
                  }
                  else if (outElemType == TIDL_UnsignedChar)
                  {
                    *(uint8_t *)d5 = tidl_sat<uint8_t>(val > 0 ? val + 0.5f : val - 0.5f);
                  }
                  else if (outElemType == TIDL_SignedShort)
                  {
                    *(int16_t *)d5 = tidl_sat<int16_t>(val > 0 ? val + 0.5f : val - 0.5f);
                  }
                  else if (outElemType == TIDL_UnsignedShort)
                  {
                    *(uint16_t *)d5 = tidl_sat<uint16_t>(val > 0 ? val + 0.5f : val - 0.5f);
                  }
                  else if (outElemType == TIDL_SignedWord)
                  {
                    *(int32_t *)d5 = tidl_sat<int32_t>(val > 0 ? val + 0.5f : val - 0.5f);
                  }
                  else if (outElemType == TIDL_UnsignedWord)
                  {
                    *(uint32_t *)d5 = tidl_sat<uint32_t>(val > 0 ? val + 0.5f : val - 0.5f);
                  }
                  else if (outElemType == TIDL_SignedDoubleWord)
                  {
                    *(int64_t *)d5 = (int64_t)val;
                  }
                  else if (outElemType == TIDL_UnsignedDoubleWord)
                  {
                    *(uint64_t *)d5 = (uint64_t)val;
                  }
                  else
                  {
                    *(float32_tidl *)d5 = val;
                  }
                }
              }
              s5 += src_pp_b;
              d5 += dst_pp_b;
            }
            s4 += src_lp_b;
            d4 += dst_lp_b;
          }
          s3 += src_cp_b;
          d3 += dst_cp_b;
        }
        s2 += src_d2p_b;
        d2 += dst_d2p_b;
      }
      s1 += src_d1p_b;
      d1 += dst_d1p_b;
    }
    s0 += src_bp_b;
    d0 += dst_bp_b;
  }

  return TIDL_SUCCESS;
}

static int32_t TIDL_dataConvertRefProcess(TIDL_Handle intAlgHandle,
                                          sTIDL_AlgLayer_t *algLayer,
                                          const sTIDL_Layer_t *tidlLayer,
                                          void *inPtrs[],
                                          void *outPtrs[])
{

  sTIDL_DataParams_t *inDataParams = &intAlgHandle->createParams->net->TIDLLayers[algLayer->inLayerIdx[0]].outData;
  const sTIDL_dataConvertParams_t *params = &tidlLayer->layerParams.dataConvertParams;
  int32_t isKernelHighPrecision = TIDL_isKernelHighPrecision(tidlLayer->layerKernelType);
  int32_t outWidth = tidlLayer->outData.dimValues[TIDL_DIM_WIDTH];
  int32_t outHeight = tidlLayer->outData.dimValues[TIDL_DIM_HEIGHT];
  int32_t inChs = inDataParams->dimValues[TIDL_DIM_NUMCH];
  int32_t indim2 = inDataParams->dimValues[TIDL_DIM_DIM2];
  int32_t indim1 = inDataParams->dimValues[TIDL_DIM_DIM1];
  int32_t inBhs = inDataParams->dimValues[TIDL_DIM_BATCH];
  int32_t inPitch = (int32_t)inDataParams->pitch[TIDL_LINE_PITCH];
  int32_t outPitch = (int32_t)tidlLayer->outData.pitch[TIDL_LINE_PITCH];
  int32_t inBhPitch = (int32_t)inDataParams->pitch[TIDL_ROI_PITCH];
  int32_t outBhPitch = (int32_t)tidlLayer->outData.pitch[TIDL_ROI_PITCH];
  int32_t inChPitch = (int32_t)inDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t outChPitch = (int32_t)tidlLayer->outData.pitch[TIDL_CHANNEL_PITCH];
  int32_t ind1Pitch = (int32_t)inDataParams->pitch[TIDL_DIM1_PITCH];
  int32_t outd1Pitch = (int32_t)tidlLayer->outData.pitch[TIDL_DIM1_PITCH];
  int32_t ind2Pitch = (int32_t)inDataParams->pitch[TIDL_DIM2_PITCH];
  int32_t outd2Pitch = (int32_t)tidlLayer->outData.pitch[TIDL_DIM2_PITCH];
  int32_t inPtrOffset = 0;
  int32_t outPtrOffset = 0;
  int32_t status = IALG_EOK;
  float32_tidl min = 0.0f, max = 0.0f;

  int32_t inElPitch = 1;
  int32_t outElPitch = 1;
  int32_t inZeroPoint = inDataParams->tensorZeroPoint;
  int32_t outZeroPoint = tidlLayer->outData.tensorZeroPoint;

  if ((uint32_t)params->type == TIDL_DC_TYPE_INPUT)
  {
    inZeroPoint = params->inZeroPoint;
    if (((uint32_t)intAlgHandle->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) == TIDL_FLOW_CTRL_REF_STAT)
    {
      if (inDataParams->elementType == TIDL_SinglePrecFloat)
      {
        TIDL_TensorMinMaxinFloat((float32_tidl *)inPtrs[0], inDataParams, 0, 1.0, &min, &max);
        inZeroPoint = 0;
      }
      else if (inDataParams->elementType == TIDL_BFloat16)
      {
        TIDL_TensorMinMaxinFloat((bfloat16_tidl *)inPtrs[0], inDataParams, 0, 1.0, &min, &max);
        inZeroPoint = 0;
      }
      else if (inDataParams->elementType == TIDL_SignedChar)
      {
        TIDL_TensorMinMaxinFloat((int8_t *)inPtrs[0], inDataParams, params->inZeroPoint, inDataParams->tensorScale, &min, &max);
      }
      else if (inDataParams->elementType == TIDL_UnsignedChar)
      {
        TIDL_TensorMinMaxinFloat((uint8_t *)inPtrs[0], inDataParams, params->inZeroPoint, inDataParams->tensorScale, &min, &max);
      }
      else if (inDataParams->elementType == TIDL_SignedShort)
      {
        TIDL_TensorMinMaxinFloat((int16_t *)inPtrs[0], inDataParams, params->inZeroPoint, inDataParams->tensorScale, &min, &max);
      }
      else if (inDataParams->elementType == TIDL_UnsignedShort)
      {
        TIDL_TensorMinMaxinFloat((uint16_t *)inPtrs[0], inDataParams, params->inZeroPoint, inDataParams->tensorScale, &min, &max);
      }
#if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
      else if (inDataParams->elementType == TIDL_SignedWord)
      {
        TIDL_TensorMinMaxinFloat((int32_t *)inPtrs[0], inDataParams, params->inZeroPoint, inDataParams->tensorScale, &min, &max);
      }
      else if (inDataParams->elementType == TIDL_UnsignedWord)
      {
        TIDL_TensorMinMaxinFloat((uint32_t *)inPtrs[0], inDataParams, params->inZeroPoint, inDataParams->tensorScale, &min, &max);
      }
#endif
      else
      {
        tidl_printf(0, " Un-supported Data type in %s %d \n", __FILE__, __LINE__);
        status = TIDL_ERR_INVALID_TYPE;
      }

      if (status == TIDL_SUCCESS)
      {
        min *= (float32_tidl)(1U << TIDL_INTERNAL_INDATA_Q);
        max *= (float32_tidl)(1U << TIDL_INTERNAL_INDATA_Q);

        if (TIDL_getDatElementSign(tidlLayer->outData.elementType) == 1)
        {
          TIDL_UpdateScaleFactors(intAlgHandle, algLayer->layerIdx, 1, min, max);
        }
        else
        {
          TIDL_UpdateScaleFactors(intAlgHandle, algLayer->layerIdx, 1, 0, max);
        }
      }
    }
    if (status == TIDL_SUCCESS)
    {
      if ((uint32_t)params->layout == TIDL_LT_NHWC)
      {
        inElPitch = inChs;
        inPitch = inChs * outWidth;
        inChPitch = 1;
      }
    }
  }
  else if ((uint32_t)params->type == TIDL_DC_TYPE_OUTPUT)
  {
    outZeroPoint = params->outZeroPoint;
    inZeroPoint = params->inZeroPoint;

    if (isKernelHighPrecision == TRUE)
    {
      inZeroPoint = inDataParams->tensorZeroPoint;
      outZeroPoint = tidlLayer->outData.tensorZeroPoint;
    }

    if ((uint32_t)params->layout == TIDL_LT_NHWC)
    {
      outElPitch = inChs;
      outPitch = inChs * outWidth;
      outChPitch = 1;
    }
  }
  else
  {
    outZeroPoint = params->outZeroPoint;
    inZeroPoint = params->inZeroPoint;

    if (isKernelHighPrecision == TRUE)
    {
      inZeroPoint = inDataParams->tensorZeroPoint;
      outZeroPoint = tidlLayer->outData.tensorZeroPoint;
    }

    if (params->layout != params->outLayout)
    {
      if (((uint32_t)params->layout == TIDL_LT_NCHW) && ((uint32_t)params->outLayout == TIDL_LT_NHWC))
      {
        outElPitch = inChs;
        outPitch = inChs * outWidth;
        outChPitch = 1;
      }
      else
      {
        inElPitch = inChs;
        inPitch = inChs * outWidth;
        inChPitch = 1;
      }
    }
  }
  if (status == TIDL_SUCCESS)
  {
    if(inDataParams->elementType == TIDL_BFloat16 || tidlLayer->outData.elementType == TIDL_BFloat16)
    {
      status = TIDL_bf16DataConvertRefProcess(
          inDataParams->elementType, tidlLayer->outData.elementType, (uint32_t)params->type,
          inBhs, indim1, indim2, inChs, outHeight, outWidth,
          inBhPitch, outBhPitch,
          ind1Pitch, outd1Pitch,
          ind2Pitch, outd2Pitch,
          inChPitch, outChPitch,
          inPitch, outPitch,
          inElPitch, outElPitch,
          inPtrs, outPtrs);
    }
    else if (inDataParams->elementType == TIDL_SinglePrecFloat)
    {
      inZeroPoint = 0;
      if (intAlgHandle->createParams->net->quantizationStyle == TIDL_QuantStyleAsymNP2_TFL)
      {
        status = TIDL_refDataConvertVarOutType(
            (float32_tidl *)inPtrs[0], (void *)outPtrs[0], inPtrOffset, outPtrOffset,
            inBhs, indim1, indim2, inChs, outHeight, outWidth,
            inBhPitch, outBhPitch, ind1Pitch, outd1Pitch, ind2Pitch, outd2Pitch, inChPitch, outChPitch, inPitch, outPitch, inElPitch, outElPitch,
            inZeroPoint, outZeroPoint, 1.0, ((float32_tidl)1 / tidlLayer->outData.tensorScale), tidlLayer->outData.elementType,
            tidlLayer->outData.roundBits, isKernelHighPrecision, params->type);
      }
      else
      {
        status = TIDL_refDataConvertVarOutType(
            (float32_tidl *)inPtrs[0], (void *)outPtrs[0], inPtrOffset, outPtrOffset,
            inBhs, indim1, indim2, inChs, outHeight, outWidth,
            inBhPitch, outBhPitch, ind1Pitch, outd1Pitch, ind2Pitch, outd2Pitch, inChPitch, outChPitch, inPitch, outPitch, inElPitch, outElPitch,
            inZeroPoint, outZeroPoint, 1.0, (tidlLayer->outData.tensorScale), tidlLayer->outData.elementType,
            tidlLayer->outData.roundBits, isKernelHighPrecision, params->type);
      }
    }
    else if (inDataParams->elementType == TIDL_SignedChar)
    {
      if (intAlgHandle->createParams->net->quantizationStyle == TIDL_QuantStyleAsymNP2_TFL)
      {
        status = TIDL_refDataConvertVarOutType(
            (int8_t *)inPtrs[0], (void *)outPtrs[0], inPtrOffset, outPtrOffset,
            inBhs, indim1, indim2, inChs, outHeight, outWidth,
            inBhPitch, outBhPitch, ind1Pitch, outd1Pitch, ind2Pitch, outd2Pitch, inChPitch, outChPitch, inPitch, outPitch, inElPitch, outElPitch,
            inZeroPoint, outZeroPoint, ((float32_tidl)1 / inDataParams->tensorScale), ((float32_tidl)1 / tidlLayer->outData.tensorScale), tidlLayer->outData.elementType,
            tidlLayer->outData.roundBits, isKernelHighPrecision, params->type);
      }
      else
      {
        status = TIDL_refDataConvertVarOutType(
            (int8_t *)inPtrs[0], (void *)outPtrs[0], inPtrOffset, outPtrOffset,
            inBhs, indim1, indim2, inChs, outHeight, outWidth,
            inBhPitch, outBhPitch, ind1Pitch, outd1Pitch, ind2Pitch, outd2Pitch, inChPitch, outChPitch, inPitch, outPitch, inElPitch, outElPitch,
            inZeroPoint, outZeroPoint, inDataParams->tensorScale, tidlLayer->outData.tensorScale, tidlLayer->outData.elementType,
            tidlLayer->outData.roundBits, isKernelHighPrecision, params->type);
      }
    }
    else if (inDataParams->elementType == TIDL_SignedShort)
    {
      status = TIDL_refDataConvertVarOutType(
          (int16_t *)inPtrs[0], (void *)outPtrs[0], inPtrOffset, outPtrOffset,
          inBhs, indim1, indim2, inChs, outHeight, outWidth,
          inBhPitch, outBhPitch, ind1Pitch, outd1Pitch, ind2Pitch, outd2Pitch, inChPitch, outChPitch, inPitch, outPitch, inElPitch, outElPitch,
          inZeroPoint, outZeroPoint, inDataParams->tensorScale, tidlLayer->outData.tensorScale, tidlLayer->outData.elementType,
          tidlLayer->outData.roundBits, isKernelHighPrecision, params->type);
    }
    else if (inDataParams->elementType == TIDL_UnsignedChar)
    {
      if (intAlgHandle->createParams->net->quantizationStyle == TIDL_QuantStyleAsymNP2_TFL)
      {
        status = TIDL_refDataConvertVarOutType(
            (uint8_t *)inPtrs[0], (void *)outPtrs[0], inPtrOffset, outPtrOffset,
            inBhs, indim1, indim2, inChs, outHeight, outWidth,
            inBhPitch, outBhPitch, ind1Pitch, outd1Pitch, ind2Pitch, outd2Pitch, inChPitch, outChPitch, inPitch, outPitch, inElPitch, outElPitch,
            inZeroPoint, outZeroPoint, ((float32_tidl)1 / inDataParams->tensorScale), ((float32_tidl)1 / tidlLayer->outData.tensorScale), tidlLayer->outData.elementType,
            tidlLayer->outData.roundBits, isKernelHighPrecision, params->type);
      }
      else
      {
        status = TIDL_refDataConvertVarOutType(
            (uint8_t *)inPtrs[0], (void *)outPtrs[0], inPtrOffset, outPtrOffset,
            inBhs, indim1, indim2, inChs, outHeight, outWidth,
            inBhPitch, outBhPitch, ind1Pitch, outd1Pitch, ind2Pitch, outd2Pitch, inChPitch, outChPitch, inPitch, outPitch, inElPitch, outElPitch,
            inZeroPoint, outZeroPoint, inDataParams->tensorScale, tidlLayer->outData.tensorScale, tidlLayer->outData.elementType,
            tidlLayer->outData.roundBits, isKernelHighPrecision, params->type);
      }
    }
    else if (inDataParams->elementType == TIDL_UnsignedShort)
    {
      status = TIDL_refDataConvertVarOutType(
          (uint16_t *)inPtrs[0], (void *)outPtrs[0], inPtrOffset, outPtrOffset,
          inBhs, indim1, indim2, inChs, outHeight, outWidth,
          inBhPitch, outBhPitch, ind1Pitch, outd1Pitch, ind2Pitch, outd2Pitch, inChPitch, outChPitch, inPitch, outPitch, inElPitch, outElPitch,
          inZeroPoint, outZeroPoint, inDataParams->tensorScale, tidlLayer->outData.tensorScale, tidlLayer->outData.elementType,
          tidlLayer->outData.roundBits, isKernelHighPrecision, params->type);
    }
#if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
    else if (inDataParams->elementType == TIDL_UnsignedWord)
    {
      status = TIDL_refDataConvertVarOutType(
          (uint32_t *)inPtrs[0], (void *)outPtrs[0], inPtrOffset, outPtrOffset,
          inBhs, indim1, indim2, inChs, outHeight, outWidth,
          inBhPitch, outBhPitch, ind1Pitch, outd1Pitch, ind2Pitch, outd2Pitch, inChPitch, outChPitch, inPitch, outPitch, inElPitch, outElPitch,
          inZeroPoint, outZeroPoint, inDataParams->tensorScale, tidlLayer->outData.tensorScale, tidlLayer->outData.elementType,
          tidlLayer->outData.roundBits, isKernelHighPrecision, params->type);
    }
    else if (inDataParams->elementType == TIDL_SignedWord)
    {
      status = TIDL_refDataConvertVarOutType(
          (int32_t *)inPtrs[0], (void *)outPtrs[0], inPtrOffset, outPtrOffset,
          inBhs, indim1, indim2, inChs, outHeight, outWidth,
          inBhPitch, outBhPitch, ind1Pitch, outd1Pitch, ind2Pitch, outd2Pitch, inChPitch, outChPitch, inPitch, outPitch, inElPitch, outElPitch,
          inZeroPoint, outZeroPoint, inDataParams->tensorScale, tidlLayer->outData.tensorScale, tidlLayer->outData.elementType,
          tidlLayer->outData.roundBits, isKernelHighPrecision, parmas->type);
    }
#endif
    else
    {
      /* TIDL_LDRA_TAG_DATACONVERT_PRIOR_CHECK_001 */
      tidl_printf(0, " Un-supported Data type in %s %d \n", __FILE__, __LINE__);
      status = TIDL_ERR_INVALID_TYPE;
    }
    if (status == TIDL_SUCCESS)
    {
      TIDL_L1DandL2CacheWbInv();
    }
  }
  return status;
}

int32_t TIDL_dataConvertRefProcess(TIDL_NetworkCommonParams *commonParams,
                                   sTIDL_AlgLayer_t *algLayer,
                                   sTIDL_Layer_t *tidlLayer,
                                   void *inPtrs[],
                                   void *outPtrs[],
                                   int32_t layerIdx)
{
  int32_t status = IALG_EOK;

  TIDL_Obj intAlgObj;
  TIDL_CreateParams createParams;

  (void)memcpy(&createParams, commonParams->createParams, sizeof(TIDL_CreateParams));
  intAlgObj.createParams = (TIDL_CreateParams *)&createParams;

  status = TIDL_dataConvertRefProcess(&intAlgObj, algLayer, tidlLayer, inPtrs, outPtrs);

  return status;
}
