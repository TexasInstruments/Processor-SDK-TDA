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

/**
 *  \file tidl_gru_ref.c
 *
 *  \brief GRU process calls & ref implementation
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include <math.h>
#include <limits>
#include "tidl_alg_utils.h"
#include "tidl_gru_ref.h"
#include "tidl_forceNegativeTest.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                  Internal/Private Function Declarations                    */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

static void TIDL_updateGRUMemorySizes(const TIDL_LayerSpecificParams *layerSpecificParams,
                                      const TIDL_NetworkCommonParams *commonParams,
                                      int32_t layerIdx,
                                      int32_t *scratchDataSizeOut,
                                      int32_t *outDataSizeOut,
                                      int32_t memorySize[TIDL_LAYER_MEMORY_MAX])
{
  int32_t outBatchPitch;
  int32_t scratchDataSize;
  int32_t paramMemSize = 0;
  int32_t outDataSize = TIDL_refGetOutDataSize(commonParams, layerSpecificParams, layerIdx);
  sTIDL_DataParams_t *inDataParams = TIDL_getDataParams(commonParams->net,
                                                        commonParams->net->TIDLLayers[layerIdx].inData[0]);

  outBatchPitch = commonParams->net->TIDLLayers[layerIdx].outData.pitch[TIDL_ROI_PITCH];

  /* ScratchSize: Ht[batch_size * hidden_size] + gates[3 * hidden_size] */
  int32_t hidden_size = commonParams->net->TIDLLayers[layerIdx].layerParams.gruParams.hidden_size;
  int32_t layout = commonParams->net->TIDLLayers[layerIdx].layerParams.gruParams.layout;
  int32_t batch_size;
  if (layout == 0)
  {
    /* X: [seq_length, batch_size, input_size] */
    batch_size = inDataParams->dimValues[TIDL_DIM_HEIGHT];
  }
  else
  {
    /* X: [batch_size, seq_length, input_size] */
    batch_size = inDataParams->dimValues[TIDL_DIM_NUMCH];
  }
  /* Scratch: Ht[batch_size * hidden_size] + gates[3 * hidden_size] */
  scratchDataSize = ((batch_size * hidden_size) + (3 * hidden_size)) * TIDL_getDatElementSize(inDataParams->elementType);

  if (scratchDataSizeOut != NULL)
  {
    *scratchDataSizeOut = scratchDataSize;
  }

  if (outDataSizeOut != NULL)
  {
    *outDataSizeOut = outDataSize;
  }

  memorySize[TIDL_LAYER_MEMORY_SCRATCH] = scratchDataSize + TIDL_ALIGNMENT_SIZE;
  memorySize[TIDL_LAYER_MEMORY_PERSISTENT] = paramMemSize + TIDL_ALIGNMENT_SIZE;
  memorySize[TIDL_LAYER_MEMORY_OUTPUT] = outDataSize + TIDL_ALIGNMENT_SIZE;
}

int32_t TIDL_gruRefAlloc(const TIDL_LayerSpecificParams *layerSpecificParams,
                          const TIDL_NetworkCommonParams *commonParams,
                          int32_t layerIdx,
                          int32_t memorySize[TIDL_LAYER_MEMORY_MAX])
{
  int32_t status = IALG_EOK;

  TIDL_updateGRUMemorySizes(layerSpecificParams, commonParams, layerIdx, NULL, NULL, memorySize);

  return status;
}

int32_t TIDL_gruRefInit(const TIDL_LayerSpecificParams *layerSpecificParams,
                         const TIDL_NetworkCommonParams *commonParams,
                         sTIDL_AlgLayer_t *algLayer,
                         int32_t layerIdx,
                         uint8_t *memory[TIDL_LAYER_MEMORY_MAX],
                         int32_t memorySize[TIDL_LAYER_MEMORY_MAX],
                         void **outPtr)
{
  int32_t status = IALG_EOK;
  int32_t currOffset = 0;
  int32_t scratchDataSize;
  int32_t outDataSize;

  TIDL_updateGRUMemorySizes(layerSpecificParams, commonParams, layerIdx, &scratchDataSize, &outDataSize, memorySize);
  /* LDRA_JUSTIFY_START
  <metric start> branch <metric end>
  <justification start>
  Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
  Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
  This does not affect runtime behavior or safety.
  <justification end> */
  if (outPtr != NULL )
  {
    /* Point to NULL if output of network */
    if (TIDL_isOutDataBuff(commonParams->net,
                           commonParams->net->TIDLLayers[layerIdx].outData.dataId,
                           commonParams->createParams->currLayersGroupId) == 1)
    {
      *outPtr = NULL;
    }
    else if (outDataSize != 0) /* Not output of network and outDataSize > 0 */
    {
      TIDL_AllocatePtr((intptr_t)memory[TIDL_LAYER_MEMORY_OUTPUT],
                       &currOffset,
                       outDataSize,
                       128,
                       outPtr);
    }
    else /* OutDataSize > 0 */
    {
      *outPtr = NULL;
    }
  }
  /* LDRA_JUSTIFY_END */

  currOffset = 0;
  TIDL_AllocatePtr((intptr_t)memory[TIDL_LAYER_MEMORY_SCRATCH],
                   &currOffset,
                   scratchDataSize,
                   128,
                   &algLayer->scratchMem);
  algLayer->scratchSize = scratchDataSize;

  return status;
}

