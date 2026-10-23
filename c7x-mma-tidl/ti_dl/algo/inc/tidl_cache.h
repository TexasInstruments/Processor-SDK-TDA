#include "tidl_alg_int.h"
#include "tidl_alg_utils.h"
#include "tidl_commonUtils.h"


int32_t TIDL_cacheAllocNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                             const TIDL_NetworkCommonParams *commonParams,
                             int32_t layerIdx,
                             int32_t memorySize[TIDL_LAYER_MEMORY_MAX]);

int32_t TIDL_cacheInitNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                                const TIDL_NetworkCommonParams  *commonParams,
                                sTIDL_AlgLayer_t               *algLayer,
                                int32_t layerIdx,
                                uint8_t *memory[TIDL_LAYER_MEMORY_MAX],
                                int32_t memorySize[TIDL_LAYER_MEMORY_MAX],
                                void    **outPtr);

int32_t TIDL_cacheProcessNew(TIDL_NetworkCommonParams *commonParams,
                              sTIDL_AlgLayer_t     * algLayer,
                              sTIDL_Layer_t        * tidlLayer,
                              void                 * inPtrs[],
                              void                 * outPtrs[],
                              int32_t                layerIdx);

/**
 @fn      TIDL_cacheGetOuterInnerCounts
 @brief   Splits dataParams->dimValues[] around growthAxis into the product
          of dims strictly more major (outerCount) and strictly more minor
          (innerCount), using the fixed memory order
          BATCH>DIM1>DIM2>NUMCH>HEIGHT>WIDTH (outermost to innermost).
*/
void TIDL_cacheGetOuterInnerCounts(const sTIDL_DataParams_t *dataParams,
                                    int32_t growthAxis,
                                    int32_t *outerCount,
                                    int32_t *innerCount);
