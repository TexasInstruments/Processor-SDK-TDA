/*
 *  Copyright (c) Texas Instruments Incorporated 2026
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 *  \file dss_display_test.c
 *
 *  \brief DSS sample application that is part of the DSS8 IP FMA diagnostic.
 *
 *  This file contains functions that display the 1920x1080 ARGB32 buffer. It
 *  includes the file where the buffer is defined, configures the Display
 *  Subsystem (DSS) and displays the buffer onto the screen using the DSS.
 *  Since the application is used for the DSS8 safety diagnostic, which has
 *  the goal of causing hardware underflow and synclost interrupt there are 
 *  some modification of the QoS setting of the DSS, specifically the 
 *  transaction priority is set to value 7 (low priority), and the orderId 
 *  registers are also modified.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdio.h>
#include <string.h>
#include <dss_display_test.h>
#include <ti/drv/dss/examples/utils/app_utils.h>
#include <ti/drv/uart/UART.h>
#include <ti/drv/uart/UART_stdio.h>
#include <ti/csl/csl_cbass.h>


#if defined (SOC_J721S2)
#include <ti/board/src/j721s2_evm/include/board_control.h>
#include <ti/board/src/j721s2_evm/include/board_i2c_io_exp.h>
#elif defined (SOC_J784S4)
#include <ti/board/src/j784s4_evm/include/board_control.h>
#include <ti/board/src/j784s4_evm/include/board_i2c_io_exp.h>
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define DSS_SYNCLOST_BUFFER_WIDTH              (3840U)
#define DSS_SYNCLOST_BUFFER_HEIGHT             (2160U)

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */
static void DssSyncLoss_Init(DssSyncLoss_Obj *appObj);
static void DssSyncLoss_DeInit(DssSyncLoss_Obj *appObj);
static void DssSyncLoss_Create(DssSyncLoss_Obj *appObj);
static void DssSyncLoss_Delete(DssSyncLoss_Obj *appObj);
static int32_t DssSyncLoss_ConfigDctrl(DssSyncLoss_Obj *appObj);
static int32_t DssSyncLoss_RunTest(DssSyncLoss_Obj *appObj);
static void DssSyncLoss_InitParams(DssSyncLoss_Obj *appObj);
static int32_t DssSyncLoss_AllocAndQueueFrames(const DssSyncLoss_Obj *appObj,
                                           DssSyncLoss_InstObj *instObj);
static int32_t DssSyncLoss_PipeCbFxn(Fvid2_Handle handle, void *appData);
static int32_t DssSyncLoss_SetOrderId();
static void DssSyncLoss_FillFrameBuffer(void);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

DssSyncLoss_Obj gDispApp_Obj;

uint32_t gDispArray1[DSS_SYNCLOST_BUFFER_WIDTH * DSS_SYNCLOST_BUFFER_HEIGHT] __attribute__((section(".data_buffer"), aligned(128)));
uint32_t gDispArray2[DSS_SYNCLOST_BUFFER_WIDTH * DSS_SYNCLOST_BUFFER_HEIGHT] __attribute__((section(".data_buffer"), aligned(128)));

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/*
 * DSS display test
 */
int32_t DssSyncLoss_DisplayTest(void)
{
    int32_t retVal = FVID2_SOK;
    DssSyncLoss_Print("DSS display application started...\r\n");

    DssSyncLoss_FillFrameBuffer();
    DssSyncLoss_Init(&gDispApp_Obj);

    retVal = DssSyncLoss_RunTest(&gDispApp_Obj);

    DssSyncLoss_DeInit(&gDispApp_Obj);

    if(FVID2_SOK == retVal)
    {
        DssSyncLoss_Print("DSS display test Passed!!\r\n");
        DssSyncLoss_Print("All tests have passed!!\r\n");
    }
    else
    {
        DssSyncLoss_Print("DSS display test Failed!!\r\n");
    }

    return (0);
}

