/*
 * Copyright (c) {2025} Texas Instruments Incorporated
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

#include "tidl_parse_tvm.h"
using namespace std;
using namespace tvm;
#include <tvm/relay/function.h>
#include <tvm/relay/expr.h>
#include <tvm/relay/analysis.h>
#include <tvm/runtime/ndarray.h>

template <>
int32_t TidlParseTVM::parse<OpNameStr("tidl.unsqueeze")>()
{
  layer.layerType = TIDL_UnsqueezeLayer;
  layer.numInBufs = 1;
  int32_t axes_status = -1;
  int32_t j, ii, numDim = 0;
  int32_t num_axes_4_squeeze = -1;
  int32_t axes[TIDL_DIM_MAX];
  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;

  const auto *fn_node = call->op.as<tvm::relay::FunctionNode>();
  auto fn = GetRef<tvm::relay::Function>(fn_node);
  const auto *attrs = fn->attrs.as<DictAttrsNode>();

  if (attrs && attrs->dict.count("axes"))
  {
    axes_status = 0;
    Array<Integer> axis_array = Downcast<Array<Integer>>(attrs->dict.at("axes"));
    num_axes_4_squeeze = axis_array.size();
    for (ii = 0; ii < num_axes_4_squeeze; ii++)
    {
      axes[ii] = Downcast<Integer>(axis_array[ii])->value;
    }
  }
  else if (md.numConstInputs >= 1)
  {
    auto axesConstIt = std::find(md.constTensorIndices.begin(), md.constTensorIndices.end(), 1);
    if (axesConstIt != md.constTensorIndices.end())
    {
      int32_t axesConstIdx = std::distance(md.constTensorIndices.begin(), axesConstIt);
      sBuffer_t buf = TIDL_extractConstantTensorData(call, 1, md, axesConstIdx);
      if (buf.ptr != NULL)
      {
        axes_status = 0;
        num_axes_4_squeeze = buf.bufSize;
        long long int *axesPtr = (long long int *)buf.ptr;
        for (int i = 0; i < num_axes_4_squeeze; i++)
        {
          axes[i] = (int32_t)axesPtr[i];
        }
        free(buf.ptr);
        buf.ptr = NULL;
        buf.bufSize = 0;
      }
    }
  }
  else if (md.numVarInputs > 1)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Unsqueeze layer : Variable input for axes is not supported");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  if (axes_status == -1)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Unsqueeze layer : No axis given for Unsqueeze");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  /** Get numDim from data tensor (input 0), whether it is variable or const */
  numDim = TIDL_getNumDimsForTensorIndex(md, 0);

  for (ii = 0; ii < TIDL_DIM_MAX; ii++)
  {
    layer.layerPCParams.unsqueezeParams.axes[ii] = 0;
    layer.layerPCParams.unsqueezeParams.axis[ii] = 0;
  }

  for (ii = 0; ii < num_axes_4_squeeze; ii++)
  {
    if (axes[ii] >= 0)
    {
      axes[ii] += (TIDL_DIM_MAX - (num_axes_4_squeeze + numDim));
    }
    else
    {
      axes[ii] += TIDL_DIM_MAX;
    }
    layer.layerPCParams.unsqueezeParams.axes[ii] = axes[ii];
  }

  for (ii = 0; ii < TIDL_DIM_MAX; ii++)
  {
    for (j = 0; j < num_axes_4_squeeze; j++)
    {
      if (ii == axes[j])
      {
        layer.layerPCParams.unsqueezeParams.axis[ii] = 1;
      }
    }
  }

  /* If data input (input 0) is a constant, store it in layer.weights
   * so tidl_addConstDataLayers can create the appropriate ConstDataLayer */
  for (int32_t i = 0; i < (int32_t)md.numConstInputs; i++)
  {
    if (md.constTensorIndices[i] == 0)
    {
      layer.weights = TIDL_extractConstantTensorData(call, 0, md, i);
      if (layer.weights.ptr == NULL)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Unsqueeze layer : Unable to read constant data input");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      break;
    }
  }

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}
