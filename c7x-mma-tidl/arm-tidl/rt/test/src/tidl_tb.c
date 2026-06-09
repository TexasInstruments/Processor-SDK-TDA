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


/** @file tidl_tb.c
*
*  @brief  This file contains test code for TI DL test application
*
*
*  @date   Oct 2016
*
*/
#include <getopt.h>
#if defined(_A72_BUILD) || defined(HOST_EMULATION)
#include <unistd.h>
#endif

#include "tidl_tb_utils.h"
#if (TIDLRT_BUILD)
#include "tidl_rt_force_negative_test.h"
#endif

#if (!HOST_EMULATION)  &&  defined(BUILD_C7X)
#include "c7x.h"
extern void tidltb_invalidateL1Dcache();
#endif

#ifdef BUILD_LIDAR_PREPROC
#include "tidl_lidar_preproc.h"
#endif

#ifndef HOST_EMULATION
//#define TIDL_UC_AVV_BUILD
#endif

#include "itidl_rt.h"
#include <signal.h>

char * gNetworkStringName;
int    gWriteStaticBins = 0;
int    currentNetwork = 0;
extern char* getFileNameFromPath(char* path);
extern char* strdup(const char*);
static int32_t gStop = 0;

#ifdef CODE_COVERAGE_ENABLED_FOR_TIDL
static uint32_t gConfigCount = 0;
#endif
#define TIDL_CFG_MAX_LINE_SIZE (3000)

#if (HOST_EMULATION)
#if defined (__C7100__) || defined (__C7120__) 
#include <c7x.h>
#endif /* __C7100__ */
#define _TSC_read() //
#include <time.h>
#else
#if defined (__C7100__) || defined (__C7120__) 
#include <c7x.h>
#endif /* __C7100__ */
#define _TSC_read() __TSC;
#ifdef DDR_BW_STATS
#define ENABLE_DDR_BW_STATS          (1)
#else
#define ENABLE_DDR_BW_STATS          (0)
#endif
#if ENABLE_DDR_BW_STATS
uint64_t ddr_read, ddr_write;
#endif
#endif


#define DUMP_NET_AS_BIN              (0)
#define MAX_FILE_NAME_SIZE           (512)

#define CONFIG_LIST_FILE_NAME    ("testvecs/config/config_list.txt")
uint32_t gFinalStatus;

/* Convenience function to split a command line string to argc, argv */
/* For example, a string "config.txt --debugTraceLevel 1" is broken  */
/* into argc = 3, argv = { "config.txt", "--debugTraceLevel", "1" }  */
#if (!BUILD_WITH_STATIC_ARRAYS)
static void splitCommandLineToArgv(int8_t *str, int32_t *argc, int8_t ***argv)
{
  char **rargv = NULL, *res;
  int rargc = 0;
  while(res = strtok((char *)str, " "))
  {
    str = NULL;
    rargv = (char **)realloc(rargv, (rargc + 1) * sizeof(char *));
    rargv[rargc] = res;
    rargc++;
  }

  assert(rargc >= 1);

  /* add a trailing NULL */
  rargv = (char **)realloc(rargv, (rargc + 1) * sizeof(char *));
  rargv[rargc] = NULL;

  *argv = (int8_t **)rargv;
  *argc = rargc;
}
/* Convenience function to add a "struct option" to a NULL-terminated list */
static struct option *addOption(struct option *lopts, char *opt, int flag)
{
  int i = 0;

  while(lopts[i].name)
  {
    i++;
  }

  /* old Sentinel */
  i++;

  lopts = (struct option *)realloc(lopts, sizeof(struct option) * (i + 1));

  /* new sentinel */
  memset(&lopts[i], 0, sizeof(struct option));
  /* new option */
  lopts[i - 1].name = strdup(opt);
  lopts[i - 1].has_arg = flag;
  lopts[i - 1].flag = NULL;
  lopts[i - 1].val = 0;

  return lopts;
}

/* Convenience function to add an option argumnet to a NULL-terminated list */
static char **addArg(char **args, char *new_arg)
{
  int i = 0;

  while(args[i])
  {
    i++;
  }

  /* old Sentinel */
  i++;

  args = (char **)realloc(args, sizeof(char *) * (i + 1));

  /* new sentinel and arg */
  args[i] = NULL;
  args[i - 1] = new_arg;

  return args;
}
#endif

