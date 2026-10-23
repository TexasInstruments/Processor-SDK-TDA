/*
*
* Copyright (c) {2025} Texas Instruments Incorporated
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


/* Activation name → TIDL activation constant.
 * Shared by LSTM, GRU, and RNN parsers. */
const std::map<std::string, int32_t> TidlParseTVM::recurrentActivationMap = {
    {"Relu",        TIDL_RelU},
    {"Tanh",        TIDL_Tanh},
    {"Sigmoid",     TIDL_Sigmoid},
    {"LeakyRelu",   TIDL_LeakyReLU},
    {"HardSigmoid", TIDL_HardSigmoid},
    {"Elu",         TIDL_ELU}
};

/* Default alpha/beta values for activations that need them.
 * Shared by LSTM, GRU, and RNN parsers. */
const std::map<int32_t, std::pair<float32_tidl, float32_tidl>> TidlParseTVM::recurrentActivationAlphaBetaMap = {
    {TIDL_LeakyReLU,   {0.01f, 0.0f}},
    {TIDL_HardSigmoid, {0.2f,  0.5f}},
    {TIDL_ELU,         {1.0f,  0.0f}}
};

/* Extract an integer value from a TVM PrimExpr dimension.
 * Returns false if the dimension is symbolic (non-IntImm).
 * Shared by LSTM, GRU, and RNN parsers. */
bool TidlParseTVM::getConstDim(const PrimExpr& expr, int64_t& val)
{
    const IntImmNode* node = expr.as<IntImmNode>();
    if (!node) return false;
    val = node->value;
    return true;
}


/* Maps TVM (dtype.code, dtype.bits) pairs to TIDL element types.
 * Mirrors the ONNX typesTable in tidl_parse_onnx_utils.cpp — absent entries
 * return -1 (unsupported), which saveAllowlistingMetaData uses to reject the node.
 * All callers must pass both code and bits so the lookup is unambiguous.
 */
static const map<pair<int,int>, int32_t> TVMtoTIDLTypesTable = {
    {{0,  8},  TIDL_SignedChar},          // kDLInt,   8-bit → int8
    {{0, 16},  TIDL_SignedShort},         // kDLInt,  16-bit → int16
    {{0, 32},  TIDL_SignedWord},          // kDLInt,  32-bit → int32
    {{0, 64},  TIDL_SignedDoubleWord},    // kDLInt,  64-bit → int64
    {{1,  1},  TIDL_Bool},               // kDLUInt,  1-bit → bool
    {{1,  8},  TIDL_UnsignedChar},       // kDLUInt,  8-bit → uint8
    {{1, 16},  TIDL_UnsignedShort},      // kDLUInt, 16-bit → uint16
    {{1, 32},  TIDL_UnsignedWord},       // kDLUInt, 32-bit → uint32
    {{1, 64},  TIDL_UnsignedDoubleWord}, // kDLUInt, 64-bit → uint64
    {{2, 32},  TIDL_SinglePrecFloat},    // kDLFloat, 32-bit → float32
    // float16 {2,16} and float64 {2,64} intentionally absent → -1
};

/* Single source of truth: maps a TVM/DLPack (dtype.code, dtype.bits) pair to a
 * TIDL eTIDL_ElementType value, or -1 if the type is unsupported. This is a free
 * function so both the TidlParseTVM parsers and the free functions in
 * tidl_relayImport.cpp (which are not class members) can share one lookup. */
int32_t TIDL_getTidlTypeFromTVMDataType(int32_t code, int32_t bits)
{
    auto it = TVMtoTIDLTypesTable.find({code, bits});
    return (it != TVMtoTIDLTypesTable.end()) ? it->second : -1;
}

int32_t TidlParseTVM::getTIDLDataTypeFromTVMDataType(int32_t tvmDataType,int32_t bits)
{
    int32_t tidlType = TIDL_getTidlTypeFromTVMDataType(tvmDataType, bits);
    return (tidlType != -1) ? tidlType : TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
}

bool TIDL_relayIsInt64Type(const Expr& expr)
{
    auto tensorType = expr->checked_type().as<TensorTypeNode>();
    if (tensorType == nullptr)
    {
        return false;
    }
    return TIDL_getTidlTypeFromTVMDataType((int32_t)tensorType->dtype.code(),
                                           (int32_t)tensorType->dtype.bits()) == TIDL_SignedDoubleWord;
}
// Helper function to convert int64 indices to int32
int32_t TidlParseTVM::convertIndicesToInt32(sBuffer_t& weights)
{
    int32_t *index_dst_ptr = (int32_t*)my_malloc(sizeof(int32_t) * weights.bufSize);
    if (index_dst_ptr == NULL)
    {
        TIDL_LOG_ERROR(gDiags.gDiagList, "ScatterNDLayer : Could not allocate memory for ScatterND constant indices");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }

    int64_t *index_src_ptr = (int64_t*)weights.ptr;
    int64_t val = 0;
    for (int32_t j = 0; j < weights.bufSize; j++)
    {
        val = index_src_ptr[j];
        index_dst_ptr[j] = (int32_t)val;
    }
    my_free(index_src_ptr);
    weights.ptr = (void*)index_dst_ptr;
    weights.dataType = TIDL_SignedWord;

    return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}

