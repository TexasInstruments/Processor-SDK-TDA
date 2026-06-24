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

template<> int32_t TidlParseOnnx:: parse<OnnxStr("ConvTranspose")> ()
{
  int32_t status;
  int32_t padType;
  int32_t idx1, idx2;
  sTIDL_ConvParams_t &convParams = layer.layerParams.convParams;
  int32_t pads[4];
  int32_t output_shape[2];
  int32_t padL=-1, padR=-1, padT=-1, padB=-1;

  layer.layerType = TIDL_Deconv2DLayer;

  layer.strideOffsetMethod = TIDL_StrideOffsetTopLeft;

  layer.outData[0].elementType = tidl_getElementType(1);

  TensorProto tensor = getInitializerTensor(graph, graph.node(index).input(1), index, status);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Transpose Conv : Unable to find initializer at index %d for node %s", 1, graph.node(index).name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  layer.layerPCParams.convParams.tensorHeight =   getTensorDim(tensor, 2);
  layer.layerPCParams.convParams.tensorWidth =    getTensorDim(tensor, 3);

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
  getIntAttr(node, "strides", &convParams.strideH, 0);
  getIntAttr(node, "strides", &convParams.strideW, 1);
  getIntAttr(node, "dilations", &convParams.dilationH, 0);
  getIntAttr(node, "dilations", &convParams.dilationW, 1);
  getIntAttr(node, "group", &convParams.numGroups, 0);
  getIntAttr(node, "kernel_shape", &convParams.kernelH, 0);
  getIntAttr(node, "kernel_shape", &convParams.kernelW, 1);
  getIntAttr(node, "output_padding", &layer.layerPCParams.convParams.outPadH, 0);
  getIntAttr(node, "output_padding", &layer.layerPCParams.convParams.outPadW, 1);

  int32_t attrIdx = getAttrIdx(node, "kernel_shape");
  int32_t attrSize = 0;
  if(attrIdx != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    attrSize = node.attribute(attrIdx).ints_size();
    if (attrSize < 2)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Deconvolution : kernel_shape is missing height axis.");
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
          TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Deconvolution : kernel_shape is only supported along width and height axis.");
          return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
      }
    }
  }

  /* Weight tensor will have shape - (inChannels x outChannels/groups x kH x kW) */
  convParams.numInChannels = getTensorDim(tensor, 0);
  convParams.numOutChannels =  getTensorDim(tensor, 1) * convParams.numGroups;

  status = getIntAttr(node, "output_shape", &output_shape[0], 0);
  if(status != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    int32_t attrIdx = getAttrIdx(node, "output_shape");
    attrSize = node.attribute(attrIdx).ints_size();
    if (attrSize > 2)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Deconvolution : output_shape attribute value should not include dimensions for batch size and channels, which are automatically inferred");
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    else if (attrSize < 2)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Deconvolution : output_shape attribute must contain height and width dimensions");
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    else 
    {
      char autoPad[50];
      status = getStringAttr(node, "auto_pad", autoPad, 0);
      
      getIntAttr(node, "output_shape", &output_shape[0], 0);
      getIntAttr(node, "output_shape", &output_shape[1], 1);

      int32_t inputH = layer.allowlistingMetaData.varTensorsDims[0][layer.allowlistingMetaData.varTensorsDims[0].size() - 2];
      int32_t inputW = layer.allowlistingMetaData.varTensorsDims[0][layer.allowlistingMetaData.varTensorsDims[0].size() - 1];

      int32_t total_padH = std::max<int64_t>(0, (inputH - 1) * convParams.strideH + layer.layerPCParams.convParams.outPadH + (convParams.kernelH - 1) * convParams.dilationH + 1 - output_shape[0]);
      int32_t total_padW = std::max<int64_t>(0, (inputW - 1) * convParams.strideW + layer.layerPCParams.convParams.outPadW + (convParams.kernelW - 1) * convParams.dilationW + 1 - output_shape[1]);
      
      if(strcmp(autoPad, "SAME_UPPER") == 0)
      {
        padT = total_padH/2;
        padB = total_padH - (total_padH/2);
        padL = total_padW/2;
        padR = total_padW - (total_padW/2);

        layer.strideOffsetMethod = TIDL_StrideOffsetCenter;
      }
      else // for pad_type is NOTSET, SAME_LOWER or VALID
      {
        padT = total_padH - (total_padH/2);
        padB = total_padH/2;
        padL = total_padW - (total_padW/2);
        padR = total_padW/2;

        layer.strideOffsetMethod = TIDL_StrideOffsetTopLeft;
      }
    }
  }
  else
  {
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
      padT = pads[0];
      padL = pads[1];
      padB = pads[2];
      padR = pads[3];
    }
    else if ((strcmp(autoPad, "SAME_UPPER") == 0) || (strcmp(autoPad, "SAME_LOWER") == 0))
    {
      int32_t inputH = layer.allowlistingMetaData.varTensorsDims[0][layer.allowlistingMetaData.varTensorsDims[0].size() - 2];
      int32_t inputW = layer.allowlistingMetaData.varTensorsDims[0][layer.allowlistingMetaData.varTensorsDims[0].size() - 1];

      int32_t total_padH = std::max<int64_t>(0, (inputH - 1) * convParams.strideH + layer.layerPCParams.convParams.outPadH + (convParams.kernelH - 1) * convParams.dilationH + 1 - (inputH *convParams.strideH));
      int32_t total_padW = std::max<int64_t>(0, (inputW - 1) * convParams.strideW + layer.layerPCParams.convParams.outPadW + (convParams.kernelW - 1) * convParams.dilationW + 1 - (inputW *convParams.strideW));
     
      if(strcmp(autoPad, "SAME_UPPER") == 0)
      {
        padT = total_padH/2;
        padB = total_padH - (total_padH/2);
        padL = total_padW/2;
        padR = total_padW - (total_padW/2);

        layer.strideOffsetMethod = TIDL_StrideOffsetCenter;
      }
      else  //SAME_LOWER
      {
        padT = total_padH - (total_padH/2);
        padB = total_padH/2;
        padL = total_padW - (total_padW/2);
        padR = total_padW/2;

        layer.strideOffsetMethod = TIDL_StrideOffsetTopLeft;
      }
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
  }

  convParams.padT = padT;
  convParams.padL = padL;
  convParams.padB = padB;
  convParams.padR = padR;

  pads[3] = max(padL, padR);
  pads[2] = max(padT, padB);

