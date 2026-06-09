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
 *  \file tidl_rnn_ref.c
 *
 *  \brief RNN float reference implementation (ONNX RNN-14)
 *
 *  Equation (ref: https://onnx.ai/onnx/operators/onnx__RNN.html#rnn-14):
 *
 *    Ht = f(Xt*(Wi^T) + Ht-1*(Ri^T) + Wbi + Rbi)
 *
 *  where:
 *    f = tanh (default activation, 1 activation per direction)
 *
 *  Weight tensor layout (ONNX):
 *    W : [num_directions, hidden_size, input_size]
 *    R : [num_directions, hidden_size, hidden_size]
 *    B : [num_directions, 2*hidden_size]  layout: [Wb, Rb]
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include <math.h>
#include <limits>
#include "tidl_alg_utils.h"
#include "tidl_rnn_ref.h"
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

static void TIDL_updateRNNMemorySizes(const TIDL_LayerSpecificParams *layerSpecificParams,
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

  /* ScratchSize: Ht[batch_size * hidden_size] + ht_new[hidden_size] */
  int32_t hidden_size = commonParams->net->TIDLLayers[layerIdx].layerParams.rnnParams.hidden_size;
  int32_t layout = commonParams->net->TIDLLayers[layerIdx].layerParams.rnnParams.layout;
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
  scratchDataSize = (batch_size * hidden_size + hidden_size) * TIDL_getDatElementSize(inDataParams->elementType);

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

int32_t TIDL_rnnRefAlloc(const TIDL_LayerSpecificParams *layerSpecificParams,
                          const TIDL_NetworkCommonParams *commonParams,
                          int32_t layerIdx,
                          int32_t memorySize[TIDL_LAYER_MEMORY_MAX])
{
  int32_t status = IALG_EOK;

  TIDL_updateRNNMemorySizes(layerSpecificParams, commonParams, layerIdx, NULL, NULL, memorySize);

  return status;
}

int32_t TIDL_rnnRefInit(const TIDL_LayerSpecificParams *layerSpecificParams,
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

  TIDL_updateRNNMemorySizes(layerSpecificParams, commonParams, layerIdx, &scratchDataSize, &outDataSize, memorySize);
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
 * @brief Sigmoid activation function for RNN
 * @param x: input value
 * @return sigmoid(x)
 */
static inline float32_tidl TIDL_rnnSigmoid(float32_tidl x)
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
 * @brief Tanh activation function for RNN, implemented using sigmoid to improve numerical stability
 * @param x: input value
 * @return tanh(x)
 */
static inline float32_tidl TIDL_rnnTanh(float32_tidl x)
{
  float32_tidl result;
  result = 2.0f * TIDL_rnnSigmoid(2.0f * x) - 1.0f;

  return result;
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
 * @return activated value
 */
static inline float32_tidl TIDL_rnnActivation(float32_tidl val, int32_t actType, float32_tidl activation_alpha,
                                               float32_tidl activation_beta, int32_t isClipSet, float32_tidl clip)
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
    activatedVal = TIDL_rnnSigmoid(val);
  }
  else if (actType == TIDL_Tanh)
  {
    activatedVal = TIDL_rnnTanh(val);
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

/**
 * @brief Float kernel for RNN operator (ONNX RNN-14)
 *
 * Implements the ONNX RNN-14 equation:
 *   Ht = f(Xt*(Wi^T) + Ht-1*(Ri^T) + Wbi + Rbi)
 *
 * where f is the activation function (default: tanh), 1 per direction.
 *
 * Tensor layouts (ONNX):
 *   W : [num_directions, hidden_size, input_size]
 *   R : [num_directions, hidden_size, hidden_size]
 *   B : [num_directions, 2*hidden_size] = [Wb, Rb]
 *
 * @param inPtr            : Input pointer (X tensor)
 * @param WPtr             : Weights pointer (W tensor)
 * @param RPtr             : Recurrence weights pointer (R tensor)
 * @param initial_hPtr     : Initial hidden state pointer (initial_h tensor), may be NULL
 * @param outPtr           : Output pointer (Y tensor)
 * @param intAlgHandle     : tidl algorithm handle
 * @param layerIdx         : index of the current layer
 * @param params           : RNN layer parameters
 * @param algLayer         : Pointer to the layer specific parameters
 * @param inDataParams     : parameters of the input data buffer
 * @param WParams          : parameters of the weights buffer
 * @param RParams          : parameters of the recurrence weights buffer
 * @param initial_hParams  : parameters of the initial hidden state buffer
 * @param outDataParams    : parameters of the output data buffer
 * @return  IALG_EOK       - Successful
 *          IALG_EFAIL     - Unspecified error
 */
template<class Tin, class Tout> static int32_t TIDL_refRNNCoreFloat(
    Tin *inPtr,
    Tin *WPtr,
    Tin *RPtr,
    Tin *initial_hPtr,
    Tout *outPtr,
    TIDL_Handle intAlgHandle,
    int32_t layerIdx,
    sTIDL_RNNParams_t *params,
    sTIDL_AlgLayer_t *algLayer,
    const sTIDL_DataParams_t *inDataParams,
    const sTIDL_DataParams_t *WParams,
    const sTIDL_DataParams_t *RParams,
    const sTIDL_DataParams_t *initial_hParams,
    const sTIDL_DataParams_t *outDataParams)
{
  int32_t status = IALG_EOK;
  sTIDL_Network_t *net = intAlgHandle->createParams->net;
  int32_t hidden_size = params->hidden_size;
  int32_t direction = params->direction;
  int32_t layout = params->layout;
  int32_t isClipSet = params->isClipSet;
  float32_tidl clip = params->clip;

  /* Determine number of directions */
  int32_t num_directions = 1;
  if (direction == TIDL_RNNBidirectional)
  {
    num_directions = 2;
  }

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

  float32_tidl *B_all = NULL;
  if (params->bias != 0)
  {
    B_all = (float32_tidl *)get_int8_t_pointer((int8_t *)net, params->bias);
  }

  int32_t *sequence_lens = NULL;
  if (params->sequence_lens != 0)
  {
    sequence_lens = (int32_t *)get_int8_t_pointer((int8_t *)net, params->sequence_lens);
  }

  /* Scratch: Ht[batch_size * hidden_size] + ht_new[hidden_size] */
  int32_t requiredScratchSize = (batch_size * hidden_size + hidden_size) * sizeof(Tin);
  if (algLayer->scratchSize < requiredScratchSize)
  {
    tidl_printf(0, "Memory for TIDL_refRNNCoreFloat accumulator is not sufficient exiting...\n");
    status = IALG_EFAIL;
  }

  if (status == IALG_EOK)
  {
    int32_t batchHidden = batch_size * hidden_size;
    Tin *scratch = (Tin *)algLayer->scratchMem;
    Tin *Ht     = scratch;
    Tin *ht_new = scratch + batchHidden;  /* temp buffer for new hidden values */

    Tin  *X = inPtr;
    Tout *Y = outPtr;

    /* Calculate output tensor offset for Y_h */
    int32_t incrementAxis = params->incrementAxis;
    int32_t YhOffset = outDataParams->pitch[incrementAxis] * seq_length;
    Tout *Y_h = Y + YhOffset;

    int32_t dirIdx, timeIdx, batchIdx, hiddenIdx, inputIdx;

    for (dirIdx = 0; dirIdx < num_directions; dirIdx++)
    {
      /*
       * Activation for current direction: 1 per direction for RNN (index = dirIdx).
       * Default activation: tanh.
       */
      int32_t activation_f_idx = dirIdx;

      /* W: [num_directions, hidden_size, input_size] */
      Tin *Wi = WPtr + dirIdx * hidden_size * input_size;

      /* R: [num_directions, hidden_size, hidden_size] */
      Tin *Ri = RPtr + dirIdx * hidden_size * hidden_size;

      /* B: [num_directions, 2*hidden_size] = [Wb, Rb] */
      Tin *Wb = NULL;
      Tin *Rb = NULL;
      if (B_all != NULL)
      {
        Tin *B_dir = B_all + dirIdx * 2 * hidden_size;
        Wb = B_dir + 0 * hidden_size;
        Rb = B_dir + 1 * hidden_size;
      }

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
        if (direction == TIDL_RNNReverse)
        {
          isReverse = 1;
        }
        else if ((direction == TIDL_RNNBidirectional) && (dirIdx == 1))
        {
          isReverse = 1;
        }
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

          /*
           * Compute: Ht = f(Xt*(Wi^T) + Ht-1*(Ri^T) + Wb + Rb)
           */
          for (hiddenIdx = 0; hiddenIdx < hidden_size; hiddenIdx++)
          {
            /* Xt*(Wi^T) */
            float32_tidl sum = 0.0f;
            for (inputIdx = 0; inputIdx < input_size; inputIdx++)
            {
              sum += (float32_tidl)xt[inputIdx] * Wi[hiddenIdx * input_size + inputIdx];
            }
            /* Ht-1*(Ri^T) */
            for (inputIdx = 0; inputIdx < hidden_size; inputIdx++)
            {
              sum += ht_prev[inputIdx] * Ri[hiddenIdx * hidden_size + inputIdx];
            }
            /* Wb bias */
            if (Wb != NULL)
            {
              sum += Wb[hiddenIdx];
            }
            /* Rb bias */
            if (Rb != NULL)
            {
              sum += Rb[hiddenIdx];
            }

            ht_new[hiddenIdx] = TIDL_rnnActivation(sum, params->activations[activation_f_idx],
                                                   params->activation_alpha[activation_f_idx],
                                                   params->activation_beta[activation_f_idx],
                                                   isClipSet, clip);
          }

          /* Copy new values back to ht_prev now that all gates are computed */
          for (hiddenIdx = 0; hiddenIdx < hidden_size; hiddenIdx++)
          {
            ht_prev[hiddenIdx] = ht_new[hiddenIdx];
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
 * @brief RNN layer reference implementation
 *
 * @param intAlgHandle    : tidl algorithm handle
 * @param algLayer        : Pointer to the layer specific parameters
 * @param tidlLayer       : Pointer to the common layer parameters
 * @param params          : RNN layer parameters
 * @param inPtr           : Pointer to input buffer (X tensor)
 * @param WPtr            : Pointer to weights buffer (W tensor)
 * @param RPtr            : Pointer to recurrence weights buffer (R tensor)
 * @param initial_hPtr    : Initial hidden state pointer (may be NULL)
 * @param outPtr          : Output pointer
 * @param inDataParams    : pointer to input data parameters
 * @param WParams         : pointer to weights data parameters
 * @param RParams         : pointer to recurrence weights data parameters
 * @param initial_hParams : pointer to initial_h data parameters
 * @param outDataParams   : pointer to output data parameters
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
int32_t TIDL_rnnRefProcess(TIDL_Handle intAlgHandle,
                            sTIDL_AlgLayer_t *algLayer,
                            const sTIDL_Layer_t *tidlLayer,
                            sTIDL_RNNParams_t *params,
                            void *inPtr,
                            void *WPtr,
                            void *RPtr,
                            void *initial_hPtr,
                            void *outPtr,
                            const sTIDL_DataParams_t *inDataParams,
                            const sTIDL_DataParams_t *WParams,
                            const sTIDL_DataParams_t *RParams,
                            const sTIDL_DataParams_t *initial_hParams,
                            const sTIDL_DataParams_t *outDataParams)
{
  int32_t status = IALG_EOK;
  int32_t layerIdx = algLayer->layerIdx;

  if (TIDL_SinglePrecFloat == ((int32_t)inDataParams->elementType))
  {
    status = TIDL_refRNNCoreFloat((float32_tidl *)inPtr,
                                  (float32_tidl *)WPtr,
                                  (float32_tidl *)RPtr,
                                  (float32_tidl *)initial_hPtr,
                                  (float32_tidl *)outPtr,
                                  intAlgHandle,
                                  layerIdx,
                                  params,
                                  algLayer,
                                  inDataParams,
                                  WParams,
                                  RParams,
                                  initial_hParams,
                                  outDataParams);
  }
  else
  {
    /* Unsupported data type */
    TIDL_LOG_ERROR(TIDL_ERROR_GROUP_RNN, TIDL_ERROR_RNN_NOT_IMPLEMENTED);
    status = IALG_EFAIL;
  }

  TIDL_L1DandL2CacheWbInv();
  return status;
}
