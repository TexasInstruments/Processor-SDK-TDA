#include "tidl_memcopy.h"

int32_t TIDL_memCopyProcessNew(TIDL_NetworkCommonParams *commonParams,
                               sTIDL_AlgLayer_t         *algLayer,
                               sTIDL_Layer_t            *tidlLayer,
                               void                     *inPtrs[],
                               void                     *outPtrs[],
                               int32_t                   layerIdx)
{
  int32_t status = IALG_EOK;
  int32_t producerLayerIdx = algLayer->inLayerIdx[0];
  int32_t producerType = commonParams->net->TIDLLayers[producerLayerIdx].layerType;
  int32_t doCopy;
  int32_t i;

  // If input is cache layer, guard by cacheExport flag
  if (producerType == TIDL_CacheLayer)
  {
    if (commonParams->cacheExport == 1)
    {
      doCopy = 1;
    }
    else
    {
      doCopy = 0;

      /*
       * Set output dimension and pitch to 0 if no data is being copied in order
       * so that the whole branch is not executed further
       */
      sTIDL_DataParams_t *outData = &tidlLayer->outData;
      for (i = 0; i < TIDL_DIM_MAX; i++)
      {
        outData->dimValues[i] = 0;
      }
      for (i = 0; i < TIDL_PITCH_MAX; i++)
      {
        outData->pitch[i] = 0;
      }

    }
  }
  else
  {
    doCopy = 1;
  }

  if (doCopy != 0)
  {
    sTIDL_DataParams_t *inDataParams  = &commonParams->net->TIDLLayers[producerLayerIdx].outData;
    sTIDL_DataParams_t *outDataParams = &tidlLayer->outData;
    int32_t elemSize = TIDL_getDatElementSize(inDataParams->elementType);
    int32_t batch, dim1, dim2, ch, h;

    for (batch = 0; batch < inDataParams->dimValues[TIDL_DIM_BATCH]; batch++)
    {
      for (dim1 = 0; dim1 < inDataParams->dimValues[TIDL_DIM_DIM1]; dim1++)
      {
        for (dim2 = 0; dim2 < inDataParams->dimValues[TIDL_DIM_DIM2]; dim2++)
        {
          for (ch = 0; ch < inDataParams->dimValues[TIDL_DIM_NUMCH]; ch++)
          {
            for (h = 0; h < inDataParams->dimValues[TIDL_DIM_HEIGHT]; h++)
            {
              int64_t inOff  = (int64_t)batch   * inDataParams->pitch[TIDL_ROI_PITCH]
                               + (int64_t)dim1  * inDataParams->pitch[TIDL_DIM1_PITCH]
                               + (int64_t)dim2  * inDataParams->pitch[TIDL_DIM2_PITCH]
                               + (int64_t)ch    * inDataParams->pitch[TIDL_CHANNEL_PITCH]
                               + (int64_t)h     * inDataParams->pitch[TIDL_LINE_PITCH];

              int64_t outOff = (int64_t)batch   * outDataParams->pitch[TIDL_ROI_PITCH]
                               + (int64_t)dim1  * outDataParams->pitch[TIDL_DIM1_PITCH]
                               + (int64_t)dim2  * outDataParams->pitch[TIDL_DIM2_PITCH]
                               + (int64_t)ch    * outDataParams->pitch[TIDL_CHANNEL_PITCH]
                               + (int64_t)h     * outDataParams->pitch[TIDL_LINE_PITCH];

              (void)memcpy((void *)((int8_t *)outPtrs[0] + (outOff * elemSize)),
                           (void *)((int8_t *)inPtrs[0]  + (inOff  * elemSize)),
                           (size_t)(inDataParams->pitch[TIDL_LINE_PITCH] * elemSize));
            }
          }
        }
      }
    }
  }

  return status;
}
