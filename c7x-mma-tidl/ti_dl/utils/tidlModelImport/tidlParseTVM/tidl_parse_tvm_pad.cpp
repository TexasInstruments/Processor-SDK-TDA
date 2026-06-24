/*
 *
 * Copyright (c) {2015 - 2025} Texas Instruments Incorporated
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

#include <tvm/relay/attrs/nn.h>

template <>
int32_t TidlParseTVM::parse<OpNameStr("pad")>()
{
  int32_t j;
  layer.layerType = TIDL_PadLayer;

  int32_t padT = 0, padB = 0, padL = 0, padR = 0;
  int32_t tot_axis, numDims;
  float32_tidl constant_value = -1;
  sBuffer_t buf;
  buf.ptr = NULL;
  buf.bufSize = 0;
  auto attrs = call->attrs.as<PadAttrs>();
  if (attrs == nullptr)
  {
    TIDL_LOG_ERROR(gDiags.gDiagList, "Pad layer: Unable to get PadAttrs");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  /** If shape inference is not done on model, we assume the tensor is of 4 dimensions by default*/
  if (layer.allowlistingMetaData.varTensorsDims.size() != 0)
  {
    numDims = layer.allowlistingMetaData.varTensorsDims[0].size();
    numDims = (numDims == 0) ? 4 : numDims;
  }
  else
  {
    numDims = 4;
  }

  std::string padMode = attrs->pad_mode;
  if ((padMode == "constant"))
  {
    constant_value=0;
    /* Pad value can be in 2nd input */
    if (call->args.size() >= 2)
    {

      // In TVM pad, if there's a constant pad value, it's typically in args[1]
      // Check if args[1] is a constant
      if (call->args[1].as<tvm::relay::ConstantNode>())
      {
        buf = TIDL_extractConstantTensorData(call, layer.allowlistingMetaData.constTensorIndices[0], layer.allowlistingMetaData, 0);
        if(buf.ptr != NULL)
        {
          float32_tidl *padPtr = (float32_tidl *)buf.ptr;
          constant_value = padPtr[0];
          layer.layerParams.padLayerParams.padConstValue = (int32_t)padPtr[0];
        }
        else
        {
          layer.layerParams.padLayerParams.padConstValue = 0;
        }
      }
      else
      {
        // args[1] is not a constant, default to 0
        layer.layerParams.padLayerParams.padConstValue = 0;
      }
    }
    /* If still not found, set default value as 0 */
    else
    {
      layer.layerParams.padLayerParams.padConstValue = 0;
    }

    /* Only zero pad is supported for constant mode */
    if (constant_value != 0)
    {
      layer.layerParams.padLayerParams.padType = TIDL_PadModeUnsupported;
    }
    else
    {
      layer.layerParams.padLayerParams.padType = TIDL_PadZero;
    }
  }
  else
  {
    layer.layerParams.padLayerParams.padType = TIDL_PadModeUnsupported;
   }

  /* Extract pad values from pad_width attribute */
  tot_axis = attrs->pad_width.size();

  if (tot_axis > 2) /* more than width and height */
  {
    for (j = 0; j < (tot_axis - 2); j++)
    {
      int32_t padBefore = attrs->pad_width[j][0].as<IntImmNode>()->value;
      int32_t padAfter = attrs->pad_width[j][1].as<IntImmNode>()->value;

      /* Cannot be non-zero in these dimensions */
      if ((padBefore != 0) || (padAfter != 0))
      {
        layer.layerParams.padLayerParams.padType = TIDL_PadNonWidthHeight;
      }
    }
  }
  
  padT = (tot_axis >= 2) ? attrs->pad_width[tot_axis - 2][0].as<IntImmNode>()->value : 0;
  padB = (tot_axis >= 2) ? attrs->pad_width[tot_axis - 2][1].as<IntImmNode>()->value : 0;
  padL = (tot_axis >= 1) ? attrs->pad_width[tot_axis - 1][0].as<IntImmNode>()->value : 0;
  padR = (tot_axis >= 1) ? attrs->pad_width[tot_axis - 1][1].as<IntImmNode>()->value : 0;

  layer.layerParams.padLayerParams.padT = padT;
  layer.layerParams.padLayerParams.padB = padB;
  layer.layerParams.padLayerParams.padL = padL;
  layer.layerParams.padLayerParams.padR = padR;

  return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}
