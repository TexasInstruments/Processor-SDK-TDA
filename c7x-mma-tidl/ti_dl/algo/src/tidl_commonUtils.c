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
@file    tidl_commonUtils.c
@brief   This file contains TILD common utilities
@version 0.1 (May 2018) : Initial version [ADK]
----------------------------------------------------------------------------
*/

#include "tidl_types.h"
#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include "../../custom/tidl_custom.h"
#include "gc_helper.h"
#include <stdint.h>
#include <limits>
#include <math.h>
#include <float.h>
#include "tidl_device_mem_properties.h"
/* As LDRA is treating "not" as MACRO, modifing code of line from "not" to "!" without effecting the functionality */
#if !defined(SOC_AM62A) && !defined(SOC_J722S) && !defined (SOC_TDA54)
#if !defined(HOST_EMULATION)
#include <ti/csl/arch/c7x/cslr_C7X_CPU.h>
#endif
#endif

#define USE_16BIT_BIAS_FOR_8BIT_MODE (1)

#if defined(SOC_J784S4) || defined(SOC_J742S2)
#define USE_STACK_MEM_TR // TODO: Temporary workaround
#endif

#ifdef TIDL_BUILD_WITH_MEMTRACES
extern int8_t *MemTrace_Buf;
int32_t curPos = 0;
#endif

#include "dmautils_ext.h"

uint32_t temp_MMA_SIZE = __C7X_VEC_SIZE_BYTES__;

#if ((TIDL_DEVICE_CACHE_COHERENT == 0))
/** CSL_c7xCleaninvalidateL1DCache function is part of CSL c7x arch, however the c7x arch build is not maintained by PDK,
 * and is expected to be maintained by user of these files
 * So this function is added within TIDL lib with TIDL_ prefix to avoid any symbols redefined errors in case it gets included in PDK build in future
 */
#ifndef HOST_EMULATION

#ifdef __cplusplus
extern "C"
{
#endif
  void TIDL_c7xSetL1DWBINV(uint64_t param);
  uint64_t TIDL_c7xGetL1DWBINV(void);
  void TIDL_c7xSetL1DINV(uint64_t param);
  void TIDL_c7xSetL2WBINV(uint64_t param);
  uint64_t TIDL_c7xGetL2WBINV(void);
#ifdef __cplusplus
}
#endif
#if ((TIDL_DEVICE_CACHE_COHERENT == 0) && (TIDL_DEVICE_L2_WRITETHROUGH == 0))
__asm__ __volatile__("TIDL_c7xSetL1DWBINV: \n"
                     "    MVC	.S1	A4,		ECR259; \n"
                     "    RET	.B1\n");
__asm__ __volatile__("TIDL_c7xGetL1DWBINV:"
                     " MVC	.S1	ECR259,	A4	;"
                     " RET	.B1;");
#elif ((TIDL_DEVICE_CACHE_COHERENT == 0) && (TIDL_DEVICE_L2_WRITETHROUGH == 1))
/*this is copied over from mcu_plus_sdk API name CacheP_setL1DWBINV()*/
__asm__ __volatile__("TIDL_c7xSetL1DWBINV: \n"
                     "    MVC	.S1	A4,		L1DWBINV; \n"
                     "    RET	.B1\n");
/* this is not verified completely. Keeping for future testing and enablement */
__asm__ __volatile__("TIDL_c7xSetL1DINV: \n"
                     "    MVC	.S1	A4,		ECR260; \n"
                     "    RET	.B1\n");
#endif
#endif
#define TIDL_DMC_L1DWBINV_WBINV_MASK ((uint64_t)0x00000001U)
#define TIDL_DMC_L1DWBINV_WBINV_SHIFT (0U)
#define TIDL_DMC_L1DWBINV_WBINV_RESETVAL ((uint64_t)0x00000000U)
#define TIDL_DMC_L1DWBINV_WBINV_MAX ((uint64_t)0x00000001U)

#ifndef HOST_EMULATION
void TIDL_c7xCleaninvalidateL1DCache(void)
{
#if ((TIDL_DEVICE_CACHE_COHERENT == 0) && (TIDL_DEVICE_L2_WRITETHROUGH == 0))
  volatile uint64_t wbinv;
  wbinv = TIDL_c7xGetL1DWBINV();
  wbinv &= ~TIDL_DMC_L1DWBINV_WBINV_MASK;
  wbinv |= (0x1U << TIDL_DMC_L1DWBINV_WBINV_SHIFT) & TIDL_DMC_L1DWBINV_WBINV_MASK;
  TIDL_c7xSetL1DWBINV(wbinv);
#elif ((TIDL_DEVICE_CACHE_COHERENT == 0) && (TIDL_DEVICE_L2_WRITETHROUGH == 1))
  TIDL_c7xSetL1DWBINV(1);
#endif
  return;
}

__asm__ __volatile__("TIDL_c7xSetL2WBINV: \n"
                     "    MVC	.S1	A4,		ECR387  ; \n"
                     "    RET	.B1\n");
__asm__ __volatile__("TIDL_c7xGetL2WBINV: \n"
                     " MVC	.S1	ECR387,	A4	; \n"
                     " RET	.B1; \n");
#define TIDL_UMC_L2WBINV_WBINV_MASK (0x00000001ULL)
#define TIDL_UMC_L2WBINV_WBINV_SHIFT (0ULL)
#define TIDL_UMC_L2WBINV_WBINV_RESETVAL (0x00000000ULL)
#define TIDL_UMC_L2WBINV_WBINV_MAX (0x00000001ULL)
#if ((TIDL_DEVICE_CACHE_COHERENT == 0) && (TIDL_DEVICE_L2_WRITETHROUGH == 0))
void TIDL_c7xCleaninvalidateL2Cache(void)
{
  volatile uint64_t wbinv;
  wbinv = TIDL_c7xGetL2WBINV() & ~TIDL_UMC_L2WBINV_WBINV_MASK;
  wbinv |= (0x1U << TIDL_UMC_L2WBINV_WBINV_SHIFT) & TIDL_UMC_L2WBINV_WBINV_MASK;
  TIDL_c7xSetL2WBINV(wbinv);
  return;
}
#endif
#endif
#endif

#define APP_PERF_DDR_STATS_CTR0 (0x00) /* A value of 0x00 configures counter 0 to return number of write transactions  */
#define APP_PERF_DDR_STATS_CTR1 (0x01) /* A value of 0x01 configures counter 1 to return number of read transactions   */
/* Use counter 2 and 3 to provide stats other than read/write transactions */
#define APP_PERF_DDR_STATS_CTR2 (0x03) /* A value of 0x03 configures counter 3 to return number of command activations */
#define APP_PERF_DDR_STATS_CTR3 (0x1C) /* A value of 0x1C configures counter 4 to return number of queue full states   */

#if defined(SOC_J721E) || defined(SOC_AM62A) || defined(SOC_J722S)
#define APP_PERF_NUM_DDR_INSTANCES (1u)
#elif defined(SOC_J721S2) || defined(SOC_J742S2)
#define APP_PERF_NUM_DDR_INSTANCES (2u)
#elif defined(SOC_J784S4)
#define APP_PERF_NUM_DDR_INSTANCES (4u)
#endif

#define APP_PERF_DDR_BURST_SIZE_BYTES (64u)
void TIDL_updateDDRBandwidthData(uint64_t *ptr, uint64_t read_bytes_total, uint64_t write_bytes_total)
{
  ptr[READ] += read_bytes_total * APP_PERF_DDR_BURST_SIZE_BYTES;
  ptr[WRITE] += write_bytes_total * APP_PERF_DDR_BURST_SIZE_BYTES;
  return;
}
/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> void tidlPerfStatsDdrStatsReadCounters.* <function end>
<justification start> DEBUG_TRACE : This function is solely for debugging purposes and is not part of the production code.
<justification end> */
void tidlPerfStatsDdrStatsReadCounters(uint32_t *val0, uint32_t *val1, uint32_t *val2, uint32_t *val3, bool raw)
{
  *val0 = 0;
  *val1 = 0;
  *val2 = 0;
  *val3 = 0;
  if (tidl_getTraceLogLevel() >= 2)
  {
#if !defined(HOST_EMULATION) && !defined(SOC_TDA54)
    static volatile uint32_t *cnt0[APP_PERF_NUM_DDR_INSTANCES];
    static volatile uint32_t *cnt1[APP_PERF_NUM_DDR_INSTANCES];
    static volatile uint32_t *cnt2[APP_PERF_NUM_DDR_INSTANCES];
    static volatile uint32_t *cnt3[APP_PERF_NUM_DDR_INSTANCES];

    static volatile uint32_t last_cnt0 = 0, last_cnt1 = 0, last_cnt2 = 0, last_cnt3 = 0;
    volatile uint32_t cur_cnt0 = 0, cur_cnt1 = 0, cur_cnt2 = 0, cur_cnt3 = 0;
    uint32_t diff_cnt0, diff_cnt1, diff_cnt2, diff_cnt3, ddr_inst;

#if defined(SOC_AM62A) || defined(SOC_J722S)
    cnt0[0] = (volatile uint32_t *)0x0f300104;
    cnt1[0] = (volatile uint32_t *)0x0f300108;
    cnt2[0] = (volatile uint32_t *)0x0f30010C;
    cnt3[0] = (volatile uint32_t *)0x0f300110;
#else
    cnt0[0] = (volatile uint32_t *)0x02980104;
    cnt1[0] = (volatile uint32_t *)0x02980108;
    cnt2[0] = (volatile uint32_t *)0x0298010C;
    cnt3[0] = (volatile uint32_t *)0x02980110;
#endif

#if defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)
    cnt0[1] = (volatile uint32_t *)0x029A0104;
    cnt1[1] = (volatile uint32_t *)0x029A0108;
    cnt2[1] = (volatile uint32_t *)0x029A010C;
    cnt3[1] = (volatile uint32_t *)0x029A0110;
#endif

#if defined(SOC_J784S4)
    cnt0[2] = (volatile uint32_t *)0x029C0104;
    cnt1[2] = (volatile uint32_t *)0x029C0108;
    cnt2[2] = (volatile uint32_t *)0x029C010C;
    cnt3[2] = (volatile uint32_t *)0x029C0110;

    cnt0[3] = (volatile uint32_t *)0x029E0104;
    cnt1[3] = (volatile uint32_t *)0x029E0108;
    cnt2[3] = (volatile uint32_t *)0x029E010C;
    cnt3[3] = (volatile uint32_t *)0x029E0110;
#endif

    for (ddr_inst = 0; ddr_inst < APP_PERF_NUM_DDR_INSTANCES; ddr_inst++)
    {
      cur_cnt0 += *cnt0[ddr_inst];
      cur_cnt1 += *cnt1[ddr_inst];
      cur_cnt2 += *cnt2[ddr_inst];
      cur_cnt3 += *cnt3[ddr_inst];
    }
    if (cur_cnt0 < last_cnt0)
    {
      /* wrap around case */
      diff_cnt0 = (0xFFFFFFFFu - last_cnt0) + cur_cnt0;
    }
    else
    {
      diff_cnt0 = cur_cnt0 - last_cnt0;
    }
    if (cur_cnt1 < last_cnt1)
    {
      /* wrap around case */
      diff_cnt1 = (0xFFFFFFFFu - last_cnt1) + cur_cnt1;
    }
    else
    {
      diff_cnt1 = cur_cnt1 - last_cnt1;
    }

    if (cur_cnt2 < last_cnt2)
    {
      /* wrap around case */
      diff_cnt2 = (0xFFFFFFFFu - last_cnt2) + cur_cnt2;
    }
    else
    {
      diff_cnt2 = cur_cnt2 - last_cnt2;
    }

    if (cur_cnt3 < last_cnt3)
    {
      /* wrap around case */
      diff_cnt3 = (0xFFFFFFFFu - last_cnt3) + cur_cnt3;
    }
    else
    {
      diff_cnt3 = cur_cnt3 - last_cnt3;
    }

    last_cnt0 = cur_cnt0;
    last_cnt1 = cur_cnt1;
    last_cnt2 = cur_cnt2;
    last_cnt3 = cur_cnt3;

    if (raw)
    {
      *val0 = (uint32_t)cur_cnt0;
      *val1 = (uint32_t)cur_cnt1;
      *val2 = (uint32_t)cur_cnt2;
      *val3 = (uint32_t)cur_cnt3;
    }
    else
    {
      *val0 = (uint32_t)diff_cnt0;
      *val1 = (uint32_t)diff_cnt1;
      *val2 = (uint32_t)diff_cnt2;
      *val3 = (uint32_t)diff_cnt3;
    }
#endif //HOST_EMULATION
  }
  return;
}

/**
 * @brief Template function for MMAv2 scale/shift approximation with runtime type detection
 * 
 * This template function computes optimal fixed-point scale and shift parameters to approximate
 * a floating-point scale ratio using the formula: scaleRatio ≈ scale / 2^shift
 * 
 * 
 * Algorithm:
 * - Performs exhaustive search over all possible scale values [1, maxScaleValue]
 * - For each scale value, computes optimal shift: shift = round(log2(scale/scaleRatio))
 * - Evaluates approximation error: error = |scaleRatio - (scale/2^shift)|
 * - Returns scale/shift pair with minimum error
 * 
 * @tparam Tscale Template parameter for scale type. Supported types:
 *                - uint8_t:  scale range [1, 255]
 *                - int8_t:   scale range [1, 127]
 *                - uint16_t: scale range [1, 65535]
 *                - int16_t:  scale range [1, 32767]
 * 
 * @param[in]  scaleRatio     Target floating-point ratio to approximate
 * @param[out] scale          Pointer to store optimal scale value [1, maxScaleValue]
 * @param[out] shift          Pointer to store optimal shift value [0, maxShiftBits]
 * @param[in]  weightBits     Number of weight bits (affects maxShiftBits: 40 for ≤8 bits, 64 for >8 bits)
 * @param[in]  maxScaleMMAv2  Maximum scale for MMAv2 (currently unused, reserved for future use)
 * @param[in]  skipCheck      If true, skip scale values where scaleRatio > approximation
 *                            (avoids weight scale expansion in quantization)
 * 
 * @return Minimum approximation error achieved: |scaleRatio - (scale/2^shift)|
 * 
 */
template<typename Tscale>
float32_tidl TIDL_getMMAv2_ScaleShiftAndError(
    float32_tidl scaleRatio,
    Tscale *scale,
    uint8_t *shift,
    int32_t weightBits,
    float32_tidl maxScaleMMAv2,
    bool skipCheck)
{
    int32_t shiftBits;
    float32_tidl curError = 0;
    float32_tidl approxScale = 0;
    float32_tidl minError = FLT_MAX;
    int32_t bestShiftBits = 0;
    int32_t bestFixedScale = 1;
    int32_t maxShiftBits = (weightBits > 8) ? 64 : 40;
    int32_t skipCount = 0;
    
    /* Determine scale limits based on type using typeid (runtime check) */
    int32_t maxScaleValue = 255; /* Default initialization for MISRA C compliance */
    const char *scaleTypeName;
    
    if (typeid(Tscale) == typeid(int8_t))
    {
        maxScaleValue = 127;
        scaleTypeName = "INT8";
    }
    else 
    /* if (typeid(Tscale) == typeid(int16_t)) */
    {
        maxScaleValue = 32767;
        scaleTypeName = "INT16";
    }
    
    /* Exhaustive search through all possible scale values */
    for(int32_t scaleIter = 1; scaleIter <= maxScaleValue; scaleIter++)
    {
        /* Calculate optimal shift for this scale value */
        shiftBits = (int32_t)round(log(((float64_tidl)scaleIter)/scaleRatio)/log((float64_tidl)2));
        
        /* Clamp shift to valid range */
        if(shiftBits > maxShiftBits)
        {
          shiftBits = maxShiftBits;
        }
        if(shiftBits < 0)
        {
          shiftBits = 0;
        }
        
        /* Compute approximation */
        approxScale = (float32_tidl)(((float32_tidl)scaleIter)/(pow(2,shiftBits)));
        
        /* Skip check: avoid values that would expand weight scales */
        if(scaleRatio > approxScale && skipCheck==true)
        {
            skipCount++;
            continue;
        }
        
        /* Calculate error */
        curError = fabsf(scaleRatio - approxScale);
        
        /* Track minimum error */
        if(curError < minError)
        {
            minError = curError;
            bestShiftBits = shiftBits;
            bestFixedScale = scaleIter;
        }
    }
    /* Handle edge case: all values skipped */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    <justification end> */
    if(skipCount == maxScaleValue)
    {
        bestShiftBits = maxShiftBits;
        bestFixedScale = maxScaleValue;
        minError = fabsf(scaleRatio - ((float32_tidl)bestFixedScale/(pow(2,bestShiftBits))));
        tidl_printf(0, "Warning: Could not find optimal %s scale approximation for ratio %f, using approx scale %f\n",
                    scaleTypeName, scaleRatio, ((float32_tidl)bestFixedScale/(pow(2,bestShiftBits))));
    }
    /* LDRA_JUSTIFY_END */
    *shift = bestShiftBits;
    *scale = (Tscale)bestFixedScale;
    return minError;
}

int32_t tidl_getWriteLevel(void)
{
  return tidlWriteLevel;
}
void tidl_setWriteLevel(int32_t val)
{
  tidlWriteLevel = val;
  return;
}
int32_t tidl_getTraceLogLevel(void)
{
  return tidlLogLevel;
}

tidlWriteBinToFile_t tidl_getWriteBinToFile(void)
{
  return tidlWriteBinToFile;
}

tidlVprintf_t tidl_getVprintf(void)
{
  return tidlVprintf;
}
#ifdef HOST_EMULATION
tidlWriteBinToFile_t tidl_getWriteBinToFileStats(void)
{
  return tidlWriteBinToFile;
}
#endif
tidlReadBinFromFile_t tidl_getReadBinFromFile(void)
{
  return tidlReadBinFromFile;
}

void *tidl_getTraceBaseName(void)
{
  return tidlTraceBaseName;
}

/**
 * @brief Function to initilize the debug parameters
 *
 * @param traceLogLevel : debug  level for trace log
 * @param traceWriteLevel : debug level for trace write
 * @param tifVprintfFuncPtr : Pointer to print buffer
 * @param writeBinFuncPtr : Pointer to write bins
 * @param readBinFuncPtr : Pointer to the read bins
 * @param traceBaseName : Name of the trace buffer
 * @return int32_t : Returns SUCCESS or FAIL
 */

int32_t TIDL_initDebugTraceParams(int32_t traceLogLevel, int32_t traceWriteLevel, tidlVprintf_t tifVprintfFuncPtr, tidlWriteBinToFile_t writeBinFuncPtr,
                                  tidlReadBinFromFile_t readBinFuncPtr, const char *traceBaseName)
{
  int32_t status = TIDL_SUCCESS;

  if (((traceLogLevel > 0) && (tifVprintfFuncPtr == NULL)) ||
      ((traceWriteLevel > 0) && (writeBinFuncPtr == NULL)))
  {
    status = TIDL_ERR_FAILURE;
  }

  if (status == TIDL_SUCCESS)
  {
    tidlWriteBinToFile = (tidlWriteBinToFile_t)writeBinFuncPtr;
    tidlReadBinFromFile = (tidlReadBinFromFile_t)readBinFuncPtr;
    tidlVprintf = (tidlVprintf_t)tifVprintfFuncPtr;
    tidlLogLevel = traceLogLevel;
    tidlWriteLevel = traceWriteLevel;
    strncpy(tidlTraceBaseName, (char *)traceBaseName, TIDL_STRING_SIZE);
  }
  return status;
}

