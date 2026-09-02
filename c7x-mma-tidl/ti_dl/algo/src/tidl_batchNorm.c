/*
*
* Copyright (c) {2015 - 2025} Texas Instruments Incorporated
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
 *  \file tidl_batchNorm.c
 *
 *  \brief This file defines kernel functions for Batch Norm layer
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include "tidl_batchNorm.h"
#include <cmath>
#include "tidl_device_utils.h"
#include "tidl_alg_utils.h"
#include <limits>
#include "tidl_batchNorm_ref.h"
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
/* Return memory to be allocated per buffer in the persistent memory */
static void TIDL_getPersistentMemorySize(sTIDL_Network_t *net,
                                        const void * workloadUnit,
                                        int32_t layerIdx,
                                        int32_t *biasParamSize,
                                        int32_t *slopeFactSize,
                                        int32_t *weightParamSize,
                                        int32_t *lutParamSize
                                        )
{
  int32_t numChannels, weightSizeInBytes;
  sTIDL_Layer_t *tidlLayer = &net->TIDLLayers[layerIdx];
  TIDL_GetTensorDimensions(net, workloadUnit, layerIdx,
                          NULL, NULL,
                          NULL, NULL,
                          NULL, &numChannels);

  numChannels  = numChannels * tidlLayer->outData.dimValues[TIDL_DIM_DIM1] * tidlLayer->outData.dimValues[TIDL_DIM_DIM2];

  *biasParamSize = numChannels *  ((int32_t)(sizeof(int32_t)));
  *slopeFactSize = numChannels *  ((int32_t)(sizeof(int32_t)));
   weightSizeInBytes = ((net->TIDLLayers[layerIdx].weightsElementSizeInBits + 7) / 8);
  *weightParamSize = numChannels * weightSizeInBytes;
  *lutParamSize = tidlLayer->actParams.lutParams.lutBufSize;
  
  return ;
}

static void TIDL_batchNormDmaswLutMem(const TIDL_NetworkCommonParams  *commonParams,
                            sTIDL_AlgLayer_t  *algLayer)
{
  uint8_t trMem[64]={0};
  int32_t lutScratchSize = algLayer->layerParams.batchNormParams.lutParams.lutParamSize;

  if((algLayer->layerParams.batchNormParams.lutScratchInt[0] != NULL) && (lutScratchSize > 0))
  {
    int32_t transferSize = 0;
    int32_t totalPrevTransferSize = 0;
    WorkloadUnitExec_CommonParams commonParamsLocalCpy = *commonParams->tidlCommonParams;

    TIDL_prePareMemcpyTr(trMem);//:TODO: Remove this

    while(lutScratchSize > 0)
    {
      if(lutScratchSize > TIDL_SAT_HI_UINT16)
      {
        transferSize = TIDL_SAT_HI_UINT16;
      }
      else
      {
        transferSize = lutScratchSize;
      }

      TIDL_memcpy2D((uint8_t *)algLayer->layerParams.batchNormParams.lutScratchInt[0] + totalPrevTransferSize,
              (uint8_t *)algLayer->layerParams.batchNormParams.lutParams.lutParamMem + totalPrevTransferSize,
              (uint16_t)transferSize,
              1,
              0,
              0,
              commonParamsLocalCpy.dmaUtilsContext,
              trMem);

      totalPrevTransferSize += transferSize;
      lutScratchSize -= transferSize;

    }
  }

  return;
}

