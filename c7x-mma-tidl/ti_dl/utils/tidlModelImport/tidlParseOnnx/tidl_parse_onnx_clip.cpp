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

template<> int32_t TidlParseOnnx:: parse<OnnxStr("Clip")> ()
{
  int32_t status = 0;
  layer.layerType = TIDL_ClipLayer;
  layer.clipParams.isClipEnabled = 1;
  NodeProto node = graph.node(index);
  int32_t minStatus = -1, maxStatus = -1;
  int32_t numInputs = node.input_size();
  sBuffer_t buf;

  /* Try to fetch min and max from attributes */
  minStatus = getFloatAttr(node, "min", &layer.clipParams.clipMin, 0);
  maxStatus = getFloatAttr(node, "max", &layer.clipParams.clipMax, 0);

  /* If min not found */
  if (minStatus == -1)
  {
    /* Try to fetch min from 2nd input*/
    if (numInputs > 1)
    {
      status = copyFloatConst(graph, index, 1, buf, INPUT_NOT_REQUIRED);
      if(status != -1)
      {
        float *ptr = (float *)buf.ptr;
        if(*ptr == -std::numeric_limits<float>::infinity())
        {
          *ptr = (float32_tidl)std::numeric_limits<float>::lowest();
        }
        layer.clipParams.clipMin = *ptr;
        my_free(ptr);
      }
      else
      {
        /* Set default value of min */
        layer.clipParams.clipMin = (float32_tidl)std::numeric_limits<float>::lowest();
      }
    }
    /* Set default value of min */
    else
    {
      layer.clipParams.clipMin = (float32_tidl)std::numeric_limits<float>::lowest();
    }
  }
  /* If max not found */
  if (maxStatus == -1)
  {
    /* Try to fetch max from 3rd input*/
    if ( numInputs > 2 )
    {
      status = copyFloatConst(graph, index, 2, buf, INPUT_NOT_REQUIRED);
      if(status != -1)
      {
        float *ptr = (float *)buf.ptr;
        if(*ptr == std::numeric_limits<float>::infinity())
        {
          *ptr = (float32_tidl)std::numeric_limits<float>::max();
        }
        layer.clipParams.clipMax = *ptr;
        my_free(ptr);
      }
      else
      {
        /* Set default value of max */
        layer.clipParams.clipMax = (float32_tidl)std::numeric_limits<float>::max();
      }
    }
    /* Set default value of max */
    else
    {
      layer.clipParams.clipMax = (float32_tidl)std::numeric_limits<float>::max();
    }
  }

  return 0;
}

