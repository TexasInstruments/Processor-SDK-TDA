#include "tidl_cache.h"

void TIDL_cacheGetOuterInnerCounts(const sTIDL_DataParams_t *dataParams,
                                    int32_t growthAxis,
                                    int32_t *outerCount,
                                    int32_t *innerCount)
{
  int32_t d;

  *outerCount = 1;
  *innerCount = 1;
  for (d = 0; d < growthAxis; d++)
  {
    *outerCount *= dataParams->dimValues[d];
  }
  for (d = growthAxis + 1; d < TIDL_DIM_MAX; d++)
  {
    *innerCount *= dataParams->dimValues[d];
  }
}

int32_t TIDL_cacheAllocNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                             const TIDL_NetworkCommonParams *commonParams,
                             int32_t layerIdx,
                             int32_t memorySize[TIDL_LAYER_MEMORY_MAX])
{
  int32_t status = IALG_EOK;

  if (commonParams->gcHelperHandle == NULL)
  {
    const sTIDL_Layer_t *tidlLayer = &commonParams->createParams->net->TIDLLayers[layerIdx];
    const sTIDL_DataParams_t *outData = &tidlLayer->outData;

    int32_t paramSize = TIDL_updateDataParamsPitch(outData);

    memorySize[TIDL_LAYER_MEMORY_SCRATCH] += 0;
    memorySize[TIDL_LAYER_MEMORY_OUTPUT]  += 0;

    if (TIDL_isOutDataBuff(commonParams->net, outData->dataId,
                           commonParams->createParams->currLayersGroupId) == 1)
    {
      /* Output is a graph-boundary buffer -- it will be written directly
         into the app-supplied buffer, no private persistent memRec needed. */
      memorySize[TIDL_LAYER_MEMORY_PERSISTENT] += 0;
    }
    else
    {
      memorySize[TIDL_LAYER_MEMORY_PERSISTENT] += (paramSize + 128);
    }
  }
  return status;
}

int32_t TIDL_cacheInitNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                                const TIDL_NetworkCommonParams  *commonParams,
                                sTIDL_AlgLayer_t               *algLayer,
                                int32_t layerIdx,
                                uint8_t *memory[TIDL_LAYER_MEMORY_MAX],
                                int32_t memorySize[TIDL_LAYER_MEMORY_MAX],
                                void    **outPtr)
{
  int32_t status = IALG_EOK;

  #ifdef HOST_EMULATION
  if (commonParams->gcHelperHandle == NULL)
  {
    sTIDL_Layer_t *tidlLayer = &commonParams->createParams->net->TIDLLayers[layerIdx];
    sTIDL_DataParams_t *outData = &tidlLayer->outData;
    int32_t growthAxis = tidlLayer->layerParams.cacheParams.axis;

    int32_t paramSize = TIDL_updateDataParamsPitch(outData);

    algLayer->layerParams.cacheParams.head = 0;

    if (TIDL_isOutDataBuff(commonParams->net, outData->dataId,
                           commonParams->createParams->currLayersGroupId) == 1)
    {
      /* Output is a graph-boundary buffer -- it's populated by the app or
         by this layer's own outPtrs[0] at process time, not from a private
         memRec here. dataBuf[] for this layer is never read again once
         isOutData==1 (TIDL_getLayerOutPtrs always takes the
         TIDL_getDataBuff branch in that case), so leave *outPtr untouched. */
      *outPtr = NULL;
      memorySize[TIDL_LAYER_MEMORY_PERSISTENT] += 0;
    }
    else
    {
      uint8_t *buf = memory[TIDL_LAYER_MEMORY_PERSISTENT];
      (void)memset(buf, 0, (size_t)paramSize);
      *outPtr = buf;
      memorySize[TIDL_LAYER_MEMORY_PERSISTENT] += (paramSize + 128);
    }

    // Cache starts empty
    outData->dimValues[growthAxis] = 0;

    memorySize[TIDL_LAYER_MEMORY_SCRATCH] += 0;
    memorySize[TIDL_LAYER_MEMORY_OUTPUT]  += 0;
  }
  #endif
  return status;
}

