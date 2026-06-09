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

#include <tidl_import_lut_core.h>
#include <tidl_import_diag.h>
void * my_malloc(int32_t size); //my_malloc - need to remove later, shouldnt be used in this file
void my_free(void *ptr);//my_free - need to remove later, shouldnt be used in this file


template int32_t TIDL_getQuantizeLUT(uint8_t*, int8_t*, int32_t, sTIDL_NonLinearAct_LUT_createParams_t*);
template int32_t TIDL_getQuantizeLUT(int16_t*, uint16_t*, int32_t, sTIDL_NonLinearAct_LUT_createParams_t*);
template int32_t TIDL_getQuantizeLUT(int16_t*, int16_t*, int32_t, sTIDL_NonLinearAct_LUT_createParams_t*);
template int32_t TIDL_getQuantizeLUT(int8_t*, uint8_t*, int32_t, sTIDL_NonLinearAct_LUT_createParams_t*);
template int32_t TIDL_getQuantizeLUT(uint8_t*, uint8_t*, int32_t, sTIDL_NonLinearAct_LUT_createParams_t*);
template int32_t TIDL_getQuantizeLUT(uint16_t*, int16_t*, int32_t, sTIDL_NonLinearAct_LUT_createParams_t*);
template int32_t TIDL_getQuantizeLUT(int8_t*, int8_t*, int32_t, sTIDL_NonLinearAct_LUT_createParams_t*);
template int32_t TIDL_getQuantizeLUT(uint16_t*, uint16_t*, int32_t, sTIDL_NonLinearAct_LUT_createParams_t*);

template void TIDL_quantizeLUT(double*, int16_t*, double*, int32_t*, int32_t*);
template void TIDL_quantizeLUT(double*, uint16_t*, double*, int32_t*, int32_t*);

template void TIDL_prepareSwInterpolLUT(int16_t*, int16_t*,  double*, double*, sTIDL_NonLinearAct_LUT_createParams_t *);
template void TIDL_prepareSwInterpolLUT(int16_t*, uint16_t*, double*, double*, sTIDL_NonLinearAct_LUT_createParams_t *);
template void TIDL_prepareSwInterpolLUT(uint16_t*, uint16_t*, double*, double*, sTIDL_NonLinearAct_LUT_createParams_t *);
template void TIDL_prepareSwInterpolLUT(uint16_t*, int16_t*, double*, double*, sTIDL_NonLinearAct_LUT_createParams_t *);

#define TIDL_NON_LIN_PARAM_SCALE (int32_t(1<<24))