OPENACC(routine(TIDL_roundSat))
int64_t TIDL_roundSat(int64_t val, int32_t bits, int32_t min, int32_t max)
{
  uint32_t temp;
  int64_t temp_val;
  temp_val = val;
  if (bits > 0)
  {
    temp = ((uint32_t)1 << ((uint32_t)bits - 1U));
    temp_val += (int64_t)temp;

    /* CHECK_MISRA("-12.7")  -> Disable rule 12.7 */
    temp_val >>= (uint64_t)bits;
    /*RESET_MISRA("12.7")  -> Reset rule 12.7 */
  }
  temp_val = (temp_val <= min) ? min : temp_val;
  temp_val = (temp_val >= max) ? max : temp_val;
  return temp_val;
}
#ifndef HOST_EMULATION
/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> int64_t TIDL_roundSatMMA.* <function end>
<justification start> NOT_IN_SCOPE : This function is specific to host emulation (HE) and is intended to be used exclusively in HE builds. It is not applicable or invoked in non-HE environments.
<justification end> */
#endif
OPENACC(routine(TIDL_roundSatMMA))
int64_t TIDL_roundSatMMA(int64_t val, int32_t bits, int32_t min, int32_t max)
{
  int64_t temp;
  int64_t temp_val;
  temp_val = val;
#if BUILD_WITH_OPENACC
  if (bits > 0)
  {
    OPENACC(routine(TIDL_openaccShiftRight))
    temp = TIDL_openaccShiftRight(val, (bits - 1)) + 1;
    temp_val = TIDL_openaccShiftRight(temp, 1);
  }
#else
  if (bits > 0)
  {
    temp = __shift_right(val, (bits - 1)) + (int64_t)1;
    temp_val = __shift_right(temp, 1);
  }
#endif

  temp_val = (temp_val <= min) ? min : temp_val;
  temp_val = (temp_val >= max) ? max : temp_val;
  return temp_val;
}

/**
 * @brief To find min and max in the tensor
 *
 * @param ptr : pointer to tensor data
 * @param dataPrms : parametes of the data buffer
 * @param min : to store the min value
 * @param max : to store the max value
 */
template<class Tsrc, class TminMax> void TIDL_TensorMinMax(const Tsrc *ptr, const sTIDL_DataParams_t *dataPrms, int32_t numBatches, TminMax *min, TminMax *max)
{
  int32_t padOffset = dataPrms->padW + (dataPrms->padH * dataPrms->pitch[TIDL_LINE_PITCH]);
  int32_t i0, i1, i2, i3, i4, i5;
  TminMax val;

  for (i0 = 0; i0 < numBatches; i0++)
  {
    for (i1 = 0; i1 < dataPrms->dimValues[TIDL_DIM_DIM1]; i1++)
    {
      for (i2 = 0; i2 < dataPrms->dimValues[TIDL_DIM_DIM2]; i2++)
      {
        for (i3 = 0; i3 < dataPrms->dimValues[TIDL_DIM_NUMCH]; i3++)
        {
          for (i4 = 0; i4 < dataPrms->dimValues[TIDL_DIM_HEIGHT]; i4++)
          {
            for (i5 = 0; i5 < dataPrms->dimValues[TIDL_DIM_WIDTH]; i5++)
            {
              val = (TminMax)ptr[padOffset + (i0 * dataPrms->pitch[TIDL_ROI_PITCH]) + (i1 * dataPrms->pitch[TIDL_DIM_DIM1]) + (i2 * dataPrms->pitch[TIDL_DIM_DIM2]) + (i3 * dataPrms->pitch[TIDL_DIM_NUMCH]) + (i4 * dataPrms->pitch[TIDL_DIM_HEIGHT]) + i5];
              *min = (val < *min) ? val : *min;
              *max = (val > *max) ? val : *max;
            }
          }
        }
      }
    }
  }
}

#if defined TIDL_COVERAGE_DEAD_CODE
/**
 * @brief  Get equivalent kernel data type based on TIDL side data type
 *
 * @param tidlElemType  : TIDL side element type
 */
int32_t TIDL_getKernelDataType(int32_t tidlElemType)
{
  int32_t kernelDataType = 0;

  if (tidlElemType == (int32_t)TIDL_SignedChar)
  {
    kernelDataType = TIDL_INT8;
  }
  else if (tidlElemType == (int32_t)TIDL_UnsignedChar)
  {
    kernelDataType = TIDL_UINT8;
  }
  else if (tidlElemType == (int32_t)TIDL_UnsignedShort)
  {
    kernelDataType = (uint32_t)TIDL_UINT16;
  }
  else if (tidlElemType == (int32_t)TIDL_SignedShort)
  {
    kernelDataType = (uint32_t)TIDL_INT16;
  }
  else if (tidlElemType == (int32_t)TIDL_SignedWord)
  {
    kernelDataType = (uint32_t)TIDL_INT32;
  }
  else if (tidlElemType == (int32_t)TIDL_UnsignedWord)
  {
    kernelDataType = (uint32_t)TIDL_UINT32;
  }
  else if (tidlElemType == (int32_t)TIDL_SignedDoubleWord)
  {
    kernelDataType = (uint32_t)TIDL_INT64;
  }
  else if (tidlElemType == (int32_t)TIDL_UnsignedDoubleWord)
  {
    kernelDataType = (uint32_t)TIDL_UINT64;
  }
  else if (tidlElemType == (int32_t)TIDL_SinglePrecFloat)
  {
    kernelDataType = (uint32_t)TIDL_FLOAT32;
  }
  else
  {
    /*do nothing*/
  }

  return kernelDataType;
}
#endif

/**
 * @brief This function is to update scale factors
 *
 * @param intAlgHandle : tidl algorithm instance handle
 * @param i : Current layer index
 * @param updateStats : flag to update stats
 * @param accMin : Min value in the accumlator buffer
 * @param accMax  : Max value in the accumlator buffer
 */
#ifndef HOST_EMULATION
/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> void TIDL_UpdateScaleFactors.* <function end>
<justification start> NOT_IN_SCOPE : This function is specific to host emulation (HE) and is intended to be used exclusively in HE builds. It is not applicable or invoked in non-HE environments.
<justification end> */
#endif
void TIDL_UpdateScaleFactors(TIDL_Handle intAlgHandle, int32_t i, int32_t updateStats, int64_t accMin, int64_t accMax)
{
  int32_t quantizationStyle = intAlgHandle->createParams->net->quantizationStyle;
  if (TIDL_QuantStyleAsymNP2_TFL != quantizationStyle) // Unnecessary
  {
    sTIDL_Network_t *net = intAlgHandle->createParams->net;
    int32_t elementSizeBytes = TIDL_getDatElementSize(net->TIDLLayers[i].outData.elementType);
    /* For float we don't have to call update anything for stats collection */
    if (elementSizeBytes == 4)
    {
      net->TIDLLayers[i].outData.roundBits = 0;
      net->TIDLLayers[i].outData.tensorScale = 1.0f;
    }
  }
  return;
}

// J7AM Code:

// This implementation is temporary : Future versions won't need this sort of reconstruction (Direct 32/64 - bit bias consumption w/o any sort of overhead)
/**
 * @brief The function does the bias split for the conv parameters
 *
 * @param srcPtr : Pointer to the source buffer
 * @param dstPtr : Pointer to the destination buffer
 * @param biasB : Pointer to the bias buffer
 * @param dataSize : no of output channels
 * @param inScaleFactor : scale factor for the input
 * @param satLow : Min value of the saturation
 * @param satHigh : Max value of the saturation
 * @param biasBMax : the max value of the bias values
 * @param inFeatSign : the sign of the input feature map
 */
template<class Tdst, class Tsrc> void TIDL_AM_conv2dBiasSplit(Tsrc *srcPtr, Tdst *dstPtr, int32_t *biasB, int32_t dataSize, float32_tidl inScaleFactor, int32_t satLow, int32_t satHigh, int32_t biasBMax, int32_t inFeatSign)
{

  TIDL_conv2dBiasSplit(srcPtr, dstPtr, biasB, dataSize, inScaleFactor, satLow, satHigh, biasBMax, inFeatSign);

  // AM Specific bias restoration:
  // printf("inScaleFactor = %d\n",inScaleFactor);
  for (int32_t idx = 0; idx < dataSize; idx++)
  {
    // tidl_printf(2,"Bias B= %d && dstPtr[%d] = %d && srcPtr[%d] = %d\n",*biasB,idx,dstPtr[idx],idx,srcPtr[idx]);
    dstPtr[idx] *= (*(biasB));
  }

  return;
}

#if defined TIDL_COVERAGE_DEAD_CODE
//
/**
 * @brief The function does the bias split for the conv parameters
 *
 * @param srcPtr : Pointer to the source buffer
 * @param dstPtr : Pointer to the destination buffer
 * @param biasB : Pointer to the bias buffer
 * @param dataSize : no of output channels
 * @param inScaleFactor : scale factor for the input
 * @param satLow : Min value of the saturation
 * @param satHigh : Max value of the saturation
 * @param biasBMax : the max value of the bias values
 * @param inFeatSign : the sign of the input feature map
 */
template<class Tdst, class Tsrc> void TIDL_conv2dBiasSplitWithFixedBiasB(Tsrc *srcPtr,
                                                                         Tdst *dstPtr,
                                                                         int32_t *biasB,
                                                                         int32_t dataSize,
                                                                         float32_tidl inScaleFactor,
                                                                         int32_t satLow,
                                                                         int32_t satHigh,
                                                                         int32_t biasBMax,
                                                                         int32_t inFeatSign)
{
  if (inScaleFactor == 0)
  {
    for (int32_t idx = 0; idx < dataSize; idx++)
    {
      dstPtr[idx] = 0;
    }
  }
  else
  {
    int32_t orgBiasVal, biasVal = 0;
    if (inFeatSign == 1)
    {
      biasBMax = biasBMax / 2;
    }

    int32_t min, max, temp;
    if (inFeatSign == 1)
    {
      max = biasBMax;
    }
    else
    {
      max = biasBMax / 2;
    }
    temp = max + 1;
    min = -1 * temp;

    for (int32_t idx = 0; idx < dataSize; idx++)
    {
      orgBiasVal = srcPtr[idx] * inScaleFactor;
      biasVal = (orgBiasVal < satLow) ? satLow : orgBiasVal;
      biasVal = (biasVal > satHigh) ? satHigh : biasVal;
      temp = (biasVal / (*biasB));
      temp = (temp < min) ? min : temp;
      temp = (temp > max) ? max : temp;
      dstPtr[idx] = temp;
    }
  }
  return;
}
#endif

/**
 * @brief The function does the bias split for the conv parameters
 *
 * @param srcPtr : Pointer to the source buffer
 * @param dstPtr : Pointer to the destination buffer
 * @param biasB : Pointer to the bias buffer
 * @param dataSize : no of output channels
 * @param inScaleFactor : scale factor for the input
 * @param satLow : Min value of the saturation
 * @param satHigh : Max value of the saturation
 * @param biasBMax : the max value of the bias values
 * @param inFeatSign : the sign of the input feature map
 */
template<class Tdst, class Tsrc> void TIDL_conv2dBiasSplit(Tsrc *srcPtr, Tdst *dstPtr, int32_t *biasB, int32_t dataSize, float32_tidl inScaleFactor, int32_t satLow, int32_t satHigh, int32_t biasBMax, int32_t inFeatSign)
{
  if (inScaleFactor == 0.0)
  {
    for (int32_t idx = 0; idx < dataSize; idx++)
    {
      dstPtr[idx] = 0;
    }
    *biasB = (int32_t)0;
  }
  else
  {
#if USE_16BIT_BIAS_FOR_8BIT_MODE
    int64_t orgBiasVal, biasVal = 0;
    int64_t biasBMaxNew = biasBMax;

    if (inFeatSign == 1)
    {
      biasBMaxNew = (int64_t)biasBMax / 2;
    }
    if (inScaleFactor > (float32_tidl)biasBMaxNew)
    {
      int64_t temp;
      int64_t biasAMin, biasAMax;

      biasAMax = (int64_t)biasBMax / 2;

      temp = biasAMax + 1;
      biasAMin = -1 * temp;
      /* LDRA_JUSTIFY_START
      <metric start> statement branch <metric end>
      <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
      <justification end> */
      if ((biasBMax == 0xFF) && (satHigh == TIDL_SAT_HI_INT32))
      {
        /* Not the cleanest way to handle but to avoid function signature
        change doing it this way. This condition indicates signed input with
        16 bit processing and hence biasB is limited to 0xFF. In this case
        biasA can still be 16 bits*/
        biasAMin = std::numeric_limits<int16_t>::lowest();
        biasAMax = std::numeric_limits<int16_t>::max();
      }
      /* LDRA_JUSTIFY_END */

      for (int32_t idx = 0; idx < dataSize; idx++)
      {
        orgBiasVal = srcPtr[idx] * inScaleFactor;
        biasVal = (orgBiasVal < satLow) ? satLow : orgBiasVal;
        biasVal = (biasVal > satHigh) ? satHigh : biasVal;
        temp = (biasVal / biasBMaxNew);
        temp = (temp < biasAMin) ? biasAMin : temp;
        temp = (temp > biasAMax) ? biasAMax : temp;
        dstPtr[idx] = temp;
      }
      *biasB = biasBMaxNew;
    }
    else if (inScaleFactor > 1.0)
    {
      for (int32_t idx = 0; idx < dataSize; idx++)
      {
        dstPtr[idx] = srcPtr[idx];
      }
      *biasB = inScaleFactor;
    }
    else
    {
      for (int32_t idx = 0; idx < dataSize; idx++)
      {
        dstPtr[idx] = srcPtr[idx] * inScaleFactor;
      }
      *biasB = (int32_t)1;
    }

#else
    int64_t biasVal = 0;
    int64_t orgBiasVal = 0;
    for (int32_t idx = 0; idx < dataSize; idx++)
    {
      orgBiasVal = srcPtr[idx] * inScaleFactor;
      biasVal = orgBiasVal < satLow ? satLow : orgBiasVal;
      biasVal = biasVal > satHigh ? satHigh : biasVal;
      dstPtr[idx] = biasVal;
      if (orgBiasVal != biasVal)
      {
        tidl_printf(0, "Conv2D bias is greater than 16 bits\n");
      }
    }
    *biasB = (int32_t)1;
#endif
  }
  return;
}

/**
 * @brief Function is to prepare transfer prooperties
 *
 * @param trMem  : pointer to trasfer memory
 */
int32_t TIDL_prePareMemcpyTr(
    void *trMem)
{
  int32_t retVal = IALG_EOK;
  DmaUtilsAutoInc3d_TrPrepareParam trPrepParams;
  DmaUtilsAutoInc3d_TransferProp transferProp;
  /* LDRA_JUSTIFY_START
  <metric start> branch <metric end>
  <justification start> PRIOR_CHECK : Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
  This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
  TIDL_LDRA_TAG : TIDL_LDRA_TAG_COMMONUTILS_PRIOR_CHECK_003
  <justification end> */
  if (trMem != NULL)
  {
    /* LDRA_JUSTIFY_END */
    trPrepParams.channelId = TIDL_DMA_CHANNEL_MEMCPY;
    trPrepParams.numTRs = (int32_t)1;
    trPrepParams.trMem = (uint8_t *)trMem;
    trPrepParams.trMemSize = (int32_t)64;

    transferProp.syncType = (uint32_t)DMAUTILSAUTOINC3D_SYNC_2D;
    transferProp.dmaDfmt = (uint32_t)DMAUTILSAUTOINC3D_DFMT_NONE;
    transferProp.circProp.circDir = (uint8_t)DMAUTILSAUTOINC3D_CIRCDIR_DST;
    transferProp.circProp.circSize1 = 0;
    transferProp.circProp.circSize2 = 0;
    transferProp.circProp.addrModeIcnt0 = (uint8_t)DMAUTILSAUTOINC3D_ADDR_LINEAR;
    transferProp.circProp.addrModeIcnt1 = (uint8_t)DMAUTILSAUTOINC3D_ADDR_LINEAR;
    transferProp.circProp.addrModeIcnt2 = (uint8_t)DMAUTILSAUTOINC3D_ADDR_LINEAR;
    transferProp.circProp.addrModeIcnt3 = (uint8_t)DMAUTILSAUTOINC3D_ADDR_LINEAR;

    transferProp.transferDim.sicnt0 = 1;
    transferProp.transferDim.sicnt1 = 1;
    transferProp.transferDim.sicnt2 = 1;
    transferProp.transferDim.sicnt3 = 1;
    transferProp.transferDim.sdim1 = 1;

    transferProp.transferDim.dicnt0 = 1;
    transferProp.transferDim.dicnt1 = 1;
    transferProp.transferDim.dicnt2 = 1;
    transferProp.transferDim.dicnt3 = 1;
    transferProp.transferDim.ddim1 = 1;

    transferProp.ioPointers.srcPtr = (uint8_t *)NULL;
    transferProp.ioPointers.dstPtr = (uint8_t *)NULL;

    retVal = DmaUtilsAutoInc3d_prepareTr(&trPrepParams, &transferProp);
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> PRIOR_CHECK : Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
     This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
     TIDL_LDRA_TAG : TIDL_LDRA_TAG_COMMONUTILS_PRIOR_CHECK_001
    <justification end> */
    if (retVal != (int32_t)DMAUTILS_SOK)
    {
      tidl_printf(0, "DmaUtilsAutoInc3d_prepareTr returned Error code");
      retVal = TIDL_ERROR_COMMNUTILS_FAIL_DMAPREPTR_ERROR;
    }
    /* LDRA_JUSTIFY_END */
  }
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start> PRIOR_CHECK : Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
  This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
  TIDL_LDRA_TAG : TIDL_LDRA_TAG_COMMONUTILS_PRIOR_CHECK_003
  <justification end> */
  else
  {
    tidl_printf(0, "DmaUtilsAutoInc3d_prepareTr returned Error code");
    retVal = IALG_EFAIL;
  }
  /* LDRA_JUSTIFY_END */
  return retVal;
}

/*
 * Async mode of transfer when Autoincrement TR is not possible
 * and src and dst needs to be accessed in 2D pattern
 *
 * This API uses the same channel as TIDL_memcpy2D which is
 * a blocking call and therefore the following sequence should
 * be avoided at all costs
 *
 * TIDL_memcpy2DAsyncTrigger();
 * TIDL_memcpy2D();
 * TIDL_memcpy2DAsyncWait();
 *
 * TODO: Fix this in a nice way
 */

/**
 * @brief Memory copy for trigger
 *
 * @param dstPtr : Pointer to dst memory
 * @param srcPtr : Pointer to src memory
 * @param width : Width of the memory copy
 * @param height : Height of the memory copy
 * @param dstStride : Stride of the dst buffer
 * @param srcStride : Stride of the src buffer
 * @param dmaUtilsContext : memory for dmaUtilsContext
 * @param trMem : pointer to transfer memory
 */