/**
 * Scalar replica of exp_highprecision_vec: same range reduction + 5th-order Taylor
 */
static inline float32_tidl TIDL_gruExpScalar(float32_tidl x)
{
  float32_tidl ln2     = 0.693147180559945f;
  float32_tidl ln2_inv = 1.442695040888963f;

  float32_tidl n_raw = x * ln2_inv;
  float32_tidl n_adj = (n_raw >= 0.0f) ? (n_raw + 0.5f) : (n_raw - 0.5f);
  int32_t n          = (int32_t) n_adj;

  float32_tidl n_f = (float32_tidl) n;
  float32_tidl f   = x - n_f * ln2;

  float32_tidl exp_f = 1.0f + f * (1.0f + f * (0.5f + f * (0.16666667f + f * (0.04166667f + f * 0.00833333f))));

  int32_t scale_bits = (n + 127) << 23;
  float32_tidl scale;
  memcpy(&scale, &scale_bits, sizeof(float32_tidl));

  float32_tidl result = exp_f * scale;

  if (n > 127)  result = 3.4e38f;
  if (n < -126) result = 0.0f;

  return result;
}

/**
 * @brief Sigmoid activation function for GRU
 * @param x: input value
 * @return sigmoid(x)
 */
static inline float32_tidl TIDL_gruSigmoid(float32_tidl x)
{
  float32_tidl result;
  if (x >= 0)
  {
    result = 1 / (1 + exp(-x));
  }
  else
  {
    result = exp(x) / (1 + exp(x));
  }

  return result;
}

/**
 * @brief Sigmoid activation function implemented using taylor series expansion
 * @param x: input value
 * @return sigmoid(x)
 */
static inline float32_tidl TIDL_gruSigmoidTaylor(float32_tidl x)
{
  x = (x > TIDL_GRU_SIGMOID_BOUND) ? TIDL_GRU_SIGMOID_BOUND : x;
  x = (x < -TIDL_GRU_SIGMOID_BOUND) ? -TIDL_GRU_SIGMOID_BOUND : x;

  int32_t neg_mask         = (x < 0.0f);
  float32_tidl abs_x       = neg_mask ? -x : x;
  float32_tidl exp_neg_abs = TIDL_gruExpScalar(-abs_x);
  float32_tidl denom       = 1.0f + exp_neg_abs;

  /* RCPSP + 2 Newton-Raphson iterations — matches __recip + NR in sigmoid_vec */
  float32_tidl recip = __recip(denom);
  recip = recip * (2.0f - denom * recip);
  recip = recip * (2.0f - denom * recip);

  float32_tidl sig_neg = 1.0f - recip;

  return neg_mask ? sig_neg : recip;
}

/**
 * @brief Tanh activation function for GRU implemented using sigmoid to improve numerical stability
 * @param x: input value
 * @return tanh(x)
 */
static inline float32_tidl TIDL_gruTanh(float32_tidl x)
{
  float32_tidl result;
  result = 2.0f * TIDL_gruSigmoid(2.0f * x) - 1.0f;

  return result;
}

/**
 * @brief Tanh activation function implemented using taylor series expansion
 * @param x: input value
 * @return tanh(x)
 */
static inline float32_tidl TIDL_gruTanhTaylor(float32_tidl x)
{
  x = (x > TIDL_GRU_TANH_BOUND) ? TIDL_GRU_TANH_BOUND : x;
  x = (x < -TIDL_GRU_TANH_BOUND) ? -TIDL_GRU_TANH_BOUND : x;

  int32_t   neg_mask     = (x < 0.0f);
  float32_tidl abs_x     = neg_mask ? -x : x;
  float32_tidl exp_neg2x = TIDL_gruExpScalar(-2.0f * abs_x);
  float32_tidl numer     = 1.0f - exp_neg2x;
  float32_tidl denom     = 1.0f + exp_neg2x;

  /* RCPSP + 2 Newton-Raphson iterations — matches __recip + NR in tanh_vec */
  float32_tidl recip = __recip(denom);
  recip       = recip * (2.0f - denom * recip);
  recip       = recip * (2.0f - denom * recip);

  float32_tidl tanh_pos = numer * recip;

  return neg_mask ? -tanh_pos : tanh_pos;
}

/**
 * @brief Apply activation function
 *
 * @param val              : input value
 * @param actType          : activation type
 * @param activation_alpha : alpha value for parametric activations
 * @param activation_beta  : beta value for parametric activations
 * @param isClipSet        : flag to indicate whether clip value is set or not
 * @param clip             : clip value
 * @param useTaylor        : flag to indicate whether to use taylor series based implementation
 * @return activated value
 */
