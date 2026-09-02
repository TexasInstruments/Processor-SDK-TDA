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
@file      tidl_strings.cpp
@brief     This file contains mapping from various TIDL constant #defines and 
@brief     enums to human-readable strings, useful for debug output.
*/
#include "tidl_strings.h"
#include "itidl_ti.h"
#include <string> 
#include <map>
#include <sstream>

// TODO: adds strings from perfsim.h

namespace TIDL_Strings
{
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

//---------------------------------------------------------------------------
// Mappings from itidl_ti.h
std::string inferencePrecisionModeString(int32_t mode)
{
   stringmap inferencePrecisionModeNames = 
   {
      { TIDL_InferencePrecisionModeFixedPoint, "TIDL_InferencePrecisionModeFixedPoint" },
      { TIDL_InferencePrecisionModeFloatingPoint, "TIDL_InferencePrecisionModeFloatingPoint" },
   };
   return find(mode, inferencePrecisionModeNames);
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
      { TIDL_RMSNormalizationLayer, "TIDL_RMSNormalization"},
      { TIDL_LSTMLayer, "TIDL_LSTMLayer"},
      { TIDL_GRULayer, "TIDL_GRULayer"},
      { TIDL_RNNLayer, "TIDL_RNNLayer"},
      { TIDL_GatherNDLayer, "TIDL_GatherNDLayer"},
      { TIDL_CastLayer, "TIDL_CastLayer" },
      { TIDL_GatherElementsLayer, "TIDL_GatherElementsLayer" },
      { TIDL_ShapeLayer, "TIDL_ShapeLayer"},
      { TIDL_SizeLayer, "TIDL_SizeLayer"},
      { TIDL_NonZeroLayer, "TIDL_NonZeroLayer"},
      { TIDL_UnsupportedLayer, "TIDL_UnsupportedLayer" },
   };
   return find(type, layerTypeNames);
}

// Short form of layer type names
std::string layerTypeShort(int32_t type)
{
  stringmap layerTypeNames = { 
      { TIDL_DataLayer,             "Data" },
      { TIDL_ConvolutionLayer,      "Convolution" },
      { TIDL_PoolingLayer,          "Pooling" },
      { TIDL_ReLULayer,             "ReLU" },
      { TIDL_PReLULayer,            "PReLU" },
      { TIDL_EltWiseLayer,          "EltWise" },
      { TIDL_InnerProductLayer,     "InnerProduct" },
      { TIDL_SoftMaxLayer,          "SoftMax" },
      { TIDL_BatchNormLayer,        "BatchNorm" },
      { TIDL_BiasLayer,             "Bias" },
      { TIDL_ScaleLayer,            "Scale" },
      { TIDL_Deconv2DLayer,         "Deconv2D" },
      { TIDL_ConcatLayer,           "Concat" },
      { TIDL_SplitLayer,            "Split" },
      { TIDL_SliceLayer,            "Slice" },
      { TIDL_CropLayer,             "Crop" },
      { TIDL_FlattenLayer,          "Flatten" },
      { TIDL_DropOutLayer,          "DropOut" },
      { TIDL_ArgOpLayer,            "ArgOp" },
      { TIDL_DetectionOutputLayer,  "DetectionOutput" },
      { TIDL_ShuffleChannelLayer,   "ShuffleChannel"  },
      { TIDL_ResizeLayer,           "Resize"  },
      { TIDL_RoiPoolingLayer,       "RoiPooling" },
      { TIDL_OdPostProcessingLayer, "OdPostProcessing" },
      { TIDL_DepthToSpaceLayer,     "DepthToSpace" },
      { TIDL_SigmoidLayer,          "Sigmoid" },
      { TIDL_TanhLayer,             "Tanh" },
      { TIDL_HardSigmoidLayer,      "HardSigmoid" },
      { TIDL_ELULayer,              "ELU" },
      { TIDL_PadLayer,              "Pad" },
      { TIDL_ColorConversionLayer,  "ColorConversion" },
      { TIDL_OdOutputReformatLayer, "OdOutputReformat" },
      { TIDL_DataConvertLayer,      "DataConvert" },
      { TIDL_CustomLayer,           "Custom" },
      { TIDL_BatchReshapeLayer,     "BatchReshape" },
      { TIDL_ReduceLayer,           "Reduce" },
      { TIDL_ScatterElementsLayer,  "ScatterElements" },
      { TIDL_SqueezeLayer,          "Squeeze" },
      { TIDL_ReshapeLayer,          "Reshape" },
      { TIDL_ConstDataLayer,        "ConstData" },
      { TIDL_GatherLayer,           "Gather" },
      { TIDL_TransposeLayer,        "Transpose" },
      { TIDL_LayerNormLayer,        "LayerNorm" },
      { TIDL_GridSampleLayer,       "GridSample" }, 
      { TIDL_TopKLayer,             "TopK"},
      { TIDL_DeformableConvLayer,   "DeformConv" },
      { TIDL_TileLayer,             "Tile" },
      { TIDL_LogicalOpLayer,        "LogicalOperator"},
      { TIDL_RMSNormalizationLayer, "RMSNormalization" },
      { TIDL_LSTMLayer,             "LSTM" },
      { TIDL_GRULayer,              "GRU" },
      { TIDL_RNNLayer,              "RNN" },
      { TIDL_GatherNDLayer,         "GatherND" },
      { TIDL_CastLayer,             "Cast" },
      { TIDL_GatherElementsLayer,   "GatherElements" },
      { TIDL_ShapeLayer,            "Shape" },
      { TIDL_SizeLayer,             "Size" },
      { TIDL_NonZeroLayer,          "NonZero"},
      { TIDL_UnsupportedLayer,      "Unsupported" },
   };
   return find(type, layerTypeNames);
}

