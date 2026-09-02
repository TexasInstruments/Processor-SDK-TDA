/*
* module name : Structure From Motion
*
* module descripton : Generates sparse 3D points from optical flow information in camera captured images
*
*/
/*

Copyright (c) {YEAR} Texas Instruments Incorporated

All rights reserved not granted herein.

Limited License.

Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive license under copyrights and patents it now or hereafter owns or controls to make, have made, use, import, offer to sell and sell ("Utilize") this software subject to the terms herein.  With respect to the foregoing patent license, such license is granted  solely to the extent that any such patent is necessary to Utilize the software alone.  The patent license shall not apply to any combinations which include this software, other than combinations with devices manufactured by or for TI ("TI Devices").  No hardware patent is licensed hereunder.

Redistributions must preserve existing copyright notices and reproduce this license (including the above copyright notice and the disclaimer and (if applicable) source code license limitations below) in the documentation and/or other materials provided with the distribution

Redistribution and use in binary form, without modification, are permitted provided that the following conditions are met:

*       No reverse engineering, decompilation, or disassembly of this software is permitted with respect to any software provided in binary form.

*       any redistribution and use are licensed by TI for use only with TI Devices.

*       Nothing shall obligate TI to provide you with source code for the software licensed and provided to you in object code.

If software source code is provided to you, modification and redistribution of the source code are permitted provided that the following conditions are met:

*       any redistribution and use of the source code, including any resulting derivative works, are licensed by TI for use only with TI Devices.

*       any redistribution and use of any object code compiled from the source code and any resulting derivative works, are licensed by TI for use only with TI Devices.

Neither the name of Texas Instruments Incorporated nor the names of its suppliers may be used to endorse or promote products derived from this software without specific prior written permission.

DISCLAIMER.

THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#ifndef SFM_TB_H
#define SFM_TB_H

#pragma CHECK_MISRA ("none")
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "math.h"

#include <../common/profile.h>
#include <../common/tiadalg_img_proc.h>
#include "ti/drv/udma/udma.h"

#define ENABLE_TRACES 1

//#define ENABLE_FILE_IO

#ifndef __mmax
#define __mmax(x,y) (x>y)?x:y
#endif

#ifndef __mmin
#define __mmin(x,y) (x>y)?y:x
#endif


#define MAX_NUM_IN_POINTS            (1242*375) // need to change if resolution changes

#define VLIB_TRIANG_MAX_POINTS_IN_TRACK               (6)

#define FRAME_DEPTH_3D_POINT_CLOUD (1)
/*
* Maximum number of output points
*/
#define MAX_NUM_OUT_POINTS           (MAX_NUM_IN_POINTS * FRAME_DEPTH_3D_POINT_CLOUD)

#define PRINT_ERROR_MSG()  printf("Error at line: %5d : in file %22s, of function : %s \n",__LINE__, __FILE__,__FUNCTION__)

#ifdef TMS320C66X
/* Out of total 288 KB of SRAM, 128 KB is configured as cache,
6 KB as stack, hence remaining 288 - 128 - 6 = 154 is
configured as SRAM to be used by SFM algorithm
*/
#define DMEM1_SIZE (154 * 1024)

/* 16KB of L1D is configured as SRAM
*/
#define DMEM0_SIZE (16 * 1024)
#else

#define DMEM2_SIZE (1024 * 1024) // 1MB Of MSMC scratch
//#define DMEM1_SIZE (480 * 1024) // 480KB sram and 64 kb cache. total size of 512 + 32.
#define DMEM1_SIZE (448 * 1024) // 480KB sram and 64 kb cache. total size of 512 + 32.
#define DMEM0_SIZE (13 * 1024) // 13 KB sram, 3 kb udmaDrv, 32KB cache

#endif

#pragma DATA_SECTION(DMEM2_SCRATCH,".l3ScratchBuf")
/* This will be used for allocation of memory in DMEM1 --> L2*/
uint8_t DMEM2_SCRATCH[DMEM2_SIZE];

#pragma DATA_SECTION(DMEM1_SCRATCH,".l2ScratchBuf")
/* This will be used for allocation of memory in DMEM1 --> L2*/
uint8_t DMEM1_SCRATCH[DMEM1_SIZE];

#pragma DATA_SECTION (DMEM0_SCRATCH,".l1ScratchBuf");
/* This will be used for allocation of memory in DMEM0 --> L1D*/
uint8_t DMEM0_SCRATCH[DMEM0_SIZE];

/*Global variable for udma driver*/
#pragma DATA_SECTION (udmaDrvObj,".l1ScratchBuf");
struct Udma_DrvObj  udmaDrvObj;

