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


/** @file tidl_tb_utils.c
*
*  @brief  This file contains utility functions for TI DL test application
*
*
*  @date   Aug 2019
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <math.h>
#include <float.h>

#include "itidl_ti.h"
#include "tidl_config.h"
#include "ti_mem_manager.h"
#include "platform_common.h"
#include "tidl_tb_utils.h"

#if !defined(SOC_AM62A) && !defined (SOC_J722S) && !defined (SOC_TDA54)
#if (!HOST_EMULATION) && !defined(_A72_BUILD)
#include <ti/csl/arch/c7x/cslr_C7X_CPU.h>
#endif
#endif

#ifndef _A72_BUILD
  #ifdef DMA_UTILS_STANDALONE
    #include "udma_standalone/udma.h"
    #if defined(SOC_TDA54) && !defined(HOST_EMULATION) 
    #include "hal/Dru/v0/include/Dru_Hal.h"
    #define CSL_MSMC_DRU0_CFG_BASE (0x6AC00000UL)
    #define UDMA_UTC_BASE_DRU0      (CSL_MSMC_DRU0_CFG_BASE)   /* Central DRU */
    #endif
  #else
    #include "ti/drv/udma/udma.h"
    #include "ti/drv/sciclient/sciclient.h"
  #endif
#endif

#define TIDL_CFG_MAX_LINE_SIZE (3000)

//#define TIDL_WRITE_PRINTS_TO_BUF
#ifdef TIDL_WRITE_PRINTS_TO_BUF
#pragma DATA_SECTION(printSpace, ".printBuf")
uint8_t printSpace[131072] = {0};
int32_t bufPos = 0;
#endif


uint8_t traceDumpName[FILE_NAME_SIZE];
uint8_t headerFileName[FILE_NAME_SIZE];
char printBuf[500];

//For Static Builds
extern int gWriteStaticBins;
extern int currentNetwork;
typedef struct
{
  int32_t currentNet; //Used to index into networkSizes and keeps track of total networks present
  int32_t networkSizes[64]; //Arbitrarily selected 64 networks as the limit, real limit depends on config.
}TIDL_binHeader;



const char * TIDL_LayerString[] =
{
"TIDL_DataLayer            ",
"TIDL_ConvolutionLayer     ",
"TIDL_PoolingLayer         ",
"TIDL_ReLULayer            ",
"TIDL_PReLULayer           ",
"TIDL_EltWiseLayer         ",
"TIDL_InnerProductLayer    ",
"TIDL_SoftMaxLayer         ",
"TIDL_BatchNormLayer       ",
"TIDL_BiasLayer            ",
"TIDL_ScaleLayer           ",
"TIDL_Deconv2DLayer        ",
"TIDL_ConcatLayer          ",
"TIDL_SplitLayer           ",
"TIDL_SliceLayer           ",
"TIDL_CropLayer            ",
"TIDL_FlattenLayer         ",
"TIDL_DropOutLayer         ",
"TIDL_ArgOpLayer           ",
"TIDL_DetectionOutputLayer ",
"TIDL_ShuffleChannelLayer  ",
"TIDL_ResizeLayer          ",
"TIDL_RoiPoolingLayer      ",
"TIDL_OdPostProcessingLayer",
"TIDL_DepthToSpaceLayer    ",
"TIDL_SigmoidLayer         ",
"TIDL_PadLayer             ",
"TIDL_ColorConversionLayer ",
"TIDL_OdOutputReformatLayer",
"TIDL_DataConvertLayer     ",
"TIDL_CustomLayer          ",
"TIDL_BatchReshapeLayer    ",
"TIDL_ReduceLayer          ",
"TIDL_ScatterElementsLayer ",
"TIDL_SqueezeLayer         ",
"TIDL_TanhLayer            ",
"TIDL_HardSigmoidLayer     ",
"TIDL_ELULayer             ",
"TIDL_ReshapeLayer         ",
"TIDL_ConstDataLayer       ",
"TIDL_GatherLayer          ",
"TIDL_TransposeLayer       ",
"TIDL_LayerNormLayer       ",
"TIDL_GridSampleLayer      ",
"TIDL_TopKLayer            ",
"TIDL_DeformableConvLayer  ",
"TIDL_TileLayer            ",
"TIDL_LogicalOpLayer       ",
"TIDL_RMSNormalizationLayer",
"TIDL_LSTMLayer            ",
"TIDL_GRULayer             ",
"TIDL_RNNLayer             ",
"TIDL_GatherNDLayer        ",
"TIDL_CastLayer            ",
"TIDL_GatherElementsLayer  ",
"TIDL_ShapeLayer           ",
"TIDL_SizeLayer            ",
"TIDL_AttentionLayer       ",
"TIDL_NonZeroLayer         ",
"TIDL_UnsupportedLayer     ",
};
sTIDL_TBInstHanle tidl_tbHanles[TIDL_TB_MAX_INS_HANDLES];
sTIDL_TBInstHanle tidl_tb_handle[TIDL_TB_MAX_INS_HANDLES];
int32_t firstOutWrite  = 0;
int32_t totalTestCount = 0;
int32_t passTestCount  = 0;
int32_t top5passTestCount = 0;
int64_t confusion_matrix[MAX_SEG_CLASS_IDX][MAX_SEG_CLASS_IDX] = {0};
TI_FILE *refCompareFile = NULL;


/*Error code printing:*/

sTIDL_errCodeMapping gTIDL_errorCodeMap[NUM_ERR_TYPES]=
{
  {(char*)"TIDL_E_CONV_INVALID_INPUT_WIDTH            ",((int32_t) (-1000 - 0))},
  {(char*)"TIDL_E_CONV_INVALID_INPUT_HEIGHT           ",((int32_t) (-1000 - 1))},
  {(char*)"TIDL_E_CONV_INVALID_OUTPUT_WIDTH           ",((int32_t) (-1000 - 2))},
  {(char*)"TIDL_E_CONV_INVALID_OUTPUT_HEIGHT          ",((int32_t) (-1000 - 3))},
  {(char*)"TIDL_E_CONV_INVALID_NUM_IN_CHANNELS        ",((int32_t) (-1000 - 4))},
  {(char*)"TIDL_E_CONV_INVALID_NUM_OUT_CHANNELS       ",((int32_t) (-1000 - 5))},
  {(char*)"TIDL_E_CONV_INVALID_KER_WIDTH              ",((int32_t) (-1000 - 6))},
  {(char*)"TIDL_E_CONV_INVALID_KER_HEIGHT             ",((int32_t) (-1000 - 7))},
  {(char*)"TIDL_E_CONV_INVALID_KER_TYPE               ",((int32_t) (-1000 - 8))},
  {(char*)"TIDL_E_CONV_INVALID_STRIDE_WIDTH           ",((int32_t) (-1000 - 9))},
  {(char*)"TIDL_E_CONV_INVALID_STRIDE_HEIGHT          ",((int32_t) (-1000 - 10))},
  {(char*)"TIDL_E_CONV_NEGATIVE_OUTPUT_SHIFT          ",((int32_t) (-1000 - 11))},
  {(char*)"TIDL_E_CONV_POOL_INVALID_POOL_TYPE         ",((int32_t) (-1000 - 12))},
  {(char*)"TIDL_E_CONV_POOL_INVALID_KER_WIDTH         ",((int32_t) (-1000 - 13))},
  {(char*)"TIDL_E_CONV_POOL_INVALID_KER_HEIGHT        ",((int32_t) (-1000 - 14))},
  {(char*)"TIDL_E_CONV_POOL_INVALID_STRIDE_WIDTH      ",((int32_t) (-1000 - 15))},
  {(char*)"TIDL_E_CONV_POOL_INVALID_STRIDE_HEIGHT     ",((int32_t) (-1000 - 16))},
  {(char*)"TIDL_E_ELTWISE_INVALID_INPUT_WIDTH         ",((int32_t) (-1020 - 0))},
  {(char*)"TIDL_E_ELTWISE_INVALID_INPUT_HEIGHT        ",((int32_t) (-1020 - 1))},
  {(char*)"TIDL_E_ELTWISE_INVALID_OUTPUT_WIDTH        ",((int32_t) (-1020 - 2))},
  {(char*)"TIDL_E_ELTWISE_INVALID_OUTPUT_HEIGHT       ",((int32_t) (-1020 - 3))},
  {(char*)"TIDL_E_ELTWISE_INVALID_ELTWISE_TYPE        ",((int32_t) (-1020 - 4))},
  {(char*)"TIDL_E_ELTWISE_INVALID_NUM_CHANNELS        ",((int32_t) (-1020 - 5))},
  {(char*)"TIDL_E_POOL_INVALID_INPUT_WIDTH            ",((int32_t) (-1030 - 0))},
  {(char*)"TIDL_E_POOL_INVALID_INPUT_HEIGHT           ",((int32_t) (-1030 - 1))},
  {(char*)"TIDL_E_POOL_INVALID_OUTPUT_WIDTH           ",((int32_t) (-1030 - 2))},
  {(char*)"TIDL_E_POOL_INVALID_OUTPUT_HEIGHT          ",((int32_t) (-1030 - 3))},
  {(char*)"TIDL_E_POOL_INVALID_POOL_TYPE              ",((int32_t) (-1030 - 4))},
  {(char*)"TIDL_E_POOL_INVALID_NUM_CHANNELS           ",((int32_t) (-1030 - 5))},
  {(char*)"TIDL_E_POOL_INVALID_KER_WIDTH              ",((int32_t) (-1030 - 6))},
  {(char*)"TIDL_E_POOL_INVALID_KER_HEIGHT             ",((int32_t) (-1030 - 7))},
  {(char*)"TIDL_E_POOL_INVALID_STRIDE_WIDTH           ",((int32_t) (-1030 - 8))},
  {(char*)"TIDL_E_POOL_INVALID_STRIDE_HEIGHT          ",((int32_t) (-1030 - 9))},
  {(char*)"TIDL_E_IP_INVALID_NUM_IN_NODES             ",((int32_t) (-1040 - 0))},
  {(char*)"TIDL_E_IP_INVALID_NUM_OUT_NODES            ",((int32_t) (-1040 - 1))},
  {(char*)"TIDL_E_IP_NEGATIVE_OUTPUT_SHIFT            ",((int32_t) (-1040 - 2))},
  {(char*)"TIDL_E_ARGMAX_INVALID_NUM_CHANNELS         ",((int32_t) (-1050 - 0))},
  {(char*)"TIDL_E_BN_INVALID_INPUT_WIDTH              ",((int32_t) (-1060 - 0))},
  {(char*)"TIDL_E_BN_INVALID_INPUT_HEIGHT             ",((int32_t) (-1060 - 1))},
  {(char*)"TIDL_E_BN_INVALID_OUTPUT_WIDTH             ",((int32_t) (-1060 - 2))},
  {(char*)"TIDL_E_BN_INVALID_OUTPUT_HEIGHT            ",((int32_t) (-1060 - 3))},
  {(char*)"TIDL_E_BN_INVALID_NUM_CHANNELS             ",((int32_t) (-1060 - 4))},
  {(char*)"TIDL_E_BN_INVALID_ENABLE_RELU              ",((int32_t) (-1060 - 5))},
  {(char*)"TIDL_E_BN_NEGATIVE_OUTPUT_SHIFT            ",((int32_t) (-1060 - 6))},
  {(char*)"TIDL_E_CROP_INVALID_INPUT_WIDTH            ",((int32_t) (-1070 - 0))},
  {(char*)"TIDL_E_CROP_INVALID_INPUT_HEIGHT           ",((int32_t) (-1070 - 1))},
  {(char*)"TIDL_E_CROP_INVALID_OUTPUT_WIDTH           ",((int32_t) (-1070 - 2))},
  {(char*)"TIDL_E_CROP_INVALID_OUTPUT_HEIGHT          ",((int32_t) (-1070 - 3))},
  {(char*)"TIDL_E_CROP_INVALID_NUM_CHANNELS           ",((int32_t) (-1070 - 4))},
  {(char*)"TIDL_E_CROP_INVALID_OFFSET_WIDTH           ",((int32_t) (-1070 - 5))},
  {(char*)"TIDL_E_CROP_INVALID_OFFSET_HEIGHT          ",((int32_t) (-1070 - 6))},
  {(char*)"TIDL_E_FLATTEN_INVALID_INPUT_WIDTH         ",((int32_t) (-1080 - 0))},
  {(char*)"TIDL_E_FLATTEN_INVALID_INPUT_HEIGHT        ",((int32_t) (-1080 - 1))},
  {(char*)"TIDL_E_FLATTEN_INVALID_OUTPUT_WIDTH        ",((int32_t) (-1080 - 2))},
  {(char*)"TIDL_E_FLATTEN_INVALID_OUTPUT_HEIGHT       ",((int32_t) (-1080 - 3))},
  {(char*)"TIDL_ERROR_SOFTMAX_INVALID_NUM_CHANNELS        ",((int32_t) (-1090 - 0))},
  {(char*)"TIDL_E_SHUFFLECHANNEL_INVALID_INPUT_WIDTH   ",((int32_t) (-1100 - 0))},
  {(char*)"TIDL_E_SHUFFLECHANNEL_INVALID_INPUT_HEIGHT  ",((int32_t) (-1100 - 1))},
  {(char*)"TIDL_E_SHUFFLECHANNEL_INVALID_OUTPUT_WIDTH  ",((int32_t) (-1100 - 2))},
  {(char*)"TIDL_E_SHUFFLECHANNEL_INVALID_OUTPUT_HEIGHT ",((int32_t) (-1100 - 3))},
  {(char*)"TIDL_E_SHUFFLECHANNEL_INVALID_NUM_GROUPS    ",((int32_t) (-1100 - 4))},
  {(char*)"TIDL_E_UNSUPPORTED_LAYER                    ",((int32_t) (-1110 - 0))},
  {(char*)"TIDL_E_DATAFLOW_INFO_NULL                   ",((int32_t) (-1110 - 1))},
  {(char*)"TIDL_E_INVALID_DEBUG_TRACE_PRM              ",((int32_t) (-1110 - 2))},
  {(char*)"TIDL_E_INVALID_NET_VERSION                  ",((int32_t) (-1110 - 3))},
  {(char*)"TIDL_E_INVALID_IO_LINE_PITCH                ",((int32_t) (-1110 - 4))},
  {(char*)"TIDL_E_QUANT_STATS_NOT_AVAILABLE            ",((int32_t) (-1110 - 5))},
  {(char*)"END_OF_ERR_LIST                             ",((int32_t) (0))}
};


