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

const vector<TidlConstraint> tidlConstraintAttention =
{
    TIDL_CSTR(
        "Number of non-singleton variable input dimensions must be <= 6",
        "Number of non-singleton variable input dimensions must be <= 6",
        "Number of non-singleton variable input dimensions must be <= 6",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            int32_t numQDims = tidlGetNonSingletonNumDims(layer->allowlistingMetaData.varTensorsDims[0]);
            int32_t numKDims = tidlGetNonSingletonNumDims(layer->allowlistingMetaData.varTensorsDims[1]);
            int32_t numVDims = tidlGetNonSingletonNumDims(layer->allowlistingMetaData.varTensorsDims[2]);
            if((numQDims != numKDims) || (numKDims != numVDims))
            {
                oss << "Number of non-singleton variable input dimensions must be <= 6, found " << numQDims << " for Query, " <<numKDims<< " for Key " <<numVDims<< " for Value";
                logs = oss.str();
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Key and Value input dimensions should be same",
        "Key and Value input dimensions should be same",
        "Key and Value input dimensions should be same",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            std::vector<int32_t> keyDimensions = layer->allowlistingMetaData.varTensorsDims[1];
            std::vector<int32_t> valueDimensions = layer->allowlistingMetaData.varTensorsDims[2];
            if(keyDimensions.size() != valueDimensions.size())
            {
                oss << "Number of dimensions for query and key does not match, key Dimension found : " << keyDimensions.size() << " while query dimension found to be : " << valueDimensions.size();
                logs = oss.str();
                return false;
            }
            for(int32_t i = 0; i < valueDimensions.size(); i++)
            {
                if(valueDimensions[i] != keyDimensions[i])
                {
                    oss << "Dimension of key and query didn't match at the input index" << i << " key dimension found " << keyDimensions[i] << " query dimension found " << valueDimensions[i];
                    logs = oss.str();
                    return false;
                }
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "past_key, past_value and nonpad_kv_seqlen is not supported",
        "past_key, past_value and nonpad_kv_seqlen is not supported",
        "past_key, past_value and nonpad_kv_seqlen is not supported",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            int32_t numVarInputs = layer->allowlistingMetaData.numVarInputs;
            int32_t numConstInputs = layer->allowlistingMetaData.numConstInputs;
            int32_t numInputs = (numVarInputs + numConstInputs);
            if(numInputs >= 4)
            {
                if((numConstInputs == 0 && numVarInputs > 3) || (numInputs > 4))
                {
                    oss << "Number of inputs should be 3 or 4 (if attn_mask is present), number of variable inputs found : " << numVarInputs << ", number of constant input found : " << numConstInputs;
                    logs = oss.str();
                    return false;
                }
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "kv_num_heads must be same as input channel dimension for key/value input",
        "kv_num_heads must be same as input channel dimension for key/value input",
        "kv_num_heads must be same as input channel dimension for key/value input",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            int32_t numDims = layer->allowlistingMetaData.varTensorsDims[1].size();
            int32_t channelDim = layer->allowlistingMetaData.varTensorsDims[1][numDims-3];
            if((numDims >= 4) && (channelDim != layer->layerParams.attentionParams.kv_num_heads))
            {
                oss << "kv_num_heads must be same as input channel dimension, found " << layer->layerParams.attentionParams.kv_num_heads << " kv_num_heads and " << channelDim << " input channel dimension";
                logs = oss.str();
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "q_num_heads must be same as input channel dimension for query input",
        "q_num_heads must be same as input channel dimension for query input",
        "q_num_heads must be same as input channel dimension for query input",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            int32_t numDims = layer->allowlistingMetaData.varTensorsDims[1].size();
            int32_t channelDim = layer->allowlistingMetaData.varTensorsDims[1][numDims-3];
            if((numDims >= 4) && (channelDim != layer->layerParams.attentionParams.q_num_heads))
            {
                oss << "q_num_heads must be same as input channel dimension, found " << layer->layerParams.attentionParams.q_num_heads << " q_num_heads and " << channelDim << " input channel dimension";
                logs = oss.str();
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Default value of softmax_precision is accepted, other values are not supported",
        "Default value of softmax_precision is accepted, other values are not supported",
        "Default value of softmax_precision is accepted, other values are not supported",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            int32_t softmax_precision = layer->layerParams.attentionParams.softmax_precision;
            if(softmax_precision != -1)
            {
                oss << "Default value of softmax_precision is accepted, other values are not supported, found" << layer->layerParams.attentionParams.softmax_precision;
                logs = oss.str();
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Causal attention is not supported",
        "Causal attention is not supported",
        "Causal attention is not supported",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            int32_t is_causal = layer->layerParams.attentionParams.isCausal;
            if(is_causal != 0)
            {
                oss << "Attention layer with is_causal value 0 is supported, found" << layer->layerParams.attentionParams.isCausal;
                logs = oss.str();
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "qk_matmul_output_mode is not supported",
        "qk_matmul_output_mode is not supported",
        "qk_matmul_output_mode is not supported",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            int32_t qk_matmul_output_mode = layer->layerParams.attentionParams.qk_matmul_output_mode;
            if(qk_matmul_output_mode != 0)
            {
                oss << "Attention layer with qk_matmul_output_mode value 0 is supported, found" << layer->layerParams.attentionParams.qk_matmul_output_mode;
                logs = oss.str();
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "",
        "",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            if(layer->optimized == 1)
            {
                oss << "Attention layer is not optimized, not supported as an individual operator";
                logs = oss.str();
                return false;
            }
            return true;
        }
    ),
};