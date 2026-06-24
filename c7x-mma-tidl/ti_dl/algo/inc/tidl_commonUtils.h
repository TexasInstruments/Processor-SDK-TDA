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
@file    tidl_commonUtils.h
@brief   This file contains TILD common utilities
@version 0.1 (May 2018) : Initial version [ADK]
----------------------------------------------------------------------------
*/

#ifndef ITIDL_COMMON_UTILS_H
#define ITIDL_COMMON_UTILS_H

#if defined(__C7100__) || defined(__C7120__) || defined(__C7504__) || defined(__C7524__) || defined (__C7604__)
#if !defined(_HOST_BUILD)
#include <c7x.h>
#define RESTRICT restrict
#else
#include <c7x.h>
#define RESTRICT
#endif
#endif

#include "tidl_alg_int.h"
#include "tidl_bufParams.h"
#include "tidl_device_utils_nc.h"
#include "tidl_commonUtils_stats.h"
#include "tidl_traceUtils.h"
#include <typeinfo>
#include <limits>
#ifdef BUILD_WITH_CUDA
#include <chrono>
#endif
#include "tidl_device_mem_properties.h"

#define TIDL_INTERNAL_POOLING_WEIGHT_Q_U8 ((uint32_t)8)
#define TIDL_INTERNAL_POOLING_WEIGHT_Q_U16 ((uint32_t)12)
#define TIDL_INTERNAL_INDATA_Q ((uint32_t)7)

#if defined(PERF_MODELLING) || !defined(HOST_EMULATION) || defined(SOC_TDA54) || defined(BUILD_WITH_CUDA)
#define ENABLE_PROFILE  (1)
#else
#define ENABLE_PROFILE  (0)
#endif

#define TIDL_MAX_CORENUM (TIDL_MAX_NUM_CORES) /* IMP: Should be in synch and same as TIDL_TB_MAX_CORENUM*/

/*To get Numcores based on Start Core Idx */
#define GET_NUMCORES_FOR_STARTCOREIDX(NUMCORES, STARTCOREIDX) ((NUMCORES) + (STARTCOREIDX))
#define GET_RELATIVE_COREIDX(COREID, STARTCOREIDX) ((COREID) - (STARTCOREIDX))

/* To enable / disable compression related functions.
 * 0 --> Functions are disabled.
 * 1 --> Functions are enabled. */
#define COMPRESSION_ENABLED 0

#if defined(SOC_J784S4) || defined(SOC_J742S2) || defined(SOC_J722S)
#define TIDL_DEVICE_MULTICORE (1)
#else
#define TIDL_DEVICE_MULTICORE (0)
#endif

#if defined(SOC_AM62A) || defined(SOC_J722S)
#define TIDL_DEVICE_L2_WRITETHROUGH (1)
#else
#define TIDL_DEVICE_L2_WRITETHROUGH (0)
#endif

#if defined(SOC_J721E) || defined(SOC_J721S2)
#define TIDL_DEVICE_CACHE_COHERENT (1)
#else
#define TIDL_DEVICE_CACHE_COHERENT (0)
#endif

/**
----------------------------------------------------------------------------
@ingroup    TIDL_COMMON_UTILS
@fn         TIDL_getCoreNum
@brief      Function returns the core number on which the code is executing
@param      void : None

@return     Core number on which the code is executing
----------------------------------------------------------------------------
*/

#if defined TIDL_COVERAGE_DEAD_CODE

uint32_t TIDL_getCoreNum(void);
#endif

#if (TIDL_DEVICE_CACHE_COHERENT == 0)
#ifndef HOST_EMULATION
void TIDL_c7xCleaninvalidateL1DCache(void);
void TIDL_c7xCleaninvalidateL2Cache(void);
#endif
#endif

#define TIDL_GLOBAL_SYNC_OFFSET (1) /* Space reserved for global frame level sync */
/* Modes to define execution state of given workload - may require bit wise operations */
typedef enum
{
  TIDL_SYNC_WL_STATE_DEFAULT = -1,
  TIDL_SYNC_WL_STATE_RESET = 1,
  TIDL_SYNC_WL_STATE_DMA_READY = 2,
  TIDL_SYNC_WL_STATE_COMPLETE = 4,
} eTIDL_syncWlState;