/**
 * @brief Check if layer's input/output buffer counts are within supported limits
 *
 * This function validates that the number of input and output tensors for a layer
 * doesn't exceed TIDL's maximum supported buffer counts (TIDL_NUM_IN_BUFS and
 * TIDL_NUM_OUT_BUFS). This check is performed in the parser before allowlisting
 * constraints are evaluated, following the same pattern as ONNX RT flow.
 *
 * @param layer The layer structure containing allowlisting metadata
 * @return true if buffer counts are supported, false otherwise
 */
bool TidlParseTVM::isLayerIONumSupported(sTIDL_LayerPC_t &layer)
{
  bool isLayerSupported = true;
  sTIDL_allowlistingMetaData& md = layer.allowlistingMetaData;

  if (md.numInputs > TIDL_NUM_IN_BUFS)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Layer %s : Unsupported number of input tensors (> %d)", (char *)layer.name, TIDL_NUM_IN_BUFS);
    isLayerSupported = false;
  }
  else if (md.numOutputs > TIDL_NUM_OUT_BUFS)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Layer %s : Unsupported number of output tensors (> %d)", (char *)layer.name, TIDL_NUM_OUT_BUFS);
    isLayerSupported = false;
  }

  return isLayerSupported;
}

int32_t TidlParseTVM::parseLogicalBinaryOp(int32_t opType, const char* errMsg,
                                            int32_t weightIdx, int32_t biasIdx)
{
    layer.layerType = TIDL_LogicalOpLayer;
    layer.layerParams.logicalOpLayerParams.operatorType = opType;

    sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;

    layer.numInBufs       = md.numVarInputs;
    layer.weights.ptr     = NULL;
    layer.weights.bufSize = 0;
    layer.bias.ptr        = NULL;
    layer.bias.bufSize    = 0;

    int32_t constIdx = 0;
    for (int32_t i = 0; i < (int32_t)md.constTensorIndices.size(); i++)
    {
        int32_t constTensorIdx = md.constTensorIndices[i];

        if (constTensorIdx != weightIdx && constTensorIdx != biasIdx)
            continue;

        if (constTensorIdx == weightIdx)
        {
            layer.weights = TIDL_extractConstantTensorData(call, constTensorIdx, md, constIdx++);
            if (layer.weights.ptr == nullptr)
            {
                TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "%s", errMsg);
                return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
            }
            layer.numInBufs++;
        }
        else if (constTensorIdx == biasIdx)
        {
            layer.bias = TIDL_extractConstantTensorData(call, constTensorIdx, md, constIdx++);
            if (layer.bias.ptr == nullptr)
            {
                TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "%s", errMsg);
                return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
            }
            layer.numInBufs++;
        }
    }

    return 0;
}

