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
 *  \brief Software Refresh of Lookup Table Memories Every Frame
 *
 *  This example implements a DSS display use-case where a video pipeline is configured
 *  and frames are continuously streamed to a display. During runtime, the CLUT
 *  (Color Lookup Table) is refreshed on every frame to emulate software-driven
 *  register updates and to help detect transient memory faults (e.g. bit flips).
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdio.h>
#include <string.h>
#include <dss_display_test.h>
#include <dss_display_buffer1.h>
#include <dss_display_buffer2.h>
#include <ti/drv/dss/examples/utils/app_utils.h>
#include <ti/drv/uart/UART.h>
#include <ti/drv/uart/UART_stdio.h>

#if defined (SOC_J721S2)
#include <ti/board/src/j721s2_evm/include/board_control.h>
#include <ti/board/src/j721s2_evm/include/board_i2c_io_exp.h>
#include <ti/csl/soc/j721s2/src/cslr_soc_baseaddress.h>
#elif defined (SOC_J784S4)
#include <ti/board/src/j784s4_evm/include/board_control.h>
#include <ti/board/src/j784s4_evm/include/board_i2c_io_exp.h>
#include <ti/csl/soc/j784s4/src/cslr_soc_baseaddress.h>
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define TEST_VP_ID                      (CSL_DSS_VP_ID_1)
#define TEST_OVERLAY_ID                 (CSL_DSS_OVERLAY_ID_1)
#define TEST_DCTRL_OVERLAY_NODE_ID      (DSS_DCTRL_NODE_OVERLAY1)
#define TEST_DCTRL_VP_NODE_ID           (DSS_DCTRL_NODE_VP1)
#define TEST_DCTRL_OUT_NODE_ID          (DSS_DCTRL_NODE_EDP_DPI0)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/** 
 * \brief Initializes DSS driver, FVID2, and application objects.
 *
 * \param appObj Pointer to application object.
 *
 * \return None.
 */
static void DispApp_init(DispApp_Obj *appObj);

/** 
 * \brief De-initializes DSS driver and releases resources.
 *
 * \param appObj Pointer to application object.
 *
 * \return None.
 */
static void DispApp_deInit(DispApp_Obj *appObj);

/** 
 * \brief Creates display driver instances and configures pipelines.
 *
 * \param appObj Pointer to application object.
 *
 * \return None.
 */
static void DispApp_create(DispApp_Obj *appObj);

/** 
 * \brief Deletes display instances and checks error statistics.
 *
 * \param appObj Pointer to application object.
 *
 * \return None.
 */
static void DispApp_delete(DispApp_Obj *appObj);

/** 
 * \brief Configures DSS display controller (DCTRL) pipeline and routing.
 *
 * \param appObj Pointer to application object.
 *
 * \return FVID2_SOK on success, error code otherwise.
 */
static int32_t DispApp_configDctrl(DispApp_Obj *appObj);

/** 
 * \brief Runs the display test loop with per-frame CLUT refresh.
 *
 * \param appObj Pointer to application object.
 *
 * \return FVID2_SOK on success, error code otherwise.
 */
static int32_t DispApp_runTest(DispApp_Obj *appObj);

/** 
 * \brief Initializes display and pipe configuration parameters.
 *
 * \param appObj Pointer to application object.
 *
 * \return None.
 */
static void DispApp_initParams(DispApp_Obj *appObj);

/** 
 * \brief Allocates frame buffers and queues them to the driver.
 *
 * \param appObj Pointer to application object.
 * \param instObj Pointer to instance object.
 *
 * \return FVID2_SOK on success, error code otherwise.
 */
static int32_t DispApp_allocAndQueueFrames(const DispApp_Obj *appObj,
                                           DispApp_InstObj *instObj);

/** 
 * \brief Callback function triggered on frame completion.
 *
 * \param handle Driver handle.
 * \param appData Pointer to application data.
 *
 * \return FVID2_SOK.
 */
static int32_t DispApp_pipeCbFxn(Fvid2_Handle handle, void *appData);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

DispApp_Obj gDispApp_Obj;
uint32_t gTestStopTime, gTestStartTime;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/*
 * DSS display test
 */
