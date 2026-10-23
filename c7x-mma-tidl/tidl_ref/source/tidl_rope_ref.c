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
/** ----------------------------------------------------------------------------
 *  \file tidl_rope.c
 *  \brief Reference kernel for RotaryEmbedding (ONNX standard operator).
 *
 *  Implements rotary positional embeddings for query/key tensors.
 *  Supports both 3D and 4D input tensors, with optional position_ids.
 ----------------------------------------------------------------------------
*/

#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include "tidl_alg_utils.h"
#include <math.h>
#include "tidl_rope_ref.h"

/* Apply rotary embedding in place over a single vector.
 * vec: input/output vector
 * cosC: cosine cache values
 * sinC: sine cache values
 * pos: position index
 * rotaryDim: dimension to apply rotation (must be even)
 * interleaved: 0 = consecutive [x1,x2,x3,x4], 1 = interleaved [x1,x3,x2,x4]
 */
static void TIDL_ropeApplyRotary(float *vec, const float *cosC, const float *sinC,
                                 int32_t pos, int32_t rotaryDim, int32_t interleaved)
{
    int32_t half = rotaryDim / 2;
    const float *cosp = cosC + (pos * half);
    const float *sinp = sinC + (pos * half);
    for (int32_t i = 0; i < half; i++)
    {
        float c = cosp[i];
        float s = sinp[i];
        if (interleaved)
        {
            float a = vec[2 * i];
            float b = vec[2 * i + 1];
            vec[2 * i]     = a * c - b * s;
            vec[2 * i + 1] = a * s + b * c;
        }
        else
        {
            float a = vec[i];
            float b = vec[i + half];
            vec[i]        = a * c - b * s;
            vec[i + half] = a * s + b * c;
        }
    }
}

/**
 * @brief This is main function perform RMS normalization on float elementTypes
 *
 * @tparam Tin  : template for input
 * @tparam Tout : template for output
 * @param inPtr : Input pointer on which rmsNorm is applied
 * @param outPtr : Output pointer after rmsNorm operation
 * @param intAlgHandle : tidl algorithm handle
 * @param layerIdx : index of the current layer
 * @param params : copy of rmsNorm parameters
 * @param algLayer : Pointer to the layer specific parameters
 * @param inDataParams : parameters of the input data buffer
 * @param outDataParams : parameters of the output data buffer
 * @return  TIDL_SUCCESS - Successful
 *          TIDL_ERR_FAILURE - Unspecified error
 */
