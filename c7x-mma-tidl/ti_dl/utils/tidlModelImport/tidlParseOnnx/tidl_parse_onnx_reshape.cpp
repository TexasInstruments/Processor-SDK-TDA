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

template<> int32_t TidlParseOnnx:: parse<OnnxStr("Reshape")> ()
{
  int32_t status = 0, attrIdx = -1, allowzero = 0;
  layer.layerType = TIDL_ReshapeLayer;
  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;

  status = copyFloatConst(graph, index, 1, layer.weights, INPUT_REQUIRED);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Reshape layer : Unable to find initializer at index %d for node %s", 1, graph.node(index).name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  /* Load const data tensor (input[0]) into layer.bias when it is a constant initializer. */
  {
    auto constIt = std::find(md.constTensorIndices.begin(), md.constTensorIndices.end(), 0);
    if (constIt != md.constTensorIndices.end())
    {
      status = copyFloatConst(graph, index, 0, layer.bias, INPUT_REQUIRED);
      if (status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
      {
        TIDL_LOG_ERROR(gDiags.gDiagList, "Allowlisting : Reshape layer : Unable to read const data initializer at index 0 for node %s", graph.node(index).name().c_str());
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }
    else
    {
      layer.bias.ptr     = NULL;
      layer.bias.bufSize = 0;
    }
  }

  status = checkShapeInferenceforOnnx(md);

  NodeProto node = graph.node(index);
  attrIdx = getIntAttr(node, "allowzero", &allowzero, 0);

  layer.layerParams.reshapeParams.minusOneDimIdx = -1;
  layer.layerParams.reshapeParams.passthroughMask = 0;
  if (status != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    int64_t* shape    = (int64_t*)layer.weights.ptr;
    int32_t  remDim   = TIDL_DIM_MAX - layer.weights.bufSize;
    int32_t  negIdx   = -1;
    int32_t  ptMask6D = 0;
    for (int32_t i = 0; i < layer.weights.bufSize; i++)
    {
      if (shape[i] == 0 && allowzero == 0)
      {
        ptMask6D |= (1 << (remDim + i));
      }
      else if (shape[i] == -1)
      {
        negIdx = i;
      }
    }
    layer.layerParams.reshapeParams.passthroughMask = ptMask6D;
    layer.layerParams.reshapeParams.minusOneDimIdx  = (negIdx >= 0) ? (remDim + negIdx) : -1;
  }

  /* changing the shapes to int32_t */
  int64_t* shape = (int64_t*)layer.weights.ptr;
  int32_t bufSize = layer.weights.bufSize;
  int32_t* newShape = (int32_t*)malloc(bufSize*sizeof(int32_t));
  for (int32_t i = 0; i < layer.weights.bufSize; i++)
  {
    newShape[i] = shape[i];
  }
  if(layer.weights.ptr != NULL)
  {
    my_free(layer.weights.ptr);
  }
  layer.weights.ptr = malloc(bufSize*sizeof(int32_t));
  memcpy(layer.weights.ptr, newShape, bufSize*sizeof(int32_t));

  my_free(newShape);

  layer.layerPCParams.reshapeParams.allowZero = allowzero;

  return 0;
}

