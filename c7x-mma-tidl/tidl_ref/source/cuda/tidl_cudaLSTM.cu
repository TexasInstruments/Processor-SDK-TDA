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


__device__ static float tidl_lstm_activation(float val, int32_t actType,
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
__global__ void TIDL_cudaLSTM_step_kernel(
    const Tin  *Xt,          
    const Tin  *Ht_prev,     
    const Tin  *Ct_prev,     
    const Tin  *W,           
    const Tin  *R,           
    const Tin  *B,           
    const Tin  *P,            
    Tin        *Ht_new,       
    Tin        *Ct_new,       
    Tout       *Yt,
    int32_t batch_size,
    int32_t input_size,
    int32_t hidden_size,
    int32_t input_forget,
    int32_t isClipSet,
    float   clip,
    int32_t act_f,           
    float   act_f_alpha,
    float   act_f_beta,
    int32_t act_g,          
    float   act_g_alpha,
    float   act_g_beta,
    int32_t act_h,          
    float   act_h_alpha,
    float   act_h_beta,
    const int32_t *seq_mask)
{
    int tid = blockIdx.x * blockDim.x + threadIdx.x;
    if (tid >= batch_size * hidden_size) return;

    int batchIdx  = tid / hidden_size;
    int hiddenIdx = tid % hidden_size;

    if (seq_mask != NULL && seq_mask[batchIdx] == 0)
    {
        /* Past sequence length: output zero, keep state unchanged */
        // need to fix this condition (check outOffset in ref code)
        if (Yt != NULL)
        {
            Yt[batchIdx * hidden_size + hiddenIdx] = (Tout)0.0f;
        }
        Ht_new[batchIdx * hidden_size + hiddenIdx] = Ht_prev[batchIdx * hidden_size + hiddenIdx];
        Ct_new[batchIdx * hidden_size + hiddenIdx] = Ct_prev[batchIdx * hidden_size + hiddenIdx];
        return;
    }

    // need to fix this condition (check outOffset in ref code)
    const Tin *xt     = Xt     + batchIdx * input_size;
    
    const Tin *ht_prv = Ht_prev + batchIdx * hidden_size;
    const Tin *ct_prv = Ct_prev + batchIdx * hidden_size;

    /* Compute linear part for each gate: Xt*(Wi^T) + Ht-1*(Ri^T) + Wbi + Rbi */
    float gate[4];   /* i(0), o(1), f(2), c(3) */

    for (int g = 0; g < 4; g++)
    {
        int row = g * hidden_size + hiddenIdx;
        float wx = (B != NULL) ? (float)B[g * hidden_size + hiddenIdx] : 0.0f;
        float rh = (B != NULL) ? (float)B[(4 + g) * hidden_size + hiddenIdx] : 0.0f;

        for (int i = 0; i < input_size; i++)
            wx += (float)xt[i] * (float)W[row * input_size + i];

        for (int i = 0; i < hidden_size; i++)
            rh += (float)ht_prv[i] * (float)R[row * hidden_size + i];

        gate[g] = wx + rh;
    }

    /* Add peephole contributions to i, f gates (before activation) */
    const Tin *Pi = (P != NULL) ? P + 0 * hidden_size : NULL;
    const Tin *Pf = (P != NULL) ? P + 2 * hidden_size : NULL;
    const Tin *Po = (P != NULL) ? P + 1 * hidden_size : NULL;
    float ct_prv_val = (float)ct_prv[hiddenIdx];

    if (Pi != NULL) gate[0] += (float)Pi[hiddenIdx] * ct_prv_val;
    if (Pf != NULL) gate[2] += (float)Pf[hiddenIdx] * ct_prv_val;

    float it = tidl_lstm_activation(gate[0], act_f, act_f_alpha, act_f_beta, isClipSet, clip);
    float ft = tidl_lstm_activation(gate[2], act_f, act_f_alpha, act_f_beta, isClipSet, clip);
    float ct = tidl_lstm_activation(gate[3], act_g, act_g_alpha, act_g_beta, isClipSet, clip);

    if (input_forget) ft = 1.0f - it;

    float ct_new = ft * ct_prv_val + it * ct;

    float gate_o = gate[1];
    if (Po != NULL) gate_o += (float)Po[hiddenIdx] * ct_new;

    float ot = tidl_lstm_activation(gate_o, act_f, act_f_alpha, act_f_beta, isClipSet, clip);

    /* Note: clipping NOT applied to Ct when computing ht (matches onnxruntime) */
    float ht_new_val = ot * tidl_lstm_activation(ct_new, act_h, act_h_alpha, act_h_beta, 0, clip);

    Ht_new[batchIdx * hidden_size + hiddenIdx] = (Tin)ht_new_val;
    Ct_new[batchIdx * hidden_size + hiddenIdx] = (Tin)ct_new;

    if (Yt != NULL)
    {
        Yt[batchIdx * hidden_size + hiddenIdx] = (Tout)ht_new_val;
    }
}

template<class Tin, class Tout>
int TIDL_cudaLSTM(
    Tin    *inPtr,
    Tin    *WPtr,
    Tin    *RPtr,
    Tin    *biasPtr,
    Tin    *initial_hPtr,
    Tin    *initial_cPtr,
    Tin    *peepholesPtr,
    Tout   *outPtr,
    int32_t batch_size,
    int32_t seq_length,
    int32_t input_size,
    int32_t hidden_size,
    int32_t num_directions,
    int32_t direction,
    int32_t layout,
    int32_t input_forget,
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
    int32_t incrementAxis,
    int32_t YhOffset,
    int32_t YcOffset)
{
    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(
        TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    int batchHidden = batch_size * hidden_size;
    int wSize = num_directions * 4 * hidden_size * input_size;
    int rSize = num_directions * 4 * hidden_size * hidden_size;
    int bSize = (biasPtr != NULL)    ? num_directions * 8 * hidden_size : 0;
    int pSize = (peepholesPtr != NULL)? num_directions * 3 * hidden_size : 0;
    int xSize = batch_size * seq_length * input_size;
    int ihSize = (initial_hPtr != NULL) ? num_directions * batchHidden : 0;
    int icSize = (initial_cPtr != NULL) ? num_directions * batchHidden : 0;
    int seqSize = (sequence_lens != NULL) ? batch_size : 0;
    int outSize = (int)((long)seq_length * outDim2Pitch);  /* total output Y elements */

    /* Y_h and Y_c positions are supplied by caller via YhOffset / YcOffset.
     * Total output buffer covers from 0 through YcOffset + one direction slice. */
    int yhSize = num_directions * batchHidden;
    int ycSize = num_directions * batchHidden;

    Tin    *dpW   = NULL, *dpR  = NULL, *dpB  = NULL, *dpP  = NULL;
    Tin    *dpX   = NULL;
    Tin    *dpIH  = NULL, *dpIC = NULL;
    Tin    *dpHt  = NULL, *dpCt = NULL;
    Tout   *dpOut = NULL;
    int32_t *dpSeqMask = NULL;

    bool needsCopyW = false, needsCopyR = false, needsCopyB = false, needsCopyP = false;

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), WPtr, (void**)&dpW, sizeof(Tin) * wSize) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), WPtr, (void**)&dpW, sizeof(Tin) * wSize) != IALG_EOK)
        {
            printf("LSTM: Unable to allocate W buffer\n");
            return IALG_EFAIL;
        }
        needsCopyW = true;
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), RPtr, (void**)&dpR, sizeof(Tin) * rSize) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), RPtr, (void**)&dpR, sizeof(Tin) * rSize) != IALG_EOK)
        {
            printf("LSTM: Unable to allocate R buffer\n");
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
                printf("LSTM: Unable to allocate B buffer\n");
                return IALG_EFAIL;
            }
            needsCopyB = true;
        }
    }

    if (pSize > 0)
    {
        if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), peepholesPtr, (void**)&dpP, sizeof(Tin) * pSize) != IALG_EOK)
        {
            if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), peepholesPtr, (void**)&dpP, sizeof(Tin) * pSize) != IALG_EOK)
            {
                printf("LSTM: Unable to allocate P buffer\n");
                return IALG_EFAIL;
            }
            needsCopyP = true;
        }
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), inPtr, (void**)&dpX, sizeof(Tin) * xSize) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), inPtr, (void**)&dpX, sizeof(Tin) * xSize) != IALG_EOK)
        {
            printf("LSTM: Unable to allocate input buffer\n");
            return IALG_EFAIL;
        }
    }

    if (ihSize > 0)
    {
        if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), initial_hPtr, (void**)&dpIH, sizeof(Tin) * ihSize) != IALG_EOK)
        {
            if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), initial_hPtr, (void**)&dpIH, sizeof(Tin) * ihSize) != IALG_EOK)
            {
                printf("LSTM: Unable to allocate initial_h buffer\n");
                return IALG_EFAIL;
            }
        }
    }

    if (icSize > 0)
    {
        if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), initial_cPtr, (void**)&dpIC, sizeof(Tin) * icSize) != IALG_EOK)
        {
            if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), initial_cPtr, (void**)&dpIC, sizeof(Tin) * icSize) != IALG_EOK)
            {
                printf("LSTM: Unable to allocate initial_c buffer\n");
                return IALG_EFAIL;
            }
        }
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), outPtr, (void**)&dpOut, sizeof(Tout) * (outSize + yhSize + ycSize)) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), outPtr, (void**)&dpOut, sizeof(Tout) * (outSize + yhSize + ycSize)) != IALG_EOK)
        {
            printf("LSTM: Unable to allocate output buffer\n");
            return IALG_EFAIL;
        }
    }

    /* ---- Transient scratch (rolling Ht/Ct state, no CPU counterpart) ------ */
    /* Double-buffered: kernel reads from dpHt/dpCt, writes to dpHt_new/dpCt_new.
     * Pointers are swapped after each timestep to avoid the read/write race
     * that occurs when the same buffer is passed as both Ht_prev and Ht_new. */
    Tin *dpHt_new = NULL, *dpCt_new = NULL;
    checkCudaErr(cudaMalloc(&dpHt,     sizeof(Tin) * batchHidden));
    checkCudaErr(cudaMalloc(&dpHt_new, sizeof(Tin) * batchHidden));
    checkCudaErr(cudaMalloc(&dpCt,     sizeof(Tin) * batchHidden));
    checkCudaErr(cudaMalloc(&dpCt_new, sizeof(Tin) * batchHidden));
    if (seqSize > 0) 
        checkCudaErr(cudaMalloc(&dpSeqMask, sizeof(int32_t) * seqSize));

    if (needsCopyW)
        checkCudaErr(cudaMemcpyAsync(dpW, WPtr, sizeof(Tin) * wSize, cudaMemcpyHostToDevice, stream));
    if (needsCopyR)
        checkCudaErr(cudaMemcpyAsync(dpR, RPtr, sizeof(Tin) * rSize, cudaMemcpyHostToDevice, stream));
    if (needsCopyB && dpB != NULL)
        checkCudaErr(cudaMemcpyAsync(dpB, biasPtr, sizeof(Tin) * bSize, cudaMemcpyHostToDevice, stream));
    if (needsCopyP && dpP != NULL)
        checkCudaErr(cudaMemcpyAsync(dpP, peepholesPtr, sizeof(Tin) * pSize, cudaMemcpyHostToDevice, stream));

    checkCudaErr(cudaMemcpyAsync(dpX, inPtr, sizeof(Tin) * xSize, cudaMemcpyHostToDevice, stream));
    if (dpIH != NULL)
        checkCudaErr(cudaMemcpyAsync(dpIH, initial_hPtr, sizeof(Tin) * ihSize, cudaMemcpyHostToDevice, stream));
    if (dpIC != NULL)
        checkCudaErr(cudaMemcpyAsync(dpIC, initial_cPtr, sizeof(Tin) * icSize, cudaMemcpyHostToDevice, stream));

    for (int dirIdx = 0; dirIdx < num_directions; dirIdx++)
    {
        int act_f_idx = dirIdx * 3 + 0;
        int act_g_idx = dirIdx * 3 + 1;
        int act_h_idx = dirIdx * 3 + 2;

        Tin *dpW_dir = dpW + dirIdx * 4 * hidden_size * input_size;
        Tin *dpR_dir = dpR + dirIdx * 4 * hidden_size * hidden_size;
        Tin *dpB_dir = (dpB != NULL) ? dpB + dirIdx * 8 * hidden_size : NULL;
        Tin *dpP_dir = (dpP != NULL) ? dpP + dirIdx * 3 * hidden_size : NULL;

        /* Initialize Ht, Ct */
        if (dpIH != NULL)
            cudaMemcpyAsync(dpHt, dpIH + dirIdx * batchHidden, sizeof(Tin) * batchHidden, cudaMemcpyDeviceToDevice, stream);
        else
            cudaMemsetAsync(dpHt, 0, sizeof(Tin) * batchHidden, stream);

        if (dpIC != NULL)
            cudaMemcpyAsync(dpCt, dpIC + dirIdx * batchHidden, sizeof(Tin) * batchHidden, cudaMemcpyDeviceToDevice, stream);
        else
            cudaMemsetAsync(dpCt, 0, sizeof(Tin) * batchHidden, stream);

        int isReverse = 0;
        if (direction == TIDL_RecurrentReverse) isReverse = 1;

        /* Iterate over time steps */
        for (int timeIdx = 0; timeIdx < seq_length; timeIdx++)
        {
            int timeStep = isReverse ? (seq_length - 1 - timeIdx) : timeIdx;

            /* Pointer to Yt output slice */
            Tout *dpYt = NULL;
            if (layout == 0)
            {
                dpYt = dpOut + timeStep * outDim2Pitch + dirIdx * outChPitch;
            }
            else
            {
                dpYt = dpOut + dirIdx * outLinePitch + timeStep * outChPitch;
            }

            /* Pointer to Xt slice in GPU */
            Tin *dpXt = NULL;
            if (layout == 0)
                dpXt = dpX + timeStep * inChPitch;
            else
                dpXt = dpX + timeStep * inLinePitch;

            /* Build per-batch sequence mask for this timestep */
            if (seqSize > 0 && dpSeqMask != NULL)
            {
                /* Host-side mask computation; copy to GPU */
                int32_t mask[128]; /* max batch_size assumed <= 128 for reference */
                for (int b = 0; b < batch_size && b < 128; b++)
                    mask[b] = (timeStep < sequence_lens[b]) ? 1 : 0;
                cudaMemcpyAsync(dpSeqMask, mask, sizeof(int32_t) * batch_size, cudaMemcpyHostToDevice, stream);
            }

            long launchSize = (long)batch_size * hidden_size;
            TIDL_cudaLSTM_step_kernel<Tin, Tout>
                <<<GRID_SIZE(launchSize, THREADS_PER_BLOCK), THREADS_PER_BLOCK, 0, stream>>>(
                    dpXt, dpHt, dpCt,
                    dpW_dir, dpR_dir, dpB_dir, dpP_dir,
                    dpHt_new, dpCt_new,    /* write to separate output buffers */
                    dpYt,
                    batch_size, input_size, hidden_size,
                    input_forget, isClipSet, clip,
                    activations[act_f_idx], activation_alpha[act_f_idx], activation_beta[act_f_idx],
                    activations[act_g_idx], activation_alpha[act_g_idx], activation_beta[act_g_idx],
                    activations[act_h_idx], activation_alpha[act_h_idx], activation_beta[act_h_idx],
                    (seqSize > 0) ? dpSeqMask : NULL);

            /* Swap buffers: new state becomes prev state for next timestep */
            Tin *tmpH = dpHt; dpHt = dpHt_new; dpHt_new = tmpH;
            Tin *tmpC = dpCt; dpCt = dpCt_new; dpCt_new = tmpC;
        }

        /* Copy final Ht → Y_h, Ct → Y_c using caller-supplied offsets */
        cudaMemcpyAsync(dpOut + YhOffset + dirIdx * batchHidden, dpHt, sizeof(Tin) * batchHidden, cudaMemcpyDeviceToDevice, stream);
        cudaMemcpyAsync(dpOut + YcOffset + dirIdx * batchHidden, dpCt, sizeof(Tin) * batchHidden, cudaMemcpyDeviceToDevice, stream);
    }

    /* D2H: copy full output (Y + Y_h + Y_c) */
    checkCudaErr(cudaMemcpyAsync(outPtr, dpOut,
        sizeof(Tout) * (outSize + yhSize + ycSize), cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));

    /* Free only transient scratch buffers (managed buffers are owned by memory manager) */
    cudaFree(dpHt);     cudaFree(dpHt_new);
    cudaFree(dpCt);     cudaFree(dpCt_new);
    if (dpSeqMask != NULL) cudaFree(dpSeqMask);

    return 0;
}

/* ======================================================================== */
/*  Explicit template instantiations                                        */
/* ======================================================================== */

template int TIDL_cudaLSTM<float, float>(float*, float*, float*, float*, float*, float*, float*, float*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, float, int32_t*, float*, float*, int32_t*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