int32_t Dss_displayTest(void)
{
    int32_t retVal = FVID2_SOK;

    DispApp_init(&gDispApp_Obj);

    UART_printf("DSS display application started...\r\n");
    UART_printf("\n");
    UART_printf("=================================================\r\n");
    UART_printf("RUN CLUT MODE\r\n");
    UART_printf("=================================================\r\n\n");
    gDispAppTestParams = &gClutConfig;
    gDispApp_Obj.lutMode = CLUT_MODE;
    retVal = DispApp_runTest(&gDispApp_Obj);

    UART_printf("Number of frames = %d, elapsed msec = %d, fps = %0.2f\n",
            DISP_APP_RUN_COUNT,
            gTestStopTime - gTestStartTime,
            (float)((float)DISP_APP_RUN_COUNT / ((gTestStopTime - gTestStartTime)/1000.0)));

    UART_printf("DSS display application started...\r\n");
    UART_printf("\n");
    UART_printf("=================================================\r\n");
    UART_printf("RUN GAMMA MODE\r\n");
    UART_printf("=================================================\r\n\n");
    gDispAppTestParams = &gGammaConfig;
    gDispApp_Obj.lutMode = GAMMA_MODE;
    retVal = DispApp_runTest(&gDispApp_Obj);

    UART_printf("Number of frames = %d, elapsed msec = %d, fps = %0.2f\n",
            DISP_APP_RUN_COUNT,
            gTestStopTime - gTestStartTime,
            (float)((float)DISP_APP_RUN_COUNT / ((gTestStopTime - gTestStartTime)/1000.0)));

    DispApp_deInit(&gDispApp_Obj);

    if(FVID2_SOK == retVal)
    {
        UART_printf("All tests have passed!!\n");
    }
    else
    {
        UART_printf("Some tests have failed!!\n");
    }

    return (0);
}

static void DispApp_init(DispApp_Obj *appObj)
{
    int32_t         retVal = FVID2_SOK;
    Fvid2_InitPrms  initPrms;

    Fvid2InitPrms_init(&initPrms);
    initPrms.printFxn = &App_print;
    retVal = Fvid2_init(&initPrms);
    if(FVID2_SOK != retVal)
    {
        UART_printf("Fvid2 Init Failed!!!\r\n");
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
            UART_printf("DCTRL Create Failed!!!\r\n");
        }
    }

    if(FVID2_SOK == retVal)
    {
         UART_printf("DispApp_init() - DONE !!!\r\n");
    }

    return;
}

static void DispApp_deInit(DispApp_Obj *appObj)
{
    int32_t  retVal = FVID2_SOK;

    /* Delete DCTRL handle */
    retVal = Fvid2_delete(appObj->dctrlHandle, NULL);
    retVal += Dss_deInit();
    retVal += Fvid2_deInit(NULL);
    if(FVID2_SOK != retVal)
    {
         UART_printf("DCTRL handle delete failed!!!\r\n");
    }
    else
    {
         UART_printf("DispApp_deInit() - DONE !!!\r\n");
    }

    return;
}

static int32_t DispApp_runTest(DispApp_Obj *appObj)
{
    int32_t retVal = FVID2_SOK;
    uint32_t instCnt = 0U;
    volatile uint32_t loopCount = 0U;
    DispApp_InstObj *instObj;
    Fvid2_FrameList  frmList;

    /* Create driver */
    DispApp_create(appObj);

    UART_printf("\n");
    UART_printf("=================================================\r\n");
    UART_printf("DSS-F1 LUT Refresh: Enabled (every frame)\r\n");
    UART_printf("=================================================\r\n\n");

    UART_printf("Starting display ... !!!\r\n");
    UART_printf("Display in progress ... DO NOT HALT !!!\r\n");

    /* Start driver */
    for(instCnt = 0U; instCnt<gDispAppTestParams->numTestPipes; instCnt++)
    {
        instObj = &appObj->instObj[instCnt];

        retVal = Fvid2_start(instObj->drvHandle, NULL);
        if(FVID2_SOK != retVal)
        {
            UART_printf("Display Start Failed!!!\r\n");
            break;
        }
    }

    gTestStartTime = (TimerP_getTimeInUsecs() / 1000U);

    while(loopCount++ < DISP_APP_RUN_COUNT)
    {
        for(instCnt = 0U; instCnt<gDispAppTestParams->numTestPipes; instCnt++)
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
                    UART_printf("Display Queue Failed!!!\r\n");
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
                UART_printf("Display Dequeue Failed!!!\r\n");
                break;
            }
        }
    }   

    for(instCnt = 0U; instCnt<gDispAppTestParams->numTestPipes; instCnt++)
    {
        instObj = &appObj->instObj[instCnt];
        retVal  = Fvid2_stop(instObj->drvHandle, NULL);
        if(FVID2_SOK != retVal)
        {
            UART_printf("Display Stop Failed!!!\r\n");
            break;
        }
    }

    gTestStopTime = (TimerP_getTimeInUsecs() / 1000U);

    if(FVID2_SOK == retVal)
    {
        /* Delete driver */
        DispApp_delete(appObj);
    }

    return retVal;
}