static void DssSyncLoss_Init(DssSyncLoss_Obj *appObj)
{
    int32_t         retVal = FVID2_SOK;
    Fvid2_InitPrms  initPrms;

    Fvid2InitPrms_init(&initPrms);
    initPrms.printFxn = &DssSyncLoss_Print;

    retVal = Fvid2_init(&initPrms);
    if(FVID2_SOK != retVal)
    {
        DssSyncLoss_Print("Fvid2 Init Failed!!!\r\n");
    }

    Dss_initParamsInit(&appObj->initParams);
    appObj->initParams.socParams.dpInitParams.isHpdSupported = UFALSE;

    Dss_init(&appObj->initParams);

    if(FVID2_SOK == retVal)
    {
        /* Create DCTRL handle, used for common driver configuration */
        appObj->dctrlHandle = Fvid2_create(
            DSS_DCTRL_DRV_ID,
            DSS_DCTRL_INST_0,
            NULL,
            NULL,
            NULL);
        if(NULL == appObj->dctrlHandle)
        {
            DssSyncLoss_Print("DCTRL Create Failed!!!\r\n");
        }
    }

    if(FVID2_SOK == retVal)
    {
         DssSyncLoss_Print("DssSyncLoss_Init() - DONE !!!\r\n");
    }

    return;
}

static void DssSyncLoss_DeInit(DssSyncLoss_Obj *appObj)
{
    int32_t  retVal = FVID2_SOK;

    /* Delete DCTRL handle */
    retVal = Fvid2_delete(appObj->dctrlHandle, NULL);
    retVal += Dss_deInit();
    retVal += Fvid2_deInit(NULL);
    if(FVID2_SOK != retVal)
    {
         DssSyncLoss_Print("DCTRL handle delete failed!!!\r\n");
    }
    else
    {
         DssSyncLoss_Print("DssSyncLoss_DeInit() - DONE !!!\r\n");
    }

    return;
}

static int32_t DssSyncLoss_RunTest(DssSyncLoss_Obj *appObj)
{
    int32_t retVal = FVID2_SOK;
    uint32_t instCnt = 0U;
    volatile uint32_t loopCount = 0U;
    DssSyncLoss_InstObj *instObj;
    Fvid2_FrameList  frmList;

    /* Create driver */
    DssSyncLoss_Create(appObj);

    DssSyncLoss_Print("Starting display ... !!!\r\n");
    DssSyncLoss_Print("Display in progress ... DO NOT HALT !!!\r\n");

    /* Start driver */
    for(instCnt = 0U; instCnt<gDispAppTestParams.numTestPipes; instCnt++)
    {
        instObj = &appObj->instObj[instCnt];

        retVal = Fvid2_start(instObj->drvHandle, NULL);
        if(FVID2_SOK != retVal)
        {
            DssSyncLoss_Print("Display Start Failed!!!\r\n");
            break;
        }
    }

    while(loopCount++ < DISP_APP_RUN_COUNT)
    {
        for(instCnt = 0U; instCnt<gDispAppTestParams.numTestPipes; instCnt++)
        {
            instObj = &appObj->instObj[instCnt];

            (void) SemaphoreP_pend(instObj->syncSem, SemaphoreP_WAIT_FOREVER);

            retVal = Fvid2_dequeue(instObj->drvHandle,
                                   &frmList,
                                   0U,
                                   FVID2_TIMEOUT_NONE);

            if(FVID2_SOK == retVal)
            {
                retVal = Fvid2_queue(instObj->drvHandle, &frmList, 0U);
                if(FVID2_SOK != retVal)
                {
                    DssSyncLoss_Print("Display Queue Failed!!!\r\n");
                    break;
                }
            }
            else if (FVID2_EAGAIN == retVal)
            {
                /* Do nothing as this is first callback */
            }
            else
            {
                /* Error */
                DssSyncLoss_Print("Display Dequeue Failed!!!\r\n");
                break;
            }

            /* Check for DSS underflow errors */
            Dss_DispCurrentStatus currStatus;

            retVal = Fvid2_control(
                instObj->drvHandle,
                IOCTL_DSS_DISP_GET_CURRENT_STATUS,
                &currStatus,
                NULL);
            if(FVID2_SOK != retVal)
            {
                DssSyncLoss_Print("Failed to get Display Stats!!!\r\n");
            }

            /* Print number of underflows happened */
            if(0U != currStatus.underflowCount)
            {
                DssSyncLoss_Print("Underflow occured!\n");
                GT_2trace(DssTrace, GT_ERR, "No of Underflows for Inst %d: %d\r\n", instCnt, currStatus.underflowCount);
            }

            if (currStatus.underflowCount > UNDERFLOW_COUNT_THRESHOLD)
            {
                DssSyncLoss_Print("Too many underflow, aborting\n");
                loopCount = DISP_APP_RUN_COUNT + 1;
            }
        }
    }

    for(instCnt = 0U; instCnt<gDispAppTestParams.numTestPipes; instCnt++)
    {
        instObj = &appObj->instObj[instCnt];
        retVal  = Fvid2_stop(instObj->drvHandle, NULL);
        if(FVID2_SOK != retVal)
        {
            DssSyncLoss_Print("Display Stop Failed!!!\r\n");
            break;
        }
    }

    if(FVID2_SOK == retVal)
    {
        /* Delete driver */
        DssSyncLoss_Delete(appObj);
    }

    return retVal;
}

