/*
 * Copyright (c) {2022 - 2026} Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 * Limited License.
 * Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 * license under copyrights and patents it now or hereafter owns or controls to make,
 * have made, use, import, offer to sell and sell ("Utilize") this software subject to the
 * terms herein.
 *
 * DISCLAIMER.
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 */

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <cuda_runtime.h>
#include "tidl_cuda.h"
#include "itidl_ti.h"
#include "tidl_cudaUtilities.cu"
#include "tidl_cuda_mem_manager.h"

__device__ static float tidl_rnn_activation(float val, int32_t actType,
                                             float alpha, float beta,
                                             int32_t isClipSet, float clip)
{
    if (isClipSet)
    {
        val = fmaxf(-clip, fminf(clip, val));
    }
    if      (actType == TIDL_RelU)        return fmaxf(0.0f, val);
    else if (actType == TIDL_Sigmoid)     return 1.0f / (1.0f + expf(-val));
    else if (actType == TIDL_Tanh)        return tanhf(val);
    else if (actType == TIDL_LeakyReLU)   return (val >= 0.0f) ? val : alpha * val;
    else if (actType == TIDL_HardSigmoid) return fmaxf(0.0f, fminf(1.0f, alpha * val + beta));
    else if (actType == TIDL_ELU)         return (val >= 0.0f) ? val : alpha * (expf(val) - 1.0f);
    return val;
}

template<class Tin, class Tout>
__global__ void TIDL_cudaRNN_step_kernel(
    const Tin  *Xt,          
    const Tin  *Ht_prev,     
    const Tin  *W,           
    const Tin  *R,           
    const Tin  *B,           
    Tin        *Ht_new,      
    Tout       *Yt,          
    int32_t batch_size,
    int32_t input_size,
    int32_t hidden_size,
    int32_t isClipSet,
    float   clip,
    int32_t act_f,
    float   act_f_alpha,
    float   act_f_beta,
    const int32_t *seq_mask)
{
    int tid = blockIdx.x * blockDim.x + threadIdx.x;
    if (tid >= batch_size * hidden_size) return;

    int batchIdx  = tid / hidden_size;
    int hiddenIdx = tid % hidden_size;

    if (seq_mask != NULL && seq_mask[batchIdx] == 0)
    {
        if (Yt != NULL)
            Yt[batchIdx * hidden_size + hiddenIdx] = (Tout)0.0f;
        Ht_new[batchIdx * hidden_size + hiddenIdx] = Ht_prev[batchIdx * hidden_size + hiddenIdx];
        return;
    }

    const Tin *xt     = Xt     + batchIdx * input_size;
    const Tin *ht_prv = Ht_prev + batchIdx * hidden_size;

    const Tin *Wb = (B != NULL) ? B + 0 * hidden_size : NULL;
    const Tin *Rb = (B != NULL) ? B + 1 * hidden_size : NULL;

    int row = hiddenIdx;
    float wx = (Wb != NULL) ? (float)Wb[hiddenIdx] : 0.0f;
    float rh = (Rb != NULL) ? (float)Rb[hiddenIdx] : 0.0f;

    for (int i = 0; i < input_size; i++)
        wx += (float)xt[i] * (float)W[row * input_size + i];

    for (int i = 0; i < hidden_size; i++)
        rh += (float)ht_prv[i] * (float)R[row * hidden_size + i];

    float ht_val = tidl_rnn_activation(wx + rh, act_f, act_f_alpha, act_f_beta, isClipSet, clip);

    Ht_new[batchIdx * hidden_size + hiddenIdx] = (Tin)ht_val;
    if (Yt != NULL)
        Yt[batchIdx * hidden_size + hiddenIdx] = (Tout)ht_val;
}