// Short form of act-type strings:
std::string actTypeShort(int32_t type)
{
  stringmap actTypeNames = { 
                              { TIDL_NoAct, "NoAct" }, \
                              { TIDL_RelU, "RelU" }, \
                              { TIDL_PRelU, "PRelU" }, \
                              { TIDL_RelU6, "RelU6" }, \
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
                              { TIDL_Swish , "Swish" }, \
                              { TIDL_SoftPlus , "SoftPlus" }, \
                              { TIDL_SoftSign , "SoftSign" }, \
                              { TIDL_Ceil , "Ceil" }, \
                              { TIDL_Celu , "Celu" }, \
                              { TIDL_Selu , "Selu" }, \
                              { TIDL_Round , "Round" }, \
                              { TIDL_Sign , "Sign" }
                           };
   return find(type, actTypeNames);
}

std::string quantStyleString(uint32_t qstyle)
{
  stringmap quantStyleNames = 
  {
     { TIDL_QuantStyleFixed, "TIDL_QuantStyleFixed" },
     { TIDL_QuantStyleDynamic, "TIDL_QuantStyleDynamic" },
     { TIDL_QuantStyleNP2Fixed, "TIDL_QuantStyleNP2Fixed" },
     { TIDL_QuantStyleP2Dynamic, "TIDL_QuantStyleP2Dynamic" },
     { TIDL_QuantStyleCustom, "TIDL_QuantStyleCustom" },
  };
  return TIDL_Strings::find(qstyle, quantStyleNames);
}

std::string calibOptionString(int32_t flag)
{
   // note: these are flags, could have multiple set
   stringmap calibOptionNames = 
   {
      { TIDL_CalibOptionActivationRange, "Range" },
      { TIDL_CalibOptionWeightRange, "Weight" },
      { TIDL_CalibOptionBiasCalibration, "Bias Calibration" },
      { TIDL_CalibOptionPerChannelWeightQuantization, "PerChannel" },
      { TIDL_CalibOptionBiasRange, "Bias Range Clipping" },
   };
   return TIDL_Strings::find(flag, calibOptionNames);
}

