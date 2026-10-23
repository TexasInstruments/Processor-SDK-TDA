/*
*
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
#include <tvm/relay/attrs/transform.h>

template<>
int32_t TidlParseTVM:: parse<OpNameStr("reshape")> ()
{
  int32_t status = 0;
  auto attrs = call->attrs.as<ReshapeAttrs>();
  int32_t * shape;
  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;

  if (attrs == nullptr)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Reshape layer : Unable to get ReshapeAttrs");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  layer.layerType = TIDL_ReshapeLayer;
  layer.weights.ptr = my_malloc(attrs->newshape.size()*sizeof(int32_t));
  layer.weights.bufSize = attrs->newshape.size();
  shape = (int32_t *)layer.weights.ptr;
  for(int i=0; i < attrs->newshape.size(); i++)
  {
    shape[i] = (int32_t)attrs->newshape[i]->value;
  }

  /* Load const data tensor (input[0]) into layer.bias when it is a constant. */
  {
    auto constIt = std::find(md.constTensorIndices.begin(), md.constTensorIndices.end(), 0);
    if (constIt != md.constTensorIndices.end())
    {
      layer.bias = TIDL_extractConstantTensorData(call, 0, md, 0);
    }
    else
    {
      layer.bias.ptr     = NULL;
      layer.bias.bufSize = 0;
    }
  }

  status = checkShapeInferenceforOnnx(md);

  layer.layerParams.reshapeParams.minusOneDimIdx  = -1;
  layer.layerParams.reshapeParams.passthroughMask = 0;
  if (status != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    shape             = (int32_t*)layer.weights.ptr;
    int32_t remDim    = TIDL_DIM_MAX - layer.weights.bufSize;
    int32_t negIdx    = -1;
    int32_t ptMask6D  = 0;
    for (int32_t i = 0; i < layer.weights.bufSize; i++)
    {
      if (shape[i] == 0 && attrs->allowzero == 0)
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

  layer.layerPCParams.reshapeParams.allowZero = attrs->allowzero;

  return 0;
}