int32_t TIDL_memcpy2D(
    const void *dstPtr,
    const void *srcPtr,
    uint32_t width,
    uint16_t height,
    int32_t dstStride,
    int32_t srcStride,
    void *dmaUtilsContext,
    uint8_t *trMem)
{
  int32_t retVal = IALG_EOK;
  uint32_t temp_width = width;
  uint32_t completeTransWidth = 0;
  if (dmaUtilsContext != NULL && trMem != NULL)
  {
    // if((trMem != NULL) && (srcPtr != NULL) && (dstPtr != NULL))
    {
      /* TIDL_LDRA_TAG_COMMONUTILS_PRIOR_CHECK_001 & TIDL_LDRA_TAG_COMMONUTILS_PRIOR_CHECK_002 && TIDL_LDRA_TAG_COMMONUTILS_PRIOR_CHECK_003 */
      DmaUtilsAutoInc3d_TrPrepareParam trPrepParam;
#ifdef USE_STACK_MEM_TR
      uint8_t trMemStack[64];
      (void)TIDL_prePareMemcpyTr(trMemStack);
      trMem = trMemStack;
#endif
      uint32_t convertMask = (uint32_t)DMAUTILSAUTOINC3D_ADDRCONVERTMASK_SRCADDR |
                             (uint32_t)DMAUTILSAUTOINC3D_ADDRCONVERTMASK_DSTADDR;
      trPrepParam.channelId = TIDL_DMA_CHANNEL_MEMCPY;
      trPrepParam.numTRs = 1;
      trPrepParam.trMemSize = 64;
      trPrepParam.trMem = trMem;

      CSL_UdmapTR *tr;
      tr = (CSL_UdmapTR *)trMem;
      tr->addr = (uintptr_t)srcPtr;
      tr->daddr = (uintptr_t)dstPtr;
      // tr->icnt0  = width;
      tr->icnt1 = height;
      // tr->dicnt0 = width;
      tr->dicnt1 = height;
      tr->dim1 = srcStride;
      tr->ddim1 = dstStride;

      while ((temp_width > 0U) && (retVal == IALG_EOK))
      {
        uint32_t transWidth = (MIN(((1U << 16U) - 1U), temp_width));
        tr->icnt0 = (uint16_t)transWidth;
        tr->dicnt0 = (uint16_t)transWidth;
        tr->addr = (uintptr_t)srcPtr + (uintptr_t)completeTransWidth;
        tr->daddr = (uintptr_t)dstPtr + (uintptr_t)completeTransWidth;

        temp_width = temp_width - ((uint32_t)transWidth);
        completeTransWidth += transWidth;

        retVal = DmaUtilsAutoInc3d_convertTrVirtToPhyAddr(dmaUtilsContext, &trPrepParam, convertMask);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> PRIOR_CHECK : Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
        This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
        TIDL_LDRA_TAG : TIDL_LDRA_TAG_COMMONUTILS_PRIOR_CHECK_001
        <justification end> */
        if (retVal != (int32_t)DMAUTILS_SOK)
        {
          tidl_printf(0, "DMA utils function DmaUtilsAutoInc3d_convertTrVirtToPhyAddr returned Error Code");
          retVal = TIDL_ERROR_COMMNUTILS_FAIL_DMAVRPHYADD_ERROR;
        }
        /* LDRA_JUSTIFY_END */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> PRIOR_CHECK : Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
        This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
        TIDL_LDRA_TAG : TIDL_LDRA_TAG_COMMONUTILS_PRIOR_CHECK_001
        <justification end> */
        if (retVal != IALG_EFAIL)
        {
          /* LDRA_JUSTIFY_END */
          /* Re-use the same contexxt as feat for memcpy */
          retVal = DmaUtilsAutoInc3d_configure(dmaUtilsContext, TIDL_DMA_CHANNEL_MEMCPY, trMem, (int32_t)1);
          /* LDRA_JUSTIFY_START
          <metric start> statement branch <metric end>
          <justification start> PRIOR_CHECK : Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
          This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
          TIDL_LDRA_TAG : TIDL_LDRA_TAG_COMMONUTILS_PRIOR_CHECK_002
          <justification end> */
          if (retVal != (int32_t)DMAUTILS_SOK)
          {
            tidl_printf(0, "DMA utils function DmaUtilsAutoInc3d_configure returned Error Code");
            retVal = TIDL_ERROR_COMMNUTILS_FAIL_DMACONFIG_ERROR;
          }
          /* LDRA_JUSTIFY_END */
          /* LDRA_JUSTIFY_START
          <metric start> branch <metric end>
          <justification start> PRIOR_CHECK : Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
          This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
          TIDL_LDRA_TAG : TIDL_LDRA_TAG_COMMONUTILS_PRIOR_CHECK_002
          <justification end> */
          if (retVal != IALG_EFAIL)
          {
            /* LDRA_JUSTIFY_END */
            (void)DmaUtilsAutoInc3d_trigger(dmaUtilsContext, TIDL_DMA_CHANNEL_MEMCPY);
            #ifdef PERF_MODELLING
            DmaUtilsAutoInc3d_wait_copyWrapper(dmaUtilsContext, TIDL_DMA_CHANNEL_MEMCPY, TRUE);
            #endif
            DmaUtilsAutoInc3d_wait(dmaUtilsContext, TIDL_DMA_CHANNEL_MEMCPY);
          }
        }
      }
      /* LDRA_JUSTIFY_START
      <metric start> branch <metric end>
      <justification start> PRIOR_CHECK : Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
      This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
      TIDL_LDRA_TAG : TIDL_LDRA_TAG_COMMONUTILS_PRIOR_CHECK_002
      <justification end> */
      if (retVal != IALG_EFAIL)
      {
        /* LDRA_JUSTIFY_END */
        (void)DmaUtilsAutoInc3d_deconfigure(dmaUtilsContext, TIDL_DMA_CHANNEL_MEMCPY, trMem, 1);
      }
    }
    // else
    // {
    //  retVal = TIDL_ERROR_COMMNUTILS_FAIL_DMADECONFIG_ERROR;
    //}
  }
  else
  {
    uint32_t rowCnt;

    for (rowCnt = 0; rowCnt < height; rowCnt++)
    {
      (void)memcpy((uint8_t *)dstPtr + (rowCnt * (uint32_t)dstStride), (uint8_t *)srcPtr + (rowCnt * (uint32_t)srcStride), temp_width);
    }
  }

  return retVal;
}

template void TIDL_conv2dBiasSplit<int16_t, int16_t>(int16_t *srcPtr, int16_t *dstPtr, int32_t *biasB, int32_t dataSize, float32_tidl inScaleFactor, int32_t satLow, int32_t satHigh, int32_t biasBMax, int32_t inFeatSign);
template void TIDL_conv2dBiasSplit<int8_t, int16_t>(int16_t *srcPtr, int8_t *dstPtr, int32_t *biasB, int32_t dataSize, float32_tidl inScaleFactor, int32_t satLow, int32_t satHigh, int32_t biasBMax, int32_t inFeatSign);
template void TIDL_conv2dBiasSplit<int32_t, int16_t>(int16_t *srcPtr, int32_t *dstPtr, int32_t *biasB, int32_t dataSize, float32_tidl inScaleFactor, int32_t satLow, int32_t satHigh, int32_t biasBMax, int32_t inFeatSign);

template void TIDL_conv2dBiasSplit<int16_t, int32_t>(int32_t *srcPtr, int16_t *dstPtr, int32_t *biasB, int32_t dataSize, float32_tidl inScaleFactor, int32_t satLow, int32_t satHigh, int32_t biasBMax, int32_t inFeatSign);
template void TIDL_conv2dBiasSplit<int8_t, int32_t>(int32_t *srcPtr, int8_t *dstPtr, int32_t *biasB, int32_t dataSize, float32_tidl inScaleFactor, int32_t satLow, int32_t satHigh, int32_t biasBMax, int32_t inFeatSign);
template void TIDL_conv2dBiasSplit<int32_t, int32_t>(int32_t *srcPtr, int32_t *dstPtr, int32_t *biasB, int32_t dataSize, float32_tidl inScaleFactor, int32_t satLow, int32_t satHigh, int32_t biasBMax, int32_t inFeatSign);

// MMAv2
template void TIDL_AM_conv2dBiasSplit<int32_t, int16_t>(int16_t *srcPtr, int32_t *dstPtr, int32_t *biasB, int32_t dataSize, float32_tidl inScaleFactor, int32_t satLow, int32_t satHigh, int32_t biasBMax, int32_t inFeatSign);
template void TIDL_AM_conv2dBiasSplit<int64_t, int16_t>(int16_t *, int64_t *, int32_t *, int32_t, float32_tidl, int32_t, int32_t, int32_t, int32_t);

// Explicit template instantiations for TIDL_getMMAv2_ScaleShiftAndError
template float32_tidl TIDL_getMMAv2_ScaleShiftAndError<int8_t>(float32_tidl, int8_t*, uint8_t*, int32_t, float32_tidl, bool);
template float32_tidl TIDL_getMMAv2_ScaleShiftAndError<int16_t>(float32_tidl, int16_t*, uint8_t*, int32_t, float32_tidl, bool);

