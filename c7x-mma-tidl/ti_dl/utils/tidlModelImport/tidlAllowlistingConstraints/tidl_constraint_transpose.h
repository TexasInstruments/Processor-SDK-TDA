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

const vector<TidlConstraint> tidlConstraintTranspose = {
    TIDL_CSTR(
        "Number of non-singleton variable input dimensions must be <= 6",
        "Number of non-singleton variable input dimensions must be <= 6",
        "Number of non-singleton variable input dimensions must be <= 6",
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
        "For firmware version < 10_01_04_00, only permutes are suported when number of dimensions > 4",
        "For firmware version < 10_01_04_00, only permutes are suported when number of dimensions > 4",
        "For firmware version < 10_01_04_00, only permutes are suported when number of dimensions > 4",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            if(TIDL_isSupportedInFirmwareVersion((const char*)gParams.c7xFirmwareVersion, "10_01_04_00"))
            {
                return true;
            }

            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            int32_t shapeInferencestatus = tidlCheckShapeInferenceforOnnx(md);
            int32_t status = 0, nonZeroHigherDims = 0;

            int32_t *perm  = (int32_t*)layer->weights.ptr;
            int32_t numDim = layer->weights.bufSize;

            if (shapeInferencestatus != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL && perm != NULL)
            {
                if(numDim > 4)
                {
                    for (int i = numDim - 5; i>= 0; i--)
                    {
                        if (md.varTensorsDims[0][i] != 1)
                        {
                            nonZeroHigherDims++;
                        }
                    }
                    if (perm[numDim -1] != numDim - 1 && nonZeroHigherDims != 0)
                    {
                        return false;
                    }
                }
            }

            return true;
        }
    ),
    TIDL_CSTR(
        "Transpose over batch dimension is only supported in firmware version >= 10_01_04_00",
        "Transpose over batch dimension is only supported in firmware version >= 10_01_04_00",
        "Transpose over batch dimension is only supported in firmware version >= 10_01_04_00",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            if(TIDL_isSupportedInFirmwareVersion((const char*)gParams.c7xFirmwareVersion, "10_01_04_00"))
            {
                return true;
            }

            int32_t *perm = (int32_t*)layer->weights.ptr;
            int32_t numDim = layer->weights.bufSize, nonZeroHigherDims = 0;

            /*Update parsing logic to store in transpose params directly and check transpose params here*/
            if(perm[0] != TIDL_DIM_BATCH && numDim == TIDL_DIM_MAX)
            {
                return false;
            }
            return true;
        }
    ),
    TIDL_CSTR(
        "For inputs with more than 4 dimensions (>4D), permutations where the width dimension is mapped to either the first or second position (Dim 0 or Dim 1) in the output shape are not supported. Unsupported permutation patterns: [W, X, X, X, X, X] - Width dimension at position 0; [X, W, X, X, X, X] - Width dimension at position 1",
        "For inputs with more than 4 dimensions (>4D), permutations where the width dimension is mapped to either the first or second position (Dim 0 or Dim 1) in the output shape are not supported. Unsupported permutation patterns: [W, X, X, X, X, X] - Width dimension at position 0; [X, W, X, X, X, X] - Width dimension at position 1",
        "For inputs with more than 4 dimensions (>4D), permutations where the width dimension is mapped to either the first or second position (Dim 0 or Dim 1) in the output shape are not supported. Unsupported permutation patterns: [W, X, X, X, X, X] - Width dimension at position 0; [X, W, X, X, X, X] - Width dimension at position 1",
        [](const sTIDL_LayerPC_t *layer, string &logs){
            sTIDL_allowlistingMetaData md = layer->allowlistingMetaData;
            int32_t shapeInferenceStatus = tidlCheckShapeInferenceforOnnx(md);

            if(shapeInferenceStatus != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL &&
               TIDL_isSupportedInFirmwareVersion((const char*)gParams.c7xFirmwareVersion, "10_01_04_00"))
            {
                int32_t *perm = (int32_t*)layer->weights.ptr;
                int32_t numDim = layer->weights.bufSize;
                int32_t srcStrides[TIDL_DIM_MAX];
                int32_t dstStrides[TIDL_DIM_MAX];
                int32_t perm_final[TIDL_DIM_MAX];
                int32_t inDims[TIDL_DIM_MAX];
                int32_t outDims[TIDL_DIM_MAX];

                int32_t extraDims = TIDL_DIM_MAX - numDim;
                for(int32_t i = 0; i < extraDims; i++)
                {
                    inDims[i] = 1;
                    outDims[i] = 1;
                    perm_final[i] = i;
                }
                for(int32_t j = extraDims; j < TIDL_DIM_MAX; j++)
                {
                    perm_final[j] = perm[j - extraDims] + extraDims;
                    inDims[j] = md.varTensorsDims[0][j - extraDims];
                    outDims[j] = md.outputTensorDims[0][j - extraDims];
                }

                srcStrides[TIDL_DIM_MAX-1] = 1;
                dstStrides[TIDL_DIM_MAX-1] = 1;                
                for(int32_t i = TIDL_DIM_MAX - 2; i >= 0; i--)
                {
                    srcStrides[i] = srcStrides[i+1] * inDims[i+1];
                    dstStrides[i] = dstStrides[i+1] * outDims[i+1];
                }
                int32_t cnt = 0;

                if(perm_final[0] == 0)
                {
                    cnt+=1;
                }
                if(perm_final[0]==0 && perm_final[1]==1)
                {
                    cnt+=2;
                } 
                for(int32_t i = 1; i < TIDL_DIM_MAX; i++)
                {
                    if(((perm_final[i] - perm_final[i-1]) == 1) && (srcStrides[perm_final[i]-1] == (inDims[perm_final[i]]*srcStrides[perm_final[i]]))
                        && (dstStrides[perm_final[i]-1] == outDims[perm_final[i]]*dstStrides[perm_final[i]]))
                    {
                        if((inDims[perm_final[i]] == 1) || (inDims[perm_final[i-1] == 1]))
                        {
                            cnt++;
                            continue;
                        }
                        else if((perm_final[0] != 0) && (inDims[perm_final[i]] == 1))
                        {
                            cnt++;
                        }
                    }   
                    else if(cnt == 2)
                    {
                        break;
                    }
                }
                if((cnt < 2) && ((perm_final[0] == TIDL_DIM_WIDTH) || (perm_final[1] == TIDL_DIM_WIDTH))) //[W, X, X, X, X, X] [X, W, X, X, X, X]
                {
                    return false;
                }
            }

            return true;            
        }
    ),
};