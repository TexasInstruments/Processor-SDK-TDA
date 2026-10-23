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

const vector<TidlConstraint> tidlConstraintRotaryEmbedding =
{
    TIDL_CSTR(
        "Not supported as an individual operator",
        "Not supported as an individual operator",
        "Not supported as an individual operator",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            return false;
        }
    ),
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
        "numHeads must be positive",
        "numHeads must be positive",
        "numHeads must be positive",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            int32_t numHeads = layer->layerParams.rotaryEmbeddingParams.numHeads;
            if (numHeads <= 0)
            {
                oss << "numHeads must be positive, found numHeads = " << numHeads;
                logs = oss.str();
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "numHeads must be present for 3 dimensional variable input",
        "numHeads must be present for 3 dimensional variable input",
        "numHeads must be present for 3 dimensional variable input",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            int32_t numHeads = layer->layerParams.rotaryEmbeddingParams.numHeads;
            int32_t numDims = tidlGetNonSingletonNumDims(layer->allowlistingMetaData.varTensorsDims[0]);
            if (numHeads == 0 && numDims == 3)
            {
                oss << "numHeads must be present for 3D variable input tensor";
                logs = oss.str();
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "numHeads must be divisible by hidden_size in 3 dimensional variable input",
        "numHeads must be divisible by hidden_size in 3 dimensional variable input",
        "numHeads must be divisible by hidden_size in 3 dimensional variable input",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            int32_t numDims = tidlGetNonSingletonNumDims(layer->allowlistingMetaData.varTensorsDims[0]);
            int32_t numHeads = layer->layerParams.rotaryEmbeddingParams.numHeads;
            if (numDims == 3 && layer->allowlistingMetaData.varTensorsDims[0][2] % numHeads != 0)
            {
                oss << "numHeads must be divisible by hidden_size, found numHeads = " << numHeads <<"hidden_size = "<<layer->allowlistingMetaData.varTensorsDims[0][2];
                logs = oss.str();
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "position_ids input, if present, must be a constant initializer",
        "position_ids input, if present, must be a constant initializer",
        "position_ids input, if present, must be a constant initializer",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            /* position_ids is input index 3. If it is present but was not resolved to a
             * constant during parsing, positionIds.bufSize stays 0 -- indistinguishable from
             * "position_ids absent", which silently makes the layer fall back to sequential
             * positions instead of the (dynamic) positions the model actually intends. */
            if (layer->allowlistingMetaData.numInputs > 3 &&
                layer->layerParams.rotaryEmbeddingParams.positionIds.bufSize == 0)
            {
                oss << "position_ids is provided as a runtime (non-constant) input, which is not "
                    << "supported -- TIDL only supports position_ids as a constant initializer";
                logs = oss.str();
                return false;
            }
            return true;
        }
    ),
};