#if !defined (__C7504__)
static int32_t TIDL_batchNormInitLUTParams(const TIDL_NetworkCommonParams *commonParams,
                                         int32_t layerIdx,
                                         sTIDL_AlgLayer_t *algLayer,
                                         sTIDL_sysMemHandle_t sysMems[TIDL_SYSMEM_MAX])
{
  int32_t status = IALG_EOK;
  sTIDL_Layer_t *tidlLayer  = &commonParams->net->TIDLLayers[layerIdx];

  if (commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_BATCHNORM_FORCE_INIT_LUTPARAMS_MEM_POINTER_NULL)
  {
    sysMems[TIDL_SYSMEM_L1_SCRATCH].size = 1;
    sysMems[TIDL_SYSMEM_L1_SCRATCH].offset = 2;
  }

  algLayer->layerParams.batchNormParams.lutParams.nonLinearActMethod = tidlLayer->actParams.lutParams.nonLinearActMethod;
  #if defined (__C7100__) || defined (__C7120__) || defined (__C7524__) || defined (__C7604__)
  if((algLayer->layerParams.batchNormParams.lutParams.nonLinearActMethod == (int32_t)TIDL_HW_LUT_8B) || 
     (algLayer->layerParams.batchNormParams.lutParams.nonLinearActMethod == (int32_t)TIDL_HW_ILUT_8B) ||
     (algLayer->layerParams.batchNormParams.lutParams.nonLinearActMethod == (int32_t)TIDL_HW_INTERPOLATED_LUT_16B))
  {
    void *memPtr = NULL;

    /* LUT implementation requires that the L1D memory be reserved fully for the kernel which it will use for the L1D during process call.
      By allocating all the L1D scratch memory upfront, we prevent any following part of the init code from using the L1D mem as scratch
      space. */
    memPtr = (void*) TIDL_getMemoryChunkFromSysmem(sysMems,
                                              sysMems[TIDL_SYSMEM_L1_SCRATCH].size,
                                              TIDL_ALIGNMENT_SIZE,
                                              (uint32_t)TIDL_SYSMEM_L1_SCRATCH,
                                              (uint32_t)IALG_SCRATCH);

    if (memPtr == NULL)
    {
      tidl_printf(0, "LUT implementation of batchNorm expects L1 mem to be reserved entirely for the LUT. Please make sure L1 memory is not used for this layer \n");
      status = TIDL_ERR_FAILURE;
    }

  }
  else
  {
    /* Nothing */
  }
  #endif

  return status;
}
#endif

static int32_t TIDL_batchNormInitBiasParamMem(sTIDL_Network_t *net,
                                         int32_t layerIdx,
                                         sTIDL_AlgLayer_t *algLayer,
                                         int32_t biasParamSize,
                                         int32_t *paramMemTabOffset,
                                         uint8_t *memory[TIDL_LAYER_MEMORY_MAX])
{
  int32_t status = TIDL_SUCCESS;
  sTIDL_Layer_t *tidlLayer  = &net->TIDLLayers[layerIdx];
  sTIDL_DataParams_t * inDataParams = TIDL_getDataParams(net, tidlLayer->inData[0]);
  sTIDL_BatchNormParams_t * batchNormParams = &tidlLayer->layerParams.batchNormParams;
  sTIDL_ALgBatchNormParams_t * algBatchNormParams = &algLayer->layerParams.batchNormParams;
  int32_t flattenedHigherDims = (tidlLayer->outData.dimValues[TIDL_DIM_DIM1] * tidlLayer->outData.dimValues[TIDL_DIM_DIM2]);
  int32_t biasElemSize = 2;
  int32_t biasParamSizePerDim;
  int32_t numChannels;

  TIDL_GetTensorDimensions(net, algLayer->workloadUnit, layerIdx,
                        NULL, NULL,
                        NULL, NULL,
                        NULL, &numChannels);

  algBatchNormParams->biasParamSize = biasParamSize;

  TIDL_AllocatePtr((intptr_t)memory[TIDL_LAYER_MEMORY_PERSISTENT],
                                    paramMemTabOffset,
                                    algBatchNormParams->biasParamSize,
                                    TIDL_ALIGNMENT_SIZE,
                                    &algBatchNormParams->biasParamMem );

  int16_t * orgBiasPtr = (int16_t *)get_int8_t_pointer((int8_t *)(net), batchNormParams->bias);
  int8_t * biasPtr  = (int8_t *)algBatchNormParams->biasParamMem;
  /* LDRA_JUSTIFY_START
  <metric start> branch <metric end>
  <justification start>
  Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  This condition is maintained to avoid null pointer dereference exceptions.
  Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
  <justification end> */
  if(biasPtr != NULL)
  /* LDRA_JUSTIFY_END */
  {
  float32_tidl inScaleFactor;
  float32_tidl inTensorScale = TIDL_getTensorScale(net, algLayer->inLayerIdx[0]);

  inScaleFactor = (batchNormParams->weightScale * inTensorScale) / batchNormParams->biasScale;

  if(tidlLayer->weightsElementSizeInBits <= 8)
  {
    TIDL_conv2dBiasSplit((int16_t*)orgBiasPtr, (int16_t*)biasPtr, &algBatchNormParams->biasB, numChannels, inScaleFactor, TIDL_SAT_LO_INT16, TIDL_SAT_HI_INT16,0xFF, TIDL_getDatElementSign(inDataParams->elementType));
  }
  else
  {
    int32_t maxBiasB = 0xFFFF;
    biasElemSize = 4;
    /* This condition indicates mixed precision where processing is in 16 bit but input is signed input.
    Here as target implementation scales the input with 256, we need to limit the biasB so that it doesn't
    go beyond 8 bits as the same is later multiplied by 256 to match the scale of accumulator*/
    // TODO: Need to find a way to do this without using inDataParams. Need to access buffer info of proclink somehow.
    if ( inDataParams->elementType == TIDL_SignedChar )
    {
      maxBiasB = 0xFF;
    }
    TIDL_conv2dBiasSplit((int16_t*)orgBiasPtr, (int32_t*)biasPtr, &algBatchNormParams->biasB, numChannels, inScaleFactor, TIDL_SAT_LO_INT32, TIDL_SAT_HI_INT32, maxBiasB, TIDL_getDatElementSign(inDataParams->elementType));
  }

  /* Values for channels are same across higher dimensions and hence can be broadcasted. Starting from first index to prevent unnecessary copy in 4D cases*/
  biasParamSizePerDim = numChannels * biasElemSize;
  for (int32_t i = 1; i < flattenedHigherDims; i++)
  {
    (void)memcpy(((int8_t *)biasPtr) + (i * biasParamSizePerDim),
            biasPtr,
            biasParamSizePerDim);
  }
  }
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start>
  Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  This condition is maintained to avoid null pointer dereference exceptions.
  Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
  <justification end> */
  else
  {
    status = TIDL_ERR_NULL_POINTER;
  }
  /* LDRA_JUSTIFY_END */
  return status;
}

