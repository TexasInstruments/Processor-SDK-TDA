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

const vector<TidlConstraint> tidlConstraintDeconv =
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
        "Weight tensor size should match with proto kernel_shape",
        "Weight tensor size should match with proto kernel_shape",
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
        "Only 4x4, 3x3 and 2x2 kernels with 2x2 stride are supported",
        "Only 4x4, 3x3 and 2x2 kernels with 2x2 stride are supported",
        "Only 4x4, 3x3 and 2x2 kernels with 2x2 stride are supported",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            auto params = layer->layerParams.convParams;

            if (layer->optimized != 0)
            {
                return true;
            }

            if(params.kernelH == 4 && params.kernelW == 4 &&
                params.strideH == 2 && params.strideW == 2 &&
                params.padW == 1 && params.padH == 1)
            {
                return true;
            }
            else if((params.kernelH == 3 && params.kernelW == 3 &&
                params.strideH == 2 && params.strideW == 2))
            {
                return true;
            }
            else if (params.kernelH == 2 && params.kernelW == 2 &&
                params.strideH == 2 && params.strideW == 2 &&
                params.padW == 0 && params.padH == 0)
            {
                return true;
            }
            else if (params.kernelH == 5 && params.kernelW == 5 &&
                params.strideH == 4 && params.strideW == 4 &&
                params.padH == 1 && params.padW == 1)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    ), 
    TIDL_CSTR(
        "Change to Upsample/Resize if possible. Upsample/Resize will be more efficient",
        "Change to Upsample/Resize if possible. Upsample/Resize will be more efficient",
        "Change to Resize if possible. Resize will be more efficient",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            TIDL_GLOBAL_REPORT_WARNING("Change to Upsample/Resize if possible instead of Deconvolution. It will be more efficient")
            return true;
        }
    ),
    TIDL_CSTR(
        "16-bit Deconvolution is not suppported on AM62A and AM67A",
        "16-bit Deconvolution is not suppported on AM62A and AM67A",
        "16-bit Deconvolution is not suppported on AM62A and AM67A",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            /* This fixes [TIDL-2948] */
            if(((gParams.deviceName & ~TIDL_OTF_FLAG_BIT) == TIDL_AM62A) && gParams.numParamBits == 16)
            {
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Only default dilations values (1,1) are supported",
        "Only default dilations values (1,1) are supported",
        "Only default dilations values (1,1) are supported",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            auto params = layer->layerParams.convParams;
            if(params.dilationH != 1 || params.dilationW != 1)
            {
                return false;
            }
            return true;
        }
    )
};