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

template<> int32_t TidlParseOnnx:: parse<OnnxStr("Gather")> ()
{
  int32_t status = 0;
  int32_t axis = 0;
  int32_t numDim;
  TensorProto indexTensor = onnx::TensorProto::default_instance();
  bool indexTensorFound = false;
  layer.layerType = TIDL_GatherLayer;
  layer.numInBufs = graph.node(index).input_size();
  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;
  layer.layerParams.gatherParams.viewChannels = 1;
  layer.layerParams.gatherParams.thresholdPadSize = 0;
  layer.layerParams.gatherParams.thresholdPadValue = 0;

  /** If shape inference is not done on model, we assume the tensor is of 4 dimensions by default.
   *  numDim must reflect the DATA tensor rank for correct axis translation. */
  if (md.numConstInputs > 0 && md.constTensorIndices[0] == 0)
  {
    // data is const: get rank from constTensorsDims
    numDim = md.constTensorsDims[0].size();
    numDim = (numDim == 0) ? 4 : numDim;
  }
  else if (md.varTensorsDims.size() != 0)
  {
    numDim = md.varTensorsDims[0].size();
    numDim = (numDim == 0)? 4:numDim;
  }
  else
  {
    numDim = 4;
  }

  getIntAttr(graph.node(index), "axis", &axis, 0);

  /*Translate to TIDL Axis:*/
  if(axis >= 0)
  {
    /*+ve Axis*/
    axis += (TIDL_DIM_MAX - numDim);
  }
  else
  {
    /*-ve Axis*/
    axis = (TIDL_DIM_MAX + axis);
  }

  layer.layerParams.gatherParams.axis = axis;
  layer.layerParams.gatherParams.isIdxScalar = 0;
  
  if(md.numConstInputs > 1)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Gather layer : Both data and indices as constants is not supported");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  if(md.numConstInputs > 0)
  {
    int constTensorIdx = md.constTensorIndices[0];
    status = copyFloatConst(graph, index, constTensorIdx, layer.weights, INPUT_REQUIRED);
    if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot read initializer tensor : Only float, int32 and int64 tensor is supported");
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }

    if(md.constTensorIndices[0] == 1)
    {
      indexTensor = getInitializerTensor(graph, graph.node(index).input(1), index, status);
      if(status == TIDL_ALLOWLISTING_LAYER_CHECK_PASSED)
      {
        indexTensorFound = true;
      }
    }

    // Check if index tensor has scalar value
    if (indexTensorFound && indexTensor.dims_size() == 0)
    {
      layer.layerParams.gatherParams.isIdxScalar = 1;
    }

    // [Doubt] check status here
    if (indexTensorFound && indexTensor.data_type() == TensorProto_DataType_INT64)
    {
      /**
       * TIDL_GatherLayer doesn't support int64 indices, hence convert them to int32
      */
      int32_t *index_dst_ptr = (int32_t*)malloc (sizeof(int32_t) * layer.weights.bufSize);

      if (index_dst_ptr == NULL)
      {
        TIDL_LOG_ERROR(gDiags.gDiagList, "GatherLayer -- Node name %s : Could not allocate memory for gather constant indices",
        graph.node(index).name().c_str());
        return -1;
      }

      int64_t *index_src_ptr = (int64_t*) layer.weights.ptr;
      int64_t val = 0;

      for (int32_t j = 0; j < layer.weights.bufSize; j++)
      {
        val = index_src_ptr[j];
        index_dst_ptr[j] = (int32_t)val;
      }

      /* Update the buffer pointer*/
      free(index_src_ptr);
      layer.weights.ptr = index_dst_ptr;
    }
  }

  int8_t dataIsVar = 0, indicesIsVar = 0;
  for (int32_t i = 0; i < md.varTensorIndices.size(); i++)
  {
    if (md.varTensorIndices[i] == 0) dataIsVar = 1;
    if (md.varTensorIndices[i] == 1) indicesIsVar = 1;
  }

  int32_t dataDimCount = 0, indicesDimCount = 0;

  if(dataIsVar && indicesIsVar)
  {
    dataDimCount = md.varTensorsDims[0].size();
    indicesDimCount = md.varTensorsDims[1].size();
  }

  // Data is variable and indices is constant
  else if (dataIsVar && !indicesIsVar)
  {
    dataDimCount = md.varTensorsDims[0].size();
    indicesDimCount = md.constTensorsDims[0].size();
  }

  // Data is constant and indices is variable
  else if (!dataIsVar && indicesIsVar)
  {
    dataDimCount = md.constTensorsDims[0].size();
    indicesDimCount = md.varTensorsDims[0].size();
  }

  layer.inData[0].numDim = dataDimCount;
  layer.inData[1].numDim = indicesDimCount;

  return 0;
}