static void DssSyncLoss_Create(DssSyncLoss_Obj *appObj)
{
    int32_t retVal = FVID2_SOK;
    uint32_t instCnt = 0U;
    SemaphoreP_Params semParams;
    Dss_DctrlVpParams *vpParams;
    Dss_DctrlAdvVpParams *advVpParams;
    DssSyncLoss_InstObj *instObj;
    int32_t dpConnectedCmdArg;
    DssSyncLoss_InitParams(appObj);
    vpParams = &appObj->vpParams;
    advVpParams = &appObj->advVpParams;
    Dss_dctrlVpParamsInit(vpParams);
    Dss_dctrlAdvVpParamsInit(advVpParams);

    vpParams->vpId = CSL_DSS_VP_ID_1;
    advVpParams->vpId = CSL_DSS_VP_ID_1;

    vpParams->lcdOpTimingCfg.mInfo.standard = FVID2_STD_CUSTOM;
    vpParams->lcdOpTimingCfg.mInfo.width = 1920U;
    vpParams->lcdOpTimingCfg.mInfo.height = 1080U;
    vpParams->lcdOpTimingCfg.mInfo.pixelClock = 148500U;
    vpParams->lcdOpTimingCfg.mInfo.hFrontPorch = 88U;
    vpParams->lcdOpTimingCfg.mInfo.hBackPorch = 148U;
    vpParams->lcdOpTimingCfg.mInfo.hSyncLen = 44U;
    vpParams->lcdOpTimingCfg.mInfo.vFrontPorch = 4U;
    vpParams->lcdOpTimingCfg.mInfo.vBackPorch = 36U;
    vpParams->lcdOpTimingCfg.mInfo.vSyncLen = 5U;

    vpParams->lcdOpTimingCfg.dvoFormat = FVID2_DV_GENERIC_DISCSYNC;
    vpParams->lcdOpTimingCfg.videoIfWidth = FVID2_VIFW_36BIT;
    vpParams->lcdPolarityCfg.actVidPolarity = FVID2_POL_HIGH;
    vpParams->lcdPolarityCfg.hsPolarity = FVID2_POL_HIGH;
    vpParams->lcdPolarityCfg.vsPolarity = FVID2_POL_HIGH;
    vpParams->lcdPolarityCfg.pixelClkPolarity = FVID2_EDGE_POL_RISING;

    advVpParams->lcdAdvSignalCfg.hVAlign = CSL_DSS_VP_HVSYNC_ALIGNED;
    advVpParams->lcdAdvSignalCfg.hVClkControl = CSL_DSS_VP_HVCLK_CONTROL_ON;
    advVpParams->lcdAdvSignalCfg.hVClkRiseFall = FVID2_EDGE_POL_RISING;

    retVal = Fvid2_control(appObj->dctrlHandle, IOCTL_DSS_DCTRL_IS_DP_CONNECTED, &dpConnectedCmdArg, NULL);
    if ((FVID2_SOK == retVal) && (1 == dpConnectedCmdArg))
    {
        DssSyncLoss_ConfigDctrl(appObj);
        DssSyncLoss_SetOrderId();

        for(instCnt = 0U; instCnt<gDispAppTestParams.numTestPipes; instCnt++)
        {
            instObj = &appObj->instObj[instCnt];
            SemaphoreP_Params_init(&semParams);
            semParams.mode = SemaphoreP_Mode_BINARY;
            instObj->syncSem = SemaphoreP_create(0U, &semParams);
            instObj->drvHandle = Fvid2_create(
                DSS_DISP_DRV_ID,
                instObj->instId,
                &instObj->createParams,
                &instObj->createStatus,
                &instObj->cbParams);
            if((NULL == instObj->drvHandle) ||
            (FVID2_SOK != instObj->createStatus.retVal))
            {
                DssSyncLoss_Print("Display Create Failed!!!\r\n");
                retVal = instObj->createStatus.retVal;
            }

            if(FVID2_SOK == retVal)
            {
                retVal = Fvid2_control(
                    instObj->drvHandle,
                    IOCTL_DSS_DISP_SET_DSS_PARAMS,
                    &instObj->dispParams,
                    NULL);
                if(FVID2_SOK != retVal)
                {
                    DssSyncLoss_Print("DSS Set Params IOCTL Failed!!!\r\n");
                }
            }
            if(FVID2_SOK == retVal)
            {
                retVal = Fvid2_control(
                    instObj->drvHandle,
                    IOCTL_DSS_DISP_SET_PIPE_MFLAG_PARAMS,
                    &instObj->mflagParams,
                    NULL);
                if(FVID2_SOK != retVal)
                {
                    DssSyncLoss_Print("DSS Set Mflag Params IOCTL Failed!!!\r\n");
                }
            }

            if(FVID2_SOK == retVal)
            {
                retVal = DssSyncLoss_AllocAndQueueFrames(appObj, instObj);
                if(FVID2_SOK != retVal)
                {
                    DssSyncLoss_Print("Display Alloc and Queue Failed!!!\r\n");
                }
            }

            if(FVID2_SOK != retVal)
            {
                break;
            }
        }

        if(FVID2_SOK == retVal)
        {
            DssSyncLoss_Print("Display create complete!!\r\n");
        }     
    }
    else
    {
        printf("The display cable is not connected!!\n");
    }
    return;
}

