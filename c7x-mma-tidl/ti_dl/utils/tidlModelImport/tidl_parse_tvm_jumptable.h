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

#ifndef TIDL_PARSE_TVM_JUMPTABLE_H_
#define TIDL_PARSE_TVM_JUMPTABLE_H_

#include <unordered_map>
#include <string>


#ifdef TIDL_GEN_DOC_
// When generating documentation, avoid function pointers and just map to the TIDL Layer type
    #define TidlParseTVMJTEntry pair<int32_t, string>
    #define TVM_JTE(tvmLayer, layerType) {tvmLayer, {layerType, tvmLayer}}

#else
// When parsing, map to the parse function
    #include "tidlParseTVM/tidl_parse_tvm.h"
    #define TidlParseTVMJTEntry int32_t (TidlParseTVM::*)()
    #define TVM_JTE(tvmLayer, layerType) {tvmLayer, &TidlParseTVM::parse<OpNameStr(tvmLayer)>}
#endif /*TIDL_GEN_DOC_*/



/*
*   TVM Jump Table
*   Maps Onnx Operators to:
*   1. The TIDL Layer to which they are parsed
*   2. The Parse function
*/
unordered_map<string, TidlParseTVMJTEntry> tidlParseTVMJumpTable =
{
    TVM_JTE( "tidl.conv2d",                  TIDL_ConvolutionLayer),
    TVM_JTE( "tidl.conv_transpose",          TIDL_Deconv2DLayer),
    TVM_JTE( "min",                          TIDL_ReduceLayer),
    TVM_JTE( "max",                          TIDL_ReduceLayer),
    TVM_JTE( "squeeze",                      TIDL_SqueezeLayer),
    // TVM_JTE( "DropOut",                   TIDL_DropOutLayer),
    TVM_JTE( "depth_to_space",               TIDL_DepthToSpaceLayer),
    TVM_JTE( "space_to_depth",               TIDL_SpaceToDepthLayer),
    TVM_JTE( "pad",                          TIDL_PadLayer),
    TVM_JTE( "batch_flatten",                TIDL_FlattenLayer),
    TVM_JTE( "tidl.slice",                   TIDL_SliceLayer),
    TVM_JTE( "split",                        TIDL_SliceLayer),
    TVM_JTE( "softmax",                      TIDL_SoftMaxLayer),
    TVM_JTE( "tidl.gemm",                    TIDL_InnerProductLayer),
    TVM_JTE( "tidl.matmul",                  TIDL_InnerProductLayer),
    TVM_JTE( "tidl.scatter_nd",              TIDL_ScatterElementsLayer),
    TVM_JTE( "scatter_elements",             TIDL_ScatterElementsLayer),
    TVM_JTE( "tidl.gather",                  TIDL_GatherLayer),
    TVM_JTE( "batch_norm",                   TIDL_BatchNormLayer),
    TVM_JTE( "tidl.argmax",                  TIDL_ArgOpLayer),
    TVM_JTE( "tidl.argmin",                  TIDL_ArgOpLayer),
    TVM_JTE( "transpose",                    TIDL_TransposeLayer),
    TVM_JTE( "reshape",                      TIDL_ReshapeLayer),
    TVM_JTE( "resize2d",                     TIDL_ResizeLayer),
    TVM_JTE( "tidl.upsample",                TIDL_ResizeLayer),
    TVM_JTE( "tidl.avg_pool2d",              TIDL_PoolingLayer),
    TVM_JTE( "global_avg_pool2d",            TIDL_PoolingLayer),
    TVM_JTE( "tidl.max_pool2d",              TIDL_PoolingLayer),
    TVM_JTE( "concatenate",                  TIDL_ConcatLayer),
    TVM_JTE( "tidl.elu",                     TIDL_ELULayer),
    TVM_JTE( "tidl.hard_sigmoid",            TIDL_HardSigmoidLayer),
    TVM_JTE( "tanh",                         TIDL_TanhLayer),
    TVM_JTE( "tidl.prelu",                   TIDL_PReLULayer),
    TVM_JTE( "relu",                         TIDL_ReLULayer),
    TVM_JTE( "sigmoid",                      TIDL_SigmoidLayer),
    //- TVM_JTE( "Logistic",                 TIDL_SigmoidLayer),
    //- TVM_JTE( "Sum",                      TIDL_EltWiseLayer),
    TVM_JTE( "add",                          TIDL_EltWiseLayer),
    TVM_JTE( "multiply",                     TIDL_EltWiseLayer),
    TVM_JTE( "divide",                       TIDL_EltWiseLayer),
    TVM_JTE( "subtract",                     TIDL_EltWiseLayer),
    TVM_JTE( "clip",                         TIDL_ClipLayer),
    //- TVM_JTE( "Identity",                 TIDL_IdentityLayer),
    TVM_JTE( "cast",                         TIDL_CastLayer),
    //- TVM_JTE( "Erf",                      TIDL_IdentityLayer),
    TVM_JTE( "sqrt",                         TIDL_SqrtLayer),
    TVM_JTE( "power",                        TIDL_PowLayer),
    TVM_JTE( "mean",                         TIDL_ReduceMeanLayer),
    TVM_JTE( "tidl.expand",                  TIDL_ExpandLayer),
    TVM_JTE( "sum",                          TIDL_ReduceSumLayer),
    TVM_JTE( "leaky_relu",                   TIDL_LeakyReluLayer),
    TVM_JTE( "tidl.grid_sample",             TIDL_GridSampleLayer),
    //- TVM_JTE( "DeformConv",               TIDL_DeformableConvLayer),
    TVM_JTE( "tidl.layer_normalization",     TIDL_LayerNormLayer),
    // TVM_JTE( "QuantizeLinear",            TIDL_QuantizeLayer),
    // TVM_JTE( "DequantizeLinear",          TIDL_DequantizeLayer),
    TVM_JTE( "asin",                         TIDL_AsinLayer),
    TVM_JTE( "asinh",                        TIDL_AsinhLayer),
    TVM_JTE( "tidl.hard_swish",              TIDL_HardSwishLayer),
    TVM_JTE( "tidl.mish",                    TIDL_MishLayer),
    TVM_JTE( "log",                          TIDL_LogLayer),
    TVM_JTE( "maximum",                      TIDL_EltWiseLayer),
    TVM_JTE( "minimum",                      TIDL_EltWiseLayer),
    TVM_JTE( "tidl.unsqueeze",               TIDL_UnsqueezeLayer),
    TVM_JTE( "abs",                          TIDL_AbsLayer),
    TVM_JTE( "floor",                        TIDL_FloorLayer),
    TVM_JTE( "exp",                          TIDL_ExpLayer),
    TVM_JTE( "sin",                          TIDL_SinLayer),
    TVM_JTE( "tidl.topk",                    TIDL_TopKLayer),
    TVM_JTE( "erf",                          TIDL_ErfLayer),
    TVM_JTE( "instance_norm",                TIDL_InstanceNormLayer),
    TVM_JTE( "acos",                         TIDL_ACosLayer),
    TVM_JTE( "atan",                         TIDL_AtanLayer),
    TVM_JTE( "sinh",                         TIDL_SinhLayer),
    TVM_JTE( "negative",                     TIDL_NegLayer),
    TVM_JTE( "cos",                          TIDL_CosLayer),
    TVM_JTE( "cosh",                         TIDL_CoshLayer),
    TVM_JTE( "tan",                          TIDL_TanLayer),
};


#endif /*TIDL_PARSE_TVM_JUMPTABLE_H_ */
