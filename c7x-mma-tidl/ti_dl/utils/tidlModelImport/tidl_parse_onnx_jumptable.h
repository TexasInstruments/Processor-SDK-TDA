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

#ifndef TIDL_PARSE_ONNX_JUMPTABLE_H_
#define TIDL_PARSE_ONNX_JUMPTABLE_H_

#include <unordered_map>
#include <string>


#ifdef TIDL_GEN_DOC_
// When generating documentation, avoid function pointers and just map to the TIDL Layer type
    #define TidlParseOnnxJTEntry pair<int32_t, string>
    #define Onnx_JTE(onnxLayer, layerType) {onnxLayer, {layerType, onnxLayer}}

#else
// When parsing, map to the parse function
    #include "tidlParseOnnx/tidl_parse_onnx.h"
    #define TidlParseOnnxJTEntry int32_t (TidlParseOnnx::*)()
    #define Onnx_JTE(onnxLayer, layerType) {onnxLayer, &TidlParseOnnx::parse<OnnxStr(onnxLayer)>}
#endif /*TIDL_GEN_DOC_*/



/*
*   Onnx Jump Table
*   Maps Onnx Operators to:
*   1. The TIDL Layer to which they are parsed
*   2. The Parse function
*/
unordered_map<string, TidlParseOnnxJTEntry> tidlParseOnnxJumpTable =
{
    Onnx_JTE( "Conv",                   TIDL_ConvolutionLayer),
    Onnx_JTE( "ConvTranspose",          TIDL_Deconv2DLayer),
    Onnx_JTE( "ReduceMin",              TIDL_ReduceLayer),
    Onnx_JTE( "ReduceMax",              TIDL_ReduceLayer),
    Onnx_JTE( "Squeeze",                TIDL_SqueezeLayer),
    Onnx_JTE( "DropOut",                TIDL_DropOutLayer),
    Onnx_JTE( "DepthToSpace",           TIDL_DepthToSpaceLayer),
    Onnx_JTE( "SpaceToDepth",           TIDL_SpaceToDepthLayer),
    Onnx_JTE( "Pad",                    TIDL_PadLayer),
    Onnx_JTE( "Flatten",                TIDL_FlattenLayer),
    Onnx_JTE( "Slice",                  TIDL_SliceLayer),
    Onnx_JTE( "Split",                  TIDL_SliceLayer),
    Onnx_JTE( "Softmax",                TIDL_SoftMaxLayer),
    Onnx_JTE( "Gemm",                   TIDL_InnerProductLayer),
    Onnx_JTE( "MatMul",                 TIDL_InnerProductLayer),
    Onnx_JTE( "ScatterND",              TIDL_ScatterElementsLayer),
    Onnx_JTE( "ScatterElements",        TIDL_ScatterElementsLayer),
    Onnx_JTE( "Gather",                 TIDL_GatherLayer),
    Onnx_JTE( "BatchNormalization",     TIDL_BatchNormLayer),
    Onnx_JTE( "ArgMax",                 TIDL_ArgOpLayer),
    Onnx_JTE( "ArgMin",                 TIDL_ArgOpLayer),
    Onnx_JTE( "Transpose",              TIDL_TransposeLayer),
    Onnx_JTE( "Reshape",                TIDL_ReshapeLayer),
    Onnx_JTE( "Resize",                 TIDL_ResizeLayer),
    Onnx_JTE( "Upsample",               TIDL_ResizeLayer),
    Onnx_JTE( "AveragePool",            TIDL_PoolingLayer),
    Onnx_JTE( "GlobalAveragePool",      TIDL_PoolingLayer),
    Onnx_JTE( "MaxPool",                TIDL_PoolingLayer),
    Onnx_JTE( "Concat",                 TIDL_ConcatLayer),
    Onnx_JTE( "Elu",                    TIDL_ELULayer),
    Onnx_JTE( "HardSigmoid",            TIDL_HardSigmoidLayer),
    Onnx_JTE( "Tanh",                   TIDL_TanhLayer),
    Onnx_JTE( "PRelu",                  TIDL_PReLULayer),
    Onnx_JTE( "Relu",                   TIDL_ReLULayer),
    Onnx_JTE( "Sigmoid",                TIDL_SigmoidLayer),
    Onnx_JTE( "Logistic",               TIDL_SigmoidLayer),
    Onnx_JTE( "Sum",                    TIDL_EltWiseLayer),
    Onnx_JTE( "Add",                    TIDL_EltWiseLayer),
    Onnx_JTE( "Mul",                    TIDL_EltWiseLayer),
    Onnx_JTE( "Div",                    TIDL_EltWiseLayer),
    Onnx_JTE( "Sub",                    TIDL_EltWiseLayer),
    Onnx_JTE( "Clip",                   TIDL_ClipLayer),
    Onnx_JTE( "Identity",               TIDL_IdentityLayer),
    Onnx_JTE( "Cast",                   TIDL_CastLayer),
    Onnx_JTE( "Erf",                    TIDL_IdentityLayer),
    Onnx_JTE( "Sqrt",                   TIDL_SqrtLayer),
    Onnx_JTE( "Pow",                    TIDL_PowLayer),
    Onnx_JTE( "ReduceMean",             TIDL_ReduceMeanLayer),
    Onnx_JTE( "Expand",                 TIDL_ExpandLayer),
    Onnx_JTE( "ReduceSum",              TIDL_ReduceSumLayer),
    Onnx_JTE( "LeakyRelu",              TIDL_LeakyReluLayer),
    Onnx_JTE( "GridSample",             TIDL_GridSampleLayer),
    Onnx_JTE( "DeformConv",             TIDL_DeformableConvLayer),
    Onnx_JTE( "LayerNormalization",     TIDL_LayerNormLayer),
    Onnx_JTE( "QuantizeLinear",         TIDL_QuantizeLayer),
    Onnx_JTE( "DequantizeLinear",       TIDL_DequantizeLayer),
    Onnx_JTE( "Asin",                   TIDL_AsinLayer),
    Onnx_JTE( "Asinh",                  TIDL_AsinhLayer),
    Onnx_JTE( "HardSwish",              TIDL_HardSwishLayer),
    Onnx_JTE( "Mish",                   TIDL_MishLayer),
    Onnx_JTE( "Log",                    TIDL_LogLayer),
    Onnx_JTE( "Max",                    TIDL_EltWiseLayer),
    Onnx_JTE( "Min",                    TIDL_EltWiseLayer),
    Onnx_JTE( "Unsqueeze",              TIDL_UnsqueezeLayer),
    Onnx_JTE( "Abs",                    TIDL_AbsLayer),
    Onnx_JTE( "Floor",                  TIDL_FloorLayer),
    Onnx_JTE( "Exp",                    TIDL_ExpLayer),
    Onnx_JTE( "Sin",                    TIDL_SinLayer),
    Onnx_JTE( "TopK",                   TIDL_TopKLayer),
    Onnx_JTE( "Erf",                    TIDL_ErfLayer),
    Onnx_JTE( "InstanceNormalization",  TIDL_InstanceNormLayer),
    Onnx_JTE( "Acos",                   TIDL_AcosLayer),
    Onnx_JTE( "Atan",                   TIDL_AtanLayer),
    Onnx_JTE( "Sinh",                   TIDL_SinhLayer),
    Onnx_JTE( "Neg",                    TIDL_NegLayer),
    Onnx_JTE( "Cos",                    TIDL_CosLayer),
    Onnx_JTE( "Cosh",                   TIDL_CoshLayer),
    Onnx_JTE( "Tan",                    TIDL_TanLayer),
    Onnx_JTE( "Swish",                  TIDL_SwishLayer),
    Onnx_JTE( "Tile",                   TIDL_TileLayer),
    Onnx_JTE( "And",                    TIDL_LogicalOpLayer),
    Onnx_JTE( "Or",                     TIDL_LogicalOpLayer),
    Onnx_JTE( "Xor",                    TIDL_LogicalOpLayer),
    Onnx_JTE( "Equal",                  TIDL_LogicalOpLayer),
    Onnx_JTE( "Greater",                TIDL_LogicalOpLayer),
    Onnx_JTE( "GreaterOrEqual",         TIDL_LogicalOpLayer),
    Onnx_JTE( "Less",                   TIDL_LogicalOpLayer),
    Onnx_JTE( "LessOrEqual",            TIDL_LogicalOpLayer),
    Onnx_JTE( "Not",                    TIDL_LogicalOpLayer),
    Onnx_JTE( "IsInf",                  TIDL_LogicalOpLayer),
    Onnx_JTE( "IsNaN",                  TIDL_LogicalOpLayer),
    Onnx_JTE( "Where",                  TIDL_LogicalOpLayer),
    Onnx_JTE( "RMSNormalization",       TIDL_RMSNormalizationLayer),
    Onnx_JTE( "Softplus",               TIDL_SoftPlusLayer),
    Onnx_JTE( "Softsign",               TIDL_SoftSignLayer),
    Onnx_JTE( "Ceil",                   TIDL_CeilLayer),
    Onnx_JTE( "Celu",                   TIDL_CeluLayer),
    Onnx_JTE( "Selu",                   TIDL_SeluLayer),
    Onnx_JTE( "Round",                  TIDL_RoundLayer),
    Onnx_JTE( "Sign",                   TIDL_SignLayer),
    Onnx_JTE( "Mod",                    TIDL_EltWiseLayer),
    Onnx_JTE( "GroupNormalization",     TIDL_GroupNormLayer),
    Onnx_JTE( "LSTM",                   TIDL_LSTMLayer),
    Onnx_JTE( "GRU",                    TIDL_GRULayer),
    Onnx_JTE( "RNN",                    TIDL_RNNLayer),
    Onnx_JTE( "GatherND",               TIDL_GatherNDLayer),
    Onnx_JTE( "CastLike",               TIDL_CastLikeLayer),
    Onnx_JTE( "GatherElements",         TIDL_GatherElementsLayer),
    Onnx_JTE( "Shape",                  TIDL_ShapeLayer),
    Onnx_JTE( "Size",                   TIDL_SizeLayer),
    Onnx_JTE( "Attention",              TIDL_AttentionLayer),
    Onnx_JTE( "NonZero",                TIDL_NonZeroLayer),
};


#endif /*TIDL_PARSE_ONNX_JUMPTABLE_H_ */