static inline float32_tidl TIDL_gruActivation(float32_tidl val, int32_t actType, float32_tidl activation_alpha,
                                              float32_tidl activation_beta, int8_t isClipSet, float32_tidl clip,
                                              int8_t useTaylor)
{
  if (isClipSet == 1)
  {
    val = (val > clip) ? clip : val;
    val = (val < -clip) ? -clip : val;
  }

  float32_tidl activatedVal;
  if (actType == TIDL_RelU)
  {
    activatedVal = (val > 0) ? val : 0;
  }
  else if (actType == TIDL_Sigmoid)
  {
    activatedVal = (useTaylor == 1) ? TIDL_gruSigmoidTaylor(val) : TIDL_gruSigmoid(val);
  }
  else if (actType == TIDL_Tanh)
  {
    activatedVal = (useTaylor == 1) ? TIDL_gruTanhTaylor(val) : TIDL_gruTanh(val);
  }
  else if (actType == TIDL_LeakyReLU)
  {
    activatedVal = (val >= 0) ? val : (activation_alpha * val);
  }
  else if (actType == TIDL_HardSigmoid)
  {
    activatedVal = (val * activation_alpha) + activation_beta;
    activatedVal = (activatedVal < 0) ? 0 : activatedVal;
    activatedVal = (activatedVal > 1) ? 1 : activatedVal;
  }
  else if (actType == TIDL_ELU)
  {
    activatedVal = (val >= 0) ? val : (activation_alpha * (exp(val) - 1));
  }
  else
  {
    /* Unsupported activation, return input as is */
    activatedVal = val;
  }

  return activatedVal;
}

/* Volatile accumulators, bias pre-loaded — matches C7x per-lane FP order */
template<class Tin> static void TIDL_gruGateComputeTaylor(
    const Tin *xt, const Tin *ht_prev,
    const Tin *W_dir, const Tin *R_dir, const Tin *B_dir,
    Tin *gate_z, Tin *gate_r, Tin *gate_h,
    int32_t linear_before_reset,
    int32_t f_act_type, float32_tidl f_alpha, float32_tidl f_beta,
    int8_t isClipSet, float32_tidl clip,
    int32_t input_size, int32_t hidden_size)
{
  /* [3*hidden_size, input_size],  gate order: z, r, h */
  const Tin *Wz  = W_dir + 0 * hidden_size * input_size;
  const Tin *Wr  = W_dir + 1 * hidden_size * input_size;
  const Tin *Wh  = W_dir + 2 * hidden_size * input_size;

  /* [3*hidden_size, hidden_size], gate order: z, r, h */
  const Tin *Rz  = R_dir + 0 * hidden_size * hidden_size;
  const Tin *Rr  = R_dir + 1 * hidden_size * hidden_size;
  const Tin *Rh  = R_dir + 2 * hidden_size * hidden_size;

  /* [6*hidden_size] = [Wbz,Wbr,Wbh,Rbz,Rbr,Rbh] */
  const Tin *Wbz = (B_dir != NULL) ? B_dir + 0 * hidden_size : NULL;
  const Tin *Wbr = (B_dir != NULL) ? B_dir + 1 * hidden_size : NULL;
  const Tin *Wbh = (B_dir != NULL) ? B_dir + 2 * hidden_size : NULL;
  const Tin *Rbz = (B_dir != NULL) ? B_dir + 3 * hidden_size : NULL;
  const Tin *Rbr = (B_dir != NULL) ? B_dir + 4 * hidden_size : NULL;
  const Tin *Rbh = (B_dir != NULL) ? B_dir + 5 * hidden_size : NULL;

  /* Compute update gate (pre-activation) = Wbz + Xt*(Wz^T) + Rbz + Ht-1*(Rz^T) */
  for (int32_t hiddenIdx = 0; hiddenIdx < hidden_size; hiddenIdx++)
  {
    volatile float32_tidl wx_z = (Wbz != NULL) ? Wbz[hiddenIdx] : 0.0f;
    for (int32_t inputIdx = 0; inputIdx < input_size; inputIdx++)
    {
      wx_z += (float32_tidl)xt[inputIdx] * Wz[hiddenIdx * input_size + inputIdx];
    }
    volatile float32_tidl rh_z = (Rbz != NULL) ? Rbz[hiddenIdx] : 0.0f;
    for (int32_t inputIdx = 0; inputIdx < hidden_size; inputIdx++)
    {
      rh_z += ht_prev[inputIdx] * Rz[hiddenIdx * hidden_size + inputIdx];
    }
    gate_z[hiddenIdx] = (float32_tidl)wx_z + (float32_tidl)rh_z;
  }

  /* Compute reset gate = f(Wbr + Xt*(Wr^T) + Rbr + Ht-1*(Rr^T))
   * Must be fully computed before gate_h (linear_before_reset=0 uses all gate_r[j]) */
  for (int32_t hiddenIdx = 0; hiddenIdx < hidden_size; hiddenIdx++)
  {
    volatile float32_tidl wx_r = (Wbr != NULL) ? Wbr[hiddenIdx] : 0.0f;
    for (int32_t inputIdx = 0; inputIdx < input_size; inputIdx++)
    {
      wx_r += (float32_tidl)xt[inputIdx] * Wr[hiddenIdx * input_size + inputIdx];
    }
    volatile float32_tidl rh_r = (Rbr != NULL) ? Rbr[hiddenIdx] : 0.0f;
    for (int32_t inputIdx = 0; inputIdx < hidden_size; inputIdx++)
    {
      rh_r += ht_prev[inputIdx] * Rr[hiddenIdx * hidden_size + inputIdx];
    }
    gate_r[hiddenIdx] = TIDL_gruActivation((float32_tidl)wx_r + (float32_tidl)rh_r,
                                        f_act_type, f_alpha, f_beta, isClipSet, clip, 1);
  }

  /* Compute hidden gate (pre-activation) */
  for (int32_t hiddenIdx = 0; hiddenIdx < hidden_size; hiddenIdx++)
  {
    volatile float32_tidl wx_h = (Wbh != NULL) ? Wbh[hiddenIdx] : 0.0f;
    for (int32_t inputIdx = 0; inputIdx < input_size; inputIdx++)
    {
      wx_h += (float32_tidl)xt[inputIdx] * Wh[hiddenIdx * input_size + inputIdx];
    }
    if (linear_before_reset == 0)
    {
      /* ht = (wx_h + Rbh + (rt(.)Ht-1)*(Rh^T)) */
      volatile float32_tidl rh_h = (Rbh != NULL) ? Rbh[hiddenIdx] : 0.0f;
      for (int32_t inputIdx = 0; inputIdx < hidden_size; inputIdx++)
      {
        rh_h += Rh[hiddenIdx * hidden_size + inputIdx] * (gate_r[inputIdx] * ht_prev[inputIdx]);
      }
      gate_h[hiddenIdx] = (float32_tidl)wx_h + (float32_tidl)rh_h;
    }
    else
    {
      /* ht = (wx_h + rt*(Rbh + (Ht-1)*(Rh^T))) */
      volatile float32_tidl rh = (Rbh != NULL) ? Rbh[hiddenIdx] : 0.0f;
      for (int32_t inputIdx = 0; inputIdx < hidden_size; inputIdx++)
      {
        rh += Rh[hiddenIdx * hidden_size + inputIdx] * ht_prev[inputIdx];
      }
      gate_h[hiddenIdx] = (float32_tidl)wx_h + gate_r[hiddenIdx] * (float32_tidl)rh;
    }
  }
}