void tidl_printf(int8_t traceLevel, const char *format, ...)
{
  va_list args;

  if (traceLevel < tidl_getTraceLogLevel())
  {
    (void)va_start(args, format);
    (void)tidl_getVprintf()(format, args);
    va_end(args);
  }
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> float32_tidl \* tidl_convertParamsToFloat.* <function end>
<justification start> DEBUG_TRACE : This function is solely for debugging purposes and is not part of the production code.
<justification end> */
float32_tidl *tidl_convertParamsToFloat(void *buf,
                                        int32_t numElements,
                                        int32_t dataBits,
                                        float32_tidl dataScale,
                                        void *scratchPtr,
                                        int32_t scratchSize)
{
  float32_tidl *weightPtrFloat = NULL;

  if (((numElements * (int64_t)sizeof(float32_tidl)) <= scratchSize) &&
      (dataScale != 0.0))
  {
    int32_t i;
    weightPtrFloat = (float32_tidl *)scratchPtr;

    for (i = 0; i < numElements; i++)
    {
      if (dataBits <= 8)
      {
        int8_t *inPtr = (int8_t *)buf;
        weightPtrFloat[i] = (float32_tidl)inPtr[i] / dataScale;
      }
      else if (dataBits <= 16)
      {
        int16_t *inPtr = (int16_t *)buf;
        weightPtrFloat[i] = ((float32_tidl)inPtr[i]) / dataScale;
      }
      else if (dataBits == 32)
      {
        float32_tidl *inPtr = (float32_tidl *)buf;
        weightPtrFloat[i] = inPtr[i] / dataScale;
      }
      else
      {
        tidl_printf(0, "Invalid dataBits in tidl_convertParamsToFloat\n");
        weightPtrFloat = NULL;
        break;
      }
    }
  }

  return weightPtrFloat;
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> int32_t tidl_writeTraceParamBuf.* <function end>
<justification start> DEBUG_TRACE : This function is solely for debugging purposes and is not part of the production code.
<justification end> */
int32_t tidl_writeTraceParamBuf(sTIDL_Network_t *net,
                                sTIDL_AlgLayer_t *algLayer,
                                sTIDL_Layer_t *tidlLayer,
                                int32_t dataBuffId,
                                int8_t *orgScratchPtr,
                                uint32_t orgScratchSize)
{
  int32_t status = TIDL_SUCCESS;

  if (tidl_getWriteLevel() == 4)
  {
    if ((tidlLayer->layerType == TIDL_ConvolutionLayer) ||
        (tidlLayer->layerType == TIDL_Deconv2DLayer) ||
        (tidlLayer->layerType == TIDL_BatchNormLayer) ||
        (tidlLayer->layerType == TIDL_BatchReshapeLayer) ||
        (tidlLayer->layerType == TIDL_InnerProductLayer))
    {
      int8_t *scratchPtr = (int8_t *)(orgScratchPtr + TRACE_STRINGS_MEM_SIZE);
      int8_t *traceDumpName = (int8_t *)(orgScratchPtr);
      int8_t *traceStringName = (int8_t *)(orgScratchPtr + (1U * TRACE_STRING_SIZE));
      int8_t *DataIDString = (int8_t *)(orgScratchPtr + (2U * TRACE_STRING_SIZE));
      int32_t scratchSize = orgScratchSize - TRACE_STRINGS_MEM_SIZE;
      int32_t dataSize = 0;

      if (((char *)tidl_getTraceBaseName())[0] != '\0') // check for empty string instead of null ptr as ptr is statically declared and will never be null
      {
        (void)strcpy((char *)traceDumpName, (char *)tidl_getTraceBaseName());
      }
      else
      {
        (void)strcpy((char *)traceDumpName, (char *)"");
      }

      (void)sprintf((char *)DataIDString, "%04d", dataBuffId);
      (void)strcat((char *)traceDumpName, (char *)DataIDString);
      (void)strcat((char *)traceDumpName, "_");

      if ((tidlLayer->layerType == TIDL_ConvolutionLayer) || (tidlLayer->layerType == TIDL_Deconv2DLayer))
      {
        void *weightPtr = ((int8_t *)net + tidlLayer->layerParams.convParams.weights);
        float32_tidl *weightPtrFloat;

        dataSize = (tidlLayer->layerParams.convParams.numOutChannels *
                    tidlLayer->layerParams.convParams.numInChannels *
                    tidlLayer->layerParams.convParams.kernelW *
                    tidlLayer->layerParams.convParams.kernelH) /
                   tidlLayer->layerParams.convParams.numGroups;

        weightPtrFloat = tidl_convertParamsToFloat(weightPtr,
                                                   dataSize,
                                                   tidlLayer->weightsElementSizeInBits,
                                                   tidlLayer->layerParams.convParams.weightScale,
                                                   scratchPtr,
                                                   scratchSize);

        if (weightPtrFloat == NULL)
        {
          status = TIDL_ERR_FAILURE;
          tidl_printf(0, " In sufficient Memory to write Weight Traces\n");
        }
        if (status == TIDL_SUCCESS)
        {
          sprintf((char *)traceStringName, "%sweights_float.bin", traceDumpName);

          (void)tidl_getWriteBinToFile()((const char *)traceStringName, weightPtrFloat, dataSize * (int64_t)sizeof(float32_tidl), 1);

          if (tidlLayer->layerParams.convParams.enableBias != 0)
          {
            float32_tidl *biasPtrFloat;
            void *biasPtr = ((int8_t *)(net) + tidlLayer->layerParams.convParams.bias);
            dataSize = tidlLayer->layerParams.convParams.numOutChannels;

            //: TODO: Ideally this we should dump after bias split
            biasPtrFloat = tidl_convertParamsToFloat(biasPtr,
                                                     dataSize,
                                                     tidlLayer->weightsElementSizeInBits,
                                                     tidlLayer->layerParams.convParams.biasScale,
                                                     scratchPtr,
                                                     scratchSize);
            if (biasPtrFloat == NULL)
            {
              status = TIDL_ERR_FAILURE;
              tidl_printf(0, " In sufficient Memory to write Bias Traces\n");
            }
            if (status == TIDL_SUCCESS)
            {
              sprintf((char *)traceStringName, "%sbias_float.bin", traceDumpName);
              (void)tidl_getWriteBinToFile()((const char *)traceStringName, biasPtrFloat, (uint64_t)dataSize * sizeof(float32_tidl), 1);
            }
          }
        }
      }
    }
    if (status == TIDL_SUCCESS)
    {
      /** Wb/inv L1D/L2 cache afte writing traces*/
      TIDL_L1DandL2CacheWbInv();
    }
  }
  return status;
}

/**
 * @brief Function to write traces from data buffer
 *
 * @param ptr                : Pointer to the data buffer
 * @param net                : tidl network structure
 * @param dataBuffId         : id of the databuffer
 * @param orgScratchPtr      : pointer to the scratch buffer
 * @param orgScratchSize     : Size of the scratch buffer
 * @param startHeight        : Offset to the current line
 * @param startChannel       : Offset to the current channel
 * @param startBatch         : Offset to the current batch
 * @param height             : No of lines for writing
 * @param numChannelsToWrite : No of channels for writing
 * @param numBatchesToWrite  : Number of batches for writing
 * @param totalBatches       : Total number of batches in outData of curr layer
 * @param currChPitch        : current channel pitch
 *
 */
/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> int32_t tidl_writeTraceDataBuf.* <function end>
<justification start> DEBUG_TRACE : This function is solely for debugging purposes and is not part of the production code.
<justification end> */
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
                               int32_t numChannelsToWrite,
                               int32_t numBatchesToWrite,
                               int32_t totalBatches,
                               int32_t currChPitch,
                               int32_t coreId,
                               const char *traceBaseNameCreateParams,
                               int32_t isInputData)
{
  int32_t temp_currChPitch = currChPitch;
  int32_t temp_height = height;
  int32_t temp_numChannels = numChannelsToWrite;
  int32_t status = TIDL_SUCCESS;
  if (tidl_getWriteLevel() > 0)
  {
    int8_t *scratchPtr = (int8_t *)(orgScratchPtr + TRACE_STRINGS_MEM_SIZE);
    int8_t *traceDumpName = (int8_t *)(orgScratchPtr);
    int8_t *traceBaseName = (int8_t *)(orgScratchPtr + (1U * TRACE_STRING_SIZE));
    int8_t *DataIDString = (int8_t *)(orgScratchPtr + (2U * TRACE_STRING_SIZE));
    int32_t layerIdx;
    int32_t scratchSize = orgScratchSize - TRACE_STRINGS_MEM_SIZE;
    int32_t tidlWriteLevelOrig = tidl_getWriteLevel();
    sTIDL_DataParams_t *dataBuffParam;
    dataBuffParam = TIDL_getDataParams(net, dataBuffId);
    layerIdx = TIDL_getLayerNum(net, dataBuffId);

    if (algLayer->gcHelperHandle == NULL)
    {
      numBatchesToWrite = dataBuffParam->dimValues[TIDL_DIM_BATCH];
    }

    /* for trace Level 4 enable trace level 3 by default */
    if (tidl_getWriteLevel() == 4)
    {
      tidl_setWriteLevel(3);
    }
    if (dataBuffParam != NULL)
    {
      sTIDL_DataParams_t dataBuffParamOrig = *dataBuffParam;

      if (net->TIDLLayers[layerIdx].layerType == TIDL_DataConvertLayer)
      {
        int32_t inLayout = net->TIDLLayers[layerIdx].layerParams.dataConvertParams.layout, outLayout = net->TIDLLayers[layerIdx].layerParams.dataConvertParams.outLayout;

        if (((net->TIDLLayers[layerIdx].layerParams.dataConvertParams.layout == (int32_t)TIDL_LT_NHWC) && (net->TIDLLayers[layerIdx].layerParams.dataConvertParams.type == (int32_t)TIDL_DC_TYPE_OUTPUT)) ||
            ((net->TIDLLayers[layerIdx].layerParams.dataConvertParams.type == (int32_t)TIDL_DC_TYPE_INTERMEDIATE) && (inLayout != outLayout)))
        {
          dataBuffParam->dimValues[TIDL_DIM_WIDTH] = dataBuffParamOrig.dimValues[TIDL_DIM_NUMCH];
          dataBuffParam->dimValues[TIDL_DIM_HEIGHT] = dataBuffParamOrig.dimValues[TIDL_DIM_WIDTH];
          dataBuffParam->dimValues[TIDL_DIM_NUMCH] = dataBuffParamOrig.dimValues[TIDL_DIM_HEIGHT];
          dataBuffParam->dimValues[TIDL_DIM_DIM2] = dataBuffParamOrig.dimValues[TIDL_DIM_DIM2];
          dataBuffParam->dimValues[TIDL_DIM_DIM1] = dataBuffParamOrig.dimValues[TIDL_DIM_DIM1];
          dataBuffParam->dimValues[TIDL_DIM_BATCH] = dataBuffParamOrig.dimValues[TIDL_DIM_BATCH];

          dataBuffParam->pitch[TIDL_LINE_PITCH] = dataBuffParam->dimValues[TIDL_DIM_WIDTH];
          dataBuffParam->pitch[TIDL_CHANNEL_PITCH] = dataBuffParam->pitch[TIDL_LINE_PITCH] * dataBuffParam->dimValues[TIDL_DIM_HEIGHT];
          dataBuffParam->pitch[TIDL_DIM2_PITCH] = dataBuffParam->pitch[TIDL_CHANNEL_PITCH] * dataBuffParam->dimValues[TIDL_DIM_NUMCH];
          dataBuffParam->pitch[TIDL_DIM1_PITCH] = dataBuffParam->pitch[TIDL_DIM2_PITCH] * dataBuffParam->dimValues[TIDL_DIM_DIM2];
          dataBuffParam->pitch[TIDL_ROI_PITCH] = dataBuffParam->pitch[TIDL_DIM1_PITCH] * dataBuffParam->dimValues[TIDL_DIM_DIM1];
          //: TODO: This needs to be correctly handled for super tiling cases
          temp_height = dataBuffParam->dimValues[TIDL_DIM_HEIGHT];
          temp_numChannels = dataBuffParam->dimValues[TIDL_DIM_NUMCH];
          temp_currChPitch = dataBuffParam->pitch[TIDL_CHANNEL_PITCH];
        }
      }
      if (((dataBuffParam->numBatchW > 1) || (dataBuffParam->numBatchH > 1)) && (dataBuffParam->dimValues[TIDL_DIM_BATCH] == 1))
      {
        int32_t elementSizeBytes;
        elementSizeBytes = TIDL_getDatElementSize(dataBuffParam->elementType);
        if (elementSizeBytes == 4)
        {
          tidl_setWriteLevel((tidl_getWriteLevel() == 3) ? 1 : tidl_getWriteLevel());
        }

        int32_t dim1 = (dataBuffParam->dimValues[TIDL_DIM_DIM1]);
        int32_t dim2 = (dataBuffParam->dimValues[TIDL_DIM_DIM2]);
        int32_t outWidth = (dataBuffParam->dimValues[TIDL_DIM_WIDTH] - (dataBuffParam->batchPadW * (dataBuffParam->numBatchW - 1))) / dataBuffParam->numBatchW;
        int32_t outHeight = dataBuffParam->dimValues[TIDL_DIM_HEIGHT];
        int32_t outChannels = dataBuffParam->dimValues[TIDL_DIM_NUMCH];
        int32_t outNumBatches = dataBuffParam->numBatchW;
        int32_t dataSize = outWidth * outHeight * outChannels * outNumBatches;
        int32_t linePitch = dataBuffParam->pitch[TIDL_LINE_PITCH];
        int32_t chPitch = dataBuffParam->pitch[TIDL_CHANNEL_PITCH];
        int32_t dim1Pitch = dataBuffParam->pitch[TIDL_DIM1_PITCH];
        int32_t dim2Pitch = dataBuffParam->pitch[TIDL_DIM2_PITCH];
        int32_t batchPitch = dataBuffParam->pitch[TIDL_ROI_PITCH];
        int32_t inBlkPitchW = outWidth + dataBuffParam->batchPadW;
        int32_t paddeOutSize = linePitch * (dataBuffParam->dimValues[TIDL_DIM_HEIGHT] + (dataBuffParam->padH * 2) + 1);
        int32_t padedDataSize = dataBuffParam->dimValues[TIDL_DIM_BATCH] * dataBuffParam->dimValues[TIDL_DIM_NUMCH] * paddeOutSize;
        int8_t *outPtr = (int8_t *)scratchPtr;
        int8_t *outWithPadPtr = (int8_t *)(scratchPtr + (dataSize * elementSizeBytes));
        float32_tidl *floatPtr = (float32_tidl *)(scratchPtr + (dataSize * elementSizeBytes));
        int32_t i;
        int32_t j, k1, l, m;
        int32_t batchIdx;
        int32_t batchOffset;
        int32_t offset;
        int32_t totalSize = 0;

        if (tidl_getWriteLevel() == 1)
        {
          totalSize = (dataSize * elementSizeBytes);
        }
        else if (tidl_getWriteLevel() == 2)
        {
          totalSize = ((dataSize * elementSizeBytes) + (padedDataSize * elementSizeBytes));
        }
        else if (tidl_getWriteLevel() == 3)
        {
          totalSize = (dataSize * elementSizeBytes) + ((int32_t)sizeof(float32_tidl) * dataSize);
        }
        else
        {
          tidl_printf(0, "Un supported tidl_getWriteLevel() \n");
          status = TIDL_ERR_FAILURE;
        }

        if (dataBuffParam->elementType == TIDL_SinglePrecFloat)
        {
          float32_tidl curMin = FLT_MAX, curMax = -FLT_MAX;
          TIDL_TensorMinMax((float32_tidl *)ptr, dataBuffParam, dataBuffParam->dimValues[TIDL_DIM_BATCH], &curMin, &curMax);
          tidl_printf(0, " %3d %10.5f %10.5f %10.5f %d\n", dataBuffId, dataBuffParam->tensorScale, curMin, curMax, dataBuffParam->elementType);
        }
        else
        {
          tidl_printf(0, " %3d %10.5f %10.5f %10.5f %d\n", dataBuffId, dataBuffParam->tensorScale, dataBuffParam->minTensorValue, dataBuffParam->maxTensorValue, dataBuffParam->elementType);
        }

        if ((totalSize > scratchSize) && (status == TIDL_SUCCESS))
        {
          tidl_printf(0, " In sufficient Memory to write Traces, as requested size is : %d, allocated size : %d \n", totalSize, scratchSize);
          status = TIDL_ERR_FAILURE;
        }

        if (status == TIDL_SUCCESS)
        {
          if (traceBaseNameCreateParams != NULL)
          {
            (void)strcpy((char *)traceDumpName, (char *)traceBaseNameCreateParams);
          }
          else
          {
            (void)strcpy((char *)traceDumpName, (char *)"");
          }

          (void)sprintf((char *)DataIDString, "%s%04d_%04d_%04d_%05d_%05d_%05dx%05d",
                        traceDumpName, dataBuffId,
                        dataBuffParam->dimValues[TIDL_DIM_BATCH],
                        dataBuffParam->dimValues[TIDL_DIM_DIM1],
                        dataBuffParam->dimValues[TIDL_DIM_DIM2],
                        dataBuffParam->dimValues[TIDL_DIM_NUMCH],
                        dataBuffParam->dimValues[TIDL_DIM_WIDTH],
                        dataBuffParam->dimValues[TIDL_DIM_HEIGHT]);

          (void)strcpy((char *)traceBaseName, (char *)DataIDString);

          if ((startHeight != 0) || (startChannel != 0))
          {
            (void)strcpy((char *)traceDumpName, (char *)traceBaseName);
            (void)strcat((char *)traceDumpName, ".y");

            if (elementSizeBytes == 4)
            {
              (void)strcpy((char *)traceDumpName, (char *)traceBaseName);
              (void)strcat((char *)traceDumpName, "_float.bin");
            }

            (void)tidl_getReadBinFromFile()((const char *)traceDumpName, outPtr, dataSize * elementSizeBytes, 1);
            if (tidl_getWriteLevel() == 2)
            {
              (void)strcpy((char *)traceDumpName, (char *)traceBaseName);
              (void)strcat((char *)traceDumpName, "_paded.y");
              (void)tidl_getReadBinFromFile()((const char *)traceDumpName, outWithPadPtr, padedDataSize * elementSizeBytes, 1);
            }

            if (tidl_getWriteLevel() == 3)
            {
              (void)strcpy((char *)traceDumpName, (char *)traceBaseName);
              (void)strcat((char *)traceDumpName, "_float.bin");
              (void)tidl_getReadBinFromFile()((const char *)traceDumpName, floatPtr, dataSize * ((int32_t)sizeof(float32_tidl)), 1);
            }
          }

          for (batchIdx = 0; batchIdx < 1; batchIdx++)
          {
            batchOffset = dim1 * dim2 * outChannels * outWidth * outHeight;

            for (m = 0; m < dim1; m++)
            {
              for (l = 0; l < dim2; l++)
              {
                for (j = 0; j < temp_numChannels; j++)
                {
                  for (i = 0; i < temp_height; i++)
                  {
                    for (k1 = 0; k1 < outNumBatches; k1++)
                    {
                      offset = (m * dim2 * outChannels * outHeight * outWidth) + (l * outChannels * outHeight * outWidth) + ((j + startChannel) * outHeight * outWidth) +
                               ((i + startHeight) * outWidth) + (k1 * batchOffset);
                      if (dataBuffParam->elementType == 0)
                      {
                        uint8_t *src, *dst;
                        int32_t k;
                        src = (uint8_t *)(ptr + (k1 * inBlkPitchW) + (m * dim1Pitch) + (l * dim2Pitch) + (j * temp_currChPitch) + (i * linePitch) + (dataBuffParam->padH * linePitch) + dataBuffParam->padW);
                        dst = (uint8_t *)(outPtr + offset);
                        for (k = 0; k < outWidth; k++)
                        {
                          dst[k] = src[k];
                          if (tidl_getWriteLevel() == 3)
                          {
                            floatPtr[offset + k] = ((float32_tidl)src[k] - dataBuffParam->tensorZeroPoint) / (dataBuffParam->tensorScale);
                          }
                        }
                      }
                      else if (dataBuffParam->elementType == 1)
                      {
                        int8_t *src;
                        int8_t *dst;
                        int32_t k;
                        src = (ptr + (k1 * inBlkPitchW) + (m * dim1Pitch) + (l * dim2Pitch) + (j * temp_currChPitch) + (i * linePitch) + (dataBuffParam->padH * linePitch) + dataBuffParam->padW);
                        dst = (outPtr + offset);
                        for (k = 0; k < outWidth; k++)
                        {
                          // dst[k] = src[k] + 128;
                          dst[k] = src[k];
                          if (tidl_getWriteLevel() == 3)
                          {
                            floatPtr[offset + k] = ((float32_tidl)src[k] - dataBuffParam->tensorZeroPoint) / (dataBuffParam->tensorScale);
                          }
                        }
                      }
                      else if (dataBuffParam->elementType == 2)
                      {
                        uint16_t *src, *dst;
                        int32_t k;
                        src = ((uint16_t *)ptr + (k1 * inBlkPitchW) + (m * dim1Pitch) + (l * dim2Pitch) + (j * temp_currChPitch) + (i * linePitch) + (dataBuffParam->padH * linePitch) + dataBuffParam->padW);
                        dst = ((uint16_t *)outPtr + offset);
                        for (k = 0; k < outWidth; k++)
                        {
                          dst[k] = src[k];
                          if (tidl_getWriteLevel() == 3)
                          {
                            floatPtr[offset + k] = ((float32_tidl)src[k] - dataBuffParam->tensorZeroPoint) / (dataBuffParam->tensorScale);
                          }
                        }
                      }
                      else if (dataBuffParam->elementType == 3)
                      {
                        int16_t *src, *dst;
                        int32_t k;
                        src = ((int16_t *)ptr + (k1 * inBlkPitchW) + (m * dim1Pitch) + (l * dim2Pitch) + (j * temp_currChPitch) + (i * linePitch) + (dataBuffParam->padH * linePitch) + dataBuffParam->padW);
                        dst = ((int16_t *)outPtr + offset);
                        for (k = 0; k < outWidth; k++)
                        {
                          dst[k] = src[k];
                          if (tidl_getWriteLevel() == 3)
                          {
                            floatPtr[offset + k] = ((float32_tidl)src[k] - dataBuffParam->tensorZeroPoint) / (dataBuffParam->tensorScale);
                          }
                        }
                      }
                      else
                      {
                        (void)memcpy((outPtr + (offset * elementSizeBytes)),
                                     (ptr + (((k1 * inBlkPitchW) + (m * dim1Pitch) + (l * dim2Pitch) + (j * temp_currChPitch) + (i * linePitch) + (dataBuffParam->padH * linePitch) +
                                              dataBuffParam->padW) *
                                             elementSizeBytes)),
                                     ((size_t)outWidth * (size_t)elementSizeBytes));
                      }
                    }
                    if (tidl_getWriteLevel() == 2)
                    {
                      if ((temp_height != dataBuffParam->dimValues[TIDL_DIM_HEIGHT]) && (startHeight != 0))
                      {
                        tidl_printf(0, "tidl_getWriteLevel() = 2 with ST is not supported now \n");
                        status = IALG_EFAIL;
                      }
                      else
                      {
                        (void)memcpy((outWithPadPtr + (batchIdx * dataBuffParam->dimValues[TIDL_DIM_NUMCH] * paddeOutSize) + (j * paddeOutSize)), (ptr + (batchIdx * batchPitch) + (j * chPitch)), paddeOutSize);
                      }
                    }
                  }
                }
              }
            }
            (void)strcpy((char *)traceDumpName, (char *)traceBaseName);
            (void)strcat((char *)traceDumpName, ".y");
            if (elementSizeBytes == 4)
            {
              (void)strcpy((char *)traceDumpName, (char *)traceBaseName);
              (void)strcat((char *)traceDumpName, "_float.bin");
            }
            (void)tidl_getWriteBinToFile()((const char *)traceDumpName, outPtr, dataSize * elementSizeBytes, 1);
            if (tidl_getWriteLevel() == 2)
            {
              (void)strcpy((char *)traceDumpName, (char *)traceBaseName);
              (void)strcat((char *)traceDumpName, "_paded.y");
              (void)tidl_getWriteBinToFile()((const char *)traceDumpName, outWithPadPtr, padedDataSize * elementSizeBytes, 1);
            }

            if (tidl_getWriteLevel() == 3)
            {
              (void)strcpy((char *)traceDumpName, (char *)traceBaseName);
              (void)strcat((char *)traceDumpName, "_float.bin");
              (void)tidl_getWriteBinToFile()((const char *)traceDumpName, floatPtr, dataSize * ((int32_t)sizeof(float32_tidl)), 1);
            }
          }
        }
      }
      else
      {
        int32_t elementSizeBytes;
        elementSizeBytes = TIDL_getDatElementSize(dataBuffParam->elementType);
        if (elementSizeBytes == 4)
        {
          tidl_setWriteLevel((tidl_getWriteLevel() == 3) ? 1 : tidl_getWriteLevel());
        }
        int32_t dim1 = dataBuffParam->dimValues[TIDL_DIM_DIM1];
        int32_t dim2 = dataBuffParam->dimValues[TIDL_DIM_DIM2];
        int32_t dataSize = totalBatches * dataBuffParam->dimValues[TIDL_DIM_DIM1] * dataBuffParam->dimValues[TIDL_DIM_DIM2] * dataBuffParam->dimValues[TIDL_DIM_NUMCH] * dataBuffParam->dimValues[TIDL_DIM_HEIGHT] * dataBuffParam->dimValues[TIDL_DIM_WIDTH];
        int32_t linePitch = dataBuffParam->pitch[TIDL_LINE_PITCH];
        int32_t chPitch = dataBuffParam->pitch[TIDL_CHANNEL_PITCH];
        int32_t dim2Pitch = dataBuffParam->pitch[TIDL_DIM2_PITCH];
        int32_t dim1Pitch = dataBuffParam->pitch[TIDL_DIM1_PITCH];
        int32_t batchPitch = dataBuffParam->pitch[TIDL_ROI_PITCH];
        int32_t paddeOutSize = linePitch * (dataBuffParam->dimValues[TIDL_DIM_HEIGHT] + (dataBuffParam->padH * 2) + 1);
        int32_t padedDataSize = totalBatches * dataBuffParam->dimValues[TIDL_DIM_NUMCH] * paddeOutSize;
        int8_t *outPtr = (int8_t *)scratchPtr;
        int8_t *outWithPadPtr = (int8_t *)(scratchPtr + (dataSize * elementSizeBytes));
        float32_tidl *floatPtr = (float32_tidl *)(scratchPtr + (dataSize * elementSizeBytes));
        int32_t i;
        int32_t j, l, m;
        int32_t batchIdx;
        int32_t batchOffset;
        int32_t offset;
        int32_t totalSize = 0;

        if (tidl_getWriteLevel() == 1)
        {
          totalSize = (dataSize * elementSizeBytes);
        }
        else if (tidl_getWriteLevel() == 2)
        {
          totalSize = ((dataSize * elementSizeBytes) + (padedDataSize * elementSizeBytes));
        }
        else if (tidl_getWriteLevel() == 3)
        {
          totalSize = (dataSize * elementSizeBytes) + ((int32_t)sizeof(float32_tidl) * dataSize);
        }
        else
        {
          tidl_printf(0, "Un supported tidl_getWriteLevel() \n");
          status = TIDL_ERR_FAILURE;
        }

        if (dataBuffParam->elementType == TIDL_SinglePrecFloat)
        {
          float32_tidl curMin = FLT_MAX, curMax = -FLT_MAX;
          TIDL_TensorMinMax((float32_tidl *)ptr, dataBuffParam, numBatchesToWrite, &curMin, &curMax);
          tidl_printf(0, " %3d %15.5f %15.5f %15.5f %d\n", dataBuffId, dataBuffParam->tensorScale, curMin, curMax, dataBuffParam->elementType);
        }
        else
        {
          tidl_printf(0, " %3d %15.5f %15.5f %15.5f %d\n", dataBuffId, dataBuffParam->tensorScale, dataBuffParam->minTensorValue, dataBuffParam->maxTensorValue, dataBuffParam->elementType);
        }

        if ((totalSize > scratchSize) && (status == TIDL_SUCCESS))
        {
          tidl_printf(0, " In sufficient Memory to write Traces, requested size is : %d, allocated size : %d \n", totalSize, scratchSize);
          status = TIDL_ERR_FAILURE;
        }

        if (status == TIDL_SUCCESS)
        {
          if (traceBaseNameCreateParams != NULL)
          {
            (void)strcpy((char *)traceDumpName, (char *)traceBaseNameCreateParams);
          }
          else
          {
            (void)strcpy((char *)traceDumpName, (char *)"");
          }

          (void)sprintf((char *)DataIDString, "%s%04d_%04d_%04d_%05d_%05d_%05dx%05d",
                        traceDumpName, dataBuffId,
                        totalBatches,
                        dataBuffParam->dimValues[TIDL_DIM_DIM1],
                        dataBuffParam->dimValues[TIDL_DIM_DIM2],
                        dataBuffParam->dimValues[TIDL_DIM_NUMCH],
                        dataBuffParam->dimValues[TIDL_DIM_WIDTH],
                        dataBuffParam->dimValues[TIDL_DIM_HEIGHT]);

          (void)strcpy((char *)traceBaseName, (char *)DataIDString);

          if ((startHeight != 0) || (startChannel != 0) || (startBatch != 0))
          {
            (void)strcpy((char *)traceDumpName, (char *)traceBaseName);
            (void)strcat((char *)traceDumpName, ".y");

            if(dataBuffParam->elementType == TIDL_SinglePrecFloat)
            {
              (void)strcpy((char *)traceDumpName, (char *)traceBaseName);
              (void)strcat((char *)traceDumpName, "_float.bin");
            }
            (void)tidl_getReadBinFromFile()((const char *)traceDumpName, outPtr, dataSize * elementSizeBytes, 1);
            if (tidl_getWriteLevel() == 2)
            {
              (void)strcpy((char *)traceDumpName, (char *)traceBaseName);
              (void)strcat((char *)traceDumpName, "_paded.y");
              (void)tidl_getReadBinFromFile()((const char *)traceDumpName, outWithPadPtr, padedDataSize * elementSizeBytes, 1);
            }

            if (tidl_getWriteLevel() == 3)
            {
              (void)strcpy((char *)traceDumpName, (char *)traceBaseName);
              (void)strcat((char *)traceDumpName, "_float.bin");
              (void)tidl_getReadBinFromFile()((const char *)traceDumpName, floatPtr, dataSize * ((int32_t)sizeof(float32_tidl)), 1);
            }
          }

          int32_t currentBatch = 0;
          for (batchIdx = 0; batchIdx < numBatchesToWrite; batchIdx++)
          {
            if ((algLayer->gcHelperHandle != NULL) && (numBatchesToWrite == 1) && (isInputData == 0)) // only one batch data written at a time
            {
              currentBatch = startBatch;
            }
            else
            {
              currentBatch = batchIdx;
            }
            batchOffset = currentBatch * dataBuffParam->dimValues[TIDL_DIM_DIM1] * dataBuffParam->dimValues[TIDL_DIM_DIM2] * dataBuffParam->dimValues[TIDL_DIM_NUMCH] * dataBuffParam->dimValues[TIDL_DIM_HEIGHT] * dataBuffParam->dimValues[TIDL_DIM_WIDTH];
            for (m = 0; m < dim1; m++)
            {
              for (l = 0; l < dim2; l++)
              {
                for (j = 0; j < temp_numChannels; j++)
                {
                  for (i = 0; i < temp_height; i++)
                  {
                    offset = (m * dataBuffParam->dimValues[TIDL_DIM_DIM2] * dataBuffParam->dimValues[TIDL_DIM_NUMCH] * dataBuffParam->dimValues[TIDL_DIM_HEIGHT] * dataBuffParam->dimValues[TIDL_DIM_WIDTH]) + (l * dataBuffParam->dimValues[TIDL_DIM_NUMCH] * dataBuffParam->dimValues[TIDL_DIM_HEIGHT] * dataBuffParam->dimValues[TIDL_DIM_WIDTH]) +
                             ((j + startChannel) * dataBuffParam->dimValues[TIDL_DIM_HEIGHT] * dataBuffParam->dimValues[TIDL_DIM_WIDTH]) +
                             ((i + startHeight) * dataBuffParam->dimValues[TIDL_DIM_WIDTH]) + batchOffset;
                    if (dataBuffParam->elementType == 0)
                    {
                      uint8_t *src, *dst;
                      int32_t k;
                      src = (uint8_t *)(ptr + (batchIdx * batchPitch) + (m * dim1Pitch) + (l * dim2Pitch) + (j * temp_currChPitch) + (i * linePitch) + (dataBuffParam->padH * linePitch) + dataBuffParam->padW);
                      dst = (uint8_t *)(outPtr + offset);
                      for (k = 0; k < dataBuffParam->dimValues[TIDL_DIM_WIDTH]; k++)
                      {
                        dst[k] = src[k];
                        if (tidl_getWriteLevel() == 3)
                        {
                          floatPtr[offset + k] = ((float32_tidl)src[k] - dataBuffParam->tensorZeroPoint) / (dataBuffParam->tensorScale);
                        }
                      }
                    }
                    else if (dataBuffParam->elementType == 1)
                    {
                      int8_t *src;
                      int8_t *dst;
                      int32_t k;
                      src = (ptr + (batchIdx * batchPitch) + (m * dim1Pitch) + (l * dim2Pitch) + (j * temp_currChPitch) + (i * linePitch) + (dataBuffParam->padH * linePitch) + dataBuffParam->padW);
                      dst = (outPtr + offset);
                      for (k = 0; k < dataBuffParam->dimValues[TIDL_DIM_WIDTH]; k++)
                      {
                        // dst[k] = src[k] + 128;
                        dst[k] = src[k];
                        if (tidl_getWriteLevel() == 3)
                        {
                          floatPtr[offset + k] = ((float32_tidl)src[k] - dataBuffParam->tensorZeroPoint) / (dataBuffParam->tensorScale);
                        }
                      }
                    }
                    else if (dataBuffParam->elementType == 2)
                    {
                      uint16_t *src;
                      uint16_t *dst;
                      int32_t k;
                      src = (((uint16_t *)ptr) + (batchIdx * batchPitch) + (m * dim1Pitch) + (l * dim2Pitch) + (j * temp_currChPitch) + (i * linePitch) + (dataBuffParam->padH * linePitch) + dataBuffParam->padW);
                      dst = (((uint16_t *)outPtr) + offset);
                      for (k = 0; k < dataBuffParam->dimValues[TIDL_DIM_WIDTH]; k++)
                      {
                        dst[k] = src[k];
                        if (tidl_getWriteLevel() == 3)
                        {
                          floatPtr[offset + k] = ((float32_tidl)src[k] - dataBuffParam->tensorZeroPoint) / (dataBuffParam->tensorScale);
                        }
                      }
                    }
                    else if (dataBuffParam->elementType == 3)
                    {
                      int16_t *src;
                      int16_t *dst;
                      int32_t k;
                      src = (((int16_t *)ptr) + (batchIdx * batchPitch) + (m * dim1Pitch) + (l * dim2Pitch) + (j * temp_currChPitch) + (i * linePitch) + (dataBuffParam->padH * linePitch) + dataBuffParam->padW);
                      dst = (((int16_t *)outPtr) + offset);
                      for (k = 0; k < dataBuffParam->dimValues[TIDL_DIM_WIDTH]; k++)
                      {
                        // dst[k] = src[k] + 128;
                        dst[k] = src[k];
                        if (tidl_getWriteLevel() == 3)
                        {
                          floatPtr[offset + k] = ((float32_tidl)src[k] - dataBuffParam->tensorZeroPoint) / (dataBuffParam->tensorScale);
                        }
                      }
                    }
                    else
                    {
                      (void)memcpy((outPtr + ((((currentBatch * dataBuffParam->dimValues[TIDL_DIM_DIM1] * dataBuffParam->dimValues[TIDL_DIM_DIM2] * dataBuffParam->dimValues[TIDL_DIM_NUMCH] * dataBuffParam->dimValues[TIDL_DIM_HEIGHT] * dataBuffParam->dimValues[TIDL_DIM_WIDTH]) + (m * dataBuffParam->dimValues[TIDL_DIM_DIM2] * dataBuffParam->dimValues[TIDL_DIM_NUMCH] * dataBuffParam->dimValues[TIDL_DIM_HEIGHT] * dataBuffParam->dimValues[TIDL_DIM_WIDTH]) + (l * dataBuffParam->dimValues[TIDL_DIM_NUMCH] * dataBuffParam->dimValues[TIDL_DIM_HEIGHT] * dataBuffParam->dimValues[TIDL_DIM_WIDTH])) + (j * dataBuffParam->dimValues[TIDL_DIM_HEIGHT] * dataBuffParam->dimValues[TIDL_DIM_WIDTH]) +
                                               (i * dataBuffParam->dimValues[TIDL_DIM_WIDTH])) *
                                              elementSizeBytes)),
                                   (ptr + (((batchIdx * batchPitch) + (m * dim1Pitch) + (l * dim2Pitch) + (j * temp_currChPitch) + (i * linePitch) + (dataBuffParam->padH * linePitch) +
                                            dataBuffParam->padW) *
                                           elementSizeBytes)),
                                   ((size_t)dataBuffParam->dimValues[TIDL_DIM_WIDTH] * (size_t)elementSizeBytes));
                    }
                  }
                  if (tidl_getWriteLevel() == 2)
                  {
                    if ((temp_height != dataBuffParam->dimValues[TIDL_DIM_HEIGHT]) && (startHeight != 0))
                    {
                      tidl_printf(0, "tidl_getWriteLevel() = 2 with ST is not supported now \n");
                      status = IALG_EFAIL;
                    }
                    else
                    {
                      (void)memcpy((outWithPadPtr + (batchIdx * dataBuffParam->dimValues[TIDL_DIM_NUMCH] * paddeOutSize) + (j * paddeOutSize)), (ptr + (batchIdx * batchPitch) + (j * chPitch)), paddeOutSize);
                    }
                  }
                }
              }
            }
          }
          (void)strcpy((char *)traceDumpName, (char *)traceBaseName);
          (void)strcat((char *)traceDumpName, ".y");
          if (elementSizeBytes == 4)
          {
            (void)strcpy((char *)traceDumpName, (char *)traceBaseName);
            (void)strcat((char *)traceDumpName, "_float.bin");
          }
          (void)tidl_getWriteBinToFile()((const char *)traceDumpName, outPtr, dataSize * elementSizeBytes, 1);
          if (tidl_getWriteLevel() == 2)
          {
            (void)strcpy((char *)traceDumpName, (char *)traceBaseName);
            (void)strcat((char *)traceDumpName, "_paded.y");
            (void)tidl_getWriteBinToFile()((const char *)traceDumpName, outWithPadPtr, padedDataSize * elementSizeBytes, 1);
          }

          if (tidl_getWriteLevel() == 3)
          {
            (void)strcpy((char *)traceDumpName, (char *)traceBaseName);
            (void)strcat((char *)traceDumpName, "_float.bin");
            (void)tidl_getWriteBinToFile()((const char *)traceDumpName, floatPtr, dataSize * ((int32_t)sizeof(float32_tidl)), 1);
          }
        }
      }
      *dataBuffParam = dataBuffParamOrig;
    }
    tidl_setWriteLevel(tidlWriteLevelOrig);

    /** Wb/inv L1D/L2 cache after writing traces*/
    TIDL_L1DandL2CacheWbInv();
  }
  return status;
}

#if 0
/**
 * @brief Function to write traces from data buffer
 *
 * @param ptr : Pointer to the data buffer
 * @param net : tidl network structure
 * @param dataBuffId : id of the databuffer
 * @param orgScratchPtr : pointer to the scratch buffer
 * @param orgScratchSize : Size of the scratch buffer
 * @param currentLineWriteOffset : Offset to the current line
 * @param currentNumLines : No of lines for writting
 * @param currChPitch : current channel pitch
 */
int32_t tidl_writeTraceDataBuf(int8_t * ptr, sTIDL_Network_t * net, int32_t dataBuffId, int8_t * orgScratchPtr, uint32_t orgScratchSize, int32_t currentLineWriteOffset, int32_t currentNumLines, int32_t currChPitch)
{
  int32_t status = TIDL_SUCCESS;
  if (tidl_getWriteLevel() > 0)
  {
    int8_t * scratchPtr =  (int8_t *)(orgScratchPtr  + TRACE_STRINGS_MEM_SIZE);
    int8_t * traceDumpName = (int8_t *)(orgScratchPtr);
    int8_t * traceBaseName = (int8_t *)(orgScratchPtr  + (1*TRACE_STRING_SIZE));
    int8_t * DataIDString  = (int8_t *)(orgScratchPtr  + (2*TRACE_STRING_SIZE));
    int32_t layerIdx;
    int32_t scratchSize =  orgScratchSize - TRACE_STRINGS_MEM_SIZE;
    int32_t tidlWriteLevelOrig = tidl_getWriteLevel();
    sTIDL_DataParams_t * dataBuffParam;
    dataBuffParam = TIDL_getDataParams(net, dataBuffId);
    layerIdx = TIDL_getLayerNum(net, dataBuffId);


    /* for trace Level 4 enable trace level 3 by default */
    if ( tidl_getWriteLevel() == 4 )
    {
      tidl_setWriteLevel(3) ;
    }
    if (dataBuffParam != NULL)
    {
     sTIDL_DataParams_t dataBuffParamOrig = *dataBuffParam;

      if ( net->TIDLLayers[layerIdx].layerType == TIDL_DataConvertLayer )
      {
        int32_t inLayout = net->TIDLLayers[layerIdx].layerParams.dataConvertParams.layout, outLayout = net->TIDLLayers[layerIdx].layerParams.dataConvertParams.outLayout;
        if ( (net->TIDLLayers[layerIdx].layerParams.dataConvertParams.layout == TIDL_LT_NHWC && net->TIDLLayers[layerIdx].layerParams.dataConvertParams.type == TIDL_DC_TYPE_OUTPUT) ||
             (net->TIDLLayers[layerIdx].layerParams.dataConvertParams.type == TIDL_DC_TYPE_INTERMEDIATE) && (inLayout != outLayout))
        {
          dataBuffParam->dimValues[TIDL_DIM_WIDTH]  = dataBuffParamOrig.dimValues[TIDL_DIM_NUMCH];
          dataBuffParam->dimValues[TIDL_DIM_HEIGHT] = dataBuffParamOrig.dimValues[TIDL_DIM_WIDTH];
          dataBuffParam->dimValues[TIDL_DIM_NUMCH]  = dataBuffParamOrig.dimValues[TIDL_DIM_HEIGHT];
          dataBuffParam->dimValues[TIDL_DIM_BATCH]  = dataBuffParamOrig.dimValues[TIDL_DIM_BATCH];

          dataBuffParam->pitch[TIDL_LINE_PITCH]    = dataBuffParam->dimValues[TIDL_DIM_WIDTH];
          dataBuffParam->pitch[TIDL_CHANNEL_PITCH] = dataBuffParam->pitch[TIDL_LINE_PITCH] * dataBuffParam->dimValues[TIDL_DIM_HEIGHT];
          dataBuffParam->pitch[TIDL_ROI_PITCH]     = dataBuffParam->pitch[TIDL_CHANNEL_PITCH] * dataBuffParam->dimValues[TIDL_DIM_NUMCH];
          //:TODO: This needs to be correctly handled for super tiling cases
          currentNumLines = dataBuffParam->dimValues[TIDL_DIM_HEIGHT];
          currChPitch     = dataBuffParam->pitch[TIDL_CHANNEL_PITCH];
        }
      }
    if ((dataBuffParam->numBatchW>1 || dataBuffParam->numBatchH>1)&& (dataBuffParam->dimValues[TIDL_DIM_BATCH] == 1))
    {
      int32_t elementSizeBytes;
      elementSizeBytes = TIDL_getDatElementSize(dataBuffParam->elementType);
      if(elementSizeBytes == 4)
      {
        tidl_setWriteLevel(tidl_getWriteLevel() == 3 ? 1 : tidl_getWriteLevel());
      }

      int32_t outWidth = (dataBuffParam->dimValues[TIDL_DIM_WIDTH] - dataBuffParam->batchPadW * (dataBuffParam->numBatchW -1))/dataBuffParam->numBatchW;
      int32_t outHeight = dataBuffParam->dimValues[TIDL_DIM_HEIGHT];
      int32_t outChannels = dataBuffParam->dimValues[TIDL_DIM_NUMCH];
      int32_t outNumBatches = dataBuffParam->numBatchW;
      int32_t dataSize = outWidth * outHeight * outChannels * outNumBatches;
      int32_t linePitch = dataBuffParam->pitch[TIDL_LINE_PITCH];
      int32_t chPitch = dataBuffParam->pitch[TIDL_CHANNEL_PITCH];
      int32_t batchPitch  = dataBuffParam->pitch[TIDL_ROI_PITCH];
      int32_t inBlkPitchW        =  outWidth + dataBuffParam->batchPadW;
      int32_t paddeOutSize = linePitch * (dataBuffParam->dimValues[TIDL_DIM_HEIGHT] + (dataBuffParam->padH * 2) + 1);
      int32_t padedDataSize =  dataBuffParam->dimValues[TIDL_DIM_BATCH] * dataBuffParam->dimValues[TIDL_DIM_NUMCH] * paddeOutSize;
      int8_t * outPtr        = (int8_t *)scratchPtr;
      int8_t * outWithPadPtr = (int8_t *)(scratchPtr + (dataSize*elementSizeBytes));
      float32_tidl  * floatPtr = (float32_tidl  *)(scratchPtr + (dataSize*elementSizeBytes));
      int32_t i;
      int32_t j, k1;
      int32_t batchIdx;
      int32_t batchOffset;
      int32_t offset;
      int32_t totalSize = 0;

      if (tidl_getWriteLevel() == 1)
      {
        totalSize = (dataSize*elementSizeBytes);
      }
      else if (tidl_getWriteLevel() == 2)
      {
        totalSize = ((dataSize*elementSizeBytes) + (padedDataSize*elementSizeBytes));
      }
      else if (tidl_getWriteLevel() == 3)
      {
        totalSize = (dataSize*elementSizeBytes) + ((int32_t)sizeof(float32_tidl)*dataSize);
      }
      else
      {
        tidl_printf(0, "Un supported tidl_getWriteLevel() \n");
        status = TIDL_ERR_FAILURE;
      }

      if(dataBuffParam->elementType == TIDL_SinglePrecFloat)
      {
        float32_tidl curMin = FLT_MAX, curMax = -FLT_MAX;
        TIDL_TensorMinMax((float32_tidl*)ptr, dataBuffParam, &curMin, &curMax);
        tidl_printf(0, " %3d %10.5f %10.5f %10.5f %d\n", dataBuffId, dataBuffParam->tensorScale, curMin, curMax, dataBuffParam->elementType);
      }
      else
      {
        tidl_printf(0, " %3d %10.5f %10.5f %10.5f %d\n", dataBuffId, dataBuffParam->tensorScale, dataBuffParam->minTensorValue, dataBuffParam->maxTensorValue, dataBuffParam->elementType);
      }


      if ((totalSize > scratchSize) && (status == TIDL_SUCCESS))
      {
        tidl_printf(0, " In sufficient Memory to write Traces, as requested size is : %d, allocated size : %d \n", totalSize, scratchSize);
        status = TIDL_ERR_FAILURE;
      }

      if(status == TIDL_SUCCESS)
      {
        if(tidl_getTraceBaseName() != NULL)
        {
          (void)strcpy((char *)traceDumpName, (char *)tidl_getTraceBaseName());
        }
        else
        {
          (void)strcpy((char *)traceDumpName, (char *)"");
        }

        (void)sprintf((char *)DataIDString, "%04d", dataBuffId);
        (void)strcat((char *)traceDumpName, (char *)DataIDString);
        (void)strcat((char *)traceDumpName, "_");
        (void)sprintf((char *)DataIDString, "%05d", outNumBatches);
        (void)strcat((char *)traceDumpName, (char *)DataIDString);
        (void)strcat((char *)traceDumpName, "_");
        (void)sprintf((char *)DataIDString, "%05d", outChannels);
        (void)strcat((char *)traceDumpName, (char *)DataIDString);
        (void)strcat((char *)traceDumpName, "_");
        (void)sprintf((char *)DataIDString, "%05d", outWidth);
        (void)strcat((char *)traceDumpName, (char *)DataIDString);
        (void)strcat((char *)traceDumpName, "x");
        (void)sprintf((char *)DataIDString, "%05d", outHeight);
        (void)strcat((char *)traceDumpName, (char *)DataIDString);
        (void)strcpy((char *)traceBaseName, (char *)traceDumpName);

        if(currentLineWriteOffset != 0)
        {
          (void)strcpy((char *)traceDumpName, (char *)traceBaseName);
          (void)strcat((char *)traceDumpName, ".y");

          if(elementSizeBytes == 4)
          {
            (void)strcpy((char *)traceDumpName, (char *)traceBaseName);
            (void)strcat((char *)traceDumpName, "_float.bin");
          }

          (void)tidl_getReadBinFromFile()((const char *)traceDumpName, outPtr, dataSize*elementSizeBytes, 1);
          if (tidl_getWriteLevel() == 2)
          {
            (void)strcpy((char *)traceDumpName, (char *)traceBaseName);
            (void)strcat((char *)traceDumpName, "_paded.y");
            (void)tidl_getReadBinFromFile()((const char *)traceDumpName, outWithPadPtr, padedDataSize*elementSizeBytes, 1);
          }

          if (tidl_getWriteLevel() == 3)
          {
            (void)strcpy((char *)traceDumpName, (char *)traceBaseName);
            (void)strcat((char *)traceDumpName, "_float.bin");
            (void)tidl_getReadBinFromFile()((const char *)traceDumpName, floatPtr, dataSize*((int32_t)sizeof(float32_tidl)), 1);
          }
        }

      for (batchIdx = 0; batchIdx < 1; batchIdx++)
      {
        batchOffset =  outChannels  * outWidth* outHeight;

        for (j = 0; j < outChannels; j++)
        {
          for (i = 0; i < currentNumLines; i++)
          {
            for (k1 = 0; k1 < outNumBatches; k1++)
            {
              offset = (j*outHeight * outWidth) +
                       ((i + currentLineWriteOffset)*outWidth) + k1*batchOffset;
            if (dataBuffParam->elementType == 0)
            {
              uint8_t * src, *dst;
              int32_t k;
              src = (uint8_t *)(ptr + (k1 * inBlkPitchW) + (j*currChPitch) + (i*linePitch) + (dataBuffParam->padH*linePitch) + dataBuffParam->padW);
              dst = (uint8_t *)(outPtr + offset);
              for (k = 0; k < outWidth; k++)
              {
                dst[k] = src[k];
                if (tidl_getWriteLevel() == 3)
                {
                  floatPtr[offset + k] = ((float32_tidl)src[k]) / (dataBuffParam->tensorScale);
                }
              }
            }
            else if (dataBuffParam->elementType == 1)
            {
              int8_t * src;
              int8_t * dst;
              int32_t k;
              src = (ptr + (k1 * inBlkPitchW) + (j*currChPitch) + (i*linePitch) + (dataBuffParam->padH*linePitch) + dataBuffParam->padW);
              dst = (outPtr + offset);
              for (k = 0; k < outWidth; k++)
              {
                //dst[k] = src[k] + 128;
                dst[k] = src[k];
                if (tidl_getWriteLevel() == 3)
                {
                  floatPtr[offset + k] = ((float32_tidl)src[k]) / (dataBuffParam->tensorScale);
                }
              }
            }
            else if (dataBuffParam->elementType == 2)
            {
              uint16_t * src, *dst;
              int32_t k;
              src = ((uint16_t *)ptr + (k1 * inBlkPitchW) + (j*currChPitch) + (i*linePitch) + (dataBuffParam->padH*linePitch) + dataBuffParam->padW);
              dst = ((uint16_t *)outPtr + offset);
              for (k = 0; k < outWidth; k++)
              {
                dst[k] = src[k];
                if (tidl_getWriteLevel() == 3)
                {
                  floatPtr[offset + k] = ((float32_tidl)src[k]) / (dataBuffParam->tensorScale);
                }
              }
            }
            else if (dataBuffParam->elementType == 3)
            {
              int16_t * src, *dst;
              int32_t k;
              src = ((int16_t *)ptr + (k1 * inBlkPitchW) + (j*currChPitch) + (i*linePitch) + (dataBuffParam->padH*linePitch) + dataBuffParam->padW);
              dst = ((int16_t *)outPtr + offset);
              for (k = 0; k < outWidth; k++)
              {
                dst[k] = src[k];
                if (tidl_getWriteLevel() == 3)
                {
                  floatPtr[offset + k] = ((float32_tidl)src[k]) / (dataBuffParam->tensorScale);
                }
              }
            }
            else
            {
              (void)memcpy((outPtr + (offset*elementSizeBytes)),
                (ptr + (((k1 * inBlkPitchW) + (j*currChPitch) + (i*linePitch) + (dataBuffParam->padH*linePitch) +
                  dataBuffParam->padW)*elementSizeBytes)),
                (size_t)(outWidth * elementSizeBytes));
            }
          }
          if (tidl_getWriteLevel() == 2)
          {
            if((currentNumLines != dataBuffParam->dimValues[TIDL_DIM_HEIGHT]) && (currentLineWriteOffset != 0))
            {
              tidl_printf(0, "tidl_getWriteLevel() = 2 with ST is not supported now \n");
              status = IALG_EFAIL;
            }
            else
            {
              (void)memcpy((outWithPadPtr + batchIdx *dataBuffParam->dimValues[TIDL_DIM_NUMCH] * paddeOutSize + j*paddeOutSize), (ptr + (batchIdx * batchPitch) + (j*chPitch)), paddeOutSize);
            }
          }
        }
      }
        (void)strcpy((char *)traceDumpName, (char *)traceBaseName);
        (void)strcat((char *)traceDumpName, ".y");
        if(elementSizeBytes == 4)
        {
          (void)strcpy((char *)traceDumpName, (char *)traceBaseName);
          (void)strcat((char *)traceDumpName, "_float.bin");
        }
        (void)tidl_getWriteBinToFile()((const char *)traceDumpName, outPtr, dataSize*elementSizeBytes, tidl_getTracePtr());
        if (tidl_getWriteLevel() == 2)
        {
          (void)strcpy((char *)traceDumpName, (char *)traceBaseName);
          (void)strcat((char *)traceDumpName, "_paded.y");
          (void)tidl_getWriteBinToFile()((const char *)traceDumpName, outWithPadPtr, padedDataSize*elementSizeBytes, tidl_getTracePtr());
        }

        if (tidl_getWriteLevel() == 3)
        {
          (void)strcpy((char *)traceDumpName, (char *)traceBaseName);
          (void)strcat((char *)traceDumpName, "_float.bin");
          (void)tidl_getWriteBinToFile()((const char *)traceDumpName, floatPtr, dataSize*((int32_t)sizeof(float32_tidl)), tidl_getTracePtr());
        }
      }
    }
    }
    else
    {
      int32_t elementSizeBytes;
      elementSizeBytes = TIDL_getDatElementSize(dataBuffParam->elementType);
      if(elementSizeBytes == 4)
      {
        tidl_setWriteLevel(tidl_getWriteLevel() == 3 ? 1 : tidl_getWriteLevel());
      }

      int32_t dataSize = dataBuffParam->dimValues[TIDL_DIM_BATCH] * dataBuffParam->dimValues[TIDL_DIM_NUMCH] * dataBuffParam->dimValues[TIDL_DIM_HEIGHT] * dataBuffParam->dimValues[TIDL_DIM_WIDTH];
      int32_t linePitch = dataBuffParam->pitch[TIDL_LINE_PITCH];
      int32_t chPitch = dataBuffParam->pitch[TIDL_CHANNEL_PITCH];
      int32_t batchPitch  = dataBuffParam->pitch[TIDL_ROI_PITCH];
      int32_t paddeOutSize = linePitch * (dataBuffParam->dimValues[TIDL_DIM_HEIGHT] + (dataBuffParam->padH * 2) + 1);
      int32_t padedDataSize =  dataBuffParam->dimValues[TIDL_DIM_BATCH] * dataBuffParam->dimValues[TIDL_DIM_NUMCH] * paddeOutSize;
      int8_t * outPtr        = (int8_t *)scratchPtr;
      int8_t * outWithPadPtr = (int8_t *)(scratchPtr + (dataSize*elementSizeBytes));
      float32_tidl  * floatPtr = (float32_tidl  *)(scratchPtr + (dataSize*elementSizeBytes));
      int32_t i;
      int32_t j;
      int32_t batchIdx;
      int32_t batchOffset;
      int32_t offset;
      int32_t totalSize = 0;

      if (tidl_getWriteLevel() == 1)
      {
        totalSize = (dataSize*elementSizeBytes);
      }
      else if (tidl_getWriteLevel() == 2)
      {
        totalSize = ((dataSize*elementSizeBytes) + (padedDataSize*elementSizeBytes));
      }
      else if (tidl_getWriteLevel() == 3)
      {
        totalSize = (dataSize*elementSizeBytes) + ((int32_t)sizeof(float32_tidl)*dataSize);
      }
      else
      {
        tidl_printf(0, "Un supported tidl_getWriteLevel() \n");
        status = TIDL_ERR_FAILURE;
      }

      if(dataBuffParam->elementType == TIDL_SinglePrecFloat)
      {
        float32_tidl curMin = FLT_MAX, curMax = -FLT_MAX;
        TIDL_TensorMinMax((float32_tidl*)ptr, dataBuffParam, &curMin, &curMax);
        tidl_printf(0, " %3d %15.5f %15.5f %15.5f %d\n", dataBuffId, dataBuffParam->tensorScale, curMin, curMax, dataBuffParam->elementType);
      }
      else
      {
        tidl_printf(0, " %3d %15.5f %15.5f %15.5f %d\n", dataBuffId, dataBuffParam->tensorScale, dataBuffParam->minTensorValue, dataBuffParam->maxTensorValue, dataBuffParam->elementType);
      }


      if ((totalSize > scratchSize) && (status == TIDL_SUCCESS))
      {
        tidl_printf(0, " In sufficient Memory to write Traces, requested size is : %d, allocated size : %d \n", totalSize, scratchSize);
        status = TIDL_ERR_FAILURE;
      }

      if(status == TIDL_SUCCESS)
      {
        if(tidl_getTraceBaseName() != NULL)
        {
          (void)strcpy((char *)traceDumpName, (char *)tidl_getTraceBaseName());
        }
        else
        {
          (void)strcpy((char *)traceDumpName, (char *)"");
        }

        (void)sprintf((char *)DataIDString, "%04d", dataBuffId);
        (void)strcat((char *)traceDumpName, (char *)DataIDString);
        (void)strcat((char *)traceDumpName, "_");
        (void)sprintf((char *)DataIDString, "%05d", dataBuffParam->dimValues[TIDL_DIM_BATCH]);
        (void)strcat((char *)traceDumpName, (char *)DataIDString);
        (void)strcat((char *)traceDumpName, "_");
        (void)sprintf((char *)DataIDString, "%05d", dataBuffParam->dimValues[TIDL_DIM_NUMCH]);
        (void)strcat((char *)traceDumpName, (char *)DataIDString);
        (void)strcat((char *)traceDumpName, "_");
        (void)sprintf((char *)DataIDString, "%05d", dataBuffParam->dimValues[TIDL_DIM_WIDTH]);
        (void)strcat((char *)traceDumpName, (char *)DataIDString);
        (void)strcat((char *)traceDumpName, "x");
        (void)sprintf((char *)DataIDString, "%05d", dataBuffParam->dimValues[TIDL_DIM_HEIGHT]);
        (void)strcat((char *)traceDumpName, (char *)DataIDString);
        (void)strcpy((char *)traceBaseName, (char *)traceDumpName);

        if(currentLineWriteOffset != 0)
        {
          (void)strcpy((char *)traceDumpName, (char *)traceBaseName);
          (void)strcat((char *)traceDumpName, ".y");

          if(elementSizeBytes == 4)
          {
            (void)strcpy((char *)traceDumpName, (char *)traceBaseName);
            (void)strcat((char *)traceDumpName, "_float.bin");
          }

          (void)tidl_getReadBinFromFile()((const char *)traceDumpName, outPtr, dataSize*elementSizeBytes, 1);
          if (tidl_getWriteLevel() == 2)
          {
            (void)strcpy((char *)traceDumpName, (char *)traceBaseName);
            (void)strcat((char *)traceDumpName, "_paded.y");
            (void)tidl_getReadBinFromFile()((const char *)traceDumpName, outWithPadPtr, padedDataSize*elementSizeBytes, 1);
          }

          if (tidl_getWriteLevel() == 3)
          {
            (void)strcpy((char *)traceDumpName, (char *)traceBaseName);
            (void)strcat((char *)traceDumpName, "_float.bin");
            (void)tidl_getReadBinFromFile()((const char *)traceDumpName, floatPtr, dataSize*((int32_t)sizeof(float32_tidl)),1 );
          }
        }

      for (batchIdx = 0; batchIdx < dataBuffParam->dimValues[TIDL_DIM_BATCH]; batchIdx++)
      {
        batchOffset = batchIdx * dataBuffParam->dimValues[TIDL_DIM_NUMCH]  * dataBuffParam->dimValues[TIDL_DIM_HEIGHT] * dataBuffParam->dimValues[TIDL_DIM_WIDTH];

        for (j = 0; j < dataBuffParam->dimValues[TIDL_DIM_NUMCH]; j++)
        {
          for (i = 0; i < currentNumLines; i++)
          {
            offset = (j*dataBuffParam->dimValues[TIDL_DIM_HEIGHT] * dataBuffParam->dimValues[TIDL_DIM_WIDTH]) +
                     ((i + currentLineWriteOffset)*dataBuffParam->dimValues[TIDL_DIM_WIDTH]) + batchOffset;

            if (dataBuffParam->elementType == 0)
            {
              uint8_t * src, *dst;
              int32_t k;
              src = (uint8_t *)(ptr + batchIdx * batchPitch + (j*currChPitch) + (i*linePitch) + (dataBuffParam->padH*linePitch) + dataBuffParam->padW);
              dst = (uint8_t *)(outPtr + offset);
              for (k = 0; k < dataBuffParam->dimValues[TIDL_DIM_WIDTH]; k++)
              {
                dst[k] = src[k];
                if (tidl_getWriteLevel() == 3)
                {
                  floatPtr[offset + k] = ((float32_tidl)src[k]) / (dataBuffParam->tensorScale);
                }
              }
            }
            else if (dataBuffParam->elementType == 1)
            {
              int8_t * src;
              int8_t * dst;
              int32_t k;
              src = (ptr + batchIdx * batchPitch + (j*currChPitch) + (i*linePitch) + (dataBuffParam->padH*linePitch) + dataBuffParam->padW);
              dst = (outPtr + offset);
              for (k = 0; k < dataBuffParam->dimValues[TIDL_DIM_WIDTH]; k++)
              {
                //dst[k] = src[k] + 128;
                dst[k] = src[k];
                if (tidl_getWriteLevel() == 3)
                {
                  floatPtr[offset + k] = ((float32_tidl)src[k]) / (dataBuffParam->tensorScale);
                }
              }
            }
            else if (dataBuffParam->elementType == 2)
            {
              uint16_t * src;
              uint16_t * dst;
              int32_t k;
              src = (((uint16_t *)ptr) + batchIdx * batchPitch + (j*currChPitch) + (i*linePitch) + (dataBuffParam->padH*linePitch) + dataBuffParam->padW);
              dst = (((uint16_t *)outPtr) + offset);
              for (k = 0; k < dataBuffParam->dimValues[TIDL_DIM_WIDTH]; k++)
              {
                dst[k] = src[k];
                if (tidl_getWriteLevel() == 3)
                {
                  floatPtr[offset + k] = ((float32_tidl)src[k]) / (dataBuffParam->tensorScale);
                }
              }
            }
            else if (dataBuffParam->elementType == 3)
            {
              int16_t * src;
              int16_t * dst;
              int32_t k;
              src = (((int16_t *)ptr) + batchIdx * batchPitch + (j*currChPitch) + (i*linePitch) + (dataBuffParam->padH*linePitch) + dataBuffParam->padW);
              dst = (((int16_t *)outPtr) + offset);
              for (k = 0; k < dataBuffParam->dimValues[TIDL_DIM_WIDTH]; k++)
              {
                //dst[k] = src[k] + 128;
                dst[k] = src[k];
                if (tidl_getWriteLevel() == 3)
                {
                  floatPtr[offset + k] = ((float32_tidl)src[k]) / (dataBuffParam->tensorScale);
                }
              }
            }
            else
            {
              (void)memcpy((outPtr + ((batchIdx * dataBuffParam->dimValues[TIDL_DIM_NUMCH]  * dataBuffParam->dimValues[TIDL_DIM_HEIGHT] * dataBuffParam->dimValues[TIDL_DIM_WIDTH])
              + (j*dataBuffParam->dimValues[TIDL_DIM_HEIGHT] * dataBuffParam->dimValues[TIDL_DIM_WIDTH]) +
                (i*dataBuffParam->dimValues[TIDL_DIM_WIDTH]))*elementSizeBytes),
                (ptr + (((batchIdx * batchPitch) + (j*currChPitch) + (i*linePitch) + (dataBuffParam->padH*linePitch) +
                  dataBuffParam->padW)*elementSizeBytes)),
                (size_t)(dataBuffParam->dimValues[TIDL_DIM_WIDTH] * elementSizeBytes));
            }

          }
          if (tidl_getWriteLevel() == 2)
          {
            if((currentNumLines != dataBuffParam->dimValues[TIDL_DIM_HEIGHT]) && (currentLineWriteOffset != 0))
            {
              tidl_printf(0, "tidl_getWriteLevel() = 2 with ST is not supported now \n");
              status = IALG_EFAIL;
            }
            else
            {
              (void)memcpy((outWithPadPtr + batchIdx *dataBuffParam->dimValues[TIDL_DIM_NUMCH] * paddeOutSize + j*paddeOutSize), (ptr + (batchIdx * batchPitch) + (j*chPitch)), paddeOutSize);
            }
          }
        }
      }
        (void)strcpy((char *)traceDumpName, (char *)traceBaseName);
        (void)strcat((char *)traceDumpName, ".y");
        if(elementSizeBytes == 4)
        {
          (void)strcpy((char *)traceDumpName, (char *)traceBaseName);
          (void)strcat((char *)traceDumpName, "_float.bin");
        }
        (void)tidl_getWriteBinToFile()((const char *)traceDumpName, outPtr, dataSize*elementSizeBytes, tidl_getTracePtr());
        if (tidl_getWriteLevel() == 2)
        {
          (void)strcpy((char *)traceDumpName, (char *)traceBaseName);
          (void)strcat((char *)traceDumpName, "_paded.y");
          (void)tidl_getWriteBinToFile()((const char *)traceDumpName, outWithPadPtr, padedDataSize*elementSizeBytes, tidl_getTracePtr());
        }

        if (tidl_getWriteLevel() == 3)
        {
          (void)strcpy((char *)traceDumpName, (char *)traceBaseName);
          (void)strcat((char *)traceDumpName, "_float.bin");
          (void)tidl_getWriteBinToFile()((const char *)traceDumpName, floatPtr, dataSize*((int32_t)sizeof(float32_tidl)), tidl_getTracePtr());
        }
      }
    }
    *dataBuffParam = dataBuffParamOrig;

    }
    tidl_setWriteLevel(tidlWriteLevelOrig) ;

    /** Wb/inv L1D/L2 cache after writing traces*/
    TIDL_L1DandL2CacheWbInv();
  }
  return status;
}
#endif

/**
 * @brief Function to generate Fill Seam Predicate Registers
 *
 * @param numTiles : no of tiles in the cuurent tensor
 * @param bufPredicateStore : buffer to store predicate stores
 * @param srcAddr : pointer to source data buffer
 * @param dstAddr : pointer to dst data buffer
 * @return int32_t : total bytes to store
 */
int32_t TIDL_generateFillSeamPredicateRegisters(
    int32_t numTiles,
    void *bufPredicateStore,
    const TIDL_bufParams3D_t *srcAddr,
    const TIDL_bufParams3D_t *dstAddr)
{
  int32_t numBlocks;
  int32_t mmaWidth;
  int32_t totalBytes;
  int32_t i, j, k;
  uint32_t numBytes = TIDL_sizeof(srcAddr->data_type);
  int32_t inputWidth = srcAddr->dim_x;
  int32_t processHeight = dstAddr->dim_y;
  int32_t processWidth = srcAddr->stride_y / numBytes;
  int32_t matrixColumn = processWidth * processHeight;
  int32_t blockCounter = 0;
  int64_t *predRegister = (int64_t *)bufPredicateStore;
  uint64_t predictedValue = 0x0;

  numBlocks = numTiles;
  totalBytes = numBlocks * 8;

  j = 0;
  k = 1;

  if (numBytes == 1U)
  {
    mmaWidth = temp_MMA_SIZE;
  }
  /* numBytes== 2U */
  else
  {
    mmaWidth = temp_MMA_SIZE / 2U;
  }

  while (blockCounter < numBlocks)
  {
    for (i = 0; i < mmaWidth; i++)
    {
      if (j < inputWidth)
      {
        if ((uint32_t)mmaWidth == (temp_MMA_SIZE / 2U))
        {
          predictedValue |= (uint64_t)((uint64_t)1 << ((uint64_t)i * 2U));
          predictedValue |= (uint64_t)((uint64_t)1 << (((uint64_t)i * 2U) + 1U));
        }
        /* (uint32_t)mmaWidth == temp_MMA_SIZE */
        else
        {
          predictedValue |= (uint64_t)((uint64_t)1 << (uint64_t)i);
        }
        j++;
        k++;
      }
      else if ((j >= inputWidth) && (k <= processWidth))
      {
        j++;
        k++;
      }
      else if ((blockCounter == (numBlocks - 1)) && (((blockCounter * mmaWidth) + i) >= matrixColumn))
      {
        k = 1;
        j = processWidth;
      }
      else
      {
        k = 1;
        j = 0;
        if ((uint32_t)mmaWidth == (temp_MMA_SIZE / 2U))
        {
          predictedValue |= (uint64_t)((uint64_t)1 << ((uint64_t)i * 2U));
          predictedValue |= (uint64_t)((uint64_t)1 << (((uint64_t)i * 2U) + 1U));
        }
        /* (uint32_t)mmaWidth == temp_MMA_SIZE */
        else
        {
          predictedValue |= (uint64_t)((uint64_t)1 << (uint64_t)i);
        }
        j++;
        k++;
      }
    }
    *predRegister = predictedValue;

    predRegister++;
    blockCounter++;
    predictedValue = 0x0;
  }
  return totalBytes;
}

/**
 * @brief Function to store saturation float values
 *
 * @param tidlLayer : Pointer to the common layer parameters
 * @param min : pointer to store min value
 * @param max : pointer to store max value
 * @return None
 */
#ifndef HOST_EMULATION
/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> void TIDL_getSaturationFloat.* <function end>
<justification start> NOT_IN_SCOPE : This function is specific to host emulation (HE) and is intended to be used exclusively in HE builds. It is not applicable or invoked in non-HE environments.
<justification end> */
#endif
void TIDL_getSaturationFloat(
    sTIDL_Layer_t *tidlLayer,
    float32_tidl *min,
    float32_tidl *max)
{
  if (((tidlLayer->actParams.actType == TIDL_NoAct) && (tidlLayer->clipParams.isClipEnabled == 0)) ||
      (tidlLayer->actParams.actType == TIDL_PRelU) ||
      (tidlLayer->actParams.actType == TIDL_GELU) ||
      (tidlLayer->actParams.actType == TIDL_LeakyReLU))
  {
    *max = FLT_MAX;
    *min = -FLT_MAX;
  }
  else if (tidlLayer->actParams.actType == TIDL_RelU)
  {
    *max = FLT_MAX;
    *min = 0.0;
  }
  else if (tidlLayer->actParams.actType == TIDL_RelU6)
  {
    *max = 6.0;
    *min = 0.0;
  }
  else if (tidlLayer->clipParams.isClipEnabled == 1)
  {
    *max = tidlLayer->clipParams.clipMax;
    *min = tidlLayer->clipParams.clipMin;
  }
  else
  {
#ifndef BUILD_WITH_OPENACC
    tidl_printf(0, "actType Not supported in Line %d of %s \n", __LINE__, __FILE__);
    tidl_printf(0, "layerType = %d, tidlLayer->actParams.actType = %d \n", tidlLayer->layerType, tidlLayer->actParams.actType);
#endif
    /*return IALG_EFAIL;*/
  }
  /*return IALG_EOK;*/
}

/**
 * @brief Function to calculate saturation float values
 *
 * @param tidlLayer : Pointer to the common layer parameters
 * @param outAcc : current accumlator value
 */
#ifndef HOST_EMULATION
/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> float32_tidl TIDL_floatSat.* <function end>
<justification start> NOT_IN_SCOPE : This function is specific to host emulation (HE) and is intended to be used exclusively in HE builds. It is not applicable or invoked in non-HE environments.
<justification end> */
#endif
OPENACC(routine(TIDL_floatSat))
float32_tidl TIDL_floatSat(
    float32_tidl outAcc,
    sTIDL_Layer_t *tidlLayer)
{
  float32_tidl temp_outAcc = outAcc;
  float32_tidl min, max;
  TIDL_getSaturationFloat(tidlLayer, &min, &max);
  temp_outAcc = (temp_outAcc > max) ? max : temp_outAcc;
  temp_outAcc = (temp_outAcc < min) ? min : temp_outAcc;
  return temp_outAcc;
}

int32_t TIDL_getProcessingElementSizeInBytes(const sTIDL_Layer_t *tidlLayer)
{
  int32_t procElemSizeInBytes;
  if (tidlLayer->weightsElementSizeInBits <= 8)
  {
    procElemSizeInBytes = (int32_t)1;
  }
  else if (tidlLayer->weightsElementSizeInBits <= 16)
  {
    procElemSizeInBytes = (int32_t)2;
  }
  else
  {
    procElemSizeInBytes = (int32_t)4;
  }

  return procElemSizeInBytes;
}

#if defined TIDL_COVERAGE_DEAD_CODE
int32_t TIDL_conv2dGetKernelDataType(int32_t tidlElemType, int32_t procElemSize)
{
  int32_t mmaDataType;

  if (tidlElemType == (int32_t)TIDL_SignedChar)
  {
    mmaDataType = (uint32_t)TIDL_INT8;
    if (procElemSize == 2)
    {
      mmaDataType = (uint32_t)TIDL_INT16;
    }
  }
  else if (tidlElemType == (int32_t)TIDL_UnsignedChar)
  {
    mmaDataType = (uint32_t)TIDL_UINT8;
    if (procElemSize == 2)
    {
      mmaDataType = (uint32_t)TIDL_UINT16;
    }
  }
  else if (tidlElemType == (int32_t)TIDL_UnsignedShort)
  {
    mmaDataType = (uint32_t)TIDL_UINT16;
  }
  else if (tidlElemType == (int32_t)TIDL_SignedShort)
  {
    mmaDataType = (uint32_t)TIDL_INT16;
  }
  else
  {
    mmaDataType = (uint32_t)TIDL_UINT8;
    if (procElemSize == 2)
    {
      mmaDataType = (uint32_t)TIDL_UINT16;
    }
  }

  return mmaDataType;
}
#endif

#if defined TIDL_COVERAGE_DEAD_CODE
int32_t TIDL_getMmalibDatatypeFromTidlDatatype(int32_t tidlElemType, int32_t procElemSize)
{
  int32_t mmaDataType;

  if (tidlElemType == (int32_t)TIDL_SignedChar)
  {
    mmaDataType = (uint32_t)MMALIB_INT8;
    if (procElemSize == 2)
    {
      mmaDataType = (uint32_t)MMALIB_INT16;
    }
  }
  else if (tidlElemType == (int32_t)TIDL_UnsignedChar)
  {
    mmaDataType = (uint32_t)MMALIB_UINT8;
    if (procElemSize == 2)
    {
      mmaDataType = (uint32_t)MMALIB_UINT16;
    }
  }
  else if (tidlElemType == (int32_t)TIDL_UnsignedShort)
  {
    mmaDataType = (uint32_t)MMALIB_UINT16;
  }
  else if (tidlElemType == (int32_t)TIDL_SignedShort)
  {
    mmaDataType = (uint32_t)MMALIB_INT16;
  }
  else if (tidlElemType == (int32_t)TIDL_SignedWord)
  {
    mmaDataType = (uint32_t)MMALIB_INT32;
  }
  else if (tidlElemType == (int32_t)TIDL_UnsignedWord)
  {
    mmaDataType = (uint32_t)MMALIB_UINT32;
  }
  else if (tidlElemType == (int32_t)TIDL_SignedDoubleWord)
  {
    mmaDataType = (uint32_t)MMALIB_INT64;
  }
  else if (tidlElemType == (int32_t)TIDL_UnsignedDoubleWord)
  {
    mmaDataType = (uint32_t)MMALIB_UINT64;
  }
  else if (tidlElemType == (int32_t)TIDL_SinglePrecFloat)
  {
    mmaDataType = (uint32_t)MMALIB_FLOAT32;
  }

  return mmaDataType;
}

void TIDL_printDMATr(void *dmaTr)
{
  CSL_UdmapTR *tr = (CSL_UdmapTR *)dmaTr;
  tidl_printf(0, "sicnt_0 %16d \n", tr->icnt0);
  tidl_printf(0, "sicnt_1 %16d sdim1 %16d\n", tr->icnt1, tr->dim1);
  tidl_printf(0, "sicnt_2 %16d sdim1 %16d\n", tr->icnt2, tr->dim2);
  tidl_printf(0, "sicnt_3 %16d sdim1 %16d\n", tr->icnt3, tr->dim3);

  tidl_printf(0, "dicnt_0 %16d \n", tr->dicnt0);
  tidl_printf(0, "dicnt_1 %16d ddim1 %16d\n", tr->dicnt1, tr->ddim1);
  tidl_printf(0, "dicnt_2 %16d ddim1 %16d\n", tr->dicnt2, tr->ddim2);
  tidl_printf(0, "dicnt_3 %16d ddim1 %16d\n", tr->dicnt3, tr->ddim3);
}
#endif

/**
 * @brief Check for border pixel
 *
 * @param spatialOffsetY : Offset value in Height dimension
 * @param spatialOffsetX : Offset value in Width dimension
 * @param validPosXMin : Minimum width value to be a valid pixel
 * @param validPosXMax : Maximum width value to be a valid pixel
 * @param validPosYMin : Minimum height value to be a valid pixel
 * @param validPosYMax : Maximum height value to be a valid pixel
 * @return isBorderPixel : Returning true for border pixels
 */
OPENACC(routine(TIDL_checkPixelInPadRegion))
int32_t TIDL_checkPixelInPadRegion(int32_t spatialOffsetY,
                                   int32_t spatialOffsetX,
                                   int32_t validPosXMin,
                                   int32_t validPosXMax,
                                   int32_t validPosYMin,
                                   int32_t validPosYMax)
{
  int32_t isBorderPixel = (int32_t)0;

  if ((spatialOffsetY < validPosYMin) || (spatialOffsetY >= validPosYMax))
  {
    isBorderPixel = (int32_t)1;
  }
  if ((spatialOffsetX < validPosXMin) || (spatialOffsetX >= validPosXMax))
  {
    isBorderPixel = (int32_t)1;
  }

  return isBorderPixel;
}

/**
 * @brief This function provides the core number on which
 * the code is executing
 *
 * @param void : None
 * @retuen : Core number
 */
#if defined TIDL_COVERAGE_DEAD_CODE

uint32_t TIDL_getCoreNum(void)
{
  uint32_t corePacNum = 0;
#if defined(SOC_J784S4) || defined(SOC_J742S2) || defined(SOC_TDA54) // To check if the same field exist in J722S to extract the core number and if so use MULTICORE_MACRO
#if !defined(HOST_EMULATION) && (TIDL_MAX_CORENUM > 1)
#include <c7x.h>
  uint64_t dnum;
  /* Get the bits from bit 7 to bit 15, which represents the core pac number */
  dnum = __DNUM;
  /* Enums are common across devices, subtract enum for C7x_1 since TIDL array indexing starts from C7x_1, CSL has support for other cores so C7x_1 may not be enum 0 */
  corePacNum = CSL_REG64_FEXT(&dnum, C7X_CPU_DNUM_COREPACNUM) - CSL_C7X_CPU_COREPACK_NUM_C7X1;
#endif
#endif
  return corePacNum;
}

#endif

#if defined TIDL_COVERAGE_DEAD_CODE
/*Function to determine the saturation values given a scale and clip value*/
int32_t TIDL_getParametricSatLimit(float32_tidl clipVal, float32_tidl tensorScale, int32_t zeroPoint)
{
  int32_t satVal;
  satVal = round(clipVal / tensorScale) + zeroPoint;
  return satVal;
}
#endif

typedef union
{
  float32_tidl f;
  struct
  {
    uint32_t mantisa : 23;
    uint32_t exponent : 8;
    uint32_t sign : 1;
  } parts;
} float_cast;

#define FLOAT_MANTISSA_PRECISION 23U
#ifdef HOST_EMULATION
void TIDL_GetTensorDimensionsBatchReshape(const sTIDL_Network_t *net,
                                          const void *workloadUnit,
                                          int32_t layerIdx,
                                          int32_t *inBatch,
                                          int32_t *outBatch)
{
  sTIDL_Network_t *tidlNet = (sTIDL_Network_t *)net;
  sTIDL_DataParams_t *inDataParams = TIDL_getDataParams(tidlNet, tidlNet->TIDLLayers[layerIdx].inData[0]);
  sWLMetaDataIDExt_t wlMetaDataExt = {0, 0, 0};
  const sWorkloadUnit_t *wlUnit = (sWorkloadUnit_t *)workloadUnit;

  if (wlUnit != NULL)
  {
    getWLMetaDataIDExt(wlUnit->metadataExt, &wlMetaDataExt);
  }
  if (*inBatch != NULL)
  {
    *inBatch = inDataParams->dimValues[TIDL_DIM_BATCH];
    if (wlUnit != NULL)
    {
      *inBatch = wlMetaDataExt.inNumBatch;
    }
  }
  if (*outBatch != NULL)
  {
    *outBatch = tidlNet->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_BATCH];
    if (wlUnit != NULL)
    {
      *outBatch = wlMetaDataExt.outNumBatch;
    }
  }
  return;
}
#endif
/**
* @brief Helper function to convert float real value to scale and shift. Inputs should be positive float values only.
val can be represented as val = scale*(2^(shift)).
*
* @param val : Input float value
* @param scale : Pointer to the pass the calculated scale value
* @param shift : Pointer to the pass the calculated shift value
* @param precisionBits : Precision to which scale has to calculated

* @return void
*/
#ifndef HOST_EMULATION
/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> void TIDL_convertFloatToScaleAndShift.* <function end>
<justification start> NOT_IN_SCOPE : This function is specific to host emulation (HE) and is intended to be used exclusively in HE builds. It is not applicable or invoked in non-HE environments.
<justification end> */
#endif
OPENACC(routine(TIDL_convertFloatToScaleAndShift))
void TIDL_convertFloatToScaleAndShift(float32_tidl val, int32_t *scale, int32_t *shift, int32_t precisionBits)
{
  float_cast FloatStruct;
  uint32_t mant;

  FloatStruct.f = val;
  mant = FloatStruct.parts.mantisa;

  mant += (1U << FLOAT_MANTISSA_PRECISION);
  mant = (mant) >> (FLOAT_MANTISSA_PRECISION + (uint32_t)1U - (uint32_t)precisionBits);
  *shift = FloatStruct.parts.exponent - 127U - (uint32_t)precisionBits + (uint32_t)1U;
  *scale = (int32_t)mant;
}

