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
    bool axisFromAttr = false;
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
                    axisFromAttr = true;
                }
            } else if (axes_ref.as<IntImmNode>()) {
                axis = Downcast<Integer>(axes_ref)->value;
                axisFromAttr = true;
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
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Slice layer : Only single axis slice is supported");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    
    if(md.numInputs<4 && !axisFromAttr){
        axis=0;
    }
    else if(md.numInputs==5)
    {
        auto stepsIt = std::find(md.constTensorIndices.begin(), md.constTensorIndices.end(), 4);
        if(stepsIt != md.constTensorIndices.end())
        {
            int32_t stepsConstIdx = std::distance(md.constTensorIndices.begin(), stepsIt);
            buf=TIDL_extractConstantTensorData(call,4,md,stepsConstIdx);
            if(buf.ptr==NULL)
            {
                TIDL_LOG_ERROR(gDiags.gDiagList, "Unable to read initializer tensor at index %d", 4);
                return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
            }
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

    /* Read starts and ends when supplied as constant inputs (Slice-10 style).
     * The axes tensor is optional: some lowerings only supply data+starts+ends
     * (axis is implied by attrs / defaults to 0), so starts/ends must not be
     * gated on an axes tensor being present as well. */
    auto startsIt = std::find(md.constTensorIndices.begin(), md.constTensorIndices.end(), 1);
    auto endsIt   = std::find(md.constTensorIndices.begin(), md.constTensorIndices.end(), 2);
    auto axesIt   = std::find(md.constTensorIndices.begin(), md.constTensorIndices.end(), 3);
    if(startsIt != md.constTensorIndices.end() &&
       endsIt   != md.constTensorIndices.end())
    {
        int32_t startsConstIdx = std::distance(md.constTensorIndices.begin(), startsIt);
        int32_t endsConstIdx   = std::distance(md.constTensorIndices.begin(), endsIt);

        sBuffer_t starts;
        starts=TIDL_extractConstantTensorData(call,1,md,startsConstIdx);
        if(starts.ptr==NULL)
        {
            TIDL_LOG_ERROR(gDiags.gDiagList, "Unable to read initializer tensor at index %d", 1);
            return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
        }
        long long int *startsPtr = (long long int *)starts.ptr;

        sBuffer_t ends;
        ends=TIDL_extractConstantTensorData(call,2,md,endsConstIdx);
        if(ends.ptr==NULL)
        {
            TIDL_LOG_ERROR(gDiags.gDiagList, "Unable to read initializer tensor at index %d", 2);
            return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
        }
        long long int *endsPtr = (long long int *)ends.ptr;

        if(starts.bufSize > 1)
        {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowisting : Slice layer : Only single axis slice is supported");
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
        else
        {
            sliceParams.slicePoints[0] = startsPtr[0];
            if(endsPtr[0] == 0x7fffffffffffffff)
            {
                sliceParams.slicePoints[1] = 0x7fffffff;
            }
            else if(endsPtr[0] == (-0x7fffffffffffffff))
            {
            sliceParams.slicePoints[1] = -0x7fffffff;
            }
            else
            {
                sliceParams.slicePoints[1] = endsPtr[0];
            }
        }
        free(startsPtr);
        starts.ptr = NULL;
        starts.bufSize = 0;
        free(endsPtr);
        ends.ptr = NULL;
        ends.bufSize = 0;

        /* axes tensor is optional; only override axis when it is actually present */
        if(axesIt != md.constTensorIndices.end())
        {
            int32_t axesConstIdx = std::distance(md.constTensorIndices.begin(), axesIt);
            sBuffer_t axes;
            axes=TIDL_extractConstantTensorData(call,3,md,axesConstIdx);
            if(axes.ptr==NULL)
            {
                TIDL_LOG_ERROR(gDiags.gDiagList, "Unable to read initializer tensor at index %d", 3);
                return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
            }
            long long int *axesPtr = (long long int *)axes.ptr;
            axis = axesPtr[0];
            free(axesPtr);
            axes.ptr = NULL;
            axes.bufSize = 0;
        }
    }

    /** Get numDim from data tensor (input 0), whether it is variable or const */
    numDim = TIDL_getNumDimsForTensorIndex(md, 0);

    /** Adjust axis to TIDL dimensions*/
    axis = TIDL_relayNormalizeAxis(axis, numDim);

    layer.layerParams.sliceParams.axis = axis;

    /* If data input (input 0) is a constant, store it in layer.weights
     * so tidl_addConstDataLayers can create the appropriate ConstDataLayer */
    auto dataConstIt = std::find(md.constTensorIndices.begin(), md.constTensorIndices.end(), 0);
    if (dataConstIt != md.constTensorIndices.end())
    {
        int32_t dataConstIdx = std::distance(md.constTensorIndices.begin(), dataConstIt);
        layer.weights = TIDL_extractConstantTensorData(call, 0, md, dataConstIdx);
        if (layer.weights.ptr == NULL)
        {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Slice layer : Unable to read constant data input");
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
    }

  return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}