bool TidlParseTVM::isLayerIODataTypesSupported(sTIDL_LayerPC_t &layer)
{
    sTIDL_allowlistingMetaData& md = layer.allowlistingMetaData;
    std::vector<int32_t>& varTensorIndices = md.varTensorIndices;
    std::string failReason = "";

    // Early-reject if any input or output has an unknown type (-1).
    // These are TVM types with no TIDL mapping — nothing downstream can handle them.
    for (int32_t i = 0; i < md.numVarInputs; i++){
        if ((md.inputDataTypes.size() > (size_t)i) && md.inputDataTypes[varTensorIndices[i]] == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED){
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Layer %s : Unknown input data type at variable tensor index: %d", (char *)layer.name, varTensorIndices[i]);
            return false;
        }
    }
    for (int32_t i = 0; i < md.numOutputs; i++){
        if ((md.outputDataTypes.size() > (size_t)i) && md.outputDataTypes[i] == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED){
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Layer %s : Unknown output data type at variable tensor index: %d", (char *)layer.name, i);
            return false;
        }
    }

    // Detect whether any input or output carries a type from the unsupported list (int32/int64/uint32/uint64). 
    // If none do, the layer passes.
    bool hasUnsupportedType = false;
    for (int32_t i = 0; i < md.numVarInputs; i++){
        if ((md.inputDataTypes.size() > (size_t)i) && (find(unsupportedDataTypes.begin(), unsupportedDataTypes.end(), md.inputDataTypes[varTensorIndices[i]]) != unsupportedDataTypes.end())){
            hasUnsupportedType = true; 
            break;
        }
    }
    for (int32_t i = 0; i < md.numOutputs; i++){
        if ((md.outputDataTypes.size() > (size_t)i) && (find(unsupportedDataTypes.begin(), unsupportedDataTypes.end(), md.outputDataTypes[i]) != unsupportedDataTypes.end())){
            hasUnsupportedType = true; 
            break;
        }
    }

    if (!hasUnsupportedType) return true;

    // Per-layer allowlist. The switch is the allowlist — a case means the layer type is permitted to have unsupported dtypes (subject to any per-case constraints); 
    // default rejects. Set failReason on failure; log once at the end.
    // To add a new layer: add a case. If no extra checks needed, place it in the
    // no-op group above default.
    switch (layer.layerType)
    {
    case TIDL_GatherLayer:
    case TIDL_GatherNDLayer:
    case TIDL_GatherElementsLayer:
        // int64/uint64 on the data tensor (index 0) is not supported
        if ((varTensorIndices[0] == 0) &&
            (md.inputDataTypes.size() > (size_t)varTensorIndices[0]) &&
            (md.inputDataTypes[varTensorIndices[0]] == TIDL_SignedDoubleWord || md.inputDataTypes[varTensorIndices[0]] == TIDL_UnsignedDoubleWord)){
            failReason = "Unsupported int64/uint64 data type for data input tensor (index " + std::to_string(varTensorIndices[0]) + ")";
        }
        break;

    case TIDL_ArgOpLayer:
        if ((md.inputDataTypes.size() > (size_t)varTensorIndices[0]) &&
            (find(unsupportedDataTypes.begin(), unsupportedDataTypes.end(), md.inputDataTypes[varTensorIndices[0]]) != unsupportedDataTypes.end())){
            failReason = "Unsupported data type for variable input tensor (index " + std::to_string(varTensorIndices[0]) + ")";
        }
        break;

    case TIDL_TopKLayer:
        // output[0] (values) and input[0] cannot be unsupported; output[1] (indices, int64) is allowed
        if ((md.inputDataTypes.size() > (size_t)varTensorIndices[0]) &&
            (find(unsupportedDataTypes.begin(), unsupportedDataTypes.end(), md.inputDataTypes[varTensorIndices[0]]) != unsupportedDataTypes.end())){
            failReason = "Unsupported data type for input tensor (index " + std::to_string(varTensorIndices[0]) + ")";
        }
        else if ((md.outputDataTypes.size() > 0) &&
                (find(unsupportedDataTypes.begin(), unsupportedDataTypes.end(), md.outputDataTypes[0]) != unsupportedDataTypes.end())){
           failReason = "Unsupported data type for values output tensor (index 0)";
        }
        break;

    case TIDL_EltWiseLayer:
        if (layer.layerParams.eltWiseParams.eltWiseType == TIDL_EltWiseMod){
            // Mod allows int32 and int64 only
            for (int32_t i = 0; i < (int32_t)md.inputDataTypes.size(); i++){
                if (md.inputDataTypes[i] != TIDL_SignedDoubleWord && md.inputDataTypes[i] != TIDL_SignedWord){
                    failReason = "Unsupported data type for Mod input tensor (index " + std::to_string(i) + ", only int32/int64 allowed)";
                    break;
                }
            }
        }else{
            failReason = "Unsupported data type for EltWise layer (only Mod supports int32/int64)";
        }
        break;

    // Layer types that need no additional constraint — allowed to have unsupported dtypes
    case TIDL_ScatterElementsLayer:
    case TIDL_SliceLayer:
    case TIDL_FlattenLayer:
    case TIDL_CastLayer:
    case TIDL_ReshapeLayer:
    case TIDL_SqueezeLayer:
    case TIDL_UnsqueezeLayer:
    case TIDL_TransposeLayer:
    case TIDL_ConcatLayer:
    case TIDL_SplitLayer:
    case TIDL_ShapeLayer:
    case TIDL_SizeLayer:
    case TIDL_TileLayer:
    case TIDL_LogicalOpLayer:
    case TIDL_NonZeroLayer:
        break;

    default:
        failReason = "Unsupported data type for one of the input/output tensors";
        break;
    }

    if (!failReason.empty()){
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Layer %s : %s", (char *)layer.name, failReason.c_str());
        return false;
    }

    return true;
}

/* Returns the number of dimensions of the tensor at relay input position
 * tensorIndex, whether that tensor is a variable or a constant input. Falls
 * back to defaultNumDim if the tensor isn't found among the var/const inputs,
 * or if its recorded dims are empty. Centralizes the previously copy-pasted
 * var/const-index scan used by Slice and Unsqueeze to derive numDim from the
 * data tensor (input 0). */
int32_t TIDL_getNumDimsForTensorIndex(const sTIDL_allowlistingMetaData &md, int32_t tensorIndex, int32_t defaultNumDim)
{
    int32_t numDim = defaultNumDim;

    for (int32_t k = 0; k < (int32_t)md.varTensorIndices.size(); k++)
    {
        if (md.varTensorIndices[k] == tensorIndex)
        {
            if (md.varTensorsDims[k].size() != 0)
                numDim = md.varTensorsDims[k].size();
            break;
        }
    }

    for (int32_t k = 0; k < (int32_t)md.constTensorIndices.size(); k++)
    {
        if (md.constTensorIndices[k] == tensorIndex)
        {
            if (md.constTensorsDims[k].size() != 0)
                numDim = md.constTensorsDims[k].size();
            break;
        }
    }

    return numDim;
}