/* Standard gate computation as per onnxruntime */
template<class Tin> static void TIDL_gruGateComputeStandard(
    const Tin *xt, const Tin *ht_prev,
    const Tin *W_dir, const Tin *R_dir, const Tin *B_dir,
    Tin *gate_z, Tin *gate_r, Tin *gate_h,
    int32_t linear_before_reset,
    int32_t f_act_type, float32_tidl f_alpha, float32_tidl f_beta,
    int8_t isClipSet, float32_tidl clip,
    int32_t input_size, int32_t hidden_size)
{
  /* W_dir: [3*hidden_size, input_size],  gate order: z, r, h */
  const Tin *Wz  = W_dir + 0 * hidden_size * input_size;
  const Tin *Wr  = W_dir + 1 * hidden_size * input_size;
  const Tin *Wh  = W_dir + 2 * hidden_size * input_size;

   /* R_dir: [3*hidden_size, hidden_size], gate order: z, r, h */
  const Tin *Rz  = R_dir + 0 * hidden_size * hidden_size;
  const Tin *Rr  = R_dir + 1 * hidden_size * hidden_size;
  const Tin *Rh  = R_dir + 2 * hidden_size * hidden_size;

  /* B_dir: [6*hidden_size] = [Wbz,Wbr,Wbh,Rbz,Rbr,Rbh] */
  const Tin *Wbz = (B_dir != NULL) ? B_dir + 0 * hidden_size : NULL;
  const Tin *Wbr = (B_dir != NULL) ? B_dir + 1 * hidden_size : NULL;
  const Tin *Wbh = (B_dir != NULL) ? B_dir + 2 * hidden_size : NULL;
  const Tin *Rbz = (B_dir != NULL) ? B_dir + 3 * hidden_size : NULL;
  const Tin *Rbr = (B_dir != NULL) ? B_dir + 4 * hidden_size : NULL;
  const Tin *Rbh = (B_dir != NULL) ? B_dir + 5 * hidden_size : NULL;

  /* Compute update gate (pre-activation): zt = (Xt*(Wz^T) + Ht-1*(Rz^T) + Wbz + Rbz) */
  for (int32_t hiddenIdx = 0; hiddenIdx < hidden_size; hiddenIdx++)
  {
    float32_tidl sum = 0.0f;
    for (int32_t inputIdx = 0; inputIdx < input_size; inputIdx++)
    {
      sum += (float32_tidl)xt[inputIdx] * Wz[hiddenIdx * input_size + inputIdx];
    }
    for (int32_t inputIdx = 0; inputIdx < hidden_size; inputIdx++)
    {
      sum += ht_prev[inputIdx] * Rz[hiddenIdx * hidden_size + inputIdx];
    }
    if (Wbz != NULL)
    {
      sum += Wbz[hiddenIdx];
    }
    if (Rbz != NULL)
    {
      sum += Rbz[hiddenIdx];
    }
    gate_z[hiddenIdx] = sum;
  }

  /* Compute reset gate: rt = f(Xt*(Wr^T) + Ht-1*(Rr^T) + Wbr + Rbr) */
  for (int32_t hiddenIdx = 0; hiddenIdx < hidden_size; hiddenIdx++)
  {
    float32_tidl sum = 0.0f;
    for (int32_t inputIdx = 0; inputIdx < input_size; inputIdx++)
    {
      sum += (float32_tidl)xt[inputIdx] * Wr[hiddenIdx * input_size + inputIdx];
    }
    for (int32_t inputIdx = 0; inputIdx < hidden_size; inputIdx++)
    {
      sum += ht_prev[inputIdx] * Rr[hiddenIdx * hidden_size + inputIdx];
    }
    if (Wbr != NULL)
    {
      sum += Wbr[hiddenIdx];
    }
    if (Rbr != NULL)
    {
      sum += Rbr[hiddenIdx];
    }
    gate_r[hiddenIdx] = TIDL_gruActivation(sum, f_act_type, f_alpha, f_beta, isClipSet, clip, 0);
  }

  /* Hidden candidate h (pre-activation) */
  for (int32_t hiddenIdx = 0; hiddenIdx < hidden_size; hiddenIdx++)
  {
    float32_tidl sum_Wx = 0.0f;
    float32_tidl sum_Rh = 0.0f;
    for (int32_t inputIdx = 0; inputIdx < input_size; inputIdx++)
    {
      sum_Wx += (float32_tidl)xt[inputIdx] * Wh[hiddenIdx * input_size + inputIdx];
    }
    if (linear_before_reset == 0)
    {
      /* (rt (.) Ht-1)*(Rh^T) + Wbh + Rbh */
      for (int32_t inputIdx = 0; inputIdx < hidden_size; inputIdx++)
      {
        sum_Rh += (gate_r[inputIdx] * ht_prev[inputIdx]) * Rh[hiddenIdx * hidden_size + inputIdx];
      }
      float32_tidl sum = sum_Wx + sum_Rh;
      if (Wbh != NULL)
      {
        sum += Wbh[hiddenIdx];
      }
      if (Rbh != NULL)
      {
        sum += Rbh[hiddenIdx];
      }
      gate_h[hiddenIdx] = sum;
    }
    else
    {
      /* Ht-1*(Rh^T) + Rbh, then rt(.) applied to full sum */
      for (int32_t inputIdx = 0; inputIdx < hidden_size; inputIdx++)
      {
        sum_Rh += ht_prev[inputIdx] * Rh[hiddenIdx * hidden_size + inputIdx];
      }
      if (Rbh != NULL)
      {
        sum_Rh += Rbh[hiddenIdx];
      }
      float32_tidl sum = sum_Wx + gate_r[hiddenIdx] * sum_Rh;
      if (Wbh != NULL)
      {
        sum += Wbh[hiddenIdx];
      }
      gate_h[hiddenIdx] = sum;
    }
  }
}

