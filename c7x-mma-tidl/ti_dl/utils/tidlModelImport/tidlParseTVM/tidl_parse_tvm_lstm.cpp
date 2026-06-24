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

/**
 * @file tidl_parse_tvm_lstm.cpp
 * @brief TVM LSTM (Long Short-Term Memory) layer parser implementation for TIDL
 *
 * This file implements comprehensive parsing logic for TVM LSTM operators,
 * including parameter validation, tensor handling, and error management.
 *
 * TVM LSTM Specification Support:
 * - Input tensors: X, W, R, B (optional), sequence_lens (optional), initial_h (optional), initial_c (optional)
 * - Attributes: activation_alpha, activation_beta, activations, clip, direction,
 *               hidden_size, input_forget, layout
 * - Direction support: forward, reverse, bidirectional
 * - Layout support: 0 (seq_length, batch_size, input_size), 1 (batch_size, seq_length, input_size)
 * - Activation support: sigmoid, tanh, relu
 */

#include "tidl_parse_tvm.h"
using namespace std;

#include <tvm/relay/expr.h>
#include <tvm/relay/attrs/nn.h>
#include <tvm/relay/function.h>

// activation map to convert strings to TIDL activation constants.
static std::map<std::string, int32_t> activationMap = {
    {"Relu",            TIDL_RelU},
    {"Tanh",            TIDL_Tanh},
    {"Sigmoid",         TIDL_Sigmoid},
    {"LeakyRelu",       TIDL_LeakyReLU},
    {"HardSigmoid",     TIDL_HardSigmoid},
    {"Elu",             TIDL_ELU}
};

// Helper function to get TIDL activation name for logging
const char* getTidlActivationName(int32_t activation) {
    switch (activation) {
        case TIDL_Sigmoid: return "Sigmoid";
        case TIDL_Tanh: return "Tanh";
        case TIDL_RelU: return "ReLU";
        default: return "Unknown";
    }
}

// Extracts the integer value of a TVM PrimExpr dimension.
// Returns false if the dimension is symbolic (non-IntImm), true otherwise.
static bool getConstDim(const PrimExpr& expr, int64_t& val) {
    const IntImmNode* node = expr.as<IntImmNode>();
    if (!node) return false;
    val = node->value;
    return true;
}

static std::map<int32_t, std::pair<float32_tidl, float32_tidl>> activationAlphaBetaMap = {
    {TIDL_LeakyReLU,       {0.01f, 0.0f}},
    {TIDL_HardSigmoid,     {0.2f, 0.5f}},
    {TIDL_ELU,             {1.0f, 0.0f}}
};

/**
 * @brief Parse TVM LSTM layer and populate TIDL layer parameters
 *
 * This template specialization handles the complete parsing of TVM LSTM operators,
 * including comprehensive validation of inputs, attributes, and tensor dimensions.
 *
 * @return TIDL_IMPORT_DIAGNOSIS_RETURN_OK on success,
 *         TIDL_ALLOWLISTING_LAYER_CHECK_FAILED on failure
 */
