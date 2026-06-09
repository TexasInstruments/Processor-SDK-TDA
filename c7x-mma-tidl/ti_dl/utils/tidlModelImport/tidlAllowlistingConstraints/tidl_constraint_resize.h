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

const vector<TidlConstraint> tidlConstraintResize =
{
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
        "Only 'nearest' and 'linear' resize mode are supported",
        "Only 'nearest' and 'linear' resize mode are supported",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            if (layer->layerParams.resizeParams.mode == -1)
            {
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Resize is only supported along width and height axis",
        "Resize is only supported along width and height axis",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            const sTIDL_ResizeLayerParams_t& params = layer->layerParams.resizeParams;
            for(int32_t i = 0; i <= TIDL_DIM_NUMCH; i++)
            {
                if(params.resizeRatio[i] != 1.0)
                {
                    return false;
                }
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Scales < 1 are not supported",
        "Scales < 1 are not supported",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            const sTIDL_ResizeLayerParams_t& params = layer->layerParams.resizeParams;
            float32_tidl hRatio = params.resizeRatio[TIDL_DIM_HEIGHT];
            float32_tidl wRatio = params.resizeRatio[TIDL_DIM_WIDTH];
            if(hRatio < 1.0 || wRatio < 1.0)
            {
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Width and Height scale should be same",
        "Width and Height scale should be same",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            const sTIDL_ResizeLayerParams_t& params = layer->layerParams.resizeParams;
            float32_tidl hRatio = params.resizeRatio[TIDL_DIM_HEIGHT];
            float32_tidl wRatio = params.resizeRatio[TIDL_DIM_WIDTH];
            if(hRatio != wRatio)
            {
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Only Power of 2 scales are supported",
        "Only Power of 2 scales are supported",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            const sTIDL_ResizeLayerParams_t& params = layer->layerParams.resizeParams;
            float32_tidl hRatio = params.resizeRatio[TIDL_DIM_HEIGHT];
            float32_tidl wRatio = params.resizeRatio[TIDL_DIM_WIDTH];
            if(hRatio == 0 || wRatio == 0)
            {
                return false;
            }
            int32_t isHRatioP2 = (ceil(log2(hRatio)) == floor(log2(hRatio))) ? 1 : 0;
            int32_t isWRatioP2 = (ceil(log2(wRatio)) == floor(log2(wRatio))) ? 1 : 0;
            /* assuming hRatio and wRatio are same and >= 1.0 , if not - already denied due to upper constraint */
            if (wRatio == 1.0  ||
                wRatio == 2.0  ||
                wRatio == 4.0)
            {
                /* 
                    Ratio = 1 will be either merged or get converted to Reshape
                    Ratio = 2 or 4 are perfectly ok    
                 */
            }
            else if(isWRatioP2 == 0)
            {
                ostringstream oss;
                oss << "Non-power of 2 scales in layer " << (char*)layer->name << " is not supported";
                logs = oss.str();
                return false;
            }
            else
            {
                /*
                    Any resize ratio which is power of 2 and greater than 4
                    will be placed by combination of 4x4 resize layer and 2x2
                    resize layer
                */
                TIDL_GLOBAL_REPORT_WARNING("Resize layer - %s with scales > 4 is not optimal", (char*)layer->name);
            }
            return true;
        }
    ),

    TIDL_CSTR(
        "Only default antialias = 0 is supported",
        "Only default antialias = 0 is supported",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            if (layer->layerPCParams.resizeParams.antialias != 0)
            {
                return false;
            }
            return true;
        }
    ),

    TIDL_CSTR(
        "Only default keep_aspect_ratio_policy = 'stretch' is supported",
        "Only default keep_aspect_ratio_policy = 'stretch' is supported",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            if (layer->layerPCParams.resizeParams.keep_aspect_ratio_policy != 0)
            {
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Only 'half_pixel', 'pytorch_half_pixel' and 'asymmetric' coordinate_transformation_mode is supported",
        "Only 'half_pixel', 'pytorch_half_pixel' and 'asymmetric' coordinate_transformation_mode is supported",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            if(gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX || gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX_RT || gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TVM_RELAY)
            {
                int32_t coordinate_transformation_mode = layer->layerPCParams.resizeParams.coordinate_transformation_mode;
                if(coordinate_transformation_mode == TIDL_Resize_align_corners)
                {
                    TIDL_GLOBAL_REPORT_WARNING("Resize layer - %s with 'align_corners' coordinate_transformation_mode is not optimal and might result in incorrect output", (char*)layer->name);
                }
                if(!(coordinate_transformation_mode == TIDL_Resize_half_pixel || coordinate_transformation_mode == TIDL_Resize_asymmetric
                    || coordinate_transformation_mode == TIDL_Resize_pytorch_half_pixel || coordinate_transformation_mode == TIDL_Resize_align_corners))
                {
                    return false;
                }
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Only 'round_prefer_ceil' nearest mode is supported when coordinate_transformation_mode is 'half_pixel' or 'pytorch_half_pixel'",
        "Only 'round_prefer_ceil' nearest mode is supported when coordinate_transformation_mode is 'half_pixel' or 'pytorch_half_pixel'",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            if(gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX || gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX_RT || gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TVM_RELAY)
            {
                int32_t coordinate_transformation_mode = layer->layerPCParams.resizeParams.coordinate_transformation_mode;
                int32_t mode = layer->layerParams.resizeParams.mode;
                int32_t nearest_mode = layer->layerPCParams.resizeParams.nearest_mode;
                if((coordinate_transformation_mode == TIDL_Resize_half_pixel || coordinate_transformation_mode == TIDL_Resize_pytorch_half_pixel)
                    && mode == TIDL_ResizeNearest)
                {
                    if(nearest_mode == TIDL_Resize_round_prefer_floor)
                    {
                        TIDL_GLOBAL_REPORT_WARNING("Resize layer - %s with 'round_prefer_floor' nearest mode is not optimal", (char*)layer->name);
                    }
                    if(!(nearest_mode == TIDL_Resize_round_prefer_ceil || nearest_mode == TIDL_Resize_round_prefer_floor))
                    {
                        return false;
                    }
                }
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "'pytorch_half_pixel' coordinate_transformation_mode is not supported for Resize output shapes <= 1",
        "'pytorch_half_pixel' coordinate_transformation_mode is not supported for Resize output shapes <= 1",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            if(gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX || gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX_RT || gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TVM_RELAY)
            {
                int32_t coordinate_transformation_mode = layer->layerPCParams.resizeParams.coordinate_transformation_mode;
                sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
                vector<int32_t> outDims = md.outputTensorDims[0];
                int32_t outDimsSize = outDims.size();
                if(coordinate_transformation_mode == TIDL_Resize_pytorch_half_pixel)
                {
                    int32_t wOut = 2;
                    int32_t hOut = 2;
                    wOut = outDimsSize >= 1 ? outDims[outDimsSize - 1] : wOut;
                    hOut = outDimsSize >= 2 ? outDims[outDimsSize - 2] : hOut;
                    if(wOut <= 1 || hOut <= 1)
                    {
                        return false;
                    }
                }
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Only 'floor' nearest mode is supported when coordinate_transformation_mode is 'asymmetric'",
        "Only 'floor' nearest mode is supported when coordinate_transformation_mode is 'asymmetric'",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            if(gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX || gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX_RT || gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TVM_RELAY)
            {
                int32_t coordinate_transformation_mode = layer->layerPCParams.resizeParams.coordinate_transformation_mode;
                int32_t mode = layer->layerParams.resizeParams.mode;
                int32_t nearest_mode = layer->layerPCParams.resizeParams.nearest_mode;
                if(coordinate_transformation_mode == TIDL_Resize_asymmetric && mode == TIDL_ResizeNearest)
                {
                    if(nearest_mode != TIDL_Resize_floor)
                    {
                        return false;
                    }
                }
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "'linear' resize mode is not supported when coordinate_transformation_mode is 'asymmetric'",
        "'linear' resize mode is not supported when coordinate_transformation_mode is 'asymmetric'",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            if(gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX || gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX_RT || gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TVM_RELAY)
            {
                int32_t coordinate_transformation_mode = layer->layerPCParams.resizeParams.coordinate_transformation_mode;
                int32_t mode = layer->layerParams.resizeParams.mode;
                if(coordinate_transformation_mode == TIDL_Resize_asymmetric && mode == TIDL_ResizeBilinear)
                {
                    return false;
                }
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Only default exclude_outside = 0 is supported",
        "Only default exclude_outside = 0 is supported",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            if(gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX || gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX_RT || gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TVM_RELAY)
            {
                if(layer->layerPCParams.resizeParams.exclude_outside != 0)
                {
                    return false;
                }
            }
            return true;
        }
    ),

};