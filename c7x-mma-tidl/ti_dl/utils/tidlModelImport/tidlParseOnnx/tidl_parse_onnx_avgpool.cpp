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

template<> int32_t TidlParseOnnx:: parse<OnnxStr("AveragePool")> ()
{
  int32_t status = 0;


  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;

  sTIDL_PoolingParams_t &poolParams = layer.layerParams.poolParams;
  poolParams.countIncludePad = 0;

  std::vector<int32_t> attr;

  int32_t attrIdx = getAttrIdx(graph.node(index), "kernel_shape");
  int32_t attrSize = 0;
  if(attrIdx != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    attrSize = graph.node(index).attribute(attrIdx).ints_size();
    if (attrSize == 2)
    {
      attr.push_back(getIntAttr(graph.node(index), "kernel_shape", &poolParams.kernelW, 0));
      attr.push_back(getIntAttr(graph.node(index), "kernel_shape", &poolParams.kernelH, 1));
    }
    else if (attrSize < 2)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : AvgPool : kernel_shape is missing height axis.");
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    else
    {
      int32_t temp;
      for(int32_t i = 2; i < attrSize; i++)
      {
        status = getIntAttr(graph.node(index), "kernel_shape", &temp, i);
        if (status != -1 && temp != 1)
        {
          TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : AvgPool : kernel_shape is only supported along width and height axis.");
          return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
      }
    }
  }
  else
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : AvgPool : kernel_shape attribute is missing");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  if(getAttrIdx(graph.node(index), "count_include_pad") != -1)
  {
    attr.push_back(getIntAttr(graph.node(index), "count_include_pad", &poolParams.countIncludePad, 0));
  }

  if(std::find(attr.begin(), attr.end(), -1) != attr.end())
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : AvgPool : Missing attribute");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  int32_t isGlobalAvgPool = 0;
  if(gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX_RT)
  {
    if((md.varTensorsDims[0][md.varTensorsDims[0].size()-2] == poolParams.kernelH) && (md.varTensorsDims[0][md.varTensorsDims[0].size()-1] == poolParams.kernelW)) //global avg pool - e.g. squeezenet1.1
    {
      isGlobalAvgPool = 1;
    }
  }
  if(isGlobalAvgPool)
  {
    status = this->parse<OnnxStr("GlobalAveragePool")>();
  }
  else
  {
    status = this->parse<OnnxStr("MaxPool")>();
    poolParams.poolingType = TIDL_AveragePooling;

    // Convert 1x1 average pooling into max pooling (operations are identical)
    // mxnet_resnet50_v1d
    if (poolParams.kernelH == 1 && poolParams.kernelW == 1 &&
        ((poolParams.strideH == 2 && poolParams.strideW == 2) ||
        (poolParams.strideH == 1 && poolParams.strideW == 1)))
    {
      poolParams.poolingType = TIDL_MaxPooling;
    }
  }

  return 0;
}

