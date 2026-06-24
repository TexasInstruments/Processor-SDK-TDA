/*
*
* Copyright (c) {2015 - 2025} Texas Instruments Incorporated
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

#ifndef LUT_MODULE_H_
#define LUT_MODULE_H_ 1

#include <c7x.h>
#include <cmath>
#include "tidl_types.h"
#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include "tidl_mathlib_utils.h"
#include "tidl_layerNorm.h"

#if defined(__C7604__)
#include "bfloat16.h"
#endif

#define TABLE_SIZE_8BIT 256
#define TABLE_SIZE_16BIT 65536

#ifdef TIDL_COVERAGE_DEAD_CODE
#if defined (__C7100__) || defined (__C7120__)
/******************************************************************************/
/*                                                                            */
/* Defines                                                                    */
/*                                                                            */
/******************************************************************************/
// vector initialization for vperm operation for Exp and Log
#if __C7X_VEC_SIZE_BYTES__ == 64
const c7x::uchar_vec TIDL_vperm_data = (const c7x::uchar_vec)(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63);
const c7x::uchar_vec TIDL_vperm_data_interweave = (const c7x::uchar_vec)(0, 1, 2, 3, 32, 33, 34, 35, 4, 5, 6, 7, 36, 37, 38, 39, 8, 9, 10, 11, 40, 41, 42, 43, 12, 13, 14, 15, 44, 45, 46, 47, 16, 17, 18, 19, 48, 49, 50, 51, 20, 21, 22, 23, 52, 53, 54, 55, 24, 25, 26, 27, 56, 57, 58, 59, 28, 29, 30, 31, 60, 61, 62, 63);
const c7x::uchar_vec TIDL_vperm_data_dp_interweave = (const c7x::uchar_vec)(0, 1, 2, 3, 4, 5, 6, 7, 32, 33, 34, 35, 36, 37, 38, 39, 8, 9, 10, 11, 12, 13, 14, 15, 40, 41, 42, 43, 44, 45, 46, 47, 16, 17, 18, 19, 20, 21, 22, 23, 48, 49, 50, 51, 52, 53, 54, 55, 24, 25, 26, 27, 28, 29, 30, 31, 56, 57, 58, 59, 60, 61, 62, 63);
#elif __C7X_VEC_SIZE_BYTES__ == 32
const c7x::uchar_vec TIDL_vperm_data = (const c7x::uchar_vec)(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31);
const c7x::uchar_vec TIDL_vperm_data_interweave = (const c7x::uchar_vec)(0, 1, 2, 3, 16, 17, 18, 19, 4, 5, 6, 7, 20, 21, 22, 23, 8, 9, 10, 11, 24, 25, 26, 27, 12, 13, 14, 15, 28, 29, 30, 31);
const c7x::uchar_vec TIDL_vperm_data_dp_interweave = (const c7x::uchar_vec)(0, 1, 2, 3, 4, 5, 6, 7, 16, 17, 18, 19, 20, 21, 22, 23, 8, 9, 10, 11, 12, 13, 14, 15, 24, 25, 26, 27, 28, 29, 30, 31);
#endif // #if __C7X_VEC_SIZE_BYTES__
#endif
#endif

#define TIDL_NON_LIN_PARAM_SCALE (int32_t((uint32_t)1 << (uint32_t)24))

#define TIDL_LOGIT_CLIP_MIN (0.000009999999747378752f)

#if defined (__C7524__) || defined (__C7604__)
static inline void TIDL_ILUTInit(int8_t * restrict pData)
{
  for(int32_t i = 0; i < TABLE_SIZE_8BIT; i++)
  {
    __ilut_init(i, pData[i]);
  }
}
#endif

#ifdef BUILD_WITH_OPENACC_DEPRECATED
  static inline float32_tidl OPENACC_pow(float32_tidl base, float32_tidl exponent) {
    /* Return infinity for zero raised to a negative exponent */
    if (base == 0.0f && exponent < 0.0f) {
        return INFINITY; 
    }
    return powf(base, exponent); 
  }
  static inline float32_tidl OPENACC_sin(float32_tidl x) {
    /* Compute sine of x, where x is in radians */
    return sinf(x); 
  }
  static inline float32_tidl OPENACC_sqrt(float32_tidl x) {
    /* Return NaN for negative inputs since sqrt is not defined for negative numbers in real numbers */
    if (x < 0.0f) {
        return NAN; 
    }
    return sqrtf(x); 
  }
  static inline float32_tidl OPENACC_log(float32_tidl x) {
    /* Return negative infinity for non-positive inputs */
    if (x <= 0.0f) {
        return -INFINITY; 
    }
    return logf(x); 
  }
  static inline float32_tidl OPENACC_exp(float32_tidl x) {
    /* Compute exponential of x */
    return expf(x); 
  }
  static inline float32_tidl OPENACC_mish(float32_tidl x) {
    /* Calculate the softplus function */
    float32_tidl softplus = logf(1.0f + expf(x));  
    /* Multiply x by the tanh of the softplus */
    return x * tanhf(softplus);  
  }
  static inline float32_tidl OPENACC_asin(float32_tidl x) {
    /* Return NaN for input values outside the domain */
    if (x < -1.0f || x > 1.0f) {
        return NAN; 
    }
    return asinf(x); 
  }
  static inline int32_t OPENACC__cmp_gt_pred(int32_t a, int32_t b) {
    return a > b ? 1 : 0;
  }
  /* Shift Left */
  static inline int32_t OPENACC__shift_left(int32_t value, int32_t shift) {
    if (shift < 0 || shift >= 32) return 0;
    return value << shift;
  }
  /* Shift Right */
  static inline int32_t OPENACC__shift_right(int32_t value, int32_t shift) {
    if (shift < 0 || shift >= 32) return 0;
    return value >> shift;
  }
  /* Select function based on predicate */
  static inline int32_t OPENACC__select(int32_t predicate, int32_t trueValue, int32_t falseValue) {
    return predicate ? trueValue : falseValue;
  }
  /* Custom exp_taylor function */
  static inline float32_tidl OPENACC_exp_taylor(float32_tidl x)
  {
    float32_tidl twoPwF, ePwX;
    float32_tidl ln2      = 0.693147180559945f;
    float32_tidl oneByLn2 = 1.44269504090f;
    float32_tidl oneBy6   = 0.1666667f;
    float32_tidl oneBy24  = 0.0416667f;
    float32_tidl y        = oneByLn2 * x;
    int32_t yI            = (int32_t)y;
    float32_tidl yf       = y - yI;
    float32_tidl oneBy65356 = 0.0000152587890625f;
    float32_tidl floatRes = yf * ln2;
    float32_tidl floatRes2 = floatRes * floatRes;
    float32_tidl floatRes3 = floatRes2 * floatRes;
    float32_tidl floatRes4 = floatRes2 * floatRes2;
    twoPwF = 1.0f + floatRes + (floatRes2 * 0.5f);
    twoPwF += (floatRes3 * oneBy6);
    twoPwF += (floatRes4 * oneBy24);

    int32_t vp = OPENACC__cmp_gt_pred(yI, 0);
    int32_t tempShiftL = OPENACC__shift_left(1 << 16, yI);
    int32_t tempShiftR = OPENACC__shift_right(1 << 16, -yI);
    tempShiftL = OPENACC__select(vp, tempShiftL, tempShiftR);

    ePwX = twoPwF * (float32_tidl)(tempShiftL);
    ePwX *= oneBy65356;

    if (yI < -16) {
        ePwX = 0.0f;
    }

    if (yI > 14) {
        ePwX = FLT_MAX;
    }

    return ePwX;
  }
  /* Function to perform reciprocal of a number */
  static inline float32_tidl OPENACC__recip(float32_tidl inp) 
  {
    /* Compute the reciprocal */
    if (inp == 0.0f) return FLT_MAX; 
    float32_tidl denom = (float32_tidl)(1.0f / (inp));
    return denom;
  }
  static inline float32_tidl OPENACC_div_sp(float32_tidl x, float32_tidl y)
  {
    float32_tidl recp_y = OPENACC__recip(y);
    float32_tidl TWO  = 2.0f;
    float32_tidl result;
    recp_y = recp_y*(TWO - (y*recp_y));
    result = x*recp_y;
    return result;
  }
  static inline float32_tidl OPENACC__recip_sqrt(float32_tidl inp) 
  {
    /* Compute the reciprocal square root using a built-in function */
    if (sqrt(inp) == 0.0f) return FLT_MAX; 
    float32_tidl denom = (float32_tidl)(1.0f / sqrt(inp));
    return denom;
  }
  static inline float32_tidl OPENACC_asinh(float32_tidl x) {
    float32_tidl ln2 = 0.69314718056f; /* ln(2) */
    float32_tidl pol_bound = 0.5f;
    float32_tidl half = 0.5f;
    float32_tidl sign = 1.0f;
    /* Coefficients for the polynomial */
    float32_tidl c2 = -0.166605362341955f;
    float32_tidl c4 = 0.0734464812833510f;
    float32_tidl c6 = -0.0330279320352987f;

    float32_tidl res, sqrt_, pol;
    float32_tidl x2, x4, x6, x_abs;
    float32_tidl temp;

    x_abs = fabsf(x); /* |x| */
    x2 = x * x; /* x^2 */

    if (x < 0.0f) {
        sign = -1*sign;
    }

    /* powers of x */
    x4 = x2 * x2;
    x6 = x4 * x2;

    /* use polynomial estimation for |x| <= 0.5 */
    pol = (x2 * c2) + (x4 * c4) + (x6 * c6);
    pol = (pol * x) + x;

    /* calculate sqrt(x^2 + 1) */
    sqrt_ = sqrtf(x2 + 1.0f);

    /* calculate asinh(x) for |x| > 0.5 */
    temp = (sqrt_ * half) + (x_abs * half); /* (x + sqrt(x^2 + 1))/2 */
    res = logf(temp) + ln2; /* ln((x + sqrt(x^2 + 1))/2) + ln(2) */
    res = res * sign; /* restore sign */

    /* set the result to the right value depending on the input value */
    if (x_abs <= pol_bound) {
        res = pol;
    }

    return res; 
  }