#ifdef TIDL_WRITE_PRINTS_TO_BUF

static int32_t tidl_buffer_printf(const char * format, va_list args)
{
  int status = 0;
  bufPos += vsprintf((char *)(bufPos + printSpace),format,args) + 1;
  fflush(stdout);

  return status;
}

#endif

void tidl_buffer_printf_dump()
{
#if defined(J7_LOKI) && defined(TIDL_WRITE_PRINTS_TO_BUF)
  FILE * fp;
  printf("tidl_buffer_printf_dump");
  fp = fopen("printTraceBuf.txt", "wb");
  printf("bufPos %d\n", bufPos);
  fwrite(printSpace, bufPos, 1, fp);
  fclose(fp);
#endif
}

void tidl_printStatus(int32_t status)
{
  int32_t i = 0;
  int32_t isFound = 0;
  while(isFound == 0)
  {
    if(gTIDL_errorCodeMap[i].errCode == status)
    {
      isFound = 1;
      break;
    }
    else if(gTIDL_errorCodeMap[i].errCode == 0)
    {
      //Error not in error list
      isFound = -1;
      break;
    }
    i++;
  }

  if(isFound == 1)
  {
     tidl_tb_printf(0, "Error Type: %s\n", gTIDL_errorCodeMap[i].tokenName);
  }
  else if(isFound == -1)
  {
     tidl_tb_printf(0, "Invalid Error Type!\n", gTIDL_errorCodeMap[i].tokenName);
  }

}

#ifndef _A72_BUILD
#if defined(SOC_TDA54) && !defined(HOST_EMULATION)
Dru_Hal_InstanceType druHalInstance;
#else
struct Udma_DrvObj  udmaDrvObj;
#endif
#ifndef DMA_UTILS_STANDALONE
#if defined(SOC_J721E) || defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)
int32_t tidl_sciclientDmscGetVersion(char *version_str, uint32_t version_str_size)
{
    int32_t retVal = 0;
#if _HOST_BUILD
    return retVal;
#endif

struct tisci_msg_version_req req = {0};
    const Sciclient_ReqPrm_t      reqPrm =
    {
        TISCI_MSG_VERSION,
        TISCI_MSG_FLAG_AOP,
        (const uint8_t *)&req,
        0,
        SCICLIENT_SERVICE_WAIT_FOREVER
    };
    struct tisci_msg_version_resp response;
    Sciclient_RespPrm_t           respPrm =
    {
        0,
        (uint8_t *) &response,
        sizeof (response)
    };

    retVal = Sciclient_service(&reqPrm, &respPrm);
    if (0 == retVal)
    {
        if (respPrm.flags == TISCI_MSG_FLAG_ACK)
        {
            if(version_str == NULL)
            {
                tidl_tb_printf(0, "SCICLIENT: DMSC FW version [%s]\n", (char *) response.str);
                tidl_tb_printf(0, "SCICLIENT: DMSC FW revision 0x%x  \n", response.version);
                tidl_tb_printf(0, "SCICLIENT: DMSC FW ABI revision %d.%d\n",
                    response.abi_major, response.abi_minor);
            }
            else
            {
                snprintf(version_str, version_str_size, "version %s, revision 0x%x, ABI %d.%d",
                    (char *) response.str,
                    response.version,
                    response.abi_major, response.abi_minor
                    );
            }
        }
        else
        {
            retVal = -1;
        }
    }
    if(retVal!=0)
    {
        tidl_tb_printf(0, "SCICLIENT: ERROR: DMSC Firmware Get Version failed !!!\n");
    }

    return (retVal);
}
#endif
#endif
#endif

char line[TIDL_CFG_MAX_LINE_SIZE];
int32_t getNumberOfLinesIntheFile(char * fileName)
{
  FILE * fp1;
  int32_t lineCnt = 0;

  fp1 = fopen((const char *)fileName, "r");
  if (fp1 == NULL)
  {
    tidl_tb_printf(0, "Could not open %s file for reading \n", fileName);
    return 0;
  }
  while (!feof(fp1))
  {
    fgets(line, TIDL_CFG_MAX_LINE_SIZE, fp1);
    if(line[strlen(line) - 1] != '\n')
    {
      tidl_tb_printf(0, "Malformed line %d in %s file, no terminating newline \n", lineCnt, fileName);
      return 0;
    }
    lineCnt++;
  }
  return(lineCnt);
}

char stringNameMem[1000];

char * getNetworkStringName(char * fileName)
{
  char *stringArray[4] = {"caffe","onnx","tensorflow","tflite"};
  char * found;
  char * newString = NULL;
  int32_t i;

  for ( i = 0; i < 4; i++)  {
    found = strstr(fileName, stringArray[i]);
    if ( found )
    {
      break;
    }
  }

  if ( found )
  {
    newString = stringNameMem;

    strcpy(newString, found);

    for ( i = 0; newString[i] && newString[i]!= ' ';i++)
    {
      if ( newString[i]== '/' )
      {
        newString[i] = '_';
      }
    }
    newString[i] = '\0';
  }
  return newString;
}
char str[TIDL_CFG_MAX_LINE_SIZE];
int32_t getConfigFileNamesList(char * fileName, int32_t *multiInstance, int8_t** configNames)
{
  FILE * fp1;
  int32_t config_cmd, configCnt= 0;
  char * linePtr = line;
  int32_t stringSize;
  int32_t mtDetected = 0;
  int32_t pos = 0;

  fp1 = fopen((const char *)fileName, "r");
  if (fp1 == NULL)
  {
    tidl_tb_printf(0, "Could not open %s file for reading \n", fileName);
    return 0;
  }
  while (!feof(fp1))
  {
    fgets(line, TIDL_CFG_MAX_LINE_SIZE, fp1);
    sscanf(line, "%d%n",&config_cmd, &pos);
    if(config_cmd == 4)
    {
      /* config_cmd 4 indicates disable test case for target. For host emulation, it is equivalent to config_cmd = 1 */
#if (HOST_EMULATION)
      config_cmd = 1;
#else
      config_cmd = 2;
#endif
    }
    if((config_cmd == 1) || (config_cmd == 3))
    {
      if(mtDetected == 0)
      {
        mtDetected = 1;
        *multiInstance = (config_cmd == 1 ? 0: 1);
      }

      str[0] = '\0';
      linePtr = moveToNextElement(str, line + pos);
      linePtr[strlen(linePtr) - 1] = '\0';
      sprintf(str, "%s", linePtr);
    }

    if(config_cmd == 0)
    {
      tidl_tb_printf(1, "\nEnd of config list found !\n");
      break;
    }
    else if(config_cmd == 2)
    {
      continue;
    }
    else if((config_cmd == 1) && (*multiInstance == 0))
    {
      stringSize = strlen(str)+1;
      configNames[configCnt] = (int8_t*)malloc(stringSize);
      memset(configNames[configCnt],0,stringSize);
      strcpy((char *)configNames[configCnt],str);
      configCnt++;
    }
    else if((config_cmd == 3) && (*multiInstance == 1))
    {
      stringSize = strlen(str)+1;
      configNames[configCnt] = (int8_t*)malloc(stringSize);
      memset(configNames[configCnt],0,stringSize);
      strcpy((char *)configNames[configCnt], str);
      configCnt++;
    }
    else
    {
      tidl_tb_printf(0, "\nUn supported  config_cmd !\n");
    }
  }
  return(configCnt);
}

char * moveToNextElement(char *in, char *LinePtr)
{
  LinePtr += strlen(in);
  while((LinePtr[0] == ' ')||(LinePtr[0] == '\t'))
  {
    LinePtr++;
  }
  return(LinePtr);
}

#if !defined(SOC_TDA54) || defined(HOST_EMULATION)
static void TIDL_printf(const char *str)
{
}
#endif

void * tidl_tb_udma_init( void)
{
#ifndef _A72_BUILD
#if defined(SOC_TDA54) && !defined(HOST_EMULATION) && !defined(TIDLRT_BUILD)
  Dru_Hal_InitParamsType druHalInitParams;
  /* DRU0 base address = 0x6AC00000UL */
  druHalInitParams.HwDruRegsPtr = (Dru_Hal_Regs *) UDMA_UTC_BASE_DRU0;
  druHalInitParams.UserArgsPtr = NULL;
  Dru_Hal_Init(&druHalInstance, &druHalInitParams);
  return &druHalInstance;
#else
  Udma_InitPrms initPrms;
  UdmaInitPrms_init(UDMA_INST_ID_MAIN_0, &initPrms);
  initPrms.printFxn = &TIDL_printf;
#ifndef DMA_UTILS_STANDALONE
  initPrms.skipGlobalEventReg = 1;
#endif
  initPrms.virtToPhyFxn = tidltb_virtToPhyAddrConversion;
  Udma_init(&udmaDrvObj, &initPrms);
  return &udmaDrvObj;
#endif
#else
  return NULL;
#endif
}