static void TIDL_batchNormInitSlopeFactMem(sTIDL_Network_t *net,
                                         int32_t layerIdx,
                                         sTIDL_AlgLayer_t *algLayer,
                                         int32_t slopeFactSize,
                                         int32_t *paramMemTabOffset,
                                         uint8_t *memory[TIDL_LAYER_MEMORY_MAX]) /*basePtr = params->net*/
{
  sTIDL_Layer_t *tidlLayer  = &net->TIDLLayers[layerIdx];
  sTIDL_ALgBatchNormParams_t * algBatchNormParams = &algLayer->layerParams.batchNormParams;
  int32_t flattenedHigherDims = (tidlLayer->outData.dimValues[TIDL_DIM_DIM1] * tidlLayer->outData.dimValues[TIDL_DIM_DIM2]);
  int32_t slopeFactSizePerDim = slopeFactSize / flattenedHigherDims;
  int32_t numChannels;
  int32_t index;

  TIDL_GetTensorDimensions(net, algLayer->workloadUnit, layerIdx,
                          NULL, NULL,
                          NULL, NULL,
                          NULL, &numChannels);

  algBatchNormParams->slopeFactSize = slopeFactSize;

  TIDL_AllocatePtr((intptr_t)memory[TIDL_LAYER_MEMORY_PERSISTENT],
                                    paramMemTabOffset,
                                    algBatchNormParams->slopeFactSize,
                                    TIDL_ALIGNMENT_SIZE,
                                    &algBatchNormParams->slopeFactMem);

  algBatchNormParams->slopeFact = (int32_t *)algBatchNormParams->slopeFactMem;
  int32_t preluScale = 1, i;
  float32_tidl slopeFactfp32 = (float32_tidl)0.0f;
  void * slopePtr  = get_int8_t_pointer((int8_t *)(net),tidlLayer->actParams.slope);
  /* LDRA_JUSTIFY_START
  <metric start> branch <metric end>
  <justification start>
  Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  This condition is maintained to avoid null pointer dereference exceptions.
  Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
  <justification end> */
  if(algBatchNormParams->slopeFact != NULL)
  /* LDRA_JUSTIFY_END */
  {
  for (i = 0; i < numChannels; i++)
  {
    if(tidlLayer->actParams.actType == TIDL_PRelU)
    {
      if(tidlLayer->weightsElementSizeInBits <= 8)
      {
        preluScale = ((int8_t*)slopePtr)[i];
        slopeFactfp32 = (((float32_tidl)preluScale * (float32_tidl)(1U << 8U)) / tidlLayer->actParams.slopeScale);
        algBatchNormParams->slopeFact[i] = (int32_t)slopeFactfp32;
      }
      else
      {
        preluScale = ((int16_t*)slopePtr)[i];
        slopeFactfp32 = (((float32_tidl)preluScale * (float32_tidl)(1U << 8U)) / tidlLayer->actParams.slopeScale);
        slopeFactfp32 = slopeFactfp32 * (float32_tidl)256.0f;
        algBatchNormParams->slopeFact[i] = (int32_t)slopeFactfp32;
      }
    }
    else
    {
      algBatchNormParams->slopeFact[i] = 1;
    }
  }

  /* Values for channels are same across higher dimensions and hence can be broadcasted. Starting from first index to prevent unnecessary copy in 4D cases*/
  for (index= 1; index < flattenedHigherDims; index++)
  {
    (void)memcpy((void*)(((int8_t *) algBatchNormParams->slopeFact) + (index * slopeFactSizePerDim)),
            (const void*)algBatchNormParams->slopeFact,
            slopeFactSizePerDim);
  }
  }
  return;
}


