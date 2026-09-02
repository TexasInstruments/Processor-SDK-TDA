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


/** @file tidl_tb_utils.h
*
*  @brief  This file contains utility functions for TI DL test application
*
*
*  @date   Aug 2019
*
*/

#ifndef _TIDL_TB_UTISL_H_
#define _TIDL_TB_UTISL_H_

#include <stdint.h>

#define NUM_ERR_TYPES (128U)
#define TIDL_DEBUG_CTRL_BASE (0x9CFFF000)
#define TIDL_DEBUG_CTRL_ENABLE_COMPARISON_OFFSET (0)
#define TIDL_DEBUG_CTRL_SET_ITERATION_OFFSET (sizeof(uint32_t))


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <math.h>
#include <float.h>
#if (HOST_EMULATION)
#include <malloc.h>
#endif
#include "itidl_ti.h"
#include "itidl_rt.h"
#include "tidl_config.h"
#include "ti_mem_manager.h"
#include "platform_common.h"

#define TIDLTB_ASSERT_MSG(x)   {\
    if(!(x)) \
    {\
      tidl_tb_printf(0, "Error at line: %5d : in file %22s, of function : %s \n", __LINE__, __FILE__,__FUNCTION__);\
    }\
  }

#define TIDLTB_ASSERT_EXIT(x)  {\
        if(!(x)) \
        {\
          tidl_tb_printf(0, "Error at line: %5d : in file %22s, of function : %s \n", __LINE__, __FILE__,__FUNCTION__);\
          exit(-1);\
        }\
      }


typedef struct
{
sTIDL_IOBufDesc_t * ioParams;
tidl_net_config   * configParams;
void              * algHandle;
void              * net;
void              * ioDesc;
TI_FILE           * inFile;
}sTIDL_TBInstHanle;
extern TIDL_CreateParams  createParams;
extern sTIDL_IOBufDesc_t  gIOParams;
extern  uint32_t gFinalStatus;

#if HOST_EMULATION
typedef struct
#else
typedef struct __attribute__((__packed__)) TIDL_perFrameInfo
#endif
{
  uint8_t   comparisonStatus[3];
  uint64_t  frameCycles;
  uint64_t  readBW_DDR;
  uint64_t  writeBW_DDR;
  uint64_t  totalBW_DDR;
} TIDL_perFrameInfo;

#if HOST_EMULATION
typedef struct
#else
typedef struct __attribute__((__packed__)) TIDL_frameInfo
#endif
{
  int32_t       frameIdx;
  uint8_t       codePosition;
  TIDL_perFrameInfo  frameInfo[1024];
} TIDL_frameInfo;

#define TIDL_TB_MAX_INS_HANDLES (16)
extern sTIDL_TBInstHanle tidl_tb_handle[];


int32_t tidl_sciclientDmscGetVersion(char *version_str, uint32_t version_str_size);
int32_t getNumberOfLinesIntheFile(char * fileName);
void * tidl_tb_udma_init( void);
char * moveToNextElement(char *in, char *LinePtr);
int32_t getConfigFileNamesList(char * fileName, int32_t *multiInstance, int8_t** configNames);
char * getNetworkStringName(char * fileName);

int32_t tidltb_getDatElementSize(int32_t elementType);
int32_t readImage(uint8_t *imageFile, uint8_t *ptr, int32_t offset, int16_t n, int16_t width, int16_t height, int32_t inBufIdx, int32_t inResizeMode);
int32_t tidl_tb_postProc(int32_t width, int32_t height, int32_t n, int32_t frameCount, void *fPtr, int32_t elementType);
int32_t tidl_NetOutputPostproc(tidl_net_config *params, sTIDL_IOBufDesc_t * ioPrms, sTIDLRT_Tensor_t * out[], int32_t frameCount);

int32_t getFileNameAndIdxFromList(char* listFile, int32_t frameCount, int32_t fileIdx, char* currFile);

extern int32_t firstOutWrite;
extern int32_t totalTestCount;
extern int32_t passTestCount;
extern int32_t top5passTestCount;
extern int64_t confusion_matrix[MAX_SEG_CLASS_IDX][MAX_SEG_CLASS_IDX];

