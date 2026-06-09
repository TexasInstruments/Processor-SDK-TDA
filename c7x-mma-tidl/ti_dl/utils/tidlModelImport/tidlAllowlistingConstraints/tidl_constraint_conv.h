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

const vector<TidlConstraint> tidlConstraintConv =
{
    TIDL_CSTR(
        "Only one variable input is allowed",
        "Only one variable input is allowed",
        "Only one variable input is allowed",
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
            int32_t numDims = tidlGetNonSingletonNumDims(layer->allowlistingMetaData.varTensorsDims[0]);
            if(numDims > 4)
            {
                oss << "Maximum number of input dimension supported is 4, found " << numDims << " input dimensions";
                logs = oss.str();
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Weight tensor size should match with proto kernel_shape", 
        "Weight tensor dimension must match the kernel_shape",
        "Weight tensor dimension must match the kernel_shape",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            auto convParams = layer->layerParams.convParams;
            if ((layer->layerPCParams.convParams.tensorHeight != convParams.kernelH) || (layer->layerPCParams.convParams.tensorWidth != convParams.kernelW))
            {
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Stride must be the same along both horizontal and vertical dimensions",
        "Stride must be the same along both horizontal and vertical dimensions",
        "Stride must be the same along both horizontal and vertical dimensions",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            auto convParams = layer->layerParams.convParams;
            return (convParams.strideH == convParams.strideW);
        }
    ),
    TIDL_CSTR(
        "Kernel size 3x3 with stride 3 is not supported in AM62A and AM67A",
        "Kernel size 3x3 with stride 3 is not supported in AM62A and AM67A",
        "Kernel size 3x3 with stride 3 is not supported in AM62A and AM67A",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            auto convParams = layer->layerParams.convParams;
            if(convParams.kernelH == 3 && convParams.kernelW == 3 &&
               convParams.strideH == 3 && convParams.strideW == 3 &&
               (gParams.deviceName & ~TIDL_OTF_FLAG_BIT) == TIDL_AM62A)
            {
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "NumGroups greater than In or Out channels not supported for Caffe",
        "",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            auto convParams = layer->layerParams.convParams;
            if(gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_CAFFE)
            {
                if((convParams.numGroups > convParams.numInChannels) ||
                   (convParams.numGroups > convParams.numOutChannels))
                {
                    return false;
                }
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Kernel size greater than 7 with stride 2 is not supported",
        "Kernel size greater than 7 with stride 2 is not supported",
        "Kernel size greater than 7 with stride 2 is not supported",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            auto convParams = layer->layerParams.convParams;
            if(convParams.kernelH > 7  && convParams.kernelW > 7 &&
               convParams.strideH == 2 && convParams.strideW == 2)
            {
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Depthwise (Fully Grouped) convolution is only supported for 1x3s1, 3x3s1, 3x3s2, 5x5s1, 5x5s2, 7x7s1 & 7x7s2 filters",
        "Depthwise (Fully Grouped) convolution is only supported for 1x3s1, 3x3s1, 3x3s2, 5x5s1, 5x5s2, 7x7s1 & 7x7s2 filters",
        "Depthwise (Fully Grouped) convolution is only supported for 1x3s1, 3x3s1, 3x3s2, 5x5s1, 5x5s2, 7x7s1 & 7x7s2 filters",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            auto convParams = layer->layerParams.convParams;
            ostringstream oss;
            /* Depthwise separable convolution */
            if (((convParams.numGroups == convParams.numInChannels) || (convParams.numInChannels == 1)) &&
                (convParams.numGroups == convParams.numOutChannels))
            {
                #define DEPTHWISE_SUPPORTED_PARAMS (4)
                bool validated = false;
                int32_t i;
                int32_t supportedConfigs[][DEPTHWISE_SUPPORTED_PARAMS] =
                {
                    // kernelW, kernelH, strideW, strideH
                    {        1,       1,       1,       1},
                    {        3,       1,       1,       1},
                    {        3,       3,       1,       1},
                    {        3,       3,       2,       2},
                    {        5,       5,       1,       1},
                    {        5,       5,       2,       2},
                    {        7,       7,       1,       1},
                    {        7,       7,       2,       2},
                };
                for(i = 0; i < sizeof(supportedConfigs)/(DEPTHWISE_SUPPORTED_PARAMS * sizeof(int32_t)); i++)
                {
                    if (convParams.kernelW == supportedConfigs[i][0] &&
                        convParams.kernelH == supportedConfigs[i][1] &&
                        convParams.strideW == supportedConfigs[i][2] &&
                        convParams.strideH == supportedConfigs[i][3])
                    {
                        validated = true;
                        break;
                    }
                    else
                    {
                        validated = false;
                    }
                }
                if (!validated)
                {
                    oss << "Depthwise convolution layer with Kernel " << convParams.kernelH << "x" << convParams.kernelW;
                    oss << " and Stride " << convParams.strideH << "x" << convParams.strideW;
                    oss << " is not supported";
                    logs = oss.str();
                    return false;
                }
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "For Depthwise convolution layer, Pad (along width) greater than the MMA width is not supported",
        "",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            auto convParams = layer->layerParams.convParams;
            /* Depthwise separable convolution */
            if (((convParams.numGroups == convParams.numInChannels) || (convParams.numInChannels == 1)) &&
                (convParams.numGroups == convParams.numOutChannels) &&
                (gParams.numFeatureBits < 32))
            {
                /* MMALIB Bug MMALIB-566*/
                int32_t simd_width = -1;
                if((gParams.deviceName & ~TIDL_OTF_FLAG_BIT) == TIDL_AM62A)
                {
                    simd_width = 32U;
                }
                else
                {
                    simd_width = 64U;
                }
                if( gParams.numFeatureBits > 8 )
                {
                    simd_width = simd_width / 2;
                }
                /* This fixes TIDL-2946 */
                if(convParams.padW > simd_width)
                {
                    return false;
                }
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Padding greater than input width is not supported for AM62A and AM67A",
        "Padding greater than input width is not supported for AM62A and AM67A",
        "Padding greater than input width is not supported for AM62A and AM67A",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            auto convParams = layer->layerParams.convParams;
            if (((convParams.numGroups == convParams.numInChannels) || (convParams.numInChannels == 1)) &&
                (convParams.numGroups == convParams.numOutChannels))
            {
            }
            else if((gParams.deviceName & ~TIDL_OTF_FLAG_BIT) == TIDL_AM62A)
            {
                bool isSupportedPad = true;
                if(gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TVM_RELAY)
                {
                    isSupportedPad = true;
                }
                else if((gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE_RT) ||
                        (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX_RT))
                {
                    if(convParams.padW > layer->allowlistingMetaData.varTensorsDims[0][layer->allowlistingMetaData.varTensorsDims[0].size()-1])
                    {
                        isSupportedPad = false;
                    }
                }
                else /* TIDL-RT */
                {
                    /* inData is not populated before allow-listing call in OSRT */
                    if(convParams.padW > layer->inData[0].dimValues[TIDL_DIM_WIDTH])
                    {
                        isSupportedPad = false;
                    }
                }

                if(!isSupportedPad)
                {
                    return false;
                }
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Stride 4 is only supported with Kernel size 11x11",
        "Stride 4 is only supported with Kernel size 11x11",
        "Stride 4 is only supported with Kernel size 11x11",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            auto convParams = layer->layerParams.convParams;
            sTIDL_allowlistingMetaData md  = layer->allowlistingMetaData;
            int32_t shapeInferencestatus = tidlCheckShapeInferenceforOnnx(md); 
            int32_t i;
            bool validated = false;
            int32_t validatedParams[][8] =
            {
                // -1 means dont care
                // kernelH kernelW strideH strideW dilationH dilationW padH padW
                {        1,      1,      1,      1,       -1,       -1,  -1,  -1},
                {        1,      1,      2,      2,       -1,       -1,  -1,  -1},
                {        2,      2,      1,      1,       -1,       -1,  -1,  -1},
                {        2,      2,      2,      2,        1,        1,  -1,  -1},
                {        3,      3,      1,      1,       -1,       -1,  -1,  -1},
                {        3,      3,      2,      2,        1,        1,  -1,  -1},
                {        3,      3,      3,      3,        1,        1,  -1,  -1},
                {        5,      5,      1,      1,       -1,       -1,  -1,  -1},
                {        5,      5,      2,      2,        1,        1,  -1,  -1},
                {        7,      7,      1,      1,       -1,       -1,  -1,  -1},
                {        7,      7,      2,      2,        1,        1,  -1,  -1},
                {        1,      3,      1,      1,       -1,       -1,  -1,  -1},
                {        3,      1,      1,      1,       -1,       -1,  -1,  -1},
                {        1,      5,      1,      1,       -1,       -1,  -1,  -1},
                {        5,      1,      1,      1,       -1,       -1,  -1,  -1},
                {        1,      7,      1,      1,       -1,       -1,  -1,  -1},
                {        7,      1,      1,      1,       -1,       -1,  -1,  -1},
            };

            for(i = 0; i < sizeof(validatedParams)/8/sizeof(int32_t); i++)
            {
                if((validatedParams[i][0] == -1 || convParams.kernelH == validatedParams[i][0]) &&
                   (validatedParams[i][1] == -1 || convParams.kernelW == validatedParams[i][1]))
                {
                    validated = true;
                }
                else
                {
                    validated = false;
                    continue;
                }

                if((validatedParams[i][2] == -1 || convParams.strideH == validatedParams[i][2]) &&
                   (validatedParams[i][3] == -1 || convParams.strideW == validatedParams[i][3]))
                {
                    validated = true;
                }
                else
                {
                    validated = false;
                    continue;
                }

                if((validatedParams[i][4] == -1 || convParams.dilationH == validatedParams[i][4]) &&
                   (validatedParams[i][5] == -1 || convParams.dilationW == validatedParams[i][5]))
                {
                    validated = true;
                }
                else
                {
                    validated = false;
                    continue;
                }

                if((validatedParams[i][6] == -1 || convParams.padH == validatedParams[i][6]) &&
                   (validatedParams[i][7] == -1 || convParams.padW == validatedParams[i][7]))
                {
                    validated = true;
                }
                else
                {
                    validated = false;
                    continue;
                }

                if(validated)
                {
                    break;
                }
            }
            
            if(shapeInferencestatus != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
            {
                int32_t widthIdx = layer->allowlistingMetaData.varTensorsDims[0].size()-1;
                int32_t heightIdx = layer->allowlistingMetaData.varTensorsDims[0].size()-2;
                int32_t inWidth    = layer->allowlistingMetaData.varTensorsDims[0][widthIdx];
                int32_t inHeight    = layer->allowlistingMetaData.varTensorsDims[0][heightIdx];
                
                if (validated == false && // check if previous checks fails
                    // check if satisfy: N X N stride N
                    (convParams.kernelH == convParams.kernelW) &&   // N X N
                    (convParams.strideH == convParams.strideW) &&   // same stride in both dimensions
                    (convParams.kernelH == convParams.strideH) &&   // stride N
                    (convParams.kernelH >= 3)                  &&   // N > 3
                    (inWidth % convParams.kernelW == 0)        &&
                    (inHeight % convParams.kernelH == 0)        &&
                    (convParams.dilationH == 1) && (convParams.dilationW == 1) &&
                    (convParams.numGroups == 1) &&
                    ((convParams.padT == 0) && (convParams.padL == 0) && 
                    (convParams.padB == 0) && (convParams.padR == 0)))
                {
                    validated = true;
                }
            }

            if((!validated) &&
                convParams.strideH == 4 && convParams.strideW == 4 &&
                convParams.kernelH != 11 && convParams.kernelW != 11)
            {
                return false;
            }

            if(!validated)
            {
                oss << "Kernel size (" << convParams.kernelH << "x"  << convParams.kernelW << ") with stride (" << convParams.strideH << "x" << convParams.strideW << ") is not supported";
                logs = oss.str();
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Input width less than MAX(Pad Left, Pad Right) is not supported for AM62A and AM67A",
        "Input width less than MAX(Pad Left, Pad Right) is not supported for AM62A and AM67A",
        "Input width less than MAX(PadL, PadR) is not supported for AM62A and AM67A",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            int32_t status = tidlCheckShapeInferenceforOnnx(md);
            if(status !=  TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL &&
              (gParams.deviceName & ~TIDL_OTF_FLAG_BIT) == TIDL_AM62A)
            {
                auto convParams = layer->layerParams.convParams;
                if(md.varTensorsDims[0][md.varTensorsDims[0].size()-1] < max(convParams.padL,convParams.padR))
                {
                    return false;
                }
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "",
        "",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            auto convParams = layer->layerParams.convParams;
            if(convParams.kernelH == 11 && convParams.kernelW == 11 &&
               convParams.strideH == 4 && convParams.strideW == 4)
            {
                TIDL_GLOBAL_REPORT_WARNING("Kernel size 11x11 with stride 4 in [%s] has gone through limited verification", (char*)layer->name);
            }
            return true;
        }
    ),
#if MODEL_CHECK_CONV_SUGGESTIONS
    TIDL_CSTR(
        "",
        "",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            auto convParams = layer->layerParams.convParams;
            /* inData is not populated before allow-listing call in OSRT */
            if((convParams.kernelH * convParams.kernelW * layer->inData[0].dimValues[TIDL_DIM_NUMCH] / convParams.numGroups + convParams.enableBias) % 64)
            {
                TIDL_GLOBAL_REPORT_WARNING("In Data in [%s] is not 64 byte aligned. The performance will not be efficient on MMA.", (char*)layer->name);
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "",
        "",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            auto convParams = layer->layerParams.convParams;
            /* outData is not populated before allow-listing call in OSRT */
            if(layer->outData[0].dimValues[TIDL_DIM_NUMCH] / convParams.numGroups % 64)
            {
                TIDL_GLOBAL_REPORT_WARNING("Out Data in [%s] is not 64 byte aligned. The performance will not be efficient on MMA.", (char*)layer->name);
            }
            return true;
        }
    ),
#endif
};