#endif



template <class Tin, class Tout>
static void TIDL_getOffsetSignedFlag(Tin *inPtr, Tout *outPtr, int32_t * readOffsetLUT, int32_t * isOutputSigned)
{
  *readOffsetLUT = 0;
  int32_t imin = 0;
  imin = std::numeric_limits<Tin>::lowest();
  int32_t omin = 0;
  omin = std::numeric_limits<Tout>::lowest();
  if(imin < 0)
  {
    *readOffsetLUT = -1*imin;
  }
  *isOutputSigned = 0;
  if(omin < 0)
  {
    *isOutputSigned = 1;
  }
}

template <class Tin, class Tout>
int32_t TIDL_gelu_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  float32_tidl mulFact = 0.044715f;
  float32_tidl inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);
  #ifdef BUILD_WITH_OPENACC_DEPRECATED
    OPENACC(routine(OPENACC__recip_sqrt))
    float32_tidl outValF = 2.0f * OPENACC__recip_sqrt(M_PI/2.0);
  #else
    float32_tidl recipSqrtval = (float32_tidl)(M_PI/2.0);    
    float32_tidl mulfactor = __recip_sqrt(recipSqrtval);  
    float32_tidl outValF = 2.0f * mulfactor;
  #endif
  float32_tidl inValFIntr = (mulFact * inValF * inValF * inValF);
        inValFIntr = inValF + inValFIntr;
        outValF = outValF * inValFIntr;   // z = sqrt(2/PI)*(x+0.044715x^3)

  /* This range is chosen so that input to tanh is always from -3 to 3.
      tanh(x) = (e^(2x)-1)/(e^(2x)+1)
  */
  outValF = (outValF > 6.0f)? 6.0f : outValF;
  outValF = (outValF < -6.0f)? -6.0f : outValF;
  #ifdef BUILD_WITH_OPENACC_DEPRECATED
    OPENACC(routine(OPENACC_exp_taylor))
    outValF = OPENACC_exp_taylor(outValF);
    float32_tidl numer = outValF - 1.0f;
    float32_tidl denom = outValF + 1.0f;
    OPENACC(routine(OPENACC__recip))
    float32_tidl mulFactor = OPENACC__recip(denom);
  #else
    outValF = exp_taylor(outValF);
    float32_tidl numer = outValF - 1.0f;
    float32_tidl denom = outValF + 1.0f;
    float32_tidl mulFactor = __recip(denom);
  #endif
  outValF = inValF * (1.0f  + (numer * mulFactor)); // 0.5x(1+tanh(z)) 
  int32_t outMin = 0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = 0;
  outMax = std::numeric_limits<Tout>::max();
  #ifdef BUILD_WITH_OPENACC_DEPRECATED
    int32_t outValInt = (int32_t)((outValF * Sy) + (float32_tidl)Zy);
  #else
    int32_t outValInt = __float_to_int((outValF * Sy) + (float32_tidl)Zy);
  #endif
  outValInt = (outValInt > outMax)? outMax : outValInt;
  outValInt = (outValInt < outMin)? outMin : outValInt;
  *outData   = (Tout)outValInt;
  return status;
}

template <class Tin, class Tout>
int32_t TIDL_gelu_fused_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  float32_tidl mulFact = 0.044715f;
  float32_tidl inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);
  #ifdef BUILD_WITH_OPENACC_DEPRECATED
    OPENACC(routine(OPENACC__recip_sqrt))
    float32_tidl outValF = 2.0f * OPENACC__recip_sqrt(M_PI/2.0);
  #else
    float32_tidl recipSqrtval = (float32_tidl)(M_PI/2.0);    
    float32_tidl mulfactor = __recip_sqrt(recipSqrtval);  
    float32_tidl outValF = 2.0f * mulfactor;
  #endif
  float32_tidl inValFIntr = (mulFact * inValF * inValF * inValF);
        inValFIntr = inValF + inValFIntr;
        outValF = outValF * inValFIntr;   // z = sqrt(2/PI)*(x+0.044715x^3)

  /* This range is chosen so that input to tanh is always from -3 to 3.
      tanh(x) = (e^(2x)-1)/(e^(2x)+1)
  */
  outValF = (outValF > 6.0f)? 6.0f : outValF;
  outValF = (outValF < -6.0f)? -6.0f : outValF;
  #ifdef BUILD_WITH_OPENACC_DEPRECATED
    OPENACC(routine(OPENACC_exp_taylor))
    outValF = OPENACC_exp_taylor(outValF);
    float32_tidl numer = outValF - 1.0f;
    float32_tidl denom = outValF + 1.0f;
    OPENACC(routine(OPENACC__recip))
    float32_tidl mulFactor = OPENACC__recip(denom);
  #else
    outValF = exp_taylor(outValF);
    float32_tidl numer = outValF - 1.0f;
    float32_tidl denom = outValF + 1.0f;
    float32_tidl mulFactor = __recip(denom);
  #endif
  outValF = inValF * (1.0f  + (numer * mulFactor)); // 0.5x(1+tanh(z)) 
  outValF = outValF * 0.5f;
  int32_t outMin = 0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = 0;
  outMax = std::numeric_limits<Tout>::max();
  #ifdef BUILD_WITH_OPENACC_DEPRECATED
    int32_t outValInt = (int32_t)((outValF * Sy) + (float32_tidl)Zy);
  #else
    int32_t outValInt = __float_to_int((outValF * Sy) + (float32_tidl)Zy);
  #endif
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  outValInt = (outValInt > outMax)? outMax : outValInt;
  outValInt = (outValInt < outMin)? outMin : outValInt;
  /* LDRA_JUSTIFY_END */
  *outData   = (Tout)outValInt;
  return status;
}

template <class Tin, class Tout>
int32_t TIDL_tanh_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF, numer, denom;
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);
  inValF = 2.0f * inValF;

  /* This range is chosen so that input to tanh is always from -3 to 3.
      tanh(x) = (e^(2x)-1)/(e^(2x)+1)
  */
  inValF = (inValF > 6.0f)? 6.0f : inValF;
  inValF = (inValF < -6.0f)? -6.0f : inValF;
  #ifdef BUILD_WITH_OPENACC_DEPRECATED
    OPENACC(routine(OPENACC_exp_taylor))
    outValF = OPENACC_exp_taylor(inValF);
    numer = outValF - 1.0f;
    denom = outValF + 1.0f;
    OPENACC(routine(OPENACC__recip))
    outValF = numer *  OPENACC__recip(denom);
  #else
    outValF = exp_taylor(inValF);
    numer = outValF - 1.0f;
    denom = outValF + 1.0f;
    outValF = numer *  __recip(denom);
  #endif

  int32_t outMin = 0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = 0;
  outMax = std::numeric_limits<Tout>::max();
  #ifdef BUILD_WITH_OPENACC_DEPRECATED
    int32_t outValInt = (int32_t)((outValF * Sy) + (float32_tidl)Zy);
  #else
    int32_t outValInt = __float_to_int((outValF * Sy) + (float32_tidl)Zy);
  #endif
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  outValInt = (outValInt > outMax)? outMax : outValInt;
  outValInt = (outValInt < outMin)? outMin : outValInt;
  /* LDRA_JUSTIFY_END */
  *outData   = (Tout)outValInt;
  return status;
}