#if COMPRESSION_ENABLED
/**** Compression Utilities for Weight compression ******/
/**
----------------------------------------------------------------------------
@ingroup    TIDL_CommonUtils
@fn         TIDL_getUncompressedBitstreamLengthTensor
@brief      This function to calculate tensor input bitstream length for compression.

@param      histogram : Pointer to the histogram. Currently is Int32_t type
            should be changed to Int64_t if necessary.
@param      bitdepth: Tensor bitdepth
@return     bitstream length in bytes
----------------------------------------------------------------------------
*/
int64_t TIDL_getUncompressedBitstreamLengthTensor(const int64_t *histogram)
{
  int32_t histogramLength;
  int32_t i0;
  int64_t samples = 0;
  int64_t bitstreamLengthBits, bitstreamLengthBytes;
  int32_t bitdepth = 8;
  histogramLength = 1U << bitdepth;

  for (i0 = 0; i0 < histogramLength; i0++)
  {
    samples += *(histogram + i0);
  }

  bitstreamLengthBits = (int64_t)samples * (int64_t)bitdepth;
  bitstreamLengthBytes = bitstreamLengthBits / 8;
  bitstreamLengthBytes = ((bitstreamLengthBits % 8) == 0) ? bitstreamLengthBytes : (bitstreamLengthBytes + 1);

  return bitstreamLengthBytes;
}
#endif

