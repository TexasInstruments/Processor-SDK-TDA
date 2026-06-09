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



#ifndef _TIDL_CONFIG_H_
#define _TIDL_CONFIG_H_

#include "configparser.h"

#define FILE_NAME_SIZE  (512)
#define TIDL_TB_CURR_LAYERS_GROUP_ID (1)
typedef struct
{
    uint32_t noZeroCoeffsPercentage;
    uint32_t updateNetWithStats;
    uint32_t inFileFormat;
    uint32_t writeInput;
    uint32_t writeOutput;
    uint32_t compareRef;
    uint32_t numFrames;
    uint32_t startFrameIdx;
    uint32_t currFrameIdx;
    uint32_t debugTraceLevel;
    uint32_t writeTraceLevel;
    uint32_t logStats;
    uint32_t dumpNetInitBackupData;
    uint32_t initOnly;
    uint32_t enableLayerPerfTraces;
    uint32_t preProcType;
    uint32_t postProcType;
    uint32_t postProcDataId;
    uint32_t totNumClasses;
    uint32_t writeBinsAsHeader;
    uint32_t flowCtrl;
    uint32_t reservedCtrl;
    uint32_t inResizeMode;
    uint32_t numItrPerf;
    uint32_t targetPriority;
    uint32_t coreNum;
    uint32_t enableCodeCoverage;
    uint32_t releaseIOTensorsAtCreate;
    uint32_t performanceTestcase;
    uint32_t inFileFormatRandomType;
    uint32_t computeChecksum;

    int32_t  coreStartIdx;
    int32_t  forceNegativeTest;

    float quantRangeExpansionFactor;
    float quantRangeUpdateFactor;
    float maxPreEmptDelay;

    uint8_t netBinFile[FILE_NAME_SIZE];
    uint8_t paramsBinFile[FILE_NAME_SIZE];
    uint8_t inData[FILE_NAME_SIZE];
    uint8_t outData[FILE_NAME_SIZE];
    uint8_t testCaseName[FILE_NAME_SIZE];
    uint8_t traceDumpBaseName[FILE_NAME_SIZE];
    uint8_t testCaseDesc[FILE_NAME_SIZE];
    uint8_t configLine[FILE_NAME_SIZE];
    uint8_t configParam[FILE_NAME_SIZE];
    uint8_t meanImage[FILE_NAME_SIZE];
    uint8_t ioConfigFile[FILE_NAME_SIZE];
    uint8_t tempBufferDir[FILE_NAME_SIZE];

} tidl_net_config;

extern tidl_net_config gParams;
extern sTokenMapping gsTokenMap_tidl_net[MAX_ITEMS_TO_PARSE];
#define MAX_SEG_CLASS_IDX (64)
int32_t tidl_tb_printf(int8_t traceLevel, const char *format, ...);
void tidl_tb_progressBar(float progress);
/* DMA (or block processing) + intrinsic code --> traget flow*/
#define TIDL_FLOW_CTRL_DEFAULT  (0x00000000)
/* No DMA + ref code*/
#define TIDL_FLOW_CTRL_REF_ONLY (0x00000001)
/* No DMA + ref code + stat collection */
#define TIDL_FLOW_CTRL_REF_STAT (0x00000002)
/* DMA + MMA-Natc */
#define TIDL_FLOW_CTRL_MMA_NATC (0x00000004)
/* DMA + DSP-Natc */
#define TIDL_FLOW_CTRL_DSP_NATC (0x00000008)
/* AVX Optimised kernels for refernce flow */
#define TIDL_FLOW_CTRL_AVX_REF (0x00000020)


#ifndef PREFIXED_OUTFILES
#define PLAT_SUFFIX ""
#else
#ifdef HOST_EMULATION
#define PLAT_SUFFIX "host."
#else
#if defined(LOKI_BUILD)
#define PLAT_SUFFIX "loki."
#else
#define PLAT_SUFFIX "target."
#endif
#endif
#endif

#endif /* _TIDL_CONFIG_H_*/
