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

template<> int32_t TidlParseOnnx:: parse<OnnxStr("Squeeze")> ()
{
  /** Num inputs for squeeze is 2 from onnx opset 13 (includes axes as an input), but it's an initializer actually, hence hard coding numInputs to 1*/
  layer.layerType = TIDL_SqueezeLayer;
  layer.numInBufs = 1;
  NodeProto node  = graph.node(index);

  int32_t axes_status = -1, axesIdx = -1;
  int32_t j, ii, numDim = 0;
  int32_t num_axes_4_squeeze = -1;
  int32_t axes[TIDL_DIM_MAX];

  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;

  /* Get dims of data tensor (input 0), whether it is variable or const */
  std::vector<int32_t> dataDims;
  for (int32_t k = 0; k < (int32_t)md.varTensorIndices.size(); k++)
  {
    if (md.varTensorIndices[k] == 0)
    {
      dataDims = md.varTensorsDims[k];
      break;
    }
  }
  for (int32_t k = 0; k < (int32_t)md.constTensorIndices.size(); k++)
  {
    if (md.constTensorIndices[k] == 0)
    {
      dataDims = md.constTensorsDims[k];
      break;
    }
  }

  axesIdx = getAttrIdx(node, "axes");
  if (axesIdx != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    axes_status = axesIdx;
    num_axes_4_squeeze = node.attribute(axesIdx).ints_size();
    for(ii = 0; ii< num_axes_4_squeeze; ii++)
    {
      getIntAttr(node, "axes",   &axes[ii], ii);
    }
  }
  else if(md.numInputs == 2 &&
          std::find(md.constTensorIndices.begin(), md.constTensorIndices.end(), 1) != md.constTensorIndices.end())
  {
    /* axes can be an constant input in opset 18*/
    sBuffer_t buf;
    axes_status = copyFloatConst(graph, index, 1, buf, INPUT_REQUIRED);
    if (axes_status != TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
    {
      int32_t status = 0;
      TensorProto tensor = getInitializerTensor(graph, node.input(1), index, status);
      num_axes_4_squeeze = buf.bufSize;
      for (int i = 0; i < num_axes_4_squeeze; i++)
      {
          if (tensor.data_type() == TensorProto_DataType_INT64)
          {
              axes[i] = (int32_t)(*((int64_t*)buf.ptr + i));
          }
          else if (tensor.data_type() == TensorProto_DataType_INT32)
          {
              axes[i] = (int32_t)(*((int32_t*)buf.ptr + i));
          }
          else if (tensor.data_type() == TensorProto_DataType_INT8)
          {
              axes[i] = (int32_t)(*((int8_t*)buf.ptr + i));
          }
          else if (tensor.data_type() == TensorProto_DataType_UINT8)
          {
              axes[i] = (int32_t)(*((uint8_t*)buf.ptr + i));
          }
      }
    }
    free (buf.ptr);
  }
  /* Below change fixes jira issue: "[TIDL-7518]: Updated Squeeze layer parser code when axes is not provided" */
  else if (md.numInputs == 1)
  {
    // If axes is not provided, all the single dimensions will be removed from the shape 
    for (int i = 0; i < (int32_t)dataDims.size(); i++)
    {
      int axes_size = 0;
      if(dataDims[i] == 1){
        axes[axes_size] = i;
        axes_size += 1;
      }
      num_axes_4_squeeze = axes_size;
    }
  }
  else
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Squeeze layer : Variable input for axes is not supported");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  if (axes_status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Squeeze layer : No axis given for Squeeze");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  /** Get numDim from data tensor (input 0), whether it is variable or const */
  numDim = (dataDims.size() != 0) ? (int32_t)dataDims.size() : 4;

  if(dataDims.size() != 0){
    for (ii=0 ; ii < num_axes_4_squeeze; ii++)
    {
      int32_t axisToSqueeze = axes[ii] < 0 ? numDim + axes[ii] : axes[ii];
      if(dataDims[axisToSqueeze] != 1)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Squeeze layer : Input Dimension across given axes should be 1");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }
  }

  /* Updating the axes to TIDL max dimensions*/
  for (ii = 0; ii < num_axes_4_squeeze; ii++)
  {
    if(axes[ii] >= 0)
    {
      /*+ve Axis*/
      axes[ii] = axes[ii] + (TIDL_DIM_MAX - numDim);
    }
    else
    {
      /*-ve Axis*/
       axes[ii] += (TIDL_DIM_MAX);
    }
  }

  for(ii = 0; ii< TIDL_DIM_MAX; ii++)
  {
    layer.layerPCParams.squeezeParams.axis[ii] = 0;
  }

  for(ii = 0; ii< TIDL_DIM_MAX; ii++)
  {
    for(j=0;j<num_axes_4_squeeze;j++)
    {
      if(ii == axes[j])
      {
        layer.layerPCParams.squeezeParams.axis[ii] = 1; // squeeze this particular axis
      }
    }
  }

  /* If data input (input 0) is a constant initializer, store it in layer.weights
   * so tidl_addConstDataLayers can create the appropriate ConstDataLayer */
  for (int32_t i = 0; i < (int32_t)md.numConstInputs; i++)
  {
    if (md.constTensorIndices[i] == 0)
    {
      int32_t status = copyFloatConst(graph, index, 0, layer.weights, INPUT_REQUIRED);
      if (status != 0)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Squeeze layer : Unable to read constant data input");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      break;
    }
  }

  return 0;
}
