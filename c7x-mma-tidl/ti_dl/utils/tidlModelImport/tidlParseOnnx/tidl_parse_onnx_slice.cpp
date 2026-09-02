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

template<> int32_t TidlParseOnnx:: parse<OnnxStr("Slice")> ()
{
  int32_t status, j, isMultipleAxis;
  int32_t axis = 0, numDim;
  sTIDL_SliceLayerParams_t &sliceParams = layer.layerParams.sliceParams;
  sBuffer_t buf, starts, ends, axes;

  layer.layerType  = TIDL_SliceLayer;

  NodeProto node = graph.node(index);
  sliceParams.stride = 1;

  /* In Slice-1, axis can be attribute */
  int32_t axis_status = getIntAttr(node, "axes", &axis, 0);
  /* In Slice-1, starts and ends can be attribute */
  int32_t startsAttrStatus =  getIntAttr(node, "starts", &sliceParams.slicePoints[0], 0);
  int32_t endsAttrStatus   =  getIntAttr(node, "ends",   &sliceParams.slicePoints[1], 0);

  if (getIntAttr(node, "starts", &isMultipleAxis, 1) != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Slice layer : TIDL_RT only supports single slice axis");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }



  if(node.input_size() < 4)
  {
    if (axis_status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
    {
      axis = 0;
    }
  }
  else
  {
    /* if Steps input is present */
    if (node.input_size() == 5)
    {
      status = copyFloatConst(graph, index, 4, buf, INPUT_REQUIRED);
      if(status != 0)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Slice layer : Unable to find float intializer");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      long long int *ptr = (long long int *)buf.ptr;
      if (ptr[0] != 1)
      {
        sliceParams.stride = ptr[0];
      }
      free(ptr);
      buf.ptr = NULL;
      buf.bufSize = 0;
    }
  }

  /* Read starts, ends and axes input Slice-10 and above*/
  if (
    (startsAttrStatus == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL) &&
    (endsAttrStatus == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)   &&
    (axis_status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  )
  {
      sBuffer_t starts;
      status = copyFloatConst(graph, index, 1, starts, INPUT_REQUIRED);
      if(status != 0)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Slice layer : Unable to find float intializer");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      long long int *startsPtr = (long long int *)starts.ptr;

      sBuffer_t ends;
      status = copyFloatConst(graph, index, 2, ends, INPUT_REQUIRED);
      if(status != 0)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Slice layer : Unable to find float intializer");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      long long int *endsPtr = (long long int *)ends.ptr;

      sBuffer_t axes;
      status = copyFloatConst(graph, index, 3, axes, INPUT_REQUIRED);
      if(status != 0)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Slice layer : Unable to find float intializer");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      long long int *axesPtr = (long long int *)axes.ptr;

      if(starts.bufSize > 1)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowisting : Slice layer : TIDL_RT only supports single slice axis");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      else
      {
        sliceParams.slicePoints[0] = startsPtr[0];
        if(endsPtr[0] == 0x7fffffffffffffff)
        {
          sliceParams.slicePoints[1] = 0x7fffffff;
        }
        else
        {
          sliceParams.slicePoints[1] = endsPtr[0];
        }
        axis = axesPtr[0];
      }
      free(startsPtr);
      starts.ptr = NULL;
      starts.bufSize = 0;
      free(endsPtr);
      ends.ptr = NULL;
      ends.bufSize = 0;
      free(axesPtr);
      axes.ptr = NULL;
      axes.bufSize = 0;
  }

  /** Get numDim from data tensor (input 0), whether it is variable or const */
  numDim = 4; // Default
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

  /** Adjust axis to TIDL dimensions*/
  if(axis >= 0)
  {
    /*+ve Axis*/
    axis += (TIDL_DIM_MAX - numDim);
  }
  else
  {
    /*-ve Axis*/
    axis += TIDL_DIM_MAX;
  }
  layer.layerParams.sliceParams.axis = axis;

  /* If data input (input 0) is a constant initializer, store it in layer.weights
   * so tidl_addConstDataLayers can create the appropriate ConstDataLayer */
  for (int32_t i = 0; i < (int32_t)layer.allowlistingMetaData.numConstInputs; i++)
  {
    if (layer.allowlistingMetaData.constTensorIndices[i] == 0)
    {
      status = copyFloatConst(graph, index, 0, layer.weights, INPUT_REQUIRED);
      if (status != 0)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Slice layer : Unable to read constant data input");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      break;
    }
  }

  return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}

