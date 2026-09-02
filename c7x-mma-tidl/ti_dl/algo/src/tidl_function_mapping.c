/*
*
* Copyright (c) {2015 - 2020} Texas Instruments Incorporated
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
----------------------------------------------------------------------------
@file    tidl_function_mapping.c
@brief   This file function pointer mapping for each layer
@version 0.1 (Feb 2022) : Initial version [AJ]
----------------------------------------------------------------------------
*/

#include "tidl_layer.h"

#include "tidl_function_mapping.h"
#include "tidl_forceNegativeTest.h"

static const TIDL_CoreFunctionMap gTIDLCoreFunctionMapping[] =
{
  {/*TIDL_DataLayer             */ NULL, NULL, NULL},
  {/*TIDL_ConvolutionLayer      */ TIDL_conv2dAllocNew, TIDL_conv2DInitNew, TIDL_conv2dProcessNew},
  {/*TIDL_PoolingLayer          */ TIDL_poolingAllocNew, TIDL_poolingInitNew, TIDL_poolingProcessNew},
  {/*TIDL_ReLULayer             */ NULL, NULL, NULL},
  {/*TIDL_PReLULayer            */ NULL, NULL, NULL},
  {/*TIDL_EltWiseLayer          */ TIDL_deviceCommonAlloc, TIDL_deviceCommonInit, TIDL_eltWiseProcessNew},
  {/*TIDL_InnerProductLayer     */ TIDL_innerProductAllocNew, TIDL_innerProductInitNew, TIDL_innerProductProcessNew},
  {/*TIDL_SoftMaxLayer          */ TIDL_softmaxAllocNew, TIDL_softmaxInitNew, TIDL_softmaxProcessNew},
  {/*TIDL_BatchNormLayer        */ TIDL_batchNormAllocNew, TIDL_batchNormInitNew, TIDL_batchNormProcessNew},
  {/*TIDL_BiasLayer             */ NULL, NULL, NULL},
  {/*TIDL_ScaleLayer            */ NULL, NULL, NULL},
  {/*TIDL_Deconv2DLayer         */ TIDL_conv2dAllocNew, TIDL_conv2DInitNew, TIDL_conv2dProcessNew},
  {/*TIDL_ConcatLayer           */ TIDL_concatAllocNew, TIDL_concatInitNew, TIDL_concatProcessNew},
  {/*TIDL_SplitLayer            */ NULL, NULL, NULL},
  {/*TIDL_SliceLayer            */ TIDL_deviceCommonAlloc, TIDL_deviceCommonInit, TIDL_sliceProcessNew},
  {/*TIDL_CropLayer             */ TIDL_deviceCommonAlloc, TIDL_deviceCommonInit, TIDL_cropProcessNew},
  {/*TIDL_FlattenLayer          */ NULL, NULL, NULL},
  {/*TIDL_DropOutLayer          */ NULL, NULL, NULL},
  {/*TIDL_ArgOpLayer            */ TIDL_deviceCommonAlloc, TIDL_deviceCommonInit, TIDL_argOpProcessNew},
  {/*TIDL_DetectionOutputLayer  */ TIDL_DetectionOutputAlloc, TIDL_DetectionOutputInit, TIDL_DetectionOutputProcess},
  {/*TIDL_ShuffleChannelLayer   */ TIDL_shuffleChannelAllocNew, TIDL_shuffleChannelInitNew, TIDL_shuffleChannelProcessNew},
  {/*TIDL_ResizeLayer           */ TIDL_resizeAllocNew, TIDL_resizeInitNew, TIDL_resizeProcessNew},
  {/*TIDL_RoiPoolingLayer       */ NULL, NULL, NULL},
  {/*TIDL_OdPostProcessingLayer */ NULL, NULL, NULL},
  {/*TIDL_DepthToSpaceLayer     */ NULL, NULL, NULL},
  {/*TIDL_SigmoidLayer          */ NULL, NULL, NULL},
  {/*TIDL_PadLayer              */ TIDL_padAllocNew, TIDL_padInitNew, TIDL_padProcessNew},
  {/*TIDL_ColorConversionLayer  */ TIDL_deviceCommonAlloc, TIDL_deviceCommonInit, TIDL_colorConvertProcessNew},
  {/*TIDL_OdOutputReformatLayer */ TIDL_OdOutputReformatAlloc, TIDL_OdOutputReformatInit, TIDL_OdOutputReformatProcess},
  {/*TIDL_DataConvertLayer      */ TIDL_deviceCommonAlloc, TIDL_deviceCommonInit, TIDL_dataConvertProcessNew},
  {/*TIDL_CustomLayer           */ TIDL_customAllocNew, TIDL_customInitNew, TIDL_customProcessNew},
  {/*TIDL_BatchReshapeLayer     */ TIDL_deviceCommonAlloc, TIDL_deviceCommonInit, TIDL_batchReshapeProcessNew},
  {/*TIDL_ReduceLayer           */ TIDL_deviceCommonAlloc, TIDL_deviceCommonInit, TIDL_reduceProcessNew},
  {/*TIDL_ScatterElementsLayer  */ TIDL_scatterElementsAllocNew, TIDL_scatterElementsInitNew, TIDL_scatterElementsProcessNew},
  {/*TIDL_SqueezeLayer          */ TIDL_deviceCommonAlloc, TIDL_deviceCommonInit, NULL},
  {/*TIDL_TanhLayer             */ NULL, NULL, NULL},
  {/*TIDL_HardSigmoidLayer      */ NULL, NULL, NULL},
  {/*TIDL_ELULayer              */ NULL, NULL, NULL},
  {/*TIDL_ReshapeLayer          */ TIDL_reshapeAllocNew, TIDL_reshapeInitNew, TIDL_reshapeProcessNew},
  {/*TIDL_ConstDataLayer        */ TIDL_constDataAllocNew, TIDL_constDataInitNew, TIDL_constDataProcessNew},
  {/*TIDL_GatherLayer           */ TIDL_gatherLayerAllocNew, TIDL_gatherLayerInitNew, TIDL_gatherLayerProcessNew},
  {/*TIDL_TransposeLayer        */ TIDL_transposeAllocNew, TIDL_transposeInitNew, TIDL_transposeProcessNew},
  {/*TIDL_LayerNormLayer        */ TIDL_deviceCommonAlloc, TIDL_deviceCommonInit, TIDL_layerNormProcess},
  {/*TIDL_GridSampleLayer        */ TIDL_gridsampleAlloc, TIDL_gridsampleInit, TIDL_gridsampleProcess},
  {/*TIDL_TopKLayer             */ TIDL_deviceCommonAlloc, TIDL_deviceCommonInit, TIDL_topKProcess},
  {/*TIDL_DeformConvLayer        */ TIDL_deformConvAlloc, TIDL_deformConvInit, TIDL_deformConvProcess},
  {/*TIDL_TileLayer             */ TIDL_deviceCommonAlloc, TIDL_deviceCommonInit, TIDL_tileProcessNew},
  {/*TIDL_LogicalOpLayer        */ TIDL_deviceCommonAlloc, TIDL_deviceCommonInit, TIDL_logicalOpLayerProcessNew},
  #ifdef TIDL_KERNEL_NOT_IMPLEMENTED
  {/*TIDL_RMSNormalizationLayer */ TIDL_rmsNormAlloc, TIDL_rmsNormInit, TIDL_rmsNormProcess},
  #else
  {/*TIDL_RMSNormalizationLayer */ NULL, NULL, NULL},
  #endif
  {/*TIDL_LSTMLayer             */ TIDL_lstmAlloc, TIDL_lstmInit, TIDL_lstmProcess},
  {/*TIDL_GRULayer              */ TIDL_gruAlloc, TIDL_gruInit, TIDL_gruProcess},
  {/*TIDL_RNNLayer              */ TIDL_rnnAlloc, TIDL_rnnInit, TIDL_rnnProcess},
  {/*TIDL_GatherNDLayer         */ TIDL_gatherNDAllocNew, TIDL_gatherNDInitNew, TIDL_gatherNDProcessNew},
  {/*TIDL_CastLayer             */ TIDL_deviceCommonAlloc, TIDL_deviceCommonInit, TIDL_castProcessNew},
  {/*TIDL_GatherElementsLayer   */ TIDL_gatherElementsAllocNew, TIDL_gatherElementsInitNew, TIDL_gatherElementsProcessNew},
  {/*TIDL_ShapeLayer            */ TIDL_deviceCommonAlloc, TIDL_deviceCommonInit, TIDL_shapeProcessNew},
  {/*TIDL_SizeLayer             */ TIDL_deviceCommonAlloc, TIDL_deviceCommonInit, TIDL_sizeProcessNew},
  {/*TIDL_AttentionLayer        */ NULL, NULL, NULL},
  {/*TIDL_NonZeroLayer          */ TIDL_deviceCommonAlloc, TIDL_deviceCommonInit, TIDL_nonZeroLayerProcessNew},
  {/*TIDL_UnsupportedLayer      */ NULL, NULL, NULL},
};