void tidl_log_init(void);
void appLogvPrintf(const char *format, va_list va_args_ptr);
int tidl_tb_pritnf(const char *format, ...);
int32_t tidl_tb_utils_printf(const char * format, va_list args);
void tidl_buffer_printf_dump();
void tidl_updateNetPicth(sTIDL_Network_t * tidlNet);
#if BUILD_WITH_STATIC_ARRAYS
extern uint8_t  tidl_tb_input_tensor[];
extern uint8_t  tidl_tb_net[];
extern uint8_t  tidl_tb_output_tensor[];
extern uint8_t  tidl_tb_pIn_tensor[]; //Post Processed input tensor 
extern uint8_t  tidl_tb_pOutx_tensor[];
extern uint8_t  tidl_tb_gIOParams[];//IO DESC
extern uint8_t  tidl_tb_tidl_net_config[];
#endif
extern TI_FILE *refCompareFile;

int32_t tidltb_printNetInfo(sTIDL_Network_t * pTIDLNetStructure,
                            int32_t layersGroupId);
TI_FILE * tidl_openNetInput(tidl_net_config *params);
int32_t tidl_readFile(void * file, uint8_t * fileString);
int32_t tidl_writeNetWithStats(tidl_net_config * params, sTIDL_Network_t * net);
int32_t tidl_tb_WriteBinToFile(const char * fileName, void * addr, int32_t size, int32_t count);
int32_t tidl_tb_ReadBinFromFile(const char * fileName, void * addr, int32_t size, int32_t count);

int32_t readIODescFile(char * fileName, sTIDL_IOBufDesc_t * gIOParams);
void printIOTensorInfo(sTIDL_IOBufDesc_t * ioParams);
int32_t tidl_writeAsStaticArray(uint8_t * ptr, int8_t * FileName, int32_t size, const char arrayName[], const char secName[], uint32_t isDataConst, uint32_t dumpAsBin);
int32_t tidl_writeMultiNetworkBin(uint8_t *data_ptr, int8_t * FileName, int32_t data_size);
int32_t tidl_getStaticOffset(void* ptr);
int32_t tidl_getTotalStaticNetworks(void* ptr);
void setDefaultParams(tidl_net_config * params);
void updateDefaultParams(tidl_net_config * params);
int32_t tidl_getFileSize(uint8_t * fileString);
int32_t tidl_allocInOutTensors(sTIDL_IOBufDesc_t *ioPrms, sTIDLRT_Tensor_t *ins[], sTIDLRT_Tensor_t *outs[]);
int32_t tidl_applyDimsFromFile(TI_FILE *dimsFp, sTIDL_IOBufDesc_t *ioPrms, sTIDLRT_Tensor_t *ins[]);
int32_t tidl_freeInOutTensors(sTIDL_IOBufDesc_t *ioPrms, sTIDLRT_Tensor_t * ins[], sTIDLRT_Tensor_t * outs[]);
int32_t tidl_CompareNetOutputMem(tidl_net_config *params, sTIDL_IOBufDesc_t * ioPrms, sTIDLRT_Tensor_t * out[]);
int32_t tidl_WriteNetOutputMem (tidl_net_config *params, sTIDL_IOBufDesc_t * ioPrms, sTIDLRT_Tensor_t * out[], int32_t frameCount);
int32_t tidl_ReadNetInput(TI_FILE * fp1, tidl_net_config *params, sTIDL_IOBufDesc_t * ioPrms,
                              sTIDLRT_Tensor_t   *ins[], int32_t frameCount);

void tidl_wait_for_uc_start();
void tidl_wait_for_uc_end();
void tidl_write_frameHeader(int32_t frameIdx, int32_t codePosition);
void tidl_write_frameInfo(int32_t frameIdx, uint64_t totalCycles, uint64_t ddrRead, uint64_t ddrWrite,  int32_t isMatching);
int32_t tidl_is_comparison_required();

void tidl_genSetStaticInput(sTIDL_IOBufDesc_t * ioPrms, IVISION_BufDesc   *BufDescList);

typedef struct 
{
  char *tokenName;
  int32_t errCode;
} sTIDL_errCodeMapping;

extern sTIDL_errCodeMapping gTIDL_errorCodeMap[NUM_ERR_TYPES];
void tidl_printStatus(int32_t status);
uint32_t TIDL_TEST_getCoreNum(void);

#endif /* _TIDL_TB_UTISL_H_ */