int32_t tidltb_getDatElementSize(int32_t elementType)
{
  if ((elementType == TIDL_SignedChar) || (elementType == TIDL_UnsignedChar) || (elementType == TIDL_Bool)) 
  {
    return 1;
  }
  else if ((elementType == TIDL_SignedShort) || (elementType == TIDL_UnsignedShort))
  {
    return 2;
  }
  else if ((elementType == TIDL_SinglePrecFloat) || (elementType == TIDL_UnsignedWord) || (elementType == TIDL_SignedWord))
  {
    return 4;
  }
  else if ((elementType == TIDL_UnsignedDoubleWord) || (elementType == TIDL_SignedDoubleWord))
  {
    return 8;
  }
  else
  {
    return 1;
  }
}
int32_t getAVXKernelEnv()
{
int32_t env = 0;
#if (HOST_EMULATION)
    char *str;
    str = getenv("TIDL_RT_AVX_REF");
    if(!str)
    {
        env = 0x00000020;
    }
    else
    {
        env = atoi(str);
        if((env != 0) && (env != 1) && (env != 3))
        {
          printf("TIDL_RT_AVX_REF is expected to be either 0 or 1 or 3. -  %d is not supported. Setting it to zero\n", env);
          env = 0;
        }
        if(env)
        {
          env = env << 5;
        }
   }
#endif
    return env;
}


void setDefaultParams(tidl_net_config * params)
{
  params->updateNetWithStats        = 0xFFFFFFFF;
  params->quantRangeExpansionFactor = 1.0;
  params->quantRangeUpdateFactor    = -1.0;
  params->inFileFormat              = 1;
  params->noZeroCoeffsPercentage    = 100;
  params->writeOutput               = 1;
  params->postProcType              = 0;
  params->postProcDataId            = 0;
  params->writeBinsAsHeader         = 0;
  params->targetPriority            = 0;
  params->coreNum                   = 1;
  params->coreStartIdx              = 1;
  params->enableLayerPerfTraces     = 0;
  params->numFrames                 = 1;
  params->totNumClasses             = 32;
  params->debugTraceLevel           = 0;
  params->writeTraceLevel           = 0;
  params->dumpNetInitBackupData     = 0;
  params->initOnly                  = 0;
  params->currFrameIdx              = 0;
  params->startFrameIdx             = 0;
  params->reservedCtrl              = 0;
  params->inResizeMode              = 1; // Bilinear
  params->numItrPerf                = 1;
  params->inFileFormatRandomType    = 1 ;
  params->maxPreEmptDelay           = FLT_MAX;
#if (HOST_EMULATION)
  params->flowCtrl                  = TIDL_FLOW_CTRL_REF_ONLY;
  params->flowCtrl                 |= getAVXKernelEnv() ;
#else
  params->flowCtrl                  = TIDL_FLOW_CTRL_DEFAULT;
#endif
  params->traceDumpBaseName[0]   = '\0';
  params->outData[0]             = '\0';
  strcpy((char*)params->tempBufferDir, "/dev/shm");
  params->forceNegativeTest         = 0;
  params->enableCodeCoverage        = 0;
  params->releaseIOTensorsAtCreate  = 0;
#if (HOST_EMULATION)
  params->logStats                  = 0;
#else
  params->logStats                  = 1;
#endif
  params->computeChecksum           = 0;
}

void updateDefaultParams(tidl_net_config * params)
{
  if(params->updateNetWithStats == 0xFFFFFFFF)
  {
    if(params->flowCtrl & TIDL_FLOW_CTRL_REF_STAT)
    {
      params->updateNetWithStats = TIDL_UPDATE_QUANT_STATS;
    }
    else if(params->flowCtrl & TIDL_FLOW_CTRL_PERF_MODEL)
    {
      params->updateNetWithStats = TIDL_UPDATE_PERF_MODEL_STATS;
    }
    else
    {
      params->updateNetWithStats = TIDL_UPDATE_NONE;
    }
  }

  if( (params->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) == 0)
  {
    /* During TIDL inference currently we don't have support
    to dynamically update the tensor range. So force it to zero */
    params->quantRangeUpdateFactor = 0.0;
  }
}

int32_t getFileNameAndIdxFromList(char* listFile, int32_t frameCount, int32_t fileIdx, char* currFile)
{
  TI_FILE * fp1;
  int32_t i;
  int32_t status = 0;
  char * fgetStatus = NULL;
  char *linePtr = line;
  fp1 = FOPEN((const char *)listFile, "rb");
  if (fp1 == NULL)
  {
    tidl_tb_printf(0, "Could not open %s file for reading \n", listFile);
    return 0;
  }
  for (i = 0; i < frameCount; i++)
  {
    fgetStatus = FGETS(line, TIDL_CFG_MAX_LINE_SIZE, fp1);
    if ( fgetStatus == NULL )
    {
      return 1;
    }
  }
  fgetStatus = FGETS(line, TIDL_CFG_MAX_LINE_SIZE, fp1);
  for (i = 0; i <= fileIdx; i++)
  {
    status = sscanf(linePtr, "%s", currFile);
    linePtr = moveToNextElement((char *)currFile,linePtr);
  }
  tidl_tb_printf(2, "%s\n", currFile);
  FCLOSE(fp1);
  return status;
}

const sTIDL_DataParams_t *tidl_getDataBufDims(const sTIDL_Network_t *net, int32_t dataBuffId)
{
  int32_t i, j;
  for (i = 0; i < net->numLayers; i++)
  {
    for (j = 0; j < net->TIDLLayers[i].numOutBufs; j++)
    {
      if ((net->TIDLLayers[i].outData.dataId == dataBuffId))
      {

        return &net->TIDLLayers[i].outData;
      }
    }
  }
  return NULL;
}

int32_t tidl_tb_getLayerIdOfDataId(sTIDL_Network_t *net, int32_t dataId)
{
  int32_t i, j;
  for (i = 0; i < net->numLayers; i++)
  {
    for (j = 0; j < net->TIDLLayers[i].numOutBufs; j++)
    {
      if ((net->TIDLLayers[i].outData.dataId == dataId))
      {

        return i;
      }
    }
  }
  return -1;
}


int32_t genRandNumS32(uint64_t* pseed)
{
  uint64_t seed = *pseed;
  int32_t curRandVal;

  seed = seed*0x3;
  *pseed = seed;
  curRandVal = seed & 0x7fffffff;

  return curRandVal;
}

int32_t checkForPadRegion(int32_t wIndex, int32_t hIndex,
  int32_t width, int32_t height,
  int32_t padW, int32_t padH)
  {
    int32_t padRegion = 0 ;
    /* Padding pattern assumed
    pppppppppppp
    pvvvvvvvvvvv
    pvvvvvvvvvvv
    pvvvvvvvvvvv
    pppppppppppp
    p
    */
    if((hIndex < padH) || (hIndex > (height - 2*padH)) || (wIndex < padW))
      padRegion = 1;
    return padRegion ;
  }

int32_t initRandomDataS8(int8_t *ptr, int16_t batch, int16_t n,
                          int16_t width, int16_t height, int16_t pitch,
                          int32_t chOffset, int32_t padW, int32_t padH, int8_t randomType)
{
  int32_t   i0, i1, i2, i3;
  uint64_t seed = 0x0a0b0c0d0e0fefd;
  int32_t run_count = 0;
  int32_t isPadRegion = 0 ;

  for(i2 = 0; i2 < batch; i2++)
  {
    for(i0 = 0; i0 < n; i0++)
    {
      for(i1 = 0; i1 < height; i1++)
      {
        for(i3 = 0; i3 < width; i3++)
        {
          int8_t *wAddr = &ptr[i2 * n * chOffset + i0 * chOffset + i1 * pitch + i3];
          isPadRegion = checkForPadRegion(i3, i1, width, height, padW, padH);
          if (isPadRegion)
          {
            *wAddr = (int8_t)(0x66); // some data other than zero
          }
          else
          {
            if (randomType == 0) // Pure random
              *wAddr = (int8_t)genRandNumS32(&seed);
            else if (randomType == 1) // Pattern
              *wAddr = (int8_t)(run_count++);
          }
        }
      }
    }
  }
  return(0);
}

int32_t initRandomDataS16(int16_t *ptr, int16_t batch, int16_t n,
                          int16_t width, int16_t height, int16_t pitch,
                          int32_t chOffset,  int32_t padW, int32_t padH, int8_t randomType)
{
  int32_t   i0, i1, i2, i3;
  uint64_t seed = 0x0a0b0c0d0e0fefd;
  int32_t run_count = 0;
  int32_t isPadRegion = 0 ;

  for(i2 = 0; i2 < batch; i2++)
  {
    for (i0 = 0; i0 < n; i0++)
    {
      for (i1 = 0; i1 < height; i1++)
      {
        for (i3 = 0; i3 < width; i3++)
        {
          int16_t *wAddr = &ptr[i2 * n * chOffset + i0 * chOffset + i1 * pitch + i3];
          isPadRegion = checkForPadRegion(i3, i1, width, height, padW, padH);
          if (isPadRegion)
          {
            *wAddr = (int16_t)(0x6666); // some data other than zero
          }
          else
          {
            if (randomType == 0) // Pure random
              *wAddr = (int16_t)genRandNumS32(&seed);
            else if (randomType == 1) // Pattern
              *wAddr = (int16_t)(run_count++);
          }
        }
      }
    }
  }

  return(0);

}

static inline double tidl_tb_sat(double val, double min, double max)
{
    double out;
    out = (val<min) ? min : val;
    out = (out>max) ? max : out;
    return out;
}