/* This code is a copy paste from the import tool */
/* TODO: get all of these into a common file      */
#if (!BUILD_WITH_STATIC_ARRAYS)
static int32_t applyConfigArgs(int argc, char **argv)
{
  struct option *lopts;
  int c = 0, lindex;

  lopts = (struct option *)calloc(sizeof(struct option), 1);

  while(gsTokenMap_tidl_net[c].tokenName)
  {
    lopts = addOption(lopts, gsTokenMap_tidl_net[c].tokenName, required_argument);
    c++;
  }

  optind = 1;
  int32_t ret = 0;
  while((c = getopt_long(argc, argv, "", lopts, &lindex)) != -1)
  {
    const char *lopt = NULL;

    if(!c) {
      int k;
      char **args;

      lopt = lopts[lindex].name;
      args = (char **)calloc(sizeof(char *), 1);
      args = addArg(args, optarg);

      k = 0;
      while (argv[optind + k] && strncmp("-", argv[optind + k], strlen("-")))
      {
        args = addArg(args, argv[optind + k]);
        k++;
      }

      ret = readparamarg(lopt, &gsTokenMap_tidl_net[0], args);

      free(args);
    }
    if(ret == -1)
      break;
  }

  return ret;

}
static void addPlatSuffix(char *fileName)
{
	TIDLTB_ASSERT_MSG(strlen(fileName) + strlen(PLAT_SUFFIX) + 1 <= FILE_NAME_SIZE);

	char *last = &fileName[strlen(fileName)];
	char *save = last;
	do {
		last--;
		if(last[0] == '/' || last[0] == '\\')
			break;
		last[strlen(PLAT_SUFFIX)] = last[0];
	} while(last != fileName);

	memcpy(last + 1, PLAT_SUFFIX, strlen(PLAT_SUFFIX));
	save[strlen(PLAT_SUFFIX)] = '\0';
}
#endif
uint64_t currentFrameIdx = 0;
uint64_t currentInstance = 0;
void *inPtrs[TIDL_TB_MAX_INS_HANDLES][10];
void *gPrivArgs = NULL ;

static int32_t deleteRThandles()
{
  int32_t status = IALG_EOK, configCnt = 0;
  tidl_net_config * params = (tidl_net_config *)(&gParams);
  void * handle;
  TI_FILE * fp1;

  for(configCnt = 0; ((configCnt < TIDL_TB_MAX_INS_HANDLES) && (status == IALG_EOK)); configCnt++)
  {
    handle = tidl_tb_handle[configCnt].algHandle;
    fp1    = tidl_tb_handle[configCnt].inFile ;

    if (handle == NULL)
    {
      continue;
    }

    if(fp1 != NULL)
    {
      FCLOSE(fp1);
    }
    if (params->updateNetWithStats)
    {
      //Need to get the Net structure which is part of TIDL object
      sTIDL_Network_t *tidlNet = (sTIDL_Network_t*) gPrivArgs;
      tidl_writeNetWithStats(params,tidlNet);
    }
    status = TIDLRT_delete(handle);
#ifdef TEST_MULTIPRIORITY
    status = TIDLRT_delete(handle1);
#endif

    TIDLTB_ASSERT_MSG(status == IALG_EOK);
    if(status != IALG_EOK)
    {
      tidl_printStatus(status);
    }
#if (!BUILD_WITH_STATIC_ARRAYS)
    if (tidl_tb_handle[configCnt].net != NULL)
    {
      free(tidl_tb_handle[configCnt].net);
      tidl_tb_handle[configCnt].net = NULL ;
    }
    if (tidl_tb_handle[configCnt].ioDesc != NULL)
    {
      free(tidl_tb_handle[configCnt].ioDesc);
      tidl_tb_handle[configCnt].ioDesc = NULL;
    }
    if (tidl_tb_handle[configCnt].ioParams != NULL)
    {
      free(tidl_tb_handle[configCnt].ioParams);
      tidl_tb_handle[configCnt].ioParams = NULL ;
    }
    if (tidl_tb_handle[configCnt].configParams != NULL)
    {
      free(tidl_tb_handle[configCnt].configParams);
      tidl_tb_handle[configCnt].configParams = NULL;
    }
#endif
  }

  return status;
}



