/*
*
* Copyright (c) {2015 - 2017} Texas Instruments Incorporated
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

#ifndef TIDL_ONNX_IMPORT_H
#define TIDL_ONNX_IMPORT_H 1

#include "onnx/onnx-ml.proto3.pb.h"
using namespace std;
using namespace onnx;

int32_t TIDL_onnxMapConvBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapReluBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapPReluBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapLeakyReluBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapClipBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapSigmoidBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapTanhBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapHardSigmoidBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapELUBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapConcatBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapMaxPoolBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapGlobalAvgPoolBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapAvgPoolBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapUpsampleBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapResizeBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapConvTransposeBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapReshapeBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapTransposeBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapArgOpBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapBNBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxGemmBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapAddBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapSubBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapDivBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapSoftmaxBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapSoftplusBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapSoftsignBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapCeilBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapRoundBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapCeluBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapSeluBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapSplitBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapSliceBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapFlattenBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapPadBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapMulBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapDivBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapDepthToSpaceBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapDropOutBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapSqueezeBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapReduceMaxBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapReduceMinBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapScatterElementsBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapIdentityBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapScatterNDBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapGatherBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapDequantizeLinearBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapReduceMeanBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapReduceSumBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapPowBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapLayerNormBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapSqrtBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t TIDL_onnxMapQuantizeLinearBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
int32_t tidl_onnxLayerUpdateConsumerCount(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
                                 int32_t i, int32_t layerIndex, GraphProto& onnxGraph);
int32_t tidl_onnxLayerFillTensorNames(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
                                 int32_t i, int32_t layerIndex, GraphProto& onnxGraph);

std::vector<int32_t> getNodeInputShape(GraphProto& onnxGraph, const std::string name, int32_t debugLevel);
std::vector<int32_t> getNodeOutputShape(GraphProto& onnxGraph, const std::string name, int32_t debugLevel);
int32_t TIDL_onnxSaveAllowlistingMetaData(GraphProto& onnGraph, int32_t i, sTIDL_LayerPC_t &layer);
#endif  /* TIDL_ONNX_IMPORT_H */
