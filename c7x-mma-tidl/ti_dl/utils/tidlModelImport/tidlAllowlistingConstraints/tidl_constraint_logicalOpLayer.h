/*
*
* Copyright (c) {2015 - 2026} Texas Instruments Incorporated
*
* All rights reserved not granted herein.
*
* Limited License.
*
* Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
* license under copyrights and patents it now or hereafter owns or controls to make,
* have made, use, import, offer to sell and sell ("Userize") this software subject to the
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

const vector<TidlConstraint> tidlConstraintLogicalOpLayer = 
{
    TIDL_CSTR(
        "Number of non-singleton variable input dimensions must be <= 6",
        "Number of non-singleton variable input dimensions must be <= 6",
        "Number of non-singleton variable input dimensions must be <= 6",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            if(gParams.modelType != TIDL_IMPORT_MODEL_FORMAT_ONNX)
            {
                for(auto &varDims : md.varTensorsDims)
                {
                    int32_t numDims = tidlGetNonSingletonNumDims(varDims);
                    if(numDims > 6)
                    {
                        oss << "Maximum number of input dimensions supported are 6, found " << numDims << " input dimensions";
                        logs = oss.str();
                        return false;
                    }
                }
            }
            return true;
        }
    ),
     TIDL_CSTR(
        "Input tensor cannot be a constant for Not, IsInf and IsNan operator",
        "Input tensor cannot be a constant for Not, IsInf and IsNan operator",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            int32_t opType = layer->layerParams.logicalOpLayerParams.operatorType;
            if (opType == TIDL_Not || opType == TIDL_IsInf || opType == TIDL_IsNaN)
            {
                if(md.constTensorIndices.size() > 0)
                {
                    return false;
                }
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Condition cannot be a constant input for Where operator",
        "Condition cannot be a constant input for Where operator",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            int32_t opType = layer->layerParams.logicalOpLayerParams.operatorType;
            if (opType == TIDL_Where)
            {
                for(int32_t i = 0; i < md.constTensorIndices.size(); i++)
                {
                    if (md.constTensorIndices[i] != 1 && md.constTensorIndices[i] != 2)
                    {
                        return false;
                    }
                }
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Only boolean inputs/outputs are supported for And, Or, Xor and Not operator",
        "Only boolean inputs/outputs are supported for And, Or, Xor and Not operator",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            int32_t opType = layer->layerParams.logicalOpLayerParams.operatorType;
            if (opType == TIDL_And || opType == TIDL_Or || opType == TIDL_Xor || opType == TIDL_Not)
            {
                for(int32_t i = 0; i < md.inputDataTypes.size(); i++)
                {
                    if (md.inputDataTypes[i] != TIDL_Bool)
                    {
                        return false;
                    }
                }

                for(int32_t i = 0; i < md.outputDataTypes.size(); i++)
                {
                    if (md.outputDataTypes[i] != TIDL_Bool)
                    {
                        return false;
                    }
                }
            }

            return true;
        }
    ),
    TIDL_CSTR(
        "Only boolean, integer (uint8, int8, uint16, int16, uint32, int32) & float32 inputs and boolean output is supported for Equal, Greater, GreaterOrEqual, Less and LessOrEqual operator",
        "Only boolean, integer (uint8, int8, uint16, int16, uint32, int32) & float32 inputs and boolean output is supported for Equal, Greater, GreaterOrEqual, Less and LessOrEqual operator",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            int32_t opType = layer->layerParams.logicalOpLayerParams.operatorType;
            if (opType == TIDL_Equal || opType == TIDL_Greater || opType == TIDL_GreaterOrEqual ||
                opType == TIDL_Less || opType == TIDL_LessOrEqual)
            {
                for(int32_t i = 0; i < md.inputDataTypes.size(); i++)
                {
                    int32_t inputType = md.inputDataTypes[i];
                    if (inputType != TIDL_Bool && inputType != TIDL_UnsignedChar && inputType != TIDL_SignedChar &&
                        inputType != TIDL_UnsignedShort && inputType != TIDL_SignedShort && inputType != TIDL_UnsignedWord &&
                        inputType != TIDL_SignedWord && inputType != TIDL_SinglePrecFloat)
                    {
                        return false;
                    }
                }

                for(int32_t i = 0; i < md.outputDataTypes.size(); i++)
                {
                    if (md.outputDataTypes[i] != TIDL_Bool)
                    {
                        return false;
                    }
                }
            }

            return true;
        }
    ),
    TIDL_CSTR(
        "Only float32 input and boolean output is supported for IsInf and IsNan operator",
        "Only float32 input and boolean output is supported for IsInf and IsNan operator",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            int32_t opType = layer->layerParams.logicalOpLayerParams.operatorType;
            if (opType == TIDL_IsInf || opType == TIDL_IsNaN)
            {
                for(int32_t i = 0; i < md.inputDataTypes.size(); i++)
                {
                    if (md.inputDataTypes[i] != TIDL_SinglePrecFloat)
                    {
                        return false;
                    }
                }

                for(int32_t i = 0; i < md.outputDataTypes.size(); i++)
                {
                    if (md.outputDataTypes[i] != TIDL_Bool)
                    {
                        return false;
                    }
                }
            }

            return true;
        }
    ),
    TIDL_CSTR(
        "Only boolean condition input is supported for Where operator",
        "Only boolean condition input is supported for Where operator",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            int32_t opType = layer->layerParams.logicalOpLayerParams.operatorType;
            if (opType == TIDL_Where)
            {
                if(md.inputDataTypes[0] != TIDL_Bool)
                {
                    return false;
                }
            }

            return true;
        }
    ),
    TIDL_CSTR(
        "Only boolean, integer (uint8, int8, uint16, int16, uint32, int32) & float32 inputs (X and Y) is supported for Where operator",
        "Only boolean, integer (uint8, int8, uint16, int16, uint32, int32) & float32 inputs (X and Y) is supported for Where operator",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            int32_t opType = layer->layerParams.logicalOpLayerParams.operatorType;
            if (opType == TIDL_Where)
            {
                if(md.inputDataTypes.size() > 2)
                {
                    int32_t xType = md.inputDataTypes[1], yType = md.inputDataTypes[2];
                    if (xType != TIDL_Bool && xType != TIDL_UnsignedChar && xType != TIDL_SignedChar &&
                        xType != TIDL_UnsignedShort && xType != TIDL_SignedShort && xType != TIDL_UnsignedWord &&
                        xType != TIDL_SignedWord && xType != TIDL_SinglePrecFloat)
                    {
                        return false;
                    }

                    if (yType != TIDL_Bool && yType != TIDL_UnsignedChar && yType != TIDL_SignedChar &&
                        yType != TIDL_UnsignedShort && yType != TIDL_SignedShort && yType != TIDL_UnsignedWord &&
                        yType != TIDL_SignedWord && yType != TIDL_SinglePrecFloat)
                    {
                        return false;
                    }
                }
            }

            return true;
        }
    ),
};