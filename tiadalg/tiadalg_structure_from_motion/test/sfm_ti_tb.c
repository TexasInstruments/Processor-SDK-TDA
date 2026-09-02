/*
* module name : Structure From Motion
*
* module descripton : Generates sparse 3D points from optical flow information in camera captured images
*
*/
/*

Copyright (c) 2009-2017 Texas Instruments Incorporated

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
/** @file SFM_tb.c
*
*  @brief  This is a test bench file for Structure From Motion (SFM) module.
*          This test interface is common for PD and TSR.
*
*  @date   October 2014
*
*/

#pragma CHECK_MISRA ("none")

#include "../common/tiadalg_test_interface.h"

#include "sfm_ti_config.h"
#include "tiadalg_interface.h"
#include "sfm_ti_tb.h"

#ifndef HOST_EMULATION
#ifdef TIADALG_OS_BUILD
//#include <ti/sysbios/family/c7x/Cache.h>
#endif
#endif

#include "float.h"

#ifndef ENABLE_FILE_IO
#include <./../data/structure_from_motion/dof/dof_0.h>
#include <./../data/structure_from_motion/dof/dof_1.h>
#include <./../data/structure_from_motion/dof/dof_2.h>
#include <./../data/structure_from_motion/dof/dof_3.h>
#include <./../data/structure_from_motion/dof/dof_4.h>
#include <./../data/structure_from_motion/dof/dof_5.h>
#include <./../data/structure_from_motion/dof/dof_6.h>
#include <./../data/structure_from_motion/dof/dof_7.h>
#include <./../data/structure_from_motion/dof/dof_8.h>
#include <./../data/structure_from_motion/dof/dof_9.h>
#include <./../data/structure_from_motion/dof/dof_10.h>
#include <./../data/structure_from_motion/dof/dof_11.h>
#include <./../data/structure_from_motion/dof/dof_12.h>
#include <./../data/structure_from_motion/dof/dof_13.h>
#include <./../data/structure_from_motion/dof/dof_14.h>

#include <./../data/structure_from_motion/output_ref/out_5.h>
#include <./../data/structure_from_motion/output_ref/out_6.h>
#include <./../data/structure_from_motion/output_ref/out_7.h>
#include <./../data/structure_from_motion/output_ref/out_8.h>
#include <./../data/structure_from_motion/output_ref/out_9.h>
#include <./../data/structure_from_motion/output_ref/out_10.h>
#include <./../data/structure_from_motion/output_ref/out_11.h>
#include <./../data/structure_from_motion/output_ref/out_12.h>
#include <./../data/structure_from_motion/output_ref/out_13.h>
#include <./../data/structure_from_motion/output_ref/out_14.h>
#endif

// PC mode is nonos scenario always, whereas on platform it can be nonos flow or freertos flow
#if defined(HOST_EMULATION) || !defined(TIADALG_FREERTOS_BUILD)
int32_t main(int32_t argc, char *argv[])
#else
int32_t test_main(int32_t argc, char *argv[])
#endif
{

  int32_t status = 0;

  printf("Program is loaded and started executing \n");

#ifndef HOST_EMULATION
  /*---------------------------------------------------------------------------
  Initialization of cache. Currently 128KB of L2 , 16 KB
  of L1, and full L1P is configured as cache.
  ----------------------------------------------------------------------------*/
  InitCache();

  volatile uint64_t l2ccCtrl = 0x0021060D00000000;
  volatile uint64_t l2ccPriorityMask = 0xFFFFFFF8FFFFFFFF;
  uint32_t gQoS_C7x_1_Priority = 6;

  l2ccCtrl = l2ccCtrl & l2ccPriorityMask;


  l2ccCtrl = l2ccCtrl | (((uint64_t)(gQoS_C7x_1_Priority))<<32); //Right Shift by 32 bits

  __ECR385 = l2ccCtrl;
#endif

#ifndef HOST_EMULATION
  _TSC_enable();
#endif

  /*---------------------------------------------------------------------------
  Setting the default value of the current test case.
  ----------------------------------------------------------------------------*/
  gConfig_sfm[0].maxFrames         = 1;
  gConfig_sfm[0].maxRansacItr      = 90;
  gConfig_sfm[0].maxTriangItr      = 10;
  gConfig_sfm[0].maxNumTracks      = (uint16_t)MAX_NUM_IN_POINTS;
  gConfig_sfm[0].profileEn         = 0;
  gConfig_sfm[0].compareEn         = 0;
  gConfig_sfm[0].sfmEn             = 0;
  gConfig_sfm[0].fMatrixPrunEn     = 1;
  gConfig_sfm[0].fMatrixInTh       = 2;
  gConfig_sfm[0].pointPruneAngle   = 0.0;
  gConfig_sfm[0].fMatrixCalcMethod = SFM_TI_FMAT_EXT_PRM_NORM_DEFAULT;

  /*---------------------------------------------------------------------------
  Read the config file parameters set value
  ----------------------------------------------------------------------------*/
#if defined(ENABLE_FILE_IO) || defined (ENABLE_YUV_WRITE)
  char configFile[FILE_NAME_SIZE]={0};
  if (argc > 1)
  {
    strcpy(configFile, argv[1]) ;
  }
  else
  {
    strcpy(configFile,"./../tiadalg_structure_from_motion/test/sfm.cfg");
  }
  sprintf((char*)&gConfig_sfm[0].compareRsultPath[0], "../testvecs/output/");
  status = readparamfile(configFile, &gsTokenMap_sfm[0]) ;
#else
  gConfig_sfm[0].sfmEn            = 1;
  gConfig_sfm[0].imageWidth       = 1024;
  gConfig_sfm[0].imageHeight      = 512;
  gConfig_sfm[0].keyPointStep     = 8;
  gConfig_sfm[0].flowConfThr      = 103;
  gConfig_sfm[0].maxNumTracks     = 10000;
  gConfig_sfm[0].maxTriangItr     = 3;
  gConfig_sfm[0].profileEn        = 1;
  gConfig_sfm[0].fMatrixPrunEn    = 1;
  gConfig_sfm[0].fMatrixInTh      = 70;
  gConfig_sfm[0].pointPruneAngle  = 2.0;
  gConfig_sfm[0].flowInvalidPadX  = 0;
  gConfig_sfm[0].flowInvalidPadY  = 0;
  gConfig_sfm[0].enAlgPtCldVis         = 0;

  gConfig_sfm[0].startFrame       = 0;
  gConfig_sfm[0].maxFrames        = 15;
#endif

  if(status == -1)
  {
    printf("Parser Failed \n");
    return -1 ;
  }
#ifdef USE_HOST_FILE_IO
  FILEio_init();
#endif
  /*---------------------------------------------------------------------------
  Validate the applet
  ----------------------------------------------------------------------------*/
  if(gConfig_sfm[0].sfmEn){
    status = sfmTest(gConfig_sfm);
  }

  /*---------------------------------------------------------------------------
  Do the compression of .ply file with reference .ply file
  ----------------------------------------------------------------------------*/
  if(gConfig_sfm[0].compareEn){
    //status = sfmCompare(gConfig_sfm);
  }

  return status;
}