template<class Tin, class Tout> static int32_t TIDL_refRotaryEmbeddingCoreFloat(Tin *inPtr,
                                                                       Tout *outPtr,
                                                                       sTIDL_RotaryEmbeddingParams_t *params,
                                                                       const sTIDL_Layer_t *tidlLayer,
                                                                       const sTIDL_DataParams_t *inDataParams,
                                                                       const sTIDL_DataParams_t *outDataParams,
                                                                       void *inPtrs[])
{
    int32_t rotaryDim = params->rotationDim;
    int32_t interleaved = params->interleaved;

    /* Extract inputs from inPtrs array */
    /* ONNX RotaryEmbedding input order: inPtrs[0] = X, inPtrs[1] = cos_cache, inPtrs[2] = sin_cache,
     * inPtrs[3] = position_ids (optional) */
    Tin *x_in = (Tin *)inPtr;
    float *cos_cache = (float *)inPtrs[1];
    float *sin_cache = (float *)inPtrs[2];
    int32_t *position_ids = (tidlLayer->numInBufs > 3) ? (int32_t *)inPtrs[3] : NULL;
    Tout *y_out = (Tout *)outPtr;

    /* Check if position_ids is present based on numInBufs */
    /* numInBufs == 3: no position_ids, numInBufs == 4: position_ids present */
    int32_t has_position_ids = (tidlLayer->numInBufs == 4) ? 1 : 0;

    /* Live position from a seqlens_k source (e.g. wired to the same seqlens_k
     * buffer a downstream cache-managed GQA layer uses), takes priority over
     * the baked position_ids buffer. */
    int32_t seqlensKIdx = params->seqlensKBufIdx;
    const int32_t *seqlensK = (seqlensKIdx >= 0 && seqlensKIdx < tidlLayer->numInBufs)
                              ? (const int32_t *)inPtrs[seqlensKIdx] : NULL;

    /* Input shape is (batch, seq_len, num_heads, head_size) */
    int32_t batch = inDataParams->dimValues[TIDL_DIM_DIM2];
    int32_t seq_len = inDataParams->dimValues[TIDL_DIM_NUMCH];
    int32_t numHeads = inDataParams->dimValues[TIDL_DIM_HEIGHT];
    int32_t head_size = inDataParams->dimValues[TIDL_DIM_WIDTH];
    int32_t total_elements = batch * numHeads * seq_len * head_size;

    /* Allocate temporary buffer for vector processing */
    float *vec_tmp = (float *)malloc(sizeof(float) * head_size);
    if (!vec_tmp)
    {
        return TIDL_ERR_FAILURE;
    }

    /* Copy input to output first */
    for (int32_t i = 0; i < total_elements; i++)
    {
        y_out[i] = x_in[i];
    }

    /* Process: (batch, seq_len, num_heads, head_size) */
    for (int32_t b = 0; b < batch; b++)
    {
        for (int32_t s = 0; s < seq_len; s++)
        {
            for (int32_t h = 0; h < numHeads; h++)
            {
                /* Determine position index: live seqlens_k takes priority, then
                 * baked position_ids, then the sequential fallback. */
                int32_t pos;
                if (seqlensK != NULL)
                {
                    /* Same convention as GQA: seqlens_k[b] = total_valid_positions - 1
                     * = (past + seq_len) - 1, so past_len[b] = seqlens_k[b] + 1 - seq_len,
                     * and this token's absolute position = past_len[b] + s. */
                    pos = seqlensK[b] + 1 - seq_len + s;
                }
                else if (has_position_ids && position_ids != NULL)
                {
                    /* position_ids is present: use it to index into caches */
                    pos = (int32_t)position_ids[b * seq_len + s];
                }
                else
                {
                    /* position_ids is NOT present: cos_cache/sin_cache are 3D
                     * (batch_size, sequence_length, half), flattened row-major,
                     * so the cache row for (b, s) is b * seq_len + s, not s alone. */
                    pos = b * seq_len + s;
                }

                int32_t offset = ((b * seq_len + s) * numHeads + h) * head_size;

                /* Copy head vector to temporary buffer */
                for (int32_t d = 0; d < head_size; d++)
                {
                    vec_tmp[d] = (float)y_out[offset + d];
                }

                /* Apply rotary embedding with the determined position */
                TIDL_ropeApplyRotary(vec_tmp, cos_cache, sin_cache, pos, rotaryDim, interleaved);

                /* Copy back to output */
                for (int32_t d = 0; d < head_size; d++)
                {
                    y_out[offset + d] = (Tout)vec_tmp[d];
                }
            }
        }
    }

    free(vec_tmp);
    return IALG_EOK;
}

int32_t TIDL_RotaryEmbeddingRefProcess(const sTIDL_Layer_t *tidlLayer,
                                    sTIDL_RotaryEmbeddingParams_t *params,
                                    void *inPtrs[],
                                    void *outPtr,
                                    const sTIDL_DataParams_t *inDataParams,
                                    const sTIDL_DataParams_t *outDataParams)
{
    int32_t status = TIDL_SUCCESS;

    if (inDataParams->elementType == TIDL_SinglePrecFloat)
    {
        status = TIDL_refRotaryEmbeddingCoreFloat<float32_tidl, float32_tidl>((float32_tidl *)inPtrs[0],
                                            (float32_tidl *)outPtr,
                                            params,
                                            tidlLayer,
                                            inDataParams,
                                            outDataParams,
                                            inPtrs);
    }
    else
    {
        status = TIDL_ERR_FAILURE;
    }

    TIDL_L1DandL2CacheWbInv();
    return status;
}