static void DispApp_create(DispApp_Obj *appObj)
{
    int32_t retVal = FVID2_SOK;
    uint32_t instCnt = 0U;
    SemaphoreP_Params semParams;
    Dss_DctrlVpParams *vpParams;
    Dss_DctrlAdvVpParams *advVpParams;
    DispApp_InstObj *instObj;
    int32_t dpConnectedCmdArg;

    DispApp_initParams(appObj);
    vpParams = &appObj->vpParams;
    advVpParams = &appObj->advVpParams;
    Dss_dctrlVpParamsInit(vpParams);
    Dss_dctrlAdvVpParamsInit(advVpParams);

    vpParams->vpId = TEST_VP_ID;
    advVpParams->vpId = TEST_VP_ID;

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

        DispApp_configDctrl(appObj);
        for(instCnt = 0U; instCnt<gDispAppTestParams->numTestPipes; instCnt++)
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
                UART_printf("Display Create Failed!!!\r\n");
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
                    UART_printf("DSS Set Params IOCTL Failed!!!\r\n");
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
                    UART_printf("DSS Set Mflag Params IOCTL Failed!!!\r\n");
                }
            }

            if(FVID2_SOK == retVal)
            {
                retVal = DispApp_allocAndQueueFrames(appObj, instObj);
                if(FVID2_SOK != retVal)
                {
                    UART_printf("Display Alloc and Queue Failed!!!\r\n");
                }
            }

            if(FVID2_SOK != retVal)
            {
                break;
            }
        }

        if(FVID2_SOK == retVal)
        {
            UART_printf("Display create complete!!\r\n");
        }
    }
    else
    {
        UART_printf("The display cable is not connected!!\n");
    }
    return;
}

static void DispApp_delete(DispApp_Obj *appObj)
{
    int32_t retVal;
    uint32_t instCnt;
    Dss_DctrlVpParams *vpParams;
    Dss_DctrlPathInfo *pathInfo;
    Dss_DctrlVpErrorStats *pErrorStats;
    DispApp_InstObj *instObj;
    Dss_DispCurrentStatus currStatus;
    Fvid2_FrameList frmList;

    vpParams = &appObj->vpParams;
    pathInfo = &appObj->dctrlPathInfo;
    pErrorStats = &appObj->errorStats;

    for(instCnt = 0U; instCnt<gDispAppTestParams->numTestPipes; instCnt++)
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
            UART_printf("Failed to get Display Stats!!!\r\n");
        }

        /* Print Synclost errors */
        if(0U != currStatus.underflowCount)
        {
            GT_2trace(DssTrace, GT_ERR, "No of Underflows for Inst %d: %d\r\n", instCnt, currStatus.underflowCount);
        }
        else
        {
            UART_printf("Underflow did not occur\r\n");
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
            UART_printf("Display Delete Failed!!!\r\n");
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
        UART_printf("Failed to get VP Stats!!!\r\n");
    }

    /* Print Synclost errors */
    if(0U != pErrorStats->syncLost)
    {
        GT_1trace(DssTrace, GT_ERR, "No of Sync Lost: %d\r\n", pErrorStats->syncLost);
    }
    else
    {
        UART_printf("Sync Lost did not occur\r\n");
    }

    retVal = Fvid2_control(
        appObj->dctrlHandle,
        IOCTL_DSS_DCTRL_CLEAR_PATH,
        pathInfo,
        NULL);
    if(FVID2_SOK != retVal)
    {
        UART_printf("Clear Path Failed!!!\r\n");
    }

    retVal = Fvid2_control(
        appObj->dctrlHandle,
        IOCTL_DSS_DCTRL_STOP_VP,
        vpParams,
        NULL);
    if(FVID2_SOK != retVal)
    {
        UART_printf("VP Stop Failed!!!\r\n");
    }

    if(FVID2_SOK == retVal)
    {
         UART_printf("Display delete complete!!\r\n");
    }

    return;
}