TIMemObject memObj_DMEM2;
TIMemObject memObj_DMEM1;
TIMemObject memObj_DMEM0;

#define MAX_INPUT_BUF_SIZE (MAX_NUM_IN_POINTS*(sizeof(int32_t) + 3) + 16) // dof element of 4 bytes and input RGB 3 bytes
#define MAX_OUTPUT_BUF_SIZE (MAX_NUM_IN_POINTS*sizeof(SFM_TI_output) + MAX_NUM_IN_POINTS*3 + 16) // 3 for RGB
#define MAX_IMG_BUF_SIZE ((1920*1088*3)>>1)

#pragma DATA_SECTION (IN_BUF, ".ioBufs");
uint8_t IN_BUF[MAX_INPUT_BUF_SIZE];

#pragma DATA_SECTION (IMG_BUF, ".ioBufs");
uint8_t IMG_BUF[MAX_IMG_BUF_SIZE];

#pragma DATA_SECTION (OUT_BUF, ".ioBufs");
uint8_t OUT_BUF[MAX_OUTPUT_BUF_SIZE];

//#define ENABLE_YUV_WRITE

#ifdef ENABLE_FILE_IO
#define ENABLE_PLY_TXT_WRITE
//#define ENABLE_PLY_BIN_WRITE


FILE* fp_in_feat;

FILE * fp_in_cam;
FILE * fp_out;
FILE* fp_in1;
FILE* fp_out1;
float* dist;
uint8_t* imgPtr;

#endif //ENABLE_FILE_IO

//#define TIADALG_UC_AVV_BUILD

#ifdef TIADALG_UC_AVV_BUILD

//UC VARIABLES:
#define MULTICORE_SYNC_BASE      (0x41C28000)
#define SMALL_WAIT_TIME          (0x64)
#define MULTICORE_SYNC_VAL       (0x12345678)
#define TIADALG_DBUG_CTRL_BASE   (0xD9900000)
#define C7_1_RD_OFFSET           (0xB)
#define C7_1_WR_OFFSET           (0x3)
#define MULTICORE_CSI_COUNTER    (0x41C28050)
#define MULTICORE_QoS_Control    (0x41C2805D)
#endif

typedef struct IM_Fxns
{
  IVISION_Fxns * ivision;

} IM_Fxns;


int32_t sfmTest(ssfm_Config * testcaseParams);
int32_t TestApp_AllocMemRecords(IALG_MemRec * memRec,int32_t numMemRec);
int32_t TestApp_FreeMemRecords(IALG_MemRec * memRec,int32_t numMemRec);
int32_t TestApp_AllocIOBuffers(IVISION_InBufs * inBufs,
                               IVISION_OutBufs * outBufs,
                               int32_t totInPixs,
                               uint16_t inNumTracks,
                               int32_t inImgClrFmt);

int32_t TestApp_ReadInBuf(
  void * dst0,
  void * dst1,
  int32_t currFrameNum,
  int32_t enAlgPtCldVis,
  char * dofFolderName,
  char* inImgName,
  int32_t img_width,
  int32_t img_height,
  int32_t inImgClrFmt);

int32_t TestApp_ReadInCameraPrm(float * dstExt,
                                float * dstInt,
                                int32_t currFrameNum,
                                int32_t startFrameNum,
                                char * fileName,
                                int32_t maxFrameNum);

#if defined(ENABLE_FILE_IO) || defined (ENABLE_YUV_WRITE)
int32_t TestApp_WriteOutImgBuf(SFM_TI_output * src,int32_t currFrameNum,
                            int32_t startFrameNum,
                            char * featFileName,char* inImgFileName,
                            char* outImgFileName,int32_t maxFrameNum,
                            int32_t imgWidth, int32_t imgHeight,
                            int32_t outNumPoints);
#endif

int32_t TestApp_WriteOutPlyBuf(SFM_TI_output * src,int32_t currFrameNum,
                            int32_t startFrameNum,
                            char * featFileName,char* inImgFileName,
                            char* outImgFileName,int32_t maxFrameNum,
                            int32_t imgWidth, int32_t imgHeight,
                            int32_t outNumPoints);

void TestApp_WritePlyHeader(FILE* fp_out, int32_t outNumPoints);
int32_t sfmCompare(ssfm_Config * testcaseParams);
int32_t TestApp_memContamination(IALG_MemRec * memRec,int32_t numMemRec, int32_t frmIdx);

#if (!HOST_EMULATION)
void SetMAR();
void InitCache();
#define L1D_SRAM_ADDR (0x00F00000)

#define MAR148() (*(volatile uint32_t *)0x01848250)
#endif

void * algo_tb_udma_init( void);

void ALGO_printf(const char *str);

#endif
