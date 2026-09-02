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

template<> int32_t TidlParseOnnx:: parse<OnnxStr("Split")> ()
{
  int32_t status, j;
  int32_t axis = 0;
  int32_t splitSize, splitIdx = -1, numDim, attrIdx = -1, split_status = -1, num_out_status = -1, num_outputs = 0;
  sTIDL_SliceLayerParams_t &sliceParams = layer.layerParams.sliceParams;
  sTIDL_SlicePCParams_t &slicePCParams  = layer.layerPCParams.sliceParams;

  layer.layerType = TIDL_SliceLayer;
  layer.numOutBufs = graph.node(index).output_size();
  sliceParams.stride = 1;

  for (j = 0; j < layer.numOutBufs; j++)
  {
    layer.outData[j].elementType = tidl_getElementType(1); /*?*/
  }

  NodeProto node = graph.node(index);
  getIntAttr(node, "axis", &axis, 0);

  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;
  /** Get numDim from data tensor (input 0), whether it is variable or const */
  numDim = 4;
  for (int32_t k = 0; k < (int32_t)layer.allowlistingMetaData.varTensorIndices.size(); k++)
  {
    if (layer.allowlistingMetaData.varTensorIndices[k] == 0)
    {
      if (layer.allowlistingMetaData.varTensorsDims[k].size() != 0)
        numDim = layer.allowlistingMetaData.varTensorsDims[k].size();
      break;
    }
  }
  for (int32_t k = 0; k < (int32_t)layer.allowlistingMetaData.constTensorIndices.size(); k++)
  {
    if (layer.allowlistingMetaData.constTensorIndices[k] == 0)
    {
      if (layer.allowlistingMetaData.constTensorsDims[k].size() != 0)
        numDim = layer.allowlistingMetaData.constTensorsDims[k].size();
      break;
    }
  }

  if (axis < 0)
  {
    axis += numDim;
  }

  layer.inData[0].numDim = numDim;

  layer.layerParams.sliceParams.axis = axis + (TIDL_DIM_MAX - numDim);

  sliceParams.slicePoints[0] = 0;
  splitIdx = getAttrIdx(node, "split");

  if (splitIdx != -1)
  {
    split_status = splitIdx;
    splitSize = node.attribute(splitIdx).ints_size();
    if(splitSize != layer.numOutBufs)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Split layer : Unsupported slice - numSplits %d doesn't match with number of output bufs %d",
      splitSize, layer.numOutBufs);
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    for (j = 0; j < layer.numOutBufs; j++)
    {
      /* split attribute of split layer indicates the length of each slice */
      sliceParams.slicePoints[j+1] = sliceParams.slicePoints[j] + node.attribute(splitIdx).ints(j);
    }
  }
  else
  {
    /**Split can be a input in opset 18*/
    sBuffer_t buf;
    split_status = copyFloatConst(graph, index, 1, buf, INPUT_NOT_REQUIRED);
    if (split_status != -1)
    {
      if (buf.bufSize != layer.numOutBufs)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Split layer : Unsupported slice - numSplits %d doesn't match with number of output bufs %d",
        splitSize, layer.numOutBufs);
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      int64_t *split_ptr = (int64_t*)buf.ptr;
      for (j = 0; j < layer.numOutBufs; j++)
      {
        /* split attribute of split layer indicates the length of each slice */
        sliceParams.slicePoints[j+1] = sliceParams.slicePoints[j] + split_ptr[j];
      }
      free (buf.ptr);
      buf.ptr = NULL;
      buf.bufSize = 0;
    }
    else if(md.numVarInputs <= 1)
    {
      /*Split input does not exist*/
      slicePCParams.setSlicePoints = 1;
    }
    else
    {
      /*Split as variable information is not supported*/
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Split layer : Variable input for split is not supported");
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
  }

  num_out_status = getIntAttr(node, "num_outputs", &num_outputs, 0);

  /* Either num_outputs or split should be defined, but not both*/
  if (num_out_status != -1 && split_status == -1)
  {
    if (layer.numOutBufs != num_outputs)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Split layer : Either num_outputs or split should be defined, but not both");
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
  }

  /* If data input (input 0) is a constant initializer, store it in layer.weights
   * so tidl_addConstDataLayers can create the appropriate ConstDataLayer */
  for (int32_t i = 0; i < (int32_t)md.numConstInputs; i++)
  {
    if (md.constTensorIndices[i] == 0)
    {
      status = copyFloatConst(graph, index, 0, layer.weights, INPUT_REQUIRED);
      if (status != 0)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Split layer : Unable to read constant data input");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      break;
    }
  }

  return 0;
}

