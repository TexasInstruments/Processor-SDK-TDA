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
 *  \file tidl_batchNorm_ref.c
 *
 *  \brief This file defines kernel functions for Batch Norm layer
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include <cmath>
#include "tidl_alg_utils.h"
#include <limits>
#include "tidl_lut_module.h"
#include "tidl_batchNorm_ref.h"
#include "tidl_forceNegativeTest.h"
#ifdef BUILD_WITH_CUDA
#include "tidl_cuda.h"
#endif

using namespace floating_point::bf16_c7x;



//#define TIDL_REF_BATCH_NORM_DEBUG
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
/**
 * @brief Function will do multiplication with slope and rounding
 *
 * @param out  : Input value that requires rounding with slope
 * @param slopeFact : The amount of slope that need to be multiplied
 * @param slopeQBits : roundbits for rounding
 * @return int32_t : output after rounding with slope
 */
int32_t TIDL_refBatchNormCoreRoundSlope(
  int32_t out,
  int32_t slopeFact,
  int32_t slopeQBits)
{
  int32_t temp_out = out;
#ifdef TIDL_COVERAGE_DEAD_CODE
  if(out < 0)
  {
    temp_out = (((int64_t)out * slopeFact) >> (uint64_t)slopeQBits);
  }
#else
  temp_out = (((int64_t)out * slopeFact) >> (uint64_t)slopeQBits);
#endif
  return temp_out;
}
/**
 * @brief This is main function perform batchNorm on differnt elementTypes
 *
 * @tparam Tin  : template for input
 * @tparam Tw   : template for weights
 * @tparam Tb   : template for Bias
 * @tparam Tout : template for output
 * @tparam Tsat : template for saturate values
 * @param inPtr : Input pointer on which bacthNorm is applied
 * @param outPtr : Output pointer after batchNorm opreation
 * @param weightsPtr : Pointer to weights buffer
 * @param slopePtr : Pointer to the Slope buffer
 * @param biasPtr  : Pointer to the Bias values
 * @param intAlgHandle : tidl algorothm handle
 * @param layerIdx :index of the current layer
 * @param params : copy of bacthNorm parameters
 * @param algLayer : Pointer to the layer specific parameters
 * @param inDataParams : parameters of the input data buffer
 * @param outDataParams : parameters of the output data buffer
 * @param satLow : min value for the saturation
 * @param satHigh : max value for the saturation
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
template <class Tin, class Tw, class Tb, class Tout, class Tacc>
static int32_t TIDL_refBatchNormCore(Tin  *inPtr,
                                     Tout *outPtr,
                                     const Tw *weightsPtr,
                                     const Tw *slopePtr,
                                     const Tb *biasPtr,
                                     Tacc     *refAccPtr,
                                     TIDL_Handle intAlgHandle,
                                     int32_t layerIdx,
                                     sTIDL_BatchNormParams_t *params,
                                     sTIDL_AlgLayer_t *algLayer,
                                     const sTIDL_DataParams_t *inDataParams,
                                     const sTIDL_DataParams_t *outDataParams)
{
  int32_t status = TIDL_SUCCESS;
  Tacc     *temp_refAccPtr = refAccPtr;
  sTIDL_Network_t * net = intAlgHandle->createParams->net;
  int32_t i0, i1, i2, i3, i4, i5;
  int32_t imWidth    = (int32_t)inDataParams->dimValues[TIDL_DIM_WIDTH];
  int32_t imHeight   = (int32_t)inDataParams->dimValues[TIDL_DIM_HEIGHT];
  int32_t numDIM1    = (int32_t)inDataParams->dimValues[TIDL_DIM_DIM1];
  int32_t numDIM2    = (int32_t)inDataParams->dimValues[TIDL_DIM_DIM2];
  int32_t inPitch    = (int32_t)inDataParams->pitch[TIDL_LINE_PITCH];
  int32_t inChPitch  = (int32_t)inDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t inDIM1Pitch = (int32_t)inDataParams->pitch[TIDL_DIM1_PITCH];
  int32_t inDIM2Pitch = (int32_t)inDataParams->pitch[TIDL_DIM2_PITCH];
  int32_t inBatchPitch  = (int32_t)inDataParams->pitch[TIDL_ROI_PITCH];
  int32_t outPitch   = (int32_t)outDataParams->pitch[TIDL_LINE_PITCH];
  int32_t outChPitch = (int32_t)outDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t outDIM1Pitch = (int32_t)outDataParams->pitch[TIDL_DIM1_PITCH];
  int32_t outDIM2Pitch = (int32_t)outDataParams->pitch[TIDL_DIM2_PITCH];
  int32_t outBatchPitch = (int32_t)outDataParams->pitch[TIDL_ROI_PITCH];
  Tin  *inData = (Tin *)inPtr;
  Tout *outData = (Tout *)outPtr;
  Tacc out = 0;
  int32_t numTotBatches = inDataParams->dimValues[TIDL_DIM_BATCH];
  int32_t accMemSize;
  Tw weightVal;
  int32_t slopeQBits = 0;
  int32_t slopeFact  = 1;
  Tw preluScale = Tw(1.0f);
  uint32_t accumulatorSize = (uint32_t)(sizeof(*refAccPtr));

  params->numChannels = inDataParams->dimValues[TIDL_DIM_NUMCH];

  accMemSize = (numTotBatches * outBatchPitch* (int32_t)accumulatorSize);

  if(intAlgHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_BATCHNORM_FORCE_REF_BATCHNORM_CORE_ACCMEM_SCRATCH_NULL)
  {
    algLayer->scratchSize = 0;
  }

  if(algLayer->scratchSize >= accMemSize)
  {
     temp_refAccPtr = (Tacc *)algLayer->scratchMem;
  }
  else
  {
    tidl_printf(0,"Memory for  TIDL_refBatchNormCore accumulator is not sufficient exiting...\n    ");
    status = TIDL_ERR_FAILURE;
  }

  int32_t numChannels = params->numChannels;
  Tacc min = 0, max = 0;

  max = std::numeric_limits<Tacc>::lowest();
  min = std::numeric_limits<Tacc>::max();
  if(TIDL_SUCCESS == status)
  {
    /* OPENACC(data copyin(inData[: 1 + ((numTotBatches-1)*inBatchPitch) + ((numChannels-1)*inChPitch) + ((imHeight-1)*inPitch) + (imWidth-1)], weightsPtr[:numChannels-1], biasPtr[:numChannels-1], slopePtr[:numChannels-1]) \
            copyout(refAccPtr[: numTotBatches * outBatchPitch]))
    OPENACC(parallel loop collapse(5)) */
    #ifdef BUILD_WITH_CUDA_BATCHNORM
    // call CUDA batchnorm wrapper
    status = TIDL_cudaBatchNorm<Tin, Tw, Tb, Tacc>(inData, weightsPtr, slopePtr, biasPtr, temp_refAccPtr,
                                       numTotBatches, numChannels, imWidth, imHeight, numDIM1, numDIM2,
                                       inBatchPitch, inChPitch, inDIM1Pitch, inDIM2Pitch, inPitch,
                                       outBatchPitch, outChPitch, outDIM1Pitch, outDIM2Pitch, outPitch,
                                       intAlgHandle->createParams->net->TIDLLayers[layerIdx].actParams.actType,
                                       intAlgHandle->createParams->net->TIDLLayers[layerIdx].actParams.slopeScale);
    #else
    for (i5 = 0; i5 < numTotBatches; i5++)
    {
      for (i4 = 0; i4 < numDIM1; i4++)
      {
        for (i3 = 0; i3 < numDIM2; i3++)
        {
          Tin  inDataVal;
          for (i2 = 0; i2 < numChannels; i2++)
          {
            weightVal = weightsPtr[i2];
            if(intAlgHandle->createParams->net->TIDLLayers[layerIdx].actParams.actType == TIDL_PRelU)
            {
              preluScale = slopePtr[i2];
              slopeQBits = 8;
              int32_t tempSlopeFact = (int32_t)preluScale*(((int32_t)1) << slopeQBits);
              float32_tidl floatSlopeFact = (float32_tidl)(tempSlopeFact) /
              intAlgHandle->createParams->net->TIDLLayers[layerIdx].actParams.slopeScale;
              slopeFact = (int32_t)(floatSlopeFact);
              /* for 16-bit weights the deviation is more because of float to int conversion : TIDL-1332 */
              if(sizeof(Tw) == sizeof(int16_t))
              {
                slopeFact = (int32_t)(floatSlopeFact*256);
                slopeQBits += 8;
              }
            }


            for (i0=0; i0<imHeight; i0++)
            {
    #ifdef TIDL_REF_BATCH_NORM_DEBUG
              printf("i = %d \n",i0);
    #endif
              for (i1=0; i1<imWidth; i1++)
              {
                inDataVal = (Tin)(inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1]);
                out = (((Tacc)inDataVal* (Tacc)weightVal) + (biasPtr[i2]));
                if (out < 0)
                {
                  if (typeid(Tacc) == typeid(float32_tidl))
                  {
                    out = (Tacc)out * (Tacc)preluScale;
                  }
                  else
                  {
                    out = TIDL_refBatchNormCoreRoundSlope((int32_t)out, slopeFact, slopeQBits);
                  }
                }
                min = (out<min) ? out : min;
                max = (out>max) ? out : max;

    #ifdef TIDL_REF_BATCH_NORM_DEBUG
                printf("[%d , %d, %d, %d, %d], ", i1, inDataVal, weightVal, biasPtr[i2], out);
    #endif
                temp_refAccPtr[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = out;
              }
    #ifdef TIDL_REF_BATCH_NORM_DEBUG
                printf("\n");
    #endif
            }
          }
        }
      }
    }
    #endif

    if(((uint32_t)intAlgHandle->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) == TIDL_FLOW_CTRL_REF_STAT)
    {
      if(TIDL_getDatElementSign(outDataParams->elementType) == 1)
      {
        TIDL_UpdateScaleFactors(intAlgHandle, layerIdx, 1, (int64_t)min, (int64_t)max);
      }
      else
      {
        TIDL_UpdateScaleFactors(intAlgHandle, layerIdx, 1, 0, (int64_t)max);
      }
    }

    int32_t satLow;
    int32_t satHigh;
    int32_t mixedPrecision = 0;
    int32_t outRoundBits;
    Tout temp = Tout();
    temp  = std::numeric_limits<Tout>::lowest();
    satLow = (int32_t)temp;
    temp  = std::numeric_limits<Tout>::max();

    satHigh = (int32_t)(temp);
    outRoundBits = outDataParams->roundBits;
    if ( sizeof(*weightsPtr) != sizeof(*outData))
    {
      mixedPrecision = 1;
      /* This is to match with target implementation wherin the saturations
    happens after applying outRoundBits - 8 shift and then the right shift
    of 8 is applied */
      outRoundBits = outRoundBits - 8;
      if ( (net->TIDLLayers[layerIdx].outData.elementType == TIDL_SignedChar) || (net->TIDLLayers[layerIdx].outData.elementType == TIDL_SignedShort)  )
      {
        satLow  = std::numeric_limits<int16_t>::lowest();
        satHigh = std::numeric_limits<int16_t>::max();
      }
      else
      {
        satLow  = std::numeric_limits<uint16_t>::lowest();
        satHigh = std::numeric_limits<uint16_t>::max();
      }
    }

    /* OPENACC(data copyout(outData[: 1 + ((numTotBatches-1)*outBatchPitch) + ((numChannels-1)*outChPitch) + ((imHeight-1)*outPitch) + (imWidth-1)]) \
                copyin(refAccPtr[: numTotBatches * outBatchPitch]))
    OPENACC(parallel loop collapse(6)) */
    #ifdef BUILD_WITH_CUDA_BATCHNORM
    // call CUDA batchnorm saturation wrapper
    float floatSatLow, floatSatHigh;
    TIDL_getSaturationFloat(&net->TIDLLayers[layerIdx], &floatSatLow, &floatSatHigh);
    status = TIDL_cudaBatchNormSaturation<Tacc, Tout>(temp_refAccPtr, outData,
                                       numTotBatches, numChannels, imWidth, imHeight, numDIM1, numDIM2,
                                       outBatchPitch, outChPitch, outDIM1Pitch, outDIM2Pitch, outPitch,
                                       outRoundBits, satLow, satHigh, mixedPrecision, floatSatLow, floatSatHigh);
    #else
    for (i5=0; i5<numTotBatches; i5++)
        {
          for (i4=0; i4<numDIM1; i4++)
          {
            for (i3=0; i3<numDIM2; i3++)
            {
              for (i2=0; i2<params->numChannels; i2++)
              {
                for (i0=0; i0<imHeight; i0++)
                {
                  for (i1=0; i1<imWidth; i1++)
                  {
                    out = temp_refAccPtr[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1];
                    if (std::is_same<Tacc, float32_tidl>::value)
                    {
                      if (net->TIDLLayers[layerIdx].outData.elementType == TIDL_BFloat16)
                      {
                        out = TIDL_BF16Sat(out, &net->TIDLLayers[layerIdx]);
                      }
                      else
                      {
                        //OPENACC(routine(TIDL_floatSat))
                        out = TIDL_floatSat(out, &net->TIDLLayers[layerIdx]);
                      }
                    }
                    else
                    {
                      //OPENACC(routine(TIDL_roundSat))
                      out = (int32_t)TIDL_roundSat((int64_t)out, outRoundBits , (int32_t)satLow, (int32_t)satHigh);
                      if ( mixedPrecision == 1 )
                      {
                        out = (uint64_t)out >> 8U;
                      }
                    }
                    outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)out;
                  }
                }
              }
            }
          }
        }
        #endif
      }
      return status;
}

/**
 * @brief This is main function perform High Accuracy Sigmoid Process.
 *
 * @tparam Tin  : template for input
 * @tparam Tout : template for output
 * @param inPtr : Input pointer on which bacthNorm is applied
 * @param outPtr : Output pointer after batchNorm opreation
 * @param inDataParams : parameters of the input data buffer
 * @param outDataParams : parameters of the output data buffer
 * @param params : copy of bacthNorm parameters
 * @param satLow : min value for the saturation
 * @param satHigh : max value for the saturation
 */
