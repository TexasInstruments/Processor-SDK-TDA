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
@file    tidl_traceUtils.h
@brief   This file contains TILD common stats utilities
@version 0.1 (May 2025) : Initial version [ADK]
----------------------------------------------------------------------------
*/

#ifndef TIDL_TRACEUTILS_STATS_H_
#define TIDL_TRACEUTILS_STATS_H_

#include "itidl_ti.h"
#include "tidl_types.h"
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <limits>

typedef int32_t(*tidlVprintf_t)(const char * format, va_list arg);
typedef int32_t(*tidlWriteBinToFile_t)(const char * fileName, void * addr, int32_t size, int32_t count);
typedef int32_t(*tidlReadBinFromFile_t)(const char * fileName, void * addr, int32_t size, int32_t count);

static int32_t tidlLogLevel;
static int32_t tidlWriteLevel;
static int32_t tidlDdrPerfStatsEnabled;
static tidlVprintf_t tidlVprintf;
static tidlWriteBinToFile_t tidlWriteBinToFile;
static tidlReadBinFromFile_t tidlReadBinFromFile;
static char tidlTraceBaseName[TIDL_STRING_SIZE];

#define TIDL_NUM_ACTIVATION_HISTOGRAM_BINS (1024U)

#define TRACE_STRING_SIZE (1024U)
#define TRACE_STRINGS_MEM_SIZE (3U*1024U)

/* Define TRUE/FALSE to go with Bool */
#ifndef TRUE

#define TRUE		( 1)
#define FALSE		( 0)

#endif

int32_t tidl_getTraceLogLevel(void);
int32_t tidl_getDdrPerfStatsEnabled(void);

int32_t TIDL_initDebugTraceParams(int32_t traceLogLevel, int32_t traceWriteLevel, tidlVprintf_t tifVprintfFuncPtr, tidlWriteBinToFile_t writeBinFuncPtr,
                                    tidlReadBinFromFile_t readBinFuncPtr, const char * traceBaseName, int32_t enableDDRPerfStats);

void TIDL_getSaturationLimits(int32_t elementType, int32_t *satLow, int32_t *satHigh);

void tidl_setWriteLevel(int32_t val);

int32_t tidl_getWriteLevel(void);

int32_t tidl_getTraceLogLevel(void);

tidlWriteBinToFile_t tidl_getWriteBinToFile(void);

tidlVprintf_t tidl_getVprintf(void);

tidlWriteBinToFile_t tidl_getWriteBinToFileStats(void);

tidlReadBinFromFile_t tidl_getReadBinFromFile(void);

void* tidl_getTraceBaseName(void);

/* changing to int32_t explicist cast to avoid MISRA-C issues */
static inline int32_t TIDL_isKernelHighPrecision(int32_t layerKernelType)
{
  const int32_t Precision[2] = {(int32_t)1, (int32_t)0};
 /* Adding original implementation for readability
 if(layerKernelType == TIDL_HighPrecisionKernel)
 {
   isHighPrecision = TRUE;
 }
 */
  return Precision[layerKernelType];
}

#endif