template <class Tin, class Tout>
int32_t TIDL_softplus_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF;
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);

  /* Two-regime softplus (no exp computation):
  * Small |x| (<=3.6): 3-term minimax Horner in s=x^2
  *   softplus(x) = x/2 + ln2 + s*(0.12392657 + s*(-0.00435581 + s*0.00012791))
  *   Max polynomial error < 0.003; combined max error 0.027
  *   (half the error of the prior 4-term Taylor polynomial at 0.054).
  * Large |x| (>3.6): max(x,0), error = exp(-|x|) <= exp(-3.6) ~= 0.027 */

  const float32_tidl log2baseE = 0.693147180559945f;

  float32_tidl s     = inValF * inValF;
  float32_tidl inner = -0.00435581f + s * 0.00012791f;
  inner              = 0.12392657f  + s * inner;

  float32_tidl outPoly  = inValF * 0.5f + log2baseE + s * inner;
  float32_tidl absxf    = (inValF >= 0.0f) ?  inValF : -inValF;
  float32_tidl maxXzero = (inValF >= 0.0f) ?  inValF :  0.0f;
  outValF  = (absxf <= 3.6f) ? outPoly : maxXzero;

  int32_t outMin = 0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = 0;
  outMax = std::numeric_limits<Tout>::max();
  #ifdef BUILD_WITH_OPENACC_DEPRECATED
    int32_t outValInt = (int32_t)((outValF * Sy) + (float32_tidl)Zy);
  #else
    int32_t outValInt = __float_to_int((outValF * Sy) + (float32_tidl)Zy);
  #endif
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  outValInt = (outValInt > outMax)? outMax : outValInt;
  outValInt = (outValInt < outMin)? outMin : outValInt;
  /* LDRA_JUSTIFY_END */
  *outData   = (Tout)outValInt;
  return status;
}

#if defined(__C7604__)
template <>
inline int32_t TIDL_softplus_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
      const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF;
  const floating_point::bfloat16_t * inData = NULL;
  inData = (const floating_point::bfloat16_t *) dataIn;
  floating_point::bfloat16_t * outData = (floating_point::bfloat16_t *) dataOut;
  inValF = (float32_tidl)*inData ;

  /* Two-regime softplus (no exp computation):
  * Small |x| (<=3.6): 3-term minimax Horner in s=x^2
  *   softplus(x) = x/2 + ln2 + s*(0.12392657 + s*(-0.00435581 + s*0.00012791))
  *   Max polynomial error < 0.003; combined max error 0.027
  *   (half the error of the prior 4-term Taylor polynomial at 0.054).
  * Large |x| (>3.6): max(x,0), error = exp(-|x|) <= exp(-3.6) ~= 0.027 */

  const float32_tidl log2baseE = 0.693147180559945f;

  float32_tidl s     = inValF * inValF;
  float32_tidl inner = -0.00435581f + s * 0.00012791f;
  inner              = 0.12392657f  + s * inner;

  float32_tidl outPoly  = inValF * 0.5f + log2baseE + s * inner;
  float32_tidl absxf    = (inValF >= 0.0f) ?  inValF : -inValF;
  float32_tidl maxXzero = (inValF >= 0.0f) ?  inValF :  0.0f;
  outValF  = (absxf <= 3.6f) ? outPoly : maxXzero;

  *outData   = (floating_point::bfloat16_t) outValF;
  return status;
}
#endif

template <class Tin, class Tout>
int32_t TIDL_softsign_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF, absInValF;
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);
  absInValF = (inValF < 0.0f) ? (-1.0f * inValF) : inValF;

  float32_tidl denom = absInValF + 1.0f;
  float32_tidl r = __recip(denom);
  outValF = inValF * r;
  int32_t outMin = 0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = 0;
  outMax = std::numeric_limits<Tout>::max();
  #ifdef BUILD_WITH_OPENACC_DEPRECATED
    int32_t outValInt = (int32_t)((outValF * Sy) + (float32_tidl)Zy);
  #else
    int32_t outValInt = __float_to_int((outValF * Sy) + (float32_tidl)Zy);
  #endif
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  outValInt = (outValInt > outMax)? outMax : outValInt;
  outValInt = (outValInt < outMin)? outMin : outValInt;
  /* LDRA_JUSTIFY_END */
  *outData   = (Tout)outValInt;
  return status;
}

#if defined(__C7604__)
template <>
inline int32_t TIDL_softsign_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF, absInValF;
  const floating_point::bfloat16_t * inData = NULL;
  inData = (const floating_point::bfloat16_t *) dataIn;
  floating_point::bfloat16_t * outData = (floating_point::bfloat16_t *) dataOut;
  inValF = (float32_tidl)*inData;
  absInValF = (inValF < 0.0f) ? (-1.0f * inValF) : inValF;

  float32_tidl denom = absInValF + 1.0f;
  float32_tidl r = __recip(denom);
  outValF = inValF * r;

  *outData   = (floating_point::bfloat16_t)outValF;
  return status;
}
#endif


template <class Tin, class Tout>
int32_t TIDL_ceil_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF;
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);
  if (inValF == (int32_t)inValF) {
    outValF = inValF;  // Already an integer
  } else if (inValF > 0) {
      outValF = (int32_t)inValF + 1;  // Round up for positive
  } else {
      outValF = (int32_t)inValF;  // Truncation gives ceiling for negative
  }

  float32_tidl float_zy = (float32_tidl)Zy;
  float64_tidl double_out = (float64_tidl)outValF*Sy;
  int32_t outValInt = (int32_t)round((float64_tidl)double_out + float_zy);
  
  int32_t outMin = (int32_t)0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = (int32_t)0;
  outMax = std::numeric_limits<Tout>::max();
  outValInt = (outValInt > outMax)? outMax : outValInt;
  outValInt = (outValInt < outMin)? outMin : outValInt;

  *outData   = (Tout)outValInt;
  return status;
}

template <class Tin, class Tout>
int32_t TIDL_celu_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF, outValF1, outValF2;
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);
  
  outValF1 = inValF < 0.0f ? 0.0f : inValF;
  outValF2 = inValF < 0.0f ? param1*(std::exp(inValF/param1)-1) : 0.0f;
  outValF = outValF1 + outValF2;

  float32_tidl float_zy = (float32_tidl)Zy;
  float64_tidl double_out = (float64_tidl)outValF*Sy;
  int32_t outValInt = (int32_t)round((float64_tidl)double_out + float_zy);

  int32_t outMin = (int32_t)0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = (int32_t)0;
  outMax = std::numeric_limits<Tout>::max();
  outValInt = (outValInt > outMax)? outMax : outValInt;
  outValInt = (outValInt < outMin)? outMin : outValInt;

  *outData   = (Tout)outValInt;
  return status;
}

template <class Tin, class Tout>
int32_t TIDL_selu_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF;
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);
  
  outValF = inValF <= 0.0 ? param2 * (param1 * std::exp(inValF) - param1) : (param2 * inValF);

  float32_tidl float_zy = (float32_tidl)Zy;
  float64_tidl double_out = (float64_tidl)outValF*Sy;
  int32_t outValInt = (int32_t)round((float64_tidl)double_out + float_zy);

  int32_t outMin = (int32_t)0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = (int32_t)0;
  outMax = std::numeric_limits<Tout>::max();
  outValInt = (outValInt > outMax)? outMax : outValInt;
  outValInt = (outValInt < outMin)? outMin : outValInt;

  *outData   = (Tout)outValInt;
  return status;
}

template <class Tin, class Tout>
int32_t TIDL_round_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF;
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);

  int32_t integerPart = (int32_t)(std::abs(inValF));
  float32_tidl fractionalPart = std::abs(inValF) - (float32_tidl)integerPart;
  if (fractionalPart < 0.5f)
  {
    // Round down
    outValF = (float32_tidl)integerPart;
  } 
  else if (fractionalPart > 0.5f) 
  {
    // Round up
    outValF = (float32_tidl)integerPart + 1.0f;
  } 
  else 
  {
    // Exactly half - round to nearest even
    if (integerPart % 2 == 0) {
        // Even number, round down
        outValF = (float32_tidl)integerPart;
    } else {
        // Odd number, round up to make it even
        outValF = (float32_tidl)integerPart + 1.0f;
    }
  }
  outValF = inValF < 0.0 ? (float32_tidl)outValF * (-1.0f) : (float32_tidl)outValF;
  float32_tidl float_zy = (float32_tidl)Zy;
  float64_tidl double_out = (float64_tidl)outValF*Sy;
  int32_t outValInt = (int32_t)round((float64_tidl)double_out + float_zy);

  int32_t outMin = (int32_t)0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = (int32_t)0;
  outMax = std::numeric_limits<Tout>::max();
  outValInt = (outValInt > outMax)? outMax : outValInt;
  outValInt = (outValInt < outMin)? outMin : outValInt;

  *outData   = (Tout)outValInt;
  return status;
}

