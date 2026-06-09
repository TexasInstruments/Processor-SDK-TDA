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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <cmath>
#ifdef _WIN32
#include <asprintf.h>
#endif

#include "tidl_import_quantize.h"
#include "tidl_deviceInfo.h"
#include "tidl_import_diag.h"
#include "tidl_import_lut.h"

#define __MODULE__ "[QUANTIZATION]"

#define QUAN_STYLE2_ROUND ((gParams.quantRoundAdd*1.0 / 100))
char *currLayerName;
int currElemenType;

FILE *paramDebugFile = NULL;
int debugLayeId = 0;
#define ENABLE_HIST_BASED_RANGE (0)
#define USE_16BIT_BIAS_FOR_8BIT_MODE (1)
#define SOFTMAX_16_BIT_SCALE_LIM (254U)
#define SYMMETRIC_8BIT_SIGNED_MAX (127.0)
#define SYMMETRIC_16BIT_SIGNED_MAX (32767.0)
#define SYMMETRIC_16BIT_UNSIGNED_MAX (65535.0)
#define TIDL_MAX_GRID_TRAVERSAL (32768)
static std::unordered_set<int> quantizationPassThroughLayers = {
  TIDL_CropLayer,
  TIDL_ReshapeLayer,
  TIDL_TransposeLayer,
  TIDL_DataConvertLayer,
  TIDL_DetectionOutputLayer,
  TIDL_GridSampleLayer, /*Passthrough logic is only checking idx '0' input which is gridsample's input, so we can use it as a passthrough layer*/
  TIDL_BatchReshapeLayer,
  TIDL_GatherLayer,
  TIDL_ReduceLayer,
  TIDL_SliceLayer,
  TIDL_TopKLayer,
  TIDL_PoolingLayer,
  TIDL_ResizeLayer,
  TIDL_TileLayer
}; 
static std::unordered_map<int, bool> quantizationPassThroughMap = {};
static std::map<int32_t, pair<float32_tidl, float32_tidl>> floatRangeMap = {};
static std::unordered_map<int32_t, pair<float32_tidl, float32_tidl>> activationProducerRangeMap = {
  {TIDL_Sigmoid, {-6.0f, 6.0f}},
  {TIDL_Tanh,    {-4.0f, 4.0f}},
  {TIDL_HardSigmoid, {-3.0f, 3.0f}},
  {TIDL_ELU,    {-4.0f, FLT_MAX}},
  {TIDL_GELU,  {-5.0f, FLT_MAX}}, // GeLU@-5.0 = -1.43325786e-06
  {TIDL_Asin,  {-1.0f, 1.0f}},
  {TIDL_HardSwish, {-3.0f, FLT_MAX}},
  {TIDL_Sign,  {-0.1f, 0.1f}}
};

//Mapping table that maps the Activation Type string with the TIDL integer list
static const uint8_t importActTypeMapping[TIDL_TOTAL_NONLINEAR_ACT_OPS][TIDL_MAX_ACTTYPE_NAME]
{
  {"NoAct"      },          // 0
  {"RelU"       },          // 1
  {"PRelU"      },          // 2
  {"RelU6"      },          // 3
  {"Clip"       },          // 4
  {"sigmoid"    },          // 5
  {"tanh"       },          // 6
  {"hardsigmoid"},          // 7
  {"elu"        },          // 8
  {"gelu"       },          // 9
  {"leakyrelU"  },          // 10
  {"asin"       },          // 11
  {"hardswish"  },          // 12
  {"mish"       },          // 13
  {"log"        },          // 14
  {"asinh"      },          // 15
  {"abs"        },          // 16
  {"sin"        },          // 17
  {"exp"        },          // 18
  {"pow"        },          // 19
  {"floor"      },          // 20
  {"erf"        },          // 21
  {"sqrt"       },          // 22
  {"acos"       },          // 23
  {"atan"       },          // 24
  {"sinh"       },          // 25
  {"cos"        },          // 26
  {"cosh"       },          // 27
  {"neg"        },          // 28
  {"tan"        },          // 29
  {"logit"      },          // 30
  {"reciprocal" },          // 31
  {"silu"  },               // 32
  {"swish"  },              // 33
  {"softplus"  },           // 34
  {"softsign"  },           // 35
  {"ceil"  },               // 36
  {"celu"  },               // 37
  {"selu"  },               // 38
  {"round"  },              // 39
};

float32_tidl TIDL_clamp(float32_tidl value, float32_tidl minVal, float32_tidl maxVal)
{
  if(value < minVal) return minVal;
  if(value > maxVal) return maxVal;
  return value;
}

//DUPLICATED FROM STRINGS.CPP:
typedef std::map<int, const std::string> stringmap;
// Given a key, find it in the map, or construct an <unknown:N> value
static std::string find(int32_t key, const stringmap& smap)
{
   auto it = smap.find(key);
   if (it != smap.end())
      return it->second;
   std::ostringstream oss;
   oss << "<unknown:" << key << ">";
   return oss.str();
}
std::string layerTypeString(int32_t type)
{
   stringmap layerTypeNames = 
   {
      { TIDL_DataLayer, "TIDL_DataLayer" },
      { TIDL_ConvolutionLayer, "TIDL_ConvolutionLayer" },
      { TIDL_PoolingLayer, "TIDL_PoolingLayer" },
      { TIDL_ReLULayer, "TIDL_ReLULayer" },
      { TIDL_PReLULayer, "TIDL_PReLULayer" },
      { TIDL_EltWiseLayer, "TIDL_EltWiseLayer" },
      { TIDL_InnerProductLayer, "TIDL_InnerProductLayer" },
      { TIDL_SoftMaxLayer, "TIDL_SoftMaxLayer" },
      { TIDL_BatchNormLayer, "TIDL_BatchNormLayer" },
      { TIDL_BiasLayer, "TIDL_BiasLayer" },
      { TIDL_ScaleLayer, "TIDL_ScaleLayer" },
      { TIDL_Deconv2DLayer, "TIDL_Deconv2DLayer" },
      { TIDL_ConcatLayer, "TIDL_ConcatLayer" },
      { TIDL_SplitLayer, "TIDL_SplitLayer" },
      { TIDL_SliceLayer, "TIDL_SliceLayer" },
      { TIDL_CropLayer, "TIDL_CropLayer" },
      { TIDL_FlattenLayer, "TIDL_FlattenLayer" },
      { TIDL_DropOutLayer, "TIDL_DropOutLayer" },
      { TIDL_ArgOpLayer, "TIDL_ArgOpLayer" },
      { TIDL_DetectionOutputLayer, "TIDL_DetectionOutputLayer" },
      { TIDL_ShuffleChannelLayer, "TIDL_ShuffleChannelLayer" },
      { TIDL_ResizeLayer, "TIDL_ResizeLayer" },
      { TIDL_RoiPoolingLayer, "TIDL_RoiPoolingLayer" },
      { TIDL_OdPostProcessingLayer, "TIDL_OdPostProcessingLayer" },
      { TIDL_DepthToSpaceLayer, "TIDL_DepthToSpaceLayer" },
      { TIDL_SigmoidLayer, "TIDL_SigmoidLayer" },
      { TIDL_TanhLayer, "TIDL_TanhLayer" },
      { TIDL_HardSigmoidLayer, "TIDL_HardSigmoidLayer" },
      { TIDL_ELULayer, "TIDL_ELULayer" },
      { TIDL_PadLayer, "TIDL_PadLayer" },
      { TIDL_ColorConversionLayer, "TIDL_ColorConversionLayer" },
      { TIDL_OdOutputReformatLayer, "TIDL_OdOutputReformatLayer" },
      { TIDL_DataConvertLayer, "TIDL_DataConvertLayer" },
      { TIDL_CustomLayer, "TIDL_CustomLayer" },
      { TIDL_BatchReshapeLayer, "TIDL_BatchReshapeLayer" },
      { TIDL_ReduceLayer, "TIDL_ReduceLayer" },
      { TIDL_ScatterElementsLayer, "TIDL_ScatterElementsLayer" },
      { TIDL_SqueezeLayer, "TIDL_SqueezeLayer" },
      { TIDL_ReshapeLayer, "TIDL_ReshapeLayer" },
      { TIDL_ConstDataLayer, "TIDL_ConstDataLayer"},
      { TIDL_GatherLayer, "TIDL_GatherLayer"},
      { TIDL_TransposeLayer, "TIDL_TransposeLayer" },
      { TIDL_LayerNormLayer, "TIDL_LayerNormLayer"},
      { TIDL_GridSampleLayer, "TIDL_GridSampleLayer"},
      { TIDL_DeformableConvLayer, "TIDL_DeformableConvLayer"},
      { TIDL_TopKLayer, "TIDL_TopKLayer"},
      { TIDL_TileLayer, "TIDL_TileLayer"},
      { TIDL_LogicalOpLayer, "TIDL_LogicalOpLayer"},
      { TIDL_LSTMLayer, "TIDL_LSTMLayer"},
      { TIDL_GRULayer, "TIDL_GRULayer"},
      { TIDL_RNNLayer, "TIDL_RNNLayer"},
      { TIDL_SoftPlusLayer, "TIDL_SoftPlusLayer" },
      { TIDL_SoftSignLayer, "TIDL_SoftSignLayer" },
      { TIDL_CeilLayer, "TIDL_CeilLayer" },
      { TIDL_CeluLayer, "TIDL_CeluLayer" },
      { TIDL_SeluLayer, "TIDL_SeluLayer" },
      { TIDL_RoundLayer, "TIDL_RoundLayer" },
      { TIDL_SignLayer, "TIDL_SignLayer" },
      { TIDL_UnsupportedLayer, "TIDL_UnsupportedLayer" },
   };
   return find(type, layerTypeNames);
}

std::string actTypeShort(int32_t type)
{
  stringmap actTypeNames = { 
                              { TIDL_NoAct, "NoAct" }, \
                              { TIDL_RelU, "RelU" }, \
                              { TIDL_PRelU, "PRelU" }, \
                              { TIDL_RelU6, "RelU6" }, \
                              { TIDL_Clip, "Clip" }, \
                              { TIDL_Sigmoid, "Sigmoid" }, \
                              { TIDL_Tanh, "Tanh" }, \
                              { TIDL_HardSigmoid, "HardSigmoid" }, \
                              { TIDL_ELU, "ELU" }, \
                              { TIDL_GELU, "GELU" }, \
                              { TIDL_LeakyReLU, "LeakyReLU" }, \
                              { TIDL_Asin, "Asin" }, \
                              { TIDL_HardSwish, "HardSwish" }, \
                              { TIDL_Mish, "Mish" }, \
                              { TIDL_Log, "Log" }, \
                              { TIDL_Asinh, "Asinh" }, \
                              { TIDL_Abs, "Abs" }, \
                              { TIDL_Sin, "Sin" }, \
                              { TIDL_Exp, "Exp" }, \
                              { TIDL_Pow, "Pow" }, \
                              { TIDL_Floor, "Floor" }, \
                              { TIDL_Erf, "Erf" }, \
                              { TIDL_Sqrt, "Sqrt" }, \
                              { TIDL_Acos, "Acos" }, \
                              { TIDL_Atan, "Atan" }, \
                              { TIDL_Sinh, "Sinh" }, \
                              { TIDL_Cos, "Cos" }, \
                              { TIDL_Cosh, "Cosh" }, \
                              { TIDL_Neg, "Neg"},\
                              { TIDL_Tan,"Tan"}, \
                              { TIDL_Logit, "Logit" }, \
                              { TIDL_Reciprocal, "Reciprocal" }, \
                              { TIDL_SiLU , "SiLU" }, \
                              { TIDL_SoftPlus , "Softplus" }, \
                              { TIDL_SoftSign , "Softsign" }, \
                              { TIDL_Ceil , "Ceil" }, \
                              { TIDL_Celu , "Celu" }, \
                              { TIDL_Selu , "Selu" }, \
                              { TIDL_Round , "Round" }, \
                              { TIDL_Sign , "Sign" }
                           };
   return find(type, actTypeNames);
}

/**
 * @brief Function to dump layer quantization information to a CSV file
 *
 * This function dumps the following information for each layer to a CSV file:
 * - Layer Index
 * - Output Data Name (from outDataNames[0] - the 0th output tensor name)
 * - Layer Type (converted to string using layerTypeString)
 * - Activation Type (converted to string using actTypeShort)
 * - Tensor Scale (for output tensor)
 * - Tensor Zero Point (for output tensor)
 * - Min Tensor Value (for output tensor)
 * - Max Tensor Value (for output tensor)
 *
 * The function follows the pattern used in TIDL_writeInfo for file creation
 *
 * @param pOrgTIDLNetStructure : Pointer to the original TIDL network structure
 * @param name : Base name for the output CSV file
 * @param numLayers : Number of layers in the network
 * @param currLayersGroupId : Current layer group ID (for multi-group networks)
 * @return int32_t : TIDL_IMPORT_DIAGNOSIS_RETURN_OK on success, error code on failure
 */
int32_t TIDL_dumpQuantizationInfo(const sTIDL_OrgNetwork_t * pOrgTIDLNetStructure,
                                   const char * name,
                                   uint32_t numLayers,
                                   uint32_t currLayersGroupId)
{
  FILE * fp = NULL;
  char fileName[512];
  char numString[64];
  
  if (pOrgTIDLNetStructure == NULL || name == NULL)
  {
    TIDL_GLOBAL_REPORT_ERROR("Invalid parameters passed to TIDL_dumpQuantizationInfo");
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  // Construct the CSV filename similar to TIDL_writeInfo
  strcpy(fileName, name);
  strcat(fileName, "_quant_info.csv");

  // Open the CSV file for writing
  fp = fopen(fileName, "w+");
  if (fp == NULL)
  {
    TIDL_GLOBAL_REPORT_ERROR("Could not open %s file for writing, Aborting", (const char *)fileName);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  // Write CSV header
  fprintf(fp, "LayerIdx,OutputDataName,LayerType,ActivationType,TensorScale,TensorZeroPoint,MinTensorValue,MaxTensorValue\n");

  // Iterate through all layers and dump quantization information
  for (uint32_t i = 0; i < numLayers; i++)
  {
    const sTIDL_LayerPC_t * pLayer = &pOrgTIDLNetStructure->TIDLPCLayers[i];
    
    // Get output tensor information (outData[0] contains the primary output)
    const sTIDL_DataParams_t * outData = &pLayer->outData[0];
    
    // Write layer index
    fprintf(fp, "%u,", i);
    
    // Write output data name (0th output from outDataNames)
    if (pLayer->outDataNames[0][0] != '\0')
    {
      fprintf(fp, "%s,", (const char *)pLayer->outDataNames[0]);
    }
    else
    {
      fprintf(fp, "Layer_%u,", i);
    }
    
    // Write layer type using layerTypeString
    std::string layerTypeStr = layerTypeString(pLayer->layerType);
    fprintf(fp, "%s,", layerTypeStr.c_str());
    
    // Write activation type using actTypeShort
    std::string actTypeStr = actTypeShort(pLayer->actParams.actType);
    fprintf(fp, "%s,", actTypeStr.c_str());
    
    // Write tensor scale
    fprintf(fp, "%.10f,", outData->tensorScale);
    
    // Write tensor zero point
    fprintf(fp, "%d,", outData->tensorZeroPoint);
    
    // Write min tensor value
    fprintf(fp, "%.10f,", outData->minTensorValue);
    
    // Write max tensor value
    fprintf(fp, "%.10f\n", outData->maxTensorValue);
  }

  fclose(fp);
  
  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, 
                          "Quantization information dumped to: %s", fileName);
  
  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

//Fucntion converts the Activation Type String to TIDL Integer type
void TIDL_convertImportActTypefromStrToTIDL(sTIDL_NonLinearAct_LUT_importActParams_t *importActParams)
{
  for(int32_t i = 0; i < importActParams->numActType; i++)
  {
    for(int32_t j = 0; j < TIDL_TOTAL_NONLINEAR_ACT_OPS; j++)
    {
      if(!strcmp((char*)importActParams->importActTypeStr[i], (char*)importActTypeMapping[j]))
      {
        importActParams->importActTypeTIDL[i] = j;
      }
    }
  }
}

//Function to convert the capital and small letter conbination from the user
void TIDL_convertActTypeStrtoLwrCase(sTIDL_NonLinearAct_LUT_importActParams_t *importActParams)
{
  uint8_t temp[TIDL_MAX_ACTTYPE_NAME];
  for(int32_t i = 0; i < importActParams->numActType; i++)
  {
    strcpy((char*)temp, (char*)importActParams->importActTypeStr[i]);
    for(int32_t j = 0; j < TIDL_MAX_ACTTYPE_NAME; j++)
    {
      temp[j] = tolower(temp[j]);
    }
    strcpy((char*)importActParams->importActTypeStr[i], (char*)temp);
  }
}

void TIDL_prepareLUTForActivations_wrapper(sTIDL_OrgNetwork_t * orgTIDLNetStructure, int32_t tidlNetVersion, tidl_import_config *configParams)
{
  uint32_t idx;
  int32_t numLayers = orgTIDLNetStructure->numLayers;
  int32_t numActType = 0;
  sTIDL_LayerPC_t *pTIDL_LayerPC ;
  sTIDL_NonLinearAct_LUT_createParams_t lutGenParams;
  sTIDL_NonLinearAct_LUT_importActParams_t importActParams;
  importActParams.numActType = 0;
  memset(importActParams.importActMethod, -1, (TIDL_MAX_ALG_IN_BUFS * sizeof(int32_t)));
  memset(importActParams.importActTypeTIDL, -1, (TIDL_MAX_ALG_IN_BUFS * sizeof(int32_t)));
  memset(importActParams.importActTypeStr, -1, (TIDL_MAX_ALG_IN_BUFS * TIDL_MAX_ACTTYPE_NAME * sizeof(uint8_t)));

  if (strcmp((char *)configParams->actType, "") != 0)
  {
    numActType = tidl_getStringsFromList((char *)configParams->actType, (char *)importActParams.importActTypeStr, TIDL_MAX_ACTTYPE_NAME);
    importActParams.numActType = numActType;
    for(int32_t i = 0; i < numActType; i++)
    {
      importActParams.importActMethod[i] = configParams->actMethod[i];
    }
    TIDL_convertActTypeStrtoLwrCase(&importActParams);
    TIDL_convertImportActTypefromStrToTIDL(&importActParams);
  }
  
  for (idx = 0; idx < numLayers; idx++)
  {
    pTIDL_LayerPC = &orgTIDLNetStructure->TIDLPCLayers[idx] ;
    if (pTIDL_LayerPC->layerType == TIDL_BatchNormLayer)
    {
      const sTIDL_DataParams_t * inData = TIDL_getOutData(orgTIDLNetStructure, orgTIDLNetStructure->TIDLPCLayers[idx].inData[0].dataId);
      const sTIDL_DataParams_t * outData = TIDL_getOutData(orgTIDLNetStructure, orgTIDLNetStructure->TIDLPCLayers[idx].outData[0].dataId);
      lutGenParams.actType            = pTIDL_LayerPC->actParams.actType;
      lutGenParams.inTensorZeroPoint  = inData->tensorZeroPoint;
      lutGenParams.inTensorScale      = inData->tensorScale;
      lutGenParams.outTensorZeroPoint = outData->tensorZeroPoint;
      lutGenParams.outTensorScale     = outData->tensorScale;
      lutGenParams.inElemType         = inData->elementType;
      lutGenParams.outElemType        = outData->elementType;
      //Set default values
      lutGenParams.alpha = 0.0; lutGenParams.beta = 0.0 ; lutGenParams.pow = 0.0 ;
      lutGenParams.tidlNetVersion     = tidlNetVersion;

      if(lutGenParams.actType == TIDL_ELU){
        lutGenParams.alpha = (double)pTIDL_LayerPC->layerParams.batchNormParams.inDataQ;
      }
      else if(lutGenParams.actType == TIDL_HardSigmoid){
        lutGenParams.alpha = (double)pTIDL_LayerPC->layerParams.batchNormParams.inDataQ;
        lutGenParams.beta  = (double)pTIDL_LayerPC->layerParams.batchNormParams.weightsQ;
        
      }
      else if(lutGenParams.actType == TIDL_LeakyReLU){
        /* Using slopeScale to get leakyRelu alpha, cannot use leakyReluParams as layerType is TIDL_BatchNormLayer */
        lutGenParams.alpha = (double)pTIDL_LayerPC->actParams.slopeScale;
      }
      else if(lutGenParams.actType == TIDL_Pow){
        lutGenParams.pow = (double)pTIDL_LayerPC->layerParams.batchNormParams.inDataQ;     
      }
      else if(lutGenParams.actType == TIDL_Swish)
      {
        lutGenParams.alpha = (double)pTIDL_LayerPC->actParams.alpha;
      }
      else if(lutGenParams.actType == TIDL_Celu){
        lutGenParams.alpha = (double)pTIDL_LayerPC->layerParams.batchNormParams.inDataQ;     
      }
      else if(lutGenParams.actType == TIDL_Selu){
        lutGenParams.alpha = (double)pTIDL_LayerPC->layerParams.batchNormParams.inDataQ;
        lutGenParams.beta  = (double)pTIDL_LayerPC->layerParams.batchNormParams.weightsQ;  
      }
      //Avoid allocating for the following operators since there is no separate kernel implementation
      if((lutGenParams.actType != TIDL_NoAct) && (lutGenParams.actType != TIDL_RelU) && (lutGenParams.actType != TIDL_PRelU) && (lutGenParams.actType != TIDL_RelU6) && 
          (lutGenParams.actType != TIDL_Clip))
      {
        /* Allocate the required memory for the Non Linear Activation layers */
        TIDL_nonLinearActLUT_allocate(&lutGenParams, &pTIDL_LayerPC->lutData, &pTIDL_LayerPC->actParams.lutParams, &importActParams);

        TIDL_nonLinearActLUT_prepare(&lutGenParams, &pTIDL_LayerPC->lutData, &pTIDL_LayerPC->actParams.lutParams);
      }
      
    }
  }

}

/**
 * @brief Function to find the max scale across diffent weights and biases
 *
 * @param pOrgTIDLNetStructure : Network structure to get weight and bias pointers
 * @param dataId : ID of the data buffer being processed
 * @return float32_tidl : returns the max weight scale
 */
float32_tidl  TIDL_maxWeightScale(const sTIDL_OrgNetwork_t * pOrgTIDLNetStructure, int32_t dataId)
{
  int32_t k, l;
  float32_tidl maxScale = 0.0001f;
  float32_tidl scale_for_non_conv = 1.0f/256.0f; //1
  float32_tidl scale;
  for (k = 0; k < pOrgTIDLNetStructure->numLayers; k++)
  {
    for (l = 0; l < pOrgTIDLNetStructure->TIDLPCLayers[k].numInBufs; l++)
    {
      scale = scale_for_non_conv;
      if (dataId == pOrgTIDLNetStructure->TIDLPCLayers[k].inData[l].dataId)
      {
        if ((pOrgTIDLNetStructure->TIDLPCLayers[k].layerType == TIDL_ConvolutionLayer) ||
          (pOrgTIDLNetStructure->TIDLPCLayers[k].layerType == TIDL_Deconv2DLayer))
        {
          int32_t numInChannels = pOrgTIDLNetStructure->TIDLPCLayers[k].layerParams.convParams.numInChannels;
          int32_t numOutChannels = pOrgTIDLNetStructure->TIDLPCLayers[k].layerParams.convParams.numOutChannels;
          int32_t numGroups = pOrgTIDLNetStructure->TIDLPCLayers[k].layerParams.convParams.numGroups;
          scale = pOrgTIDLNetStructure->TIDLPCLayers[k].layerParams.convParams.weightScale;
          if ((pOrgTIDLNetStructure->TIDLPCLayers[k].layerType == TIDL_ConvolutionLayer) &&
              ((numGroups == numInChannels) && (numGroups == numOutChannels) && (numInChannels == numOutChannels)) &&
              (( gParams.calibrationOption & TIDL_CalibOptionPerChannelWeightQuantization) ==
                       TIDL_CalibOptionPerChannelWeightQuantization ))
          {
            /* For per channel quantization we reduce weight bits to avoid bias saturation and hence
            set scale to minimum to avoid reducing activation bits */
            scale = scale_for_non_conv;
          }
          if (pOrgTIDLNetStructure->TIDLPCLayers[k].layerParams.convParams.enableBias == 1)
          {
            scale /= pOrgTIDLNetStructure->TIDLPCLayers[k].layerParams.convParams.biasScale;
          }
          else
          {
            scale = scale_for_non_conv;
          }

        }
        else if (pOrgTIDLNetStructure->TIDLPCLayers[k].layerType == TIDL_InnerProductLayer)
        {
          scale = pOrgTIDLNetStructure->TIDLPCLayers[k].layerParams.innerProductParams.weightScale /
            pOrgTIDLNetStructure->TIDLPCLayers[k].layerParams.innerProductParams.biasScale;
        }
        else if (pOrgTIDLNetStructure->TIDLPCLayers[k].layerType == TIDL_BatchNormLayer)
        {
          scale = pOrgTIDLNetStructure->TIDLPCLayers[k].layerParams.batchNormParams.weightScale /
            pOrgTIDLNetStructure->TIDLPCLayers[k].layerParams.batchNormParams.biasScale;
        }
        else
        {
          scale = scale_for_non_conv;
        }
      }
      if (scale > maxScale)
      {
        maxScale = scale;
      }
    }
  }
  return (maxScale);
}

void TIDL_findRangePitch(float * data, int32_t dataSize, int32_t linePitch, float * minOut, float * maxOut, float scale)
{
  float min = FLT_MAX;
  float max = -FLT_MAX;
  int32_t i;
  for (i = 0; i < dataSize; i++)
  {
    min = ((data[i*linePitch] * scale) < min) ? (data[i*linePitch] * scale) : min;
    max = ((data[i*linePitch] * scale) > max) ? (data[i*linePitch] * scale) : max;
  }
  *minOut = (min < *minOut) ? min : *minOut;
  *maxOut = (max > *maxOut) ? max : *maxOut;
}

template <typename T>
void TIDL_findRange(T* data, int32_t dataSize, float * minOut, float * maxOut, float scale)
{
  float min = FLT_MAX;
  float max = -FLT_MAX;
  int32_t i;
  for (i = 0; i < dataSize; i++)
  {
    min = (((float)data[i] * scale) < min) ? ((float)data[i] * scale) : min;
    max = (((float)data[i] * scale) > max) ? ((float)data[i] * scale) : max;
  }
  *minOut = (min < *minOut) ? min : *minOut;
  *maxOut = (max > *maxOut) ? max : *maxOut;
#if ENABLE_HIST_BASED_RANGE
  int * histPtr = (int *)my_malloc(HIST_SIZE * sizeof(int));
  float orgMax;
  TIDL_computeHist(data, dataSize, histPtr, &orgMax);
  int curPer = 0;
  int maxPer = dataSize*0.995;
  for (i = 0; i < HIST_SIZE; i++)
  {
    curPer += histPtr[i];
    if (curPer >= maxPer)
      break;
  }
  my_free(histPtr);

  *maxOut = (orgMax*i) / (HIST_SIZE);
  *minOut = -1 * *maxOut;
#endif

}

int32_t TIDL_findRangeHist(float32_tidl * data,
                        int32_t dataSize,
                        int32_t numBins,
                        float32_tidl percentileRangeShrink,
                        float32_tidl * min,
                        float32_tidl * max)
{
  int32_t binIdx, minBinIdx, maxBinIdx;
  float32_tidl minValue = *min;
  float32_tidl maxValue = *max;
  float32_tidl val, val_norm;
  int32_t i1, i2;
  int32_t * histogramArray = (int32_t*)my_malloc(numBins * sizeof(int32_t));
  if(histogramArray != NULL)
  {
    memset(histogramArray, 0, (numBins * sizeof(int32_t)));
  }
  else
  {
    TIDL_GLOBAL_REPORT_ERROR("TIDL_findRangeHist - Not enough memory available for histogram");
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  if((maxValue - minValue) != 0) /*not all values in tensor constant */
  {
    for(int i1 = 0; i1 < dataSize; i1++)
    {
      val = data[i1];
      val_norm = (val - minValue)/(maxValue - minValue) * (numBins - 1);
      binIdx = (int32_t)(val_norm + 0.5);  /* Round to nearest integer */
      if ( binIdx > (numBins-1) )
      {
        binIdx = (numBins-1);
      }
      histogramArray[binIdx]++;
    }
    int32_t pct_freq = (int32_t)((percentileRangeShrink / 100.0) * dataSize);
    int32_t count = 0;
    if (*min < 0)  /* minimum to be shrinked only for signed */
    {
      for(i2 = 0; i2 < numBins; i2++)
      {
        count += histogramArray[i2];
        if(count >= pct_freq)
        {
          minBinIdx = i2;
          break;
        }
      }
    }
    else
    {
      minBinIdx = 0;
    }
    count = 0;
    for(i2 = numBins - 1; i2 >= 0; i2--)
    {
      count += histogramArray[i2];
      if(count >= pct_freq)
      {
        maxBinIdx = i2;
        break;
      }
    }
    *min = minValue + (float32_tidl)minBinIdx / (numBins-1) * (maxValue - minValue);
    *max = minValue + (float32_tidl)maxBinIdx / (numBins-1) * (maxValue - minValue);
    *max = (*max > maxValue) ? maxValue : *max; /*to ensure back calculated bin value not greater than original max */
    *min = (*min < minValue) ? minValue : *min;
  }
  else
  {
    *min = minValue;
    *max = maxValue;
  }
  if(histogramArray != NULL)
  {
    my_free(histogramArray);
  }
  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}


void TIDL_quickSort(float32_tidl * dataSort, int32_t left, int32_t right)
{
  if(left >= right)
  {
    return;
  }
  float32_tidl pivot = dataSort[right];
  int32_t cnt = left;
  float32_tidl temp;
  for (int i = left; i <= right; i++)
  {
    if(dataSort[i] <= pivot)
    {
      temp = dataSort[i];
      dataSort[i] = dataSort[cnt];
      dataSort[cnt] = temp;
      cnt++;
    }
  }
  TIDL_quickSort(dataSort, left, cnt - 2);
  TIDL_quickSort(dataSort, cnt, right);
}

/* Find Kth smallest value in the array.
   Based on quicksort, but there is no need to sort the whole array.
   After partitioning, only need to search in one half.
   Also, tail recursion (with help from compiler) can avoid stack overflow.
   Okay, converting tail recursion to while loop so that we do not need
       to rely on compiler optimization to avoid stack overflow issue.
*/
float32_tidl TIDL_findKthSmallest(float32_tidl * dataSort,
                                  int32_t dataSize, int32_t k)
{
  int32_t left  = 0;
  int32_t right = dataSize - 1;
  int32_t iters = dataSize;

  while (iters-- >= 0)                          /* while (1) */
  {
    if(left >= right)
    {
      return dataSort[left];
    }
    float32_tidl pivot = dataSort[right];
    int32_t cnt = left;
    float32_tidl temp;
    if (dataSort[left + k] < pivot)             /* introduce randomness */
    {
      dataSort[right] = dataSort[left + k];
      dataSort[left + k] = pivot;
      pivot = dataSort[right];
    }
    for (int i = left; i < right; i++)          /* partition */
    {
      if(dataSort[i] < pivot)
      {
        temp = dataSort[i];
        dataSort[i] = dataSort[cnt];
        dataSort[cnt] = temp;
        cnt++;
      }
    }
    dataSort[right] = dataSort[cnt];
    dataSort[cnt]   = pivot;

    int32_t new_left, new_right, new_k;
    if (k < cnt - 1 - left + 1)                 /* <  pivot */
    {
      new_left = left;
      new_right = cnt - 1;
      new_k = k;
    }
    else if (k == cnt - left)                   /* == pivot */
    {
      return dataSort[cnt];
    }
    else                                        /* >= pivot */
    {
      new_left = cnt + 1;
      new_right = right;
      new_k = k - (cnt + 1 - left);
    }

    /* Converting tail recursion into explicit loop, so that we don't need
       to rely on the compiler optimization to avoid stack overflow issue */
    left  = new_left;
    right = new_right;
    k     = new_k;
  }

  return dataSort[left];
}


float32_tidl TIDL_findMedian(float32_tidl * dataSort, int32_t dataSize)
{
#if 0
  float32_tidl temp, median;
  /* sort data - bubble sort too slow, hence quick sort implemented */
  TIDL_quickSort(dataSort, 0, dataSize - 1);
  /* find median value */
  if (dataSize % 2 == 0)
  {
    median = (dataSort[dataSize/2] + dataSort[(dataSize/2) + 1])/2.0;
  }
  else
  {
    median = dataSort[(dataSize+1)/2];
  }
  return median;
#else
  return TIDL_findKthSmallest(dataSort, dataSize, dataSize/2 + 1);
#endif
}


#define TIDL_WEIGHTS_CLAMP_VALUE (15)
#define TIDL_WEIGHTS_CLAMP_RATIO (16)
int32_t TIDL_findRangeUsingMedian(float32_tidl * data,
                              int32_t dataSize,
                              int32_t weightsElementSizeInBits,
                              float32_tidl * min,
                              float32_tidl * max)
{
  float32_tidl weightsClampValue = (pow(2,(weightsElementSizeInBits-1)))/8 - 1;
  float32_tidl weightsClampRatio = TIDL_WEIGHTS_CLAMP_RATIO;
  float32_tidl minValue = *min;
  float32_tidl maxValue = *max;
  float32_tidl absMaxRoundPow2, clampMargin, clampMax;
  float32_tidl absMax = (fabs(minValue) > fabs(maxValue)) ? fabs(minValue) : fabs(maxValue);

  float32_tidl * dataSort = (float32_tidl*)my_malloc(dataSize*sizeof(float32_tidl));
  if(dataSort == NULL)
  {
    TIDL_GLOBAL_REPORT_ERROR("TIDL_findRangeUsingMedian - Not enough memory available for data sorting to find median");
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  for(int i = 0; i < dataSize; i++)
  {
    dataSort[i] = fabs(data[i]);
  }
  float32_tidl weightMedian = TIDL_findMedian(dataSort, dataSize);

  if((absMax > weightsClampValue) &&  (absMax > weightMedian * weightsClampRatio))
  {
    absMax = (absMax < (weightMedian * weightsClampRatio)) ? absMax : (weightMedian * weightsClampRatio);
    absMaxRoundPow2 = pow(2, ceil(log(absMax)/log(2.0)));
    clampMargin = 1.0;
    clampMax = absMaxRoundPow2 - clampMargin;
    *max = (maxValue > clampMax) ? clampMax : maxValue;
    *min = (minValue < -1.0*clampMax) ? -1.0*clampMax : minValue;
  }
  else
  {
    *min = minValue;
    *max = maxValue;
  }
  if(dataSort != NULL)
  {
    my_free(dataSort);
  }
  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

#define HIST_SIZE (1000)
template <typename T>
void TIDL_computeHist(T *origParams, int paramNum, int *histPtr, float *orgMax)
{
  int i;
  float orgMaxFloat = 0;
  for (i = 0; i < HIST_SIZE; i++)
  {
    histPtr[i] = 0;
  }
  for (i = 0; i < paramNum; i++)
  {
    orgMaxFloat = orgMaxFloat <= fabs((float)origParams[i]) ? fabs((float)origParams[i]) : orgMaxFloat;
  }
  if(orgMaxFloat == 0)
    orgMaxFloat = 1;
  for (i = 0; i < paramNum; i++)
  {
    if (orgMaxFloat != 0.0)
    {
      int idx = (int)((fabs((float)origParams[i])* (HIST_SIZE - 1)) / orgMaxFloat);
      if (idx >= 0 && idx < HIST_SIZE)
      {
        histPtr[idx] += 1;
      }
    }
  }
  *orgMax = orgMaxFloat;
}

/* compares quantized values and the original values (for parameters) */
template <class quantParamType>
int TIDL_CompareParams(quantParamType *quantizedParams, float *origParams, int paramNum, float scale) {
  /* absolute value of difference is considered */
  float meanDifference = 0;
  float maxDifference = 0;

  float meanRelDifference = 0;
  float maxRelDifference = 0;
  float orgMaxFloat   = 0;
  float quantMaxFloat = 0;

  float meanOrigFloat = 0;

  int relValidNum = 0;
  int maxRelDiffIndex = 0;
  for (int i = 0; i < paramNum; i++)
  {
    float quantParamFloat = quantizedParams[i] / scale;
    float origFloat = origParams[i];
    float difference = quantParamFloat>origFloat ? (quantParamFloat - origFloat) : (origFloat - quantParamFloat); /* abs value */
    float absOrigFloat = origFloat>0 ? origFloat : -origFloat;
    float absQuantFloat = quantParamFloat>0 ? quantParamFloat : -quantParamFloat;
    int  absQuantizedParams = quantizedParams[i] > 0 ? quantizedParams[i] : -quantizedParams[i];
    meanOrigFloat += absOrigFloat;

    meanDifference += difference;

    if (maxDifference < difference)
    {
      maxDifference = difference;
    }
    if (orgMaxFloat < absOrigFloat)
    {
      orgMaxFloat = absOrigFloat;
    }
    if (quantMaxFloat < absQuantFloat)
    {
      quantMaxFloat = absQuantFloat;
    }
    float relDifference = 0;

    if (absQuantizedParams > 2)
    {
      relDifference = (difference / absOrigFloat) * 100;
      relValidNum++;
    }

    if (maxRelDifference < relDifference)
    {
      maxRelDifference = relDifference;
      maxRelDiffIndex = i;
    }
    meanRelDifference += relDifference;

  }
  meanDifference /= paramNum;

  if (relValidNum != 0)
    meanRelDifference /= relValidNum;
  else
    meanRelDifference = -1;

  meanOrigFloat /= paramNum;
  if (paramDebugFile != NULL)
  {
    int * histPtr = (int *)my_malloc(HIST_SIZE * sizeof(int));
    float temp;
    TIDL_computeHist(origParams, paramNum, histPtr, &temp);
    fprintf(paramDebugFile, "%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f, , , ,", debugLayeId, meanDifference, maxDifference, meanOrigFloat, meanRelDifference, orgMaxFloat, quantMaxFloat, origParams[maxRelDiffIndex], quantizedParams[maxRelDiffIndex] / scale, maxRelDifference, scale);
    for (int i = 0; i < HIST_SIZE; i++)
    {
      fprintf(paramDebugFile, "%d,", histPtr[i]);
    }
    fprintf(paramDebugFile, "\n");
    my_free(histPtr);
  }
  return 0;

}
static int32_t TIDL_isDepthwiseConvLayer(sTIDL_OrgNetwork_t * pOrgTIDLNetStructure, int32_t i)
{
  int32_t isDepthwiseConvLayer = 0;

    if(pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_ConvolutionLayer)
    {
      int32_t numInChannels = pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.convParams.numInChannels;
      int32_t numOutChannels = pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.convParams.numOutChannels;
      int32_t numGroups = pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.convParams.numGroups;
      if((numInChannels == numGroups) && (numOutChannels == numGroups) && (numInChannels == numOutChannels))
      {
        return 1;
      }
  }
  return 0;
}

static int32_t TIDL_depthwiseConvExists(sTIDL_OrgNetwork_t * pOrgTIDLNetStructure, int32_t layerIndex)
{
  int32_t isDepthwiseConvLayer = 0;
  for(int i = 0; i < layerIndex; i++)
  {
    if(TIDL_isDepthwiseConvLayer(pOrgTIDLNetStructure , i))
    {
      isDepthwiseConvLayer = 1;
      break;
    }
  }
  return isDepthwiseConvLayer;
}


#define TIDL_MINIMUM_QUANTIZATION_RANGE  (pow(10.0,-15.0))


float TIDL_GetMaxQuantScale(float min,
                            float max,
                            int32_t weightBits
                            )

{
  float absRange = (fabs(max) > fabs(min)) ? fabs(max) : fabs(min);

  /* If absolute range is below minimum and treat it as zero */
  if ( absRange < TIDL_MINIMUM_QUANTIZATION_RANGE)
  {
    absRange = 0;
  }
  else if (gParams.quantizationStyle == TIDL_QuantStyleP2Dynamic)
  {
    absRange = (float)ceil(log((double)absRange) / log((double)2));
    absRange = pow(2.0, (double)absRange);
  }
  else
  {

  }
  float maxWeightsScalePossible = -1;
  if (absRange != 0)
  {
    maxWeightsScalePossible = ((1.0*(1 << (weightBits - 1))) / absRange);
  }
  return (maxWeightsScalePossible);
}


template <class Tout>
float TIDL_QuantizeSignedMax(Tout * params,
                            float * data,
                            int32_t dataSize,
                            float min,
                            float max,
                            int32_t weightBits,
                            float maxScale,
                            int32_t isOutMaxSat,
                            float inTensorScale,
                            float outTensorScale
                            )

{
  int32_t i;
  float quantPrec = FLT_MAX;
  float pData;
  int32_t param; 


  /* Finding maximum scale that can be used for current parameters
  based on the quantization scheme  requested by user */
  float maxWeightsScalePossible = TIDL_GetMaxQuantScale(min, max, weightBits);
  quantPrec = maxWeightsScalePossible;

  /* Clip the scale to avoid Bias staturation */
  if ( quantPrec > maxScale ) //VT: Update!
  {
    quantPrec = maxScale;
  }

  /* If the current layers output tensor scale needs to satisfy
      any requirements on output tensor scale clipping (Example relu6).
      Adjust the weight scale as per IP requirement (Power of 2 divider)
  */

  if(isOutMaxSat)
  {
    if(quantPrec == -1)
    {
      quantPrec = 1;
    }
    float accScale = inTensorScale*quantPrec;
    float outDiv = (accScale / outTensorScale);
    float pow2Div = (float)ceil(log((double)outDiv) / log((double)2));
    pow2Div = pow(2.0, (double)pow2Div);
    if(pow2Div > outDiv)
    {
      pow2Div = pow2Div / 2;
    }
    accScale = outTensorScale * pow2Div;
    quantPrec = accScale / inTensorScale;
  }

  /* Convert the floating point parameters to fixed point based on the selected scale */
  for (i = 0; i < dataSize; i++)
  {
    if(quantPrec == -1)
    {
      params[i] = 0;
    }
    else
    {
      pData = data[i];
      pData = pData > max ? max : pData;
      pData = pData < min ? min : pData;
      if (pData > 0)
      {
        param = (pData *  quantPrec + QUAN_STYLE2_ROUND);
      }
      else
      {
        param = (pData *  quantPrec - QUAN_STYLE2_ROUND);
      }
      param = param > ((1 << (weightBits - 1)) - 1) ? ((1 << (weightBits - 1)) - 1) : param;
      params[i] = param < (-1 * (1 << (weightBits - 1))) ? (-1 * (1 << (weightBits - 1))) : param;
      /* If weigtBits is 1 then it indicates that there are no bits avaialable to quantize weights
      hence explicitly set weights as 0 */
      if ( weightBits == 1)
      {
        params[i] = 0;
      }
    }
  }
  TIDL_CompareParams(params, data, dataSize, quantPrec);

  if((gParams.debugTraceLevel > 0) && (weightBits <=8) &&
     ((((maxWeightsScalePossible / quantPrec) >= 1.5) && (isOutMaxSat == 1)) ||
      (((maxWeightsScalePossible / quantPrec) >  1.0) && (isOutMaxSat == 0))))
  {
      TIDL_GLOBAL_REPORT_WARNING("Weight Scale Clipped - %10.2f, %10.2f, %10.2f, %4d, %2d, %4d, %s \n", (maxWeightsScalePossible / quantPrec),
                                    maxWeightsScalePossible, quantPrec, currElemenType, isOutMaxSat, debugLayeId, currLayerName);
  }

  return (quantPrec);
}

template float TIDL_QuantizeSignedMax<signed char>(signed char * params, float * data, int32_t dataSize, float min, float max, int32_t weightsElementSizeInBits, float maxScale, int32_t isOutMaxSat, float inTensorScale, float outTensorScale);
template float TIDL_QuantizeSignedMax<signed short>(signed short * params, float * data, int32_t dataSize, float min, float max, int32_t weightsElementSizeInBits, float maxScale, int32_t isOutMaxSat, float inTensorScale, float outTensorScale);



float TIDL_findMaxQuantizationScale(float min, float max, int32_t elementSizeInBits, int32_t sign)
{
  float absRange = (fabs(max) > fabs(min)) ? fabs(max) : fabs(min);

  if (gParams.quantizationStyle == TIDL_QuantStyleP2Dynamic)
  {
    absRange = (float)ceil(log((double)absRange) / log((double)2));
    absRange = pow(2.0, (double)absRange);
  }

  float quantPrec;
  if (absRange != 0)
  {
    /* One sign bit it Bias A is fixed and one side
       bit in Bias B depends on the input tensor element type */
    quantPrec = ((1.0*(1ll << (elementSizeInBits - sign - 1))) / absRange);
  }
  else
  {
    quantPrec = FLT_MAX;
  }

  return quantPrec;
}

#define TIDL_CONCAT_INTERNAL_WEIGHT_Q_U8    ((uint32_t)7)
#define TIDL_CONCAT_INTERNAL_WEIGHT_Q_U16   ((uint32_t)15)
#define TIDL_INTERNAL_POOLING_WEIGHT_Q_U8   ((uint32_t)8)
#define TIDL_INTERNAL_POOLING_WEIGHT_Q_U16   ((uint32_t)12)
#define TIDL_ELTWISE_INTERNAL_WEIGHT_Q_U8   ((uint32_t)6)
#define TIDL_ELTWISE_INTERNAL_WEIGHT_Q_U16   ((uint32_t)14)
#define TIDL_INTERNAL_INDATA_Q              ((uint32_t)7)

/**
 * @brief Function which finds the first producer layer
 *
 * @param pLayer : Pointer to the current layer
 * @return int32_t : returns updated required or not
 */

int32_t TIDL_isProducerLayer(sTIDL_LayerPC_t * pLayer)
{
  int32_t isComputeLayer = 0;
  if(pLayer->layerType == TIDL_ConvolutionLayer ||
     pLayer->layerType == TIDL_Deconv2DLayer ||
     pLayer->layerType == TIDL_EltWiseLayer  ||
     pLayer->layerType == TIDL_InnerProductLayer ||
     pLayer->layerType == TIDL_DeformableConvLayer
     )
  {
    isComputeLayer = 1;
  }
  else if(pLayer->layerType == TIDL_DataConvertLayer && pLayer->layerParams.dataConvertParams.type == TIDL_DC_TYPE_INPUT)
  {
    isComputeLayer = 1;
  }
  return isComputeLayer;  
}

/**
 * @brief Function which finds the first grid producer layer 8-bit -> 16-bit
 *
 * @param pLayer : Pointer to the current layer
 * @return int32_t : returns updated required or not
 */

int32_t TIDL_isGridProducerLayer(sTIDL_LayerPC_t * pLayer, tidl_import_config * configParams)
{
  int32_t isComputeLayer = 0;
  if(pLayer->layerType == TIDL_ConvolutionLayer ||
     pLayer->layerType == TIDL_Deconv2DLayer ||
     pLayer->layerType == TIDL_EltWiseLayer  ||
     pLayer->layerType == TIDL_InnerProductLayer ||
     pLayer->layerType == TIDL_DeformableConvLayer
     )
  {
    if(TIDL_doesLayerSupportMixedPrecision(pLayer))
    {
      isComputeLayer = 1;
    }    
  }
  else if(pLayer->layerType == TIDL_DataConvertLayer)
  {
    if(pLayer->layerParams.dataConvertParams.type == TIDL_DC_TYPE_INPUT)
    {
      isComputeLayer = 1;
    }
  }
  return isComputeLayer;  
}


/**
 * @brief Check if layer is an initializer input 
 *
 * @param pLayer : Pointer to the current layer
 * @return int32_t : returns updated required or not
 */

int32_t TIDL_isGridProducerInputLayer(sTIDL_LayerPC_t * pLayer, tidl_import_config * configParams)
{
  int32_t isGridInputLayer = 0;
  if(pLayer->layerType == TIDL_ConstDataLayer || pLayer->layerType == TIDL_DataLayer)
  {
    isGridInputLayer = 1;
  }
  return isGridInputLayer;
}

/**
 * @brief Function which finds the first producer layer whose range can be constrained 
 *
 * @param pLayer : Pointer to the current layer
 * @return int32_t : returns updated required or not
 */

int32_t TIDL_isComputeLayer(sTIDL_LayerPC_t * pLayer, tidl_import_config * configParams)
{
  int32_t isComputeLayer = 0;
  if(pLayer->layerType == TIDL_ConvolutionLayer ||
     pLayer->layerType == TIDL_Deconv2DLayer ||
     pLayer->layerType == TIDL_EltWiseLayer  ||
     pLayer->layerType == TIDL_InnerProductLayer ||
     pLayer->layerType == TIDL_DeformableConvLayer ||
     (pLayer->layerType == TIDL_ReduceLayer && pLayer->layerParams.reduceParams.ops == TIDL_inReduceOpMean))
  {
    isComputeLayer = 1;
  }
  else if(pLayer->layerType == TIDL_DataConvertLayer)
  {
    if(pLayer->layerParams.dataConvertParams.type == TIDL_DC_TYPE_INPUT)
    {
      isComputeLayer = 1;
    }
  }
  return isComputeLayer;  
}

/**
 * @brief Function to check if TensorScale update is required or not
 *
 * @param pLayer : Pointer to the current layer
 * @return int32_t : returns updated required or not
 */
int32_t TIDL_canUpdateTensorScale(sTIDL_LayerPC_t * pLayer, tidl_import_config * configParams)
{
  int32_t canUpdate = 1;
  if (pLayer->layerType == TIDL_PoolingLayer )
  {
    if ( pLayer->layerParams.poolParams.poolingType == TIDL_MaxPooling )
    {
      canUpdate = 0;
    }
  }
  else if (pLayer->layerType == TIDL_DataConvertLayer)
  {
    const sTIDL_dataConvertParams_t *params = &pLayer->layerParams.dataConvertParams;
    const sTIDL_DataConvertPCParams_t *pcParams = &pLayer->layerPCParams.dataConvertParams;
    canUpdate = 1;

    if ( params->type == TIDL_DC_TYPE_OUTPUT )
    {
      int32_t configParamsOutIndex;
      configParamsOutIndex = TIDL_getConfigParamOutIndexFromLayerName((const char *)pLayer->outDataNames[0]);
      if ( configParams->outTensorScale[configParamsOutIndex] != 0.0)
      {
        /* This indicates that for data convert layer on output side, output tensor scale is given by the
        user and we  have to adhere to it hence disable updating the tensor scale*/
        canUpdate = 0;
      }
    }

    if (params->type == TIDL_DC_TYPE_INTERMEDIATE &&
        pcParams->canUpdateTensorScale == 0)
    {
      /*Intermediate Dataconvert layers cannot change scales*/
      canUpdate = 0;
    }
  }
  else if ((pLayer->layerType == TIDL_FlattenLayer)     ||
            (pLayer->layerType == TIDL_CropLayer)       ||
            (pLayer->layerType == TIDL_SplitLayer)      ||
            (pLayer->layerType == TIDL_SliceLayer)      ||
            (pLayer->layerType == TIDL_PadLayer)        ||
            (pLayer->layerType == TIDL_GridSampleLayer) ||
            (pLayer->layerType == TIDL_TransposeLayer) ||
            (pLayer->layerType == TIDL_ReshapeLayer) ||
            (pLayer->layerType == TIDL_OdOutputReformatLayer) ||
            (pLayer->layerType == TIDL_TransposeLayer))
  {
    canUpdate = 0;
  }
  else
  {
    canUpdate = 1;
  }
  return canUpdate;
}
int32_t  isOutputTensorMaxSatAvailable(sTIDL_LayerPC_t *TIDLPCLayers, float * outScale)
{
  float clipMax;
  if (((TIDLPCLayers->actParams.actType == TIDL_Clip) && (TIDLPCLayers->actParams.clipMax > 0) &&
  ((TIDLPCLayers->actParams.clipMin == 0) || ( (-1*TIDLPCLayers->actParams.clipMin) == TIDLPCLayers->actParams.clipMax))) ||
      (TIDLPCLayers->actParams.actType == TIDL_RelU6))
  {
    clipMax = TIDLPCLayers->actParams.clipMax;
    if(TIDLPCLayers->actParams.actType == TIDL_RelU6)
    {
      clipMax = 6.0;
    }
    int32_t elemBits = tidl_getElementSizeInBits(TIDLPCLayers->outData[0].elementType);
    if(TIDLPCLayers->actParams.clipMin  < 0)
    {
      elemBits -= 1;
    }
    *outScale = (1 << elemBits) / clipMax;
    return 1;
  }
  return 0;
}
int32_t TIDLIT_getProcessingElementSizeInBytes(const sTIDL_LayerPC_t  * tidlLayer)
{
  int32_t procElemSizeInBytes;
  if ( tidlLayer->weightsElementSizeInBits <= 8 )
  {
    procElemSizeInBytes = 1;
  }
  else if ( tidlLayer->weightsElementSizeInBits <= 16 )
  {
    procElemSizeInBytes = 2;
  }
  else
  {
    procElemSizeInBytes = 4;
  }

  return procElemSizeInBytes;
}

static int32_t TIDL_getMinConsumerWeightElemBits(const sTIDL_OrgNetwork_t * pOrgTIDLNetStructure, int32_t dataId)
{
  int32_t layerIdx, inBufIdx;
  int32_t weightElementBits = 128;

  for (layerIdx = 0; layerIdx < pOrgTIDLNetStructure->numLayers; layerIdx++)
  {
    for (inBufIdx = 0; inBufIdx < pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].numInBufs; inBufIdx++)
    {
      if (dataId == pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].inData[inBufIdx].dataId)
      {
        if (  pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].weightsElementSizeInBits < weightElementBits  )
        {
          weightElementBits = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].weightsElementSizeInBits;
        }
      }
    }
  }

  return weightElementBits;
}


void TIDL_rangeToScaleP2(float32_tidl *bufferScale, float32_tidl min, float32_tidl max, int32_t maxPrecisionBits, int32_t quantizedElemType)
{
  float32_tidl rangeVal = max  - min;
  float32_tidl absRange = (fabs(max) > fabs(min)) ? fabs(max) : fabs(min);
  float32_tidl maxP2Scale = pow(2.0, maxPrecisionBits);
  /*Symmetric only:*/
  if(quantizedElemType == TIDL_SignedChar)
  {
    /*Map range to -128 to +127*/
    /*Find P2 value closest to representing this range:*/
    float32_tidl p2Val = pow(2.0, ceil(log(absRange)/log(2.0)));
    *bufferScale = round((128.0 / p2Val));
    *bufferScale = *bufferScale > maxP2Scale ? maxP2Scale : *bufferScale;
  }
  else if(quantizedElemType == TIDL_SignedShort)
  {
    /*Find P2 value closest to representing this range:*/
    float32_tidl p2Val = pow(2.0, ceil(log(absRange)/log(2.0)));
    *bufferScale = round((32768.0 / p2Val)); 
    *bufferScale = *bufferScale > maxP2Scale ? maxP2Scale : *bufferScale;
  }
  else if(quantizedElemType == TIDL_UnsignedChar)
  {
    /*Find P2 value closest to representing this range:*/
    float32_tidl p2Val = pow(2.0, ceil(log(rangeVal)/log(2.0)));
    *bufferScale = round((256.0 / p2Val));
    *bufferScale = *bufferScale > maxP2Scale ? maxP2Scale : *bufferScale;
  }
  else if((quantizedElemType == TIDL_UnsignedShort))
  {
    /*Find P2 value closest to representing this range:*/
    float32_tidl p2Val = pow(2.0, ceil(log(rangeVal)/log(2.0)));
    *bufferScale = round((65536.0 / p2Val)); 
    *bufferScale = *bufferScale > maxP2Scale ? maxP2Scale : *bufferScale;
  }
  
}

template <class TzeroPoint>
bool TIDL_asymRangeToScale(float32_tidl *bufferScale, TzeroPoint *zeroPoint, float32_tidl min, float32_tidl max, int32_t tensorType, int32_t quantizedElemType, int32_t quantizeConstraint = TIDL_QuantizeUnconstrained)
{
  /*The goal of this function is to make maximum use of a given dataType
    Representation: Xfloat = (Xfixed - zp)/scale
    max =  (127 - zp)/scale*/
  float32_tidl rangeBuffer, floatZP;
  bool zeroOutBuf = false;
  rangeBuffer = max - min;
  if(abs(rangeBuffer) < TIDL_MINIMUM_QUANTIZATION_RANGE)
  {
    rangeBuffer = 0;
    zeroOutBuf = true;
  }
 
  if((max < 0.0f || min > 0.0f) && tensorType == TIDL_ASYMMETRIC_TENSOR){
    rangeBuffer = std::max(std::abs(min), std::abs(max));
  }
  else if (max < 0.0f || min > 0.0f)
  {
    tensorType = TIDL_SYMMETRIC_TENSOR;
  }

  /*Quantizing for P2 cases:*/
  if(quantizeConstraint == TIDL_QuantizeConstainedP2)
  {
    TIDL_rangeToScaleP2(bufferScale, min, max, (quantizedElemType == TIDL_SignedChar || quantizedElemType == TIDL_UnsignedChar) ? 7 : 15, quantizedElemType);
    *zeroPoint = 0;
    return zeroOutBuf;
  }

  if(zeroOutBuf == false)
  {
    float32_tidl quantizedTypeMax = 0.0, quantizedTypeMin = 0.0;
    float32_tidl zpSatMax = 0.0, zpSatMin = 0.0;
    float32_tidl zpOffset = 0.0;

    if(TIDL_ASYMMETRIC_TENSOR == tensorType)
    {
      if(quantizedElemType == TIDL_SignedChar)
      {
        *bufferScale = (255.0/rangeBuffer);
        floatZP = round((127.0 - (*(bufferScale) * max)));
        /*Saturate zero point to respective datatype container*/
        floatZP = floatZP > std::numeric_limits<int8_t>::max() ? std::numeric_limits<int8_t>::max() : floatZP;
        floatZP = floatZP < std::numeric_limits<int8_t>::min() ? std::numeric_limits<int8_t>::min() : floatZP;
        *zeroPoint = (int8_t) floatZP; 

        quantizedTypeMax = 127, quantizedTypeMin = -128;
        zpSatMax = std::numeric_limits<int8_t>::max(), zpSatMin = std::numeric_limits<int8_t>::min();
        zpOffset = 127.0;
      }
      else if(quantizedElemType == TIDL_SignedShort)
      {
        *bufferScale = (65535.0/rangeBuffer);
        floatZP = round((32767.0 - (*(bufferScale) * max)));
        /*Saturate zero point to respective datatype container*/
        floatZP = floatZP > std::numeric_limits<int16_t>::max() ? std::numeric_limits<int16_t>::max() : floatZP;
        floatZP = floatZP < std::numeric_limits<int16_t>::min() ? std::numeric_limits<int16_t>::min() : floatZP;
        *zeroPoint = (int16_t) floatZP;

        quantizedTypeMax = 32767.0, quantizedTypeMin = -32768.0;
        zpSatMax = std::numeric_limits<int16_t>::max(), zpSatMin = std::numeric_limits<int16_t>::min();
        zpOffset = 32767.0;
      }
      else if(quantizedElemType == TIDL_UnsignedChar)
      {
        *bufferScale = (255.0/rangeBuffer);
        floatZP = round((255.0 - (*(bufferScale) * max))); 
        /*Saturate zero point to respective datatype container*/
        floatZP = floatZP > std::numeric_limits<uint8_t>::max() ? std::numeric_limits<uint8_t>::max() : floatZP;
        floatZP = floatZP < std::numeric_limits<uint8_t>::min() ? std::numeric_limits<uint8_t>::min() : floatZP;
        *zeroPoint = (uint8_t) floatZP;

        quantizedTypeMax = 255.0, quantizedTypeMin = 0.0;
        zpSatMax = std::numeric_limits<uint8_t>::max(), zpSatMin = std::numeric_limits<uint8_t>::min();
        zpOffset = 255.0;
      }
      else if(quantizedElemType == TIDL_UnsignedShort)
      {
        *bufferScale = (65535.0/rangeBuffer);
        floatZP = round((65535.0 - (*(bufferScale) * max)));
        /*Saturate zero point to respective datatype container*/
        floatZP = floatZP > std::numeric_limits<uint16_t>::max() ? std::numeric_limits<uint16_t>::max() : floatZP;
        floatZP = floatZP < std::numeric_limits<uint16_t>::min() ? std::numeric_limits<uint16_t>::min() : floatZP;
        *zeroPoint = (uint16_t) floatZP;

        quantizedTypeMax = 65535.0, quantizedTypeMin = 0.0;
        zpSatMax = std::numeric_limits<uint16_t>::max(), zpSatMin = std::numeric_limits<uint16_t>::min();
        zpOffset = 65535.0;
      }
      else if (quantizedElemType == TIDL_SinglePrecFloat)
      {
        *bufferScale = 1.0;
        *zeroPoint = 0;
      }
    }
    else
    {
      float absRange = (fabs(max) > fabs(min)) ? fabs(max) : fabs(min);
      if(quantizedElemType == TIDL_SignedChar)
      {
        *bufferScale = 127.0/absRange;
        *zeroPoint = 0;

        quantizedTypeMax = 127, quantizedTypeMin = -128;
      }
      else if(quantizedElemType == TIDL_SignedShort)
      {
        *bufferScale = SYMMETRIC_16BIT_SIGNED_MAX/absRange;
        *zeroPoint = 0;

        quantizedTypeMax = 32767.0, quantizedTypeMin = -32768.0;
      }
      else if(quantizedElemType == TIDL_UnsignedChar)
      {
        *bufferScale = 255.0/absRange;
        *zeroPoint = 0;

        quantizedTypeMax = 255.0, quantizedTypeMin = 0.0;
      }
      else if(quantizedElemType == TIDL_UnsignedShort)
      {
        *bufferScale = SYMMETRIC_16BIT_UNSIGNED_MAX/absRange;
        *zeroPoint = 0;

        quantizedTypeMax = 65535.0, quantizedTypeMin = 0.0;
      }
    }

    if(*bufferScale < 1.0 && max <= quantizedTypeMax && min >= quantizedTypeMin){
      *bufferScale = 1.0;
      if(TIDL_ASYMMETRIC_TENSOR == tensorType){
        floatZP = round((zpOffset - (*(bufferScale) * max)));
        /*Saturate zero point to respective datatype container*/
        floatZP = floatZP > zpSatMax ? zpSatMax : floatZP;
        floatZP = floatZP < zpSatMin ? zpSatMin : floatZP;
         /* Cast to appropriate type based on quantizedElemType */
        if(quantizedElemType == TIDL_SignedChar){
          *zeroPoint = (int8_t) floatZP;
        }
        else if(quantizedElemType == TIDL_SignedShort){
          *zeroPoint = (int16_t) floatZP;
        }
        else if(quantizedElemType == TIDL_UnsignedChar){
          *zeroPoint = (uint8_t) floatZP;
        }
        else if(quantizedElemType == TIDL_UnsignedShort){  
          *zeroPoint = (uint16_t) floatZP;
        }
      }
    }
  }
  else if(max != 0)
  {
    /*In this situation all values are very close to each other, and we want to represent the values with the maximum fixed point value:*/
    max = abs(max);
    if(quantizedElemType == TIDL_SignedChar)
    {
      *bufferScale = 127.0/std::abs(max);
      *zeroPoint = 0;
    }
    else if(quantizedElemType == TIDL_SignedShort)
    {
      *bufferScale = 32767.0/std::abs(max);
      *zeroPoint = 0;
    }
    else if(quantizedElemType == TIDL_UnsignedChar)
    {
      *bufferScale = 255.0/std::abs(max);
      *zeroPoint = 0;
    }
    else if(quantizedElemType == TIDL_UnsignedShort)
    {
      *bufferScale = 65535.0/std::abs(max);
      *zeroPoint = 0;
    }
  }
  else
  {
    if(quantizedElemType == TIDL_SignedChar)
    {
      *bufferScale = 127.0;
      *zeroPoint = 0;
    }
    else if(quantizedElemType == TIDL_SignedShort)
    {
      *bufferScale = 32767.0;
      *zeroPoint = 0;
    }
    else if(quantizedElemType == TIDL_UnsignedChar)
    {
      *bufferScale = 255.0;
      *zeroPoint = 0;
    }
    else if(quantizedElemType == TIDL_UnsignedShort)
    {
      *bufferScale = 65535.0;
      *zeroPoint = 0;
    }
    else 
    {
      *bufferScale = 1.0;
      *zeroPoint = 0;
    }

    // expanding the scale to avoid interferring with other inputs in similar scales
    *bufferScale = *bufferScale * 1000;
  }
  return zeroOutBuf;
}

float32_tidl TIDL_getMMAv2_ScaleShiftAndError(float scaleRatio, uint8_t *scale, uint8_t *shift, int32_t weightBits, float32_tidl maxScaleMMAv2, bool skipCheck=false)
{
  int32_t shiftBits;
  float32_tidl curError = 0;
  float32_tidl approxScale = 0;
  float32_tidl minError = FLT_MAX;
  int32_t bestShiftBits;
  int32_t bestFixedScale;
  int32_t maxShiftBits = 40;
  if(weightBits > 8)
  {
    maxShiftBits = 64U;
  }
  int32_t scaleIter, skipCount = 1;;
  for(scaleIter = 1U; scaleIter <= 255U; scaleIter++)
  {
    /*Loop over all possible mmaScale values*/
    /* scaleRatio ~= mmaScale/2^mmaShift*/
    /* mmaShift = ln(mmaScale/scaleRatio)*/
    shiftBits = (int32_t)round(log(((double)scaleIter)/scaleRatio)/log((double)2));
    if(shiftBits > maxShiftBits)
    {
      shiftBits = maxShiftBits;
    }
    if(shiftBits < 0)
    {
      shiftBits = 0U;
    }
    approxScale = (float32_tidl)(((float32_tidl)scaleIter)/(pow(2,shiftBits)));
    if(scaleRatio > approxScale && skipCheck==true)
    {
      /*Error would expand weight scales, so skip these values*/
      continue;
      skipCount++;
    }
    curError = abs(scaleRatio - approxScale);
    if(curError < minError)
    {
      minError = curError;
      bestShiftBits = shiftBits;
      bestFixedScale = scaleIter;
    }
    //absRange = (float)ceil(log((double)absRange) / log((double)2));
    //absRange = pow(2.0, (double)absRange);
  }
  if(skipCount == 255)
  {
    /*All values skipped, so take the minimum error value*/
    bestShiftBits = maxShiftBits;
    bestFixedScale = 255U;
    minError = abs(scaleRatio - ((float32_tidl)bestFixedScale/(pow(2,bestShiftBits))));
    /*Emit a warning saying could not find optimal weight scale*/
    TIDL_GLOBAL_REPORT_WARNING("Could not find optimal MMAv2 weight scale approximation for scale ratio %f, using approx scale %f\n", scaleRatio, ((float32_tidl)bestFixedScale/(pow(2,bestShiftBits))));
  }
  *shift = bestShiftBits;
  *scale = bestFixedScale;
  return minError;
}

int32_t TIDL_UpdateScaleFactors(sTIDL_OrgNetwork_t * net,
                                     int32_t i,
                                     int32_t updateStats,
                                     int64_t accMin,
                                     int64_t accMax,
                                     tidl_import_config * configParams)
{
  int32_t j;
  float32_tidl accScale = 1.0f;
  float32_tidl minScale;
  float32_tidl outMin = 1.0f;
  float32_tidl outMax = 1.0f;
  float32_tidl max = 1.0f;
  int32_t elementSizeBits;
  int32_t elementSizeBytes;
  double maxP2;
  float32_tidl curMin;
  float32_tidl curMax;
  float32_tidl outDiv;
  int32_t internal_pooling_weight = 0;
  int32_t procElemSize;
  int32_t canUpdateTensorScale = 0;
  int32_t minConsumerWtElemBits;
  
  elementSizeBytes = tidl_getElementSizeInBits(net->TIDLPCLayers[i].outData[0].elementType)/8;
  procElemSize     = TIDLIT_getProcessingElementSizeInBytes(&net->TIDLPCLayers[i]);

  /* Find the minimum consumer weight element bits and based on decide how much bias can expand. The minimum
  weight element bits will ensure that constraint to avoid bias saturation is satisfied for all the consumer layers */
  minConsumerWtElemBits = TIDL_getMinConsumerWeightElemBits(net, net->TIDLPCLayers[i].outData[0].dataId );

#if USE_16BIT_BIAS_FOR_8BIT_MODE
  float32_tidl biasExpanScale = 256.0f;
#else
  float32_tidl biasExpanScale = 256.0f*128.0f;
#endif
  if (minConsumerWtElemBits > 8)
  {
    biasExpanScale = 128.0f*256.0f;
  }
  if ((net->TIDLPCLayers[i].outData[0].elementType == TIDL_SignedChar) ||
      (net->TIDLPCLayers[i].outData[0].elementType == TIDL_SignedShort))
  {
    biasExpanScale /= 2.0f;
  }

  /* For float we don't have to call update anything for stats collection */
  if ( elementSizeBytes == 4 )
  {
    net->TIDLPCLayers[i].outData[0].roundBits = 0;
    net->TIDLPCLayers[i].outData[0].tensorScale = 1.0f;
    return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
  }
  net->TIDLPCLayers[i].outData[0].tensorZeroPoint = 0;
  /* Updating the output type of the layer based on the input elementTypes */
  if(net->TIDLPCLayers[i].layerType == TIDL_EltWiseLayer || 
    net->TIDLPCLayers[i].layerType == TIDL_ConcatLayer || 
    net->TIDLPCLayers[i].layerType == TIDL_PoolingLayer
  ){
    int32_t numInBufs = net->TIDLPCLayers[i].numInBufs;
    int32_t inElementTypeSign = 0;
    for(int32_t inpIdx = 0; inpIdx < numInBufs; inpIdx++)
    {
      const sTIDL_DataParams_t * indata = TIDL_getOutData(net, net->TIDLPCLayers[i].inData[inpIdx].dataId);
      if(indata == NULL)
      {
        TIDL_GLOBAL_REPORT_ERROR("Cannot find Producer");
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }
      inElementTypeSign |= TIDL_getDatElementSign(indata->elementType);
    }
    if(TIDL_getDatElementSign(net->TIDLPCLayers[i].outData[0].elementType) != inElementTypeSign && net->TIDLPCLayers[i].actParams.actType == TIDL_NoAct)
    {
      int32_t outElementType = net->TIDLPCLayers[i].outData[0].elementType;
      if(inElementTypeSign == 1)
      {
        outElementType = tidl_getElementSizeInBits(outElementType) == 8 ? TIDL_SignedChar : TIDL_SignedShort;
      }
      else
      {
        outElementType = tidl_getElementSizeInBits(outElementType) == 8 ? TIDL_UnsignedChar : TIDL_UnsignedShort;
      }
      net->TIDLPCLayers[i].outData[0].elementType = outElementType;
    }
  }
  if ((net->TIDLPCLayers[i].layerType == TIDL_BatchNormLayer)       ||
      (net->TIDLPCLayers[i].layerType == TIDL_ConvolutionLayer)     ||
      (net->TIDLPCLayers[i].layerType == TIDL_DeformableConvLayer)  ||
      (net->TIDLPCLayers[i].layerType == TIDL_InnerProductLayer)    ||
      (net->TIDLPCLayers[i].layerType == TIDL_Deconv2DLayer)        ||
      (net->TIDLPCLayers[i].layerType == TIDL_EltWiseLayer)         ||
      (net->TIDLPCLayers[i].layerType == TIDL_PoolingLayer)         ||
      (net->TIDLPCLayers[i].layerType == TIDL_DataConvertLayer)     ||
      (net->TIDLPCLayers[i].layerType == TIDL_ConcatLayer)          ||
      (net->TIDLPCLayers[i].layerType == TIDL_ConstDataLayer))
  {

    const sTIDL_DataParams_t * indata = TIDL_getOutData(net,
      net->TIDLPCLayers[i].inData[0].dataId);
    if(indata == NULL)
    {
      TIDL_GLOBAL_REPORT_ERROR("Cannot find Producer");
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }

    if (net->TIDLPCLayers[i].layerType == TIDL_BatchNormLayer)
    {
      if (net->TIDLPCLayers[i].actParams.actType != TIDL_GELU && net->TIDLPCLayers[i].actParams.actType != TIDL_Logit &&
          net->TIDLPCLayers[i].actParams.actType != TIDL_Reciprocal && net->TIDLPCLayers[i].actParams.actType != TIDL_SiLU && net->TIDLPCLayers[i].actParams.actType != TIDL_Swish)
      {
        accScale = net->TIDLPCLayers[i].layerParams.batchNormParams.weightScale * indata->tensorScale;
      }
      else
      {
        accScale = indata->tensorScale;
      }
    }
    else if (net->TIDLPCLayers[i].layerType == TIDL_ConstDataLayer)
    {
      accScale = net->TIDLPCLayers[i].layerParams.constDataParams.weightScale;
    }
    else if ((net->TIDLPCLayers[i].layerType == TIDL_ConvolutionLayer) ||
              (net->TIDLPCLayers[i].layerType == TIDL_Deconv2DLayer))
    {
      accScale = net->TIDLPCLayers[i].layerParams.deformConvParams.weightScale * indata->tensorScale;
    }
    else if((net->TIDLPCLayers[i].layerType == TIDL_DeformableConvLayer))
    {
      accScale = net->TIDLPCLayers[i].layerParams.convParams.weightScale * indata->tensorScale;
    }
    else if (net->TIDLPCLayers[i].layerType == TIDL_InnerProductLayer)
    {

      accScale = net->TIDLPCLayers[i].layerParams.innerProductParams.weightScale * indata->tensorScale;
      if(net->TIDLPCLayers[i].layerParams.innerProductParams.weightScale == 0)
      {
        /*MatMul*/
        const sTIDL_DataParams_t * secondBuffer = TIDL_getOutData(net, net->TIDLPCLayers[i].inData[1].dataId);
        if(secondBuffer == NULL)
        {
          TIDL_GLOBAL_REPORT_ERROR("Cannot find Producer");
          return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
        }
        accScale = secondBuffer->tensorScale * indata->tensorScale;
      }
    }
    else if(net->TIDLPCLayers[i].layerType == TIDL_PoolingLayer)
    {
        /* The lines below must match the same lines in function TIDL_poolingInit()
         * Local average pooling 2x2, 3x3, with stride 2 implement an optimized kernel that assumes the scale factors bit-depth are
         * the same as the input data type, which is either 8-bits or 16-bits.
         * The remaining local average pooling assume 16-bits or 32-bits scale factors.
         * */
        if(net->TIDLPCLayers[i].layerParams.poolParams.poolingType == TIDL_AveragePooling)
        {
          if((net->TIDLPCLayers[i].layerParams.poolParams.strideW== 2) &&
             (net->TIDLPCLayers[i].layerParams.poolParams.strideH== 2) &&
             (((net->TIDLPCLayers[i].layerParams.poolParams.kernelW== 2) &&
               (net->TIDLPCLayers[i].layerParams.poolParams.kernelH== 2)) ||
              ((net->TIDLPCLayers[i].layerParams.poolParams.kernelW== 3) &&
               (net->TIDLPCLayers[i].layerParams.poolParams.kernelH== 3))))
          {
            if(procElemSize == 1)
            {
              internal_pooling_weight= TIDL_INTERNAL_POOLING_WEIGHT_Q_U8;
            }
            else
            {
              internal_pooling_weight= TIDL_INTERNAL_POOLING_WEIGHT_Q_U16;
            }
          }
          else
          {
            internal_pooling_weight = TIDL_INTERNAL_POOLING_WEIGHT_Q_U16;
         }
          accScale = ((float32_tidl)(((uint32_t)1) << internal_pooling_weight)) * indata->tensorScale;
        }
        else
        {
          accScale = indata->tensorScale;
        }
    }
    else if ((net->TIDLPCLayers[i].layerType == TIDL_EltWiseLayer) ||
            (net->TIDLPCLayers[i].layerType == TIDL_ConcatLayer))
    {
      uint32_t weightQ = 0;
      if(net->TIDLPCLayers[i].layerType == TIDL_EltWiseLayer)
      {
        if( procElemSize == 1)
        {
          weightQ = TIDL_ELTWISE_INTERNAL_WEIGHT_Q_U8;
        }
        else
        {
          weightQ = TIDL_ELTWISE_INTERNAL_WEIGHT_Q_U16;
        }
      }
      if(net->TIDLPCLayers[i].layerType == TIDL_ConcatLayer)
      {
        if(elementSizeBytes== 1)
        {
          weightQ = TIDL_CONCAT_INTERNAL_WEIGHT_Q_U8;
        }
        else
        {
          weightQ = TIDL_CONCAT_INTERNAL_WEIGHT_Q_U16;
        }
      }

      if((net->TIDLPCLayers[i].layerType == TIDL_EltWiseLayer) && (net->TIDLPCLayers[i].layerParams.eltWiseParams.eltWiseType == TIDL_EltWiseProduct))
      {
        accScale = 1.0f;
        for(j = 0; j < net->TIDLPCLayers[i].numInBufs; j++)
        {
            const sTIDL_DataParams_t * eltWiseIndata = TIDL_getOutData(net,
            net->TIDLPCLayers[i].inData[j].dataId);
            if(eltWiseIndata == NULL)
            {
              TIDL_GLOBAL_REPORT_ERROR("Cannot find Producer");
              return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
            }
            accScale = accScale * eltWiseIndata->tensorScale;
        }
      }
      else
      {
        minScale = FLT_MAX;
        for(j = 0; j < net->TIDLPCLayers[i].numInBufs; j++)
        {
            int32_t inLayerIdx = tidl_getInLayer(*net, net->numLayers, net->TIDLPCLayers[i].inData[j].dataId);
            if(inLayerIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
            {
              TIDL_GLOBAL_REPORT_ERROR("Cannot find producer of layer idx %d with dataId %d", i, net->TIDLPCLayers[i].inData[j].dataId);
              return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
            }
            /*If tensor min/max values are 0, do not use scale/zp from this tensor (Other inputs take priority)*/
            if((net->TIDLPCLayers[i].layerType == TIDL_ConcatLayer || net->TIDLPCLayers[i].layerType == TIDL_EltWiseLayer) && (net->TIDLPCLayers[inLayerIdx].outData[0].minTensorValue == 0 && net->TIDLPCLayers[inLayerIdx].outData[0].maxTensorValue == 0))
            {
              /* Dont use scale if data has all zeros */
              continue;
            }
            const sTIDL_DataParams_t * eltWiseIndata = TIDL_getOutData(net,
            net->TIDLPCLayers[i].inData[j].dataId);
            if(eltWiseIndata == NULL)
            {
              TIDL_GLOBAL_REPORT_ERROR("Cannot find Producer");
              return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
            }
            minScale = (eltWiseIndata->tensorScale  < minScale) ? eltWiseIndata->tensorScale : minScale;
        }
        accScale = minScale * (float32_tidl)(((uint32_t)1)  << weightQ);
      }
    }
    else if (net->TIDLPCLayers[i].layerType == TIDL_DataConvertLayer && 
             (net->TIDLPCLayers[i].layerParams.dataConvertParams.type == TIDL_DC_TYPE_INPUT ||
              net->TIDLPCLayers[i].layerParams.dataConvertParams.type == TIDL_DC_TYPE_OUTPUT))
    
    {
      sTIDL_dataConvertParams_t *params = &net->TIDLPCLayers[i].layerParams.dataConvertParams;
       /* multiply by the quantization scale */
      accScale = indata->tensorScale  * (1 << TIDL_INTERNAL_INDATA_Q);
      /*TIDL-4415: Float to int16 conversion handling for dataconvert:*/
      if(params->type == TIDL_DC_TYPE_INPUT && (net->TIDLPCLayers[i].outData[0].elementType == TIDL_UnsignedShort || net->TIDLPCLayers[i].outData[0].elementType == TIDL_SignedShort) && net->TIDLPCLayers[i].inData[0].elementType == TIDL_SinglePrecFloat)
      {
        /*Determine the nearest power of 2 value*/
        float outTensorScale; int32_t outZeroPoint;
        TIDL_asymRangeToScale(&outTensorScale, &outZeroPoint, net->TIDLPCLayers[i].outData[0].minTensorValue, net->TIDLPCLayers[i].outData[0].maxTensorValue, net->TIDLPCLayers[i].outData[0].tensorType, net->TIDLPCLayers[i].outData[0].elementType);
        float outScaleP2;
        outScaleP2 = round(log(outTensorScale)/log(2.0));
        float outScaleMultiplier = (1 << (int32_t)outScaleP2);
        if(net->TIDLPCLayers[i].outData[0].elementType == TIDL_SignedShort)
        {
          /*Signed 16-bit*/
          outScaleMultiplier == outScaleMultiplier >= (1 << 15U) ? (1 << 14U) : outScaleMultiplier; 
        }
        else
        {
          /*Unsigned 16-bit*/
          outScaleMultiplier == outScaleMultiplier >= (1 << 16U) ? (1 << 15U) : outScaleMultiplier; 
        }
        accScale = indata->tensorScale * outScaleMultiplier;
      }
      if ( params->type == TIDL_DC_TYPE_OUTPUT)
      {
        int32_t configParamsOutIndex;
        configParamsOutIndex = TIDL_getConfigParamOutIndexFromLayerName((const char *)&net->TIDLPCLayers[i].outDataNames[0]);
        if ( configParams->outTensorScale[configParamsOutIndex] != 0.0)
        {
          /* if user has requested any specific scale on the output side then we have to adhere to it hence
          accumulator scale will be determined by the output tensor scale */
          accScale = (int)((net->TIDLPCLayers[i].outData[0].tensorScale / indata->tensorScale ) * (1 << TIDL_INTERNAL_INDATA_Q));
          /* When converting from 16 bit fixed point to 8 bit fixed point the scale become < 1 (some thing like ~0.5). This will be treated as 0 during scale computation in inference.
This is as good as not applying any additional scale on input and directly applying the shift. So made the accScale =1 for this case here and inference as well */
          if(accScale < 1)
          {
            accScale = 1;
          }
          accScale = accScale*indata->tensorScale;

        }
        else if ( (net->TIDLPCLayers[i].outData[0].elementType == TIDL_UnsignedWord) ||
            (net->TIDLPCLayers[i].outData[0].elementType == TIDL_SignedWord) ||
            (net->TIDLPCLayers[i].outData[0].elementType == TIDL_UnsignedDoubleWord) ||
            (net->TIDLPCLayers[i].outData[0].elementType == TIDL_SignedDoubleWord))
        {
          if ( indata->tensorScale == 1.0 )
          {
            accScale = 1.0;
          }
          else
          {
            TIDL_GLOBAL_REPORT_ERROR("Output datatype is word or double word but input and scale are not 1.\nConversion to word or double word is only supported in context of \
                    ArgMax/Min layer where the scales are already expected to be 1. Aborting");
            return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
          }
        }
      }

    }
    else if (net->TIDLPCLayers[i].layerType == TIDL_DataConvertLayer &&
             net->TIDLPCLayers[i].layerParams.dataConvertParams.type == TIDL_DC_TYPE_INTERMEDIATE &&
             net->TIDLPCLayers[i].layerPCParams.dataConvertParams.canUpdateTensorScale == 1)
    {
      accScale = indata->tensorScale;
    }
    
    else
    {
      accScale = 1.0f;
    }

    if(accScale > FLT_MAX)
    {
      accScale = FLT_MAX;
    }

    if(updateStats == 1)
    {
      if ((net->TIDLPCLayers[i].layerType == TIDL_PoolingLayer) &&
          (net->TIDLPCLayers[i].layerParams.poolParams.poolingType == TIDL_AveragePooling))
      {
        float32_tidl tscale =  ((float32_tidl)(((uint32_t)1) << internal_pooling_weight)) * indata->tensorScale;

        curMin = (float32_tidl)accMin / tscale;
        curMax = (float32_tidl)accMax / tscale;
      }
      else
      {
        curMin = (float32_tidl)accMin / accScale;
        curMax = (float32_tidl)accMax / accScale;
      }

    }
    else
    {
      curMin = net->TIDLPCLayers[i].outData[0].minTensorValue;
      curMax = net->TIDLPCLayers[i].outData[0].maxTensorValue;
    }

    /* Do not apply expansion factor for data conversion layer */
    if((gParams.quantRangeExpansionFactor != 1.0f) &&
      (net->TIDLPCLayers[i].layerType != TIDL_DataConvertLayer) )
    {
        curMin = curMin * gParams.quantRangeExpansionFactor;
        curMax = curMax * gParams.quantRangeExpansionFactor;
    }

    if (net->TIDLPCLayers[i].actParams.actType == TIDL_Clip)
    {
      curMin = net->TIDLPCLayers[i].actParams.clipMin;
      net->TIDLPCLayers[i].outData[0].minTensorValue = curMin;
      curMax = net->TIDLPCLayers[i].actParams.clipMax;
      net->TIDLPCLayers[i].outData[0].maxTensorValue = curMax;
      /*Unsigned output is a better fit:*/
      if(net->TIDLPCLayers[i].actParams.clipMin >= 0.0)
      {
        if(net->TIDLPCLayers[i].outData[0].elementType == TIDL_SignedChar)
        {
          net->TIDLPCLayers[i].outData[0].elementType = TIDL_UnsignedChar;
        }
        else if(net->TIDLPCLayers[i].outData[0].elementType == TIDL_SignedShort)
        {
          net->TIDLPCLayers[i].outData[0].elementType = TIDL_UnsignedShort;
        }
      }
    }

    if (net->TIDLPCLayers[i].actParams.actType == TIDL_RelU6)
    {
      curMin = net->TIDLPCLayers[i].outData[0].minTensorValue = 0;
      curMax = net->TIDLPCLayers[i].outData[0].maxTensorValue = 6.0f;
    }
    if ((net->TIDLPCLayers[i].layerType == TIDL_PoolingLayer) &&
        (net->TIDLPCLayers[i].layerParams.poolParams.poolingType == TIDL_AveragePooling)&&
        (net->TIDLPCLayers[i].actParams.actType != TIDL_Clip))
    {
      if((net->TIDLPCLayers[i].layerParams.poolParams.kernelW== 0) &&
         (net->TIDLPCLayers[i].layerParams.poolParams.kernelH== 0))
      {
        curMax = curMax * 1.25;
        outMin = outMin * 1.25;
      }
    }
    if((net->TIDLPCLayers[i].layerType == TIDL_InnerProductLayer)&&
    (net->TIDLPCLayers[i].actParams.actType != TIDL_Clip))
    {
      curMax = curMax * 1.25;
      outMin = outMin * 1.25;
    }

    if (net->TIDLPCLayers[i].layerType == TIDL_DataConvertLayer &&
        net->TIDLPCLayers[i].layerParams.dataConvertParams.type == TIDL_DC_TYPE_INTERMEDIATE &&
        net->TIDLPCLayers[i].layerPCParams.dataConvertParams.canUpdateTensorScale == 1)
    {
      int32_t outLayerIdx = tidl_getOutLayer(*net, net->numLayers, net->TIDLPCLayers[i].outData[0].dataId);
    }
    outMin = curMin * accScale;
    outMax = curMax * accScale;
    max = (fabs(outMax) > fabs(outMin)) ? fabs(outMax) : fabs(outMin);


    if (TIDL_getDatElementSign(net->TIDLPCLayers[i].outData[0].elementType) == 1)
    {
      elementSizeBits = (elementSizeBytes * 8) - 1;
      maxP2 = (int32_t)ceil(log((float64_tidl)max) / log((float64_tidl)2));
      maxP2 = pow(2.0, (double)maxP2);
      if(max == maxP2)
      {
        max -= 1;
      }
    }
    else
    {
      elementSizeBits = (elementSizeBytes * 8);
    }



    /* If max== INFINITY, there is a discrepancy between host emulation and target in the result of maxP2
     * if maxP2= (int32_t)ceil(log((float64_tidl)max) / log((float64_tidl)2));
     * So to avoid any issue, we force maxP2 to be 0 only for that particular corner case when max==INFINITY
     * which shouldn't happen in real-world use-case anyway.
     */
    if (max <= FLT_MAX)
    {
        outDiv =  max / (1 << elementSizeBits);
        if(outDiv >= 1.0)
        {
          maxP2 = (int32_t)ceil(log((float64_tidl)outDiv) / log((float64_tidl)2));
          maxP2 = pow(2.0, (double)maxP2);
          if(maxP2 >= 2*outDiv)
          {
            maxP2 = maxP2 / 2;
          }
          maxP2 = (int32_t)ceil(log((float64_tidl)maxP2) / log((float64_tidl)2));
        }
        else
        {
          maxP2 = 0;
        }
    }
    else
    {
        maxP2= 0; /* this will force the code to take the 'else' side of the next 'if (maxP2 > elementSizeBits)' */
    }

    canUpdateTensorScale = TIDL_canUpdateTensorScale(&net->TIDLPCLayers[i], configParams);

    if ((maxP2 > 0) &&  canUpdateTensorScale)
    {
      net->TIDLPCLayers[i].outData[0].roundBits = maxP2;
      if ((net->TIDLPCLayers[i].layerType == TIDL_PoolingLayer) &&
          (net->TIDLPCLayers[i].layerParams.poolParams.poolingType == TIDL_AveragePooling))
      {
        net->TIDLPCLayers[i].outData[0].tensorScale = accScale / (float32_tidl)(((uint32_t)1) << net->TIDLPCLayers[i].outData[0].roundBits);
      }
      else
      {
        if(net->TIDLPCLayers[i].quantizeConstraint != TIDL_QuantizeConstainedP2)
        {
          net->TIDLPCLayers[i].outData[0].tensorScale = accScale / (float32_tidl)(((uint32_t)1)  << net->TIDLPCLayers[i].outData[0].roundBits);
        }
        int32_t dataIDWeightScale = net->TIDLPCLayers[i].outData[0].dataId;
        int32_t outLayerdx = tidl_getOutLayer(*net, net->numLayers, net->TIDLPCLayers[i].outData[0].dataId);
        if(outLayerdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
        {
          return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
        }
        if(TIDL_isPadOTF(gParams.deviceName))
        {
          if((net->TIDLPCLayers[i].layerType == TIDL_Deconv2DLayer) &&( net->TIDLPCLayers[outLayerdx].layerType == TIDL_DataConvertLayer))
          {
            /* This piece of code is for Deconv to see outData of the DC layer succeeding it, for collection of stats */
            dataIDWeightScale = net->TIDLPCLayers[outLayerdx].outData[0].dataId;
          }
          else if((outLayerdx!=-1) && (net->TIDLPCLayers[outLayerdx].layerType == TIDL_DataConvertLayer))
          {
            if(net->TIDLPCLayers[outLayerdx].layerParams.dataConvertParams.type == TIDL_DC_TYPE_INTERMEDIATE)
            {
              dataIDWeightScale = net->TIDLPCLayers[outLayerdx].outData[0].dataId;
            }
          }
        }

        float32_tidl wightScale = TIDL_maxWeightScale(net,dataIDWeightScale);
        float outTensorScale;
        int32_t isOutMaxSat = isOutputTensorMaxSatAvailable(&net->TIDLPCLayers[i], &outTensorScale);
        if(net->TIDLPCLayers[i].quantizeConstraint == TIDL_QuantizeConstainedP2)
        {
          outTensorScale = net->TIDLPCLayers[i].outData[0].tensorScale;
          /*Updated round bits:*/
          net->TIDLPCLayers[i].outData[0].roundBits = round(log(accScale/outTensorScale)/log(2));
        }
       /* When output tensor needs to be clipped at specific value (Like Relu6), the scale for this tensor has the fixed.
           So, in this case tensor scale reduction is not possible. While selecting the weight scale, this criteria is already considered.
           For QAT case, the tensor scale is always reduced to satisfy the bias scale limitation in the next layer. So the TIDL_CalibOptionBiasRange
           is used to by-pass the saturation condition
        */
        if((isOutMaxSat) && ((gParams.calibrationOption & TIDL_CalibOptionBiasRange) == 0))
        {
         if ((net->TIDLPCLayers[i].layerType == TIDL_EltWiseLayer) ||
             (net->TIDLPCLayers[i].layerType == TIDL_ConcatLayer))
            {
                while(((((float32_tidl)(((uint32_t)1)  << net->TIDLPCLayers[i].outData[0].roundBits))*  outTensorScale) >= accScale) &&
                      (net->TIDLPCLayers[i].outData[0].roundBits > 0))

                {
                  net->TIDLPCLayers[i].outData[0].roundBits--;
                }
                accScale = ((float32_tidl)(((uint32_t)1)  << net->TIDLPCLayers[i].outData[0].roundBits))*  outTensorScale;
                net->TIDLPCLayers[i].outData[0].tensorScale = accScale / (float32_tidl)(((uint32_t)1)  << net->TIDLPCLayers[i].outData[0].roundBits);
            }

          wightScale = 0.0001f;
          if((outTensorScale != net->TIDLPCLayers[i].outData[0].tensorScale) && (gParams.debugTraceLevel > 0))
          {
           TIDL_GLOBAL_REPORT_WARNING("Tensorscale value is not met for the relu6/Clip Activation -  %d \n", i);
          }
        }
        float currTensorScale = net->TIDLPCLayers[i].outData[0].tensorScale;
        int32_t numAddBits = 0;

        /* Find the number of bits needs to be reduced in the tensor scale to avoid saturation in Bias of next layer */
        while ((currTensorScale * wightScale) > biasExpanScale)
        {
          currTensorScale /= 2.0f;
          numAddBits++;
        }
        /* if TIDL_CalibOptionBiasRange is enabled, then  the scale reduction
           is NOT applied on Tensor scale. This would be enabled in QAT cases where tensor
           scale is selected at training time. In this bias would be saturated which is accounted
          during training
          */
        if((numAddBits > 0) && ((gParams.calibrationOption & TIDL_CalibOptionBiasRange) == 0))
        {
          numAddBits = (numAddBits + 1) / 2;
          net->TIDLPCLayers[i].outData[0].roundBits += numAddBits;
          net->TIDLPCLayers[i].outData[0].tensorScale /= (1 << numAddBits);
          if(gParams.debugTraceLevel > 0)
          {
            TIDL_GLOBAL_REPORT_WARNING("Tensor Scale for layer %d is reduced by %d Bits \n", i, numAddBits);
          }
        }

      }
    }
    else
    {
      if (net->TIDLPCLayers[i].layerType == TIDL_DataConvertLayer &&
         (net->TIDLPCLayers[i].layerParams.dataConvertParams.type == TIDL_DC_TYPE_OUTPUT ||
          net->TIDLPCLayers[i].layerParams.dataConvertParams.type == TIDL_DC_TYPE_INPUT))
      {
        sTIDL_dataConvertParams_t *params = &net->TIDLPCLayers[i].layerParams.dataConvertParams;
        if ( params->type == TIDL_DC_TYPE_OUTPUT)
        {
          int32_t configParamsOutIndex;
          configParamsOutIndex = TIDL_getConfigParamOutIndexFromLayerName((const char *)&net->TIDLPCLayers[i].outDataNames[0]);
          if ( configParams->outTensorScale[configParamsOutIndex] != 0.0)
          {
            /* This indicates that for data convert layer on output side, output tensor scale is given by the
            user and we  have to adhere to it hence disable updating the tensor scale. If output is float then this
            gets automatically taken care as scale to be multiplied is float*/
            if (net->TIDLPCLayers[i].outData[0].elementType == TIDL_SinglePrecFloat )
            {
              net->TIDLPCLayers[i].outData[0].roundBits   = 0;
            }
            else
            {
              maxP2 = accScale / net->TIDLPCLayers[i].outData[0].tensorScale ;
              maxP2 = (int32_t)round(log((float64_tidl)maxP2) / log((float64_tidl)2));;
              /* Shift by the same amount which was multiplied to the accumular to bring the scale back to original value */
              //net->TIDLPCLayers[i].outData[0].roundBits   = TIDL_INTERNAL_INDATA_Q;
              if(maxP2 >= 0)
              {
                net->TIDLPCLayers[i].outData[0].roundBits     = maxP2;
              }
              else
              {
                net->TIDLPCLayers[i].outData[0].roundBits     = 0;
              }
            }

         }
        }
        else
        {
          net->TIDLPCLayers[i].outData[0].roundBits  = 0;
          net->TIDLPCLayers[i].outData[0].tensorScale = accScale;
        }
      }
      else
      {
        net->TIDLPCLayers[i].outData[0].roundBits  = 0;
        net->TIDLPCLayers[i].outData[0].tensorScale = accScale;
      }
    }
  }
  else  if ((net->TIDLPCLayers[i].layerType == TIDL_ArgOpLayer) ||
            (net->TIDLPCLayers[i].layerType == TIDL_DetectionOutputLayer))
  {
    net->TIDLPCLayers[i].outData[0].roundBits = 0;
    net->TIDLPCLayers[i].outData[0].tensorScale = 1.0f;
  }
  else if(net->TIDLPCLayers[i].layerType == TIDL_SoftMaxLayer)
  {
    if(net->TIDLPCLayers[i].outData[0].elementType == TIDL_SignedChar){
      net->TIDLPCLayers[i].outData[0].elementType = TIDL_UnsignedChar;
    }
    else if(net->TIDLPCLayers[i].outData[0].elementType == TIDL_SignedShort){
      net->TIDLPCLayers[i].outData[0].elementType = TIDL_UnsignedShort;
    }

    if(net->TIDLPCLayers[i].outData[0].elementType < TIDL_UnsignedWord)
    {
      /*8/16-bit*/
      if(net->TIDLPCLayers[i].outData[0].elementType < TIDL_UnsignedShort)
      {
        /*8-bit*/
        /*Always have unsigned output with ZP=0*/
        net->TIDLPCLayers[i].outData[0].tensorScale = 256;
        net->TIDLPCLayers[i].outData[0].tensorZeroPoint = 0;
      }
      else
      {
        if (gParams.softmax16BitScaleUpdate == 0)
        {
          /*16-bit*/
          /*Always have unsigned output with ZP=0*/
          //net->TIDLPCLayers[i].outData[0].dimValues[]
          int32_t dim_y;
          int32_t axis = net->TIDLPCLayers[i].layerParams.softMaxParams.axis;
          if(axis > TIDL_DIM_WIDTH || axis < TIDL_DIM_BATCH)
          {
            axis = 0;
          }
          dim_y = net->TIDLPCLayers[i].inData[0].dimValues[axis];
          /*Scale is selected in such a way to enable MMA Kernel*/
          net->TIDLPCLayers[i].outData[0].tensorScale = SOFTMAX_16_BIT_SCALE_LIM*dim_y;
          net->TIDLPCLayers[i].outData[0].tensorZeroPoint = 0;
        }
        else
        {
          net->TIDLPCLayers[i].outData[0].tensorScale = 256*256;
          net->TIDLPCLayers[i].outData[0].tensorZeroPoint = 0;          
        }

      }
    }
    else
    {
      net->TIDLPCLayers[i].outData[0].roundBits = 0;
      net->TIDLPCLayers[i].outData[0].tensorScale = 1.0f;
    }
  }
  else  if (net->TIDLPCLayers[i].layerType == TIDL_CustomLayer)
  {
    float32_tidl wightScale = TIDL_maxWeightScale(net,net->TIDLPCLayers[i].outData[0].dataId);
    float32_tidl maxOutputTensorScale = 1.0;

    /* Make sure inData is updated by copying outData from producer layer */
    for (int bufIdx = 0; bufIdx < net->TIDLPCLayers[i].numInBufs; bufIdx++)
    {
      int32_t inLayerdx = tidl_getInLayer(*net, net->numLayers, net->TIDLPCLayers[i].inData[bufIdx].dataId);
      if(inLayerdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
      {
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }
      net->TIDLPCLayers[i].inData[bufIdx] = net->TIDLPCLayers[inLayerdx].outData[0];
    }


    /* Maximum possible output tensor scale without bias saturation in the consumer scale */
    maxOutputTensorScale = biasExpanScale/ wightScale;

    net->TIDLPCLayers[i].outData[0].tensorScale = TIDL_getCustomLayerOutputTensorScale(net,
                                                                                  net->TIDLPCLayers[i].weights.ptr,
                                                                                  i,
                                                                                  net->TIDLPCLayers[i].outData[0].minTensorValue,
                                                                                  net->TIDLPCLayers[i].outData[0].maxTensorValue,
                                                                                  maxOutputTensorScale);
  }
  else if(net->TIDLPCLayers[i].layerType == TIDL_ScatterElementsLayer)
  {
    if(net->TIDLPCLayers[i].layerParams.scatterElementsParams.axis != -1 || net->TIDLPCLayers[i].layerParams.scatterElementsParams.reduction != TIDL_ScatterElementsAdd)
    {
      // scatter elements out scale is same as the update scale
      int32_t updateLyrIdx = tidl_getInLayer(*net, net->numLayers, net->TIDLPCLayers[i].inData[2].dataId);
      if(updateLyrIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
      {
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }
      if (net->TIDLPCLayers[updateLyrIdx].layerType == TIDL_ConstDataLayer)
      {
        // take the data scale, if the updates is a const tensor and having all zeros, currently assuming all zeros if const tensor
        j = 0;
      }
      else
      {
        j = 2;
      }
      const sTIDL_DataParams_t * indata = TIDL_getOutData(net,
        net->TIDLPCLayers[i].inData[j].dataId);
      if(indata == NULL)
      {
        TIDL_GLOBAL_REPORT_ERROR("Cannot find Producer");
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }

      net->TIDLPCLayers[i].outData[0].roundBits = 0;
      net->TIDLPCLayers[i].outData[0].tensorScale    = indata->tensorScale;

      /*Updated code:*/
      /*Check if both updates and data are DC layers:*/
      int32_t dataLayerIdx = tidl_getInLayer(*net, net->numLayers, net->TIDLPCLayers[i].inData[0].dataId);

      /* Scatter layer's updates and data inputs should be DataConvert, following condition skips a reshape if present */
      if(net->TIDLPCLayers[dataLayerIdx].layerType == TIDL_ReshapeLayer)
      {
        dataLayerIdx = tidl_getInLayer(*net, net->numLayers, net->TIDLPCLayers[dataLayerIdx].inData[0].dataId);
      }
      if(net->TIDLPCLayers[updateLyrIdx].layerType == TIDL_ReshapeLayer)
      {
        updateLyrIdx = tidl_getInLayer(*net, net->numLayers, net->TIDLPCLayers[updateLyrIdx].inData[0].dataId);
      }

      if(net->TIDLPCLayers[updateLyrIdx].layerType == TIDL_DataConvertLayer && net->TIDLPCLayers[dataLayerIdx].layerType == TIDL_DataConvertLayer)
      {
        float32_tidl updateScale, dataScale, scatterScale;
        uint8_t fixedScale, shiftVal;
        updateScale = net->TIDLPCLayers[updateLyrIdx].outData[0].tensorScale;
        dataScale = net->TIDLPCLayers[dataLayerIdx].outData[0].tensorScale;
        scatterScale = min(updateScale, dataScale);
        net->TIDLPCLayers[i].outData[0].tensorScale = scatterScale;
        net->TIDLPCLayers[i].outData[0].roundBits = 0;
        /*Correct the data convert layer scales & ZP*/
        if(updateScale == scatterScale)
        {
          /*Update "data" DC layer's scale computation logic:*/ 
          TIDL_getMMAv2_ScaleShiftAndError((scatterScale/dataScale), &fixedScale, &shiftVal, 8U, TIDL_MMAV2_MAX_SCALE_16);
          net->TIDLPCLayers[dataLayerIdx].outData[0].tensorScale = scatterScale;
          net->TIDLPCLayers[dataLayerIdx].outData[0].roundBits = shiftVal;
        }
        else
        {
          /*Update "Updates" DC layer's scale computation logic:*/
          TIDL_getMMAv2_ScaleShiftAndError((scatterScale/updateScale), &fixedScale, &shiftVal, 8U, TIDL_MMAV2_MAX_SCALE_16);
          net->TIDLPCLayers[updateLyrIdx].outData[0].tensorScale = scatterScale;
          net->TIDLPCLayers[updateLyrIdx].outData[0].roundBits = shiftVal; 
        }
      }

      /* Scatter layer's updates and data inputs should be DataConvert, following condition skips a reshape if present */
      int32_t dataLayerConsumerIdx = tidl_getOutLayer(*net, net->numLayers, net->TIDLPCLayers[dataLayerIdx].outData[0].dataId);
      if(net->TIDLPCLayers[dataLayerConsumerIdx].layerType == TIDL_ReshapeLayer)
      {
        net->TIDLPCLayers[dataLayerConsumerIdx].outData[0].tensorScale = net->TIDLPCLayers[dataLayerIdx].outData[0].tensorScale;
        net->TIDLPCLayers[dataLayerConsumerIdx].outData[0].roundBits = net->TIDLPCLayers[dataLayerIdx].outData[0].roundBits;
      }
      int32_t updateLayerConsumerIdx = tidl_getOutLayer(*net, net->numLayers, net->TIDLPCLayers[updateLyrIdx].outData[0].dataId);
      if(net->TIDLPCLayers[updateLayerConsumerIdx].layerType == TIDL_ReshapeLayer)
      {
        net->TIDLPCLayers[updateLayerConsumerIdx].outData[0].tensorScale = net->TIDLPCLayers[updateLyrIdx].outData[0].tensorScale;
        net->TIDLPCLayers[updateLayerConsumerIdx].outData[0].roundBits = net->TIDLPCLayers[updateLyrIdx].outData[0].roundBits;
      }
    }
    else
    {
      /* ScatterND case*/
      TIDL_asymRangeToScale(&(net->TIDLPCLayers[i].outData[0].tensorScale), &(net->TIDLPCLayers[i].outData[0].tensorZeroPoint),
                                net->TIDLPCLayers[i].outData[0].minTensorValue, net->TIDLPCLayers[i].outData[0].maxTensorValue,
                                TIDL_SYMMETRIC_TENSOR, net->TIDLPCLayers[i].outData[0].elementType);

      /*If Next layer is slice then we take range from the slice layer.
          Out of range updates are usually added at the last element/line. This may inflate the range of the scatterND layer.
          To get the accurate layer, we slice the output of the scatterND and thus the range of data is considered for the quantization*/
      int32_t outLayerdx = tidl_getOutLayer(*net, net->numLayers, net->TIDLPCLayers[i].outData[0].dataId);
      if(outLayerdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
      {
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }

      if(net->TIDLPCLayers[outLayerdx].layerType == TIDL_SliceLayer)
      {
        TIDL_asymRangeToScale(&(net->TIDLPCLayers[i].outData[0].tensorScale), &(net->TIDLPCLayers[i].outData[0].tensorZeroPoint),
                                net->TIDLPCLayers[outLayerdx].outData[0].minTensorValue, net->TIDLPCLayers[outLayerdx].outData[0].maxTensorValue,
                                TIDL_SYMMETRIC_TENSOR, net->TIDLPCLayers[i].outData[0].elementType);
      }

      /* In case of const data layer, it is constrained to a zero tensor, 
        ensuring the output element type to be same as the updates input element type */
      int32_t dataLayerIdx = tidl_getInLayer(*net, net->numLayers, net->TIDLPCLayers[i].inData[0].dataId);
      int32_t updateLayerIdx = tidl_getInLayer(*net, net->numLayers, net->TIDLPCLayers[i].inData[1].dataId);
      if(dataLayerIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL || updateLayerIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
      {
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }

      if(net->TIDLPCLayers[dataLayerIdx].layerType == TIDL_ConstDataLayer)
      {
        net->TIDLPCLayers[dataLayerIdx].outData[0].elementType = net->TIDLPCLayers[i].outData[0].elementType;
      }
    }


  }
  else if(net->TIDLPCLayers[i].layerType == TIDL_LayerNormLayer)
  {
    if (configParams->quantizationStyle == TIDL_QuantStyleP2Dynamic)
    {
      TIDL_rangeToScaleP2(&(net->TIDLPCLayers[i].outData[0].tensorScale), net->TIDLPCLayers[i].outData[0].minTensorValue, net->TIDLPCLayers[i].outData[0].maxTensorValue, 16, net->TIDLPCLayers[i].outData[0].elementType);
    }
    else
    {
      TIDL_asymRangeToScale(&(net->TIDLPCLayers[i].outData[0].tensorScale), &(net->TIDLPCLayers[i].outData[0].tensorZeroPoint), net->TIDLPCLayers[i].outData[0].minTensorValue, net->TIDLPCLayers[i].outData[0].maxTensorValue, TIDL_SYMMETRIC_TENSOR, net->TIDLPCLayers[i].outData[0].elementType);
    }
  }
  else if ((net->TIDLPCLayers[i].layerType == TIDL_EltWiseLayer) &&
           (net->TIDLPCLayers[i].layerParams.eltWiseParams.eltWiseType == TIDL_EltWiseMod))
  {
    TIDL_asymRangeToScale(&(net->TIDLPCLayers[i].outData[0].tensorScale), &(net->TIDLPCLayers[i].outData[0].tensorZeroPoint), net->TIDLPCLayers[i].outData[0].minTensorValue,
                          net->TIDLPCLayers[i].outData[0].maxTensorValue, TIDL_SYMMETRIC_TENSOR, net->TIDLPCLayers[i].outData[0].elementType);
    net->TIDLPCLayers[i].outData[0].roundBits = 0;
  }
  else if (! ((net->TIDLPCLayers[i].layerType == TIDL_DataLayer) && (net->TIDLPCLayers[i].numOutBufs > 0)) )
  {
    /* tensorScale for input data layer is already set, and if tried to write here, all scales are overwritten with first scale since inData[0].dataId = 0
    for all input data layers */
    const sTIDL_DataParams_t * indata = TIDL_getOutData(net,
      net->TIDLPCLayers[i].inData[0].dataId);
    if(indata == NULL)
    {
      TIDL_GLOBAL_REPORT_ERROR("Cannot find Producer");
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }
    net->TIDLPCLayers[i].outData[0].roundBits = 0;
    net->TIDLPCLayers[i].outData[0].tensorScale    = indata->tensorScale;
    net->TIDLPCLayers[i].outData[0].tensorZeroPoint    = indata->tensorZeroPoint; 
    net->TIDLPCLayers[i].outData[0].elementType   = indata->elementType;
    if (net->TIDLPCLayers[i].outData[0].tensorScale == 0)
    {
      /* Tensor scale is 0. This is not allowed */
      net->TIDLPCLayers[i].outData[0].tensorScale = 1;
    }
  }
  if(net->TIDLPCLayers[i].outData[0].tensorScale == 0)
  {
    net->TIDLPCLayers[i].outData[0].tensorScale = FLT_MIN;
  }

  if(net->TIDLPCLayers[i].outData[0].tensorScale > FLT_MAX)
  {
    net->TIDLPCLayers[i].outData[0].tensorScale = FLT_MAX;
  }
  if( (net->TIDLPCLayers[i].layerType == TIDL_DataConvertLayer && 
       net->TIDLPCLayers[i].layerParams.dataConvertParams.type == TIDL_DC_TYPE_INTERMEDIATE &&
       net->TIDLPCLayers[i].layerPCParams.dataConvertParams.canUpdateTensorScale == 0) || 
      (net->TIDLPCLayers[i].layerType == TIDL_CropLayer && net->TIDLPCLayers[i].layerParams.cropParams.multiCoreMode != TIDL_NOT_MULTI_CORE) )
  {
      /* This piece of code is for making the intermediate DC layer and crop layer added for multi-core act as a bypass layer act as a bypass layer */
      net->TIDLPCLayers[i].outData[0].roundBits = 0;
      const sTIDL_DataParams_t * indata = TIDL_getOutData(net,
      net->TIDLPCLayers[i].inData[0].dataId);
      if(indata == NULL)
      {
        TIDL_GLOBAL_REPORT_ERROR("Cannot find Producer");
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }
      net->TIDLPCLayers[i].outData[0].tensorScale    = indata->tensorScale;
      net->TIDLPCLayers[i].outData[0].tensorZeroPoint   = indata->tensorZeroPoint;
  }
  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}


#define TIDL_NUM_WEIGHT_HISTOGRAM_BINS (1024U)
int32_t TIDL_importQuantLayerParams_HPTQ(sTIDL_OrgNetwork_t   * pOrgTIDLNetStructure,
                                            sTIDL_Network_t        *pTIDLNetStructure,
                                            tidl_import_config       *configParams,
                                            int32_t layerIndex)
{
  int32_t i;
  int32_t numBins = TIDL_NUM_WEIGHT_HISTOGRAM_BINS;
  char filenameStr[1000];
  sprintf(filenameStr, "%s_paramDebug.csv", configParams->outputNetFile);

  if (configParams->modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE)
  {
    //Scale update for the first layer (Needs to generalize for multi input):
    pOrgTIDLNetStructure->TIDLPCLayers[1].inData[0].tensorScale = gParams.inQuantFactor[0];
    pOrgTIDLNetStructure->TIDLPCLayers[1].inData[0].tensorZeroPoint = gParams.inZeroPoint[0];
    pOrgTIDLNetStructure->TIDLPCLayers[0].outData[0].tensorScale = gParams.inQuantFactor[0];
    pOrgTIDLNetStructure->TIDLPCLayers[0].outData[0].tensorZeroPoint = gParams.inZeroPoint[0];
  }

  paramDebugFile = fopen(filenameStr, "w+");
  if (paramDebugFile == NULL)
  {
    TIDL_GLOBAL_REPORT_ERROR("Could not open %s file. Aborting", filenameStr);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  fprintf(paramDebugFile, "LayerId , meanDifference, maxDifference, meanOrigFloat, meanRelDifference, orgmax, quantizedMax,orgAtmaxDiff, quantizedAtMaxDiff,maxRelDifference, Scale , , , , Hist \n");

  for (i = 0; i < layerIndex; i++)
  {
    int32_t weightsElementSizeInBits;
    debugLayeId = i;
    int32_t numGroups = pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.convParams.numGroups;
    int32_t numInChannels = pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.convParams.numInChannels;
    int32_t numOutChannels = pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.convParams.numOutChannels;

    if  ((pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_ConvolutionLayer) ||
        (pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_InnerProductLayer) ||
        (pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_Deconv2DLayer) ||
        (pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_BatchNormLayer) ||
        (pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_DetectionOutputLayer)
        )
    {
      float min_clipped = FLT_MAX;
      float max_clipped = -FLT_MAX;
      float min = FLT_MAX;
      float max = -FLT_MAX;
      float maxWeightScale = FLT_MAX;

      if ((pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_ConvolutionLayer) ||
          (pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_Deconv2DLayer))
      {
       //Deal with cases when bias is disabled.. bias HAS to be enabled to support asymmetric quantization
       //Modify bias based on:
       /*****************************************************
        zp,wts,etc..

       *******************************************************/
       if (pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_ConvolutionLayer)
       {
         //TFLITE IMPLICIT ASSUMPTION THAT WEIGHTS ARE INT8
         int32_t outIdx;
         int32_t wtIdx;
         int8_t *pWeights = (int8_t*) pOrgTIDLNetStructure->TIDLPCLayers[i].weights.ptr;
         float32_tidl *pWeightScales = (float32_tidl*)pOrgTIDLNetStructure->TIDLPCLayers[i].weightScales.ptr;
         int32_t *pBias = (int32_t*) pOrgTIDLNetStructure->TIDLPCLayers[i].bias.ptr;
         int32_t originalBias;
         float32_tidl unitConvResult = 0;
         int32_t numInWeights = (numInChannels/numGroups)*(pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.convParams.kernelH * pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.convParams.kernelW);
         int8_t z_x = pOrgTIDLNetStructure->TIDLPCLayers[i].inData[0].tensorZeroPoint;
         int8_t z_y = pOrgTIDLNetStructure->TIDLPCLayers[i].outData[0].tensorZeroPoint;
         float32_tidl scale_x = pOrgTIDLNetStructure->TIDLPCLayers[i].inData[0].tensorScale;
         float32_tidl scale_y = pOrgTIDLNetStructure->TIDLPCLayers[i].outData[0].tensorScale;
         float32_tidl scale_w = 1;
         float32_tidl nScale = 1;
         float32_tidl finalBias = 0;

         for(outIdx = 0; outIdx < numOutChannels; outIdx++)
         {
           unitConvResult = 0;
           scale_w = pWeightScales[outIdx];
           for(wtIdx = 0; wtIdx < numInWeights ; wtIdx++) //Is dilation taken care of here??
           {
             unitConvResult += pWeights[wtIdx + outIdx*numInWeights];
           }
           originalBias = pBias[outIdx];
           nScale = scale_y/(scale_x * scale_w);
           finalBias = originalBias + ( z_y * nScale - (z_x * unitConvResult));
           float32_tidl absBias = finalBias < 0 ? (finalBias*-1) : finalBias;
           if(absBias > ((float)2147483647))
           {
             //Saturation case, i.e. filter and input are irrelevant:
             //Zero weights out: (Consequence of a huge bias is that your output is more or less going to be a DC term)
             for(wtIdx = 0; wtIdx < numInWeights ; wtIdx++) //Is dilation taken care of here??
             {
               pWeights[wtIdx + outIdx*numInWeights] = 0;
             }
             //At this point only a bias term exists and weight scale needs to be modified for it to fit in the 32-bit bias container:
             //Default weight scale to 1 and recalc bias:
             //Bias in output domain:
             finalBias = (finalBias/nScale);
             //In weight scale = 1 domain:
             pWeightScales[outIdx] = 1;
             //Switching to original domain with new scale:
             finalBias = ((finalBias * scale_y)/scale_x);
             pBias[outIdx] = round(finalBias);
           }
           else
           {
              pBias[outIdx] = round(finalBias);
           }
         }
       }
      }
      else if (pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_InnerProductLayer)
      {
        //TFLITE IMPLICIT ASSUMPTION THAT WEIGHTS ARE INT8 && only for single ROI
        int32_t outIdx;
        int32_t wtIdx;
        int8_t *pWeights = (int8_t*) pOrgTIDLNetStructure->TIDLPCLayers[i].weights.ptr;
        float32_tidl *pWeightScales = (float32_tidl*)pOrgTIDLNetStructure->TIDLPCLayers[i].weightScales.ptr;
        int32_t *pBias = (int32_t*) pOrgTIDLNetStructure->TIDLPCLayers[i].bias.ptr;
        int32_t originalBias;
        float32_tidl unitConvResult = 0;
        /* TODO: CHECK THIS AREA AGAIN*/
        int32_t numInWeights = (pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.innerProductParams.numInCols);
        int32_t numInBias = pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.innerProductParams.numOutCols;

        pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.innerProductParams.weightScale = *pWeightScales; //Global weight scale populated here!
        int8_t z_x = pOrgTIDLNetStructure->TIDLPCLayers[i].inData[0].tensorZeroPoint;
        int8_t z_y = pOrgTIDLNetStructure->TIDLPCLayers[i].outData[0].tensorZeroPoint;
        float32_tidl scale_x = pOrgTIDLNetStructure->TIDLPCLayers[i].inData[0].tensorScale;
        float32_tidl scale_y = pOrgTIDLNetStructure->TIDLPCLayers[i].outData[0].tensorScale;
        float32_tidl scale_w = 1;
        float32_tidl nScale = 1;
        float32_tidl finalBias = 0;
        for(outIdx = 0; outIdx <  numInBias; outIdx++)
        {
          unitConvResult = 0;
          scale_w = pWeightScales[0];

          for(wtIdx = 0; wtIdx < numInWeights ; wtIdx++) //Is dilation taken care of here??
          {
            unitConvResult += pWeights[wtIdx + outIdx*numInWeights];
          }

          originalBias = pBias[outIdx];
          nScale = scale_y/(scale_x * scale_w);
          finalBias = originalBias + ( z_y * nScale - (z_x * unitConvResult));
          float32_tidl absBias = finalBias < 0 ? (finalBias*-1) : finalBias;
          if(absBias > ((float)2147483647))
          {
            //Saturation case, i.e. filter and input are irrelevant:
            //Zero weights out: (Consequence of a huge bias is that your output is more or less going to be a DC term)
            for(wtIdx = 0; wtIdx < numInWeights ; wtIdx++) //Is dilation taken care of here??
            {
              pWeights[wtIdx + outIdx*numInWeights] = 0;
            }
            //At this point only a bias term exists and weight scale needs to be modified for it to fit in the 32-bit bias container:
            //Default weight scale to 1 and recalc bias:
            //Bias in output domain:
            finalBias = (finalBias/nScale);
            //In weight scale = 1 domain:
            pWeightScales[outIdx] = 1;
            //Switching to original domain with new scale:
            finalBias = ((finalBias * scale_y)/scale_x);
            pBias[outIdx] = round(finalBias);
          }
          else
          {
            pBias[outIdx] = round(finalBias);
          }
        }
      }
      else if (pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_BatchNormLayer)
      {
        //TFL doesn't have any implementation - needs to be potentially handled for other frameworks for PTQ ingest
      }
    }
    else if(pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_SoftMaxLayer)
    {
      pOrgTIDLNetStructure->TIDLPCLayers[i].outData[0].elementType = TIDL_SignedChar;
      pOrgTIDLNetStructure->TIDLPCLayers[i].inData[0].elementType  = TIDL_SignedChar;
    }
    else if(pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_PoolingLayer /*  check for global avg pooling also  */)
    {
      if(pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.poolParams.poolingType == TIDL_AveragePooling)
      {
        int32_t internal_pooling_weight;
        int32_t procElemSize     = TIDLIT_getProcessingElementSizeInBytes(&pOrgTIDLNetStructure->TIDLPCLayers[i]);
        float32_tidl accScale = 1.0f;
        const sTIDL_DataParams_t * indata = TIDL_getOutData(pOrgTIDLNetStructure, pOrgTIDLNetStructure->TIDLPCLayers[i].inData[0].dataId);
        if(indata == NULL)
        {
          TIDL_GLOBAL_REPORT_ERROR("Cannot find Producer");
          return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
        }
        if((pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.poolParams.strideW== 2) &&
            (pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.poolParams.strideH== 2) &&
            (((pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.poolParams.kernelW== 2) &&
              (pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.poolParams.kernelH== 2)) ||
            ((pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.poolParams.kernelW== 3) &&
              (pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.poolParams.kernelH== 3))))
        {

          if(procElemSize == 1 && ((pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.poolParams.kernelW== 3) &&
              (pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.poolParams.kernelH== 3)))
          {
            //internal_pooling_weight= TIDL_INTERNAL_POOLING_WEIGHT_Q_U8;
            internal_pooling_weight= TIDL_INTERNAL_POOLING_WEIGHT_Q_U16;/*Setting this to 2^12 for 8-bit case also to get higher accuracy while losing out on performance*/
          }
          else if(procElemSize == 1){
            internal_pooling_weight= TIDL_INTERNAL_POOLING_WEIGHT_Q_U8;
          }
          else
          {
            internal_pooling_weight= TIDL_INTERNAL_POOLING_WEIGHT_Q_U16;
          }

        }
        else
        {
          internal_pooling_weight = TIDL_INTERNAL_POOLING_WEIGHT_Q_U16;
        }
        if(pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.poolParams.kernelW != 0 && pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.poolParams.kernelH != 0)
        {
          accScale = ((float32_tidl)(((uint32_t)1) << internal_pooling_weight)) * indata->tensorScale;
          pOrgTIDLNetStructure->TIDLPCLayers[i].outData[0].roundBits  = internal_pooling_weight;
          pOrgTIDLNetStructure->TIDLPCLayers[i].outData[0].tensorScale = indata->tensorScale;
        }
      }
    }
    /* In case of DataConvertLayer mapped from quantized layer of TFL asymQuant populate the max possible bits for roundBits (used in shift) */
    else if( (pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_DataConvertLayer) && (gParams.quantizationStyle == TIDL_QuantStyleAsymNP2_TFL) )
    {
      if((pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.dataConvertParams.type == TIDL_DC_TYPE_INTERMEDIATE) &&
        (pOrgTIDLNetStructure->TIDLPCLayers[i].outData[0].roundBits == -1))
        {
          /* This piece of code is for making the intermediate DC layer act as a bypass layer */
          pOrgTIDLNetStructure->TIDLPCLayers[i].outData[0].roundBits = 0;
          const sTIDL_DataParams_t * indata = TIDL_getOutData(pOrgTIDLNetStructure,
          pOrgTIDLNetStructure->TIDLPCLayers[i].inData[0].dataId);
          if(indata == NULL)
          {
            TIDL_GLOBAL_REPORT_ERROR("Cannot find Producer");
            return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
          }
          pOrgTIDLNetStructure->TIDLPCLayers[i].outData[0].tensorScale    = indata->tensorScale;
          pOrgTIDLNetStructure->TIDLPCLayers[i].outData[0].tensorZeroPoint   = indata->tensorZeroPoint;
        }
      else
      {
        /* find the maxP2 number of shifts possible for scale to avoid overflow*/
        float maxP2 = ((pOrgTIDLNetStructure->TIDLPCLayers[i].inData[0].tensorScale) * (1 << TIDL_INTERNAL_INDATA_Q)) /
                          (pOrgTIDLNetStructure->TIDLPCLayers[i].outData[0].tensorScale) ;
        maxP2 = (int32_t)round(log((float64_tidl)maxP2) / log((float64_tidl)2));;

        /* In case of inData/outData in float, the scale and bias is used in float32 mode , roundBits is not used,
           Remaining cases we need to check bias is overflowing*/
        if ( (pOrgTIDLNetStructure->TIDLPCLayers[i].inData[0].elementType != TIDL_SinglePrecFloat) && \
             (pOrgTIDLNetStructure->TIDLPCLayers[i].outData[0].elementType != TIDL_SinglePrecFloat))
        {
          /* bias = out_zf - (in_zf * (out_scale/in_scale))  - scale is reciprocal in case of TIDL_QuantStyleAsymNP2_TFL */
          float bias  =  pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.dataConvertParams.outZeroPoint - \
                            (pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.dataConvertParams.inZeroPoint * \
                            ((pOrgTIDLNetStructure->TIDLPCLayers[i].inData[0].tensorScale)  /(pOrgTIDLNetStructure->TIDLPCLayers[i].outData[0].tensorScale)));
          /* Type of q1 is of type int8_t in split bias case*/
          int64_t q1AbsMax,q2AbsMax;
          q1AbsMax = std::numeric_limits<int8_t>::max(); // 127
          /* Type of q2 is determined by source */
          if (pOrgTIDLNetStructure->TIDLPCLayers[i].inData[0].elementType == TIDL_SignedChar)
          {
            q2AbsMax = -1 * std::numeric_limits<int8_t>::min();
          }
          else
          {
            q2AbsMax = std::numeric_limits<uint8_t>::max();
          }
          int64_t biasAbsMax = (int64_t)(bias * (1 << (int32_t)maxP2));
          while(biasAbsMax > (q2AbsMax * q1AbsMax) ){
            maxP2--;
            biasAbsMax = bias * (1 << (int32_t)maxP2);
          }
        }
        if(maxP2 >= 0)
        {
          pOrgTIDLNetStructure->TIDLPCLayers[i].outData[0].roundBits     = maxP2;
        }
        else
        {
          pOrgTIDLNetStructure->TIDLPCLayers[i].outData[0].roundBits     = 0;
        }
        /* In case of DataconvertLayer from dequantize set the outData.tensorScale  to 1 */
        if(pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.dataConvertParams.type ==  TIDL_DC_TYPE_OUTPUT)
        {
          if( ((pOrgTIDLNetStructure->TIDLPCLayers[i].inData[0].elementType == TIDL_UnsignedChar) || (pOrgTIDLNetStructure->TIDLPCLayers[i].inData[0].elementType ==  TIDL_SignedChar)) \
          && (pOrgTIDLNetStructure->TIDLPCLayers[i].outData[0].elementType ==  TIDL_SinglePrecFloat))
            pOrgTIDLNetStructure->TIDLPCLayers[i].outData[0].tensorScale = 1;
        }
      }
    }
  }
  fclose(paramDebugFile);

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

int8_t TIDL_isLayerParamsToBeUpdated(int32_t layerType)
{
  if  ((layerType == TIDL_ConvolutionLayer)     ||
        (layerType == TIDL_InnerProductLayer)   ||
        (layerType == TIDL_DeformableConvLayer) ||
        (layerType == TIDL_Deconv2DLayer)       ||
        (layerType == TIDL_BatchNormLayer)      ||
        (layerType == TIDL_ConstDataLayer))
  {
    return 1;
  }

  return 0;
}

bool TIDL_allConsumerSliceLayers(sTIDL_OrgNetwork_t *pOrgTIDLNetStructure, int32_t numLayers, sTIDL_LayerPC_t *currLayer, std::pair<float32_tidl, float32_tidl> *range) {
  if(currLayer->layerType == TIDL_SliceLayer) {
    range->first = std::min(range->first, currLayer->outData[0].minTensorValue);
    range->second = std::max(range->second, currLayer->outData[0].maxTensorValue);
    return true;
  }
  if(!(currLayer->layerType == TIDL_ReshapeLayer ||
    currLayer->layerType == TIDL_TransposeLayer ||
    (currLayer->layerType == TIDL_DataConvertLayer && currLayer->layerParams.dataConvertParams.type != TIDL_DC_TYPE_OUTPUT) ||
    currLayer->layerType == TIDL_CropLayer)
  ) {
    return false;
  }

  std::vector<int32_t> consumers = {};
  for(int32_t j = 0; j < currLayer->numOutBufs; j++) {
    std::vector<int32_t> _consumers = tidl_getOutLayers(*pOrgTIDLNetStructure, numLayers, currLayer->outData[j].dataId);
    consumers.insert(consumers.end(), _consumers.begin(), _consumers.end());
  }

  bool areAllSliceLayers = true;
  for(int32_t &consumerLayerIdx: consumers) {
    sTIDL_LayerPC_t *consumerLayer = &pOrgTIDLNetStructure->TIDLPCLayers[consumerLayerIdx];
    areAllSliceLayers = areAllSliceLayers && TIDL_allConsumerSliceLayers(pOrgTIDLNetStructure, numLayers, consumerLayer, range);
    if(!areAllSliceLayers) {
      break;
    }
  }

  return areAllSliceLayers;
}

int32_t TIDL_updateAsymRangeConstraints(sTIDL_OrgNetwork_t *pOrgTIDLNetStructure, int32_t numLayers, tidl_import_config *params) {
  /* Layer Specific optimizations */
  /*Traverse inputs and find the first consumer which satisfy topK source constraints:*/
  for(int32_t i = 0; i < numLayers; i++) {
    sTIDL_LayerPC_t *currLayer = &pOrgTIDLNetStructure->TIDLPCLayers[i];
    if(currLayer->layerType == TIDL_TopKLayer) {
      int32_t gridLayerIdx = tidl_getInLayer(*pOrgTIDLNetStructure, numLayers, currLayer->inData[0].dataId);
      sTIDL_LayerPC_t *gridLayer = &pOrgTIDLNetStructure->TIDLPCLayers[gridLayerIdx];
      int32_t traverseCount = 0;
      while (gridLayerIdx != -1 && TIDL_isComputeLayer(gridLayer, params) == 0 && TIDL_isGridProducerInputLayer(gridLayer, params) == 0)
      {
        gridLayerIdx = gridLayer->inData[0].dataId;
        gridLayerIdx = tidl_getInLayer(*pOrgTIDLNetStructure, numLayers, gridLayerIdx);
        gridLayer = &pOrgTIDLNetStructure->TIDLPCLayers[gridLayerIdx];
        traverseCount++;
        if (traverseCount > TIDL_MAX_GRID_TRAVERSAL)
        {
          TIDL_GLOBAL_REPORT_ERROR("Infinite loop in grid layer traversal");
          return -1;
        }
      }

      if (gridLayerIdx == -1)
      {
        TIDL_GLOBAL_REPORT_ERROR("Unable to find grid inputs for gridsample layer : %s", currLayer->outDataNames[0]);
        return -1;
      }
      else if (gridLayer->numOutBufs == 1 && gridLayer->actParams.actType == TIDL_NoAct)
      {
        /*Update the Grid Layer with TopK range */
        gridLayer->actParams.actType = TIDL_Clip;
        float32_tidl minVal = currLayer->outData[0].minTensorValue;
        float32_tidl maxVal = currLayer->outData[0].maxTensorValue;

        if(minVal > 0) {
          minVal /= 1.025;
        }
        else {
          minVal *= 1.025; 
        } 

        if(maxVal > 0) {
          maxVal *= 1.025;
        }
        else {
          maxVal /= 1.025; 
        }

        gridLayer->actParams.clipMin = minVal;
        gridLayer->actParams.clipMax = maxVal;
        gridLayer->outData[0].maxTensorValue = maxVal;
        gridLayer->outData[0].minTensorValue = minVal;

        gridLayerIdx = tidl_getInLayer(*pOrgTIDLNetStructure, numLayers, currLayer->inData[0].dataId);
        gridLayer = &pOrgTIDLNetStructure->TIDLPCLayers[gridLayerIdx];
        while (gridLayerIdx != -1 && TIDL_isComputeLayer(gridLayer, params) == 0 && TIDL_isGridProducerInputLayer(gridLayer, params) == 0)
        {
          gridLayer->outData[0].minTensorValue = minVal;
          gridLayer->outData[0].maxTensorValue = maxVal;
          gridLayerIdx = gridLayer->inData[0].dataId;
          gridLayerIdx = tidl_getInLayer(*pOrgTIDLNetStructure, numLayers, gridLayerIdx);
          gridLayer = &pOrgTIDLNetStructure->TIDLPCLayers[gridLayerIdx];
        }
      }
    }
  }
  
  /* Generic Range Based Optimizations */
  for(int32_t i = 0; i < numLayers; i++) {
    sTIDL_LayerPC_t *currLayer = &pOrgTIDLNetStructure->TIDLPCLayers[i];
    if(currLayer->layerType == TIDL_ScatterElementsLayer || currLayer->layerType == TIDL_TopKLayer) {
      continue;
    }
    std::vector<int32_t> consumers = {};
    for(int32_t j = 0; j < currLayer->numOutBufs; j++) {
      std::vector<int32_t> _consumers = tidl_getOutLayers(*pOrgTIDLNetStructure, numLayers, currLayer->outData[j].dataId);
      consumers.insert(consumers.end(), _consumers.begin(), _consumers.end());
    }

    std::pair<float32_tidl, float32_tidl> outTensorRange = {FLT_MAX, FLT_MIN};
    bool areAllSliceLayers = true;
    for(int32_t &consumerLayerIdx: consumers) {
      sTIDL_LayerPC_t *consumerLayer = &pOrgTIDLNetStructure->TIDLPCLayers[consumerLayerIdx];
      areAllSliceLayers = areAllSliceLayers && TIDL_allConsumerSliceLayers(pOrgTIDLNetStructure, numLayers, consumerLayer, &outTensorRange);
      if(!areAllSliceLayers) {
        break;
      }
    }

    if(areAllSliceLayers) {
      currLayer->outData[0].minTensorValue = outTensorRange.first;
      currLayer->outData[0].maxTensorValue = outTensorRange.second;
      if(currLayer->actParams.actType == TIDL_Clip) {
        currLayer->actParams.clipMin = TIDL_clamp(currLayer->actParams.clipMin, outTensorRange.first, outTensorRange.second);
        currLayer->actParams.clipMax = TIDL_clamp(currLayer->actParams.clipMax, outTensorRange.first, outTensorRange.second);
      }
      else if(currLayer->actParams.actType == TIDL_NoAct) {
        currLayer->actParams.actType = TIDL_Clip;
        currLayer->actParams.clipMin = outTensorRange.first;
        currLayer->actParams.clipMax = outTensorRange.second;
      } 
    }
  }
  return 0;
}

/*Why is pTIDLNetStructure being passed here?*/
int32_t TIDL_updateParamsRange(sTIDL_OrgNetwork_t   * pOrgTIDLNetStructure,
                            sTIDL_Network_t        *pTIDLNetStructure,
                            int32_t layerIndex)
{
  int32_t i,j;
  int32_t numBins = TIDL_NUM_WEIGHT_HISTOGRAM_BINS;

  int32_t extQuantPrms = TIDL_canBypassCalibration(pOrgTIDLNetStructure, &gParams);

  for (i = 0; i < layerIndex; i++)
  {
    for (j = 0; j < TIDL_MAX_QUANT_PARAMS; j++)
    {
      pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[j].size = 0;
    }

    if  (TIDL_isLayerParamsToBeUpdated(pOrgTIDLNetStructure->TIDLPCLayers[i].layerType))
    {
      if ((pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_ConvolutionLayer) ||
          (pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_Deconv2DLayer))
      {
        pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[TIDL_BIAS_QUANT_PARAMS].scalePtr =
                                        &pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.convParams.biasScale;
        pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[TIDL_WEIGHT_QUANT_PARAMS].scalePtr =
                                        &pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.convParams.weightScale;
      }
      else if (pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_DeformableConvLayer)
      {
        pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[TIDL_BIAS_QUANT_PARAMS].scalePtr =
                                        &pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.deformConvParams.biasScale;
        pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[TIDL_WEIGHT_QUANT_PARAMS].scalePtr =
                                        &pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.deformConvParams.weightScale;      
      }
      else if (pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_InnerProductLayer)
      {
        pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[TIDL_BIAS_QUANT_PARAMS].scalePtr =
                                        &pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.innerProductParams.biasScale;
        pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[TIDL_WEIGHT_QUANT_PARAMS].scalePtr =
                                        &pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.innerProductParams.weightScale;
      }
      else if (pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_BatchNormLayer)
      {
        pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[TIDL_BIAS_QUANT_PARAMS].scalePtr =
                                        &pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.batchNormParams.biasScale;
        pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[TIDL_WEIGHT_QUANT_PARAMS].scalePtr =
                                        &pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.batchNormParams.weightScale;
        pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[TIDL_SLOPE_QUANT_PARAMS].scalePtr =
                                        &pOrgTIDLNetStructure->TIDLPCLayers[i].actParams.slopeScale;
      }
      else if (pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_ConstDataLayer)
      {
        pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[TIDL_WEIGHT_QUANT_PARAMS].scalePtr =
                                        &pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.constDataParams.weightScale;
      }
      pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[TIDL_BIAS_QUANT_PARAMS].size   = pOrgTIDLNetStructure->TIDLPCLayers[i].bias.bufSize;
      pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[TIDL_WEIGHT_QUANT_PARAMS].size = pOrgTIDLNetStructure->TIDLPCLayers[i].weights.bufSize;
      pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[TIDL_SLOPE_QUANT_PARAMS].size   = pOrgTIDLNetStructure->TIDLPCLayers[i].slope.bufSize;
      if (pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_BatchNormLayer && pOrgTIDLNetStructure->TIDLPCLayers[i].actParams.actType == TIDL_LeakyReLU)
      {
        /*Prevent updates of slopeScale for LeakyReLU*/
        pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[TIDL_SLOPE_QUANT_PARAMS].size = 0;
      }
    }
  }
  void **data;
  float min;
  float max;


  for (i = 0; i < layerIndex; i++)
  {
    int32_t  weightsElementSizeInBits = pOrgTIDLNetStructure->TIDLPCLayers[i].weightsElementSizeInBits;
    for (j = 0; j < TIDL_MAX_QUANT_PARAMS; j++)
    {
      int32_t dataSize = pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[j].size;
      if (dataSize)
      {
        min = FLT_MAX;
        max = -FLT_MAX;
        if(j == TIDL_BIAS_QUANT_PARAMS)
        {
          data = (void **)&(pOrgTIDLNetStructure->TIDLPCLayers[i].bias.ptr);
        }
        else if(j == TIDL_WEIGHT_QUANT_PARAMS)
        {
          data = (void **)&(pOrgTIDLNetStructure->TIDLPCLayers[i].weights.ptr);
        }
        else if(j == TIDL_SLOPE_QUANT_PARAMS)
        {
          data = (void **)&(pOrgTIDLNetStructure->TIDLPCLayers[i].slope.ptr);
        }
        TIDL_findRange((float*)(*data), dataSize, &min, &max, 1.0);
        /* Check for updating Indices for gather layer. Find range with int32_t casted pointer instead of float */
        if(pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_ConstDataLayer)
        {
          int32_t outIdx = tidl_getOutLayer(pOrgTIDLNetStructure, pOrgTIDLNetStructure->numLayers, pOrgTIDLNetStructure->TIDLPCLayers[i].outData[0].dataId);
          if(outIdx != -1 && pOrgTIDLNetStructure->TIDLPCLayers[outIdx].layerType == TIDL_GatherLayer)
          {
            min = FLT_MAX, max = -FLT_MAX;
            TIDL_findRange((int32_t*)(*data), dataSize, &min, &max, 1.0);
          }
        }
        if(j == TIDL_WEIGHT_QUANT_PARAMS)
        {
          if (( pTIDLNetStructure->calibrationOption & TIDL_CalibOptionWeightRange ) == TIDL_CalibOptionWeightRange)
          {
            if ( pTIDLNetStructure->calibrationParams.weightRangeMethod == TIDL_WeightRangeMethodHistogram )
            {
              TIDL_IMPORT_CHECK_AND_RETURN(TIDL_findRangeHist((float*)(*data), dataSize, numBins, pTIDLNetStructure->calibrationParams.percentileWtRangeShrink, &min, &max), "");
            }
            if ( pTIDLNetStructure->calibrationParams.weightRangeMethod == TIDL_WeightRangeMethodMedian)
            {
              TIDL_IMPORT_CHECK_AND_RETURN(TIDL_findRangeUsingMedian((float*)(*data), dataSize, weightsElementSizeInBits, &min, &max), "");
            }
          }
        }
        pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[j].prmPtr = data;
        *(pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[j].scalePtr) = TIDL_GetMaxQuantScale(min, max, weightsElementSizeInBits);

        if(pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_ConstDataLayer && j == TIDL_WEIGHT_QUANT_PARAMS)
        {
          if(pOrgTIDLNetStructure->TIDLPCLayers[i].quantizeConstraint != TIDL_QuantizeUnconstrained)
          {
            /*Scale is determined via a constraint in this case, updating the scale pointer to a pre-determined value*/
            *(pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[j].scalePtr) = pOrgTIDLNetStructure->TIDLPCLayers[i].outData->tensorScale;
          }
          else
          {
            pOrgTIDLNetStructure->TIDLPCLayers[i].outData->tensorScale =  *(pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[j].scalePtr);
          }
        }

        if ((j == TIDL_WEIGHT_QUANT_PARAMS) &&
            (pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_BatchNormLayer) &&
            ((pOrgTIDLNetStructure->TIDLPCLayers[i].actParams.actType == TIDL_Sigmoid)||(pOrgTIDLNetStructure->TIDLPCLayers[i].actParams.actType == TIDL_Tanh)||(pOrgTIDLNetStructure->TIDLPCLayers[i].actParams.actType == TIDL_HardSigmoid)||(pOrgTIDLNetStructure->TIDLPCLayers[i].actParams.actType == TIDL_ELU)))
        {
          /* The default value of weightsElementSizeInBits is set during initially which is set based on numParamBits as given by the user. For mixed precision
          the default value is updated based on whether a particular layer is running at higher precision or not. Hence at this point we should read the updated
          value of weightsElementSizeInBits as decided based on the precision of the layer*/
         *(pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[j].scalePtr) = (1.0*((1 << (pOrgTIDLNetStructure->TIDLPCLayers[i].weightsElementSizeInBits -1))));
        }


        if(*(pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[j].scalePtr) == -1)
        {
          if(j == TIDL_BIAS_QUANT_PARAMS)
          {
            *(pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[j].scalePtr) = FLT_MAX;
          }
          else
          {
            *(pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[j].scalePtr) = 1;
          }
        }
        if( extQuantPrms != 1)
        {
          pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[j].max = max;
          pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[j].min = min;
        }
      }
    }
  }

  if(gParams.quantizationStyle == TIDL_QuantStyleAsymNP2 && gParams.enableTFROptimization == 1)
  {
    for (i = 0; i < layerIndex; i++)
    {
      sTIDL_LayerPC_t& layer = pOrgTIDLNetStructure->TIDLPCLayers[i];
      if (floatRangeMap.find(i) != floatRangeMap.end())
      {
        layer.outData[0].minTensorValue = floatRangeMap[i].first;
        layer.outData[0].maxTensorValue = floatRangeMap[i].second;
      }
      else
      {
        floatRangeMap[i] = {layer.outData[0].minTensorValue, layer.outData[0].maxTensorValue};
        if(layer.actParams.actType == TIDL_Clip)
        {
          floatRangeMap[i] = {layer.actParams.clipMin, layer.actParams.clipMax};
        }
      }
    }
  }
  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

int32_t TIDL_QuantPerChannelWeight(sTIDL_OrgNetwork_t   * pOrgTIDLNetStructure,
                                            sTIDL_Network_t        *pTIDLNetStructure,
                                            tidl_import_config       *configParams,
                                            int32_t i)
{
    float min_clipped = FLT_MAX;
    float max_clipped = -FLT_MAX;
    float maxWeightScale = FLT_MAX;
    float min = FLT_MAX;
    float max = -FLT_MAX;
    int32_t weightsElementSizeInBits = pOrgTIDLNetStructure->TIDLPCLayers[i].weightsElementSizeInBits;
    int32_t numInChannels = pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.convParams.numInChannels;
    int32_t numOutChannels = pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.convParams.numOutChannels;

    float outTensorScale;
    int32_t isOutMaxSat = isOutputTensorMaxSatAvailable(&pOrgTIDLNetStructure->TIDLPCLayers[i], &outTensorScale);
    const sTIDL_DataParams_t * indata = TIDL_getOutData(pOrgTIDLNetStructure,
                  pOrgTIDLNetStructure->TIDLPCLayers[i].inData[0].dataId);
    if(indata == NULL)
    {
      TIDL_GLOBAL_REPORT_ERROR("Cannot find Producer");
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }

    if(pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[TIDL_BIAS_QUANT_PARAMS].size > 0)
    {
        min = pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[TIDL_BIAS_QUANT_PARAMS].min;
        max = pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[TIDL_BIAS_QUANT_PARAMS].max;
        float maxBiasScale = TIDL_findMaxQuantizationScale(min, max, weightsElementSizeInBits*2, TIDL_getDatElementSign(indata->elementType));
        maxWeightScale = maxBiasScale / indata->tensorScale;
    }
    float32_tidl * data = (float32_tidl *)pOrgTIDLNetStructure->TIDLPCLayers[i].weights.ptr;
    int32_t dataSize = pOrgTIDLNetStructure->TIDLPCLayers[i].weights.bufSize;
    min = FLT_MAX;
    max = -FLT_MAX;
    TIDL_findRange(data, dataSize, &min, &max, 1.0);
    min_clipped = min;
    max_clipped = max;
    /* dataSizePerChannel is the number of weights per channel */
    uint32_t dataSizePerChannel = (dataSize / numInChannels);
    if(dataSizePerChannel > 0)
    {
      int i1;
      float32_tidl * perChannelWeightScale = (float32_tidl *)my_malloc(numInChannels * sizeof(float32_tidl));
      uint8_t * quantizedParams = (uint8_t *)my_malloc(dataSize * sizeof(float32_tidl)); /* allocate 32 bit memory to ensure memory doesn't have to be allocated in each iteration of bias calibration */
      for(i1 = 0; i1 < numOutChannels; i1++)
      {
        min = FLT_MAX;
        max = -FLT_MAX;
        TIDL_findRange(&data[i1*dataSizePerChannel], dataSizePerChannel, &min, &max, 1.0);
        min = ( min < min_clipped ) ? min_clipped : min;
        max = ( max > max_clipped ) ? max_clipped : max;
        if (weightsElementSizeInBits <= 8)
        {
          perChannelWeightScale[i1] = TIDL_QuantizeSignedMax((int8_t *)&quantizedParams[i1*dataSizePerChannel*((weightsElementSizeInBits - 1) / 8 + 1)], &data[i1*dataSizePerChannel], dataSizePerChannel, min, max, weightsElementSizeInBits, maxWeightScale, isOutMaxSat, indata->tensorScale, outTensorScale);
        }
        else // weightsElementSizeInBits == 2
        {
          perChannelWeightScale[i1] = TIDL_QuantizeSignedMax((int16_t *)&quantizedParams[i1*dataSizePerChannel*((weightsElementSizeInBits - 1) / 8 + 1)], &data[i1*dataSizePerChannel], dataSizePerChannel, min, max, weightsElementSizeInBits, maxWeightScale, isOutMaxSat, indata->tensorScale, outTensorScale);
        }
      }
      if (perChannelWeightScale[i1] == -1)
      {
        /* weightScale = -1 means all weights are very small and set to 0. Given all weights are 0, scale forced to 1
        in order to prevent it from blowing up thereby ensuring tensorScale doesn't get reduced to prevent bias saturation */
        perChannelWeightScale[i1] = 1;
      }
      pOrgTIDLNetStructure->TIDLPCLayers[i].perChannelWeightScale.ptr = perChannelWeightScale;
      pOrgTIDLNetStructure->TIDLPCLayers[i].perChannelWeightScale.bufSize = numInChannels;
      pOrgTIDLNetStructure->TIDLPCLayers[i].weights.ptr = quantizedParams;
      my_free(data);
    }

    return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

float32_tidl TIDL_getUpperBoundMMAv2_ScaleShiftAndError(float scaleRatio, uint8_t *scale, uint8_t *shift, int32_t weightBits, float32_tidl maxScaleMMAv2)
{
  int32_t shiftBits;
  float32_tidl curError = 0;
  float32_tidl approxScale = 0;
  float32_tidl minError = FLT_MAX;
  int32_t bestShiftBits;
  int32_t bestFixedScale;
  int32_t maxShiftBits = 40;
  if(weightBits > 8)
  {
    maxShiftBits = 64U;
  }
  int32_t scaleIter;
  for(scaleIter = 1U; scaleIter <= 255U; scaleIter++)
  {
    /*Loop over all possible mmaScale values*/
    /* scaleRatio ~= mmaScale/2^mmaShift*/
    /* mmaShift = ln(mmaScale/scaleRatio)*/
    shiftBits = (int32_t)round(log(((double)scaleIter)/scaleRatio)/log((double)2));
    if(shiftBits > maxShiftBits)
    {
      shiftBits = maxShiftBits;
    }
    if(shiftBits < 0)
    {
      shiftBits = 0U;
    }
    approxScale = (float32_tidl)(((float32_tidl)scaleIter)/(pow(2,shiftBits)));
    if (approxScale > scaleRatio)
    {
      curError = abs(scaleRatio - approxScale);
      if(curError < minError)
      {
        minError = curError;
        bestShiftBits = shiftBits;
        bestFixedScale = scaleIter;
      }
    }
  }
  *shift = bestShiftBits;
  *scale = bestFixedScale;
  return minError;
}



void TIDL_getMMAv2_ScaleAndShift(float scaleRatio, uint8_t *scale, uint8_t *shift, int32_t weightBits, float32_tidl maxScaleMMAv2)
{
  int32_t shiftBits = 0;
  float newScaleRatio = scaleRatio;
  int32_t maxShiftBits = 40U; /*HW Capability*/

  if(weightBits > 8)
  {
    maxShiftBits = 64U; /*HW Capability*/
  }

  if(scaleRatio > TIDL_MMAV2_MAX_SCALE)
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "TIDL_getMMAv2_ScaleAndShift: ScaleRatio exceeds representation capability");
  }
  //Since exponent goes only in one direction, repeated multiplication with 2 is performed till it exceeds the max range.
  while(1)
  {
    newScaleRatio *= 2;
    if(shiftBits >= maxShiftBits )
    {
      break; //Max capability of a right shift of 40 for 8-bit.
    }
    else if(newScaleRatio > (maxScaleMMAv2))
    {
      newScaleRatio /= 2;
      break;
    }
    shiftBits++;
  }
  *shift = shiftBits;
  /*Round and store scale*/
  *scale = (uint8_t) round(newScaleRatio);
}

template <class Tout>
void TIDL_QuantizeFloatToFixed(Tout *quantizedBuffer,
                               float32_tidl *buffer,
                               int32_t bufferSize,
                               int32_t bufferPitch,
                               float32_tidl bufferScale,
                               int32_t zeroPoint)
{
  /*Straightforward quantization of a buffer based on :
    Xfloat = (Xfixed - zp)/scale
    Xfixed = Xfloat * scale + zp*/
  int32_t i0;
  float32_tidl intermediateValue = 0;
  float32_tidl max = std::numeric_limits<Tout>::max();
  float32_tidl min = std::numeric_limits<Tout>::min();
  for(i0 = 0; i0 < bufferSize; i0++)
  {
    intermediateValue = round((buffer[i0*bufferPitch]*bufferScale) + zeroPoint);
    //Saturate upwards:
    intermediateValue = intermediateValue > max ? max : intermediateValue;
    //Saturate downwards:
    intermediateValue = intermediateValue < min ? min : intermediateValue;
    quantizedBuffer[i0*bufferPitch] = (Tout) intermediateValue;
  }
}

int32_t TIDL_calculateBitsRequired(int64_t quantizedValue)
{
  int32_t numBits = 0;
  numBits = 64;
  while(quantizedValue < (1.0*(pow(2,numBits)) - 1))
  {
    numBits--;
  }
  return (numBits + 1U);
}

template <class Tweights>
int32_t TIDL_calculateAsymmetricAbsBiasTermBits(sTIDL_OrgNetwork_t * pOrgTIDLNetStructure, Tweights* weightsPtr, int32_t layerIdx, int32_t outIdx, int32_t weightIdx = 0, float inScale = 1.0f, float outScale = 1.0f)
{
  int32_t numBits;
  if(pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerType == TIDL_ConvolutionLayer)
  {
    Tweights* pWeights = weightsPtr;
    int32_t numGroups = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.convParams.numGroups;
    int32_t numInChannels = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.convParams.numInChannels;
    int32_t numOutChannels = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.convParams.numOutChannels;
    float32_tidl *pWeightScales = (float32_tidl*)pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].weightScales.ptr;
    float32_tidl *pBiasOriginal = (float32_tidl*)pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].bias.ptr;
    int32_t inLayerIdx = tidl_getInLayer(*pOrgTIDLNetStructure, pOrgTIDLNetStructure->numLayers, pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].inData[0].dataId);
    int32_t z_x = pOrgTIDLNetStructure->TIDLPCLayers[inLayerIdx].outData[0].tensorZeroPoint;
    int32_t z_y = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].outData[0].tensorZeroPoint;
    double scale_x = pOrgTIDLNetStructure->TIDLPCLayers[inLayerIdx].outData[0].tensorScale;
    double scale_y = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].outData[0].tensorScale;
    int32_t numInWeights = (numInChannels/numGroups)*(pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.convParams.kernelH * pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.convParams.kernelW);
    double scale_w = 0;
    double unitConvResult = 0;
    double nScale = 1;
    double originalBias = 0;
    double finalBias = 0;
    double termB = 0;
    double termC = 0;
    double termD = 0;
    int32_t wtIdx;
    scale_w = pWeightScales[outIdx];
    for(wtIdx = 0; wtIdx < numInWeights ; wtIdx++)
    {
      unitConvResult += pWeights[wtIdx + outIdx*numInWeights];
    }
    originalBias = pBiasOriginal[outIdx];
    nScale = (scale_x * scale_w) / scale_y;
    termB = (originalBias * scale_x * scale_w);
    termC = (z_y * nScale);
    termD = (z_x * unitConvResult);
    finalBias = termB + (termC - termD);
    double absBias = finalBias < 0 ? (finalBias*-1) : finalBias;
    numBits = 63;
    while(absBias < (1.0*(pow(2,numBits)) - 1))
    {
      numBits--;
    }
    numBits++;
    if(numBits == 64)
    {
      TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "B=%f C=%f D=%f",termB, termC, termD);
      TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "ScaleX = %f | scaleW = %f | scaleW = %f",scale_x,scale_w,scale_w);
    }
    return numBits;
  }
  else if(pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerType == TIDL_InnerProductLayer)
  {
    Tweights* pWeights = weightsPtr;
    int32_t numInCols = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.innerProductParams.numInCols;
    int32_t numOutCols = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.innerProductParams.numOutCols;
    float32_tidl *pWeightScales = (float32_tidl*)pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].weightScales.ptr;
    float32_tidl *pBiasOriginal = (float32_tidl*)pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].bias.ptr;
    int32_t inLayerIdx = tidl_getInLayer(*pOrgTIDLNetStructure, pOrgTIDLNetStructure->numLayers, pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].inData[0].dataId);
    int32_t z_x = pOrgTIDLNetStructure->TIDLPCLayers[inLayerIdx].outData[0].tensorZeroPoint;
    int32_t z_y = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].outData[0].tensorZeroPoint;
    double scale_x = inScale;
    double scale_y = outScale;
    int32_t numInWeights = numInCols;
    double scale_w = 0;
    double unitConvResult = 0;
    double nScale = 1;
    double originalBias = 0;
    double finalBias = 0;
    double termB = 0;
    double termC = 0;
    double termD = 0;
    int32_t wtIdx;
    scale_w = pWeightScales[outIdx];
    for(wtIdx = 0; wtIdx < numInWeights ; wtIdx++)
    {
      unitConvResult += pWeights[wtIdx * numOutCols + weightIdx];
    }
    originalBias = pBiasOriginal[outIdx];
    nScale = (scale_x * scale_w) / scale_y;
    termB = (originalBias * scale_x * scale_w);
    termC = (z_y * nScale);
    termD = (z_x * unitConvResult);
    finalBias = termB + (termC - termD);
    double absBias = finalBias < 0 ? (finalBias*-1) : finalBias;
    numBits = 63;
    while(absBias < (1.0*(pow(2,numBits)) - 1))
    {
      numBits--;
    }
    numBits++;
    if(numBits == 64)
    {
      TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "B=%f C=%f D=%f",termB, termC, termD);
      TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "ScaleX = %f | scaleW = %f | scaleW = %f",scale_x,scale_w,scale_w);
    }
  }
  return numBits;
}


template <class Tbias, class Tweights>
int32_t TIDL_asymDeriveBiasTerm(Tbias *derivedBiasPtr, Tweights* weightsPtr, sTIDL_OrgNetwork_t * pOrgTIDLNetStructure, int32_t layerIdx, int32_t weightsElementSizeInBits, float32_tidl maxScaleMMAv2)
{
  if(pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerType == TIDL_ConvolutionLayer)
  {
    /*Convolution specific*/
    int32_t outIdx;
    int32_t wtIdx;
    Tweights* pWeights = weightsPtr;
    int32_t numGroups = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.convParams.numGroups;
    int32_t numInChannels = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.convParams.numInChannels;
    int32_t numOutChannels = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.convParams.numOutChannels;
    float32_tidl *pWeightScales = (float32_tidl*)pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].weightScales.ptr;
    float32_tidl *pBiasOriginal = (float32_tidl*)pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].bias.ptr;
    uint8_t *derivedScales = (uint8_t *)pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].derivedScales.ptr;
    uint8_t *derivedShifts = (uint8_t *)pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].derivedShifts.ptr;
    double unitConvResult = 0;
    int32_t numInWeights = (numInChannels/numGroups)*(pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.convParams.kernelH * pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.convParams.kernelW);
    const sTIDL_DataParams_t * inData = TIDL_getOutData(pOrgTIDLNetStructure, pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].inData[0].dataId);
    if(inData == NULL)
    {
      TIDL_GLOBAL_REPORT_ERROR("Cannot find Producer");
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }
    int32_t z_y = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].outData[0].tensorZeroPoint;
    double scale_x = inData->tensorScale;
    int32_t z_x = inData->tensorZeroPoint;
    double scale_y = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].outData[0].tensorScale;
    double scale_w = 1;
    double nScale = 1;
    double scaleRatio = 1;
    double finalBias = 0;
    double originalBias = 0;

    for(outIdx = 0; outIdx < numOutChannels; outIdx++)
    {
      unitConvResult = 0;
      scale_w = pWeightScales[outIdx];
      for(wtIdx = 0; wtIdx < numInWeights ; wtIdx++)
      {
        unitConvResult += pWeights[wtIdx + outIdx*numInWeights];
      }
      originalBias = pBiasOriginal[outIdx];
      nScale = (scale_x * scale_w) / scale_y;
      finalBias = (originalBias * scale_x * scale_w) + ((z_y * nScale) - (z_x * unitConvResult));
      double absBias = finalBias < 0 ? (finalBias*-1) : finalBias;
      if( (absBias > ((double)std::numeric_limits<Tbias>::max())) /*|| (((absBias * derivedScales[outIdx]) > ((double)std::numeric_limits<Tbias>::max())) && (weightsElementSizeInBits > 8))*/)
      {
        //TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Readjusting bias values!");
        //Saturation case, i.e. filter and input are irrelevant:
        //Zero weights out: (Consequence of a huge bias is that your output is more or less going to be a DC term)
        for(wtIdx = 0; wtIdx < numInWeights ; wtIdx++)
        {
          pWeights[wtIdx + outIdx*numInWeights] = 0;
        }
        unitConvResult = 0; /*As weights are now zero*/
        //Default weight scale to 1 and recalc bias:
        //Bias in output domain:
        scale_w = 1.0;
        while((scale_y/(scale_w * scale_x)) > TIDL_MMAV2_MAX_SCALE)
        {
          scale_w *= 10.0;
        }
        pWeightScales[outIdx] = scale_w;
        //Switching to original domain with new scale:
        nScale = (scale_x * scale_w) / scale_y;
        TIDL_getMMAv2_ScaleShiftAndError((1/nScale), &derivedScales[outIdx], &derivedShifts[outIdx], weightsElementSizeInBits, maxScaleMMAv2);
        finalBias = (originalBias * scale_x * scale_w)  + ((z_y * nScale) - (z_x * unitConvResult));
        derivedBiasPtr[outIdx] = round(finalBias);
      }
      else if((((absBias * derivedScales[outIdx] ) > ((double)std::numeric_limits<Tbias>::max())) && (weightsElementSizeInBits > 8)))
      {
        /*Reduce maxScaleMMAv2 till it satisfies the satuaration prevention condition:*/
        float32_tidl localMaxScale = maxScaleMMAv2;
        while((absBias * derivedScales[outIdx]) > ((double)std::numeric_limits<Tbias>::max()))
        {
          localMaxScale /= 2;
          scaleRatio = scale_y / (scale_x * scale_w);
          TIDL_getMMAv2_ScaleShiftAndError(scaleRatio, &derivedScales[outIdx], &derivedShifts[outIdx], weightsElementSizeInBits, localMaxScale);
          if(localMaxScale <= 0)
          {
            TIDL_GLOBAL_REPORT_ERROR("Error in computing bias scales, Aborting");
            return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
          }
        }
      }
      else
      {
        derivedBiasPtr[outIdx] = round(finalBias);
      }
    }
  }
  else if(pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerType == TIDL_InnerProductLayer)
  {
    /*FC specific*/
    int32_t outIdx;
    int32_t wtIdx;
    int32_t dataIdx = 0;
    Tweights* pWeights = weightsPtr;
    int32_t numInCols = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.innerProductParams.numInCols;
    int32_t numOutCols = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.innerProductParams.numOutCols;
    int32_t numBChannels = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.innerProductParams.numBChannels;
    int32_t weightChPitch = numInCols * numOutCols;
    float32_tidl *pWeightScales = (float32_tidl*)pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].weightScales.ptr;
    float32_tidl *pBiasOriginal = (float32_tidl*)pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].bias.ptr;
    uint8_t *derivedScales = (uint8_t *)pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].derivedScales.ptr;
    uint8_t *derivedShifts = (uint8_t *)pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].derivedShifts.ptr;
    double unitConvResult = 0;
    int32_t numInWeights = numInCols;
    const sTIDL_DataParams_t * inData = TIDL_getOutData(pOrgTIDLNetStructure, pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].inData[0].dataId);
    if(inData == NULL)
    {
      TIDL_GLOBAL_REPORT_ERROR("Cannot find Producer");
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }
    int32_t z_y = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].outData[0].tensorZeroPoint;
    double scale_x = inData->tensorScale;
    int32_t z_x = inData->tensorZeroPoint;
    double scale_y = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].outData[0].tensorScale;
    double scale_w = 1;
    double nScale = 1;
    double scaleRatio = 1;
    double finalBias = 0;
    double originalBias = 0;
    for(int32_t i1 = 0; i1 < numBChannels; i1++)
    {
      for(outIdx = 0; outIdx < numOutCols; outIdx++)
      {
        dataIdx = outIdx + (i1 * numOutCols);
        unitConvResult = 0;
        scale_w = pWeightScales[dataIdx];
        for(wtIdx = 0; wtIdx < numInWeights ; wtIdx++)
        {
          unitConvResult += pWeights[wtIdx*numOutCols + outIdx + (i1 * weightChPitch)];
        }
        originalBias = pBiasOriginal[dataIdx];
        nScale = (scale_x * scale_w) / scale_y;
        finalBias = (originalBias * scale_x * scale_w) + ((z_y * nScale) - (z_x * unitConvResult));
        double absBias = finalBias < 0 ? (finalBias*-1) : finalBias;
        if( (absBias > ((double)std::numeric_limits<Tbias>::max())) /*|| (((absBias * derivedScales[outIdx]) > ((double)std::numeric_limits<Tbias>::max())) && (weightsElementSizeInBits > 8))*/)
        {
          //TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Readjusting bias values!");
          //Saturation case, i.e. filter and input are irrelevant:
          //Zero weights out: (Consequence of a huge bias is that your output is more or less going to be a DC term)
          for(wtIdx = 0; wtIdx < numInWeights ; wtIdx++)
          {
            pWeights[wtIdx*numOutCols + outIdx + (i1 * weightChPitch)] = 0;
          }
          unitConvResult = 0; /*As weights are now zero*/
          //Default weight scale to 1 and recalc bias:
          //Bias in output domain:
          scale_w = 1.0;
          while((scale_y/(scale_w * scale_x)) > TIDL_MMAV2_MAX_SCALE)
          {
            scale_w *= 10.0;
          }
          pWeightScales[dataIdx] = scale_w;
          //Switching to original domain with new scale:
          nScale = (scale_x * scale_w) / scale_y;
          TIDL_getMMAv2_ScaleShiftAndError((1/nScale), &derivedScales[dataIdx], &derivedShifts[dataIdx], weightsElementSizeInBits, maxScaleMMAv2, true);
          finalBias = (originalBias * scale_x * scale_w)  + ((z_y * nScale) - (z_x * unitConvResult));
          derivedBiasPtr[dataIdx] = round(finalBias);
        }
        else if((((absBias * derivedScales[dataIdx] ) > ((double)std::numeric_limits<Tbias>::max())) && (weightsElementSizeInBits > 8)))
        {
          /*Reduce maxScaleMMAv2 till it satisfies the satuaration prevention condition:*/
          float32_tidl localMaxScale = maxScaleMMAv2;
          while((absBias * derivedScales[dataIdx]) > ((double)std::numeric_limits<Tbias>::max()))
          {
            localMaxScale /= 2;
            scaleRatio = scale_y / (scale_x * scale_w);
            TIDL_getMMAv2_ScaleShiftAndError(scaleRatio, &derivedScales[dataIdx], &derivedShifts[dataIdx], weightsElementSizeInBits, localMaxScale, true);
            if(localMaxScale <= 0)
            {
              TIDL_GLOBAL_REPORT_ERROR("Error in computing bias scales. Aborting");
              return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
            }
          }
        }
        else
        {
          derivedBiasPtr[dataIdx] = round(finalBias);
        }
      }
    }

  }

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

void TIDL_updateDataBufferScaleAndZeroPoint(sTIDL_OrgNetwork_t * pOrgTIDLNetStructure,
                                int32_t producerLayerIdx)
{
  /*Updates the tensor scale and zp*/
  int32_t layerIdx;
  sTIDL_LayerPC_t * currLayer;
  sTIDL_LayerPC_t * producerLayer;
  int32_t producerElemType;
  int32_t producerDataId;
  int32_t inIdx;

  producerLayer = &pOrgTIDLNetStructure->TIDLPCLayers[producerLayerIdx];
  producerElemType = producerLayer->outData[0].elementType;
  producerDataId   = producerLayer->outData[0].dataId;
  if ( producerLayer->numOutBufs > 0)
  {
    /* Go through all the layers */
    for ( layerIdx = 0; layerIdx < pOrgTIDLNetStructure->numLayers; layerIdx++)
    {
      currLayer = &pOrgTIDLNetStructure->TIDLPCLayers[layerIdx];
      /* Find layers whos input contains the data Id. This will indicate the producer
      layer's output goes to this layer */
      for (inIdx = 0; inIdx < currLayer->numInBufs; inIdx++)
      {
        /* Indicates one of the consumer layer is found */
        if ( producerDataId == currLayer->inData[inIdx].dataId)
        {
          /*Update element type:*/
          currLayer->inData[inIdx].tensorScale = producerLayer->outData[0].tensorScale;
          currLayer->inData[inIdx].tensorZeroPoint = producerLayer->outData[0].tensorZeroPoint;
          currLayer->inData[inIdx].elementType = producerLayer->outData[0].elementType;
        }
      }
    }
  }
}

double TIDL_asymGetScaleRatio(float32_tidl outTensorScale, float32_tidl inTensorScale, float32_tidl *weightScale, float32_tidl minScaleMMAv2, float32_tidl maxScaleMMAv2, sTIDL_OrgNetwork_t *pOrgTIDLNetStructure, int32_t layerIdx)
{
    float32_tidl scaleRatio;
    scaleRatio = (outTensorScale) / ((*weightScale) * inTensorScale);
    if((scaleRatio > maxScaleMMAv2) || (scaleRatio < minScaleMMAv2))
    {
      TIDL_GLOBAL_REPORT_WARNING("Convolution layer: %s has a scale ratio which exceeds representation capacity, clipping weights! [Out: %f Wt: %f  In: %f]\n", (char*)pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].name, outTensorScale, *weightScale, inTensorScale);
      if(scaleRatio > maxScaleMMAv2)
      {
        (*weightScale) = outTensorScale / (maxScaleMMAv2 * inTensorScale);
        scaleRatio = maxScaleMMAv2;
      }
      else
      {
        (*weightScale) = outTensorScale / (minScaleMMAv2 * inTensorScale);
        scaleRatio = minScaleMMAv2;
      }
    }
    return scaleRatio;
}

bool TIDL_areConsumerLayersAsym(sTIDL_OrgNetwork_t * pOrgTIDLNetStructure,
                                int32_t producerLayerIdx)
{
  int32_t layerIdx;
  sTIDL_LayerPC_t * currLayer;
  sTIDL_LayerPC_t * producerLayer;
  int32_t producerElemType;
  int32_t producerDataId;
  int32_t inIdx;

  producerLayer = &pOrgTIDLNetStructure->TIDLPCLayers[producerLayerIdx];
  producerElemType = producerLayer->outData[0].elementType;
  producerDataId   = producerLayer->outData[0].dataId;
  bool isAsym = true;

  if ( producerLayer->numOutBufs > 0)
  {
    /* Go through all the layers */
    for ( layerIdx = 0; layerIdx < pOrgTIDLNetStructure->numLayers; layerIdx++)
    {
      currLayer = &pOrgTIDLNetStructure->TIDLPCLayers[layerIdx];
      /* Find layers whos input contains the data Id. This will indicate the producer
      layer's output goes to this layer */
      for (inIdx = 0; inIdx < currLayer->numInBufs; inIdx++)
      {
        /* Indicates one of the consumer layer is found */
        if ( producerDataId == currLayer->inData[inIdx].dataId)
        {
          /*Check if this consumer layer can support asymmetric*/
          isAsym = TIDL_doesLayerSupportAsymTensors(currLayer, pOrgTIDLNetStructure);
          if(isAsym == false)
          {
            break;
          }
        }
      }
      /*If any consumer cannot support, overall output of producer layer cannot be asymmetric:*/
      if(isAsym == false)
      {
        break;
      }
    }
  }
  return isAsym;
}

void TIDL_dumpWeightScalesToFile(sTIDL_OrgNetwork_t * pOrgTIDLNetStructure, int32_t layerIdx)
{
  FILE *fp;
  char fileName[256];
  int32_t i;
  int32_t numOutChannels = 0;
  /*Dump innerproduct scales:*/
  TIDL_GLOBAL_REPORT_ERROR("Dumping weight scales for layer %d", layerIdx);
  sprintf(fileName, "layer_%d_weightScales.bin", layerIdx);
  fp = fopen(fileName, "wb");
  if(fp == NULL)
  {
    TIDL_GLOBAL_REPORT_ERROR("Error opening file to dump weight scales");
    return;
  }
  if(pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerType == TIDL_ConvolutionLayer)
  {
    numOutChannels = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.convParams.numOutChannels;
  }
  else if(pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerType == TIDL_InnerProductLayer)
  {
    numOutChannels = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.innerProductParams.numOutCols * pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.innerProductParams.numBChannels;
  }
  float32_tidl *weightScales = (float32_tidl*)pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].weightScales.ptr;
  for(i = 0; i < numOutChannels; i++)
  {
    fwrite(&weightScales[i], sizeof(float32_tidl), 1, fp);
  }
  fclose(fp);
}

int32_t TIDL_asymUpdateScalesAndShifts(sTIDL_OrgNetwork_t * pOrgTIDLNetStructure, int32_t layerIdx)
{
    // TIDL_GLOBAL_REPORT_WARNING("Scale compute for layer %d", layerIdx);
    int32_t weightsElementSizeInBits = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].weightsElementSizeInBits;
    float min = FLT_MAX;
    float max = -FLT_MAX;
    float32_tidl minScaleMMAv2 = TIDL_MMAV2_MIN_SCALE_8;
    float32_tidl maxScaleMMAv2 = TIDL_MMAV2_MAX_SCALE;
    float32_tidl maxBiasTermBits = TIDL_MMAV2_MAX_BIAS_BITS_8;
    int32_t      maxWeightBits   = 7U;
    if(weightsElementSizeInBits > 8)
    {
      minScaleMMAv2   = TIDL_MMAV2_MIN_SCALE_16;
      maxScaleMMAv2   = TIDL_MMAV2_MAX_SCALE_16;
      maxBiasTermBits = TIDL_MMAV2_MAX_BIAS_BITS_16;
      maxWeightBits   = 15U;
    }

    {
      // if the layer range is entirely on one side of zero, then we can convert the output tensor to unsigned and use asymmetric quantization
      // this is only possible if all consumer layers can support asymmetric tensors, which is checked in TIDL_areConsumerLayersAsym
      // Cases where layers can't support unsigned outputs with singed outputs are excluded from this optimization
      sTIDL_LayerPC_t * currLayer = &pOrgTIDLNetStructure->TIDLPCLayers[layerIdx];
      if(currLayer->numOutBufs == 1 && TIDL_areConsumerLayersAsym(pOrgTIDLNetStructure, layerIdx)){
        if(currLayer->outData[0].minTensorValue >= 0) {
          bool skip = false;
          if(currLayer->layerType == TIDL_InnerProductLayer) {
            const sTIDL_DataParams_t * inData = TIDL_getOutData(pOrgTIDLNetStructure, pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].inData[0].dataId);
            if(inData == NULL)
            {
              TIDL_GLOBAL_REPORT_ERROR("Cannot find Producer");
              return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
            }
            if(inData->elementType == TIDL_SignedChar || inData->elementType == TIDL_SignedShort) {
              currLayer->outData[0].tensorType = TIDL_SYMMETRIC_TENSOR;
              skip = true;
            }
          }

          if(!skip) {
            if(currLayer->outData[0].elementType == TIDL_SignedChar) {
              currLayer->outData[0].elementType = TIDL_UnsignedChar;
              currLayer->outData[0].tensorType = TIDL_ASYMMETRIC_TENSOR;
            }
            else if(currLayer->outData[0].elementType == TIDL_SignedShort) {
              currLayer->outData[0].elementType = TIDL_SignedShort;
              currLayer->outData[0].tensorType = TIDL_ASYMMETRIC_TENSOR;
            }
          }
        }
      }
    }

    sTIDL_DataParams_t * outData = &pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].outData[0];
    if(outData->elementType == TIDL_SignedChar)
    {
      pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].minPSAT = -128;
      pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].maxPSAT = +127;
    }
    else if(outData->elementType == TIDL_UnsignedChar)
    {
      pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].minPSAT =    0;
      pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].maxPSAT = +255;
    }
    else if(outData->elementType == TIDL_SignedShort)
    {
      pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].minPSAT = -32768;
      pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].maxPSAT = +32767;
    }
    else if(outData->elementType == TIDL_UnsignedShort)
    {
      pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].minPSAT =     0;
      pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].maxPSAT = +65535;
    }

    /** Clip the min max tensor values if the layer has clip activation */
    if (pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].actParams.actType == TIDL_Clip)
    {
      outData->minTensorValue = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].actParams.clipMin;
      outData->maxTensorValue = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].actParams.clipMax;
    }

    if(pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerType == TIDL_ConvolutionLayer)
    {
      int32_t numInChannels = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.convParams.numInChannels;
      int32_t numOutChannels = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.convParams.numOutChannels;
      const sTIDL_DataParams_t * inData = TIDL_getOutData(pOrgTIDLNetStructure, pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].inData[0].dataId);
      if(inData == NULL)
      {
        TIDL_GLOBAL_REPORT_ERROR("Cannot find Producer");
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }
      sTIDL_DataParams_t * outData = &pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].outData[0];
      sTIDL_LayerPC_t * currLayer = &pOrgTIDLNetStructure->TIDLPCLayers[layerIdx];
      /*Quantize weights (in a symmetric manner, so that the extra zero point term doesn't need to be computed)*/
      /*Naive approach considering only the weight ranges and not the accumulator ranges (VT: TBD)*/
      float32_tidl *weightPtr = (float32_tidl *)pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].weights.ptr;
      int32_t weightBufferSize = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].weights.bufSize;
      uint8_t * quantizedWeights = (uint8_t *)my_malloc(weightBufferSize * sizeof(float32_tidl));
      uint32_t dataPerChannel = (weightBufferSize / numOutChannels);
      float32_tidl *weightScales = (float32_tidl *)pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].weightScales.ptr;
      int32_t weightElementType = weightsElementSizeInBits > 8 ? TIDL_SignedShort : TIDL_SignedChar;


      if(weightsElementSizeInBits <= 8)
      {
        int32_t *derivedBias = (int32_t *)pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].derivedBias.ptr;
      }
      else
      {
        int64_t *derivedBias = (int64_t *)pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].derivedBias.ptr;
      }
      uint8_t *derivedScales = (uint8_t *)pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].derivedScales.ptr;
      uint8_t *derivedShifts = (uint8_t *)pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].derivedShifts.ptr;

      int32_t i0;
      #if 1
      for(i0 = 0; i0 < numOutChannels; i0++)
      {
        min = FLT_MAX;
        max = -FLT_MAX;
        TIDL_findRange(&weightPtr[i0 * dataPerChannel], dataPerChannel, &min, &max, 1.0);
        int32_t zeroPoint = 0;
        bool zeroOutBuf = TIDL_asymRangeToScale(&weightScales[i0], &zeroPoint, min, max, TIDL_SYMMETRIC_TENSOR, weightElementType);
        #if 1
        /*Calculate the number of bits needed to represent ProdMax and ProdMin inline*/
        int32_t numBitsMax = 0;
        int32_t numBitsMin = 0;
        if(outData->maxTensorValue != 0)
        {
          numBitsMax = (int32_t)ceil(log2(fabs(weightScales[i0] * inData->tensorScale * outData->maxTensorValue)));
          if((weightScales[i0] * inData->tensorScale * outData->maxTensorValue) < 0)
          {
            numBitsMax++; //sign bit
          }
        }
        if(outData->minTensorValue != 0)
        {
          numBitsMin = (int32_t)ceil(log2(fabs(weightScales[i0] * inData->tensorScale * outData->minTensorValue)));
          if((weightScales[i0] * inData->tensorScale * outData->minTensorValue) < 0)
          {
            numBitsMin++; //sign bit
          }
        }
        if(numBitsMin > maxBiasTermBits || numBitsMax > maxBiasTermBits)
        {
          /*Adjust weight scale (Reduce it, such that bits fit)*/
          float32_tidl scaleAdjust = 1.0f;
          if(numBitsMin > numBitsMax)
          {
            scaleAdjust = pow(2, (numBitsMin - maxBiasTermBits));
          }
          else
          {
            scaleAdjust = pow(2, (numBitsMax - maxBiasTermBits));
          }
          weightScales[i0] /= scaleAdjust;
        }
        #endif
      }
      #endif
      #if 0
      /*Temporarily use a global weight scale*/
      min = FLT_MAX;
      max = -FLT_MAX;
      TIDL_findRange(&weightPtr[0], weightBufferSize, &min, &max, 1.0);
      for(i0 = 0; i0 < numOutChannels; i0++)
      {
        int8_t zeroPoint = 0;
        TIDL_asymRangeToScale(&weightScales[i0], &zeroPoint, min, max, TIDL_SYMMETRIC_TENSOR, weightElementType);
      }
      #endif
      /********************************************************************************/
      /*Hold actual quantizatiion of weights till the accumulator scale is analyzed:*/
      /*Update output tensor scale based on current range for stats collection:*/
      float32_tidl inTensorScale = inData->tensorScale;
      int32_t inZeroPoint = inData->tensorZeroPoint; /*Oversized zero point container*/
      if((inData->tensorType == TIDL_SYMMETRIC_TENSOR))
      {
        /*Check if producer layer could actually support asymmetric and only then propagate zero point*/
        inZeroPoint = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].inData[0].tensorZeroPoint = 0;
      }

      float32_tidl outTensorScale;
      int32_t outZeroPoint = 0;
      int32_t isOutMaxSat = 0;
      if (pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].actParams.actType == TIDL_Clip)
      {
        outData->minTensorValue = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].actParams.clipMin;
        outData->maxTensorValue = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].actParams.clipMax;
        isOutMaxSat = 1;
      }
      if (pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].actParams.actType == TIDL_RelU6)
      {
        outData->minTensorValue = 0.0;
        outData->maxTensorValue = 6.0;
        isOutMaxSat = 1;
      }
      
      TIDL_asymRangeToScale(&outTensorScale, &outZeroPoint, outData->minTensorValue, outData->maxTensorValue, outData->tensorType, outData->elementType, pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].quantizeConstraint);
      if(currLayer->quantizeConstraint == TIDL_QuantizeUnconstrained)
      {
        outData->tensorScale     = outTensorScale;
        outData->tensorZeroPoint = outZeroPoint;
      }
      TIDL_updateDataBufferScaleAndZeroPoint(pOrgTIDLNetStructure, outData->dataId);

      /*Out scale is 8/9-bits (Sym/Asym) at this point*/
      /******************************************************************************************************/
      /*                               MMAv2 Constraints on Scales
          mmaScale/2^mmaShift = scale_y / (scale_x * scale_w[i0])
          This implies a maximum value of "2^TIDL_MMAV2_SCALE_BITS" for the above equation, post which hardware
          can't support the representation. scale_x is a property of the previous layer, scale_y is not going
          to be modified temporarily and worst case, weight clipping will be done.
          Also, scale_bias = scale_x * scale_w[i0], so that quantities in the accumulator can directly be
          added.
      */
      /******************************************************************************************************/
      /*Determine hardware quantities:*/
      float32_tidl scaleRatio;
      for(i0 = 0; i0 < numOutChannels; i0++)
      {
        scaleRatio = TIDL_asymGetScaleRatio(outTensorScale, inTensorScale, &weightScales[i0], minScaleMMAv2, maxScaleMMAv2, pOrgTIDLNetStructure, layerIdx);
        /*Update scale & shift for MMA*/
        //TIDL_getMMAv2_ScaleAndShift(scaleRatio, &derivedScales[i0], &derivedShifts[i0], weightsElementSizeInBits, maxScaleMMAv2);
        float32_tidl maxErr = TIDL_getMMAv2_ScaleShiftAndError(scaleRatio, &derivedScales[i0], &derivedShifts[i0], weightsElementSizeInBits, maxScaleMMAv2);
        if(((float)maxErr/scaleRatio) > TIDL_MAX_SCALE_REPRESENTATION_ERROR && gParams.preQuantizedModel == 0)
        {
          /*Relative error of > .1% in scaleRatio, adjust weightScale and outScale appropriately*/
          /*scaleRatio = sy/sw*sx , where sx is fixed*/
          float32_tidl scaleHW = (float32_tidl)derivedScales[i0]/pow(2,derivedShifts[i0]);
          float32_tidl weightScale = weightScales[i0];
          float32_tidl idealWtScale = 0;
          /*Adjust only based on weight scale (Naive approach):*/
          idealWtScale = outTensorScale/(scaleHW * inTensorScale);
          /*If this is less than a 2% change in weightscale, allow it:*/
          if(((float)(abs(idealWtScale - weightScale)/weightScale)) < TIDL_MAX_SCALE_UPDATE)
          {
            weightScales[i0] = idealWtScale;
          }
        }
        /*Quantize and store weights:*/
        if (weightsElementSizeInBits <= 8)
        {
          /*8-bit*/
          TIDL_QuantizeFloatToFixed(&((int8_t*)quantizedWeights)[i0 * dataPerChannel], &weightPtr[i0 * dataPerChannel], dataPerChannel, 1, weightScales[i0], 0U);
        }
        else
        {
          /*16-bit*/
          TIDL_QuantizeFloatToFixed(&((int16_t*)quantizedWeights)[i0 * dataPerChannel], &weightPtr[i0 * dataPerChannel], dataPerChannel, 1, weightScales[i0], 0U);
        }
        /*Bias overflow prevention:*/
        {
          int32_t numBiasBits = 0;
          if(weightsElementSizeInBits <= 8)
          {
            numBiasBits = TIDL_calculateAsymmetricAbsBiasTermBits(pOrgTIDLNetStructure, (int8_t*)quantizedWeights, layerIdx, i0, inTensorScale, outTensorScale);
          }
          else
          {
            numBiasBits = TIDL_calculateAsymmetricAbsBiasTermBits(pOrgTIDLNetStructure, (int16_t*)quantizedWeights, layerIdx, i0, inTensorScale, outTensorScale);
          }
          int32_t numReductions = 0;
          while(numBiasBits > maxBiasTermBits)
          {
            /*Update weight scales and requantize weights to calculate new bias values*/
            /*Original weight scale = 2^weightbits/Range => weightbits reduced by 1 at a time*/
            numReductions++;
            weightScales[i0] /= TIDL_WEIGHT_SCALE_REDUCTION_FACTOR;
            if(numReductions > maxWeightBits)
            {
              /*Extreme case where weights are effectively 0, and thus sw is set to 1 and all weights are zeroed out*/
              weightScales[i0] = 1;
            }
            /*Requantize weights:*/
            if (weightsElementSizeInBits <= 8)
            {
              /*8-bit*/
              TIDL_QuantizeFloatToFixed(&((int8_t*)quantizedWeights)[i0 * dataPerChannel], &weightPtr[i0 * dataPerChannel], dataPerChannel, 1, weightScales[i0], 0U);
            }
            else
            {
              /*16-bit*/
              TIDL_QuantizeFloatToFixed(&((int16_t*)quantizedWeights)[i0 * dataPerChannel], &weightPtr[i0 * dataPerChannel], dataPerChannel, 1, weightScales[i0], 0U);
            }
            if(weightsElementSizeInBits <= 8)
            {
              numBiasBits = TIDL_calculateAsymmetricAbsBiasTermBits(pOrgTIDLNetStructure, (int8_t*)quantizedWeights, layerIdx, i0, inTensorScale, outTensorScale);
            }
            else
            {
              numBiasBits = TIDL_calculateAsymmetricAbsBiasTermBits(pOrgTIDLNetStructure, (int16_t*)quantizedWeights, layerIdx, i0, inTensorScale, outTensorScale);
            }
            if(numReductions > maxWeightBits)
            {
              TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Debug: numBiasBits = %d numReductions = %d",numBiasBits,numReductions);
              break;
            }
          }
          /*Update scales again*/
          scaleRatio = TIDL_asymGetScaleRatio(outTensorScale, inTensorScale, &weightScales[i0], minScaleMMAv2, maxScaleMMAv2, pOrgTIDLNetStructure, layerIdx);
          TIDL_getMMAv2_ScaleShiftAndError(scaleRatio, &derivedScales[i0], &derivedShifts[i0], weightsElementSizeInBits, maxScaleMMAv2);
        }
      }
      /*Free original weight buffer (float) & point to the quantized buffer*/
      my_free(weightPtr);
      pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].weights.ptr = quantizedWeights;

      /*Bias quantization and zero point adjustment*/
      {
        void *derivedBiasPtr = (float32_tidl *)pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].derivedBias.ptr;
        /*Assuming signed bias terms only:*/
        if (weightsElementSizeInBits <= 8)
        {
          TIDL_IMPORT_CHECK_AND_RETURN(TIDL_asymDeriveBiasTerm((int32_t*) derivedBiasPtr, (int8_t*)quantizedWeights, pOrgTIDLNetStructure, layerIdx, weightsElementSizeInBits, maxScaleMMAv2), "");
        }
        else
        {
          TIDL_IMPORT_CHECK_AND_RETURN(TIDL_asymDeriveBiasTerm((int64_t*) derivedBiasPtr, (int16_t*)quantizedWeights, pOrgTIDLNetStructure, layerIdx, weightsElementSizeInBits, maxScaleMMAv2), "");
        }
      }

      TIDL_updateDataBufferScaleAndZeroPoint(pOrgTIDLNetStructure, outData->dataId);

    }
    else if(pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerType == TIDL_InnerProductLayer)
    {
      sTIDL_LayerPC_t * currLayer = &pOrgTIDLNetStructure->TIDLPCLayers[layerIdx];
      int32_t numAxis = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.innerProductParams.numOutCols;
      int32_t innerDims = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.innerProductParams.numInCols;
      int32_t numBChannels = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.innerProductParams.numBChannels;
      int32_t weightPitch = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.innerProductParams.numOutCols;
      int32_t weightChannelPitch = innerDims * weightPitch;
      if(pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.innerProductParams.constIdx == 0)
      {
        //Row quantization:
        numAxis = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.innerProductParams.numInRows;
        weightPitch = 1;
      }
      const sTIDL_DataParams_t * inData = TIDL_getOutData(pOrgTIDLNetStructure, pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].inData[0].dataId);
      if(inData == NULL)
      {
        TIDL_GLOBAL_REPORT_ERROR("Cannot find Producer");
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }
      sTIDL_DataParams_t * outData = &pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].outData[0];
      /*Quantize weights (in a symmetric manner, so that the extra zero point term doesn't need to be computed)*/
      int32_t constLyrIdx = tidl_getInLayer(*pOrgTIDLNetStructure, pOrgTIDLNetStructure->numLayers, pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].inData[1].dataId);
      if(constLyrIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
      {
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }
      float32_tidl *weightPtr;
      int32_t weightBufferSize = 0;
      if(pOrgTIDLNetStructure->TIDLPCLayers[constLyrIdx].layerType == TIDL_ConstDataLayer)
      {
        if(pOrgTIDLNetStructure->TIDLPCLayers[constLyrIdx].weights.ptr)
          {
            weightPtr = (float32_tidl*)pOrgTIDLNetStructure->TIDLPCLayers[constLyrIdx].weights.ptr;
            weightBufferSize = pOrgTIDLNetStructure->TIDLPCLayers[constLyrIdx].weights.bufSize;
          }
      }
      else
      {
        TIDL_GLOBAL_REPORT_ERROR("Unable to find MatMul weight coefficients for %s, Aborting",pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].outDataNames[0]);
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }
      uint8_t * quantizedWeights = (uint8_t *)my_malloc(weightBufferSize * sizeof(float32_tidl));
      uint32_t dataPerChannel = (weightBufferSize / (numBChannels * numAxis));
      
      float32_tidl *weightScales = (float32_tidl *)pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].weightScales.ptr;
      int32_t weightElementType = weightsElementSizeInBits > 8 ? TIDL_SignedShort : TIDL_SignedChar;

      if(weightsElementSizeInBits <= 8)
      {
        int32_t *derivedBias = (int32_t *)pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].derivedBias.ptr;
      }
      else
      {
        int64_t *derivedBias = (int64_t *)pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].derivedBias.ptr;
      }
      uint8_t *derivedScales = (uint8_t *)pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].derivedScales.ptr;
      uint8_t *derivedShifts = (uint8_t *)pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].derivedShifts.ptr;

      int32_t i0, i1;

      for(i1 = 0; i1 < numBChannels; i1++)
      {
        for(i0 = 0; i0 < numAxis; i0++)
        {
          min = FLT_MAX;
          max = -FLT_MAX;
          TIDL_findRangePitch(&weightPtr[i0 + (i1 * weightChannelPitch)], dataPerChannel, weightPitch, &min, &max, 1.0);
          int32_t zeroPoint = 0;
          bool zeroOutBuf = TIDL_asymRangeToScale(&weightScales[i0 + (i1 * numAxis)], &zeroPoint, min, max, TIDL_SYMMETRIC_TENSOR, weightElementType);
          /*Constrict scale based on maximum accumulator bits for asym*/
          #if 1
          /*Calculate the number of bits needed to represent ProdMax and ProdMin inline*/
          int32_t numBitsMax = 0;
          int32_t numBitsMin = 0;
          if(outData->maxTensorValue != 0)
          {
            numBitsMax = (int32_t)ceil(log2(fabs(weightScales[i0 + (i1 * numAxis)] * inData->tensorScale * outData->maxTensorValue)));
            if((weightScales[i0 + (i1 * numAxis)] * inData->tensorScale * outData->maxTensorValue) < 0)
            {
              numBitsMax++; //sign bit
            }
          }
          if(outData->minTensorValue != 0)
          {
            numBitsMin = (int32_t)ceil(log2(fabs(weightScales[i0 + (i1 * numAxis)] * inData->tensorScale * outData->minTensorValue)));
            if((weightScales[i0 + (i1 * numAxis)] * inData->tensorScale * outData->minTensorValue) < 0)
            {
              numBitsMin++; //sign bit
            }
          }
          if(numBitsMin > maxBiasTermBits || numBitsMax > maxBiasTermBits)
          {
            /*Adjust weight scale (Reduce it, such that bits fit)*/
            float32_tidl scaleAdjust = 1.0f;
            if(numBitsMin > numBitsMax)
            {
              scaleAdjust = pow(2, (numBitsMin - maxBiasTermBits));
            }
            else
            {
              scaleAdjust = pow(2, (numBitsMax - maxBiasTermBits));
            }
            weightScales[i0 + (i1 * numAxis)] /= scaleAdjust;
          }
          #endif
        }
      }

      /********************************************************************************/
      /*Hold actual quantizatiion of weights till the accumulator scale is analyzed:*/
      /*Update output tensor scale based on current range for stats collection:*/
      float32_tidl inTensorScale = inData->tensorScale;
      int32_t inZeroPoint = inData->tensorZeroPoint; /*Oversized zero point container*/
      if((inData->tensorType == TIDL_SYMMETRIC_TENSOR))
      {
        /*Check if producer layer could actually support asymmetric and only then propagate zero point*/
        inZeroPoint = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].inData[0].tensorZeroPoint = 0;
      }

      float32_tidl outTensorScale;
      int32_t outZeroPoint = 0;
      int32_t isOutMaxSat = 0;

      if (pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].actParams.actType == TIDL_Clip)
      {
        outData->minTensorValue = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].actParams.clipMin;
        outData->maxTensorValue = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].actParams.clipMax;
        isOutMaxSat = 1;
      }
      if (pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].actParams.actType == TIDL_RelU6)
      {
        outData->minTensorValue = 0.0;
        outData->maxTensorValue = 6.0;
        isOutMaxSat = 1;
      }
      if(TIDL_getDatElementSign(outData->elementType) == 0 || 
        pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].actParams.actType == TIDL_RelU6 ||
        pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].actParams.actType == TIDL_RelU ||
        (pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].actParams.actType == TIDL_Clip && outData->minTensorValue >= 0)) 
      {
        /* InnerProduct doesn't support SignedxSigned -> Unsigned 
           converting the output to Asymmetric Signed ouput, when supported
        */ 
       pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].outData[0].elementType = tidl_getElementSizeInBits(pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].outData[0].elementType) == 8U ? TIDL_SignedChar : TIDL_SignedShort;
       pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].outData[0].tensorType = TIDL_areConsumerLayersAsym(pOrgTIDLNetStructure, layerIdx) ? TIDL_ASYMMETRIC_TENSOR : TIDL_SYMMETRIC_TENSOR;
      }
      TIDL_asymRangeToScale(&outTensorScale, &outZeroPoint, outData->minTensorValue, outData->maxTensorValue, outData->tensorType, outData->elementType, pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].quantizeConstraint);
      if(currLayer->quantizeConstraint == TIDL_QuantizeUnconstrained)
      {
        outData->tensorScale     = outTensorScale;
        outData->tensorZeroPoint = outZeroPoint;
      }
      TIDL_updateDataBufferScaleAndZeroPoint(pOrgTIDLNetStructure, outData->dataId);

      /*Out scale is 8/9-bits (Sym/Asym) at this point*/
      /******************************************************************************************************/
      /*                               MMAv2 Constraints on Scales
          mmaScale/2^mmaShift = scale_y / (scale_x * scale_w[i0])
          This implies a maximum value of "2^TIDL_MMAV2_SCALE_BITS" for the above equation, post which hardware
          can't support the representation. scale_x is a property of the previous layer, scale_y is not going
          to be modified temporarily and worst case, weight clipping will be done.
          Also, scale_bias = scale_x * scale_w[i0], so that quantities in the accumulator can directly be
          added.
      */
      /******************************************************************************************************/
      /*Determine hardware quantities:*/
      float32_tidl scaleRatio;
      bool weightScaleCorrection = false;
      for (i1 = 0; i1 < numBChannels; i1++)
      {
        for(i0 = 0; i0 < numAxis; i0++)
        {
          int32_t dataIdx = i0 + (i1 * numAxis);
          int32_t weightOffset = i0 + (i1 * weightChannelPitch);
          scaleRatio = TIDL_asymGetScaleRatio(outTensorScale, inTensorScale, &weightScales[dataIdx], minScaleMMAv2, maxScaleMMAv2, pOrgTIDLNetStructure, layerIdx);
          /*Update scale & shift for MMA*/
          //TIDL_getMMAv2_ScaleAndShift(scaleRatio, &derivedScales[i0], &derivedShifts[i0], weightsElementSizeInBits, maxScaleMMAv2);
          float32_tidl maxErr = TIDL_getMMAv2_ScaleShiftAndError(scaleRatio, &derivedScales[dataIdx], &derivedShifts[dataIdx], weightsElementSizeInBits, maxScaleMMAv2, true);
          
          if(((float)maxErr/scaleRatio) > TIDL_MAX_SCALE_REPRESENTATION_ERROR && gParams.preQuantizedModel == 0)
          {
            /*Relative error of > .1% in scaleRatio, adjust weightScale and outScale appropriately*/
            /*scaleRatio = sy/sw*sx , where sx is fixed*/
            float32_tidl scaleHW = (float32_tidl)derivedScales[dataIdx]/pow(2,derivedShifts[dataIdx]);
            float32_tidl weightScale = weightScales[dataIdx];
            float32_tidl idealWtScale = 0;
            /*Adjust only based on weight scale (Naive approach):*/
            idealWtScale = outTensorScale/(scaleHW * inTensorScale);
            /*If this is less than a 2% change in weightscale or 16 bit, allow it:*/
            if(((float)(abs(idealWtScale - weightScale)/weightScale)) < TIDL_MAX_SCALE_UPDATE || weightsElementSizeInBits == 16)
            {
              weightScales[dataIdx] = idealWtScale;
            }
            weightScaleCorrection = true;
          }

          /*Quantize and store weights:*/
          if (weightsElementSizeInBits <= 8)
          {
            /*8-bit*/
            TIDL_QuantizeFloatToFixed(&((int8_t*)quantizedWeights)[weightOffset], &weightPtr[weightOffset], dataPerChannel, weightPitch, weightScales[dataIdx], 0U);
          }
          else
          {
            /*16-bit*/
            TIDL_QuantizeFloatToFixed(&((int16_t*)quantizedWeights)[weightOffset], &weightPtr[weightOffset], dataPerChannel, weightPitch, weightScales[dataIdx], 0U);
          }
          /*Bias overflow prevention:*/
          {
            int32_t numBiasBits = 0;
            if(weightsElementSizeInBits <= 8)
            {
              numBiasBits = TIDL_calculateAsymmetricAbsBiasTermBits(pOrgTIDLNetStructure, (int8_t*)quantizedWeights, layerIdx, dataIdx, weightOffset, inTensorScale, outTensorScale);
            }
            else
            {
              numBiasBits = TIDL_calculateAsymmetricAbsBiasTermBits(pOrgTIDLNetStructure, (int16_t*)quantizedWeights, layerIdx, dataIdx, weightOffset, inTensorScale, outTensorScale);
            }
            int32_t numReductions = 0;
            while(numBiasBits > maxBiasTermBits)
            {
              /*Update weight scales and requantize weights to calculate new bias values*/
              /*Original weight scale = 2^weightbits/Range => weightbits reduced by 1 at a time*/
              numReductions++;
              weightScales[dataIdx] /= TIDL_WEIGHT_SCALE_REDUCTION_FACTOR;
              if(numReductions > maxWeightBits)
              {
                /*Extreme case where weights are effectively 0, and thus sw is set to 1 and all weights are zeroed out*/
                weightScales[dataIdx] = 1;
              }
              /*Requantize weights:*/
              if (weightsElementSizeInBits <= 8)
              {
                /*8-bit*/
                TIDL_QuantizeFloatToFixed(&((int8_t*)quantizedWeights)[weightOffset], &weightPtr[weightOffset], dataPerChannel, weightPitch, weightScales[dataIdx], 0U);
              }
              else
              {
                /*16-bit*/
                TIDL_QuantizeFloatToFixed(&((int16_t*)quantizedWeights)[weightOffset], &weightPtr[weightOffset], dataPerChannel, weightPitch, weightScales[dataIdx], 0U);
              }
              if(weightsElementSizeInBits <= 8)
              {
                numBiasBits = TIDL_calculateAsymmetricAbsBiasTermBits(pOrgTIDLNetStructure, (int8_t*)quantizedWeights, layerIdx, dataIdx, weightOffset, inTensorScale, outTensorScale);
              }
              else
              {
                numBiasBits = TIDL_calculateAsymmetricAbsBiasTermBits(pOrgTIDLNetStructure, (int16_t*)quantizedWeights, layerIdx, dataIdx, weightOffset, inTensorScale, outTensorScale);
              }
              if(numReductions > maxWeightBits)
              {
                TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Debug: numBiasBits = %d numReductions = %d",numBiasBits,numReductions);
                break;
              }
            }
            /*Update scales again*/
            if(!weightScaleCorrection)
            {
              scaleRatio = TIDL_asymGetScaleRatio(outTensorScale, inTensorScale, &weightScales[dataIdx], minScaleMMAv2, maxScaleMMAv2, pOrgTIDLNetStructure, layerIdx);
              TIDL_getMMAv2_ScaleShiftAndError(scaleRatio, &derivedScales[dataIdx], &derivedShifts[dataIdx], weightsElementSizeInBits, maxScaleMMAv2, true);
            }
          }
        }
      }



      if(pOrgTIDLNetStructure->TIDLPCLayers[constLyrIdx].layerType == TIDL_ConstDataLayer)
      {
        if(pOrgTIDLNetStructure->TIDLPCLayers[constLyrIdx].weights.ptr)
          my_free(pOrgTIDLNetStructure->TIDLPCLayers[constLyrIdx].weights.ptr);
        pOrgTIDLNetStructure->TIDLPCLayers[constLyrIdx].weights.ptr = quantizedWeights;
      }

      /*Bias quantization and zero point adjustment*/

      void *derivedBiasPtr = (float32_tidl *)pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].derivedBias.ptr;
      /*Assuming signed bias terms only:*/
      if (weightsElementSizeInBits <= 8)
      {
        TIDL_IMPORT_CHECK_AND_RETURN(TIDL_asymDeriveBiasTerm((int32_t*) derivedBiasPtr, (int8_t*)quantizedWeights, pOrgTIDLNetStructure, layerIdx, weightsElementSizeInBits, maxScaleMMAv2), "");
      }
      else
      {
        TIDL_IMPORT_CHECK_AND_RETURN(TIDL_asymDeriveBiasTerm((int64_t*) derivedBiasPtr, (int16_t*)quantizedWeights, pOrgTIDLNetStructure, layerIdx, weightsElementSizeInBits, maxScaleMMAv2), "");
      }

      TIDL_updateDataBufferScaleAndZeroPoint(pOrgTIDLNetStructure, outData->dataId);
      /*Dump weight scales to file:*/
      // TIDL_dumpWeightScalesToFile(pOrgTIDLNetStructure, layerIdx);
    }
    else if(pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerType == TIDL_EltWiseLayer && 
           (pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.eltWiseParams.eltWiseType == TIDL_EltWiseSum ||
            pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.eltWiseParams.eltWiseType == TIDL_EltWiseProduct))
    {
      /*Updates for Eltwise layer:
        Input and output scale derivation: 
          - Iterate through all inputs and set the scales and zero
          - Store dervied scales & shifts into a container
          - Restriction for 7-bit scale
      */

      // quantizing the weights of const input
      int32_t weightElementType = weightsElementSizeInBits > 8 ? TIDL_SignedShort : TIDL_SignedChar;
      for(int32_t i = 0; i < pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].numInBufs; i++){
        int32_t constLayerIdx = tidl_getInLayer(*pOrgTIDLNetStructure, pOrgTIDLNetStructure->numLayers, pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].inData[i].dataId);
        if(constLayerIdx == -1){
          continue;
        }
        if(pOrgTIDLNetStructure->TIDLPCLayers[constLayerIdx].layerType != TIDL_ConstDataLayer){
          continue;
        }
        sTIDL_LayerPC_t& constLayer = pOrgTIDLNetStructure->TIDLPCLayers[constLayerIdx];
        float32_tidl *weightScale = &constLayer.outData[0].tensorScale;
        uint8_t * quantizedWeights = (uint8_t *)my_malloc(constLayer.weights.bufSize * sizeof(float32_tidl));

        float min = FLT_MAX;
        float max = -FLT_MAX;
        TIDL_findRange((float*)constLayer.weights.ptr, constLayer.weights.bufSize, &min, &max, 1.0);
        bool zeroOutBuf = TIDL_asymRangeToScale(weightScale, &constLayer.outData[0].tensorZeroPoint, min, max, TIDL_ASYMMETRIC_TENSOR, weightElementType);
        /*Quantize and store weights:*/
        if (weightsElementSizeInBits <= 8)
        {
          /*8-bit*/
          TIDL_QuantizeFloatToFixed(&((int8_t*)quantizedWeights)[0], (float*)constLayer.weights.ptr, constLayer.weights.bufSize, 1.0, *weightScale, constLayer.outData[0].tensorZeroPoint);
        }
        else
        {
          /*16-bit*/
          TIDL_QuantizeFloatToFixed(&((int16_t*)quantizedWeights)[0], (float*)constLayer.weights.ptr, constLayer.weights.bufSize, 1.0, *weightScale, constLayer.outData[0].tensorZeroPoint);
        }
        my_free(pOrgTIDLNetStructure->TIDLPCLayers[constLayerIdx].weights.ptr);
        pOrgTIDLNetStructure->TIDLPCLayers[constLayerIdx].weights.ptr = quantizedWeights;
      }
      float32_tidl outTensorScale = std::numeric_limits<float32_tidl>::max();
      float32_tidl maxScaleRatio = std::numeric_limits<float32_tidl>::min();
      int32_t outZeroPoint = 0;
      const sTIDL_DataParams_t * inData[TIDL_NUM_IN_BUFS];
      float32_tidl scaleRatios[TIDL_NUM_IN_BUFS];
      sTIDL_DataParams_t * outData = &pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].outData[0];
      /*Loop over all inputs and get the input data buffers*/
      int32_t numInputs = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].numInBufs;
      int32_t i;
      for( i = 0; i < numInputs; i++)
      {
        inData[i] = TIDL_getOutData(pOrgTIDLNetStructure, pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].inData[i].dataId);
      }
      TIDL_asymRangeToScale(&outTensorScale, &outZeroPoint, outData->minTensorValue, outData->maxTensorValue, outData->tensorType, outData->elementType, pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].quantizeConstraint);
      outData->tensorScale     = outTensorScale;
      outData->tensorZeroPoint = outZeroPoint;
      TIDL_updateDataBufferScaleAndZeroPoint(pOrgTIDLNetStructure, outData->dataId);

      if (pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.eltWiseParams.eltWiseType == TIDL_EltWiseSum)
      {
        void *derivedScales = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].derivedScales.ptr;

        float32_tidl derivedBias = 0;
        for( i = 0; i < numInputs; i++)
        {
          /*Update the scale and zp*/
          float32_tidl scaleRatio = outData->tensorScale/inData[i]->tensorScale;
          scaleRatios[i] = scaleRatio;
          maxScaleRatio = maxScaleRatio < scaleRatio  ? scaleRatio : maxScaleRatio;
        }
        derivedBias = (float32_tidl)outData->tensorZeroPoint;
        for( i = 0; i < numInputs; i++)
        {
          derivedBias -= (float32_tidl)inData[i]->tensorZeroPoint * scaleRatios[i];
        }
        /*Max Scale ratio is not dependent on the bias term*/
        /*MMA scale ratio based on Rmax/127*/
        uint8_t mmaScale;
        uint8_t mmaShift;
        float32_tidl maxIntValue = SYMMETRIC_8BIT_SIGNED_MAX;
        float32_tidl maxIntRep = MAX_INT_REP_8BIT;
        if(weightsElementSizeInBits > 8U)
        {
          maxIntRep = MAX_INT_REP_16BIT;
          maxIntValue = SYMMETRIC_16BIT_SIGNED_MAX;
        }
        /*Use the max scale ratio as the common scale ratio and multiply by 127 (signed limit) to determine the scale values:*/
        /*The maximum ratio corresponds to int7 max value, i.e. 
            maxScaleRatio = 127 * mmaScale/2^mmaShift
        */
        // TIDL_getUpperBoundMMAv2_ScaleShiftAndError(maxScaleRatio/maxIntRep, &mmaScale, &mmaShift, maxWeightBits, maxScaleMMAv2);

        TIDL_getMMAv2_ScaleAndShift(maxScaleRatio/maxIntRep, &mmaScale, &mmaShift, maxWeightBits, maxScaleMMAv2);
        float32_tidl commonFactor = (float32_tidl)mmaScale/pow(2, mmaShift); /*i.e. mmaScale/2^mmaShift*/
        for( i = 0; i < numInputs; i++)
        {
          /*Determine weights:*/
          float32_tidl _derivedScale = ((float32_tidl)scaleRatios[i]) / commonFactor; 
          /*Ensure derivedScale fits in the respective int8 or int16 container:*/
          _derivedScale = std::min(_derivedScale, maxIntValue);
          if (weightsElementSizeInBits <= 8U)
          {
            *((int8_t*)derivedScales + i) = (int8_t) round(_derivedScale);
          }
          else 
          {
            *((int16_t*)derivedScales + i) = (int16_t) round(_derivedScale);            
          }
        }
        
        derivedBias = (float32_tidl)derivedBias * (1 / commonFactor);

        pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.eltWiseParams.mmaScale = mmaScale;
        pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.eltWiseParams.mmaShift = mmaShift;
        pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.eltWiseParams.biasTerm = round(derivedBias);
      }
      else if (pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.eltWiseParams.eltWiseType == TIDL_EltWiseProduct)
      {
        float32_tidl commonFactor = outTensorScale;
        float32_tidl biasTerm = outZeroPoint/outTensorScale;
        for (int32_t k = 0; k < pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].numInBufs; k++)
        {
          commonFactor *= (1/inData[k]->tensorScale);
          biasTerm *= inData[k]->tensorScale;
        }

        uint8_t mmaScale;
        uint8_t mmaShift;
        TIDL_getMMAv2_ScaleShiftAndError(commonFactor, &mmaScale, &mmaShift, maxWeightBits, maxScaleMMAv2);
        pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.eltWiseParams.mmaScale = mmaScale;
        pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.eltWiseParams.mmaShift = mmaShift;
        pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.eltWiseParams.biasTerm = round(biasTerm);       
      }
    }
    else if(pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerType == TIDL_ConcatLayer)
    {
      /*Updates for concat layer:
        Input and output scale derivation: 
          - Iterate through all inputs and set the scales and zero
          - Store dervied scales & shifts into a container
          - Restriction for 7-bit scale
      */
      // quantizing the weights of const input
      int32_t weightElementType = weightsElementSizeInBits > 8 ? TIDL_SignedShort : TIDL_SignedChar;
      for(int32_t i = 0; i < pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].numInBufs; i++){
        int32_t constLayerIdx = tidl_getInLayer(*pOrgTIDLNetStructure, pOrgTIDLNetStructure->numLayers, pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].inData[i].dataId);
        if(constLayerIdx == -1){
          continue;
        }
        if(pOrgTIDLNetStructure->TIDLPCLayers[constLayerIdx].layerType != TIDL_ConstDataLayer){
          continue;
        }
        sTIDL_LayerPC_t& constLayer = pOrgTIDLNetStructure->TIDLPCLayers[constLayerIdx];
        float32_tidl *weightScale = &constLayer.outData[0].tensorScale;
        uint8_t * quantizedWeights = (uint8_t *)my_malloc(constLayer.weights.bufSize * sizeof(float32_tidl));

        float min = FLT_MAX;
        float max = -FLT_MAX;
        TIDL_findRange((float*)constLayer.weights.ptr, constLayer.weights.bufSize, &min, &max, 1.0);
        // if(min > 0)
        // {
        //   if(weightElementType == TIDL_SignedChar)
        //   {
        //     weightElementType = TIDL_UnsignedChar;
        //   }
        //   else if(weightElementType == TIDL_SignedShort)
        //   {
        //     weightElementType = TIDL_UnsignedShort;
        //   }
        // }
        bool zeroOutBuf = TIDL_asymRangeToScale(weightScale, &constLayer.outData[0].tensorZeroPoint, min, max, TIDL_ASYMMETRIC_TENSOR, weightElementType);
        constLayer.outData[0].elementType = weightElementType;
        /*Quantize and store weights:*/
        if (weightsElementSizeInBits <= 8)
        {
          /*8-bit*/
          TIDL_QuantizeFloatToFixed(&((int8_t*)quantizedWeights)[0], (float*)constLayer.weights.ptr, constLayer.weights.bufSize, 1.0, *weightScale, constLayer.outData[0].tensorZeroPoint);
        }
        else
        {
          /*16-bit*/
          TIDL_QuantizeFloatToFixed(&((int16_t*)quantizedWeights)[0], (float*)constLayer.weights.ptr, constLayer.weights.bufSize, 1.0, *weightScale, constLayer.outData[0].tensorZeroPoint);
        }
        my_free(pOrgTIDLNetStructure->TIDLPCLayers[constLayerIdx].weights.ptr);
        pOrgTIDLNetStructure->TIDLPCLayers[constLayerIdx].weights.ptr = quantizedWeights;
      }
      float32_tidl outTensorScale = std::numeric_limits<float32_tidl>::max();
      int32_t outZeroPoint = 0;
      const sTIDL_DataParams_t * inData[TIDL_NUM_IN_BUFS];
      sTIDL_DataParams_t * outData = &pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].outData[0];
      /*Loop over all inputs and get the input data buffers*/
      int32_t numInputs = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].numInBufs;
      int32_t i;

      for( i = 0; i < numInputs; i++)
      {
        inData[i] = TIDL_getOutData(pOrgTIDLNetStructure, pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].inData[i].dataId);
        #ifdef TIDL_FULL_ASYM
      /*Determine the output tensor scale based on the min scale*/
        if(outTensorScale > inData[i]->tensorScale)
        {
          outTensorScale = inData[i]->tensorScale;
          outZeroPoint = inData[i]->tensorZeroPoint;
        }
        #endif
      }

      bool sameScaleAndZp = true;
      i = 1;
      while(sameScaleAndZp && i < numInputs)
      {
        sameScaleAndZp = (inData[i]->tensorScale == inData[0]->tensorScale) && (inData[i]->tensorZeroPoint == inData[0]->tensorZeroPoint) && (inData[i]->elementType == inData[0]->elementType);
        i++;
      }
      if(sameScaleAndZp)
      {
        outData->tensorScale = inData[0]->tensorScale;
        outData->tensorZeroPoint = inData[0]->tensorZeroPoint;
        outData->elementType = inData[0]->elementType;
      }
      else 
      {
        TIDL_asymRangeToScale(&outTensorScale, &outZeroPoint, outData->minTensorValue, outData->maxTensorValue, outData->tensorType, outData->elementType, pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].quantizeConstraint);
        outData->tensorScale = outTensorScale;
        outData->tensorZeroPoint = outZeroPoint;
      }
      TIDL_updateDataBufferScaleAndZeroPoint(pOrgTIDLNetStructure, outData->dataId);
      uint8_t *derivedScales = (uint8_t *)pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].derivedScales.ptr;
      uint8_t *derivedShifts = (uint8_t *)pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].derivedShifts.ptr;
      void *derivedBiasPtr = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].derivedBias.ptr;
      for (i = 0; i < numInputs; i++)
      {
        /*Update the scale and zp*/
        float32_tidl scaleRatio = outData->tensorScale / inData[i]->tensorScale;
        TIDL_getMMAv2_ScaleShiftAndError(scaleRatio, &derivedScales[i], &derivedShifts[i], maxWeightBits, maxScaleMMAv2);

        if (weightsElementSizeInBits <= 8U)
        {
          *((int32_t *)derivedBiasPtr + i) = (int32_t)(((float32_tidl)outData->tensorZeroPoint * (inData[i]->tensorScale / outData->tensorScale)) - (inData[i]->tensorZeroPoint));
        }
        else
        {
          *((int64_t *)derivedBiasPtr + i) = (int64_t)(((float32_tidl)outData->tensorZeroPoint * (inData[i]->tensorScale / outData->tensorScale)) - (inData[i]->tensorZeroPoint));
        }
      }
    }
    else if(pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerType == TIDL_BatchNormLayer)
    {
      /*LUT LeakyRELU: Only the scales and zero points need to be derived since this is an activation*/
      float32_tidl outTensorScale;
      int32_t outZeroPoint = 0;
      sTIDL_LayerPC_t * currLayer = &pOrgTIDLNetStructure->TIDLPCLayers[layerIdx];
      if(currLayer->actParams.actType == TIDL_Sin || currLayer->actParams.actType == TIDL_Cos || currLayer->actParams.actType == TIDL_Tanh || currLayer->actParams.actType == TIDL_Sign) {
        outData->minTensorValue = std::max(outData->minTensorValue, -1.0f);
        outData->maxTensorValue = std::min(outData->maxTensorValue, 1.0f);
      }
      else if(currLayer->actParams.actType == TIDL_Sigmoid || currLayer->actParams.actType == TIDL_HardSigmoid) {
        outData->minTensorValue = std::max(outData->minTensorValue, 0.0f);
        outData->maxTensorValue = std::min(outData->maxTensorValue, 1.0f);
      }
      else if(currLayer->actParams.actType == TIDL_Asin || currLayer->actParams.actType == TIDL_Tan) {
        outData->minTensorValue = std::max(outData->minTensorValue, -1.57079632679f);
        outData->maxTensorValue = std::min(outData->maxTensorValue, 1.57079632679f);
      }
      else if(currLayer->actParams.actType == TIDL_Acos) {
        outData->minTensorValue = std::max(outData->minTensorValue, 0.0f);
        outData->maxTensorValue = std::min(outData->maxTensorValue, 3.14159265359f);
      }

      /* [MERGE_CHECK] Check if need to be wrapped with CHECK_AND_RETURN */
      TIDL_asymRangeToScale(&outTensorScale, &outZeroPoint, outData->minTensorValue, outData->maxTensorValue, outData->tensorType, outData->elementType, pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].quantizeConstraint);
      if(currLayer->quantizeConstraint == TIDL_QuantizeUnconstrained)
      {
        outData->tensorScale     = outTensorScale;
        outData->tensorZeroPoint = outZeroPoint;
      }
      if(currLayer->actParams.actType == TIDL_Round || currLayer->actParams.actType == TIDL_Ceil || currLayer->actParams.actType == TIDL_Floor)
      {
        /* Check if output min/max are in range of (dtype_min, dtype_max), if in range set scaleValue to 1*/
        float32_tidl clipMin = TIDL_clamp(outData->minTensorValue, pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].minPSAT, pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].maxPSAT);
        float32_tidl clipMax = TIDL_clamp(outData->maxTensorValue, pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].minPSAT, pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].maxPSAT);
        if (clipMin == outData->minTensorValue && clipMax == outData->maxTensorValue)
        {
          outData->tensorScale = 1;
          outData->tensorZeroPoint = 0;
        }
      }
      TIDL_updateDataBufferScaleAndZeroPoint(pOrgTIDLNetStructure, outData->dataId);
    }
    else if(pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerType == TIDL_LayerNormLayer)
    {
      sTIDL_LayerPC_t * currLayer = &pOrgTIDLNetStructure->TIDLPCLayers[layerIdx];
      /* [MERGE_CHECK] Check if need to be wrapped with CHECK_AND_RETURN */
      TIDL_asymRangeToScale(&outData->tensorScale, &outData->tensorZeroPoint, outData->minTensorValue, outData->maxTensorValue, TIDL_SYMMETRIC_TENSOR, outData->elementType, pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].quantizeConstraint);
      TIDL_updateDataBufferScaleAndZeroPoint(pOrgTIDLNetStructure, outData->dataId);
    }
    else if(pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerType == TIDL_SoftMaxLayer)
    {
      sTIDL_LayerPC_t * currLayer = &pOrgTIDLNetStructure->TIDLPCLayers[layerIdx];
      outData->minTensorValue = 0;
      outData->maxTensorValue = std::min(1.0f, outData->maxTensorValue);
      TIDL_asymRangeToScale(&outData->tensorScale, &outData->tensorZeroPoint, outData->minTensorValue, outData->maxTensorValue, TIDL_SYMMETRIC_TENSOR, outData->elementType, pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].quantizeConstraint);
      TIDL_updateDataBufferScaleAndZeroPoint(pOrgTIDLNetStructure, outData->dataId);
    }
    else if (pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerType == TIDL_DataConvertLayer && 
             pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerParams.dataConvertParams.type == TIDL_DC_TYPE_OUTPUT)
    {
      TIDL_asymRangeToScale(&outData->tensorScale, &outData->tensorZeroPoint, outData->minTensorValue, outData->maxTensorValue, outData->tensorType, outData->elementType, pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].quantizeConstraint);
      TIDL_updateDataBufferScaleAndZeroPoint(pOrgTIDLNetStructure, outData->dataId);
    }
    else if(quantizationPassThroughLayers.find(pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerType) != quantizationPassThroughLayers.end())
    {
      sTIDL_LayerPC_t * currLayer = &pOrgTIDLNetStructure->TIDLPCLayers[layerIdx];
      if(currLayer->layerType == TIDL_DataConvertLayer && currLayer->layerParams.dataConvertParams.type == TIDL_DC_TYPE_INPUT) {
        const sTIDL_DataParams_t * inData = TIDL_getOutData(pOrgTIDLNetStructure, pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].inData[0].dataId);
        if(gParams.numParamBits == tidl_getElementSizeInBits(inData->elementType) && 
           inData->tensorScale != 0 && inData->tensorScale != FLT_MIN) {
          outData->elementType = inData->elementType;
          outData->tensorScale = inData->tensorScale;
          outData->tensorZeroPoint = inData->tensorZeroPoint;
        } 
        else {
          /*If range is positive, use unsigned datatypes:*/
          if(outData->minTensorValue > 0)
          {
            if(outData->elementType == TIDL_SignedChar)
            {
              outData->elementType = TIDL_UnsignedChar;
            }
            else if(outData->elementType == TIDL_SignedShort)
            {
              outData->elementType = TIDL_UnsignedShort;
            }
          }
          TIDL_asymRangeToScale(&outData->tensorScale, &outData->tensorZeroPoint, outData->minTensorValue, outData->maxTensorValue, TIDL_SYMMETRIC_TENSOR, outData->elementType, pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].quantizeConstraint);
        }
        TIDL_updateDataBufferScaleAndZeroPoint(pOrgTIDLNetStructure, outData->dataId);
      }
      else if(currLayer->layerType == TIDL_DataConvertLayer && currLayer->layerParams.dataConvertParams.type == TIDL_DC_TYPE_MIXED_PRECISION) 
      {
        const sTIDL_DataParams_t* inData = TIDL_getOutData(pOrgTIDLNetStructure, currLayer->inData[0].dataId);
        currLayer->outData[0].roundBits = inData->roundBits;
        currLayer->outData[0].tensorScale = inData->tensorScale;
        currLayer->outData[0].tensorZeroPoint = inData->tensorZeroPoint;
        currLayer->outData[0].minTensorValue = inData->minTensorValue;
        currLayer->outData[0].maxTensorValue = inData->maxTensorValue;
        currLayer->outData[0].elementType = TIDL_increasePrecision(inData->elementType);
      }
      else if(currLayer->layerType == TIDL_PoolingLayer && currLayer->layerParams.poolParams.poolingType == TIDL_AveragePooling)
      {
        TIDL_asymRangeToScale(&outData->tensorScale, &outData->tensorZeroPoint, outData->minTensorValue, outData->maxTensorValue, outData->tensorType, outData->elementType, pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].quantizeConstraint);
      }
      else if(currLayer->layerType == TIDL_DetectionOutputLayer)
      {
        TIDL_asymRangeToScale(&outData->tensorScale, &outData->tensorZeroPoint, outData->minTensorValue, outData->maxTensorValue, TIDL_SYMMETRIC_TENSOR, outData->elementType, pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].quantizeConstraint);
      }
      else 
      {
        const sTIDL_DataParams_t* inData = TIDL_getOutData(pOrgTIDLNetStructure, currLayer->inData[0].dataId);
        currLayer->outData[0].roundBits = inData->roundBits;
        currLayer->outData[0].tensorScale = inData->tensorScale;
        currLayer->outData[0].tensorZeroPoint = inData->tensorZeroPoint;
        currLayer->outData[0].elementType = inData->elementType;
        currLayer->outData[0].minTensorValue = inData->minTensorValue;
        currLayer->outData[0].maxTensorValue = inData->maxTensorValue;
      }
      TIDL_updateDataBufferScaleAndZeroPoint(pOrgTIDLNetStructure, currLayer->outData[0].dataId);
    }
    return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}


int32_t TIDL_updateActivationProducerRanges(sTIDL_OrgNetwork_t *pOrgTIDLNetStructure, int32_t numLayers, int32_t layerIndex, tidl_import_config *configParams) {
  std::string metaLayersNamesListStr = (char*)configParams->metaLayersNamesList;
  for(int32_t i = 0; i < numLayers; i++) {
    sTIDL_LayerPC_t * currLayer = &pOrgTIDLNetStructure->TIDLPCLayers[i];
    /* Constraints for Activation producer layers */ 
    if(currLayer->layerKernelType == TIDL_HighPrecisionKernel && 
      currLayer->layerType == TIDL_BatchNormLayer && 
      currLayer->actParams.actType != TIDL_NoAct && 
      currLayer->actParams.actType != TIDL_RelU && 
      currLayer->actParams.actType != TIDL_Clip && 
      activationProducerRangeMap.find(currLayer->actParams.actType) != activationProducerRangeMap.end() &&
      (metaLayersNamesListStr.find(((char*)currLayer->outDataNames[0])) == std::string::npos)
    )
    {
      int32_t producerLayerIdx = tidl_getInLayer(*pOrgTIDLNetStructure, numLayers, currLayer->inData[0].dataId);
      while(producerLayerIdx != -1 && 
         quantizationPassThroughLayers.find(pOrgTIDLNetStructure->TIDLPCLayers[producerLayerIdx].layerType) != quantizationPassThroughLayers.end() && 
         pOrgTIDLNetStructure->TIDLPCLayers[producerLayerIdx].numOutBufs == 1
      ) {
        producerLayerIdx = tidl_getInLayer(*pOrgTIDLNetStructure, numLayers, pOrgTIDLNetStructure->TIDLPCLayers[producerLayerIdx].inData[0].dataId);
      }
      if(producerLayerIdx == -1) {
        TIDL_GLOBAL_REPORT_ERROR("Error finding producer layer for BatchNorm, Aborting");
        return -1;
      }
      sTIDL_LayerPC_t * producerLayer = &pOrgTIDLNetStructure->TIDLPCLayers[producerLayerIdx];

      if(!(producerLayer->numOutBufs == 1 && quantizationPassThroughLayers.find(producerLayer->layerType) == quantizationPassThroughLayers.end() && TIDL_isProducerLayer(producerLayer))) {
        continue;
      }

      float32_tidl minVal = activationProducerRangeMap[currLayer->actParams.actType].first;
      float32_tidl maxVal = activationProducerRangeMap[currLayer->actParams.actType].second;

      if(producerLayer->numOutBufs == 1) {
        if(producerLayer->actParams.actType == TIDL_Clip) {
          producerLayer->actParams.clipMin = TIDL_clamp(producerLayer->actParams.clipMin, minVal, maxVal);
          producerLayer->actParams.clipMax = TIDL_clamp(producerLayer->actParams.clipMax, minVal, maxVal);
        } else if(producerLayer->actParams.actType == TIDL_RelU) {
          producerLayer->actParams.actType = TIDL_Clip;
          producerLayer->outData[0].minTensorValue = 0.0f;
          producerLayer->outData[0].maxTensorValue = TIDL_clamp(producerLayer->outData[0].maxTensorValue, 0.0f, maxVal);
          producerLayer->actParams.clipMin = producerLayer->outData[0].minTensorValue;
          producerLayer->actParams.clipMax = producerLayer->outData[0].maxTensorValue;
        } else {
          producerLayer->actParams.actType = TIDL_Clip;
          producerLayer->outData[0].minTensorValue = TIDL_clamp(producerLayer->outData[0].minTensorValue, minVal, maxVal);
          producerLayer->outData[0].maxTensorValue = TIDL_clamp(producerLayer->outData[0].maxTensorValue, minVal, maxVal);
          producerLayer->actParams.clipMin = producerLayer->outData[0].minTensorValue;
          producerLayer->actParams.clipMax = producerLayer->outData[0].maxTensorValue;
        }
      }
    }
  }

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

int32_t TIDL_importQuantLayerParams(sTIDL_OrgNetwork_t   * pOrgTIDLNetStructure,
                                            sTIDL_Network_t        *pTIDLNetStructure,
                                            tidl_import_config       *configParams,
                                            int32_t layerIndex)
{
  int32_t i,j;
  int32_t numBins = TIDL_NUM_WEIGHT_HISTOGRAM_BINS;
  char filenameStr[1000];

  TIDL_IMPORT_CHECK_AND_RETURN(TIDL_updateAsymRangeConstraints(pOrgTIDLNetStructure, pOrgTIDLNetStructure->numLayers, configParams), "");
  /* Find the maximum scale possible for each layer’s Bias
  and weight parameters based on the actual range of their parameters.
  These. would be used while deciding the actual parameter and tensors
  scales in the next loop which would stratify the requirements of DL acceleration IP
  */
  TIDL_IMPORT_CHECK_AND_RETURN(TIDL_updateParamsRange(pOrgTIDLNetStructure, pTIDLNetStructure,layerIndex), "");
  TIDL_IMPORT_CHECK_AND_RETURN(TIDL_updateActivationProducerRanges(pOrgTIDLNetStructure, pTIDLNetStructure->numLayers, layerIndex, configParams), "");

  sprintf(filenameStr, "%s_paramDebug.csv", configParams->outputNetFile);

  paramDebugFile = fopen(filenameStr, "w+");
  if (paramDebugFile == NULL)
  {
    TIDL_GLOBAL_REPORT_ERROR("Could not open %s file, Aborting", filenameStr);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  fprintf(paramDebugFile, "LayerId , meanDifference, maxDifference, meanOrigFloat, meanRelDifference, orgmax, quantizedMax,orgAtmaxDiff, quantizedAtMaxDiff,maxRelDifference, Scale , , , , Hist \n");

  for (i = 0; i < layerIndex; i++)
  {
    if (pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_ConstDataLayer && pOrgTIDLNetStructure->TIDLPCLayers[i].outData[0].elementType == TIDL_SignedWord)
    {
      /* Gather indices input is of type int32, no need to quantize */
      continue;
    }
    //TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "layer %d quantization : %s && %d",i,(char*)pOrgTIDLNetStructure->TIDLPCLayers[i].name,pOrgTIDLNetStructure->TIDLPCLayers[i].layerType);
    int32_t weightsElementSizeInBits = pOrgTIDLNetStructure->TIDLPCLayers[i].weightsElementSizeInBits;
    const sTIDL_DataParams_t * indata = TIDL_getOutData(pOrgTIDLNetStructure,
                        pOrgTIDLNetStructure->TIDLPCLayers[i].inData[0].dataId);
    if(indata == NULL)
    {
      TIDL_GLOBAL_REPORT_ERROR("Cannot find Producer");
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }

    debugLayeId = i;
    currLayerName = (char*)pOrgTIDLNetStructure->TIDLPCLayers[i].name;
    currElemenType = indata->elementType;
    bool areConsumersAsym = TIDL_areConsumerLayersAsym(pOrgTIDLNetStructure, i);
    if(pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_BatchNormLayer) {
      pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.batchNormParams.mmaV2Bias = 0;
    }
    if(TIDL_QuantStyleAsymNP2 == gParams.quantizationStyle && (
      TIDL_doesLayerSupportAsymTensors(&pOrgTIDLNetStructure->TIDLPCLayers[i], pOrgTIDLNetStructure) ||
      (pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_DataConvertLayer && pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.dataConvertParams.type == TIDL_DC_TYPE_INPUT)
    )) {
      TIDL_IMPORT_CHECK_AND_RETURN(TIDL_asymUpdateScalesAndShifts(pOrgTIDLNetStructure, i), "");
    }
    else if(pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_DataConvertLayer && pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.dataConvertParams.type == TIDL_DC_TYPE_MIXED_PRECISION) 
    {
      TIDL_IMPORT_CHECK_AND_RETURN(TIDL_asymUpdateScalesAndShifts(pOrgTIDLNetStructure, i), "");
    }
    else if(!(TIDL_ConstDataLayer == pOrgTIDLNetStructure->TIDLPCLayers[i].layerType && areConsumersAsym && TIDL_QuantStyleAsymNP2 == gParams.quantizationStyle))
    {
      for (j = 0; j < TIDL_MAX_QUANT_PARAMS; j++)
      {
        int32_t dataSize = pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[j].size;
        if (dataSize)
        {
          /* PerChannelWeightQuantization for DWS convolution layer is handled differently compared to res of the layers */
          if((j == TIDL_WEIGHT_QUANT_PARAMS) &&  TIDL_isDepthwiseConvLayer(pOrgTIDLNetStructure,i) &&
            ((configParams->calibrationOption & TIDL_CalibOptionPerChannelWeightQuantization) == TIDL_CalibOptionPerChannelWeightQuantization))
          {
            TIDL_IMPORT_CHECK_AND_RETURN(TIDL_QuantPerChannelWeight(pOrgTIDLNetStructure,pTIDLNetStructure,configParams,i), "");
          }
          else
          {
            float *scalePtr = pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[j].scalePtr;
            void **prmPtr = pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[j].prmPtr;
            float min;
            float max;
            float maxWeightScale = FLT_MAX;
            float outTensorScale;
            void * params = (void *)my_malloc(dataSize * sizeof(float32_tidl));

            /* Check whether the current layers output tensor scale needs to satisfy
              any requirements on output tensor scale clipping (Example relu6).
            */
            int32_t isOutMaxSat = isOutputTensorMaxSatAvailable(&pOrgTIDLNetStructure->TIDLPCLayers[i], &outTensorScale);
            if(pOrgTIDLNetStructure->TIDLPCLayers[i].quantizeConstraint == TIDL_QuantizeConstainedP2)
            {
              isOutMaxSat = 1;
              outTensorScale = pOrgTIDLNetStructure->TIDLPCLayers[i].outData[0].tensorScale;
            }
            if(j != TIDL_WEIGHT_QUANT_PARAMS)
            {
              isOutMaxSat = 0;
            }

            /* Finding the maximum scale that can be used for weight parameters which would not saturate the
            Bias parameters of the current layers based on current layers input scale
            */
            if((j == TIDL_WEIGHT_QUANT_PARAMS) && ((configParams->calibrationOption & TIDL_CalibOptionBiasRange) == 0) &&
              (pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[TIDL_BIAS_QUANT_PARAMS].size > 0)  &&
              ((pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_ConvolutionLayer) ||
                (pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_Deconv2DLayer)))
            {
                min = pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[TIDL_BIAS_QUANT_PARAMS].min;
                max = pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[TIDL_BIAS_QUANT_PARAMS].max;

                float maxBiasScale = TIDL_findMaxQuantizationScale(min, max, weightsElementSizeInBits*2, TIDL_getDatElementSign(indata->elementType));
                maxWeightScale = maxBiasScale / indata->tensorScale;

            }
            min = pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[j].min;
            max = pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[j].max;

            int32_t outElemType = pOrgTIDLNetStructure->TIDLPCLayers[i].outData[0].elementType;
            if(TIDL_ConstDataLayer == pOrgTIDLNetStructure->TIDLPCLayers[i].layerType && pOrgTIDLNetStructure->TIDLPCLayers[i].quantizeConstraint != TIDL_QuantizeUnconstrained)
            {
              if(outElemType == TIDL_SignedChar)
              {
                TIDL_QuantizeFloatToFixed(((int8_t*)params), (float*)(*prmPtr), dataSize, 1, *scalePtr, 0U);
              }
              else if(outElemType == TIDL_UnsignedChar)
              {
                TIDL_QuantizeFloatToFixed(((uint8_t*)params), (float*)(*prmPtr), dataSize, 1, *scalePtr, 0U);
              }
              else if(outElemType == TIDL_SignedShort)
              {
                TIDL_QuantizeFloatToFixed(((int16_t*)params), (float*)(*prmPtr), dataSize, 1, *scalePtr, 0U);
              }
              else if(outElemType == TIDL_UnsignedShort)
              {
                TIDL_QuantizeFloatToFixed(((uint16_t*)params), (float*)(*prmPtr), dataSize, 1, *scalePtr, 0U);
              }
            }
            else
            {
              if((j == TIDL_BIAS_QUANT_PARAMS) || (weightsElementSizeInBits > 8))
              {
                *scalePtr = TIDL_QuantizeSignedMax((int16_t *)params, (float*)(*prmPtr), dataSize, min, max, weightsElementSizeInBits, maxWeightScale, isOutMaxSat, indata->tensorScale, outTensorScale);
              }
              else
              {
                *scalePtr = TIDL_QuantizeSignedMax((int8_t *)params, (float*)(*prmPtr), dataSize, min, max, weightsElementSizeInBits, maxWeightScale, isOutMaxSat, indata->tensorScale, outTensorScale);
              }
            }

            if ((j == TIDL_WEIGHT_QUANT_PARAMS) &&
                (pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_BatchNormLayer) &&
                ((pOrgTIDLNetStructure->TIDLPCLayers[i].actParams.actType == TIDL_Sigmoid) || (pOrgTIDLNetStructure->TIDLPCLayers[i].actParams.actType == TIDL_PRelU) ||(pOrgTIDLNetStructure->TIDLPCLayers[i].actParams.actType == TIDL_Tanh)||(pOrgTIDLNetStructure->TIDLPCLayers[i].actParams.actType == TIDL_HardSigmoid)||(pOrgTIDLNetStructure->TIDLPCLayers[i].actParams.actType == TIDL_ELU)))
            {
              /* The default value of weightsElementSizeInBits is set during initially which is set based on numParamBits as given by the user. For mixed precision
              the default value is updated based on whether a particular layer is running at higher precision or not. Hence at this point we should read the updated
              value of weightsElementSizeInBits as decided based on the precision of the layer*/
              *scalePtr = (1.0*((1 << (pOrgTIDLNetStructure->TIDLPCLayers[i].weightsElementSizeInBits -1))));
            }

            if(weightsElementSizeInBits == 8 && pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_BatchNormLayer && pOrgTIDLNetStructure->TIDLPCLayers[i].actParams.actType == TIDL_NoAct && gParams.deviceName != TIDL_TDA4VM) {
              /* For BN layers converted from scalar eltwise the range is being calculated with 128 as limit this is causing
                constant shift in output. To avoid this we are checking if the BN params are scalar and if so computing the scale manually and quantizing the params */
              bool scalarBN = true;
              int32_t idx = 1;
              while(scalarBN && idx < dataSize) {
                if (j == TIDL_BIAS_QUANT_PARAMS) {
                  scalarBN = (((int16_t*)params)[idx] == ((int16_t*)params)[0]);
                }
                else if(j == TIDL_WEIGHT_QUANT_PARAMS) {
                  scalarBN = (((int8_t*)params)[idx] == ((int8_t*)params)[0]);
                }
                idx++;
              }

              if(scalarBN) {
                float* val = (float*)(*prmPtr);
                if(*val == 0) {
                  *scalePtr = 1.0;
                }else {
                  *scalePtr = 127.0 / std::abs(*val);
                }

                if(j == TIDL_BIAS_QUANT_PARAMS) {
                  TIDL_QuantizeFloatToFixed(((int16_t*)params), (float*)(*prmPtr), dataSize, 1, *scalePtr, 0U);
                }
                else if(j == TIDL_WEIGHT_QUANT_PARAMS) {
                  TIDL_QuantizeFloatToFixed(((int8_t*)params), (float*)(*prmPtr), dataSize, 1, *scalePtr, 0U);
                }
              }
            }

            if(*scalePtr == -1)
            {
              if(j == TIDL_BIAS_QUANT_PARAMS)
              {
                *scalePtr = FLT_MAX;
              }
              else
              {
                *scalePtr = 1;
              }
            }
            my_free((*prmPtr));
            *prmPtr = (void*)params;
          }
        }
      }
      if (weightsElementSizeInBits == 8 && pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_BatchNormLayer && pOrgTIDLNetStructure->TIDLPCLayers[i].actParams.actType == TIDL_NoAct && gParams.deviceName != TIDL_TDA4VM)
      {
        /* For BN layers converted from scalar eltwise the range is being calculated with 128 as limit this is causing
          constant shift in output. To avoid this we are checking if the BN params are scalar and if so computing the scale manually and quantizing the params */
        bool scalarBN = true;
        int32_t idx = 1;
        int16_t* biasParams = (int16_t*) *pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[TIDL_BIAS_QUANT_PARAMS].prmPtr;
        int8_t* weightParams = (int8_t*) *pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[TIDL_WEIGHT_QUANT_PARAMS].prmPtr;
        int32_t dataSize = pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[TIDL_WEIGHT_QUANT_PARAMS].size;

        while (scalarBN && idx < dataSize)
        {
          scalarBN = scalarBN && (biasParams[idx] == biasParams[0]) && (weightParams[idx] == weightParams[0]);
          idx++;
        }

        if (scalarBN)
        {
          const sTIDL_DataParams_t *inData = TIDL_getOutData(pOrgTIDLNetStructure, pOrgTIDLNetStructure->TIDLPCLayers[i].inData[0].dataId);
          float sx = inData->tensorScale;
          float sw = *pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[TIDL_WEIGHT_QUANT_PARAMS].scalePtr;
          float sb = *pOrgTIDLNetStructure->TIDLPCLayers[i].quantParams[TIDL_BIAS_QUANT_PARAMS].scalePtr;
          float bias = (float)biasParams[0] / sb;

          float derivedBias = bias * sx * sw;
          derivedBias = std::min(32767.0f, derivedBias);
          derivedBias = std::max(-32768.0f, derivedBias);
          derivedBias = std::round(derivedBias);

          for(int32_t i = 0; i < dataSize; i++) {
            biasParams[i] = 0;
          }
          pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.batchNormParams.mmaV2Bias = derivedBias;
        }
      }
      TIDL_IMPORT_CHECK_AND_RETURN(TIDL_UpdateScaleFactors(pOrgTIDLNetStructure, i, 0, 0, 0, configParams), "");
    }
  }
  fclose(paramDebugFile);

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}



/* This function should always be in sync with similar function in TIDL with name
   TIDL_findCurrentOffsetForPerChannelMeanStats
*/
static int32_t tidlFindCurrentOffsetForPerChannelMean(sTIDL_Network_t * net, int32_t currLayerNum, int32_t * totalMemReq)
{
  int layerIdx = 0;
  int32_t totNumOutChannels = 0;
  int32_t currNumOutChannels = 0;
  for (layerIdx = 0; layerIdx < net->numLayers; layerIdx++)
  {

    if (layerIdx == (currLayerNum))
    {
      currNumOutChannels = totNumOutChannels;
    }

    if (net->TIDLLayers[layerIdx].layerType == TIDL_InnerProductLayer)
    {
      if(net->TIDLLayers[layerIdx].layerParams.innerProductParams.constIdx == 1)
      {
        /*Bias is along the column axis*/
        totNumOutChannels += net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_WIDTH] * net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_NUMCH];
      }
      else
      {
        totNumOutChannels += net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_NUMCH];
      }
    }
    else if ((net->TIDLLayers[layerIdx].layerType != TIDL_DataLayer) && (net->TIDLLayers[layerIdx].layerType != TIDL_ConstDataLayer))
    {
      //:TODO: This can eventually be done only for the layers where bias is applicable
      totNumOutChannels += net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_NUMCH];
    }
  }
  if (totalMemReq != NULL)
  {
    *totalMemReq = totNumOutChannels * sizeof(float32_tidl);
  }
  /* return -1 if data layer else actual offset */
  /* not including TIDL_ConstDataLayer results in less totalMemReq and gives issue in updating pointers of this size (unwanted memory gets updated) - returning -1 for constData layer also */
  return ((net->TIDLLayers[currLayerNum].layerType != TIDL_DataLayer && net->TIDLLayers[currLayerNum].layerType != TIDL_ConstDataLayer) ? (currNumOutChannels * sizeof(float32_tidl)) : -1);
}

int32_t tidlReadPerChannelMeanStatistics(sTIDL_Network_t * tidlNetStructure,
                                                                                void * perChannelMeanStats,
                                                                                int32_t perChannelMeanMemSize,
                                                                                tidl_import_config * configParams)
{
  if ( tidlNetStructure->isQuantStatsAvailable == 1 )
  {
    if (( tidlNetStructure->calibrationOption & TIDL_CalibOptionBiasCalibration) == TIDL_CalibOptionBiasCalibration )
    {
        int32_t memRequired;
        tidlFindCurrentOffsetForPerChannelMean(tidlNetStructure,
                                                              0,
                                                              &memRequired);
        if ( perChannelMeanMemSize != memRequired )
        {
          TIDL_GLOBAL_REPORT_ERROR("tidlReadPerChannelMeanStatistics : Not enough memory to read per channel mean statistics");
          return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
        }

        if ( perChannelMeanStats == NULL )
        {
          TIDL_GLOBAL_REPORT_ERROR("tidlReadPerChannelMeanStatistics : perChannelMeanStats pointer is NULL");
          return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
        }

        char perChannelMeanFileName[1024];
        char outDirName[1024];

        getAbsPath((char *)configParams->outputNetFile, outDirName);
        getDirFromPath(outDirName);
        sprintf(perChannelMeanFileName, "%s/%s_LayerPerChannelMean.bin", outDirName, getFileNameFromPath(inConfigFilename));

        FILE * fp = fopen(perChannelMeanFileName,"rb");
        if ( fp != NULL )
        {
          fread(perChannelMeanStats, 1, perChannelMeanMemSize, fp);
          fclose(fp);
        }
        else
        {
          TIDL_GLOBAL_REPORT_ERROR("tidlReadPerChannelMeanStatistics : Unable to read Per Channel Mean statistics");
          return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
        }
    }
  }

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

void TIDL_computeMeanActivationShift(float32_tidl * perChannelActShift,
                                    float32_tidl * perChannelMeanPtrFloat,
                                    float32_tidl * perChannelMeanPtrQuantized,
                                    float32_tidl biasCalibrationFactor,
                                    int32_t layerIndex)
{
  int32_t currOffsetInBytes, currOffsetInfloats, perChannelMeanMemSize;
  
  sTIDL_Network_t * net = &tIDLNetStructure;
  float32_tidl meanFloat, meanQuantized;
  float32_tidl activationDelta;
  float max_diff = -FLT_MAX;
  int32_t max_index;
  for(int i1 = 0; i1 < layerIndex; i1++)
  {
    sTIDL_DataParams_t * dataPrms = &net->TIDLLayers[i1].outData;
    currOffsetInBytes = tidlFindCurrentOffsetForPerChannelMean(net,
                                                              i1,
                                                              &perChannelMeanMemSize);
    currOffsetInfloats = ( currOffsetInBytes / sizeof(float32_tidl) );
    int32_t numValues = dataPrms->dimValues[TIDL_DIM_NUMCH];
    if(net->TIDLLayers[i1].layerType == TIDL_InnerProductLayer && net->TIDLLayers[i1].layerParams.innerProductParams.constIdx == 1)
    {
      numValues = dataPrms->dimValues[TIDL_DIM_WIDTH];
    }
    for(int i2 = 0; i2 < numValues; i2++)
    {
      if(currOffsetInBytes != -1)    /* if layer is not a data layer */
      {
        meanFloat = perChannelMeanPtrFloat[currOffsetInfloats + i2];
        meanQuantized = perChannelMeanPtrQuantized[currOffsetInfloats + i2];
        perChannelActShift[currOffsetInfloats + i2] = (meanFloat - meanQuantized) * biasCalibrationFactor;
      }
    }
  }
}

void TIDL_updateBiasForBiasCalibration(sTIDL_OrgNetwork_t * pOrgTIDLNetStructure, sTIDL_OrgNetwork_t * pOrgTIDLNetStructureOrig, float32_tidl * perChannelMeanDelta, int32_t layerIndex)
{
  int32_t currOffsetInBytes, currOffsetInfloats, perChannelMeanMemSize;
  for (int i = 0; i < layerIndex; i++)
  {
    if(pOrgTIDLNetStructureOrig->TIDLPCLayers[i].layerType == TIDL_BatchNormLayer && pOrgTIDLNetStructure->TIDLPCLayers[i].actParams.actType <= TIDL_Clip) {
      bool scalarBN = true;
      int32_t idx = 1;
      int32_t dataSize = pOrgTIDLNetStructureOrig->TIDLPCLayers[i].quantParams[TIDL_WEIGHT_QUANT_PARAMS].size;
      void *weightParams = *pOrgTIDLNetStructureOrig->TIDLPCLayers[i].quantParams[TIDL_WEIGHT_QUANT_PARAMS].prmPtr;
      void *biasParams = *pOrgTIDLNetStructureOrig->TIDLPCLayers[i].quantParams[TIDL_BIAS_QUANT_PARAMS].prmPtr;
      while (scalarBN && idx < dataSize)
      {
        if(pOrgTIDLNetStructure->TIDLPCLayers[i].weightsElementSizeInBits == 8) {
          scalarBN = scalarBN && (((int8_t*)weightParams)[idx] == ((int8_t*)weightParams)[0]) && (((int16_t*)biasParams)[idx] == ((int16_t*)biasParams)[0]);
        }
        else {
          scalarBN = scalarBN && (((int16_t*)weightParams)[idx] == ((int16_t*)weightParams)[0]) && (((int16_t*)biasParams)[idx] == ((int16_t*)biasParams)[0]);
        }
        idx++;
      }

      if(scalarBN) {
        continue;
      }
    }
    /*Batchnorm layer being here is suspicious - ESP in activation mode!!*/
    if  ((((pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_ConvolutionLayer) ||
        (pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_Deconv2DLayer)) &&
        (pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.convParams.enableBias)) ||
        (pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_BatchNormLayer && (pOrgTIDLNetStructure->TIDLPCLayers[i].actParams.actType == TIDL_RelU || pOrgTIDLNetStructure->TIDLPCLayers[i].actParams.actType == TIDL_NoAct)) ||
        (pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_InnerProductLayer && pOrgTIDLNetStructure->TIDLPCLayers[i].layerParams.innerProductParams.constIdx == 1) || 
        (pOrgTIDLNetStructure->TIDLPCLayers[i].layerType == TIDL_DetectionOutputLayer)
        )    /* if conv or deconv layer, enableBias must be 1 for update */
    {
      currOffsetInBytes = tidlFindCurrentOffsetForPerChannelMean(&tIDLNetStructure,
                                                              i,
                                                              &perChannelMeanMemSize);
      currOffsetInfloats = ( currOffsetInBytes / sizeof(float32_tidl) );

      float32_tidl * data = (float32_tidl*)pOrgTIDLNetStructure->TIDLPCLayers[i].bias.ptr;
      uint32_t dataSize = pOrgTIDLNetStructure->TIDLPCLayers[i].bias.bufSize;
      if(dataSize > 0)
      {
        for(int i1 = 0; i1 < dataSize; i1++)
        {
          data[i1] += perChannelMeanDelta[currOffsetInfloats + i1];
        }
      }
    }
  }
}

/* This function runs the quant stats tool either in float mode or fixed mode.
   Tensor ranges after running stats tools are updated in pOrgTIDLNetStructure */
int32_t TIDL_quantStatsFixedOrFloat(sTIDL_OrgNetwork_t    * pOrgTIDLNetStructure,
                                  sTIDL_Network_t   * pTIDLNetStructure,
                                  tidl_import_config * configParams,
                                  int32_t statsCollectionType)
{
  int32_t numLayers = pOrgTIDLNetStructure->numLayers;

  if ( statsCollectionType == STATS_COLLECTION_FLOAT)
  {
    tidl_import_config* importConfigParamsFloat = NULL;
    importConfigParamsFloat = (tidl_import_config*)malloc(sizeof(tidl_import_config));
    sTIDL_OrgNetwork_t * pOrgTIDLNetStructureFloat = new sTIDL_OrgNetwork_t;
    if ( pOrgTIDLNetStructureFloat == NULL )
    {
      TIDL_GLOBAL_REPORT_ERROR("TIDL_quantStatsFixedOrFloat: Unable to allocate memory for pOrgTIDLNetStructureFloat ");
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }

    if(importConfigParamsFloat == NULL)
    {
      TIDL_GLOBAL_REPORT_ERROR("TIDL_quantStatsFixedOrFloat: Unable to allocate memory for importConfigParamsFloat");
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }

    *pOrgTIDLNetStructureFloat = *pOrgTIDLNetStructure;
    TIDL_allocAndCopyModelParams(pOrgTIDLNetStructureFloat,
                                                     pOrgTIDLNetStructure,
                                                     numLayers);

    memcpy(importConfigParamsFloat, configParams, sizeof(tidl_import_config));

    importConfigParamsFloat->numParamBits = 32;
    importConfigParamsFloat->numFeatureBits = 32;

    for (int i = 0; i < TIDL_MAX_ALG_IN_BUFS; i++)
    {
      importConfigParamsFloat->inElementType[i] = TIDL_SinglePrecFloat;
    }
    for (int i = 0; i < TIDL_MAX_ALG_OUT_BUFS; i++)
    {
      importConfigParamsFloat->outElementType[i] = TIDL_SinglePrecFloat;
    }
    tidl_updateWeightElemSize(pOrgTIDLNetStructureFloat, importConfigParamsFloat, numLayers);
    tidl_convertElementTypeGivenParambits(pOrgTIDLNetStructureFloat, numLayers, 32);
    TIDL_IMPORT_CHECK_AND_RETURN(updatePadAndWriteModel(pOrgTIDLNetStructureFloat, pTIDLNetStructure, importConfigParamsFloat), "");
    pTIDLNetStructure->isQuantStatsAvailable = 0;

    /* Call the stats collection in float mode */
    TIDL_IMPORT_CHECK_AND_RETURN(tidlRunQuantStatsTool(pOrgTIDLNetStructureFloat,
                                      pTIDLNetStructure,
                                      importConfigParamsFloat,
                                      numLayers), "");

    TIDL_copyTensorStats(pOrgTIDLNetStructure, pOrgTIDLNetStructureFloat, 1);

    TIDL_freeModelParams(pOrgTIDLNetStructureFloat, numLayers);
    if ( pOrgTIDLNetStructureFloat != NULL )
    {
      delete pOrgTIDLNetStructureFloat;
    }

    if ( importConfigParamsFloat != NULL )
    {
      free(importConfigParamsFloat);
      importConfigParamsFloat = NULL;
    }
  }
  else
  {
    TIDL_IMPORT_CHECK_AND_RETURN(TIDL_importQuantLayerParams(pOrgTIDLNetStructure,
                                    pTIDLNetStructure,
                                    configParams,
                                    numLayers), "");
    
    /* Wrapper Function to fill the lut Data values*/
    int32_t tidlNetVersion = tIDLNetStructure.netVersion;
    TIDL_prepareLUTForActivations_wrapper(pOrgTIDLNetStructure, tidlNetVersion, &gParams);

    TIDL_IMPORT_CHECK_AND_RETURN(updatePadAndWriteModel(pOrgTIDLNetStructure, pTIDLNetStructure, configParams), "");
    pTIDLNetStructure->isQuantStatsAvailable = 0;
    pOrgTIDLNetStructure->quantStats = TIDL_QUANT_STATS_NONE;
    /* Call the stats collection in fixed point mode mode */
    TIDL_IMPORT_CHECK_AND_RETURN(tidlRunQuantStatsTool(pOrgTIDLNetStructure,
                                      pTIDLNetStructure,
                                      configParams,
                                      numLayers), "");
    
  }

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}
#define TIDL_PI (3.141593)
#define TIDL_BIAS_CALIBRATION_WARMUP_FACTOR (10.0)
#define TIDL_BIAS_CALIBRATION_USE_COSINE_DECAY (0)
#define TIDL_GRID_INPUT_SCALE_8BIT (16384U)
#define TIDL_GRID_INPUT_SCALE_16BIT (16384U)
#define TIDL_DCN_OFFSET_MAX (+31.0)
#define TIDL_DCN_OFFSET_MIN (-32.0)

int32_t TIDL_increaseWeightPrecision(sTIDL_LayerPC_t * layer, int32_t weightsElementSizeInBits)
{
  int32_t retVal = weightsElementSizeInBits;

  if ( weightsElementSizeInBits <= 8 )
  {
    retVal = 16;
  }
  else
  {
    retVal = weightsElementSizeInBits;
  }

  return retVal;
}

/*Quantization Constraints:*/
int32_t TIDL_updateNetworkWithQuantizationConstraints(sTIDL_OrgNetwork_t * pOrgTIDLNetStructure,
                              tidl_import_config * params)
{
  /*This function analyses producer/consumer layers and appropriately populates the "tensorType" and datatype property*/
  int32_t layerIdx;
  int32_t outDataId;
  sTIDL_LayerPC_t * currLayer, *gridLayer, *offsetLayer, *maskLayer;
  int32_t outBufIdx;
  bool isAsymSupported = false;
  bool areConsumersAsym = false;
  int32_t numLayers = pOrgTIDLNetStructure->numLayers;
  for( layerIdx = 0; layerIdx < numLayers; layerIdx++)
  {
    currLayer = &pOrgTIDLNetStructure->TIDLPCLayers[layerIdx];
    outDataId = currLayer->outData[0].dataId;
    /*Set defaults:*/
    currLayer->quantizeConstraint = TIDL_QuantizeUnconstrained;
    if(currLayer->layerType == TIDL_GridSampleLayer)
    {
      /*Traverse inputs and find the first consumer which satisfy grid constraints:*/
      int32_t gridLayerIdx = tidl_getInLayer(*pOrgTIDLNetStructure,  numLayers, currLayer->inData[1].dataId);
      gridLayer = &pOrgTIDLNetStructure->TIDLPCLayers[gridLayerIdx];
      int32_t traverseCount = 0;
      while(gridLayerIdx != -1 && TIDL_isGridProducerLayer(gridLayer, params) == 0 && TIDL_isGridProducerInputLayer(gridLayer, params) == 0)
      {
        gridLayerIdx = gridLayer->inData[0].dataId;
        /*Increase precision of passthrough layers:*/
        gridLayer->outData[0].elementType = TIDL_increasePrecision(gridLayer->outData[0].elementType);
        gridLayer->weightsElementSizeInBits = TIDL_increaseWeightPrecision(gridLayer, gridLayer->weightsElementSizeInBits);
        if(gridLayer->layerType == TIDL_BatchNormLayer && 0)
        {
          /*Special case for BN layer (x2 -1 : Range: -1,+1 always)*/
          gridLayer->outData[0].tensorScale = TIDL_GRID_INPUT_SCALE_16BIT;
          gridLayer->outData[0].elementType = TIDL_SignedShort;
          gridLayer->outData[0].tensorZeroPoint = 0;
          gridLayer->actParams.actType = TIDL_Clip;
          gridLayer->actParams.clipMin = -1.0f;
          gridLayer->actParams.clipMax = 1.0f;
          gridLayer->quantizeConstraint = TIDL_QuantizeConstainedP2;
        }
        gridLayerIdx = tidl_getInLayer(*pOrgTIDLNetStructure, numLayers, gridLayerIdx);
        gridLayer = &pOrgTIDLNetStructure->TIDLPCLayers[gridLayerIdx];
        traverseCount++;
        if(traverseCount > TIDL_MAX_GRID_TRAVERSAL)
        {
          TIDL_GLOBAL_REPORT_ERROR("Infinite loop in grid layer traversal");
          return -1;
        }
      }
      if(gridLayerIdx == -1)
      {
        TIDL_GLOBAL_REPORT_ERROR("Unable to find grid inputs for gridsample layer : %s", currLayer->outDataNames[0]);
        return -1;
      }
      else
      {
        /*Update with GS Constraints:*/
        if(params->numParamBits <= 8)
        {
          gridLayer->outData[0].tensorScale = TIDL_GRID_INPUT_SCALE_8BIT;
          gridLayer->outData[0].elementType = TIDL_SignedShort;
          gridLayer->weightsElementSizeInBits = TIDL_increaseWeightPrecision(gridLayer, gridLayer->weightsElementSizeInBits);
          gridLayer->outData[0].tensorZeroPoint = 0;
          gridLayer->quantizeConstraint = TIDL_QuantizeConstainedP2;
          if(gridLayer->layerType == TIDL_EltWiseLayer && 0) /*TEMP!*/
          {
            /*Update skip connection properties*/
            gridLayer->actParams.actType = TIDL_Clip;
            gridLayer->actParams.clipMin = 0.0f;
            gridLayer->actParams.clipMax = 1.0f;
            gridLayer->outData[0].elementType = TIDL_UnsignedShort;
            gridLayer->quantizeConstraint = TIDL_QuantizeUnconstrained;
          }
        }
        else if(params->numParamBits <= 16)
        {
          gridLayer->outData[0].tensorScale = TIDL_GRID_INPUT_SCALE_16BIT;
          gridLayer->outData[0].elementType = TIDL_SignedShort;
          gridLayer->outData[0].tensorZeroPoint = 0;
          gridLayer->quantizeConstraint = TIDL_QuantizeConstainedP2;
          if(gridLayer->layerType == TIDL_EltWiseLayer && 0) /*TEMP!*/
          {
            /*Update skip connection properties*/
            gridLayer->actParams.actType = TIDL_Clip;
            gridLayer->actParams.clipMin = 0.0f;
            gridLayer->actParams.clipMax = 1.0f;
            gridLayer->outData[0].elementType = TIDL_UnsignedShort;
            gridLayer->quantizeConstraint = TIDL_QuantizeUnconstrained;
          }
        }
        gridLayer->weightsElementSizeInBits = TIDL_increaseWeightPrecision(gridLayer, gridLayer->weightsElementSizeInBits);
      }
    }
    else if(currLayer->layerType == TIDL_DeformableConvLayer)
    {
      /*Enforce power of 2 constraint on DCN's offsets & sigmoid output*/
      /*Add P2 constraint for offsets:*/
      int32_t offsetLayerIdx = tidl_getInLayer(*pOrgTIDLNetStructure,  numLayers, currLayer->inData[1].dataId);
      offsetLayer = &pOrgTIDLNetStructure->TIDLPCLayers[offsetLayerIdx];
      while(offsetLayerIdx != -1 && (TIDL_canUpdateTensorScale(offsetLayer, params) == 0 || offsetLayer->layerType == TIDL_DataConvertLayer || offsetLayer->layerType == TIDL_TransposeLayer))
      {
        offsetLayerIdx = offsetLayer->inData[0].dataId;
        offsetLayerIdx = tidl_getInLayer(*pOrgTIDLNetStructure, numLayers, offsetLayerIdx);
        offsetLayer = &pOrgTIDLNetStructure->TIDLPCLayers[offsetLayerIdx];
      }
      if(offsetLayerIdx == -1)
      {
        return -1;
      }
      else
      {
        float32_tidl minVal, maxVal;
        minVal = offsetLayer->outData[0].minTensorValue;
        maxVal = offsetLayer->outData[0].maxTensorValue;
        /*Enforce boundary condition for DCN offsets:*/
        if(maxVal > TIDL_DCN_OFFSET_MAX)
        {
          TIDL_GLOBAL_REPORT_WARNING("DCN: Offset maximum values being clipped for layer %s", (char*)offsetLayer->outDataNames[0]);
          maxVal = TIDL_DCN_OFFSET_MAX;
        }
        if(minVal < TIDL_DCN_OFFSET_MIN)
        {
          TIDL_GLOBAL_REPORT_WARNING("DCN: Offset minimum values being clipped for layer %s", (char*)offsetLayer->outDataNames[0]);
          minVal = TIDL_DCN_OFFSET_MIN;
        }
        /*Update with DCN Constraints:*/
        if(params->numParamBits <= 8)
        {
          TIDL_rangeToScaleP2(&offsetLayer->outData[0].tensorScale, minVal, maxVal, 4U, TIDL_SignedChar);
          offsetLayer->outData[0].elementType = TIDL_SignedChar;
          offsetLayer->outData[0].tensorZeroPoint = 0;
          // offsetLayer->outData[0].roundBits = round(log(offsetLayer->outData[0].tensorScale)/log(2));
          offsetLayer->quantizeConstraint = TIDL_QuantizeConstainedP2;
        }
        else if(params->numParamBits <= 16)
        {
          TIDL_rangeToScaleP2(&offsetLayer->outData[0].tensorScale, minVal, maxVal, 8U, TIDL_SignedShort);
          offsetLayer->outData[0].elementType = TIDL_SignedShort;
          offsetLayer->outData[0].tensorZeroPoint = 0;
          // offsetLayer->outData[0].roundBits = round(log(offsetLayer->outData[0].tensorScale)/log(2));
          offsetLayer->quantizeConstraint = TIDL_QuantizeConstainedP2;
        }
      }
      
      /*Add P2 constraint for mask:*/
      int32_t maskLayerIdx = tidl_getInLayer(*pOrgTIDLNetStructure,  numLayers, currLayer->inData[2].dataId);
      maskLayer = &pOrgTIDLNetStructure->TIDLPCLayers[maskLayerIdx];
      while(maskLayerIdx != -1 && (TIDL_canUpdateTensorScale(maskLayer, params) == 0 || maskLayer->layerType == TIDL_DataConvertLayer || maskLayer->layerType == TIDL_TransposeLayer))
      {
        maskLayerIdx = maskLayer->inData[0].dataId;
        maskLayerIdx = tidl_getInLayer(*pOrgTIDLNetStructure, numLayers, maskLayerIdx);
        maskLayer = &pOrgTIDLNetStructure->TIDLPCLayers[maskLayerIdx];
      }
      if(maskLayerIdx == -1)
      {
        return -1;
      }
      else
      {
        /*Update with DCN Constraints:*/
        if(params->numParamBits <= 8)
        {
          TIDL_rangeToScaleP2(&maskLayer->outData[0].tensorScale, maskLayer->outData[0].minTensorValue, maskLayer->outData[0].maxTensorValue, 8U, TIDL_UnsignedChar);
          maskLayer->outData[0].elementType = TIDL_UnsignedChar;
          maskLayer->outData[0].tensorZeroPoint = 0;
          // maskLayer->outData[0].roundBits = round(log(maskLayer->outData[0].tensorScale)/log(2));
          maskLayer->quantizeConstraint = TIDL_QuantizeConstainedP2;
        }
        else if(params->numParamBits <= 16)
        {
          TIDL_rangeToScaleP2(&maskLayer->outData[0].tensorScale, maskLayer->outData[0].minTensorValue, maskLayer->outData[0].maxTensorValue, 8U, TIDL_UnsignedShort);
          maskLayer->outData[0].elementType = TIDL_UnsignedShort;
          maskLayer->outData[0].tensorZeroPoint = 0;
          // maskLayer->outData[0].roundBits = round(log(maskLayer->outData[0].tensorScale)/log(2));
          maskLayer->quantizeConstraint = TIDL_QuantizeConstainedP2;
        }
      }
    }
  }
  return 0;
}


int32_t TIDL_runIterativeCalibration(sTIDL_OrgNetwork_t * pOrgTIDLNetStructure,
                                               sTIDL_Network_t       * pTIDLNetStructure,
                                               tidl_import_config * configParams
                                               )
{
    sTIDL_OrgNetwork_t * pOrgTIDLNetStructureBkpFloat = NULL;
    float32_tidl     * perChannelMeanPtrFloat = NULL;
    float32_tidl     * perChannelMeanPtrQuantized = NULL;
    float32_tidl     * perChannelMeanDelta = NULL;
    float32_tidl biasCalibrationFactor;
    int32_t perChannelMeanMemSize;
    int32_t numFramesOrig;
    float quantRangeUpdateFactorOrig;
    int32_t numLayers;
    int32_t numBiasCalibFrames;
    const float32_tidl math_pi = TIDL_PI;
    float32_tidl biasCalibrationFactorStart = configParams->biasCalibrationFactor;
    float32_tidl biasCalibrationFactorEnd = 1e-4;
    float32_tidl curIteration;
    const float32_tidl warmupFactor = TIDL_BIAS_CALIBRATION_WARMUP_FACTOR;
    int warmpIters;

    pOrgTIDLNetStructureBkpFloat = new sTIDL_OrgNetwork_t;

    /* Set number of frames for bias calibration */
    numFramesOrig = configParams->numFrames;
    quantRangeUpdateFactorOrig = configParams->quantRangeUpdateFactor;

    numBiasCalibFrames = configParams->numFramesBiasCalibration;

    if (configParams->biasCalibrationIterations == -1)
    {
      if((configParams->calibrationOption == 0) || (configParams->calibrationOption == TIDL_CalibOptionBiasRange))
      {
        configParams->biasCalibrationIterations = 1;
      }
      else
      {
        configParams->biasCalibrationIterations = 50;
      }
    }
    if (configParams->numFramesBiasCalibration == -1)
    {
      if ((configParams->inFileFormat == 2) || (configParams->inFileFormat == 5))
      {
        if (configParams->numFrames == -1)
        {
          numBiasCalibFrames = getNumberOfLinesIntheFile((char *)configParams->inData);
        }
        else
        {
          numBiasCalibFrames = configParams->numFrames;
        }
      }
      else if(((configParams->modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE_RT) || (configParams->modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX_RT)) && (configParams->inFileFormat == 1))
      {
        numBiasCalibFrames = configParams->numFrames;
      }
      else if (configParams->inFileFormat == 1)
      {
        /* Raw data format keep default same as number of frames*/
        numBiasCalibFrames = configParams->numFrames;
      }
      else if ((configParams->inFileFormat == 0) || (configParams->inFileFormat == 3))
      {
        numBiasCalibFrames = 1;
      }
    }

    configParams->numFrames = numBiasCalibFrames;

    numLayers = pOrgTIDLNetStructure->numLayers;

    /* At this point TIDLNetStructure is not populated so copy PC net to device net */
    tidl_copyPCNetToDeviceNet(pOrgTIDLNetStructure,
                                              pTIDLNetStructure,
                                              configParams,
                                              numLayers);

    pTIDLNetStructure->numLayers = numLayers;

    if (( configParams->calibrationOption & TIDL_CalibOptionBiasCalibration) == TIDL_CalibOptionBiasCalibration)
    {
      /* Allocate memory for perChannelMeanDelta */
      tidlFindCurrentOffsetForPerChannelMean(pTIDLNetStructure, 0, &perChannelMeanMemSize);

      perChannelMeanPtrFloat = (float32_tidl*)malloc(perChannelMeanMemSize);
      if(perChannelMeanPtrFloat == NULL)
      {
        TIDL_GLOBAL_REPORT_ERROR("TIDL_runBiasCalibration - Not enough memory available perChannelMeanPtrFloat");
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }

      perChannelMeanPtrQuantized = (float32_tidl*)malloc(perChannelMeanMemSize);
      if(perChannelMeanPtrQuantized == NULL)
      {
        TIDL_GLOBAL_REPORT_ERROR("TIDL_runBiasCalibration - Not enough memory available for perChannelMeanPtrQuantized");
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }

      perChannelMeanDelta = (float32_tidl*)malloc(perChannelMeanMemSize);
      if(perChannelMeanDelta == NULL)
      {
        TIDL_GLOBAL_REPORT_ERROR("TIDL_runBiasCalibration - Not enough memory available for mean delta");
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }
      else
      {
        memset(perChannelMeanDelta, 0, perChannelMeanMemSize);
      }
    }

     *pOrgTIDLNetStructureBkpFloat = *pOrgTIDLNetStructure;
     /* Allocate separate memory for backing up parameters before quantization */
     TIDL_allocAndCopyModelParams(pOrgTIDLNetStructureBkpFloat, pOrgTIDLNetStructure, numLayers);

     /* Run Stats collection in float to find the original per channel mean */
     TIDL_IMPORT_CHECK_AND_RETURN(TIDL_quantStatsFixedOrFloat((pOrgTIDLNetStructure),
                                 (pTIDLNetStructure),
                                 configParams,
                                 STATS_COLLECTION_FLOAT), "");
     /*Apply layer constraints:*/
     TIDL_IMPORT_CHECK_AND_RETURN(TIDL_updateNetworkWithQuantizationConstraints(&orgTIDLNetStructure, &gParams ), "");
     if (( configParams->calibrationOption & TIDL_CalibOptionBiasCalibration) == TIDL_CalibOptionBiasCalibration)
     {
       /*Read the original float mean stats per channel */
       TIDL_IMPORT_CHECK_AND_RETURN(tidlReadPerChannelMeanStatistics(pTIDLNetStructure,
                                        perChannelMeanPtrFloat,
                                        perChannelMeanMemSize,
                                        configParams), "");
    }

    warmpIters = configParams->biasCalibrationIterations/10;

    /*****    Loop for bias calibration  *****/
    for(int i = 0; i < configParams->biasCalibrationIterations; i++)
    {
      /* Run quant stats tool to get per channel mean means after quantization */
      TIDL_GLOBAL_REPORT_SUBHEADING(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, TIDL_ModelDiagnostic::DK_NoColor, "Fixed-point Calibration Iteration [%d / %d]:", (i + 1U), configParams->biasCalibrationIterations);
      TIDL_IMPORT_CHECK_AND_RETURN(TIDL_quantStatsFixedOrFloat(pOrgTIDLNetStructure,
                                  pTIDLNetStructure,
                                  configParams,
                                  STATS_COLLECTION_FIXED_POINT), "");
      if (( configParams->calibrationOption & TIDL_CalibOptionBiasCalibration) == TIDL_CalibOptionBiasCalibration)
      {
        /*Read the per Channel  mean stats per channel */
       TIDL_IMPORT_CHECK_AND_RETURN(tidlReadPerChannelMeanStatistics(pTIDLNetStructure,
                                        perChannelMeanPtrQuantized,
                                        perChannelMeanMemSize,
                                        configParams), "");
        curIteration = (float32_tidl)i;

#if TIDL_BIAS_CALIBRATION_USE_COSINE_DECAY
        biasCalibrationFactor = biasCalibrationFactorEnd +
                                0.5*(biasCalibrationFactorStart - biasCalibrationFactorEnd) *
                                (1.0 + std::cos(math_pi* curIteration /configParams->biasCalibrationIterations));
#else
        biasCalibrationFactor = configParams->biasCalibrationFactor;
        if ( warmpIters > 0 )
        {
          if((curIteration < warmpIters) ||
            ( curIteration >= (configParams->biasCalibrationIterations - warmpIters)))
          {
            biasCalibrationFactor = (biasCalibrationFactor / warmupFactor);
          }
        }
#endif
        /* Mean delta calculation */
        TIDL_computeMeanActivationShift(perChannelMeanDelta,
                                        perChannelMeanPtrFloat,
                                        perChannelMeanPtrQuantized,
                                        biasCalibrationFactor,
                                        numLayers);
        /* Update bias as per the mean shift observed */
        TIDL_updateBiasForBiasCalibration(pOrgTIDLNetStructureBkpFloat, pOrgTIDLNetStructure, perChannelMeanDelta, numLayers);
      }

      /* Copy the updated bias to orgTIDLNetStructure, which will be used in next iteration for quantization */
      TIDL_copyModelParams(pOrgTIDLNetStructure, pOrgTIDLNetStructureBkpFloat, numLayers);

      //TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "\n \n \n *****************   Calibration iteration number %d completed ************************ \n \n \n \n", i);
    }
    /******   End of bias calibration loop  *******/

    if ( perChannelMeanPtrFloat != NULL )
    {
      my_free(perChannelMeanPtrFloat );
    }

    if ( perChannelMeanPtrQuantized != NULL )
    {
      my_free(perChannelMeanPtrQuantized );
    }

    if (perChannelMeanDelta != NULL)
    {
      my_free(perChannelMeanDelta);
    }

    TIDL_freeModelParams(pOrgTIDLNetStructureBkpFloat, numLayers);
    if ( pOrgTIDLNetStructureBkpFloat != NULL )
    {
      delete pOrgTIDLNetStructureBkpFloat;
    }

    /* revert to original number of frames */
    configParams->numFrames = numFramesOrig;
    configParams->quantRangeUpdateFactor = quantRangeUpdateFactorOrig;


    return 0;
}


static int32_t TIDL_isIterativeCalibrationRequired(tidl_import_config * configParams)
{
  int32_t isRequired = 0;
  if (( configParams->calibrationOption & TIDL_CalibOptionBiasCalibration) ==
                                          TIDL_CalibOptionBiasCalibration)
  {
    isRequired  = 1;
  }
  else if (( configParams->calibrationOption & TIDL_CalibOptionPerChannelWeightQuantization) ==
                                              TIDL_CalibOptionPerChannelWeightQuantization)
  {
    isRequired = 1;
  }
  else if (( configParams->calibrationOption & TIDL_CalibOptionActivationRange ) ==
                                              TIDL_CalibOptionActivationRange)
  {
    if ( configParams->activationRangeMethod == TIDL_ActivationRangeMethodGlobalHistogram)
    {
      isRequired  = 1;
    }
  }
  else
  {
    isRequired  = 0;
  }

  return isRequired;
}



int32_t TIDL_convertElementTypeToSigned(int32_t elementType)
{
  int32_t outElemType = elementType;
  if ( elementType == TIDL_UnsignedChar )
  {
    outElemType = TIDL_SignedChar;
  }
  else if ( elementType == TIDL_UnsignedShort )
  {
    outElemType = TIDL_SignedShort;
  }
  return outElemType;
}

int32_t TIDL_increasePrecision(int32_t elementType)
{
  int32_t outElemType;
  if ( elementType == TIDL_SignedChar )
  {
    outElemType = TIDL_SignedShort;
  }
  else if ( elementType == TIDL_UnsignedChar )
  {
    outElemType = TIDL_UnsignedShort;
  }
  else
  {
    outElemType = elementType;
  }

  return outElemType;
}

/* Function checks if a given dataId is the final output of the network and returns -1 if it is not
otherwise returns the corresponding output buffer index */
int32_t TIDL_isLayerNetworkOutput(sTIDL_OrgNetwork_t * pOrgTIDLNetStructure, int32_t dataId)
{
  int32_t layerIdx, inBufIdx, userOutBufIdx;
  sTIDL_LayerPC_t * currLayer;

  userOutBufIdx  = -1;

  /* Go through all the layers */
  for ( layerIdx = 0; layerIdx < pOrgTIDLNetStructure->numLayers; layerIdx++)
  {
    currLayer = &pOrgTIDLNetStructure->TIDLPCLayers[layerIdx];
    /* Find layers whos input contains the data Id. This will indicate the producer
    layer's output goes to this layer */
    for ( inBufIdx = 0; inBufIdx < currLayer->numInBufs;inBufIdx++)
    {
      if ( dataId == currLayer->inData[inBufIdx].dataId )
      {
        /* Check if the consumer layer is a data layer. This will indicate
        that this is the final output of the network */
        if ( currLayer->layerType == TIDL_DataLayer )
        {
          char * consumerLayerName = (char *)&currLayer->outDataNames[0][0];

          for (userOutBufIdx = 0; userOutBufIdx < numNetOutData; userOutBufIdx++)
          {
            if (strcmp(consumerLayerName, outDataNames[userOutBufIdx]) == 0)
            {
              break;
            }
          }
          goto Exit;
        }
      }
    }
  }

Exit:

  return  userOutBufIdx;
}

int32_t TIDL_checkConsumerProducerDataType(sTIDL_OrgNetwork_t * pOrgTIDLNetStructure,
                                                         int32_t producerLayerIdx)
{
  int32_t layerIdx;
  sTIDL_LayerPC_t * currLayer;
  sTIDL_LayerPC_t * producerLayer;
  int32_t producerElemType;
  int32_t producerDataId;
  int32_t inIdx;
  int32_t updated = 0;

  producerLayer = &pOrgTIDLNetStructure->TIDLPCLayers[producerLayerIdx];
  producerElemType = producerLayer->outData[0].elementType;
  producerDataId   = producerLayer->outData[0].dataId;

  if ( producerLayer->numOutBufs > 0)
  {
    /* Go through all the layers */
    for ( layerIdx = 0; layerIdx < pOrgTIDLNetStructure->numLayers; layerIdx++)
    {
      currLayer = &pOrgTIDLNetStructure->TIDLPCLayers[layerIdx];
      /* Find layers whos input contains the data Id. This will indicate the producer
      layer's output goes to this layer */
      for (inIdx = 0; inIdx < currLayer->numInBufs; inIdx++)
      {
        /* Indicates one of the consumer layer is found */
        if ( producerDataId == currLayer->inData[inIdx].dataId)
        {
          /* Check the element type of producer and consumer are same or not. If not then it
          indicates mismatch which should be corrected*/
          if ( producerElemType != currLayer->inData[inIdx].elementType)
          {
            /* producer 8 bit and consumer expects 16 bit, make producer output to be 16 bit */
            if ( producerElemType <= TIDL_SignedChar )
            {
              producerLayer->outData[0].elementType = TIDL_increasePrecision(producerLayer->outData[0].elementType);
              producerLayer->weightsElementSizeInBits = TIDL_increaseWeightPrecision(producerLayer,
                                                              producerLayer->weightsElementSizeInBits );
              /* Check if producer layer supports mixed precision, if not then make output and input to be same
              bit depth*/
              if (TIDL_doesLayerSupportMixedPrecision(producerLayer) == 0 )
              {
                int32_t i;
                for ( i = 0; i < producerLayer->numInBufs; i++)
                {
                  producerLayer->inData[i].elementType = TIDL_increasePrecision(producerLayer->inData[i].elementType);
                }
              }

            }
            else
            {
              /* Producer is 16 bit and consumer is 8bit, make consumer's all inputs to 16 bit */
              int32_t i;
              currLayer->weightsElementSizeInBits = TIDL_increaseWeightPrecision(currLayer,
                                                              currLayer->weightsElementSizeInBits );
              for ( i = 0; i < currLayer->numInBufs; i++)
              {
                currLayer->inData[i].elementType = TIDL_increasePrecision(currLayer->inData[i].elementType);
              }
              /* Check if consumer layer supports mixed precision, if not then make output and input to be same
              bit depth*/
              if (TIDL_doesLayerSupportMixedPrecision(currLayer) == 0 )
              {
                currLayer->outData[0].elementType = TIDL_increasePrecision(currLayer->outData[0].elementType);
              }
            }
            updated = 1;
          }
        }
      }

      if ( updated == 1 )
      {
        break;
      }
    }
  }
  return updated;
}


int32_t TIDL_isAllFeatureRangeAvailable(sTIDL_OrgNetwork_t *pOrgNetStructure, int32_t layerIndex)
{
  int32_t i, featureRangeAvailable = 1;

  for (i = 0; i < layerIndex; i++)
  {
    if ((pOrgNetStructure->TIDLPCLayers[i].layerType != TIDL_DataLayer) && (pOrgNetStructure->TIDLPCLayers[i].actParams.actType != TIDL_Clip)
        && (pOrgNetStructure->TIDLPCLayers[i].layerType != TIDL_PadLayer)
        && (
          ( pOrgNetStructure->TIDLPCLayers[i].layerType != TIDL_BatchNormLayer) ||
          (pOrgNetStructure->TIDLPCLayers[i].actParams.actType == TIDL_PRelU && !gParams.preQuantizedModel) ||
           (pOrgNetStructure->TIDLPCLayers[i].actParams.actType == TIDL_Sigmoid && !gParams.preQuantizedModel)  ||
           (pOrgNetStructure->TIDLPCLayers[i].actParams.actType == TIDL_Tanh && !gParams.preQuantizedModel) ||
           (pOrgNetStructure->TIDLPCLayers[i].actParams.actType == TIDL_HardSigmoid && !gParams.preQuantizedModel)  ||
           (pOrgNetStructure->TIDLPCLayers[i].actParams.actType == TIDL_ELU && !gParams.preQuantizedModel)
           )
        && (pOrgNetStructure->TIDLPCLayers[i].layerType != TIDL_DataConvertLayer) && (pOrgNetStructure->TIDLPCLayers[i].layerType != TIDL_ConstDataLayer)
        && (pOrgNetStructure->TIDLPCLayers[i].layerType != TIDL_ReshapeLayer)
        && (pOrgNetStructure->TIDLPCLayers[i].layerType != TIDL_SliceLayer)
        && (pOrgNetStructure->TIDLPCLayers[i].layerType != TIDL_TransposeLayer)
        && (!( (pOrgNetStructure->TIDLPCLayers[i].layerType == TIDL_CropLayer) && (pOrgNetStructure->TIDLPCLayers[i].layerParams.cropParams.multiCoreMode != TIDL_NOT_MULTI_CORE) ))
    )
        /* For pre-calibrated models, layers added by TIDL should be eliminated from this check to ensure bias calibration is not run on these models
        due to featureRangeAvailable set to 0 in this function */
    {
      TIDL_GLOBAL_REPORT_WARNING("Range not supplied for layer - %s - %s, running calibration\n",  TIDL_LayerString[pOrgNetStructure->TIDLPCLayers[i].layerType], (char*)pOrgNetStructure->TIDLPCLayers[i].outDataNames[0]);
      featureRangeAvailable = 0;
    }
  }
  if(featureRangeAvailable == 1)
  {
    //Shouldn't mutate for QDQ:
    if (pOrgNetStructure->TIDLPCLayers[i].layerType != TIDL_DataLayer)
    {
      pOrgNetStructure->TIDLPCLayers[i].outData[0].minTensorValue = pOrgNetStructure->TIDLPCLayers[i].actParams.clipMin;
      pOrgNetStructure->TIDLPCLayers[i].outData[0].maxTensorValue = pOrgNetStructure->TIDLPCLayers[i].actParams.clipMax;
      float max = fabs(pOrgNetStructure->TIDLPCLayers[i].outData[0].minTensorValue) > fabs(pOrgNetStructure->TIDLPCLayers[i].outData[0].maxTensorValue) ?
                  fabs(pOrgNetStructure->TIDLPCLayers[i].outData[0].minTensorValue) : fabs(pOrgNetStructure->TIDLPCLayers[i].outData[0].maxTensorValue);
      max = (float)ceil(log((double)max) / log((double)2));
      max = pow(2.0, (double)max);
      if (max != 0)
      {
        pOrgNetStructure->TIDLPCLayers[i].outData[0].tensorScale = ((1.0*(1 << (NUM_WHGT_BITS - 1))) / max);
      }
      else
      {
        pOrgNetStructure->TIDLPCLayers[i].outData[0].tensorScale = 1.0;
      }

    }
  }
  return (featureRangeAvailable);
}

void TIDL_setDefaultWeightElementBits(sTIDL_OrgNetwork_t * pOrgTIDLNetStructure,
                                           tidl_import_config * params,
                                           int32_t numLayers)
{
  int32_t layerIdx;
  sTIDL_LayerPC_t * currLayer;
  int32_t inIdx;
  /* This is required because in mixed precision weightElementSizeInBits is used to decide
  the processing size */
  for ( layerIdx = 0; layerIdx < numLayers; layerIdx++)
  {
    currLayer = &pOrgTIDLNetStructure->TIDLPCLayers[layerIdx];
    /* Set default weightElementSizeInBits based on original precision */
    currLayer->weightsElementSizeInBits = NUM_WHGT_BITS;

    if ( currLayer->layerType == TIDL_DataConvertLayer )
    {
        /* If for data convert layer input is float then processing is
        expected to be in float and hence set weightsElementSizeInBits
        to 32 bits as it indicates the size of processing */
      currLayer->weightsElementSizeInBits = tidl_getElementSizeInBits(currLayer->inData[0].elementType);
    }

    for ( inIdx = 0;inIdx < currLayer->numInBufs; inIdx++)
    {
      if (( currLayer->outData[0].elementType != TIDL_SinglePrecFloat ) &&
          ( currLayer->inData[inIdx].elementType != TIDL_SinglePrecFloat ))
      {
        /* Data Convert layer are not handled via mixed precision flow */
        /* Scatter/Gather weightsElementSizeInBits is wrongly increased because of indices type (int32_t), avoiding it */
        if ( currLayer->layerType != TIDL_DataConvertLayer && currLayer->layerType != TIDL_ScatterElementsLayer && currLayer->layerType != TIDL_GatherLayer )
        {
          /* If input and output data size of any layer is different then
          increase weight preicision to indicate mixed precision */
          if ( tidl_getElementSizeInBits(currLayer->outData[0].elementType) !=
                tidl_getElementSizeInBits(currLayer->inData[inIdx].elementType) )
          {
            currLayer->weightsElementSizeInBits = TIDL_increaseWeightPrecision(currLayer, currLayer->weightsElementSizeInBits);
          }
        }
        else if(currLayer->layerType == TIDL_GatherLayer)
        {
          currLayer->weightsElementSizeInBits = 32;
        }
      }
    }
  }
}


bool TIDL_canPropagateAsym(sTIDL_OrgNetwork_t *pOrgTIDLNetStructure, sTIDL_LayerPC_t * currLayer)
{
  if(quantizationPassThroughMap.find(currLayer->outData[0].dataId) != quantizationPassThroughMap.end())
  {
    return quantizationPassThroughMap[currLayer->outData[0].dataId];
  }
  int32_t layerIdx = TIDL_getLayerIdx(pOrgTIDLNetStructure, pOrgTIDLNetStructure->numLayers, (char*)currLayer->outDataNames[0]);
  bool canPropagate = TIDL_areConsumerLayersAsym(pOrgTIDLNetStructure, layerIdx);

  return quantizationPassThroughMap[currLayer->outData[0].dataId] = canPropagate;
}

bool TIDL_isExecutionTIDLRT (int32_t modelType)
{
  bool isTIDLRT = false;
  if ( (modelType == TIDL_IMPORT_MODEL_FORMAT_CAFFE) ||
       (modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX)  ||
       (modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE) ||
       (modelType == TIDL_IMPORT_MODEL_FORMAT_TENSORFLOW))
  {
    isTIDLRT = true;
  }
  return isTIDLRT;
}

/*Asymmetric utility functions:*/
bool TIDL_doesLayerSupportAsymTensors(sTIDL_LayerPC_t * currLayer, sTIDL_OrgNetwork_t *pOrgTIDLNetStructure)
{
  /*This function checks if the given layerType supports asymmetric tensors*/
  bool isAsymSupported = false;

  /*Disabling asymmetric flow for j721e/tda4vm*/
  if((gParams.deviceName & (~TIDL_OTF_FLAG_BIT)) == TIDL_TDA4VM)
  {
    isAsymSupported = false;
    return isAsymSupported;
  }

  int32_t inElemSize = tidl_getElementSizeInBits(currLayer->inData[0].elementType);
  int32_t outElemSize = tidl_getElementSizeInBits(currLayer->outData[0].elementType);
  int32_t isMixedPrecision = 0;
  if (currLayer->inData[0].elementType != TIDL_SinglePrecFloat && currLayer->outData[0].elementType != TIDL_SinglePrecFloat)
  {
    isMixedPrecision = inElemSize != outElemSize ? 1 : 0;
  }
  if(currLayer->layerType == TIDL_ConvolutionLayer && (currLayer->weightsElementSizeInBits == 8U || (TIDL_isSupportedInFirmwareVersion((const char*)gParams.c7xFirmwareVersion,"11_00_08_00") && currLayer->weightsElementSizeInBits == 16U)))
  {
    if (isMixedPrecision && currLayer->layerParams.convParams.numGroups > 1)
    {
      /*Grouped mixed precision conv doesn't support asym*/
      isAsymSupported = false;
    }
    else
    {
      isAsymSupported = true;
    }
  }
  else if(currLayer->layerType == TIDL_InnerProductLayer && (currLayer->weightsElementSizeInBits == 8U || (TIDL_isSupportedInFirmwareVersion((const char*)gParams.c7xFirmwareVersion,"11_00_08_00") && currLayer->weightsElementSizeInBits == 16U)))
  {
    /*Enable column scale shift & bias*/
    if( currLayer->layerParams.innerProductParams.constIdx == 1 && currLayer->layerParams.innerProductParams.inputBTranspose == 0)
    {
      bool hasHigherDims = false;
      int32_t i1, i2;
      for (i1 = 0; i1 < currLayer->numInBufs; i1++)
      {
        for (i2 = 0; i2 <= TIDL_DIM_DIM2; i2++)
        {
          if(currLayer->inData[i1].dimValues[i2] > 1)
          {
            hasHigherDims = true;
            break;
          }
        }
      }
      /*Currently only single channel mat-mul supports per-axis*/
      if (!hasHigherDims)
      {
        isAsymSupported = true;
      }
    }
  }
  else if(currLayer->layerType == TIDL_ConcatLayer && 
    ((currLayer->weightsElementSizeInBits == 8U && TIDL_isSupportedInFirmwareVersion((const char*)gParams.c7xFirmwareVersion,"10_01_04_00")) ||
      (currLayer->weightsElementSizeInBits == 16U && TIDL_isSupportedInFirmwareVersion((const char*)gParams.c7xFirmwareVersion,"11_00_08_00"))))
  {
    isAsymSupported = true;
  }
  else if(currLayer->layerType == TIDL_EltWiseLayer && currLayer->numInBufs == 2) 
  {
    if (currLayer->layerParams.eltWiseParams.eltWiseType == TIDL_EltWiseSum)
    {
      if((currLayer->weightsElementSizeInBits == 8U && TIDL_isSupportedInFirmwareVersion((const char*)gParams.c7xFirmwareVersion,"10_01_04_00")) ||
         (currLayer->weightsElementSizeInBits == 16U && TIDL_isSupportedInFirmwareVersion((const char*)gParams.c7xFirmwareVersion,"11_00_08_00")))
      {
        isAsymSupported = true;
      }
    }
    else if (currLayer->layerParams.eltWiseParams.eltWiseType == TIDL_EltWiseProduct)
    {
      if(currLayer->weightsElementSizeInBits == 8U && TIDL_isSupportedInFirmwareVersion((const char*)gParams.c7xFirmwareVersion,"11_01_06_00"))
      {
        isAsymSupported = true;
      }
    }
  }
  else if(currLayer->layerType == TIDL_LayerNormLayer && 
  ((currLayer->weightsElementSizeInBits == 8U && TIDL_isSupportedInFirmwareVersion((const char*)gParams.c7xFirmwareVersion,"10_01_04_00")) ||
    (currLayer->weightsElementSizeInBits == 16U && TIDL_isSupportedInFirmwareVersion((const char*)gParams.c7xFirmwareVersion,"11_00_08_00"))))
  {
    isAsymSupported = true;
  }
  else if(currLayer->layerType == TIDL_SoftMaxLayer && 
  ((currLayer->weightsElementSizeInBits == 8U && TIDL_isSupportedInFirmwareVersion((const char*)gParams.c7xFirmwareVersion,"10_01_04_00")) ||
    (currLayer->weightsElementSizeInBits == 16U && TIDL_isSupportedInFirmwareVersion((const char*)gParams.c7xFirmwareVersion,"11_00_08_00"))))
  {
    isAsymSupported = true;
  }
  else if(currLayer->layerType == TIDL_BatchNormLayer)
  {
    if((currLayer->weightsElementSizeInBits == 8U && 
      (currLayer->actParams.actType == TIDL_LeakyReLU || 
      (currLayer->actParams.actType == TIDL_GELU && TIDL_isSupportedInFirmwareVersion((const char*)gParams.c7xFirmwareVersion,"10_01_04_00")) ||
      (currLayer->actParams.actType == TIDL_SiLU && TIDL_isSupportedInFirmwareVersion((const char*)gParams.c7xFirmwareVersion,"11_00_08_00")))) ||
      ((currLayer->actParams.actType != TIDL_RelU && currLayer->actParams.actType != TIDL_Clip && currLayer->actParams.actType != TIDL_PRelU &&
       currLayer->actParams.actType != TIDL_NoAct) && TIDL_isSupportedInFirmwareVersion((const char*)gParams.c7xFirmwareVersion,"11_01_06_00"))
    )
    {
      isAsymSupported = true;
    }
  }
  else if (currLayer->layerType == TIDL_DataConvertLayer && 
           currLayer->layerParams.dataConvertParams.type == TIDL_DC_TYPE_OUTPUT &&
           currLayer->outData[0].elementType != TIDL_SinglePrecFloat &&
           currLayer->inData[0].elementType != currLayer->outData[0].elementType &&
           TIDL_isExecutionTIDLRT (gParams.modelType) == false)
  {
    isAsymSupported = true;
  }
  else if (currLayer->layerType == TIDL_DataLayer && currLayer->numInBufs > 0)
  {
    isAsymSupported = true;
  }
  else if(quantizationPassThroughLayers.find(currLayer->layerType) != quantizationPassThroughLayers.end() && pOrgTIDLNetStructure != NULL && TIDL_isSupportedInFirmwareVersion((const char*)gParams.c7xFirmwareVersion,"10_01_04_00"))
  {
    if(currLayer->layerType == TIDL_DataConvertLayer && 
      (currLayer->layerParams.dataConvertParams.type == TIDL_DC_TYPE_OUTPUT || 
        (currLayer->layerParams.dataConvertParams.type == TIDL_DC_TYPE_MIXED_PRECISION  &&
        !TIDL_isSupportedInFirmwareVersion((const char*)gParams.c7xFirmwareVersion,"11_02_00_00"))
      )
    )
    {
      isAsymSupported = false;
    }
    else if(currLayer->layerType == TIDL_DetectionOutputLayer)
    {
      isAsymSupported = true;
    }
    else if(currLayer->layerType == TIDL_DataConvertLayer && currLayer->layerParams.dataConvertParams.type == TIDL_DC_TYPE_INPUT)
    {
      isAsymSupported = true;
    }
    else if(currLayer->layerType == TIDL_ReduceLayer && currLayer->layerParams.reduceParams.ops == TIDL_inReduceOpMean) {
      isAsymSupported = false;
    }
    else if(currLayer->layerType == TIDL_ResizeLayer && currLayer->layerParams.resizeParams.mode != TIDL_ResizeNearest) {
      isAsymSupported = false;
    }
    else if(currLayer->layerType == TIDL_TopKLayer) {
      /* For topK if the consumer layers are not asymmetrical, then this layer cannot be asymmetrical */
      std::vector<int32_t> outLayerIdxs = tidl_getOutLayers(*pOrgTIDLNetStructure, pOrgTIDLNetStructure->numLayers, currLayer->outData[0].dataId);
      isAsymSupported = true;
      for(int32_t outLayerIdx: outLayerIdxs){
        isAsymSupported = isAsymSupported && TIDL_doesLayerSupportAsymTensors(&pOrgTIDLNetStructure->TIDLPCLayers[outLayerIdx], pOrgTIDLNetStructure);
      }
    }
    else if(currLayer->layerType == TIDL_PoolingLayer && currLayer->layerParams.poolParams.poolingType != TIDL_MaxPooling) {
      isAsymSupported = false;
    }
    else 
    {
      isAsymSupported = TIDL_canPropagateAsym(pOrgTIDLNetStructure, currLayer);
    }
  }
  else
  { 
    /* Do Nothing*/
  }

  if(isMixedPrecision && 
    !((currLayer->layerType == TIDL_DataConvertLayer && currLayer->layerParams.dataConvertParams.type == TIDL_DC_TYPE_INPUT) || (currLayer->layerType == TIDL_DetectionOutputLayer)) && 
    !(TIDL_doesLayerSupportMixedPrecision(currLayer) && currLayer->layerType != TIDL_EltWiseLayer)
  )
  {
    isAsymSupported = false;
  }
  return isAsymSupported;
}



void TIDL_updateDataBufferInNet(sTIDL_OrgNetwork_t * pOrgTIDLNetStructure,
                                int32_t producerLayerIdx)
{
  /*Updates the tensor & elementType*/
  int32_t layerIdx;
  sTIDL_LayerPC_t * currLayer;
  sTIDL_LayerPC_t * producerLayer;
  int32_t producerElemType;
  int32_t producerDataId;
  int32_t inIdx;

  producerLayer = &pOrgTIDLNetStructure->TIDLPCLayers[producerLayerIdx];
  producerElemType = producerLayer->outData[0].elementType;
  producerDataId   = producerLayer->outData[0].dataId;
  if ( producerLayer->numOutBufs > 0)
  {
    /* Go through all the layers */
    for ( layerIdx = 0; layerIdx < pOrgTIDLNetStructure->numLayers; layerIdx++)
    {
      currLayer = &pOrgTIDLNetStructure->TIDLPCLayers[layerIdx];
      /* Find layers whos input contains the data Id. This will indicate the producer
      layer's output goes to this layer */
      for (inIdx = 0; inIdx < currLayer->numInBufs; inIdx++)
      {
        /* Indicates one of the consumer layer is found */
        if ( producerDataId == currLayer->inData[inIdx].dataId)
        {
          /*Update element type:*/
          currLayer->inData[inIdx].elementType = producerElemType;
          currLayer->inData[inIdx].tensorType = producerLayer->outData[0].tensorType;
        }
      }
    }
  }
}



int32_t TIDL_asymUpdateNetworkWithConstraints(sTIDL_OrgNetwork_t * pOrgTIDLNetStructure,
                              tidl_import_config * params,
                              int32_t numLayers)
{
  /*This function analyses producer/consumer layers and appropriately populates the "tensorType" and datatype property*/
  int32_t layerIdx;
  int32_t outDataId;
  sTIDL_LayerPC_t * currLayer;
  int32_t outBufIdx;
  bool isAsymSupported = false;
  bool areConsumersAsym = false;

  quantizationPassThroughMap.clear();
  for( layerIdx = 0; layerIdx < numLayers; layerIdx++)
  {
    currLayer = &pOrgTIDLNetStructure->TIDLPCLayers[layerIdx];
    outDataId = currLayer->outData[0].dataId;
    outBufIdx = TIDL_isLayerNetworkOutput(pOrgTIDLNetStructure, outDataId);
    isAsymSupported = TIDL_doesLayerSupportAsymTensors(currLayer, pOrgTIDLNetStructure);
    if(isAsymSupported && (gParams.quantizationStyle == TIDL_QuantStyleAsymNP2)) /*Update for tfl pre-quant*/
    {
      /*Update to a signed dataType*/
        if(currLayer->layerType == TIDL_ConvolutionLayer && currLayer->layerParams.convParams.enableBias == 0)
        {
          /*Bias needs to be toggled on for MAC layers to handle zero point:*/
          int32_t numBiasValues = currLayer->layerParams.convParams.numOutChannels;
          currLayer->layerParams.convParams.enableBias = 1U;
          currLayer->bias.bufSize = numBiasValues;
          currLayer->bias.ptr     = (float32_tidl*) malloc(sizeof(float32_tidl) * numBiasValues);
          if(currLayer->bias.ptr != NULL)
          {
            int32_t i;
            float32_tidl *biasPtr = (float32_tidl*)currLayer->bias.ptr;
            for(i = 0; i < numBiasValues; i++)
            {
              biasPtr[i] = 0.0;
            }
          }
          else
          {
            TIDL_GLOBAL_REPORT_ERROR("Error allocating bias buffer, Aborting");
            return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
          }
        }
        /*Only FC with column bias/scale/shift*/
        else if(currLayer->layerType == TIDL_InnerProductLayer && currLayer->layerParams.innerProductParams.isBias == 0 && (currLayer->layerParams.innerProductParams.constIdx == 1))
        {
          /*Bias needs to be toggled on for MAC layers to handle zero point:*/
          int32_t numBiasValues = currLayer->layerParams.innerProductParams.numOutCols * currLayer->layerParams.innerProductParams.numBChannels;
          currLayer->layerParams.innerProductParams.isBias = 1U;
          currLayer->bias.bufSize = numBiasValues;
          currLayer->bias.ptr     = (float32_tidl*) malloc(sizeof(float32_tidl) * numBiasValues);
          if(currLayer->bias.ptr != NULL)
          {
            int32_t i;
            float32_tidl *biasPtr = (float32_tidl*)currLayer->bias.ptr;
            for(i = 0; i < numBiasValues; i++)
            {
              biasPtr[i] = 0.0;
            }
          }
          else
          {
            TIDL_GLOBAL_REPORT_ERROR("Error allocating bias buffer for Innerproduct, Aborting");
            return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
          }
        }

        /*Both input and output buffers need to be updated with tensorType:
        if any of the consumer layers are symmetric, the output of this layer
        cannot be symmetric, similarly, if the producer for this layer can't
        support asymmetric, the corresponding input buffers produced by them is
        going to be symmetric*/

        areConsumersAsym = TIDL_areConsumerLayersAsym(pOrgTIDLNetStructure, layerIdx );
        if(quantizationPassThroughLayers.find(currLayer->layerType) != quantizationPassThroughLayers.end() && quantizationPassThroughMap.find(currLayer->outData[0].dataId) != quantizationPassThroughMap.end())
        {
          int32_t inLayerIdx = tidl_getInLayer(*pOrgTIDLNetStructure, pOrgTIDLNetStructure->numLayers, currLayer->inData[0].dataId);
          bool isParentAsymOutput = pOrgTIDLNetStructure->TIDLPCLayers[inLayerIdx].outData[0].tensorType == TIDL_ASYMMETRIC_TENSOR; 
          quantizationPassThroughMap[currLayer->outData[0].dataId] = quantizationPassThroughMap[currLayer->outData[0].dataId] && isParentAsymOutput; 
          areConsumersAsym = quantizationPassThroughMap[currLayer->outData[0].dataId]; 
          if(!areConsumersAsym) {
            continue;
          }
        }

        if(areConsumersAsym)
        {
          if (!(currLayer->layerType == TIDL_DataConvertLayer && 
              currLayer->layerParams.dataConvertParams.type == TIDL_DC_TYPE_OUTPUT))
          {
            currLayer->outData[0].elementType = TIDL_convertElementTypeToSigned(currLayer->outData[0].elementType);
            TIDL_updateDataBufferInNet(pOrgTIDLNetStructure, layerIdx);
          }
          currLayer->outData[0].tensorType  = TIDL_ASYMMETRIC_TENSOR;
        }
        else
        {
          currLayer->outData[0].tensorType  = TIDL_SYMMETRIC_TENSOR;
          TIDL_updateDataBufferInNet(pOrgTIDLNetStructure, layerIdx);
        }
        if((currLayer->actParams.actType == TIDL_Clip && currLayer->actParams.clipMin >= 0) ||
          currLayer->actParams.actType == TIDL_RelU ||
          currLayer->actParams.actType == TIDL_RelU6 ||
          currLayer->actParams.actType == TIDL_Sigmoid ||
          currLayer->actParams.actType == TIDL_HardSigmoid || 
          currLayer->actParams.actType == TIDL_Abs ||
          currLayer->actParams.actType == TIDL_Exp || 
          currLayer->layerType == TIDL_SoftMaxLayer)
        {
          /*Unsigned output is a better fit:*/
          if(currLayer->outData[0].elementType == TIDL_SignedChar){
            currLayer->outData[0].elementType = TIDL_UnsignedChar;
          }
          else if(currLayer->outData[0].elementType == TIDL_SignedShort){
            currLayer->outData[0].elementType = TIDL_UnsignedShort;
          }
        }

        if(currLayer->layerType == TIDL_SoftMaxLayer){
          if(currLayer->outData[0].elementType == TIDL_SignedChar){
            currLayer->outData[0].elementType = TIDL_UnsignedChar;
          }
          if(currLayer->outData[0].elementType == TIDL_SignedShort){
            currLayer->outData[0].elementType = TIDL_UnsignedShort;
          }
        }

        if (currLayer->layerType == TIDL_EltWiseLayer && currLayer->numInBufs == 2)
        {
          if (currLayer->inData[0].elementType == TIDL_UnsignedChar &&
              currLayer->inData[1].elementType == TIDL_UnsignedChar)
          {
            currLayer->outData[0].elementType = TIDL_UnsignedChar;
          }
          if (currLayer->inData[0].elementType == TIDL_UnsignedShort &&
              currLayer->inData[1].elementType == TIDL_UnsignedShort)
          {
            currLayer->outData[0].elementType = TIDL_UnsignedShort;
          }
        }
        currLayer->layerKernelType = TIDL_HighPrecisionKernel; /*Run layer with it's high precision variant*/
        TIDL_updateDataBufferInNet(pOrgTIDLNetStructure, layerIdx);
    }
    else
    {
      currLayer->outData[0].tensorType = TIDL_SYMMETRIC_TENSOR;
      if((currLayer->outData[0].elementType == TIDL_SignedChar) && (currLayer->outData[0].tensorZeroPoint < TIDL_SIGNED_ZP_THRESHOLD))
      {
        currLayer->outData[0].elementType = TIDL_UnsignedChar;
      }
      currLayer->outData[0].tensorZeroPoint = 0;
    }
  }

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

/* First Pass : Initialize each layers input and output datatype based on
user given/automated parameters.
   Second Pass : if there is any difference between two layers datatype then
   correct it*/
void TIDL_convert8bitLayersTo16Bit(sTIDL_OrgNetwork_t * pOrgTIDLNetStructure,
                                           tidl_import_config * params,
                                           int32_t numLayers)
{
  int32_t layerIdx;
  sTIDL_LayerPC_t * currLayer;
  int32_t outDataId;
  int32_t precision;
  int32_t outBufIdx;
  int32_t inIdx;

  /* First Pass : Initialize each layers input and output datatype based on
user given/automated parameters.*/
  for ( layerIdx = 0; layerIdx < numLayers; layerIdx++)
  {
    currLayer = &pOrgTIDLNetStructure->TIDLPCLayers[layerIdx];
    outDataId = currLayer->outData[0].dataId;

    precision = getUserPrecisionFromDataId(pOrgTIDLNetStructure, params, outDataId);
    if(precision == PRECISION_LAYER_DEFAULT)
    {
      precision = currLayer->mixedPrecisionState;
    }

    if ( currLayer->layerType == TIDL_ConvolutionLayer )
    {
      if (( params->calibrationOption & TIDL_CalibOptionMixedPrecisionInput) ==
                      TIDL_CalibOptionMixedPrecisionInput)
      {
        /* If convolution layers input is a data layer then enable higher weight precision*/
        /* we should search for producer in whole network instead of using dataId */
        if (pOrgTIDLNetStructure->TIDLPCLayers[currLayer->inData[0].dataId].layerType == TIDL_DataLayer )
        {
          precision = PRECISION_LAYER_PARAMS_IN_16BIT;
        }
      }

      if (( params->calibrationOption & TIDL_CalibOptionMixedPrecisionDepthwise) ==
                      TIDL_CalibOptionMixedPrecisionDepthwise)
      {
        int32_t numGroups = currLayer->layerParams.convParams.numGroups;
        int32_t numInChannels = currLayer->layerParams.convParams.numInChannels;
        int32_t numOutChannels = currLayer->layerParams.convParams.numOutChannels;
        /* If the layer is depthewise convolution layer, then enable higher weight precision*/
        if((numGroups == numInChannels) && (numGroups == numOutChannels) && (numInChannels == numOutChannels))
        {
          precision = PRECISION_LAYER_PARAMS_IN_16BIT;
        }
      }
    }

    outBufIdx = TIDL_isLayerNetworkOutput(pOrgTIDLNetStructure, outDataId);

    if ( outBufIdx != - 1)
    {
      /* Increase activation precision if output size is 2 bytes */
      if ( params->outElementSize[outBufIdx] == 2 )
      {
        precision = PRECISION_LAYER_FEATURE_IN_16BIT;
      }
    }

    if ( precision != PRECISION_LAYER_DEFAULT)
    {
      currLayer->weightsElementSizeInBits = TIDL_increaseWeightPrecision(currLayer,
                                                      currLayer->weightsElementSizeInBits );
      if (precision == PRECISION_LAYER_FEATURE_IN_16BIT )
      {
        currLayer->outData[0].elementType = TIDL_increasePrecision(currLayer->outData[0].elementType);
        /* If the layer doesn't support mixed precision then input dataType for the same should
        also increase. Two scenario's this can happen, if input is signed or currently  we don't have
        implementation to convert 8 bit to 16 bit */
        if (TIDL_doesLayerSupportMixedPrecision(currLayer) == 0 )
        {
          for ( inIdx = 0; inIdx < currLayer->numInBufs; inIdx++ )
          {
            currLayer->inData[inIdx].elementType = TIDL_increasePrecision(currLayer->inData[inIdx].elementType);
          }
        }
      }
      else if (precision == PRECISION_LAYER_PARAMS_IN_16BIT )
      {
        /* If layer doesn't support mixed precision then increase the precision of both inputs and output in this
        case */
        if (TIDL_doesLayerSupportMixedPrecision(currLayer) == 0 )
        {
          for ( inIdx = 0; inIdx < currLayer->numInBufs; inIdx++ )
          {
            currLayer->inData[inIdx].elementType = TIDL_increasePrecision(currLayer->inData[inIdx].elementType);
          }
          currLayer->outData[0].elementType = TIDL_increasePrecision(currLayer->outData[0].elementType);
        }
      }
    }

    if(currLayer->layerType == TIDL_GridSampleLayer)
    {
      currLayer->inData[1].elementType = TIDL_increasePrecision(currLayer->inData[1].elementType);
    }
    if(currLayer->layerType == TIDL_TopKLayer && params->use16BitForTopK)
    {
      currLayer->weightsElementSizeInBits = TIDL_increaseWeightPrecision(currLayer, currLayer->weightsElementSizeInBits );
      currLayer->inData[0].elementType = TIDL_increasePrecision(currLayer->inData[0].elementType);
      currLayer->outData[0].elementType = currLayer->inData[0].elementType;
    }
  }

  int32_t updated;
  int32_t outputElemType;
  /* Second pass: Run through the network and update the consumer/producer data type if
  there is a mismtach between the two. Repeat this process till we go to a stage
  that no update is done in the network */
  do
  {
    updated = 0;
    for ( layerIdx = 0; layerIdx < numLayers; layerIdx++)
    {
      updated = TIDL_checkConsumerProducerDataType(pOrgTIDLNetStructure, layerIdx);
      if ( updated == 1 )
      {
        break;
      }
    }
  }while (updated != 0 );

}

void TIDL_getWeightsBiasValueBkp(float** &weightValueArray, float** &biasValueArray, sTIDL_OrgNetwork_t* orgTIDLNetStructure){
  int32_t numLayers = orgTIDLNetStructure->numLayers;

  //allocate memory for backup
  biasValueArray = new float*[numLayers];
  weightValueArray = new float*[numLayers];

  for(int i=0; i<numLayers; i++)
  {
    int biasBufSize = orgTIDLNetStructure->TIDLPCLayers[i].bias.bufSize;
    int weightBufSize = orgTIDLNetStructure->TIDLPCLayers[i].weights.bufSize;

    // if layer has weights
    if(weightBufSize > 0 && orgTIDLNetStructure->TIDLPCLayers[i].weights.ptr != NULL)
    {
      // create a pointer to float array that will store the weights
      float* currLayerWeightValues = new float[weightBufSize];

      for(int j=0; j<weightBufSize; j++)
      {
        currLayerWeightValues[j] = ((float*)orgTIDLNetStructure->TIDLPCLayers[i].weights.ptr)[j];
      }
      weightValueArray[i] = currLayerWeightValues;
    }
    else weightValueArray[i] = NULL;

    // if layer has a bias
    if(biasBufSize > 0 && orgTIDLNetStructure->TIDLPCLayers[i].bias.ptr != NULL)
    {
      // create a pointer to float array that will store the bias
      float* currLayerBiasValues = new float[biasBufSize];

      for(int j=0; j<biasBufSize; j++)
      {
        currLayerBiasValues[j] = ((float*)orgTIDLNetStructure->TIDLPCLayers[i].bias.ptr)[j];
      }
      biasValueArray[i] = currLayerBiasValues;
    }
    else biasValueArray[i] = NULL;
  }
}

int32_t TIDL_deleteWeightsBiasValueArray (float** &weightValueArray, float** &biasValueArray, sTIDL_OrgNetwork_t* orgTIDLNetStructure)
{
  //repeat code for deleting weights as well
  for(int i=0; i<orgTIDLNetStructure->numLayers; i++)
  {
    delete[] biasValueArray[i];
    delete[] weightValueArray[i];
    if(biasValueArray[i] != NULL)
    {
      biasValueArray[i] = NULL;
    }
    if(weightValueArray[i] != NULL)
    {
      weightValueArray[i] = NULL;
    }
  }

  delete[] biasValueArray;
  delete[] weightValueArray;
  if(biasValueArray != NULL)
  {
    biasValueArray = NULL;
  }
  if(weightValueArray != NULL)
  {
    weightValueArray = NULL;
  }

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

// This function post-processes the network after it has been imported and optimized.
int32_t TIDL_import_quantize(uint32_t layerIndex)
{
  /* At this point model is frozen */
  int32_t i;

  floatRangeMap.clear();
  //stores bias value array for all the layers(iff params are getting exported)
  float** biasValueArray = NULL;
  //stores weight value array for all the layers(iff params are getting exported)
  float** weightValueArray = NULL;

  /* generate the prototxt file that is going to store the quantization parameters for the current subgraph if the parameters are supposed to be exported*/
  if(gParams.isQuantParamsToBeExported) TIDL_getQuantParamsPrototxtPath(&gParams);

  /* Set default outElementSize based on numParamBits*/
  for ( i = 0; i < TIDL_MAX_ALG_OUT_BUFS; i++)
  {
    if ( gParams.outElementSize[i] == -1 )
    {
      if ( gParams.numParamBits <= 8 )
      {
        gParams.outElementSize[i] = 1;
      }
      else if ( gParams.numParamBits <= 16 )
      {
        gParams.outElementSize[i] = 2;
      }
      else
      {
        gParams.outElementSize[i] = 4;
      }
    }
  }

  TIDL_setDefaultWeightElementBits(&orgTIDLNetStructure, &gParams , layerIndex );

  if ( gParams.numParamBits <= 8 )
  {
    TIDL_IMPORT_CHECK_AND_RETURN(TIDL_updateNetworkWithQuantizationConstraints(&orgTIDLNetStructure, &gParams ), "");
    TIDL_convert8bitLayersTo16Bit(&orgTIDLNetStructure, &gParams , layerIndex );
  }

  if(gParams.enableHighResOptimization == 1)
  {
    gParams.compileConstraintsFlag |= 0x80;
  }

  /*Set default kernel execution to be via High Throughput kernels*/
  TIDL_setDefaultKernelType(&orgTIDLNetStructure, layerIndex);

  if ( (gParams.numParamBits < 32) )
  {
    /* Per channel quantization is only applicable with power of quantization, hence force it if its not */
    if (( gParams.calibrationOption & TIDL_CalibOptionPerChannelWeightQuantization) == TIDL_CalibOptionPerChannelWeightQuantization)
    {
      int32_t depthwiseConvExists = TIDL_depthwiseConvExists(&orgTIDLNetStructure, layerIndex);
      if(depthwiseConvExists == 1)
      {
        gParams.quantizationStyle = TIDL_QuantStyleP2Dynamic;
      }
    }

    tIDLNetStructure.isQuantStatsAvailable = 0;
    if(gParams.quantizationStyle == TIDL_QuantStyleAsymNP2_TFL)
    {
      tIDLNetStructure.isQuantStatsAvailable =  TIDL_isAllFeatureRangeAvailable(&orgTIDLNetStructure, layerIndex);
      if(tIDLNetStructure.isQuantStatsAvailable == 0)
      {
        TIDL_GLOBAL_REPORT_ERROR("Import Error: Model with all ranges not supplied, Aborting");
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }

      TIDL_IMPORT_CHECK_AND_RETURN(TIDL_importQuantLayerParams_HPTQ(&orgTIDLNetStructure,
                                    &tIDLNetStructure,
                                    &gParams,
                                    orgTIDLNetStructure.numLayers), "");
      TIDL_IMPORT_CHECK_AND_RETURN(updatePadAndWriteModel(&orgTIDLNetStructure, &tIDLNetStructure, &gParams), "");
    }
    else if(gParams.preQuantizedModel)
    {
      int32_t prototxtEnabled = 0;
      TIDL_IMPORT_CHECK_AND_RETURN(TIDL_asymUpdateNetworkWithConstraints(&orgTIDLNetStructure, &gParams , layerIndex ), "");
      if(TIDL_QuantStyleAsymNP2 == gParams.quantizationStyle)
      {
        // TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Asymmetric quantization with calib = %d",gParams.calibrationOption);
        TIDL_asymAllocScalesPointers(&orgTIDLNetStructure, layerIndex);
      }
      if (TIDL_canBypassCalibration(&orgTIDLNetStructure, &gParams))
      {
        // if there is enough data inorder to bypass calibration, take up data from the prototxt file
        prototxtEnabled = 1;
        TIDL_IMPORT_CHECK_AND_RETURN(TIDL_importQuantParamsFromProtoTxt(&orgTIDLNetStructure, &tIDLNetStructure, &gParams), "");
      }
      else 
      {
        tIDLNetStructure.isQuantStatsAvailable =  TIDL_isAllFeatureRangeAvailable(&orgTIDLNetStructure, layerIndex);

        if(tIDLNetStructure.isQuantStatsAvailable == 0)
        {
          /* Some layers don't have the range information
          * Run stats collection in float to find ranges for all the layers 
          */
          TIDL_IMPORT_CHECK_AND_RETURN(TIDL_runIterativeCalibration(&orgTIDLNetStructure,
                                    &tIDLNetStructure,
                                    &gParams), "");
        }
        // get a backup of the weights (for per-channel) & bias tensor values before they get quantized if they are to be exported
        if(gParams.isQuantParamsToBeExported) TIDL_getWeightsBiasValueBkp(weightValueArray, biasValueArray, &orgTIDLNetStructure);
      }

      string quantParamsPrototxtFile((char*)gParams.quantParamsPrototxtFile);
      if (prototxtEnabled == 1 && quantParamsPrototxtFile != "")
      {
        // get a backup of the bias tensor values before they get quantized if they ae to be exported
        TIDL_getWeightsBiasValueBkp(weightValueArray, biasValueArray, &orgTIDLNetStructure);
      }
      TIDL_IMPORT_CHECK_AND_RETURN(TIDL_importQuantLayerParams(&orgTIDLNetStructure,
                                    &tIDLNetStructure,
                                    &gParams,
                                    orgTIDLNetStructure.numLayers), "");

      /* Wrapper Function to fill the lut Data values*/
      int32_t tidlNetVersion = tIDLNetStructure.netVersion;
      TIDL_prepareLUTForActivations_wrapper(&orgTIDLNetStructure, tidlNetVersion, &gParams);
      TIDL_IMPORT_CHECK_AND_RETURN(updatePadAndWriteModel(&orgTIDLNetStructure, &tIDLNetStructure, &gParams), "");

      if (prototxtEnabled == 1 && quantParamsPrototxtFile != "")
      {
        std::string outputNetQuantParamsPrototxtFilePathStr = std::string((char*)gParams.quantParamsPrototxtFile);
        strcpy((char*)gParams.outputNetQuantParamsPrototxtFile, outputNetQuantParamsPrototxtFilePathStr.c_str());

        // update the prototxt file with the quantization parameters
        TIDL_exportQuantParamsToProtoTxt(&orgTIDLNetStructure,
                              &tIDLNetStructure,
                              &gParams,
                              orgTIDLNetStructure.numLayers,
                              weightValueArray,
                              biasValueArray);
      }
    }
    else
    {
      int32_t prototxtEnabled = 0;
      TIDL_IMPORT_CHECK_AND_RETURN(TIDL_asymUpdateNetworkWithConstraints(&orgTIDLNetStructure, &gParams , layerIndex ), "");      
      if(TIDL_QuantStyleAsymNP2 == gParams.quantizationStyle)
      {
        // TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Asymmetric quantization with calib = %d",gParams.calibrationOption);
        TIDL_asymAllocScalesPointers(&orgTIDLNetStructure, layerIndex);
      }

      if(!TIDL_canBypassCalibration(&orgTIDLNetStructure, &gParams))
      {
        // if calibration cannot be bypassed, implement calibration
        TIDL_IMPORT_CHECK_AND_RETURN(TIDL_runIterativeCalibration(&orgTIDLNetStructure,
                                  &tIDLNetStructure,
                                  &gParams), "");

        // get a backup of the bias tensor values before they get quantized if they ae to be exported
        if(gParams.isQuantParamsToBeExported) TIDL_getWeightsBiasValueBkp(weightValueArray, biasValueArray, &orgTIDLNetStructure);
      }
      else
      {
        prototxtEnabled = 1;
        // if there is enough data inorder to bypass calibration, take up data from the prototxt file
        TIDL_IMPORT_CHECK_AND_RETURN(TIDL_importQuantParamsFromProtoTxt(&orgTIDLNetStructure, &tIDLNetStructure, &gParams), "");
      }
      /** if prototxtEnabled, params are directly imported from prototxt, take weights & biases backup to re-update the prototxt
       * if user has made any changes to the prototxt file
       */
      string quantParamsPrototxtFile((char*)gParams.quantParamsPrototxtFile);
      if (prototxtEnabled == 1 && quantParamsPrototxtFile != "")
      {
        // get a backup of the bias tensor values before they get quantized if they ae to be exported
        TIDL_getWeightsBiasValueBkp(weightValueArray, biasValueArray, &orgTIDLNetStructure);
      }

      TIDL_IMPORT_CHECK_AND_RETURN(TIDL_importQuantLayerParams(&orgTIDLNetStructure,
                                      &tIDLNetStructure,
                                      &gParams,
                                      orgTIDLNetStructure.numLayers), "");

      /* Wrapper Function to fill the lut Data values*/
      int32_t tidlNetVersion = tIDLNetStructure.netVersion;
      TIDL_prepareLUTForActivations_wrapper(&orgTIDLNetStructure, tidlNetVersion, &gParams);
      TIDL_IMPORT_CHECK_AND_RETURN(updatePadAndWriteModel(&orgTIDLNetStructure, &tIDLNetStructure, &gParams), "");

      /* re-update the prototxt*/
      if (prototxtEnabled == 1 && quantParamsPrototxtFile != "")
      {
        std::string outputNetQuantParamsPrototxtFilePathStr = std::string((char*)gParams.quantParamsPrototxtFile);
        strcpy((char*)gParams.outputNetQuantParamsPrototxtFile, outputNetQuantParamsPrototxtFilePathStr.c_str());

        // update the prototxt file with the quantization parameters
        TIDL_exportQuantParamsToProtoTxt(&orgTIDLNetStructure,
                              &tIDLNetStructure,
                              &gParams,
                              orgTIDLNetStructure.numLayers,
                              weightValueArray,
                              biasValueArray);
      }
    }

    // export the calibrated quantization parameters into the prototxt file(iff user given path doesnt exist i.e for the first time)
    if(gParams.isQuantParamsToBeExported && biasValueArray != NULL)
    {
      TIDL_exportQuantParamsToProtoTxt(&orgTIDLNetStructure,
                                    &tIDLNetStructure,
                                    &gParams,
                                    orgTIDLNetStructure.numLayers,
                                    weightValueArray,
                                    biasValueArray);
    }

    // delete the dynamically allocated memory
    if (weightValueArray != NULL && biasValueArray != NULL)
    {
      TIDL_deleteWeightsBiasValueArray(weightValueArray, biasValueArray, &orgTIDLNetStructure);
    }
  }
  else
  {
    for (int i = 0; i < TIDL_MAX_ALG_IN_BUFS; i++)
    {
      gParams.inElementType[i] = TIDL_SinglePrecFloat;
    }
    tIDLNetStructure.isQuantStatsAvailable = 1;
    TIDL_IMPORT_CHECK_AND_RETURN(updatePadAndWriteModel(&orgTIDLNetStructure, &tIDLNetStructure, &gParams), "");
    /* Float inference is only supported in ref only flow so do not execute network compiler */
    gParams.executeNetworkCompiler = 0;
  }

   return 0;
}