template <class Tin, class Tout>
static void TIDL_HighAccuracySigmoidProcess(Tin  *inPtr,
                                            Tout *outPtr,
                                            sTIDL_BatchNormParams_t *params,
                                            sTIDL_DataParams_t *inDataParams, 
                                            sTIDL_DataParams_t *outDataParams)
{
  int32_t i0, i1, i2, i3, i4, i5;
  int32_t imWidth    = (int32_t)inDataParams->dimValues[TIDL_DIM_WIDTH];
  int32_t imHeight   = (int32_t)inDataParams->dimValues[TIDL_DIM_HEIGHT];
  int32_t inDim1   = (int32_t)inDataParams->dimValues[TIDL_DIM_DIM1];
  int32_t inDim2   = (int32_t)inDataParams->dimValues[TIDL_DIM_DIM2];
  int32_t inPitch    = (int32_t)inDataParams->pitch[TIDL_LINE_PITCH];
  int32_t inChPitch  = (int32_t)inDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t inDim1Pitch  = (int32_t)inDataParams->pitch[TIDL_DIM1_PITCH];
  int32_t inDim2Pitch  = (int32_t)inDataParams->pitch[TIDL_DIM2_PITCH];
  int32_t inBatchPitch  = (int32_t)inDataParams->pitch[TIDL_ROI_PITCH];
  int32_t outPitch   = (int32_t)outDataParams->pitch[TIDL_LINE_PITCH];
  int32_t outChPitch = (int32_t)outDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t outDim1Pitch  = (int32_t)outDataParams->pitch[TIDL_DIM1_PITCH];
  int32_t outDim2Pitch  = (int32_t)outDataParams->pitch[TIDL_DIM2_PITCH];
  int32_t outBatchPitch = (int32_t)outDataParams->pitch[TIDL_ROI_PITCH];
  int32_t numTotBatches = inDataParams->dimValues[TIDL_DIM_BATCH];
  Tin  *inData = (Tin *)inPtr;
  Tout *outData = (Tout *)outPtr;
  Tout satHigh = Tout(), satLow = Tout();
  float inValF, outValF;
    // float inValF_2;
  float inputScale = inDataParams->tensorScale;
  float inDataScaleFact = ((-1.0) * 1.44269504090f)/inputScale;
  satHigh = std::numeric_limits<Tout>::max();
  satLow = std::numeric_limits<Tout>::lowest();
  int32_t numChannels = params->numChannels;
  Tout out = 0;
  #if 0
  // call CUDA wrapper for High Accuracy Sigmoid
  TIDL_cudaHighAccuracySigmoid<Tin, Tout>(inData, outData,
                                          numTotBatches, numChannels, imWidth, imHeight, inDim1, inDim2,
                                          inBatchPitch, inChPitch, inDim1Pitch, inDim2Pitch, inPitch,
                                          outBatchPitch, outChPitch, outDim1Pitch, outDim2Pitch, outPitch,
                                          inputScale, outDataParams->tensorScale);
  #else
    // OPENACC(parallel loop collapse(4))
  for (i5 = 0; i5 < numTotBatches; i5++)
  {
    for(i4 = 0; i4 < inDim1; i4++)
    {
      for(i3 = 0; i3 < inDim2; i3++)
      {
        for (i2 = 0; i2 < numChannels; i2++)
        {
          for (i0=0; i0<imHeight; i0++)
          {
            for (i1=0; i1<imWidth; i1++)
            {
              if(typeid(Tin) == typeid(bfloat16_tidl))
              {
                using namespace floating_point::bf16_c7x;
                Tin inDataVal;
                inDataVal = (inData[(i5*inBatchPitch) + (i4*inDim1Pitch) + (i3*inDim2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1]);
                bfloat16_tidl outValF = exp_taylor_sigmoid_bf16(inDataVal);
                out = (Tout) ((outValF > (bfloat16_tidl)satHigh) ? (bfloat16_tidl)satHigh : outValF); /*Upper bound only*/
                outData[(i5*outBatchPitch) + (i4*outDim1Pitch) + (i3*outDim2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)out;
              }
              else
              {
                Tin  inDataVal;
                inDataVal = (inData[(i5*inBatchPitch) + (i4*inDim1Pitch) + (i3*inDim2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1]);
                inValF = ((float)inDataVal - inDataParams->tensorZeroPoint) * inDataScaleFact;
                outValF = div_sp(1.0, (exp_taylor_sigmoid(inValF) + 1.0f));
                outValF = outValF*outDataParams->tensorScale;
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
                  Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
                <justification end> */
                out = (Tout) ((outValF > (float32_tidl)satHigh) ? (float32_tidl)satHigh : outValF); /*Upper bound only*/
                /* LDRA_JUSTIFY_END */
                outData[(i5*outBatchPitch) + (i4*outDim1Pitch) + (i3*outDim2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)out;
              }
            }
          }
        }
      }
    }
  }
  #endif
}

/**
 * @brief This is main function perform Sigmoid on differnt elementTypes
 *
 * @tparam Tin  : template for input
 * @tparam Tout : template for output
 * @param inPtr : Input pointer on which bacthNorm is applied
 * @param outPtr : Output pointer after batchNorm opreation
 * @param intAlgHandle : tidl algorothm handle
 * @param layerIdx :index of the current layer
 * @param params : copy of bacthNorm parameters
 * @param inDataParams : parameters of the input data buffer
 * @param outDataParams : parameters of the output data buffer
 * @param slope : Pointer to slope values for sigmoid
 * @param offset : Pointer to offset values for sigmoid
 * @param satLow : min value for the saturation
 * @param satHigh : max value for the saturation
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
template <class Tin, class Tout, class Tacc >
static int32_t TIDL_refSigmoidCore(Tin  *inPtr,
                                   Tout *outPtr,
                                   TIDL_Handle intAlgHandle,
                                   sTIDL_AlgLayer_t *algLayer,
                                   int32_t layerIdx,
                                   sTIDL_BatchNormParams_t *params,
                                   sTIDL_DataParams_t *inDataParams,
                                   sTIDL_DataParams_t *outDataParams,
                                   Tout *slope,
                                   Tout *offset,
                                   Tout offsetScale,
                                   Tacc *scratchMemPtr
                                   )
{
  int32_t status = TIDL_SUCCESS;
  int32_t i0, i1, i2, i3;
  int32_t imWidth    = (int32_t)inDataParams->dimValues[TIDL_DIM_WIDTH];
  int32_t imHeight   = (int32_t)inDataParams->dimValues[TIDL_DIM_HEIGHT];
  int32_t inPitch    = (int32_t)inDataParams->pitch[TIDL_LINE_PITCH];
  int32_t inChPitch  = (int32_t)inDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t inBatchPitch  = (int32_t)inDataParams->pitch[TIDL_ROI_PITCH];
  int32_t outPitch   = (int32_t)outDataParams->pitch[TIDL_LINE_PITCH];
  int32_t outChPitch = (int32_t)outDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t outBatchPitch = (int32_t)outDataParams->pitch[TIDL_ROI_PITCH];
  int32_t numTotBatches = inDataParams->dimValues[TIDL_DIM_BATCH];
  Tacc outVal;
  params->numChannels = inDataParams->dimValues[TIDL_DIM_NUMCH];
  Tin  *inData = (Tin *)inPtr;
  Tout *outData = (Tout *)outPtr;
  Tout out = 0;
  Tout satHigh = 0, satLow = 0;

  int32_t numChannels = params->numChannels;
  /*Use the 4 point approximation method for 8-bit*/
#if defined (__C7504__)
  if(((intAlgHandle->createParams->reservedCtrl & TIDL_HIGH_ACCURACY_SIGMOID) == 0) && ((typeid(Tout) == typeid(int8_t)) || (typeid(Tout) == typeid(uint8_t))))
  {
    float inputScale = inDataParams->tensorScale;
    float thresholdScale = 16.0f; // 4-bits to avoid thresholds overflow
    uint32_t threshold0 = 5.0f*thresholdScale*inputScale;
    uint32_t threshold1 = 2.375f*thresholdScale*inputScale;
    uint32_t threshold2 = 1.0f*thresholdScale*inputScale;
    uint16_t inDataScale = thresholdScale;

    Tacc *refAccPtr;
    int32_t accMemSize;
    uint32_t accumulatorSize = (uint32_t)(sizeof(Tacc));
    accMemSize = (numTotBatches * outBatchPitch* (int32_t)accumulatorSize);
    if(intAlgHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_BATCHNORM_FORCE_REF_BATCHNORM_CORE_ACCMEM_SCRATCH_NULL)
    {
      algLayer->scratchSize = 0;
    }

    if(algLayer->scratchSize >= accMemSize)
    {
       refAccPtr = (Tacc *)scratchMemPtr;
    }
    else
    {
      tidl_printf(0,"Memory for  TIDL_refSigmoidCore accumulator is not sufficient exiting...\n    ");
      status = TIDL_ERR_FAILURE;
    }

/* OPENACC(data copyin(inData[: 1 + ((numTotBatches-1)*inBatchPitch) + ((numChannels-1)*inChPitch) + ((imHeight-1)*inPitch) + (imWidth-1)]) \
             copy(outData[: 1 + ((numTotBatches-1)*outBatchPitch) + ((numChannels-1)*outChPitch) + ((imHeight-1)*outPitch) + (imWidth-1)]) \
             present(refAccPtr[: 1 + ((numTotBatches-1)*outBatchPitch) + ((numChannels-1)*outChPitch) + ((imHeight-1)*outPitch) + (imWidth-1)])) */
{ // openacc start
    if(TIDL_SUCCESS == status)
    {
      int32_t min = 0, max = 0;
      min = std::numeric_limits<Tacc>::max();
      max = std::numeric_limits<Tacc>::lowest();
      satHigh = std::numeric_limits<Tout>::max();
      satLow = std::numeric_limits<Tout>::lowest();
      int32_t temp_min = 0, temp_max = 0;

      #if 0
      // call CUDA sigmoid kernel
      TIDL_cudaSigmoid<Tin, Tout, Tacc>(inData, refAccPtr,
                                        numTotBatches, numChannels, imWidth, imHeight,
                                        inBatchPitch, inChPitch, inPitch,
                                        outBatchPitch, outChPitch, outPitch,
                                        threshold0, threshold1, threshold2, inDataScale,
                                        slope, offset, offsetScale);
      #else
      //OPENACC(parallel loop collapse(4))
      for (i3 = 0; i3 < numTotBatches; i3++)
      {
        for (i2 = 0; i2 < numChannels; i2++)
        {
          for (i0=0; i0<imHeight; i0++)
          {
            for (i1=0; i1<imWidth; i1++)
            {
              Tin  inDataVal; 
              int32_t inDataValAbs, constZero = 0;
              uint32_t inDataValShl8;
              inDataVal = (inData[(i3*inBatchPitch) + (i2*inChPitch) + (i0*inPitch) + i1]);
              if (inDataVal < inDataParams->tensorZeroPoint)
              {
                inDataValAbs = inDataParams->tensorZeroPoint - inDataVal;
              }
              else
              {
                inDataValAbs = (int32_t)inDataVal - inDataParams->tensorZeroPoint;
              }
              inDataValShl8 = inDataValAbs * inDataScale;
              if(inDataValShl8 >= threshold0)
              {
                outVal = (slope[0] * inDataValAbs) + (offset[0]*offsetScale);
              }
              else if ((inDataValShl8 < threshold0) && (inDataValShl8 >= threshold1))
              {
                outVal = (slope[1] * inDataValAbs) + (offset[1]*offsetScale);
              }
              else if ((inDataValShl8 < threshold1) && (inDataValShl8 >= threshold2))
              {
                outVal = (slope[2] * inDataValAbs) + (offset[2]*offsetScale);
              }
              /* LDRA_JUSTIFY_START
              <metric start> statement branch <metric end>
              <justification start>
              Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
              Need conditional check for all the use case so else cannot be reached.
              Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
              This does not affect runtime behavior or safety.
              <justification end> */
              else if ((inDataValShl8 < threshold2) && (inDataValShl8 >= constZero))
              /* LDRA_JUSTIFY_END */
              {
                outVal = (slope[3] * inDataValAbs) + (offset[3]*offsetScale);
              }
              /* LDRA_JUSTIFY_START
              <metric start> statement branch <metric end>
              <justification start>
              Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
              Need conditional check for all the use case so else cannot be reached.
              Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
              This does not affect runtime behavior or safety.
              <justification end> */
			        else
			        {
				        /*do nothing*/
			        }
              /* LDRA_JUSTIFY_END */

              if(inDataVal < constZero + inDataParams->tensorZeroPoint)
              {
                outVal = (offset[0]*offsetScale) - outVal;
              }

              if(outVal<min)
              {
                temp_min = outVal;
              }
              else
              {
                temp_min = min;
              }
              min = temp_min;
              if (outVal>max)
              {
                temp_max = outVal;
              }
              else
              {
                temp_max = max;
              }
              max = temp_max;
              refAccPtr[(i3*outBatchPitch) + (i2*outChPitch) + (i0*outPitch) + i1] = outVal;
            }
          }
        }
      }
      #endif
      /* LDRA_JUSTIFY_START
      <metric start> statement branch <metric end>
      <justification start>
      Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
      This can be reached only through compilation time unreachable during inference time.
      Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
      This does not affect runtime behavior or safety.
      <justification end> */
      if(((uint32_t)intAlgHandle->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) == TIDL_FLOW_CTRL_REF_STAT)
      {
        if(TIDL_getDatElementSign(outDataParams->elementType) == 1)
        {
          TIDL_UpdateScaleFactors(intAlgHandle, layerIdx, 1, min, max);
        } 
        else
        {
          TIDL_UpdateScaleFactors(intAlgHandle, layerIdx, 1, 0, max);
        } 
      }
      /* LDRA_JUSTIFY_END */

      #if 0
      // call CUDA saturation kernel
      TIDL_cudaSigmoidSaturation<Tacc, Tout>(refAccPtr, outData,
                                            numTotBatches, numChannels, imWidth, imHeight,
                                            outBatchPitch, outChPitch, outPitch,
                                            outDataParams->roundBits, satLow, satHigh);
      #else
      //OPENACC(parallel loop collapse(4))
      for (i3=0; i3<numTotBatches; i3++)
      {
        for (i2=0; i2<params->numChannels; i2++)
        {
          for (i0=0; i0<imHeight; i0++)
          {
            for (i1=0; i1<imWidth; i1++)
            {
              outVal = refAccPtr[(i3*outBatchPitch) + (i2*outChPitch) + (i0*outPitch) + i1];
              //OPENACC(routine(TIDL_roundSat))
              out = (Tout)TIDL_roundSat(outVal, outDataParams->roundBits, satLow, satHigh);
              outData[(i3*outBatchPitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)out;
            }
          }
        }
      }
      #endif
    }
}
  } // //OPENACC end
  else
  {
    TIDL_HighAccuracySigmoidProcess(inPtr, outPtr, params, inDataParams, outDataParams);
    // float inValF, outValF;
    // // float inValF_2;
    // float inputScale = inDataParams->tensorScale;
    // float inDataScaleFact = ((-1.0) * 1.44269504090f)/inputScale;
    // satHigh = std::numeric_limits<Tout>::max();
    // satLow = std::numeric_limits<Tout>::lowest();

    // // OPENACC(parallel loop collapse(4))
    // for (i3 = 0; i3 < numTotBatches; i3++)
    // {
    //   for (i2 = 0; i2 < numChannels; i2++)
    //   {
    //     for (i0=0; i0<imHeight; i0++)
    //     {
    //       for (i1=0; i1<imWidth; i1++)
    //       {
    //         Tin  inDataVal;
    //         inDataVal = (inData[(i3*inBatchPitch) + (i2*inChPitch) + (i0*inPitch) + i1]);
    //         inValF = inDataVal * inDataScaleFact;
    //         outValF = div_sp(1.0, (exp_taylor_sigmoid(inValF) + 1.0f));
    //         outValF = outValF*outDataParams->tensorScale;
    //         out = (Tout) ((outValF > satHigh) ? satHigh : outValF); /*Upper bound only*/
    //         outData[(i3*outBatchPitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)out;
    //       }
    //     }
    //   }
    // }
  }
#else
  TIDL_HighAccuracySigmoidProcess(inData, outData, params, inDataParams, outDataParams);
#endif
  return status;
}
/**
 * @brief  This funciton handles differnt input bit-depths for reference batchNorm
 *
 * @tparam Tw   : template for weights
 * @tparam Tb   : template for Bias
 * @param inPtr : Input pointer on which bacthNorm is applied
 * @param outPtr : Output pointer after batchNorm opreation
 * @param weightsPtr : Pointer to weights buffer
 * @param slopePtr : Pointer to the Slope buffer
 * @param biasPtr  : Pointer to the Bias values
 * @param params : copy of bacthNorm parameters
 * @param intAlgHandle : tidl algorothm handle
 * @param layerIdx :index of the current layer
 * @param algLayer : Pointer to the layer specific parameters
 * @param inDataParams : parameters of the input data buffer
 * @param outDataParams : parameters of the output data buffer
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
template <class Tw, class Tb, class Tacc>
static int32_t TIDL_refBatchNormBitDepth(void *inPtr,
                                         void *outPtr,
                                         const Tw *weightsPtr,
                                         const Tw *slopePtr,
                                         const Tb *biasPtr,
                                         Tacc     *accPtr,
                                         sTIDL_BatchNormParams_t *params,
                                         TIDL_Handle intAlgHandle,
                                         int32_t layerIdx,
                                         sTIDL_AlgLayer_t *algLayer,
                                         const sTIDL_DataParams_t *inDataParams,
                                         const sTIDL_DataParams_t *outDataParams)
{
  int32_t status = TIDL_SUCCESS;
  if(TIDL_SignedChar == ((int32_t)outDataParams->elementType))
  {
    if(TIDL_SignedChar == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refBatchNormCore((int8_t *)inPtr,
                                     (int8_t *)outPtr,
                                     weightsPtr,
                                     slopePtr,
                                     biasPtr,
                                     accPtr,
                                     intAlgHandle,
                                     layerIdx,
                                     params,
                                     algLayer,
                                     inDataParams,
                                     outDataParams);
    }
    else if(TIDL_UnsignedChar == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refBatchNormCore((uint8_t *)inPtr,
                                     (int8_t *)outPtr,
                                     weightsPtr,
                                     slopePtr,
                                     biasPtr,
                                     accPtr,
                                     intAlgHandle,
                                     layerIdx,
                                     params,
                                     algLayer,
                                     inDataParams,
                                     outDataParams);
    }
    else if(TIDL_SignedShort == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refBatchNormCore((int16_t *)inPtr,
                                     (int8_t *)outPtr,
                                     weightsPtr,
                                     slopePtr,
                                     biasPtr,
                                     accPtr,
                                     intAlgHandle,
                                     layerIdx,
                                     params,
                                     algLayer,
                                     inDataParams,
                                     outDataParams);
    }
    else if(TIDL_UnsignedShort == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refBatchNormCore((uint16_t *)inPtr,
                                     (int8_t *)outPtr,
                                     weightsPtr,
                                     slopePtr,
                                     biasPtr,
                                     accPtr,
                                     intAlgHandle,
                                     layerIdx,
                                     params,
                                     algLayer,
                                     inDataParams,
                                     outDataParams);
    }
    else
    {
      status = TIDL_ERR_FAILURE;
    }
  }
  else if(TIDL_UnsignedChar == ((int32_t)outDataParams->elementType))
  {
    if(TIDL_SignedChar == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refBatchNormCore((int8_t *)inPtr,
                                     (uint8_t *)outPtr,
                                     weightsPtr,
                                     slopePtr,
                                     biasPtr,
                                     accPtr,
                                     intAlgHandle,
                                     layerIdx,
                                     params,
                                     algLayer,
                                     inDataParams,
                                     outDataParams);
    }
    else if(TIDL_UnsignedChar == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refBatchNormCore((uint8_t *)inPtr,
                                     (uint8_t *)outPtr,
                                     weightsPtr,
                                     slopePtr,
                                     biasPtr,
                                     accPtr,
                                     intAlgHandle,
                                     layerIdx,
                                     params,
                                     algLayer,
                                     inDataParams,
                                     outDataParams);
    }
    else if(TIDL_SignedShort == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refBatchNormCore((int16_t *)inPtr,
                                     (uint8_t *)outPtr,
                                     weightsPtr,
                                     slopePtr,
                                     biasPtr,
                                     accPtr,
                                     intAlgHandle,
                                     layerIdx,
                                     params,
                                     algLayer,
                                     inDataParams,
                                     outDataParams);
    }
    else if(TIDL_UnsignedShort == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refBatchNormCore((uint16_t *)inPtr,
                                     (uint8_t *)outPtr,
                                     weightsPtr,
                                     slopePtr,
                                     biasPtr,
                                     accPtr,
                                     intAlgHandle,
                                     layerIdx,
                                     params,
                                     algLayer,
                                     inDataParams,
                                     outDataParams);
    }
    else
    {
      status = TIDL_ERR_FAILURE;
    }
  }
  else if(TIDL_SignedShort == ((int32_t)outDataParams->elementType))
  {
    if(TIDL_SignedChar == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refBatchNormCore((int8_t *)inPtr,
                                     (int16_t *)outPtr,
                                     weightsPtr,
                                     slopePtr,
                                     biasPtr,
                                     accPtr,
                                     intAlgHandle,
                                     layerIdx,
                                     params,
                                     algLayer,
                                     inDataParams,
                                     outDataParams);
    }
    else if(TIDL_UnsignedChar == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refBatchNormCore((uint8_t *)inPtr,
                                     (int16_t *)outPtr,
                                     weightsPtr,
                                     slopePtr,
                                     biasPtr,
                                     accPtr,
                                     intAlgHandle,
                                     layerIdx,
                                     params,
                                     algLayer,
                                     inDataParams,
                                     outDataParams);
    }
    else if(TIDL_SignedShort == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refBatchNormCore((int16_t *)inPtr,
                                     (int16_t *)outPtr,
                                     weightsPtr,
                                     slopePtr,
                                     biasPtr,
                                     accPtr,
                                     intAlgHandle,
                                     layerIdx,
                                     params,
                                     algLayer,
                                     inDataParams,
                                     outDataParams);
    }
    else if(TIDL_UnsignedShort == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refBatchNormCore((uint16_t *)inPtr,
                                     (int16_t *)outPtr,
                                     weightsPtr,
                                     slopePtr,
                                     biasPtr,
                                     accPtr,
                                     intAlgHandle,
                                     layerIdx,
                                     params,
                                     algLayer,
                                     inDataParams,
                                     outDataParams);
    }
    else
    {
      status = TIDL_ERR_FAILURE;
    }
  }
  else if(TIDL_UnsignedShort == ((int32_t)outDataParams->elementType))
  {
    if(TIDL_SignedChar == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refBatchNormCore((int8_t *)inPtr,
                                     (uint16_t *)outPtr,
                                     weightsPtr,
                                     slopePtr,
                                     biasPtr,
                                     accPtr,
                                     intAlgHandle,
                                     layerIdx,
                                     params,
                                     algLayer,
                                     inDataParams,
                                     outDataParams);
    }
    else if(TIDL_UnsignedChar == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refBatchNormCore((uint8_t *)inPtr,
                                     (uint16_t *)outPtr,
                                     weightsPtr,
                                     slopePtr,
                                     biasPtr,
                                     accPtr,
                                     intAlgHandle,
                                     layerIdx,
                                     params,
                                     algLayer,
                                     inDataParams,
                                     outDataParams);
    }
    else if(TIDL_SignedShort == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refBatchNormCore((int16_t *)inPtr,
                                     (uint16_t *)outPtr,
                                     weightsPtr,
                                     slopePtr,
                                     biasPtr,
                                     accPtr,
                                     intAlgHandle,
                                     layerIdx,
                                     params,
                                     algLayer,
                                     inDataParams,
                                     outDataParams);
    }
    else if(TIDL_UnsignedShort == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refBatchNormCore((uint16_t *)inPtr,
                                     (uint16_t *)outPtr,
                                     weightsPtr,
                                     slopePtr,
                                     biasPtr,
                                     accPtr,
                                     intAlgHandle,
                                     layerIdx,
                                     params,
                                     algLayer,
                                     inDataParams,
                                     outDataParams);
    }
    else
    {
      status = TIDL_ERR_FAILURE;
    }
  }
  else if(TIDL_SinglePrecFloat == ((int32_t)outDataParams->elementType))
  {
    status = TIDL_refBatchNormCore((float32_tidl *)inPtr,
                                    (float32_tidl *)outPtr,
                                    weightsPtr,
                                    slopePtr,
                                    biasPtr,
                                    accPtr,
                                    intAlgHandle,
                                    layerIdx,
                                    params,
                                    algLayer,
                                    inDataParams,
                                    outDataParams);
  }
  else if(TIDL_BFloat16 == ((int32_t)outDataParams->elementType))
  {
    status = TIDL_refBatchNormCore((bfloat16_tidl *)inPtr,
                                    (bfloat16_tidl *)outPtr,
                                    weightsPtr,
                                    slopePtr,
                                    biasPtr,
                                    accPtr,
                                    intAlgHandle,
                                    layerIdx,
                                    params,
                                    algLayer,
                                    inDataParams,
                                    outDataParams);
  }
  else
  {
    status = TIDL_ERR_FAILURE;
  }
  return status;
}
/**
 * @brief This function is sigmoid reference implementation
 *
 * @param intAlgHandle : tidl algorothm handle
 * @param algLayer : Pointer to the layer specific parameters
 * @param tidlLayer : Pointer to the common layer parameters
 * @param params : copy of batch norm layer parameters
 * @param inPtr : Pointer to input buffers to be processed
 * @param outPtr : Pointer to output buffers to be processed
 * @param inDataParams : pointer to input data parameters
 * @param outDataParams : pointer to output data parameters
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
static int32_t TIDL_refSigmoidProcess(TIDL_Handle intAlgHandle,
                                      sTIDL_AlgLayer_t *algLayer,
                                      const sTIDL_Layer_t    *tidlLayer,
                                      sTIDL_BatchNormParams_t *params,
                                      void *inPtr,
                                      void *outPtr,
                                      sTIDL_DataParams_t *inDataParams,
                                      sTIDL_DataParams_t *outDataParams)
{
  int32_t status = TIDL_SUCCESS;
  int32_t layerIdx = algLayer->layerIdx;
  if(TIDL_UnsignedShort == ((int32_t)outDataParams->elementType))
  {
    /*
    slope = 0, 0.03125, 0.125, 0.25
    offset = 1, 0.84375, 0.625, 0.5
    Convert Slope and offset values to 15-bits (NUM_WHGT_BITS-1) */
    uint16_t TIDL_sigmoidSlope[4] = {0, 1024, 4096, 8192};
    uint16_t TIDL_sigmoidOffset[4] = {32768, 27648, 20480, 16384};
    uint16_t OffsetScale = 1;
    if (inDataParams->tensorScale > 1.0)
    {
      OffsetScale = (uint16_t)(inDataParams->tensorScale);
      TIDL_sigmoidOffset[0] = ((float32_tidl)TIDL_sigmoidOffset[0]*inDataParams->tensorScale)/(float32_tidl)OffsetScale;
      TIDL_sigmoidOffset[1] = ((float32_tidl)TIDL_sigmoidOffset[1]*inDataParams->tensorScale)/(float32_tidl)OffsetScale;
      TIDL_sigmoidOffset[2] = ((float32_tidl)TIDL_sigmoidOffset[2]*inDataParams->tensorScale)/(float32_tidl)OffsetScale;
      TIDL_sigmoidOffset[3] = ((float32_tidl)TIDL_sigmoidOffset[3]*inDataParams->tensorScale)/(float32_tidl)OffsetScale;
    }
    else
    {
      OffsetScale = 1;
      TIDL_sigmoidOffset[0] = (float32_tidl)TIDL_sigmoidOffset[0]*inDataParams->tensorScale;
      TIDL_sigmoidOffset[1] = (float32_tidl)TIDL_sigmoidOffset[1]*inDataParams->tensorScale;
      TIDL_sigmoidOffset[2] = (float32_tidl)TIDL_sigmoidOffset[2]*inDataParams->tensorScale;
      TIDL_sigmoidOffset[3] = (float32_tidl)TIDL_sigmoidOffset[3]*inDataParams->tensorScale;
    }

    if(TIDL_SignedShort == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refSigmoidCore((int16_t*)inPtr,
                                   (uint16_t*)outPtr,
                                   intAlgHandle,
                                   algLayer,
                                   layerIdx,
                                   params,
                                   inDataParams,
                                   outDataParams,
                                   TIDL_sigmoidSlope,
                                   TIDL_sigmoidOffset,
                                   (uint16_t)OffsetScale,
                                   (int32_t*)algLayer->scratchMem
                                   );
    }

    else if(TIDL_UnsignedShort == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refSigmoidCore((uint16_t*)inPtr,
                                   (uint16_t*)outPtr,
                                   intAlgHandle,
                                   algLayer,
                                   layerIdx,
                                   params,
                                   inDataParams,
                                   outDataParams,
                                   TIDL_sigmoidSlope,
                                   TIDL_sigmoidOffset,
                                   (uint16_t)OffsetScale,
                                   (int32_t*)algLayer->scratchMem
                                   );
    }
	  else /* Mixed precision is not supported in Sigmoid. */
	  {
	  	status = TIDL_ERR_NOT_IMPLEMENTED;
	  }
  }
  else if(TIDL_UnsignedChar == ((int32_t)outDataParams->elementType))
  {
    /*
    slope = 0, 0.03125, 0.125, 0.25
    offset = 1, 0.84375, 0.625, 0.5
    Convert Slope and offset values to 7-bits (NUM_WHGT_BITS-1) */
    uint8_t TIDL_sigmoidSlope[4] = {0, 4, 16, 32};
    uint8_t TIDL_sigmoidOffset[4] = {128, 108, 80, 64};
    uint8_t OffsetScale = 1;
    /* Divide 16-bit offsets to two 8-bit offsets(Offset0, OffsetScale) and so on */
    if (inDataParams->tensorScale > 1.0)
    {
      OffsetScale = (uint8_t)(inDataParams->tensorScale);
      TIDL_sigmoidOffset[0] = ((float32_tidl)TIDL_sigmoidOffset[0]*inDataParams->tensorScale)/(float32_tidl)OffsetScale;
      TIDL_sigmoidOffset[1] = ((float32_tidl)TIDL_sigmoidOffset[1]*inDataParams->tensorScale)/(float32_tidl)OffsetScale;
      TIDL_sigmoidOffset[2] = ((float32_tidl)TIDL_sigmoidOffset[2]*inDataParams->tensorScale)/(float32_tidl)OffsetScale;
      TIDL_sigmoidOffset[3] = ((float32_tidl)TIDL_sigmoidOffset[3]*inDataParams->tensorScale)/(float32_tidl)OffsetScale;
    }
    else
    {
      OffsetScale = 1;
      TIDL_sigmoidOffset[0] = (float32_tidl)TIDL_sigmoidOffset[0]*inDataParams->tensorScale;
      TIDL_sigmoidOffset[1] = (float32_tidl)TIDL_sigmoidOffset[1]*inDataParams->tensorScale;
      TIDL_sigmoidOffset[2] = (float32_tidl)TIDL_sigmoidOffset[2]*inDataParams->tensorScale;
      TIDL_sigmoidOffset[3] = (float32_tidl)TIDL_sigmoidOffset[3]*inDataParams->tensorScale;
    }

    if(TIDL_SignedChar == ((int32_t)inDataParams->elementType)) 
    {
      status = TIDL_refSigmoidCore((int8_t*)inPtr,
                                  (uint8_t*)outPtr,
                                   intAlgHandle,
                                   algLayer,
                                   layerIdx,
                                   params,
                                   inDataParams,
                                   outDataParams,
                                   TIDL_sigmoidSlope,
                                   TIDL_sigmoidOffset,
                                   (uint8_t)OffsetScale,
                                   (int32_t*)algLayer->scratchMem
                                   );
    }
    else if(TIDL_UnsignedChar == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refSigmoidCore((uint8_t*)inPtr,
                                  (uint8_t*)outPtr,
                                   intAlgHandle,
                                   algLayer,
                                   layerIdx,
                                   params,
                                   inDataParams,
                                   outDataParams,
                                   TIDL_sigmoidSlope,
                                   TIDL_sigmoidOffset,
                                   (uint8_t)OffsetScale,
                                   (int32_t*)algLayer->scratchMem
                                   );
    }
	  else /* Mixed precision is not supported in Sigmoid. */ 
	  {
	  	status = TIDL_ERR_NOT_IMPLEMENTED;
	  }
  }
  else if(TIDL_BFloat16 == ((int32_t)outDataParams->elementType))
  {
    TIDL_HighAccuracySigmoidProcess((bfloat16_tidl *)inPtr,
                                    (bfloat16_tidl *)outPtr,
                                    params,
                                    inDataParams,
                                    outDataParams);
  }
  else  //if(TIDL_SinglePrecFloat == ((int32_t)buffParams->inElementType))
  {
    int32_t i0, i1, i2, i3, i4, i5;
    int32_t imWidth    = (int32_t)inDataParams->dimValues[TIDL_DIM_WIDTH];
    int32_t imHeight   = (int32_t)inDataParams->dimValues[TIDL_DIM_HEIGHT];
    int32_t inDim1     = (int32_t)inDataParams->dimValues[TIDL_DIM_DIM1];
    int32_t inDim2     = (int32_t)inDataParams->dimValues[TIDL_DIM_DIM2];
    int32_t inPitch    = (int32_t)inDataParams->pitch[TIDL_LINE_PITCH];
    int32_t inChPitch  = (int32_t)inDataParams->pitch[TIDL_CHANNEL_PITCH];
    int32_t inDim1Pitch  = (int32_t)inDataParams->pitch[TIDL_DIM_DIM1];
    int32_t inDim2Pitch  = (int32_t)inDataParams->pitch[TIDL_DIM_DIM2];
    int32_t inBatchPitch  = (int32_t)inDataParams->pitch[TIDL_DIM_BATCH];
    int32_t outPitch   = (int32_t)outDataParams->pitch[TIDL_LINE_PITCH];
    int32_t outChPitch = (int32_t)outDataParams->pitch[TIDL_CHANNEL_PITCH];
    int32_t outDim1Pitch  = (int32_t)outDataParams->pitch[TIDL_DIM_DIM1];
    int32_t outDim2Pitch  = (int32_t)outDataParams->pitch[TIDL_DIM_DIM2];
    int32_t outBatchPitch  = (int32_t)outDataParams->pitch[TIDL_DIM_BATCH];
    int32_t numTotBatches = inDataParams->dimValues[TIDL_DIM_BATCH];
    params->numChannels = inDataParams->dimValues[TIDL_DIM_NUMCH];
    float32_tidl *inData = (float32_tidl *)inPtr ;
    float32_tidl *outData = (float32_tidl *)outPtr;
    float32_tidl out = 0;
    float32_tidl Max = 1.0, Min = 0;

    int32_t c = params->numChannels;

    #if 0
    // Call CUDA float sigmoid wrapper
    status = TIDL_cudaFloatSigmoid<float32_tidl, float32_tidl>(inData, outData,
                                   numTotBatches, params->numChannels, imWidth, imHeight, inDim1, inDim2,
                                   inBatchPitch, inChPitch, inDim1Pitch, inDim2Pitch, inPitch,
                                   outBatchPitch, outChPitch, outDim1Pitch, outDim2Pitch, outPitch);
    #else
    /* OPENACC(data copyin(inData[0:1+((numTotBatches-1)*c*inChPitch) + ((c-1)*inChPitch) + ((imHeight-1)*inPitch) + (imWidth-1)]) \
                 copy(outData[0:1+((numTotBatches-1)*c*outChPitch) + ((c-1)*outChPitch) + ((imHeight-1)*outPitch) + (imWidth-1)]))

    OPENACC(parallel loop collapse(4)) */
    for (i5 = 0; i5 < numTotBatches; i5++)
    {
      for(i4 = 0; i4 < inDim1; i4++)
      {
        for(i3 = 0; i3 < inDim2; i3++)
        {
          for (i2 = 0; i2 < c; i2++)
          {
            for (i0=0; i0<imHeight; i0++)
            {
              for (i1=0; i1<imWidth; i1++)
              {
                float32_tidl  inDataVal;
                inDataVal = (inData[(i5*inBatchPitch) + (i4*inDim1Pitch) + (i3*inDim2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1]);
                out = 1.0/(1.0+exp(-inDataVal));
                
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
                Sigmoid output never goes less than 0.
                Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
                This does not affect runtime behavior or safety.
                <justification end> */
                Min = (out<Min) ? out : Min;
                /* LDRA_JUSTIFY_END */
                
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
                Sigmoid output never goes more than 1.
                Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
                This does not affect runtime behavior or safety.
                <justification end> */
                Max = (out>Max) ? out : Max;
                /* LDRA_JUSTIFY_END */
                outData[(i5*outBatchPitch) + (i4*outDim1Pitch) + (i3*outDim2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (float32_tidl)out;
              }
            }
          }
        }
      }
    }
    #endif

    if(((uint32_t)intAlgHandle->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) == TIDL_FLOW_CTRL_REF_STAT)
    {
      //OPENACC(routine(TIDL_getDatElementSign))
      /* LDRA_JUSTIFY_START
      <metric start> branch <metric end>
      <justification start>
      Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
      Float Datatype always returns as signed.
      Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
      This does not affect runtime behavior or safety.
      <justification end> */
      if(TIDL_getDatElementSign(outDataParams->elementType) == 1)
      {
        //OPENACC(routine(TIDL_UpdateScaleFactors))
        TIDL_UpdateScaleFactors(intAlgHandle, layerIdx, 1, Min, Max);
      }
      /* LDRA_JUSTIFY_END */

      /* LDRA_JUSTIFY_START
      <metric start> statement branch <metric end>
      <justification start>
      Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
      Float Datatype always returns as signed.
      Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
      This does not affect runtime behavior or safety.
      <justification end> */
      else
      {
        //OPENACC(routine(TIDL_UpdateScaleFactors))
        TIDL_UpdateScaleFactors(intAlgHandle, layerIdx, 1, 0, Max);
      }
      /* LDRA_JUSTIFY_END */
    }
  }
  return status;
}

/**
 * @brief This is main function perform Non Linear function tramsformation on differnt elementTypes
 *
 * @tparam Tin  : template for input
 * @tparam Tout : template for output
 * @param inPtr : Input pointer on which bacthNorm is applied
 * @param outPtr : Output pointer after batchNorm opreation
 * @param layerIdx :index of the current layer
 * @param params : copy of bacthNorm parameters
 * @param inDataParams : parameters of the input data buffer
 * @param outDataParams : parameters of the output data buffer
 * @param tidlLayer : Pointer to the common layer parameters
 * @param LUT_data_table : pointer to 256-element table for look up
 * @param readOffsetLUT : initial read offset for indexing LUT directly using input data
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
template <class Tin, class Tout>
static int32_t TIDL_refNonLinearLUTCore(Tin  *inPtr,
                                   Tout *outPtr,
                                   int32_t layerIdx,
                                   sTIDL_BatchNormParams_t *params,
                                   sTIDL_DataParams_t *inDataParams,
                                   sTIDL_DataParams_t *outDataParams,
                                   const sTIDL_Layer_t    *tidlLayer,
                                   Tout *LUT_data_table
                                   )
{
  int32_t status = TIDL_SUCCESS;
  int32_t i0, i1, i2, i3, i4, i5;
  int32_t imWidth    = (int32_t)inDataParams->dimValues[TIDL_DIM_WIDTH];
  int32_t imHeight   = (int32_t)inDataParams->dimValues[TIDL_DIM_HEIGHT];
  int32_t inDIM2     = (int32_t)inDataParams->dimValues[TIDL_DIM_DIM2];
  int32_t inDIM1     = (int32_t)inDataParams->dimValues[TIDL_DIM_DIM1];
  int32_t inPitch    = (int32_t)inDataParams->pitch[TIDL_LINE_PITCH];
  int32_t inChPitch  = (int32_t)inDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t inBatchPitch  = (int32_t)inDataParams->pitch[TIDL_ROI_PITCH];
  int32_t inDIM2Pitch  = (int32_t)inDataParams->pitch[TIDL_DIM_DIM2];
  int32_t inDIM1Pitch  = (int32_t)inDataParams->pitch[TIDL_DIM_DIM1];
  int32_t outPitch   = (int32_t)outDataParams->pitch[TIDL_LINE_PITCH];
  int32_t outChPitch = (int32_t)outDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t outBatchPitch = (int32_t)outDataParams->pitch[TIDL_ROI_PITCH];
  int32_t numTotBatches = inDataParams->dimValues[TIDL_DIM_BATCH];
  params->numChannels = inDataParams->dimValues[TIDL_DIM_NUMCH];
  int32_t outDIM2Pitch  = (int32_t)outDataParams->pitch[TIDL_DIM_DIM2];
  int32_t outDIM1Pitch  = (int32_t)outDataParams->pitch[TIDL_DIM_DIM1];
  /*workload already adds offset so not to add here
  */
  Tin  *inData = (Tin *)inPtr;
  Tout *outData = (Tout *)outPtr;

  int32_t readOffsetLUT = 0;
  int32_t imin = 0;
  imin = std::numeric_limits<Tin>::lowest();

  if(imin < 0)
  {
    readOffsetLUT = -1 * imin;
  }

  int32_t c = params->numChannels;
  #ifdef BUILD_WITH_OPENACC_DISABLED
    int32_t LUT_size;
    if(TIDL_UnsignedChar == ((int32_t)outDataParams->elementType))
    {
      LUT_size = TABLE_SIZE_8BIT;
    }
    else if(TIDL_SignedChar == ((int32_t)outDataParams->elementType))
    {
      LUT_size = TABLE_SIZE_8BIT;
    }
    else if(TIDL_UnsignedShort == ((int32_t)outDataParams->elementType))
    {
      LUT_size = TABLE_SIZE_16BIT;
    }
    else if(TIDL_SignedShort == ((int32_t)outDataParams->elementType))
    {
      LUT_size = TABLE_SIZE_16BIT;
    }
    else
    {
       /* Bit size not matching */
    }
  #endif
  /*OPENACC(data copyin(inData[:1+ ((numTotBatches-1)*inBatchPitch) + ((inDIM1-1)*inDIM1Pitch) + ((inDIM2-1)*inDIM2Pitch) + ((c-1)*inChPitch) + ((imHeight-1)*inPitch) + (imWidth-1)], \
                      LUT_data_table[:LUT_size]) \
               copy(outData[:1+ ((numTotBatches-1)*outBatchPitch) + ((inDIM1-1)*outDIM1Pitch) + ((inDIM2-1)*outDIM2Pitch) + ((c-1)*outChPitch) + ((imHeight-1)*outPitch) + (imWidth-1)]))*/
  {
  //OPENACC(parallel loop collapse(6))
  #if 0
  // call CUDA non linear LUT kernel
  status = TIDL_cudaNonLinearLUT<Tin, Tout>(inData, outData, LUT_data_table,
                                            numTotBatches, c, imWidth, imHeight, inDIM1, inDIM2,
                                            inBatchPitch, inChPitch, inDIM1Pitch, inDIM2Pitch, inPitch,
                                            outBatchPitch, outChPitch, outDIM1Pitch, outDIM2Pitch, outPitch,
                                            readOffsetLUT);
  #else
  for (i5 = 0; i5 < numTotBatches; i5++)
  {
    for(i4 = 0; i4 < inDIM1; i4++)
    {
      for(i3 = 0; i3 < inDIM2; i3++)
      {
        for (i2 = 0; i2 < c; i2++)
        {
          for (i0 = 0; i0 < imHeight; i0++)
          {
            for (i1 = 0; i1 < imWidth; i1++)
            {
              Tin  inDataVal;
              inDataVal = (inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1]);
              outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = LUT_data_table[inDataVal+readOffsetLUT];
            }
          }
        }
      }
    }

  }
  #endif
  }
  return status;
}

#if defined (__C7100__) || defined (__C7120__)
template <class Tin, class Tout>
static Tout TIDL_functionWithLookup(int32_t f1, int32_t f2, Tin quant_input, Tout* new_quant_lut, int32_t inoffset, int32_t isOutSigned)
{
  uint16_t quant_int = quant_input + inoffset;
  uint8_t quant_int_shifted = quant_int >> 8;
  uint8_t quant_int_frac = quant_int & 0xFF;

  int16_t b = (int16_t)new_quant_lut[quant_int_shifted * 2];
  Tout c = new_quant_lut[(quant_int_shifted * 2) + 1];
  uint16_t x = quant_int_frac;

  int32_t b_x = b * x;
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  if (f1 > 0) {
      b_x = b_x << ((f1 <= 8) ? f1 : 8);
  }
  else 
  /* LDRA_JUSTIFY_END */
  {
      /* LDRA_JUSTIFY_START
      <metric start> statement branch <metric end>
      <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
      <justification end> */
      b_x = b_x >> ((f1 >= -24) ? (-f1) : 24);
      /* LDRA_JUSTIFY_END */ 
  }

  int32_t interp_val = ( b_x + c); 
  
  if (f2 > 0) {
      /* LDRA_JUSTIFY_START
      <metric start> statement branch <metric end>
      <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
      <justification end> */
      interp_val = interp_val << ((f2 <= 15) ? f2 : 15);
      /* LDRA_JUSTIFY_END */
  } 
  else {
      /* LDRA_JUSTIFY_START
      <metric start> statement branch <metric end>
      <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
      <justification end> */
      interp_val = interp_val >> ((f2 > -17) ? (-f2) : 17);
      /* LDRA_JUSTIFY_END */
  }

  if (isOutSigned == 1)
  {
    interp_val = (interp_val >= -32768) ? interp_val : -32768;
    interp_val = (interp_val <= 32767) ? interp_val : 32767;
  }
  else
  {
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    <justification end> */
    interp_val = (interp_val >= 0) ? interp_val : 0;
    interp_val = (interp_val <= 65535) ? interp_val : 65535;
    /* LDRA_JUSTIFY_END */
  }
   return (Tout)interp_val;
}

/**
 * @brief This is main function perform Non Linear Interpol LUT function tramsformation on differnt elementTypes
 *
 * @tparam Tin  : template for input
 * @tparam Tout : template for output
 * @param inPtr : Input pointer on which bacthNorm is applied
 * @param outPtr : Output pointer after batchNorm opreation
 * @param layerIdx :index of the current layer
 * @param params : copy of bacthNorm parameters
 * @param inDataParams : parameters of the input data buffer
 * @param outDataParams : parameters of the output data buffer
 * @param tidlLayer : Pointer to the common layer parameters
 * @param LUT_data_table : pointer to 256-element table for look up
 * @param readOffsetLUT : initial read offset for indexing LUT directly using input data
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
template <class Tin, class Tout>
static int32_t TIDL_refNonLinearInterpolLUTCore(Tin  *inPtr,
                                   Tout *outPtr,
                                   int32_t layerIdx,
                                   sTIDL_BatchNormParams_t *params,
                                   sTIDL_DataParams_t *inDataParams,
                                   sTIDL_DataParams_t *outDataParams,
                                   const sTIDL_Layer_t    *tidlLayer,
                                   Tout *LUT_data_table,
                                   int32_t factorB,
                                   int32_t factorC
                                   )
{
  int32_t status = TIDL_SUCCESS;
  int32_t i0, i1, i2, i3, i4, i5;
  int32_t imWidth    = (int32_t)inDataParams->dimValues[TIDL_DIM_WIDTH];
  int32_t imHeight   = (int32_t)inDataParams->dimValues[TIDL_DIM_HEIGHT];
  int32_t inDIM2     = (int32_t)inDataParams->dimValues[TIDL_DIM_DIM2];
  int32_t inDIM1     = (int32_t)inDataParams->dimValues[TIDL_DIM_DIM1];
  int32_t inPitch    = (int32_t)inDataParams->pitch[TIDL_LINE_PITCH];
  int32_t inChPitch  = (int32_t)inDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t inBatchPitch  = (int32_t)inDataParams->pitch[TIDL_ROI_PITCH];
  int32_t inDIM2Pitch  = (int32_t)inDataParams->pitch[TIDL_DIM_DIM2];
  int32_t inDIM1Pitch  = (int32_t)inDataParams->pitch[TIDL_DIM_DIM1];
  int32_t outPitch   = (int32_t)outDataParams->pitch[TIDL_LINE_PITCH];
  int32_t outChPitch = (int32_t)outDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t outBatchPitch = (int32_t)outDataParams->pitch[TIDL_ROI_PITCH];
  int32_t numTotBatches = inDataParams->dimValues[TIDL_DIM_BATCH];
  params->numChannels = inDataParams->dimValues[TIDL_DIM_NUMCH];
  int32_t outDIM2Pitch  = (int32_t)outDataParams->pitch[TIDL_DIM_DIM2];
  int32_t outDIM1Pitch  = (int32_t)outDataParams->pitch[TIDL_DIM_DIM1];
  
  int32_t readOffsetLUT, isOutputSigned;

  readOffsetLUT = 0;
  isOutputSigned = 0;
  int32_t imin = 0;
  imin = std::numeric_limits<Tin>::lowest();
  int32_t omin = 0;
  omin = std::numeric_limits<Tout>::lowest();

  if(imin < 0)
  {
    readOffsetLUT = -1 * imin;
  }
  if(omin<0)
  {
    isOutputSigned = 1;
  }

  /*workload already adds offset so not to add here
  */
  Tin  *inData = (Tin *)inPtr;
  Tout *outData = (Tout *)outPtr;

  int32_t c = params->numChannels;

  #if 0
  // call CUDA non linear interpolation LUT kernel
  status = TIDL_cudaNonLinearInterpolLUT<Tin, Tout>(inData, outData, LUT_data_table,
                                                    numTotBatches, c, imWidth, imHeight, inDIM1, inDIM2,
                                                    inBatchPitch, inChPitch, inDIM1Pitch, inDIM2Pitch, inPitch,
                                                    outBatchPitch, outChPitch, outDIM1Pitch, outDIM2Pitch, outPitch,
                                                    factorB, factorC, readOffsetLUT);
  #else
  for (i5 = 0; i5 < numTotBatches; i5++)
  {
    for(i4 = 0; i4 < inDIM1; i4++)
    {
      for(i3 = 0; i3 < inDIM2; i3++)
      {
        for (i2 = 0; i2 < c; i2++)
        {
          for (i0 = 0; i0 < imHeight; i0++)
          {
            for (i1 = 0; i1 < imWidth; i1++)
            {
              Tin  inDataVal;
              inDataVal = (inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1]);
              outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = TIDL_functionWithLookup(factorB, factorC, (Tin)inDataVal, (Tout*)LUT_data_table, readOffsetLUT, isOutputSigned);
            }
          }
        }
      }
    }

  }
  #endif
  
  return status;
}
#endif

template <class Tin, class Tout>
static int32_t TIDL_refNonLinearNonLUTCore(TIDL_Handle intAlgHandle,
                                                Tin  *inPtr,
                                                Tout *outPtr,
                                                int32_t layerIdx,
                                                sTIDL_BatchNormParams_t *params,
                                                sTIDL_DataParams_t *inDataParams,
                                                sTIDL_DataParams_t *outDataParams,
                                                const sTIDL_Layer_t    *tidlLayer)
{
  int32_t status = TIDL_SUCCESS;
  int32_t i0, i1, i2, i3, i4, i5;
  float32_tidl alpha = -1.0, beta = -1.0;
  int32_t imWidth       = (int32_t)inDataParams->dimValues[TIDL_DIM_WIDTH];
  int32_t imHeight      = (int32_t)inDataParams->dimValues[TIDL_DIM_HEIGHT];
  int32_t inDIM2        = (int32_t)inDataParams->dimValues[TIDL_DIM_DIM2];
  int32_t inDIM1        = (int32_t)inDataParams->dimValues[TIDL_DIM_DIM1];
  int32_t inPitch       = (int32_t)inDataParams->pitch[TIDL_LINE_PITCH];
  int32_t inChPitch     = (int32_t)inDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t inDIM2Pitch   = (int32_t)inDataParams->pitch[TIDL_DIM_DIM2];
  int32_t inDIM1Pitch   = (int32_t)inDataParams->pitch[TIDL_DIM_DIM1];
  int32_t inBatchPitch  = (int32_t)inDataParams->pitch[TIDL_ROI_PITCH];
  int32_t outPitch      = (int32_t)outDataParams->pitch[TIDL_LINE_PITCH];
  int32_t outChPitch    = (int32_t)outDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t outBatchPitch = (int32_t)outDataParams->pitch[TIDL_ROI_PITCH];
  int32_t outDIM2Pitch  = (int32_t)outDataParams->pitch[TIDL_DIM_DIM2];
  int32_t outDIM1Pitch  = (int32_t)outDataParams->pitch[TIDL_DIM_DIM1];
  int32_t numTotBatches = inDataParams->dimValues[TIDL_DIM_BATCH];
  params->numChannels   = inDataParams->dimValues[TIDL_DIM_NUMCH];
  int32_t Zx            = inDataParams->tensorZeroPoint;
  float32_tidl Sx       = inDataParams->tensorScale;
  int32_t Zy            = outDataParams->tensorZeroPoint;
  float32_tidl Sy       = outDataParams->tensorScale;

  /*workload already adds offset so not to add here
  */
  Tin  *inData = (Tin *)inPtr;
  Tout *outData = (Tout *)outPtr;

  if(tidlLayer->actParams.actType == TIDL_ELU)
  {
    alpha = (float)tidlLayer->layerParams.batchNormParams.inDataQ/((float)TIDL_NON_LIN_PARAM_SCALE);
  }
  if(tidlLayer->actParams.actType == TIDL_HardSigmoid)
  {
    alpha = (float)tidlLayer->layerParams.batchNormParams.inDataQ/((float)TIDL_NON_LIN_PARAM_SCALE);
    beta = (float)tidlLayer->layerParams.batchNormParams.weightsQ/((float)TIDL_NON_LIN_PARAM_SCALE);
  }

  if(tidlLayer->actParams.actType == TIDL_HardSwish)
  {
    alpha = 0.166666666f;
    beta = 0.5f;
  }

  if(tidlLayer->actParams.actType == TIDL_Swish)
  {
    alpha = (float)tidlLayer->actParams.alpha;
    beta = 0.0f;
  }
  //As of now it is dead code since there is no NON LUT ci code
  #ifdef TIDL_COVERAGE_DEAD_CODE   
  if(tidlLayer->actParams.actType == TIDL_Pow)
  {
    alpha = (float)tidlLayer->layerParams.batchNormParams.inDataQ/((float)TIDL_NON_LIN_PARAM_SCALE);
  }
  #endif

  int32_t numchan = params->numChannels;
  /*OPENACC(data copyin(inData[:1 + (numTotBatches-1)*inBatchPitch + (inDIM1-1)*inDIM1Pitch + (inDIM2-1)*inDIM2Pitch + (numchan-1)*inChPitch + (imHeight-1)*inPitch + (imWidth-1)]) \
               copy(outData[:1 + (numTotBatches-1)*outBatchPitch + (inDIM1-1)*outDIM1Pitch + (inDIM2-1)*outDIM2Pitch + (numchan-1)*outChPitch + (imHeight-1)*outPitch + (imWidth-1)]))*/
  {
  //OPENACC(parallel loop collapse(6))
  for(i5 = 0; i5 < numTotBatches; i5++)
  {
    for(i4 = 0; i4 < inDIM1; i4++)
    {
      for(i3 = 0; i3 < inDIM2; i3++)
      {
        for(i2 = 0; i2 < numchan; i2++)
        {
          for (i0=0; i0<imHeight; i0++)
          {
            for (i1=0; i1<imWidth; i1++)
            {
              Tin  inDataVal;
              Tout outDataVal;
              inDataVal = (inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1]);
              
              if(tidlLayer->actParams.actType == TIDL_Tanh)
              {
                // OPENACC(routine(TIDL_tanh_nonLut))
                status = TIDL_tanh_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }
              else if(tidlLayer->actParams.actType == TIDL_HardSigmoid)
              {
                // OPENACC(routine(TIDL_hardSigmoid_nonLut))
                status = TIDL_hardSigmoid_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }
              else if(tidlLayer->actParams.actType == TIDL_ELU)
              {
                // OPENACC(routine(TIDL_elu_nonLut))
                status = TIDL_elu_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }
              else if(tidlLayer->actParams.actType == TIDL_GELU)
              {
                if(intAlgHandle->createParams->net->netVersion >= TIDL_NET_VERSION_FW_11_00_00_00)
                {
                  // OPENACC(routine(TIDL_gelu_fused_nonLut))
                  status = TIDL_gelu_fused_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
                }
                else
                {
                  #if TIDL_NET_VERSION_FW_PASSIVE_CODE
                  // OPENACC(routine(TIDL_gelu_nonLut))
                  status = TIDL_gelu_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
                  #else
                  status = TIDL_ERR_FAILURE;
                  break;
                  #endif
                }
              }
              #ifdef TIDL_COVERAGE_DEAD_CODE  
              else if(tidlLayer->actParams.actType == TIDL_Acos)
              {
                // OPENACC(routine(TIDL_acos_nonLut))
                status = TIDL_acos_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }
              else if(tidlLayer->actParams.actType == TIDL_Tan)
              {
                // OPENACC(routine(TIDL_tan_nonLut))
                status = TIDL_tan_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }
              else if(tidlLayer->actParams.actType == TIDL_Asin)
              {
                // OPENACC(routine(TIDL_asin_nonLut))
                status = TIDL_asin_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }
              //As of now it is dead code since there is no NON LUT ci code
              else if(tidlLayer->actParams.actType == TIDL_Mish)
              {
                // OPENACC(routine(TIDL_mish_nonLut))
                status = TIDL_mish_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }
              else if(tidlLayer->actParams.actType == TIDL_Log)
              {
                // OPENACC(routine(TIDL_log_nonLut))
                status = TIDL_log_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }
              //As of now it is dead code since there is no NON LUT ci code
              else if(tidlLayer->actParams.actType == TIDL_Asinh)
              {
                // OPENACC(routine(TIDL_asinh_nonLut))
                status = TIDL_asinh_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }
              else if(tidlLayer->actParams.actType == TIDL_Exp)
              {
                // OPENACC(routine(TIDL_exp_nonLut))
                status = TIDL_exp_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }
              //As of now it is dead code since there is no NON LUT ci code
              else if(tidlLayer->actParams.actType == TIDL_Logit)
              {
                // OPENACC(routine(TIDL_logit_nonLut))
                status = TIDL_logit_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }
              //As of now it is dead code since there is no NON LUT ci code
              else if(tidlLayer->actParams.actType == TIDL_Reciprocal)
              {
                // OPENACC(routine(TIDL_reciprocal_nonLut))
                status = TIDL_reciprocal_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }
              //As of now it is dead code since there is no NON LUT ci code
              else if(tidlLayer->actParams.actType == TIDL_Pow)
              {
                // OPENACC(routine(TIDL_pow_nonLut))
                status = TIDL_pow_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }
              else if(tidlLayer->actParams.actType == TIDL_SiLU)
              {
                status = TIDL_silu_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }
              #endif
              else if(tidlLayer->actParams.actType == TIDL_Atan)
              {
                // OPENACC(routine(TIDL_atan_nonLut))
                status = TIDL_atan_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }
              else if(tidlLayer->actParams.actType == TIDL_Sinh)
              {
                // OPENACC(routine(TIDL_sinh_nonLut))
                status = TIDL_sinh_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }
              else if(tidlLayer->actParams.actType == TIDL_Cos)
              {
                // OPENACC(routine(TIDL_cos_nonLut))
                status = TIDL_cos_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }
              else if(tidlLayer->actParams.actType == TIDL_Cosh)
              {
                // OPENACC(routine(TIDL_cosh_nonLut))
                status = TIDL_cosh_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }
              else if(tidlLayer->actParams.actType == TIDL_Neg)
              {
                // OPENACC(routine(TIDL_neg_nonLut))
                status = TIDL_neg_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }
              else if(tidlLayer->actParams.actType == TIDL_HardSwish)
              {
                // OPENACC(routine(TIDL_hardswish_nonLut))
                status = TIDL_hardswish_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }
              else if(tidlLayer->actParams.actType == TIDL_Swish)
              {
                status = TIDL_swish_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }
              else if(tidlLayer->actParams.actType == TIDL_Abs)
              {
                // OPENACC(routine(TIDL_abs_nonLut))
                status = TIDL_abs_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }
              else if(tidlLayer->actParams.actType == TIDL_Floor)
              {
                // OPENACC(routine(TIDL_floor_nonLut))
                status = TIDL_floor_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }
              else if(tidlLayer->actParams.actType == TIDL_Sin)
              {
                // OPENACC(routine(TIDL_sin_nonLut))
                status = TIDL_sin_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }
              else if(tidlLayer->actParams.actType == TIDL_Erf)
              {
                // OPENACC(routine(TIDL_erf_nonLut))
                status = TIDL_erf_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }
              else if(tidlLayer->actParams.actType == TIDL_Sqrt)
              {
                // OPENACC(routine(TIDL_sqrt_nonLut))
                status = TIDL_sqrt_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }
              else if(tidlLayer->actParams.actType == TIDL_SoftPlus)
              {
                // OPENACC(routine(TIDL_softplus_nonLut))
                status = TIDL_softplus_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }
              else if(tidlLayer->actParams.actType == TIDL_SoftSign)
              {
                // OPENACC(routine(TIDL_softsign_nonLut))
                status = TIDL_softsign_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }
              else if(tidlLayer->actParams.actType == TIDL_Ceil)
              {
                // OPENACC(routine(TIDL_ceil_nonLut))
                status = TIDL_ceil_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }
              else if(tidlLayer->actParams.actType == TIDL_Celu)
              {
                // OPENACC(routine(TIDL_celu_nonLut))
                status = TIDL_celu_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }
              else if(tidlLayer->actParams.actType == TIDL_Selu)
              {
                // OPENACC(routine(TIDL_selu_nonLut))
                status = TIDL_selu_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }
              else if(tidlLayer->actParams.actType == TIDL_Round)
              {
                // OPENACC(routine(TIDL_round_nonLut))
                status = TIDL_round_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }
              /* LDRA_JUSTIFY_START
              <metric start> branch <metric end>
              <justification start>
              Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
              Prior check occurs for other activation types.
              Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
              This does not affect runtime behavior or safety.
              <justification end> */
              else if(tidlLayer->actParams.actType == TIDL_Sign)
              /* LDRA_JUSTIFY_END */
              {
                // OPENACC(routine(TIDL_round_nonLut))
                status = TIDL_sign_nonLut<Tin, Tout>((const void *)&inDataVal,(void *)&outDataVal, Zx, Sx, Zy, Sy, alpha, beta);
              }     
              /* LDRA_JUSTIFY_START
              <metric start> statement branch <metric end>
              <justification start>
              Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
              Prior check occurs for other activation types.
              Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
              This does not affect runtime behavior or safety.
              <justification end> */
              else
              {
                status = TIDL_ERR_NOT_IMPLEMENTED;
                break;
              }
              /* LDRA_JUSTIFY_END */
              outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = outDataVal;
            }
          }
        }
      }
    }

  }
  }
  return status;
}

