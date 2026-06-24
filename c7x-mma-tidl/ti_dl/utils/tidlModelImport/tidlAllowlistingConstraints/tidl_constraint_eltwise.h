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

const vector<TidlConstraint> tidlConstraintEltwise = {
    TIDL_CSTR(
        "Only 2 inputs are supported in Add/Mul/Sub/Div/Max/Min/Sum/Mod layers",
        "Only 2 inputs are supported in Add/Mul/Sub/Div/Max/Min/Sum/Mod layers",
        "Only 2 inputs are supported in Add/Mul/Sub/Div layers",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            if(md.numInputs != 2)
            {
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Number of non-singleton variable input dimensions in Add/Mul/Sub/Div/Max/Min/Sum/Mod must be <= 6",
        "Number of non-singleton variable input dimensions in Add/Mul/Sub/Div/Max/Min/Sum/Mod must be <= 6",
        "Number of non-singleton variable input dimensions in Add/Mul/Sub/Div must be <= 6",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            ostringstream oss;
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            if(gParams.modelType != TIDL_IMPORT_MODEL_FORMAT_ONNX)
            {
                for(auto &varDims : md.varTensorsDims)
                {
                    int32_t numDims = tidlGetNonSingletonNumDims(varDims);
                    if(numDims > 6)
                    {
                        oss << "Maximum number of input dimensions supported are 6, found " << numDims << " input dimensions";
                        logs = oss.str();
                        return false;
                    }
                }
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Both inputs as variable are supported in Sub only if firmware version >= 10_01_06_00",
        "Both inputs as variable are supported in Sub only if firmware version >= 10_01_06_00",
        "Both inputs as variable are supported in Sub only if firmware version >= 10_01_06_00",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            if (!TIDL_isSupportedInFirmwareVersion((const char*)gParams.c7xFirmwareVersion, "10_01_06_00"))
            {
                sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
                if(md.varTensorIndices.size() == 2) //2 variable inputs
                {
                    auto oper = layer->layerParams.eltWiseParams.eltWiseType;
                    if(oper == TIDL_EltWiseSub)
                    {
                        return false;
                    }
                }
            }
            return true;
        }
    ), 
    TIDL_CSTR(
        "The variable inputs in Add/Mul/Div/Sub/Max/Min/Sum/Mod layer must of be same dimensions or broadcast-able",
        "The variable inputs in Add/Mul/Div/Sub/Max/Min/Sum/Mod layer must of be same dimensions or broadcast-able",
        "The variable inputs in Add/Mul/Div/Sub layer must of be same dimensions or broadcast-able",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;

            if(md.varTensorIndices.size() == 2) //2 variable inputs
            {
                bool isEltwise = true;
                if (md.varTensorsDims[0].size() == md.varTensorsDims[1].size()) /* must be same dimensions */
                {
                    int32_t heightDim = md.varTensorsDims[0].size() - 2;
                    int32_t widthDim = md.varTensorsDims[0].size() - 1;

                    for (int32_t idx = 0; idx < md.varTensorsDims[0].size(); idx++)
                    {
                        if (md.varTensorsDims[0][idx] != md.varTensorsDims[1][idx])
                        {
                            /*
                            special case for allowing NCHW x NC11
                            only possible mismatch is height & width both are mismatching and both are 1
                            */
                            /*Identify which tensor needs to be broadcasted*/
                            int32_t broadCastTensorIdx = -1;
                            if(md.varTensorsDims[0][idx] == 1)
                            {
                                broadCastTensorIdx = 0;
                            }
                            else if(md.varTensorsDims[1][idx] == 1)
                            {
                                broadCastTensorIdx = 1;
                            }
                            else
                            {
                            /*In this case tensor dimensions are genuinely mismatching*/
                                broadCastTensorIdx = -1;
                            }

                            /*Firmware specific check to control allowlisting for broadcast mul (Available in Firmware > 09_02_06_00)*/
                            if(strcmp("09_02_06_00",(char*)gParams.c7xFirmwareVersion) == 0)
                            {
                                isEltwise = false;
                                break;
                            }

                            if( broadCastTensorIdx != -1)
                            {
                                /*Only broadcast on height & width is allowed with a strict format of NxCx1x1 for the tensor being broadcasted:*/
                                if ((idx == heightDim )||(idx == widthDim))
                                {
                                    if(md.varTensorsDims[broadCastTensorIdx][heightDim] == md.varTensorsDims[broadCastTensorIdx][widthDim] == 1)
                                    {
                                        isEltwise = true;
                                        break;
                                    }
                                    else
                                    {
                                        isEltwise = false;
                                        break;
                                    }
                                }
                                else
                                {
                                    isEltwise = false;
                                }
                            }
                            else
                            {
                                isEltwise = false;
                            }
                        }
                    }
                }
                /* Not same number of dimensions */
                else
                {
                    isEltwise = false;
                }

                if (!isEltwise)
                {
                    return true;
                }
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "Eltwise operator(Add/Mul/Div/Sub/Max/Min/Sum/Mod layer) is supported only with operands of similar dimensions or broadcast supported patterns of both inputs",
        "Eltwise operator(Add/Mul/Div/Sub/Max/Min/Sum/Mod layer) is supported only with operands of similar dimensions or broadcast supported patterns of both inputs",
        "Eltwise operator(Add/Mul/Div/Sub layer) is supported only with operands of similar dimensions or broadcast supported patterns of both inputs",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;

            if(md.varTensorIndices.size() == 2) //2 variable inputs
            {
            }
            else //addition with constant input
            {
                if(md.constTensorsDims[0].size() != 0)
                {
                    //find non singleton dimensions in const tensor
                    int32_t nonSingletonDimNum = 0;
                    int32_t nonSingletonDimIdx = 0;
                    for(int i = 0; i < md.constTensorsDims[0].size(); i++)
                    {
                        if(md.constTensorsDims[0][i] != 1)
                        {
                            nonSingletonDimNum++;
                            nonSingletonDimIdx = i;
                        }
                    }
                    //const tensor > 1D check
                    if((nonSingletonDimNum > 1) ||  /* not batchnorm but broadcast cases */
                       (
                            /* even if there is only one non-singular dim in const -- it is not in channel */
                            (nonSingletonDimNum == 1) && (nonSingletonDimIdx != (md.constTensorsDims[0].size()-3)) &&
                            /* ONLY VALID IF NOT TFLITE - TFLITE CONVERSION DOES NOT WORK FOR LESS THAN 3 DIMS AND
                            CONSIDERS LAST DIM AS WIDTH HERE EVEN THOUGH SHOULD GO TO BROADCAST, HENCE  STRICTLY NOT
                            VALID CHECK FOR TFLITE */
                            (
                                (gParams.modelType != TIDL_IMPORT_MODEL_FORMAT_TFLITE) &&
                                (gParams.modelType != TIDL_IMPORT_MODEL_FORMAT_TENSORFLOW) &&
                                (gParams.modelType != TIDL_IMPORT_MODEL_FORMAT_TFLITE_RT)
                            )
                       )
                    )
                    {
                        bool isEltwise = false;
                        if (md.constTensorsDims.size() == 1 && md.varTensorIndices.size() == 1) //eltwise mul between a variable tensor and constant tensor
                        {
                            int32_t varTensorNumDims    = md.varTensorsDims[0].size();
                            int32_t constTensorNumDims  = md.constTensorsDims[0].size();

                            /* Dimension matching to create a pattern */
                            /**
                             * Store the status of comparison of dims in this array
                             * If same and non-singular       => 4
                             * If same and singular           => 3
                             * not same and one is singular   => 2
                             * not same and none is singular  => 1
                            */
                            int32_t dimMatchStatus[TIDL_DIM_MAX];
                            /* Start dim indices from the last i.e., width */
                            int32_t vIdx = varTensorNumDims-1, cIdx = constTensorNumDims-1;
                            for (int32_t idx = TIDL_DIM_WIDTH; idx >= 0; idx--)
                            {
                                /* reset comaprison status */
                                dimMatchStatus[idx] = 0;
                                /* check if tensor dim exists */
                                if ((vIdx >= 0) && (cIdx >= 0))
                                {
                                    /* both same */
                                    if (md.varTensorsDims[0][vIdx] == md.constTensorsDims[0][cIdx])
                                    {
                                    /* same and singular */
                                        if (md.varTensorsDims[0][vIdx] == 1)
                                        {
                                            dimMatchStatus[idx] = 3;
                                        }
                                        /* same and not singular */
                                        else
                                        {
                                            dimMatchStatus[idx] = 4;
                                        }
                                    }
                                    /* not same */
                                    else
                                    {
                                        /* not same and one of them is singular */
                                        if ((md.varTensorsDims[0][vIdx] == 1) || (md.constTensorsDims[0][cIdx] == 1))
                                        {
                                            dimMatchStatus[idx] = 2;
                                        }
                                        /* not same and none is singular */
                                        else
                                        {
                                            dimMatchStatus[idx] = 1;
                                        }
                                    }
                                    vIdx--; cIdx--;
                                }
                                /* var tensor dim exists, const does not
                                    this makes certain the corresponding const dim is 1
                                */
                                else if ((vIdx >= 0) and (cIdx < 0))
                                {
                                    /* singluar and same */
                                    if (md.varTensorsDims[0][vIdx] == 1)
                                    {
                                        dimMatchStatus[idx] = 3;
                                    }
                                    /* one is singular and not same */
                                    else
                                    {
                                        dimMatchStatus[idx] = 2;
                                    }
                                    vIdx--;
                                }
                                else if ((vIdx < 0) and (cIdx >= 0))
                                {
                                    /* singluar and same */
                                    if (md.constTensorsDims[0][cIdx] == 1)
                                    {
                                        dimMatchStatus[idx] = 3;
                                    }
                                    /* one is singular and not same */
                                    else
                                    {
                                        dimMatchStatus[idx] = 2;
                                    }
                                    cIdx--;
                                }
                                /* must be 1 */
                                else
                                {
                                    /* same and singular */
                                    dimMatchStatus[idx] = 3;
                                }
                            }


                            int32_t matchConsolidatedResult = 0;
                            for(int32_t idx = 0; idx < TIDL_DIM_MAX; idx++)
                            {
                                matchConsolidatedResult = matchConsolidatedResult*10 + dimMatchStatus[idx];
                            }
                            /**
                            * Supported special patterns
                            * 1x1x1x1x1xW   &   1x1x1x1xHxW    ===> 333324
                            * 1x1x1x1x1xW   &   1x1x1xCxHxW    ===> 333224
                            * 1x1x1x1x1xW   &   1x1xD2xCxHxW   ===> 332224
                            *
                            * 1x1x1x1xHxW   &   1x1x1xCxHxW    ===> 333244
                            * 1x1x1x1xHxW   &   1x1xD2xCxHxW   ===> 332244
                            *
                            * 1x1x1xCxHxW   &   1x1xD2xCxHxW   ===> 332444
                            *
                            * 1x1xD2x1xHxW  &   1x1xD2xCxHxW   ===> 334244
                            *
                            * 1x1x1x1xHxW   &   1x1x1x1xHx1    ===> 333342
                            * 1x1x1xCx1xW   &   1x1x1xCxHxW    ===> 333424
                            * 1x1xD2xCxHxW  &   1x1xD2xCx1xW   ===> 334424
                            */
                            /**
                             * TODO: conflict if batch is there with last 2 patterns
                             * for e.g. -> 3X3X224 && 4x3x3x224
                            */

                            std::vector<int32_t> suuportedDimCmp{333324, 333224, 332224, 333234,
                                                                333244, 332244,
                                                                332444,
                                                                334244,
                                                                333342,
                                                                332344, /* Ex: 2x1xHxW & 1x1xHxW */
                                                                332324,  /* Ex: 2x1xHxW & W */
                                                                333424,
                                                                334424
                                                                };

                            for (const int32_t& j: suuportedDimCmp)
                            {
                                if (j == matchConsolidatedResult)
                                {
                                    isEltwise = true;
                                    break;
                                }
                                else
                                {

                                }
                            }

                            /* Similar dimension are by default supported
                            to check if similar dimension it is sufficient to check
                            if all elements in match status are only 3 or 4
                            */
                            if (!isEltwise) /* Check only if earlier broadcast checks fail*/
                            {
                                int32_t isAllDimSame = 0;
                                for (int32_t idx = 0; idx < TIDL_DIM_MAX; idx++)
                                {
                                    if ((dimMatchStatus[idx] == 3) || (dimMatchStatus[idx] == 4))
                                    {
                                        isAllDimSame += 0;  /* Added 0 when a 3 or 4 is found*/
                                    }
                                    else
                                    {
                                        isAllDimSame += 1; /* Becomes non zero whenever something else is encountered*/
                                    }
                                }

                                if (isAllDimSame == 0)
                                {
                                    isEltwise = true;
                                }
                            }
                        }

                        if(!isEltwise)
                        {
                            return true;
                        }
                    }
                }
            }
            return true;
        }
    ), 
    TIDL_CSTR(
        "1D vector dimension should match with channel or width dimension",
        "1D vector dimension should match with channel or width dimension",
        "1D vector dimension should match with channel or width dimension",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;

            if(md.varTensorIndices.size() == 2) //2 variable inputs
            {
            }
            else //addition with constant input
            {
                if(md.constTensorsDims[0].size() != 0)
                {
                    //find non singleton dimensions in const tensor
                    int32_t nonSingletonDimNum = 0;
                    int32_t nonSingletonDimIdx = 0;
                    for(int i = 0; i < md.constTensorsDims[0].size(); i++)
                    {
                        if(md.constTensorsDims[0][i] != 1)
                        {
                            nonSingletonDimNum++;
                            nonSingletonDimIdx = i;
                        }
                    }
                    //const tensor > 1D check
                    if((nonSingletonDimNum > 1) ||  /* not batchnorm but broadcast cases */
                       (
                            /* even if there is only one non-singular dim in const -- it is not in channel */
                            (nonSingletonDimNum == 1) && (nonSingletonDimIdx != (md.constTensorsDims[0].size()-3)) &&
                            /* ONLY VALID IF NOT TFLITE - TFLITE CONVERSION DOES NOT WORK FOR LESS THAN 3 DIMS AND
                            CONSIDERS LAST DIM AS WIDTH HERE EVEN THOUGH SHOULD GO TO BROADCAST, HENCE  STRICTLY NOT
                            VALID CHECK FOR TFLITE */
                            (
                                (gParams.modelType != TIDL_IMPORT_MODEL_FORMAT_TFLITE) &&
                                (gParams.modelType != TIDL_IMPORT_MODEL_FORMAT_TENSORFLOW) &&
                                (gParams.modelType != TIDL_IMPORT_MODEL_FORMAT_TFLITE_RT)
                            )
                       )
                    )
                    {
                    }
                    else
                    {
                        if(gParams.modelType != TIDL_IMPORT_MODEL_FORMAT_ONNX)
                        {
                            /** Const input is either single value or 1D tensor
                             * If single value, always supported
                             * If 1D tensor -> must be same as channel of var input
                             */
                            int32_t varTensorNumDim = md.varTensorsDims[0].size();
                            int32_t varTensorChDim = varTensorNumDim - 3;
                            if((nonSingletonDimNum == 1)  &&          /* 1D tensor */
                               (md.constTensorsDims[0][nonSingletonDimIdx] != md.varTensorsDims[0][varTensorChDim]) /* shape not same as channel of var input */
                            ) 
                            {
                                return false;
                            }
                        }
                    }
                }
            }
            return true;
        }
    ),
#ifdef TIDL_EN_MULTI_TENSOR_ELT_WISE
    TIDL_CSTR(
        "Too many inputs for elementwise operator",
        "Too many inputs for elementwise operator",
        "Too many inputs for elementwise operator",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            sTIDL_EltWiseParams_t& params = layer->layerParams.eltWiseParams;
            /* inData is not populated before allow-listing call in OSRT */
            if(((layer->numInBufs > 2) && (params.eltWiseType == TIDL_EltWiseProduct)) ||
               ((layer->numInBufs > ((TIDL_SIMD_WIDTH >> 1)/((layer->inData[0].elementType >> 1) + 1)))
                && (params.eltWiseType == TIDL_EltWiseSum)))
            {
                return false;
            }
            return true;
        }
    ),
#else
    TIDL_CSTR(
        "Only 2 inputs are supported",
        "",
        "",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            if(layer->numInBufs > 2)
            {
                return false;
            }
            return true;
        }
    ),
#endif
};
