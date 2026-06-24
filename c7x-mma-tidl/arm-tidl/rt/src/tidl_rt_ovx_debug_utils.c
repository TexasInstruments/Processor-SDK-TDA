/*
*
* Copyright (c) {2015 - 2022} Texas Instruments Incorporated
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

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "itidl_ti.h"
#include "tidl_rt_profile.h"
#include "tidl_rt_ovx_debug_utils.h" //Added this header file to include TIDLRT_LogMetaData prototype

#if defined (SOC_TDA54)
  #define TDA5_C7X_FREQ_MHZ      (1500U)
#endif

static sTIDLProfilePrintInfo_t gCyclesProfilePrintInfo[] =
{
  {"Layer"                      ,  -1},
  {"LayerType"                  ,  -2},
  {"Layer Cycles"               ,  TIDL_PROFILE_LAYER},
  {"kernelOnlyCycles"           ,  TIDL_PROFILE_KERNEL_ONLY},
  {"coreLoopCycles"             ,  TIDL_PROFILE_CORE_LOOP},
  {"LayerSetupCycles"           ,  TIDL_PROFILE_LAYER_SETUP},
  {"dmaPipeupCycles"            ,  TIDL_PROFILE_DMA_PIPEUP},
  {"LayerHandleCopy"            ,  TIDL_PROFILE_LAYER_SETUP_HANDLE_COPY},
  {"RestoreCycles"              ,  TIDL_PROFILE_RESTORE},
  {"Multic7xContextCopyCycles"  ,  TIDL_PROFILE_CONTEXT_CROSSCORE_COPY},
  {"DDRBWReadInBytes"           ,  TIDL_PROFILE_DDR_BW_READ},
  {"DDRBWWriteInBytes"          ,  TIDL_PROFILE_DDR_BW_WRITE},
  {"ShapeInferCycles"           ,  TIDL_PROFILE_SHAPE_INFER}
};

uint64_t TIDLRT_ReadProfileData(const TIDL_LayerMetaData *pMetaData, int32_t profileIdx, int32_t layerNum);

uint64_t TIDLRT_ReadProfileData(const TIDL_LayerMetaData *pMetaData, int32_t profileIdx, int32_t layerNum)
{
  uint64_t retVal = (uint64_t) layerNum;
  /* LDRA_JUSTIFY_START
  <metric start> branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  TIDLRT_ReadProfileData is always called with valid profileIdx in
  TIDLRT_LogMetaData with the macro TIDL_PROFILE_LAYER (0)
  <justification end> */
  if(profileIdx != -1)
  /* LDRA_JUSTIFY_END */
  {
    retVal = pMetaData[layerNum].profilePoint[profileIdx];
  }
  return retVal;
}

/* Print profiling meta data to console and also log it to a debug performance trace file */
/* This function is also called by TVM C7x runtime on C7x, where there is no filesystem.
 * Set baseName to NULL for cases where you don't want to log to file. */
