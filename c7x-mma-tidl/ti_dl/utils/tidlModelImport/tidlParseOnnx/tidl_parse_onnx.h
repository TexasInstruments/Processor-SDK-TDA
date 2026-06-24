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

#ifndef TIDL_PARSE_ONNX_H_
#define TIDL_PARSE_ONNX_H_

#include "onnx/onnx-ml.proto3.pb.h"
#include "ti_dl.h"
#include "tidl_import_config.h"
#include "tidl_import_common.h"
#include "tidl_import_api.h"
#include "tidl_import_diag.h"
#include "tidl_import_common_model_check.h"
#include "tidl_common_utils_infer_import.h"

using namespace std;
using namespace onnx;

extern int64_t onnxOpSetVersion;;
extern tidl_import_config gParams;
extern "C" int32_t TIDL_isInputConstInGraph(GraphProto &onnGraph, const string name);


#define S(T,I)  (I < sizeof(T)) ? T[I] : 0
#define OnnxStr(T) \
    S(T, 0),S(T, 1),S(T, 2),S(T, 3),S(T, 4),S(T, 5),S(T, 6),S(T, 7),S(T, 8),S(T, 9),\
    S(T,10),S(T,11),S(T,12),S(T,13),S(T,14),S(T,15),S(T,16),S(T,17),S(T,18),S(T,19),\
    S(T,20),S(T,21),S(T,22),S(T,23),S(T,24),S(T,25),S(T,26),S(T,27),S(T,28),S(T,29),\
    S(T,30),S(T,31),S(T,32),S(T,33),S(T,34),S(T,35),S(T,36),S(T,37),S(T,38),S(T,39),\
    S(T,40),S(T,41),S(T,42),S(T,43),S(T,44),S(T,45),S(T,46),S(T,47),S(T,48),S(T,49)

#define INPUT_NOT_REQUIRED (0)
#define INPUT_REQUIRED (1)

const std::vector<int32_t> unsupportedDataTypes = {TIDL_UnsignedDoubleWord, TIDL_SignedDoubleWord, TIDL_SignedWord, TIDL_UnsignedWord};
const std::vector<int32_t> supportedLayerTypes = {
    TIDL_GatherLayer,
    TIDL_GatherNDLayer, 
    TIDL_GatherElementsLayer, 
    TIDL_ArgOpLayer, 
    TIDL_ScatterElementsLayer, 
    TIDL_SliceLayer,
    TIDL_FlattenLayer, 
    TIDL_CastLayer, 
    TIDL_ReshapeLayer, 
    TIDL_SqueezeLayer, 
    TIDL_UnsqueezeLayer, 
    TIDL_TransposeLayer, 
    TIDL_SliceLayer, 
    TIDL_ConcatLayer, 
    TIDL_SplitLayer, 
    TIDL_TopKLayer,
    TIDL_ShapeLayer,
    TIDL_SizeLayer,
    TIDL_TileLayer,
    TIDL_LogicalOpLayer,
    TIDL_NonZeroLayer,
    TIDL_EltWiseLayer
};

static std::map<std::string, int32_t> activationMap = {
    {"Relu",            TIDL_RelU},
    {"Tanh",            TIDL_Tanh},
    {"Sigmoid",         TIDL_Sigmoid},
    {"LeakyRelu",       TIDL_LeakyReLU},
    {"HardSigmoid",     TIDL_HardSigmoid},
    {"Elu",             TIDL_ELU}
};

static std::map<int32_t, std::pair<float32_tidl, float32_tidl>> activationAlphaBetaMap = {
    {TIDL_LeakyReLU,       {0.01f, 0.0f}},
    {TIDL_HardSigmoid,     {0.2f, 0.5f}},
    {TIDL_ELU,             {1.0f, 0.0f}}
};

/// @brief Parses Onnx layers
struct TidlParseOnnx
{
    sTIDL_LayerPC_t layer;

    GraphProto& graph;
    int32_t index;

    int32_t status;
    bool isModelShapeInfered;

    /// @brief Constructor of TidlParseOnnx
    TidlParseOnnx(GraphProto& graph, int32_t index, sTIDL_LayerPC_t layer, bool& isModelShapeInfered);

    /// @brief The parse function: fills in attributes into sTIDL_Layer_PC_t struct
    template <char... S>
    int32_t parse();


    // Utility Functions
    int32_t saveAllowlistingMetaData();
    int32_t numVariableLayerInputs(GraphProto& onnGraph, int32_t i, std::vector<int32_t>& varIdx);
    TensorProto getInitializerTensor(GraphProto& onnGraph, const string name, int32_t nIdx, int32_t& status);
    bool doesQuantOrDequantHaveConstTensor(GraphProto& onnGraph, const string name);
    int32_t getAttrIdx(const NodeProto& node, char * name);
    int32_t getNodeIdx(GraphProto& onnxGraph, const char *bufName);
    vector<vector<int32_t>> getNodeInputOutputTypes(GraphProto& onnxGraph, int32_t idx);
    vector<int32_t> getNodeInputShape(GraphProto& onnxGraph, const string name, int32_t debugLevel);
    vector<int32_t> getNodeOutputShape(GraphProto& onnxGraph, const string name, int32_t debugLevel);
    int32_t getIntAttr(const NodeProto& node, char * name, int32_t * valuePtr, int32_t idx);
    int32_t getStringAttr(const NodeProto& node, char * name, char * valuePtr, int32_t idx);
    int32_t getTensorDim(TensorProto& tensor, int32_t idx);
    int32_t copyTensor(GraphProto& onnGraph, TensorProto& tensor, void ** ptr, int32_t * size, const string name, int32_t inputRequired);
    bool isTensorDQ(GraphProto& onnGraph, TensorProto& tensor);
    int32_t getNodeIdxFromInput(GraphProto& onnxGraph, const char *bufName);
    int32_t copyFloatConst(GraphProto& onnGraph, int32_t nIdx, int32_t inIdx, sBuffer_t &buf, int32_t inputRequired);
    int32_t getFloatAttr(const NodeProto& node, char * name, float * valuePtr, int32_t idx);
    void broadcastFloatTensor(sBuffer_t &buf, int32_t bufSize);
    int32_t getConstSize (TensorProto& tensor);
    int32_t containsSuffix(std::string str, std::string suffix);
    int32_t getTIDLDataTypeFromOnnxDataType (int32_t onnxDataType);
    std::string getOriginalNameFromDuplicateName(std::string duplicateName);
    int32_t getInputDataType(GraphProto& onnxGraph, int32_t nodeIdx, int32_t inputIdx);
    int32_t convertIndicesToInt32(sBuffer_t& weights);
    bool isLayerIONumSupported(sTIDL_LayerPC_t &layer);
    bool isLayerIODataTypesSupported(sTIDL_LayerPC_t &layer);
};





#endif /*TIDL_PARSE_ONNX_H_ */