int32_t sfmTest(ssfm_Config * testcaseParams)
{
  int32_t         numMemRec;
  IALG_MemRec*    memRec;
  int32_t         status;
  uint32_t        testcase;
  long long       t0, t1;
  uint8_t*        pDmem2;
  uint8_t*        pDmem1;
  uint8_t*        pDmem0;
  TIMemHandle     memHdl_DMEM2,memHdl_DMEM1,memHdl_DMEM0;

  int32_t         frameIdx;
  int32_t         startFrame,endFrame;

#if (!HOST_EMULATION)
  _TSC_enable();
#endif
  /*-----------------------------------------------------------------
  Pointer to algorithm instance
  -----------------------------------------------------------------*/
  IM_Fxns *             handle;

  /*-----------------------------------------------------------------
  Algorithm parameters memory in stack
  -----------------------------------------------------------------*/
  SFM_TI_CreateParams   createParams;

  /*-----------------------------------------------------------------
  For test purpose it is allocated in stack. Application may get
  this descriptors from its buffer manager
  -----------------------------------------------------------------*/
  IVISION_BufDesc       inBufDesc[SFM_TI_IN_BUFDESC_TOTAL];
  IVISION_BufDesc*      inBufDescList[SFM_TI_IN_BUFDESC_TOTAL];

  /*-----------------------------------------------------------------
  For test purpose it is allocated in stack. Application may get
  this descriptors from its buffer manager
  -----------------------------------------------------------------*/
  IVISION_BufDesc       outBufDesc[SFM_TI_OUT_BUFDESC_TOTAL];
  IVISION_BufDesc*      outBufDescList[SFM_TI_OUT_BUFDESC_TOTAL];

  /*-----------------------------------------------------------------
  InArgs and InBufs declaration required in SFM process call
  ----------------------------------------------------------------*/

  SFM_TI_InArgs         inArgs;
  IVISION_InBufs        inBufs;

  /*-----------------------------------------------------------------
  InArgs and InBufs declaration required in SFM process call
  ----------------------------------------------------------------*/
  SFM_TI_OutArgs        outArgs;
  IVISION_OutBufs       outBufs;
#ifdef ENABLE_PROFILE
  TI_DSP_PrfInfo*       prfInfo;
#endif
  outArgs.iVisionOutArgs.size = sizeof(SFM_TI_OutArgs);
  inArgs.iVisionInArgs.size   = sizeof(SFM_TI_InArgs);

  /*--------------------------------------------------------------------------*/
  /*Create memory handle for L2 SRAM of DSP.                                  */
  /*--------------------------------------------------------------------------*/
  pDmem2 = DMEM2_SCRATCH;
  pDmem1 = DMEM1_SCRATCH;
  pDmem0 = DMEM0_SCRATCH;

  memHdl_DMEM2 = &memObj_DMEM2;
  memHdl_DMEM1 = &memObj_DMEM1;
  memHdl_DMEM0 = &memObj_DMEM0;

  tiadalg_create_memory_handle(memHdl_DMEM2, pDmem2, DMEM2_SIZE);
  tiadalg_create_memory_handle(memHdl_DMEM1, pDmem1, DMEM1_SIZE);
  tiadalg_create_memory_handle(memHdl_DMEM0, pDmem0, DMEM0_SIZE);

  /*--------------------------------------------------------------------------
  Currently Only test case running is supported in single run.
  -------------------------------------------------------------------------*/
  sfm_numTestCases  = (testcaseParams[0].sfmEn == 1) ? 1: 0;

  printf("\n**************************************************************");
  printf("\n********~~~~~~~~~TI-Structure From Motion~~~~~~~~~~~~~~******");
  printf("\n**************************************************************\n");

  for ( testcase = 0 ; testcase < sfm_numTestCases; testcase++ )
  {

    /*-----------------------------------------------------------------
    Set buffer descriptor pointers array base
    -----------------------------------------------------------------*/
    inBufs.bufDesc       = inBufDescList;
    outBufs.bufDesc      = outBufDescList;

    inBufs.numBufs       = SFM_TI_IN_BUFDESC_TOTAL;
    outBufs.numBufs      = SFM_TI_OUT_BUFDESC_TOTAL;

    inBufDescList[SFM_TI_IN_BUFDESC_IN_DOF_BUFFER]         = &inBufDesc[SFM_TI_IN_BUFDESC_IN_DOF_BUFFER];
    inBufDescList[SFM_TI_IN_BUFDESC_IN_LUMA_IMG_BUFFER]    = &inBufDesc[SFM_TI_IN_BUFDESC_IN_LUMA_IMG_BUFFER];
    outBufDescList[SFM_TI_OUT_BUFDESC_FEATURE_PLANES]      = &outBufDesc[SFM_TI_OUT_BUFDESC_FEATURE_PLANES];
    outBufDescList[SFM_TI_OUT_BUFDESC_LUMA_PTCLD_BUFFER]   = &outBufDesc[SFM_TI_OUT_BUFDESC_LUMA_PTCLD_BUFFER];
    outBufDescList[SFM_TI_OUT_BUFDESC_CHROMA_PTCLD_BUFFER] = &outBufDesc[SFM_TI_OUT_BUFDESC_CHROMA_PTCLD_BUFFER];
    outBufDescList[SFM_TI_OUT_BUFDESC_LUMA_OCPGD_BUFFER]   = &outBufDesc[SFM_TI_OUT_BUFDESC_LUMA_OCPGD_BUFFER];
    outBufDescList[SFM_TI_OUT_BUFDESC_CHROMA_OCPGD_BUFFER] = &outBufDesc[SFM_TI_OUT_BUFDESC_CHROMA_OCPGD_BUFFER];

    /*-----------------------------------------------------------------
    Set algorithm parameters
    -----------------------------------------------------------------*/
    createParams.maxNumTracks  = testcaseParams[testcase].maxNumTracks;
    createParams.imgWidth      = testcaseParams[testcase].imageWidth;
    createParams.imgHeight     = testcaseParams[testcase].imageHeight;
    createParams.keyPointStep  = testcaseParams[testcase].keyPointStep;
    createParams.enAlgPtCldVis = 0;
    createParams.enAlgOcpGrdVis= 0;
    createParams.ocpGrdSizeInPixels = 500;

    createParams.rsvd1         = testcaseParams[testcase].profileEn;

    createParams.udmaDrvObj    = algo_tb_udma_init();

    createParams.flowCtrl      = 0;// 0-->intrinsic flow, 1 --> ref flow

#ifdef TIADALG_UC_AVV_BUILD
      volatile uint32_t* tiadalg_DebugLevel = (uint32_t*)(TIADALG_DBUG_CTRL_BASE);

      if(tiadalg_DebugLevel[0] == 0x12345678)
      {
        createParams.flowCtrl    = tiadalg_DebugLevel[1];
      }
#endif

    createParams.enAlgPtCldVis      = testcaseParams[testcase].enAlgPtCldVis;
    createParams.imgColorFmt   = 0; // 0--> yuvnv12, 1 BGR

    /*-----------------------------------------------------------------
    Query algorithm memory requirement using standard IALG interface
    -----------------------------------------------------------------*/
    numMemRec = SFM_TI_VISION_FXNS.ialg.algNumAlloc();
    memRec    = (IALG_MemRec *)malloc(numMemRec*sizeof(IALG_MemRec));

    status = SFM_TI_VISION_FXNS.ialg.algAlloc(
      (IALG_Params *)(&createParams), NULL, memRec);

    if(status != IALG_EOK)
    {
      PRINT_ERROR_MSG();
      goto EXIT_LOOP;
    }
#if ENABLE_TRACES
    printf("Structure from motion Applet Create Done\n");
#endif
    /*-----------------------------------------------------------------
    Allocate memory for all the mem records
    Application has to implement the below function
    Refer the example implementation below in this file
    -----------------------------------------------------------------*/
    status = TestApp_AllocMemRecords(memRec,numMemRec);

    TestApp_memContamination(memRec, numMemRec, 1);

    if(status != IALG_EOK)
    {
      PRINT_ERROR_MSG();
      goto EXIT_LOOP;
    }

    startFrame = testcaseParams[testcase].startFrame;
    endFrame   = startFrame + testcaseParams[testcase].maxFrames;

#ifdef ENABLE_FILE_IO
    TestApp_ReadInCameraPrm((float*)&inArgs.camExtPrm,
      (float*)&createParams.camIntPrm,
      startFrame,startFrame,
      (char *)testcaseParams[testcase].inCamPrmFileName,
      startFrame);
#else
    createParams.camIntPrm[0] = 1024.0;
    createParams.camIntPrm[1] = 0.0;
    createParams.camIntPrm[2] = 1024.0;
    createParams.camIntPrm[3] = 0.0;
    createParams.camIntPrm[4] = 1024.0;
    createParams.camIntPrm[5] = 512.0;
    createParams.camIntPrm[6] = 0.0;
    createParams.camIntPrm[7] = 0.0;
    createParams.camIntPrm[8] = 1.0;
#endif

    createParams.camIntPrm[0] *= 0.5;
    createParams.camIntPrm[2] *= 0.5;
    createParams.camIntPrm[4] *= 0.5;
    createParams.camIntPrm[5] *= 0.5;

    /*-----------------------------------------------------------------
    Initialize the algorithm instance with the allocated memory
    and user create parameters
    -----------------------------------------------------------------*/
    status = SFM_TI_VISION_FXNS.ialg.algInit((IALG_Handle)(&handle),
      memRec,NULL,(IALG_Params *)(&createParams));


    handle = (IM_Fxns *) memRec[0].base;

    if(status != IALG_EOK)
    {
      PRINT_ERROR_MSG();
      goto EXIT_LOOP;
    }

#if ENABLE_TRACES
    printf("SFM Application Init Done\n");
#endif

    inArgs.iVisionInArgs.subFrameInfo       = 0;
    inArgs.iVisionInArgs.size               = sizeof(SFM_TI_InArgs);

    TestApp_AllocIOBuffers(&inBufs,
                           &outBufs,
                           testcaseParams[testcase].imageWidth*testcaseParams[testcase].imageHeight,
                           testcaseParams[testcase].maxNumTracks,
                           createParams.imgColorFmt);

    for(frameIdx = startFrame; frameIdx < endFrame; frameIdx++)
    {
      // if contamination is happening then alg deactivate has to be called
      if(frameIdx != startFrame)
      {
        // do only scratch contamination
        /*DDR contnet is alway up to date by the algorithm.
        deactivate sets the algState as not active so that activate should happen by algorithm inside the process call
        if it was not done by test app.
        */
        handle->ivision->ialg.algDeactivate((IALG_Handle)handle);

        TestApp_memContamination(memRec,numMemRec,0x0);
      }

      /*-----------------------------------------------------------------------
      Read the Intrinsic and extrinsic Parameter of camera corresponding to
      current frame
      -----------------------------------------------------------------------*/
#ifdef ENABLE_FILE_IO
      TestApp_ReadInCameraPrm((float*)&inArgs.camExtPrm,
        (float*)&createParams.camIntPrm,
        frameIdx,startFrame,
        (char *)testcaseParams[testcase].inCamPrmFileName,
        endFrame);
#else
#include <cam_prm.h>
      memcpy(&inArgs.camExtPrm, &cam_ext_prm[(frameIdx-startFrame)*16], 16*sizeof(float));
#endif
#ifdef ENABLE_FILE_IO
      TestApp_ReadInBuf((void*)inBufs.bufDesc[SFM_TI_IN_BUFDESC_IN_DOF_BUFFER]->bufPlanes[0].buf,
                        (void*)inBufs.bufDesc[SFM_TI_IN_BUFDESC_IN_LUMA_IMG_BUFFER]->bufPlanes[0].buf,
                        frameIdx,
                        testcaseParams[testcase].enAlgPtCldVis,
                        (char*)testcaseParams[testcase].inDofFileName,
                        (char*)testcaseParams[testcase].inImgFileName,
                        testcaseParams[testcase].imageWidth,
                        testcaseParams[testcase].imageHeight,
                        createParams.imgColorFmt);
#else
      /*luma buffer need not be copy , as it is just pass through*/
      const uint32_t* dofTableList[15] = {dofTable_0, dofTable_1, dofTable_2,
                             dofTable_3, dofTable_4, dofTable_5,
                             dofTable_6, dofTable_7, dofTable_8, dofTable_9,
                             dofTable_10,dofTable_11,dofTable_12,dofTable_13,
                             dofTable_14
                             };

      inBufs.bufDesc[SFM_TI_IN_BUFDESC_IN_DOF_BUFFER]->bufPlanes[0].buf = (void*)dofTableList[frameIdx-startFrame];
#endif
      inBufs.bufDesc[SFM_TI_IN_BUFDESC_IN_DOF_BUFFER]->bufferId    = frameIdx;
      outBufs.bufDesc[SFM_TI_OUT_BUFDESC_FEATURE_PLANES]->bufferId = frameIdx;

      /*-----------------------------------------------------------------------
      Population of inArgs parameters
      ---------------------------------------------------------------------*/
      inArgs.maxRansacItr      = testcaseParams[testcase].maxRansacItr;
      inArgs.maxTriangItr      = testcaseParams[testcase].maxTriangItr;
      inArgs.fMatrixPrunEn     = testcaseParams[testcase].fMatrixPrunEn;
      inArgs.fMatrixInTh       = testcaseParams[testcase].fMatrixInTh;
      inArgs.pointPruneAngle   = testcaseParams[testcase].pointPruneAngle;
      inArgs.fMatrixCalcMethod = testcaseParams[testcase].fMatrixCalcMethod;
      inArgs.flowConfThr       = testcaseParams[testcase].flowConfThr;
      inArgs.flowInvalidPadX   = testcaseParams[testcase].flowInvalidPadX;
      inArgs.flowInvalidPadY   = testcaseParams[testcase].flowInvalidPadY;
      inArgs.reserved0         = 0x0;
      inArgs.reset             = 0x0;

      //printf("Frame#%5d \n",frameIdx);

#ifdef TIADALG_UC_AVV_BUILD
      volatile uint32_t* tiadalg_DebugLevel = (uint32_t*)(TIADALG_DBUG_CTRL_BASE);

      if((*tiadalg_DebugLevel) == 0x12345678)
      {
        inArgs.reserved0 = tiadalg_DebugLevel[2];
      }
      if(1)
      {
        printf("c7x_1: Initialisation complete, waiting for Sync!\n");
      }
      volatile uint32_t* syncReadPtr_UC  = (uint32_t*)(MULTICORE_SYNC_BASE) + C7_1_RD_OFFSET;
      volatile uint32_t* syncWritePtr_UC = (uint32_t*)(MULTICORE_SYNC_BASE) + C7_1_WR_OFFSET;
      volatile int32_t i0 = 0;

      (*syncWritePtr_UC) = MULTICORE_SYNC_VAL; //Inform init. is complete

      while((*syncReadPtr_UC) != MULTICORE_SYNC_VAL)
      {
        //Do nothing till sync value is received.
        for(i0 = 0 ; i0 < SMALL_WAIT_TIME; i0++)
        {
          //Avoiding pressure on the memory system while polling
        }
      }

      (*syncReadPtr_UC) = 0x0; //Clear sync value from readPtr

      if(inArgs.reserved0)
      {
        printf("c7x_1 : Received Sync, starting core processing...\n");
      }
#endif

#ifndef HOST_EMULATION
#ifdef TIADALG_OS_BUILD
      //Cache_wbInvAll();
#endif
#endif
      printf("process call started \n");

#if (!HOST_EMULATION)
      t0 = _TSC_read();
#endif

      /*-----------------------------------------------------------------------
      SFM Algorithm process call
      ---------------------------------------------------------------------*/
      status = handle->ivision->algProcess((IVISION_Handle)handle,
        &inBufs,
        &outBufs,
        (IVISION_InArgs  *)&inArgs,
        (IVISION_OutArgs *)&outArgs);

#if (!HOST_EMULATION)
      t1 = _TSC_read();
#endif

      printf("process call finished \n");

      if(outArgs.iVisionOutArgs.inFreeBufIDs[0] != frameIdx)
        printf("\n Problem in in free Id");

      if(outArgs.iVisionOutArgs.outFreeBufIDs[0] != frameIdx)
        printf("\n Problem in out free Id");

      //if(inArgs.reserved0)
      if(1)
      {
        printf("  Out 3D Point         = %15d\n",outArgs.outNumPoints);
        printf("  numIterFMat          = %15d\n",outArgs.sfmStats.numIterFMat);
        printf("  numInlierFmat        = %15d\n",outArgs.sfmStats.numInlierFmat);
        printf("  numCur3DPnts         = %15d\n",outArgs.sfmStats.numCur3DPnts);
        printf("  isStatic             = %15d\n",outArgs.sfmStats.isStatic);
#ifndef HOST_EMULATION
        printf("  Total Cycle          = %15.2f Mega Cycles\n",((float)(t1 - t0))/1000000.0f);
#endif
      }

#ifdef TIADALG_UC_AVV_BUILD
#ifndef HOST_EMULATION
      ((uint32_t*)(TIADALG_DBUG_CTRL_BASE))[3 + frameIdx] = (int32_t)(((float)(t1 - t0))/1000.0f);
#endif
#endif

#ifdef ENABLE_PROFILE

      long long totOverHead = 0;
      long long prfCycSum = 0;

      /*-----------------------------------------------------------------------
      createParams :: rsvd1 is used for enabling the profiling of internals
      of SFM algorithm
      -----------------------------------------------------------------------*/
      if(createParams.rsvd1 != 0x0){

        prfInfo = (TI_DSP_PrfInfo*)outArgs.sfmStats.rsvd1;

        printf("  Sub-Modules Profile Info\n");
        int32_t md;

        for(md = 0; md < TI_DSP_PROFILE_MAX; md ++){
          if((prfInfo->prfData[md].moduleId != -1))
            printf("    MODULE%3d TotalCycle = %10lld TotalCount = %10d avgCycle = %5lld\n",\
            prfInfo->prfData[md].moduleId,
            prfInfo->prfData[md].totalTime,
            prfInfo->prfData[md].count,
            (prfInfo->prfData[md].totalTime/prfInfo->prfData[md].count));
            prfCycSum += prfInfo->prfData[md].totalTime;
        }
        totOverHead = prfInfo->totalOvehead;
      }

      printf("  Total Cycle From Profile Module   = %15lld\n", prfCycSum);
#endif
      if(status != IALG_EOK)
      {
        PRINT_ERROR_MSG();
        goto EXIT_LOOP;
      }

#if ENABLE_TRACES
      printf("Processing Completed for frame %d \n",frameIdx);
#endif

      /*-----------------------------------------------------------------------
      Write out the 3D points location in .txt  file and also on YUV file
      -----------------------------------------------------------------------*/
#if (!HOST_EMULATION)
#ifdef TIADALG_OS_BUILD
//  Cache_wbInvAll();
#endif
#endif

#if defined(ENABLE_FILE_IO) || defined (ENABLE_YUV_WRITE)
      SFM_TI_output*  outputSfm;
      outputSfm  = (SFM_TI_output *)outBufs.bufDesc[SFM_TI_OUT_BUFDESC_FEATURE_PLANES]->bufPlanes[0].buf;

      if(testcaseParams[testcase].enAlgPtCldVis == 0)
      {
        TestApp_WriteOutImgBuf(outputSfm,frameIdx,startFrame,
          (char *)testcaseParams[testcase].outFeatFileName,
          (char *)testcaseParams[testcase].inImgFileName,
          (char *)testcaseParams[testcase].outImgFileName,
          endFrame,
          testcaseParams[testcase].imageWidth,
          testcaseParams[testcase].imageHeight,
          outArgs.outNumPoints);
      }
      else
      {
        uint8_t* outImgL = (uint8_t *)outBufs.bufDesc[SFM_TI_OUT_BUFDESC_LUMA_PTCLD_BUFFER]->bufPlanes[0].buf;
        uint8_t* outImgC = (uint8_t *)outBufs.bufDesc[SFM_TI_OUT_BUFDESC_CHROMA_PTCLD_BUFFER]->bufPlanes[0].buf;

        char fileName[200];
        tiadalg_buffer_context bmp_context;

        if(createParams.imgColorFmt == 0)
        {
          sprintf(fileName,(char*)testcaseParams[testcase].outImgFileName,frameIdx);
          FILE* fp;
          fp = fopen(fileName,"wb");
          int32_t chroma_plane_size = ((createParams.imgHeight + 1)>>1)*createParams.imgWidth;

          if(fp == NULL)
          {
            printf("could not open the output file for writing %s \n", fileName);
          }

          memset(outImgL, 0x0, createParams.imgWidth*createParams.imgHeight);

          fwrite(outImgL, 1, createParams.imgWidth*createParams.imgHeight, fp);
          fwrite(outImgC, 1, chroma_plane_size, fp);

          fclose(fp);
        }
        else
        {
          sprintf(fileName,(char*)testcaseParams[testcase].inImgFileName,frameIdx);
          tiadalg_read_bmp_context(fileName, &bmp_context, NULL, 0x0);
          sprintf(fileName,(char*)testcaseParams[testcase].outImgFileName,frameIdx);
          tiadalg_save_bmp_context(fileName, &bmp_context, outImgL);
        }
      }
#endif

#if defined(ENABLE_PLY_TXT_WRITE) || defined(ENABLE_PLY_BIN_WRITE)
      TestApp_WriteOutPlyBuf(outputSfm,frameIdx,startFrame,
        (char *)testcaseParams[testcase].outFeatFileName,
        (char *)testcaseParams[testcase].inImgFileName,
        (char *)testcaseParams[testcase].outImgFileName,
        endFrame,
        testcaseParams[testcase].imageWidth,
        testcaseParams[testcase].imageHeight,
        outArgs.outNumPoints);
#endif

#if !defined(ENABLE_FILE_IO)

#ifdef COMPARE_REF_OUTPUT
      int32_t isPass = 1;

      const SFM_TI_output* outTableList[15] = {NULL, NULL, NULL, NULL, NULL, outTable_5, outTable_6, outTable_7,
                                    outTable_8, outTable_9, outTable_10, outTable_11, outTable_12, outTable_13, outTable_14};

      SFM_TI_output* curOutTable = (SFM_TI_output*)outBufs.bufDesc[SFM_TI_OUT_BUFDESC_FEATURE_PLANES]->bufPlanes[0].buf;
      const SFM_TI_output* refOutTable = (SFM_TI_output*)outTableList[frameIdx-startFrame];

      if(refOutTable != NULL){
        int32_t num_obj = outArgs.outNumPoints;
        int32_t i;
        for(i = 0; i < num_obj; i++){
          if( (fabsf(curOutTable[i].point2dX - refOutTable[i].point2dX) > 0.5f)||
              (fabsf(curOutTable[i].point2dY - refOutTable[i].point2dY) > 0.5f)||
              (fabsf(curOutTable[i].point3dX - refOutTable[i].point3dX) > 0.5f)||
              (fabsf(curOutTable[i].point3dY - refOutTable[i].point3dY) > 0.5f)||
              (fabsf(curOutTable[i].point3dZ - refOutTable[i].point3dZ) > 0.5f)||
              (curOutTable[i].trackAge != refOutTable[i].trackAge) ||
              (curOutTable[i].trackId  != refOutTable[i].trackId)
          ){
            isPass= 0;
            printf("Mismatch Happened for object no %d \n",i);
            printf("point2dX %f, %f \n",curOutTable[i].point2dX,refOutTable[i].point2dX);
            printf("point2dY %f, %f \n",curOutTable[i].point2dY,refOutTable[i].point2dY);
            printf("point3dX %f, %f \n",curOutTable[i].point3dX,refOutTable[i].point3dX);
            printf("point3dY %f, %f \n",curOutTable[i].point3dY,refOutTable[i].point3dY);
            printf("point3dZ %f, %f \n",curOutTable[i].point3dZ,refOutTable[i].point3dZ);
            printf("trackAge %d, %d \n",curOutTable[i].trackAge,refOutTable[i].trackAge);
            printf("trackId %f, %f \n",curOutTable[i].trackId,refOutTable[i].trackId);
          }
        }
      }
      if(isPass == 0) printf("Frame # %d FAIL :-( !! \n\n",frameIdx);
      if(isPass == 1) printf("Frame # %d PASS :-) !!\n\n",frameIdx);
      if(isPass == 0)
#else

      {
        char file_name2[200];
        sprintf(file_name2,"%s//out_%d.h","./../data/structure_from_motion/output/",frameIdx);
        FILE* fp = fopen(file_name2, "w");

        if(fp == NULL)
        {
          printf("could not open the file for writing the reference output \n");
          exit(0);
        }

        SFM_TI_output* out_ptr = (SFM_TI_output*) outBufs.bufDesc[SFM_TI_OUT_BUFDESC_FEATURE_PLANES]->bufPlanes[0].buf;

        //sprintf(file_name2,"#pragma DATA_SECTION (outTable_%d, \".ioBufs\") \n",frame_num_out);
        //fprintf(fp,"%s",file_name2);

        sprintf(file_name2,"const SFM_TI_output outTable_%d[%d] = {",frameIdx, outArgs.outNumPoints);

        fprintf(fp,"%s",file_name2);
        int32_t i;

        for(i =0 ; i < outArgs.outNumPoints; i++)
        {
          fprintf(fp,"{");
          fprintf(fp,"%05.1f, %05.1f, %05.1f, %05.1f, %05.1f, %d, %d",
                  out_ptr[i].point2dX, out_ptr[i].point2dY,
                  out_ptr[i].point3dX, out_ptr[i].point3dY, out_ptr[i].point3dZ,
                  out_ptr[i].trackAge,out_ptr[i].trackId
                  );
          fprintf(fp,"},\n");
        }

        fprintf(fp,"}; \n\n\n");
        fclose(fp);
      }
#endif
#endif

    }

EXIT_LOOP:

    status = handle->ivision->ialg.algFree((IALG_Handle)(handle), memRec);

    if(status != IALG_EOK)
    {
      PRINT_ERROR_MSG();
    }

    /* Here Free memory for all the mem records */
    status = TestApp_FreeMemRecords(memRec,numMemRec);

    if(status != IALG_EOK)
    {
      PRINT_ERROR_MSG();
    }

    if ( memRec != NULL )
    {
      free(memRec);
    }

#if 0 // in and out buffers are statically allocated , not through malloc
    for ( levelCnt = 0 ; levelCnt < inBufDesc.numPlanes ; levelCnt++)
    {
      if ( inBufDesc.bufPlanes[levelCnt].buf != NULL )
      {
        free(inBufDesc.bufPlanes[levelCnt].buf);
      }
    }

    for ( levelCnt = 0 ; levelCnt < outBufDesc.numPlanes ; levelCnt++)
    {
      if ( outBufDesc.bufPlanes[levelCnt].buf != NULL )
      {
        free(outBufDesc.bufPlanes[levelCnt].buf);
      }
    }
#endif

  }
  return status;
}

