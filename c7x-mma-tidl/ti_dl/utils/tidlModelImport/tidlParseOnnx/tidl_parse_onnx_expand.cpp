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

template<> int32_t TidlParseOnnx:: parse<OnnxStr("Expand")> ()
{
  int32_t status = 0;
  layer.layerType = TIDL_ExpandLayer;
  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;

  layer.numInBufs = 1;
  for (int32_t i = 0; i < (int32_t)md.constTensorIndices.size(); i++)
  {
    int constTensorIdx = md.constTensorIndices[i];
    if (constTensorIdx == 1)
    {
      /* Shape input (input 1) is a constant — absorb into expandParams */
      sBuffer_t buffer;
      status = copyFloatConst(graph, index, 1, buffer, INPUT_REQUIRED);
      if (status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot read initializer tensor : Only float, int32 and int64 tensor is supported");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }

      for (int j = 0; j < TIDL_DIM_MAX; j++)
      {
        layer.layerPCParams.expandParams.shape[j] = 1;
      }

      int offset = std::max(0, ((int32_t)TIDL_DIM_MAX - buffer.bufSize));
      for (int j = 0; j < buffer.bufSize; j++)
      {
        layer.layerPCParams.expandParams.shape[j + offset] = (int32_t)((int64_t*)buffer.ptr)[j];
      }

      if (buffer.ptr)
      {
        free(buffer.ptr);
        buffer.ptr = NULL;
        buffer.bufSize = 0;
      }
    }
    else if (constTensorIdx == 0)
    {
      /* Data input (input 0) is a constant initializer — store in layer.weights
       * so tidl_addConstDataLayers can create the appropriate ConstDataLayer */
      status = copyFloatConst(graph, index, 0, layer.weights, INPUT_REQUIRED);
      if (status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Expand layer : Unable to read constant data input");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }
  }
  return 0;
}