/**
 * @brief This function is Non Linear function LUT-based reference implementation
 *
 * @param intAlgHandle : tidl algorothm handle
 * @param algLayer : Pointer to the layer specific parameters
 * @param tidlLayer : Pointer to the common layer parameters
 * @param params : copy of batch norm layer parameters
 * @param inPtr : Pointer to input buffers to be processed
 * @param outPtr : Pointer to output buffers to be processed
 * @param inDataParams : pointer to input data parameters
 * @param outDataParams : pointer to output data parameters
 * @param LUT_data_table : pointer to 256-element table for look up
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
static int32_t TIDL_refNonLinearLUTProcess(TIDL_Handle intAlgHandle,
                                      sTIDL_AlgLayer_t *algLayer,
                                      const sTIDL_Layer_t    *tidlLayer,
                                      sTIDL_BatchNormParams_t *params,
                                      void *inPtr,
                                      void *outPtr,
                                      sTIDL_DataParams_t *inDataParams,
                                      sTIDL_DataParams_t *outDataParams,
                                      void *LUT_data_table)
{
  int32_t status = TIDL_SUCCESS;
  int32_t layerIdx = algLayer->layerIdx;

  if(TIDL_UnsignedChar == ((int32_t)outDataParams->elementType))
  {
    if(TIDL_SignedChar == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refNonLinearLUTCore((int8_t*)inPtr,
                                  (uint8_t*)outPtr,
                                   layerIdx,
                                   params,
                                   inDataParams,
                                   outDataParams,
                                   tidlLayer,
                                   (uint8_t*)LUT_data_table
                                   );
    }
    else if(TIDL_UnsignedChar == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refNonLinearLUTCore((uint8_t*)inPtr,
                                  (uint8_t*)outPtr,
                                   layerIdx,
                                   params,
                                   inDataParams,
                                   outDataParams,
                                   tidlLayer,
                                   (uint8_t*)LUT_data_table
                                   );
    }
    else 
    {
      status = TIDL_ERR_NOT_IMPLEMENTED;
    }
  }
  else if(TIDL_SignedChar == ((int32_t)outDataParams->elementType))
  {
    if(TIDL_SignedChar == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refNonLinearLUTCore((int8_t*)inPtr,
                                  (int8_t*)outPtr,
                                   layerIdx,
                                   params,
                                   inDataParams,
                                   outDataParams,
                                   tidlLayer,
                                   (int8_t*)LUT_data_table
                                   );
    }
    else if(TIDL_UnsignedChar == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refNonLinearLUTCore((uint8_t*)inPtr,
                                  (int8_t*)outPtr,
                                   layerIdx,
                                   params,
                                   inDataParams,
                                   outDataParams,
                                   tidlLayer,
                                   (int8_t*)LUT_data_table
                                   );
    }
	  else
	  {
	  	status = TIDL_ERR_NOT_IMPLEMENTED;
	  }
  }

  else if(TIDL_UnsignedShort == ((int32_t)outDataParams->elementType))
  {
    if(TIDL_SignedShort == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refNonLinearLUTCore((int16_t*)inPtr,
                                  (uint16_t*)outPtr,
                                   layerIdx,
                                   params,
                                   inDataParams,
                                   outDataParams,
                                   tidlLayer,
                                   (uint16_t*)LUT_data_table
                                   );
    }
    else if(TIDL_UnsignedShort == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refNonLinearLUTCore((uint16_t*)inPtr,
                                  (uint16_t*)outPtr,
                                   layerIdx,
                                   params,
                                   inDataParams,
                                   outDataParams,
                                   tidlLayer,
                                   (uint16_t*)LUT_data_table
                                   );
    }
    
    else
    {
      status = TIDL_ERR_NOT_IMPLEMENTED;
    }
  }
  else if(TIDL_SignedShort == ((int32_t)outDataParams->elementType))
  {
    if(TIDL_SignedShort == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refNonLinearLUTCore((int16_t*)inPtr,
                                  (int16_t*)outPtr,
                                   layerIdx,
                                   params,
                                   inDataParams,
                                   outDataParams,
                                   tidlLayer,
                                   (int16_t*)LUT_data_table
                                   );
    }
    else if(TIDL_UnsignedShort == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refNonLinearLUTCore((uint16_t*)inPtr,
                                  (int16_t*)outPtr,
                                   layerIdx,
                                   params,
                                   inDataParams,
                                   outDataParams,
                                   tidlLayer,
                                   (int16_t*)LUT_data_table
                                   );
    }
	  else
	  {
	  	status = TIDL_ERR_NOT_IMPLEMENTED; 
	  }
  }
  else
  {
	  status = TIDL_ERR_NOT_IMPLEMENTED;
  }
  return status;
}

#if defined (__C7100__) || defined (__C7120__)
/**
 * @brief This function is Non Linear function Interpol LUT-based reference implementation
 *
 * @param intAlgHandle : tidl algorothm handle
 * @param algLayer : Pointer to the layer specific parameters
 * @param tidlLayer : Pointer to the common layer parameters
 * @param params : copy of batch norm layer parameters
 * @param inPtr : Pointer to input buffers to be processed
 * @param outPtr : Pointer to output buffers to be processed
 * @param inDataParams : pointer to input data parameters
 * @param outDataParams : pointer to output data parameters
 * @param LUT_data_table : pointer to 256-element table for look up
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
static int32_t TIDL_refNonLinearInterpolLUTProcess(TIDL_Handle intAlgHandle,
                                      sTIDL_AlgLayer_t *algLayer,
                                      const sTIDL_Layer_t    *tidlLayer,
                                      sTIDL_BatchNormParams_t *params,
                                      void *inPtr,
                                      void *outPtr,
                                      sTIDL_DataParams_t *inDataParams,
                                      sTIDL_DataParams_t *outDataParams,
                                      void *LUT_data_table)
{
  int32_t status = TIDL_SUCCESS;
  int32_t layerIdx = algLayer->layerIdx;
  int32_t factorB = algLayer->layerParams.batchNormParams.lutParams.factorB;
  int32_t factorC = algLayer->layerParams.batchNormParams.lutParams.factorC;

  if(TIDL_UnsignedShort == ((int32_t)outDataParams->elementType))
  {
    if(TIDL_SignedShort == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refNonLinearInterpolLUTCore((int16_t*)inPtr,
                                  (uint16_t*)outPtr,
                                   layerIdx,
                                   params,
                                   inDataParams,
                                   outDataParams,
                                   tidlLayer,
                                   (uint16_t*)LUT_data_table,
                                   factorB,
                                   factorC
                                   );
    }
    else if(TIDL_UnsignedShort == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refNonLinearInterpolLUTCore((uint16_t*)inPtr,
                                  (uint16_t*)outPtr,
                                   layerIdx,
                                   params,
                                   inDataParams,
                                   outDataParams,
                                   tidlLayer,
                                   (uint16_t*)LUT_data_table,
                                   factorB,
                                   factorC
                                   );
    }
    else
    {
      status = TIDL_ERR_NOT_IMPLEMENTED;
    }

  }
  else if(TIDL_SignedShort == ((int32_t)outDataParams->elementType))
  {
    if(TIDL_SignedShort == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refNonLinearInterpolLUTCore((int16_t*)inPtr,
                                  (int16_t*)outPtr,
                                   layerIdx,
                                   params,
                                   inDataParams,
                                   outDataParams,
                                   tidlLayer,
                                   (int16_t*)LUT_data_table,
                                   factorB,
                                   factorC
                                   );
    }
    else if(TIDL_UnsignedShort == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refNonLinearInterpolLUTCore((uint16_t*)inPtr,
                                  (int16_t*)outPtr,
                                   layerIdx,
                                   params,
                                   inDataParams,
                                   outDataParams,
                                   tidlLayer,
                                   (int16_t*)LUT_data_table,
                                   factorB,
                                   factorC
                                   );
    }
	  else
	  {
	  	status = TIDL_ERR_NOT_IMPLEMENTED;
	  }
  }
  else
  {
	  status = TIDL_ERR_NOT_IMPLEMENTED;
  }
  return status;
}
#endif
/**
 * @brief This function is Non Linear function Non-LUT based reference implementation
 *
 * @param intAlgHandle : tidl algorothm handle
 * @param algLayer : Pointer to the layer specific parameters
 * @param tidlLayer : Pointer to the common layer parameters
 * @param params : copy of batch norm layer parameters
 * @param inPtr : Pointer to input buffers to be processed
 * @param outPtr : Pointer to output buffers to be processed
 * @param inDataParams : pointer to input data parameters
 * @param outDataParams : pointer to output data parameters
 * @param LUT_data_table : pointer to 256-element table for look up
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
template<class Tin, class Tout>
static int32_t TIDL_refNonLinearNonLUTProcess(TIDL_Handle intAlgHandle,
                                      sTIDL_AlgLayer_t *algLayer,
                                      const sTIDL_Layer_t    *tidlLayer,
                                      sTIDL_BatchNormParams_t *params,
                                      void *inPtr,
                                      void *outPtr,
                                      sTIDL_DataParams_t *inDataParams,
                                      sTIDL_DataParams_t *outDataParams)
{
  int32_t status = TIDL_SUCCESS;
  int32_t layerIdx = algLayer->layerIdx;

  status = TIDL_refNonLinearNonLUTCore<Tin, Tout>(intAlgHandle, 
                              (Tin*)inPtr,
                              (Tout*)outPtr,
                               layerIdx,
                               params,
                               inDataParams,
                               outDataParams,
                               tidlLayer);

  return status;
}

static int32_t TIDL_refNonLinearNonLUTProcessWrapper(TIDL_Handle intAlgHandle,
                                      sTIDL_AlgLayer_t *algLayer,
                                      const sTIDL_Layer_t    *tidlLayer,
                                      sTIDL_BatchNormParams_t *params,
                                      void *inPtr,
                                      void *outPtr,
                                      sTIDL_DataParams_t *inDataParams,
                                      sTIDL_DataParams_t *outDataParams)
{
  int32_t status = TIDL_SUCCESS;
  
  if(TIDL_SignedShort == ((int32_t)outDataParams->elementType))
  {
    if(TIDL_SignedShort == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refNonLinearNonLUTProcess<int16_t, int16_t>(intAlgHandle,
                                                                  algLayer,
                                                                  tidlLayer,
                                                                  params,
                                                                  inPtr,
                                                                  outPtr,
                                                                  inDataParams,
                                                                  outDataParams);
    }
    
    else if(TIDL_UnsignedShort == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refNonLinearNonLUTProcess<uint16_t, int16_t>(intAlgHandle,
                                                                  algLayer,
                                                                  tidlLayer,
                                                                  params,
                                                                  inPtr,
                                                                  outPtr,
                                                                  inDataParams,
                                                                  outDataParams);
    }
    
    else
    {
      status = TIDL_ERR_NOT_IMPLEMENTED;
    }
    
  }
  

  else if(TIDL_SignedChar == ((int32_t)outDataParams->elementType))
  {
    if(TIDL_SignedChar == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refNonLinearNonLUTProcess<int8_t, int8_t>(intAlgHandle,
                                                                  algLayer,
                                                                  tidlLayer,
                                                                  params,
                                                                  inPtr,
                                                                  outPtr,
                                                                  inDataParams,
                                                                  outDataParams);
    }
    else if(TIDL_UnsignedChar == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refNonLinearNonLUTProcess<uint8_t, int8_t>(intAlgHandle,
                                                                  algLayer,
                                                                  tidlLayer,
                                                                  params,
                                                                  inPtr,
                                                                  outPtr,
                                                                  inDataParams,
                                                                  outDataParams);
    }
    else
    {
      status = TIDL_ERR_NOT_IMPLEMENTED;
    }
  }

  else if(TIDL_UnsignedShort == ((int32_t)outDataParams->elementType))
  {
    if(TIDL_SignedShort == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refNonLinearNonLUTProcess<int16_t, uint16_t>(intAlgHandle,
                                                                  algLayer,
                                                                  tidlLayer,
                                                                  params,
                                                                  inPtr,
                                                                  outPtr,
                                                                  inDataParams,
                                                                  outDataParams);
    }
    else if(TIDL_UnsignedShort == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refNonLinearNonLUTProcess<uint16_t, uint16_t>(intAlgHandle,
                                                                  algLayer,
                                                                  tidlLayer,
                                                                  params,
                                                                  inPtr,
                                                                  outPtr,
                                                                  inDataParams,
                                                                  outDataParams);
    }
    else
    {
      /*do nothing*/
    }
  }

  

  else if(TIDL_UnsignedChar == ((int32_t)outDataParams->elementType))
  {
    if(TIDL_SignedChar == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refNonLinearNonLUTProcess<int8_t, uint8_t>(intAlgHandle,
                                                                  algLayer,
                                                                  tidlLayer,
                                                                  params,
                                                                  inPtr,
                                                                  outPtr,
                                                                  inDataParams,
                                                                  outDataParams);
    }
    else if(TIDL_UnsignedChar == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refNonLinearNonLUTProcess<uint8_t, uint8_t>(intAlgHandle,
                                                                  algLayer,
                                                                  tidlLayer,
                                                                  params,
                                                                  inPtr,
                                                                  outPtr,
                                                                  inDataParams,
                                                                  outDataParams);
    }
    else
    {
      /*do nothing*/
    }
  }

  else
  {
    status = TIDL_ERR_NOT_IMPLEMENTED;
  }

  
  return status;
}