int32_t TestApp_AllocMemRecords(IALG_MemRec * memRec,int32_t numMemRec)
{
  int32_t i;

  TIMemHandle memHdl_DMEM2 = &memObj_DMEM2;
  TIMemHandle memHdl_DMEM1 = &memObj_DMEM1;
  TIMemHandle memHdl_DMEM0 = &memObj_DMEM0;

  for (i = 0; i < numMemRec; i++)
  {
    if(memRec[i].space == IALG_DARAM2) {
      memRec[i].base = tiadalg_get_memory_chunk(memHdl_DMEM2, memRec[i].size, memRec[i].alignment);

      printf("Requested IALG_DARAM2 is %d, and Addr is %lx \n", memRec[i].size, (uint64_t)memRec[i].base);

      if(memRec[i].base == NULL) {
        memRec[i].base = (void *) malloc( memRec[i].size);
        memRec[i].space = IALG_EXTERNAL;
        printf("Requested memory could not be allocated \n");
        printf("Hence allocating from EXtmem is %d, and Addr is %lx \n", memRec[i].size, (uint64_t)memRec[i].base);
      }
    }
    else if(memRec[i].space == IALG_DARAM1) {
      memRec[i].base = tiadalg_get_memory_chunk(memHdl_DMEM1, memRec[i].size, memRec[i].alignment);

      printf("Requested IALG_DARAM1 is %d, and Addr is %lx \n", memRec[i].size, (uint64_t)memRec[i].base);

      if(memRec[i].base == NULL) {
        memRec[i].base = (void *) malloc( memRec[i].size);
        memRec[i].space = IALG_EXTERNAL;
        printf("Requested memory could not be allocated \n");
        printf("Hence allocating from EXtmem is %d, and Addr is %lx \n", memRec[i].size, (uint64_t)memRec[i].base);
      }
    }
    else if(memRec[i].space == IALG_DARAM0) {
      /*L1D SRAM space, currently alg requires 16kb SRAM and 16kb cache.
      */
      memRec[i].base = tiadalg_get_memory_chunk(memHdl_DMEM0, memRec[i].size, memRec[i].alignment);
      printf("Requested IALG_DARAM0 is %d, and Addr is %lx \n", memRec[i].size, (uint64_t)memRec[i].base);

      if(memRec[i].base == NULL) {
        memRec[i].base = tiadalg_get_memory_chunk(memHdl_DMEM1, memRec[i].size, memRec[i].alignment);
        memRec[i].space = IALG_DARAM1;
        printf("Requested memory could not be allocated \n");
        printf("Hence allocating from memHdl_DMEM1 is %d, and Addr is %lx \n", memRec[i].size, (uint64_t)memRec[i].base);
      }

      if(memRec[i].base == NULL) {
        memRec[i].base = (void *) malloc( memRec[i].size);
        memRec[i].space = IALG_EXTERNAL;
        printf("Requested memory could not be allocated \n");
        printf("Hence allocating EXtmem is %d, and Addr is %lx \n", memRec[i].size, (uint64_t)memRec[i].base);
      }

    }
    else {
      /* From heap present in external memory
      */

      memRec[i].base = (void *) malloc( memRec[i].size);

      printf("Requested EXtmem is %d, and Addr is %lx \n", memRec[i].size, (uint64_t)memRec[i].base);

    }
    if(memRec[i].base == NULL)
    {
      return IALG_EFAIL;
    }
  }
  return IALG_EOK;
}
int32_t TestApp_memContamination(IALG_MemRec * memRec,int32_t numMemRec, int32_t isFirstFrm){

  int32_t i,j;
  int32_t size;
  float*  src;

  for (i = 0; i < numMemRec; i++)
  {
    if(memRec[i].attrs == IALG_SCRATCH){

      size = memRec[i].size >> 2;
      src  = (float*)memRec[i].base;
      for(j = 0; j < size; j++)
        src[j] = FLT_MAX;
    }
    if((memRec[i].attrs == IALG_PERSIST) && (isFirstFrm == 1)){

      size = memRec[i].size >> 2;
      src  = (float*)memRec[i].base;
      for(j = 0; j < size; j++)
        src[j] = FLT_MAX;
    }
  }
  return(0);
}

