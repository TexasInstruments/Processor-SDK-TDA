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

#include <tvm/relay/expr.h>

template <>
int32_t TidlParseTVM::parse<OpNameStr("subtract")>()
{
  const auto *op_node = call->op.as<tvm::OpNode>();
  const auto op_name = GetRef<tvm::Op>(op_node)->name;

  layer.layerType = TIDL_EltWiseLayer;
  layer.layerParams.eltWiseParams.eltWiseType = TIDL_EltWiseSub;
  layer.numInBufs = layer.allowlistingMetaData.numInputs;

  if (layer.allowlistingMetaData.numConstInputs > 0){

    int32_t numEle = 1;
    for(int i = 0 ; i < layer.allowlistingMetaData.constTensorsDims[0].size() ; i++){
      numEle *= layer.allowlistingMetaData.constTensorsDims[0][i];
    }
    
    int weight_zp = 0;
    float weight_scale = 1.0f;
    layer.weights = TIDL_extractConstantTensorData(call, layer.allowlistingMetaData.constTensorIndices[0], layer.allowlistingMetaData, 0, weight_zp, weight_scale);
    if(layer.weights.ptr==NULL)
    {
      TIDL_LOG_ERROR(gDiags.gDiagList, "Unable to read initializer tensor at index %d", layer.allowlistingMetaData.constTensorIndices[0]);
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }
    if(layer.allowlistingMetaData.numConstInputs==2){
      layer.bias=TIDL_extractConstantTensorData(call, layer.allowlistingMetaData.constTensorIndices[1], layer.allowlistingMetaData, 1, weight_zp, weight_scale);
      if(layer.bias.ptr==NULL)
      {
          TIDL_LOG_ERROR(gDiags.gDiagList, "Unable to read initializer tensor at index %d", layer.allowlistingMetaData.constTensorIndices[1]);
          return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }
    }
  }

  return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}

template <typename Tvalue>
Tvalue TIDL_relayGetConstantVal(const relay::Expr &expr)
{
  Tvalue value;
  expr.as<ConstantNode>()->data.CopyToBytes(&value, sizeof(Tvalue));
  return value;
}