int32_t TIDL_batchNormInitWeightParamMem(const TIDL_NetworkCommonParams *commonParams,
                                         int32_t layerIdx,
                                         sTIDL_AlgLayer_t *algLayer,
                                         int32_t weightParamSize,
                                         int32_t *paramMemTabOffset,
                                         uint8_t *memory[TIDL_LAYER_MEMORY_MAX]) /*basePtr = params->net*/
{
  int32_t status = IALG_EOK;
  sTIDL_Layer_t *tidlLayer  = &commonParams->net->TIDLLayers[layerIdx];
  sTIDL_ALgBatchNormParams_t * algBatchNormParams = &algLayer->layerParams.batchNormParams;
  int32_t weightParamSizeLocal = weightParamSize;

  algBatchNormParams->weightParamSize = weightParamSizeLocal;

  TIDL_AllocatePtr((intptr_t)memory[TIDL_LAYER_MEMORY_PERSISTENT],
                                    paramMemTabOffset,
                                    algBatchNormParams->weightParamSize,
                                    TIDL_ALIGNMENT_SIZE,
                                    &algBatchNormParams->weightParamMem);

  if (commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_BATCHNORM_FORCE_WEIGHT_PARAM_ZERO)
  {
    weightParamSizeLocal = 0;
  }

  if((weightParamSizeLocal != 0) && (algBatchNormParams->weightParamMem != NULL))
  {
    int32_t flattenedHigherDims = (tidlLayer->outData.dimValues[TIDL_DIM_DIM1] * tidlLayer->outData.dimValues[TIDL_DIM_DIM2]);
    int32_t weightParamSizePerDim = algBatchNormParams->weightParamSize / flattenedHigherDims;
    void * weightPtr  = get_int8_t_pointer((int8_t *)(commonParams->net),tidlLayer->layerParams.batchNormParams.weights);

    /* Values for channels are same across higher dimensions and hence can be broadcasted. Starting from first index to prevent unnecessary copy in 4D cases*/
    for (int32_t i = 0; i < flattenedHigherDims; i++)
    {
      (void)memcpy((void*)(((int8_t *) algBatchNormParams->weightParamMem) + (i * weightParamSizePerDim)),
              (const void*)weightPtr,
              weightParamSizePerDim);
    }
  }

  else
  {
    status = TIDL_ERR_FAILURE;
  }

  return status;
}

static void TIDL_batchNormInitLutParamMem(sTIDL_Network_t *net,
                                         int32_t layerIdx,
                                         sTIDL_AlgLayer_t *algLayer,
                                         int32_t lutParamSize,
                                         int32_t *paramMemTabOffset,
                                         uint8_t *memory[TIDL_LAYER_MEMORY_MAX]) /*basePtr = params->net*/
{
  sTIDL_Layer_t *tidlLayer  = &net->TIDLLayers[layerIdx];
  sTIDL_AlgNonLinearActivation_LUT_Params_t * alglutParams = &algLayer->layerParams.batchNormParams.lutParams;

  alglutParams->lutParamSize = lutParamSize;
  alglutParams->factorB = tidlLayer->actParams.lutParams.factorB;
  alglutParams->factorC = tidlLayer->actParams.lutParams.factorC;

  TIDL_AllocatePtr((intptr_t)memory[TIDL_LAYER_MEMORY_PERSISTENT],
                                    paramMemTabOffset,
                                    alglutParams->lutParamSize,
                                    TIDL_ALIGNMENT_SIZE,
                                    &alglutParams->lutParamMem);
  if((lutParamSize != 0) && (alglutParams->lutParamMem != NULL))
  {
    void * lutDataPtr  = get_int8_t_pointer((int8_t *)(net),tidlLayer->actParams.lutParams.lutoffset);
    (void)memcpy((void*)alglutParams->lutParamMem, (void*)lutDataPtr, lutParamSize);
  }
  else
  {
    /* do nothing */
  }
  return;
}