int32_t TestApp_FreeMemRecords(IALG_MemRec * memRec,int32_t numMemRec)
{
  int32_t i;
  for (i = 0; i < numMemRec; i++)
  {
    if(memRec[i].base == NULL)
    {
      return IALG_EFAIL;
    }
    if(memRec[i].space == IALG_EXTERNAL) {
      free(memRec[i].base);
    }
  }
  return IALG_EOK;
}

int32_t TestApp_AllocIOBuffers(IVISION_InBufs * inBufs,
                               IVISION_OutBufs * outBufs,
                               int32_t totInPixs,
                               uint16_t inNumTracks,
                               int32_t imgClrFmt)
{
  /*Dof input buffer*/
  inBufs->bufDesc[SFM_TI_IN_BUFDESC_IN_DOF_BUFFER]->numPlanes                          = 1;
  inBufs->bufDesc[SFM_TI_IN_BUFDESC_IN_DOF_BUFFER]->bufPlanes[0].frameROI.topLeft.x    = 0;
  inBufs->bufDesc[SFM_TI_IN_BUFDESC_IN_DOF_BUFFER]->bufPlanes[0].frameROI.topLeft.y    = 0;
  inBufs->bufDesc[SFM_TI_IN_BUFDESC_IN_DOF_BUFFER]->bufPlanes[0].width                 = totInPixs*4;
  inBufs->bufDesc[SFM_TI_IN_BUFDESC_IN_DOF_BUFFER]->bufPlanes[0].height                = 1;
  inBufs->bufDesc[SFM_TI_IN_BUFDESC_IN_DOF_BUFFER]->bufPlanes[0].frameROI.width        =
                            inBufs->bufDesc[SFM_TI_IN_BUFDESC_IN_DOF_BUFFER]->bufPlanes[0].width;
  inBufs->bufDesc[SFM_TI_IN_BUFDESC_IN_DOF_BUFFER]->bufPlanes[0].frameROI.height       = 1;
  inBufs->bufDesc[SFM_TI_IN_BUFDESC_IN_DOF_BUFFER]->bufPlanes[0].planeType             = 0;

  /*size = sizeof(SFM_TI_trackInfo) * inNumTracks;*/

  inBufs->bufDesc[SFM_TI_IN_BUFDESC_IN_DOF_BUFFER]->bufPlanes[0].buf = IN_BUF;

  if((inBufs->bufDesc[SFM_TI_IN_BUFDESC_IN_DOF_BUFFER]->bufPlanes[0].buf == NULL))
  {
    PRINT_ERROR_MSG();
  }

  /*Input Image buffer*/
  inBufs->bufDesc[SFM_TI_IN_BUFDESC_IN_LUMA_IMG_BUFFER]->numPlanes                          = 1;
  inBufs->bufDesc[SFM_TI_IN_BUFDESC_IN_LUMA_IMG_BUFFER]->bufPlanes[0].frameROI.topLeft.x    = 0;
  inBufs->bufDesc[SFM_TI_IN_BUFDESC_IN_LUMA_IMG_BUFFER]->bufPlanes[0].frameROI.topLeft.y    = 0;
  inBufs->bufDesc[SFM_TI_IN_BUFDESC_IN_LUMA_IMG_BUFFER]->bufPlanes[0].width                 = totInPixs; // only luma is the input
  inBufs->bufDesc[SFM_TI_IN_BUFDESC_IN_LUMA_IMG_BUFFER]->bufPlanes[0].height                = 1;
  inBufs->bufDesc[SFM_TI_IN_BUFDESC_IN_LUMA_IMG_BUFFER]->bufPlanes[0].frameROI.width        =
                          inBufs->bufDesc[SFM_TI_IN_BUFDESC_IN_LUMA_IMG_BUFFER]->bufPlanes[0].width;
  inBufs->bufDesc[SFM_TI_IN_BUFDESC_IN_LUMA_IMG_BUFFER]->bufPlanes[0].frameROI.height       = 1;
  inBufs->bufDesc[SFM_TI_IN_BUFDESC_IN_LUMA_IMG_BUFFER]->bufPlanes[0].planeType             = 0;

  /*size = sizeof(SFM_TI_trackInfo) * inNumTracks;*/

  inBufs->bufDesc[SFM_TI_IN_BUFDESC_IN_LUMA_IMG_BUFFER]->bufPlanes[0].buf = IN_BUF + totInPixs*sizeof(uint32_t);

  if((inBufs->bufDesc[SFM_TI_IN_BUFDESC_IN_LUMA_IMG_BUFFER]->bufPlanes[0].buf == NULL))
  {
    PRINT_ERROR_MSG();
  }

  if((totInPixs*(sizeof(uint32_t) + 1)) > MAX_INPUT_BUF_SIZE) // 1 for luma plane
  {
    printf("Input memory requested is more than satic buffer \n");
    PRINT_ERROR_MSG();
  }

  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_FEATURE_PLANES]->numPlanes                          = 1;
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_FEATURE_PLANES]->bufPlanes[0].frameROI.topLeft.x    = 0;
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_FEATURE_PLANES]->bufPlanes[0].frameROI.topLeft.y    = 0;
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_FEATURE_PLANES]->bufPlanes[0].width                 = inNumTracks*sizeof(SFM_TI_output);
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_FEATURE_PLANES]->bufPlanes[0].height                = 1;
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_FEATURE_PLANES]->bufPlanes[0].frameROI.width        =
                          outBufs->bufDesc[SFM_TI_OUT_BUFDESC_FEATURE_PLANES]->bufPlanes[0].width;
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_FEATURE_PLANES]->bufPlanes[0].frameROI.height       = 1;
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_FEATURE_PLANES]->bufPlanes[0].planeType             = 0;

  /*size = outBufs->bufDesc[0]->bufPlanes[0].width  *
  outBufs->bufDesc[0]->bufPlanes[0].height *
  sizeof(SFM_TI_output);*/

  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_FEATURE_PLANES]->bufPlanes[0].buf = OUT_BUF;

  if((outBufs->bufDesc[SFM_TI_OUT_BUFDESC_FEATURE_PLANES]->bufPlanes[0].buf == NULL))
  {
    PRINT_ERROR_MSG();
  }

  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_LUMA_PTCLD_BUFFER]->numPlanes                          = 1;
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_LUMA_PTCLD_BUFFER]->bufPlanes[0].frameROI.topLeft.x    = 0;
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_LUMA_PTCLD_BUFFER]->bufPlanes[0].frameROI.topLeft.y    = 0;
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_LUMA_PTCLD_BUFFER]->bufPlanes[0].width                 = totInPixs; // 3/2 for YUV; input is Y and output is YUV
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_LUMA_PTCLD_BUFFER]->bufPlanes[0].height                = 1;
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_LUMA_PTCLD_BUFFER]->bufPlanes[0].frameROI.width        =
                          outBufs->bufDesc[SFM_TI_OUT_BUFDESC_LUMA_PTCLD_BUFFER]->bufPlanes[0].width;
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_LUMA_PTCLD_BUFFER]->bufPlanes[0].frameROI.height       = 1;
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_LUMA_PTCLD_BUFFER]->bufPlanes[0].planeType             = 0;

  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_LUMA_PTCLD_BUFFER]->bufPlanes[0].buf = OUT_BUF + inNumTracks*sizeof(SFM_TI_output);

  if((outBufs->bufDesc[SFM_TI_OUT_BUFDESC_LUMA_PTCLD_BUFFER]->bufPlanes[0].buf == NULL))
  {
    PRINT_ERROR_MSG();
  }

  if(((inNumTracks*sizeof(SFM_TI_output)) + ((totInPixs)>>0)) > MAX_OUTPUT_BUF_SIZE) // 3/2 for YUV
  {
    printf("Input memory requested is more than satic buffer \n");
    PRINT_ERROR_MSG();
  }

  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_CHROMA_PTCLD_BUFFER]->numPlanes                          = 1;
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_CHROMA_PTCLD_BUFFER]->bufPlanes[0].frameROI.topLeft.x    = 0;
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_CHROMA_PTCLD_BUFFER]->bufPlanes[0].frameROI.topLeft.y    = 0;
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_CHROMA_PTCLD_BUFFER]->bufPlanes[0].width                 = totInPixs >> 1; // 3/2 for YUV; input is Y and output is YUV
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_CHROMA_PTCLD_BUFFER]->bufPlanes[0].height                = 1;
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_CHROMA_PTCLD_BUFFER]->bufPlanes[0].frameROI.width        =
                          outBufs->bufDesc[SFM_TI_OUT_BUFDESC_CHROMA_PTCLD_BUFFER]->bufPlanes[0].width;
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_CHROMA_PTCLD_BUFFER]->bufPlanes[0].frameROI.height       = 1;
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_CHROMA_PTCLD_BUFFER]->bufPlanes[0].planeType             = 0;

  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_CHROMA_PTCLD_BUFFER]->bufPlanes[0].buf = OUT_BUF + inNumTracks*sizeof(SFM_TI_output) + totInPixs;

  if((outBufs->bufDesc[SFM_TI_OUT_BUFDESC_CHROMA_PTCLD_BUFFER]->bufPlanes[0].buf == NULL))
  {
    PRINT_ERROR_MSG();
  }

  if(((inNumTracks*sizeof(SFM_TI_output)) + ((totInPixs*3)>>1)) > MAX_OUTPUT_BUF_SIZE) // 3/2 for YUV
  {
    printf("Input memory requested is more than satic buffer \n");
    PRINT_ERROR_MSG();
  }

  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_LUMA_OCPGD_BUFFER]->numPlanes                          = 1;
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_LUMA_OCPGD_BUFFER]->bufPlanes[0].frameROI.topLeft.x    = 0;
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_LUMA_OCPGD_BUFFER]->bufPlanes[0].frameROI.topLeft.y    = 0;
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_LUMA_OCPGD_BUFFER]->bufPlanes[0].width                 = totInPixs; // 3/2 for YUV; input is Y and output is YUV
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_LUMA_OCPGD_BUFFER]->bufPlanes[0].height                = 1;
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_LUMA_OCPGD_BUFFER]->bufPlanes[0].frameROI.width        =
                          outBufs->bufDesc[SFM_TI_OUT_BUFDESC_LUMA_OCPGD_BUFFER]->bufPlanes[0].width;
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_LUMA_OCPGD_BUFFER]->bufPlanes[0].frameROI.height       = 1;
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_LUMA_OCPGD_BUFFER]->bufPlanes[0].planeType             = 0;

  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_LUMA_OCPGD_BUFFER]->bufPlanes[0].buf = OUT_BUF + inNumTracks*sizeof(SFM_TI_output);

  if((outBufs->bufDesc[SFM_TI_OUT_BUFDESC_LUMA_OCPGD_BUFFER]->bufPlanes[0].buf == NULL))
  {
    PRINT_ERROR_MSG();
  }

  if(((inNumTracks*sizeof(SFM_TI_output)) + ((totInPixs)>>0)) > MAX_OUTPUT_BUF_SIZE) // 3/2 for YUV
  {
    printf("Input memory requested is more than satic buffer \n");
    PRINT_ERROR_MSG();
  }

  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_CHROMA_OCPGD_BUFFER]->numPlanes                          = 1;
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_CHROMA_OCPGD_BUFFER]->bufPlanes[0].frameROI.topLeft.x    = 0;
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_CHROMA_OCPGD_BUFFER]->bufPlanes[0].frameROI.topLeft.y    = 0;
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_CHROMA_OCPGD_BUFFER]->bufPlanes[0].width                 = totInPixs >> 1; // 3/2 for YUV; input is Y and output is YUV
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_CHROMA_OCPGD_BUFFER]->bufPlanes[0].height                = 1;
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_CHROMA_OCPGD_BUFFER]->bufPlanes[0].frameROI.width        =
                          outBufs->bufDesc[SFM_TI_OUT_BUFDESC_CHROMA_OCPGD_BUFFER]->bufPlanes[0].width;
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_CHROMA_OCPGD_BUFFER]->bufPlanes[0].frameROI.height       = 1;
  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_CHROMA_OCPGD_BUFFER]->bufPlanes[0].planeType             = 0;

  outBufs->bufDesc[SFM_TI_OUT_BUFDESC_CHROMA_OCPGD_BUFFER]->bufPlanes[0].buf = OUT_BUF + inNumTracks*sizeof(SFM_TI_output) + totInPixs;

  if((outBufs->bufDesc[SFM_TI_OUT_BUFDESC_CHROMA_OCPGD_BUFFER]->bufPlanes[0].buf == NULL))
  {
    PRINT_ERROR_MSG();
  }

  if(((inNumTracks*sizeof(SFM_TI_output)) + ((totInPixs*3)>>1)) > MAX_OUTPUT_BUF_SIZE) // 3/2 for YUV
  {
    printf("Input memory requested is more than satic buffer \n");
    PRINT_ERROR_MSG();
  }

  return IALG_EOK;
}

