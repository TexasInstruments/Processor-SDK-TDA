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
#include <tvm/relay/function.h>
#include <tvm/relay/expr.h>
using namespace std;

template<>
int32_t TidlParseTVM::parse<OpNameStr("tidl.gather_elements")> ()
{
    layer.layerType = TIDL_GatherElementsLayer;
    layer.numInBufs = layer.allowlistingMetaData.numInputs;

    sTIDL_allowlistingMetaData &md = layer.allowlistingMetaData;

    const auto* fn_node = call->op.as<tvm::relay::FunctionNode>();
    auto fn = GetRef<tvm::relay::Function>(fn_node);
    const auto* attrs = fn->attrs.as<DictAttrsNode>();

    // Determine numDim from the data tensor (input 0) for axis normalization
    int32_t numDim = 4;
    if (md.numConstInputs > 0 && md.constTensorIndices[0] == 0)
    {
        numDim = md.constTensorsDims[0].size();
        numDim = (numDim == 0) ? 4 : numDim;
    }
    else if (!md.varTensorsDims.empty())
    {
        numDim = md.varTensorsDims[0].size();
        numDim = (numDim == 0) ? 4 : numDim;
    }

    // axis attribute
    int32_t axis = 0;
    if (attrs->dict.count("axis"))
    {
        axis = (int32_t)Downcast<Integer>(attrs->dict.at("axis"))->value;
    }

    if (axis >= 0)
        axis += (TIDL_DIM_MAX - numDim);
    else
        axis = (TIDL_DIM_MAX + axis);

    layer.layerParams.gatherElementsParams.axis        = axis;
    layer.layerParams.gatherElementsParams.isIdxScalar = 0;

    if (md.numConstInputs > 1)
    {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList,
            "GatherElements layer : Both data and indices as constants is not supported");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }

    if (md.numConstInputs > 0)
    {
        int32_t constTensorIdx = md.constTensorIndices[0];
        sBuffer_t buf = TIDL_extractConstantTensorData(call, constTensorIdx, md, 0);
        if (buf.ptr == nullptr)
        {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList,
                "GatherElements layer : Cannot read constant input tensor");
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }

        // Check if index tensor is scalar
        if (constTensorIdx == 1 && md.constTensorsDims[0].empty())
        {
            layer.layerParams.gatherElementsParams.isIdxScalar = 1;
        }

        // TIDL_GatherElementsLayer doesn't support int64 indices — down-convert to int32
        if (constTensorIdx == 1 && TIDL_relayIsInt64Type(call->args[1]))
        {
            int32_t status = convertIndicesToInt32(buf);
            if (status != TIDL_ALLOWLISTING_LAYER_CHECK_PASSED)
                return status;
        }

        layer.weights = buf;
    }

    return 0;
}