std::string activationRangeMethodString(int32_t type)
{
   stringmap activationRangeMethodNames = 
   {
      { TIDL_ActivationRangeMethodHistogram, "TIDL_ActivationRangeMethodHistogram" },
      { TIDL_ActivationRangeMethodGlobalHistogram, "TIDL_ActivationRangeMethodGlobalHistogram" },
   };
   return TIDL_Strings::find(type, activationRangeMethodNames);
}

std::string weightRangeMethodString(int32_t type)
{
   stringmap weightRangeMethodNames = 
   {
      { TIDL_WeightRangeMethodHistogram, "TIDL_WeightRangeMethodHistogram" },
      { TIDL_WeightRangeMethodMedian, "TIDL_WeightRangeMethodMedian" },
   };
   return find(type, weightRangeMethodNames);
}

std::string optimiseExtMemString(int32_t type)
{
   stringmap optimiseExtMemNames = 
   {
      { TIDL_OptimiseExtMemL0, "TIDL_OptimiseExtMemL0" },
      { TIDL_OptimiseExtMemL1, "TIDL_OptimiseExtMemL1" },
      { TIDL_OptimiseExtMemL2, "TIDL_OptimiseExtMemL2" },
   };
   return find(type, optimiseExtMemNames);
}

std::string kernelTypeString(int32_t type)
{
   stringmap kernelTypeNames = 
   {
      { TIDL_Sparse, "TIDL_Sparse" },
      { TIDL_Dense, "TIDL_Dense" },
   };
   return find(type, kernelTypeNames);
}

std::string poolingTypeString(int32_t type)
{
   stringmap poolingTypeNames = 
   {
      { TIDL_MaxPooling, "TIDL_MaxPooling" },
      { TIDL_AveragePooling, "TIDL_AveragePooling" },
   };
   return find(type, poolingTypeNames);
}

std::string getDimensionString (int32_t dim)
{
   stringmap getDimensionNames = 
   {
      { TIDL_DIM_WIDTH,  "TIDL_DIM_WIDTH" },
      { TIDL_DIM_HEIGHT, "TIDL_DIM_HEIGHT" },
      { TIDL_DIM_NUMCH,  "TIDL_DIM_NUMCH" },
      { TIDL_DIM_DIM1,   "TIDL_DIM_DIM1" },
      { TIDL_DIM_DIM2,   "TIDL_DIM_DIM2" },
      { TIDL_DIM_BATCH,  "TIDL_DIM_BATCH" },
   };
   return find(dim, getDimensionNames);
}

std::string multiCoreString(int32_t type)
{
   stringmap multiCoresTypesNames = 
   {
      { TIDL_NOT_MULTI_CORE, "TIDL_NOT_MULTI_CORE" },
      { TIDL_MULTI_CORE_SPATIAL, "TIDL_MULTI_CORE_SPATIAL" },
      { TIDL_MULTI_CORE_CHANNEL, "TIDL_MULTI_CORE_CHANNEL" },
      { TIDL_MULTI_CORE_BATCH, "TIDL_MULTI_CORE_BATCH" },
      { TIDL_MULTI_CORE_SPATIAL_WITH_JOIN , "TIDL_MULTI_CORE_SPATIAL_WITH_JOIN" },
      { TIDL_MULTI_CORE_CHANNEL_WITH_JOIN, "TIDL_MULTI_CORE_CHANNEL_WITH_JOIN" },
      { TIDL_MULTI_CORE_BATCH_WITH_JOIN, "TIDL_MULTI_CORE_BATCH_WITH_JOIN" },
   };
   return find(type, multiCoresTypesNames);
}

