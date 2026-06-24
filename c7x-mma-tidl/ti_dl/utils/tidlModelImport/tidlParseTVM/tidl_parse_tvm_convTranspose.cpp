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

#include <tvm/relay/expr.h>
#include <tvm/relay/attrs/nn.h>
#include <tvm/relay/function.h>

template <>
int32_t TidlParseTVM::parse<OpNameStr("tidl.conv_transpose")>()
{
  int32_t status;
  int32_t pads[4];
  int32_t output_shape[2];
  int32_t padL = -1, padR = -1, padT = -1, padB = -1;
  sTIDL_ConvParams_t &convParams = layer.layerParams.convParams;

  layer.layerType = TIDL_Deconv2DLayer;

  layer.strideOffsetMethod = TIDL_StrideOffsetTopLeft;

  layer.outData[0].elementType = tidl_getElementType(1);

  const auto *fn_node = call->op.as<tvm::relay::FunctionNode>();
  auto fn = GetRef<tvm::relay::Function>(fn_node);
  const auto *param = fn->attrs.as<DictAttrsNode>();

  auto weight = call->args[1];
  Array<PrimExpr> weight_shape =  weight->checked_type().as<TensorTypeNode>()->shape;

  layer.layerPCParams.convParams.tensorHeight = weight_shape[2].as<IntImmNode>()->value;
  layer.layerPCParams.convParams.tensorWidth = weight_shape[3].as<IntImmNode>()->value;

  convParams.numGroups = 1;
  convParams.dilationW = 1;
  convParams.dilationH = 1;
  convParams.strideW = 1;
  convParams.strideH = 1;
  convParams.padW = 0;
  convParams.padH = 0;
  convParams.enableBias = 0;
  convParams.enablePooling = 0;

  if (param->dict.count("strides"))
  {
    Array<PrimExpr> strides = Downcast<Array<PrimExpr>>(param->dict.at("strides"));
    convParams.strideH = Downcast<IntImm>(strides[0])->value;
    convParams.strideW = Downcast<IntImm>(strides[1])->value;
  }

  if (param->dict.count("dilation"))
  {
    Array<PrimExpr> dilations = Downcast<Array<PrimExpr>>(param->dict.at("dilation"));
    convParams.dilationH = Downcast<IntImm>(dilations[0])->value;
    convParams.dilationW = Downcast<IntImm>(dilations[1])->value;
  }

  if (param->dict.count("groups"))
  {
    convParams.numGroups = Downcast<Integer>(param->dict.at("groups"))->value;
  }

  if (param->dict.count("kernel_size"))
  {
    Array<PrimExpr> kshape = Downcast<Array<PrimExpr>>(param->dict.at("kernel_size"));
    convParams.kernelH = Downcast<IntImm>(kshape[0])->value;
    convParams.kernelW = Downcast<IntImm>(kshape[1])->value;
  }

  if (param->dict.count("output_padding"))
  {
    Array<PrimExpr> output_padding = Downcast<Array<PrimExpr>>(param->dict.at("output_padding"));
    layer.layerPCParams.convParams.outPadH = Downcast<IntImm>(output_padding[0])->value;
    layer.layerPCParams.convParams.outPadW = Downcast<IntImm>(output_padding[1])->value;
  }
  else
  {
    layer.layerPCParams.convParams.outPadH = 0;
    layer.layerPCParams.convParams.outPadW = 0;
  }

  convParams.numInChannels = weight_shape[0].as<IntImmNode>()->value;
  convParams.numOutChannels = weight_shape[1].as<IntImmNode>()->value * convParams.numGroups;

  if (param->dict.count("output_shape"))
  {
    Array<PrimExpr> out_shape = Downcast<Array<PrimExpr>>(param->dict.at("output_shape"));
    output_shape[0] = Downcast<IntImm>(out_shape[0])->value;
    output_shape[1] = Downcast<IntImm>(out_shape[1])->value;


    std::string autoPad = "";
    if (param->dict.count("auto_pad"))
    {
      autoPad = Downcast<String>(param->dict.at("auto_pad"));
    }

    int32_t inputH = layer.allowlistingMetaData.varTensorsDims[0][layer.allowlistingMetaData.varTensorsDims[0].size() - 2];
    int32_t inputW = layer.allowlistingMetaData.varTensorsDims[0][layer.allowlistingMetaData.varTensorsDims[0].size() - 1];

    int32_t total_padH = std::max<int64_t>(0, (inputH - 1) * convParams.strideH + layer.layerPCParams.convParams.outPadH + (convParams.kernelH - 1) * convParams.dilationH + 1 - output_shape[0]);
    int32_t total_padW = std::max<int64_t>(0, (inputW - 1) * convParams.strideW + layer.layerPCParams.convParams.outPadW + (convParams.kernelW - 1) * convParams.dilationW + 1 - output_shape[1]);

    if (autoPad == "SAME_UPPER")
    {
      padT = total_padH / 2;
      padB = total_padH - (total_padH / 2);
      padL = total_padW / 2;
      padR = total_padW - (total_padW / 2);

      layer.strideOffsetMethod = TIDL_StrideOffsetCenter;
    }
    else // for pad_type is NOTSET, SAME_LOWER or VALID
    {
      padT = total_padH - (total_padH / 2);
      padB = total_padH / 2;
      padL = total_padW - (total_padW / 2);
      padR = total_padW / 2;

      layer.strideOffsetMethod = TIDL_StrideOffsetTopLeft;
    }
  }
  else
  {
    std::string autoPad = "";
    if (param->dict.count("auto_pad"))
    {
      autoPad = Downcast<String>(param->dict.at("auto_pad"));
    }

    if (autoPad == "" || autoPad == "NOTSET")
    {
      if (param->dict.count("padding"))
      {
        Array<PrimExpr> padding = Downcast<Array<PrimExpr>>(param->dict.at("padding"));
        if (padding.size() == 1)
        {
          pads[0] = pads[1] = pads[2] = pads[3] = Downcast<IntImm>(padding[0])->value;
        }
        else if (padding.size() == 2)
        {
          pads[0] = pads[2] = Downcast<IntImm>(padding[0])->value;
          pads[1] = pads[3] = Downcast<IntImm>(padding[1])->value;
        }
        else if (padding.size() == 4)
        {
          pads[0] = Downcast<IntImm>(padding[0])->value;
          pads[1] = Downcast<IntImm>(padding[1])->value;
          pads[2] = Downcast<IntImm>(padding[2])->value;
          pads[3] = Downcast<IntImm>(padding[3])->value;
        }

        if ((pads[0] < pads[2]) && (pads[1] < pads[3])) //padL < padR && padT < padB
        {
          layer.strideOffsetMethod = TIDL_StrideOffsetCenter;
        }
        padT = pads[0];
        padL = pads[1];
        padB = pads[2];
        padR = pads[3];
      }
      else
      {
        padL = 0;
        padR = 0;
        padT = 0;
        padB = 0;
      }
    }
    else if ((autoPad == "SAME_UPPER") || (autoPad == "SAME_LOWER"))
    {
      int32_t inputH = layer.allowlistingMetaData.varTensorsDims[0][layer.allowlistingMetaData.varTensorsDims[0].size() - 2];
      int32_t inputW = layer.allowlistingMetaData.varTensorsDims[0][layer.allowlistingMetaData.varTensorsDims[0].size() - 1];

      int32_t total_padH = std::max<int64_t>(0, (inputH - 1) * convParams.strideH + layer.layerPCParams.convParams.outPadH + (convParams.kernelH - 1) * convParams.dilationH + 1 - (inputH * convParams.strideH));
      int32_t total_padW = std::max<int64_t>(0, (inputW - 1) * convParams.strideW + layer.layerPCParams.convParams.outPadW + (convParams.kernelW - 1) * convParams.dilationW + 1 - (inputW * convParams.strideW));

      if (autoPad == "SAME_UPPER")
      {
        padT = total_padH / 2;
        padB = total_padH - (total_padH / 2);
        padL = total_padW / 2;
        padR = total_padW - (total_padW / 2);

        layer.strideOffsetMethod = TIDL_StrideOffsetCenter;
      }
      else //SAME_LOWER
      {
        padT = total_padH - (total_padH / 2);
        padB = total_padH / 2;
        padL = total_padW - (total_padW / 2);
        padR = total_padW / 2;

        layer.strideOffsetMethod = TIDL_StrideOffsetTopLeft;
      }
    }
    else if (autoPad == "VALID")
    {
      pads[3] = 0;
      pads[2] = 0;
      padL = 0;
      padR = 0;
      padT = 0;
      padB = 0;
    }
    else
    {
      TIDL_LOG_ERROR(gDiags.gDiagList, "ConvTranspose: Unsupported auto_pad attribute value '%s'. Supported values are: NOTSET, SAME_UPPER, SAME_LOWER, VALID", autoPad.c_str());
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }
  }

  convParams.padT = padT;
  convParams.padL = padL;
  convParams.padB = padB;
  convParams.padR = padR;

  pads[3] = max(padL, padR);
  pads[2] = max(padT, padB);

  convParams.padW = pads[3];
  convParams.padH = pads[2];

  auto weight_data = weight.as<tvm::relay::ConstantNode>()->data;

  int shape0 = weight_shape[0].as<IntImmNode>()->value;
  int shape1 = weight_shape[1].as<IntImmNode>()->value;
  int shape2 = weight_shape[2].as<IntImmNode>()->value;
  int shape3 = weight_shape[3].as<IntImmNode>()->value;

  layer.weights.bufSize = shape0 * shape1 * shape2 * shape3;
  layer.weights.ptr = NDArrtoFloat(weight_data, layer.weights.bufSize);

  int32_t i0, i1, i2, i3;
  int32_t NoByG = convParams.numOutChannels / convParams.numGroups;
  int32_t tensorSize = convParams.numInChannels * NoByG * convParams.kernelH * convParams.kernelW;
  void **ptr;
  ptr = &layer.weights.ptr;
  float *srcWtPtr = (float *)*ptr;
  float *weightPtr = (float *)my_calloc(tensorSize, sizeof(float));
  if (weightPtr == NULL)
  {
    TIDL_LOG_ERROR(gDiags.gDiagList, "Could not allocate transpose buffer for deconvolution");
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  int32_t linePitch = convParams.kernelH * convParams.kernelW;

  if (convParams.numGroups > 1)
  {
    for (i0 = 0; i0 < (convParams.numInChannels / convParams.numGroups); i0++)
    {
      for (i1 = 0; i1 < NoByG; i1++)
      {
        for (i2 = 0; i2 < (convParams.numGroups); i2++)
        {
          for (i3 = 0; i3 < linePitch; i3++)
          {
            weightPtr[i0 * (linePitch * convParams.numGroups * NoByG) + i1 * (linePitch * convParams.numGroups) + i2 * linePitch + i3] = srcWtPtr[(i0 * NoByG * linePitch * convParams.numGroups) + (i1 * linePitch) + (i2 * linePitch * NoByG) + i3];
          }
        }
      }
    }
  }
  else
  {
    for (i1 = 0; i1 < NoByG; i1++)
    {
      for (i2 = 0; i2 < convParams.numInChannels; i2++)
      {
        for (i3 = 0; i3 < linePitch; i3++)
        {
          weightPtr[i1 * (linePitch * convParams.numInChannels) + i2 * linePitch + i3] = srcWtPtr[i2 * (linePitch * NoByG) + i1 * linePitch + i3];
        }
      }
    }
  }

  free(*ptr);
  *ptr = (void *)weightPtr;

  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;
  if (md.numConstInputs >= 2)
  {
    convParams.enableBias = 1;
    layer.bias = TIDL_extractConstantTensorData(call, 2, md, 1);
  }

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}