/**
 * @brief Float kernel for GRU operator (ONNX GRU-14)
 *
 * Implements the ONNX GRU-14 equations:
 *   zt = f(Xt*(Wz^T) + Ht-1*(Rz^T) + Wbz + Rbz)
 *   rt = f(Xt*(Wr^T) + Ht-1*(Rr^T) + Wbr + Rbr)
 *
 *   When linear_before_reset = 0 (default):
 *     ht = g(Xt*(Wh^T) + (rt (.) Ht-1)*(Rh^T) + Wbh + Rbh)
 *
 *   When linear_before_reset = 1:
 *     ht = g(Xt*(Wh^T) + rt (.) (Ht-1*(Rh^T) + Rbh) + Wbh)
 *
 *   Ht = (1 - zt) (.) ht + zt (.) Ht-1
 *
 * where f and g are activation functions and (.) is element-wise product.
 * ONNX gate order in W, R, B tensors: z (update), r (reset), h (hidden)
 *
 * @param inPtr            : Input pointer (X tensor)
 * @param WPtr             : Weights pointer (W tensor)
 * @param RPtr             : Recurrence weights pointer (R tensor)
 * @param initial_hPtr     : Initial hidden state pointer (initial_h tensor), may be NULL
 * @param outPtr           : Output pointer (Y tensor)
 * @param intAlgHandle     : tidl algorithm handle
 * @param layerIdx         : index of the current layer
 * @param params           : GRU layer parameters
 * @param algLayer         : Pointer to the layer specific parameters
 * @param inDataParams     : parameters of the input data buffer
 * @param WParams          : parameters of the weights buffer
 * @param RParams          : parameters of the recurrence weights buffer
 * @param initial_hParams  : parameters of the initial hidden state buffer
 * @param outDataParams    : parameters of the output data buffer
 * @param useTaylor        : flag to indicate whether to use taylor series based implementation for activations
 * @return  IALG_EOK       - Successful
 *          IALG_EFAIL     - Unspecified error
 */