static int32_t DispApp_allocAndQueueFrames(const DispApp_Obj *appObj,
                                           DispApp_InstObj *instObj)
{
    int32_t  retVal = FVID2_SOK;
    uint32_t frmId, numFrames;
    Fvid2_Frame *frm;
    Fvid2_FrameList frmList;

    UART_printf("DispApp_allocAndQueueFrames: Starting...\r\n");

    Fvid2FrameList_init(&frmList);
    frm = &instObj->frames[0U];
    numFrames = DISP_APP_MAX_FRAMES_PER_HANDLE;

    UART_printf("DispApp_allocAndQueueFrames: numFrames=%u\r\n", numFrames);

    /* init memory pointer for 'numFrames'  */
    for(frmId = 0U; frmId<numFrames; frmId++)
    {
        /* init Fvid2_Frame to 0's  */
        Fvid2Frame_init(&frm[frmId]);

        if(instObj->instId == gDispAppTestParams->instId[0U])
        {
            if (CLUT_MODE == appObj->lutMode)
            {
                frm[frmId].addr[0U] = (uint64_t)&gDispArray1[0];
            }
            else if (GAMMA_MODE == appObj->lutMode)
            {
                frm[frmId].addr[0U] = (uint64_t)&gDispArray2[0];
            }
        }

        frm[frmId].fid = FVID2_FID_FRAME;
        frm[frmId].appData = instObj;

        /* Set number of frame in frame list - one at a time */
        frmList.numFrames  = 1U;
        frmList.frames[0U] = &frm[frmId];

        UART_printf("Queuing frame %u, addr=0x%llx...\r\n", frmId, frm[frmId].addr[0U]);

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
            UART_printf("Display Queue Failed for frame %u!!! retVal=%d\r\n", frmId, retVal);
            break;
        }

        UART_printf("Frame %u queued successfully\r\n", frmId);
    }

    UART_printf("DispApp_allocAndQueueFrames: Done. retVal=%d\r\n", retVal);
    return (retVal);
}

static void DispApp_initParams(DispApp_Obj *appObj)
{
    uint32_t instCnt = 0U, numPipes = 0U, i;
    Dss_DispParams *dispParams;
    DispApp_InstObj *instObj;

    numPipes = gDispAppTestParams->numTestPipes;

    for(instCnt = 0U; instCnt < numPipes; instCnt++)
    {
        /* Initialize video pipes */
        instObj = &appObj->instObj[instCnt];
        instObj->instId = gDispAppTestParams->instId[instCnt];
        Dss_dispCreateParamsInit(&instObj->createParams);
        Fvid2CbParams_init(&instObj->cbParams);
        instObj->cbParams.cbFxn = &DispApp_pipeCbFxn;
        instObj->cbParams.appData = instObj;

        dispParams = &instObj->dispParams;
        Dss_dispParamsInit(dispParams);
        dispParams->pipeCfg.pipeType = gDispAppTestParams->pipeType[instCnt];
        dispParams->pipeCfg.inFmt.width = gDispAppTestParams->inWidth[instCnt];
        dispParams->pipeCfg.inFmt.height = gDispAppTestParams->inHeight[instCnt];
        for(i = 0U; i < FVID2_MAX_PLANES; i++)
        {
            dispParams->pipeCfg.inFmt.pitch[i] =
                                        gDispAppTestParams->pitch[instCnt][i];
        }
        dispParams->pipeCfg.inFmt.dataFormat =
                                        gDispAppTestParams->inDataFmt[instCnt];

        dispParams->lutRefreshEnable = gDispAppTestParams->lutRefreshEnable;

        if (CLUT_MODE == appObj->lutMode)
        {
            /* Load Color Look-up Table for the bitmap formats */
            for(uint32_t i = 0; i < CSL_DSS_NUM_LUT_ENTRIES; i++)
            {
                dispParams->pipeCfg.clutData[i] = gclutData[i];
            }
        }
        else if (GAMMA_MODE == appObj->lutMode)
        {
            dispParams->pipeCfg.gammaEnable = UTRUE;

            for(uint32_t i = 0; i < CSL_DSS_NUM_LUT_ENTRIES; i++)
            {
               dispParams->pipeCfg.clutData[i] = gClutInverseGammaData[i];
            }
        }
        else
        {
            // Do nothing
        }

        dispParams->pipeCfg.inFmt.scanFormat =
                                        gDispAppTestParams->inScanFmt[instCnt];
        dispParams->pipeCfg.outWidth = gDispAppTestParams->outWidth[instCnt];
        dispParams->pipeCfg.outHeight = gDispAppTestParams->outHeight[instCnt];
        dispParams->pipeCfg.scEnable = gDispAppTestParams->scEnable[instCnt];
        dispParams->alphaCfg.globalAlpha =
                                gDispAppTestParams->globalAlpha[instCnt];
        dispParams->alphaCfg.preMultiplyAlpha =
                                gDispAppTestParams->preMultiplyAlpha[instCnt];
        dispParams->layerPos.startX = gDispAppTestParams->posx[instCnt];
        dispParams->layerPos.startY = gDispAppTestParams->posy[instCnt];

        Dss_dispPipeMflagParamsInit(&instObj->mflagParams);
    }
}