template<> int32_t TidlParseTVM::parse<OpNameStr("tidl.lstm")> ()
{
    int32_t status = TIDL_IMPORT_DIAGNOSIS_RETURN_OK;

    // Validate that call is valid
    if (!call.defined()) {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "LSTM layer: Invalid call object");
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }

    // Extract composite function and parameters
    std::string op_name;
    const auto* fn_node = call->op.as<tvm::relay::FunctionNode>();
    if (!fn_node) {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "LSTM layer: Unable to access function node");
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }

    auto fn = GetRef<tvm::relay::Function>(fn_node);
    const auto* param = fn->attrs.as<DictAttrsNode>();
    if (!param) {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "LSTM layer: Unable to access function attributes");
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }

    if (param->dict.count("Composite"))
    {
        op_name = Downcast<String>(param->dict.at("Composite"));
    }

    std::string span_name = "";
    if(call->span.defined() && call->span->source_name.defined()){
        span_name = call->span->source_name->name;
    }
    // Get references to layer parameters
    sTIDL_LSTMParams_t &lstmParams = layer.layerParams.lstmParams;
    sTIDL_LSTMPCParams_t &lstmPCParams = layer.layerPCParams.lstmParams;
    sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;
    layer.layerType = TIDL_LSTMLayer;
    layer.numInBufs = md.numVarInputs;
    /* Parse Attributes */
    lstmParams.isClipSet = 0;
    lstmParams.direction = TIDL_RecurrentForward;
    lstmParams.input_forget = 0;
    lstmParams.layout = 0;

    try {

        // Validate minimum required inputs (X, W, R)
        if (call->args.size() < 3) {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "AllowListing : LSTM layer: Requires at least 3 inputs (X, W, R), but only has %d for node %s",
                           (int)call->args.size(), span_name);
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }

        // ========================================================================
        // STEP 2: Parse and validate attributes
        // ========================================================================

        // Parse activation_alpha attribute (array of floats)
        bool activationAlphaFound = false;
        if (param->dict.count("activation_alpha")) {
            Array<FloatImm> alphaArray = Downcast<Array<FloatImm>>(param->dict.at("activation_alpha"));
            int32_t numActivations = (int32_t)alphaArray.size();
            if (numActivations > TIDL_LSTM_MAX_ACTIVATIONS) {
                TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : LSTM layer: Upto %d activation_alpha values are supported, provided %d for node %s",
                               TIDL_LSTM_MAX_ACTIVATIONS, numActivations, span_name);
                return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
            }
            for (int32_t idx = 0; idx < numActivations; idx++) {
                lstmParams.activation_alpha[idx] = (float32_tidl)alphaArray[idx]->value;
            }
            activationAlphaFound = true;
        }

        // Parse activation_beta attribute (array of floats)
        bool activationBetaFound = false;
        if (param->dict.count("activation_beta")) {
            Array<FloatImm> betaArray = Downcast<Array<FloatImm>>(param->dict.at("activation_beta"));
            int32_t numActivations = (int32_t)betaArray.size();
            if (numActivations > TIDL_LSTM_MAX_ACTIVATIONS) {
                TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : LSTM layer: Upto %d activation_beta values are supported, provided %d for node %s",
                               TIDL_LSTM_MAX_ACTIVATIONS, numActivations, span_name);
                return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
            }
            for (int32_t idx = 0; idx < numActivations; idx++) {
                lstmParams.activation_beta[idx] = (float32_tidl)betaArray[idx]->value;
            }
            activationBetaFound = true;
        }

        // Parse activations attribute (array of strings)
        int32_t availableActivations = 0;
        if (param->dict.count("activations")) {
            Array<String> activationsArray = Downcast<Array<String>>(param->dict.at("activations"));
            availableActivations = (int32_t)activationsArray.size();

            if (availableActivations != 3 && availableActivations != 6) {
                TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : LSTM layer: Only 3 or 6 (if bidirectional) activations are supported, provided %d for node %s",
                               availableActivations, span_name);
                return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
            }

            for (int32_t idx = 0; idx < availableActivations; idx++) {
                std::string activation = activationsArray[idx];
                if(activationMap.find(activation) == activationMap.end())
                {
                    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : LSTM layer : Activation %s is not supported for node %s", activation.c_str(), span_name);
                    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
                }
                lstmParams.activations[idx] = activationMap[activation];
            }
        }

        // Parse clip attribute
        if (param->dict.count("clip")) {
            float32_tidl clipValue = (float32_tidl)Downcast<FloatImm>(param->dict.at("clip"))->value;
            lstmParams.clip = clipValue;
            lstmParams.isClipSet = 1;
        }

        if (param->dict.count("direction")) {
            std::string directionStr;
            // Handle direction parameter with proper type checking
            if (auto str_imm = param->dict.at("direction").as<tvm::tir::StringImmNode>()) {
                directionStr = str_imm->value;
            }
            else {
                directionStr = Downcast<String>(param->dict.at("direction"));
            }

            if (directionStr == "forward")
            {
                lstmParams.direction = TIDL_RecurrentForward;
            }
            else if (directionStr == "reverse")
            {
                lstmParams.direction = TIDL_RecurrentReverse;
            }
            else if (directionStr == "bidirectional")
            {
                lstmParams.direction = TIDL_RecurrentBidirectional;
            } else
            {
                lstmParams.direction = TIDL_RecurrentUnsupported;
            }
        }

        int32_t num_directions = 1;
        if(lstmParams.direction == TIDL_RecurrentBidirectional)
        {
            num_directions = 2;
        }

        // Fill default activations if not provided based on num_directions
        if (availableActivations == 0) {
            availableActivations = 3 * num_directions;
            for (int32_t idx = 0; idx < num_directions; idx++) {
                lstmParams.activations[3 * idx]       = TIDL_Sigmoid; /* f */
                lstmParams.activations[(3 * idx) + 1] = TIDL_Tanh;    /* g */
                lstmParams.activations[(3 * idx) + 2] = TIDL_Tanh;    /* h */
            }
        }

        if (availableActivations != 3 * num_directions) {
            TIDL_LOG_ERROR(gDiags.gDiagList, "Allowlisting : LSTM layer: Invalid number of activations (%d) for num_directions=%d for node %s",
                           availableActivations, num_directions, span_name);
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }

        // Fill default alpha/beta values for activations not provided via attributes
        if (!activationAlphaFound || !activationBetaFound) {
            for(int32_t idx = 0; idx < availableActivations; idx++)
            {
                if(activationAlphaBetaMap.find(lstmParams.activations[idx]) != activationAlphaBetaMap.end())
                {
                    if(!activationAlphaFound)
                    {
                        lstmParams.activation_alpha[idx] = activationAlphaBetaMap[lstmParams.activations[idx]].first;
                    }
                    if(!activationBetaFound)
                    {
                        lstmParams.activation_beta[idx] = activationAlphaBetaMap[lstmParams.activations[idx]].second;
                    }
                }
                else
                {
                    /* For activations which are not in the map, set alpha and beta to 0 */
                    if(!activationAlphaFound)
                    {
                        lstmParams.activation_alpha[idx] = 0.0f;
                    }
                    if(!activationBetaFound)
                    {
                        lstmParams.activation_beta[idx] = 0.0f;
                    }
                }
            }
        }

        // Parse hidden_size attribute
        if (param->dict.count("hidden_size")) {
            lstmParams.hidden_size = Downcast<Integer>(param->dict.at("hidden_size"))->value;
        }
        else{
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "AllowListing : LSTM layer: Missing hidden_size attribute for node %s", span_name);
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }

        // Parse input_forget attribute
        if (param->dict.count("input_forget")) {
            lstmParams.input_forget = Downcast<Integer>(param->dict.at("input_forget"))->value;
        }

        // Parse layout attribute
        if (param->dict.count("layout")) {
            lstmParams.layout = Downcast<Integer>(param->dict.at("layout"))->value;
        }

        /* Parse Inputs */

        int constIdx = 0; // local varibale to extract constant data from correct index;
        // Extract weight tensor W (required): [num_directions, 4*hidden_size, input_size]
        layer.weights = TIDL_extractConstantTensorData(call, 1, md, constIdx);
        if (layer.weights.ptr == nullptr) {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "AllowListing : LSTM layer: Failed to copy weight tensor W at index 1 for node %s", span_name);
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
        else {
            /* Const layer will get added for this input */
            layer.numInBufs++;
            constIdx++;
        }

        // Extract recurrence weights tensor R (required): [num_directions, 4*hidden_size, hidden_size]
        lstmPCParams.recurrenceWeights = TIDL_extractConstantTensorData(call, 2, md, constIdx);
        if (lstmPCParams.recurrenceWeights.ptr == nullptr) {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "AllowListing : LSTM layer: Failed to copy recurrence weight tensor R at index 2 for node %s", span_name);
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
        else {
            /* Const layer will get added for this input */
            layer.numInBufs++;
            constIdx++;
        }

        // Extract bias tensor B (optional)
        if (call->args.size() > 3 && call->args[3].defined()) {
            layer.bias = TIDL_extractConstantTensorData(call, 3, md, constIdx);
            if (layer.bias.ptr != nullptr) {
                /* Const layer will get added for this input */
                layer.numInBufs++;
                constIdx++;
            }
        }

        // Extract sequence lengths (optional) — stored as int32* to match constraint expectation
        if (call->args.size() > 4 && call->args[4].defined() &&
            call->args[4].as<tvm::relay::ConstantNode>()) {
            auto tensorExpr = call->args[4].as<tvm::relay::ConstantNode>()->data;
            int32_t numElements = 1;
            for (auto d : md.constTensorsDims[constIdx]) numElements *= d;
            int32_t* data = (int32_t*)my_malloc(numElements * sizeof(int32_t));
            lstmPCParams.sequence_lens.ptr  = data;
            tensorExpr.CopyToBytes(data, numElements * sizeof(int32_t));
            lstmPCParams.sequence_lens.bufSize = numElements;
            constIdx++;
        }

        // sequence_lens (arg index 4) is consumed at import time only and has no runtime
        // buffer. TIDL_relayImportNode pre-populates inDataNames[4]="const_X_4" for every
        // ConstantNode arg before the parser runs. Leaving that slot intact causes
        // tidl_addInDataLayer to create a spurious TIDL_DataLayer with uninitialised
        // dimValues, which then fails tidlInputTensorDimCheck.
        //
        // Fix: compact inDataNames[]/inData[] by shifting slots 5..N down by one so the
        // canonical relay LSTM slot layout is restored:
        //   W=1, R=2, B=3, initial_h=4, initial_c=5, peepholes=6
        // This is relay-specific; the ONNX path never pre-populates slots this way.
        // Only apply when sequence_lens is actually a constant node that was pre-populated.
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

        // Extract initial hidden state (optional):
        if (call->args.size() > 5 && call->args[5].defined()) {
            lstmPCParams.initial_h = TIDL_extractConstantTensorData(call, 5, md, constIdx);
            if (lstmPCParams.initial_h.ptr != nullptr) {
                /* Const layer will get added for this input */
                layer.numInBufs++;
                constIdx++;
            }
        }

        // Extract initial cell state (optional):
        if (call->args.size() > 6 && call->args[6].defined()) {
            lstmPCParams.initial_c = TIDL_extractConstantTensorData(call, 6, md, constIdx);
            if (lstmPCParams.initial_c.ptr != nullptr) {
                /* Const layer will get added for this input */
                layer.numInBufs++;
                constIdx++;
            }
        }

        // Extract peepholes tensor P (optional):
        if (call->args.size() > 7 && call->args[7].defined()) {
            lstmPCParams.peepholes = TIDL_extractConstantTensorData(call, 7, md, constIdx);
            if (lstmPCParams.peepholes.ptr != nullptr) {
                /* Const layer will get added for this input */
                layer.numInBufs++;
                constIdx++;
            }
        }

        if(constIdx != md.constTensorIndices.size())
        {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "AllowListing : LSTM layer: Number of constants extracted (%d) is not equal to expected number of constants (%d) for node %s", constIdx, md.constTensorIndices.size(), span_name);
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }

        for(int32_t varIdx: md.varTensorIndices)
        {
            if(varIdx == TIDL_RecurrentInputB)
            {
                lstmParams.isBiasPresent = 1;
            }
            else if(varIdx == TIDL_RecurrentInputInitialH)
            {
                lstmParams.isInitialHPresent = 1;
            }
            else if(varIdx == TIDL_RecurrentInputInitialC)
            {
                lstmParams.isInitialCPresent = 1;
            }
            else if(varIdx == TIDL_RecurrentInputPeepholes)
            {
                lstmParams.isPeepholesPresent = 1;
            }
        }
        for(int32_t constIdx: md.constTensorIndices)
        {
            if(constIdx == TIDL_RecurrentInputB)
            {
                lstmParams.isBiasPresent = 1;
            }
            else if(constIdx == TIDL_RecurrentInputInitialH)
            {
                lstmParams.isInitialHPresent = 1;
            }
            else if(constIdx == TIDL_RecurrentInputInitialC)
            {
                lstmParams.isInitialCPresent = 1;
            }
            else if(constIdx == TIDL_RecurrentInputPeepholes)
            {
                lstmParams.isPeepholesPresent = 1;
            }
        }

        /* Validate the input and weights dimensions */
        std::vector<int32_t> inputXDims = md.varTensorsDims[0];
        if (inputXDims.size() != 3)
        {
            TIDL_LOG_ERROR(gDiags.gDiagList, "AllowListing : LSTM layer: Input X must be a 3D tensor");
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }

        /* Extract dimensions based on layout */
        int32_t seq_length, batch_size, input_size;
        if (lstmParams.layout == 0)
        {
            /* input shape: [seq_length, batch_size, input_size] */
            seq_length = inputXDims[0];
            batch_size = inputXDims[1];
            input_size = inputXDims[2];
        }
        else
        {
            /* input shape: [batch_size, seq_length, input_size] */
            batch_size = inputXDims[0];
            seq_length = inputXDims[1];
            input_size = inputXDims[2];
        }

        // Validate weight tensor dimensions
        auto weight_expr = call->args[1];
        auto weight_type = weight_expr->checked_type().as<TensorTypeNode>();
        if (!weight_type) {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "AllowListing : LSTM layer: Unable to get weight tensor type");
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }

        Array<PrimExpr> weightShape = weight_type->shape;
        if (weightShape.size() != 3) {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "AllowListing : LSTM layer: Weight tensor W must be 3D [num_directions, 4*hidden_size, input_size]");
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }

        int64_t w0, w1, w2;
        if (!getConstDim(weightShape[0], w0) || !getConstDim(weightShape[1], w1) || !getConstDim(weightShape[2], w2)) {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "AllowListing : LSTM layer: Weight tensor W has symbolic dimensions for node %s", span_name);
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
        if (w0 != num_directions || w1 != 4 * lstmParams.hidden_size || w2 != input_size) {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "AllowListing : LSTM layer: Weight tensor W dimensions [%d, %d, %d] don't match expected [%d, %d, %d]",
                           (int)w0, (int)w1, (int)w2, num_directions, 4 * lstmParams.hidden_size, input_size);
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }

        // Validate recurrence weight tensor dimensions
        auto recur_weight_expr = call->args[2];
        auto recur_weight_type = recur_weight_expr->checked_type().as<TensorTypeNode>();
        if (!recur_weight_type) {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "AllowListing : LSTM layer: Unable to get recurrence weight tensor type");
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }

        Array<PrimExpr> recurWeightShape = recur_weight_type->shape;
        if (recurWeightShape.size() != 3) {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "AllowListing : LSTM layer: Recurrence weight tensor R must be 3D [num_directions, 4*hidden_size, hidden_size]");
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }

        int64_t rw0, rw1, rw2;
        if (!getConstDim(recurWeightShape[0], rw0) || !getConstDim(recurWeightShape[1], rw1) || !getConstDim(recurWeightShape[2], rw2)) {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "AllowListing : LSTM layer: Recurrence weight tensor R has symbolic dimensions for node %s", span_name);
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
        if (rw0 != num_directions || rw1 != 4 * lstmParams.hidden_size || rw2 != lstmParams.hidden_size) {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "AllowListing : LSTM layer: Recurrence weight tensor R dimensions [%d, %d, %d] don't match expected [%d, %d, %d]",
                           (int)rw0, (int)rw1, (int)rw2, num_directions, 4 * lstmParams.hidden_size, lstmParams.hidden_size);
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }

        // Validate bias tensor dimensions if present
        if (call->args.size() > 3 && call->args[3].defined() && lstmParams.isBiasPresent) {
            auto bias_expr = call->args[3];
            auto bias_type = bias_expr->checked_type().as<TensorTypeNode>();
            if (bias_type) {
                Array<PrimExpr> biasShape = bias_type->shape;
                if (biasShape.size() != 2) {
                    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "AllowListing : LSTM layer: Bias tensor B must be 2D [num_directions, 8*hidden_size] for node %s", span_name);
                    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
                }
                int64_t b0, b1;
                if (!getConstDim(biasShape[0], b0) || !getConstDim(biasShape[1], b1)) {
                    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "AllowListing : LSTM layer: Bias tensor B has symbolic dimensions for node %s", span_name);
                    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
                }
                if (b0 != num_directions || b1 != 8 * lstmParams.hidden_size) {
                    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "AllowListing : LSTM layer: Bias tensor B dimensions [%d, %d] don't match expected [%d, %d]",
                                   (int)b0, (int)b1, num_directions, 8 * lstmParams.hidden_size);
                    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
                }
            }
        }

        // Validate sequence_lens input if present
        if (call->args.size() > 4 && call->args[4].defined()) {
            auto seqlen_expr = call->args[4];
            auto seqlen_type = seqlen_expr->checked_type().as<TensorTypeNode>();
            if (seqlen_type) {
                Array<PrimExpr> seqlenShape = seqlen_type->shape;
                if (seqlenShape.size() != 1) {
                    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "AllowListing : LSTM layer: Sequence length tensor must be 1D [batch_size] for node %s", span_name);
                    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
                }
                int64_t sl0;
                if (!getConstDim(seqlenShape[0], sl0)) {
                    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "AllowListing : LSTM layer: Sequence length tensor has symbolic dimensions for node %s", span_name);
                    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
                }
                if (sl0 != batch_size) {
                    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "AllowListing : LSTM layer: Sequence length dimensions [%d] don't match expected [%d]",
                                   (int)sl0, batch_size);
                    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
                }
            }
        }

        // Validate initial_h input if present
        if (call->args.size() > 5 && call->args[5].defined()) {
            auto initial_h_expr = call->args[5];
            auto initial_h_type = initial_h_expr->checked_type().as<TensorTypeNode>();
            if (initial_h_type) {
                Array<PrimExpr> initialh_shape = initial_h_type->shape;
                if (initialh_shape.size() != 3) {
                    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "AllowListing : LSTM layer: initial_h tensor must be 3D [num_directions, batch_size, hidden_size] for node %s", span_name);
                    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
                }
                int64_t ih0, ih1, ih2;
                if (!getConstDim(initialh_shape[0], ih0) || !getConstDim(initialh_shape[1], ih1) || !getConstDim(initialh_shape[2], ih2)) {
                    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "AllowListing : LSTM layer: initial_h tensor has symbolic dimensions for node %s", span_name);
                    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
                }
                if (ih0 != num_directions || ih1 != batch_size || ih2 != lstmParams.hidden_size) {
                    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "AllowListing : LSTM layer: initial_h dimensions [%d, %d, %d] don't match expected [%d, %d, %d]",
                               (int)ih0, (int)ih1, (int)ih2, num_directions, batch_size, lstmParams.hidden_size);
                    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
                }
            }
        }

        // Validate initial_c input if present
        if (call->args.size() > 6 && call->args[6].defined()) {
            auto initial_c_expr = call->args[6];
            auto initial_c_type = initial_c_expr->checked_type().as<TensorTypeNode>();
            if (initial_c_type) {
                Array<PrimExpr> initialc_shape = initial_c_type->shape;
                if (initialc_shape.size() != 3) {
                    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "AllowListing : LSTM layer: initial_c tensor must be 3D [num_directions, batch_size, hidden_size] for node %s", span_name);
                    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
                }
                int64_t ic0, ic1, ic2;
                if (!getConstDim(initialc_shape[0], ic0) || !getConstDim(initialc_shape[1], ic1) || !getConstDim(initialc_shape[2], ic2)) {
                    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "AllowListing : LSTM layer: initial_c tensor has symbolic dimensions for node %s", span_name);
                    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
                }
                if (ic0 != num_directions || ic1 != batch_size || ic2 != lstmParams.hidden_size) {
                    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "AllowListing : LSTM layer: initial_c dimensions [%d, %d, %d] don't match expected [%d, %d, %d]",
                               (int)ic0, (int)ic1, (int)ic2, num_directions, batch_size, lstmParams.hidden_size);
                    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
                }
            }
        }

        // Validate P input if present
        if (call->args.size() > 7 && call->args[7].defined()) {
            auto peephole_expr = call->args[7];
            auto peephole_type = peephole_expr->checked_type().as<TensorTypeNode>();
            if (peephole_type) {
                Array<PrimExpr> peephole_shape = peephole_type->shape;
                if (peephole_shape.size() != 2) {
                    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "AllowListing : LSTM layer: Peephole tensor P must be 2D [num_directions, 3*hidden_size] for node %s", span_name);
                    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
                }
                int64_t p0, p1;
                if (!getConstDim(peephole_shape[0], p0) || !getConstDim(peephole_shape[1], p1)) {
                    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "AllowListing : LSTM layer: Peephole tensor P has symbolic dimensions for node %s", span_name);
                    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
                }
                if (p0 != num_directions || p1 != 3 * lstmParams.hidden_size) {
                    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "AllowListing : LSTM layer: peephole dimensions [%d, %d] don't match expected [%d, %d]",
                               (int)p0, (int)p1, num_directions, 3 * lstmParams.hidden_size);
                    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
                }
            }
        }

    } catch (const std::exception& e) {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "AllowListing : LSTM layer: Exception occurred while parsing: %s", e.what());
        if (lstmPCParams.sequence_lens.ptr) { my_free(lstmPCParams.sequence_lens.ptr); lstmPCParams.sequence_lens.ptr = nullptr; }
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    } catch (...) {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "AllowListing : LSTM layer: Unknown exception occurred while parsing");
        if (lstmPCParams.sequence_lens.ptr) { my_free(lstmPCParams.sequence_lens.ptr); lstmPCParams.sequence_lens.ptr = nullptr; }
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }

    return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}
