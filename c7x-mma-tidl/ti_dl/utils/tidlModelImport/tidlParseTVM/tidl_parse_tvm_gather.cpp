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
using namespace tvm;
#include <tvm/relay/function.h>
#include <tvm/relay/expr.h>
#include <tvm/relay/analysis.h>
#include <tvm/runtime/ndarray.h>
#include <cmath>

template<>
int32_t TidlParseTVM:: parse<OpNameStr("tidl.gather")> ()
{
    int32_t status = 0, axis, numDim;
    layer.layerType = TIDL_GatherLayer;

    sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;
    layer.numInBufs = md.numInputs;
    layer.layerParams.gatherParams.viewChannels = 1;
    layer.layerParams.gatherParams.thresholdPadSize = 0;
    layer.layerParams.gatherParams.thresholdPadValue = 0;

    const auto* fn_node = call->op.as<tvm::relay::FunctionNode>();
    auto fn = GetRef<tvm::relay::Function>(fn_node);
    const auto* attrs = fn->attrs.as<DictAttrsNode>();

    /** If shape inference is not done on model, we assume the tensor is of 4 dimensions by default.
     *  numDim must reflect the DATA tensor rank for correct axis translation. */
    if (md.numConstInputs > 0 && md.constTensorIndices[0] == 0)
    {
        // data is const: get rank from constTensorsDims
        numDim = md.constTensorsDims[0].size();
        numDim = (numDim == 0) ? 4 : numDim;
    }
    else if (md.varTensorsDims.size() != 0)
    {
        numDim = md.varTensorsDims[0].size();
        numDim = (numDim == 0) ? 4 : numDim;
    }
    else
    {
        numDim = 4;
    }

    // Get axis attribute - default to 0 if not specified
    // For TVM, we need to check if there's an attrs field and extract axis from there
    axis = 0;  // Default axis value
    if (attrs->dict.count("axis"))
    {
        axis = Downcast<Integer>(attrs->dict.at("axis"))->value;
    }

    /* Translate to TIDL axis exactly as the ONNX Gather parser does (avoid
     * TIDL_relayNormalizeAxis here: its NHWC channel-swap remap for axis
     * values 1/2/3 does not apply to Gather and diverges from ONNX). */
    if (axis >= 0)
    {
        axis += (TIDL_DIM_MAX - numDim);
    }
    else
    {
        axis = (TIDL_DIM_MAX + axis);
    }
    layer.layerParams.gatherParams.axis = axis;
    layer.layerParams.gatherParams.isIdxScalar = 0;

    if (md.numConstInputs > 1)
    {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Gather layer : Both data and indices as constants is not supported");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }

    // Handle constant indices input
    if (md.numConstInputs > 0)
    {
        int32_t constArgIdx = md.constTensorIndices[0];
        if (constArgIdx == 0)
        {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Data cannot be a constant. Only indices can be constant.");
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }

        // Extract the indices data
        sBuffer_t buf = TIDL_extractConstantTensorData(call, constArgIdx, md, 0);
        if(buf.ptr==NULL)
        {
            TIDL_LOG_ERROR(gDiags.gDiagList, "Unable to read initializer tensor at index %d", constArgIdx);
            return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
        }
        if(md.constTensorsDims[0].size()==0)
        {
            layer.layerParams.gatherParams.isIdxScalar = 1;
        }
        // Decide the conversion based on what TIDL_extractConstantTensorData actually
        // produced (buf.dataType), not on a separately re-derived relay dtype check --
        // those can disagree (e.g. a float-typed constant), which silently reinterprets
        // float bytes as int32 garbage.
        if (buf.dataType == TIDL_SignedDoubleWord || buf.dataType == TIDL_UnsignedDoubleWord)
        {
             // TIDL_GatherLayer doesn't support int64 indices, hence convert them to int32
            int32_t *index_dst_ptr = (int32_t*)malloc(sizeof(int32_t) * buf.bufSize);

            if (index_dst_ptr == NULL)
            {
                TIDL_LOG_ERROR(gDiags.gDiagList, "GatherLayer : Could not allocate memory for gather constant indices");
                return -1;
            }

            int64_t *index_src_ptr = (int64_t*)buf.ptr;
            int64_t val = 0;

            for (int32_t j = 0; j < buf.bufSize; j++)
            {
                val = index_src_ptr[j];
                index_dst_ptr[j] = (int32_t)val;
            }

            layer.weights.ptr = index_dst_ptr;
            layer.weights.bufSize = buf.bufSize;
            layer.weights.dataType = TIDL_SignedWord;
            /* Update the buffer pointer*/
            free(buf.ptr);
        }
        else if (buf.dataType == TIDL_SinglePrecFloat)
        {
            // Constant-folded indices can come back as float; round to the
            // nearest integer rather than reinterpreting the float bytes as int32.
            int32_t *index_dst_ptr = (int32_t*)malloc(sizeof(int32_t) * buf.bufSize);

            if (index_dst_ptr == NULL)
            {
                TIDL_LOG_ERROR(gDiags.gDiagList, "GatherLayer : Could not allocate memory for gather constant indices");
                return -1;
            }

            float *index_src_ptr = (float*)buf.ptr;

            for (int32_t j = 0; j < buf.bufSize; j++)
            {
                index_dst_ptr[j] = (int32_t)lroundf(index_src_ptr[j]);
            }

            layer.weights.ptr = index_dst_ptr;
            layer.weights.bufSize = buf.bufSize;
            layer.weights.dataType = TIDL_SignedWord;
            free(buf.ptr);
        }
        else
        {
            // Already int32, just assign
            layer.weights.ptr = buf.ptr;
            layer.weights.bufSize = buf.bufSize;
            layer.weights.dataType = TIDL_SignedWord;
        }
    }

    int8_t dataIsVar = 0, indicesIsVar = 0;
    for (int32_t i = 0; i < md.varTensorIndices.size(); i++)
    {
        if (md.varTensorIndices[i] == 0) dataIsVar = 1;
        if (md.varTensorIndices[i] == 1) indicesIsVar = 1;
    }

    int32_t dataDimCount = 0, indicesDimCount = 0;

    if (dataIsVar && indicesIsVar)
    {
        dataDimCount = md.varTensorsDims[0].size();
        indicesDimCount = md.varTensorsDims[1].size();
    }
    // Data is variable and indices is constant
    else if (dataIsVar && !indicesIsVar)
    {
        dataDimCount = md.varTensorsDims[0].size();
        indicesDimCount = md.constTensorsDims[0].size();
    }
    // Data is constant and indices is variable
    else if (!dataIsVar && indicesIsVar)
    {
        dataDimCount = md.constTensorsDims[0].size();
        indicesDimCount = md.varTensorsDims[0].size();
    }

    layer.inData[0].numDim = dataDimCount;
    layer.inData[1].numDim = indicesDimCount;

    return 0;
}
