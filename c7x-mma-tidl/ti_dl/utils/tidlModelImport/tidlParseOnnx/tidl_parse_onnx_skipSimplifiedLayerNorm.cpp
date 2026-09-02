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
 * have made, use, import, offer to sell and sell ("Utilize") this software subject to
 * the terms herein.  With respect to the foregoing patent license, such license is granted
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
 *
 */



#include "tidl_parse_onnx.h"
using namespace std;
using namespace onnx;

template<> int32_t TidlParseOnnx:: parse<OnnxStr("SkipSimplifiedLayerNormalization")> ()
{
    int32_t status = 0;
    float32_tidl epsilon = 1e-5;
    int8_t hasSumOut = 0;

    layer.layerType = TIDL_SkipSimplifiedLayerNormLayer;
    layer.numInBufs = 2;  // input[0]=data (variable), input[1]=skip (variable)

    NodeProto node = graph.node(index);

    // Parse attributes
    getFloatAttr(node, "epsilon", &epsilon, 0);
    if(epsilon == (float32_tidl)0)
    {
        epsilon = epsilon + 1e-5;
    }

    /* Copy gamma (scale) weights - input index 2 */
    status = copyFloatConst(graph, index, 2, layer.weights, INPUT_REQUIRED);

    if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
    {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot read initializer tensor SkipSimplifiedLayerNormalization Scale Input");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }

    /* Optional bias input - input index 3 */
    if (graph.node(index).input_size() > 3 && strcmp(graph.node(index).input(3).c_str(), "") != 0)
    {
        TIDL_IMPORT_CHECK_AND_RETURN(copyFloatConst(graph, index, 3, layer.bias, INPUT_REQUIRED), "");
    }
    else
    {
        layer.bias.ptr = NULL;
        layer.bias.bufSize = 0;
    }

    // Determine if sum output is present (output index 3)
    for (int32_t i = 0; i < graph.node(index).output_size(); i++)
    {
        if (i == 0)
        {
            // Main output, allow this.
        }
        else if (i == 3)
        {
            // Optional input_skip_bias_sum output, allow this.
            if (strcmp(graph.node(index).output(i).c_str(), "") != 0)
            {
                hasSumOut = 1;
            }
        }
        else
        {
            if (strcmp(graph.node(index).output(i).c_str(),"") != 0)
            {
                TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : SkipSimplifiedLayerNormalization layer : Output %s at index %d is not allowed", graph.node(index).output(i).c_str(), i);
                return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
            }
        }
    }

    layer.layerPCParams.skipSimplifiedLayerNormParams.epsilon = epsilon;
    layer.layerPCParams.skipSimplifiedLayerNormParams.hasSumOut = hasSumOut;

    return status;
}