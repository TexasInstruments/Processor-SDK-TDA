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
using namespace std;
using namespace onnx;

/*
 * MatMulNBits (com.microsoft contrib op)
 *
 * Y = A * dequant(B)^T (+ bias), where the right-hand-side weight matrix B is
 * block-wise N-bit weight-quantized. This is implemented as a dedicated native
 * TIDL layer: the bit-packed weights are kept packed and dequantized inside the
 * reference kernel. Only input A is a variable input; B/scales/zero_points/bias
 * are constant initializers absorbed as parameter byte-offsets (not input
 * buffers), and are written into the network params blob at serialization time.
 */
template<> int32_t TidlParseOnnx::parse<OnnxStr("MatMulNBits")> ()
{
  int32_t status = TIDL_IMPORT_DIAGNOSIS_RETURN_OK;

  sTIDL_MatMulNBitsParams_t   &params   = layer.layerParams.matMulNBitsParams;
  sTIDL_MatMulNBitsPCParams_t &pcParams = layer.layerPCParams.matMulNBitsParams;
  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;

  layer.layerType = TIDL_MatMulNBitsLayer;
  /* Only A is a variable input; the quantized weight, scales, zero_points and
     bias are constant initializers that get absorbed as parameter offsets. */
  layer.numInBufs = md.numVarInputs;

  NodeProto node = graph.node(index);

  /* ---- Required attributes: K, N, block_size ---- */
  int32_t K = 0, N = 0, blockSize = 0;
  if ((getIntAttr(node, "K", &K, 0) == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL) ||
      (getIntAttr(node, "N", &N, 0) == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL) ||
      (getIntAttr(node, "block_size", &blockSize, 0) == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL))
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : MatMulNBits layer : Missing required attribute (K/N/block_size) for node %s", node.name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  /* ---- Optional attributes: bits (default 4), accuracy_level (default 0) ---- */
  int32_t bits = 4;
  (void)getIntAttr(node, "bits", &bits, 0);
  int32_t accuracyLevel = 0;
  (void)getIntAttr(node, "accuracy_level", &accuracyLevel, 0);

  if ((bits != 4) && (bits != 8))
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : MatMulNBits layer : Only bits = 4 or 8 are supported, provided %d for node %s", bits, node.name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  if ((blockSize <= 0) || (K <= 0) || (N <= 0))
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : MatMulNBits layer : Invalid K/N/block_size (%d/%d/%d) for node %s", K, N, blockSize, node.name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  params.K             = K;
  params.N             = N;
  params.blockSize     = blockSize;
  params.bits          = bits;
  params.accuracyLevel = accuracyLevel;
  params.kBlocks       = (K + blockSize - 1) / blockSize;
  params.zeroPointType = 0;
  params.isBias        = 0;
  params.weights       = 0;
  params.scales        = 0;
  params.zeroPoints    = 0;
  params.bias          = 0;

  /* ---- B: bit-packed uint8 weights (input 1) : copy raw bytes ---- */
  {
    int32_t tStatus = 0;
    TensorProto bT = getInitializerTensor(graph, node.input(1), index, tStatus);
    if ((tStatus != 0) || (bT.raw_data().size() == 0))
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : MatMulNBits layer : Unable to read packed weights (input 1) for node %s", node.name().c_str());
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    int32_t nBytes = (int32_t)bT.raw_data().size();
    pcParams.packedWeights.ptr = my_malloc(nBytes);
    if (pcParams.packedWeights.ptr == NULL)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : MatMulNBits layer : Unable to allocate packed weights buffer for node %s", node.name().c_str());
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    memcpy(pcParams.packedWeights.ptr, bT.raw_data().c_str(), nBytes);
    pcParams.packedWeights.bufSize = nBytes;
  }

  /* ---- scales: float per-block (input 2) ---- */
  status = copyFloatConst(graph, index, 2, pcParams.scales, INPUT_REQUIRED);
  if (status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : MatMulNBits layer : Unable to read scales (input 2) for node %s", node.name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  /* ---- zero_points (input 3, optional) : packed uint8 OR unpacked float ---- */
  if ((node.input_size() > 3) && (node.input(3).size() > 0))
  {
    int32_t tStatus = 0;
    TensorProto zT = getInitializerTensor(graph, node.input(3), index, tStatus);
    if (tStatus == 0)
    {
      if (zT.data_type() == TensorProto_DataType_UINT8)
      {
        if (zT.raw_data().size() == 0)
        {
          TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : MatMulNBits layer : Empty packed zero_points for node %s", node.name().c_str());
          return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
        int32_t nBytes = (int32_t)zT.raw_data().size();
        pcParams.zeroPoints.ptr = my_malloc(nBytes);
        if (pcParams.zeroPoints.ptr == NULL)
        {
          TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : MatMulNBits layer : Unable to allocate zero_points buffer for node %s", node.name().c_str());
          return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
        memcpy(pcParams.zeroPoints.ptr, zT.raw_data().c_str(), nBytes);
        pcParams.zeroPoints.bufSize = nBytes;
        params.zeroPointType = 1;
      }
      else if (zT.data_type() == TensorProto_DataType_FLOAT)
      {
        status = copyFloatConst(graph, index, 3, pcParams.zeroPoints, INPUT_NOT_REQUIRED);
        if (status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
        {
          TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : MatMulNBits layer : Unable to read float zero_points for node %s", node.name().c_str());
          return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
        params.zeroPointType = 2;
      }
      else
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : MatMulNBits layer : Unsupported zero_points data type %d for node %s", zT.data_type(), node.name().c_str());
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }
  }

  /* ---- bias (input 5, optional) : float [N] ---- (input 4 = g_idx, deprecated) */
  if ((node.input_size() > 5) && (node.input(5).size() > 0))
  {
    status = copyFloatConst(graph, index, 5, pcParams.bias, INPUT_NOT_REQUIRED);
    if (status != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
    {
      params.isBias = 1;
    }
  }

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}
