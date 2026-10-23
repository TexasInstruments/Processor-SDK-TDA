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
template<> int32_t TidlParseOnnx::parse<OnnxStr("GroupQueryAttention")> ()
{
  int32_t status = TIDL_IMPORT_DIAGNOSIS_RETURN_OK;

  sTIDL_GroupQueryAttentionParams_t   &params   = layer.layerParams.gqaParams;
  sTIDL_GroupQueryAttentionPCParams_t &pcParams = layer.layerPCParams.gqaParams;
  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;

  layer.layerType = TIDL_GroupQueryAttentionLayer;
  layer.numInBufs = md.numVarInputs;

  NodeProto node = graph.node(index);

  int32_t numHeads = 0, kvNumHeads = 0;
  if ((getIntAttr(node, "num_heads", &numHeads, 0) == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL) ||
      (getIntAttr(node, "kv_num_heads", &kvNumHeads, 0) == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL) ||
      (numHeads <= 0) || (kvNumHeads <= 0))
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : GroupQueryAttention : Missing/invalid num_heads or kv_num_heads for node %s", node.name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  int32_t localWindowSize = -1, doRotary = 0, rotaryInterleaved = 0;
  float32_tidl softcap = 0.0f, scale = 0.0f;
  (void)getIntAttr(node, "local_window_size", &localWindowSize, 0);
  (void)getIntAttr(node, "do_rotary", &doRotary, 0);
  (void)getIntAttr(node, "rotary_interleaved", &rotaryInterleaved, 0);
  (void)getFloatAttr(node, "softcap", &softcap, 0);
  (void)getFloatAttr(node, "scale", &scale, 0);

  /* Detect packed QKV: key (input 1) and value (input 2) absent/empty. */
  int32_t isPackedQKV = ((node.input_size() < 3) ||
                         (node.input(1).size() == 0) || (node.input(2).size() == 0)) ? 1 : 0;

  /* Past/present KV cache present when past_key (input 3) is a valid name. */
  int32_t hasPast = ((node.input_size() > 3) && (node.input(3).size() > 0)) ? 1 : 0;

  /* head_size from query last dim / heads. query hidden = numHeads*hs (sep) or
     (numHeads + 2*kvNumHeads)*hs (packed). */
  int32_t qHidden = (md.varTensorsDims.size() > 0 && md.varTensorsDims[0].size() > 0) ?
                      md.varTensorsDims[0].back() : 0;
  int32_t headSize = 0;

  if (qHidden > 0)
  {
    headSize = isPackedQKV ? (qHidden / (numHeads + 2 * kvNumHeads)) : (qHidden / numHeads);
  }

  /* Fallback: derive head_size from the explicit scale attribute.
   * The standard relationship is scale = 1/sqrt(head_size), so
   * head_size = round(1 / scale^2). Only accept if the result is a
   * positive integer close to the exact value (tolerance 0.5). */
  if (headSize <= 0 && scale != 0.0f)
  {
    double hsExact  = 1.0 / ((double)scale * (double)scale);
    int32_t hsRound = (int32_t)round(hsExact);
    if (hsRound > 0 && fabs(hsExact - hsRound) < 0.5)
    {
      headSize = hsRound;
    }
  }

  if (headSize <= 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : GroupQueryAttention : Unable to infer head_size for node %s", node.name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  /* Find the TIDL buffer index of seqlens_k (ONNX input 5) by scanning varTensorIndices.
   * This is necessary because optional inputs (key, value, past_key, past_value) may be
   * absent, shifting the TIDL buffer positions relative to the ONNX input indices. */
  int32_t seqlensKBufIdx = -1;
  for (int32_t i = 0; i < (int32_t)md.varTensorIndices.size(); i++)
  {
    if (md.varTensorIndices[i] == 5) { seqlensKBufIdx = i; break; }
  }

  params.numHeads          = numHeads;
  params.kvNumHeads        = kvNumHeads;
  params.headSize          = headSize;
  params.scale             = (scale != 0.0f) ? scale : (float32_tidl)(1.0 / sqrt((double)headSize));
  params.softcap           = softcap;
  params.localWindowSize   = localWindowSize;
  params.doRotary          = doRotary;
  params.rotaryInterleaved = rotaryInterleaved;
  params.isPackedQKV       = isPackedQKV;
  params.hasPast           = hasPast;
  params.rotaryDim         = 0;
  params.cosCache          = 0;
  params.sinCache          = 0;
  params.seqlensKBufIdx    = seqlensKBufIdx;

  /* cos_cache (input 7) and sin_cache (input 8) : optional float initializers. */
  if (doRotary && (node.input_size() > 8) &&
      (node.input(7).size() > 0) && (node.input(8).size() > 0))
  {
    status = copyFloatConst(graph, index, 7, pcParams.cosCache, INPUT_REQUIRED);
    status |= copyFloatConst(graph, index, 8, pcParams.sinCache, INPUT_REQUIRED);
    if (status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : GroupQueryAttention : Unable to read cos/sin cache for node %s", node.name().c_str());
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    /* Reference kernel applies rotary over full head_size. */
    params.rotaryDim = headSize;
  }

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}