static void DssSyncLoss_Delete(DssSyncLoss_Obj *appObj)
{
    int32_t retVal;
    uint32_t instCnt;
    Dss_DctrlVpParams *vpParams;
    Dss_DctrlPathInfo *pathInfo;
    Dss_DctrlVpErrorStats *pErrorStats;
    DssSyncLoss_InstObj *instObj;
    Dss_DispCurrentStatus currStatus;
    Fvid2_FrameList frmList;

    vpParams = &appObj->vpParams;
    pathInfo = &appObj->dctrlPathInfo;
    pErrorStats = &appObj->errorStats;

    for(instCnt = 0U; instCnt<gDispAppTestParams.numTestPipes; instCnt++)
    {
        instObj = &appObj->instObj[instCnt];

        /* Check for DSS underflow errors */
        retVal = Fvid2_control(
            instObj->drvHandle,
            IOCTL_DSS_DISP_GET_CURRENT_STATUS,
            &currStatus,
            NULL);
        if(FVID2_SOK != retVal)
        {
            DssSyncLoss_Print("Failed to get Display Stats!!!\r\n");
        }

        /* Print Synclost errors */
        if(0U != currStatus.underflowCount)
        {
            DssSyncLoss_Print("Underflow occured during the executon!\n");
            GT_2trace(DssTrace, GT_ERR, "No of Underflows for Inst %d: %d\r\n", instCnt, currStatus.underflowCount);
        }
        else
        {
            DssSyncLoss_Print("Underflow did not occur\r\n");
        }

        /* Dequeue all the request from the driver */
        while (BTRUE)
        {
            retVal = Fvid2_dequeue(
                instObj->drvHandle,
                &frmList,
                0U,
                FVID2_TIMEOUT_NONE);
            if(FVID2_SOK != retVal)
            {
                break;
            }
        }

        retVal = Fvid2_delete(instObj->drvHandle, NULL);
        if(FVID2_SOK != retVal)
        {
            DssSyncLoss_Print("Display Delete Failed!!!\r\n");
            break;
        }
    }

    /* Check for DSS synclost errors */
    retVal = Fvid2_control(
        appObj->dctrlHandle,
        IOCTL_DSS_DCTRL_GET_VP_ERROR_STATS,
        pErrorStats,
        NULL);
    if(FVID2_SOK != retVal)
    {
        DssSyncLoss_Print("Failed to get VP Stats!!!\r\n");
    }

    /* Print Synclost errors */
    if(0U != pErrorStats->syncLost)
    {
        DssSyncLoss_Print("Synclost occured during the execution!\n");
        GT_1trace(DssTrace, GT_ERR, "No of Sync Lost: %d\r\n", pErrorStats->syncLost);
    }
    else
    {
        DssSyncLoss_Print("Sync Lost did not occur\r\n");
    }

    retVal = Fvid2_control(
        appObj->dctrlHandle,
        IOCTL_DSS_DCTRL_CLEAR_PATH,
        pathInfo,
        NULL);
    if(FVID2_SOK != retVal)
    {
        DssSyncLoss_Print("Clear Path Failed!!!\r\n");
    }

    retVal = Fvid2_control(
        appObj->dctrlHandle,
        IOCTL_DSS_DCTRL_STOP_VP,
        vpParams,
        NULL);
    if(FVID2_SOK != retVal)
    {
        DssSyncLoss_Print("VP Stop Failed!!!\r\n");
    }

    if(FVID2_SOK == retVal)
    {
         DssSyncLoss_Print("Display delete complete!!\r\n");
    }

    return;
}

