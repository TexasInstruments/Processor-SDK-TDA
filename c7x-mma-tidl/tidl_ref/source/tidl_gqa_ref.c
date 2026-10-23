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
 *  \file tidl_gqa_ref.c
 *  \brief Reference kernel for GroupQueryAttention (com.microsoft contrib op).
 *
 *  Implements grouped/multi-query attention with KV cache, optional rotary
 *  embedding, softcap and local sliding-window, for prefill and decode.
 ----------------------------------------------------------------------------
*/

#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include "tidl_alg_utils.h"
#include <math.h>
#include "tidl_gqa_ref.h"

/* Apply rotary embedding in place over a single head vector of headSize. */
static void TIDL_gqaApplyRotary(float *vec, const float *cosC, const float *sinC,
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

int32_t TIDL_GroupQueryAttentionRefProcess(TIDL_NetworkCommonParams *commonParams,
                               sTIDL_AlgLayer_t *algLayer,
                               sTIDL_Layer_t *tidlLayer,
                               void *inPtrs[],
                               void *outPtrs[],
                               int32_t layerIdx)
{
    (void)layerIdx;
    sTIDL_Network_t *net = commonParams->net;
    sTIDL_GroupQueryAttentionParams_t *p = &tidlLayer->layerParams.gqaParams;

    int32_t numHeads   = p->numHeads;
    int32_t kvHeads    = p->kvNumHeads;
    int32_t hs         = p->headSize;
    int32_t group      = numHeads / kvHeads;
    float   scale      = p->scale;
    float   softcap    = p->softcap;
    int32_t window     = p->localWindowSize;
    int32_t doRotary   = p->doRotary;
    /* When set, past_key/past_value already carry the full
     * [past ++ this-call's-new] history (an upstream TIDL_CacheLayer appended
     * before this layer ran) and rotation already happened upstream
     * (doRotary must be 0) - so there is nothing to copy/append/rotate here,
     * and no present_key/present_value to write; attention reads K/V
     * directly out of past_key/past_value. */
    int32_t cacheManaged = p->cacheManaged;

    sTIDL_DataParams_t *qParams = &net->TIDLLayers[algLayer->inLayerIdx[0]].outData;
    int32_t batch       = qParams->dimValues[TIDL_DIM_DIM2];
    int32_t seq         = qParams->dimValues[TIDL_DIM_HEIGHT];
    int32_t qBatchPitch = qParams->pitch[TIDL_DIM2_PITCH];
    int32_t qLinePitch  = qParams->pitch[TIDL_LINE_PITCH];

    /* Past KV cache: present output = [pastSeq tokens] + [seq new tokens].
     * pastSeq is the live valid length (a TIDL_CacheLayer producer keeps its
     * pitch frozen at compiled capacity while dimValues[axis] shrinks/grows),
     * so batch/head strides below must come from pkParams/pvParams->pitch,
     * never from a dimValues product. */
    int32_t pastIdx = p->isPackedQKV ? 1 : 3;
    int32_t pastSeq = 0;
    sTIDL_DataParams_t *pkParams = NULL;
    sTIDL_DataParams_t *pvParams = NULL;
    if (p->hasPast && algLayer->inLayerIdx[pastIdx] >= 0)
    {
        pkParams = &net->TIDLLayers[algLayer->inLayerIdx[pastIdx]].outData;
        pvParams = &net->TIDLLayers[algLayer->inLayerIdx[pastIdx + 1]].outData;
        int32_t pkH = pkParams->dimValues[TIDL_DIM_HEIGHT];
        if (pkH > 0) pastSeq = pkH;
    }

    /* seqlens_k: per-batch past sequence lengths for variable-length batches.
     * Buffer index is stored in params (computed at import from varTensorIndices)
     * because optional ONNX inputs before index 5 may be absent, shifting TIDL positions.
     * When absent, all batch elements use the uniform pastSeq.
     *
     * ONNX RT convention: seqlens_k[b] = total_valid_positions - 1 = (past + seq) - 1.
     * Therefore: past_seqlen[b] = seqlens_k[b] + 1 - seq.
     * This formula works for both prefill (seq > 1) and decode (seq == 1). */
    int32_t seqlensKIdx = p->seqlensKBufIdx;
    const int32_t *seqlensK = (seqlensKIdx >= 0 && seqlensKIdx < tidlLayer->numInBufs &&
                                algLayer->inLayerIdx[seqlensKIdx] >= 0)
                              ? (const int32_t *)inPtrs[seqlensKIdx] : NULL;

    /* Compute maxPastSeq across batch for output buffer sizing. Not needed in
     * cacheManaged mode: pastSeq (from past_key's own HEIGHT dim) already IS
     * totalSeq, since the upstream Cache layer appended this call's new
     * tokens before this layer ran. Cache tracks a single shared valid length
     * for the whole batch, so cacheManaged mode assumes uniform sequence
     * length across batch elements (seqlens_k per-batch divergence is not
     * supported together with an internally-managed cache). */
    int32_t maxPastSeq = pastSeq;
    if (!cacheManaged && seqlensK) {
        for (int32_t b = 0; b < batch; b++) {
            int32_t ps = seqlensK[b] + 1 - seq;
            if (ps > maxPastSeq) maxPastSeq = ps;
        }
    }
    int32_t totalSeq = cacheManaged ? pastSeq : (seq + maxPastSeq);

    const float *qIn = (const float *)inPtrs[0];
    const float *kIn = p->isPackedQKV ? qIn : (const float *)inPtrs[1];
    const float *vIn = p->isPackedQKV ? qIn : (const float *)inPtrs[2];

    sTIDL_DataParams_t *kParams = p->isPackedQKV ? qParams : &net->TIDLLayers[algLayer->inLayerIdx[1]].outData;
    sTIDL_DataParams_t *vParams = p->isPackedQKV ? qParams : &net->TIDLLayers[algLayer->inLayerIdx[2]].outData;
    int32_t kBatchPitch = kParams->pitch[TIDL_DIM2_PITCH];
    int32_t kLinePitch  = kParams->pitch[TIDL_LINE_PITCH];
    int32_t vBatchPitch = vParams->pitch[TIDL_DIM2_PITCH];
    int32_t vLinePitch  = vParams->pitch[TIDL_LINE_PITCH];

    int32_t qOff = 0;
    int32_t kOff = p->isPackedQKV ? (numHeads * hs) : 0;
    int32_t vOff = p->isPackedQKV ? ((numHeads + kvHeads) * hs) : 0;

    const float *cosC = (doRotary && p->cosCache) ? (const float *)get_int8_t_pointer((int8_t *)net, p->cosCache) : NULL;
    const float *sinC = (doRotary && p->sinCache) ? (const float *)get_int8_t_pointer((int8_t *)net, p->sinCache) : NULL;
    int32_t rotaryDim = p->rotaryDim > 0 ? p->rotaryDim : hs;

    float *out = (float *)outPtrs[0];
    /* Packed output [1,1,B,1,1,perBatchElems]: each batch row holds
     * [attn_out | present_key | present_value] contiguously, or just
     * [attn_out] alone when cacheManaged (present_key/value are not produced -
     * Cache already owns that state). */
    int32_t perBatchElems = cacheManaged ? p->attnOutElems
                                          : (p->attnOutElems + p->presentKeyElems + p->presentValueElems);
    int32_t outStride = numHeads * hs;

    float *qv = (float *)malloc(sizeof(float) * hs);
    float *kv = (float *)malloc(sizeof(float) * hs);
    float *sc = (float *)malloc(sizeof(float) * totalSeq);
    if (!qv || !kv || !sc) { free(qv); free(kv); free(sc); return TIDL_ERR_FAILURE; }

    const float *pkIn = (p->hasPast && pastSeq > 0) ? (const float *)inPtrs[pastIdx]     : NULL;
    const float *pvIn = (p->hasPast && pastSeq > 0) ? (const float *)inPtrs[pastIdx + 1] : NULL;

    /* Build present_key/value: copy past cache then append current tokens.
     * Skipped when cacheManaged: past_key/past_value already hold the full
     * history (Cache appended this call's new, already-rotated tokens before
     * this layer ran), so attention below reads pkIn/pvIn directly. */
    if (!cacheManaged)
    {
        for (int32_t b = 0; b < batch; b++)
        {
            int32_t pastSeq_b = seqlensK ? (seqlensK[b] + 1 - seq) : pastSeq;
            float *pK = out + b * perBatchElems + p->attnOutElems;
            float *pV = out + b * perBatchElems + p->attnOutElems + p->presentKeyElems;
            for (int32_t kh = 0; kh < kvHeads; kh++)
            {
                /* Copy past KV into slots [0..pastSeq_b-1]. */
                if (pkIn)
                {
                    const float *pkSrc = pkIn + b * pkParams->pitch[TIDL_DIM2_PITCH] + kh * pkParams->pitch[TIDL_CHANNEL_PITCH];
                    const float *pvSrc = pvIn + b * pvParams->pitch[TIDL_DIM2_PITCH] + kh * pvParams->pitch[TIDL_CHANNEL_PITCH];
                    float *pkDst = pK + kh * totalSeq * hs;
                    float *pvDst = pV + kh * totalSeq * hs;
                    for (int32_t d = 0; d < pastSeq_b * hs; d++) { pkDst[d] = pkSrc[d]; pvDst[d] = pvSrc[d]; }
                }
                /* Append current KV at slots [pastSeq_b..pastSeq_b+seq-1]. */
                for (int32_t s = 0; s < seq; s++)
                {
                    const float *ksrc = kIn + b * kBatchPitch + s * kLinePitch + kOff + kh * hs;
                    const float *vsrc = vIn + b * vBatchPitch + s * vLinePitch + vOff + kh * hs;
                    for (int32_t d = 0; d < hs; d++) kv[d] = ksrc[d];
                    if (cosC) TIDL_gqaApplyRotary(kv, cosC, sinC, pastSeq_b + s, rotaryDim, p->rotaryInterleaved);
                    float *pkd = pK + (kh * totalSeq + pastSeq_b + s) * hs;
                    float *pvd = pV + (kh * totalSeq + pastSeq_b + s) * hs;
                    for (int32_t d = 0; d < hs; d++) { pkd[d] = kv[d]; pvd[d] = vsrc[d]; }
                }
            }
        }
    }

    for (int32_t b = 0; b < batch; b++)
    {
        /* cacheManaged: totalSeq already equals this batch's full history
         * length (uniform across batch, per the Cache single-valid-length
         * limitation noted above), so pastSeq_b = totalSeq - seq. */
        int32_t pastSeq_b = cacheManaged ? (totalSeq - seq) : (seqlensK ? (seqlensK[b] + 1 - seq) : pastSeq);
        float *pK = cacheManaged ? NULL : (out + b * perBatchElems + p->attnOutElems);
        float *pV = cacheManaged ? NULL : (out + b * perBatchElems + p->attnOutElems + p->presentKeyElems);
        /* cacheManaged: read K/V straight out of past_key/past_value (Cache's
         * already-assembled buffer) via pitch-based addressing, same
         * convention as pkSrc/pvSrc above - no local pK/pV copy exists. */
        const float *pkBase = cacheManaged ? (pkIn + b * pkParams->pitch[TIDL_DIM2_PITCH]) : NULL;
        const float *pvBase = cacheManaged ? (pvIn + b * pvParams->pitch[TIDL_DIM2_PITCH]) : NULL;
        float *outAttn = out + b * perBatchElems;
        for (int32_t s = 0; s < seq; s++)
        {
            for (int32_t h = 0; h < numHeads; h++)
            {
                int32_t kh = h / group;
                const float *qsrc = qIn + b * qBatchPitch + s * qLinePitch + qOff + h * hs;
                for (int32_t d = 0; d < hs; d++) qv[d] = qsrc[d];
                if (cosC) TIDL_gqaApplyRotary(qv, cosC, sinC, pastSeq_b + s, rotaryDim, p->rotaryInterleaved);

                int32_t lo = 0;
                int32_t hi = pastSeq_b + s;     /* causal: valid past keys for this batch + current 0..s */
                if (window > 0 && hi - window + 1 > lo) lo = hi - window + 1;
                float mx = -1e30f;
                for (int32_t j = lo; j <= hi; j++)
                {
                    const float *kd = cacheManaged ? (pkBase + kh * pkParams->pitch[TIDL_CHANNEL_PITCH] + j * hs)
                                                    : (pK + (kh * totalSeq + j) * hs);
                    float dot = 0.0f;
                    for (int32_t d = 0; d < hs; d++) dot += qv[d] * kd[d];
                    dot *= scale;
                    if (softcap > 0.0f) dot = softcap * tanhf(dot / softcap);
                    sc[j] = dot;
                    if (dot > mx) mx = dot;
                }
                float sum = 0.0f;
                for (int32_t j = lo; j <= hi; j++) { sc[j] = expf(sc[j] - mx); sum += sc[j]; }
                float inv = sum > 0.0f ? 1.0f / sum : 0.0f;
                float *od = outAttn + s * outStride + h * hs;
                for (int32_t d = 0; d < hs; d++) od[d] = 0.0f;
                for (int32_t j = lo; j <= hi; j++)
                {
                    float w = sc[j] * inv;
                    const float *vd = cacheManaged ? (pvBase + kh * pvParams->pitch[TIDL_CHANNEL_PITCH] + j * hs)
                                                    : (pV + (kh * totalSeq + j) * hs);
                    for (int32_t d = 0; d < hs; d++) od[d] += w * vd[d];
                }
            }
        }
    }

    free(qv); free(kv); free(sc);
    return IALG_EOK;
}