template <class Tin, class Tout>
int32_t TIDL_sign_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF;
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);

  if(inValF != 0.0)
  {
    outValF = inValF > 0.0 ? 1.0 : -1.0;
  }
  else
  {
    outValF = 0.0;
  }

  int32_t outMin = 0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = 0;
  outMax = std::numeric_limits<Tout>::max();
  int32_t outValInt = float_to_int_c7x((outValF*Sy)+(float32_tidl)Zy);
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  outValInt = (outValInt > outMax)? outMax : outValInt;
  outValInt = (outValInt < outMin)? outMin : outValInt;
  /* LDRA_JUSTIFY_END */
  *outData   = (Tout)outValInt;
  return status;
}


template <class Tin, class Tout>
int32_t TIDL_sinh_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF, numer;
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);

  /* This range is chosen so that input to sinh is always from -3 to 3. similar to tanh
      sinh(x) = (e^(x) - e^(-x)) / 2)
  */
  outValF = exp_taylor(inValF);
  numer = outValF - __recip(outValF);
  outValF = numer * 0.5f;

  int32_t outMin = 0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = 0;
  outMax = std::numeric_limits<Tout>::max();
  int32_t outValInt = float_to_int_c7x((outValF*Sy)+(float32_tidl)Zy);
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  outValInt = (outValInt > outMax)? outMax : outValInt;
  outValInt = (outValInt < outMin)? outMin : outValInt;
  /* LDRA_JUSTIFY_END */
  *outData   = (Tout)outValInt;
  return status;
}

template <class Tin, class Tout>
int32_t TIDL_cosh_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF, numer;
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);

  /* This range is chosen so that input to cosh is always from -3 to 3. similar to tanh
      cosh(x) = (e^(x) + e^(-x)) / 2)
  */
  outValF = exp_taylor(inValF);
  numer = outValF + __recip(outValF);
  outValF = numer * 0.5f;

  int32_t outMin = (int32_t)0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = (int32_t)0;
  outMax = std::numeric_limits<Tout>::max();
  int32_t outValInt = float_to_int_c7x((outValF*Sy)+(float32_tidl)Zy);
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  outValInt = (outValInt > outMax)? outMax : outValInt;
  outValInt = (outValInt < outMin)? outMin : outValInt;
  /* LDRA_JUSTIFY_END */
  *outData   = (Tout)outValInt;
  return status;
}

template <class Tin, class Tout>
int32_t TIDL_elu_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF;
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);
  #ifdef BUILD_WITH_OPENACC_DEPRECATED
    OPENACC(routine(OPENACC_exp_taylor))
    outValF = (inValF < 0.0f) ? (param1*(OPENACC_exp_taylor(inValF) - 1.0f)) : inValF;
  #else
    outValF = (inValF < 0.0f) ? (param1*(exp_taylor(inValF) - 1.0f)) : inValF;
  #endif

  int32_t outMin = (int32_t)0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = (int32_t)0;
  outMax = std::numeric_limits<Tout>::max();
  #ifdef BUILD_WITH_OPENACC_DEPRECATED
    int32_t outValInt = (int32_t)((outValF * Sy) + (float32_tidl)Zy);
  #else
    int32_t outValInt = __float_to_int((outValF * Sy) + (float32_tidl)Zy);
  #endif
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  outValInt = (outValInt > outMax)? outMax : outValInt;
  outValInt = (outValInt < outMin)? outMin : outValInt;
  /* LDRA_JUSTIFY_END */
  *outData   = (Tout)outValInt;
  return status;
}

template <class Tin, class Tout>
int32_t TIDL_hardSigmoid_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF;
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);
  outValF = (param1*inValF) + param2;

  float32_tidl omin = (float32_tidl)0.0f;
  float32_tidl omax = (float32_tidl)1.0f;
  outValF = (outValF > omax) ? omax : outValF;
  outValF = (outValF < omin) ? omin : outValF;
  float32_tidl float_zy = (float32_tidl)Zy;
  float64_tidl double_out = (float64_tidl)outValF*Sy;
  int32_t outValInt = (int32_t)round((float64_tidl)double_out + float_zy);

  int32_t outMin = (int32_t)0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = (int32_t)0;
  outMax = std::numeric_limits<Tout>::max();
  outValInt = (outValInt > outMax)? outMax : outValInt;
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  outValInt = (outValInt < outMin)? outMin : outValInt;
  /* LDRA_JUSTIFY_END */
  *outData   = (Tout)outValInt;
  return status;
}

#if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
template <class Tin, class Tout>
int32_t TIDL_asin_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF;
  
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);

  #ifdef BUILD_WITH_OPENACC_DEPRECATED
    OPENACC(routine(OPENACC_asin))
    outValF = OPENACC_asin(inValF);
  #else
    outValF = MATHLIB_asin(inValF);
  #endif

  float32_tidl float_zy = (float32_tidl)Zy;
  float64_tidl double_out = (float64_tidl)outValF*Sy;
  int32_t outValInt = (int32_t)round((float64_tidl)double_out + float_zy);

  int32_t outMin = (int32_t)0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = (int32_t)0;
  outMax = std::numeric_limits<Tout>::max();
  outValInt = (outValInt > outMax)? outMax : outValInt;
  outValInt = (outValInt < outMin)? outMin : outValInt;

  *outData = (Tout)outValInt;

  return status;
}
#endif

#if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
template <class Tin, class Tout>
int32_t TIDL_acos_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF;
  
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);

  outValF = MATHLIB_acos(inValF);

  int32_t outValInt = float_to_int_c7x((outValF*Sy)+(float32_tidl)Zy);

  int32_t outMin = (int32_t)0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = (int32_t)0;
  outMax = std::numeric_limits<Tout>::max();
  outValInt = (outValInt > outMax)? outMax : outValInt;
  outValInt = (outValInt < outMin)? outMin : outValInt;

  *outData = (Tout)outValInt;

  return status;
}
#endif

template <class Tin, class Tout>
int32_t TIDL_atan_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF;
  
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);

  outValF = MATHLIB_atan(inValF);

  int32_t outValInt = float_to_int_c7x((outValF*Sy)+(float32_tidl)Zy);

  int32_t outMin = (int32_t)0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = (int32_t)0;
  outMax = std::numeric_limits<Tout>::max();
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  outValInt = (outValInt > outMax)? outMax : outValInt;
  outValInt = (outValInt < outMin)? outMin : outValInt;
  /* LDRA_JUSTIFY_END */

  *outData = (Tout)outValInt;

  return status;
}

#if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
template <class Tin, class Tout>
int32_t TIDL_asinh_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF;
  
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);

  #ifdef BUILD_WITH_OPENACC_DEPRECATED
    OPENACC(routine(OPENACC_asinh))
    outValF = OPENACC_asinh(inValF);
  #else
    outValF = MATHLIB_asinh(inValF);
  #endif

  float32_tidl float_zy = (float32_tidl)Zy;
  float64_tidl double_out = (float64_tidl)outValF*Sy;
  int32_t outValInt = (int32_t)round((float64_tidl)double_out + float_zy);

  int32_t outMin = (int32_t)0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = (int32_t)0;
  outMax = std::numeric_limits<Tout>::max();
  outValInt = (outValInt > outMax)? outMax : outValInt;
  outValInt = (outValInt < outMin)? outMin : outValInt;

  *outData = (Tout)outValInt;

  return status;
}
#endif

template <class Tin, class Tout>
int32_t TIDL_hardswish_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF;
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);
  
  outValF = (param1*inValF) + param2;

  float32_tidl omin = (float32_tidl)0.0f;
  float32_tidl omax = (float32_tidl)1.0f;
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  outValF = (outValF > omax) ? omax : outValF;
  outValF = (outValF < omin) ? omin : outValF;
  /* LDRA_JUSTIFY_END */
  outValF = inValF * outValF;

  float32_tidl float_zy = (float32_tidl)Zy;
  float64_tidl double_out = (float64_tidl)outValF*Sy;
  //Modified from round to float_to_int_c7x to match with CI flow
  int32_t outValInt = (int32_t)float_to_int_c7x((float32_tidl)double_out + float_zy);

  int32_t outMin = (int32_t)0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = (int32_t)0;
  outMax = std::numeric_limits<Tout>::max();
  outValInt = (outValInt > outMax)? outMax : outValInt;
  outValInt = (outValInt < outMin)? outMin : outValInt;

  *outData   = (Tout)outValInt;
  return status;
}