static int32_t DispApp_configDctrl(DispApp_Obj *appObj)
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

    pathInfo->edgeInfo[pathInfo->numEdges].startNode = gDispAppTestParams->pipeNodeId[0U];
    pathInfo->edgeInfo[pathInfo->numEdges].endNode = TEST_DCTRL_OVERLAY_NODE_ID;
    pathInfo->numEdges++;
    pathInfo->edgeInfo[pathInfo->numEdges].startNode = TEST_DCTRL_OVERLAY_NODE_ID;
    pathInfo->edgeInfo[pathInfo->numEdges].endNode = TEST_DCTRL_VP_NODE_ID;
    pathInfo->numEdges++;
    pathInfo->edgeInfo[pathInfo->numEdges].startNode = TEST_DCTRL_VP_NODE_ID;
    pathInfo->edgeInfo[pathInfo->numEdges].endNode = TEST_DCTRL_OUT_NODE_ID;
    pathInfo->numEdges++;
    if(1U < gDispAppTestParams->numTestPipes)
    {
        for(i = 1U; i < gDispAppTestParams->numTestPipes; i++)
        {
            pathInfo->edgeInfo[pathInfo->numEdges].startNode =
                                            gDispAppTestParams->pipeNodeId[i];
            pathInfo->edgeInfo[pathInfo->numEdges].endNode =
                                            TEST_DCTRL_OVERLAY_NODE_ID;
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
        UART_printf("Dctrl Set Path IOCTL Failed!!!\r\n");
    }

    retVal = Fvid2_control(
        appObj->dctrlHandle,
        IOCTL_DSS_DCTRL_SET_VP_PARAMS,
        vpParams,
        NULL);
    if(FVID2_SOK != retVal)
    {
        UART_printf("Dctrl Set VP Params IOCTL Failed!!!\r\n");
    }

    retVal = Fvid2_control(
        appObj->dctrlHandle,
        IOCTL_DSS_DCTRL_SET_ADV_VP_PARAMS,
        advVpParams,
        NULL);
    if(FVID2_SOK != retVal)
    {
        UART_printf("DCTRL Set Advance VP Params IOCTL Failed!!!\r\n");
    }

    overlayParams->overlayId = TEST_OVERLAY_ID;
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
        UART_printf("DCTRL Set Overlay Params IOCTL Failed!!!\r\n");
    }

    layerParams->overlayId = TEST_OVERLAY_ID;
    layerParams->pipeLayerNum[gDispAppTestParams->pipeId[0U]] =
                                                CSL_DSS_OVERLAY_LAYER_NUM_0;
    if(gDispAppTestParams->numTestPipes > 1U)
    {
        for(i = 1U; i<gDispAppTestParams->numTestPipes;i++)
        {
            layerParams->pipeLayerNum[gDispAppTestParams->pipeId[i]] = i;
        }
    }

    if(CSL_DSS_VID_PIPE_ID_MAX > gDispAppTestParams->numTestPipes)
    {
        for(i = gDispAppTestParams->numTestPipes; i < CSL_DSS_VID_PIPE_ID_MAX; i++)
        {
            layerParams->pipeLayerNum[gDispAppTestParams->invalidPipeId[j++]] =
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
        UART_printf("DCTRL Set Layer Params IOCTL Failed!!!\r\n");
    }

    retVal = Fvid2_control(
        appObj->dctrlHandle,
        IOCTL_DSS_DCTRL_SET_GLOBAL_DSS_PARAMS,
        globalDssParams,
        NULL);
    if(FVID2_SOK != retVal)
    {
        UART_printf("DCTRL Set Global DSS Params IOCTL Failed!!!\r\n");
    }

    return (retVal);
}

static int32_t DispApp_pipeCbFxn(Fvid2_Handle handle, void *appData)
{
    int32_t retVal  = FVID2_SOK;
    DispApp_InstObj *instObj = (DispApp_InstObj *) appData;
    GT_assert (DssTrace, (NULL != instObj));
    (void) SemaphoreP_post(instObj->syncSem);

    return (retVal);
}

void App_print(const char *format, ...)
{
    char printBuffer[DISP_APP_PRINT_BUFFER_SIZE];
    va_list arguments;

    /* Start the var args processing. */
    va_start(arguments, format);
    vsnprintf (printBuffer, sizeof(printBuffer), format, arguments);
    DSS_log("%s", printBuffer);
    /* End the var args processing. */
    va_end(arguments);
}