static int32_t DssSyncLoss_AllocAndQueueFrames(const DssSyncLoss_Obj *appObj,
                                           DssSyncLoss_InstObj *instObj)
{
    int32_t  retVal = FVID2_SOK;
    uint32_t frmId, numFrames;
    Fvid2_Frame *frm;
    Fvid2_FrameList frmList;

    Fvid2FrameList_init(&frmList);
    frm = &instObj->frames[0U];

    numFrames = DISP_APP_MAX_FRAMES_PER_HANDLE;
    /* init memory pointer for 'numFrames'  */
    for(frmId = 0U; frmId<numFrames; frmId++)
    {
        /* init Fvid2_Frame to 0's  */
        Fvid2Frame_init(&frm[frmId]);        

        if(instObj->instId == gDispAppTestParams.instId[0U])
        {
            if(0U == frmId)
            {
                frm[frmId].addr[0U] = (uint64_t)gDispArray1;
            }
            else
            {
                frm[frmId].addr[0U] = (uint64_t)gDispArray2;
            }
        }
        else
        {
            if(0U == frmId)
            {
                frm[frmId].addr[0U] = (uint64_t)gDispArray1;
            }
            else
            {
                frm[frmId].addr[0U] = (uint64_t)gDispArray2;
            }
        }

        frm[frmId].fid = FVID2_FID_FRAME;
        frm[frmId].appData = instObj;

        /* Set number of frame in frame list - one at a time */
        frmList.numFrames  = 1U;
        frmList.frames[0U] = &frm[frmId];

        /*
         * queue the frames in frmList
         * All allocate frames are queued here as an example.
         * In general atleast 2 frames per channel need to queued
         * before starting display,
         * else frame will get dropped until frames are queued
         */
        retVal = Fvid2_queue(instObj->drvHandle, &frmList, 0U);
        if(FVID2_SOK != retVal)
        {
            DssSyncLoss_Print("Display Queue Failed!!!\r\n");
            break;
        }
    }

    return (retVal);
}

