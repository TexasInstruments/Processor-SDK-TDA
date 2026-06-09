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

const vector<TidlConstraint> tidlConstraintPooling = {
    TIDL_CSTR(
        "Input should be variable and not constant/initializer",
        "Input should be variable",
        "Input should be variable",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;

            if(md.numVarInputs != 1)
            {
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Number of non-singleton variable input dimensions must be <= 4",
        "Number of non-singleton variable input dimensions must be <= 4",
        "Number of non-singleton variable input dimensions must be <= 4",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            if(gParams.modelType != TIDL_IMPORT_MODEL_FORMAT_ONNX)
            {
                int32_t numDims = tidlGetNonSingletonNumDims(layer->allowlistingMetaData.varTensorsDims[0]);
                if(numDims > 4)
                {
                    oss << "Maximum number of input dimension supported is 4, found " << numDims << " input dimensions";
                    logs = oss.str();
                    return false;
                }
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "GlobalAveragePool does not support plane size > 1024",
        "GlobalAveragePool does not support plane size > 1024",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            const sTIDL_PoolingParams_t& params = layer->layerParams.poolParams;
            sTIDL_allowlistingMetaData md  = layer->allowlistingMetaData;
            int32_t shapeInferencestatus = tidlCheckShapeInferenceforOnnx(md);

            if(params.kernelH == 0 && params.kernelW == 0)
            {
                if (shapeInferencestatus != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
                {
                    int32_t size = md.varTensorsDims[0].size(), planeSize = 1;
                    /**
                     * Following the generic pattern for global average pool input tensor,
                     * N X C x D1 x D2 x ....
                     * Average is calculated over D1, D2, .. and so on
                     * For image case -> N x C x H x W, average is calculated over H & W
                     * You skip batch & channel dimensions (0,1) & calculate planeSize from dimIdx 2 onwards
                     */

                    if (size >= 3)
                    {
                        for (int32_t i = 2; i < size; i++)
                        {
                        planeSize *= md.varTensorsDims[0][i];
                        }
                    }

                    if (planeSize > 1024)
                    {
                        return false;
                    }
                }
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Only 1D and 2D GlobalAveragePool is supported",
        "Only 1D and 2D GlobalAveragePool is supported",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            // In tidl_optimizeGlobalAveragePoolingLayers function inside TIDL_optimizeNet
            // only 1D pooling and 2D pooling (default) is supported. The case for more dimensions
            // is not yet implemented.
            ostringstream oss;
            if((layer->layerParams.poolParams.kernelH == 0) && (layer->layerParams.poolParams.kernelW == 0))
            { 
                if (layer->layerParams.poolParams.avgDims > 2)
                {
                    oss << "Only 1D and 2D GlobalAveragePool is supported";
                    logs = oss.str();
                    return false;
                }
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Pooling functionality is validated for 3x3,2x2 and 1x1 kernels with both stride 1 and stride 2 (horizontally and vertically), with the exception of 2x2 kernel with stride 1",
        "Pooling functionality is validated for 3x3,2x2 and 1x1 kernels with both stride 1 and stride 2 (horizontally and vertically), with the exception of 2x2 kernel with stride 1",
        "Pooling functionality is validated for 3x3,2x2 and 1x1 kernels with both stride 1 and stride 2 (horizontally and vertically), with the exception of 2x2 kernel with stride 1",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            const sTIDL_PoolingParams_t& params = layer->layerParams.poolParams;
            sTIDL_allowlistingMetaData md  = layer->allowlistingMetaData;
            int32_t shapeInferencestatus = tidlCheckShapeInferenceforOnnx(md);
            bool validated = false;

            /*Global Average case: (Note: A given parameter should ideally be checked in a single constraint, 
              however we are using the same fields for global average pool which is why this is duplicated below))*/
            if(params.kernelH == 0 && params.kernelW == 0)
            {
                validated = true; // Global pooling
                if (shapeInferencestatus != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
                {
                    int32_t size = md.varTensorsDims[0].size(), planeSize = 1;
                    /**
                     * Following the generic pattern for global average pool input tensor,
                     * N X C x D1 x D2 x ....
                     * Average is calculated over D1, D2, .. and so on
                     * For image case -> N x C x H x W, average is calculated over H & W
                     * You skip batch & channel dimensions (0,1) & calculate planeSize from dimIdx 2 onwards
                     */

                    if (size >= 3)
                    {
                        for (int32_t i = 2; i < size; i++)
                        {
                        planeSize *= md.varTensorsDims[0][i];
                        }
                    }

                    if (planeSize > 1024)
                    {
                        validated = false;
                    }
                }
            }

            /* configurations supported by both maxpool and avgpool layers*/
            if(params.kernelH == 2 && params.kernelW == 2 && params.strideH == 2 && params.strideW == 2)
            {
                validated = true;
            }

            if(params.kernelH == 3 && params.kernelW == 3 && (params.strideH == 1 && params.strideW == 1))
            {
                validated = true;
            }

            if(params.kernelH == 3 && params.kernelW == 3 && params.strideH == 2 && params.strideW == 2)
            {
                validated = true;
            }
            
            /* configurations only supported by maxpool layer*/
            if(params.poolingType == TIDL_MaxPooling){

                if(params.kernelH == 1 && params.kernelW == 1)
                {
                    if((params.strideH == 2 && params.strideW == 1) || (params.strideH == 1 && params.strideW == 2)
                    || (params.strideH == 3 && params.strideW == 1) || (params.strideH == 1 && params.strideW == 3))
                    {
                    validated = true;
                    }
                }

                if(params.kernelH == 1 && params.kernelW == 1 &&
                ((params.strideH == 2 && params.strideW == 2) || (params.strideH == 1 && params.strideW == 1 )))
                {
                    validated = true;
                }

                if(params.kernelH == 1 && params.kernelW == 2 && params.strideH == 1 && params.strideW == 2)
                {
                    validated = true;
                }
                else if(params.kernelH == 1 && params.kernelW == 3 && params.strideH == 1 && params.strideW == 3)
                {
                    validated = true;
                }
                else if(params.kernelH == 2 && params.kernelW == 1 && params.strideH == 2 && params.strideW == 1)
                {
                    validated = true;
                }
                else if(params.kernelH == 3 && params.kernelW == 1 && params.strideH == 3 && params.strideW == 1)
                {
                    validated = true;
                }
            }

            if (!validated)
            {
                oss << "Kernel size (" << params.kernelH << "x"  << params.kernelW << ") with stride (" << params.strideH << "x" << params.strideW << ") is not supported";
                logs = oss.str();
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
            const sTIDL_PoolingParams_t& params = layer->layerParams.poolParams;

            if(params.kernelH == 4 && params.kernelW == 4)
            {
                TIDL_GLOBAL_REPORT_WARNING("4x4 pooling in [%s] is not efficient on MMA", (char*)layer->name);
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Only default dilations values (1,1) are supported",
        "Only default dilations values (1,1) are supported",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            int32_t dilationH = layer->layerPCParams.poolParams.dilationH;
            int32_t dilationW = layer->layerPCParams.poolParams.dilationW;
            if(dilationH != 1 || dilationW != 1)
            {
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Number of outputs should be 1",
        "Number of outputs should be 1",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            sTIDL_allowlistingMetaData md  = layer->allowlistingMetaData;
            if(md.numOutputs > 1)
            {
                return false;
            }
            return true;
        }
    )
};