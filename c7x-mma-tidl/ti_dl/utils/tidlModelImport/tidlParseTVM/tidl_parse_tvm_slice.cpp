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

template<> int32_t TidlParseTVM::parse<OpNameStr("tidl.slice")>()
{
    int32_t axis = 0, numDim,isMultipleAxis = 1; 
    sTIDL_SliceLayerParams_t &sliceParams = layer.layerParams.sliceParams;
    sBuffer_t buf;
    sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;
    layer.layerType = TIDL_SliceLayer;
    sliceParams.stride = 1;

    const auto* fn_node = call->op.as<tvm::relay::FunctionNode>();
    auto fn = GetRef<tvm::relay::Function>(fn_node);
    const auto* attrs = fn->attrs.as<DictAttrsNode>();
    if(attrs)
    {
        if (attrs->dict.count("axes")) 
        {
            auto axes_ref = attrs->dict.at("axes");
            if (axes_ref.as<ArrayNode>()) {
                auto axes_array = Downcast<Array<Integer>>(axes_ref);
                if (axes_array.size() > 0) {
                    axis = axes_array[0]->value;
                }
            } else if (axes_ref.as<IntImmNode>()) {
                axis = Downcast<Integer>(axes_ref)->value;
            }
        }
        if (attrs->dict.count("starts")) 
        {
            auto starts_ref = attrs->dict.at("starts");
            if (starts_ref.as<ArrayNode>()) {
                auto starts_array = Downcast<Array<Integer>>(starts_ref);
                isMultipleAxis = starts_array.size();  

                if (starts_array.size() > 0) {
                    sliceParams.slicePoints[0] = starts_array[0]->value;
                }
            } else if (starts_ref.as<IntImmNode>()) {
                sliceParams.slicePoints[0] = Downcast<Integer>(starts_ref)->value;
                isMultipleAxis = 1;
            }

        }
        if (attrs->dict.count("ends")) 
        {
            auto ends_ref = attrs->dict.at("ends");

            if (ends_ref.as<ArrayNode>()) {
                auto ends_array = Downcast<Array<Integer>>(ends_ref);
                if (ends_array.size() > 0) {
                    sliceParams.slicePoints[1] = ends_array[0]->value;
                }
                // DO NOT update isMultipleAxis here
            } else if (ends_ref.as<IntImmNode>()) {
                sliceParams.slicePoints[1] = Downcast<Integer>(ends_ref)->value;
            }
        }
        
    }
    if(isMultipleAxis!=1)
    {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Slice layer : TIDL_RT only supports single slice axis");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    
    if(md.numInputs<4){
        axis=0;
    }
    else if(md.numInputs==5)
    {
        if(call->args[4].as<tvm::relay::ConstantNode>())
        {    
            buf=TIDL_extractConstantTensorData(call,4,md,3);
            long long int *ptr = (long long int *)buf.ptr;
            if (ptr[0] != 1)
            {
                sliceParams.stride = ptr[0];
            }
            free(ptr);
            buf.ptr = NULL;
            buf.bufSize = 0;
        }

    }

    if(md.numConstInputs>=3)
    {
        sBuffer_t starts;
        starts=TIDL_extractConstantTensorData(call,1,md,0);
        long long int *startsPtr = (long long int *)starts.ptr;

        sBuffer_t ends;
        ends= buf=TIDL_extractConstantTensorData(call,2,md,1);
        long long int *endsPtr = (long long int *)ends.ptr;

        sBuffer_t axes;
        axes= buf=TIDL_extractConstantTensorData(call,3,md,2);
        long long int *axesPtr = (long long int *)axes.ptr;
    
        if(starts.bufSize > 1)
        {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowisting : Slice layer : TIDL_RT only supports single slice axis");
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
        else
        {
            sliceParams.slicePoints[0] = startsPtr[0];
            if(endsPtr[0] == 0x7fffffffffffffff)
            {
                sliceParams.slicePoints[1] = 0x7fffffff;
            }
            else
            {
                sliceParams.slicePoints[1] = endsPtr[0];
            }
            axis = axesPtr[0];
        }
        free(startsPtr);
        starts.ptr = NULL;
        starts.bufSize = 0;
        free(endsPtr);
        ends.ptr = NULL;
        ends.bufSize = 0;
        free(axesPtr);
        axes.ptr = NULL;
        axes.bufSize = 0;
    }
    if (layer.allowlistingMetaData.varTensorsDims.size() != 0)
    {
        numDim = layer.allowlistingMetaData.varTensorsDims[0].size();
        numDim = (numDim == 0)? 4:numDim;
    }
    else
    {
        numDim = 4;
    }

    /** Adjust axis to TIDL dimensions*/
    axis = TIDL_relayNormalizeAxis(axis, numDim);
    
    layer.layerParams.sliceParams.axis = axis;

  return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}