int32_t TIDL_getNonLinearActMethod(int32_t inDataType, int32_t outDataType, int32_t nonLinearactType, int32_t deviceID){
  int32_t lutDataType, nonLinearActMethod ;
  int32_t status = TIDL_IMPORT_DIAGNOSIS_RETURN_OK ;

  if ((inDataType == (int32_t)TIDL_UnsignedChar || inDataType == (int32_t)TIDL_SignedChar) &&
      (outDataType == (int32_t)TIDL_UnsignedChar || outDataType == (int32_t)TIDL_SignedChar))
  {
    lutDataType = TIDL_LUT_8_BIT_DTYPE;
  }
  else if ((inDataType == (int32_t)TIDL_UnsignedShort || inDataType == (int32_t)TIDL_SignedShort) &&
      (outDataType == (int32_t)TIDL_UnsignedShort || outDataType == (int32_t)TIDL_SignedShort))
  {
    lutDataType = TIDL_LUT_16_BIT_DTYPE;
  }
  else{
    TIDL_GLOBAL_REPORT_ERROR("Invalid Data type  [IN, OUT] = [%d, %d] supplied for activation Method  = %d", 
      inDataType, outDataType, nonLinearactType);
    status =  TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  if(deviceID > TIDL_NUM_LUT_METHOD_DEVICE){
    TIDL_GLOBAL_REPORT_ERROR("Invalid Device ID  %d supplied for activation Method  = %d", 
      deviceID, nonLinearactType);
    status =  TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  if(nonLinearactType > TIDL_LUT_TOTAL_NONLINEAR_OPS){
    TIDL_GLOBAL_REPORT_ERROR("Invalid Activation method %d supplied", nonLinearactType);
    status =  TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  if(status == TIDL_IMPORT_DIAGNOSIS_RETURN_OK){
    nonLinearActMethod = TIDL_LUTMethodMapTable[lutDataType][nonLinearactType][deviceID];
  }
  else{
    nonLinearActMethod = status;    
  }
  return nonLinearActMethod ;
}

void TIDL_prepareXval(double *Xdata)
{
  int32_t i, j;
  for (i = 0; i < 256; i++)
  {
    for (j = 0; j < 2; j++)
    {
      double x = (i * 1.0) / 256;
      Xdata[(i * 2) + j] = std::pow(x, (2 - j - 1));
    }
  }
  return;
}

static double TIDL_hardswish(double valInFp64)
{
  double valOutFp64;
  double hardswish_alpha = 0.166666666f;
  double hardswish_beta = 0.5f;

  valOutFp64 = (hardswish_alpha * valInFp64) + hardswish_beta;

  double omin = 0;
  double omax = 1;
  valOutFp64 = (valOutFp64 > omax) ? omax : valOutFp64;
  valOutFp64 = (valOutFp64 < omin) ? omin : valOutFp64;
  valOutFp64 = valInFp64 * valOutFp64;

  return valOutFp64;
}

static double TIDL_swish(double valInFp64, double alpha)
{
  //f(x) = x * sigmoid((alpha = 1) * x) = x * (1 / (1 + e^(-x)))
  double valOutFp64;

  valOutFp64 = std::exp(-1.0 * alpha * valInFp64);
  valOutFp64 = (1.0 / (1.0 + valOutFp64));
  valOutFp64 = valInFp64 * valOutFp64;

  return valOutFp64;
}

static double TIDL_mish(double valInFp64)
{
  //f(x) = x * tanh(softplus(x)) = x * tanh(ln(1 + e^x))
  double valOutFp64;
  valOutFp64 = std::log(1.0f + std::exp(valInFp64));
  valOutFp64 = std::tanh(valOutFp64);
  valOutFp64 = valOutFp64 * valInFp64;
  return valOutFp64;
}

static inline double div_sp(double x, double y)
{
  double recp_y = (1/y);

  double TWO  = 2.0f;
  double result;

  recp_y = recp_y * (TWO - (y * recp_y));
  result = x * recp_y;

  return result;
}

static inline double exp_taylor(float32_tidl x)
{
  float32_tidl twoPwF, ePwX;

  float32_tidl ln2      = 0.693147180559945f;
  float32_tidl oneByLn2 = 1.44269504090f;
  float32_tidl oneBy6   = 0.1666667f;
  float32_tidl oneBy24  = 0.0416667f;
  float32_tidl y        = oneByLn2 * x;
  int32_t yI            = (int32_t)y;
  float32_tidl yf       = y - (float32_tidl)yI;
  float32_tidl oneBy65356 = 0.0000152587890625f;

  float32_tidl floatRes = yf * ln2;
  int32_t tempShift;

  float32_tidl floatRes2 = floatRes * floatRes;
  float32_tidl floatRes3 = floatRes2 * floatRes;
  float32_tidl floatRes4 = floatRes2 * floatRes2;
  twoPwF = 1.0f + floatRes + (floatRes2 * 0.5f);
  twoPwF = twoPwF + (floatRes3 * oneBy6);
  twoPwF = twoPwF + (floatRes4 * oneBy24);

  /*pre left shift of 16 is done to generate the result multiple of (1<<16). Later it is divided
    by (1<<16) to get accurate result.
   */
  if (yI > 0)
  {
    tempShift = ((1 << 16) << yI);
    ePwX = twoPwF * (float32_tidl)(tempShift);
    ePwX = ePwX * oneBy65356;
  }
  else
  {
    tempShift = ((1 << 16) >> -yI);
    ePwX = twoPwF * (float32_tidl)(tempShift);
    ePwX = ePwX * oneBy65356;
  }

  ePwX = (yI > (int32_t)14) ? (float)FLT_MAX : ePwX;
  ePwX = (yI < (int32_t)-16) ? (float)0.0f : ePwX;

  return (double)ePwX;
}

static double TIDL_sigmoid(double valInFp64)
{
  double valOutFp64;
  valOutFp64 = div_sp((double)1.0, (exp_taylor((float32_tidl)-valInFp64) + (double)1.0));
  return valOutFp64;
}

static double TIDL_hardsigmoid(double valInFp64, double alpha, double beta)
{
  double valOutFp64;
  alpha = alpha/((double)TIDL_NON_LIN_PARAM_SCALE);
  beta = beta/((double)TIDL_NON_LIN_PARAM_SCALE);
  valOutFp64 = (alpha * valInFp64) + beta;

  double omin = 0;
  double omax = 1;
  valOutFp64 = (valOutFp64 > omax) ? omax : valOutFp64;
  valOutFp64 = (valOutFp64 < omin) ? omin : valOutFp64;
  
  return valOutFp64;
}

static double TIDL_elu(double valInFp64, double alpha)
{
  double valOutFp64;
  alpha = alpha/((double)TIDL_NON_LIN_PARAM_SCALE);
  valOutFp64 = valInFp64 < 0.0 ? (alpha * (std::exp(valInFp64) - 1)) : valInFp64;
  return valOutFp64;
}

static double TIDL_celu(double valInFp64, double alpha)
{
  double valOutFp64, valOutFp64_temp1, valOutFp64_temp2, tempOut;
  alpha = alpha/((double)TIDL_NON_LIN_PARAM_SCALE);
  tempOut = alpha*(std::exp(valInFp64/alpha)-1);
  valOutFp64_temp1 = valInFp64 < 0.0 ? 0.0 : valInFp64;
  valOutFp64_temp2 = tempOut < 0.0 ? tempOut : 0.0;
  valOutFp64 = valOutFp64_temp1 + valOutFp64_temp2;
  return valOutFp64;
}

static double TIDL_selu(double valInFp64, double alpha, double gamma)
{
  double valOutFp64;
  alpha = alpha/((double)TIDL_NON_LIN_PARAM_SCALE);
  gamma = gamma/((double)TIDL_NON_LIN_PARAM_SCALE);
  valOutFp64 = valInFp64 <= 0.0 ? gamma * (alpha * std::exp(valInFp64) - alpha) : gamma * valInFp64;
  return valOutFp64;
}

static double TIDL_round(double valInFp64)
{
  double valOutFp64;
  int32_t integerPart = (int32_t)(std::abs(valInFp64));
  float32_tidl fractionalPart = std::abs(valInFp64) - (float32_tidl)integerPart;
  if (fractionalPart < 0.5f)
  {
    // Round down
    valOutFp64 = (float32_tidl)integerPart;
  } 
  else if (fractionalPart > 0.5f) 
  {
    // Round up
    valOutFp64 = (float32_tidl)integerPart + 1.0f;
  } 
  else 
  {
    // Exactly half - round to nearest even
    if (integerPart % 2 == 0) {
        // Even number, round down
        valOutFp64 = (float32_tidl)integerPart;
    } else {
        // Odd number, round up to make it even
        valOutFp64 = (float32_tidl)integerPart + 1.0f;
    }
  }
  valOutFp64 = valInFp64 < 0.0 ? valOutFp64 * (-1.0f) : valOutFp64;
  return valOutFp64;
}

static double TIDL_pow(double valInFp64, double pow)
{
  double valOutFp64;
  pow = pow/((double)TIDL_NON_LIN_PARAM_SCALE);
  valOutFp64 = std::pow(valInFp64, pow);
  return valOutFp64;
}

static double TIDL_leakyRelu(double valInFp64, double alpha, int32_t inZeroPoint)
{
  double valOutFp64;
  if (valInFp64 >= (double)0)
  {
    valOutFp64 = valInFp64;
  }
  else
  {
    valOutFp64 = valInFp64 * alpha;
  }
  return valOutFp64;
}

static double TIDL_gelu(double valInFp64)
{
  double valOutFp64;
  valOutFp64 = std::sqrt((double)(2.0/M_PI)) * (valInFp64 + (0.044715 * valInFp64 * valInFp64 * valInFp64));
  valOutFp64 = valInFp64 * ((double)1  + std::tanh(valOutFp64)); 
  return valOutFp64;
}

static double TIDL_gelu_fused(double valInFp64)
{
  double valOutFp64;
  valOutFp64 = (double)0.5 * valInFp64 * ((double)1 + std::erf(valInFp64 * 0.70710678118));
  return valOutFp64;
}

static double TIDL_logit(double valInFp64)
{
  double valOutFp64, denomFp64;

  denomFp64 = (double)(1.0 - valInFp64);
  denomFp64 = denomFp64 < TIDL_LOGIT_CLIP_MIN ? TIDL_LOGIT_CLIP_MIN : denomFp64;
  valInFp64 = valInFp64 < TIDL_LOGIT_CLIP_MIN ? TIDL_LOGIT_CLIP_MIN : valInFp64;

  valOutFp64 = std::log(valInFp64 / denomFp64);
  return valOutFp64;
}

static double TIDL_NonLinearActOut(double valInFp64, int32_t actType, double alpha, double beta, double pow, int32_t in_zero_point, bool useFusedKernel = false)
{
  double valOutFP64;

  if (actType == TIDL_Sigmoid)
  {
    valOutFP64 = TIDL_sigmoid(valInFp64);
  }
  else if (actType == TIDL_Tanh)
  {
    valOutFP64 = std::tanh(valInFp64);
  }
  else if (actType == TIDL_HardSigmoid)
  {
    valOutFP64 = TIDL_hardsigmoid(valInFp64, alpha, beta);
  }
  else if (actType == TIDL_ELU)
  {
    valOutFP64 = TIDL_elu(valInFp64, alpha);
  }
  else if (actType == TIDL_GELU)
  {
    if(useFusedKernel != (bool)0)
    {
      valOutFP64 = TIDL_gelu_fused(valInFp64);
    }
    else
    {
      valOutFP64 = TIDL_gelu(valInFp64);
    }
  }
  else if (actType == TIDL_Asin)
  {
    valOutFP64 = std::asin(valInFp64);
  }
  else if (actType == TIDL_Asinh)
  {
    valOutFP64 = std::asinh(valInFp64);
  }
  else if (actType == TIDL_HardSwish)
  {
    valOutFP64 = TIDL_hardswish(valInFp64);
  }
  else if (actType == TIDL_Swish)
  {
    valOutFP64 = TIDL_swish(valInFp64, alpha);
  }
  else if (actType == TIDL_Mish)
  {
    valOutFP64 = TIDL_mish(valInFp64);
  }
  else if (actType == TIDL_LeakyReLU)
  {
    valOutFP64 = TIDL_leakyRelu(valInFp64, alpha, in_zero_point);
  }
  else if (actType == TIDL_Log)
  {
    valOutFP64 = std::log(valInFp64);
  }
  else if (actType == TIDL_Pow)
  {
    valOutFP64 = TIDL_pow(valInFp64, pow);
  }
  else if (actType == TIDL_Sin)
  {
    valOutFP64 = std::sin(valInFp64);
  }
  else if (actType == TIDL_Exp)
  {
    valOutFP64 = std::exp(valInFp64);
  }
  else if (actType == TIDL_Abs)
  {
    valOutFP64 = std::abs(valInFp64);
  }
  else if (actType == TIDL_Sqrt)
  {
    valOutFP64 = std::sqrt(valInFp64);
  }
  else if (actType == TIDL_Floor)
  {
    valOutFP64 = std::floor(valInFp64);
  }
  else if (actType == TIDL_Erf)
  {
    valOutFP64 = std::erf(valInFp64);
  }
  else if (actType == TIDL_Acos)
  {
    valOutFP64 = std::acos(valInFp64);
  }
  else if (actType == TIDL_Atan)
  {
    valOutFP64 = std::atan(valInFp64);
  }
  else if (actType == TIDL_Cos)
  {
    valOutFP64 = std::cos(valInFp64);
  }
  else if (actType == TIDL_Cosh)
  {
    valOutFP64 = std::cosh(valInFp64);
  }
  else if (actType == TIDL_Sinh)
  {
    valOutFP64 = std::sinh(valInFp64);
  }
  else if (actType == TIDL_Neg)
  {
    valOutFP64 = -(valInFp64);
  }
  else if (actType == TIDL_Tan)
  {
    valOutFP64 = std::tan(valInFp64);
  }
  else if (actType == TIDL_Logit)
  {
    valOutFP64 = TIDL_logit(valInFp64);
  }
  else if (actType == TIDL_Reciprocal)
  {
    const double epsilon = 1e-5;
    valOutFP64 = (valInFp64 == 0) ? 0.0 : (double)(1.0 / (valInFp64 + (epsilon * (valInFp64 >= 0 ? 1.0 : -1.0))));
  }
  else if (actType == TIDL_SiLU)
  {
    valOutFP64 = (valInFp64 == 0) ? 0.0 : (double)(valInFp64 * (1.0 / (1.0 + std::exp(-1 * valInFp64))));
  }
  else if (actType == TIDL_SoftPlus)
  {
    valOutFP64 = (double)(std::log(1.0 + std::exp(valInFp64)));
  }
  else if (actType == TIDL_SoftSign)
  {
    valOutFP64 = (valInFp64 == 0) ? 0.0 : (double)(valInFp64 / (1.0 + std::abs(valInFp64)));
  }
  else if (actType == TIDL_Ceil)
  {
    valOutFP64 = std::ceil(valInFp64);
  }
  else if (actType == TIDL_Celu)
  {
    valOutFP64 = TIDL_celu(valInFp64, alpha);
  }
  else if (actType == TIDL_Selu)
  {
    valOutFP64 = TIDL_selu(valInFp64, alpha, beta);
  }
  else if (actType == TIDL_Round)
  {
    valOutFP64 = TIDL_round(valInFp64);
  }
  else if (actType == TIDL_Sign)
  {
    if(valInFp64 != 0.0f)
    {
      valOutFP64 = valInFp64 > 0.0f ? 1.0f : -1.0f;
    }
    else
    {
      valOutFP64 = 0.0f;
    }
  }
  else
  {
    /*do nothing*/
  }

  return valOutFP64;
}

template <class Tin>
static double TIDL_dequantize(Tin val, float32_tidl scale, int32_t zero_point)
{
  /*In TIDL import flow scale is not as per standard defintion of ONNX, rather it is 1/scale */
  double val_double = ((double)1.0 * (((float)val - (float)zero_point) / (float)scale));
  return val_double;
}

template <class Tout>
static Tout TIDL_quantize(Tout *outptr, double val_double, float32_tidl scale, int32_t zero_point)
{
  /*In TIDL import flow scale is not as per standard defintion of ONNX, rather it is 1/scale */
  double temp_val = (val_double * scale) + zero_point;
  int32_t outMin, outMax, outVal;

  outMin = std::numeric_limits<Tout>::lowest();
  outMax = std::numeric_limits<Tout>::max();
  outVal = round(temp_val);
  outVal = (outVal > outMax) ? outMax : outVal;
  outVal = (outVal < outMin) ? outMin : outVal;
  return (Tout)outVal;
}

template <class Tin, class Tout>
int32_t TIDL_getQuantizeLUT(Tin *inPtr, Tout *Y, int32_t bin, sTIDL_NonLinearAct_LUT_createParams_t* pLUTCreateParams)
{
  int32_t status = TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
  Tin valIn;
  Tout valOut;
  double valInFP64, valOutFP64;
  float32_tidl in_scale, out_scale;
  int32_t in_zero_point, out_zero_point, actType;
  double alpha, beta, pow;
  bool useFusedKernel = false;

  actType = pLUTCreateParams->actType;
  in_zero_point = pLUTCreateParams->inTensorZeroPoint;
  in_scale = pLUTCreateParams->inTensorScale;
  out_zero_point = pLUTCreateParams->outTensorZeroPoint;
  out_scale = pLUTCreateParams->outTensorScale;
  alpha = pLUTCreateParams->alpha;
  beta = pLUTCreateParams->beta;
  pow = pLUTCreateParams->pow;

  // to use fused kernel for gelu from 10_01_04_00
  if(pLUTCreateParams->tidlNetVersion >= TIDL_NET_VERSION_FW_11_00_00_00)
  {
    useFusedKernel = true;
  }


  int32_t offset = std::numeric_limits<Tin>::lowest();

  if (Y == NULL)
  {
    status = TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  else
  {
    for (int32_t idx = 0; idx < NUM_ELEMENTS_PERBINS; idx++)
    { 
      valIn = (bin * NUM_ELEMENTS_PERBINS) + idx + offset;
      valInFP64 = TIDL_dequantize(valIn, in_scale, in_zero_point);
      valOutFP64 = TIDL_NonLinearActOut(valInFP64, actType, alpha, beta, pow, in_zero_point, useFusedKernel);
      valOut = TIDL_quantize(Y, valOutFP64, out_scale, out_zero_point);
      Y[idx] = valOut;
    }
  }
  return status;
}


static void TIDL_getMinMaxVal(double *list, int num_val, double *min, double *max)
{
  double min_val = list[0];
  double max_val = list[0];
  for (int32_t i = 1; i < num_val; i++)
  {
    if (list[i] > max_val)
    {
      max_val = list[i];
    }
    if (list[i] < min_val)
    {
      min_val = list[i];
    }
  }
  *min = min_val;
  *max = max_val;
  return ;
}

static void TIDL_getScaleZeroPoint(double min_val, double max_val, void *quant_min_val, void *quant_max_val, int16_t symmetric, int16_t power2scale, double *scale_factor, void *zero_point, int16_t force_symmetric, int16_t i_signed)
{
  int16_t use_symmetric;
  if ((force_symmetric == 1) || ((symmetric == 1) && (min_val < 0)))
  {
    use_symmetric = 1;
  }
  else
  {
    use_symmetric = 0;
    if (i_signed == 0)
    {
      *((uint16_t *)quant_max_val) = *((uint16_t *)quant_max_val) - *((uint16_t *)quant_min_val);
      *((uint16_t *)quant_min_val) = 0;
    }
    else
    {
      *((int16_t *)quant_max_val) = *((int16_t *)quant_max_val) - *((int16_t *)quant_min_val);
      *((int16_t *)quant_min_val) = 0;
    }
  }
  int quant_half;

  if (use_symmetric == 1)
  {
    min_val = (min_val < 0) ? min_val : 0;
    max_val = (max_val > 0) ? max_val : 0;
    if (i_signed == 0)
      quant_half = (*((uint16_t *)quant_max_val) - *((uint16_t *)quant_min_val) + 1) / 2;
    else
      quant_half = (int)std::fmax(abs(*((int16_t *)quant_min_val)), abs(*((int16_t *)quant_max_val)));
    *scale_factor = ((double)std::fmax(std::fabs(min_val), std::fabs(max_val))) / (quant_half);
  }
  else
  {
    int quant_range;
    if (i_signed == 1)
    {
      quant_range = *((int16_t *)quant_max_val) - *((int16_t *)quant_min_val);
    }
    else
    {
      quant_range = *((uint16_t *)quant_max_val) - *((uint16_t *)quant_min_val);
    }
    double orig_range = ((double)(max_val - min_val));
    if (min_val < 0 & max_val < 0)
    {
      max_val = 0;
    }
    else if (min_val > 0 & max_val > 0)
    {
      min_val = 0;
    }
    orig_range = ((double)(max_val - min_val));
    if (power2scale == 1)
    {
      double abs_quant_range = std::fabs((double)quant_range);
      double sign_quant_range = quant_range / abs_quant_range;
      double ceil_log_quant_range = std::ceil(std::log2f(abs_quant_range));
      quant_range = (int)(sign_quant_range * std::pow(2, ceil_log_quant_range));
    }
    *scale_factor = orig_range / (quant_range);
  }
  if (power2scale == 1)
  {
    double abs_scale_factor = std::fabs(*scale_factor);
    double sign_scale_factor = *scale_factor / abs_scale_factor;
    double ceil_log_scale_factor = std::ceil(std::log2(abs_scale_factor));
    *scale_factor = sign_scale_factor * std::pow(2, ceil_log_scale_factor);
  }

  if (use_symmetric == 1)
  {
    if (i_signed == 0)
    {
      *((uint16_t *)zero_point) = (*((uint16_t *)quant_max_val) - *((uint16_t *)quant_min_val) + 1) / 2;
    }
    else
    {
      *((int16_t *)zero_point) = 0;
    }
  }
  else
  {
    min_val = std::fmin(min_val, 0.0);
    if (i_signed == 1)
    {
      *((int16_t *)zero_point) = (int16_t)(((0 - min_val) / *scale_factor) + *((int16_t *)quant_min_val) + 0.5);
    }
    else
    {
      *((uint16_t *)zero_point) = (uint16_t)(((0 - min_val) / *scale_factor) + *((uint16_t *)quant_min_val) + 0.5);
    }
  }
  return ;
}

static void TIDL_quantizeValues(double *column_data, int num_val, double min_val, double max_val, void *quant_min_val, void *quant_max_val, int16_t symmetric, int16_t power2scale, void *quant_values, double *scale_factor, void *zero_point, int16_t force_symmetric, int16_t o_signed)
{
  TIDL_getScaleZeroPoint(min_val, max_val, quant_min_val, quant_max_val, symmetric, power2scale, scale_factor, zero_point, force_symmetric, o_signed);
  for (int32_t i = 0; i < num_val; i++)
  {
    if (o_signed == 1)
    {
      double val = (column_data[i] / *scale_factor) + *((int16_t *)zero_point);
      int32_t outVal = round(val);
      outVal = (outVal > 32767) ? 32767 : outVal;
      outVal = (outVal < -32768) ? -32768 : outVal;
      ((int16_t *)quant_values)[i] = (int16_t)(outVal);
    }
    else
    {
      double val = (column_data[i] / *scale_factor) + *((uint16_t *)zero_point);
      int32_t outVal = round(val);
      outVal = (outVal > 65535) ? 65535 : outVal;
      outVal = (outVal < 0) ? 0 : outVal;
      ((uint16_t *)quant_values)[i] = (int16_t)(outVal);
    }
  }
  return ;
}

template <class Tout>
void TIDL_quantizeLUT(double *double_LUTdata, Tout *LUTdata, double *scale, int32_t *quant_min_val, int32_t *quant_max_val)
{
  double *column = (double *)my_malloc(256 * sizeof(double));
  Tout *quant_column = (Tout *)my_malloc(256 * sizeof(Tout));
  double coeff_min_val, coeff_max_val;
  int32_t lut_zero_points[2] = {0};
  int16_t is_symmetric = 0;
  int16_t is_signed = 0;
  int16_t power2scale = 1;

  int32_t imin = 0;
  imin = std::numeric_limits<Tout>::lowest();

  is_symmetric = (imin < 0) ? 1 : 0;
  is_signed = (imin < 0) ? 1 : 0;

  for (int32_t i = 0; i < COLS; i++)
  {
    coeff_max_val = 0;
    coeff_min_val = 0;
    for (int32_t j = 0; j < NUM_ELEMENTS_PERBINS; j++)
    {
      column[j] = double_LUTdata[(COLS * j) + i];
    }
    TIDL_getMinMaxVal((double *)column, NUM_ELEMENTS_PERBINS, &coeff_min_val, &coeff_max_val);
    if (i < (COLS - 1))
    {
      TIDL_quantizeValues((double *)column, NUM_ELEMENTS_PERBINS, coeff_min_val, coeff_max_val,
                          (void *)&quant_min_val[i], (void *)&quant_max_val[i], 1, power2scale, (void *)quant_column, (double *)&scale[i],
                          (void *)&lut_zero_points[i], 1, 1);
    }
    else
    {
      TIDL_quantizeValues((double *)column, NUM_ELEMENTS_PERBINS, coeff_min_val, coeff_max_val,
                          (void *)&quant_min_val[i], (void *)&quant_max_val[i], 1, power2scale, (void *)quant_column, (double *)&scale[i],
                          (void *)&lut_zero_points[i], is_symmetric, is_signed);
    }

    for (int32_t k = 0; k < NUM_ELEMENTS_PERBINS; k++)
    {
      if (i < (COLS - 1))
      {
        ((int16_t *)LUTdata)[(COLS * k) + i] = ((int16_t *)quant_column)[k];
      }
      else
      {
        ((Tout *)LUTdata)[(COLS * k) + i] = ((Tout *)quant_column)[k];
      }
    }
  }
  my_free(column);
  my_free(quant_column);
  return ;
}

void TIDL_calculateFactors(int16_t *lut_factors, double *lut_scale)
{
  int32_t log_coef_scale_factor[LINEAR_MODE] = {0};
  for (int32_t i = 0; i < LINEAR_MODE; i++)
  {
    log_coef_scale_factor[i] = (int16_t)std::log2f(lut_scale[i]);
  }

  int16_t l1 = log_coef_scale_factor[0];
  int16_t l2 = log_coef_scale_factor[1];
  lut_factors[0] = l1 - l2 - 8;
  lut_factors[1] = l2;
  return ;
}


template <class Tin, class Tout>
void TIDL_prepareSwInterpolLUT(Tin *inPtr, Tout *outPtr, double *X, double *lutData, 
  sTIDL_NonLinearAct_LUT_createParams_t *pLUTCreateParams)
{
  int32_t status;
  Tout *Y = (Tout *)my_malloc(256 * sizeof(Tout));
  double *B_prime = (double *)my_malloc(2 * sizeof(double));
  double lutDataOut;
  double zero = 0;

  int32_t omin = 0;
  int32_t isOutputSigned = 0;
  omin = std::numeric_limits<Tout>::lowest();
  if(omin < 0)
  {
    isOutputSigned = 1;
  }

  double X_T_X_inv[4] = {0.046875715266651401, -0.023346303501945515, -0.023346303501945522, 0.01553380350194552};

  for (int32_t bin = 0; bin < NUM_ELEMENTS_PERBINS; bin++)
  {
    status = TIDL_getQuantizeLUT(inPtr, (Tout *)Y, bin, pLUTCreateParams);

    B_prime[0] = 0;
    B_prime[1] = 0;
    // B_prime[1][2] = Y[1][256] *  X [256][2]; //Matrix Multiply
    for (int32_t j = 0; j < COLS; j++)
    {
      for (int32_t i = 0; i < NUM_ELEMENTS_PERBINS; i++)
      {
        B_prime[j] += ((double)1.0 * Y[i]) * X[(i * COLS) + j];
      }
    }
    // A_B[bin] [2] = B_prime[1][2] * X_T_X_inv[2][2];
    for (int32_t k = 0; k < COLS; k++)
    {
      lutData[(bin * COLS) + k] = (B_prime[0] * X_T_X_inv[(0 * COLS) + k]) + (B_prime[1] * X_T_X_inv[(1 * COLS) + k]);
    }
    if(isOutputSigned == 0)
    {
      lutDataOut = lutData[(bin * COLS) + 1];
      lutDataOut = lutDataOut < zero ? zero : lutDataOut;
      lutData[(bin * COLS) + 1] = lutDataOut; 
    }
  }

  my_free(Y);
  my_free(B_prime);
  return;
}