template<class Tin, class Tout>
int TIDL_cudaRNN(
    Tin    *inPtr,
    Tin    *WPtr,
    Tin    *RPtr,
    Tin    *biasPtr,
    Tin    *initial_hPtr,
    Tout   *outPtr,
    int32_t batch_size,
    int32_t seq_length,
    int32_t input_size,
    int32_t hidden_size,
    int32_t num_directions,
    int32_t direction,
    int32_t layout,
    int32_t isClipSet,
    float   clip,
    int32_t *activations,       
    float   *activation_alpha,  
    float   *activation_beta,   
    int32_t *sequence_lens,
    int32_t inChPitch,
    int32_t inLinePitch,
    int32_t outDim2Pitch,
    int32_t outChPitch,
    int32_t outLinePitch,
    int32_t incrementAxis)
{
    if (TIDL_cudaGetThreadManager() == NULL || !TIDL_cudaGetThreadManager()->isInitialized)
    {
        printf("ERROR: CUDA Memory Manager not initialized in TIDL_cudaRNN\n");
        return -1;
    }

    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(
        TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    int batchHidden = batch_size * hidden_size;
    int wSize = num_directions * hidden_size * input_size;
    int rSize = num_directions * hidden_size * hidden_size;
    int bSize = (biasPtr != NULL)     ? num_directions * 2 * hidden_size : 0;
    int xSize = batch_size * seq_length * input_size;
    int ihSize = (initial_hPtr != NULL) ? num_directions * batchHidden : 0;
    int seqSize = (sequence_lens != NULL) ? batch_size : 0;
    int outSize = (int)((long)seq_length * outDim2Pitch);
    int yhSize  = num_directions * batchHidden;

    Tin    *dpW  = NULL, *dpR  = NULL, *dpB  = NULL;
    Tin    *dpX  = NULL, *dpIH = NULL;
    Tin    *dpHt = NULL;
    Tout   *dpOut = NULL;
    int32_t *dpSeqMask = NULL;

    bool needsCopyW = false, needsCopyR = false, needsCopyB = false;

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), WPtr, (void**)&dpW, sizeof(Tin) * wSize) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), WPtr, (void**)&dpW, sizeof(Tin) * wSize) != IALG_EOK)
        {   
            printf("RNN: Unable to allocate W buffer\n");
            return IALG_EFAIL;
        }
        needsCopyW = true;
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), RPtr, (void**)&dpR, sizeof(Tin) * rSize) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), RPtr, (void**)&dpR, sizeof(Tin) * rSize) != IALG_EOK)
        {
            printf("RNN: Unable to allocate R buffer\n");
            return IALG_EFAIL;
        }
        needsCopyR = true;
    }

    if (bSize > 0)
    {
        if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), biasPtr, (void**)&dpB, sizeof(Tin) * bSize) != IALG_EOK)
        {
            if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), biasPtr, (void**)&dpB, sizeof(Tin) * bSize) != IALG_EOK)
            {
                printf("RNN: Unable to allocate B buffer\n");
                return IALG_EFAIL;
            }
            needsCopyB = true;
        }
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), inPtr, (void**)&dpX, sizeof(Tin) * xSize) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), inPtr, (void**)&dpX, sizeof(Tin) * xSize) != IALG_EOK)
        {
            printf("RNN: Unable to allocate input buffer\n");
            return IALG_EFAIL;
        }
    }

    if (ihSize > 0)
    {
        if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), initial_hPtr, (void**)&dpIH, sizeof(Tin) * ihSize) != IALG_EOK)
        {
            if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), initial_hPtr, (void**)&dpIH, sizeof(Tin) * ihSize) != IALG_EOK)
            {
                printf("RNN: Unable to allocate initial_h buffer\n");
                return IALG_EFAIL;
            }
        }
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), outPtr, (void**)&dpOut, sizeof(Tout) * (outSize + yhSize)) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), outPtr, (void**)&dpOut, sizeof(Tout) * (outSize + yhSize)) != IALG_EOK)
        {
            printf("RNN: Unable to allocate output buffer\n");
            return IALG_EFAIL;
        }
    }

    /* ---- Transient scratch (rolling Ht state, no CPU counterpart) --------- */
    /* Double-buffered to avoid read/write race when Ht_prev == Ht_new pointer. */
    Tin *dpHt_new = NULL;
    cudaMalloc(&dpHt,     sizeof(Tin) * batchHidden);
    cudaMalloc(&dpHt_new, sizeof(Tin) * batchHidden);
    if (seqSize > 0) cudaMalloc(&dpSeqMask, sizeof(int32_t) * seqSize);

    if (needsCopyW)
        checkCudaErr(cudaMemcpyAsync(dpW, WPtr, sizeof(Tin) * wSize, cudaMemcpyHostToDevice, stream));
    if (needsCopyR)
        checkCudaErr(cudaMemcpyAsync(dpR, RPtr, sizeof(Tin) * rSize, cudaMemcpyHostToDevice, stream));
    if (needsCopyB && dpB != NULL)
        checkCudaErr(cudaMemcpyAsync(dpB, biasPtr, sizeof(Tin) * bSize, cudaMemcpyHostToDevice, stream));

    checkCudaErr(cudaMemcpyAsync(dpX, inPtr, sizeof(Tin) * xSize, cudaMemcpyHostToDevice, stream));
    if (dpIH != NULL)
        checkCudaErr(cudaMemcpyAsync(dpIH, initial_hPtr, sizeof(Tin) * ihSize, cudaMemcpyHostToDevice, stream));

    for (int dirIdx = 0; dirIdx < num_directions; dirIdx++)
    {
        Tin *dpW_dir = dpW + dirIdx * hidden_size * input_size;
        Tin *dpR_dir = dpR + dirIdx * hidden_size * hidden_size;
        Tin *dpB_dir = (dpB != NULL) ? dpB + dirIdx * 2 * hidden_size : NULL;

        if (dpIH != NULL)
            cudaMemcpyAsync(dpHt, dpIH + dirIdx * batchHidden, sizeof(Tin) * batchHidden, cudaMemcpyDeviceToDevice, stream);
        else
            cudaMemsetAsync(dpHt, 0, sizeof(Tin) * batchHidden, stream);

        int isReverse = (direction == TIDL_RecurrentReverse) ? 1 : 0;

        for (int timeIdx = 0; timeIdx < seq_length; timeIdx++)
        {
            int timeStep = isReverse ? (seq_length - 1 - timeIdx) : timeIdx;

            Tin *dpXt = (layout == 0)
                ? dpX + timeStep * inChPitch
                : dpX + timeStep * inLinePitch;

            if (seqSize > 0 && dpSeqMask != NULL)
            {
                int32_t mask[128];
                for (int b = 0; b < batch_size && b < 128; b++)
                    mask[b] = (timeStep < sequence_lens[b]) ? 1 : 0;
                cudaMemcpyAsync(dpSeqMask, mask, sizeof(int32_t) * batch_size, cudaMemcpyHostToDevice, stream);
            }

            Tout *dpYt = NULL;
            if (layout == 0)
                dpYt = dpOut + timeStep * outDim2Pitch + dirIdx * outChPitch;
            else
                dpYt = dpOut + dirIdx * outLinePitch + timeStep * outChPitch;

            long launchSize = (long)batch_size * hidden_size;
            TIDL_cudaRNN_step_kernel<Tin, Tout>
                <<<GRID_SIZE(launchSize, THREADS_PER_BLOCK), THREADS_PER_BLOCK, 0, stream>>>(
                    dpXt, dpHt, dpW_dir, dpR_dir, dpB_dir,
                    dpHt_new, dpYt,    /* write to separate output buffer */
                    batch_size, input_size, hidden_size,
                    isClipSet, clip,
                    activations[dirIdx], activation_alpha[dirIdx], activation_beta[dirIdx],
                    (seqSize > 0) ? dpSeqMask : NULL);

            /* Swap buffers: new state becomes prev state for next timestep */
            Tin *tmpH = dpHt; dpHt = dpHt_new; dpHt_new = tmpH;
        }

        /* Copy final Ht → Y_h */
        int32_t yhOffset = outDim2Pitch * seq_length + dirIdx * batchHidden;
        cudaMemcpyAsync(dpOut + yhOffset, dpHt, sizeof(Tin) * batchHidden, cudaMemcpyDeviceToDevice, stream);
    }

    checkCudaErr(cudaMemcpyAsync(outPtr, dpOut,
        sizeof(Tout) * (outSize + yhSize), cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));

    /* Free only transient scratch buffers (managed buffers are owned by memory manager) */
    cudaFree(dpHt); cudaFree(dpHt_new);
    if (dpSeqMask != NULL) cudaFree(dpSeqMask);

    return 0;
}

/* ======================================================================== */
/*  Explicit template instantiations                                        */
/* ======================================================================== */

template int TIDL_cudaRNN<float, float>(float*, float*, float*, float*, float*, float*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, float, int32_t*, float*, float*, int32_t*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