//: TODO: Eventually this will be removed
float32_tidl TIDL_getTensorScale(const sTIDL_Network_t *net, int32_t layerIdx)
{
  const sTIDL_DataParams_t *dataParams = &net->TIDLLayers[layerIdx].outData;
  return dataParams->tensorScale;
}

//: TODO: Eventually this will be removed
int32_t TIDL_getTensorZeroPoint(const sTIDL_Network_t *net, int32_t layerIdx)
{
  const sTIDL_DataParams_t *dataParams = &net->TIDLLayers[layerIdx].outData;
  return dataParams->tensorZeroPoint;
}

//: TODO: Eventually this will be removed
int32_t TIDL_getRoundbits(const sTIDL_Network_t *net, int32_t layerIdx)
{
  const sTIDL_DataParams_t *dataParams = &net->TIDLLayers[layerIdx].outData;
  return dataParams->roundBits;
}
#if defined TIDL_COVERAGE_DEAD_CODE
void TIDL_getminMaxValuesHistogram(const int64_t *histogram, int32_t dataType, int32_t *min, int32_t *max)
{
  int32_t bitdepth = 8;
  int32_t histogramLength;
  int32_t i0;
  histogramLength = 1U << bitdepth;

  int32_t smallestValue, largestValue;
  int64_t binCount;
  int64_t minBinCount = INT64_MAX, maxBinCount = 0;

  if (dataType == 0) // unsigned
  {
    for (i0 = 0; i0 < histogramLength; i0++)
    {
      binCount = *(histogram + i0);
      if (binCount < minBinCount)
      {
        smallestValue = i0;
        minBinCount = binCount;
      }
      if (binCount > maxBinCount)
      {
        largestValue = i0;
        maxBinCount = binCount;
      }
    }
  }
  else
  {
    smallestValue = (-histogramLength / 2) - 1;
    for (i0 = -histogramLength / 2; i0 < histogramLength; i0++)
    {
      int32_t idx = (i0 <= 0) ? -2 * i0 : (2 * i0) - 1;
      binCount = *(histogram + idx);
      binCount = *(histogram + i0);
      if (smallestValue == ((-histogramLength / 2) - 1) && binCount > 0)
      {
        smallestValue = i0;
      }
      if (binCount > 0)
      {
        largestValue = i0;
      }
    }
  }
  *min = smallestValue;
  *max = largestValue;
}
#endif
#if COMPRESSION_ENABLED
/**
----------------------------------------------------------------------------
@ingroup    TIDL_CommonUtils
@fn         TIDL_getBitstreamLengthDRUCodingZeroBasedScheme
@brief      This function calculates tensor bitstream length when encoded as
            zero based compression scheme supported by the DRU.

@param      histogram : Pointer to the histogram. Currently is Int32_t type
            should be changed to Int64_t if necessary.
@param      bitdepth: Tensor bitdepth
@param      bias:  pointer to the varibale in which the selected bias
            value is to be returned. Null will implicitly assume bias to 0
@return     bitstream length in bytes
----------------------------------------------------------------------------
*/
int64_t TIDL_getBitstreamLengthDRUCodingZeroBasedScheme(const int64_t *histogram, int32_t elementType, int32_t *bias = NULL)
{
  int32_t bitdepth = 8;
  int32_t histogramLength;
  int32_t i0;
  int64_t numNonZeros = 0;
  int32_t nonZeroSymbolSize;
  int64_t bitstreamLengthBits, bitstreamLengthBytes;
  int32_t selectedBias = 0;
  nonZeroSymbolSize = (bitdepth == 8) ? 9 : 17;
  histogramLength = 1U << bitdepth;
  if (bias != NULL)
  {
    int64_t maxHistValue = 0;
    for (i0 = 0; i0 < histogramLength; i0++)
    {
      if (*(histogram + i0) > maxHistValue)
      {
        maxHistValue = *(histogram + i0);
        selectedBias = i0;
      }
    }
    *bias = (elementType != TIDL_UnsignedChar) ? (selectedBias - 128) : selectedBias;
  }

  for (i0 = 0; i0 < histogramLength; i0++)
  {
    if (i0 != selectedBias)
    {
      numNonZeros += *(histogram + i0);
    }
  }
  bitstreamLengthBits = (int64_t)(*(histogram + selectedBias)) + (numNonZeros * (int64_t)nonZeroSymbolSize);
  bitstreamLengthBytes = bitstreamLengthBits / 8;
  bitstreamLengthBytes = ((bitstreamLengthBits % 8) == 0) ? bitstreamLengthBytes : (bitstreamLengthBytes + 1);

  return bitstreamLengthBytes;
}