template <class Tin, class Tout>
int32_t TIDL_swish_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF;
  float32_tidl expOut = (float32_tidl)0.0f;
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);
  
  // In onnx if denom is 0 in division, then output is 0
  if(inValF == (float32_tidl)0.0f)
  {
    outValF = (float32_tidl)0.0f;
  }
  else
  {
    expOut = (float32_tidl)exp_taylor((float32_tidl)-1.0f * param1 * inValF);
    outValF = inValF * (1.0f / (1.0f + expOut));
  }

  int32_t outMin = (int32_t)0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = (int32_t)0;
  outMax = std::numeric_limits<Tout>::max();
  #ifdef BUILD_WITH_OPENACC_DEPRECATED
    int32_t outValInt = (int32_t)((outValF * Sy) + (float32_tidl)Zy);
  #else
    int32_t outValInt = __float_to_int((outValF * Sy) + (float32_tidl)Zy);
  #endif
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start>  LDRA_JUSTIFICATION_BATCHNORM_SAFETY_CHECK : safe programming hard to hit this condition with real world data
  <justification end> */
  outValInt = (outValInt > outMax)? outMax : outValInt;
  outValInt = (outValInt < outMin)? outMin : outValInt;
  /* LDRA_JUSTIFY_END */
  *outData   = (Tout)outValInt;
  return status;
}

#if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
template <class Tin, class Tout>
int32_t TIDL_mish_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF;
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);
  
  #ifdef BUILD_WITH_OPENACC_DEPRECATED
    OPENACC(routine(OPENACC_mish))
    outValF = OPENACC_mish(inValF);
  #else
    outValF = MATHLIB_mish(inValF);
  #endif

  float32_tidl float_zy = (float32_tidl)Zy;
  float64_tidl double_out = (float64_tidl)outValF*Sy;
  int32_t outValInt = (int32_t)round((float64_tidl)double_out + float_zy);

  int32_t outMin = (int32_t)0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = (int32_t)0;
  outMax = std::numeric_limits<Tout>::max();
  outValInt = (outValInt > outMax)? outMax : outValInt;
  outValInt = (outValInt < outMin)? outMin : outValInt;

  *outData   = (Tout)outValInt;
  return status;
}
#endif

#if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
template <class Tin, class Tout>
int32_t TIDL_log_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF;
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);
  #ifdef BUILD_WITH_OPENACC_DEPRECATED
    OPENACC(routine(OPENACC_log))
    outValF = OPENACC_log(inValF);
  #else
    outValF = MATHLIB_ln(inValF);
  #endif

  float32_tidl float_zy = (float32_tidl)Zy;
  float64_tidl double_out = (float64_tidl)outValF*Sy;
  //Modified from round to float_to_int_c7x to match with CI flow
  int32_t outValInt = (int32_t)float_to_int_c7x((float32_tidl)double_out + float_zy);

  int32_t outMin = (int32_t)0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = (int32_t)0;
  outMax = std::numeric_limits<Tout>::max();
  outValInt = (outValInt > outMax)? outMax : outValInt;
  outValInt = (outValInt < outMin)? outMin : outValInt;

  *outData   = (Tout)outValInt;
  return status;
}
#endif

template <class Tin, class Tout>
int32_t TIDL_abs_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF;
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);
  outValF = inValF;
  if(outValF < 0.0f) {
    outValF = -1.0f * outValF;
  }

  float32_tidl float_zy = (float32_tidl)Zy;
  float64_tidl double_out = (float64_tidl)outValF*Sy;
  int32_t outValInt = (int32_t)round((float64_tidl)double_out + float_zy);

  int32_t outMin = (int32_t)0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = (int32_t)0;
  outMax = std::numeric_limits<Tout>::max();
  outValInt = (outValInt > outMax)? outMax : outValInt;
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  outValInt = (outValInt < outMin)? outMin : outValInt;
  /* LDRA_JUSTIFY_END */
  *outData   = (Tout)outValInt;
  return status;
}

template <class Tin, class Tout>
int32_t TIDL_floor_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF;
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);
  int32_t inValInt = (int32_t) inValF;
  outValF = (float32_tidl) inValInt;
  if((inValF < 0.0f) && (inValF != outValF)){
    outValF = outValF - 1.0f;
  }

  float32_tidl float_zy = (float32_tidl)Zy;
  float64_tidl double_out = (float64_tidl)outValF*Sy;
  int32_t outValInt = (int32_t)round((float64_tidl)double_out + float_zy);

  int32_t outMin = (int32_t)0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = (int32_t)0;
  outMax = std::numeric_limits<Tout>::max();
  outValInt = (outValInt > outMax)? outMax : outValInt;
  outValInt = (outValInt < outMin)? outMin : outValInt;

  *outData   = (Tout)outValInt;
  return status;
}

#if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
template <class Tin, class Tout>
int32_t TIDL_exp_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF;
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);
  #ifdef BUILD_WITH_OPENACC_DEPRECATED
    OPENACC(routine(OPENACC_exp))
    outValF = OPENACC_exp(inValF);
  #else
    outValF = MATHLIB_exp(inValF);
  #endif

  float32_tidl float_zy = (float32_tidl)Zy;
  float64_tidl double_out = (float64_tidl)outValF*Sy;
  //Modified from round to float_to_int_c7x to match with CI flow
  int32_t outValInt = (int32_t)float_to_int_c7x((float32_tidl)double_out + float_zy);

  int32_t outMin = (int32_t)0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = (int32_t)0;
  outMax = std::numeric_limits<Tout>::max();
  outValInt = (outValInt > outMax)? outMax : outValInt;
  outValInt = (outValInt < outMin)? outMin : outValInt;

  *outData   = (Tout)outValInt;
  return status;
}
#endif

template <class Tin, class Tout>
int32_t TIDL_sin_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF;
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);
  #ifdef BUILD_WITH_OPENACC_DEPRECATED
    OPENACC(routine(OPENACC_sin))
    outValF = OPENACC_sin(inValF);
  #else
    outValF = MATHLIB_sin(inValF);
  #endif

  float32_tidl float_zy = (float32_tidl)Zy;
  float64_tidl double_out = (float64_tidl)outValF*Sy;
  //Modified from round to float_to_int_c7x to match with CI flow
  int32_t outValInt = (int32_t)float_to_int_c7x((float32_tidl)double_out + float_zy);

  int32_t outMin = (int32_t)0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = (int32_t)0;
  outMax = std::numeric_limits<Tout>::max();
  outValInt = (outValInt > outMax)? outMax : outValInt;
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  outValInt = (outValInt < outMin)? outMin : outValInt;
  /* LDRA_JUSTIFY_END */
  *outData   = (Tout)outValInt;
  return status;
}

template <class Tin, class Tout>
int32_t TIDL_cos_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF;
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);
  outValF = MATHLIB_cos(inValF);

  float32_tidl float_zy = (float32_tidl)Zy;
  float64_tidl double_out = (float64_tidl)outValF*Sy;
  //Modified from round to float_to_int_c7x to match with CI flow
  int32_t outValInt = (int32_t)float_to_int_c7x((float32_tidl)double_out + float_zy);

  int32_t outMin = (int32_t)0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = (int32_t)0;
  outMax = std::numeric_limits<Tout>::max();
  outValInt = (outValInt > outMax)? outMax : outValInt;
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  outValInt = (outValInt < outMin)? outMin : outValInt;
  /* LDRA_JUSTIFY_END */
  *outData   = (Tout)outValInt;
  return status;
}

#if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
template <class Tin, class Tout>
int32_t TIDL_tan_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF;
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);
  outValF = MATHLIB_tan(inValF);

  float32_tidl float_zy = (float32_tidl)Zy;
  float64_tidl double_out = (float64_tidl)outValF*Sy;
  //Modified from round to float_to_int_c7x to match with CI flow
  int32_t outValInt = (int32_t)float_to_int_c7x((float32_tidl)double_out + float_zy);

  int32_t outMin = (int32_t)0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = (int32_t)0;
  outMax = std::numeric_limits<Tout>::max();
  outValInt = (outValInt > outMax)? outMax : outValInt;
  outValInt = (outValInt < outMin)? outMin : outValInt;

  *outData   = (Tout)outValInt;
  return status;
}
#endif

template <class Tin, class Tout>
int32_t TIDL_erf_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF;
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);
  outValF = taylor_erf(inValF);

  float32_tidl float_zy = (float32_tidl)Zy;
  float64_tidl double_out = (float64_tidl)outValF*Sy;
  //Modified from round to float_to_int_c7x to match with CI flow
  int32_t outValInt = (int32_t)float_to_int_c7x((float32_tidl)double_out + float_zy);

  int32_t outMin = (int32_t)0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = (int32_t)0;
  outMax = std::numeric_limits<Tout>::max();
  outValInt = (outValInt > outMax)? outMax : outValInt;
  outValInt = (outValInt < outMin)? outMin : outValInt;

  *outData   = (Tout)outValInt;
  return status;
}

