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

const vector<TidlConstraint> tidlConstraintInnerProduct = {
    TIDL_CSTR(
        "Number of non-singleton variable input dimensions must be <= 4",
        "Number of non-singleton variable input dimensions must be <= 4",
        "Number of non-singleton variable input dimensions must be <= 4",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            int32_t status = tidlCheckShapeInferenceforOnnx(md);
            if(status !=  TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
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
        "Height(H) and Width(W) dimension should be 1 in NHWC data format",
        "",
        "Height(H) and Width(W) dimension should be 1 in NHWC data format",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            int32_t status = tidlCheckShapeInferenceforOnnx(md);
            if(status !=  TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL &&
              (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE_RT ||
               gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE))
            {
                int32_t numDims = md.varTensorsDims[0].size();
                int32_t n = 1, c = 1, h = 1, w = 1;
                /*
                 * If number of dimensions are 4 then data format is changed to NCHW format
                 * In other cases its assumed as NHWC format
                */
                if(numDims == 4)
                {
                    n = md.varTensorsDims[0][0];
                    c = md.varTensorsDims[0][1];
                    h = md.varTensorsDims[0][2];
                    w = md.varTensorsDims[0][3];
                }
                else if(numDims == 3)
                {
                    n = md.varTensorsDims[0][0];
                    h = md.varTensorsDims[0][1];
                    w = md.varTensorsDims[0][2];
                }
                else if(numDims == 2)
                {
                    n = md.varTensorsDims[0][0];
                    w = md.varTensorsDims[0][1];
                }

                if(h != 1 || w != 1)
                {
                    return false;
                }
            }

            return true;
        }
    ),
    TIDL_CSTR(
        "Filter tensor input should have atleast 2 dimensions",
        "Filter tensor input should have atleast 2 dimensions",
        "Filter tensor input should have atleast 2 dimensions",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            int32_t status = tidlCheckShapeInferenceforOnnx(md);
            if(status !=  TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
            {
                vector<int32_t> filterDims;
                if(md.varTensorIndices.size() == 2)
                {
                    filterDims = md.varTensorsDims[1];
                }
                else if(md.constTensorIndices.size() >= 1)
                {
                    filterDims = md.constTensorsDims[0];
                }
                else
                {
                    return false;
                }

                if(filterDims.size() < 2)
                {
                    stringstream ss;
                    ss<<"Filter tensor input should have atleast 2 dimensions, observed "<<filterDims.size();
                    logs = ss.str();
                    return false;
                }
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Gemm layer does not support more than one variable inputs",
        "Gemm layer does not support more than one variable inputs",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            int32_t status = tidlCheckShapeInferenceforOnnx(md);
            if(status !=  TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
            {
                /* Gemm layer */
                if(layer->layerPCParams.innerProductParams.isGemm == 1)
                {
                    if(md.numVarInputs > 1)
                    {
                        return false;
                    }
                }
            }
            return true;
        }
    ),
    /**
     * This check has made some incorrect assumptions & needs to be updated
     * Disabling this temporarily
    */
    #if 0
    TIDL_CSTR(
        /* needs review */
        "Filter dimension and input dimension should match", // Bad Comment
        "",
        "Filter dimension and input dimension should match",
        [](const sTIDL_LayerPC_t *layer, string &logs){

            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            int32_t status = tidlCheckShapeInferenceforOnnx(md);
            if(status !=  TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
            {
                //variable input checks
                int32_t varIdx = md.varTensorIndices[0]; //Only one variable input

                if((md.constTensorIndices.size() == 1) || (md.constTensorIndices.size() == 2)) //filter, bias
                {
                    int32_t filter = 0;
                    int32_t bias = 1;

                    if(gParams.modelType != TIDL_IMPORT_MODEL_FORMAT_ONNX)
                    {
                        int32_t filterNumDim = md.constTensorsDims[filter].size();
                        int32_t varTensorNumDim = md.varTensorsDims[0].size();

                        if(filterNumDim > varTensorNumDim)
                        {
                            return false;
                        }
                        /* match filterHeight with varInput width */
                        int32_t filterHeight = md.constTensorsDims[filter][filterNumDim-2];
                        if(md.varTensorsDims[0][varTensorNumDim - 1] != filterHeight)
                        {
                            return false;
                        }
                        /* rest all dims should match */
                        int32_t j = varTensorNumDim - 3;
                        for(int32_t i = filterNumDim - 3; i >= 0; i--)
                        {
                            if(md.varTensorsDims[0][j] != md.constTensorsDims[filter][i] && md.constTensorsDims[filter][i] != 1)
                            {
                                return false;
                            }
                            j--;
                        }
                    }
                }
            }
            return true;
        }
    ),
    #endif
    TIDL_CSTR(
        "Filter and input must be of same dimensions or broadcast-able",
        "Filter and input must be of same dimensions or broadcast-able",
        "Filter and input must be of same dimensions or broadcast-able",
        [](const sTIDL_LayerPC_t *layer, string &logs){

            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            int32_t status = tidlCheckShapeInferenceforOnnx(md);
            if(status !=  TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
            {
                vector<int32_t> filterDims;
                if(md.varTensorIndices.size() == 2)
                {
                    filterDims = md.varTensorsDims[1];
                }
                else if(md.constTensorIndices.size() >= 1)
                {
                    filterDims = md.constTensorsDims[0];
                }
                else
                {
                    return false;
                }

                if(gParams.modelType != TIDL_IMPORT_MODEL_FORMAT_ONNX)
                {
                    int32_t filterNumDim = filterDims.size();
                    int32_t varTensorNumDim = md.varTensorsDims[0].size();
                    bool isIPLayer = true;
                    if (filterNumDim > 2 && varTensorNumDim > 2)
                    {
                        int32_t filterChDim = filterNumDim - 3;
                        int32_t varTensorChDim = varTensorNumDim - 3;
                        /* Both filter and varTensor have channel dimensions*/
                        if (filterDims[filterChDim] != md.varTensorsDims[0][varTensorChDim]) /* Not same channel */
                        {
                            /* one of them must be 1*/
                            if ((filterDims[filterChDim] == 1) || (md.varTensorsDims[0][varTensorChDim] == 1))
                            {
                                /* okay */
                            }
                            else
                            {
                                isIPLayer = false;
                            }
                        }
                        else
                        {
                            /* same channel dim, okay */
                        }
                    }
                    if (!isIPLayer)
                    {
                        stringstream ss;
                        ss<<"Mismatch in filter dimension and input dimension -- Filter dimension = ["<<filterDims[0]<<" "<<filterDims[1]<<" "<<filterDims[2]<<"], input dimensions = ["<<md.varTensorsDims[0][0]<<" "<<md.varTensorsDims[0][1]<<" "<<md.varTensorsDims[0][2]<<"]";
                        logs = ss.str();
                        return false;
                    }
                }
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Bias tensor input should be a vector of [1, N] or [N] where N should match output width",
        "Bias tensor input should be a vector of [1, N] or [N] where N should match output width",
        "Bias tensor input should be a vector of [1, N] or [N] where N should match output width",
        [](const sTIDL_LayerPC_t *layer, string &logs){

            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            int32_t status = tidlCheckShapeInferenceforOnnx(md);
            if(status !=  TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL && 
              (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX || gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX_RT || gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TVM_RELAY))
            {
                //variable input checks

                int32_t varIdx = md.varTensorIndices[0]; //Only one variable input

                if((md.constTensorIndices.size() == 1) || (md.constTensorIndices.size() == 2)) //filter, bias
                {
                    int32_t filter = 0;
                    int32_t bias = 1;

                    if(md.constTensorIndices.size() == 2) //both weight and bias present
                    {
                        //Bias tensor will have number of dimensions either 1 or 2 as per Gemm layer definition
                        int32_t outDims = md.outputTensorDims[0].size();    /* output shape is [M,N] */
                        int32_t outCols = outDims > 0 ? md.outputTensorDims[0][outDims - 1] : 1;
                        int32_t outRows = outDims > 1 ? md.outputTensorDims[0][outDims - 2] : 1;
                        if(md.constTensorsDims[bias].size() == 2)
                        {
                            int32_t broadcastBiasIdx = -1;
                            /* bias is [M,1] */
                            if(md.constTensorsDims[bias][0] != 1 && md.constTensorsDims[bias][0] == outRows)
                            {
                                broadcastBiasIdx = 1;
                            }
                            /* bias is [1,N] */
                            else if(md.constTensorsDims[bias][1] == outCols)
                            {
                                broadcastBiasIdx = 0;
                            }
                            else
                            {
                                /* bias is not broadcastable */
                                return false;
                            }
                            /* only [1,N] is supported */
                            if((broadcastBiasIdx == -1 || broadcastBiasIdx == 1) || md.constTensorsDims[bias][broadcastBiasIdx] != 1)
                            {
                                return false;
                            }
                        }
                        else if(md.constTensorsDims[bias].size() == 1)
                        {
                            if(md.constTensorsDims[bias][0] != outCols)
                            {
                                return false;
                            }
                        }
                        else
                        {
                            /* 3D bias is not supported */
                            return false;
                        }
                    }
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
            /* inData, outData is not populated before allow-listing call in OSRT */
            int32_t x = layer->inData[0].dimValues[TIDL_DIM_WIDTH];
            int32_t y = layer->outData[0].dimValues[TIDL_DIM_WIDTH];
            if((x * y) > (2048 * 2048))
            {
                TIDL_GLOBAL_REPORT_WARNING("Size larger than (2048*2048) in [%s] is not optimal", (char*)layer->name);
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Only supported Gemm params are transA = 0, alpha = 1.0 and beta = 1.0. The same will processed as inner product or fully connected layer in TIDL",
        "Only supported Gemm params are transA = 0, alpha = 1.0 and beta = 1.0. The same will processed as inner product or fully connected layer in TIDL",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            if(gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX || gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX_RT || gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TVM_RELAY)
            {
                sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
                int32_t status = tidlCheckShapeInferenceforOnnx(md);
                if(status !=  TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
                {
                    int32_t transA = layer->layerParams.innerProductParams.inputATranspose;
                    float alpha = layer->layerPCParams.innerProductParams.alpha;
                    float beta = layer->layerPCParams.innerProductParams.beta;
                    if ((transA != 0) || (alpha != 1.0) || (beta != 1.0))
                    {
                        return false;
                    }
                }
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Gemm layer is not supported in TIDL when bias size != output width, please use edgeai model optimizer tool to convert Gemm to (MatMul + Add) combination",
        "Gemm layer is not supported in TIDL when bias size != output width, please use edgeai model optimizer tool to convert Gemm to (MatMul + Add) combination",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            if (layer->optimized == 1 &&
                (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX_RT || gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TVM_RELAY) &&
                layer->layerPCParams.innerProductParams.isGemm == 1)
            {
                sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
                int32_t status = tidlCheckShapeInferenceforOnnx(md);
                if(status !=  TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
                {
                    if (md.constTensorIndices.size() == 2)
                    {
                        //bias present
                        int32_t output_size = 1, bias_size = 1;
                        int32_t outIdx = md.outputTensorDims[0].size() - 1;
                        if(outIdx >= 0)
                        {
                            output_size =  md.outputTensorDims[0][outIdx];
                        }
                        else
                        {
                            /*Invalid output tensor index*/
                            return false;
                        }
                        for (int32_t i = 0; i < md.constTensorsDims[1].size(); i++)
                        {
                            bias_size *= md.constTensorsDims[1][i];
                        }

                        if (bias_size != output_size)
                        {
                            return false;
                        }
                    }
                }
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "MatMul with signed inputs & unsigned output is not supported ",
        "MatMul with signed inputs & unsigned output is not supported ",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            if (layer->optimized == 1 &&
                layer->layerPCParams.innerProductParams.isGemm == 0 &&
                ((layer->inData[0].elementType == TIDL_SignedChar && layer->inData[1].elementType == TIDL_SignedChar && layer->outData[0].elementType == TIDL_UnsignedChar) ||
                (layer->inData[0].elementType == TIDL_SignedShort && layer->inData[1].elementType == TIDL_SignedShort && layer->outData[0].elementType == TIDL_UnsignedShort)))
            {
                /**
                 * MatMul with signed inputs & unsigned output is not supported
                */
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "MatMul with signed & unsigned input combination is not supported in TDA4VM & is only supported in firmware version >= 10_00_07_00",
        "MatMul with signed & unsigned input combination is not supported in TDA4VM & is only supported in firmware version >= 10_00_07_00",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){

            if (layer->optimized == 1)
            {
                if  (layer->layerPCParams.innerProductParams.isGemm == 0 &&
                    ((gParams.deviceName & ~TIDL_OTF_FLAG_BIT) == TIDL_TDA4VM) &&
                    (!TIDL_isSupportedInFirmwareVersion((const char*)gParams.c7xFirmwareVersion, "10_00_06_00")) && //fxn returns false for any version <= 10_00_06_00
                    ((layer->inData[0].elementType != layer->inData[1].elementType)))
                {
                    return false;
                }
            }
            return true;
        }
    )
};