template<class Tin, class Tout> static int32_t TIDL_refGRUCoreFloat(
    Tin *inPtr,
    Tin *WPtr,
    Tin *RPtr,
    Tin *biasPtr,
    Tin *initial_hPtr,
    Tout *outPtr,
    TIDL_Handle intAlgHandle,
    int32_t layerIdx,
    sTIDL_GRUParams_t *params,
    sTIDL_AlgLayer_t *algLayer,
    const sTIDL_DataParams_t *inDataParams,
    const sTIDL_DataParams_t *WParams,
    const sTIDL_DataParams_t *RParams,
    const sTIDL_DataParams_t *biasParams,
    const sTIDL_DataParams_t *initial_hParams,
    const sTIDL_DataParams_t *outDataParams,
    int8_t useTaylor)
{
  int32_t status = IALG_EOK;
  sTIDL_Network_t *net = intAlgHandle->createParams->net;
  int32_t hidden_size = params->hidden_size;
  int32_t direction = params->direction;
  int32_t layout = params->layout;
  int32_t linear_before_reset = params->linear_before_reset;
  int8_t isClipSet = params->isClipSet;
  float32_tidl clip = params->clip;

  /* Determine number of directions */
  int32_t num_directions = 1;
  #if defined TIDL_COVERAGE_DEAD_CODE
  if (direction == TIDL_RecurrentBidirectional)
  {
    num_directions = 2;
  }
  #endif

  /* Extract batch_size, seq_length, input_size from input dimensions based on layout */
  int32_t batch_size, seq_length, input_size;
  if (layout == 0)
  {
    /* X: [seq_length, batch_size, input_size] */
    seq_length = inDataParams->dimValues[TIDL_DIM_NUMCH];
    batch_size = inDataParams->dimValues[TIDL_DIM_HEIGHT];
    input_size = inDataParams->dimValues[TIDL_DIM_WIDTH];
  }
  else
  {
    /* X: [batch_size, seq_length, input_size] */
    batch_size = inDataParams->dimValues[TIDL_DIM_NUMCH];
    seq_length = inDataParams->dimValues[TIDL_DIM_HEIGHT];
    input_size = inDataParams->dimValues[TIDL_DIM_WIDTH];
  }

  int32_t *sequence_lens = NULL;
  if (params->sequence_lens != 0)
  {
    sequence_lens = (int32_t *)get_int8_t_pointer((int8_t *)net, params->sequence_lens);
  }

  /* Scratch layout: Ht[batch_size * hidden_size] + gates[3 * hidden_size] */
  int32_t requiredScratchSize = ((batch_size * hidden_size) + (3 * hidden_size)) * sizeof(Tin);
  if (algLayer->scratchSize < requiredScratchSize)
  {
    tidl_printf(0, "Memory for TIDL_refGRUCoreFloat accumulator is not sufficient exiting...\n");
    status = IALG_EFAIL;
  }

  if (status == IALG_EOK)
  {
    int32_t batchHidden = batch_size * hidden_size;
    Tin *scratch = (Tin *)algLayer->scratchMem;
    Tin *Ht    = scratch;
    Tin *gates = Ht + batchHidden;  /* gates[3 * hidden_size]: z, r, h */

    Tin  *X = inPtr;
    Tout *Y = outPtr;

    /* Calculate output tensor offset for Y_h */
    int32_t incrementAxis = params->incrementAxis;
    int32_t YhOffset = outDataParams->pitch[incrementAxis] * seq_length;
    Tout *Y_h = Y + YhOffset;

    int32_t dirIdx, timeIdx, batchIdx, hiddenIdx;

    for (dirIdx = 0; dirIdx < num_directions; dirIdx++)
    {
      /* Activations for current direction */
      int32_t activation_f_idx = dirIdx * 2 + 0;
      int32_t activation_g_idx = dirIdx * 2 + 1;

      /* W: [num_directions, 3*hidden_size, input_size], gate order: z, r, h */
      Tin *W_dir = WPtr + dirIdx * 3 * hidden_size * input_size;
      /* R: [num_directions, 3*hidden_size, hidden_size], gate order: z, r, h */
      Tin *R_dir = RPtr + dirIdx * 3 * hidden_size * hidden_size;
      /* B: [num_directions, 6*hidden_size] = [Wbz, Wbr, Wbh, Rbz, Rbr, Rbh] */
      Tin *B_dir = (biasPtr != NULL) ? biasPtr + dirIdx * 6 * hidden_size : NULL;

      /*
       * Initialize Ht from initial_h
       * initial_h: [num_directions, batch_size, hidden_size]
       */
      if (initial_hPtr != NULL)
      {
        Tin *initH_dir = (Tin *)initial_hPtr + dirIdx * batchHidden;
        for (hiddenIdx = 0; hiddenIdx < batchHidden; hiddenIdx++)
        {
          Ht[hiddenIdx] = initH_dir[hiddenIdx];
        }
      }
      else
      {
        for (hiddenIdx = 0; hiddenIdx < batchHidden; hiddenIdx++)
        {
          Ht[hiddenIdx] = 0.0f;
        }
      }

      /* Iterate over time steps */
      for (timeIdx = 0; timeIdx < seq_length; timeIdx++)
      {
        /*
         * Determine time step based on direction:
         * Forward:       dirIdx 0 -> forward traversal
         * Reverse:       dirIdx 0 -> reverse traversal
         * Bidirectional: dirIdx 0 -> forward, dirIdx 1 -> reverse
         */
        int32_t isReverse = 0;
        if (direction == TIDL_RecurrentReverse)
        {
          isReverse = 1;
        }
        #if defined TIDL_COVERAGE_DEAD_CODE
        else if ((direction == TIDL_RecurrentBidirectional) && (dirIdx == 1))
        {
          isReverse = 1;
        }
        else
        {
          /* Not reachable */
        }
        #endif
        int32_t timeStep = (isReverse == 0) ? timeIdx : (seq_length - 1 - timeIdx);

        for (batchIdx = 0; batchIdx < batch_size; batchIdx++)
        {
          /*
           * If sequence_lens is provided, for time steps past sequence_lens[batchIdx],
           * output Y = 0 and Ht remains unchanged.
           */
          int32_t batchSeqLen = seq_length;
          if (sequence_lens != NULL)
          {
            batchSeqLen = sequence_lens[batchIdx];
            if (batchSeqLen > seq_length)
            {
              batchSeqLen = seq_length;
            }
          }

          if (timeStep >= batchSeqLen)
          {
            if (Y != NULL)
            {
              int32_t outOffset;
              if (layout == 0)
              {
                outOffset = timeStep * outDataParams->pitch[TIDL_DIM2_PITCH] +
                            dirIdx * outDataParams->pitch[TIDL_CHANNEL_PITCH] +
                            batchIdx * outDataParams->pitch[TIDL_LINE_PITCH];
              }
              else
              {
                outOffset = batchIdx * outDataParams->pitch[TIDL_DIM2_PITCH] +
                            timeStep * outDataParams->pitch[TIDL_CHANNEL_PITCH] +
                            dirIdx * outDataParams->pitch[TIDL_LINE_PITCH];
              }
              Tout *yt = Y + outOffset;
              for (hiddenIdx = 0; hiddenIdx < hidden_size; hiddenIdx++)
              {
                yt[hiddenIdx] = (Tout)0.0f;
              }
            }
            continue;
          }

          /*
           * Input dims: layout=0: [NUMCH=seq, HEIGHT=batch, WIDTH=input]
           *             layout=1: [NUMCH=batch, HEIGHT=seq, WIDTH=input]
           */
          int32_t inOffset;
          if (layout == 0)
          {
            inOffset = timeStep * inDataParams->pitch[TIDL_CHANNEL_PITCH] +
                       batchIdx * inDataParams->pitch[TIDL_LINE_PITCH];
          }
          else
          {
            inOffset = batchIdx * inDataParams->pitch[TIDL_CHANNEL_PITCH] +
                       timeStep * inDataParams->pitch[TIDL_LINE_PITCH];
          }
          Tin *xt = X + inOffset;

          Tin *ht_prev = Ht + batchIdx * hidden_size;

          /* Gate scratch pointers: [3, hidden_size] -> z, r, h */
          Tin *gate_z = gates + 0 * hidden_size;
          Tin *gate_r = gates + 1 * hidden_size;
          Tin *gate_h = gates + 2 * hidden_size;

          if (useTaylor)
          {
            TIDL_gruGateComputeTaylor<Tin>(xt, ht_prev,
                                           W_dir, R_dir, B_dir,
                                           gate_z, gate_r, gate_h,
                                           linear_before_reset,
                                           params->activations[activation_f_idx],
                                           params->activation_alpha[activation_f_idx],
                                           params->activation_beta[activation_f_idx],
                                           isClipSet, clip,
                                           input_size, hidden_size);
          }
          else
          {
            TIDL_gruGateComputeStandard<Tin>(xt, ht_prev,
                                             W_dir, R_dir, B_dir,
                                             gate_z, gate_r, gate_h,
                                             linear_before_reset,
                                             params->activations[activation_f_idx],
                                             params->activation_alpha[activation_f_idx],
                                             params->activation_beta[activation_f_idx],
                                             isClipSet, clip,
                                             input_size, hidden_size);
          }

          /* Apply activations and update hidden state: Ht = (1 - zt) (.) ht + zt (.) Ht-1 */
          for (hiddenIdx = 0; hiddenIdx < hidden_size; hiddenIdx++)
          {
            float32_tidl zt = TIDL_gruActivation(gate_z[hiddenIdx],
                                                 params->activations[activation_f_idx],
                                                 params->activation_alpha[activation_f_idx],
                                                 params->activation_beta[activation_f_idx],
                                                 isClipSet, clip, useTaylor);
            float32_tidl ht = TIDL_gruActivation(gate_h[hiddenIdx],
                                                 params->activations[activation_g_idx],
                                                 params->activation_alpha[activation_g_idx],
                                                 params->activation_beta[activation_g_idx],
                                                 isClipSet, clip, useTaylor);
            ht_prev[hiddenIdx] = (1.0f - zt) * ht + zt * ht_prev[hiddenIdx];
          }

          /*
           * Write Y output
           * Output dims layout=0: [DIM2=seq, NUMCH=num_dir, HEIGHT=batch, WIDTH=hidden]
           *             layout=1: [DIM2=batch, NUMCH=seq, HEIGHT=num_dir, WIDTH=hidden]
           */
          if (Y != NULL)
          {
            int32_t outOffset;
            if (layout == 0)
            {
              outOffset = timeStep * outDataParams->pitch[TIDL_DIM2_PITCH] +
                          dirIdx * outDataParams->pitch[TIDL_CHANNEL_PITCH] +
                          batchIdx * outDataParams->pitch[TIDL_LINE_PITCH];
            }
            else
            {
              outOffset = batchIdx * outDataParams->pitch[TIDL_DIM2_PITCH] +
                          timeStep * outDataParams->pitch[TIDL_CHANNEL_PITCH] +
                          dirIdx * outDataParams->pitch[TIDL_LINE_PITCH];
            }
            Tout *yt = Y + outOffset;
            for (hiddenIdx = 0; hiddenIdx < hidden_size; hiddenIdx++)
            {
              yt[hiddenIdx] = (Tout)ht_prev[hiddenIdx];
            }
          }
        } /* end batchIdx */
      } /* end timeIdx */

      /* Copy final hidden state to Y_h */
      if (Y_h != NULL)
      {
        for (batchIdx = 0; batchIdx < batch_size; batchIdx++)
        {
          int32_t outOffset;
          if (layout == 0)
          {
            /* Y_h: [num_directions, batch_size, hidden_size] */
            outOffset = dirIdx * batch_size * hidden_size + batchIdx * hidden_size;
          }
          else
          {
            /* Y_h: [batch_size, num_directions, hidden_size] */
            outOffset = batchIdx * num_directions * hidden_size + dirIdx * hidden_size;
          }
          float32_tidl *ht_final = Ht + batchIdx * hidden_size;
          for (hiddenIdx = 0; hiddenIdx < hidden_size; hiddenIdx++)
          {
            Y_h[outOffset + hiddenIdx] = (Tout)ht_final[hiddenIdx];
          }
        }
      }
    } /* end dirIdx */
  }

  return status;
}