#ifdef ENABLE_FILE_IO
int32_t TestApp_ReadInBuf(
  void * dst0,
  void * dst1,
  int32_t currFrameNum,
  int32_t enAlgPtCldVis,
  char * dofFolderName,
  char* inImgName,
  int32_t img_width,
  int32_t img_height,
  int32_t inImgClrFmt)
{
  /*Dof file reading*/
  FILE* fp_in_feat;
  char fileName[TIADALG_FILE_NAME_SIZE];
  int32_t guard_pixels = 0;
  int32_t read_rows  = img_height - 2*guard_pixels;
  int32_t i;

  sprintf(fileName, "%s/dof_%d.flow",dofFolderName,currFrameNum);

  fp_in_feat   = FOPEN(fileName, "rb");

  if(fp_in_feat == NULL)
  {
    printf("input buffer could not be read \n");
  }
  else
  {
    memset(dst0, 0x0, img_width*img_height);

    fseek(fp_in_feat, guard_pixels*img_width*sizeof(int32_t), SEEK_SET); // skip top guard pixels number of rows
    dst0 = (void*)(((uint8_t*)dst0) + (guard_pixels*img_width*sizeof(int32_t)));

    for(i = 0; i < read_rows; i++)
    {
      //printf("i=%d\n",i);
      FREAD(dst0, 1, img_width*sizeof(int32_t), fp_in_feat);
      dst0 = (void*)(((uint8_t*)dst0) + (img_width*sizeof(int32_t)));
    }

    FCLOSE(fp_in_feat);
  }

#ifdef ENABLE_YUV_WRITE
  /*Input image reading for visualization drawing in algorithm itself*/
  sprintf(fileName,"%s/%08d.yuv",inImgName,currFrameNum);
  if(inImgClrFmt)
  {
    tiadalg_buffer_context bmp_context;
    tiadalg_read_bmp_context(fileName, &bmp_context, dst1, 0x1);
  }
  else
  {
    FILE* fp;
    fp = fopen(fileName,"rb");
    if(fp == NULL)
    {
      printf("input YUV file could not be read \n");
      return 0;
    }
    fread(dst1, 1, img_width*img_height, fp);// read only part
    fclose(fp);
  }
#endif
  return 1;
}

