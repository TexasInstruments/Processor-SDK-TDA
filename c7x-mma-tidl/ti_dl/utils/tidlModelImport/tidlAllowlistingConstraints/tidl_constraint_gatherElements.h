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

const vector<TidlConstraint> tidlConstraintGatherElements =
{
    TIDL_CSTR(
        "Number of non-singleton variable input dimensions must be <= 6",
        "Number of non-singleton variable input dimensions must be <= 6",
        "Number of non-singleton variable input dimensions must be <= 6",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            int32_t numDims = tidlGetNonSingletonNumDims(layer->allowlistingMetaData.varTensorsDims[0]);
            if(numDims > 6)
            {
                oss << "Maximum number of input dimension supported is 6, found " << numDims << " input dimensions";
                logs = oss.str();
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Number of output dimensions must be <= 6",
        "Number of output dimensions must be <= 6",
        "Number of output dimensions must be <= 6",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            int32_t numDims = layer->allowlistingMetaData.outputTensorDims[0].size();
            if(numDims > 6)
            {
                oss << "Maximum number of output dimension supported is 6, found " << numDims << " output dimensions";
                logs = oss.str();
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Input 'data' and 'indices' must have equal ranks, and all indices shape values must be within the data shape bounds",
        "Input 'data' and 'indices' must have equal ranks, and all indices shape values must be within the data shape bounds",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            int8_t dataIsVar = 0, indicesIsVar = 0;
            vector<int32_t> dataDims;
            vector<int32_t> indicesDims;
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            int32_t axis = layer->layerParams.gatherElementsParams.axis;
            for (int32_t i = 0; i < md.varTensorIndices.size(); i++)
            {
                if (md.varTensorIndices[i] == 0) dataIsVar = 1;
                if (md.varTensorIndices[i] == 1) indicesIsVar = 1;
            }

            if(dataIsVar && indicesIsVar)
            {
                dataDims = md.varTensorsDims[0];
                indicesDims = md.varTensorsDims[1];
            }
            else if (dataIsVar && !indicesIsVar)
            {
                dataDims = md.varTensorsDims[0];
                indicesDims = md.constTensorsDims[0];
            }
            else if (!dataIsVar && indicesIsVar)
            {
                dataDims = md.constTensorsDims[0];
                indicesDims = md.varTensorsDims[0];
            }

            if(dataDims.size() != indicesDims.size())
            {
                oss << "GatherElements: Rank of input 'data' needs to be equal to rank of input 'indices, found data shape as" << dataDims.size() << " and Indices shape as" << indicesDims.size();
                logs = oss.str();
                return false;
            }

            for (int32_t i=0; i<indicesDims.size(); i++)
            {
                if(i != axis)
                {
                    if(indicesDims[i] < 0 || indicesDims[i] > dataDims[i])
                    {
                        oss << "GatherElements: 'indices' shape should have values within bounds of 'data' shape.";
                    logs = oss.str();
                    return false;
                    }
                }
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Data and indices batch dimension should be same.",
        "Data and indices batch dimension should be same.",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){

            if(layer->optimized)
            {
                if(layer->inData[0].dimValues[TIDL_DIM_BATCH] != layer->inData[1].dimValues[TIDL_DIM_BATCH])
                {
                    return false;
                }
            }
            else
            {
                int8_t dataIsVar = 0, indicesIsVar = 0;
                vector<int32_t> dataDims;
                vector<int32_t> indicesDims;
                sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;

                for (int32_t i = 0; i < (int32_t)md.varTensorIndices.size(); i++)
                {
                    if (md.varTensorIndices[i] == 0) dataIsVar = 1;
                    if (md.varTensorIndices[i] == 1) indicesIsVar = 1;
                }

                if(dataIsVar && indicesIsVar)
                {
                    dataDims   = md.varTensorsDims[0];
                    indicesDims = md.varTensorsDims[1];
                }
                else if(dataIsVar && !indicesIsVar)
                {
                    dataDims   = md.varTensorsDims[0];
                    indicesDims = md.constTensorsDims[0];
                }
                else if(!dataIsVar && indicesIsVar)
                {
                    dataDims   = md.constTensorsDims[0];
                    indicesDims = md.varTensorsDims[0];
                }

                if((int32_t)dataDims.size() >= TIDL_DIM_MAX &&
                   (int32_t)indicesDims.size() >= TIDL_DIM_MAX
                   && dataDims[TIDL_DIM_BATCH] > 0 &&
                   indicesDims[TIDL_DIM_BATCH] > 0)
                {
                    if(dataDims[TIDL_DIM_BATCH] != indicesDims[TIDL_DIM_BATCH])
                    {
                        return false;
                    }
                }
            }

            return true;
        }
    )
};