std::string activationTypeString(int32_t type)
{
   stringmap activationTypeNames = 
   {
      { TIDL_NoAct, "TIDL_NoAct" },
      { TIDL_RelU, "TIDL_RelU" },
      { TIDL_PRelU, "TIDL_PRelU" },
      { TIDL_RelU6, "TIDL_RelU6" },
      { TIDL_Sigmoid, "TIDL_Sigmoid" },
      { TIDL_Tanh, "TIDL_Tanh" },
      { TIDL_HardSigmoid, "TIDL_HardSigmoid" },
      { TIDL_ELU, "TIDL_ELU" },
      { TIDL_GELU, "TIDL_GELU"},
      { TIDL_LeakyReLU, "TIDL_LeakyReLU"},
      { TIDL_Asin, "TIDL_Asin" },
      { TIDL_HardSwish, "TIDL_HardSwish" },
      { TIDL_Mish, "TIDL_Mish" },
      { TIDL_Log, "TIDL_Log" },
      { TIDL_Asinh, "TIDL_Asinh" },
      { TIDL_Abs, "TIDL_Abs" },
      { TIDL_Sin, "TIDL_Sin" },
      { TIDL_Exp, "TIDL_Exp" },
      { TIDL_Pow, "TIDL_Pow" },
      { TIDL_Floor, "TIDL_Floor" },
      { TIDL_Erf, "TIDL_Erf" },
      { TIDL_Sqrt, "TIDL_Sqrt" },
      { TIDL_Acos, "TIDL_Acos" },
      { TIDL_Atan, "TIDL_Atan" },
      { TIDL_Sinh, "TIDL_Sinh" },
      { TIDL_Cos, "TIDL_Cos" },
      { TIDL_Cosh, "TIDL_Cosh" },
      { TIDL_Neg, "TIDL_Neg"},
      { TIDL_Tan,"TIDL_Tan"},
      { TIDL_Logit, "TIDL_Logit" },
      { TIDL_Reciprocal, "TIDL_Reciprocal"},
      { TIDL_SiLU, "TIDL_SiLU"},
      { TIDL_Swish, "TIDL_Swish"},
      { TIDL_SoftPlus, "TIDL_SoftPlus"},
      { TIDL_SoftSign, "TIDL_SoftSign"},
      { TIDL_Ceil, "TIDL_Ceil"},
      { TIDL_Celu, "TIDL_Celu"},
      { TIDL_Selu, "TIDL_Selu"},
      { TIDL_Round, "TIDL_Round"},
      { TIDL_Sign, "TIDL_Sign"}
   };
   return find(type, activationTypeNames);
}

std::string dimString(int32_t dim)
{
   stringmap dimNames = 
   {
      { TIDL_DIM_BATCH, "TIDL_DIM_BATCH" },
      { TIDL_DIM_NUMCH, "TIDL_DIM_NUMCH" },
      { TIDL_DIM_HEIGHT, "TIDL_DIM_HEIGHT" },
      { TIDL_DIM_WIDTH, "TIDL_DIM_WIDTH" },
   };
   return find(dim, dimNames);
}

std::string pitchDimIndexString(int32_t dim)
{
   stringmap pitchDimIndexNames = 
   {
      { TIDL_ROI_PITCH, "TIDL_ROI_PITCH" },
      { TIDL_CHANNEL_PITCH, "TIDL_CHANNEL_PITCH" },
      { TIDL_LINE_PITCH, "TIDL_LINE_PITCH" },
      { TIDL_PITCH_MAX, "TIDL_PITCH_MAX" },
   };
   return find(dim, pitchDimIndexNames);
}

std::string sysMemScratchString(uint32_t type)
{
   stringmap sysMemScratchNames = 
   {
      { TIDL_SYSMEM_L1_SCRATCH, "TIDL_SYSMEM_L1_SCRATCH" },
      { TIDL_SYSMEM_L2_SCRATCH, "TIDL_SYSMEM_L2_SCRATCH" },
      { TIDL_SYSMEM_L3_SCRATCH, "TIDL_SYSMEM_L3_SCRATCH" },
      { TIDL_SYSMEM_DDR_SCRATCH, "TIDL_SYSMEM_DDR_SCRATCH" },
   };
   return find(type, sysMemScratchNames);
}

