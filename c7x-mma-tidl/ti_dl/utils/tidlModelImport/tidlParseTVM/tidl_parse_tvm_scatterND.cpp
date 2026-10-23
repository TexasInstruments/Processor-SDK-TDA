/*
 *
 * Copyright (c) {2025} Texas Instruments Incorporated
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
using namespace tvm;
#include <tvm/relay/function.h>
#include <tvm/relay/expr.h>
#include <tvm/relay/analysis.h>
#include <tvm/runtime/ndarray.h>



template<>
int32_t TidlParseTVM::parse<OpNameStr("tidl.scatter_nd")> ()
{
    layer.layerType = TIDL_ScatterElementsLayer;
    sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;
    layer.numInBufs = md.numInputs;
    const auto* fn_node = call->op.as<tvm::relay::FunctionNode>();
    auto fn = GetRef<tvm::relay::Function>(fn_node);
    const auto* attrs = fn->attrs.as<DictAttrsNode>();

    // ScatterND uses axis = -1 (no specific axis, operates on N-D indices)
    layer.layerParams.scatterElementsParams.axis = -1;

    // Get reduction attribute from the composite function's attributes
    string reduction = "update";
    if (attrs->dict.count("reduction"))
    {
        reduction = Downcast<String>(attrs->dict.at("reduction"));
    }
    // Map reduction string to TIDL reduction type
    if (strcmp(reduction.c_str(), "update") == 0)
    {
        layer.layerParams.scatterElementsParams.reduction = TIDL_ScatterElementsNone;
    }
    else if (strcmp(reduction.c_str(), "add") == 0)
    {
        layer.layerParams.scatterElementsParams.reduction = TIDL_ScatterElementsAdd;
    }
    else if (strcmp(reduction.c_str(), "mul") == 0)
    {
        layer.layerParams.scatterElementsParams.reduction = TIDL_ScatterElementsUnsupported;
    }
    else if (strcmp(reduction.c_str(), "max") == 0)
    {
        layer.layerParams.scatterElementsParams.reduction = TIDL_ScatterElementsMax;
    }
    else if (strcmp(reduction.c_str(), "min") == 0)
    {
        layer.layerParams.scatterElementsParams.reduction = TIDL_ScatterElementsMin;
    }
    else
    {
        layer.layerParams.scatterElementsParams.reduction = TIDL_ScatterElementsUnsupported;
    }

    bool indexIsInt64 = false;

    // Handle constant inputs
    if (md.numConstInputs == 1)
    {
        layer.weights = TIDL_extractConstantTensorData(call, md.constTensorIndices[0], md, 0);
        if(layer.weights.ptr==NULL)
        {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "ScatterND layer : Cannot read constant tensor : Only float, int32 and int64 tensor is supported");
            return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
        }
    }

    if (md.numConstInputs == 2)
    {
        layer.weights = TIDL_extractConstantTensorData(call, md.constTensorIndices[0], md, 0);
        if(layer.weights.ptr==NULL)
        {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "ScatterND layer : Cannot read constant tensor : Only float, int32 and int64 tensor is supported");
            return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
        }
        layer.bias = TIDL_extractConstantTensorData(call, md.constTensorIndices[1], md, 1);
        if(layer.bias.ptr==NULL)
        {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "ScatterND layer : Cannot read constant tensor : Only float, int32 and int64 tensor is supported");
            return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
        }
    }

    // Check if indices are int64 and need conversion to int32
    if (call->args[1].as<tvm::relay::ConstantNode>())
    {
        indexIsInt64 = TIDL_relayIsInt64Type(call->args[1]);
    }

    // tidl offload for scalar inputs have a bug causing inference failure so denying the scalar input cases
    for(int i=0;i<md.numVarInputs;i++)
    {
        if(md.varTensorsDims[i].size()==0)
        {
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
    }

    int32_t result = TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    // Convert int64 indices to int32 if needed
    if (indexIsInt64)
    {
        if (md.constTensorIndices[0] == 1)
        {
            result = convertIndicesToInt32(layer.weights);
        }
        else
        {
            result = convertIndicesToInt32(layer.bias);
        }
        return result;
    }

    return 0;
}