/**
 * @brief GRU layer reference implementation
 *
 * @param intAlgHandle    : tidl algorithm handle
 * @param algLayer        : Pointer to the layer specific parameters
 * @param tidlLayer       : Pointer to the common layer parameters
 * @param params          : GRU layer parameters
 * @param inPtr           : Pointer to input buffer (X tensor)
 * @param WPtr            : Pointer to weights buffer (W tensor)
 * @param RPtr            : Pointer to recurrence weights buffer (R tensor)
 * @param biasPtr         : Pointer to bias buffer (may be NULL)
 * @param initial_hPtr    : Initial hidden state pointer (may be NULL)
 * @param outPtr          : Output pointer
 * @param inDataParams    : pointer to input data parameters
 * @param WParams         : pointer to weights data parameters
 * @param RParams         : pointer to recurrence weights data parameters
 * @param biasParams      : pointer to bias data parameters
 * @param initial_hParams : pointer to initial_h data parameters
 * @param outDataParams   : pointer to output data parameters
 * @param useTaylor       : flag to indicate whether to use taylor series based implementation for activations
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
int32_t TIDL_gruRefProcess(TIDL_Handle intAlgHandle,
                            sTIDL_AlgLayer_t *algLayer,
                            const sTIDL_Layer_t *tidlLayer,
                            sTIDL_GRUParams_t *params,
                            void *inPtr,
                            void *WPtr,
                            void *RPtr,
                            void *biasPtr,
                            void *initial_hPtr,
                            void *outPtr,
                            const sTIDL_DataParams_t *inDataParams,
                            const sTIDL_DataParams_t *WParams,
                            const sTIDL_DataParams_t *RParams,
                            const sTIDL_DataParams_t *biasParams,
                            const sTIDL_DataParams_t *initial_hParams,
                            const sTIDL_DataParams_t *outDataParams,
                            int8_t useTaylor)
{
  int32_t status = IALG_EOK;
  int32_t layerIdx = algLayer->layerIdx;

  if (TIDL_SinglePrecFloat == ((int32_t)inDataParams->elementType))
  {
    status = TIDL_refGRUCoreFloat((float32_tidl *)inPtr,
                                  (float32_tidl *)WPtr,
                                  (float32_tidl *)RPtr,
                                  (float32_tidl *)biasPtr,
                                  (float32_tidl *)initial_hPtr,
                                  (float32_tidl *)outPtr,
                                  intAlgHandle,
                                  layerIdx,
                                  params,
                                  algLayer,
                                  inDataParams,
                                  WParams,
                                  RParams,
                                  biasParams,
                                  initial_hParams,
                                  outDataParams,
                                  useTaylor);
  }
  else
  {
    /* Unsupported data type */
    TIDL_LOG_ERROR(TIDL_ERROR_GROUP_GRU, TIDL_ERROR_GRU_NOT_IMPLEMENTED);
    status = IALG_EFAIL;
  }

  TIDL_L1DandL2CacheWbInv();
  return status;
}