/* Modes to indicate location where sync is needed - this determines which of eTIDL_syncWlState to update/poll on */
typedef enum
{
  TIDL_SYNC_POINT_START = 0,
  TIDL_SYNC_POINT_DMA_READY = 1,
  TIDL_SYNC_POINT_COMPLETE = 2,
  TIDL_SYNC_POINT_GLOBAL_SYNC = 3
} eTIDL_syncPoint;

extern void TIDL_syncWl(TIDL_sharedBufferProperties *ctrlSetArgs, int32_t wlToPoll[], int32_t syncPoint, int32_t wlId,
                        uint32_t coreId, int32_t numCores);

void static inline TIDL_L1DCacheWbInv(void)
{
#if ((TIDL_DEVICE_CACHE_COHERENT == 0))
#ifndef HOST_EMULATION
  TIDL_c7xCleaninvalidateL1DCache();
#endif
#endif
}

void static inline TIDL_L2DCacheWbInv(void)
{
#if ((TIDL_DEVICE_CACHE_COHERENT == 0) && (TIDL_DEVICE_L2_WRITETHROUGH == 0))
#ifndef HOST_EMULATION
  TIDL_c7xCleaninvalidateL2Cache();
#endif
#endif
}

void static inline TIDL_L1DandL2CacheWbInv(void)
{
#ifndef HOST_EMULATION
  TIDL_L1DCacheWbInv();
  TIDL_L2DCacheWbInv();
#endif
}

void tidlPerfStatsDdrStatsReadCounters(uint32_t *val0, uint32_t *val1, uint32_t *val2, uint32_t *val3, bool raw);
void TIDL_updateDDRBandwidthData(uint64_t * ptr,  uint64_t read_bytes_total, uint64_t write_bytes_total);



OPENACC(routine(TIDL_roundSat))
int64_t TIDL_roundSat(int64_t val, int32_t bits, int32_t min, int32_t max);

OPENACC(routine(TIDL_roundSatMMA))
int64_t TIDL_roundSatMMA(int64_t val, int32_t bits, int32_t min, int32_t max);

void TIDL_getSaturationLimits(int32_t elementType, int32_t *satLow, int32_t *satHigh);

void TIDL_getSaturationFloat(sTIDL_Layer_t *tidlLayer1, float32_tidl *min1, float32_tidl *max1);
float32_tidl TIDL_floatSat(float32_tidl outAcc1, sTIDL_Layer_t *tidlLayer1);

void TIDL_UpdateScaleFactors(TIDL_Handle intAlgHandle, int32_t i, int32_t updateStats, int64_t accMin, int64_t accMax);

int32_t tidl_updateAcrossLayerStates(TIDL_Handle algHandle1,
                                     int32_t currLayerNum1,
                                     uint8_t **coeffStagePtr1,
                                     int32_t isChannelwiseQuantEnable1,
                                     int32_t targetDevice1);
template <class Tdst, class Tsrc>
void TIDL_conv2dBiasSplit(Tsrc *srcPtr, Tdst *dstPtr, int32_t *biasB, int32_t dataSize, float32_tidl inScaleFactor, int32_t satLow, int32_t satHigh, int32_t biasBMax, int32_t inFeatSign);
template <class Tdst, class Tsrc>
void TIDL_AM_conv2dBiasSplit(Tsrc *srcPtr, Tdst *dstPtr, int32_t *biasB, int32_t dataSize, float32_tidl inScaleFactor, int32_t satLow, int32_t satHigh, int32_t biasBMax, int32_t inFeatSign);

template <class Tdst, class Tsrc>
void TIDL_conv2dBiasSplitWithFixedBiasB(Tsrc *srcPtr,
                                        Tdst *dstPtr,
                                        int32_t *biasB,
                                        int32_t dataSize,
                                        float32_tidl inScaleFactor,
                                        int32_t satLow,
                                        int32_t satHigh,
                                        int32_t biasBMax,
                                        int32_t inFeatSign);

template<typename Tscale>
float32_tidl TIDL_getMMAv2_ScaleShiftAndError(
    float32_tidl scaleRatio,
    Tscale *scale,
    uint8_t *shift,
    int32_t weightBits,
    float32_tidl maxScaleMMAv2,
    bool skipCheck);

