/*
 *
 * Copyright (c) {2025} Texas Instruments Incorporated
 *
 * All rights not granted herein.
 *
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 * license under copyrights and patents it now or hereafter owns or controls to make,
 * have made, use, import, offer to sell and sell ("Utilize") this software subject to
 * the terms herein.  With respect to the foregoing patent license, such license is granted
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
 *        *       No reverse engineering, decompilation, or disassembly of this software is
 *              permitted with respect to any software provided in binary form.
 *
 *        *       any redistribution and use are licensed by TI for use only with TI Devices.
 *
 *        *       Nothing shall obligate TI to provide you with source code for the software
 *              licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the
 * source code are permitted provided that the following conditions are met:
 *
 *        *       any redistribution and use of the source code, including any resulting derivative
 *              works, are licensed by TI for use only with TI Devices.
 *
 *        *       any redistribution and use of any object code compiled from the source code
 *              and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 *        *       Neither the name of Texas Instruments Incorporated nor the names of its suppliers
 *              may be used to endorse or promote products derived from this software without
 *              specific prior written permission.
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
#include <tvm/relay/function.h>
#include <tvm/relay/expr.h>
using namespace std;
using namespace tvm;
using namespace tvm::relay;

#define ATTN_MASK_IDX (3)

template<>
int32_t TidlParseTVM::parse<OpNameStr("tidl.attention")>()
{
    int32_t status = 0;
    layer.layerType = TIDL_AttentionLayer;
    layer.numInBufs = 3; // Q, K, V

    sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;

    const auto* fn_node = call->op.as<tvm::relay::FunctionNode>();
    auto fn = GetRef<tvm::relay::Function>(fn_node);
    const auto* attrs = fn->attrs.as<DictAttrsNode>();

    int32_t numVarDims = md.varTensorsDims[0].size();

    // Defaults from tensor shapes (like ONNX parser)
    int32_t kv_num_heads = md.varTensorsDims[1][1];
    int32_t q_num_heads  = md.varTensorsDims[0][1];
    float32_tidl scale   = (1.0f / (float)q_num_heads);
    int32_t is_causal    = 0;
    int32_t qk_matmul_output_mode = 0;
    float32_tidl softcap = 0.0f;
    int32_t softmax_precision = -1;

    // kv_num_heads
    if (attrs->dict.count("kv_num_heads")) {
        kv_num_heads = Downcast<Integer>(attrs->dict.at("kv_num_heads"))->value;
    } else if (numVarDims == 3) {
        TIDL_GLOBAL_REPORT_ERROR("kv_num_heads attribute is not found for Attention layer");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }

    // q_num_heads
    if (attrs->dict.count("q_num_heads")) {
        q_num_heads = Downcast<Integer>(attrs->dict.at("q_num_heads"))->value;
    } else if (numVarDims == 3) {
        TIDL_GLOBAL_REPORT_ERROR("q_num_heads attribute is not found for Attention layer");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }

    // Recompute scale if q_num_heads changed
    scale = (1.0f / (float)q_num_heads);

    // is_causal
    if (attrs->dict.count("is_causal")) {
        is_causal = Downcast<Integer>(attrs->dict.at("is_causal"))->value;
    }

    // qk_matmul_output_mode
    if (attrs->dict.count("qk_matmul_output_mode")) {
        qk_matmul_output_mode = Downcast<Integer>(attrs->dict.at("qk_matmul_output_mode"))->value;
    }

    // softcap
    if (attrs->dict.count("softcap")) {
        softcap = Downcast<FloatImm>(attrs->dict.at("softcap"))->value;
    }

    // scale (override)
    if (attrs->dict.count("scale")) {
        scale = Downcast<FloatImm>(attrs->dict.at("scale"))->value;
    }

    // softmax_precision
    if (attrs->dict.count("softmax_precision")) {
        softmax_precision = Downcast<Integer>(attrs->dict.at("softmax_precision"))->value;
        if (softmax_precision < -1 || softmax_precision > 32) {
            TIDL_GLOBAL_REPORT_ERROR("Softmax precision attribute value is out of range for Attention layer, it should be between -1 and 32");
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
    } else {
        TIDL_GLOBAL_REPORT_WARNING("Softmax precision attribute is not found for Attention layer, using default value of -1 which indicates default precision for softmax");
    }

    // Assign to layerParams
    layer.layerParams.attentionParams.isCausal = is_causal;
    layer.layerParams.attentionParams.qk_matmul_output_mode = qk_matmul_output_mode;
    layer.layerParams.attentionParams.softcap = softcap;
    layer.layerParams.attentionParams.scale = scale;
    layer.layerParams.attentionParams.softmax_precision = softmax_precision;
    layer.layerParams.attentionParams.kv_num_heads = kv_num_heads;
    layer.layerParams.attentionParams.q_num_heads = q_num_heads;

    // Optional attn_mask (4th input) -> layer.bias
    if (md.numConstInputs + md.numVarInputs > 3) {
        for(int32_t i = 0 ; i < md.constTensorIndices.size() ; i++){
            if(md.constTensorIndices[i]==ATTN_MASK_IDX){
                layer.bias = TIDL_extractConstantTensorData(call, ATTN_MASK_IDX, md, i);
                if (layer.bias.ptr == NULL || layer.bias.bufSize == 0) {
                    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot extract attn_mask tensor for Attention");
                    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
                }
                break;
            }
        }
    }

    return 0;
}