void TIDLRT_LogMetaData(const TIDL_outArgs *outArgsPtr, char* baseName)
{
  char traceFileName[512] = {0};
  int32_t numProfilePoints, i, j;
  uint64_t profileVal;
  unsigned long int sumOfLayerCycles = 0;
#if defined (SOC_TDA54) 
  double ddrBandwidthRead = 0;
  double ddrBandwidthWrite = 0;
#endif

  FILE *fp = NULL;
  /*Add error handling:*/
  if ((baseName != NULL) && (baseName[0] != '\0'))
  {
    (void )snprintf(traceFileName, 512, "%s%s", (char *)baseName, "perf.csv");
    fp = fopen(&traceFileName[0],"w");
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    Users are not expected to infer same model parallelly on multiple threads
    causing multiple writers and fp to be NULL.
    (TIDLRT_LogMetaData is called once per model inference)
    <justification end> */
    if(fp == NULL)
    {
      printf("Error opening performance debug file\n");
    }
    /* LDRA_JUSTIFY_END */
  }

  numProfilePoints = (int32_t)(sizeof(gCyclesProfilePrintInfo)/sizeof(sTIDLProfilePrintInfo_t));
  for ( j= 0; j < numProfilePoints; j++)
  {
    printf("%18s,", gCyclesProfilePrintInfo[j].string);
    if (fp != NULL)
    {
      (void)fprintf(fp, "%18s,", gCyclesProfilePrintInfo[j].string);
    }
  }
  printf("\n");
  if (fp != NULL)
  {
    (void)fprintf(fp, "\n");
  }

  for(i = 0 ; i < outArgsPtr->numLayers ; i++)
  {
    /* Data layers will get assigned execId = -1 by c7x-tidl */
    if (outArgsPtr->metaDataLayer[i].layerExecId != -1)
    {
      for ( j= 0; j < numProfilePoints; j++)
      {
        int32_t profileIdx = gCyclesProfilePrintInfo[j].profileIdx ;
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
        profileIdx value is never set to -1 in the gCyclesProfilePrintInfo table.
        All macros defined are non-negative (TIDL_PROFILE_* values).
        <justification end> */
        profileVal = (profileIdx == -1) ? (uint64_t)(outArgsPtr->metaDataLayer[i].layerExecId) :
                     (profileIdx == -2) ? (uint64_t)(outArgsPtr->metaDataLayer[i].layerType) :
                     (uint64_t)(outArgsPtr->metaDataLayer[i].profilePoint[profileIdx]);
        /* LDRA_JUSTIFY_END */
        printf("%18" PRIu64 ",", profileVal);
        if (fp != NULL)
        {
          (void)fprintf(fp, "%18" PRIu64 ",", profileVal);
        }
      }
      printf("\n");
      if (fp != NULL)
      {
        (void)fprintf(fp, "\n");
      }
    }
    sumOfLayerCycles += 
    TIDLRT_ReadProfileData(outArgsPtr->metaDataLayer, 
        TIDL_PROFILE_LAYER, i) ;
  #if defined (SOC_TDA54)      
    ddrBandwidthRead  += outArgsPtr->metaDataLayer[i].ddrBandwidthRead;
    ddrBandwidthWrite += outArgsPtr->metaDataLayer[i].ddrBandwidthWrite;
  #endif
  }

  printf(" Sum of Layer Cycles %lu \n", sumOfLayerCycles);

#if defined (SOC_TDA54)
  printf("\n# NETWORK_EXECUTION_TIME = %8.2f (in ms, c7x @1.5GHz)\n", (sumOfLayerCycles/(TDA5_C7X_FREQ_MHZ * 1000.0)));
  printf("DDR BW (in Mega Bytes/frame) : Read %lf \n", ddrBandwidthRead);
  printf("DDR BW (in Mega Bytes/frame) : Write %lf \n", ddrBandwidthWrite);
  printf("DDR BW (in Mega Bytes/frame) : Total %lf \n", ddrBandwidthRead + ddrBandwidthWrite);
#endif

  if (fp != NULL)
  {
    (void)fprintf(fp, " Sum of Layer Cycles %lu \n", sumOfLayerCycles);
#if defined (SOC_TDA54)
    (void)fprintf(fp, "\n# NETWORK_EXECUTION_TIME = %8.2f (in ms, c7x @1.5GHz)", (sumOfLayerCycles/(TDA5_C7X_FREQ_MHZ * 1000.0)));
    (void)fprintf(fp, "DDR BW (in Mega Bytes/frame) : Read %lf \n", ddrBandwidthRead);
    (void)fprintf(fp, "DDR BW (in Mega Bytes/frame) : Write %lf \n", ddrBandwidthWrite);
    (void)fprintf(fp, "DDR BW (in Mega Bytes/frame) : Total %lf \n", ddrBandwidthRead + ddrBandwidthWrite);
#endif
    (void)fclose(fp);
  }

  return;
}
