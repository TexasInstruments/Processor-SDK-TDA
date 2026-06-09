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

#include "tidl_constraint.h"

const vector<TidlConstraint> tidlConstraintBatchNorm =
{
    TIDL_CSTR(
        "Number of variable input dimensions must be <= 6",
        "Number of variable input dimensions must be <= 6",
        "Number of variable input dimensions must be <= 6",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            if(md.varTensorsDims[0].size() > 6)
            {
                return false;
            }

            return true;
        }
    ),
    TIDL_CSTR(
        "Training mode in ONNX is not supported by TIDL_RT",
        "training_mode = 1 is not supported",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            if(layer->layerPCParams.batchNormParams.trainingMode == 1)
            {
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "",
        "",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            if(gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX)
            {
                float momentum = layer->layerPCParams.batchNormParams.momentum;
                if(abs(momentum - 0.9) >= 0.000001)
                {
                    TIDL_GLOBAL_REPORT_WARNING("TIDL does not use 'momentum' attribute in BatchNorm layer")
                }
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Scale, Bias, input_mean and input_var should be constant 1-D tensor of size same as input channel dimension",
        "Scale, Bias, input_mean and input_var should be constant 1-D tensor of size same as input channel dimension",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            int32_t shapeInferencestatus = tidlCheckShapeInferenceforOnnx(md);

            /* Only for BatchNorm operator, skipping for layers that got converted to BN */
            if(shapeInferencestatus != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL &&
              (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX || gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX_RT || gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TVM_RELAY) &&
               layer->isConverted == 0)
            {
                /* Scale, Bias, input_mean and input_var should be constant input */
                if(md.numConstInputs == 4)
                {
                    vector<int32_t> scaleDims = md.constTensorsDims[0];
                    vector<int32_t> biasDims = md.constTensorsDims[1];
                    vector<int32_t> inputMeanDims = md.constTensorsDims[2];
                    vector<int32_t> inputVarDims = md.constTensorsDims[3];

                    /* All const inputs (Scale, Bias, input_mean and input_var) should be 1D tensor and size same:
                    * Number of channels (C) in image input: [N, C, H, W]
                    * C in non-image input: [N, C, D1, D2 ... Dn]
                    */
                    int32_t numChannels;
                    if(md.varTensorsDims[0].size() > 1)
                    {
                        numChannels = md.varTensorsDims[0][1];
                    }
                    else
                    {
                        numChannels = 1;
                    }

                    if(scaleDims.size() != 1 || scaleDims[0] != numChannels)
                    {
                        return false;
                    }
                    if(biasDims.size() != 1 || biasDims[0] != numChannels)
                    {
                        return false;
                    }
                    if(inputMeanDims.size() != 1 || inputMeanDims[0] != numChannels)
                    {
                        return false;
                    }
                    if(inputVarDims.size() != 1 || inputVarDims[0] != numChannels)
                    {
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            }

            return true;
        }
    ),
};