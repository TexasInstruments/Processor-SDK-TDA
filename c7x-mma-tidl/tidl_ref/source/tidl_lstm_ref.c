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
 *  \file tidl_lstm_ref.c
 *
 *  \brief LSTM process calls & ref implementation
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include <math.h>
#include <limits>
#include "tidl_alg_utils.h"
#include "tidl_lstm_ref.h"
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

static void TIDL_updateLSTMMemorySizes(const TIDL_LayerSpecificParams *layerSpecificParams,
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

  /* ScratchSize: Ht[bh] + Ct[bh] + gates[4h] */
  int32_t hidden_size = commonParams->net->TIDLLayers[layerIdx].layerParams.lstmParams.hidden_size;
  int32_t layout = commonParams->net->TIDLLayers[layerIdx].layerParams.lstmParams.layout;
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
  scratchDataSize = ((2 * batch_size * hidden_size) + (4 * hidden_size)) * TIDL_getDatElementSize(inDataParams->elementType);

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

int32_t TIDL_lstmRefAlloc(const TIDL_LayerSpecificParams *layerSpecificParams,
                          const TIDL_NetworkCommonParams *commonParams,
                          int32_t layerIdx,
                          int32_t memorySize[TIDL_LAYER_MEMORY_MAX])
{
  int32_t status = IALG_EOK;

  TIDL_updateLSTMMemorySizes(layerSpecificParams, commonParams, layerIdx, NULL, NULL, memorySize);

  return status;
}

int32_t TIDL_lstmRefInit(const TIDL_LayerSpecificParams *layerSpecificParams,
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

  TIDL_updateLSTMMemorySizes(layerSpecificParams, commonParams, layerIdx, &scratchDataSize, &outDataSize, memorySize);
  /* LDRA_JUSTIFY_START
  <metric start> branch <metric end>
  <justification start> PRIOR_CHECK : Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
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
static inline float32_tidl TIDL_lstmExpScalar(float32_tidl x)
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
 * @brief Sigmoid activation function for LSTM
 * @param x: input value
 * @return sigmoid(x)
 */
static inline float32_tidl TIDL_lstmSigmoid(float32_tidl x)
{
  return 1.0f / (1.0f + expf(-x));
}

/**
 * @brief Sigmoid activation function implemented using taylor series expansion
 * @param x: input value
 * @return sigmoid(x)
 */
static inline float32_tidl TIDL_lstmSigmoidTaylor(float32_tidl x)
{
  x = (x > TIDL_LSTM_SIGMOID_BOUND) ? TIDL_LSTM_SIGMOID_BOUND : x;
  x = (x < -TIDL_LSTM_SIGMOID_BOUND) ? -TIDL_LSTM_SIGMOID_BOUND : x;

  int32_t neg_mask         = (x < 0.0f);
  float32_tidl abs_x       = neg_mask ? -x : x;
  float32_tidl exp_neg_abs = TIDL_lstmExpScalar(-abs_x);
  float32_tidl denom       = 1.0f + exp_neg_abs;

  /* RCPSP + 2 Newton-Raphson iterations — matches __recip + NR in sigmoid_vec */
  float32_tidl recip = __recip(denom);
  recip = recip * (2.0f - denom * recip);
  recip = recip * (2.0f - denom * recip);

  float32_tidl sig_neg = 1.0f - recip;

  return neg_mask ? sig_neg : recip;
}

/**
 * @brief Tanh activation function for LSTM implemented using sigmoid to improve numerical stability
 * @param x: input value
 * @return tanh(x)
 */
static inline float32_tidl TIDL_lstmTanh(float32_tidl x)
{
  float32_tidl result;
  return tanhf(x);
}

/**
 * @brief Tanh activation function implemented using taylor series expansion
 * @param x: input value
 * @return tanh(x)
 */
static inline float32_tidl TIDL_lstmTanhTaylor(float32_tidl x)
{
  x = (x > TIDL_LSTM_TANH_BOUND) ? TIDL_LSTM_TANH_BOUND : x;
  x = (x < -TIDL_LSTM_TANH_BOUND) ? -TIDL_LSTM_TANH_BOUND : x;

  int32_t   neg_mask     = (x < 0.0f);
  float32_tidl abs_x     = neg_mask ? -x : x;
  float32_tidl exp_neg2x = TIDL_lstmExpScalar(-2.0f * abs_x);
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
static inline float32_tidl TIDL_lstmActivation(float32_tidl val, int32_t actType, float32_tidl activation_alpha,
                                               float32_tidl activation_beta,  int8_t isClipSet, float32_tidl clip,
                                               int8_t useTaylor)
{
  if(isClipSet == 1)
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
    activatedVal = (useTaylor == 1) ? TIDL_lstmSigmoidTaylor(val) : TIDL_lstmSigmoid(val);
  }
  else if (actType == TIDL_Tanh)
  {
    activatedVal = (useTaylor == 1) ? TIDL_lstmTanhTaylor(val) : TIDL_lstmTanh(val);
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
template<class Tin> static void TIDL_lstmGateComputeDevice(
    const Tin *xt,
    const Tin *ht_prev, const Tin *ct_prev,
    const Tin *W_dir, const Tin *R_dir,
    const Tin *B_dir, const Tin *P_dir,
    Tin *gate_i, Tin *gate_o,
    Tin *gate_f, Tin *gate_c,
    int32_t input_size, int32_t hidden_size)
{
  /* W_dir: [4*hidden_size, input_size],  ONNX gate order: i,o,f,c */
  const Tin *Wi  = W_dir + 0 * hidden_size * input_size;
  const Tin *Wo  = W_dir + 1 * hidden_size * input_size;
  const Tin *Wf  = W_dir + 2 * hidden_size * input_size;
  const Tin *Wc  = W_dir + 3 * hidden_size * input_size;

  /* R_dir: [4*hidden_size, hidden_size], ONNX gate order: i,o,f,c */
  const Tin *Ri  = R_dir + 0 * hidden_size * hidden_size;
  const Tin *Ro  = R_dir + 1 * hidden_size * hidden_size;
  const Tin *Rf  = R_dir + 2 * hidden_size * hidden_size;
  const Tin *Rc  = R_dir + 3 * hidden_size * hidden_size;

  /* B_dir: [8*hidden_size] = [Wbi,Wbo,Wbf,Wbc,Rbi,Rbo,Rbf,Rbc] */
  const Tin *Wbi = (B_dir != NULL) ? B_dir + 0 * hidden_size : NULL;
  const Tin *Wbo = (B_dir != NULL) ? B_dir + 1 * hidden_size : NULL;
  const Tin *Wbf = (B_dir != NULL) ? B_dir + 2 * hidden_size : NULL;
  const Tin *Wbc = (B_dir != NULL) ? B_dir + 3 * hidden_size : NULL;
  const Tin *Rbi = (B_dir != NULL) ? B_dir + 4 * hidden_size : NULL;
  const Tin *Rbo = (B_dir != NULL) ? B_dir + 5 * hidden_size : NULL;
  const Tin *Rbf = (B_dir != NULL) ? B_dir + 6 * hidden_size : NULL;
  const Tin *Rbc = (B_dir != NULL) ? B_dir + 7 * hidden_size : NULL;

  /* P_dir: [3*hidden_size] = [Pi,Po,Pf] */
  const Tin *Pi  = (P_dir != NULL) ? P_dir + 0 * hidden_size : NULL;
  const Tin *Pf  = (P_dir != NULL) ? P_dir + 2 * hidden_size : NULL;

  for (int32_t hiddenIdx = 0; hiddenIdx < hidden_size; hiddenIdx++)
  {
    /* Input gate (pre-activation): it = (Wbi + Xt*(Wi^T) + Rbi + Ht-1*(Ri^T) + Pi(.)Ct-1) */
    volatile float32_tidl wx_i = (Wbi != NULL) ? Wbi[hiddenIdx] : 0.0f;
    for (int32_t inputIdx = 0; inputIdx < input_size; inputIdx++)
    {
      wx_i += (float32_tidl)xt[inputIdx] * Wi[hiddenIdx * input_size + inputIdx];
    }
    volatile float32_tidl rh_i = (Rbi != NULL) ? Rbi[hiddenIdx] : 0.0f;
    for (int32_t inputIdx = 0; inputIdx < hidden_size; inputIdx++)
    {
      rh_i += ht_prev[inputIdx] * Ri[hiddenIdx * hidden_size + inputIdx];
    }
    gate_i[hiddenIdx] = (float32_tidl)wx_i + (float32_tidl)rh_i;
    if (Pi != NULL)
    {
      gate_i[hiddenIdx] += Pi[hiddenIdx] * ct_prev[hiddenIdx];
    }

    /* Output gate (pre-activation & without peepholes): ot = (Wbo + Xt*(Wo^T) + Rbo + Ht-1*(Ro^T)) */
    volatile float32_tidl wx_o = (Wbo != NULL) ? Wbo[hiddenIdx] : 0.0f;
    for (int32_t inputIdx = 0; inputIdx < input_size; inputIdx++)
    {
      wx_o += (float32_tidl)xt[inputIdx] * Wo[hiddenIdx * input_size + inputIdx];
    }
    volatile float32_tidl rh_o = (Rbo != NULL) ? Rbo[hiddenIdx] : 0.0f;
    for (int32_t inputIdx = 0; inputIdx < hidden_size; inputIdx++)
    {
      rh_o += ht_prev[inputIdx] * Ro[hiddenIdx * hidden_size + inputIdx];
    }
    gate_o[hiddenIdx] = (float32_tidl)wx_o + (float32_tidl)rh_o;

    /* Forget gate (pre-activation): ft = (Wbf + Xt*(Wf^T) + Rbf + Ht-1*(Rf^T) + Pf(.)Ct-1) */
    volatile float32_tidl wx_f = (Wbf != NULL) ? Wbf[hiddenIdx] : 0.0f;
    for (int32_t inputIdx = 0; inputIdx < input_size; inputIdx++)
    {
      wx_f += (float32_tidl)xt[inputIdx] * Wf[hiddenIdx * input_size + inputIdx];
    }
    volatile float32_tidl rh_f = (Rbf != NULL) ? Rbf[hiddenIdx] : 0.0f;
    for (int32_t inputIdx = 0; inputIdx < hidden_size; inputIdx++)
    {
      rh_f += ht_prev[inputIdx] * Rf[hiddenIdx * hidden_size + inputIdx];
    }
    gate_f[hiddenIdx] = (float32_tidl)wx_f + (float32_tidl)rh_f;
    if (Pf != NULL)
    {
      gate_f[hiddenIdx] += Pf[hiddenIdx] * ct_prev[hiddenIdx];
    }

    /* Cell gate (pre-activation): ct = (Wbc + Xt*(Wc^T) + Rbc + Ht-1*(Rc^T)) */
    volatile float32_tidl wx_g = (Wbc != NULL) ? Wbc[hiddenIdx] : 0.0f;
    for (int32_t inputIdx = 0; inputIdx < input_size; inputIdx++)
    {
      wx_g += (float32_tidl)xt[inputIdx] * Wc[hiddenIdx * input_size + inputIdx];
    }
    volatile float32_tidl rh_g = (Rbc != NULL) ? Rbc[hiddenIdx] : 0.0f;
    for (int32_t inputIdx = 0; inputIdx < hidden_size; inputIdx++)
    {
      rh_g += ht_prev[inputIdx] * Rc[hiddenIdx * hidden_size + inputIdx];
    }
    gate_c[hiddenIdx] = (float32_tidl)wx_g + (float32_tidl)rh_g;
  }
}

/* Standard gate computation as per onnxruntime */
template<class Tin> static void TIDL_lstmGateComputeStandard(
    const Tin *xt,
    const Tin *ht_prev, const Tin *ct_prev,
    const Tin *W_dir, const Tin *R_dir,
    const Tin *B_dir, const Tin *P_dir,
    Tin *gate_i, Tin *gate_o,
    Tin *gate_f, Tin *gate_c,
    int32_t input_size, int32_t hidden_size)
{
  /* W_dir: [4*hidden_size, input_size],  ONNX gate order: i,o,f,c */
  const Tin *Wi  = W_dir + 0 * hidden_size * input_size;
  const Tin *Wo  = W_dir + 1 * hidden_size * input_size;
  const Tin *Wf  = W_dir + 2 * hidden_size * input_size;
  const Tin *Wc  = W_dir + 3 * hidden_size * input_size;

  /* R_dir: [4*hidden_size, hidden_size], ONNX gate order: i,o,f,c */
  const Tin *Ri  = R_dir + 0 * hidden_size * hidden_size;
  const Tin *Ro  = R_dir + 1 * hidden_size * hidden_size;
  const Tin *Rf  = R_dir + 2 * hidden_size * hidden_size;
  const Tin *Rc  = R_dir + 3 * hidden_size * hidden_size;

  /* B_dir: [8*hidden_size] = [Wbi,Wbo,Wbf,Wbc,Rbi,Rbo,Rbf,Rbc] */
  const Tin *Wbi = (B_dir != NULL) ? B_dir + 0 * hidden_size : NULL;
  const Tin *Wbo = (B_dir != NULL) ? B_dir + 1 * hidden_size : NULL;
  const Tin *Wbf = (B_dir != NULL) ? B_dir + 2 * hidden_size : NULL;
  const Tin *Wbc = (B_dir != NULL) ? B_dir + 3 * hidden_size : NULL;
  const Tin *Rbi = (B_dir != NULL) ? B_dir + 4 * hidden_size : NULL;
  const Tin *Rbo = (B_dir != NULL) ? B_dir + 5 * hidden_size : NULL;
  const Tin *Rbf = (B_dir != NULL) ? B_dir + 6 * hidden_size : NULL;
  const Tin *Rbc = (B_dir != NULL) ? B_dir + 7 * hidden_size : NULL;

  /* P_dir: [3*hidden_size] = [Pi,Po,Pf] */
  const Tin *Pi  = (P_dir != NULL) ? P_dir + 0 * hidden_size : NULL;
  const Tin *Pf  = (P_dir != NULL) ? P_dir + 2 * hidden_size : NULL;

  /* Input gate (pre-activation): it = (Xt*(Wi^T) + Ht-1*(Ri^T) + Pi(.)Ct-1 + Wbi + Rbi) */
  for (int32_t hiddenIdx = 0; hiddenIdx < hidden_size; hiddenIdx++)
  {
    float32_tidl sum = 0.0f;
    for (int32_t inputIdx = 0; inputIdx < input_size; inputIdx++)
    {
      sum += (float32_tidl)xt[inputIdx] * Wi[hiddenIdx * input_size + inputIdx];
    }
    for (int32_t inputIdx = 0; inputIdx < hidden_size; inputIdx++)
    {
      sum += ht_prev[inputIdx] * Ri[hiddenIdx * hidden_size + inputIdx];
    }
    if (Pi != NULL)
    {
      sum += Pi[hiddenIdx] * ct_prev[hiddenIdx];
    }
    if (Wbi != NULL)
    {
      sum += Wbi[hiddenIdx];
    }
    if (Rbi != NULL)
    {
      sum += Rbi[hiddenIdx];
    }
    gate_i[hiddenIdx] = sum;
  }

  /* Forget gate (pre-activation): ft = (Xt*(Wf^T) + Ht-1*(Rf^T) + Pf(.)Ct-1 + Wbf + Rbf) */
  for (int32_t hiddenIdx = 0; hiddenIdx < hidden_size; hiddenIdx++)
  {
    float32_tidl sum = 0.0f;
    for (int32_t inputIdx = 0; inputIdx < input_size; inputIdx++)
    {
      sum += (float32_tidl)xt[inputIdx] * Wf[hiddenIdx * input_size + inputIdx];
    }
    for (int32_t inputIdx = 0; inputIdx < hidden_size; inputIdx++)
    {
      sum += ht_prev[inputIdx] * Rf[hiddenIdx * hidden_size + inputIdx];
    }
    if (Pf != NULL)
    {
      sum += Pf[hiddenIdx] * ct_prev[hiddenIdx];
    }
    if (Wbf != NULL)
    {
      sum += Wbf[hiddenIdx];
    }
    if (Rbf != NULL)
    {
      sum += Rbf[hiddenIdx];
    }
    gate_f[hiddenIdx] = sum;
  }

  /* Cell gate (pre-activation): ct = g(Xt*(Wc^T) + Ht-1*(Rc^T) + Wbc + Rbc) */
  for (int32_t hiddenIdx = 0; hiddenIdx < hidden_size; hiddenIdx++)
  {
    float32_tidl sum = 0.0f;
    for (int32_t inputIdx = 0; inputIdx < input_size; inputIdx++)
    {
      sum += (float32_tidl)xt[inputIdx] * Wc[hiddenIdx * input_size + inputIdx];
    }
    for (int32_t inputIdx = 0; inputIdx < hidden_size; inputIdx++)
    {
      sum += ht_prev[inputIdx] * Rc[hiddenIdx * hidden_size + inputIdx];
    }
    if (Wbc != NULL)
    {
      sum += Wbc[hiddenIdx];
    }
    if (Rbc != NULL)
    {
      sum += Rbc[hiddenIdx];
    }
    gate_c[hiddenIdx] = sum;
  }

  /* Output gate (pre-activatino & without peepholes): (Xt*(Wo^T) + Ht-1*(Ro^T) + Wbo + Rbo) */
  for (int32_t hiddenIdx = 0; hiddenIdx < hidden_size; hiddenIdx++)
  {
    float32_tidl sum = 0.0f;
    for (int32_t inputIdx = 0; inputIdx < input_size; inputIdx++)
    {
      sum += (float32_tidl)xt[inputIdx] * Wo[hiddenIdx * input_size + inputIdx];
    }
    for (int32_t inputIdx = 0; inputIdx < hidden_size; inputIdx++)
    {
      sum += ht_prev[inputIdx] * Ro[hiddenIdx * hidden_size + inputIdx];
    }
    if (Wbo != NULL)
    {
      sum += Wbo[hiddenIdx];
    }
    if (Rbo != NULL)
    {
      sum += Rbo[hiddenIdx];
    }
    gate_o[hiddenIdx] = sum;
  }
}

/**
 * @brief Float kernel for LSTM operator (ONNX LSTM-14)
 *
 * Implements the ONNX LSTM equations:
 *   it = f(Xt*(Wi^T) + Ht-1*(Ri^T) + Pi (.) Ct-1 + Wbi + Rbi)
 *   ft = f(Xt*(Wf^T) + Ht-1*(Rf^T) + Pf (.) Ct-1 + Wbf + Rbf)
 *   ct = g(Xt*(Wc^T) + Ht-1*(Rc^T) + Wbc + Rbc)
 *   Ct = ft (.) Ct-1 + it (.) ct
 *   ot = f(Xt*(Wo^T) + Ht-1*(Ro^T) + Po (.) Ct + Wbo + Rbo)
 *   Ht = ot (.) h(Ct)
 *
 * where f, g, h are activation functions and (.) is element-wise product.
 * ONNX gate order in W, R, B tensors: i, o, f, c
 *
 * @param inPtr            : Input pointer (X tensor)
 * @param WPtr             : Weights pointer (W tensor)
 * @param RPtr             : Recurrence weights pointer (R tensor)
 * @param initial_hPtr     : Initial hidden state pointer (initial_h tensor)
 * @param initial_cPtr     : Initial cell state pointer (initial_c tensor)
 * @param peepholesPtr     : Peephole weights pointer (peepholes tensor)
 * @param outPtr           : Output pointer (Y tensor)
 * @param intAlgHandle     : tidl algorithm handle
 * @param layerIdx         : index of the current layer
 * @param params           : LSTM layer parameters
 * @param algLayer         : Pointer to the layer specific parameters
 * @param inDataParams     : parameters of the input data buffer
 * @param WParams          : parameters of the weights buffer
 * @param RParams          : parameters of the recurrence weights buffer
 * @param initial_hParams  : parameters of the initial hidden state buffer
 * @param initial_cParams  : parameters of the initial cell state buffer
 * @param peepholesParams  : parameters of the peephole weights buffer
 * @param outDataParams    : parameters of the output data buffer
 * @param useTaylor        : flag to indicate whether to use taylor series expansion for activations
 * @return  IALG_EOK       - Successful
 *          IALG_EFAIL     - Unspecified error
 */
template<class Tin, class Tout> static int32_t TIDL_refLSTMCoreFloat(
    Tin *inPtr,
    Tin *WPtr,
    Tin *RPtr,
    Tin *biasPtr,
    Tin *initial_hPtr,
    Tin *initial_cPtr,
    Tin *peepholesPtr,
    Tout *outPtr,
    TIDL_Handle intAlgHandle,
    int32_t layerIdx,
    sTIDL_LSTMParams_t *params,
    sTIDL_AlgLayer_t *algLayer,
    const sTIDL_DataParams_t *inDataParams,
    const sTIDL_DataParams_t *WParams,
    const sTIDL_DataParams_t *RParams,
    const sTIDL_DataParams_t *biasParams,
    const sTIDL_DataParams_t *initial_hParams,
    const sTIDL_DataParams_t *initial_cParams,
    const sTIDL_DataParams_t *peepholesParams,
    const sTIDL_DataParams_t *outDataParams,
    int8_t useTaylor)
{
  int32_t status = IALG_EOK;
  sTIDL_Network_t *net = intAlgHandle->createParams->net;
  int32_t hidden_size = params->hidden_size;
  int32_t direction = params->direction;
  int32_t layout = params->layout;
  int32_t input_forget = params->input_forget;
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

  /* Extract batch_size, seq_length from input dimensions based on layout */
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

  /* Scratch layout: Ht[bh] + Ct[bh] + gates[4*h] */
  int32_t requiredScratchSize = ((2 * batch_size * hidden_size) + (4 * hidden_size)) * sizeof(Tin);
  if(algLayer->scratchSize < requiredScratchSize)
  {
    tidl_printf(0, "Memory for TIDL_refLSTMCoreFloat accumulator is not sufficient exiting...\n");
    status = IALG_EFAIL;
  }

  if(status == IALG_EOK)
  {
    int32_t batchHidden = batch_size * hidden_size;
    Tin *scratch = (Tin *)algLayer->scratchMem;
    Tin *Ht    = scratch;
    Tin *Ct    = Ht + batchHidden;
    Tin *gates = Ct + batchHidden;

    Tin  *X = inPtr;
    Tout *Y = outPtr;

    /* Calculate output tensor offsets for Y_h and Y_c */
    int32_t incrementAxis = params->incrementAxis;
    int32_t YhOffset = outDataParams->pitch[incrementAxis] * seq_length;
    int32_t YcOffset = YhOffset + outDataParams->pitch[incrementAxis];
    Tout *Y_h = Y + YhOffset;
    Tout *Y_c = Y + YcOffset;

    int32_t dirIdx, timeIdx, batchIdx, hiddenIdx, inputIdx;

    for (dirIdx = 0; dirIdx < num_directions; dirIdx++)
    {
      /* Activations for current direction */
      int32_t activation_f_idx = dirIdx * 3 + 0;
      int32_t activation_g_idx = dirIdx * 3 + 1;
      int32_t activation_h_idx = dirIdx * 3 + 2;

      /* W: [num_directions, 4*hidden_size, input_size], ONNX gate order: i,o,f,c */
      Tin *W_dir = WPtr + dirIdx * 4 * hidden_size * input_size;

      /* R: [num_directions, 4*hidden_size, hidden_size] */
      Tin *R_dir = RPtr + dirIdx * 4 * hidden_size * hidden_size;

      /* B: [num_directions, 8*hidden_size] = [Wbi,Wbo,Wbf,Wbc, Rbi,Rbo,Rbf,Rbc] or NULL */
      Tin *B_dir = (biasPtr != NULL) ? biasPtr + dirIdx * 8 * hidden_size : NULL;

      /* P: [num_directions, 3*hidden_size] = [Pi, Po, Pf] or NULL */
      Tin *P_dir = (peepholesPtr != NULL) ? peepholesPtr + dirIdx * 3 * hidden_size : NULL;
      Tin *Po    = (P_dir != NULL) ? P_dir + 1 * hidden_size : NULL;

      /*
      * Initialize Ht and Ct from initial states
      * initial_h/c: [num_directions, batch_size, hidden_size]
      */
      if (initial_hPtr != NULL)
      {
        Tin *initH_dir = initial_hPtr + dirIdx * batchHidden;
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

      if (initial_cPtr != NULL)
      {
        Tin *initC_dir = initial_cPtr + dirIdx * batchHidden;
        for (hiddenIdx = 0; hiddenIdx < batchHidden; hiddenIdx++)
        {
          Ct[hiddenIdx] = initC_dir[hiddenIdx];
        }
      }
      else
      {
        for (hiddenIdx = 0; hiddenIdx < batchHidden; hiddenIdx++)
        {
          Ct[hiddenIdx] = 0.0f;
        }
      }

      /* Iterate over time steps */
      for (timeIdx = 0; timeIdx < seq_length; timeIdx++)
      {
        /*
        * Determine time step based on direction:
        * Forward:       dirIdx 0 → forward traversal
        * Reverse:       dirIdx 0 → reverse traversal
        * Bidirectional: dirIdx 0 → forward, dirIdx 1 → reverse
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
          * if sequence_lens is provided, for time steps past
          * sequence_lens[batchIdx], output Y = 0 and Ht/Ct remain unchanged.
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
            inOffset = timeStep * inDataParams->pitch[TIDL_CHANNEL_PITCH] + batchIdx * inDataParams->pitch[TIDL_LINE_PITCH];
          }
          else
          {
            inOffset = batchIdx * inDataParams->pitch[TIDL_CHANNEL_PITCH] + timeStep * inDataParams->pitch[TIDL_LINE_PITCH];
          }
          Tin *xt = X + inOffset;

          Tin *ht_prev = Ht + batchIdx * hidden_size;
          Tin *ct_prev = Ct + batchIdx * hidden_size;

          /* Gate pointers within scratch: [4, hidden_size] */
          Tin *gate_i = gates + 0 * hidden_size;
          Tin *gate_o = gates + 1 * hidden_size;
          Tin *gate_f = gates + 2 * hidden_size;
          Tin *gate_c = gates + 3 * hidden_size;

          if (useTaylor == 1)
          {
            TIDL_lstmGateComputeDevice<Tin>(xt, ht_prev, ct_prev,
                                           W_dir, R_dir, B_dir, P_dir,
                                           gate_i, gate_o, gate_f, gate_c,
                                           input_size, hidden_size);
          }
          else
          {
            TIDL_lstmGateComputeStandard<Tin>(xt, ht_prev, ct_prev,
                                             W_dir, R_dir, B_dir, P_dir,
                                             gate_i, gate_o, gate_f, gate_c,
                                             input_size, hidden_size);
          }

          /* Activate gates and update cell/hidden state */
          for (hiddenIdx = 0; hiddenIdx < hidden_size; hiddenIdx++)
          {
            float32_tidl it = TIDL_lstmActivation(gate_i[hiddenIdx], params->activations[activation_f_idx],
                                                  params->activation_alpha[activation_f_idx], params->activation_beta[activation_f_idx],
                                                  isClipSet, clip, useTaylor);
            float32_tidl ft = TIDL_lstmActivation(gate_f[hiddenIdx], params->activations[activation_f_idx],
                                                  params->activation_alpha[activation_f_idx], params->activation_beta[activation_f_idx],
                                                  isClipSet, clip, useTaylor);

            if (input_forget != 0)
            {
              ft = 1.0f - it;
            }

            float32_tidl ct = TIDL_lstmActivation(gate_c[hiddenIdx], params->activations[activation_g_idx],
                                                  params->activation_alpha[activation_g_idx], params->activation_beta[activation_g_idx],
                                                  isClipSet, clip, useTaylor);

            ct_prev[hiddenIdx] = ft * ct_prev[hiddenIdx] + it * ct;

            float32_tidl outGateVal = gate_o[hiddenIdx];
            if (Po != NULL)
            {
              outGateVal += Po[hiddenIdx] * ct_prev[hiddenIdx];
            }
            float32_tidl ot = TIDL_lstmActivation(outGateVal, params->activations[activation_f_idx],
                                                  params->activation_alpha[activation_f_idx], params->activation_beta[activation_f_idx],
                                                  isClipSet, clip, useTaylor);

            /*
            * Avoiding clipping in Ct to match the onnxruntime behavior
            * https://github.com/microsoft/onnxruntime/issues/27224 - this fixes the clipping in Ct
            * Pass isClipSet instead of 0 once this issue is merged in onnxruntime
            */
            ht_prev[hiddenIdx] = ot * TIDL_lstmActivation(ct_prev[hiddenIdx], params->activations[activation_h_idx],
                                                  params->activation_alpha[activation_h_idx], params->activation_beta[activation_h_idx],
                                                  0, clip, useTaylor);
          }

          /*
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
                          dirIdx   * outDataParams->pitch[TIDL_LINE_PITCH];
            }
            Tout *yt = Y + outOffset;
            for (hiddenIdx = 0; hiddenIdx < hidden_size; hiddenIdx++)
            {
              yt[hiddenIdx] = (Tout)ht_prev[hiddenIdx];
            }
          }
        } /* end batchIdx */
      } /* end timeIdx */

      /* Copy to Y_h - final hidden state for this direction */
      if (Y_h != NULL)
      {
        for (batchIdx = 0; batchIdx < batch_size; batchIdx++)
        {
          /* Calculate offset based on layout */
          int32_t outOffset;
          if (layout == 0)
          {
            /* [num_directions, batch_size, hidden_size] */
            outOffset = dirIdx * batch_size * hidden_size + batchIdx * hidden_size;
          }
          else
          {
            /* [batch_size, num_directions, hidden_size] */
            outOffset = batchIdx * num_directions * hidden_size + dirIdx * hidden_size;
          }

          /* Copy Ht values to Y_h */
          float32_tidl *ht_final = Ht + batchIdx * hidden_size;
          for (hiddenIdx = 0; hiddenIdx < hidden_size; hiddenIdx++)
          {
            Y_h[outOffset + hiddenIdx] = (Tout)ht_final[hiddenIdx];
          }
        }
      }

      /* Copy to Y_c - final cell state for this direction */
      if (Y_c != NULL)
      {
        for (batchIdx = 0; batchIdx < batch_size; batchIdx++)
        {
          /* Calculate offset based on layout */
          int32_t outOffset;
          if (layout == 0)
          {
            /* [num_directions, batch_size, hidden_size] */
            outOffset = dirIdx * batch_size * hidden_size + batchIdx * hidden_size;
          }
          else
          {
            /* [batch_size, num_directions, hidden_size] */
            outOffset = batchIdx * num_directions * hidden_size + dirIdx * hidden_size;
          }

          /* Copy Ct values to Y_c */
          float32_tidl *ct_final = Ct + batchIdx * hidden_size;
          for (hiddenIdx = 0; hiddenIdx < hidden_size; hiddenIdx++)
          {
            Y_c[outOffset + hiddenIdx] = (Tout)ct_final[hiddenIdx];
          }
        }
      }
    } /* end dirIdx */
  }

  return status;
}

/**
 * @brief LSTM layer reference implementation
 *
 * @param intAlgHandle  : tidl algorithm handle
 * @param algLayer      : Pointer to the layer specific parameters
 * @param tidlLayer     : Pointer to the common layer parameters
 * @param params        : LSTM layer parameters
 * @param inPtr         : Pointer to input buffers to be processed
 * @param WPtr          : Pointer to weights buffer
 * @param RPtr          : Pointer to recurrence weights buffer
 * @param biasPtr       : Pointer to bias buffer (may be NULL)
 * @param initial_hPtr  : Initial hidden state pointer
 * @param initial_cPtr  : Initial cell state pointer
 * @param peepholesPtr  : Peephole weights pointer
 * @param outPtr        : Output pointer
 * @param inDataParams  : pointer to input data parameters
 * @param WParams       : pointer to weights parameters
 * @param RParams       : pointer to recurrence weights parameters
 * @param biasParams    : pointer to bias data parameters
 * @param initial_hParams : pointer to initial_h data parameters
 * @param initial_cParams : pointer to initial_c data parameters
 * @param peepholesParams : pointer to peephole weights parameters
 * @param outDataParams : pointer to output data parameters
 * @param useTaylor     : flag to indicate whether to use taylor series expansion for activations
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
int32_t TIDL_lstmRefProcess(TIDL_Handle intAlgHandle,
                            sTIDL_AlgLayer_t *algLayer,
                            const sTIDL_Layer_t *tidlLayer,
                            sTIDL_LSTMParams_t *params,
                            void *inPtr,
                            void *WPtr,
                            void *RPtr,
                            void *biasPtr,
                            void *initial_hPtr,
                            void *initial_cPtr,
                            void *peepholesPtr,
                            void *outPtr,
                            const sTIDL_DataParams_t *inDataParams,
                            const sTIDL_DataParams_t *WParams,
                            const sTIDL_DataParams_t *RParams,
                            const sTIDL_DataParams_t *biasParams,
                            const sTIDL_DataParams_t *initial_hParams,
                            const sTIDL_DataParams_t *initial_cParams,
                            const sTIDL_DataParams_t *peepholesParams,
                            const sTIDL_DataParams_t *outDataParams,
                            int8_t useTaylor)
{
  int32_t status = IALG_EOK;
  int32_t layerIdx = algLayer->layerIdx;

  if (TIDL_SinglePrecFloat == ((int32_t)inDataParams->elementType))
  {
    status = TIDL_refLSTMCoreFloat((float32_tidl *)inPtr,
                                   (float32_tidl *)WPtr,
                                   (float32_tidl *)RPtr,
                                   (float32_tidl *)biasPtr,
                                   (float32_tidl *)initial_hPtr,
                                   (float32_tidl *)initial_cPtr,
                                   (float32_tidl *)peepholesPtr,
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
                                   initial_cParams,
                                   peepholesParams,
                                   outDataParams,
                                   useTaylor);
  }
  else
  {
    /* Unsupported data type */
    TIDL_LOG_ERROR(TIDL_ERROR_GROUP_LSTM, TIDL_ERROR_LSTM_NOT_IMPLEMENTED);
    status = IALG_EFAIL;
  }

  TIDL_L1DandL2CacheWbInv();
  return status;
}
