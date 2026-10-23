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

#include "tidl_parse_tvm.h"
#include <tvm/relay/function.h>
#include <tvm/relay/expr.h>
using namespace std;


template<> int32_t TidlParseTVM::parse<OpNameStr("tidl.gru")> ()
{
    int32_t status = TIDL_IMPORT_DIAGNOSIS_RETURN_OK;

    const auto* fn_node = call->op.as<tvm::relay::FunctionNode>();
    if (!fn_node) {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "GRU layer: Unable to access function node");
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }
    auto fn = GetRef<tvm::relay::Function>(fn_node);
    const auto* param = fn->attrs.as<DictAttrsNode>();
    if (!param) {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "GRU layer: Unable to access function attributes");
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }

    std::string span_name = "";
    if (call->span.defined() && call->span->source_name.defined())
        span_name = call->span->source_name->name;

    sTIDL_GRUParams_t   &gruParams   = layer.layerParams.gruParams;
    sTIDL_GRUPCParams_t &gruPCParams = layer.layerPCParams.gruParams;
    sTIDL_allowlistingMetaData md    = layer.allowlistingMetaData;
    layer.layerType  = TIDL_GRULayer;
    layer.numInBufs  = md.numVarInputs;

    gruParams.isClipSet          = 0;
    gruParams.direction          = TIDL_RecurrentForward;
    gruParams.layout             = 0;
    gruParams.linear_before_reset = 0;

    try {
        if (call->args.size() < 3) {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : GRU layer: Requires at least 3 inputs (X, W, R), found %d for node %s",
                                 (int)call->args.size(), span_name.c_str());
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }

        /* activation_alpha */
        bool activationAlphaFound = false;
        if (param->dict.count("activation_alpha")) {
            Array<FloatImm> alphaArray = Downcast<Array<FloatImm>>(param->dict.at("activation_alpha"));
            int32_t numActivations = (int32_t)alphaArray.size();
            if (numActivations > (int32_t)TIDL_GRU_MAX_ACTIVATIONS) {
                TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : GRU layer : Upto %d activation_alpha values are supported, provided %d for node %s",
                                     TIDL_GRU_MAX_ACTIVATIONS, numActivations, span_name.c_str());
                return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
            }
            for (int32_t idx = 0; idx < numActivations; idx++)
                gruParams.activation_alpha[idx] = (float32_tidl)alphaArray[idx]->value;
            activationAlphaFound = true;
        }

        /* activation_beta */
        bool activationBetaFound = false;
        if (param->dict.count("activation_beta")) {
            Array<FloatImm> betaArray = Downcast<Array<FloatImm>>(param->dict.at("activation_beta"));
            int32_t numActivations = (int32_t)betaArray.size();
            if (numActivations > (int32_t)TIDL_GRU_MAX_ACTIVATIONS) {
                TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : GRU layer : Upto %d activation_beta values are supported, provided %d for node %s",
                                     TIDL_GRU_MAX_ACTIVATIONS, numActivations, span_name.c_str());
                return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
            }
            for (int32_t idx = 0; idx < numActivations; idx++)
                gruParams.activation_beta[idx] = (float32_tidl)betaArray[idx]->value;
            activationBetaFound = true;
        }

        /* activations */
        int32_t availableActivations = 0;
        if (param->dict.count("activations")) {
            Array<String> activationsArray = Downcast<Array<String>>(param->dict.at("activations"));
            availableActivations = (int32_t)activationsArray.size();
            if (availableActivations != 2 && availableActivations != 4) {
                TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : GRU layer : Only 2 or 4 (if bidirectional) activations are supported, provided %d for node %s",
                                     availableActivations, span_name.c_str());
                return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
            }
            for (int32_t idx = 0; idx < availableActivations; idx++) {
                std::string activation = activationsArray[idx];
                if (recurrentActivationMap.find(activation) == recurrentActivationMap.end()) {
                    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : GRU layer : Activation %s is not supported for node %s",
                                         activation.c_str(), span_name.c_str());
                    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
                }
                gruParams.activations[idx] = recurrentActivationMap.at(activation);
            }
        }

        /* clip */
        if (param->dict.count("clip")) {
            gruParams.clip     = (float32_tidl)Downcast<FloatImm>(param->dict.at("clip"))->value;
            gruParams.isClipSet = 1;
        }

        /* direction */
        if (param->dict.count("direction")) {
            std::string directionStr;
            if (auto str_imm = param->dict.at("direction").as<tvm::tir::StringImmNode>())
                directionStr = str_imm->value;
            else
                directionStr = Downcast<String>(param->dict.at("direction"));

            if      (directionStr == "forward")       gruParams.direction = TIDL_RecurrentForward;
            else if (directionStr == "reverse")       gruParams.direction = TIDL_RecurrentReverse;
            else if (directionStr == "bidirectional") gruParams.direction = TIDL_RecurrentBidirectional;
            else                                      gruParams.direction = TIDL_RecurrentUnsupported;
        }

        int32_t num_directions = (gruParams.direction == TIDL_RecurrentBidirectional) ? 2 : 1;

        /* default activations */
        if (availableActivations == 0) {
            availableActivations = 2 * num_directions;
            for (int32_t idx = 0; idx < num_directions; idx++) {
                gruParams.activations[2 * idx]       = TIDL_Sigmoid; /* f */
                gruParams.activations[(2 * idx) + 1] = TIDL_Tanh;   /* g */
            }
        }

        if (availableActivations != 2 * num_directions) {
            TIDL_LOG_ERROR(gDiags.gDiagList, "Allowlisting : GRU layer: Invalid number of activations (%d) for num_directions=%d for node %s",
                           availableActivations, num_directions, span_name.c_str());
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }

        /* default alpha/beta */
        if (!activationAlphaFound || !activationBetaFound) {
            for (int32_t idx = 0; idx < availableActivations; idx++) {
                auto it = recurrentActivationAlphaBetaMap.find(gruParams.activations[idx]);
                if (it != recurrentActivationAlphaBetaMap.end()) {
                    if (!activationAlphaFound) gruParams.activation_alpha[idx] = it->second.first;
                    if (!activationBetaFound)  gruParams.activation_beta[idx]  = it->second.second;
                } else {
                    if (!activationAlphaFound) gruParams.activation_alpha[idx] = 0.0f;
                    if (!activationBetaFound)  gruParams.activation_beta[idx]  = 0.0f;
                }
            }
        }

        /* hidden_size */
        if (param->dict.count("hidden_size")) {
            gruParams.hidden_size = (int32_t)Downcast<Integer>(param->dict.at("hidden_size"))->value;
        } else {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : GRU layer: Missing hidden_size attribute for node %s", span_name.c_str());
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }

        /* layout */
        if (param->dict.count("layout"))
            gruParams.layout = (int32_t)Downcast<Integer>(param->dict.at("layout"))->value;

        /* linear_before_reset */
        if (param->dict.count("linear_before_reset"))
            gruParams.linear_before_reset = (int32_t)Downcast<Integer>(param->dict.at("linear_before_reset"))->value;

        /* ---- Inputs ---- */
        int32_t constIdx = 0;

        /* W (required) */
        layer.weights = TIDL_extractConstantTensorData(call, 1, md, constIdx);
        if (layer.weights.ptr == nullptr) {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : GRU layer: Failed to copy weight tensor W at index 1 for node %s", span_name.c_str());
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
        layer.numInBufs++;
        constIdx++;

        /* R (required) */
        gruPCParams.recurrenceWeights = TIDL_extractConstantTensorData(call, 2, md, constIdx);
        if (gruPCParams.recurrenceWeights.ptr == nullptr) {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : GRU layer: Failed to copy recurrence weight tensor R at index 2 for node %s", span_name.c_str());
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
        layer.numInBufs++;
        constIdx++;

        /* B (optional) */
        if (call->args.size() > 3 && call->args[3].defined()) {
            layer.bias = TIDL_extractConstantTensorData(call, 3, md, constIdx);
            if (layer.bias.ptr != nullptr) {
                layer.numInBufs++;
                constIdx++;
            }
        }

        /* sequence_lens (optional) — consumed at import time only */
        if (call->args.size() > 4 && call->args[4].defined() &&
            call->args[4].as<tvm::relay::ConstantNode>()) {
            auto tensorExpr = call->args[4].as<tvm::relay::ConstantNode>()->data;
            int32_t numElements = 1;
            for (auto d : md.constTensorsDims[constIdx]) numElements *= d;
            int32_t* data = (int32_t*)my_malloc(numElements * sizeof(int32_t));
            gruPCParams.sequence_lens.ptr     = data;
            tensorExpr.CopyToBytes(data, numElements * sizeof(int32_t));
            gruPCParams.sequence_lens.bufSize = numElements;
            constIdx++;
        }

        /* sequence_lens slot compaction: shift inDataNames[5..N] down by one */
        if (call->args.size() > 4 && call->args[4].defined() &&
            call->args[4].as<tvm::relay::ConstantNode>())
        {
            int32_t numArgs = (int32_t)call->args.size();
            for (int32_t slot = TIDL_RecurrentInputSequenceLens; slot < numArgs - 1; slot++)
            {
                memcpy(layer.inDataNames[slot], layer.inDataNames[slot + 1], TIDL_STRING_SIZE);
                layer.inData[slot] = layer.inData[slot + 1];
            }
            int32_t lastSlot = numArgs - 1;
            memset(layer.inDataNames[lastSlot], 0, TIDL_STRING_SIZE);
            memset(&layer.inData[lastSlot],     0, sizeof(layer.inData[0]));
            layer.inData[lastSlot].dataId = -1;
        }

        /* initial_h (optional) */
        if (call->args.size() > 5 && call->args[5].defined()) {
            gruPCParams.initial_h = TIDL_extractConstantTensorData(call, 5, md, constIdx);
            if (gruPCParams.initial_h.ptr != nullptr) {
                layer.numInBufs++;
                constIdx++;
            }
        }

        /* isBiasPresent / isInitialHPresent flags */
        for (int32_t varIdx : md.varTensorIndices) {
            if      (varIdx == TIDL_RecurrentInputB)        gruParams.isBiasPresent    = 1;
            else if (varIdx == TIDL_RecurrentInputInitialH)  gruParams.isInitialHPresent = 1;
        }
        for (int32_t cIdx : md.constTensorIndices) {
            if      (cIdx == TIDL_RecurrentInputB)           gruParams.isBiasPresent    = 1;
            else if (cIdx == TIDL_RecurrentInputInitialH)    gruParams.isInitialHPresent = 1;
        }

        /* ---- Dimension validation ---- */
        std::vector<int32_t> inputXDims = md.varTensorsDims[0];
        if (inputXDims.size() != 3) {
            TIDL_LOG_ERROR(gDiags.gDiagList, "Allowlisting : GRU layer: Input X must be a 3D tensor for node %s", span_name.c_str());
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }

        int32_t seq_length, batch_size, input_size;
        (void)inputXDims[0];
        if (gruParams.layout == 0) {
            seq_length = inputXDims[0];
            batch_size = inputXDims[1];
            input_size = inputXDims[2];
        } else {
            batch_size = inputXDims[0];
            seq_length = inputXDims[1];
            input_size = inputXDims[2];
        }

        /* W: [num_directions, 3*hidden_size, input_size] */
        auto w_type = call->args[1]->checked_type().as<TensorTypeNode>();
        if (w_type) {
            Array<PrimExpr> wShape = w_type->shape;
            int64_t w0, w1, w2;
            if (wShape.size() == 3 && getConstDim(wShape[0], w0) && getConstDim(wShape[1], w1) && getConstDim(wShape[2], w2)) {
                if (w0 != num_directions || w1 != 3 * gruParams.hidden_size || w2 != input_size) {
                    TIDL_LOG_ERROR(gDiags.gDiagList, "Allowlisting : GRU layer: Weight tensor W shape [%d,%d,%d] doesn't match expected [%d,%d,%d] for node %s",
                                   (int)w0, (int)w1, (int)w2, num_directions, 3*gruParams.hidden_size, input_size, span_name.c_str());
                    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
                }
            }
        }

        /* R: [num_directions, 3*hidden_size, hidden_size] */
        auto r_type = call->args[2]->checked_type().as<TensorTypeNode>();
        if (r_type) {
            Array<PrimExpr> rShape = r_type->shape;
            int64_t r0, r1, r2;
            if (rShape.size() == 3 && getConstDim(rShape[0], r0) && getConstDim(rShape[1], r1) && getConstDim(rShape[2], r2)) {
                if (r0 != num_directions || r1 != 3 * gruParams.hidden_size || r2 != gruParams.hidden_size) {
                    TIDL_LOG_ERROR(gDiags.gDiagList, "Allowlisting : GRU layer: Recurrence weight tensor R shape [%d,%d,%d] doesn't match expected [%d,%d,%d] for node %s",
                                   (int)r0, (int)r1, (int)r2, num_directions, 3*gruParams.hidden_size, gruParams.hidden_size, span_name.c_str());
                    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
                }
            }
        }

        /* B: [num_directions, 6*hidden_size] */
        if (call->args.size() > 3 && call->args[3].defined() && gruParams.isBiasPresent) {
            auto b_type = call->args[3]->checked_type().as<TensorTypeNode>();
            if (b_type) {
                Array<PrimExpr> bShape = b_type->shape;
                int64_t b0, b1;
                if (bShape.size() == 2 && getConstDim(bShape[0], b0) && getConstDim(bShape[1], b1)) {
                    if (b0 != num_directions || b1 != 6 * gruParams.hidden_size) {
                        TIDL_LOG_ERROR(gDiags.gDiagList, "Allowlisting : GRU layer: Bias tensor B shape [%d,%d] doesn't match expected [%d,%d] for node %s",
                                       (int)b0, (int)b1, num_directions, 6*gruParams.hidden_size, span_name.c_str());
                        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
                    }
                }
            }
        }

        /* sequence_lens: [batch_size] */
        if (call->args.size() > 4 && call->args[4].defined()) {
            auto sl_type = call->args[4]->checked_type().as<TensorTypeNode>();
            if (sl_type) {
                Array<PrimExpr> slShape = sl_type->shape;
                int64_t sl0;
                if (slShape.size() == 1 && getConstDim(slShape[0], sl0) && sl0 != batch_size) {
                    TIDL_LOG_ERROR(gDiags.gDiagList, "Allowlisting : GRU layer: sequence_lens shape [%d] doesn't match expected [%d] for node %s",
                                   (int)sl0, batch_size, span_name.c_str());
                    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
                }
            }
        }

        /* initial_h: [num_directions, batch_size, hidden_size] */
        if (call->args.size() > 5 && call->args[5].defined() && gruParams.isInitialHPresent) {
            auto ih_type = call->args[5]->checked_type().as<TensorTypeNode>();
            if (ih_type) {
                Array<PrimExpr> ihShape = ih_type->shape;
                int64_t ih0, ih1, ih2;
                if (ihShape.size() == 3 && getConstDim(ihShape[0], ih0) && getConstDim(ihShape[1], ih1) && getConstDim(ihShape[2], ih2)) {
                    if (ih0 != num_directions || ih1 != batch_size || ih2 != gruParams.hidden_size) {
                        TIDL_LOG_ERROR(gDiags.gDiagList, "Allowlisting : GRU layer: initial_h shape [%d,%d,%d] doesn't match expected [%d,%d,%d] for node %s",
                                       (int)ih0, (int)ih1, (int)ih2, num_directions, batch_size, gruParams.hidden_size, span_name.c_str());
                        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
                    }
                }
            }
        }

    } catch (const std::exception& e) {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : GRU layer: Exception occurred while parsing: %s", e.what());
        if (gruPCParams.sequence_lens.ptr) { my_free(gruPCParams.sequence_lens.ptr); gruPCParams.sequence_lens.ptr = nullptr; }
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    } catch (...) {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : GRU layer: Unknown exception occurred while parsing");
        if (gruPCParams.sequence_lens.ptr) { my_free(gruPCParams.sequence_lens.ptr); gruPCParams.sequence_lens.ptr = nullptr; }
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }

    return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}
