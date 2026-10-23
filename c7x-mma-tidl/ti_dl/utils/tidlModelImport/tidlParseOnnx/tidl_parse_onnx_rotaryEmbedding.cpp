/*
*
* Copyright (c) {2015 - 2026} Texas Instruments Incorporated
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
#include <math.h>
using namespace std;
using namespace onnx;

/*
 * GroupQueryAttention (com.microsoft contrib op)
 *
 * Dedicated native (float) attention layer with KV cache, grouped/multi-query
 * heads, optional rotary embedding, softcap and local sliding-window. Variable
 * inputs: query[, key, value], past_key, past_value, seqlens_k,
 * total_sequence_length. cos_cache / sin_cache are constant initializers that
 * are absorbed as parameter byte-offsets (kept in the params blob). All compute
 * happens in the reference kernel.
 */
template<> int32_t TidlParseOnnx::parse<OnnxStr("RotaryEmbedding")> ()
{
  int32_t status = TIDL_IMPORT_DIAGNOSIS_RETURN_OK;

  sTIDL_RotaryEmbeddingParams_t   &params   = layer.layerParams.rotaryEmbeddingParams;
  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;

  layer.layerType = TIDL_RotaryEmbeddingLayer;
  layer.numInBufs = md.numVarInputs;

  NodeProto node = graph.node(index);

  int32_t numHeads = 0;
  int32_t interleaved = 0;
  int32_t rotary_embedding_dim = 0;
  if ((getIntAttr(node, "num_heads", &numHeads, 0) == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL) && (md.numInputs == 3))
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : RotaryEmbedding : Missing/invalid num_heads for node %s", node.name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  if(md.varTensorsDims[0].size() == 4)
  {
    numHeads = md.varTensorsDims[0][1];
  }

  (void)getIntAttr(node, "rotary_embedding_dim", &rotary_embedding_dim, 0);
  (void)getIntAttr(node, "interleaved", &interleaved, 0);

  params.numHeads             = numHeads;
  params.interleaved          = interleaved;
  params.rotationDim = rotary_embedding_dim;
  /* No live seqlens_k source for a standalone ONNX-authored RotaryEmbedding
   * node; only import-synthesized RoPE nodes (feeding a cache-managed GQA
   * layer) set this to a real buffer index. */
  params.seqlensKBufIdx = -1;

  if (md.numVarInputs < 3)
  {
    status |= copyFloatConst(graph, index, 1, layer.bias, INPUT_REQUIRED);
    status = copyFloatConst(graph, index, 2, layer.weights, INPUT_REQUIRED);
    if (status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : RotaryEmbedding : Unable to read cos/sin cache for node %s", node.name().c_str());
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
  }

  if(graph.node(index).input_size() > 3)
  {
    TensorProto tensor = getInitializerTensor(graph, graph.node(index).input(3), index, status);
    TIDL_IMPORT_CHECK_AND_RETURN(copyTensor(graph, tensor, layer.layerParams.rotaryEmbeddingParams.positionIds, "position_ids", INPUT_REQUIRED), "");
  }

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}