/*
if ((pads[0] != 0) || (pads[1] != 0))
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Pading is only supported in H and W axis ");
  }
*/
  convParams.padW = pads[3];
  convParams.padH = pads[2];

  status = copyFloatConst(graph, index, 1, layer.weights, INPUT_REQUIRED);
  if(status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Transpose Conv : Unable to copy initializer at index %d for node %s", 1, graph.node(index).name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  /*Permute 0123 -> 1023 for onnx deconv weights based on https://github.com/onnx/onnx/blob/main/docs/Operators.md#ConvTranspose*/
  /*W Shape: Ni X No/G X Fr X FC */
  int32_t i0, i1, i2, i3;
  int32_t NoByG = convParams.numOutChannels / convParams.numGroups;
  int32_t tensorSize = convParams.numInChannels * NoByG * convParams.kernelH * convParams.kernelW;
  void ** ptr;
  ptr = &layer.weights.ptr;
  float* srcWtPtr = (float*) *ptr;
  float* weightPtr = (float*)my_calloc(tensorSize, sizeof(float));
  if(weightPtr == NULL)
  {
    TIDL_LOG_ERROR(gDiags.gDiagList, "Could not allocate transpose buffer for deconvolution");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  int32_t linePitch = convParams.kernelH * convParams.kernelW;

  if (convParams.numGroups > 1){
    for (i0 = 0; i0 < (convParams.numInChannels/convParams.numGroups); i0++)
    {
      for(i1 = 0; i1 < NoByG; i1++)
      {
        for(i2 = 0; i2 < (convParams.numGroups); i2++)
        {
          for(i3 = 0; i3 < linePitch; i3++)
          {
            weightPtr[i0* (linePitch * convParams.numGroups * NoByG)+ i1 * (linePitch * convParams.numGroups) + i2 * linePitch + i3] = srcWtPtr[(i0*NoByG*linePitch*convParams.numGroups) + (i1 * linePitch) + (i2 * linePitch * NoByG)  + i3 ];
          }
        }
      }
    }
  }
  else{
    for(i1 = 0; i1 < NoByG; i1++)
    {
      for(i2 = 0; i2 < convParams.numInChannels; i2++)
      {
        for(i3 = 0; i3 < linePitch; i3++)
        {
          weightPtr[ i1 * (linePitch * convParams.numInChannels) + i2 * linePitch + i3] = srcWtPtr[i2 * (linePitch * NoByG) + i1 * linePitch  + i3 ];
        }
      }
    }
  }
  
  free(*ptr);
  *ptr = (void*)weightPtr;
  /******************************************************************************************************************************/
  if (graph.node(index).input_size() > 2)
  {
    convParams.enableBias = 1;
    status = copyFloatConst(graph, index, 2, layer.bias, INPUT_REQUIRED);
    if(status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Transpose Conv : Unable to copy initializer at index %d for node %s", 2, graph.node(index).name().c_str());
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
  }
   return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}
