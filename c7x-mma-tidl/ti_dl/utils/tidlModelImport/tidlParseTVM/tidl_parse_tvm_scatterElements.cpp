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


template<> int32_t TidlParseTVM::parse<OpNameStr("scatter_elements")> ()
{
    
    
    layer.layerType = TIDL_ScatterElementsLayer;
    
    layer.numInBufs = layer.allowlistingMetaData.numInputs;
    
    
    // auto name_hint=DLDataType2String(call->args[1]->name_hint().as<TensorTypeNode>()->dtype);
    int32_t numDim;
    sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;
    string reduction;
    if (call->attrs.as<ScatterElementsAttrs>() != NULL)
    {
        auto attrs=call->attrs.as<ScatterElementsAttrs>();
        //setting block size
        
        layer.layerParams.scatterElementsParams.axis = static_cast<int32_t>(attrs->axis->value);
        reduction = attrs->reduction;


    }
    else
    {
        layer.layerParams.scatterElementsParams.axis = 0;
        reduction="update";
    }

    /** If shape inference is not done on model, we assume the tensor is of 4 dimensions by default*/
    if (layer.allowlistingMetaData.varTensorsDims.size() != 0)
    {
        numDim = layer.allowlistingMetaData.varTensorsDims[0].size();
        numDim = (numDim == 0)? 4:numDim;
    }
    else
    {
        numDim= 4;
    }

    /** Adjust axis to TIDL dimensions*/
     layer.layerParams.scatterElementsParams.axis = TIDL_relayNormalizeAxis(layer.layerParams.scatterElementsParams.axis, numDim);

    strcpy((char*)layer.layerParams.scatterElementsParams.dataTensorName, "input");
    strcpy((char*)layer.layerParams.scatterElementsParams.indicesTensorName,"indices" );
    strcpy((char*)layer.layerParams.scatterElementsParams.updateTensorName,"updates");
    
    if( (strcmp(reduction.c_str(), "update") == 0))
    {  
        layer.layerParams.scatterElementsParams.reduction = TIDL_ScatterElementsNone;
    }
    else if(strcmp(reduction.c_str(), "add") == 0)
    {
        layer.layerParams.scatterElementsParams.reduction = TIDL_ScatterElementsAdd;
    }
    else if(strcmp(reduction.c_str(), "max") == 0)
    {
        layer.layerParams.scatterElementsParams.reduction = TIDL_ScatterElementsMax;
    }
    else if(strcmp(reduction.c_str(), "min") == 0)
    {
        layer.layerParams.scatterElementsParams.reduction =  TIDL_ScatterElementsMin;
    }
    else 
    {
        layer.layerParams.scatterElementsParams.reduction =  TIDL_ScatterElementsUnsupported;
    }
    bool indexIsInt64 = false;
    string varName;

    if(md.numConstInputs == 1)
    {
        layer.weights = TIDL_extractConstantTensorData(call, layer.allowlistingMetaData.constTensorIndices[0], layer.allowlistingMetaData, 0 );
        if(layer.weights.ptr==NULL)
        {
            TIDL_LOG_ERROR(gDiags.gDiagList, "Unable to read initializer tensor at index %d", layer.allowlistingMetaData.constTensorIndices[0]);
            return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
        }
    }

    if(md.numConstInputs == 2)
    {
        layer.weights = TIDL_extractConstantTensorData(call, layer.allowlistingMetaData.constTensorIndices[0], layer.allowlistingMetaData, 0);
        if(layer.weights.ptr==NULL)
        {
            TIDL_LOG_ERROR(gDiags.gDiagList, "Unable to read initializer tensor at index %d", layer.allowlistingMetaData.constTensorIndices[0]);
            return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
        }
        layer.bias = TIDL_extractConstantTensorData(call, layer.allowlistingMetaData.constTensorIndices[1], layer.allowlistingMetaData, 1);
        if(layer.bias.ptr==NULL)
        {
            TIDL_LOG_ERROR(gDiags.gDiagList, "Unable to read initializer tensor at index %d", layer.allowlistingMetaData.constTensorIndices[1]);
            return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
        }
    }

    if(call->args[1].as<tvm::relay::ConstantNode>())
    {
        indexIsInt64 = TIDL_relayIsInt64Type(call->args[1]);
    }
    int32_t result =-1;
    // Convert int64 indices to int32 if needed
    if(indexIsInt64)
    {
        if(layer.allowlistingMetaData.constTensorIndices[0]==1)
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
