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

#include <tvm/relay/expr.h>
#include <tvm/relay/attrs/nn.h>
#include <tvm/relay/function.h>

/**
 * @brief Parser for composite tidl.conv2d operator
 *
 * This parser handles the composite convolution function created by TVM's ONNX frontend.
 * It mirrors the logic from tidl_parse_onnx_conv.cpp to ensure consistent behavior across
 * ONNX and TVM import paths.
 *
 * The TVM ONNX frontend creates composite functions with:
 * - Composite attribute: "tidl.conv2d" or "qnn.conv2d"
 * - Original ONNX attributes preserved in DictAttrs
 * - auto_pad attribute preserved for proper padding calculation
 */
template <>
int32_t TidlParseTVM::parse<OpNameStr("tidl.conv2d")>()
{
  int32_t status;
  int32_t pads[4] = {0};
  int32_t padL = -1, padR = -1, padT = -1, padB = -1;

  std::string op_name;
  const auto* fn_node = call->op.as<tvm::relay::FunctionNode>();
  auto fn = GetRef<tvm::relay::Function>(fn_node);
  const auto* param = fn->attrs.as<DictAttrsNode>();

  if (param->dict.count("Composite"))
  {
    op_name = Downcast<String>(param->dict.at("Composite"));
  }

  // Handle QNN quantization parameters if present
  int32_t w_zp_len = 1;
  int32_t w_scale_len = 1;
  int32_t *w_zp_ptr = nullptr;  // default value is 0
  float *w_scale_ptr = nullptr; // default value is 1.0f

  sTIDL_ConvParams_t &convParams = layer.layerParams.convParams;
  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;

  layer.layerType = TIDL_ConvolutionLayer;
  layer.strideOffsetMethod = TIDL_StrideOffsetTopLeft;
  layer.outData[0].elementType = tidl_getElementType(1);

  if (op_name == "qnn.conv2d")
  {
    // For QNN conv2d, weight zero point and scale are in args[3] and args[5]
    w_zp_ptr = TIDL_relayGetConstantArr<int32_t>(call->args[3], w_zp_len);
    w_scale_ptr = TIDL_relayGetConstantArr<float>(call->args[5], w_scale_len);
  }

  // Extract weight information to get kernel dimensions
  auto weight = call->args[1];
  auto weight_checked_type = weight->checked_type().as<TensorTypeNode>();
  Array<PrimExpr> weight_shape = weight_checked_type->shape;

  // Get channel and kernel dimensions from weight tensor
  // Weight layout: 2D conv OIHW (Out, In, Height, Width), 3D conv OIDHW (Out, In, Depth, Height, Width)
  // Always read last 2 dimensions for H, W (works for any N-D convolution)
  convParams.numOutChannels = weight_shape[0].as<IntImmNode>()->value;
  convParams.numInChannels = weight_shape[1].as<IntImmNode>()->value;

  int32_t weight_ndim = weight_shape.size();
  convParams.kernelH = weight_shape[weight_ndim - 2].as<IntImmNode>()->value;
  convParams.kernelW = weight_shape[weight_ndim - 1].as<IntImmNode>()->value;

  // Initialize default values (matching ONNX parser)
  convParams.numGroups = 1;
  convParams.dilationW = 1;
  convParams.dilationH = 1;
  convParams.strideW = 1;
  convParams.strideH = 1;
  convParams.padW = 0;
  convParams.padH = 0;
  convParams.enableBias = 0;
  convParams.enablePooling = 0;

  // Determine if this is 3D conv by checking kernel_size first
  int32_t attrSize = 0;
  int32_t attr_offset = 0;  // Offset for 3D attributes (skip depth dimension)

  if (param->dict.count("kernel_size"))
  {
    Array<PrimExpr> kshape = Downcast<Array<PrimExpr>>(param->dict.at("kernel_size"));
    attrSize = kshape.size();

    if (attrSize < 2)
    {
      TIDL_osrtDebugPrint(tidl_relay_debuglevel, "Conv : kernel_shape is missing height axis\n");
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }

    // For 3D conv (kernel_size has >2 elements), skip depth dimension when reading attributes
    if (attrSize > 2)
    {
      attr_offset = attrSize - 2;  // Skip all dimensions except last 2 (H, W)

      // For 3D convolutions, validate that input depth is 1 (pseudo-3D only)
      // TIDL cannot iterate over depth dimension, so we only support depth=1
      // which can be collapsed to 2D
      if (md.varTensorsDims[0].size() > 4)
      {
        int32_t depthDim = md.varTensorsDims[0][2];  // Depth at index 2 for [N, C, D, H, W]
        if (depthDim > 1)
        {
          TIDL_osrtDebugPrint(tidl_relay_debuglevel,
              "Conv : Input tensor with depth dimension > 1 (depth=%d) is not supported. "
              "TIDL convolution does not iterate over depth dimension. Only 2D and pseudo-3D (depth=1) convolutions are supported.\n",
              depthDim);
          return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
        }
      }

      // Note: We trust TVM validation to ensure kernel/stride/dilation are compatible
      // For pseudo-3D (depth=1), kernel/stride/dilation in depth dimension don't matter
      // since there's only one element to process
    }

    // Read kernel height and width (last 2 dimensions)
    // Size validation: ensure kshape has at least attr_offset + 2 elements
    if (kshape.size() >= attr_offset + 2)
    {
      layer.layerPCParams.convParams.tensorHeight = Downcast<IntImm>(kshape[attr_offset])->value;
      layer.layerPCParams.convParams.tensorWidth = Downcast<IntImm>(kshape[attr_offset + 1])->value;
    }
    else
    {
      // Fallback: use kernel dimensions from weight tensor
      layer.layerPCParams.convParams.tensorHeight = convParams.kernelH;
      layer.layerPCParams.convParams.tensorWidth = convParams.kernelW;
    }
  }
  else
  {
    // kernel_size attribute is missing, infer attr_offset from weight dimensions
    if (weight_ndim > 4)
    {
      // For 3D conv (5D weight OIDHW), attributes like strides/dilations have 3 elements [D, H, W]
      // We need to skip the depth dimension, so attr_offset = 1
      attr_offset = weight_ndim - 4;
    }
    layer.layerPCParams.convParams.tensorHeight = convParams.kernelH;
    layer.layerPCParams.convParams.tensorWidth = convParams.kernelW;
  }

  // Extract groups
  if (param->dict.count("groups"))
  {
    convParams.numGroups = Downcast<Integer>(param->dict.at("groups"))->value;
  }

  // Extract dilations (using offset for 3D conv)
  // Note: Size check prevents out-of-bounds access
  // For valid models, dilations size should match kernel_size
  // TODO: Consider adding explicit size validation for strides/dilations vs kernel_size
  if (param->dict.count("dilation"))
  {
    Array<PrimExpr> dilations = Downcast<Array<PrimExpr>>(param->dict.at("dilation"));
    if (dilations.size() >= attr_offset + 2)
    {
      convParams.dilationH = Downcast<IntImm>(dilations[attr_offset])->value;
      convParams.dilationW = Downcast<IntImm>(dilations[attr_offset + 1])->value;
    }
  }

  // Extract strides (using offset for 3D conv)
  if (param->dict.count("strides"))
  {
    Array<PrimExpr> strides = Downcast<Array<PrimExpr>>(param->dict.at("strides"));
    if (strides.size() >= attr_offset + 2)
    {
      convParams.strideH = Downcast<IntImm>(strides[attr_offset])->value;
      convParams.strideW = Downcast<IntImm>(strides[attr_offset + 1])->value;
    }
  }

  // Handle auto_pad attribute (matching ONNX parser exactly)
  std::string autoPad = "NOTSET";
  if (param->dict.count("auto_pad"))
  {
    autoPad = Downcast<String>(param->dict.at("auto_pad"));
  }

  // Extract padding array
  Array<PrimExpr> padding_array;
  if (param->dict.count("padding"))
  {
    padding_array = Downcast<Array<PrimExpr>>(param->dict.at("padding"));
  }

  if (autoPad == "NOTSET" || autoPad == "")
  {
    // Use explicit padding values
    // ONNX format: [pad_top, pad_left, pad_bottom, pad_right]
    // TVM format can be: [pad_h, pad_w] or [pad_top, pad_left, pad_bottom, pad_right]

    if (padding_array.size() >= 4)
    {
      pads[0] = Downcast<IntImm>(padding_array[0])->value; // top
      pads[1] = Downcast<IntImm>(padding_array[1])->value; // left
      pads[2] = Downcast<IntImm>(padding_array[2])->value; // bottom
      pads[3] = Downcast<IntImm>(padding_array[3])->value; // right
    }
    else if (padding_array.size() == 2)
    {
      pads[0] = pads[2] = Downcast<IntImm>(padding_array[0])->value;
      pads[1] = pads[3] = Downcast<IntImm>(padding_array[1])->value;
    }
    else if (padding_array.size() == 1)
    {
      pads[0] = pads[1] = pads[2] = pads[3] = Downcast<IntImm>(padding_array[0])->value;
    }

    // Check for asymmetric padding (matching ONNX logic)
    if ((pads[0] < pads[2]) && (pads[1] < pads[3])) // padT < padB && padL < padR
    {
      layer.strideOffsetMethod = TIDL_StrideOffsetCenter;
    }

    pads[2] = max(pads[0], pads[2]);
    pads[3] = max(pads[1], pads[3]);
    padL = pads[1];
    padR = pads[3];
    padT = pads[0];
    padB = pads[2];
  }
  else if ((autoPad == "SAME_UPPER") || (autoPad == "SAME_LOWER"))
  {
    // For SAME padding, compute padding based on input shape
    // This logic matches the ONNX parser exactly

    if ((md.varTensorsDims[0].size() >= 2))
    {
      int32_t heightIdx = md.varTensorsDims[0].size() - 2;
      int32_t widthIdx = md.varTensorsDims[0].size() - 1;

      // Get input dimensions
      int32_t inputH = md.varTensorsDims[0][heightIdx];
      int32_t inputW = md.varTensorsDims[0][widthIdx];

      // Calculate total padding needed (matching TVM's logic in ONNX parser comments)
      int32_t totalPadH = max(inputH % convParams.strideH ?
                              convParams.dilationH * (convParams.kernelH - 1) + 1 - inputH % convParams.strideH :
                              convParams.dilationH * (convParams.kernelH - 1) + 1 - convParams.strideH, 0);
      int32_t totalPadW = max(inputW % convParams.strideW ?
                              convParams.dilationW * (convParams.kernelW - 1) + 1 - inputW % convParams.strideW :
                              convParams.dilationW * (convParams.kernelW - 1) + 1 - convParams.strideW, 0);

      if (autoPad == "SAME_UPPER")
      {
        // SAME_UPPER: extra padding goes to right/bottom
        padL = totalPadW / 2;
        padT = totalPadH / 2;
      }
      else  // SAME_LOWER
      {
        // SAME_LOWER: extra padding goes to left/top
        padL = (totalPadW + 1) / 2;
        padT = (totalPadH + 1) / 2;
      }

      padB = max(totalPadH - padT, 0);
      padR = max(totalPadW - padL, 0);

      if ((padL < padR) && (padT < padB))
      {
        layer.strideOffsetMethod = TIDL_StrideOffsetCenter;
      }
    }
    else
    {
      // Fallback when shape inference is not available
      padL = ((convParams.kernelW - 1) * convParams.dilationW) / 2;
      padT = ((convParams.kernelH - 1) * convParams.dilationH) / 2;
      padR = padL;
      padB = padT;

      if (autoPad == "SAME_UPPER")
      {
        layer.strideOffsetMethod = TIDL_StrideOffsetCenter;
      }
      else  // SAME_LOWER
      {
        layer.strideOffsetMethod = TIDL_StrideOffsetTopLeft;
      }
    }

    pads[3] = max(padR, padL);
    pads[2] = max(padB, padT);
  }
  else if (autoPad == "VALID")
  {
    // VALID padding means no padding
    pads[3] = 0;
    pads[2] = 0;
    padL = 0;
    padR = 0;
    padT = 0;
    padB = 0;
  }

  // Set padding parameters
  convParams.padW = pads[3];
  convParams.padH = pads[2];
  convParams.padT = padT;
  convParams.padL = padL;
  convParams.padB = padB;
  convParams.padR = padR;

  // Extract and copy weights
  auto weight_data = weight.as<tvm::relay::ConstantNode>()->data;

  // Calculate weight buffer size for all dimensions (2D, 3D, etc.)
  layer.weights.bufSize = 1;
  for (size_t i = 0; i < weight_shape.size(); i++) {
    int dim_size = weight_shape[i].as<IntImmNode>()->value;
    layer.weights.bufSize *= dim_size;
  }

  layer.weights.ptr = NDArrtoFloat(weight_data, layer.weights.bufSize);

  if (layer.weights.ptr == nullptr)
  {
    TIDL_osrtDebugPrint(tidl_relay_debuglevel, "Conv layer : Unable to copy weights\n");
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  int num_o_chs = weight_shape[0].as<IntImmNode>()->value;  // Out channels always at index 0

  // Apply quantization scaling if QNN parameters present
  float *weights = (float *)layer.weights.ptr;
  if (w_scale_ptr != nullptr && w_zp_ptr != nullptr)
  {
    for (int o_ch = 0; o_ch < num_o_chs; o_ch++)
    {
      int32_t zp = (w_zp_len == 1) ? w_zp_ptr[0] : w_zp_ptr[o_ch];
      float scale = (w_scale_len == 1) ? w_scale_ptr[0] : w_scale_ptr[o_ch];
      for (int i = 0; i < layer.weights.bufSize / num_o_chs; i++)
      {
        int id = o_ch * (layer.weights.bufSize / num_o_chs) + i;
        weights[id] = (weights[id] - zp) * scale;
      }
    }
  }

  // Handle bias if present (matching ONNX: if input_size > 2)
  if (md.numConstInputs >= 2)
  {
    convParams.enableBias = 1;
    layer.bias = TIDL_extractConstantTensorData(call, 2, md, 1);

    if (layer.bias.ptr == nullptr)
    {
      TIDL_osrtDebugPrint(tidl_relay_debuglevel, "Conv layer : Unable to copy bias\n");
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }
  }
  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

/**
 * @brief Helper template function to extract constant arrays from Relay expressions
 *
 * This function is used to extract quantization parameters (zero points and scales)
 * from constant tensors in QNN operations.
 *
 * @tparam Tvalue Type of values to extract (int32_t, float, etc.)
 * @param expr Relay expression containing constant tensor
 * @param len Output parameter for array length
 * @return Pointer to extracted array
 */
template <typename Tvalue>
Tvalue *TIDL_relayGetConstantArr(const relay::Expr &expr, int32_t &len)
{
  const DLTensor *dltensor = expr.as<ConstantNode>()->data.operator->();
  len = tvm::runtime::GetDataSize(*dltensor) / sizeof(Tvalue);
  return (Tvalue *)dltensor->data;
}
