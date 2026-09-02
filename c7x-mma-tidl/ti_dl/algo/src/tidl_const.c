#include "tidl_const.h"
#include "gc.h"
#include "gc_helper.h"

int32_t TIDL_constDataAllocNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                             const TIDL_NetworkCommonParams *commonParams,
                             int32_t layerIdx,
                             int32_t memorySize[TIDL_LAYER_MEMORY_MAX])
{
  int32_t status = IALG_EOK;

  if (commonParams->gcHelperHandle == NULL )
  {
    int32_t outDataSize = TIDL_updateDataParamsPitch(&commonParams->createParams->net->TIDLLayers[layerIdx].outData);

    memorySize[TIDL_LAYER_MEMORY_SCRATCH]    += 0;
    memorySize[TIDL_LAYER_MEMORY_PERSISTENT] += ((int32_t)outDataSize + 128);
    memorySize[TIDL_LAYER_MEMORY_OUTPUT]     += 0;
  }
  return status;
}

int32_t TIDL_constDataInitNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                                const TIDL_NetworkCommonParams  *commonParams,
                                sTIDL_AlgLayer_t               *algLayer,
                                int32_t layerIdx,
                                uint8_t *memory[TIDL_LAYER_MEMORY_MAX],
                                int32_t memorySize[TIDL_LAYER_MEMORY_MAX],
                                void    **outPtr)
{
  int32_t status = IALG_EOK;
  int8_t *src=NULL;
  int8_t *dst=NULL;
  int32_t paramOffset, elemSize;
  int32_t tidlRoiPitch, tidlChannelPitch, tidlLinePitch, tidlDim1Pitch, tidlDim2Pitch, tidlDimBatch, tidlDimNumch, tidlDimWidth, tidlDimHeight, tidlDimDim1, tidlDimDim2;
  int32_t outWidth, outHeight, outChannels;
  int32_t i=0,ch=0,batch=0,dim1=0,dim2=0;
  const sLink_t  *link;
  const sBufParams_t *srcBufParams;
  sTIDL_DataParams_t*  dataParams;
  sLink_t *linkPtrList[MAX_LINKS_PER_WL];

  paramOffset = commonParams->createParams->net->TIDLLayers[layerIdx].layerParams.constDataParams.offset;
  src = ((int8_t *)(commonParams->createParams->net) + paramOffset);
  
  tidlDimDim1 = 1;
  tidlDimDim2 = 1;
  #ifdef HOST_EMULATION
  int32_t outDataSize, dstPtrOffset;
  if (commonParams->gcHelperHandle == NULL )
  {
    dataParams = &commonParams->createParams->net->TIDLLayers[layerIdx].outData;
    outDataSize = TIDL_updateDataParamsPitch(&commonParams->createParams->net->TIDLLayers[layerIdx].outData);

    algLayer->layerParams.constDataParams.paramPtr = memory[TIDL_LAYER_MEMORY_PERSISTENT];
    dst = (int8_t *)algLayer->layerParams.constDataParams.paramPtr;
    (void)memset(dst, 0, outDataSize); //zero point

    elemSize    = TIDL_getDatElementSize(dataParams->elementType);
    dstPtrOffset = ((uint32_t)dataParams->pitch[TIDL_LINE_PITCH] * (uint32_t)(dataParams->padH)) + (uint32_t)(dataParams->padW);
    dst = dst + dstPtrOffset;

    tidlChannelPitch = dataParams->pitch[TIDL_CHANNEL_PITCH];
    tidlDim1Pitch = dataParams->pitch[TIDL_DIM1_PITCH];
    tidlDim2Pitch = dataParams->pitch[TIDL_DIM2_PITCH];
    tidlRoiPitch = dataParams->pitch[TIDL_ROI_PITCH];
    tidlLinePitch = dataParams->pitch[TIDL_LINE_PITCH];

    tidlDimBatch = dataParams->dimValues[TIDL_DIM_BATCH];
    tidlDimDim1  = dataParams->dimValues[TIDL_DIM_DIM1];
    tidlDimDim2  = dataParams->dimValues[TIDL_DIM_DIM2];
    tidlDimNumch = dataParams->dimValues[TIDL_DIM_NUMCH];
    tidlDimWidth =  dataParams->dimValues[TIDL_DIM_WIDTH];
    tidlDimHeight = dataParams->dimValues[TIDL_DIM_HEIGHT];

    memorySize[TIDL_LAYER_MEMORY_SCRATCH]    += 0;
    memorySize[TIDL_LAYER_MEMORY_PERSISTENT] += ((int32_t)outDataSize  + 128);
    memorySize[TIDL_LAYER_MEMORY_OUTPUT]     += 0;
  }
  else
  #endif
  {
    getLinkPtrs(layerSpecificParams->workloadUnit, NOT_VALID,0, linkPtrList);
    link = linkPtrList[0];
    dst = (int8_t *)TIDL_getPointerFromGC(&(link->sink), commonParams->gcHelperHandle, (TIDL_sysScratchPtr *) commonParams->tidlCommonParams->sysScratchPtr);
    srcBufParams = (const sBufParams_t *) TIDL_getBufferParams(&(link->src[0]), commonParams->gcHelperHandle);
    elemSize = getByteCnt(srcBufParams->dataType);

    TIDL_GetTensorDimensions( commonParams->net,
                              algLayer->workloadUnit,
                              layerIdx,
                              NULL,
                              NULL,
                              NULL,
                              &outWidth,
                              &outHeight,
                              &outChannels);

    dataParams = &commonParams->net->TIDLLayers[layerIdx].outData;
    tidlDimWidth = outWidth;
    tidlDimHeight = outHeight;
    tidlDimNumch = outChannels;
    tidlDimDim2 = dataParams->dimValues[TIDL_DIM_DIM2];
    tidlDimDim1 = dataParams->dimValues[TIDL_DIM_DIM1];
    tidlDimBatch = srcBufParams->metaParams.featMetaParams.numBatch;

    tidlLinePitch = outWidth + srcBufParams->padParams.cols;
    tidlChannelPitch = srcBufParams->bufWidth;
    tidlDim2Pitch = tidlChannelPitch * tidlDimNumch;
    tidlDim1Pitch = tidlDim2Pitch * tidlDimDim2;
    tidlRoiPitch = tidlDim1Pitch * tidlDimDim1;
  }

  if(dst != NULL)
  {
  for(batch = 0; batch < tidlDimBatch; batch++)
  {
    for (dim1 = 0; dim1 < tidlDimDim1; dim1++)
    {
      for (dim2 = 0; dim2 < tidlDimDim2; dim2++)
      {
        for(ch = 0; ch < tidlDimNumch; ch++)
        {
          for(i = 0; i < tidlDimHeight; i++)
          {
              (void)memcpy(dst + (((batch*tidlRoiPitch) + (dim1*tidlDim1Pitch) + (dim2*tidlDim2Pitch) + (ch*tidlChannelPitch) + (i*tidlLinePitch))*elemSize),
                      src  + (((batch * tidlDimDim1 * tidlDimDim2 * tidlDimNumch * tidlDimWidth * tidlDimHeight) 
                            + (dim1 * tidlDimDim2 * tidlDimNumch * tidlDimWidth * tidlDimHeight)
                            + (dim2 * tidlDimNumch * tidlDimWidth * tidlDimHeight)
                            + (ch * tidlDimWidth * tidlDimHeight)
                            + (i*tidlDimWidth))*elemSize),
                      tidlDimWidth*elemSize);
          }
        }
      }
    }
  }
  }
  else
  {
    status = TIDL_ERR_NULL_POINTER;
  }
  return status;
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> int32_t TIDL_constDataProcessNew.* <function end>
<justification start>
Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
TIDL_LDRA_TAG : TIDL_LDRA_TAG_CONSTDATA_PRIOR_CHECK_001
Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
This does not affect runtime behavior or safety.
<justification end> */
int32_t TIDL_constDataProcessNew(TIDL_NetworkCommonParams *commonParams,
                              sTIDL_AlgLayer_t     * algLayer,
                              sTIDL_Layer_t        * tidlLayer,
                              void                 * inPtrs[],
                              void                 * outPtrs[],
                              int32_t                layerIdx)
{
  int32_t status = IALG_EOK;
  return status;
}

