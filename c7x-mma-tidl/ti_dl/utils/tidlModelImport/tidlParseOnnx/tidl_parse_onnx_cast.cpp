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

template<> int32_t TidlParseOnnx:: parse<OnnxStr("Cast")> ()
{
    int castTo, saturate;
    NodeProto node = graph.node(index);
    layer.layerType = TIDL_CastLayer;
    int status = getIntAttr(node, "to", &castTo, 0);
    /* TODO: add conditions for which data conversion we allow */
    layer.layerPCParams.castParams.castTo = getTIDLDataTypeFromOnnxDataType(castTo);
    layer.layerPCParams.castParams.terminal = TIDL_CastNotTerminal;

    char roundMode[50];
    status = getStringAttr(node, "round_mode", roundMode, 0);
    if (status == TIDL_ALLOWLISTING_LAYER_CHECK_PASSED)
    {
      if(strcmp(roundMode, "up") == 0)
      {
        layer.layerPCParams.castParams.roundMode = TIDL_CastRoundUp;
      }
      else if(strcmp(roundMode, "nearest") == 0)
      {
        layer.layerPCParams.castParams.roundMode = TIDL_CastRoundNearest;
      }
      else if(strcmp(roundMode, "down") == 0)
      {
        layer.layerPCParams.castParams.roundMode = TIDL_CastRoundDown;
      }
      else
      {
        /* Default to round up if the value is not recognized */
        layer.layerPCParams.castParams.roundMode = TIDL_CastRoundUp;
      }
    }
    else
    {
      /* Default to round up if the attribute is not present */
      layer.layerPCParams.castParams.roundMode = TIDL_CastRoundUp;
    }

    status = getIntAttr(node, "saturate", &saturate, 0);
    if (status == TIDL_ALLOWLISTING_LAYER_CHECK_PASSED)
    {
      layer.layerPCParams.castParams.saturate = saturate;
    }
    else
    {
      /* Default to saturate if the attribute is not present */
      layer.layerPCParams.castParams.saturate = 1;
    }
    
    // Check if cast is connected to graph input
    for (int i = 0; i < graph.input_size(); i++)
    {
      if(strcmp(graph.node(index).input(0).c_str(), graph.input(i).name().c_str()) == 0)
      {
        layer.layerPCParams.castParams.terminal |= TIDL_CastInputTerminal;
      }
    }

    // Check if cast is connected to graph output
    for (int i = 0; i < graph.output_size(); i++)
    {
        if(strcmp(graph.node(index).output(0).c_str(), graph.output(i).name().c_str()) == 0)
        {
            layer.layerPCParams.castParams.terminal |= TIDL_CastOutputTerminal;
        }
    }
    
    return 0;
}