static void DssSyncLoss_InitParams(DssSyncLoss_Obj *appObj)
{
    uint32_t instCnt = 0U, numPipes = 0U, i;
    Dss_DispParams *dispParams;
    DssSyncLoss_InstObj *instObj;

    numPipes = gDispAppTestParams.numTestPipes;

    for(instCnt = 0U; instCnt < numPipes; instCnt++)
    {
        /* Initialize video pipes */
        instObj = &appObj->instObj[instCnt];
        instObj->instId = gDispAppTestParams.instId[instCnt];
        Dss_dispCreateParamsInit(&instObj->createParams);
        Fvid2CbParams_init(&instObj->cbParams);
        instObj->cbParams.cbFxn = &DssSyncLoss_PipeCbFxn;
        instObj->cbParams.appData = instObj;

        dispParams = &instObj->dispParams;
        Dss_dispParamsInit(dispParams);
        dispParams->pipeCfg.pipeType = gDispAppTestParams.pipeType[instCnt];
        dispParams->pipeCfg.inFmt.width = gDispAppTestParams.inWidth[instCnt];
        dispParams->pipeCfg.inFmt.height = gDispAppTestParams.inHeight[instCnt];
        for(i = 0U; i < FVID2_MAX_PLANES; i++)
        {
            dispParams->pipeCfg.inFmt.pitch[i] =
                                        gDispAppTestParams.pitch[instCnt][i];
        }
        dispParams->pipeCfg.inFmt.dataFormat =
                                        gDispAppTestParams.inDataFmt[instCnt];
        dispParams->pipeCfg.inFmt.scanFormat =
                                        gDispAppTestParams.inScanFmt[instCnt];
        dispParams->pipeCfg.outWidth = gDispAppTestParams.outWidth[instCnt];
        dispParams->pipeCfg.outHeight = gDispAppTestParams.outHeight[instCnt];
        dispParams->pipeCfg.scEnable = gDispAppTestParams.scEnable[instCnt];
        dispParams->alphaCfg.globalAlpha =
                                gDispAppTestParams.globalAlpha[instCnt];
        dispParams->alphaCfg.preMultiplyAlpha =
                                gDispAppTestParams.preMultiplyAlpha[instCnt];
        dispParams->layerPos.startX = gDispAppTestParams.posx[instCnt];
        dispParams->layerPos.startY = gDispAppTestParams.posy[instCnt];

        Dss_dispPipeMflagParamsInit(&instObj->mflagParams);
    }
}