int32_t TIDL_prePareMemcpyTr(void *trMem);
int32_t TIDL_memcpy2D(
    const void *dstPtr,
    const void *srcPtr,
    uint32_t width,
    uint16_t height,
    int32_t dstStride,
    int32_t srcStride,
    void *dmaUtilsContext,
    uint8_t *trMem);
#if defined TIDL_COVERAGE_DEAD_CODE
void TIDL_memcpy2DAsyncTrigger(const void *dstPtr,
                               const void *srcPtr,
                               uint16_t width,
                               uint16_t height,
                               int32_t dstStride,
                               int32_t srcStride,
                               void *dmaUtilsContext,
                               int32_t convert16bitTo8Bit,
                               uint8_t *trMem);
void TIDL_memcpy2DAsyncWait(void *dmaUtilsContext);
#endif

/**
 * @brief Memory copy for trigger (TIDL_memcpy2D) with cache writeback feature at destination pointer memory region
 *
 * @param dstPtr : Pointer to dst memory
 * @param srcPtr : Pointer to src memory
 * @param width : Width of the memory copy
 * @param height : Height of the memory copy
 * @param dstStride : Stride of the dst buffer
 * @param srcStride : Stride of the src buffer
 * @param dmaUtilsContext : memory for dmaUtilsContext
 * @param trMem : pointer to transfer memory
 * @param sysMems : System memory information
 * @param enableL1DWbForL2OrMsmc : Flag depicting whether L1D wb must be enabled for L2/MSMC regions
 */

int32_t static inline TIDL_memcpy2DCacheWbInv(
    const void *dstPtr,
    const void *srcPtr,
    uint32_t width,
    uint16_t height,
    int32_t dstStride,
    int32_t srcStride,
    void *dmaUtilsContext,
    uint8_t *trMem,
    sTIDL_sysMemHandle_t sysMems[TIDL_SYSMEM_MAX])
{
  int32_t retVal = IALG_EOK;
#if (TIDL_DEVICE_CACHE_COHERENT == 0)
  if (((uint8_t *)dstPtr >= (uint8_t *)sysMems[TIDL_SYSMEM_DDR_SCRATCH].base) && ((uint8_t *)dstPtr < &((uint8_t *)sysMems[TIDL_SYSMEM_DDR_SCRATCH].base)[sysMems[TIDL_SYSMEM_DDR_SCRATCH].size]))
  {
    /* DDR requires both L1D and L2 cache writeback */
    TIDL_L1DandL2CacheWbInv();
  }
  else if (((uint8_t *)dstPtr >= (uint8_t *)sysMems[TIDL_SYSMEM_L1_SCRATCH].base) && ((uint8_t *)dstPtr < &((uint8_t *)sysMems[TIDL_SYSMEM_L1_SCRATCH].base)[sysMems[TIDL_SYSMEM_L1_SCRATCH].size]))
  {
    /* No writeback needed if destination is L1D */
#if defined(SOC_AM62A) /*for am62a we allocate L1 as part of L2 aux so we need to do cache op here */
    TIDL_L1DCacheWbInv();
#endif
  }
  else
  {
    /* else block is added for MISRA C error */
  }
#endif
  retVal = TIDL_memcpy2D(dstPtr, srcPtr, width, height, dstStride, srcStride, dmaUtilsContext, trMem);
  return retVal;
}

void *my_malloc(int32_t size);
void my_free(void *ptr);