int32_t TestApp_ReadInCameraPrm(float * dstExt,
                                float * dstInt,
                                int32_t currFrameNum,
                                int32_t startFrameNum,
                                char * fileName,
                                int32_t maxFrameNum)
{
  int32_t i;
  char    tempStr[200];
  int32_t extPrmSizeInRows = 4;
  int32_t intPramStartRow  = 14;
  int32_t extPramStartRow  = 28;
  int32_t ymlFileStartNo   = 0;
  int32_t modStartNo       = startFrameNum - ymlFileStartNo;

  /* Specific format for the file is assumed. If that format is not maintained
  * in the camera parameters file, then this code will fail.
  */

  if(currFrameNum == startFrameNum) {
    fp_in_cam   = fopen(fileName, "rb");

    if(fp_in_cam == NULL)
      exit(0);

    for(i = 0; i < intPramStartRow; i++) {
      fgets(tempStr,200,fp_in_cam);
    }
    i=0;
    do{
      i++;
    }while(tempStr[i] != '[');
    sscanf(&tempStr[i+1],"%f,%f,%f",&dstInt[0],&dstInt[1],&dstInt[2]);

    fgets(tempStr,200,fp_in_cam);
    sscanf(&tempStr[0],"%f,%f,%f",&dstInt[3],&dstInt[4],&dstInt[5]);

    fgets(tempStr,200,fp_in_cam);
    sscanf(&tempStr[0],"%f,%f,%f",&dstInt[6],&dstInt[7],&dstInt[8]);

    for(i=0;i< (extPramStartRow - intPramStartRow - 2 + extPrmSizeInRows * modStartNo);i++) {
      fgets(tempStr,200,fp_in_cam);
    }

    i=0;

    if(currFrameNum == ymlFileStartNo)
    {
      do{
        i++;
      }while(tempStr[i] != '[');
    }
  } else {
    i = -1;
    fgets(tempStr,200,fp_in_cam);
  }

  /* First 2 bytes represents number of feature points information. After wards
  * ONE_FEATURE_INFO_SIZE many feature points information is placed in file.
  */
  sscanf(&tempStr[i+1],"%f,%f,%f,%f,",&dstExt[0],&dstExt[1],&dstExt[2], &dstExt[3]);
  fgets(tempStr,200,fp_in_cam);
  sscanf(&tempStr[0],"%f,%f,%f,%f,",&dstExt[4],&dstExt[5],&dstExt[6], &dstExt[7]);
  fgets(tempStr,200,fp_in_cam);
  sscanf(&tempStr[0],"%f,%f,%f,%f,",&dstExt[8],&dstExt[9],&dstExt[10], &dstExt[11]);
  if(extPrmSizeInRows == 4){
    fgets(tempStr,200,fp_in_cam);
    sscanf(&tempStr[0],"%f,%f,%f,%f,",&dstExt[12],&dstExt[13],&dstExt[14], &dstExt[15]);
    if(dstExt[15] != 1.0){
      printf("\n Something wrong has happened");
      exit(0);
    }
  }

  if(currFrameNum == (maxFrameNum - 1)){
    fclose(fp_in_cam);
  }

  return (0);
}

int32_t TestApp_WriteOutPlyBuf(SFM_TI_output * src,int32_t currFrameNum,
                            int32_t startFrameNum,
                            char * featFileName,char* inImgFileName,
                            char* outImgFileName,int32_t maxFrameNum,
                            int32_t imgWidth, int32_t imgHeight,
                            int32_t outNumPoints)
{
  int32_t  i;
  SFM_TI_output* localOutPtr;

#ifdef ENABLE_PLY_TXT_WRITE
  char     localString[200];
  sprintf(localString,"%s%010d.png.ply",featFileName,(currFrameNum));

  fp_out   = fopen(localString, "w");

  if(fp_out == NULL){
    printf("\n Could not open the output file");
    exit(0);
  }
#endif

#ifdef ENABLE_PLY_BIN_WRITE
  FILE* fp_out_bin;
  sprintf(localString,"%s%04d.bin",featFileName,(currFrameNum));
  fp_out_bin   = FOPEN(localString, "wb");

  if(fp_out_bin == NULL){
    printf("\n Could not open the output file");
    exit(0);
  }
#endif

#ifdef ENABLE_PLY_TXT_WRITE
  TestApp_WritePlyHeader(fp_out,outNumPoints);
#endif

  localOutPtr = src;

  for(i=0;i< outNumPoints; i++) {

#ifdef ENABLE_PLY_TXT_WRITE
    float    curX,curY,curZ;
    float    curImgx, curImgy;

    curX = localOutPtr->point3dX;
    curY = localOutPtr->point3dY;
    curZ = localOutPtr->point3dZ;

    curImgx = localOutPtr->point2dX;
    curImgy = localOutPtr->point2dY;

    // format is X Y Z R G B x y frmid trackID
    fprintf(fp_out,"%8.2f %8.2f %8.2f %6d %6d %6d %8.2f %8.2f %6d %6d\n",
      curX,
      curY,
      curZ,
      0x0,
      0x0,
      0x0,
      curImgx,
      curImgy,
      localOutPtr->trackAge,
      localOutPtr->trackId
      );
#endif

    localOutPtr++;
  }


#ifdef ENABLE_PLY_BIN_WRITE
  FWRITE(src,1,sizeof(SFM_TI_output)*outNumPoints,fp_out_bin);
#endif


#ifdef ENABLE_PLY_TXT_WRITE
  fclose(fp_out);
#endif

#ifdef ENABLE_PLY_BIN_WRITE
  FCLOSE(fp_out_bin);
#endif

  return (0);

}

#endif // #ifdef enable_fileio

#ifdef ENABLE_YUV_WRITE
int32_t TestApp_WriteOutImgBuf(SFM_TI_output * src,int32_t currFrameNum,
                            int32_t startFrameNum,
                            char * featFileName,char* inImgFileName,
                            char* outImgFileName,int32_t maxFrameNum,
                            int32_t imgWidth, int32_t imgHeight,
                            int32_t outNumPoints)
{
  int32_t  i;
  float    curX,curY,curZ;
  float    curImgx, curImgy;
  int32_t  int_x, int_y;
  float    dmean, dmean2, curDist;
  float    dStdDev,dMin,dMax;
  int32_t  colorMax = 64;
  int32_t  colorMaxToUse = colorMax * 3 / 4;
  int32_t  colorIndex;
  SFM_TI_output* localOutPtr;

  int32_t  NUM_STEPS = 10;
  float    maxZ4Visulization = 100.0;
  float* dist;

  tiadalg_buffer_context bmp_context;
  char fileName[200];

  sprintf(fileName,inImgFileName,currFrameNum);
  void* imgPtr   = IMG_BUF;
  tiadalg_read_bmp_context(fileName, &bmp_context, imgPtr, 0x01);

  if(outNumPoints){
    dist     = (float*)malloc(outNumPoints * sizeof(float));
    if(dist == NULL){
      printf("Memory could not be allocatd \n");
    }
  }

  dmean       = 0;
  dmean2      = 0;
  localOutPtr = src;

  for(i=0;i<outNumPoints;i++) {

    curX = localOutPtr->point3dX;
    curY = localOutPtr->point3dY;
    curZ = localOutPtr->point3dZ;

    curDist = curX*curX + curY*curY + curZ*curZ;
    dmean2 += curDist;
#if 0//def DSP_INTRINSICS
    curDist = (curDist* _rsqrsp(curDist));
#else
    curDist = sqrtf(curDist);
#endif
    dist[i] = curDist;
    dmean  += curDist;

    localOutPtr ++;
  }

  dmean   = outNumPoints ? dmean / outNumPoints : 0;
  dmean2  = outNumPoints ? dmean2 / outNumPoints : 0;

  dStdDev = dmean2 - dmean*dmean;
#if 0//def DSP_INTRINSICS
  dStdDev = (dStdDev * _rsqrsp (dStdDev));
#else
  dStdDev = sqrtf(dStdDev);
#endif

  dMin    = dmean - 3*dStdDev;
  dMax    = dmean + 3*dStdDev;
  dMax    = (dMin == dMax)? dMin + 1 : dMax;

  //clip range of depth into a certain range for color display.
  dMax = (dMax < 150.0f) ? dMax : 150.0f; //150 meters max
  dMin = (dMin > 0.0f) ? dMin : 0.0f;   //0 meters min

  dMin = 0.0;
  dMax = 192.0;

  localOutPtr = src;

  for(i=0;i< outNumPoints; i++) {

    if(0){
      curDist = dist[i];
      curDist = __mmin(curDist,maxZ4Visulization);
      colorIndex = (curDist * NUM_STEPS / maxZ4Visulization);
      colorIndex = (colorIndex * colorMaxToUse) / NUM_STEPS;
    }else{
      //curDist = localOutPtr->point3dZ;
      curDist = dist[i];
      colorIndex = (curDist - dMin) * colorMaxToUse / (maxZ4Visulization - dMin);
    }

    colorIndex = (colorIndex > 0) ? colorIndex : 0;
    colorIndex = (colorIndex < colorMaxToUse) ?  colorIndex : (colorMaxToUse - 1);

    uint8_t r = HSV_COLOR_MAP[colorIndex][0]*UCHAR_MAX;
    uint8_t g = HSV_COLOR_MAP[colorIndex][1]*UCHAR_MAX;
    uint8_t b = HSV_COLOR_MAP[colorIndex][2]*UCHAR_MAX;

    curX = localOutPtr->point3dX;
    curY = localOutPtr->point3dY;
    curZ = localOutPtr->point3dZ;

    curImgx = localOutPtr->point2dX;
    curImgy = localOutPtr->point2dY;

    int_x = curImgx;
    int_y = curImgy;

    int_x &= 0x7FFFFFFE;
    int_y &= 0x7FFFFFFE;

    int32_t index;

    if((int_x > 0) && (int_y >0) &&
      (int_x < (imgWidth -1))   &&
      (int_y < (imgHeight-1))   &&
      (curDist>dMin)           &&
      (curDist<dMax)){

#ifdef INPUT_YUV
        index = int_y*imgWidth + int_x;
        imgPtr[index    ] = y;
        imgPtr[index + 1] = y;
        imgPtr[index + imgWidth  ] = y;
        imgPtr[index + imgWidth + 1] = y;

        index = (imgWidth*imgHeight) + ((int_y*imgWidth) >> 2) + (int_x >> 1);

        imgPtr[index] = cb;

        index = (imgWidth*imgHeight) + ((imgWidth*imgHeight)>>2) + ((int_y*imgWidth) >> 2) + (int_x >> 1);

        imgPtr[index] = cr;
#else
        index = ((imgHeight - 1 - int_y)*imgWidth + int_x)*3;
        ((uint8_t*)imgPtr)[index + 0] = b;
        ((uint8_t*)imgPtr)[index + 1] = g;
        ((uint8_t*)imgPtr)[index + 2] = r;

        index = ((imgHeight - 1 - int_y)*imgWidth + int_x + 1)*3;
        ((uint8_t*)imgPtr)[index + 0] = b;
        ((uint8_t*)imgPtr)[index + 1] = g;
        ((uint8_t*)imgPtr)[index + 2] = r;

        index = ((imgHeight - 1 - int_y + 1)*imgWidth + int_x)*3;
        ((uint8_t*)imgPtr)[index + 0] = b;
        ((uint8_t*)imgPtr)[index + 1] = g;
        ((uint8_t*)imgPtr)[index + 2] = r;

        index = ((imgHeight - 1 - int_y + 1)*imgWidth + int_x + 1)*3;
        ((uint8_t*)imgPtr)[index + 0] = b;
        ((uint8_t*)imgPtr)[index + 1] = g;
        ((uint8_t*)imgPtr)[index + 2] = r;

#endif

    }
    localOutPtr++;
  }

  sprintf(fileName,outImgFileName,currFrameNum);
  tiadalg_save_bmp_context(fileName, &bmp_context, imgPtr);

  if(outNumPoints)
    free(dist);

  return (0);
}
#endif

