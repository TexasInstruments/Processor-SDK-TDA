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

#ifndef TIDL_RUNTIMES_INFER_COMMON_H_
#define TIDL_RUNTIMES_INFER_COMMON_H_

#include <vector>
#include <string>
#include "itidl_ti.h"
#include "itidl_rt.h"
#include <map>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <dlfcn.h>
#include <cmath>
#include <float.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>

/** Struct used for Dynamic loading of functions required for runtimes
 * from TIDL RT (libvx_tidl_rt.so) library */
typedef struct 
{
  void *lib;
  decltype(&::TIDLRT_create) TIDLRT_create;
  decltype(&::TIDLRT_delete) TIDLRT_delete;
  decltype(&::TIDLRT_invoke) TIDLRT_invoke;
  decltype(&::TIDLRT_deactivate) TIDLRT_deactivate;
  decltype(&::TIDLRT_setParamsDefault) TIDLRT_setParamsDefault;
  decltype(&::TIDLRT_setTensorDefault) TIDLRT_setTensorDefault;
  decltype(&::TIDLRT_getDdrStats) TIDLRT_getDdrStats;
  decltype(&::TIDLRT_isSharedMem) TIDLRT_isSharedMem;
} sTIDL_tidlRtDynamicLoading_t;

/** Parameters related to TIDL RT used for runtimes */
typedef struct
{
  void * rtHandle;
  void * rtInList;
  void * rtOutList;
  void * stats;
  void * netPtr;
  void * ioBufDesc;
} sTIDL_tidlRtParams_t;

/** Parameters passed to TIDL_subgraphRtCreate function */
typedef struct
{
  char netPath[TIDLRT_STRING_SIZE];
  char netBinPath[TIDLRT_STRING_SIZE];
  char ioBinPath[TIDLRT_STRING_SIZE];
  int32_t debugLevel;
  int32_t osrtDebugPrintLevel;
  float maxPreEmptDelay;
  int32_t targetPriority;
  int32_t coreNum;
  int32_t coreStartIdx;
  int32_t inferenceMode;
  int32_t flowCtrl;
  char tempBufferDir[TIDLRT_STRING_SIZE];  /* Path to redirect temporary buffers instead of /dev/shm */
  char traceBaseName[TIDLRT_STRING_SIZE];  /* Base name for trace files, defaults to /tmp/tidl_trace */
} sTIDL_subgraphRtCreateOptions_t;

extern "C"
{

bool TIDL_checkIsDir(const char *path);
bool TIDL_checkIsPath(const char *path);

int32_t TIDL_readBinFromFile(const char *fileName, void *addr, int32_t size, int32_t count);

std::vector<std::vector<int>> TIDL_readAllowedNodesList(std::string artifactsFolder);

int32_t TIDL_subgraphRtCreate(sTIDL_subgraphRtCreateOptions_t* subgraphRtCreateOptions, sTIDL_tidlRtDynamicLoading_t * infer_ops,
                              const char* subGraphName, sTIDL_tidlRtParams_t * subgraphParams);

int32_t TIDL_subgraphRtDelete(int32_t osrtDebugPrintLevel, sTIDL_tidlRtDynamicLoading_t * infer_ops, sTIDL_tidlRtParams_t * tidlRtParams);

void TIDL_osrtDebugPrint(int32_t osrtDebugPrintLevel, char * format, ...);

void TIDL_rtLoadSymbols(sTIDL_tidlRtDynamicLoading_t * infer_ops);

void TIDL_printSubgraphStats(sTIDLRT_PerfStats_t *stats);

} //extern "C"
#endif /*TIDL_RUNTIMES_INFER_COMMON_H_ */