void tidl_tb_dataConvert(void *src, void *dst, int32_t src_offset, int32_t dst_offset, int32_t batch,
                         int32_t nd1, int32_t nd2, int32_t nc, int32_t nl, int32_t np,
                         int32_t src_rp, int32_t dst_rp, int32_t src_d1p, int32_t dst_d1p, int32_t src_d2p, int32_t dst_d2p, int32_t src_cp, int32_t dst_cp, int32_t src_lp, int32_t dst_lp, int32_t src_pp, int32_t dst_pp,
                         float in_zf, float out_zf, float in_scale, float out_scale, int32_t in_type, int32_t out_type)
{
    int32_t ii, i0, i1, i2, i3, i4, in_idx, out_idx;
    float data, scale = out_scale / in_scale;
    for  (ii = 0; ii < batch; ii++)
    {
      for (i0 = 0; i0 < nd1; i0++)
      {
        for (i1 = 0; i1 < nd2; i1++)
        {
          for (i2 = 0; i2 < nc; i2++)
          {
              for (i3 = 0; i3 < nl; i3++)
              {
                  for (i4 = 0; i4 < np; i4++)
                  {
                    in_idx = src_offset + ii * src_rp + i0 * src_d1p + i1 * src_d2p + i2 * src_cp + i3 * src_lp + i4 * src_pp;
                    out_idx = dst_offset + ii * dst_rp + i0 * dst_d1p + i1 * dst_d2p + i2 * dst_cp + i3 * dst_lp + i4 * dst_pp;
                    /** No scale handling, direct copy condition */
                    if (in_type == out_type && 
                        scale == 1.0 && in_zf == out_zf && in_zf == 0)
                    {
                      if(in_type == TIDL_UnsignedChar)
                      {
                        ((uint8_t*)dst)[out_idx] = ((uint8_t *)src)[in_idx];
                      }
                      else if(in_type ==  TIDL_SignedChar)
                      {
                        ((int8_t*)dst)[out_idx] = ((int8_t*)src)[in_idx];
                      }
                      else if(in_type ==  TIDL_UnsignedShort)
                      {
                        ((uint16_t*)dst)[out_idx] = ((uint16_t*)src)[in_idx];
                      }
                      else if(in_type ==  TIDL_SignedShort)
                      {
                        ((int16_t*)dst)[out_idx] = ((int16_t*)src)[in_idx];
                      }
                      else if(in_type ==  TIDL_UnsignedWord)
                      {
                        ((uint32_t*)dst)[out_idx] = ((uint32_t*)src)[in_idx];
                      }
                      else if(in_type ==  TIDL_SignedWord)
                      {
                        ((int32_t*)dst)[out_idx] = ((int32_t*)src)[in_idx];
                      }
                      else if(in_type == TIDL_SinglePrecFloat)
                      {
                        ((float*)dst)[out_idx] = ((float*)src)[in_idx];
                      }
                      else if(in_type ==  TIDL_SignedDoubleWord)
                      {
                        ((int64_t*)dst)[out_idx] = ((int64_t*)src)[in_idx];
                      }
                      else if(in_type ==  TIDL_UnsignedDoubleWord)
                      {
                        ((uint64_t*)dst)[out_idx] = ((uint64_t*)src)[in_idx];
                      }
                      else if(in_type ==  TIDL_Bool)
                      {
                        ((bool*)dst)[out_idx] = ((bool*)src)[in_idx];
                      }
                      else
                      {
                        TIDLTB_ASSERT_MSG(0);
                      }
                      continue;
                    }
                    if (in_type == TIDL_UnsignedChar)
                    {
                      data = ((uint8_t *)src)[in_idx];
                    }
                    else if(in_type ==  TIDL_SignedChar)
                    {
                      data = ((int8_t*)src)[in_idx];
                    }
                    else if(in_type ==  TIDL_UnsignedShort)
                    {
                      data = ((uint16_t*)src)[in_idx];
                    }
                    else if(in_type ==  TIDL_SignedShort)
                    {
                      data = ((int16_t*)src)[in_idx];
                    }
                    else if(in_type ==  TIDL_UnsignedWord)
                    {
                      data = ((uint32_t*)src)[in_idx];
                    }
                    else if(in_type ==  TIDL_SignedWord)
                    {
                      data = ((int32_t*)src)[in_idx];
                    }
                    else if(in_type == TIDL_SinglePrecFloat)
                    {
                      data = ((float*)src)[in_idx];
                    }
                    else if(in_type ==  TIDL_SignedDoubleWord)
                    {
                      data = ((int64_t*)src)[in_idx];
                    }
                    else if(in_type == TIDL_UnsignedDoubleWord)
                    {
                      data = ((uint64_t*)src)[in_idx];
                    }
                    else if(in_type == TIDL_Bool)
                    {
                      data = ((bool*)src)[in_idx];
                    }
                    else
                    {
                      TIDLTB_ASSERT_MSG(0);
                    }
                    data = ((data - in_zf) * scale + out_zf);
                    if(out_type ==  TIDL_UnsignedChar)
                    {
                      ((uint8_t*)dst)[out_idx] = (uint8_t)tidl_tb_sat(data, 0.0, (double)UINT8_MAX);
                    }
                    else if(out_type ==  TIDL_SignedChar)
                    {
                      ((int8_t*)dst)[out_idx] = (int8_t)tidl_tb_sat(data, (double)INT8_MIN, (double)INT8_MAX);
                    }
                    else if(out_type ==  TIDL_UnsignedShort)
                    {
                      ((uint16_t*)dst)[out_idx] = (uint16_t)tidl_tb_sat(data, 0.0, (double)UINT16_MAX);
                    }
                    else if(out_type ==  TIDL_SignedShort)
                    {
                      ((int16_t*)dst)[out_idx] = (int16_t)tidl_tb_sat(data, (double)INT16_MIN, (double)INT16_MAX);
                    }
                    else if(out_type ==  TIDL_UnsignedWord)
                    {
                      ((uint32_t*)dst)[out_idx] = (uint32_t)tidl_tb_sat(data, 0.0, (double)UINT32_MAX);
                    }
                    else if(out_type ==  TIDL_SignedWord)
                    {
                      ((int32_t*)dst)[out_idx] = (int32_t)tidl_tb_sat(data, (double)INT32_MIN, (double)INT32_MAX);
                    }
                    else if(out_type == TIDL_SinglePrecFloat)
                    {
                      ((float*)dst)[out_idx] = (float)tidl_tb_sat(data, -FLT_MAX, FLT_MAX);
                    }
                    else if(out_type ==  TIDL_SignedDoubleWord)
                    {
                      ((int64_t*)dst)[out_idx] = (int64_t)tidl_tb_sat(data, (double)INT64_MIN, (double)INT64_MAX);
                    }
                    else if(out_type ==  TIDL_UnsignedDoubleWord)
                    {
                      ((uint64_t*)dst)[out_idx] = (uint64_t)tidl_tb_sat(data, 0.0, (double)UINT64_MAX);
                    }
                    else if(out_type ==  TIDL_Bool)
                    {
                      ((bool*)dst)[out_idx] = (bool)tidl_tb_sat(data, 0.0, 1.0); 
                    }
                    else
                    {
                      TIDLTB_ASSERT_MSG(0);
                    }
                  }
              }
          }
        }
      }
    }
}


int32_t readRAWData(TI_FILE *fptr, void *ptr, int32_t batch, int32_t n, int32_t dim1,
                    int32_t dim2, int32_t width, int32_t height, int32_t elemSize)
{
  int32_t   freadStatus = 0;
  freadStatus = FREAD(ptr,1,width*height*n*dim2*dim1*batch*elemSize, fptr);
  if(freadStatus < (width*height*n*dim2*dim1*batch*elemSize))
  {
    memset(((uint8_t*)ptr)+freadStatus,0x0,width*height*n*dim2*dim1*batch*elemSize-freadStatus);
    return 0;
  }
  return 1;
}

//Usage is to get Net file size or IO Desc file size
int32_t tidl_getFileSize(uint8_t * fileString)
{
  TI_FILE * fptr;
  int32_t netSize;

  fptr = FOPEN((const char *)fileString, "rb");
  if (fptr)
  {
    FSEEK(fptr, 0L, SEEK_END);
    netSize = FTELL(fptr);
    FCLOSE(fptr);
    return netSize;
  }
  else
  {
    tidl_tb_printf(0, "Could Not Open Files %s\n", fileString);
  }
  return -1;
}

int32_t tidl_getTotalStaticNetworks(void* ptr)
{
  int32_t totalNetworks;
  TIDL_binHeader *headerStruct;
  headerStruct = (TIDL_binHeader*) ptr; //BasePtr has the header info
  totalNetworks = headerStruct->currentNet;
  return totalNetworks;
}

int32_t tidl_getStaticOffset(void* ptr)
{
  int32_t offset = 0;
//  int32_t totalNetworks;
  int32_t i;
  TIDL_binHeader *headerStruct;
  headerStruct = (TIDL_binHeader*) ptr; //BasePtr has the header info
  offset = sizeof(TIDL_binHeader);
 // totalNetworks = headerStruct->currentNet;
  for(i = 0; i < currentNetwork; i++)
  {
    offset += headerStruct->networkSizes[i];
  }
  return offset;
}



int32_t tidl_writeMultiNetworkBin(uint8_t *data_ptr, int8_t * FileName, int32_t data_size)
{
  //Open the file top append to
  TI_FILE * fp;
  TIDL_binHeader *headerStruct;
  headerStruct = (TIDL_binHeader*) malloc(sizeof(TIDL_binHeader));
  if(headerStruct == NULL)
  {
    printf("Error allocating header struct");
  }
  //int32_t fileLength;
  int32_t writeLength;
  strcpy((char *)headerFileName, (char *)FileName); //Global scratch for this name being used..
  strcat((char *)headerFileName, ".bin");
  if(currentNetwork == 0)//First network in config
  {
    fp = FOPEN((const char*)headerFileName, "wb+"); //Create a new file
  }
  else
  {
    fp = FOPEN((const char*)headerFileName, "rb+"); //Open an existing file
  }

  if(fp == NULL)
  {
    printf("Error Opening %s", headerFileName);
    return -1;
  }

  if(currentNetwork == 0)
  {
    //First network handling
    //printf("Adding data for the first n/w with header = %d wide\n",sizeof(TIDL_binHeader));
    headerStruct->currentNet = 0;
    headerStruct->networkSizes[headerStruct->currentNet] = data_size; //This is always in bytes (IMPLICIT ASSUMPTION)
    headerStruct->currentNet++; //Prepare for the next layer
    //Write header
    writeLength = FWRITE(headerStruct, 1, sizeof(TIDL_binHeader), fp);
    if(writeLength != sizeof(TIDL_binHeader))
    {
      printf("Error writing header in the .bin file\n");
      FCLOSE(fp);
      return -1;
    }
    //Write actual data
    writeLength = FWRITE(data_ptr, sizeof(uint8_t), data_size, fp);
    if(writeLength != data_size)
    {
      printf("Error writing data to .bin file");
      FCLOSE(fp);
      return -1;
    }
  }
  else
  {
    //Write to the end actual data.
    FSEEK(fp, 0, SEEK_END);
    writeLength = FWRITE(data_ptr, sizeof(uint8_t), data_size, fp);
    if(writeLength != data_size)
    {
      printf("Error writing data to .bin file");
      FCLOSE(fp);
      return -1;
    }
    //Get the header information:
    FSEEK(fp, 0, SEEK_SET);
    writeLength = FREAD(headerStruct, 1, sizeof(TIDL_binHeader),fp);
    if(writeLength != sizeof(TIDL_binHeader))
    {
      printf("Error writing header in the .bin file\n");
      FCLOSE(fp);
      return -1;
    }
    //Update headerStruct:
    headerStruct->networkSizes[headerStruct->currentNet] = data_size;
    headerStruct->currentNet++;
    //Write this data back:
    FSEEK(fp, 0, SEEK_SET);
    writeLength = FWRITE(headerStruct, 1, sizeof(TIDL_binHeader), fp);
    if(writeLength != sizeof(TIDL_binHeader))
    {
      printf("Error writing header in the .bin file\n");
      FCLOSE(fp);
      return -1;
    }
  }

    free(headerStruct);
    FCLOSE(fp);

    tidl_buffer_printf_dump();
    return 0;

  }


int32_t tidl_writeAsStaticArray(uint8_t * ptr, int8_t * FileName, int32_t size, const char arrayName[], const char secName[], uint32_t isDataConst, uint32_t dumpAsBin)
{
  TI_FILE * fp;
  strcpy((char *)headerFileName, (char *)FileName);
  strcat((char *)headerFileName, ".c");
  fp = FOPEN((const char *)headerFileName, "w+");
  if (fp == NULL)
  {
    tidl_tb_printf(0, "Error in opening %s File !!!!\n", headerFileName);
  }
  else
  {
    FPRINTF(fp, "#include <stdint.h>\n");
    int32_t ctr;
    //Write luma data
    FPRINTF(fp, "#if HOST_EMULATION\n");
    FPRINTF(fp, "uint8_t  %s[%d] ", arrayName, size);
    if ( dumpAsBin == 0)
    {
      FPRINTF(fp, "= {\n");
    }
    else
    {
      FPRINTF(fp, ";\n");
    }
    FPRINTF(fp, "#else\n");
    FPRINTF(fp,"#define PRAGMA(x) _Pragma(#x)\n");
    FPRINTF(fp,"#ifdef __cplusplus\n");
    FPRINTF(fp,"#define CSPRAGMA(f,s) PRAGMA(DATA_SECTION(s))\n");
    FPRINTF(fp,"#else\n");
    FPRINTF(fp,"#define CSPRAGMA(f,s) PRAGMA(DATA_SECTION(f, s))\n");
    FPRINTF(fp,"#endif\n");

    FPRINTF(fp, "CSPRAGMA(%s, \".%s\")\n", arrayName, secName);
    if ( isDataConst )
    {
      FPRINTF(fp, "const ");
    }
    FPRINTF(fp, "uint8_t  %s[%d]", arrayName, size);
    if ( dumpAsBin == 0)
    {
      FPRINTF(fp, "= {\n");
    }
    else
    {
      FPRINTF(fp, ";\n");
    }
    FPRINTF(fp, "#endif\n");

    if ( dumpAsBin == 0 )
    {
    for (ctr = 0; ctr < (size - 1); ctr++)
      FPRINTF(fp, "%4d,", ptr[ctr]);

    FPRINTF(fp, "%4d\n};\n\n", ptr[ctr]);
    }
    else
    {
      uint8_t binFileName[256];
      TI_FILE * fpBin;
      strcpy((char *)binFileName, (char *)FileName);
      strcat((char *)binFileName, ".bin");
      fpBin = FOPEN((const char *)binFileName, "wb");
      if (fpBin == NULL)
      {
        tidl_tb_printf(0, "Error in opening %s File !!!!\n", binFileName);
      }
      FWRITE( ptr, 1, size, fpBin);
      FCLOSE(fpBin);
    }
    FCLOSE(fp);
  }
  return 0;
}