int32_t TIDL_isLayerMigrated(const sTIDL_Layer_t * layer)
{
  int32_t isMigrated = 0;

  if ( gTIDLCoreFunctionMapping[layer->layerType].process != NULL)
  {
    isMigrated = 1;
  }

  return isMigrated;  
}

static int32_t TIDL_migratedToNewAllocInitFunc(sTIDL_Layer_t * layer, const sWorkloadUnit_t * workoadUnit, const sGCHelperHandle * gcHelperHandle)
{
  int32_t isMigrated = TIDL_isLayerMigrated(layer);

  /* Go to ref flow if workloadUnit is NULL and  gcHelperHandle not NULL */
  // if ( (workoadUnit == NULL) && (gcHelperHandle != NULL))
  // {
  //   isMigrated = 0;
  // }

  return isMigrated;
}


int32_t TIDL_layerAllocNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                                   const TIDL_NetworkCommonParams *commonParams,
                                   int32_t layerIdx,
                                   int32_t memorySize[TIDL_LAYER_MEMORY_MAX])
{
  int32_t status = IALG_EOK;

  int32_t isMigrated = TIDL_migratedToNewAllocInitFunc(&commonParams->createParams->net->TIDLLayers[layerIdx],
                                                        layerSpecificParams->workloadUnit,
                                                        commonParams->gcHelperHandle);

  if (( gTIDLCoreFunctionMapping[commonParams->createParams->net->TIDLLayers[layerIdx].layerType].alloc != NULL ) &&
      ( isMigrated == 1) )
  {
    status = gTIDLCoreFunctionMapping[commonParams->createParams->net->TIDLLayers[layerIdx].layerType].alloc(layerSpecificParams,
                                                                                                    commonParams,
                                                                                                    layerIdx,
                                                                                                    memorySize);
  }
  else
  {
    status = NOT_VALID;
  }

  return status;
}

