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



#include "tidl_parse_onnx.h"
using namespace std;
using namespace onnx;

template<> int32_t TidlParseOnnx:: parse<OnnxStr("DequantizeLinear")> ()
{
  /*Dequantize nodes are not directly supported - they need to either be optimized out (with stats collected)
    or routed to DataConvert layers which implement it's functionality*/
  int32_t status = 0;
  layer.layerType = TIDL_DequantizeLayer;

  NodeProto node  = graph.node(index);
  layer.layerPCParams.dequantParams.axis = 1;
  if(getAttrIdx(node, "axis") != -1)
  {
    getIntAttr(node, "axis", &layer.layerPCParams.dequantParams.axis, 0);
  }
  /*Map Dequantize nodes corresponding to activations: */
  TensorProto tensorScale       = getInitializerTensor(graph, graph.node(index).input(1) , index, status);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Dequantize : Unable to find initializer at index %d for node %s", 1, graph.node(index).name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  int32_t zeroPointStatus = 0;
  TensorProto tensorZeroPoint;
  if(graph.node(index).input_size() > 2)
  {
    tensorZeroPoint = getInitializerTensor(graph, graph.node(index).input(2) , index, zeroPointStatus);
  }
  /*Update scale and zero point:*/
  float tZeroPoint = 0.0;
  if(zeroPointStatus != 0)
  {
    tZeroPoint = 0.0;
    layer.layerPCParams.quantizeParams.elementType = TIDL_SignedChar;
  }
  else if(TensorProto_DataType_INT8 == tensorZeroPoint.data_type())
  {
    bool tensorZPFromRaw = tensorZeroPoint.int32_data_size() == 0;
    int8_t tZP;
    if(tensorZPFromRaw)
    {
      tZP = ((int8_t* )(tensorZeroPoint.raw_data().c_str()))[0];
    }
    else
    {
      tZP = (int8_t )(tensorZeroPoint.int32_data(0));
    }
    tZeroPoint = (float)tZP;
    layer.layerPCParams.dequantParams.elementType = TIDL_SignedChar;
  }
  else
  {
    bool tensorZPFromRaw = tensorZeroPoint.int32_data_size() == 0;
    uint8_t tZP;
    if(tensorZPFromRaw)
    {
      tZP = ((uint8_t* )(tensorZeroPoint.raw_data().c_str()))[0];
    }
    else
    {
      tZP = (uint8_t )(tensorZeroPoint.int32_data(0));
    }
    tZeroPoint = (float)tZP;
    layer.layerPCParams.dequantParams.elementType = TIDL_UnsignedChar;
  }
  float  tScale;
  bool tensorScaleFromRaw = tensorScale.float_data_size() == 0;
  if(tensorScaleFromRaw)
  {
    tScale = ((float* )(tensorScale.raw_data().c_str()))[0];
  }
  else
  {
    tScale = (float )(tensorScale.float_data(0));
  }
  layer.layerPCParams.dequantParams.scale = tScale;
  layer.layerPCParams.dequantParams.zeroPoint = tZeroPoint;

  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;
  layer.numInBufs = md.numVarInputs;

  return 0;
}