// Usage is to read Net file or IO Desc file
int32_t tidl_readFile(void * file, uint8_t * fileString)
{
  TI_FILE * fptr;
  int32_t fileSize = tidl_getFileSize(fileString);
  int32_t   freadStatus = 0;

  fptr = FOPEN((const char *)fileString, "rb");
  if(fptr)
  {
    freadStatus = FREAD(file, 1, fileSize, fptr);
    if ( freadStatus == 0 )
    {
      return -1;
    }
    FCLOSE(fptr);
    return 0;
  }
  else
  {
    tidl_tb_printf(0, "Could Not Open Files %s\n",fileString);
  }
  return -1;
}

int32_t tidl_writeNet(sTIDL_Network_t * net, uint8_t * fileString, int32_t modelSize)
{
  TI_FILE * fptr;
  fptr = FOPEN((const char *)fileString, "wb+");
  if(fptr)
  {
    FWRITE(net,1, modelSize, fptr);
    FCLOSE(fptr);
    return 0;
  }
  else
  {
    tidl_tb_printf(0, "Could Not Open Files %s\n",fileString);
    return 1;
  }
}

int32_t tidl_updateNet(sTIDL_Network_t * netDst, sTIDL_Network_t * netSrc, tidl_net_config * params)
{
  int32_t i, j;

  for(i = 0; i < netSrc->numLayers; i++)
  {
    sTIDL_Layer_t *TIDLLayerSrc = &netSrc->TIDLLayers[i];
    sTIDL_Layer_t *TIDLLayerDst = &netDst->TIDLLayers[i];
    if(TIDLLayerSrc->layersGroupId == TIDL_TB_CURR_LAYERS_GROUP_ID)
    {
      /* Update quant stats */
      if(params->updateNetWithStats == TIDL_UPDATE_QUANT_STATS)
      {
        for(j = 0; j < TIDLLayerSrc->numOutBufs; j++)
        {
          TIDLLayerDst->outData.minTensorValue = TIDLLayerSrc->outData.minTensorValue;
          TIDLLayerDst->outData.maxTensorValue = TIDLLayerSrc->outData.maxTensorValue;
          TIDLLayerDst->outData.tensorScale = TIDLLayerSrc->outData.tensorScale;
          TIDLLayerDst->outData.roundBits = TIDLLayerSrc->outData.roundBits;
        }

        if ( TIDLLayerSrc->layerType == TIDL_ConvolutionLayer )
        {
          /* THis is needed for channel wise quantation as this
          varible will store the weightScale of the channel where
          absolute maximum float value is found */
          TIDLLayerDst->layerParams.convParams.weightScale =
            TIDLLayerSrc->layerParams.convParams.weightScale;
        }
      }
    }

    /* Update perfomance modelling stats */
    if((params->updateNetWithStats == TIDL_UPDATE_PERF_MODEL_STATS))
    {
      int32_t profileIdx;
      for(profileIdx = TIDL_PROFILE_LAYER; profileIdx < TIDL_PROFILE_MAX; profileIdx++)
      {
        TIDLLayerDst->profilePoints[profileIdx] = TIDLLayerSrc->profilePoints[profileIdx];
      }
    }
  }

  return 0;
}

int32_t tidl_writeNetWithStats(tidl_net_config * params, sTIDL_Network_t * net)
{
  int32_t netSize = tidl_getFileSize(params->netBinFile);
  sTIDL_Network_t * origNet = (sTIDL_Network_t *)malloc(netSize);
  if (origNet != NULL)
  {
    tidl_readFile(origNet, params->netBinFile);
    tidl_updateNet(origNet, net, params);
    if(params->updateNetWithStats == TIDL_UPDATE_QUANT_STATS)
    {
      origNet->isQuantStatsAvailable = 1;
    }
    tidl_writeNet(origNet, params->netBinFile, netSize);
    free(origNet);
  }
  else
  {
    tidl_tb_printf(0, "tidl_writeNetWithStats: Not able to allocate memory for net structure\n");
  }
  return 0;
}

int32_t tidl_tb_WriteBinToFile(const char * fileName, void * addr, int32_t size, int32_t count)
{
  bool select_layer = false;
  bool convert_to_text = false; // useful in visualizing the float data. Should be enabled in float data only for debugging
  int i;
  TI_FILE * fptr = NULL;

  if (select_layer)
  {
      char tempfileName[300];
      strcpy(tempfileName, fileName);
      int layers[] = {4, 5, 6, 29,30,31};  //specify the layers to be saved
      bool layer_found = false;
      int num_layers = sizeof(layers)/sizeof(layers[0]);
      int curr_layer;
      int32_t elements_per_line;
      char* token = strtok(tempfileName, "_");

      while((token != NULL))
      {
        curr_layer = atoi(token);
        for (i=0;i<num_layers;i++)
        {
            if(curr_layer==layers[i])
            {
              layer_found = true;
              break;
            }
        }
        token = strtok(NULL, "_");
        if(layer_found == true)
        {
          break;
        }
      }
      if(layer_found == true)
      {
        token = strtok(NULL, "_");
        token = strtok(NULL, "_");
        token = strtok(token, "x");
        elements_per_line = atoi(token);
      }

      if(layer_found)
      {
        if(convert_to_text == false)
        {
          fptr = FOPEN((const char *)fileName, "wb+");
          if (fptr)
          {
            FWRITE(addr, 1, size, fptr);
            FCLOSE(fptr);
            return 0;
          }

        }
        else
        {
          fptr = FOPEN((const char *)fileName, "w+");
          int32_t i,j;
          int32_t line_pitch=elements_per_line;
          if (fptr)
          {
            for(i = 0; i< size/4; i+=line_pitch)
            {
              for(j = 0; j < elements_per_line; j++)
              {
                FPRINTF(fptr,"%.2f ", ((float32_tidl*)addr)[i+j]);
              }
              FPRINTF(fptr,"\n");
            }
            FCLOSE(fptr);
            return 0;
          }
          else
          {
            tidl_tb_printf(0, "Could not open %s file for writing \n",fileName);
          }
        }
      }
    }
  else
  {
      TI_FILE * fptr = NULL;
      fptr = FOPEN((const char *)fileName, "wb+");
      if (fptr)
      {
        FWRITE(addr, 1, size, fptr);
        FCLOSE(fptr);
        return 0;
      }
      else
      {
        tidl_tb_printf(0, "Could not open %s file for writing \n",fileName);
      }
  }
  return -1;
}
int32_t tidl_tb_ReadBinFromFile(const char * fileName, void * addr, int32_t size, int32_t count)
{
  TI_FILE * fptr = NULL;
  fptr = FOPEN((const char *)fileName, "rb");
  if(fptr)
  {
    FREAD(addr, size, 1, fptr);
    FCLOSE(fptr);
    return 0;
  }
  else
  {
    tidl_tb_printf(0, "Could not open %s file for reading \n",fileName);
  }
  return -1;
}


TI_FILE * tidl_openNetInput(tidl_net_config *params)
{
  TI_FILE * fp1 = NULL;
#if (!BUILD_WITH_STATIC_ARRAYS)
  fp1 = FOPEN((const char *)params->inData, "rb+");
  if(fp1 == NULL)
  {
    tidl_tb_printf(0, "Could not open %s file for reading \n",params->inData);
  }
#endif
  return fp1;
}