/**
*******************************************************************************
*  \brief      Function to supply memory requirement for batchNorm layer to user
*
*
*  \param [in]  specificParams :  Pointer to specific parameter required for all the layers
*
*  \param [in]  commonParams :  Pointer to common parameter required for all the layers
*
*  \param [in]  layerIdx           : Layer Id of the layer
*
*  \param [out]  memorySize           : Output memory requirement of the layer
*
*  \return     Error:  IALG_EOK ( Successful) else failure
*
*  \remarks
*******************************************************************************
*/
int32_t TIDL_batchNormAllocNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                                   const TIDL_NetworkCommonParams *commonParams,
                                   int32_t layerIdx,
                                   int32_t memorySize[TIDL_LAYER_MEMORY_MAX])

{
  int32_t status = IALG_EOK;
  int32_t biasParamSize;
  int32_t slopeFactSize;
  int32_t weightParamSize;
  int32_t lutParamSize;

  TIDL_getPersistentMemorySize(commonParams->net,
                            layerSpecificParams->workloadUnit,
                            layerIdx,
                            &biasParamSize,
                            &slopeFactSize,
                            &weightParamSize,
                            &lutParamSize
                            );

  #ifdef HOST_EMULATION
  if (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != 0U)
  {
    status = TIDL_deviceCommonRefAlloc(layerSpecificParams, commonParams, layerIdx, memorySize);
  }
  else
  #endif
  {
    status = TIDL_deviceUtilsCommonAlloc(layerSpecificParams, commonParams, layerIdx, memorySize);
  }


  memorySize[TIDL_LAYER_MEMORY_PERSISTENT] += biasParamSize + TIDL_ALIGNMENT_SIZE;
  memorySize[TIDL_LAYER_MEMORY_PERSISTENT] += slopeFactSize + TIDL_ALIGNMENT_SIZE;
  memorySize[TIDL_LAYER_MEMORY_PERSISTENT] += weightParamSize + TIDL_ALIGNMENT_SIZE;
  memorySize[TIDL_LAYER_MEMORY_PERSISTENT] += lutParamSize + TIDL_ALIGNMENT_SIZE;

  return status;
}



/**
*******************************************************************************
*  \brief      Function to set memory needed by batchNorm layer
*
*
*  \param [in]  specificParams :  Pointer to specific parameter required for all the layers
*
*  \param [in]  commonParams :  Pointer to common parameter required for all the layers
*  \param [in]  memory           : Allocated memory pointers
*
*  \param [out]  memorySize           : Output memory requirement of the layer
*
*  \param [out]  outPtr           : Output memory pointer. Applicable only for reference flow
*
*  \return     Error:  IALG_EOK ( Successful) else failure
*
*  \remarks
*******************************************************************************
*/
int32_t TIDL_batchNormInitNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                                const TIDL_NetworkCommonParams  *commonParams,
                                sTIDL_AlgLayer_t               *algLayer,
                                int32_t layerIdx,
                                uint8_t *memory[TIDL_LAYER_MEMORY_MAX],
                                int32_t memorySize[TIDL_LAYER_MEMORY_MAX],
                                void    **outPtr)

