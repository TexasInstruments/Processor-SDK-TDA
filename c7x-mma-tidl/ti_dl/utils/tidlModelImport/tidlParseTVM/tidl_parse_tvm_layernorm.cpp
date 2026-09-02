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
int32_t TidlParseTVM::parse<OpNameStr("tidl.layer_normalization")>()
{
    int32_t axis, status = 0, numDim;
    float32_tidl epsilon = 1e-5;
    layer.layerType = TIDL_LayerNormLayer;
    layer.numInBufs = 1;
    // layer.numOutBufs = 3;  // LayerNormalization returns 3 outputs: normalized, mean, inv_stdev

    sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;

    const auto* fn_node = call->op.as<tvm::relay::FunctionNode>();
    auto fn = GetRef<tvm::relay::Function>(fn_node);
    const auto* attrs = fn->attrs.as<DictAttrsNode>();

    if (attrs->dict.count("axis"))
    {
        axis = Downcast<Integer>(attrs->dict.at("axis"))->value;
    }
    numDim = layer.allowlistingMetaData.varTensorsDims[0].size();
    if (numDim == 0)
    {
        /*Shape inference is not available - assuming 4 dimensions:*/
        numDim = 4;
    }
    axis = TIDL_relayNormalizeAxis(axis, numDim);
    layer.layerParams.layerNormParams.axis = axis;

    if (attrs->dict.count("epsilon"))
    {
        epsilon = Downcast<FloatImm>(attrs->dict.at("epsilon"))->value;
    }

    if (epsilon == (float32_tidl)0)
    {
        epsilon = epsilon + 1e-5;
    }

    layer.layerParams.layerNormParams.epsilon = epsilon;

    if (md.numConstInputs >= 1)
    {
        layer.weights = TIDL_extractConstantTensorData(call, 1, md, 0);
        if (layer.weights.ptr == NULL || layer.weights.bufSize == 0)
        {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot extract scale/gamma tensor for LayerNormalization");
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
    }
    else
    {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "LayerNormalization requires at least 1 constant input (scale/gamma)");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }

    if (md.numConstInputs >= 2)
    {
        layer.bias = TIDL_extractConstantTensorData(call, 2, md, 1);
        if (layer.bias.ptr == NULL || layer.bias.bufSize == 0)
        {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot extract bias/beta tensor for LayerNormalization");
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
    }

    return 0;
}