int32_t tidl_ReadNetInput(TI_FILE * fp1, tidl_net_config *params, sTIDL_IOBufDesc_t * ioPrms,
                              sTIDLRT_Tensor_t   *ins[], int32_t frameCount)
{
  uint16_t numBuffs = 0;
#if (!BUILD_WITH_STATIC_ARRAYS)
  int32_t  offset = 0;
#endif
  uint32_t tensorOffset = 0;
  uint32_t tensorSize = 0;
  uint32_t totalSize = 0;
  int32_t channelOffset;

  for(numBuffs = 0; numBuffs < ioPrms->numInputBuf; numBuffs++)
  {
    int32_t c  = ins[numBuffs]->dimValues[TIDL_DIM_NUMCH];
    int32_t w  = ins[numBuffs]->dimValues[TIDL_DIM_WIDTH];
    int32_t h  = ins[numBuffs]->dimValues[TIDL_DIM_HEIGHT];
    int32_t d2 = ins[numBuffs]->dimValues[TIDL_DIM_DIM2];
    int32_t d1 = ins[numBuffs]->dimValues[TIDL_DIM_DIM1];
    int32_t n  = ins[numBuffs]->dimValues[TIDL_DIM_BATCH];
    int32_t elementSizeBytes = tidltb_getDatElementSize(ins[numBuffs]->elementType);

#if (BUILD_WITH_STATIC_ARRAYS)
//    ins[numBuffs]->ptr = (void *)(tidl_tb_input_tensor + tensorOffset); //Irrelevant since post RT Input is considered only
#else
    if (params->inFileFormat < 3)
    {
      uint8_t *ptr = NULL;
      int32_t inElementType;
      float inTensorScale;
      float inZeroPoint;

      int32_t dst_rp = ins[numBuffs]->pitch[TIDL_ROI_PITCH];
      int32_t dst_d2p = ins[numBuffs]->pitch[TIDL_DIM2_PITCH];
      int32_t dst_d1p = ins[numBuffs]->pitch[TIDL_DIM1_PITCH];
      int32_t dst_cp = ins[numBuffs]->pitch[TIDL_CHANNEL_PITCH];
      int32_t dst_lp = ins[numBuffs]->pitch[TIDL_LINE_PITCH];
      int32_t dst_pp = 1;
      int32_t dst_offset =  ins[numBuffs]->padValues[2] * dst_lp + ins[numBuffs]->padValues[0];
      float dst_scale = ins[numBuffs]->scale;
      int32_t dst_zp = ins[numBuffs]->zeroPoint;

      if(ins[numBuffs]->layout == TIDL_LT_NHWC)
      {
        dst_cp = 1;
        dst_pp = c;
        dst_lp = c*w;
      }

      if (params->inFileFormat == 1)
      {
        elementSizeBytes = tidltb_getDatElementSize(ioPrms->rawDataInElementType[numBuffs]);
        ptr = (uint8_t *)malloc(n * d1 * d2 * c * h * w * elementSizeBytes);
        TIDLTB_ASSERT_EXIT(ptr);
        int32_t raw_data_size_sufficient = readRAWData(fp1, ptr, n, d1, d2, c, w, h, elementSizeBytes);
        if(raw_data_size_sufficient == 0)
        {
          printf("Warning :: File read for binary data load is not suffcient in size to fill the input tensor, Filling it with zero \n");
        }
        inElementType = ioPrms->rawDataInElementType[numBuffs];
        inTensorScale = ins[numBuffs]->scale;
        inZeroPoint   = ins[numBuffs]->zeroPoint;
        if(inElementType == TIDL_SinglePrecFloat)
        {
          inTensorScale = 1.0;
          inZeroPoint   = 0;
        }
        if(ins[numBuffs]->elementType == TIDL_SinglePrecFloat)
        {
          dst_scale = 1.0;
          dst_zp = 0;
        }
      }
      else if ((params->inFileFormat == 0) || (params->inFileFormat == 2))
      {
        char imageFileName[500];
        if (params->inFileFormat == 0)
        {
          strcpy(imageFileName, (const char *)params->inData);
        }
        else
        {
          uint32_t numBatches = ioPrms->numVirtualCores * ioPrms->numSuperBatches;
          uint32_t numInputsPerCore = ioPrms->numInputBuf/numBatches;
          getFileNameAndIdxFromList((char*)params->inData, frameCount + numBuffs/numInputsPerCore, (numBuffs % numInputsPerCore), imageFileName);
        }
        /**Should extend this to 6D if needed */
        ptr = (uint8_t*)malloc(n*c*h*w*sizeof(uint8_t));
        TIDLTB_ASSERT_EXIT(ptr);
        readImage((uint8_t*)imageFileName, ptr, offset, c, w, h, numBuffs, params->inResizeMode);
        inElementType = TIDL_UnsignedChar;
        inTensorScale = 1.0;
        inZeroPoint   = 0.0;
        elementSizeBytes = sizeof(float);
      }
      tidl_tb_dataConvert((void *)ptr, (void *)ins[numBuffs]->ptr, 0, dst_offset, n, d1, d2, c, h, w,
                          d1 * d2 * c * h * w, dst_rp,
                          d2 * c * h * w, dst_d1p,
                          c * h * w, dst_d2p,
                          h * w, dst_cp,
                          w, dst_lp,
                          1, dst_pp,
                          inZeroPoint, dst_zp, inTensorScale, dst_scale, inElementType, ins[numBuffs]->elementType);
      free(ptr);
    }
    else if (params->inFileFormat == 4)
    {
        int32_t padW = ins[numBuffs]->padValues[3] ;
        int32_t padH = ins[numBuffs]->padValues[3] ;

        if (gIOParams.inElementType[numBuffs] < TIDL_UnsignedShort)
        {
          initRandomDataS8(((int8_t *)ins[numBuffs]->ptr), n,
            c, w, h, w, w*h, padW, padH, params->inFileFormatRandomType);
        }
        else if (gIOParams.inElementType[numBuffs] < TIDL_UnsignedWord)
        {
          initRandomDataS16(((int16_t *)ins[numBuffs]->ptr), n,
            c, w, h, w, w*h, padW, padH,  params->inFileFormatRandomType);
        }
        else
        {
          tidl_tb_printf(0, "inFileFormat == 4 is only supported for 8 and 16-bit Mode, random data with float mode may create seg faults\n");
          exit(-1);
        }
    }
    else /* (params->inFileFormat == 3) */
    {
      TIDLTB_ASSERT_EXIT(0);
    }
#endif
    tensorSize = n*d1*d2*c*h*w*elementSizeBytes;
    tensorOffset += tensorSize ;
  }

  if (params->writeBinsAsHeader)
  {
    uint8_t * inPtr = NULL;

    totalSize = tensorOffset;
    inPtr = (uint8_t *)malloc(totalSize);
    TIDLTB_ASSERT_EXIT(inPtr != NULL);
    tensorOffset = 0;
    for (numBuffs = 0; numBuffs < gIOParams.numInputBuf; numBuffs++)
    {
      channelOffset = gIOParams.inChannelPitch[numBuffs];
      tensorSize   =  gIOParams.inNumBatches[numBuffs] * gIOParams.inDIM1[numBuffs] * gIOParams.inDIM2[numBuffs] * gIOParams.inNumChannels[numBuffs] * channelOffset *tidltb_getDatElementSize(gIOParams.inElementType[numBuffs]);
      memcpy(inPtr + tensorOffset, ins[numBuffs]->ptr, tensorSize);
      tensorOffset += tensorSize;
    }
    //tidl_writeAsStaticArray((uint8_t *)inPtr, (int8_t *)"./tidl_tb_input_tensor", totalSize, "tidl_tb_input_tensor", "tidl_tb_input_tensor", 1U, 0); //Not needed anymore..
    if ( inPtr )
    {
      free(inPtr);
    }
  }
  return 0;
}


int32_t tidl_allocInOutTensors(sTIDL_IOBufDesc_t *ioPrms, sTIDLRT_Tensor_t *ins[], sTIDLRT_Tensor_t *outs[])
{
  int32_t i;
  int32_t status = 0, size = 0;

  if(status == 0)
  {
    for(i = 0; i < ioPrms->numOutputBuf; i++)
    {
      status = TIDLRT_setTensorDefault(outs[i]);
      size = ioPrms->outBufSize[i];
      outs[i]->bufferSize  = ioPrms->outBufSize[i]; /* Actual buffer size needed by TIDL as specified by ioPrms->outBufSize is allocated by application */
      outs[i]->elementType = ioPrms->outElementType[i];
      int32_t elementSizeBytes = tidltb_getDatElementSize(outs[i]->elementType);
      size *=  elementSizeBytes;
      outs[i]->ptr = (void *)calloc(size, 1);
      if(outs[i]->ptr == NULL)
      {
        status = 1;
        break;
      }
      outs[i]->scale = ioPrms->outTensorScale[i];
      outs[i]->zeroPoint = ioPrms->outZeroPoint[i];
      outs[i]->layout = ioPrms->outLayout[i];
      outs[i]->pitch[TIDL_LINE_PITCH]    = ioPrms->outPadL[i] + ioPrms->outWidth[i] + ioPrms->outPadR[i];
      outs[i]->pitch[TIDL_CHANNEL_PITCH] = ioPrms->outChannelPitch[i];
      outs[i]->pitch[TIDL_DIM2_PITCH]    = outs[i]->pitch[TIDL_CHANNEL_PITCH] * ioPrms->outNumChannels[i];;
      outs[i]->pitch[TIDL_DIM1_PITCH]    = outs[i]->pitch[TIDL_DIM2_PITCH] * ioPrms->outDIM2[i];
      outs[i]->pitch[TIDL_ROI_PITCH]     = outs[i]->pitch[TIDL_DIM1_PITCH] * ioPrms->outDIM1[i];
      outs[i]->padValues[0] = ioPrms->outPadL[i];
      outs[i]->padValues[1] = ioPrms->outPadR[i];
      outs[i]->padValues[2] = ioPrms->outPadT[i];
      outs[i]->padValues[3] = ioPrms->outPadB[i];
      outs[i]->dimValues[TIDL_DIM_WIDTH]   = ioPrms->outWidth[i];
      outs[i]->dimValues[TIDL_DIM_HEIGHT]  = ioPrms->outHeight[i];
      outs[i]->dimValues[TIDL_DIM_NUMCH]   = ioPrms->outNumChannels[i];
      outs[i]->dimValues[TIDL_DIM_DIM2]   = ioPrms->outDIM2[i];
      outs[i]->dimValues[TIDL_DIM_DIM1]   = ioPrms->outDIM1[i];
      outs[i]->dimValues[TIDL_DIM_BATCH]   = ioPrms->outNumBatches[i];
      outs[i]->isDynamic = ioPrms->outIsDynamic[i];
   }
  }
  if(status == 0)
  {
    for(i = 0; i < ioPrms->numInputBuf; i++)
    {
      status = TIDLRT_setTensorDefault(ins[i]);
      size = ioPrms->inBufSize[i];
      ins[i]->bufferSize  = ioPrms->inBufSize[i]; /* Actual buffer size needed by TIDL as specified by ioPrms->outBufSize is allocated by application */
      int32_t elementSizeBytes;
      ins[i]->elementType = ioPrms->inElementType[i];
      elementSizeBytes = tidltb_getDatElementSize(ins[i]->elementType);
      size *= elementSizeBytes;
      ins[i]->ptr = (void *)calloc(size,1);
      if(ins[i]->ptr == NULL)
      {
        status = 1;
        break;
      }
      ins[i]->scale = ioPrms->inTensorScale[i];
      ins[i]->zeroPoint = ioPrms->inZeroPoint[i];
      ins[i]->layout = ioPrms->inLayout[i];
      ins[i]->pitch[TIDL_LINE_PITCH]    = ioPrms->inPadL[i] + ioPrms->inWidth[i] + ioPrms->inPadR[i];
      ins[i]->pitch[TIDL_CHANNEL_PITCH] = ioPrms->inChannelPitch[i];
      ins[i]->pitch[TIDL_DIM2_PITCH]    = ins[i]->pitch[TIDL_CHANNEL_PITCH] * ioPrms->inNumChannels[i];;
      ins[i]->pitch[TIDL_DIM1_PITCH]    = ins[i]->pitch[TIDL_DIM2_PITCH] * ioPrms->inDIM2[i];
      ins[i]->pitch[TIDL_ROI_PITCH]     = ins[i]->pitch[TIDL_DIM1_PITCH] * ioPrms->inDIM1[i];
      ins[i]->padValues[0] = ioPrms->inPadL[i];
      ins[i]->padValues[1] = ioPrms->inPadR[i];
      ins[i]->padValues[2] = ioPrms->inPadT[i];
      ins[i]->padValues[3] = ioPrms->inPadB[i];
      ins[i]->dimValues[TIDL_DIM_WIDTH]   = ioPrms->inWidth[i];
      ins[i]->dimValues[TIDL_DIM_HEIGHT]  = ioPrms->inHeight[i];
      ins[i]->dimValues[TIDL_DIM_NUMCH]   = ioPrms->inNumChannels[i];
      ins[i]->dimValues[TIDL_DIM_DIM2]  = ioPrms->inDIM2[i];
      ins[i]->dimValues[TIDL_DIM_DIM1]  = ioPrms->inDIM1[i];
      ins[i]->dimValues[TIDL_DIM_BATCH]   = ioPrms->inNumBatches[i];
      strcpy((char*)ins[i]->name,(char*)ioPrms->inDataName[i]);
      ins[i]->isDynamic = ioPrms->inIsDynamic[i];
      /* TODO: Dynamic input support in tidl_tb
       * Currently dimValues and pitches are always set from ioBufDesc max dims, and
       * tidl_ReadNetInput fills ins[i]->ptr with the ioBufDesc layout (including padding).
       * For true dynamic input support, the test bench would need to:
       *   1. Set ins[i]->dimValues to the actual input dims for this inference
       *   2. Set ins[i]->pitch[TIDL_CHANNEL_PITCH] = actual_W * actual_H (packed)
       *   3. Set ins[i]->padValues = 0 (source is flat)
       *   4. Fill ins[i]->ptr with actual-sized flat data
       * The datamove reformat path will then correctly copy actual data into the
       * C7x input buffer using the ioBufDesc destination layout. */
    }
  }

  return status;

}

int32_t tidl_freeInOutTensors(sTIDL_IOBufDesc_t * ioPrms, sTIDLRT_Tensor_t * ins[], sTIDLRT_Tensor_t * outs[])
{
  int32_t i;
  int32_t status = 0;

  if(status == 0)
  {
    for(i = 0; i < ioPrms->numOutputBuf; i++)
    {
      if(outs[i]->ptr == NULL)
      {
        status = 1;
        break;
      }
      free(outs[i]->ptr);
    }
  }
  if(status == 0)
  {
    #if (!BUILD_WITH_STATIC_ARRAYS)
    for(i = 0; i < ioPrms->numInputBuf; i++)
    {
      if(ins[i]->ptr == NULL)
      {
        status = 1;
        break;
      }
      free(ins[i]->ptr);

    }
    #endif
  }
  return status;

}