template <class Tin, class Tout>
int32_t TIDL_sqrt_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF;
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);
  #ifdef BUILD_WITH_OPENACC_DEPRECATED
    OPENACC(routine(OPENACC_sqrt))
    outValF = OPENACC_sqrt(inValF);
  #else
    outValF = MATHLIB_sqrt(inValF);
  #endif

  float32_tidl float_zy = (float32_tidl)Zy;
  float64_tidl double_out = (float64_tidl)outValF*Sy;
  //Modified from round to float_to_int_c7x to match with CI flow
  int32_t outValInt = (int32_t)float_to_int_c7x((float32_tidl)double_out + float_zy);

  int32_t outMin = (int32_t)0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = (int32_t)0;
  outMax = std::numeric_limits<Tout>::max();
  outValInt = (outValInt > outMax)? outMax : outValInt;
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  outValInt = (outValInt < outMin)? outMin : outValInt;
  /* LDRA_JUSTIFY_END */

  *outData   = (Tout)outValInt;
  return status;
}

#if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
template <class Tin, class Tout>
int32_t TIDL_pow_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF;
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);
  #ifdef BUILD_WITH_OPENACC_DEPRECATED
    OPENACC(routine(OPENACC_pow))
    outValF = OPENACC_pow(inValF, param1);
  #else
    outValF = MATHLIB_pow(inValF, param1);
  #endif

  float32_tidl float_zy = (float32_tidl)Zy;
  float64_tidl double_out = (float64_tidl)outValF*Sy;
  int32_t outValInt = (int32_t)round((float64_tidl)double_out + float_zy);

  int32_t outMin = (int32_t)0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = (int32_t)0;
  outMax = std::numeric_limits<Tout>::max();
  outValInt = (outValInt > outMax)? outMax : outValInt;
  outValInt = (outValInt < outMin)? outMin : outValInt;

  *outData   = (Tout)outValInt;
  return status;
}
#endif

template <class Tin, class Tout>
int32_t TIDL_neg_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF;
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);

  outValF = -(inValF);

  float32_tidl float_zy = (float32_tidl)Zy;
  float64_tidl double_out = (float64_tidl)outValF*Sy;
  int32_t outValInt = (int32_t)round((float64_tidl)double_out + float_zy);

  int32_t outMin = (int32_t)0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = (int32_t)0;
  outMax = std::numeric_limits<Tout>::max();
  outValInt = (outValInt > outMax)? outMax : outValInt;
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  outValInt = (outValInt < outMin)? outMin : outValInt;
  /* LDRA_JUSTIFY_END */
  *outData   = (Tout)outValInt;
  return status;
}

#if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
template <class Tin, class Tout>
int32_t TIDL_logit_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF, denom;
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);
  /**
   * logit(x) = log(x/(1-x))
  */
  inValF = (inValF < 0.0f) ? 0.0f : inValF;
  inValF = (inValF > 1.0f) ? 1.0f : inValF;

  denom = 1.0f - inValF;
  denom = (denom < TIDL_LOGIT_CLIP_MIN) ? TIDL_LOGIT_CLIP_MIN : denom;

  inValF = (inValF < TIDL_LOGIT_CLIP_MIN) ? TIDL_LOGIT_CLIP_MIN : inValF;
  #ifdef BUILD_WITH_OPENACC_DEPRECATED
    OPENACC(routine(OPENACC_log))
    outValF = OPENACC_log(inValF/denom);
  #else
    outValF = MATHLIB_ln(inValF/denom);
  #endif

  float32_tidl float_zy = (float32_tidl)Zy;
  float64_tidl double_out = (float64_tidl)outValF*Sy;

  int32_t outValInt = (int32_t)round((float64_tidl)double_out + float_zy);

  int32_t outMin = (int32_t)0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = (int32_t)0;
  outMax = std::numeric_limits<Tout>::max();
  outValInt = (outValInt > outMax)? outMax : outValInt;
  outValInt = (outValInt < outMin)? outMin : outValInt;

  *outData   = (Tout)outValInt;
  return status;
}
#endif

#if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
template <class Tin, class Tout>
int32_t TIDL_reciprocal_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF;
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = NULL;
  outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);

  // In onnx if denom is 0 in division, then output is 0
  if(inValF == (float32_tidl)0.0f)
  {
    outValF = (float32_tidl)0.0f;
  }
  else
  {
    #ifdef BUILD_WITH_OPENACC_DEPRECATED
      OPENACC(routine(OPENACC__recip))
      outValF = OPENACC__recip(inValF);
    #else
      outValF = (float32_tidl)(1.0f / inValF);
    #endif
  }

  float32_tidl float_zy = (float32_tidl)Zy;
  float64_tidl double_out = (float64_tidl)outValF*Sy;
  int32_t outValInt = (int32_t)round((float64_tidl)double_out + float_zy);

  int32_t outMin = (int32_t)0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = (int32_t)0;
  outMax = std::numeric_limits<Tout>::max();
  outValInt = (outValInt > outMax)? outMax : outValInt;
  outValInt = (outValInt < outMin)? outMin : outValInt;

  *outData   = (Tout)outValInt;
  return status;
}
#endif

#if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
template <class Tin, class Tout>
int32_t TIDL_silu_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF;
  float32_tidl mathlibOut = (float32_tidl)0.0f;
  const Tin * inData = NULL;
  inData = (const Tin *) dataIn;
  Tout * outData = NULL;
  outData = (Tout *) dataOut;
  inValF = ((float32_tidl)*inData - (float32_tidl)Zx) * (1.0f/Sx);

  // In onnx if denom is 0 in division, then output is 0
  if(inValF == (float32_tidl)0.0f)
  {
    outValF = (float32_tidl)0.0f;
  }
  else
  {
    mathlibOut = (float32_tidl)MATHLIB_exp((float32_tidl)-1.0f * inValF);
    outValF = inValF * (1.0f / (1.0f + mathlibOut));
  }

  float32_tidl float_zy = (float32_tidl)Zy;
  float64_tidl double_out = (float64_tidl)outValF*Sy;
  int32_t outValInt = (int32_t)round((float64_tidl)double_out + float_zy);

  int32_t outMin = (int32_t)0;
  outMin = std::numeric_limits<Tout>::lowest();
  int32_t outMax = (int32_t)0;
  outMax = std::numeric_limits<Tout>::max();
  outValInt = (outValInt > outMax)? outMax : outValInt;
  outValInt = (outValInt < outMin)? outMin : outValInt;

  *outData   = (Tout)outValInt;
  return status;
}
#endif

template int32_t TIDL_gelu_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_gelu_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_gelu_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_gelu_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_gelu_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_gelu_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_gelu_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_gelu_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template int32_t TIDL_gelu_fused_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_gelu_fused_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_gelu_fused_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_gelu_fused_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_gelu_fused_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_gelu_fused_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_gelu_fused_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_gelu_fused_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template int32_t TIDL_tanh_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_tanh_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_tanh_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_tanh_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_tanh_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_tanh_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_tanh_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_tanh_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template int32_t TIDL_elu_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_elu_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_elu_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_elu_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_elu_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_elu_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_elu_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_elu_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template int32_t TIDL_hardSigmoid_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_hardSigmoid_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_hardSigmoid_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_hardSigmoid_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_hardSigmoid_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_hardSigmoid_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_hardSigmoid_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_hardSigmoid_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

#if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
template int32_t TIDL_asin_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_asin_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_asin_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_asin_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_asin_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_asin_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_asin_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_asin_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
#endif

#if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
template int32_t TIDL_asinh_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_asinh_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_asinh_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_asinh_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_asinh_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_asinh_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_asinh_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_asinh_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
#endif

template int32_t TIDL_hardswish_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_hardswish_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_hardswish_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_hardswish_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_hardswish_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_hardswish_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_hardswish_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_hardswish_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template int32_t TIDL_swish_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_swish_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_swish_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_swish_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_swish_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_swish_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_swish_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_swish_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

#if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
template int32_t TIDL_mish_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_mish_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_mish_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_mish_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_mish_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_mish_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_mish_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_mish_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
#endif

#if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
template int32_t TIDL_log_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_log_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_log_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_log_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_log_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_log_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_log_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_log_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
#endif

#if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
template int32_t TIDL_pow_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_pow_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_pow_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_pow_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_pow_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_pow_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_pow_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_pow_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
#endif

template int32_t TIDL_sin_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_sin_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_sin_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_sin_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_sin_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_sin_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_sin_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_sin_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

