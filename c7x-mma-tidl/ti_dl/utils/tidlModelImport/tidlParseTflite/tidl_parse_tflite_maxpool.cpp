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



#include "tidl_parse_tflite.h"
using namespace std;

template<> int32_t TidlParseTflite:: parse<kTfLiteBuiltinMaxPool2d> ()
{
  int32_t status = 0;

  int32_t padType;

  sTIDL_PoolingParams_t &poolParams = layer.layerParams.poolParams;
  sTIDL_PoolPCParams_t &poolPCParams = layer.layerPCParams.poolParams;
  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;

  layer.layerType = TIDL_PoolingLayer;
  poolParams.poolingType = TIDL_MaxPooling;

  poolParams.strideW = 1;
  poolParams.strideH = 1;
  poolParams.padW = 0;
  poolParams.padH = 0;
  poolParams.padT = 0;
  poolParams.padB = 0;
  poolParams.padL = 0;
  poolParams.padR = 0;
  poolPCParams.originalPadT = 0;
  poolPCParams.originalPadB = 0;
  poolPCParams.originalPadL = 0;
  poolPCParams.originalPadR = 0;


  poolPCParams.dilationH = 1;
  poolPCParams.dilationW = 1;

  const TfLitePoolParams* Pool2DParams =
          reinterpret_cast<const TfLitePoolParams*>(node->builtin_data);
  poolParams.strideW = Pool2DParams->stride_width;
  poolParams.strideH = Pool2DParams->stride_height;
  poolParams.kernelW = Pool2DParams->filter_width;
  poolParams.kernelH = Pool2DParams->filter_height;

  padType = Pool2DParams->padding;
  if (padType == kTfLitePaddingSame)
  {
    int32_t inputDims = md.varTensorsDims[0].size();
    int32_t inputWidth = md.varTensorsDims[0][inputDims - 1];
    int32_t inputHeight = md.varTensorsDims[0][inputDims - 2];
    /*
     * For SAME padding, the output dimensions are calculated using ceil
     * OutputHeight = ceil(InputHeight / StrideHeight)
     * OutputWidth  = ceil(InputWidth / StrideWidth)
     * The padding is then calculated to ensure that the output dimensions
     * match the expected output dimensions. The padding is distributed equally
     * on both sides of the input. If the padding is odd, the extra padding is
     * added to the bottom and right sides of the input.
     */
    int32_t outputWidth = (inputWidth + poolParams.strideW - 1) / poolParams.strideW;
    int32_t outputHeight = (inputHeight + poolParams.strideH - 1) / poolParams.strideH;
    int32_t totalPadWidth = std::max(0, ((outputWidth - 1) * poolParams.strideW + poolParams.kernelW - inputWidth));
    int32_t totalPadHeight = std::max(0, ((outputHeight - 1) * poolParams.strideH + poolParams.kernelH - inputHeight));
    poolParams.padT = totalPadHeight / 2;
    poolParams.padB = totalPadHeight - poolParams.padT;
    poolParams.padL = totalPadWidth / 2;
    poolParams.padR = totalPadWidth - poolParams.padL;
    poolPCParams.originalPadT = poolParams.padT;
    poolPCParams.originalPadB = poolParams.padB;
    poolPCParams.originalPadL = poolParams.padL;
    poolPCParams.originalPadR = poolParams.padR;
    poolParams.padW = std::max(poolParams.padL, poolParams.padR);
    poolParams.padH = std::max(poolParams.padT, poolParams.padB);
  }

  return 0;
}