#if (!BUILD_WITH_STATIC_ARRAYS)
int32_t tidl_CompareNetOutputMem(tidl_net_config *params, sTIDL_IOBufDesc_t *ioPrms, sTIDLRT_Tensor_t * out[])
{
  int32_t i;
  int8_t * readPtr;
  int32_t status, readSize = 0;

  if(firstOutWrite)
  {
    refCompareFile = FOPEN((const char *)params->outData, "rb");
    firstOutWrite = 0;
  }

  if(refCompareFile == NULL)
  {
    tidl_tb_printf(0, "Could not open %s file for Refernce comparing \n",params->outData);
  }

  for(i = 0; i < ioPrms->numOutputBuf; i++)
  {
    readSize  =  ioPrms->outNumChannels[i] * ioPrms->outHeight[i] * ioPrms->outWidth[i];
    readSize *= sizeof(float);

    readPtr = (int8_t *)malloc(readSize);
    if ( readPtr == NULL )
    {
        tidl_tb_printf(0, "Could Allocate memory for Refernce comparing \n");
        exit(-1);
    }

    FREAD(readPtr, 1, readSize, refCompareFile);
    status = memcmp(readPtr,out[i]->ptr, readSize);
    free(readPtr);
    if (status)
    {
      tidl_tb_printf(0, "TIDL_DBUG_TRACE :  %s, %s, %d FAILED !!!!!!!!!!!\n", __FUNCTION__, __FILE__, __LINE__);
      exit(-1);
    }

  }
  tidl_tb_printf(0, "TIDL_DBUG_TRACE :  %s, %s, %d PASSED n", __FUNCTION__, __FILE__, __LINE__);

  return 0;
}
#else
int32_t tidl_CompareNetOutputMem(tidl_net_config *params, sTIDL_IOBufDesc_t * ioPrms, sTIDLRT_Tensor_t * out[])
{
  #if (!BUILD_WITH_STATIC_ARRAYS) //Dummy function now..
  int32_t i;
//  uint16_t nuChs;
 // uint32_t pitch;
  //uint32_t chOffset;
  int8_t * readPtr;
  int32_t status, tensorOffset = 0, readSize = 0;

  for(i = 0; i < ioPrms->numOutputBuf; i++)
  {
    readSize  =  ioPrms->outNumChannels[i] * ioPrms->outHeight[i] * ioPrms->outWidth[i];
    readSize *= tidltb_getDatElementSize(ioPrms->outElementType[i]);;

    readPtr = (int8_t *)tidl_tb_output_tensor + tensorOffset;
    tensorOffset += readSize;

    #if 1
    status = memcmp(readPtr,out[i]->ptr, readSize);
    #else
    status = tidl_C7xMemCmp(readPtr,out[i]->ptr, readSize);
    #endif

    if (status)
    {
      tidl_tb_printf(0, "Failure at position %d for tensor %d\n",status, i);
    }

  }
  if(!status)
  tidl_tb_printf(0, "TIDL_DBUG_TRACE :  %s, %s, %d PASSED n", __FUNCTION__, __FILE__, __LINE__);
  return 0;
  #endif
  return 0;
}
#endif

int32_t tidl_NetOutputPostproc(tidl_net_config *params, sTIDL_IOBufDesc_t * ioPrms, sTIDLRT_Tensor_t * out[], int32_t frameCount)
{
  int32_t i;
  if(params->postProcType)
  {
    int32_t numBatches = ioPrms->numVirtualCores * ioPrms->numSuperBatches;
    int32_t numOutputsPerCore = ioPrms->numOutputBuf / numBatches;
    for(i = 0; i < ioPrms->numOutputBuf; i++)
    {
      if((i % numOutputsPerCore) == params->postProcDataId)
      {
        tidl_tb_postProc(ioPrms->outWidth[i], ioPrms->outHeight[i], ioPrms->outNumChannels[i], frameCount + i/numOutputsPerCore, out[i]->ptr, out[i]->elementType);
      }
    }
  }
  return 0;
}

int32_t tidl_WriteNetOutputMem (tidl_net_config *params, sTIDL_IOBufDesc_t * ioPrms, sTIDLRT_Tensor_t * out[], int32_t frameCount)
{
  int32_t i;
  int32_t freadStatus = 0;
  int32_t tidlOutputSize = 0;
  int32_t tensorSize = 0;

  TI_FILE * fp1;

  if(firstOutWrite)
  {
    fp1 = FOPEN((const char *)params->outData, "wb+");
    if(fp1 == NULL)
    {
      tidl_tb_printf(0, "Could not open %s file for writing \n",params->outData);
      exit(0);
    }
    FCLOSE(fp1);
    firstOutWrite = 0;
  }
  for(i = 0; i < ioPrms->numOutputBuf; i++)
  {
    int32_t n = out[i]->dimValues[TIDL_DIM_BATCH];
    int32_t d1 = out[i]->dimValues[TIDL_DIM_DIM1];
    int32_t d2 = out[i]->dimValues[TIDL_DIM_DIM2];
    int32_t c = out[i]->dimValues[TIDL_DIM_NUMCH];
    int32_t w = out[i]->dimValues[TIDL_DIM_WIDTH];
    int32_t h = out[i]->dimValues[TIDL_DIM_HEIGHT];

    int32_t src_rp =  out[i]->pitch[TIDL_ROI_PITCH];
    int32_t src_d1p = out[i]->pitch[TIDL_DIM1_PITCH];
    int32_t src_d2p = out[i]->pitch[TIDL_DIM2_PITCH];
    int32_t src_cp = out[i]->pitch[TIDL_CHANNEL_PITCH];
    int32_t src_lp = out[i]->pitch[TIDL_LINE_PITCH];
    int32_t src_pp = 1;

    int32_t src_offset =  out[i]->padValues[2] * src_lp + out[i]->padValues[0];
    int32_t src_zf     =  out[i]->zeroPoint;

    if(out[i]->layout == TIDL_LT_NHWC)
    {
      src_cp = 1;
      src_pp = c;
      src_lp = c*w;
    }

    int32_t outElementType     = out[i]->elementType;
    float   outTensorScale = out[i]->scale;
    tensorSize = n*d1*d2*c*h*w;
    int32_t elementSizeBytes = tidltb_getDatElementSize(out[i]->elementType);
    if(params->writeOutput == 2)
    {
      elementSizeBytes = sizeof(float);
      outElementType = TIDL_SinglePrecFloat;
      outTensorScale = 1.0;
    }
    tensorSize  *= elementSizeBytes;

    fp1 = FOPEN((const char *)params->outData, "ab+");
    if(fp1 == NULL)
    {
      tidl_tb_printf(0, "Could not open %s file for writing \n",params->outData);
      exit(0);
    }
    void *ptr = (void *)malloc(tensorSize);
    TIDLTB_ASSERT_EXIT(ptr);

    tidl_tb_dataConvert((void *)out[i]->ptr, (void *)ptr,  src_offset, 0, n, d1, d2, c, h, w,
                        src_rp, d1 * d2 * c * h * w,
                        src_d1p, d2 * c * h * w,
                        src_d2p, c * h * w,
                        src_cp, h * w,
                        src_lp, w,
                        src_pp, 1,
                        src_zf, 0, out[i]->scale, outTensorScale, out[i]->elementType, outElementType);

    FWRITE(ptr,1,tensorSize, fp1);
    free(ptr);
    FCLOSE(fp1);
    tidlOutputSize += tensorSize;
  }
  if (params->writeBinsAsHeader)
  {
    fp1 = FOPEN((const char *)params->outData, "rb");
    TIDLTB_ASSERT_EXIT(fp1 != NULL);
    void * outPtr = malloc(tidlOutputSize);
    freadStatus = FREAD(outPtr, 1, tidlOutputSize, fp1);
    TIDLTB_ASSERT_EXIT(freadStatus != 0);
    //Deprecated..
    //tidl_writeAsStaticArray((uint8_t *)outPtr, (int8_t *)"./tidl_tb_output_tensor", tidlOutputSize, "tidl_tb_output_tensor", "tidl_tb_output_tensor", 1, 0);
    free(outPtr);
    FCLOSE(fp1);
  }
  return 0;
}

