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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>

using namespace std;

#include "ti_dl.h"
#include "tidl_import_quantize.h"
#include "tidl_import_diag.h"
#include "tidl_import_lut.h"
#include "tidl_import_lut_core.h"

#define LUT_SIZE (512U)
#define TIDL_LUT_TABLE_SIZE_8B (256U)
#define TIDL_SW_LUT_TABLE_SIZE_16B (65536U)
#define TIDL_SW_LUT_TABLE_SIZE_INTERPOL_16B (256U)

#if defined(SOC_J784S4) || defined(SOC_J742S2) || defined(SOC_J721E) || defined(SOC_J721S2)
#define TIDL_LUT_DEVICEID 0
#elif defined(SOC_AM62A)
#define TIDL_LUT_DEVICEID 1
#elif defined(SOC_J722S) || defined(SOC_TDA54)
#define TIDL_LUT_DEVICEID 2
#else
#error "unsupported SOC"
#endif

static int32_t TIDL_prepareLUTTable(sTIDL_NonLinearAct_LUT_createParams_t *pLUTCreateParams, sBuffer_t *pLUTBuf)
{
  int32_t status = TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
  void *inPtr;
  int32_t bin;
  int32_t in_zero_point, out_zero_point, actType, inDataType, outDataType;
  float32_tidl in_scale, out_scale;

  actType = pLUTCreateParams->actType;
  in_zero_point = pLUTCreateParams->inTensorZeroPoint;
  in_scale = pLUTCreateParams->inTensorScale;
  out_zero_point = pLUTCreateParams->outTensorZeroPoint;
  out_scale = pLUTCreateParams->outTensorScale;
  inDataType = pLUTCreateParams->inElemType;
  outDataType = pLUTCreateParams->outElemType;
  bin = 0;
  
  if ((int32_t)TIDL_UnsignedChar == (int32_t)outDataType)
  {
    if ((int32_t)TIDL_SignedChar == (int32_t)inDataType)
    {
      status = TIDL_getQuantizeLUT((int8_t *)inPtr, (uint8_t *)pLUTBuf->ptr, bin, pLUTCreateParams);
    }
    else if ((int32_t)TIDL_UnsignedChar == (int32_t)inDataType)
    {
      status = TIDL_getQuantizeLUT((uint8_t *)inPtr, (uint8_t *)pLUTBuf->ptr, bin, pLUTCreateParams);
    }
    else
    {
      status = TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }
  }
  else if ((int32_t)TIDL_SignedChar == (int32_t)outDataType)
  {
    if ((int32_t)TIDL_SignedChar == (int32_t)inDataType)
    {
      status = TIDL_getQuantizeLUT((int8_t *)inPtr, (int8_t *)pLUTBuf->ptr, bin, pLUTCreateParams);
    }
    else if ((int32_t)TIDL_UnsignedChar == (int32_t)inDataType)
    {
      status = TIDL_getQuantizeLUT((uint8_t *)inPtr, (int8_t *)pLUTBuf->ptr, bin, pLUTCreateParams);
    }
    else
    {
      status = TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }
  }
  else if ((int32_t)TIDL_UnsignedShort == (int32_t)outDataType)
  {
    if ((int32_t)TIDL_SignedShort == (int32_t)inDataType)
    {
      for (int32_t bin = 0; bin < NUM_ELEMENTS_PERBINS; bin++)
      {
        status = TIDL_getQuantizeLUT((int16_t *)inPtr, ((uint16_t *)pLUTBuf->ptr + (bin * NUM_ELEMENTS_PERBINS)), bin, pLUTCreateParams);
      }
    }
    else if ((int32_t)TIDL_UnsignedShort == (int32_t)inDataType)
    {
      for (int32_t bin = 0; bin < NUM_ELEMENTS_PERBINS; bin++)
      {
        status = TIDL_getQuantizeLUT((uint16_t *)inPtr, ((uint16_t *)pLUTBuf->ptr + (bin * NUM_ELEMENTS_PERBINS)), bin, pLUTCreateParams);
      }
    }
    else
    {
      status = TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }
  }
  else if ((int32_t)TIDL_SignedShort == (int32_t)outDataType)
  {
    if ((int32_t)TIDL_SignedShort == (int32_t)inDataType)
    {
      for (int32_t bin = 0; bin < NUM_ELEMENTS_PERBINS; bin++)
      {
        status = TIDL_getQuantizeLUT((int16_t *)inPtr, ((int16_t *)pLUTBuf->ptr + (bin * NUM_ELEMENTS_PERBINS)), bin, pLUTCreateParams);
      }
    }
    else if ((int32_t)TIDL_UnsignedShort == (int32_t)inDataType)
    {
      for (int32_t bin = 0; bin < NUM_ELEMENTS_PERBINS; bin++)
      {
        status = TIDL_getQuantizeLUT((uint16_t *)inPtr, ((int16_t *)pLUTBuf->ptr + (bin * NUM_ELEMENTS_PERBINS)), bin, pLUTCreateParams);
      }
    }
    else
    {
      status = TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }
  }
  else
  {
      status = TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  
  if(status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL){
    TIDL_GLOBAL_REPORT_ERROR("Couldn't prepare quantized LUT eaither due to invalid Data type  [IN, OUT] = [%d, %d] or supplied for activation Method  = %d", 
      inDataType, outDataType, pLUTCreateParams->actType);
  } 
  return status ;
}

static int32_t  TIDL_prepareInterpolatedLUT(sTIDL_NonLinearAct_LUT_createParams_t *pLUTCreateParams, sBuffer_t *pLUTBuf, sTIDL_NonLinearActivation_LUT_t *pLUTParams)
{

  int32_t status = TIDL_IMPORT_DIAGNOSIS_RETURN_OK ;
  double *X = (double *)my_malloc(LUT_SIZE * sizeof(double));
  double *double_lutData = (double *)my_malloc(LUT_SIZE * sizeof(double));
  double lut_scale[2] = {0};
  int16_t lut_factors[2] = {0};
  int32_t quant_min_lut[2] = {0};
  int32_t quant_max_lut[2] = {0};
  int32_t lutDatatype;
  void *inPtr, *outPtr;
  int32_t  inDataType, outDataType;

  inDataType = pLUTCreateParams->inElemType;
  outDataType = pLUTCreateParams->outElemType;

  TIDL_prepareXval(X);

  //slope is in the range of -32768 to 32767 ( always signed), whereas for bias we consider based upon output signed or unsigned
  quant_min_lut[0] = -32768;
  quant_max_lut[0] = 32767;

  //Tried multiple C++ aproaches to avoid this nested long if else branch but not any better way 
  //In C++, it is not possible to dynamically determine the type of a variable at runtime and use it to call a function with the correct type parameters. 
  //The language does not support this level of dynamic typing. Tried unordered map, reinterpret_cast and few more appaorches and concluded that
  // the current implementation is good for now. Oython would have given more freedom though

  if ((int32_t)TIDL_UnsignedShort == (int32_t)outDataType)
  {
    quant_min_lut[1] = 0;
    quant_max_lut[1] = 65535;

    if ((int32_t)TIDL_SignedShort == (int32_t)inDataType)
    {
      TIDL_prepareSwInterpolLUT((int16_t *)inPtr, (uint16_t *)outPtr, (double *)X, (double *)double_lutData, pLUTCreateParams);
      TIDL_quantizeLUT((double *)double_lutData, (uint16_t *)pLUTBuf->ptr, (double *)&lut_scale, (int32_t *)quant_min_lut, (int32_t *)quant_max_lut);
      TIDL_calculateFactors((int16_t *)lut_factors, (double *)&lut_scale);
    }
    else if ((int32_t)TIDL_UnsignedShort == (int32_t)inDataType)
    {
      TIDL_prepareSwInterpolLUT((uint16_t *)inPtr, (uint16_t *)outPtr, (double *)X, (double *)double_lutData, pLUTCreateParams);
      TIDL_quantizeLUT((double *)double_lutData, (uint16_t *)pLUTBuf->ptr, (double *)&lut_scale, (int32_t *)quant_min_lut, (int32_t *)quant_max_lut);
      TIDL_calculateFactors((int16_t *)lut_factors, (double *)&lut_scale);
    }
    else
    {
    }
  }
  else if ((int32_t)TIDL_SignedShort == (int32_t)outDataType)
  {
    quant_min_lut[1] = -32768;
    quant_max_lut[1] = 32767;

    if ((int32_t)TIDL_SignedShort == (int32_t)inDataType)
    {
      TIDL_prepareSwInterpolLUT((int16_t *)inPtr, (int16_t *)outPtr, (double *)X, (double *)double_lutData, pLUTCreateParams);
      TIDL_quantizeLUT((double *)double_lutData, (int16_t *)pLUTBuf->ptr, (double *)&lut_scale, (int32_t *)quant_min_lut, (int32_t *)quant_max_lut);
      TIDL_calculateFactors((int16_t *)lut_factors, (double *)&lut_scale);
    }
    else if ((int32_t)TIDL_UnsignedShort == (int32_t)inDataType)
    {
      TIDL_prepareSwInterpolLUT((uint16_t *)inPtr, (int16_t *)outPtr, (double *)X, (double *)double_lutData, pLUTCreateParams);
      TIDL_quantizeLUT((double *)double_lutData, (int16_t *)pLUTBuf->ptr, (double *)&lut_scale, (int32_t *)quant_min_lut, (int32_t *)quant_max_lut);
      TIDL_calculateFactors((int16_t *)lut_factors, (double *)&lut_scale);
    }
    else
    {
    }
  }
  if (pLUTBuf->ptr != NULL)
  {
    pLUTParams->factorB = lut_factors[0];
    pLUTParams->factorC = lut_factors[1];
  }

  my_free(X);
  my_free(double_lutData);
  return status ;
}

int32_t TIDL_nonLinearActLUT_prepare(sTIDL_NonLinearAct_LUT_createParams_t *pLUTCreateParams,
                                     sBuffer_t *pLUTBuf, sTIDL_NonLinearActivation_LUT_t *pLUTParams)
{
  int32_t nonLinearActMethod = pLUTParams->nonLinearActMethod;

  /* Switch case to decide the lut preparation based on the Activation Method*/
  switch (nonLinearActMethod)
  {
  case 0: // TIDL_SW_NONLUT_8B
  case 1: // TIDL_SW_NONLUT_16B
    /* Nothing to be prepared for this mode only update the Non Linear Activation Method*/
    break;
  case 2: // TIDL_SW_SINGLEPOINT_LUT_8B
  case 3: // TIDL_SW_SINGLEPOINT_LUT_16B
  case 4: // TIDL_HW_LUT_8B
  case 5: // TIDL_HW_ILUT_8B
    TIDL_prepareLUTTable(pLUTCreateParams, pLUTBuf);
    break;
  case 6: // TIDL_HW_INTERPOLATED_LUT_16B
    TIDL_prepareInterpolatedLUT(pLUTCreateParams, pLUTBuf, pLUTParams);
    break;
  default:
    break;
  }
  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

static int32_t TIDL_checkUnsupportedNonLutActType(int32_t nonLinearactType){

  int32_t notSupported = 0;
  if((nonLinearactType == TIDL_Asin)         || 
     (nonLinearactType == TIDL_Acos)         || 
     (nonLinearactType == TIDL_Exp)          || 
     (nonLinearactType == TIDL_Log)          ||  
     (nonLinearactType == TIDL_Mish)         || 
     (nonLinearactType == TIDL_Asinh)        ||
     (nonLinearactType == TIDL_Pow)          ||  
     (nonLinearactType == TIDL_Tan)          || 
     (nonLinearactType == TIDL_Logit)        ||
     (nonLinearactType == TIDL_Reciprocal))         
     {
        notSupported = 1;
     }
      return notSupported;
  
};

int32_t TIDL_getFinalnonLinearActMethod(sTIDL_NonLinearAct_LUT_importActParams_t *importActParams, int32_t inDataType, int32_t outDataType, int32_t nonLinearactType)
{
  int32_t nonLinearActMethod = -1;

  for (int32_t idx = 0; idx < importActParams->numActType; idx++)
  {
    //If the user given Activation Type is matched with the Incoming activation type then the nonLinearActMethod is selected based on user input
    if(importActParams->importActTypeTIDL[idx] == nonLinearactType)
    {
      nonLinearActMethod = importActParams->importActMethod[idx];
      switch (nonLinearActMethod)
      {
        case 0:
          if ((inDataType == (int32_t)TIDL_UnsignedChar || inDataType == (int32_t)TIDL_SignedChar) &&
          (outDataType == (int32_t)TIDL_UnsignedChar || outDataType == (int32_t)TIDL_SignedChar))
          {
            if(TIDL_checkUnsupportedNonLutActType(nonLinearactType))
            {
              TIDL_GLOBAL_REPORT_WARNING("The selected %d actType is not supported in TIDL_SW_NONLUT[0] actMethod, so setting it to default actMethod or try either TIDL_HW_LUT[2] or TIDL_SW_SINGLEPOINT_LUT[1] actMethod", nonLinearactType);
              nonLinearActMethod = -1;
            }
            else
            {
              nonLinearActMethod = TIDL_SW_NONLUT_8B;
            }
          }
          else if ((inDataType == (int32_t)TIDL_UnsignedShort || inDataType == (int32_t)TIDL_SignedShort) &&
              (outDataType == (int32_t)TIDL_UnsignedShort || outDataType == (int32_t)TIDL_SignedShort))
          {
            if(TIDL_checkUnsupportedNonLutActType(nonLinearactType))
            {
              TIDL_GLOBAL_REPORT_WARNING("The selected %d actType is not supported in TIDL_SW_NONLUT[0] actMethod, so setting it to default actMethod or try either TIDL_HW_LUT[2] or TIDL_SW_SINGLEPOINT_LUT[1] actMethod", nonLinearactType);
              nonLinearActMethod = -1;
            }
            else
            {
              nonLinearActMethod = TIDL_SW_NONLUT_16B;
            }
          }
          else{
            TIDL_GLOBAL_REPORT_ERROR("Invalid Data type  [IN, OUT] = [%d, %d] supplied for activation Method  = %d", 
              inDataType, outDataType, nonLinearactType);
          }
          break;

        case 1:
          if ((inDataType == (int32_t)TIDL_UnsignedChar || inDataType == (int32_t)TIDL_SignedChar) &&
          (outDataType == (int32_t)TIDL_UnsignedChar || outDataType == (int32_t)TIDL_SignedChar))
          {
            nonLinearActMethod = TIDL_SW_SINGLEPOINT_LUT_8B;
          }
          else if ((inDataType == (int32_t)TIDL_UnsignedShort || inDataType == (int32_t)TIDL_SignedShort) &&
              (outDataType == (int32_t)TIDL_UnsignedShort || outDataType == (int32_t)TIDL_SignedShort))
          {
            nonLinearActMethod = TIDL_SW_SINGLEPOINT_LUT_16B;
          }
          else{
            TIDL_GLOBAL_REPORT_ERROR("Invalid Data type  [IN, OUT] = [%d, %d] supplied for activation Method  = %d", 
              inDataType, outDataType, nonLinearactType);
          }
          break;

        case 2:
          if ((inDataType == (int32_t)TIDL_UnsignedChar || inDataType == (int32_t)TIDL_SignedChar) &&
          (outDataType == (int32_t)TIDL_UnsignedChar || outDataType == (int32_t)TIDL_SignedChar))
          {
            #if defined(SOC_J784S4) || defined(SOC_J742S2) || defined(SOC_J721E) || defined(SOC_J721S2)
            nonLinearActMethod = TIDL_HW_LUT_8B;
            #elif defined(SOC_J722S)
            nonLinearActMethod = TIDL_HW_ILUT_8B;
            #else
            nonLinearActMethod = -1;
            TIDL_GLOBAL_REPORT_WARNING("The Selected actMethod %d is not suitable for this device, please select either actMethod TIDL_SW_NONLUT[0] or TIDL_SW_SINGLEPOINT_LUT[1] for the actType %d for the Data type  [IN, OUT] = [%d, %d], , setting default actMethod now", 
                                      nonLinearActMethod, nonLinearactType, inDataType, outDataType);
            #endif
          }
          else if ((inDataType == (int32_t)TIDL_UnsignedShort || inDataType == (int32_t)TIDL_SignedShort) &&
              (outDataType == (int32_t)TIDL_UnsignedShort || outDataType == (int32_t)TIDL_SignedShort))
          {
            #if defined(SOC_J784S4) || defined(SOC_J742S2) || defined(SOC_J721E) || defined(SOC_J721S2)
            nonLinearActMethod = TIDL_HW_INTERPOLATED_LUT_16B;
            #else
            nonLinearActMethod = -1;
            TIDL_GLOBAL_REPORT_WARNING("The Selected actMethod %d is not suitable for this device, please select either actMethod TIDL_SW_NONLUT[0] or TIDL_SW_SINGLEPOINT_LUT[1]  for the actType %d for the Data type  [IN, OUT] = [%d, %d], , setting default actMethod now", 
                                      nonLinearActMethod, nonLinearactType, inDataType, outDataType);
            #endif
          }
          else{
            TIDL_GLOBAL_REPORT_ERROR("Invalid Data type  [IN, OUT] = [%d, %d] supplied for activation Method  = %d", 
              inDataType, outDataType, nonLinearactType);
          }
          break;
    
        default:
          TIDL_GLOBAL_REPORT_WARNING("Please select valid actMethod either TIDL_SW_NONLUT[0], TIDL_SW_SINGLEPOINT_LUT[1] , or TIDL_HW_LUT[2], setting default actMethod now");
          break;
      }
    }
  }
  //If the user given Activation Type is not matched then the execution flow will select nonLinearActMethod from the predefined table
  if(nonLinearActMethod == -1)
  {
    nonLinearActMethod = TIDL_getNonLinearActMethod(inDataType, outDataType,
                                                (nonLinearactType - ADJUST_NUM_NON_LINEAR_OPS), TIDL_LUT_DEVICEID);
  }
  
  return nonLinearActMethod;
}

int32_t TIDL_nonLinearActLUT_allocate(sTIDL_NonLinearAct_LUT_createParams_t *pLUTCreateParams,
                                      sBuffer_t *pLUTBuf, sTIDL_NonLinearActivation_LUT_t *pLUTParams,
                                      sTIDL_NonLinearAct_LUT_importActParams_t *importActParams)
{
  // In this function performing only allocation for the LUT buffer
  int32_t nonLinearActMethod;
  int32_t inDataType = pLUTCreateParams->inElemType;
  int32_t outDataType = pLUTCreateParams->outElemType;
  int32_t nonLinearactType = pLUTCreateParams->actType;
  //Fucntion to get the correct nonLinear Activation Method
  nonLinearActMethod = TIDL_getFinalnonLinearActMethod(importActParams, inDataType, outDataType, nonLinearactType);
  pLUTParams->nonLinearActMethod = nonLinearActMethod;
  // bufSize in unit of bytes. For other usage also it should be in unit of bytes for easy allocation and write and should be corrected
  if (pLUTBuf->ptr == NULL)
  {
    switch (nonLinearActMethod)
    {
    // No LUT cases
    case 0: // TIDL_SW_NONLUT_8B /* Nothing to be prepared for this mode only update the Non Linear Activation Method*/
    case 1: // TIDL_SW_NONLUT_16B /* Nothing to be prepared for this mode only update the Non Linear Activation Method*/
      pLUTBuf->bufSize = 0;
      break;
    // 8 bit LUT cases, 8 bit index, 8 bit output
    case 2: // TIDL_SW_SINGLEPOINT_LUT_8B
    case 4: // TIDL_HW_LUT_8B
    case 5: // TIDL_HW_ILUT_8B
      pLUTBuf->bufSize = TIDL_LUT_TABLE_SIZE_8B * sizeof(uint8_t);
      break;
    // 16 bit LUT cases - single point only, 16 bit index, 16 bit output
    case 3: // TIDL_SW_SINGLEPOINT_LUT_16B
      pLUTBuf->bufSize = TIDL_SW_LUT_TABLE_SIZE_16B * sizeof(uint16_t);
      break;
    // specail case of interpolated  LUT , 8 bit index, 32 bit output
    case 6: // TIDL_HW_INTERPOLATED_LUT_16B
      pLUTBuf->bufSize = TIDL_SW_LUT_TABLE_SIZE_INTERPOL_16B * sizeof(uint32_t);
      break;
    default:
      break;
    }
  }
  if ((pLUTBuf->bufSize != 0) && (pLUTBuf->ptr == NULL))
    pLUTBuf->ptr = my_malloc(pLUTBuf->bufSize);
  pLUTParams->lutBufSize = pLUTBuf->bufSize;

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