int32_t tidlMultiInstanceTest(int8_t** configNames, int32_t totalInsts, void * udmaDrvObjPtr, int32_t argc, char** argv, int32_t currConfigIdx)
{
  #if (!BUILD_WITH_STATIC_ARRAYS)
  int8_t **perConfigArgs;
  int32_t perConfigArgsCount = 0;
  int32_t tidlModelSize;
  int32_t tidlIOSize;
  #endif
  int32_t status = IALG_EOK, configCnt = 0;
  uint64_t i, j;
  tidl_net_config * params = (tidl_net_config *)(&gParams);
  void * handle ;
#ifdef TEST_MULTIPRIORITY
  void *handle1;
#endif
  TI_FILE * fp1;
  sTIDLRT_Params_t prms;
  sTIDLRT_PerfStats_t stats;
  uint64_t read_bytes_start, write_bytes_start;
  uint64_t read_bytes_end, write_bytes_end;
  double read_bytes_total, write_bytes_total;

  void *origNetPtr = NULL;
  void *origIOPtr = NULL;
  
  #if (BUILD_WITH_STATIC_ARRAYS)
    //In this case parent loop over configcnt loop
    int32_t totalStaticNetworks = tidl_getTotalStaticNetworks(tidl_tb_net); //All static arrays should have the same number of total n/ws in their headers
    int32_t staticNwCnt;
    for(staticNwCnt = 0; staticNwCnt < totalStaticNetworks; staticNwCnt++)
    {
      currentNetwork = staticNwCnt; //Important for static array resolution.
  #endif

  for(configCnt = 0; ((configCnt < totalInsts) && (status == IALG_EOK)); configCnt++)
  {
    #if (!BUILD_WITH_STATIC_ARRAYS)
    /* Split the line into a real infer-config filename and a list of optional arguments   */
    /* After this, perConfigArgs[0] and configNames will contain just the config file name */
    /* and the rest of the arguments will be in perConfigArgs[1] onwards                   */
    splitCommandLineToArgv(configNames[configCnt], &perConfigArgsCount, &perConfigArgs);

    setDefaultParams(params) ;
    status = readparamfile((char*)configNames[configCnt], &gsTokenMap_tidl_net[0]) ;
    if(status == -1)
    {
      tidl_tb_printf(0, "Parser Failed");
      break ;
    }

    /* We are dealing with two levels of overrides here                          */
    /* First the user can give certain test-setup level overrides                */
    /* and then each config file to be tested can have their own overrides       */
    /*                                                                           */
    /* For example, a test needs to be run with enableLayerPerfTraces=1 added    */
    /* to each inference, and mobileNetV2 needs to be run with writeTraceLevel=1 */
    /* as well                                                                   */
    /*                                                                           */
    /* So, the application can be run as:                                        */
    /* ./PC_dsp_test_dl_algo.out --debugTraceLevel 1 --enableLayerPerfTraces 1   */
    /* And the config file can have one entry as:                                */
    /* 1 other/entries                                                           */
    /* 1 /path/to/mobilenet/infer/config.txt --writeTraceLevel 1                 */
    /* 1 other/entries                                                           */
    /* 0                                                                         */
    /*                                                                           */
    /* To deal with this, first use the command line options and then inference  */
    /* level options                                                             */
    status = applyConfigArgs(argc, argv);
    if(status == -1)
    {
      tidl_tb_printf(0, "Command-Line Override Parser Failed");
      break ;
    }

    status = applyConfigArgs(perConfigArgsCount, (char **)perConfigArgs);
    if(status == -1)
    {
      tidl_tb_printf(0, "Per-Config Override Parser Failed");
      break ;
    }
    gWriteStaticBins = params->writeBinsAsHeader; //Set Global WriteStaticBins
    free(perConfigArgs);    

    if(!(params->flowCtrl & TIDL_FLOW_CTRL_REF_STAT))
    {
      if(totalInsts > 1)
      {
        tidl_tb_printf(0, "\nProcessing config file #%d : %s \n", configCnt, configNames[configCnt]);
      }
      else
      {
        tidl_tb_printf(0, "\nProcessing config file #%d : %s \n", currConfigIdx, configNames[configCnt]);
      }
    }

    if(strlen((char *)params->outData) != 0)
      addPlatSuffix((char *)params->outData);

    if(params->flowCtrl & TIDL_FLOW_CTRL_REF_STAT)
    {
      params->flowCtrl |= TIDL_FLOW_CTRL_REF_ONLY;
    }
#if HOST_EMULATION
#ifdef _WIN32
    if(((params->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == 0) &&
      (((params->flowCtrl & TIDL_FLOW_CTRL_MMA_NATC) == 0) || ((params->flowCtrl & TIDL_FLOW_CTRL_DSP_NATC) == 0)))
    {
      tidl_tb_printf(0, "CI Host emulation in WINdows is NOT supported Now So forcing it to NATC \n");
      params->flowCtrl |= (TIDL_FLOW_CTRL_MMA_NATC | TIDL_FLOW_CTRL_DSP_NATC);
    }
#endif
#endif
    updateDefaultParams(params);

  //Write out params if writeBinsAsHeaders
  if(gWriteStaticBins)
  {
    //Deprecated
    //tidl_writeAsStaticArray((uint8_t *)params, (int8_t *)"./tidl_tb_tidl_net_config", sizeof(tidl_net_config), "tidl_tb_tidl_net_config", "tidl_tb_tidl_net_config", 1, 0);
    tidl_writeMultiNetworkBin((uint8_t*)params, (int8_t *)"./tidl_tb_tidl_net_config", sizeof(tidl_net_config));
  }

  #else
    //SET gIOparams (Copy over)
    int32_t localOffset = tidl_getStaticOffset(tidl_tb_tidl_net_config); //Byte offset.
    gParams = *((tidl_net_config*) (tidl_tb_tidl_net_config + localOffset)); //Struct Copy, tidl_tb_.. is uint8_t so this is valid
    params->writeOutput = 0;
  #endif

    status = readIODescFile((char *)params->ioConfigFile, &gIOParams);
    if(status == -1)
    {
      break ;
    }
    printIOTensorInfo(&gIOParams);
    firstOutWrite  = 1;

#if (!BUILD_WITH_STATIC_ARRAYS)
    if(strlen((char *)params->traceDumpBaseName) == 0)
    {
#if defined(_A72_BUILD) || defined(HOST_EMULATION)
      char cwd[TIDL_STRING_SIZE];
      if (getcwd(cwd, sizeof(cwd)) == NULL) tidl_tb_printf(0, "Could not get current working directory \n");
      /* For EVM, only base name is required, core number is added by app_utils file write utility */
      sprintf((char *)params->traceDumpBaseName, "%s/trace/%s_" , (char *)cwd, getFileNameFromPath((char *)(configNames[configCnt])));
#if defined(HOST_EMULATION)
      if ( gIOParams.inferenceMode == TIDL_inferenceModeDefault)
      {
        /* Default mode - Core number needs to be added in test bench */
        sprintf((char *)params->traceDumpBaseName, "%s/trace/C7x_%d_%s_" , (char *)cwd, params->coreNum, getFileNameFromPath((char *)(configNames[configCnt])));
      }
      /* For high throughput and low latency mode host emulation, core number is added by mpu tiovx kernels, so no need to add in test bench */
#endif
#endif
    }
    else
    {
      addPlatSuffix((char *)params->traceDumpBaseName);
    }

#endif
    if ((params->postProcType == 4) && (totalInsts > 1))
    {
      params->postProcType = 3;
      tidl_tb_printf(0, "params->postProcType == 4 NOT suported in multi Instance mode \n");
    }
    if ((params->writeOutput == 0) &&  (totalInsts > 1))
    {
      params->writeOutput = 1;
      tidl_tb_printf(0, "params->writeOutput == 0 NOT suported in multi Instance mode \n");
    }
    if (params->writeOutput == 0)
    {
      params->postProcType = 0;
    }
    if(params->inFileFormat != 2)
    {
      if(params->inFileFormat != 5)
      {
        params->postProcType = 0;
      }
    }
    if(totalInsts > 1)
    {
      params->writeBinsAsHeader = 0;
    }

    prms.netPtr  = NULL ;
#if (BUILD_WITH_STATIC_ARRAYS)
    params->writeBinsAsHeader = 0;
    localOffset = tidl_getStaticOffset(tidl_tb_net);
    prms.netPtr = (sTIDL_Network_t *)(tidl_tb_net + localOffset);
    prms.ioBufDescPtr = &gIOParams;
        //tidl_tb_printf(0,"NetPtr Assigned\n");
#else
    tidlModelSize = tidl_getFileSize(params->netBinFile);
    tidlIOSize = tidl_getFileSize(params->ioConfigFile);
    TIDLTB_ASSERT_EXIT(tidlModelSize != -1);
    TIDLTB_ASSERT_EXIT(tidlIOSize != -1);
    prms.netPtr = (sTIDL_Network_t *)malloc(tidlModelSize);
    origNetPtr = (void *) prms.netPtr;
    prms.ioBufDescPtr = malloc(tidlIOSize);
    origIOPtr = (void *) prms.ioBufDescPtr;
    TIDLTB_ASSERT_EXIT(prms.netPtr != NULL)
    TIDLTB_ASSERT_EXIT(prms.ioBufDescPtr != NULL)
    tidl_tb_printf(2, "%12d,    %7.3f %p\n", tidlModelSize, tidlModelSize / (1024.0 * 1024), prms.netPtr);

    tidl_readFile((sTIDL_Network_t *)prms.netPtr, params->netBinFile);
    tidl_readFile(prms.ioBufDescPtr, params->ioConfigFile);

    tidl_tb_printf(1, "worstCaseDelay for Pre-emption is %3.7f \n", TIDL_getWorstcaseDelayForPreemption((sTIDL_Network_t *)prms.netPtr));

    tidl_tb_printf(2, "Network File Read done\n");

    if (params->writeBinsAsHeader)
    {
      //Deprecated
      //tidl_writeAsStaticArray((uint8_t *)prms.netPtr, (int8_t *)"./tidl_tb_net", tidlModelSize, "tidl_tb_net", "tidl_tb_net", 1, DUMP_NET_AS_BIN);
      tidl_writeMultiNetworkBin((uint8_t*)prms.netPtr, (int8_t *)"./tidl_tb_net", tidlModelSize);
    }
#endif
    if (params->debugTraceLevel > 0)
    {
      //tidltb_printNetInfo((sTIDL_Network_t *)prms.netPtr, 1); //Blocked
    }
    gWriteStaticBins = params->writeBinsAsHeader;
    
    prms.stats = NULL;
    if(params->logStats)
    {
      prms.stats = &stats;
    }

    prms.flowCtrl     = params->flowCtrl | (params->reservedCtrl << 16);
    prms.traceLogLevel          = params->debugTraceLevel;
    prms.forceNegativeTest      = params->forceNegativeTest;
    /* If user requested enableLayerPerfTraces then set traceLogLevel = 1 to enable prints */
    if ( params->enableLayerPerfTraces )
    {
      prms.traceLogLevel = 1;
    }
    prms.traceWriteLevel        = params->writeTraceLevel;
    prms.dumpNetInitBackupData  = params->dumpNetInitBackupData;
    strcpy((char *)prms.traceBaseName, (char *)params->traceDumpBaseName);
    strcpy((char *)prms.traceSubgraphName, "");
    strcpy((char *)prms.tempBufferDir, (char*)params->tempBufferDir);
    prms.TIDLWriteBinToFile     = tidl_tb_WriteBinToFile;
    prms.TIDLReadBinFromFile    = tidl_tb_ReadBinFromFile;
#if HOST_EMULATION || !defined(USE_HOST_PRINTF) || defined(_A72_BUILD)
    prms.TIDLVprintf            = vprintf ;
#else
    prms.TIDLVprintf            = appLogvPrintf;
#endif
    prms.quantRangeExpansionFactor     = params->quantRangeExpansionFactor;
    prms.quantRangeUpdateFactor        = params->quantRangeUpdateFactor;
    prms.maxPreEmptDelay               = params->maxPreEmptDelay;
#if (!BUILD_WITH_STATIC_ARRAYS)
    prms.net_capacity				      = tidlModelSize;
    prms.io_capacity				      = tidlIOSize;
#else
    prms.io_capacity				      = sizeof(sTIDL_IOBufDesc_t) ; 
#endif
    prms.targetPriority           = params->targetPriority;
    prms.coreNum                  = params->coreNum;
    prms.coreStartIdx             = params->coreStartIdx;
    prms.releaseIOTensorsAtCreate = params->releaseIOTensorsAtCreate;
    prms.computeChecksum          = params->computeChecksum;

#if (TIDLRT_BUILD)

    /*
     * FORCE NEGATIVE TEST START
     *
     * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_PRMS_DEFAULT
     * forces a call to TIDLRT_setParamsDefault for its coverage
     */
    if(prms.forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_PRMS_DEFAULT)
    {
      TIDLRT_setParamsDefault(&prms);
      return IALG_EOK; // prms->netPtr is now NULL
    }
    /*
     * FORCE NEGATIVE TEST END
     */

    /*
     * FORCE NEGATIVE TEST START
     *
     * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_TRACE_SUBGRAPH_NAME
     * forces setting of traceSubgraphName for coverage during trace dump
     */
    if(prms.forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_TRACE_SUBGRAPH_NAME)
    {
      strcpy((char *)prms.traceSubgraphName, "test_subgraph_name");
    }
    /*
     * FORCE NEGATIVE TEST END
     */

    /*
     * FORCE NEGATIVE TEST START
     *
     * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_TRACE_BASE_NAME_NULL
     * forces traceBaseName to be "" for default setting during TIDLRT_Create
     */
    if(prms.forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_TRACE_BASE_NAME_NULL)
    {
      strcpy((char *)prms.traceBaseName, "");
    }
    /*
     * FORCE NEGATIVE TEST END
     */
#endif

    status = TIDLRT_create(&prms, &handle);
  
    if(status != IALG_EOK)
    {
      TIDLRT_delete(NULL);
    }

#if (!BUILD_WITH_STATIC_ARRAYS)
    if (prms.dumpNetInitBackupData == 1) /* Continue to next instance */
    {
      continue;
    }
#endif

#ifdef TEST_MULTIPRIORITY
    prms.targetPriority = 100 ;
    status = TIDLRT_create(&prms, &handle1);
    prms.targetPriority        = params->targetPriority;
#endif
    TIDLTB_ASSERT_MSG(status == IALG_EOK);
    if(status != IALG_EOK)
    {
      // Safety for the next config file and avoid Bus Errors
      tidl_printStatus(status);
      if (origNetPtr != NULL)
      {
        free(origNetPtr);
        origNetPtr = NULL;
        prms.netPtr = NULL;
      }
      if (origIOPtr != NULL)
      {
        free(origIOPtr);
        origIOPtr = NULL;
        prms.ioBufDescPtr = NULL;
      }
      tidl_tb_printf(0, "TIDLRT_create failed...freeing memory\n");
    }

    if (status == IALG_EOK && prms.stats != NULL)
    {
      double elapsed_time = (prms.stats->proc_time_end - prms.stats->proc_time_start);
      elapsed_time = elapsed_time / 1000000;
      tidl_tb_printf(0, "\n# NETWORK_INIT_TIME = %8.2f (in ms, c7x @1GHz)\n", elapsed_time);
    }

    if(status == IALG_EOK)
    {
      if(gParams.inFileFormat == 1)
      {
        fp1 = tidl_openNetInput(&gParams);
        if(fp1 == NULL)
        {
          status = IALG_EFAIL;
        }
      }
      else
      {
        fp1 = NULL;
        status = IALG_EOK;
      }
#if (BUILD_WITH_STATIC_ARRAYS)
      status = IALG_EOK;
#endif
      TIDLTB_ASSERT_MSG(status == IALG_EOK);
      if(status != IALG_EOK)
      {
        tidl_printStatus(status);
      }
    }

    if(status == IALG_EOK)
    {
      tidl_tb_handle[configCnt].algHandle    = handle;
      tidl_tb_handle[configCnt].inFile       = fp1;
      tidl_tb_handle[configCnt].ioParams     = (sTIDL_IOBufDesc_t*)malloc(sizeof(sTIDL_IOBufDesc_t));
      tidl_tb_handle[configCnt].configParams = (tidl_net_config*)malloc(sizeof(tidl_net_config));
      tidl_tb_handle[configCnt].net          = prms.netPtr;
      tidl_tb_handle[configCnt].ioDesc       = prms.ioBufDescPtr;

#if (!BUILD_WITH_STATIC_ARRAYS)
      if (((sTIDL_Network_t *)(tidl_tb_handle[configCnt].net))->netInitBackupDataOffset[0] == 0)
      {
        tidl_tb_printf(1, " Freeing memory for user provided Net\n");
        if(tidl_tb_handle[configCnt].net) free(tidl_tb_handle[configCnt].net);
        tidl_tb_handle[configCnt].net = NULL ;
      }
#endif
      *tidl_tb_handle[configCnt].ioParams     = gIOParams;
      *tidl_tb_handle[configCnt].configParams = gParams;
      tidl_tb_printf(1, "\n Instance created for  %s\n", configNames[configCnt]);
    }
    else
    {
      tidl_tb_handle[configCnt].algHandle = NULL;
    }
  }
#if (!BUILD_WITH_STATIC_ARRAYS)
  if (prms.dumpNetInitBackupData == 1)
  {
    /* Note that dumpNetInitBackupData is set to 1 only as part of import tool to do infer call for persistent data dump
    The required functionality for this feature during import is completed as part of TIDLRT_Create call
    So no need to execute further */
    return status;
  }
#endif  
  for(configCnt = 0; configCnt < totalInsts && params->initOnly == 0; configCnt++)
  {
    currentInstance = configCnt;
    if(NULL != tidl_tb_handle[configCnt].algHandle)
    {
      /*tidl_tb_handle can only be accessed if no errors occured prior*/
      gIOParams    = *tidl_tb_handle[configCnt].ioParams;
      gParams      = *tidl_tb_handle[configCnt].configParams;
      handle = tidl_tb_handle[configCnt].algHandle;
      fp1    = tidl_tb_handle[configCnt].inFile ;   
      for(i = gParams.startFrameIdx; ((i < (gParams.startFrameIdx + gParams.numFrames)) && (status == IALG_EOK)); i = i + (gIOParams.numVirtualCores * gIOParams.numSuperBatches))
      {
        currentFrameIdx = i;
        if(i == 0)
        {
          firstOutWrite     = 1;
          totalTestCount    = 0;
          passTestCount     = 0;
          top5passTestCount = 0;
          refCompareFile = NULL;
          if ((params->postProcType == 3) || (params->postProcType == 4))
          {
            int32_t i0, i1;
            for (i0 = 0; i0 < params->totNumClasses; i0++)
            {
              for (i1 = 0; i1 < params->totNumClasses; i1++)
              {
                confusion_matrix[i0][i1] = 0;
              }
            }
          }
        }
        if(i < (gParams.startFrameIdx + gParams.numFrames))
        {
          sTIDLRT_Tensor_t *in[128];
          sTIDLRT_Tensor_t *out[128];
          sTIDLRT_Tensor_t *ins;
          sTIDLRT_Tensor_t *outs;

          tidl_tb_printf(2, "\nProcessing Cnt : %4ld, InstCnt : %4d %s!\n",i, configCnt, gParams.netBinFile);
          ins  = (sTIDLRT_Tensor_t *)malloc(gIOParams.numInputBuf*sizeof(sTIDLRT_Tensor_t));
          outs = (sTIDLRT_Tensor_t *)malloc(gIOParams.numOutputBuf*sizeof(sTIDLRT_Tensor_t));
          if((ins == NULL) || (outs == NULL))
          {
            status = IALG_EFAIL;
          }
          if(status == IALG_EOK)
          {
            for(j = 0; j < gIOParams.numInputBuf; j++)
            {
              in[j] = &(ins[j]);
            }

            for(j = 0; j < gIOParams.numOutputBuf; j++)
            {
              out[j] = &(outs[j]);
            }

            status = tidl_allocInOutTensors(&gIOParams, in, out);
          }

          if((status == IALG_EOK) && (params->inFileFormat < 5))
          {
            /* Read Inputs here */
            status = tidl_ReadNetInput(fp1,params,&gIOParams, in,i);
          }
  #ifdef BUILD_LIDAR_PREPROC
          /* inFileFormat ==5 means list of binary files,and it is the only mode supported when LIDAR PREPROC is enabled
          */
          else if((params->inFileFormat == 5) && (status == IALG_EOK))
          {
            /* Prepare Inputs here for 3d od networks */
            status = tidl_prepareLidarInput(fp1,params,&gIOParams, in, i, (sTIDL_Network_t*)prms.netPtr);
          }
  #endif
          else
          {
            tidl_tb_printf(0, "inFileFormat is not supported \n");
          }

  #if (!HOST_EMULATION) && defined(_A72_BUILD)
          if ((prms.flowCtrl & 1) == 1)
          {
            tidl_tb_printf(0, " ----------------------- TIDL Process with REF_ONLY FLOW ------------------------\n");
          }
          else
          {
            tidl_tb_printf(1, " Freeing memory for user provided Net\n");
            if(tidl_tb_handle[configCnt].net) free(tidl_tb_handle[configCnt].net);
            tidl_tb_handle[configCnt].net = NULL ;
            tidl_tb_printf(0, " ----------------------- TIDL Process with TARGET DATA FLOW ------------------------\n");
          }
  #endif

  #if (!HOST_EMULATION) && defined(BUILD_C7X)
          /* Invalidate L1D before calling TIDL invoke*/
          tidltb_invalidateL1Dcache();
  #endif        

  #if (TIDLRT_BUILD)

          /*
           * FORCE NEGATIVE TEST START
           *
           * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_ALLOC_DELLOC_SHARED_TENSORS
           * forces checks on TIDLRT_allocSharedMem, TIDLRT_isSharedMem and 
           * TIDLRT_freeSharedMem by providing invalid parameters as well as
           * allocating and freeing more than allowed limit of
           * TIDLRT_MAX_MEM_RECS
           */
          if(prms.forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_ALLOC_DELLOC_SHARED_TENSORS)
          {
            void *allocatedPtrs[TIDLRT_MAX_MEM_RECS + 1] = {NULL};

            TIDLRT_allocSharedMem(0, 0);

            for(int p = 0; p < (TIDLRT_MAX_MEM_RECS + 1); p++)
            {
              void *ptr = TIDLRT_allocSharedMem(64, 1);
              allocatedPtrs[p] = ptr;
            }
            for(int p = 0; p < (TIDLRT_MAX_MEM_RECS + 1); p++)
            {
              TIDLRT_isSharedMem(allocatedPtrs[p]);
              TIDLRT_freeSharedMem(allocatedPtrs[p]);
            }
          }
          /*
           * FORCE NEGATIVE TEST END
           */
#endif
          int32_t k = 0;
          double total_time = 0.0;
          double elapsed_time = 0.0;
          read_bytes_start = 0;
          write_bytes_start = 0;
          read_bytes_end = 0;
          write_bytes_end = 0;
          read_bytes_total  = 0.0;
          write_bytes_total = 0.0;

          for (k = 0; k < params->numItrPerf; k++)
          {

#if (TIDLRT_BUILD)
            if(prms.stats != NULL)
            {
              TIDLRT_getDdrStats(&read_bytes_start, &write_bytes_start);
            }


            /*
             * FORCE NEGATIVE TEST START
             *
             * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_MULTI_INVOKE_AND_NULL
             * forces TIDLRT_invoke to be invoked with NULL handle to cover
             * NULL handle case and invoke again to propagate firstProcessCall
             * on tidl_rt_ovx side to handle the case of firstProcessCall == 0
             */
            if(prms.forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_MULTI_INVOKE_AND_NULL)
            {
              (void)TIDLRT_invoke(NULL, in, out);
              (void)TIDLRT_invoke(handle, in, out);
            }
            /*
             * FORCE NEGATIVE TEST END
             */
#endif
            status = TIDLRT_invoke(handle, in, out);

            if(status != IALG_EOK)
            {
              TIDLRT_delete(NULL);
              tidl_freeInOutTensors(&gIOParams, in, out);
              free(ins);
              free(outs);
              if (tidl_tb_handle[configCnt].net != NULL)
              {
                free(tidl_tb_handle[configCnt].net);
                tidl_tb_handle[configCnt].net = NULL ;
              }
              if (tidl_tb_handle[configCnt].ioDesc != NULL)
              {
                free(tidl_tb_handle[configCnt].ioDesc);
                tidl_tb_handle[configCnt].ioDesc = NULL;
              }
              tidl_tb_handle[configCnt].algHandle = NULL;
              tidl_tb_printf(0, "TIDLRT_invoke failed...freeing memory\n");
            }

#if (TIDLRT_BUILD)
            if(prms.stats != NULL)
            {
              TIDLRT_getDdrStats(&read_bytes_end, &write_bytes_end);
            }
#endif
            if(prms.stats != NULL)
            {
              read_bytes_total  += (read_bytes_end  - read_bytes_start)/1000000.0;
              write_bytes_total += (write_bytes_end - write_bytes_start)/1000000.0;
              elapsed_time = (prms.stats->proc_time_end - prms.stats->proc_time_start);
              elapsed_time = elapsed_time / 1000000;
              total_time += elapsed_time;
            }
            if (gStop == 1 || status != IALG_EOK)
            {
              break;
            }
          }
#ifndef SOC_TDA54
          if(prms.stats != NULL)
          {
            tidl_tb_printf(0, "\n# NETWORK_EXECUTION_TIME = %8.2f (in ms, c7x @1GHz) with DDR_BANDWIDTH (Read + Write) = %8.2f, %8.2f, %8.2f (in Mega Bytes/frame)",
              total_time/params->numItrPerf, read_bytes_total/params->numItrPerf, write_bytes_total/params->numItrPerf,
              (read_bytes_total+write_bytes_total)/params->numItrPerf);
          }
#endif

          if(status == IALG_EOK)
          {
#if(TIDLRT_BUILD)
            // Given invoke is IALG_EOK this should return IALG_EOK
            status = TIDLRT_deactivate(handle);
#endif
            if(!(params->flowCtrl & TIDL_FLOW_CTRL_REF_STAT))
            {
              tidl_tb_printf(0, " ...");
            }
          }

          if(status == IALG_EOK)
          {
            if(params->writeOutput)
            {
              tidl_WriteNetOutputMem(params, &gIOParams, out, i);
            }
            else
            {
              tidl_CompareNetOutputMem(params, &gIOParams, out);
            }
            tidl_NetOutputPostproc(params, &gIOParams, out, i);
            if(!(params->flowCtrl & TIDL_FLOW_CTRL_REF_STAT))
            {
              tidl_tb_printf(0, " ....");
            }
            
          }
          if(status == IALG_EOK)
          {
            status = tidl_freeInOutTensors(&gIOParams, in, out);
            free(ins);
            free(outs);
            if(!(params->flowCtrl & TIDL_FLOW_CTRL_REF_STAT))
            {
              tidl_tb_printf(0, " .....");
            }
          }
        }
        TIDLTB_ASSERT_MSG(status == IALG_EOK);
        if(status != IALG_EOK)
        {
          tidl_printStatus(status);
        }
        if (gStop == 1 || status != IALG_EOK)
        {
          break;
        }
      }
      TIDLTB_ASSERT_MSG(status == IALG_EOK);
      if(status != IALG_EOK)
      {
        tidl_printStatus(status);
      }    
    }

    if (gStop == 1)
    {
      break;
    }
  }

  #ifdef CODE_COVERAGE_ENABLED_FOR_TIDL
  // if((status == IALG_EOK) && (currConfigIdx == (gConfigCount - 1)))
  /* status may not be propagated below as multi core kernels don't have the control
    callback set
  */
  /* enableCodeCoverage = 5 for covering control call */
   if ((status == IALG_EOK) && (params->enableCodeCoverage == 5)) 
  {
    tidl_tb_printf(0, "\n...Control Call Testing...\n");
    TIDLRT_controlCallTest(&prms, handle);
  }
  if ((status == IALG_EOK) && (params->enableCodeCoverage == 1))
  {
    /* LDRA coverage enabled, so call TIDLRT_control to dump coverage data */
    tidl_tb_printf(0, "\n...LDRA coverage enabled, dumping coverage data...\n");
    TIDLRT_control(&prms, handle);
  }
  #endif

  status = deleteRThandles();
  #if (BUILD_WITH_STATIC_ARRAYS)
    } //End of static nw loop
  #endif

  return status;
}

void * gUdmaDrvObjPtr = NULL;
extern TIDL_Lock_t gTIDLpFxnLockTest[16];
extern TIDL_Unlock_t gTIDLpFxnUnlockTest[16];
extern int32_t TIDL_disableInterrupts();
extern void TIDL_restoreInterrupts(int32_t oldIntState);

static void sigint_handler(int code)
{
  tidl_tb_printf(0, "Ctrl C... Waiting for graph to complete\n");
  gStop = 1;
}

int32_t tidlMain(int32_t argc, char **argv)
{

  int32_t i, configCnt = 0;
#if (!BUILD_WITH_STATIC_ARRAYS)
  char ListFileName[MAX_FILE_NAME_SIZE];
  int32_t LineCnt;
#endif
  void * udmaDrvObjPtr = NULL;

  int32_t multiInstance;

  int8_t ** configNames;

  (void)signal(SIGINT, sigint_handler);

  gUdmaDrvObjPtr =
  udmaDrvObjPtr = tidl_tb_udma_init();

#if (!BUILD_WITH_STATIC_ARRAYS)
#if (!HOST_EMULATION) && !defined(_A72_BUILD)
  
  uint32_t coreNum;
  coreNum = TIDL_TEST_getCoreNum();
#ifdef TIDL_UC_AVV_BUILD
  gTIDLpFxnLockTest[coreNum]  = NULL;
  gTIDLpFxnUnlockTest[coreNum]  = NULL;
#else
  gTIDLpFxnLockTest[coreNum]  = TIDL_disableInterrupts;
  gTIDLpFxnUnlockTest[coreNum]  = TIDL_restoreInterrupts;
#endif
#endif

  if(argc == 1)
  {
    /* No options given, so use the default config file */
    strcpy(ListFileName, CONFIG_LIST_FILE_NAME);
  }
  else if(argc >= 2)
  {
    /*
     * in this scenario user can provide the command like in either of the
     * following formats:
     *
     * (1) executable.exe config_list.txt --numFrames 1 --debugTraceLevel 1
     * (2) executable.exe --numFrames 1 --debugTraceLevel 1
     *
     * Find out if argv[1] looks like a config file name [starts with a '-'],
     * if yes, then use argv[1] as the config file and shift all the arguments
     *         by one place to be used as options
     * else, use all the arguments as options and use default config file
     */
    if(argv[1][0] != '-')
    {
      strcpy(ListFileName, argv[1]);
      for(i = 1; i < argc; i++)
        argv[i] = argv[i + 1];
      argc--;
    }
    else
    {
      strcpy(ListFileName, CONFIG_LIST_FILE_NAME);
    }
  }

  /* What if I want to run a single infer config without going           */
  /* to the trouble of modifying config_list.txt ?                       */
  /* just pass it as "./appName s:/path/to/infer/config.txt --arguments" */
  if(strlen(ListFileName) > 2 && ListFileName[0] == 's' && ListFileName[1] == ':')
  {
    configNames = (int8_t**)malloc(1*sizeof(int8_t*));
    TIDLTB_ASSERT_EXIT(configNames != NULL);
    configNames[0] = (int8_t *)strdup(&ListFileName[2]);
    configCnt = 1;
    multiInstance = 0;
  }
  else
  {
    LineCnt = getNumberOfLinesIntheFile(ListFileName);
    configNames = (int8_t**)malloc(LineCnt*sizeof(int8_t*));
    TIDLTB_ASSERT_EXIT(configNames != NULL);
    configCnt = getConfigFileNamesList(ListFileName, &multiInstance, configNames);
  }
  #else
  //For Static builds
  multiInstance = 0; //Disable
  configCnt = 1; //Only 1 test case per static bin
  configNames = (int8_t**)malloc(sizeof(int8_t*));
  configNames[0] = (int8_t*)malloc(sizeof(int8_t)*32);
  strcpy((char*)configNames[0],"STATIC_BUILD");
  #endif //For fully static builds args section could be populated, but flow flushing using invalid name
  #ifdef CODE_COVERAGE_ENABLED_FOR_TIDL
  gConfigCount = configCnt;
  #endif
  if(multiInstance)
  {
    TIDLTB_ASSERT_EXIT(configCnt <= TIDL_TB_MAX_INS_HANDLES);
    tidlMultiInstanceTest(configNames,configCnt, udmaDrvObjPtr, argc, argv, 0);
  }
  else
  {
    for(i = 0; i < configCnt; i++)
    {
      currentNetwork = i;
      gNetworkStringName = getNetworkStringName((char *)configNames[i]);
      tidlMultiInstanceTest(&configNames[i], 1, udmaDrvObjPtr,argc, argv, i);
    }
  }

  //Free Pointers in configNames first
  for(i = 0; i < configCnt; i++)
  {
    free(configNames[i]);
  }

  //Free pointer to pointer
  free(configNames);
  //ADD AN START STRING
  return (0);
}
