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

template<> int32_t TidlParseOnnx::parse<OnnxStr("Conv")> ()
{
  int32_t status;
  int32_t padType;
  int32_t idx1, idx2;
  int32_t pads[4] = {0};
  int32_t padL=-1, padR=-1, padT=-1, padB=-1;

  sTIDL_ConvParams_t &convParams      = layer.layerParams.convParams;
  sTIDL_allowlistingMetaData md  = layer.allowlistingMetaData;
  layer.layerType = TIDL_ConvolutionLayer;


  layer.strideOffsetMethod = TIDL_StrideOffsetTopLeft;

  layer.outData[0].elementType = tidl_getElementType(1);
  TensorProto weight_tensor;

  weight_tensor = getInitializerTensor(graph, graph.node(index).input(1), index, status);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Conv layer : Unable to find initializer at index %d for node %s", 1, graph.node(index).name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  convParams.numOutChannels = getTensorDim(weight_tensor, 0);
  convParams.numInChannels  = getTensorDim(weight_tensor, 1);
  convParams.kernelH   = getTensorDim(weight_tensor, 2);
  convParams.kernelW    = getTensorDim(weight_tensor, 3);

  convParams.numGroups = 1;
  convParams.dilationW = 1;
  convParams.dilationH = 1;
  convParams.strideW = 1;
  convParams.strideH = 1;
  convParams.padW = 0;
  convParams.padH = 0;
  convParams.enableBias = 0;
  convParams.enablePooling = 0;

  NodeProto node = graph.node(index);
  getIntAttr(node, "strides",   &convParams.strideH, 0);
  getIntAttr(node, "strides",   &convParams.strideW, 1);
  getIntAttr(node, "dilations", &convParams.dilationH, 0);
  getIntAttr(node, "dilations", &convParams.dilationW, 1);
  getIntAttr(node, "group",     &convParams.numGroups, 0);

  int32_t attrIdx = getAttrIdx(node, "kernel_shape");
  int32_t attrSize = 0;
  if(attrIdx != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    attrSize = node.attribute(attrIdx).ints_size();
    if(attrSize == 2)
    {
      status = getIntAttr(node, "kernel_shape", &layer.layerPCParams.convParams.tensorHeight, 0);
      if (status == -1)
      {
        layer.layerPCParams.convParams.tensorHeight = convParams.kernelH;
      }
      status = getIntAttr(node, "kernel_shape", &layer.layerPCParams.convParams.tensorWidth, 1);
      if (status == -1)
      {
        layer.layerPCParams.convParams.tensorWidth = convParams.kernelW;
      }
    }
    else if (attrSize < 2)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Conv : kernel_shape is missing height axis.");
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    else
    {
      int32_t temp;
      for(int32_t i = 2; i < attrSize; i++)
      {
        status = getIntAttr(node, "kernel_shape", &temp, i);
        if (status != -1 && temp != 1)
        {
          TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Conv : kernel_shape is only supported along width and height axis.");
          return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
      }
    }
  }
  else
  {
    layer.layerPCParams.convParams.tensorHeight = convParams.kernelH;
    layer.layerPCParams.convParams.tensorWidth = convParams.kernelW;
  }


  char autoPad[50];
  status = getStringAttr(node, "auto_pad", autoPad, 0);
  if((status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL) || (strcmp(autoPad, "NOTSET") == 0))
  {
    getIntAttr(node, "pads",       &pads[0], 0);
    getIntAttr(node, "pads",       &pads[1], 1);
    getIntAttr(node, "pads",       &pads[2], 2);
    getIntAttr(node, "pads",       &pads[3], 3);
    if((pads[0] < pads[2]) && (pads[1] < pads[3])) //padL < padR && padT < padB
    {
      layer.strideOffsetMethod = TIDL_StrideOffsetCenter;
    }
    padL = pads[1];
    padR = pads[3];
    padT = pads[0];
    padB = pads[2];
    pads[2] = max(pads[0], pads[2]);
    pads[3] = max(pads[1], pads[3]);
  }
  else if ((strcmp(autoPad, "SAME_UPPER") == 0) || (strcmp(autoPad, "SAME_LOWER") == 0))
  {
    int32_t shapeInferenceStatus = checkShapeInferenceforOnnx(md);
    if((shapeInferenceStatus == TIDL_IMPORT_DIAGNOSIS_RETURN_OK) && (md.varTensorsDims[0].size() >= 2))
    {
      int32_t heightIdx = md.varTensorsDims[0].size() - 2;
      int32_t widthIdx = md.varTensorsDims[0].size() - 1;
      // This is similar to how TVM implements padding
      int32_t inputH = md.varTensorsDims[0][heightIdx];
      int32_t inputW = md.varTensorsDims[0][widthIdx];

      int32_t totalPadH = max(inputH % convParams.strideH ? convParams.dilationH * (convParams.kernelH - 1) + 1 - inputH % convParams.strideH : convParams.dilationH * (convParams.kernelH - 1) + 1 - convParams.strideH, 0);
      int32_t totalPadW = max(inputW % convParams.strideW ? convParams.dilationW * (convParams.kernelW - 1) + 1 - inputW % convParams.strideW : convParams.dilationW * (convParams.kernelW - 1) + 1 - convParams.strideW, 0);
      
      if(strcmp(autoPad, "SAME_UPPER") == 0)
      {
        padL = totalPadW / 2;
        padT = totalPadH / 2;
      }
      else  //SAME_LOWER
      {
        padL = (totalPadW + 1) / 2;
        padT = (totalPadH + 1) / 2;
      }

      padB = max(totalPadH - padT, 0);
      padR = max(totalPadW - padL, 0);

      if ((padL < padR) && (padT < padB))
      {
        layer.strideOffsetMethod = TIDL_StrideOffsetCenter;
      }
    }
    else
    {
      /* TCs pertaining to this condition are not existent as of now, hence this cannot be validated */
      padL = ((convParams.kernelW - 1)*convParams.dilationW) / 2;
      padT = ((convParams.kernelH - 1)*convParams.dilationH) / 2;
      padR = padL;
      padB = padT;
      if(strcmp(autoPad, "SAME_UPPER") == 0)
      {
        layer.strideOffsetMethod = TIDL_StrideOffsetCenter;
      }
      else  //SAME_LOWER
      {
        layer.strideOffsetMethod = TIDL_StrideOffsetTopLeft;
      }
    }
    pads[3] = max(padR, padL);
    pads[2] = max(padB, padT);
  }
  else if(strcmp(autoPad, "VALID") == 0)
  {
    pads[3] = 0;
    pads[2] = 0;
    padL = 0;
    padR = 0;
    padT = 0;
    padB = 0;
  }


  convParams.padW = pads[3];
  convParams.padH = pads[2];

  convParams.padT = padT;
  convParams.padL = padL;
  convParams.padB = padB;
  convParams.padR = padR;

  status = copyFloatConst(graph, index, 1, layer.weights, INPUT_REQUIRED);
  if(status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Conv layer : Unable to copy initializer at index %d for node %s", 1, graph.node(index).name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  if (graph.node(index).input_size() > 2)
  {
    convParams.enableBias = 1;
    status = copyFloatConst(graph, index, 2, layer.bias, INPUT_REQUIRED);
    if(status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Conv layer : Unable to copy initializer at index %d for node %s", 2, graph.node(index).name().c_str());
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
  }

  return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}

