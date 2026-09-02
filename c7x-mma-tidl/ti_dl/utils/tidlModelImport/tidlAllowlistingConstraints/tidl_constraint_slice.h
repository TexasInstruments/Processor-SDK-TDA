/*
*
* Copyright (c) {2015 - 2024} Texas Instruments Incorporated
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

#include "tidl_constraint.h"

const vector<TidlConstraint> tidlConstraintSlice = {
    TIDL_CSTR(
        "Must have 4 inputs",
        "Must have 4 inputs",
        "Must have 4 inputs",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            if((gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE_RT) || (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE))   
            {
                if(md.numInputs != 4)
                {
                    stringstream ss;
                    ss<<"Must have 4 inputs supported, got "<< md.numInputs;
                    logs = ss.str();
                    return false;
                }
            }    
            return true;     
        }
    ),
    TIDL_CSTR(
        "Only one input should be variable",
        "Only one input should be variable",
        "Only one input should be variable",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            if((gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE_RT) || (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE))   
            {
                if(md.numVarInputs != 1)
                {
                    return false;
                }
            }    
            return true;     
        }
    ),
    TIDL_CSTR(
        "Layer should have 3 constant/initializer inputs",
        "Should have 3 constant inputs",
        "Should have 3 constant inputs",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            if((gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE_RT) || (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE))   
            {
                if(md.numConstInputs != 3)
                {
                    stringstream ss;
                    ss<<"Layer should have 3 constant/initializer inputs, got "<<md.numConstInputs;
                    logs = ss.str();
                    return false;
                }
            }    
            return true;     
        }
    ),        
    TIDL_CSTR(
        "Number of dimensions for variable inputs must be 4",
        "Number of dimensions for variable inputs must be 4",
        "Number of dimensions for variable inputs must be 4",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            if((gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE_RT) || (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE))   
            {
                if(md.varTensorsDims[0].size() != 4)
                {
                    stringstream ss;
                    ss<<"Number of dimensions for variable inputs must be 4, found "<<md.varTensorsDims[0].size();
                    logs = ss.str();
                    return false;
                }
            }    
            return true;     
        }
    ),       
    TIDL_CSTR(
        "Number of dimensions for constant/initializer inputs must be 1",
        "Number of dimensions for constant inputs must be 1",
        "Number of dimensions for constant inputs must be 1",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            if((gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE_RT) || (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE))   
            {
                for(int i = 0; i < md.constTensorsDims.size(); i++)
                {
                    if(md.constTensorsDims[i].size() != 1)
                    {
                        stringstream ss;
                        ss<<"Number of dimensions for constant/initializer inputs must be 1, found "<<md.constTensorsDims[i].size()<<"for constant input "<<i;
                        logs = ss.str();
                        return false;
                    }
                }
            }    
            return true;     
        }
    ),   
    TIDL_CSTR(
        "Constant/initializer inputs must have 4 values",
        "Constant inputs must have 4 values",
        "Constant inputs must have 4 values",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            if((gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE_RT) || (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE))   
            {
                for(int i = 0; i < md.constTensorsDims.size(); i++)
                {
                    if(md.constTensorsDims[i][0] != 4)
                    {
                        stringstream ss;
                        ss<<"Constant/initializer inputs must have 4 values, got "<<md.constTensorsDims[i][0];
                        logs = ss.str();
                        return false;
                    }
                }
            }    
            return true;     
        }
    ), 
    TIDL_CSTR(
        "Number of dimensions for output tensor must be 4",
        "Number of dimensions for output tensor must be 4",
        "Number of dimensions for output tensor must be 4",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            if((gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE_RT) || (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE))   
            {
                if(md.outputTensorDims[0].size() != 4)
                {
                    stringstream ss;
                    ss<<"Number of dimensions for output tensor must be 4, got "<<md.outputTensorDims[0].size();
                    logs = ss.str();
                    return false;
                }
            }    
            return true;     
        }
    ),    
    TIDL_CSTR(
        "Only batch size = 1 is supported",
        "Only batch size = 1 is supported",
        "Only batch size = 1 is supported",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            if((gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE_RT) || (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE))   
            {
                if(md.varTensorsDims[0][0] != 1)
                {
                    stringstream ss;
                    ss<<"Only batch size = 1 supported, got batch size = "<<md.varTensorsDims[0][0];
                    logs = ss.str();
                    return false;
                }
            }    
            return true;     
        }
    ), 
    TIDL_CSTR(
        "Slice with stride greater than 0 is supported",
        "Slice with stride greater than 0 is supported",
        "Slice with stride greater than 0 is supported",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            auto params = layer->layerParams.sliceParams;
            if (params.stride < 0)
            {
                stringstream ss;
                ss<<"Only slice with stride > 0 is supported, stride found = "<<params.stride;
                logs = ss.str();
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Only supports non-batch dimension",
        "Only supports non-batch dimension",
        "Only supports non-batch dimension",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            auto params = layer->layerParams.sliceParams;
            if (params.axis == TIDL_DIM_BATCH)
            {
                return false;
            }   
            return true;
        }
    ),
};