#if (!HOST_EMULATION)
void SetMAR()
{

  uint32_t i ;
  /* MAR Reg  Add         Cache able Region
  * MAR 144 (0x01848240h) 9000 0000h - 90FF FFFFh
  * MAR 160 (0x01848280)  A000 0000h - A0FF FFFFh
  */
  /* MAR Reg  Add         Cache able Region
  * MAR 128 (0x01848200) 8000 0000h - 80FF FFFFh
  * MAR 133 (0x01848214) 8500 0000h - 85FF FFFFh
  * MAR 134 (0x01848218) 8600 0000h - 86FF FFFFh
  * MAR 144 (0x01848240) 9000 0000h - 90FF FFFFh
  * MAR 159 (0x0184827c) 9F00 0000h - 9FFF FFFFh
  * MAR 160 (0x01848280) A000 0000h - A0FF FFFFh
  */
  uint32_t* MAR = (uint32_t*)0x01848000 ;

  for(i = 144 ;i <= 160 ; i++)
  {
    MAR[i] = 0x1;
  }

  for(i = 128 ;i <= 133 ; i++)
  {
    MAR[i] = 0x1;
  }

  return ;
}

void InitCache()
{
#ifdef _TMS320C6600
  /*--------------------------------------------------------------------------*/
  /* 7-> full,6 -> 1024KB,5 -> 512KB,4-> 256KB,3 -> 128KB,2-> 64KB,0-->0      */
  /* IMPORTANT ::Currently 160 KB of L2 is assumed to be SRAM, hence in final */
  /* application any amount L2 can be configured as SRAM, but DMVAL will not  */
  /* use that.                                                                */
  /*--------------------------------------------------------------------------*/

  *L2CFG  = 3;

  /*-------------------------------------------------------------------------*/
  /* Full L1P is configured as 32KB of cache                                 */
  /* Nothing in L1P is assumed as SRAM, so no dependency from final applicati*/
  /* -on configuration. Only thing if full L1P is configured as cache then   */
  /* standalone and application performance numbers will match.              */
  /*-------------------------------------------------------------------------*/
  *L1PCFG = 4;

  /*--------------------------------------------------------------------------*/
  /* 1 -> 4 KB, 2-> 8KB, 3 ->16 KB, 4 -> 32KB as cache                       */
  /* Currently 16KB as cache has been allocated for L1D, Rest 16 KB is configu*/
  /* red as SRAM. Any change in cache configuration should be done in conjuca*/
  /* of linker command file modification in test application                 */
  /* IMPORTANT :: Here 16 KB of L1 D is assumed as SRAM hence there is depend*/
  /* -dency from final application configuration of L1D. If atleast 16 KB of */
  /* L1D is not configured as SRAM in final application then that will be a  */
  /* functional issue.                                                       */
  /*-------------------------------------------------------------------------*/
  *L1DCFG = 3;

  SetMAR() ;
#else
#ifndef TIADALG_OS_BUILD
  //enable_cache_mmu((uint64_t)pte_lvl0);
  //CSL_c7xSetL2CacheSize(2);
  //CSL_c7xSetL1DCacheSize(4);
#endif
#endif
}
#endif //!HOST_EMULATION