#if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
template int32_t TIDL_exp_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_exp_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_exp_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_exp_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_exp_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_exp_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_exp_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_exp_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
#endif

template int32_t TIDL_floor_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_floor_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_floor_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_floor_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_floor_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_floor_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_floor_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_floor_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template int32_t TIDL_abs_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_abs_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_abs_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_abs_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_abs_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_abs_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_abs_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_abs_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template int32_t TIDL_erf_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_erf_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_erf_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_erf_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_erf_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_erf_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_erf_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_erf_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template int32_t TIDL_sqrt_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_sqrt_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_sqrt_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_sqrt_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_sqrt_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_sqrt_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_sqrt_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_sqrt_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

#if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
template int32_t TIDL_acos_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_acos_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_acos_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_acos_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_acos_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_acos_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_acos_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_acos_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
#endif

template int32_t TIDL_atan_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_atan_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_atan_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_atan_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_atan_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_atan_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_atan_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_atan_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template int32_t TIDL_sinh_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_sinh_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_sinh_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_sinh_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_sinh_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_sinh_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_sinh_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_sinh_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template int32_t TIDL_cos_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_cos_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_cos_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_cos_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_cos_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_cos_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_cos_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_cos_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template int32_t TIDL_cosh_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_cosh_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_cosh_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_cosh_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_cosh_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_cosh_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_cosh_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_cosh_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

#if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
template int32_t TIDL_tan_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_tan_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_tan_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_tan_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_tan_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_tan_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_tan_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_tan_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
#endif

template int32_t TIDL_neg_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_neg_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_neg_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_neg_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_neg_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_neg_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_neg_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_neg_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

#if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
template int32_t TIDL_logit_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_logit_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_logit_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_logit_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_logit_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_logit_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_logit_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_logit_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
#endif

#if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
template int32_t TIDL_reciprocal_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_reciprocal_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_reciprocal_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_reciprocal_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_reciprocal_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_reciprocal_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_reciprocal_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_reciprocal_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
#endif

#if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
template int32_t TIDL_silu_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_silu_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_silu_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_silu_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_silu_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_silu_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_silu_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_silu_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
#endif

template int32_t TIDL_softplus_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_softplus_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_softplus_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_softplus_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_softplus_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_softplus_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_softplus_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_softplus_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template int32_t TIDL_softsign_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_softsign_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_softsign_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_softsign_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_softsign_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_softsign_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_softsign_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_softsign_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template int32_t TIDL_ceil_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_ceil_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_ceil_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_ceil_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_ceil_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_ceil_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_ceil_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_ceil_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template int32_t TIDL_celu_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_celu_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_celu_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_celu_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_celu_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_celu_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_celu_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_celu_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template int32_t TIDL_selu_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_selu_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_selu_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_selu_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_selu_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_selu_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_selu_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_selu_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template int32_t TIDL_round_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_round_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_round_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_round_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_round_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_round_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_round_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_round_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template int32_t TIDL_sign_nonLut<int8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_sign_nonLut<int8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_sign_nonLut<uint8_t, uint8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_sign_nonLut<uint8_t, int8_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_sign_nonLut<int16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_sign_nonLut<int16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_sign_nonLut<uint16_t, uint16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);
template int32_t TIDL_sign_nonLut<uint16_t, int16_t>(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

#if defined(__C7604__)
/* BFloat16 template specializations - only available on C7604 (TDA54) */

template <>
inline int32_t TIDL_tanh_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF, numer, denom;
  const floating_point::bfloat16_t *inData = (const floating_point::bfloat16_t *) dataIn;
  floating_point::bfloat16_t *outData      = (floating_point::bfloat16_t *) dataOut;

  inValF                 = (float32_tidl) *inData;
  float32_tidl tanhInput = 2.0f * inValF;
  // Clamp to [-6, 6] to avoid overflow in exp_taylor
  tanhInput = (tanhInput > 6.0f) ? 6.0f : tanhInput;
  tanhInput = (tanhInput < -6.0f) ? -6.0f : tanhInput;
  outValF   = exp_taylor(tanhInput);
  numer     = outValF - 1.0f;
  denom     = outValF + 1.0f;
  outValF   = numer * __recip(denom);

  *outData = (floating_point::bfloat16_t) outValF;
  return status;
}

#endif

#if defined(__C7604__)
template <>
inline int32_t TIDL_gelu_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2)
{
  int32_t status                           = TIDL_SUCCESS;
  const floating_point::bfloat16_t *inData = (const floating_point::bfloat16_t *) dataIn;
  floating_point::bfloat16_t *outData      = (floating_point::bfloat16_t *) dataOut;
  float32_tidl mulFact                     = 0.044715f;

  float32_tidl inValF       = (float32_tidl) *inData;
  float32_tidl recipSqrtval = (float32_tidl) (M_PI / 2.0);
  float32_tidl mulfactor    = __recip_sqrt(recipSqrtval);
  float32_tidl outValF      = 2.0f * mulfactor;
  float32_tidl inValFIntr   = (mulFact * inValF * inValF * inValF);
  inValFIntr                = inValF + inValFIntr;
  outValF                   = outValF * inValFIntr; // z = sqrt(2/PI)*(x+0.044715x^3)

  // Clamp to [-6, 6] to avoid overflow in exp_taylor
  outValF                = (outValF > 6.0f) ? 6.0f : outValF;
  outValF                = (outValF < -6.0f) ? -6.0f : outValF;
  outValF                = exp_taylor(outValF);
  float32_tidl numer     = outValF - 1.0f;
  float32_tidl denom     = outValF + 1.0f;
  float32_tidl mulFactor = __recip(denom);
  outValF                = inValF * (1.0f + (numer * mulFactor)); // 0.5x(1+tanh(z))

  *outData = (floating_point::bfloat16_t) outValF;
  return status;
}

template <>
inline int32_t TIDL_gelu_fused_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2)
{
  int32_t status                           = TIDL_SUCCESS;
  const floating_point::bfloat16_t *inData = (const floating_point::bfloat16_t *) dataIn;
  floating_point::bfloat16_t *outData      = (floating_point::bfloat16_t *) dataOut;
  float32_tidl mulFact                     = 0.044715f;

  float32_tidl inValF       = (float32_tidl) *inData;
  float32_tidl recipSqrtval = (float32_tidl) (M_PI / 2.0);
  float32_tidl mulfactor    = __recip_sqrt(recipSqrtval);
  float32_tidl outValF      = 2.0f * mulfactor;
  float32_tidl inValFIntr   = (mulFact * inValF * inValF * inValF);
  inValFIntr                = inValF + inValFIntr;
  outValF                   = outValF * inValFIntr; // z = sqrt(2/PI)*(x+0.044715x^3)

  // Clamp to [-6, 6] to avoid overflow in exp_taylor
  outValF                = (outValF > 6.0f) ? 6.0f : outValF;
  outValF                = (outValF < -6.0f) ? -6.0f : outValF;
  outValF                = exp_taylor(outValF);
  float32_tidl numer     = outValF - 1.0f;
  float32_tidl denom     = outValF + 1.0f;
  float32_tidl mulFactor = __recip(denom);
  outValF                = inValF * (1.0f + (numer * mulFactor)); // x*(1+tanh(z))
  outValF                = outValF * 0.5f;                        // 0.5*x*(1+tanh(z))

  *outData = (floating_point::bfloat16_t) outValF;
  return status;
}

template <>
inline int32_t TIDL_elu_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2)
{
  int32_t status                           = TIDL_SUCCESS;
  const floating_point::bfloat16_t *inData = (const floating_point::bfloat16_t *) dataIn;
  floating_point::bfloat16_t *outData      = (floating_point::bfloat16_t *) dataOut;

  float32_tidl inValF  = (float32_tidl) *inData;
  float32_tidl outValF = (inValF < 0.0f) ? (param1 * (exp_taylor(inValF) - 1.0f)) : inValF;

  *outData = (floating_point::bfloat16_t) outValF;
  return status;
}

template <>
inline int32_t TIDL_celu_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2)
{
  int32_t status                           = TIDL_SUCCESS;
  const floating_point::bfloat16_t *inData = (const floating_point::bfloat16_t *) dataIn;
  floating_point::bfloat16_t *outData      = (floating_point::bfloat16_t *) dataOut;

  float32_tidl inValF  = (float32_tidl) *inData;
  float32_tidl outValF = (inValF < 0.0f) ? param1 * (exp_taylor(inValF / param1) - 1.0f) : inValF;

  *outData = (floating_point::bfloat16_t) outValF;
  return status;
}