std::string customLayerExecModeString(uint32_t type)
{
   stringmap customLayerExecModeNames =
   {
      { TIDL_EXEC_MODE_STATS_COLLECTION, "TIDL_EXEC_MODE_STATS_COLLECTION" },
      { TIDL_EXEC_MODE_INFER_PROCESS, "TIDL_EXEC_MODE_INFER_PROCESS" },
      { TIDL_EXEC_MODE_INFER_PROCESS_REF, "TIDL_EXEC_MODE_INFER_PROCESS_REF" },
   };
   return find(type, customLayerExecModeNames);
}

std::string strideOffsetMethodString(uint32_t method)
{
   stringmap strideOffsetMethodNames =
   {
      { TIDL_StrideOffsetTopLeft, "TIDL_StrideOffsetTopLeft" },
      { TIDL_StrideOffsetCenter, "TIDL_StrideOffsetCenter" },
      { TIDL_StrideOffsetMax, "TIDL_StrideOffsetMax" },
   };
   return find(method, strideOffsetMethodNames);
}

std::string elementTypeString(uint32_t type)
{
   stringmap elementTypeNames =
   {
      { TIDL_UnsignedChar, "TIDL_UnsignedChar" },
      { TIDL_SignedChar, "TIDL_SignedChar" },
      { TIDL_UnsignedShort, "TIDL_UnsignedShort" },
      { TIDL_SignedShort, "TIDL_SignedShort" },
      { TIDL_UnsignedWord, "TIDL_UnsignedWord" },
      { TIDL_SignedWord, "TIDL_SignedWord" },
      { TIDL_SinglePrecFloat, "TIDL_SinglePrecFloat" },
      { TIDL_UnsignedDoubleWord, "TIDL_UnsignedDoubleWord"},
      { TIDL_SignedDoubleWord, "TIDL_SignedDoubleWord"},
      { TIDL_Bool,         "TIDL_Bool"},
      { TIDL_BFloat16, "TIDL_BFloat16"},
   };
   return find(type, elementTypeNames);
}

std::string eltwiseOpTypeString(uint32_t type)
{
   stringmap eltwiseOpTypeNames =
   {
     { TIDL_EltWiseProduct, "Mul" },
     { TIDL_EltWiseSum,     "Add" },
     { TIDL_EltWiseMax,     "Max" },
     { TIDL_EltWiseSub,     "Sub" },
     { TIDL_EltWiseDiv,     "Div" },
     { TIDL_EltWiseMin,     "Min" },
     { TIDL_EltWiseMod,     "Mod" },
   };
   return find(type, eltwiseOpTypeNames);
}

std::string logicalOpTypeString(uint32_t type)
{
   stringmap logicalOpTypeNames = 
   {
     { TIDL_And, "And" },
     { TIDL_Or, "Or" },
     { TIDL_Xor, "Xor" },
     { TIDL_Equal, "Equal"},
     { TIDL_Greater, "Greater"},
     { TIDL_GreaterOrEqual, "GreaterOrEqual"},
     { TIDL_Less, "Less"},
     { TIDL_LessOrEqual, "LessOrEqual"},
     { TIDL_Not, "Not"},
     { TIDL_IsInf, "IsInf"},
     { TIDL_IsNaN, "IsNan"},
     { TIDL_Where, "Where"},
   };
   return find(type, logicalOpTypeNames);
}

std::string argOpTypeString(uint32_t type)
{
   stringmap argOpTypeNames = 
   {
     { TIDL_ArgMin, "ArgMin" },
     { TIDL_ArgMax, "ArgMax" },
   };
   return find(type, argOpTypeNames);
}

std::string outputDetectionTypeString(int32_t type)
{
   stringmap outputDetectionTypeNames = 
   {
      { TIDL_processingTypeDetectionOutput, "TIDL_processingTypeDetectionOutput" },
      { TIDL_processingTypeOdPostProcessing, "TIDL_processingTypeOdPostProcessing" },
   };
   return find(type, outputDetectionTypeNames);
}

