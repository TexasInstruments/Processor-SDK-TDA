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

#include "tidl_constraint.h"

const vector<TidlConstraint> tidlConstraintRNN =
{
    TIDL_CSTR(
        "RNN is not suppported as an individual operator",
        "RNN is not suppported as an individual operator",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            return false;
        }
    ),
    TIDL_CSTR(
        "Number of non-singleton variable input dimensions must be <= 6",
        "Number of non-singleton variable input dimensions must be <= 6",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            int32_t numDims = tidlGetNonSingletonNumDims(layer->allowlistingMetaData.varTensorsDims[0]);
            if(numDims > 6)
            {
                oss << "Maximum number of input dimension supported is 6, found " << numDims << " input dimensions";
                logs = oss.str();
                return false;
            }

            return true;
        }
    ),
    TIDL_CSTR(
        "B and sequence_lens input should be constant inputs",
        "B and sequence_lens input should be constant inputs",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;

            std::vector<int32_t> constIndices = {3, 4};
            for(int32_t constIdx : constIndices)
            {
                if(std::find(md.varTensorIndices.begin(), md.varTensorIndices.end(), constIdx) != md.varTensorIndices.end())
                {
                    return false;
                }
            }

            return true;
        }
    ),
    TIDL_CSTR(
        "Only Tanh activation is supported",
        "Only Tanh activation is supported",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            const sTIDL_RNNParams_t &rnnParams = layer->layerParams.rnnParams;
            int32_t num_directions = 1;
            if(rnnParams.direction == TIDL_RNNBidirectional)
            {
                num_directions = 2;
            }
            int32_t availableActivations = num_directions;

            for(int32_t activationIdx = 0; activationIdx < availableActivations; activationIdx++)
            {
                if(rnnParams.activations[activationIdx] != TIDL_Tanh)
                {
                    return false;
                }
            }

            return true;
        }
    ),
    TIDL_CSTR(
        "Only clip > 0 is supported",
        "Only clip > 0 is supported",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            const sTIDL_RNNParams_t &rnnParams = layer->layerParams.rnnParams;

            if(rnnParams.isClipSet == 1 && rnnParams.clip <= 0)
            {
                return false;
            }

            return true;
        }
    ),
    TIDL_CSTR(
        "Only default direction = 'forward' is supported",
        "Only default direction = 'forward' is supported",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            const sTIDL_RNNParams_t &rnnParams = layer->layerParams.rnnParams;

            if(rnnParams.direction != TIDL_RNNForward)
            {
                return false;
            }

            return true;
        }
    ),
    TIDL_CSTR(
        "Only default layout = 0 is supported",
        "Only default layout = 0 is supported",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            const sTIDL_RNNParams_t &rnnParams = layer->layerParams.rnnParams;

            if(rnnParams.layout != 0)
            {
                return false;
            }

            return true;
        }
    ),
    TIDL_CSTR(
        "At least one output should be present",
        "At least one output should be present",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;

            bool outputPresent = false;
            for(int32_t outIdx = 0; outIdx < md.numOutputs; outIdx++)
            {
                if(md.outputTensorDims[outIdx].size() > 0)
                {
                    outputPresent = true;
                    break;
                }
            }

            return outputPresent;
        }
    ),
};