/**
----------------------------------------------------------------------------
@ingroup    TIDL_CommonUtils
@fn         TIDL_getExpGolombCodeLength
@brief      This function returns the length of the VLC for input symbol.

@param      symbol : Symbol for which the exponential golomb code length is to be
            calculated.
@return     length of the exponential golomb code for the given symbol
----------------------------------------------------------------------------
*/
int32_t TIDL_getExpGolombCodeLength(const int32_t symbol)
{
  int32_t codeLength = 0;
  codeLength = 2 * int32_t(floor(log2f(float32_tidl(symbol + 1)))) + 1;
  return codeLength;
}
#if defined TIDL_COVERAGE_DEAD_CODE
/**
----------------------------------------------------------------------------
@ingroup    TIDL_CommonUtils
@fn         TIDL_DRUbiasSubtraction
@brief      This function replicates the DMAUtils bias subtraction logic.

@param      symbol : Pointer to the histogram. Currently is Int32_t type
            should be changed to Int64_t if necessary.
@param      bias: Tensor bitdepth
----------------------------------------------------------------------------
*/
inline int32_t TIDL_DRUCompBiasSubtraction(int32_t symbol, int32_t bias)
{
  uint32_t length = 8;
  uint32_t temp = symbol - bias;
  /*prevent underflowing*/
  temp &= (1U << (length)) - 1U;
  return temp;
}
#endif

/**
----------------------------------------------------------------------------
@ingroup    TIDL_CommonUtils
@fn         TIDL_ExpGolombSignedToUnsignedRemapping
@brief      This function remaps signed integer values to unsigned integer
            valus required for exponential Golomb coding.

@param      value : Value to be remapped.
@return     Remapped value
----------------------------------------------------------------------------
*/
inline int32_t TIDL_ExpGolombSignedToUnsignedRemapping(int32_t value1)
{
  int32_t remappedVal;
  if (value1 > 0)
  {
    remappedVal = ((2 * value1) - 1);
  }
  else
  {
    remappedVal = (-2 * value1);
  }
  return (remappedVal);
}

/**
----------------------------------------------------------------------------
@ingroup    TIDL_CommonUtils
@fn         TIDL_getBitstreamLengthDRUExpGolombCodingScheme
@brief      This function calculates tensor bitstream length when encoded as
            exponential golomb coding scheme supported by the DRU.

@param      histogram : Pointer to the histogram. Currently is Int32_t type
            should be changed to Int64_t if necessary.
@param      bitdepth: Tensor bitdepth
@param      bias:  To be added
@return     bitstream length in bytes
----------------------------------------------------------------------------
*/

int64_t TIDL_getBitstreamLengthDRUExpGolombCodingScheme(const int64_t *histogram, int32_t elementType, int32_t bias)
{
  int32_t bitdepth = 8;
  int32_t histogramLength;
  int32_t i0;
  int64_t symbolBitwidth, bitstreamLengthBits = 0, bitstreamLengthBytes;
  int64_t temp_bitstreamLengthBytes = 0;
  histogramLength = 1U << bitdepth;

  for (i0 = 0; i0 < histogramLength; i0++)
  {
    int32_t symbol;
    if (elementType != TIDL_UnsignedChar)
    {
      symbol = TIDL_ExpGolombSignedToUnsignedRemapping(TIDL_DRUCompBiasSubtraction((i0 - 128), bias));
    }
    else
    {
      symbol = TIDL_DRUCompBiasSubtraction(i0, bias);
    }
    symbolBitwidth = TIDL_getExpGolombCodeLength(symbol);
    bitstreamLengthBits += (int64_t)(*(histogram + i0)) * (int64_t)(symbolBitwidth);
  }
  bitstreamLengthBytes = bitstreamLengthBits / 8;
  if (((bitstreamLengthBits % 8) == 0))
  {
    temp_bitstreamLengthBytes = bitstreamLengthBytes;
  }
  else
  {
    temp_bitstreamLengthBytes = bitstreamLengthBytes + 1;
  }
  bitstreamLengthBytes = temp_bitstreamLengthBytes;
  return bitstreamLengthBytes;
}

void TIDL_computeHistogram1D(int64_t *histogram, uint8_t *bufferPtr, uint64_t bufLength, int32_t elementType)
{
  /*Assumption that histogram buffer has been allocated by the function calling computeHistogram (e.g. 8*256 bytes for 8-bit datatype)*/
  int32_t offset = 0;
  int32_t unsignedVal;
  if (elementType == TIDL_SignedChar)
  {
    offset = 128;
  }
  if ((elementType == TIDL_SignedShort) || (elementType == TIDL_UnsignedShort))
  {
    offset = 128;
  }
  int32_t i;
  for (i = 0; i < bufLength; i++)
  {
    unsignedVal = (int32_t)(bufferPtr[i]) + offset;
    *(histogram + unsignedVal) += 1;
  }
}