int32_t tidltb_printNetInfo(sTIDL_Network_t * pTIDLNetStructure,
                            int32_t layersGroupId)
{
  int32_t i,j;
  tidl_tb_printf(2, "-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
  tidl_tb_printf(2, "%5s|%-30s|%-6s|%-6s|%-6s|%-32s|%-10s|%-36s|%-36s|\n", "Num", "TIDL Layer Name", "Group", "#Ins", "#Outs", "Inbuf Ids", "Outbuf Id", "In NCHW", "Out NCHW");
  tidl_tb_printf(2, "-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");


  for (i = 0 ; i < pTIDLNetStructure->numLayers; i++)
  {
    strcpy(printBuf,"");
    sprintf(printBuf + strlen(printBuf),"%5d|%-30s|",i,
                TIDL_LayerString[pTIDLNetStructure->TIDLLayers[i].layerType]);
    sprintf(printBuf + strlen(printBuf),"%6d|%6d|%6d|",pTIDLNetStructure->TIDLLayers[i].layersGroupId,
                             pTIDLNetStructure->TIDLLayers[i].numInBufs,
                             pTIDLNetStructure->TIDLLayers[i].numOutBufs);

    for (j = 0; j < pTIDLNetStructure->TIDLLayers[i].numInBufs; j++)
    {
      sprintf(printBuf + strlen(printBuf),"%3d ",pTIDLNetStructure->TIDLLayers[i].inData[j]);
    }
    for (j = (pTIDLNetStructure->TIDLLayers[i].numInBufs > 0 ?
          pTIDLNetStructure->TIDLLayers[i].numInBufs : 0); j < 8; j++)
    {
      sprintf(printBuf + strlen(printBuf),"  x ");
    }
    sprintf(printBuf + strlen(printBuf),"|");
    sprintf(printBuf + strlen(printBuf),"%3d       |",pTIDLNetStructure->TIDLLayers[i].outData.dataId);
    for (j = 0; j < TIDL_DIM_MAX; j++) //changed
    {
      sprintf(printBuf + strlen(printBuf),"%8d ",pTIDLNetStructure->TIDLLayers[i].outData.dimValues[j]);
    }
    sprintf(printBuf + strlen(printBuf),"|");
    tidl_tb_printf(2, "%s\n",printBuf);
  }
  //tidl_tb_printf(0, "-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
  return 0;
}

int32_t readIODescFile(char * fileName, sTIDL_IOBufDesc_t * gIOParams)
{
    #if (!BUILD_WITH_STATIC_ARRAYS)
    TI_FILE * fptr;
    int32_t freadStatus =0;
    fptr = FOPEN((const char *)fileName, "rb");
    if (!fptr)
    {
      tidl_tb_printf(0, "Could Not Open Files %s\n", fileName);
      return -1;
    }
    freadStatus = FREAD(gIOParams, 1, sizeof(sTIDL_IOBufDesc_t), fptr);
    FCLOSE(fptr);

    if ( freadStatus == 0 )
    {
      tidl_tb_printf(0, "Could Not Read %s\n", fileName);
      return -1;
    }

    if(gWriteStaticBins)
    {
      //Write out
      //Deprecated
      //tidl_writeAsStaticArray((uint8_t *)gIOParams, (int8_t *)"./tidl_tb_gIOParams", sizeof(sTIDL_IOBufDesc_t), "tidl_tb_gIOParams", "tidl_tb_gIOParams", 1, 0);
      tidl_writeMultiNetworkBin((uint8_t*)gIOParams, (int8_t *)"./tidl_tb_gIOParams", sizeof(sTIDL_IOBufDesc_t));
    }
    #else
    //Set statically
    int32_t localOffset = tidl_getStaticOffset(tidl_tb_gIOParams);
    (*gIOParams) =((sTIDL_IOBufDesc_t)*((sTIDL_IOBufDesc_t*)(tidl_tb_gIOParams + localOffset))); //Struct copy over
    #endif
    return 0;
}

void printIOTensorInfo(sTIDL_IOBufDesc_t * ioParams)
{
  int32_t idx;
  for (idx = 0; idx < ioParams->numInputBuf; idx++)
  {
    tidl_tb_printf(1, "Input : dataId=%d, name=%s, elementType %d, scale=%f, zero point=%d, layout=%d\n", ioParams->inDataId[idx],
                                                                  ioParams->inDataName[idx],
                                                                  ioParams->inElementType[idx],
                                                                  ioParams->inTensorScale[idx],
                                                                  ioParams->inZeroPoint[idx],
                                                                  ioParams->inLayout[idx]);
  }
  for (idx = 0; idx < ioParams->numOutputBuf; idx++)
  {
    tidl_tb_printf(1, "Ouput : dataId=%d, name=%s, elementType %d, scale=%f, zero point=%d, layout=%d \n ", ioParams->outDataId[idx],
                                                                  ioParams->outDataName[idx],
                                                                  ioParams->outElementType[idx],
                                                                  ioParams->outTensorScale[idx],
                                                                  ioParams->outZeroPoint[idx],
                                                                  ioParams->outLayout[idx]);
  }
}


void tidl_genSetStaticInput(sTIDL_IOBufDesc_t * ioPrms, IVISION_BufDesc   *BufDescList)
{
  int i = 0;
  int32_t elementSizeBytes;
  uint32_t inBufSize;
  int32_t tensorSize;
  int32_t tensorOffset;
  uint32_t totalSize = 0;
  for(i = 0; i < ioPrms->numInputBuf; i++)
  {
    elementSizeBytes  = tidltb_getDatElementSize(ioPrms->inElementType[i]);
    inBufSize = ioPrms->inChannelPitch[i] * ioPrms->inNumChannels[i] * ioPrms->inDIM2[i] * ioPrms->inDIM1[i] * ioPrms->inNumBatches[i] * elementSizeBytes;
    totalSize += inBufSize;
  }
#if (!BUILD_WITH_STATIC_ARRAYS)
  /* Generate for Non-Static Builds */
  if (gWriteStaticBins)
  {
    tidl_tb_printf(0, "Writing Static Headers for Input Tensors\n");
    uint8_t * inPtr = NULL;
    inPtr = (uint8_t *)malloc(totalSize);
    TIDLTB_ASSERT_EXIT(inPtr != NULL);
    tensorOffset = 0;
    for (i = 0; i < ioPrms->numInputBuf; i++)
    {
      void * ivPtr = BufDescList[i].bufPlanes[0].buf;//Ivision Pointer is set at this point
      elementSizeBytes  = tidltb_getDatElementSize(ioPrms->inElementType[i]);
      tensorSize = ioPrms->inChannelPitch[i] * ioPrms->inNumChannels[i] * ioPrms->inDIM2[i] * ioPrms->inDIM1[i] * ioPrms->inNumBatches[i] * elementSizeBytes;
      memcpy(inPtr + tensorOffset, ivPtr, tensorSize);
      tensorOffset += tensorSize;
    }
    tidl_writeMultiNetworkBin((uint8_t*)inPtr, (int8_t *)"./tidl_tb_pIn_tensor", tensorOffset);
    if ( inPtr )
    {
      free(inPtr);
    }
  }
  else
  {
    tidl_tb_printf(3,"Skipping static gen-set function\n");
  }
#else
  //Set for Static Builds
  tidl_tb_printf(0, "Setting Static Headers for Input Tensors : %d\n",ioPrms->numInputBuf);
  tensorOffset = 0;
  for (i = 0; i < ioPrms->numInputBuf; i++)
  {
    int32_t localOffset = tidl_getStaticOffset(tidl_tb_pIn_tensor); //To jump to a particular n/w's data.
    elementSizeBytes  = tidltb_getDatElementSize(ioPrms->inElementType[i]);
    tensorSize = ioPrms->inChannelPitch[i] * ioPrms->inNumChannels[i] * ioPrms->inDIM2[i] * ioPrms->inDIM1[i] * ioPrms->inNumBatches[i] * elementSizeBytes;
    BufDescList[i].bufPlanes[0].buf = (void *)(tidl_tb_pIn_tensor + tensorOffset + localOffset);
    tensorOffset += tensorSize;

  }
#endif

}

int32_t gTempTraceCount = 0;
int32_t tidl_tb_utils_printf(const char * format, va_list args)
{
  int status = 0;
  gTempTraceCount++;

#if defined(_A72_BUILD)
  status = vprintf(format, args);
#elif defined(TIDL_WRITE_PRINTS_TO_BUF)
  status = tidl_buffer_printf(format, args);
#else
  #ifdef USE_HOST_PRINTF
    status = appLogvPrintf(format, args);
  #else
    status = vprintf(format, args);
  #endif
#endif
  return status;
}

int32_t tidl_tb_printf(int8_t traceLevel, const char *format, ...)
{
  int32_t status = 0;

  if(traceLevel <= gParams.debugTraceLevel)
  {
    va_list args;
    va_start(args, format);
    status = tidl_tb_utils_printf(format, args);
    va_end(args);
    fflush(stdout);
  }
  return status;
}

/*Print progress of current state [0,1]
  Note: This function needs to be called in the same iterator and no other print must be present*/
#define TIDL_PROGRESS_BAR_WIDTH (77U)
void tidl_tb_progressBar(float progress)
{
  int32_t i;
  tidl_tb_printf(0,"[");
  int32_t pos = progress * TIDL_PROGRESS_BAR_WIDTH;
  for(i = 0; i < TIDL_PROGRESS_BAR_WIDTH; i++)
  {
    if (i < pos)
    {
      tidl_tb_printf(0,"=");
    }
    else if(i == pos)
    {
      tidl_tb_printf(0,">");
    }
    else
    {
      tidl_tb_printf(0," ");
    }
  }
  tidl_tb_printf(0,"]");
  tidl_tb_printf(0," %d %\r", (int)(progress * 100.0));
}

#define TIDL_TB_MAX_CORENUM (TIDL_MAX_NUM_CORES) /* IMP: Should be in synch and same as TIDL_MAX_CORENUM*/

#if (!HOST_EMULATION) && !defined(_A72_BUILD) && (TIDL_TB_MAX_CORENUM > 1)
#include <c7x.h>
  uint32_t TIDL_TEST_getCoreNum(void)
  {
    uint32_t corePacNum = 0;
    #if 0
    uint64_t dnum;
    /* Get the bits from bit 7 to bit 15, which represents the core pac number */
    dnum = __DNUM;
    /* Enums are common across devices, subtract enum for C7x_1 since TIDL array indexing starts from C7x_1, CSL has support for other cores so C7x_1 may not be enum 0 */
    corePacNum = CSL_REG64_FEXT(&dnum, C7X_CPU_DNUM_COREPACNUM) - CSL_C7X_CPU_COREPACK_NUM_C7X1;
    #endif
    return corePacNum ;
  }
#else
  uint32_t TIDL_TEST_getCoreNum(void){
    return 0;
  }
#endif

#define MCU_CTRL_MMR_ADDR (0x4500000UL)
#define DV_REG0_SET (0x00010200U)
#define DV_REG1_SET (0x00010204U)
#define DV_REG0_CLR (0x00010300U)
#define DV_REG1_CLR (0x00010304U)
#define TIDL_SDK_UC_MULTICORE_SYNC_VAL (0x40)



#ifdef TIDL_ENABLE_PER_FRAME_PERFORMANCE
#pragma DATA_SECTION (gTIDLFrameInfo,  ".tidlFrameInfo");
TIDL_frameInfo gTIDLFrameInfo;
#endif

volatile uint32_t * gDebugCtrlEnableComparison = (volatile uint32_t *)(TIDL_DEBUG_CTRL_BASE + TIDL_DEBUG_CTRL_ENABLE_COMPARISON_OFFSET);

void tidl_wait_for_uc_start()
{
#ifdef TIDL_PRESILICON_SDK_UC_ENABLE
  //AVV IPC
  //Set ready
  *(volatile uint32_t *)(MCU_CTRL_MMR_ADDR + DV_REG0_SET) = TIDL_SDK_UC_MULTICORE_SYNC_VAL;
  uint32_t val = 0;
  do{
    val = *(volatile uint32_t *)(MCU_CTRL_MMR_ADDR + DV_REG1_SET);
    val &= TIDL_SDK_UC_MULTICORE_SYNC_VAL;
  }while(val == 0); //Wait for run
  //Clear ready
  *(volatile uint32_t *)(MCU_CTRL_MMR_ADDR + DV_REG0_CLR) = TIDL_SDK_UC_MULTICORE_SYNC_VAL;
#endif
}

void tidl_wait_for_uc_end()
{
#ifdef TIDL_PRESILICON_SDK_UC_ENABLE
  //AVV IPC
  //Clear running bit
  *(volatile uint32_t *)(MCU_CTRL_MMR_ADDR+DV_REG1_CLR) = TIDL_SDK_UC_MULTICORE_SYNC_VAL;
  //__asm volatile("   dsb          ;");
  //__asm volatile("   isb          ;");
  //End AVV IPC
#endif
}

int32_t tidl_is_comparison_required()
{
  int32_t isComparisonRequired = 1;

#ifdef TIDL_PRESILICON_SDK_UC_ENABLE
      isComparisonRequired = 0;
      if ((*gDebugCtrlEnableComparison) == 0x12345678)
      {
        isComparisonRequired = 1;
      }
 #endif
 return isComparisonRequired;
}

void tidl_write_frameHeader(int32_t frameIdx, int32_t codePosition)
{
#ifdef TIDL_ENABLE_PER_FRAME_PERFORMANCE
  TIDL_frameInfo*  fData = (TIDL_frameInfo *)&gTIDLFrameInfo;
  fData->frameIdx     = frameIdx;
  fData->codePosition = codePosition;
#endif
}

void tidl_write_frameInfo(int32_t frameIdx, uint64_t totalCycles, uint64_t ddrRead, uint64_t ddrWrite,  int32_t isMatching)
{
#ifdef TIDL_ENABLE_PER_FRAME_PERFORMANCE
  TIDL_frameInfo*  fData = (TIDL_frameInfo *)&gTIDLFrameInfo;

  if(isMatching)
  {
    fData->frameInfo[frameIdx].comparisonStatus[0] = 0x12;
    fData->frameInfo[frameIdx].comparisonStatus[1] = 0xA5;
    fData->frameInfo[frameIdx].comparisonStatus[2] = 0x50;
  }
  else
  {
    fData->frameInfo[frameIdx].comparisonStatus[0] = 0xFA;
    fData->frameInfo[frameIdx].comparisonStatus[1] = 0x11;
    fData->frameInfo[frameIdx].comparisonStatus[2] = 0x00;
  }

  fData->frameInfo[frameIdx].frameCycles = totalCycles;
  fData->frameInfo[frameIdx].readBW_DDR = ddrRead;
  fData->frameInfo[frameIdx].writeBW_DDR = ddrWrite;
  fData->frameInfo[frameIdx].totalBW_DDR = ddrRead + ddrWrite;
#endif
}

