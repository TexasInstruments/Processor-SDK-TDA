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

#include "tidl_parse_tvm.h"
using namespace std;

#include <tvm/relay/attrs/nn.h>
#include <tvm/relay/function.h>

template <>
int32_t TidlParseTVM::parse<OpNameStr("tidl.avg_pool2d")>()
{
  layer.layerType = TIDL_PoolingLayer;
  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;
  sTIDL_PoolingParams_t &poolParams = layer.layerParams.poolParams;
  poolParams.countIncludePad = 0;
  const auto* fn_node = call->op.as<tvm::relay::FunctionNode>();
  auto fn = GetRef<tvm::relay::Function>(fn_node);
  const auto* param = fn->attrs.as<DictAttrsNode>();

  Array<PrimExpr> kernel_shape;
  if(param->dict.count("kernel_shape")){
      kernel_shape= Downcast<Array<PrimExpr>>(param->dict.at("kernel_shape"));
  }
  else
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : AvgPool : kernel_shape attribute is missing");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  if(kernel_shape.size() == 2)
  {
    poolParams.kernelH = kernel_shape[0].as<IntImmNode>()->value;
    poolParams.kernelW = kernel_shape[1].as<IntImmNode>()->value;
  }
  else if (kernel_shape.size() < 2)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : AvgPool : kernel_shape is missing height axis.");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  else
  {
    for(int32_t i = 2; i < kernel_shape.size(); i++)
    {
      if (kernel_shape[i].as<IntImmNode>()->value != 1)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : AvgPool : kernel_shape is only supported along width and height axis.");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }
  }

  if(param->dict.count("count_include_pad"))
  {
    poolParams.countIncludePad = Downcast<Integer>(param->dict.at("count_include_pad"))->value;
  }

  int32_t isGlobalAvgPool = 0;
  if(gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TVM_RELAY)
  {
    if((md.varTensorsDims[0][md.varTensorsDims[0].size()-2] == poolParams.kernelH) && (md.varTensorsDims[0][md.varTensorsDims[0].size()-1] == poolParams.kernelW)) //global avg pool - e.g. squeezenet1.1
    {
      isGlobalAvgPool = 1;
    }
  }

  int32_t status = 0;
  if (isGlobalAvgPool)
  {
    status = this->parse<OpNameStr("global_avg_pool2d")>();
  }
  else
  {
    status = this->parse<OpNameStr("tidl.max_pool2d")>();
    poolParams.poolingType = TIDL_AveragePooling;

    // Convert 1x1 average pooling into max pooling (operations are identical)
    // mxnet_resnet50_v1d
    if (poolParams.kernelH == 1 && poolParams.kernelW == 1 &&
      ((poolParams.strideH == 2 && poolParams.strideW == 2) ||
      (poolParams.strideH == 1 && poolParams.strideW == 1)))
      poolParams.poolingType = TIDL_MaxPooling;
  }

  return status;
}