{
  int32_t status = IALG_EOK;
  int32_t currOffset = 0;
  int32_t biasParamSize;
  int32_t slopeFactSize;
  int32_t weightParamSize;
  int32_t lutParamSize;
  /*Copy over lutParams from tidlLayer in commanParams to algLayer:*/
  sTIDL_Layer_t *tidlLayer = &commonParams->net->TIDLLayers[layerIdx];
  algLayer->layerParams.batchNormParams.lutParams.factorB = tidlLayer->actParams.lutParams.factorB;
  algLayer->layerParams.batchNormParams.lutParams.factorC = tidlLayer->actParams.lutParams.factorC;
  algLayer->layerParams.batchNormParams.lutParams.nonLinearActMethod = tidlLayer->actParams.lutParams.nonLinearActMethod;

  #if !defined (__C7504__)
  status = TIDL_batchNormInitLUTParams(commonParams,
                                      layerIdx,
                                      algLayer,
                                      commonParams->tidlCommonParams->sysMems);

  if(status != IALG_EOK)
  {
    status = IALG_EFAIL;
  }
  else
  #endif
  {
    TIDL_getPersistentMemorySize(commonParams->net,
                                layerSpecificParams->workloadUnit,
                                layerIdx,
                                &biasParamSize,
                                &slopeFactSize,
                                &weightParamSize,
                                &lutParamSize);

    /* Initialize Bias Param Mem */
    status = TIDL_batchNormInitBiasParamMem(commonParams->net,
                                  layerIdx,
                                  algLayer,
                                  biasParamSize,
                                  &currOffset,
                                  memory);
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    This condition is maintained to avoid null pointer dereference exceptions.
    Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
    <justification end> */
    if (status == IALG_EOK) {
    /* LDRA_JUSTIFY_END */
    /* Initialize slope Factor Mem */
    TIDL_batchNormInitSlopeFactMem(commonParams->net,
                                  layerIdx,
                                  algLayer,
                                  slopeFactSize,
                                  &currOffset,
                                  memory);
    /* Initialize weight param Mem */
    status = TIDL_batchNormInitWeightParamMem(commonParams,
                                  layerIdx,
                                  algLayer,
                                  weightParamSize,
                                  &currOffset,
                                  memory);
    
    /* Initialize lut param Mem */
    TIDL_batchNormInitLutParamMem(commonParams->net,
                                  layerIdx,
                                  algLayer,
                                  lutParamSize,
                                  &currOffset,
                                  memory);
    }
    /* APIs calculate offset with regard to base ptr. In this situation, the device utils API would not
    * know that the memory has already been utilized. Thus we add an offset equal to the size of memory
    * space already allocated and remove the offset after the API has run so the caller function sees the
    * same memory.
    **/
   if (status == IALG_EOK)
   {
    memory[TIDL_LAYER_MEMORY_PERSISTENT] += TIDL_ALIGN_CEIL(currOffset, TIDL_ALIGNMENT_SIZE);

    #ifdef HOST_EMULATION
    if (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != 0U)
    {
      status = TIDL_deviceCommonRefInit(layerSpecificParams,
                                  commonParams,
                                  algLayer,
                                  layerIdx,
                                  memory,
                                  memorySize,
                                  outPtr);
    }
    else
	  #endif
    {
      status = TIDL_deviceUtilsCommonInit(layerSpecificParams,
                                        commonParams,
                                        algLayer,
                                        layerIdx,
                                        memory,
                                        memorySize,
                                        outPtr);
    }

    memory[TIDL_LAYER_MEMORY_PERSISTENT] -= TIDL_ALIGN_CEIL(currOffset, TIDL_ALIGNMENT_SIZE);
    memorySize[TIDL_LAYER_MEMORY_PERSISTENT] += TIDL_ALIGN_CEIL(currOffset, TIDL_ALIGNMENT_SIZE);
   }
   else
   {
    status = IALG_EFAIL;
   }

  }

  return status;
}



int32_t TIDL_batchNormProcessNew(TIDL_NetworkCommonParams *commonParams,
                              sTIDL_AlgLayer_t     * algLayer,
                              sTIDL_Layer_t        * tidlLayer,
                              void                 * inPtrs[],
                              void                 * outPtrs[],
                              int32_t                layerIdx)

{
  int32_t status = IALG_EOK;

  if(commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_BATCHNORM_NONLUTCORE_FORCE_NETVERSION)
  {
    commonParams->net->netVersion = 0x20241120;
  }
  if(commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_BATCHNORM_FLOATNONLINEAR_FORCE_NETVERSION)
  {
    commonParams->net->netVersion = 0x20241120;
  }
  
  /* Setup various pointers needed by convolution layers */
  #ifdef HOST_EMULATION
  if (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != 0U)
  {
    status = TIDL_batchNormRefProcess(commonParams,
                                   algLayer,
                                   tidlLayer,
                                   inPtrs,
                                   outPtrs,
                                   layerIdx);
  }
  else
  #endif
  {
    TIDL_batchNormDmaswLutMem(commonParams, algLayer);
    status = TIDL_deviceUtilsCommonProcess(commonParams,
                                   algLayer,
                                   tidlLayer,
                                   inPtrs,
                                   outPtrs,
                                   layerIdx);

  }

  return status;
}