template <>
inline int32_t TIDL_hardSigmoid_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2)
{
  int32_t status                           = TIDL_SUCCESS;
  const floating_point::bfloat16_t *inData = (const floating_point::bfloat16_t *) dataIn;
  floating_point::bfloat16_t *outData      = (floating_point::bfloat16_t *) dataOut;

  float32_tidl inValF  = (float32_tidl) *inData;
  float32_tidl outValF = (param1 * inValF) + param2;
  outValF              = (outValF > 1.0f) ? 1.0f : outValF;
  outValF              = (outValF < 0.0f) ? 0.0f : outValF;

  *outData = (floating_point::bfloat16_t) outValF;
  return status;
}

template <>
inline int32_t TIDL_atan_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2)
{
  int32_t status                           = TIDL_SUCCESS;
  const floating_point::bfloat16_t *inData = (const floating_point::bfloat16_t *) dataIn;
  floating_point::bfloat16_t *outData      = (floating_point::bfloat16_t *) dataOut;

  float32_tidl inValF  = (float32_tidl) *inData;
  float32_tidl outValF = MATHLIB_atan(inValF);

  *outData = (floating_point::bfloat16_t) outValF;
  return status;
}

template <>
inline int32_t TIDL_sin_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2)
{
  int32_t status                           = TIDL_SUCCESS;
  const floating_point::bfloat16_t *inData = (const floating_point::bfloat16_t *) dataIn;
  floating_point::bfloat16_t *outData      = (floating_point::bfloat16_t *) dataOut;

  float32_tidl inValF  = (float32_tidl) *inData;
  float32_tidl outValF = MATHLIB_sin(inValF);

  *outData = (floating_point::bfloat16_t) outValF;
  return status;
}

template <>
inline int32_t TIDL_cos_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2)
{
  int32_t status                           = TIDL_SUCCESS;
  const floating_point::bfloat16_t *inData = (const floating_point::bfloat16_t *) dataIn;
  floating_point::bfloat16_t *outData      = (floating_point::bfloat16_t *) dataOut;

  float32_tidl inValF  = (float32_tidl) *inData;
  float32_tidl outValF = MATHLIB_cos(inValF);

  *outData = (floating_point::bfloat16_t) outValF;
  return status;
}

template <>
inline int32_t TIDL_sinh_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF, numer;
  const floating_point::bfloat16_t *inData = (const floating_point::bfloat16_t *) dataIn;
  floating_point::bfloat16_t *outData      = (floating_point::bfloat16_t *) dataOut;

  inValF = (float32_tidl) *inData;
  // sinh(x) = (e^x - e^(-x)) / 2 = (e^x - 1/e^x) / 2
  outValF = exp_taylor(inValF);
  numer   = outValF - __recip(outValF);
  outValF = numer * 0.5f;

  *outData = (floating_point::bfloat16_t) outValF;
  return status;
}

template <>
inline int32_t TIDL_cosh_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF, numer;
  const floating_point::bfloat16_t *inData = (const floating_point::bfloat16_t *) dataIn;
  floating_point::bfloat16_t *outData      = (floating_point::bfloat16_t *) dataOut;

  inValF = (float32_tidl) *inData;
  // cosh(x) = (e^x + e^(-x)) / 2 = (e^x + 1/e^x) / 2
  outValF = exp_taylor(inValF);
  numer   = outValF + __recip(outValF);
  outValF = numer * 0.5f;

  *outData = (floating_point::bfloat16_t) outValF;
  return status;
}

template <>
inline int32_t TIDL_abs_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2)
{
  int32_t status                           = TIDL_SUCCESS;
  const floating_point::bfloat16_t *inData = (const floating_point::bfloat16_t *) dataIn;
  floating_point::bfloat16_t *outData      = (floating_point::bfloat16_t *) dataOut;

  float32_tidl inValF  = (float32_tidl) *inData;
  float32_tidl outValF = fabsf(inValF);

  *outData = (floating_point::bfloat16_t) outValF;
  return status;
}

template <>
inline int32_t TIDL_neg_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2)
{
  int32_t status                           = TIDL_SUCCESS;
  const floating_point::bfloat16_t *inData = (const floating_point::bfloat16_t *) dataIn;
  floating_point::bfloat16_t *outData      = (floating_point::bfloat16_t *) dataOut;

  float32_tidl inValF  = (float32_tidl) *inData;
  float32_tidl outValF = -inValF;

  *outData = (floating_point::bfloat16_t) outValF;
  return status;
}

template <>
inline int32_t TIDL_floor_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2)
{
  int32_t status = TIDL_SUCCESS;
  float32_tidl inValF, outValF;
  const floating_point::bfloat16_t *inData = (const floating_point::bfloat16_t *) dataIn;
  floating_point::bfloat16_t *outData      = (floating_point::bfloat16_t *) dataOut;

  inValF           = (float32_tidl) *inData;
  int32_t inValInt = (int32_t) inValF;
  outValF          = (float32_tidl) inValInt;
  if ((inValF < 0.0f) && (inValF != outValF)) {
    outValF = outValF - 1.0f;
  }

  *outData = (floating_point::bfloat16_t) outValF;
  return status;
}

template <>
inline int32_t TIDL_sqrt_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2)
{
  int32_t status                           = TIDL_SUCCESS;
  const floating_point::bfloat16_t *inData = (const floating_point::bfloat16_t *) dataIn;
  floating_point::bfloat16_t *outData      = (floating_point::bfloat16_t *) dataOut;

  float32_tidl inValF  = (float32_tidl) *inData;
  float32_tidl outValF = MATHLIB_sqrt(inValF);

  *outData = (floating_point::bfloat16_t) outValF;
  return status;
}

template <>
inline int32_t TIDL_erf_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2)
{
  int32_t status                           = TIDL_SUCCESS;
  const floating_point::bfloat16_t *inData = (const floating_point::bfloat16_t *) dataIn;
  floating_point::bfloat16_t *outData      = (floating_point::bfloat16_t *) dataOut;

  float32_tidl inValF  = (float32_tidl) *inData;
  float32_tidl outValF = taylor_erf(inValF);

  *outData = (floating_point::bfloat16_t) outValF;
  return status;
}

#if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
template <>
inline int32_t TIDL_silu_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2)
{
  int32_t status                           = TIDL_SUCCESS;
  const floating_point::bfloat16_t *inData = (const floating_point::bfloat16_t *) dataIn;
  floating_point::bfloat16_t *outData      = (floating_point::bfloat16_t *) dataOut;
  float32_tidl outValF;

  float32_tidl inValF = (float32_tidl) *inData;
  // In onnx if denom is 0 in division, then output is 0
  if (inValF == (float32_tidl) 0.0f) {
    outValF = (float32_tidl) 0.0f;
  } else {
    float32_tidl mathlibOut = (float32_tidl) MATHLIB_exp((float32_tidl) -1.0f * inValF);
    outValF                 = inValF * (1.0f / (1.0f + mathlibOut));
  }

  *outData = (floating_point::bfloat16_t) outValF;
  return status;
}
#endif

template <>
inline int32_t TIDL_hardswish_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2)
{
  int32_t status                           = TIDL_SUCCESS;
  const floating_point::bfloat16_t *inData = (const floating_point::bfloat16_t *) dataIn;
  floating_point::bfloat16_t *outData      = (floating_point::bfloat16_t *) dataOut;

  float32_tidl inValF  = (float32_tidl) *inData;
  float32_tidl clipped = (param1 * inValF) + param2;
  clipped              = (clipped > 1.0f) ? 1.0f : clipped;
  clipped              = (clipped < 0.0f) ? 0.0f : clipped;
  float32_tidl outValF = inValF * clipped;

  *outData = (floating_point::bfloat16_t) outValF;
  return status;
}
#endif

#if defined(__C7604__)
template int32_t TIDL_tanh_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2);
template int32_t TIDL_elu_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2);
template int32_t TIDL_hardSigmoid_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2);
template int32_t TIDL_gelu_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2);
template int32_t TIDL_gelu_fused_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2);
template int32_t TIDL_atan_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2);
template int32_t TIDL_sin_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2);
template int32_t TIDL_sinh_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2);
template int32_t TIDL_cos_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2);
template int32_t TIDL_cosh_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2);
template int32_t TIDL_neg_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2);
template int32_t TIDL_abs_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2);
template int32_t TIDL_floor_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2);
template int32_t TIDL_sqrt_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2);
template int32_t TIDL_erf_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2);
template int32_t TIDL_hardswish_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2);
template int32_t TIDL_celu_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2);
#if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
template int32_t TIDL_silu_nonLut<floating_point::bfloat16_t, floating_point::bfloat16_t>(
    const void *dataIn, void *dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy,
    float32_tidl param1, float32_tidl param2);
#endif
#endif // __C7604__

#endif /* LUT_MODULE_H_ */

/* ======================================================================== */
/*  End of file:  tidl_lut_module.h                                                  */
/* ======================================================================== */

