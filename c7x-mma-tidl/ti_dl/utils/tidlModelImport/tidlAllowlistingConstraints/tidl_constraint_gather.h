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

const vector<TidlConstraint> tidlConstraintGather = {
    /**
     * Only line/vector gather is supported
     * Input data should be 2D (HxW or 1xHxW or 1x1xHxW etc.)
     * Indice data should be 1D (W or 1xW or 1x1xW etc)
     */
    TIDL_CSTR(
        "Input data dimensions must be greater than 1D",
        "Input data dimensions must be greater than 1D",
        "Input data dimensions must be greater than 1D",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            int32_t inDataDim = (md.numConstInputs > 0 && md.constTensorIndices[0] == 0)
                                ? (int32_t)md.constTensorsDims[0].size()
                                : (int32_t)md.varTensorsDims[0].size();

            if (inDataDim < 2)
            {
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
        "Input shape of dimension higher than axis should be 1",
        "Input shape of dimension higher than axis should be 1",
        "Input shape of dimension higher than axis should be 1",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            int32_t axis = layer->layerParams.gatherParams.axis;

            bool dataIsConst = (md.numConstInputs > 0 && md.constTensorIndices[0] == 0);
            vector<int32_t> dataDims    = dataIsConst ? md.constTensorsDims[0] : md.varTensorsDims[0];
            vector<int32_t> indicesDims;
            if (dataIsConst)
            {
                if (md.numVarInputs > 0) indicesDims = md.varTensorsDims[0];
            }
            else if (md.numConstInputs >= 1)
            {
                indicesDims = md.constTensorsDims[0];
            }

            int32_t offset = TIDL_DIM_MAX - (int32_t)dataDims.size();

            if(md.numConstInputs >= 1)
            {
                /*
                 * If axis dimension shape is 1 and indices have only one value
                 * i.e. 0 then this can be converted to reshape
                 */
                if((int32_t)dataDims.size() > (axis - offset) &&
                   dataDims[axis - offset] == 1 &&
                   ((indicesDims.size() == 0) || (indicesDims.size() == 1 && indicesDims[0] == 1)))
                {
                    return true;
                }

                /*
                 * If indices is const and contiguous, it can be converted to slice.
                 * Only valid when indices is the const input (weights.ptr holds indices).
                 */
                if (!dataIsConst && layer->weights.ptr != NULL && layer->weights.bufSize >= 1)
                {
                    int32_t *idxPtr = (int32_t*)layer->weights.ptr;

                    // Check if contiguous indices value
                    int8_t allowed = 1;
                    for (int32_t j = 1; j < layer->weights.bufSize; j++)
                    {
                        if (idxPtr[j] - idxPtr[j-1] != 1)
                        {
                            allowed = 0;
                            break;
                        }
                    }

                    if (allowed && axis >= TIDL_DIM_NUMCH)
                    {
                        return true;
                    }
                }
            }

            if(layer->optimized == 0 && axis != TIDL_DIM_WIDTH)
            {
                for(int32_t i = 0; (i < (axis - offset) && i < (int32_t)dataDims.size()); i++)
                {
                    if(dataDims[i] != 1)
                    {
                        return false;
                    }
                }
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Only 1D indices are supported",
        "Only 1D indices are supported",
        "Only 1D indices are supported",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            bool dataIsConst = (md.numConstInputs > 0 && md.constTensorIndices[0] == 0);
            vector<int32_t> indicesDims;
            if (md.numVarInputs > 1)
            {
                indicesDims = md.varTensorsDims[1];
            }
            else if (dataIsConst && md.numVarInputs >= 1)
            {
                // data is const, indices is var
                indicesDims = md.varTensorsDims[0];
            }
            else if (!dataIsConst && md.numConstInputs >= 1)
            {
                // data is var, indices is const
                indicesDims = md.constTensorsDims[0];
            }
            if (indicesDims.size() > 0)
            {
                for(int32_t i = 0; i < (int32_t)indicesDims.size() - 1; i++)
                {
                    if(indicesDims[i] != 1)
                    {
                        return false;
                    }
                }
            }
            return true;
        }
    ),
};