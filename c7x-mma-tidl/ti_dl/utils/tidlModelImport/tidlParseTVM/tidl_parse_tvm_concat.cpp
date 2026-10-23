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
#include <tvm/relay/attrs/transform.h>

template <>
int32_t TidlParseTVM::parse<OpNameStr("concatenate")>()
{
    int32_t dims, axis, type;
    const TupleTypeNode* inTupleType;
    const TensorTypeNode* inTensorType;
    sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;
    layer.layerType = TIDL_ConcatLayer;
    layer.numInBufs = md.numInputs;

    /* check supported datatypes for concat, both inputs and output */
    vector<int32_t> supportedTypes = {TIDL_SignedChar, TIDL_UnsignedChar, TIDL_SignedShort, TIDL_UnsignedShort, TIDL_SinglePrecFloat};
    inTupleType = call->args[0]->checked_type().as<TupleTypeNode>();
    for(auto inputIdx=0; inputIdx < (int32_t)(inTupleType->fields.size()); inputIdx++)
    {
      inTensorType = inTupleType->fields[inputIdx].as<TensorTypeNode>();
      if(find(supportedTypes.begin(), supportedTypes.end(), getTIDLDataTypeFromTVMDataType((int32_t)(inTensorType->dtype.code()), (int32_t)(inTensorType->dtype.bits()))) == supportedTypes.end())
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Concat layer : Unsupported data type");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }
    for(auto outType : md.outputDataTypes)
    {
      if(find(supportedTypes.begin(), supportedTypes.end(), outType) == supportedTypes.end())
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Concat layer : Unsupported data type");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }

    dims = TIDL_relayExprDims(call->args[0]);
    if (dims == 0)
    {
        /*Shape inference is not available - assuming 4 dimensions:*/
        dims = 4;
    }

    auto attrs = call->attrs.as<ConcatenateAttrs>();
    axis = TIDL_relayNormalizeAxis(attrs->axis, dims);
    layer.layerParams.concatParams.axis = axis;

    if (md.numConstInputs == 1)
    {
        layer.weights = TIDL_extractConstantTensorData(call, layer.allowlistingMetaData.constTensorIndices[0], layer.allowlistingMetaData, 0);
        if(layer.weights.ptr==NULL)
        {
          TIDL_LOG_ERROR(gDiags.gDiagList, "Unable to read initializer tensor at index %d", layer.allowlistingMetaData.constTensorIndices[0]);
          return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
    }
    else if(md.numConstInputs > 1)
    {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Concat layer : Only one const input is supported");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }

    return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}