/**
 * @brief This is main function perform Tanh operation if inp-output is in float-float manner
 * @tparam Tin  : template for input
 * @tparam Tout : template for output
 * @param inPtr : Input pointer on which bacthNorm is applied
 * @param outPtr : Output pointer after batchNorm opreation
 * @param layerIdx :index of the current layer
 * @param params : copy of bacthNorm parameters
 * @param inDataParams : parameters of the input data buffer
 * @param outDataParams : parameters of the output data buffer
 * @param tidlLayer : Pointer to the common layer parameters
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
template <class Tin, class Tout>
static int32_t TIDL_refFloatNonLinear(TIDL_Handle intAlgHandle,
                                   Tin  *inPtr,
                                   Tout *outPtr,
                                   int32_t layerIdx,
                                   sTIDL_BatchNormParams_t *params,
                                   sTIDL_DataParams_t *inDataParams,
                                   sTIDL_DataParams_t *outDataParams,
                                   const sTIDL_Layer_t    *tidlLayer
                                   )
{
  int32_t status = TIDL_SUCCESS;
  int32_t i0, i1, i2, i3, i4, i5;
  int32_t imWidth    = (int32_t)inDataParams->dimValues[TIDL_DIM_WIDTH];
  int32_t imHeight   = (int32_t)inDataParams->dimValues[TIDL_DIM_HEIGHT];
  int32_t inDIM2     = (int32_t)inDataParams->dimValues[TIDL_DIM_DIM2];
  int32_t inDIM1     = (int32_t)inDataParams->dimValues[TIDL_DIM_DIM1];
  int32_t inPitch    = (int32_t)inDataParams->pitch[TIDL_LINE_PITCH];
  int32_t inChPitch  = (int32_t)inDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t inDIM2Pitch  = (int32_t)inDataParams->pitch[TIDL_DIM_DIM2];
  int32_t inDIM1Pitch  = (int32_t)inDataParams->pitch[TIDL_DIM_DIM1];
  int32_t inBatchPitch  = (int32_t)inDataParams->pitch[TIDL_ROI_PITCH];
  int32_t outPitch   = (int32_t)outDataParams->pitch[TIDL_LINE_PITCH];
  int32_t outChPitch = (int32_t)outDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t outDIM2Pitch  = (int32_t)outDataParams->pitch[TIDL_DIM_DIM2];
  int32_t outDIM1Pitch  = (int32_t)outDataParams->pitch[TIDL_DIM_DIM1];
  int32_t outBatchPitch = (int32_t)outDataParams->pitch[TIDL_ROI_PITCH];
  int32_t numTotBatches = inDataParams->dimValues[TIDL_DIM_BATCH];
  params->numChannels = inDataParams->dimValues[TIDL_DIM_NUMCH];
  /*workload already adds offset so not to add here
  */
  Tin  *inData = (Tin *)inPtr;
  Tout *outData = (Tout *)outPtr;
  float32_tidl alpha = (float)tidlLayer->layerParams.batchNormParams.inDataQ/((float)TIDL_NON_LIN_PARAM_SCALE);
  float32_tidl beta = (float)tidlLayer->layerParams.batchNormParams.weightsQ/((float)TIDL_NON_LIN_PARAM_SCALE);

  int32_t c =  params->numChannels;

  /* OPENACC(data copyin(inData[((numTotBatches-1)*inBatchPitch) + ((c-1)*inChPitch) + ((imHeight-1)*inPitch) + (imWidth-1)]) \
               copy(outData[((numTotBatches-1)*outBatchPitch) + ((c-1)*outChPitch) + ((imHeight-1)*outPitch) + (imWidth-1)]))

  OPENACC(parallel loop collapse(4)) */
  for (i5 = 0; i5 < numTotBatches; i5++)
  {
    for(i4 = 0; i4 < inDIM1; i4++)
    {
      for(i3 = 0; i3 < inDIM2; i3++)
      {
        for (i2 = 0; i2 < c; i2++)
        {
          for (i0=0; i0<imHeight; i0++)
          {
            for (i1=0; i1<imWidth; i1++)
            {
              Tin  inDataVal;
              if(tidlLayer->actParams.actType == TIDL_Tanh)
              {
                inDataVal = (inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1]);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)std::tanh(inDataVal);
              }
              else if(tidlLayer->actParams.actType == TIDL_ELU)
              {
                inDataVal = (inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1]);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)((inDataVal < 0.0) ? (alpha*(std::exp(inDataVal) - 1)) : inDataVal);
              }
              else if(tidlLayer->actParams.actType == TIDL_HardSigmoid)
              {
                inDataVal = (inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1]);
                float32_tidl outValF = alpha*inDataVal + beta;
                float32_tidl omin = 0;
                float32_tidl omax = 1;
                outValF = (outValF > omax) ? omax : outValF;
                outValF = (outValF < omin) ? omin : outValF;
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_GELU)
              {
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                float outValF;
                if(intAlgHandle->createParams->net->netVersion >= TIDL_NET_VERSION_FW_11_00_00_00)
                {
                  outValF = 0.5 * inDataVal * (1 + std::erf(inDataVal * 0.70710678118));
                }
                else
                {
                  #if TIDL_NET_VERSION_FW_PASSIVE_CODE
                  // tanh approx.
                  outValF = std::sqrt((2.0/M_PI)) * (inDataVal + (0.044715 * inDataVal * inDataVal * inDataVal));   // z = sqrt(2/PI)*(x+0.044715x^3)
                  outValF = inDataVal * (1  + std::tanh(outValF));   // 0.5x(1+tanh(z))
                  #else
                  status = TIDL_ERR_FAILURE;
                  break;
                  #endif
                }
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_LeakyReLU)
              {
                float32_tidl outValF = 0;
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                float32_tidl alpha = tidlLayer->actParams.slopeScale;
                if(inDataVal >= 0.0)
                {
                  outValF = inDataVal;
                }
                else
                {
                  outValF = inDataVal * alpha;
                }
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Asin){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                float32_tidl outValF = std::asin(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Asinh){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                float32_tidl outValF = std::asinh(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_HardSwish){
                float32_tidl hardswish_alpha = 0.16666666666f;
                float32_tidl hardswish_beta = 0.5f;

                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                float32_tidl outValF = hardswish_alpha*inDataVal + hardswish_beta;
                
                float32_tidl omin = 0;
                float32_tidl omax = 1;
                outValF = (outValF > omax) ? omax : outValF;
                outValF = (outValF < omin) ? omin : outValF;

                outValF = outValF * inDataVal;

                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Swish){
                float32_tidl swish_alpha = (float32_tidl)tidlLayer->actParams.alpha;
                float32_tidl swish_beta = 1.0f;

                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                float32_tidl reciprocal_outValF = std::exp(-1.0f*swish_alpha*inDataVal) + swish_beta;
                float32_tidl outValF = 1.0f/reciprocal_outValF;

                outValF = outValF * inDataVal;

                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Mish){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                
                float32_tidl outValF = std::log(1.0f + std::exp(inDataVal));
                outValF = std::tanh(outValF);
                outValF = outValF * inDataVal;

                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Log){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                float32_tidl outValF = std::log(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Abs){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                float32_tidl outValF = std::abs(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Floor){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                float32_tidl outValF = std::floor(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Exp){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                float32_tidl outValF = std::exp(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Sin){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                float32_tidl outValF = std::sin(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Erf){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                float32_tidl outValF = std::erf(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Sqrt){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                float32_tidl outValF = std::sqrt(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Acos){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                float32_tidl outValF = std::acos(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Atan){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                float32_tidl outValF = std::atan(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Sinh){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                float32_tidl outValF = std::sinh(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Cos){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                float32_tidl outValF = std::cos(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Cosh){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                float32_tidl outValF = std::cosh(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Neg){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                float32_tidl outValF = -(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Tan){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                float32_tidl outValF = std::tan(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Pow){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                float32_tidl outValF = std::pow(inDataVal, alpha);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Logit){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                /**
                 * logit(x) = log(x/(1-x))
                */
                inDataVal = std::min(inDataVal, 1.0f);
                inDataVal = std::max(inDataVal, 0.0f);

                float32_tidl denom = 1.0f - inDataVal;
                denom = std::max(denom, TIDL_LOGIT_CLIP_MIN);
                
                inDataVal = std::max(inDataVal, TIDL_LOGIT_CLIP_MIN);

                float32_tidl outValF = std::log(inDataVal/denom);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Reciprocal){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                float32_tidl outValF;

                // In onnx if denom is 0 in division, then output is 0
                if(inDataVal == 0)
                {
                  outValF = 0;
                }
                else
                {
                  outValF = 1 / inDataVal;
                  float32_tidl outMin = std::numeric_limits<float32_tidl>::lowest();
                  float32_tidl outMax = std::numeric_limits<float32_tidl>::max();
                  /* LDRA_JUSTIFY_START
                  <metric start> statement branch <metric end>
                  <justification start>
                  Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
                  Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
                  <justification end> */
                  outValF = (outValF > outMax)? outMax : outValF;
                  /* LDRA_JUSTIFY_END */
                  /* LDRA_JUSTIFY_START
                  <metric start> statement branch <metric end>
                  <justification start>
                  Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
                  Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
                  <justification end> */
                  outValF = (outValF < outMin)? outMin : outValF;
                  /* LDRA_JUSTIFY_END */
                }
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_SiLU){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                float32_tidl outValF;

                // In onnx if denom is 0 in division, then output is 0
                if(inDataVal == 0)
                {
                  outValF = 0;
                }
                else
                {
                  outValF = 1.0 / (1.0 + std::exp(-1.0 * inDataVal));
                  outValF = inDataVal * outValF;
                  float32_tidl outMin = std::numeric_limits<float32_tidl>::lowest();
                  float32_tidl outMax = std::numeric_limits<float32_tidl>::max();
                  
                  /* LDRA_JUSTIFY_START
                  <metric start> statement branch <metric end>
                  <justification start>
                  Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
                  Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
                  <justification end> */
                  outValF = (outValF > outMax)? outMax : outValF;
                  /* LDRA_JUSTIFY_END */
                  /* LDRA_JUSTIFY_START
                  <metric start> statement branch <metric end>
                  <justification start>
                  Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
                  Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
                  <justification end> */
                  outValF = (outValF < outMin)? outMin : outValF;
                  /* LDRA_JUSTIFY_END */
                }
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_SoftPlus){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                float32_tidl outValF = std::log(std::exp(inDataVal) + 1);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_SoftSign){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                float32_tidl outValF = inDataVal / (1 + std::abs(inDataVal));
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Ceil){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                float32_tidl outValF = std::ceil(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Celu){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                float32_tidl outValF = std::max(0.0f, inDataVal) + std::min(0.0f, alpha * (std::exp(inDataVal / alpha) - 1.0f));
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Selu){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                float32_tidl outValF = inDataVal <= 0.0 ? beta * (alpha * std::exp(inDataVal) - alpha) : (beta * inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Round){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                float32_tidl outValF;
                int32_t integerPart = (int32_t)(std::abs(inDataVal));
                float32_tidl fractionalPart = std::abs(inDataVal) - (float32_tidl)integerPart;
                if (fractionalPart < 0.5f)
                {
                  // Round down
                  outValF = (float32_tidl)integerPart;
                } 
                else if (fractionalPart > 0.5f) 
                {
                  // Round up
                  outValF = (float32_tidl)integerPart + 1.0f;
                } 
                else 
                {
                  // Exactly half - round to nearest even
                  if (integerPart % 2 == 0) {
                      // Even number, round down
                      outValF = (float32_tidl)integerPart;
                  } else {
                      // Odd number, round up to make it even
                      outValF = (float32_tidl)integerPart + 1.0f;
                  }
                }
                outValF = inDataVal < 0.0 ? outValF * -1.0f : outValF;
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Sign){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                float32_tidl outValF;
                if(inDataVal != 0.0)
                {
                  outValF = inDataVal > 0.0 ? 1.0 : -1.0;
                }
                else
                {
                  outValF = 0.0;
                }
                
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              /* LDRA_JUSTIFY_END */
              
              /* LDRA_JUSTIFY_START
              <metric start> statement branch <metric end>
              <justification start>
              Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
              Prior check occurs for other activation types.
              Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
              This does not affect runtime behavior or safety.
              <justification end> */
              else
              {
                status = TIDL_ERR_NOT_IMPLEMENTED;
              }
            }
          }
      }
      }
    }
  }
  return status;
}

/**
 * @brief This is main function perform non-linear activation operation if inp-output is in bf16-bf16 manner
 * @tparam Tin  : template for input
 * @tparam Tout : template for output
 * @param inPtr : Input pointer on which bacthNorm is applied
 * @param outPtr : Output pointer after batchNorm opreation
 * @param layerIdx :index of the current layer
 * @param params : copy of bacthNorm parameters
 * @param inDataParams : parameters of the input data buffer
 * @param outDataParams : parameters of the output data buffer
 * @param tidlLayer : Pointer to the common layer parameters
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
template <class Tin, class Tout>
static int32_t TIDL_refBFloat16NonLinear(TIDL_Handle intAlgHandle,
                                   Tin  *inPtr,
                                   Tout *outPtr,
                                   int32_t layerIdx,
                                   sTIDL_BatchNormParams_t *params,
                                   sTIDL_DataParams_t *inDataParams,
                                   sTIDL_DataParams_t *outDataParams,
                                   const sTIDL_Layer_t    *tidlLayer
                                   )
{
  int32_t status = TIDL_SUCCESS;
  int32_t i0, i1, i2, i3, i4, i5;
  int32_t imWidth    = (int32_t)inDataParams->dimValues[TIDL_DIM_WIDTH];
  int32_t imHeight   = (int32_t)inDataParams->dimValues[TIDL_DIM_HEIGHT];
  int32_t inDIM2     = (int32_t)inDataParams->dimValues[TIDL_DIM_DIM2];
  int32_t inDIM1     = (int32_t)inDataParams->dimValues[TIDL_DIM_DIM1];
  int32_t inPitch    = (int32_t)inDataParams->pitch[TIDL_LINE_PITCH];
  int32_t inChPitch  = (int32_t)inDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t inDIM2Pitch  = (int32_t)inDataParams->pitch[TIDL_DIM_DIM2];
  int32_t inDIM1Pitch  = (int32_t)inDataParams->pitch[TIDL_DIM_DIM1];
  int32_t inBatchPitch  = (int32_t)inDataParams->pitch[TIDL_ROI_PITCH];
  int32_t outPitch   = (int32_t)outDataParams->pitch[TIDL_LINE_PITCH];
  int32_t outChPitch = (int32_t)outDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t outDIM2Pitch  = (int32_t)outDataParams->pitch[TIDL_DIM_DIM2];
  int32_t outDIM1Pitch  = (int32_t)outDataParams->pitch[TIDL_DIM_DIM1];
  int32_t outBatchPitch = (int32_t)outDataParams->pitch[TIDL_ROI_PITCH];
  int32_t numTotBatches = inDataParams->dimValues[TIDL_DIM_BATCH];
  params->numChannels = inDataParams->dimValues[TIDL_DIM_NUMCH];
  /*workload already adds offset so not to add here
  */
  Tin  *inData = (Tin *)inPtr;
  Tout *outData = (Tout *)outPtr;
  bfloat16_tidl alpha = (bfloat16_tidl)tidlLayer->layerParams.batchNormParams.inDataQ/((bfloat16_tidl)TIDL_NON_LIN_PARAM_SCALE);
  bfloat16_tidl beta = (bfloat16_tidl)tidlLayer->layerParams.batchNormParams.weightsQ/((bfloat16_tidl)TIDL_NON_LIN_PARAM_SCALE);

  int32_t c =  params->numChannels;

  /* OPENACC(data copyin(inData[((numTotBatches-1)*inBatchPitch) + ((c-1)*inChPitch) + ((imHeight-1)*inPitch) + (imWidth-1)]) \
               copy(outData[((numTotBatches-1)*outBatchPitch) + ((c-1)*outChPitch) + ((imHeight-1)*outPitch) + (imWidth-1)]))

  OPENACC(parallel loop collapse(4)) */
  for (i5 = 0; i5 < numTotBatches; i5++)
  {
    for(i4 = 0; i4 < inDIM1; i4++)
    {
      for(i3 = 0; i3 < inDIM2; i3++)
      {
        for (i2 = 0; i2 < c; i2++)
        {
          for (i0=0; i0<imHeight; i0++)
          {
            for (i1=0; i1<imWidth; i1++)
            {
              Tin  inDataVal;
              if(tidlLayer->actParams.actType == TIDL_Tanh)
              {
                inDataVal = (inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1]);
                bfloat16_tidl outValF = taylor_tanh_bf16(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if(tidlLayer->actParams.actType == TIDL_HardSigmoid)
              {
                inDataVal = (inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1]);
                bfloat16_tidl outValF = alpha*inDataVal + beta;
                bfloat16_tidl omin = 0;
                bfloat16_tidl omax = 1;
                outValF = (outValF > omax) ? omax : outValF;
                outValF = (outValF < omin) ? omin : outValF;
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if(tidlLayer->actParams.actType == TIDL_ELU)
              {
                inDataVal = (inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1]);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)((inDataVal < (bfloat16_tidl)0.0f) ? (alpha*(exp_taylor_bf16(inDataVal) - (bfloat16_tidl)1.0f)) : inDataVal);
              }
              else if (tidlLayer->actParams.actType == TIDL_GELU)
              {
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                bfloat16_tidl outValF;
                if(intAlgHandle->createParams->net->netVersion >= TIDL_NET_VERSION_FW_11_00_00_00)
                {
                  outValF = gelu_bf16(inDataVal);
                }
                else
                {
                  status = TIDL_ERR_FAILURE;
                  break;
                }
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_LeakyReLU)
              {
                bfloat16_tidl outValF = 0;
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                bfloat16_tidl alpha = tidlLayer->actParams.slopeScale;
                if(inDataVal >= (bfloat16_tidl)0.0f)
                {
                  outValF = inDataVal;
                }
                else
                {
                  outValF = inDataVal * alpha;
                }
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_HardSwish){
                bfloat16_tidl hardswish_alpha = (bfloat16_tidl)0.16666666666f;
                bfloat16_tidl hardswish_beta = (bfloat16_tidl)0.5f;

                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                bfloat16_tidl outValF = hardswish_alpha*inDataVal + hardswish_beta;
                
                bfloat16_tidl omin = 0;
                bfloat16_tidl omax = 1;
                outValF = (outValF > omax) ? omax : outValF;
                outValF = (outValF < omin) ? omin : outValF;

                outValF = outValF * inDataVal;

                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Swish){
                bfloat16_tidl swish_alpha = (bfloat16_tidl)tidlLayer->actParams.alpha;
                bfloat16_tidl swish_beta = (bfloat16_tidl)1.0f;

                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                bfloat16_tidl temp = ((bfloat16_tidl)-1.0f)*swish_alpha*inDataVal;
                bfloat16_tidl reciprocal_outValF = exp_taylor_bf16(temp) + swish_beta;
                bfloat16_tidl outValF = ((bfloat16_tidl)1.0f)/reciprocal_outValF;

                outValF = outValF * inDataVal;

                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Exp){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                bfloat16_tidl outValF = exp_taylor_bf16(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Erf){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                bfloat16_tidl outValF = taylor_erf_bf16(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Reciprocal){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                bfloat16_tidl outValF;

                // In onnx if denom is 0 in division, then output is 0
                if(inDataVal == (bfloat16_tidl)0)
                {
                  outValF = 0;
                }
                else
                {
                  outValF = ((bfloat16_tidl)1.0f) / inDataVal;
                  bfloat16_tidl outMin = std::numeric_limits<Tout>::lowest();
                  bfloat16_tidl outMax = std::numeric_limits<Tout>::max();
                  /* LDRA_JUSTIFY_START
                  <metric start> statement branch <metric end>
                  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
                  <justification end> */
                  outValF = (outValF > outMax)? outMax : outValF;
                  /* LDRA_JUSTIFY_END */
                  /* LDRA_JUSTIFY_START

                  <metric start> statement branch <metric end>
                  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
                  <justification end> */
                  outValF = (outValF < outMin)? outMin : outValF;
                  /* LDRA_JUSTIFY_END */
                }
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_SiLU){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                bfloat16_tidl outValF;

                // In onnx if denom is 0 in division, then output is 0
                if(inDataVal == (bfloat16_tidl)0)
                {
                  outValF = 0;
                }
                else
                {
                  bfloat16_tidl denom = ((bfloat16_tidl)1.0f) + exp_taylor_bf16(((bfloat16_tidl)-1.0f) * inDataVal);
                  outValF = (bfloat16_tidl)__recip(denom);
                  /* Newton-Raphson refinement for reciprocal: x*(2 - a*x) */
                  outValF = outValF * ((bfloat16_tidl)2.0f - denom * outValF);
                  outValF = inDataVal * outValF;
                  bfloat16_tidl outMin = std::numeric_limits<Tout>::lowest();
                  bfloat16_tidl outMax = std::numeric_limits<Tout>::max();
                  
                  /* LDRA_JUSTIFY_START
                  <metric start> statement branch <metric end>
                  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
                  <justification end> */
                  outValF = (outValF > outMax)? outMax : outValF;
                  /* LDRA_JUSTIFY_END */
                  /* LDRA_JUSTIFY_START
                  <metric start> statement branch <metric end>
                  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
                  <justification end> */
                  outValF = (outValF < outMin)? outMin : outValF;
                  /* LDRA_JUSTIFY_END */
                }
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Asin){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                bfloat16_tidl outValF = taylor_asin_bf16(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Asinh){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                bfloat16_tidl outValF = taylor_asinh_bf16(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Mish){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                
                bfloat16_tidl outValF = taylor_log_bf16((bfloat16_tidl)1.0f + exp_taylor_bf16(inDataVal));
                outValF = taylor_tanh_bf16(outValF);
                outValF = outValF * inDataVal;

                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Log){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                bfloat16_tidl outValF = taylor_log_bf16(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Abs){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                bfloat16_tidl outValF = inDataVal;
                if(outValF < (bfloat16_tidl)0.0f)
                {
                  outValF = -(outValF);
                }
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Floor){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                
                int32_t inValInt = (int32_t)inDataVal;
                bfloat16_tidl outValF = (bfloat16_tidl)inValInt;
                if((inDataVal < (bfloat16_tidl)0.0f) && (inDataVal != outValF))
                {
                  outValF = outValF - (bfloat16_tidl)1.0f;
                }

                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Sin){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                bfloat16_tidl outValF = taylor_sin_bf16(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Sqrt){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                bfloat16_tidl outValF = sqrt_bf16(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Acos){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                bfloat16_tidl outValF = taylor_acos_bf16(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Atan){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                bfloat16_tidl outValF = taylor_atan_bf16(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Sinh){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                bfloat16_tidl outValF = taylor_sinh_bf16(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Cos){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                bfloat16_tidl outValF = taylor_cos_bf16(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Cosh){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                bfloat16_tidl outValF = taylor_cosh_bf16(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Neg){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                bfloat16_tidl outValF = -(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Tan){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                bfloat16_tidl outValF = taylor_tan_bf16(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Pow){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                bfloat16_tidl outValF = taylor_pow_bf16(inDataVal, alpha);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Logit){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                /**
                 * logit(x) = log(x/(1-x))
                */
                bfloat16_tidl logitIn = inDataVal;
                //Clamp inDataVal in [0,1] range
                logitIn = (logitIn > (bfloat16_tidl)1.0f) ? (bfloat16_tidl)1.0f : logitIn;
                logitIn = (logitIn < (bfloat16_tidl)0.0f) ? (bfloat16_tidl)0.0f : logitIn;

                bfloat16_tidl denom = (bfloat16_tidl)1.0f - logitIn;
                denom = (denom < (bfloat16_tidl)TIDL_LOGIT_CLIP_MIN) ? (bfloat16_tidl)TIDL_LOGIT_CLIP_MIN : denom;
                logitIn = (logitIn < (bfloat16_tidl)TIDL_LOGIT_CLIP_MIN) ? (bfloat16_tidl)TIDL_LOGIT_CLIP_MIN : logitIn;
                
                bfloat16_tidl outValF = taylor_log_bf16(logitIn/denom);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_SoftPlus){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                bfloat16_tidl outValF = softplus_bf16(inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_SoftSign){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                bfloat16_tidl absVal = (inDataVal < (bfloat16_tidl)0.0f) ? -(inDataVal) : inDataVal;
                bfloat16_tidl outValF = inDataVal / ((bfloat16_tidl)1.0f + absVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Ceil){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                int32_t inValInt = (int32_t)inDataVal;
                bfloat16_tidl outValF = (bfloat16_tidl)inValInt;
                if((inDataVal > (bfloat16_tidl)0.0f) && (inDataVal != outValF))
                {
                  outValF = outValF + (bfloat16_tidl)1.0f;
                }
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Celu){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                bfloat16_tidl temp = alpha * (exp_taylor_bf16(inDataVal / alpha) - (bfloat16_tidl)1.0f);
                
                bfloat16_tidl val1 = inDataVal < (bfloat16_tidl)0.0f ? (bfloat16_tidl)0.0f : inDataVal;
                bfloat16_tidl val2 = temp < (bfloat16_tidl)0.0f ? temp : (bfloat16_tidl)0.0f; 

                bfloat16_tidl outValF = val1 + val2;
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Selu){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                bfloat16_tidl outValF = inDataVal <= (bfloat16_tidl)0.0f ? beta * (alpha * exp_taylor_bf16(inDataVal) - alpha) : (beta * inDataVal);
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Round){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                bfloat16_tidl outValF;
                
                bfloat16_tidl absInDataVal = (inDataVal < (bfloat16_tidl)0.0f) ? -(inDataVal) : inDataVal;

                int32_t integerPart = (int32_t)(absInDataVal);
                bfloat16_tidl fractionalPart = absInDataVal - (bfloat16_tidl)integerPart;
                if (fractionalPart < (bfloat16_tidl)0.5f)
                {
                  // Round down
                  outValF = (bfloat16_tidl)integerPart;
                } 
                else if (fractionalPart > (bfloat16_tidl)0.5f) 
                {
                  // Round up
                  outValF = (bfloat16_tidl)integerPart + (bfloat16_tidl)1.0f;
                } 
                else 
                {
                  // Exactly half - round to nearest even
                  if (integerPart % 2 == 0) {
                      // Even number, round down
                      outValF = (bfloat16_tidl)integerPart;
                  } else {
                      // Odd number, round up to make it even
                      outValF = (bfloat16_tidl)integerPart + (bfloat16_tidl)1.0f;
                  }
                }
                outValF = inDataVal < (bfloat16_tidl)0.0f ? outValF * (bfloat16_tidl)-1.0f : outValF;
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              else if (tidlLayer->actParams.actType == TIDL_Sign){
                inDataVal = inData[(i5*inBatchPitch) + (i4*inDIM1Pitch) + (i3*inDIM2Pitch) + (i2*inChPitch) + (i0*inPitch) + i1];
                bfloat16_tidl outValF;
                if(inDataVal != (bfloat16_tidl)0.0f)
                {
                  outValF = inDataVal > (bfloat16_tidl)0.0f ? (bfloat16_tidl)1.0f : (bfloat16_tidl)-1.0f;
                }
                else
                {
                  outValF = (bfloat16_tidl)0.0f;
                }
                
                outData[(i5*outBatchPitch) + (i4*outDIM1Pitch) + (i3*outDIM2Pitch) + (i2*outChPitch) + (i0*outPitch) + i1] = (Tout)outValF;
              }
              /* LDRA_JUSTIFY_END */
              
              /* LDRA_JUSTIFY_START
              <metric start> statement branch <metric end>
              <justification start> PRIOR_CHECK : Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
              Prior check occurs for other activation types.
              <justification end> */
              else
              {
                status = TIDL_ERR_NOT_IMPLEMENTED;
              }
            }
          }
      }
      }
    }
  }
  return status;
}

/**
 * @brief This function is tanh implementation in fully float manner
 *
 * @param intAlgHandle : tidl algorothm handle
 * @param algLayer : Pointer to the layer specific parameters
 * @param tidlLayer : Pointer to the common layer parameters
 * @param params : copy of batch norm layer parameters
 * @param inPtr : Pointer to input buffers to be processed
 * @param outPtr : Pointer to output buffers to be processed
 * @param inDataParams : pointer to input data parameters
 * @param outDataParams : pointer to output data parameters
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
static int32_t TIDL_fullyFloatReferenceNonLinear(TIDL_Handle intAlgHandle,
                                      sTIDL_AlgLayer_t *algLayer,
                                      const sTIDL_Layer_t    *tidlLayer,
                                      sTIDL_BatchNormParams_t *params,
                                      void *inPtr,
                                      void *outPtr,
                                      sTIDL_DataParams_t *inDataParams,
                                      sTIDL_DataParams_t *outDataParams)
{
  int32_t status = TIDL_SUCCESS;
  int32_t layerIdx = algLayer->layerIdx;


  status = TIDL_refFloatNonLinear(intAlgHandle, 
                              (float32_tidl*)inPtr,
                              (float32_tidl*)outPtr,
                                layerIdx,
                                params,
                                inDataParams,
                                outDataParams,
                                tidlLayer
                                );


  return status;
}

/**
 * @brief This function is non-linear activations implementation for bfloat16
 *
 * @param intAlgHandle : tidl algorithm handle
 * @param algLayer : Pointer to the layer specific parameters
 * @param tidlLayer : Pointer to the common layer parameters
 * @param params : copy of batch norm layer parameters
 * @param inPtr : Pointer to input buffers to be processed
 * @param outPtr : Pointer to output buffers to be processed
 * @param inDataParams : pointer to input data parameters
 * @param outDataParams : pointer to output data parameters
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
static int32_t TIDL_BFloat16ReferenceNonLinear(TIDL_Handle intAlgHandle,
                                      sTIDL_AlgLayer_t *algLayer,
                                      const sTIDL_Layer_t    *tidlLayer,
                                      sTIDL_BatchNormParams_t *params,
                                      void *inPtr,
                                      void *outPtr,
                                      sTIDL_DataParams_t *inDataParams,
                                      sTIDL_DataParams_t *outDataParams)
{
  int32_t status = TIDL_SUCCESS;
  int32_t layerIdx = algLayer->layerIdx;

  status = TIDL_refBFloat16NonLinear(intAlgHandle,
                              (bfloat16_tidl*)inPtr,
                              (bfloat16_tidl*)outPtr,
                                layerIdx,
                                params,
                                inDataParams,
                                outDataParams,
                                tidlLayer
                                );

  return status;
}

 /**
 * @brief BatchNorm layer reference implementation
 *
 * @param intAlgHandle : tidl algorothm handle
 * @param algLayer : Pointer to the layer specific parameters
 * @param tidlLayer : Pointer to the common layer parameters
 * @param params : copy of batch norm layer parameters
 * @param inPtr : Pointer to input buffers to be processed
 * @param outPtr : Pointer to output buffers to be processed
 * @param inDataParams : pointer to input data parameters
 * @param outDataParams : pointer to output data parameters
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
static int32_t TIDL_refBatchNormProcess(TIDL_Handle intAlgHandle,
                                        sTIDL_AlgLayer_t *algLayer,
                                        const sTIDL_Layer_t    *tidlLayer,
                                        sTIDL_BatchNormParams_t *params,
                                        void *inPtr,
                                        void *outPtr,
                                        const sTIDL_DataParams_t *inDataParams,
                                        const sTIDL_DataParams_t *outDataParams)
{
  int32_t status = TIDL_SUCCESS;
  int32_t layerIdx = algLayer->layerIdx;
  void * weightPtr = ((int8_t *)(intAlgHandle->createParams->net) + params->weights);
  sTIDL_ALgBatchNormParams_t * algBatchNormParams = &algLayer->layerParams.batchNormParams;
  int16_t * orgBiasPtr = (int16_t *)get_int8_t_pointer((int8_t *)(intAlgHandle->createParams->net), params->bias);
  void * biasPtr   = ((int8_t *)algBatchNormParams->biasParamMem);
  void * slopePtr  = ((int8_t *)(intAlgHandle->createParams->net) + tidlLayer->actParams.slope);
  int32_t i0;
  int32_t numChannels = intAlgHandle->createParams->net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_NUMCH];
  float inScaleFactor = (params->weightScale * inDataParams->tensorScale) / params->biasScale;
  void * accPtr = NULL;

  if(tidlLayer->weightsElementSizeInBits == 32)
  {
    biasPtr = ((int8_t *)(intAlgHandle->createParams->net) + params->bias);

    status = TIDL_refBatchNormBitDepth(inPtr,
                                       outPtr,
                                       (float32_tidl *)weightPtr,
                                       (float32_tidl *)slopePtr,
                                       (float32_tidl *)biasPtr,
                                       (float32_tidl *)accPtr,
                                       params,
                                       intAlgHandle,
                                       layerIdx,
                                       algLayer,
                                       inDataParams,
                                       outDataParams);
  }
  else if(outDataParams->elementType == TIDL_BFloat16)
  {
    biasPtr = ((int8_t *)(intAlgHandle->createParams->net) + params->bias);
    
    status = TIDL_refBatchNormBitDepth(inPtr,
                                        outPtr,
                                        (bfloat16_tidl *)weightPtr,
                                        (bfloat16_tidl *)slopePtr,
                                        (float32_tidl *)biasPtr,
                                        (float32_tidl *)accPtr,
                                        params,
                                        intAlgHandle,
                                        layerIdx,
                                        algLayer,
                                        inDataParams,
                                        outDataParams);
  }
  else if(tidlLayer->weightsElementSizeInBits <= 8)
  {

    
    TIDL_conv2dBiasSplit((int16_t*)orgBiasPtr, (int16_t*)biasPtr, &algBatchNormParams->biasB, numChannels,
    inScaleFactor, TIDL_SAT_LO_INT16, TIDL_SAT_HI_INT16,0xFF, TIDL_getDatElementSign(inDataParams->elementType));
    
    for (i0 = 0; i0 < numChannels; i0++)
    {
      ((int16_t*)biasPtr)[i0] = ((int16_t*)biasPtr)[i0] * algBatchNormParams->biasB;
    }
    if(params->mmaV2Bias != 0){
      for (i0 = 0; i0 < numChannels; i0++)
      {
        ((int16_t*)biasPtr)[i0] = (int16_t) params->mmaV2Bias;
      }
    }

    status = TIDL_refBatchNormBitDepth(inPtr,
                                       outPtr,
                                       (int8_t *)weightPtr,
                                       (int8_t *)slopePtr,
                                       (int16_t *)biasPtr,
                                       (int32_t *)accPtr,
                                       params,
                                       intAlgHandle,
                                       layerIdx,
                                       algLayer,
                                       inDataParams,
                                       outDataParams);
  }
  else
  {
    int32_t maxBiasB = 0xFFFF;
    /* This condition indicates mixed precision where processing is in 16 bit but input is signed input.
    Here as target implementation scales the input with 256, we need to limit the biasB so that it doesn't
    go beyond 8 bits as the same is later multiplied by 256 to match the scale of accumulator*/
    if ( inDataParams->elementType == TIDL_SignedChar )
    {
      maxBiasB = 0xFF;
    }
    TIDL_conv2dBiasSplit((int16_t*)orgBiasPtr, (int32_t*)biasPtr, &algBatchNormParams->biasB, numChannels,
    inScaleFactor, TIDL_SAT_LO_INT32, TIDL_SAT_HI_INT32, maxBiasB, TIDL_getDatElementSign(inDataParams->elementType));

    for (i0 = 0; i0 < numChannels; i0++)
    {
      ((int32_t*)biasPtr)[i0] = ((((int32_t*)biasPtr)[i0])) * algBatchNormParams->biasB;
    }

    status = TIDL_refBatchNormBitDepth(inPtr,
                                       outPtr,
                                       (int16_t *)weightPtr,
                                       (int16_t *)slopePtr,
                                       (int32_t *)biasPtr,
                                       (int64_t *)accPtr,
                                       params,
                                       intAlgHandle,
                                       layerIdx,
                                       algLayer,
                                       inDataParams,
                                       outDataParams);
  }

  return status;
}

static int32_t TIDL_floatReferenceKernelSupportsActivation( const sTIDL_Layer_t * tidlLayer){

  int32_t doesSupport = 0;
  if((tidlLayer->actParams.actType == TIDL_Tanh) || 
    (tidlLayer->actParams.actType == TIDL_HardSigmoid) || 
    (tidlLayer->actParams.actType == TIDL_ELU) || 
    (tidlLayer->actParams.actType == TIDL_GELU) || 
    (tidlLayer->actParams.actType == TIDL_LeakyReLU) || 
    (tidlLayer->actParams.actType == TIDL_Asin) || 
    (tidlLayer->actParams.actType == TIDL_Asinh) || 
    (tidlLayer->actParams.actType == TIDL_Mish) || 
    (tidlLayer->actParams.actType == TIDL_HardSwish) ||
    (tidlLayer->actParams.actType == TIDL_Log) ||
    (tidlLayer->actParams.actType == TIDL_Pow) ||
    (tidlLayer->actParams.actType == TIDL_Sin) ||
    (tidlLayer->actParams.actType == TIDL_Exp) ||
    (tidlLayer->actParams.actType == TIDL_Abs) ||
    (tidlLayer->actParams.actType == TIDL_Sqrt) ||
    (tidlLayer->actParams.actType == TIDL_Acos) ||
    (tidlLayer->actParams.actType == TIDL_Atan) ||
    (tidlLayer->actParams.actType == TIDL_Sinh) ||
    (tidlLayer->actParams.actType == TIDL_Cos) ||
    (tidlLayer->actParams.actType == TIDL_Cosh) ||
    (tidlLayer->actParams.actType == TIDL_Neg) ||
    (tidlLayer->actParams.actType == TIDL_Tan) ||
    (tidlLayer->actParams.actType == TIDL_Floor) ||
    (tidlLayer->actParams.actType == TIDL_Erf) ||
    (tidlLayer->actParams.actType == TIDL_Logit) ||
    (tidlLayer->actParams.actType == TIDL_SiLU) ||
    (tidlLayer->actParams.actType == TIDL_Swish) ||
    (tidlLayer->actParams.actType == TIDL_Reciprocal) ||
    (tidlLayer->actParams.actType == TIDL_SoftPlus) ||
    (tidlLayer->actParams.actType == TIDL_SoftSign) ||
    (tidlLayer->actParams.actType == TIDL_Ceil) || 
    (tidlLayer->actParams.actType == TIDL_Celu) ||
    (tidlLayer->actParams.actType == TIDL_Selu) ||
    (tidlLayer->actParams.actType == TIDL_Round) ||
    (tidlLayer->actParams.actType == TIDL_Sign))
  {
    doesSupport = 1;
  }
  return doesSupport;

};

int32_t TIDL_batchNormRefProcess(TIDL_NetworkCommonParams *commonParams,
                              sTIDL_AlgLayer_t     * algLayer,
                              sTIDL_Layer_t        * tidlLayer,
                              void                 * inPtrs[],
                              void                 * outPtrs[],
                              int32_t                layerIdx)
{
  int32_t status = IALG_EOK;

  sTIDL_BatchNormParams_t   *params = &tidlLayer->layerParams.batchNormParams;
  sTIDL_DataParams_t        *inDataParams;
  inDataParams   = &commonParams->createParams->net->TIDLLayers[algLayer->inLayerIdx[0]].outData;

  TIDL_Obj intAlgObj;
  TIDL_CreateParams createParams;

  (void)memcpy(&createParams, commonParams->createParams, sizeof(TIDL_CreateParams));
  intAlgObj.createParams = (TIDL_CreateParams *) &createParams;


  if((tidlLayer->actParams.actType == TIDL_NoAct) || (tidlLayer->actParams.actType == TIDL_RelU) || 
     (tidlLayer->actParams.actType == TIDL_RelU6) || (tidlLayer->actParams.actType == TIDL_PRelU) ||
     (inDataParams->elementType == TIDL_SinglePrecFloat) ||
     (inDataParams->elementType == TIDL_BFloat16))
    {
      algLayer->layerParams.batchNormParams.lutParams.nonLinearActMethod = -1;
    }
    
  

  if((algLayer->layerParams.batchNormParams.lutParams.nonLinearActMethod == TIDL_HW_LUT_8B) ||
     (algLayer->layerParams.batchNormParams.lutParams.nonLinearActMethod == TIDL_HW_ILUT_8B) ||
     (algLayer->layerParams.batchNormParams.lutParams.nonLinearActMethod == TIDL_SW_SINGLEPOINT_LUT_8B) ||
     (algLayer->layerParams.batchNormParams.lutParams.nonLinearActMethod == TIDL_SW_SINGLEPOINT_LUT_16B))
  {
    void* lutPtr = algLayer->layerParams.batchNormParams.lutParams.lutParamMem;
    status = TIDL_refNonLinearLUTProcess(&intAlgObj,
                                    algLayer,
                                    tidlLayer,
                                    params,
                                    inPtrs[0],
                                    outPtrs[0],
                                    inDataParams,
                                    &tidlLayer->outData,
                                    lutPtr);
  }
  #if defined (__C7100__) || defined (__C7120__)
  else if((algLayer->layerParams.batchNormParams.lutParams.nonLinearActMethod == TIDL_HW_INTERPOLATED_LUT_16B))
  {
    void* lutPtr = algLayer->layerParams.batchNormParams.lutParams.lutParamMem;
    status = TIDL_refNonLinearInterpolLUTProcess(&intAlgObj,
                                    algLayer,
                                    tidlLayer,
                                    params,
                                    inPtrs[0],
                                    outPtrs[0],
                                    inDataParams,
                                    &tidlLayer->outData,
                                    lutPtr);

  }
  #endif
  /* Moving this top since there is no Non LUT reference code similar to other operators in lut_module.h file*/
  else if (tidlLayer->actParams.actType == TIDL_Sigmoid)
  {
    status = TIDL_refSigmoidProcess(&intAlgObj,
                                  algLayer,
                                  tidlLayer,
                                  params,
                                  inPtrs[0],
                                  outPtrs[0],
                                  inDataParams,
                                  &tidlLayer->outData);
  }
  else if((algLayer->layerParams.batchNormParams.lutParams.nonLinearActMethod == TIDL_SW_NONLUT_8B) ||
          (algLayer->layerParams.batchNormParams.lutParams.nonLinearActMethod == TIDL_SW_NONLUT_16B))
  {
    if(intAlgObj.createParams->forceNegativeTest == TIDL_SAFETY_FLAG_BATCHNORM_FORCE_BATCHNORM_CORE_ERROR_NONLUT_REFERENCE)
    {
      tidlLayer->actParams.actType = -1;
    }
    status = TIDL_refNonLinearNonLUTProcessWrapper(&intAlgObj,
                                    algLayer,
                                    tidlLayer,
                                    params,
                                    inPtrs[0],
                                    outPtrs[0],
                                    inDataParams,
                                    &tidlLayer->outData);
  }
  else if((TIDL_floatReferenceKernelSupportsActivation(tidlLayer) != 0) && (inDataParams->elementType == TIDL_SinglePrecFloat))
  {
    if(intAlgObj.createParams->forceNegativeTest == TIDL_SAFETY_FLAG_BATCHNORM_FORCE_BATCHNORM_CORE_ERROR_FLOAT_REFERENCE)
    {
      tidlLayer->actParams.actType = -1;
    }
    status = TIDL_fullyFloatReferenceNonLinear(&intAlgObj,
                                        algLayer,
                                        tidlLayer,
                                        params,
                                        inPtrs[0],
                                        outPtrs[0],
                                        inDataParams,
                                        &tidlLayer->outData);
  }
  else if((TIDL_floatReferenceKernelSupportsActivation(tidlLayer) != 0) && ((inDataParams->elementType == TIDL_BFloat16)))
  {
    if(intAlgObj.createParams->forceNegativeTest == TIDL_SAFETY_FLAG_BATCHNORM_FORCE_BATCHNORM_CORE_ERROR_FLOAT_REFERENCE)
    {
      tidlLayer->actParams.actType = -1;
    }
    status = TIDL_BFloat16ReferenceNonLinear(&intAlgObj,
                                        algLayer,
                                        tidlLayer,
                                        params,
                                        inPtrs[0],
                                        outPtrs[0],
                                        inDataParams,
                                        &tidlLayer->outData);
  }
  else
  {
    status = TIDL_refBatchNormProcess(&intAlgObj,
                                    algLayer,
                                    tidlLayer,
                                    params,
                                    inPtrs[0],
                                    outPtrs[0],
                                    inDataParams,
                                    &tidlLayer->outData);                             
  }


  TIDL_L1DandL2CacheWbInv();

  return status;
}
