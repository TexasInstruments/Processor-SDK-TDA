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


#include <bits/stdc++.h>
#include <sys/stat.h>
#include <dirent.h>

#include "tidl_runtimes_infer_common.h"

/** Print debug prints based on debug print level */
void TIDL_osrtDebugPrint(int32_t osrtDebugPrintLevel, char * format, ...)
{
  va_list args;
  if (osrtDebugPrintLevel)
  {
    (void)va_start(args, format);
    (void)vprintf(format, args);
    va_end(args);
  }
}

/* Used to load symbols for dynamic loading */
template <typename T>
T TIDL_loadSymbol(void *lib, const char* symbol) {
    T sym = reinterpret_cast<T>(dlsym(lib, symbol));
    assert(sym);
    return sym;
}

extern "C"
{

bool TIDL_checkIsDir(const char *path) {
    const char *real = realpath(path, NULL);
    if(!real)
        return false;

    struct stat st;
    int res = stat(real, &st);
    if(res)
        return false;

    bool ret = false;
    if ((st.st_mode & S_IFMT) == S_IFDIR) {
        ret = true;
        free(const_cast<char *>(real));
    }

    return ret;
}

bool TIDL_checkIsPath(const char *path) {
    const char *real = realpath(path, NULL);
    if(!real)
        return false;

    struct stat st;
    int res = stat(real, &st);
    if(res)
        return false;

    bool ret = false;
    if (S_ISREG(st.st_mode)) {
        ret = true;
        free(const_cast<char *>(real));
    }

    return ret;
}


int32_t TIDL_readBinFromFile(const char *fileName, void *addr, int32_t size, int32_t count)
{
    FILE *fptr = NULL;
    fptr = fopen((const char *)fileName, "rb");
    if (fptr)
    {
      fread(addr, size, 1, fptr);
      fclose(fptr);
      return 0;
    }
    else
    {
      printf("Could not open %s file for reading \n", fileName);
    }
    return -1;
}

/** List of TIDL allowed nodes is written to a text file during compilation 
 * Read it back during inference */
std::vector<std::vector<int>> TIDL_readAllowedNodesList(std::string artifactsFolder)
{
  FILE *fp;
  std::string fileName;

  fileName = artifactsFolder + "/allowedNode.txt";

  fp = fopen(fileName.c_str(), "r");
  if(fp == NULL)
  {
      printf("\n******** WARNING ******* : Could not open %s for reading... Entire model will run on ARM without any delegation to TIDL !\n", fileName.c_str());
      return {{}};
  }

  std::vector<std::vector<int>> supportedNodeGroups;
  int32_t supportedNodeGroupsSize;
  fscanf(fp, "%d\n", &supportedNodeGroupsSize);
  
  for(int i = 0; i < supportedNodeGroupsSize; i++)
  {
    std::vector<int> subgraph;
    int32_t subgraphSize;
    int32_t nodeIdx;

    fscanf(fp, "%d\n", &subgraphSize);
    
    for(int j = 0; j < subgraphSize; j++)
    {
      fscanf(fp, "%d\n", &nodeIdx);
      subgraph.push_back(nodeIdx);
    }
    supportedNodeGroups.push_back(subgraph);
  }
  fclose(fp);
  return supportedNodeGroups;
}

/* Compute processing, copy input and copy output times from available stats and print */
void TIDL_printSubgraphStats(sTIDLRT_PerfStats_t *stats)
{
  double proc_time    = (stats->proc_time_end - stats->proc_time_start)  / 1000;
  double cp_in_time   = (stats->cpIn_time_end - stats->cpIn_time_start)  / 1000;
  double cp_out_time  = (stats->cpOut_time_end - stats->cpOut_time_start)/ 1000;

  printf("Sub Graph Stats %f %f %f \n", cp_in_time, proc_time, cp_out_time);
}

/** This function reads tidl_net.bin and tidl_io_1.bin files in netPtr and ioBufDesc pointer parameters
  * of TIDL RT, sets other default TIDL RT parameters and create a TIDL RT handle to be used for 
  * further processing
  */
int32_t TIDL_subgraphRtCreate(sTIDL_subgraphRtCreateOptions_t* subgraphRtCreateOptions, sTIDL_tidlRtDynamicLoading_t * infer_ops, 
                              const char* subGraphName, sTIDL_tidlRtParams_t * subgraphParams)
{
  TIDL_osrtDebugPrint(subgraphRtCreateOptions->osrtDebugPrintLevel, "************ in TIDL_subgraphRtCreate ************ \n ");
  int status = 0;
  sTIDLRT_Params_t prms;
  FILE *fp_network;
  FILE *fp_config;
  void *handle = NULL;

  status = infer_ops->TIDLRT_setParamsDefault(&prms);
  if(subgraphRtCreateOptions->flowCtrl != -1)
  {
    prms.flowCtrl = subgraphRtCreateOptions->flowCtrl;
  }

  strcpy(prms.tempBufferDir, subgraphRtCreateOptions->tempBufferDir);

  std::string network_file;
  std::string config_file;

  std::string name = "";
  if (subGraphName != NULL)
  {
    name.assign(subGraphName);
    network_file = std::string(subgraphRtCreateOptions->netPath) + "/" + subGraphName +"_tidl_net.bin";
    config_file  = std::string(subgraphRtCreateOptions->netPath) + "/" + subGraphName +"_tidl_io_1.bin";
  }
  else
  {
    network_file = std::string(subgraphRtCreateOptions->netBinPath);
    config_file  = std::string(subgraphRtCreateOptions->ioBinPath);

    size_t slash_pos = network_file.find_last_of('/');
    if (slash_pos == std::string::npos)
    {
      name = network_file;
    }
    else
    {
      name = network_file.substr(slash_pos + 1);
    }

    size_t dot_pos = name.find('.');
    if (dot_pos != std::string::npos)
    {
      name = name.substr(0 , dot_pos);
    }
  }
  char *cstr = new char[network_file.length() + 1];
  strcpy(cstr, network_file.c_str());

  fp_network = fopen(cstr, "rb");
  
  delete[] cstr;

  if (fp_network == NULL)
  {
    printf("Invoke  : ERROR: Unable to open network file %s \n", network_file.c_str());
    return -1;
  }
  prms.stats = (sTIDLRT_PerfStats_t*)malloc(sizeof(sTIDLRT_PerfStats_t));

  fseek(fp_network, 0, SEEK_END);
  prms.net_capacity = ftell(fp_network);
  fseek(fp_network, 0, SEEK_SET);
  fclose(fp_network);
  prms.netPtr = malloc(prms.net_capacity);
  
  prms.TIDLReadBinFromFile = TIDL_readBinFromFile;
  status = prms.TIDLReadBinFromFile(&network_file[0], prms.netPtr, prms.net_capacity, 1);
  
  cstr = new char[config_file.length() + 1];
  strcpy(cstr, config_file.c_str());
  
  fp_config = fopen(cstr, "rb");
  
  delete[] cstr;
  
  if (fp_config == NULL)
  {
    printf("Invoke  : ERROR: Unable to open IO config file %s \n", config_file.c_str());
    return -1;
  }
  fseek(fp_config, 0, SEEK_END);
  prms.io_capacity = ftell(fp_config);
  fseek(fp_config, 0, SEEK_SET);
  fclose(fp_config);
  prms.ioBufDescPtr = malloc(prms.io_capacity);
  status = prms.TIDLReadBinFromFile(&config_file[0], prms.ioBufDescPtr, prms.io_capacity, 1);

  int32_t debugLevel = subgraphRtCreateOptions->debugLevel;
  if (debugLevel <= 2)
  {
    prms.traceLogLevel = debugLevel;
    prms.traceWriteLevel = 0;
  }
  else if (debugLevel == 3)
  {
    prms.traceLogLevel = 1;
    prms.traceWriteLevel = 1;
  }
  else if (debugLevel == 4)
  {
    prms.traceLogLevel = 1;
    prms.traceWriteLevel = 3;
  }
  else if (debugLevel == 5)
  {
    prms.traceLogLevel = debugLevel;
    prms.traceWriteLevel = 3;
  }
  else
  {
    prms.traceLogLevel = debugLevel;
    prms.traceWriteLevel = 0;
  }  
  
  prms.targetPriority = subgraphRtCreateOptions->targetPriority;
  prms.maxPreEmptDelay = subgraphRtCreateOptions->maxPreEmptDelay;
  prms.coreNum = subgraphRtCreateOptions->coreNum;
  prms.coreStartIdx = subgraphRtCreateOptions->coreStartIdx;
  
  strncpy(prms.traceSubgraphName, name.c_str(), TIDLRT_STRING_SIZE - 1);
  if(subgraphRtCreateOptions->traceBaseName[0] != '\0')
  {
    strncpy(prms.traceBaseName, subgraphRtCreateOptions->traceBaseName, TIDLRT_STRING_SIZE - 1);
    prms.traceBaseName[TIDLRT_STRING_SIZE - 1] = '\0';
  }
  sTIDL_IOBufDesc_t *ioBufDesc = (sTIDL_IOBufDesc_t *)prms.ioBufDescPtr;
  if(subgraphRtCreateOptions->inferenceMode != -1) /* Provision to force inference Mode for a particular RT handle creation if necessary */
  {
    /* This is currently used to force inferenceMode to default for floating point compilation pass in OSRT */
    ioBufDesc->inferenceMode = subgraphRtCreateOptions->inferenceMode;
  }

  status = infer_ops->TIDLRT_create(&prms, &handle);

  subgraphParams->rtInList  = (void *)malloc(ioBufDesc->numInputBuf * sizeof(sTIDLRT_Tensor_t));
  subgraphParams->rtOutList = (void *)malloc(ioBufDesc->numOutputBuf * sizeof(sTIDLRT_Tensor_t));
  subgraphParams->rtHandle    = handle;
  subgraphParams->stats       = prms.stats;
  subgraphParams->netPtr      = prms.netPtr;
  subgraphParams->ioBufDesc   = prms.ioBufDescPtr;

  TIDL_osrtDebugPrint(subgraphRtCreateOptions->osrtDebugPrintLevel, "************ TIDL_subgraphRtCreate done ************ \n ");
  if(subgraphParams->netPtr) free(subgraphParams->netPtr);
  return status;
}

int32_t TIDL_subgraphRtDelete(int32_t osrtDebugPrintLevel, sTIDL_tidlRtDynamicLoading_t * infer_ops, sTIDL_tidlRtParams_t * tidlRtParams)
{
  TIDL_osrtDebugPrint(osrtDebugPrintLevel, "************ in TIDL_subgraphRtDelete ************ \n ");
  int32_t status = 0;
  if(tidlRtParams->rtHandle)
  {
    status = infer_ops->TIDLRT_deactivate(tidlRtParams->rtHandle);
    status = infer_ops->TIDLRT_delete(tidlRtParams->rtHandle);
  }
  free(tidlRtParams->rtInList);
  free(tidlRtParams->rtOutList);
  free(tidlRtParams->stats);
  free(tidlRtParams->ioBufDesc);
  return status;
}

/* Dynamically load symbols from TIDL RT (libvx_tidl_rt.so) library needed for runtimes */
void TIDL_rtLoadSymbols(sTIDL_tidlRtDynamicLoading_t * infer_ops)
{
  infer_ops->TIDLRT_create = TIDL_loadSymbol<decltype(infer_ops->TIDLRT_create)>  (infer_ops->lib, "TIDLRT_create");
  infer_ops->TIDLRT_delete = TIDL_loadSymbol<decltype(infer_ops->TIDLRT_delete)>  (infer_ops->lib, "TIDLRT_delete");
  infer_ops->TIDLRT_invoke = TIDL_loadSymbol<decltype(infer_ops->TIDLRT_invoke)>  (infer_ops->lib, "TIDLRT_invoke");
  infer_ops->TIDLRT_deactivate = TIDL_loadSymbol<decltype(infer_ops->TIDLRT_deactivate)>(infer_ops->lib, "TIDLRT_deactivate");
  infer_ops->TIDLRT_setParamsDefault = TIDL_loadSymbol<decltype(infer_ops->TIDLRT_setParamsDefault)>(infer_ops->lib, "TIDLRT_setParamsDefault");
  infer_ops->TIDLRT_setTensorDefault = TIDL_loadSymbol<decltype(infer_ops->TIDLRT_setTensorDefault)>(infer_ops->lib, "TIDLRT_setTensorDefault");
  infer_ops->TIDLRT_getDdrStats = TIDL_loadSymbol<decltype(infer_ops->TIDLRT_getDdrStats)>(infer_ops->lib, "TIDLRT_getDdrStats");
  infer_ops->TIDLRT_isSharedMem = TIDL_loadSymbol<decltype(infer_ops->TIDLRT_isSharedMem)>(infer_ops->lib, "TIDLRT_isSharedMem");
}

} //extern C