void TIDL_computeCompressionAlgo(uint8_t *bufferPtr, uint64_t bufLength, int32_t elementType, int32_t *retBias, int32_t *retAlg)
{
  // 8-bit:
  int32_t bitdepth = 8;
  int32_t i;
  int64_t *histogram;
  int32_t min, max;
  int32_t bias = NULL;
  int64_t histogramBuffer[256];
  histogram = &histogramBuffer[0];
  (void)memset(histogram, 0, (uint32_t)8 * ((uint32_t)1 << (uint32_t)bitdepth));
  TIDL_computeHistogram1D(histogram, bufferPtr, bufLength, elementType);
  TIDL_getminMaxValuesHistogram(histogram, elementType, &min, &max);
  // Compression ratio:
  int64_t uncompressedLength = TIDL_getUncompressedBitstreamLengthTensor(histogram);
  int64_t zbLen = TIDL_getBitstreamLengthDRUCodingZeroBasedScheme(histogram, elementType, &bias);
  // Iterate over possible egBiases:
  int32_t egBias = 0;
  int64_t egLen, minLen = INT64_MAX;
  for (i = 0; i < (1U << bitdepth); i++)
  {
    egLen = TIDL_getBitstreamLengthDRUExpGolombCodingScheme(histogram, elementType, i);
    if (egLen < minLen)
    {
      egBias = i;
      minLen = egLen;
    }
  }
  egLen = TIDL_getBitstreamLengthDRUExpGolombCodingScheme(histogram, elementType, egBias);
  tidl_printf(2, "Compression Factors: ZB = %f, Bias = %d\n", (((float32_tidl)zbLen) / uncompressedLength), bias);
  tidl_printf(2, "Compression Factors: EG = %f, Bias = %d\n", (((float32_tidl)egLen) / uncompressedLength), egBias);
  if ((egLen < zbLen) && (egLen < uncompressedLength))
  {
    *retAlg = 1;
    *retBias = egBias;
  }
  else if ((zbLen < egLen) && (zbLen < uncompressedLength))
  {
    *retAlg = 0;
    *retBias = bias;
  }
  else
  {
    *retAlg = -1;
  }
}

#define TIDL_MAX_SBSIZE (512U * 1024U)
int32_t TIDL_set1DCompressionProperties(int32_t bufSize, sTIDL_compressionDimensions *compDim)
{
  int32_t status = TIDL_SUCCESS;
  int32_t remainingBufSize = 0;
  int32_t pow2Factor = 1;
  int32_t nonPow2Factor = 1U;

  /*Single Superblock Case:*/
  if (((bufSize % 16U) != 0) || (bufSize == 0))
  {
    /*Innermost count has to be a multiple of 2*/
    status = TIDL_ERR_INVALID_VALUE;
  }

  /*Divide such that sbICNT0 is a power of 2 value < 64K*/
  if (TIDL_SUCCESS == status)
  {
    remainingBufSize = bufSize / 16U;
    while (((remainingBufSize % pow2Factor) == 0) && (pow2Factor < remainingBufSize))
    {
      pow2Factor *= 2U;
    }
    pow2Factor /= 2U; /*Correct the factor*/
    nonPow2Factor = remainingBufSize / pow2Factor;
    /*BufSize = 16 x pow2Factor x nonPow2Factor*/
    if (bufSize > TIDL_MAX_SBSIZE)
    {
      if (pow2Factor == 1U)
      {
        status = TIDL_ERR_INVALID_VALUE;
      }
      else
      {
        /*Need to maximize superblock size*/
        int32_t pow2subFactor = 1U;
        while (pow2Factor > 2048U)
        {
          pow2subFactor *= 2U;
          pow2Factor /= 2U;
        }
        /*16 x pow2Factor | pow2subFactor | nonPow2Factor*/
        compDim->sbIcnt0 = 16U * pow2Factor;
        compDim->sbIcnt1 = pow2subFactor;
        compDim->sIcnt0 = nonPow2Factor;
        compDim->sbDim1 = compDim->sbIcnt0;
        compDim->sDim0 = compDim->sbDim1 * compDim->sbIcnt1;
      }
    }
    else
    {
      int32_t pow2subFactor = 1U;
      while (pow2Factor > 2048U)
      {
        pow2subFactor *= 2U;
        pow2Factor /= 2U;
      }
      /*16 x pow2Factor | pow2subFactor | nonPow2Factor*/
      compDim->sbIcnt0 = 16U * pow2Factor;
      compDim->sbIcnt1 = pow2subFactor * nonPow2Factor;
      compDim->sIcnt0 = 1U;
      compDim->sbDim1 = compDim->sbIcnt0;
      compDim->sDim0 = 0U;
    }
  }

  return status;
}

int32_t TIDL_compressFlatSW(uint8_t *const pInput,
                            uint8_t *const pOutput,
                            uint8_t *const sectr,
                            uint8_t *const cdbTable,
                            uint8_t *trMem,
                            sTIDL_compressionDimensions *pCompDims)
{

  int32_t retVal = CSL_PASS;

  uint32_t TrSize;

  DmaUtilsAutoInc3d_TrPrepareParam trPrepParam;
  DmaUtilsAutoInc3d_TransferProp transferProp;

  TrSize = DmaUtilsAutoInc3d_getTrMemReq(1U);

  trPrepParam.channelId = 0U; // Dummy Value
  trPrepParam.numTRs = 1U;
  trPrepParam.trMem = trMem;
  trPrepParam.trMemSize = TrSize;
  transferProp.syncType = DMAUTILSAUTOINC3D_SYNC_2D;
  transferProp.dmaDfmt = DMAUTILSAUTOINC3D_DFMT_COMP;
  transferProp.transferDim.sicnt0 = pCompDims->sIcnt0;
  transferProp.transferDim.sicnt1 = 1U;
  transferProp.transferDim.sicnt2 = 1U;
  transferProp.transferDim.sicnt3 = 1U;

  transferProp.transferDim.dicnt0 = pCompDims->sIcnt0;
  transferProp.transferDim.dicnt1 = 1U;
  transferProp.transferDim.dicnt2 = 1U;
  transferProp.transferDim.dicnt3 = 1U;

  transferProp.circProp.circSize1 = 0;
  transferProp.circProp.circSize2 = 0;
  transferProp.circProp.addrModeIcnt0 = 0;
  transferProp.circProp.addrModeIcnt1 = 0;
  transferProp.circProp.addrModeIcnt2 = 0;
  transferProp.circProp.addrModeIcnt3 = 0;
  transferProp.circProp.circDir = 0;

  transferProp.cmpProp.cmpAlg = pCompDims->algorithm;
  transferProp.cmpProp.cmpBias = pCompDims->bias;
  transferProp.cmpProp.varKStartK = 0;   // generic not supported
  transferProp.cmpProp.varKUpdateK = 0;  // generic not supported
  transferProp.cmpProp.varKElemSize = 0; // generic not supported
  transferProp.cmpProp.varKSubType = 0;  // generic not supported
  transferProp.cmpProp.varKSubSel0 = 0;  // generic not supported
  transferProp.cmpProp.varKSubSel1 = 0;  // generic not supported
  transferProp.cmpProp.varKSubSel2 = 0;  // generic not supported
  transferProp.cmpProp.varKSubSel3 = 0;  // generic not supported
  transferProp.cmpProp.sbIcnt0 = pCompDims->sbIcnt0 / 16;
  transferProp.cmpProp.sbIcnt1 = pCompDims->sbIcnt1; // 2 rows per superblock
  transferProp.cmpProp.sbDim1 = pCompDims->sbDim1;
  transferProp.cmpProp.sbAM0 = 0;
  transferProp.cmpProp.sbAM1 = 0;

  transferProp.transferDim.sdim1 = 0;
  transferProp.transferDim.sdim2 = 0;
  transferProp.transferDim.sdim3 = 0;

  transferProp.transferDim.ddim1 = 0U; // table dims
  transferProp.transferDim.ddim2 = 0;
  transferProp.transferDim.ddim3 = 0;

  transferProp.cmpProp.sDim0 = pCompDims->sDim0;
  transferProp.cmpProp.dDim0 = 8U; /*CDB Entry Size*/

  transferProp.ioPointers.srcPtr = pInput;
  transferProp.ioPointers.dstPtr = pOutput;
  transferProp.ioPointers.strPtr = sectr;
  transferProp.ioPointers.cdbPtr = cdbTable;

  retVal = DmaUtilsAutoInc3d_prepareTr(&trPrepParam, &transferProp);
  if (retVal != CSL_PASS)
  {
    goto Exit;
  }

  DmaUitlsAutoInc3d_CompressSW(trMem);

Exit:
{
  return retVal;
}
}
#endif /*  End of "COMPRESSION_ENABLED" */

/* #define SYNCHRONIZATION_DEBUG */

/** Sync function for multi-core execution */
#if TIDL_DEVICE_MULTICORE
void TIDL_syncWl(TIDL_sharedBufferProperties *ctrlSetArgs, int32_t wlToPoll[], int32_t syncPoint, int32_t wlExecId, uint32_t coreId, int32_t numCores)
{
#ifdef SYNCHRONIZATION_DEBUG
  printf("Core Id %d Wl Exec Id -- %d ...starting sync \n", coreId, wlExecId);
#endif
  volatile int8_t *syncBuf;
  volatile int8_t updatedStateToWrite = NOT_VALID; /* don't update state if this value is NOT_VALID */
  volatile int8_t stateToPoll = NOT_VALID;         /* no need to poll/sync if this value is NOT_VALID */
  volatile int32_t wlToPollInt[TIDL_MAX_NUM_CORES];
  int8_t syncPtrOffset = (int8_t)TIDL_GLOBAL_SYNC_OFFSET;
  (void)memset((void *)wlToPollInt, 0, (uint32_t)TIDL_MAX_NUM_CORES * sizeof(int32_t));

  if (syncPoint == (int32_t)TIDL_SYNC_POINT_START)
  {
    updatedStateToWrite = NOT_VALID;
    for (int32_t i = 0; i < numCores; i++)
    {
      wlToPollInt[i] = wlToPoll[i]; /* wlToPoll will come as dependId in the function arguments */
    }
    stateToPoll = TIDL_SYNC_WL_STATE_COMPLETE;
  }
  else if (syncPoint == (int32_t)TIDL_SYNC_POINT_DMA_READY)
  {
    updatedStateToWrite = TIDL_SYNC_WL_STATE_DMA_READY;                                  /* current core state to be updated to indicate it is ready to start cross core DMA */
    syncBuf = (int8_t *)ctrlSetArgs[coreId].syncBufferPtr[TIDL_Virtual] + syncPtrOffset; /* Current core's sync buffer */
    syncBuf[wlExecId] = updatedStateToWrite;                                             /* Update the state for current core as soon as code reaches this point */

    stateToPoll = (uint32_t)TIDL_SYNC_WL_STATE_DMA_READY | (uint32_t)TIDL_SYNC_WL_STATE_COMPLETE; /* Just polling on TIDL_SYNC_WL_STATE_DMA_READY can result in race condition
    if one of the cores' passes sync polling condition and completes execution(TIDL_SYNC_WL_STATE_COMPLETE) before other cores have consumed TIDL_SYNC_WL_STATE_DMA_READY state*/
    for (int32_t i = 0; i < numCores; i++)
    {
      wlToPollInt[i] = wlExecId; /* Should poll on current layer id across cores to check if they are ready for DMA / have completed execution */
    }
    updatedStateToWrite = NOT_VALID; /* State already updated above, no need to update again */
  }
  else if (syncPoint == (int32_t)TIDL_SYNC_POINT_COMPLETE)
  {
    updatedStateToWrite = TIDL_SYNC_WL_STATE_COMPLETE;
    stateToPoll = NOT_VALID; /* No need to poll/sync here - just update the state */
  }
  else if (syncPoint == (int32_t)TIDL_SYNC_POINT_GLOBAL_SYNC)
  {
    updatedStateToWrite = TIDL_SYNC_WL_STATE_RESET;
    syncBuf = (int8_t *)ctrlSetArgs[coreId].syncBufferPtr[TIDL_Virtual]; /* Current core's sync buffer */
    syncBuf[0] = updatedStateToWrite;                                    /* Update the state for current core as soon as code reaches this point */
    for (int32_t i = 0; i < numCores; i++)
    {
      wlToPollInt[i] = 0; /* Poll at offset 0 in sync buf which is reserved for global sync */
    }
    stateToPoll = (uint32_t)TIDL_SYNC_WL_STATE_RESET | (uint32_t)TIDL_SYNC_WL_STATE_DMA_READY | (uint32_t)TIDL_SYNC_WL_STATE_COMPLETE; /* Poll on any of the states >= current state */
    updatedStateToWrite = NOT_VALID;                                                                                                   /* State already updated above, no need to update again */
    syncPtrOffset = 0;
  }
  else
  {
    tidl_printf(0, "Incorrect sync point - %d , continuing without sync\n", syncPoint);
    stateToPoll = NOT_VALID;
  }

#ifdef SYNCHRONIZATION_DEBUG
  tidl_printf(0, "coreId %d syncPoint - %d  stateToPoll - %d updatedStateToWrite - %d wlToPollInt - %d %d \n", coreId,
              syncPoint, stateToPoll, updatedStateToWrite, wlToPollInt[0], wlToPollInt[1], wlToPollInt[2], wlToPollInt[3]);
#endif

  int32_t waitCondition = 0;
  int32_t flag = 0;
  while (waitCondition == 0)
  {
    for (int32_t i = 0; (i < numCores) && (flag == 0); i++)
    {
      waitCondition = 1;
      if (stateToPoll != NOT_VALID)
      {
        if (wlToPollInt[i] != NOT_VALID) /* can be NOT_VALID for don't care condition */
        {
          syncBuf = (int8_t *)ctrlSetArgs[i].syncBufferPtr[TIDL_Virtual] + syncPtrOffset;
          if (((uint16_t)syncBuf[wlToPollInt[i]] & (uint16_t)stateToPoll) != (uint16_t)syncBuf[wlToPollInt[i]])
          {
#ifdef SYNCHRONIZATION_DEBUG
            tidl_printf(0, "Core id %d Wl Exec id - %d , Polling on - %d , wlToPoll %d buffer val[%d] %d \n", coreId, wlExecId, stateToPoll, wlToPollInt[i], i, syncBuf[wlToPollInt[i]]);
#endif
            waitCondition = 0;
            flag = 1;
          }
        }
      }
      else
      {
        flag = 1;
      }
    }
    flag = 0;
    if (waitCondition != 0)
    {
      syncBuf = (int8_t *)ctrlSetArgs[coreId].syncBufferPtr[TIDL_Virtual] + syncPtrOffset;
      if (updatedStateToWrite != NOT_VALID)
      {
#ifdef SYNCHRONIZATION_DEBUG
        tidl_printf(0, "Core Id %d Wl Exec id - %d Writing state %d \n", coreId, wlExecId, updatedStateToWrite);
#endif
        syncBuf[wlExecId] = updatedStateToWrite;
      }
    }
  }
}
#endif

#if defined TIDL_COVERAGE_DEAD_CODE
int32_t TIDL_isConcatBalanced(TIDL_NetworkCommonParams *commonParams, sTIDL_Layer_t *layer, sTIDL_AlgLayer_t *algLayer, int32_t layerIdx)
{
  /*Concat with different tensor sizes being concatenated along the width and height dimensions aren't
    supported*/
  int32_t isSupported = 1;
  const sTIDL_ConcatParams_t &concatParams = layer->layerParams.concatParams;
  if (layer->layerType == TIDL_ConcatLayer)
  {
    if ((concatParams.axis == TIDL_DIM_HEIGHT) || (concatParams.axis == TIDL_DIM_WIDTH))
    {
      int32_t inBufIdx;
      sTIDL_DataParams_t *inDataParams = &commonParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[0]].outData;
      int32_t width = inDataParams->dimValues[TIDL_DIM_WIDTH];
      int32_t height = inDataParams->dimValues[TIDL_DIM_HEIGHT];
      /*Loop from the second buffer onwards and check if width or height change:*/
      for (inBufIdx = 1; inBufIdx < layer->numInBufs; inBufIdx++)
      {
        sTIDL_DataParams_t *inDataParams = &commonParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[inBufIdx]].outData;
        if ((width != inDataParams->dimValues[TIDL_DIM_WIDTH]) ||
            (height != inDataParams->dimValues[TIDL_DIM_HEIGHT]))
        {
          isSupported = 0;
          break;
        }
      }
    }
  }
  return isSupported;
}
#endif
/*
sTIDL_DataParams_t * TIDL_getDataParams(sTIDL_Network_t * pTIDLNetStructure,
int32_t dataId)
{
  int32_t i,j;
  sTIDL_DataParams_t *dataPtr = NULL;
  if(pTIDLNetStructure->numLayers < TIDL_NUM_MAX_LAYERS)
  {
    for (i = 0 ; i < pTIDLNetStructure->numLayers; i++)
    {
      for (j = 0; j < pTIDLNetStructure->TIDLLayers[i].numOutBufs; j++)
      {
        if(pTIDLNetStructure->TIDLLayers[i].outData.dataId == dataId)
        {
          dataPtr = &(pTIDLNetStructure->TIDLLayers[i].outData);
          break;
        }
      }
      if(dataPtr != NULL)
      {
        break;
      }
    }
  }
  return(dataPtr);
}*/
#ifndef HOST_EMULATION
/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> int32_t float_to_int_c7x.* <function end>
<justification start> NOT_IN_SCOPE : This function is specific to host emulation (HE) and is intended to be used exclusively in HE builds. It is not applicable or invoked in non-HE environments.
<justification end> */
#endif
int32_t float_to_int_c7x(float32_tidl x)
{
  int32_t x_int = (int32_t)(x);
  // Added handling of extreme cases of int_32 data type by clipping
  int32_t outMax = 0;
  outMax = std::numeric_limits<int32_t>::max();
  int32_t outMin = 0;
  outMin = std::numeric_limits<int32_t>::lowest();
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  x_int = (x > outMax) ? outMax : x_int;
  x_int = (x < outMin) ? outMin : x_int;
  /* LDRA_JUSTIFY_END */

#ifdef TIDL_DEBUG_C7x_ROUND
  int32_t x_int_c7x = __float_to_int(x);
#endif
  /*
    round makes I.5f data to (I+1) , so -1.5 becomes -2, and 1.5 becomes 2.0
    whereas c7x intrinsic __float_to_int, does diffrent job for positive and negative
    -6.5 becomes -6 and 6.5 becomes 6 --> in round() this output willbe -7 and 7 --> correction needed
    -7.5 becomes -8 and 7.5 becomes 8 --> in round() this output willbe -8 and 8 --> no correction needed
    so there is need of correction when fraction part is 0.5 and integer part is even before doing round
  */
  if ((TIDL_ABS_FLOAT(x - float32_tidl(x_int)) > 0.5f) || ((TIDL_ABS_FLOAT(x - float32_tidl(x_int)) == 0.5f) && (((uint32_t)x_int & (uint32_t)0x1) == (uint32_t)0x1)))
  {
    if (x > 0.0f)
    {
      x_int += 1;
    }
    else
    {
      x_int -= 1;
    }
  }

#ifdef TIDL_DEBUG_C7x_ROUND
  if (x_int != x_int_c7x)
  {
    printf("ci and ref mismatch is likely for float value %f , %d, %d \n", x, x_int, x_int_c7x);
  }
#endif

  return x_int;
}

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
int32_t TIDL_isOutDataBuff(const sTIDL_Network_t *pTIDLNetStructure,
                           int32_t dataId, int32_t layersGroupId)
{
  int32_t i, j;
  int32_t status = 0;
#ifndef HOST_EMULATION
/* LDRA_JUSTIFY_START
   <metric start> statement branch <metric end>
   <justification start>  LDRA_JUSTIFICATION_ALGUTILS_SAFETY_CHECK : safe programming hard to hit this condition with real world data
   <justification end> */
#endif
  if (pTIDLNetStructure->numLayers >= TIDL_NUM_MAX_LAYERS)
  {
    status = -1;
  }
  else
#ifndef HOST_EMULATION
/* LDRA_JUSTIFY_END */
#endif
  {
    for (i = 0; i < pTIDLNetStructure->numLayers; i++)
    {
      for (j = 0; j < pTIDLNetStructure->TIDLLayers[i].numInBufs; j++)
      {
        if ((pTIDLNetStructure->TIDLLayers[i].layersGroupId != layersGroupId) &&
            (pTIDLNetStructure->TIDLLayers[i].inData[j] == dataId))
        {
          status = 1;
          break;
        }
      }
      if (status == 1)
      {
        break;
      }
    }
  }

  return status;
}