void TestApp_WritePlyHeader(FILE* fp_out, int32_t outNumPoints)
{
  fprintf(fp_out,"ply\n");
  fprintf(fp_out,"format ascii 1.0\n");
  fprintf(fp_out,"comment PCL generated by ticv using rply\n");
  fprintf(fp_out,"element vertex %d\n",outNumPoints);
  fprintf(fp_out,"property float x\n");
  fprintf(fp_out,"property float y\n");
  fprintf(fp_out,"property float z\n");
  fprintf(fp_out,"property uchar red\n");
  fprintf(fp_out,"property uchar green\n");
  fprintf(fp_out,"property uchar blue\n");
  fprintf(fp_out,"property float x2D\n");
  fprintf(fp_out,"property float y2D\n");
  fprintf(fp_out,"property int frame\n");
  fprintf(fp_out,"property int track\n");
  fprintf(fp_out,"element camera 1\n");
  fprintf(fp_out,"property float view_px\n");
  fprintf(fp_out,"property float view_py\n");
  fprintf(fp_out,"property float view_pz\n");
  fprintf(fp_out,"property float x_axisx\n");
  fprintf(fp_out,"property float x_axisy\n");
  fprintf(fp_out,"property float x_axisz\n");
  fprintf(fp_out,"property float y_axisx\n");
  fprintf(fp_out,"property float y_axisy\n");
  fprintf(fp_out,"property float y_axisz\n");
  fprintf(fp_out,"property float z_axisx\n");
  fprintf(fp_out,"property float z_axisy\n");
  fprintf(fp_out,"property float z_axisz\n");
  fprintf(fp_out,"property float focal\n");
  fprintf(fp_out,"property float scalex\n");
  fprintf(fp_out,"property float scaley\n");
  fprintf(fp_out,"property float centerx\n");
  fprintf(fp_out,"property float centery\n");
  fprintf(fp_out,"property int viewportx\n");
  fprintf(fp_out,"property int viewporty\n");
  fprintf(fp_out,"property float k1\n");
  fprintf(fp_out,"property float k2\n");
  fprintf(fp_out,"end_header\n");

}
#if 0
int32_t sfmCompare(ssfm_Config * testcaseParams) {

  SFM_TI_output* refInfo;
  SFM_TI_output* testInfo;
  SFM_TI_output* curInfo;
  SFM_TI_output* curInfo_2;

  uint8_t refFileName[FILE_NAME_SIZE];
  uint8_t testFileName[FILE_NAME_SIZE];
  uint8_t reportFileName[FILE_NAME_SIZE];

  FILE*   fp_ref;
  FILE*   fp_test;
  FILE*   fp_report;

  int32_t startFrame   = testcaseParams[0].startFrame;
  int32_t endFrame     = startFrame + testcaseParams[0].maxFrames;
  int32_t frameIdx;

  int32_t refTotalFeat;
  int32_t testTotalFeat, matchTotalFeat;;
  int32_t tempR,tempG,tempB;

  uint8_t curString[200];
  int32_t numInliers5, numInliers10, numInliers15,testOrphans, refOrphans;
  int32_t prevTrackId, prevFrmId,i,j,holeTracks;

  float maxDiff, point3dX, point3dY, point3dZ, point2dX, point2dY;
  int32_t frmId, trackId;
  int16_t refFrmId, refTrackId, testFrmId, testTrackId;
  int32_t curTotalPts;
  float dist1, dist2;

  refInfo  = (SFM_TI_output*)malloc(sizeof(SFM_TI_output)*MAX_NUM_OUT_POINTS);
  testInfo = (SFM_TI_output*)malloc(sizeof(SFM_TI_output)*MAX_NUM_OUT_POINTS);

  sprintf((char*)&reportFileName[0],"%s\compareResult.txt",testcaseParams[0].compareRsultPath);

  fp_report = fopen((const char*)&reportFileName[0],"w");

  if(fp_report == NULL)
    exit(0);

  fprintf(fp_report,"frameIdx          testTotalPts        refTotalPts         inlier(+/- 0.5)     inlier(+/- 1.0)     inlier(+/- 1.5)     %testOrphans        %refOrphans\n");

  // Read the data from file and put into memory
  for(frameIdx = startFrame;frameIdx <  (endFrame - VLIB_TRIANG_MAX_POINTS_IN_TRACK); frameIdx++){

    sprintf((char*)&refFileName[0],"%s(%d).ply",testcaseParams[0].refFeatFileName, (frameIdx + VLIB_TRIANG_MAX_POINTS_IN_TRACK -1));
    fp_ref = fopen((const char*)&refFileName[0],"r");

    if(fp_ref == NULL)
      exit(0);

    sprintf((char*)testFileName,"%s%010d.png.ply",testcaseParams[0].outFeatFileName, (frameIdx + VLIB_TRIANG_MAX_POINTS_IN_TRACK -1));
    fp_test = fopen((const char*)&testFileName[0],"r");

    if(fp_test == NULL)
      exit(0);

    fgets((char*)curString,200,fp_ref);
    fgets((char*)curString,200,fp_ref);
    fgets((char*)curString,200,fp_ref);
    fgets((char*)curString,200,fp_ref);

    sscanf((char*)&curString[15],"%d",&refTotalFeat);



    fgets((char*)curString,200,fp_test);
    fgets((char*)curString,200,fp_test);
    fgets((char*)curString,200,fp_test);
    fgets((char*)curString,200,fp_test);

    sscanf((char*)&curString[15],"%d",&testTotalFeat);

    if(refTotalFeat == 0){
      fprintf(fp_report,"%3d          %10d          %10d Reference Triangulation Failed\n",
        (frameIdx + VLIB_TRIANG_MAX_POINTS_IN_TRACK - 1), testTotalFeat, refTotalFeat);
      continue;
    }

    if(testTotalFeat == 0){
      fprintf(fp_report,"%3d          %10d          %10d Test Triangulation Failed\n",
        (frameIdx + VLIB_TRIANG_MAX_POINTS_IN_TRACK - 1), testTotalFeat, refTotalFeat);
      continue;
    }

    numInliers5    = 0;
    numInliers10    = 0;
    numInliers15   = 0;
    testOrphans    = 0;
    refOrphans     = 0;
    matchTotalFeat = 0;

    for(i =0 ;i < 37- 4 -1; i++){
      fgets((char*)curString,200,fp_ref);
      fgets((char*)curString,200,fp_test);
    }

    curInfo      = refInfo;
    curInfo_2    = testInfo;

    for(i = 0; i < MAX_NUM_OUT_POINTS; i++){
      curInfo->frmId   = -1;
      curInfo_2->frmId = -1;
      curInfo++;
      curInfo_2++;
    }

    curInfo      = refInfo;
    prevTrackId  = -1;
    curTotalPts  = 0;
    prevFrmId    = frameIdx + VLIB_TRIANG_MAX_POINTS_IN_TRACK -1;
    fgets((char*)curString,200,fp_ref);

    for(i = 0;i < refTotalFeat; i++) {

      fgets((char*)curString,200,fp_ref);

      sscanf((char*)&curString[0],"%f %f %f %d %d %d %f %f %d %d\n",
        &point3dX,
        &point3dY,
        &point3dZ,
        &tempR,
        &tempG,
        &tempB,
        &point2dX,
        &point2dY,
        &frmId,
        &trackId
        );

      if((frmId > (frameIdx + VLIB_TRIANG_MAX_POINTS_IN_TRACK - 1)) || (frmId < (frameIdx + 1)))
        break;

      holeTracks = (prevFrmId - frmId)*MAX_NUM_IN_POINTS + (trackId - prevTrackId - 1);

      if(holeTracks > MAX_NUM_IN_POINTS)
        printf("\n something went wrong");

      for(j = 0; j < holeTracks; j++) {
        curInfo->frmId = -1;
        curInfo++;
        curTotalPts++;
      }

      curInfo->point3dX  = point3dX;
      curInfo->point3dY  = point3dY;
      curInfo->point3dZ  = point3dZ;

      curInfo->point2dX  = point2dX;
      curInfo->point2dY  = point2dY;

      curInfo->frmId     = frmId;
      curInfo->trackId   = trackId;

      prevTrackId        = trackId;
      prevFrmId          = frmId;
      curInfo++;
      curTotalPts++;
      if(curTotalPts == MAX_NUM_OUT_POINTS)
        break;
    }

    curInfo      = testInfo;
    prevTrackId  = -1;
    curTotalPts  = 0;

    prevFrmId    = frameIdx + VLIB_TRIANG_MAX_POINTS_IN_TRACK - 1;

    fgets((char*)curString,200,fp_test);

    for(i = 0;i < testTotalFeat; i++) {
      fgets((char*)curString,200,fp_test);

      sscanf((char*)&curString[0],"%f %f %f %d %d %d %f %f %d %d\n",
        &point3dX,
        &point3dY,
        &point3dZ,
        &tempR,
        &tempG,
        &tempB,
        &point2dX,
        &point2dY,
        &frmId,
        &trackId
        );

      if((frmId > (frameIdx + VLIB_TRIANG_MAX_POINTS_IN_TRACK - 1)) || (frmId < (frameIdx + 1)))
        break;

      holeTracks = (prevFrmId - frmId)*MAX_NUM_IN_POINTS + (trackId - prevTrackId - 1);

      if(holeTracks > MAX_NUM_IN_POINTS)
        printf("\n something went wrong");

      for(j = 0; j < holeTracks; j++) {
        curInfo->frmId = -1;
        curInfo++;
        curTotalPts++;
      }

      curInfo->point3dX  = point3dX;
      curInfo->point3dY  = point3dY;
      curInfo->point3dZ  = point3dZ;

      curInfo->point2dX  = point2dX;
      curInfo->point2dY  = point2dY;

      curInfo->frmId     = frmId;
      curInfo->trackId   = trackId;

      prevTrackId        = trackId;
      prevFrmId          = frmId;
      curInfo++;
      curTotalPts++;
      if(curTotalPts == MAX_NUM_OUT_POINTS)
        break;
    }

    curInfo   = refInfo;
    curInfo_2 = testInfo;

    for(i = 0; i < MAX_NUM_OUT_POINTS; i++){

      refFrmId      = curInfo->frmId;
      refTrackId    = curInfo->trackId;

      testFrmId     = curInfo_2->frmId;
      testTrackId   = curInfo_2->trackId;


      if((refFrmId != -1) && (testFrmId != -1))
      {
        // test and ref both has generated a 3D point from this track

        /* 3D points comparison
        */
        dist1  = ((curInfo->point3dX - curInfo_2->point3dX)*(curInfo->point3dX - curInfo_2->point3dX)) +
          ((curInfo->point3dY - curInfo_2->point3dY)*(curInfo->point3dY - curInfo_2->point3dY)) +
          ((curInfo->point3dZ - curInfo_2->point3dZ)*(curInfo->point3dZ - curInfo_2->point3dZ));

        dist1  = sqrtf(dist1);

        dist2  = ((curInfo->point2dX - curInfo_2->point2dX)*(curInfo->point2dX - curInfo_2->point2dX)) +
          ((curInfo->point2dY - curInfo_2->point2dY)*(curInfo->point2dY - curInfo_2->point2dY)) ;

        dist2  = sqrtf(dist2);


        if((dist1 < .5) && (dist2 < .5))
          numInliers5++;

        if((dist1 < 1.0) && (dist2 < 1.0))
          numInliers10++;

        if((dist1 < 1.5) && (dist2 < 1.5))
          numInliers15++;

        matchTotalFeat++;

      }else if(refFrmId != -1) {
        refOrphans++;
      }else if(testFrmId != -1){
        testOrphans++;
      } else {
        // feature dropped by ref and test both.
      }

      curInfo++;
      curInfo_2++;
    }

    fprintf(fp_report,"%3d          %10d          %10d          %10.2f          %10.2f          %10.2f          %10d          %10d\n",
      (frameIdx + VLIB_TRIANG_MAX_POINTS_IN_TRACK - 1), testTotalFeat, refTotalFeat,
      ((numInliers5*100.0)/matchTotalFeat),
      ((numInliers10*100.0)/matchTotalFeat),
      ((numInliers15*100.0)/matchTotalFeat),
      testOrphans,
      refOrphans
      );

    fclose(fp_ref);
    fclose(fp_test);
  }
  fclose(fp_report);
  //fclose(refInfo);
  //fclose(testInfo);
  return(0);
}

void host_file_io_ti_init(){
  int32_t i;
  /* making all packets as available
  */
  for(i = 0; i < HOST_MALLOC_TOT_PACKET; i++){
    SCRATCH_PACKET_VALIDITY[i] = 0x0;
  }
}

void* host_file_io_ti_malloc(int32_t size){

  int32_t  contSize   = 0;
  int32_t  numPackets = 0;
  int32_t  baseIndx   = 0;
  void*    baseAddr;
  int32_t i;

  baseAddr   = &SCRATCH_NON_CACHED[baseIndx][0];

  for(i = 0; i < HOST_MALLOC_TOT_PACKET; i++){

    if(SCRATCH_PACKET_VALIDITY[i] == 0x0){

      contSize += HOST_MALLOC_PACKET_SIZE;
      numPackets++;
    }else{

      contSize   = 0;
      numPackets = 0;
      baseIndx   = i + 1;
      baseAddr   = &SCRATCH_NON_CACHED[baseIndx][0];
    }

    if(contSize >= size)
      break;

  }

  if(i == HOST_MALLOC_TOT_PACKET){
    printf("\n Could not allocate memory for host file i/o \n");
    return(0x0);
  }


  for(i = baseIndx; i < baseIndx + numPackets; i++)
    SCRATCH_PACKET_VALIDITY[i] = 1;

  /* Indication for freeing up the packets. At the time of freeing
  * total number of packets to be freed to be known.
  */
  SCRATCH_PACKET_VALIDITY[baseIndx] = numPackets;

  return(baseAddr);
}

void host_file_io_ti_free(void* ptr){

  int32_t numPackets;
  int32_t baseIndx;
  int32_t i;

  for(i = 0; i < HOST_MALLOC_TOT_PACKET; i++){
    if(ptr == &SCRATCH_NON_CACHED[i][0])
      break;
  }

  baseIndx = i;

  if( i >= HOST_MALLOC_TOT_PACKET)
    printf("\n Error");

  numPackets = SCRATCH_PACKET_VALIDITY[baseIndx];

  for(i = baseIndx; i < baseIndx + numPackets; i++){
    SCRATCH_PACKET_VALIDITY[i] = 0x0;
  }

  if( i >= HOST_MALLOC_TOT_PACKET)
    printf("\n Error");
}
#endif

#ifndef TIADALG_OS_BUILD
/* DDR for c7x_1 for Scratch Memory [ size 720.00 MB ] */
#define DDR_C7X_1_SCRATCH_PHY_ADDR  (0x880000000u)
#define DDR_C7X_1_SCRATCH_VIRT_ADDR (0x100000000u)

#define VIRT_PHY_ADDR_OFFSET (DDR_C7X_1_SCRATCH_PHY_ADDR - DDR_C7X_1_SCRATCH_VIRT_ADDR)

uint64_t algotb_virtToPhyAddrConversion(const void *virtAddr,
                                      uint32_t chNum,
                                      void *appData)
{
  uint64_t phyAddr = (uint64_t)virtAddr;

  if ((uint64_t)virtAddr >= DDR_C7X_1_SCRATCH_VIRT_ADDR)
  {
    phyAddr = ((uint64_t)virtAddr + VIRT_PHY_ADDR_OFFSET);
  }
  return phyAddr;
}
#else
uint64_t algotb_virtToPhyAddrConversion(const void *virtAddr,
                                      uint32_t chNum,
                                      void *appData)
{
  uint64_t phyAddr = (uint64_t)virtAddr;
  return phyAddr;
}
#endif

void * algo_tb_udma_init( void)
{
#ifndef _A72_BUILD
  Udma_InitPrms initPrms;
  UdmaInitPrms_init(UDMA_INST_ID_MAIN_0, &initPrms);
  initPrms.printFxn = &ALGO_printf;
  initPrms.skipGlobalEventReg = 1;

#ifndef HOST_EMULATION
  initPrms.virtToPhyFxn = algotb_virtToPhyAddrConversion;
#endif

  Udma_init(&udmaDrvObj, &initPrms);
  return &udmaDrvObj;
#else
  return NULL;
#endif
}

void ALGO_printf(const char *str)
{
}