int32_t TIDL_cacheProcessNew(TIDL_NetworkCommonParams *commonParams,
                              sTIDL_AlgLayer_t     * algLayer,
                              sTIDL_Layer_t        * tidlLayer,
                              void                 * inPtrs[],
                              void                 * outPtrs[],
                              int32_t                layerIdx)
{
  int32_t status = IALG_EOK;

  #ifdef HOST_EMULATION
  if (commonParams->gcHelperHandle == NULL)
  {
    sTIDL_Network_t *net = commonParams->net;
    sTIDL_DataParams_t *outData = &tidlLayer->outData;
    int32_t growthAxis = tidlLayer->layerParams.cacheParams.axis;
    int32_t elemSize = TIDL_getDatElementSize(outData->elementType);
    int32_t capacity = tidlLayer->layerParams.cacheParams.maxCapacity;
    const int32_t *isOptionalInput = tidlLayer->layerParams.cacheParams.isOptionalInput;
    int8_t *buf = (int8_t *)outPtrs[0];
    int32_t head = algLayer->layerParams.cacheParams.head;
    int32_t validLen = outData->dimValues[growthAxis];
    int32_t outerCount, innerCount;
    int32_t j;
    int32_t deferClear = 0;

    if (head < 0)
    {
      /* Resolve a flush's deferred reset requested on this layer's previous
         call (see cacheClear handling below). */
      head = 0;
      validLen = 0;
    }

    if (commonParams->cacheClear == 1)
    {
      if (commonParams->cacheExport == 1)
      {
        /* Flush: this call's export to the app (see the outDimValues
           gating in TIDL_algProcess) must still report this call's
           pre-clear contents, so leave head/validLen untouched here and
           defer the actual reset to this layer's next invocation. */
        deferClear = 1;
      }
      else
      {
        /* Not exporting this call (cacheImport==1 fill, or abandon with
           neither import nor export): nothing needs the old contents
           visible this call, reset immediately. */
        head = 0;
        validLen = 0;
      }
    }

    TIDL_cacheGetOuterInnerCounts(outData, growthAxis, &outerCount, &innerCount);

    for (j = 0; j < tidlLayer->numInBufs; j++)
    {
      int8_t *inPtr = (int8_t *)inPtrs[j];
      int32_t producerLayerIdx;
      int32_t L;
      int32_t s, o;

      if (inPtr == NULL)
      {
        continue;
      }

      if ((isOptionalInput[j] == 1) && (commonParams->cacheImport != 1))
      {
        /* This input is gated off this call - full no-op, valid length
          left unchanged. */
        continue;
      }

      producerLayerIdx = algLayer->inLayerIdx[j];

      L = net->TIDLLayers[producerLayerIdx].outData.dimValues[growthAxis];

      if ((validLen + L) > capacity)
      {
        tidl_printf(0, "TIDL_CacheLayer (layer %d): appended length exceeds "
                      "compiled capacity %d along axis %d - oldest entries are "
                      "being overwritten\n", layerIdx, capacity, growthAxis);
      }

      for (s = 0; s < L; s++)
      {
        int32_t ringPos = (head + s) % capacity;

        for (o = 0; o < outerCount; o++)
        {
          int8_t *dst = buf + (((int64_t)o * capacity + ringPos) * innerCount * elemSize);
          int8_t *src = inPtr + (((int64_t)o * L + s) * innerCount * elemSize);

          (void)memcpy(dst, src, (size_t)(innerCount * elemSize));
        }
      }
      head = (head + L) % capacity;
      validLen = ((validLen + L) > capacity) ? capacity : (validLen + L);
    }

    algLayer->layerParams.cacheParams.head = (deferClear == 1) ? -1 : head;
    outData->dimValues[growthAxis] = validLen;
  }
  #endif

  return status;
}