static int32_t DssSyncLoss_ConfigDctrl(DssSyncLoss_Obj *appObj)
{
    int32_t retVal = FVID2_SOK;
    uint32_t i = 0U, j = 0U;
    Dss_DctrlVpParams *vpParams;
    Dss_DctrlOverlayParams *overlayParams;
    Dss_DctrlOverlayLayerParams *layerParams;
    Dss_DctrlPathInfo *pathInfo;
    Dss_DctrlAdvVpParams *advVpParams;
    Dss_DctrlGlobalDssParams *globalDssParams;
    vpParams = &appObj->vpParams;
    overlayParams = &appObj->overlayParams;
    layerParams = &appObj->layerParams;
    pathInfo = &appObj->dctrlPathInfo;
    advVpParams = &appObj->advVpParams;
    globalDssParams= &appObj->globalDssParams;

    Dss_dctrlOverlayParamsInit(overlayParams);
    Dss_dctrlOverlayLayerParamsInit(layerParams);
    Dss_dctrlPathInfoInit(pathInfo);
    Dss_dctrlGlobalDssParamsInit(globalDssParams);

    /* QoS priority settings for DSS */
    /* This will lower the priority of the transactions between DSS and CBA */
    globalDssParams->cbaCfg.priHigh = 0x7U;
    globalDssParams->cbaCfg.priLow = 0x7U;

    pathInfo->edgeInfo[pathInfo->numEdges].startNode = gDispAppTestParams.pipeNodeId[0U];
    pathInfo->edgeInfo[pathInfo->numEdges].endNode = DSS_DCTRL_NODE_OVERLAY1;
    pathInfo->numEdges++;
    pathInfo->edgeInfo[pathInfo->numEdges].startNode = DSS_DCTRL_NODE_OVERLAY1;
    pathInfo->edgeInfo[pathInfo->numEdges].endNode = DSS_DCTRL_NODE_VP1;
    pathInfo->numEdges++;
    pathInfo->edgeInfo[pathInfo->numEdges].startNode = DSS_DCTRL_NODE_VP1;
    pathInfo->edgeInfo[pathInfo->numEdges].endNode = DSS_DCTRL_NODE_EDP_DPI0;
    pathInfo->numEdges++;
    if(1U < gDispAppTestParams.numTestPipes)
    {
        for(i = 1U; i < gDispAppTestParams.numTestPipes; i++)
        {
            pathInfo->edgeInfo[pathInfo->numEdges].startNode =
                                            gDispAppTestParams.pipeNodeId[i];
            pathInfo->edgeInfo[pathInfo->numEdges].endNode =
                                            DSS_DCTRL_NODE_OVERLAY1;
            pathInfo->numEdges++;
        }
    }

    retVal = Fvid2_control(
        appObj->dctrlHandle,
        IOCTL_DSS_DCTRL_SET_PATH,
        pathInfo,
        NULL);
    if(FVID2_SOK != retVal)
    {
        DssSyncLoss_Print("Dctrl Set Path IOCTL Failed!!!\r\n");
    }

    retVal = Fvid2_control(
        appObj->dctrlHandle,
        IOCTL_DSS_DCTRL_SET_VP_PARAMS,
        vpParams,
        NULL);
    if(FVID2_SOK != retVal)
    {
        DssSyncLoss_Print("Dctrl Set VP Params IOCTL Failed!!!\r\n");
    }

    retVal = Fvid2_control(
        appObj->dctrlHandle,
        IOCTL_DSS_DCTRL_SET_ADV_VP_PARAMS,
        advVpParams,
        NULL);
    if(FVID2_SOK != retVal)
    {
        DssSyncLoss_Print("DCTRL Set Advance VP Params IOCTL Failed!!!\r\n");
    }

    overlayParams->overlayId = CSL_DSS_OVERLAY_ID_1;
    overlayParams->colorbarEnable = UFALSE;
    overlayParams->overlayCfg.colorKeyEnable = UTRUE;
    overlayParams->overlayCfg.colorKeySel = CSL_DSS_OVERLAY_TRANS_COLOR_DEST;
    overlayParams->overlayCfg.backGroundColor = 0xC8C800U;

    retVal = Fvid2_control(
        appObj->dctrlHandle,
        IOCTL_DSS_DCTRL_SET_OVERLAY_PARAMS,
        overlayParams,
        NULL);
    if(FVID2_SOK != retVal)
    {
        DssSyncLoss_Print("DCTRL Set Overlay Params IOCTL Failed!!!\r\n");
    }

    layerParams->overlayId = CSL_DSS_OVERLAY_ID_1;
    layerParams->pipeLayerNum[gDispAppTestParams.pipeId[0U]] =
                                                CSL_DSS_OVERLAY_LAYER_NUM_0;
    if(gDispAppTestParams.numTestPipes > 1U)
    {
        for(i = 1U; i<gDispAppTestParams.numTestPipes;i++)
        {
            layerParams->pipeLayerNum[gDispAppTestParams.pipeId[i]] = i;
        }
    }

    if(CSL_DSS_VID_PIPE_ID_MAX > gDispAppTestParams.numTestPipes)
    {
        for(i = gDispAppTestParams.numTestPipes; i < CSL_DSS_VID_PIPE_ID_MAX; i++)
        {
            layerParams->pipeLayerNum[gDispAppTestParams.invalidPipeId[j++]] =
                                                CSL_DSS_OVERLAY_LAYER_INVALID;
        }
    }

    retVal = Fvid2_control(
        appObj->dctrlHandle,
        IOCTL_DSS_DCTRL_SET_LAYER_PARAMS,
        layerParams,
        NULL);
    if(FVID2_SOK != retVal)
    {
        DssSyncLoss_Print("DCTRL Set Layer Params IOCTL Failed!!!\r\n");
    }

    retVal = Fvid2_control(
        appObj->dctrlHandle,
        IOCTL_DSS_DCTRL_SET_GLOBAL_DSS_PARAMS,
        globalDssParams,
        NULL);
    if(FVID2_SOK != retVal)
    {
        DssSyncLoss_Print("DCTRL Set Global DSS Params IOCTL Failed!!!\r\n");
    }

    return (retVal);
}

