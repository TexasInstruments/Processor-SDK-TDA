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

#include "tidl_constraint.h"

const vector<TidlConstraint> tidlConstraintScatterElements =
{
    TIDL_CSTR(
        "Number of non-singleton variable input dimensions must be <= 4",
        "Number of non-singleton variable input dimensions must be <= 4",
        "Number of non-singleton variable input dimensions must be <= 4",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            int32_t numDims = tidlGetNonSingletonNumDims(layer->allowlistingMetaData.varTensorsDims[0]);
            if(numDims > 4)
            {
                oss << "Maximum number of input dimension supported is 4, found " << numDims << " input dimensions";
                logs = oss.str();
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "ScatterElements only supports the 'none' reduction type",
        "ScatterElements only supports the 'none' reduction type",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            int32_t axis = layer->layerParams.scatterElementsParams.axis;
            int32_t reduction = layer->layerParams.scatterElementsParams.reduction;
            int32_t isScatterNd = (axis == -1) ? 1 : 0;
            if (!isScatterNd && ((reduction != TIDL_ScatterElementsNone)))
            {
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "ScatterND does not support the 'mul' reduction type",
        "ScatterND does not support the 'mul' reduction type",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            int32_t axis = layer->layerParams.scatterElementsParams.axis;
            int32_t reduction = layer->layerParams.scatterElementsParams.reduction;
            int32_t isScatterNd = (axis == -1) ? 1 : 0;
            if (isScatterNd && reduction == TIDL_ScatterElementsUnsupported)
            {
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Updates tensor should not have more than 1 channel",
        "Updates tensor should not have more than 1 channel",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            int32_t axis = layer->layerParams.scatterElementsParams.axis;
            int32_t reduction = layer->layerParams.scatterElementsParams.reduction;
            int32_t isScatterNd = (axis == -1) ? 1 : 0;
            if(isScatterNd && (reduction == TIDL_ScatterElementsAdd))
            {
                sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
                if(md.varTensorsDims.size() > 2)
                {
                    int32_t varTensorChDim = md.varTensorsDims[2].size() - 3;
                    if(md.varTensorsDims[2].size() > varTensorChDim)
                    {
                        if(md.varTensorsDims[2][varTensorChDim] > 1)
                        {
                            return false;
                        }
                    }
                }
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Only scatter along width axis is supported",
        "Only scatter along width axis is supported",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            int32_t axis = layer->layerParams.scatterElementsParams.axis;
            int32_t isScatterNd = (axis == -1) ? 1 : 0;
            if((!isScatterNd) && (axis != TIDL_DIM_WIDTH))
            {
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "For ScatterND, indices tensor's last dimension must be at most (output_rank - 1)",
        "For ScatterND, indices tensor's last dimension must be at most (output_rank - 1)",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            int32_t shapeInferenceStatus = tidlCheckShapeInferenceforOnnx(md);

            int32_t axis = layer->layerParams.scatterElementsParams.axis;
            int32_t isScatterNd = (axis == -1) ? 1 : 0;
            vector<int32_t> indicesDims;
            int32_t dataIsVar = 0, indIsVar = 0;

            if (shapeInferenceStatus != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL && ((gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX_RT)||(gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TVM_RELAY)))
            {
                if(isScatterNd)
                {
                    for (int i = 0; i < md.varTensorIndices.size(); i++)
                    {
                        if (md.varTensorIndices[i] == 0)
                        {
                            dataIsVar = 1;
                        }
                        if (md.varTensorIndices[i] == 1)
                        {
                            indIsVar = 1;
                        }
                    }

                    // Both data and indices are variable
                    if(dataIsVar && indIsVar)
                    {
                        indicesDims = md.varTensorsDims[1];
                    }

                    // Data is variable and indices is constant
                    else if (dataIsVar && !indIsVar)
                    {
                        indicesDims = md.constTensorsDims[0];
                    }

                    // Data is constant and indices is variable
                    else if (!dataIsVar && indIsVar)
                    {
                        indicesDims = md.varTensorsDims[0];
                    }
                    
                    // Both data and indices are constant
                    else
                    {
                        indicesDims = md.constTensorsDims[1];
                    }

                    if((md.outputTensorDims[0].size() - indicesDims[indicesDims.size()-1]) >= 2)
                    {
                        return false;
                    }
                }
            }
            return true;
        }
    ), 
    TIDL_CSTR(
        "The constant input 'data' must be a zero tensor",
        "The constant input 'data' must be a zero tensor",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            int32_t axis = layer->layerParams.scatterElementsParams.axis;
            int32_t isScatterNd = (axis == -1) ? 1 : 0;
            int32_t dataIsConst = 0;

            if(isScatterNd && layer->layerParams.scatterElementsParams.reduction != TIDL_ScatterElementsNone)
            {
                for (int i = 0; i < md.constTensorIndices.size(); i++)
                {
                    if (md.constTensorIndices[i] == 0)
                    {
                        dataIsConst = 1;
                    }
                }

                if (dataIsConst)
                {
                    int32_t *perm  = (int32_t*)layer->weights.ptr;
                    int32_t totalSize = layer->weights.bufSize;
                    for (int i = 0; i < totalSize; ++i) {
                        if (perm[i] != 0.0f) {
                            return false;
                        }
                    }
                    return true;                    
                }
                else
                {
                    TIDL_GLOBAL_REPORT_WARNING("Input data is expected to be a zero tensor; otherwise, ScatterND or ScatterElements layers may not function as expected.\n");
                }
            }
            return true;
        }
    ),
};