std::string metaArchTypeString(int32_t type)
{
   stringmap metaArchTypeNames = 
   {
      { TIDL_metaArchCaffeJacinto, "TIDL_metaArchCaffeJacinto" },
      { TIDL_metaArchTFSSD, "TIDL_metaArchTFSSD" },
      { TIDL_metaArchTFFasterRcnn, "TIDL_metaArchTFFasterRcnn" },
      { TIDL_metaArchTIDLSSD, "TIDL_metaArchTIDLSSD" },
      { TIDL_metaArchTIDLYolo, "TIDL_metaArchTIDLYolo" },
      { TIDL_metaArchTIDLRetinaNet, "TIDL_metaArchTIDLRetinaNet" },
   };
   return find(type, metaArchTypeNames);
}

std::string processingTypeString(int32_t type)
{
   stringmap processingTypeNames = 
   {
      { TIDL_processingTypeDetectionOutput, "TIDL_processingTypeDetectionOutput" },
      { TIDL_processingTypeOdPostProcessing, "TIDL_processingTypeOdPostProcessing" },
   };
   return find(type, processingTypeNames);
}

std::string scoreConverterString(int32_t type)
{
   stringmap scoreConverterNames = 
   {
      { TIDL_scoreConverterLINEAR, "TIDL_scoreConverterLINEAR" },
      { TIDL_scoreConverterSIGMOID, "TIDL_scoreConverterSIGMOID" },
      { TIDL_scoreConverterSOFTMAX, "TIDL_scoreConverterSOFTMAX" },
   };
   return find(type, scoreConverterNames);
}

std::string yuvFormatString(int32_t type)
{
   stringmap yuvFormatNames = 
   {
      { TIDL_inYuvFormatYuv420_NV12, "TIDL_inYuvFormatYuv420_NV12" },
      { TIDL_inYuvFormatYuv422I_YUYV, "TIDL_inYuvFormatYuv422I_YUYV" },
      { TIDL_inYuvFormatYuv422I_UYVY, "TIDL_inYuvFormatYuv422I_UYVY" },
   };
   return find(type, yuvFormatNames);
}


std::string resizeModeString(int32_t type)
{
   stringmap resizeModeNames = 
   {
     { TIDL_ResizeNearest, "TIDL_ResizeNearest" },
     { TIDL_ResizeBilinear, "TIDL_Bilinear" },
     { TIDL_ResizePadZero, "TIDL_ResizePadZero"}
   };
   return find(type, resizeModeNames);
}

std::string roiPoolingTypeString(int32_t type)
{
   stringmap roiPoolingTypeNames = 
   {
     { TIDL_RoiPoolingFasterRCNN, "TIDL_RoiPollingFasterRCNN" },
   };
   return find(type, roiPoolingTypeNames);
}

std::string dataConvertLayoutString(int32_t layout)
{
   stringmap dataConvertLayoutNames = 
   {
     { TIDL_LT_NCHW, "TIDL_LT_NCHW" },
     { TIDL_LT_NHWC, "TIDL_LT_NHWC" },
   };
   return find(layout, dataConvertLayoutNames);
}

std::string dataConvertTypeString(int32_t type)
{
   stringmap dataConvertTypeNames = 
   {
     { TIDL_DC_TYPE_INPUT, "TIDL_DC_TYPE_INPUT" },
     { TIDL_DC_TYPE_OUTPUT, "TIDL_DC_TYPE_OUTPUT" },
     { TIDL_DC_TYPE_INTERMEDIATE, "TIDL_DC_TYPE_INTERMEDIATE" },
     { TIDL_DC_TYPE_MIXED_PRECISION, "TIDL_DC_TYPE_MIXED_PRECISION"},
     { TIDL_DC_TYPE_PTQ_TO_NATIVE ,"TIDL_DC_TYPE_PTQ_TO_NATIVE"},
     { TIDL_DC_TYPE_NATIVE_TO_PTQ ,"TIDL_DC_TYPE_NATIVE_TO_PTQ"},
   };
   return find(type, dataConvertTypeNames);
}