static inline void TIDL_getTscTime(uint64_t *time1)
{
#if ENABLE_PROFILE
#ifdef BUILD_WITH_CUDA
  /* Use x86 high-resolution clock for CUDA builds */
  auto now = std::chrono::high_resolution_clock::now();
  *time1 = (uint64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
#else
  /* Use C7x Time Stamp Counter for target builds */
  *time1 = __TSC;
#endif
#endif
}

static inline void TIDL_readEmifCounter(uint32_t *count, uint32_t counterIdx)
{
#if !defined(_HOST_BUILD)
#if 0
  uint32_t * currPtr = (uint32_t *) (0x02980104 + ((counterIdx) * 0x4));
  *count = *currPtr;
#else
  *count = 0;
#endif
#endif
}

static inline void TIDL_updateprofileData(uint64_t *ptr, uint32_t idx, uint64_t tStart, uint64_t tEnd)
{
#if ENABLE_PROFILE
  ptr[idx] += (tEnd - tStart);
#endif
}

void tidl_printf(int8_t traceLevel, const char *format, ...);

int32_t tidl_writeTraceParamBuf(sTIDL_Network_t *net,
                                sTIDL_AlgLayer_t *algLayer,
                                sTIDL_Layer_t *tidlLayer,
                                int32_t dataBuffId,
                                int8_t *orgScratchPtr,
                                uint32_t orgScratchSize);

int32_t tidl_writeLayerMinMax(sTIDL_Network_t *net, int32_t currLayersGroupId, int8_t *scratchPtr, uint32_t scratchSize);

int32_t tidl_writeTraceDataBuf(int8_t *ptr,
                               sTIDL_Network_t *net,
                               sTIDL_AlgLayer_t *algLayer,
                               int32_t dataBuffId,
                               int8_t *orgScratchPtr,
                               uint32_t orgScratchSize,
                               int32_t startHeight,
                               int32_t startChannel,
                               int32_t startBatch,
                               int32_t height,
                               int32_t numChannels,
                               int32_t numBatches,
                               int32_t totalBatches,
                               int32_t currChPitch,
                               int32_t coreId,
                               const char *traceBaseName,
                               int32_t isInputData);

// Static inline function definitions:
static inline int32_t TIDL_calcLCM(int32_t a, int32_t b)
{
  int32_t lcm = 0, i;
  for (i = 1; i <= b; i++)
  {
    lcm = a * i;
    if ((lcm % b) == 0)
    {
      break;
    }
  }
  return lcm;
}

static inline int32_t TIDL_ceilDiv(int32_t n, int32_t d)
{
  int32_t result;

  if ((n % d) == 0)
  {
    result = n / d;
  }
  else
  {
    result = ((n / d) + 1);
  }

  return result;
}

static inline int32_t TIDL_clipNeg(int32_t x)
{
  int32_t result;

  if (x < 0)
  {
    result = 0;
  }
  else
  {
    result = x;
  }
  return result;
}

template <typename Tdst>
static inline Tdst tidl_sat(float32_tidl val)
{
  float32_tidl out;
  float32_tidl max = 0.0f;
  float32_tidl min = 0.0f;
  float32_tidl temp_val = 0.0f;
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  This is a safety check to ensure that the values doesn't cross min and max boundaries.
  <justification end> */
  max = (float32_tidl) std::numeric_limits<Tdst>::max();
  min = (float32_tidl) std::numeric_limits<Tdst>::lowest();
  /* LDRA_JUSTIFY_END */
  temp_val = (val < min) ? min : val;
  out = (temp_val > max) ? max : temp_val;

  return (Tdst)out;
}

int32_t tidl_writeLayerMinMax(sTIDL_Network_t *net, int32_t currLayersGroupId, int8_t *scratchPtr, uint32_t scratchSize);

int32_t tidl_writeTraceDataBuf(int8_t *ptr,
                               sTIDL_Network_t *net,
                               int32_t dataBuffId,
                               int8_t *orgScratchPtr,
                               uint32_t orgScratchSize,
                               int32_t startHeight,
                               int32_t startChannel,
                               int32_t height,
                               int32_t numChannels,
                               int32_t currChPitch,
                               int32_t coreId,
                               const char *traceBaseName);

int32_t TIDL_generateFillSeamPredicateRegisters(
    int32_t numTiles,
    void *bufPredicateStore,
    const TIDL_bufParams3D_t *srcAddr,
    const TIDL_bufParams3D_t *dstAddr);

static inline uint32_t TIDL_isAsymQuantEnabledTFL(int32_t quantizationStyle)
{
  uint32_t quantStyleAsym = FALSE;
  if ((quantizationStyle == TIDL_QuantStyleAsymNP2_TFL))
  {
    quantStyleAsym = TRUE;
  }
  return quantStyleAsym;
}

static inline int32_t TIDL_isAsymQuantEnabled(int32_t quantizationStyle, int32_t procElemSize)
{
  int32_t quantStyleAsym = FALSE;
  if ((quantizationStyle == TIDL_QuantStyleAsymNP2) && (procElemSize != 32))
  {
    quantStyleAsym = TRUE;
  }
  return quantStyleAsym;
}

void TIDL_printOutDataProperties(sTIDL_DataParams_t *outData, int32_t layerIdx);

#define TIDL_FLOW_CTRL_REF_ONLY (0x00000001U)
#define TIDL_FLOW_CTRL_REF_STAT (0x00000002U)
#define TIDL_FLOW_CTRL_MMA_NATC (0x00000004U)
#define TIDL_FLOW_CTRL_DSP_NATC (0x00000008U)
#define TIDL_FLOW_CTRL_REF_COMP (0x00000010U)
/* AVX Optimised kernels for reference flow */
#define TIDL_FLOW_CTRL_AVX_REF (0x00000020U)
#define TIDL_FLOW_CTRL_AVX_FLOAT (0x00000040U)
#define TIDL_FLOW_CTRL_PERF_MODEL (0x00000080U)

int32_t TIDL_getProcessingElementSizeInBytes(const sTIDL_Layer_t *tidlLayer);
#if defined TIDL_COVERAGE_DEAD_CODE
int32_t TIDL_conv2dGetKernelDataType(int32_t tidlElemType, int32_t procElemSize);
#endif
uint32_t TIDL_GetKernelDataTypeFromNcDataType(int32_t ncDataType);

/**
 * @brief  Get equivalent kernel data type based on TIDL side data type
 *
 * @param tidlElemType  : TIDL side element type
 */
int32_t TIDL_getKernelDataType(int32_t tidlElemType);

#if defined TIDL_COVERAGE_DEAD_CODE
int32_t TIDL_getMmalibDatatypeFromTidlDatatype(int32_t tidlElemType, int32_t procElemSize);

void TIDL_printDMATr(void *dmaTr);
#endif
int32_t TIDL_checkPixelInPadRegion(
    int32_t spatialOffsetY,
    int32_t spatialOffsetX,
    int32_t validPosXMin,
    int32_t validPosXMax,
    int32_t validPosYMin,
    int32_t validPosYMax);
int32_t TIDL_getParametricSatLimit(float32_tidl clipVal, float32_tidl tensorScale, int32_t zeroPoint);

void TIDL_convertFloatToScaleAndShift(float32_tidl val, int32_t *scale, int32_t *shift, int32_t precisionBits);

void TIDL_GetTensorDimensionsBatchReshape(const sTIDL_Network_t *net,
                                          const void *workloadUnit,
                                          int32_t layerIdx,
                                          int32_t *inBatch,
                                          int32_t *outBatch);

/* Function to derive tensorScale value from net structure for a given layer*/
// TODO: Need to remove this later when value comes from gc
float32_tidl TIDL_getTensorScale(const sTIDL_Network_t *net, int32_t layerIdx);

// TODO: Need to remove this later when value comes from gc
int32_t TIDL_getTensorZeroPoint(const sTIDL_Network_t *net, int32_t layerIdx);

/* Function to derive roundbits value from net structure for a given layer*/
// TODO: Need to remove this later when value comes from gc
int32_t TIDL_getRoundbits(const sTIDL_Network_t *net, int32_t layerIdx);

#if COMPRESSION_ENABLED
void TIDL_computeCompressionAlgo(uint8_t *bufferPtr, uint64_t bufLength, int32_t elementType, int32_t *retBias, int32_t *retAlg);
int32_t TIDL_set1DCompressionProperties(int32_t bufSize, sTIDL_compressionDimensions *compDim);
int32_t TIDL_compressFlatSW(uint8_t *const pInput,
                            uint8_t *const pOutput,
                            uint8_t *const sectr,
                            uint8_t *const cdbTable,
                            uint8_t *trMem,
                            sTIDL_compressionDimensions *pCompDims);
#endif

int32_t TIDL_isConcatBalanced(TIDL_NetworkCommonParams *commonParams, sTIDL_Layer_t *layer, sTIDL_AlgLayer_t *algLayer, int32_t layerIdx);


/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION_SUPPORT
@fn         TIDL_isOutDataBuff
@brief      Function to Check whether current data buffer is Output from
            current algorithm instance

@param      pTIDLNetStructure : Base pointer for net
@param      dataId            : Current data buffer ID
@param      layersGroupId     : current algorithm instance Group ID
@remarks    None
@return     IALG_EOK   - Successful
            IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/
int32_t TIDL_isOutDataBuff(const sTIDL_Network_t * pTIDLNetStructure,
int32_t dataId, int32_t layersGroupId);

#endif /*ITIDL_COMMON_UTILS_H */