static int32_t DssSyncLoss_SetOrderId()
{
    uint32_t orderId = 2U;  
    uint32_t ch;
    int32_t  status;

    /* Get the base addresses of DSS orderId registers */
    CSL_cbass_qosRegs_ep *dmaQos  = (CSL_cbass_qosRegs_ep *) CSL_QOS_DSS0_DSS_INST0_VBUSM_DMA_MMR_BASE;
    CSL_cbass_qosRegs_ep *fbdcQos = (CSL_cbass_qosRegs_ep *) CSL_QOS_DSS0_DSS_INST0_VBUSM_FBDC_MMR_BASE;

    /* Set ORDERID for all DMA channels */
    for (ch = 0U; ch < CSL_QOS_DSS0_DSS_INST0_VBUSM_DMA_CHANNEL_COUNT; ch++)
    {
        status = CSL_cbassQosSetOrderID(dmaQos, ch, orderId);
        /* optional: handle/LOG status if needed */
    }

    /* Set ORDERID for all FBDC channels */
    for (ch = 0U; ch < CSL_QOS_DSS0_DSS_INST0_VBUSM_FBDC_CHANNEL_COUNT; ch++)
    {
        status = CSL_cbassQosSetOrderID(fbdcQos, ch, orderId);
    }

    return status;
}

static void DssSyncLoss_FillFrameBuffer(void)
{
    uint32_t x, y;    

    /* BUFFER 1 - Vertical Color Bars */
    for(y = 0U; y < DSS_SYNCLOST_BUFFER_HEIGHT; y++)
    {
        for(x = 0U; x < DSS_SYNCLOST_BUFFER_WIDTH; x++)
        {
            uint32_t bar = x / (DSS_SYNCLOST_BUFFER_WIDTH / 8U);
            uint32_t color;

            switch(bar)
            {
                case 0U: color = 0xFFFFFFFFU; break; /* White   */
                case 1U: color = 0xFFFFFF00U; break; /* Yellow  */
                case 2U: color = 0xFF00FFFFU; break; /* Cyan    */
                case 3U: color = 0xFF00FF00U; break; /* Green   */
                case 4U: color = 0xFFFF00FFU; break; /* Magenta */
                case 5U: color = 0xFFFF0000U; break; /* Red     */
                case 6U: color = 0xFF0000FFU; break; /* Blue    */
                default: color = 0xFF000000U; break; /* Black   */
            }

            gDispArray1[(y * DSS_SYNCLOST_BUFFER_WIDTH) + x] = color;
        }
    }

    /* BUFFER 2 - Horizontal Black/White Stripes */
    for(y = 0U; y < DSS_SYNCLOST_BUFFER_HEIGHT; y++)
    {
        uint32_t color;

        if(((y / 64U) % 2U) == 0U)
        {
            color = 0xFFFFFFFFU; /* White */
        }
        else
        {
            color = 0xFF000000U; /* Black */
        }

        for(x = 0U; x < DSS_SYNCLOST_BUFFER_WIDTH; x++)
        {
            gDispArray2[(y * DSS_SYNCLOST_BUFFER_WIDTH) + x] = color;
        }
    }

    CacheP_wb(gDispArray1, DSS_SYNCLOST_BUFFER_WIDTH * DSS_SYNCLOST_BUFFER_HEIGHT * 4U);
    CacheP_wb(gDispArray2, DSS_SYNCLOST_BUFFER_WIDTH * DSS_SYNCLOST_BUFFER_HEIGHT * 4U);
}

static int32_t DssSyncLoss_PipeCbFxn(Fvid2_Handle handle, void *appData)
{
    int32_t retVal  = FVID2_SOK;
    DssSyncLoss_InstObj *instObj = (DssSyncLoss_InstObj *) appData;
    GT_assert (DssTrace, (NULL != instObj));

    (void) SemaphoreP_post(instObj->syncSem);

    return (retVal);
}

void DssSyncLoss_Print(const char *format, ...)
{
    char printBuffer[DISP_APP_PRINT_BUFFER_SIZE];
    va_list arguments;

    /* Start the var args processing. */
    va_start(arguments, format);
    vsnprintf (printBuffer, sizeof(printBuffer), format, arguments);
    UART_printf("%s", printBuffer);
    /* End the var args processing. */
    va_end(arguments);
}