std::string padTypeString(int32_t type)
{
   stringmap padTypeNames = 
   {
     { TIDL_PadZero, "TIDL_PadZero" },
     { TIDL_PadNonZeroConstant, "TIDL_PadNonZeroConstant" },
     { TIDL_PadPerChannel, "TIDL_PadPerChannel" },
     { TIDL_PadWrapAround, "TIDL_PadWrapAround"},
   };
   return find(type, padTypeNames);
}

std::string odOutputTypeString(int32_t type)
{
   stringmap odOutputTypeNames = 
   {
     { TIDL_DetectionBoxesTflite, "TIDL_DetectionBoxesTflite" },
     { TIDL_DetectionClassesTflite, "TIDL_DetectionClassesTflite" },
     { TIDL_DetectionScoresTflite, "TIDL_DetectionScoresTflite" },
     { TIDL_NumDetectionsTflite, "TIDL_NumDetectionsTflite"},
     { TIDL_DetectionBoxesSsd, "TIDL_DetectionBoxesSsd" },
     { TIDL_DetectionClassesSsd, "TIDL_DetectionClassesSsd" },
     { TIDL_DetectionScoresSsd, "TIDL_DetectionScoresSsd"},
     { TIDL_DetectionBoxesAndScoresMmdetection, "TIDL_DetectionBoxesAndScoresMmdetection" },
     { TIDL_DetectionClassesMmdetection, "TIDL_DetectionClassesMmdetection"},
     { TIDL_DetectionBoxesScoresLabelsYoloV5, "TIDL_DetectionBoxesScoresLabelsYoloV5"},
     { TIDL_DetectionBoxesScoresLabelsTfliteEfficientdet, "TIDL_DetectionBoxesScoresLabelsTfliteEfficientdet"},
     { TIDL_Detection3dOd, "TIDL_Detection3dOd"},
     { TIDL_DetectionBoxes3dOd, "TIDL_DetectionBoxes3dOd"},
     { TIDL_DetectionClasses3dOd, "TIDL_DetectionClasses3dOd"},
     { TIDL_DetectionScores3dOd, "TIDL_DetectionScores3dOd"}
   };
   return find(type, odOutputTypeNames);
}

std::string inDataFormatString(int32_t format)
{
   stringmap inDataFormatNames = 
   {
     { TIDL_inDataFormatBGRPlanar, "TIDL_inDataFormatBGRPlanar" },
     { TIDL_inDataFormatRGBPlanar, "TIDL_inDataFormatRGBPlanar" },
   };
   return find(format, inDataFormatNames);
}

std::string inResizeTypeString(int32_t type)
{
   stringmap inResizeTypeNames = 
   {
     { TIDL_inResizeTypeDefault, "TIDL_inResizeTypeDefault" },
     { TIDL_inResizeTypeKeepAR, "TIDL_inResizeTypeKeepAR" },
   };
   return find(type, inResizeTypeNames);
}

std::string gridSampleModeString(int32_t type)
{
   stringmap gridSampleModeNames = 
   {
     { TIDL_ModeNearest, "TIDL_gridSampleModeNearest"},
     { TIDL_ModeBilinear, "TIDL_gridSampleModeBilinear"},
     { TIDL_ModeBicubic, "TIDL_gridSampleModeBicubic"},
   };
   return find(type, gridSampleModeNames);
}

std::string gridSamplePadModeString(int32_t type)
{
   stringmap gridSamplePadModeNames = 
   {
     { TIDL_ZeroPad, "TIDL_gridSamplePadModeZero"},
     { TIDL_BorderPad, "TIDL_gridSamplePadModeBorder"},
     { TIDL_ReflectionPad, "TIDL_gridSamplePadModeReflection"},
   };
   return find(type, gridSamplePadModeNames);
}

std::string recurrentDirectionString(int32_t direction)
{
   stringmap recurrentDirectionNames =
   {
     { TIDL_RecurrentForward,       "TIDL_RecurrentForward"},
     { TIDL_RecurrentReverse,       "TIDL_RecurrentReverse"},
     { TIDL_RecurrentBidirectional, "TIDL_RecurrentBidirectional"},
   };

   return find(direction, recurrentDirectionNames);
}

} // namespace TIDL_Strings