int32_t TIDL_layerInitNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                                const TIDL_NetworkCommonParams  *commonParams,
                                sTIDL_AlgLayer_t               *algLayer,
                                int32_t layerIdx,
                                uint8_t *memory[TIDL_LAYER_MEMORY_MAX],
                                int32_t memorySize[TIDL_LAYER_MEMORY_MAX],
                                void    **outPtr)
{
  int32_t status = IALG_EOK;

  int32_t isMigrated = TIDL_migratedToNewAllocInitFunc(&commonParams->createParams->net->TIDLLayers[layerIdx],
                                                        layerSpecificParams->workloadUnit,
                                                        commonParams->gcHelperHandle);

  if (( gTIDLCoreFunctionMapping[commonParams->createParams->net->TIDLLayers[layerIdx].layerType].init != NULL ) &&
      ( isMigrated == 1)  )
  {
    status = gTIDLCoreFunctionMapping[commonParams->createParams->net->TIDLLayers[layerIdx].layerType].init(layerSpecificParams,
                                                                                                    commonParams,
                                                                                                    algLayer,
                                                                                                    layerIdx,
                                                                                                    memory,
                                                                                                    memorySize,
                                                                                                    outPtr);
  }
  else
  {
    /* TIDL_LDRA_TAG_DEVICE_UTILS_PRIOR_CHECK_001 */
    status = NOT_VALID;
  }

  return status;
}


int32_t TIDL_layerProcessNew(TIDL_NetworkCommonParams *commonParams,
                              sTIDL_AlgLayer_t     * algLayer,
                              sTIDL_Layer_t        * tidlLayer,
                              void                 * inPtrs[],
                              void                 * outPtrs[],
                              int32_t                layerIdx)
{
  int32_t status = IALG_EOK;

  int32_t isMigrated = TIDL_migratedToNewAllocInitFunc(&commonParams->createParams->net->TIDLLayers[layerIdx],
                                                        algLayer->workloadUnit,
                                                        commonParams->gcHelperHandle);
  if((commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_LAYERPROCESS_FORCE_REDUCELAYER) ||
     (commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_LAYERPROCESS_FORCE_REDUCELAYER_NULL))
  {
    isMigrated = 0;
    tidlLayer->layerType = TIDL_ReshapeLayer;
  }
  
  if (( gTIDLCoreFunctionMapping[commonParams->createParams->net->TIDLLayers[layerIdx].layerType].process != NULL ) &&
      ( isMigrated == 1) )
  {
    status = gTIDLCoreFunctionMapping[commonParams->createParams->net->TIDLLayers[layerIdx].layerType].process(commonParams,
                                                                                                    algLayer,
                                                                                                    tidlLayer,
                                                                                                    inPtrs,
                                                                                                    outPtrs,
                                                                                                    layerIdx);
  }
  else
  {
    status = NOT_VALID;
  }

  return status;
}

