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

template<> int32_t TidlParseOnnx:: parse<OnnxStr("ReduceSum")> ()
{
  int32_t numDim;
  int32_t status = 0;
  layer.layerType = TIDL_ReduceSumLayer;
  layer.numInBufs = 1;
  NodeProto node  = graph.node(index);
  getIntAttr(node, "keepdims", &layer.layerPCParams.reduceSumParams.keepDims, 0);

  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;
  /** If shape inference is not done on model, we assume the tensor is of 4 dimensions by default*/
  if(md.varTensorsDims.size() != 0)
  {
    numDim = md.varTensorsDims[0].size();
    numDim = (numDim == 0)? 4:numDim;
  }
  else
  {
    numDim = 4;
  }

  /* axes attribute is there in lower than 18 opset */
  int32_t axis = 0, axisStatus = 0;
  int32_t axesIdx  = getAttrIdx(graph.node(index), "axes");
  if(axesIdx != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    int32_t axesSize = graph.node(index).attribute(axesIdx).ints_size();
    if(axesSize > 1)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : ReduceSum layer : TIDL does not support reduction in multiple axis");
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    else if(axesSize == 0)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : ReduceSum layer : Unable to find axis for reduction");
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    axisStatus = getIntAttr(node, "axes", &axis, 0);
  }
  else
  {
    /* Attribute added in opset 18 */
    int32_t noop_with_empty_axes = 0;
    getIntAttr(node, "noop_with_empty_axes", &noop_with_empty_axes, 0);

    /* axis is an input in opset >= 18 */
    sBuffer_t axes;
    axes.bufSize = 0;
    if(md.numConstInputs >= 1)
    {
      status = copyFloatConst(graph, index, md.constTensorIndices[0], axes, INPUT_NOT_REQUIRED);
      if(status != TIDL_IMPORT_DIAGNOSIS_RETURN_OK)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : ReduceMax layer : Unable to find initializer at index %d for node %s", md.constTensorIndices[0], graph.node(index).name().c_str());
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }

    if(axes.bufSize > 0) /* Check whether axes input is given. */
    {
      if (axes.bufSize >= 1)
      {
        axis = ((int64_t *)axes.ptr)[0];
        if(axes.bufSize > 1)
        {
          TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : ReduceMax layer : TIDL does not support reduction in multiple axis");
          return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
      }
    }
    else
    {
      /* If axes value is not given, and noop_with_empty_axes is true then perform no operation on the input tensor. Return it as such.*/
      if(noop_with_empty_axes)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : ReduceMax layer : No operation to be performed on the input tensor");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      /* If noop_with_empty_axes is not true(not set) then reduce all the axes in the input tensor. */
      else
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : ReduceMax layer : TIDL does not support reduction in multiple axis");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }
  }

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
  layer.layerPCParams.reduceSumParams.axis = axis;

  return 0;
}

