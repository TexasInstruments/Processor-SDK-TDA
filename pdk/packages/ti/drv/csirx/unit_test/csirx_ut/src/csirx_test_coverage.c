/*
 *  Copyright (c) Texas Instruments Incorporated 2026
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
 *  \file csirx_test_coverage.c
 *
 *  \brief CSIRX driver coverage improvement tests.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <csirx_test.h>
#include <csirx_drvPriv.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/** \brief Test result pass */
#define CSIRX_COV_TEST_PASS                     ((int32_t)FVID2_SOK)
/** \brief Test result fail */
#define CSIRX_COV_TEST_FAIL                     ((int32_t)FVID2_EFAIL)

/** \brief Frame buffer size for coverage tests */
#define CSIRX_COV_FRAME_WIDTH                   ((uint32_t)640U)
#define CSIRX_COV_FRAME_HEIGHT                  ((uint32_t)480U)
#define CSIRX_COV_FRAME_BPP                     ((uint32_t)2U)
#define CSIRX_COV_FRAME_SIZE                    (CSIRX_COV_FRAME_WIDTH * \
                                                 CSIRX_COV_FRAME_HEIGHT * \
                                                 CSIRX_COV_FRAME_BPP)

/** \brief Maximum frames for coverage tests */
#define CSIRX_COV_NUM_FRAMES                    ((uint32_t)4U)

/** \brief Invalid instance ID for negative testing */
#define CSIRX_COV_INVALID_INST_ID               ((uint32_t)0xFFFFU)

/** \brief Invalid IOCTL command for negative testing */
#define CSIRX_COV_INVALID_IOCTL_CMD             ((uint32_t)0xDEADU)

/** \brief Invalid data type for boundary testing */
#define CSIRX_COV_INVALID_DATA_TYPE             ((uint32_t)0xFFU)

/** \brief Invalid channel ID */
#define CSIRX_COV_INVALID_CH_ID                 ((uint32_t)0xFFFFU)

/** \brief DPHY lane 2 ISO ready register offset (LDD PHY block) */
#define CSIRX_COV_DPHYRX_ISO_DL_CTRL_L2        ((uint32_t)0xC30U)

/** \brief Timeout iterations for polling a DPHY lane ready bit */
#define CSIRX_COV_DPHY_LANE_POLL_TIMEOUT        ((uint32_t)100000U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static int32_t CsirxCov_testDphyCfgErrorPath(CsirxTestTaskObj *taskObj);
static int32_t CsirxCov_testNullPointerArgs(CsirxTestTaskObj *taskObj);
static int32_t CsirxCov_testInvalidCmd(CsirxTestTaskObj *taskObj);
static int32_t CsirxCov_testCompoundConds(CsirxTestTaskObj *taskObj);
static int32_t CsirxCov_testDmaCfgEdgeCases(CsirxTestTaskObj *taskObj);
static int32_t CsirxCov_testUdmaErrorMap(CsirxTestTaskObj *taskObj);
static int32_t CsirxCov_testEventRegEdgeCases(CsirxTestTaskObj *taskObj);
static int32_t CsirxCov_testCheckDphyrxConfig(CsirxTestTaskObj *taskObj);
static int32_t CsirxCov_testAsfEventGroups(CsirxTestTaskObj *taskObj);
static int32_t CsirxCov_testStopChFailBreak(CsirxTestTaskObj *taskObj);
static int32_t CsirxCov_testDphyrxCoreLaneReady(CsirxTestTaskObj *taskObj);
static int32_t CsirxCov_testHandlers(CsirxTestTaskObj *taskObj);
static int32_t CsirxCov_testErrorEventIsrPaths(CsirxTestTaskObj *taskObj);
static int32_t CsirxCov_testCpIntdEventIsrPaths(CsirxTestTaskObj *taskObj);
static int32_t CsirxCov_testInfoEventIsrPaths(CsirxTestTaskObj *taskObj);
static int32_t CsirxCov_testAsfEsmHighEventIsrPaths(CsirxTestTaskObj *taskObj);
static int32_t CsirxCov_testAsfEsmLowEventIsrPaths(CsirxTestTaskObj *taskObj);
static int32_t CsirxCov_testAsfEsmCfgEventIsrPaths(CsirxTestTaskObj *taskObj);
static int32_t CsirxCov_testChStartStopIoctl(CsirxTestTaskObj *taskObj);
static int32_t CsirxCov_testVirtContextExhausted(CsirxTestTaskObj *taskObj);
static int32_t CsirxCov_testQueueDequeueEdgeCases(CsirxTestTaskObj *taskObj);
static int32_t CsirxCov_testStreamIdleWaitLoop(CsirxTestTaskObj *taskObj);
static int32_t CsirxCov_testCreateEdgeCases(CsirxTestTaskObj *taskObj);
static int32_t CsirxCov_testGetFreeChNumExhausted(CsirxTestTaskObj *taskObj);
static int32_t CsirxCov_testNullHandleApis(CsirxTestTaskObj *taskObj);
static int32_t CsirxCov_testDeleteRunningHandle(CsirxTestTaskObj *taskObj);
static int32_t CsirxCov_testEventCbMismatch(CsirxTestTaskObj *taskObj);
static int32_t CsirxCov_testSetChCfgParamsCountLimit(CsirxTestTaskObj *taskObj);
static int32_t CsirxCov_testNullDrvHandle(CsirxTestTaskObj *taskObj);

static void    CsirxCov_initCreateParams(Csirx_CreateParams *createParams,
                                         uint32_t numCh,
                                         uint32_t chType,
                                         uint32_t dataType);
static int32_t CsirxCov_frameCompletionCb(Fvid2_Handle handle,
                                          Ptr appData);
static uint64_t CsirxCov_dummyTimestampFxn(void *args);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

extern struct Udma_DrvObj gUdmaDrvObj;
extern uint32_t gAppTrace;
extern CsirxDrv_CommonObj gCsirxCommonObj;


extern int32_t CsirxDrv_udmaToFvid2ErrorMap(int32_t udmaErr);

extern int32_t CsirxDrv_checkDphyrxConfig(const Csirx_DPhyCfg *programmedCfg,
                                           const Csirx_DPhyCfg *newCfg);

extern int32_t CsirxDrv_getDMACfgParams(CsirxDrv_ChObj *chObj);
extern int32_t CsirxDrv_setChUdmaParams(CsirxDrv_ChObj *chObj);
extern int32_t CsirxDrv_clearUdmaParams(CsirxDrv_ChObj *chObj);
extern void CsirxDrv_udmaCQEventCb(Udma_EventHandle eventHandle,
                            uint32_t eventType,
                            void *appData);
#if defined(SOC_J721E)
extern void CsirxDrv_udmaTDCEventCb(Udma_EventHandle eventHandle,
                             uint32_t eventType,
                             void *appData);
#endif
extern int32_t CsirxDrv_udmaRxTrpdInit(Udma_ChHandle rxChHandle,
                                uint8_t *pTrpdMem,
                                const uint32_t *destBuf,
                                const Csirx_ChCfg *chCfg,
                                uint32_t chIdx);
extern uint32_t CsirxDrv_getBpp(uint32_t dt);
extern uint32_t CsirxDrv_getStorageBpp(uint32_t dt);
extern int32_t  CsirxDrv_delete(Fdrv_Handle handle, void *reserved);

/** \brief Frame drop buffer for coverage tests */
static uint8_t gCovFrmDropBuf[CSIRX_COV_FRAME_SIZE]
    __attribute__((aligned(128), section(".data_buffer")));

/** \brief Shared driver handle created once in CsirxCov_runAllTests. */
static Fvid2_Handle gCovSharedHandle = NULL;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/* This test issues IOCTL_CSIRX_SET_DPHY_CONFIG with invalid DPHY parameters
 *       to trigger the error path inside Csirx_dphyCfg
 */
static int32_t CsirxCov_testDphyCfgErrorPath(CsirxTestTaskObj *taskObj)
{
    int32_t retVal = FVID2_SOK;
    int32_t testResult = CSIRX_COV_TEST_PASS;
    Fvid2_Handle drvHandle = NULL;
    Csirx_DPhyCfg dphyCfg;

    /* Use the shared driver handle */
    drvHandle = gCovSharedHandle;
    if (NULL == drvHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: DphyCfg - gCovSharedHandle is NULL\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* Test 1: DPHY config with invalid lane band speed. */
    if (NULL != drvHandle)
    {
        Csirx_initDPhyCfg(&dphyCfg);
        /* Set invalid/out-of-range lane band speed */
        dphyCfg.leftLaneBandSpeed  = 0xFFFFU;
        dphyCfg.rightLaneBandSpeed = 0xFFFFU;

        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_SET_DPHY_CONFIG,
                               (void *)&dphyCfg,
                               NULL);
        if (FVID2_SOK == retVal)
        {
            testResult = CSIRX_COV_TEST_FAIL;
            GT_1trace(gAppTrace, GT_INFO,
                      " CSIRX_COV: DphyCfg - invalid speed rejected"
                      " as expected: %d\r\n", retVal);
        }
    }

    /* Test 2: DPHY config with NULL cmdArgs pointer. */
    if (NULL != drvHandle)
    {
        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_SET_DPHY_CONFIG,
                               NULL,
                               NULL);
        if (FVID2_SOK == retVal)
        {
            testResult = CSIRX_COV_TEST_FAIL;
            GT_0trace(gAppTrace, GT_ERR,
                      " CSIRX_COV: DphyCfg - NULL args should fail!\r\n");
        }
    }

    /* Test 3: DPHY config with zero lanes. */
    if (NULL != drvHandle)
    {
        Csirx_initDPhyCfg(&dphyCfg);
        /* Zero clock/lane speed -- triggers the false branch in
         * compound conditions */
        dphyCfg.leftLaneBandSpeed  = 0U;
        dphyCfg.rightLaneBandSpeed = 0U;

        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_SET_DPHY_CONFIG,
                               (void *)&dphyCfg,
                               NULL);
        if (FVID2_SOK != retVal)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }
    }

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: DphyCfg error path test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

/* Exercises the default/unknown command branch. */
static int32_t CsirxCov_testInvalidCmd(CsirxTestTaskObj *taskObj)
{
    int32_t retVal = FVID2_SOK;
    int32_t testResult = CSIRX_COV_TEST_PASS;
    Fvid2_Handle drvHandle = NULL;

    drvHandle = gCovSharedHandle;
    if (NULL == drvHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: InvalidIOCTL - gCovSharedHandle is NULL\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* Invalid/unknown IOCTL command */
    if (NULL != drvHandle)
    {
        retVal = Fvid2_control(drvHandle,
                               CSIRX_COV_INVALID_IOCTL_CMD,
                               NULL,
                               NULL);
        if (FVID2_SOK == retVal)
        {
            GT_0trace(gAppTrace, GT_ERR,
                      " CSIRX_COV: InvalidIOCTL - unknown cmd should fail!\r\n");
            testResult = CSIRX_COV_TEST_FAIL;
        }
    }

    /* IOCTL_CSIRX_TRIG_ASF_EVENT */
    if (NULL != drvHandle)
    {
        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_TRIG_ASF_EVENT,
                               NULL,
                               NULL);
        if (FVID2_SOK != retVal)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }
    }

    /* IOCTL_CSIRX_PRINT_DEBUG_LOGS */
    if (NULL != drvHandle)
    {
        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_PRINT_DEBUG_LOGS,
                               NULL,
                               NULL);
        if (FVID2_SOK != retVal)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }
    }

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: Invalid IOCTL cmd test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

/* Create with invalid driver ID */
static int32_t CsirxCov_testInvalidDriver(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    Fvid2_Handle drvHandle = NULL;
    Csirx_CreateParams createParams;
    Csirx_CreateStatus createStatus;
    Fvid2_CbParams cbPrms;
    uint32_t instId;

    instId = taskObj->instObj.instCfgInfo->csiDrvInst;

    Fvid2CbParams_init(&cbPrms);
    cbPrms.cbFxn = &CsirxCov_frameCompletionCb;
    cbPrms.appData = (Ptr)taskObj;

    CsirxCov_initCreateParams(&createParams, 0U,
                              CSIRX_CH_TYPE_CAPT,
                              FVID2_CSI2_DF_RAW12);

    drvHandle = Fvid2_create(0xDEADU,
                             instId,
                             (void *)&createParams,
                             (void *)&createStatus,
                             &cbPrms);
    if (NULL != drvHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: Boundary - invalid drvId should fail!\r\n");
        (void)Fvid2_delete(drvHandle, NULL);
        testResult = CSIRX_COV_TEST_FAIL;
    }

    drvHandle = Fvid2_create(CSIRX_CAPT_DRV_ID,
                             instId,
                             (void *)&createParams,
                             NULL,  /* NULL createStatus */
                             &cbPrms);
    if (NULL != drvHandle)
    {
        GT_0trace(gAppTrace, GT_INFO,
                  " CSIRX_COV: DrvEdge - created with NULL status \r\n");
        (void)Fvid2_delete(drvHandle, NULL);
        testResult = CSIRX_COV_TEST_FAIL;
    }

    drvHandle = Fvid2_create(CSIRX_CAPT_DRV_ID,
                             instId,
                             (void *)&createParams,
                             (void *)&createStatus,
                             NULL);  /* NULL cbPrms */
    if (NULL != drvHandle)
    {
        GT_0trace(gAppTrace, GT_INFO,
                  " CSIRX_COV: DrvEdge - created with NULL cbPrms \r\n");
        (void)Fvid2_delete(drvHandle, NULL);
        testResult = CSIRX_COV_TEST_FAIL;
    }

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: Invalid Driver ID test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

static int32_t CsirxCov_testNullPointerArgs(CsirxTestTaskObj *taskObj)
{
    int32_t retVal = FVID2_SOK;
    int32_t testResult = CSIRX_COV_TEST_PASS;
    Csirx_InitParams initPrms;
    SemaphoreP_Params semPrms;
    /* 512 exceeds the largest SOC pool (j784s4/j721e = 360, j7200 = 150) */
    static SemaphoreP_Handle extraSems[512U];
    uint32_t numExtraSems = 0U;
    uint32_t semIdx;
    Fvid2_InitPrms           fvid2InitPrms;

    retVal = Csirx_init(NULL);
    if(retVal != FVID2_EBADARGS)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: Csirx_init(NULL) should fail \r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    Csirx_initParamsInit(&initPrms);
    retVal = Csirx_init(&initPrms);
    if (FVID2_EDRIVER_INUSE != retVal)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  APP_NAME ": System Re-Init Did not Fail!!!\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    retVal = Csirx_deInit();
    if(FVID2_EFAIL != retVal)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  APP_NAME ": System DeInit did not Fail!!!\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    SemaphoreP_Params_init(&semPrms);
    semPrms.mode = SemaphoreP_Mode_BINARY;
    for (semIdx = 0U; semIdx < 512U; semIdx++)
    {
        extraSems[semIdx] = SemaphoreP_create(1U, &semPrms);
        if (NULL == extraSems[semIdx])
        {
            break;
        }
        numExtraSems++;
    }
    if (numExtraSems < 512U)
    {
        /* Pool is exhausted; Csirx_init must fail at CsirxDrv_modInstObjInit. */
        retVal = Csirx_init(&initPrms);
        if (FVID2_SOK == retVal)
        {
            GT_0trace(gAppTrace, GT_ERR,
                      APP_NAME ": Csirx_init should fail on exhausted"
                      " semaphore pool!\r\n");
            testResult = CSIRX_COV_TEST_FAIL;
            (void)Csirx_deInit();
        }
    }
    else
    {
        GT_0trace(gAppTrace, GT_ERR,
                  APP_NAME ": Semaphore pool not exhausted within 512"
                  " allocations - increase limit!\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* Fvid2_deInit frees its two internal semaphores (lockSem + printSem),
     * leaving exactly two free pool slots.  Plug those two slots so the
     * pool remains fully exhausted when Fvid2_init is called below; this
     * forces SemaphoreP_create to return NULL and covers the error trace
     * at fvid2_drvMgr.c line 512.                                        */
    (void)Fvid2_deInit(NULL);
    {
        SemaphoreP_Handle plugSem1 = SemaphoreP_create(1U, &semPrms);
        SemaphoreP_Handle plugSem2 = SemaphoreP_create(1U, &semPrms);
        Fvid2InitPrms_init(&fvid2InitPrms);
        fvid2InitPrms.printFxn = &App_fvidPrint;
        (void)Fvid2_init(&fvid2InitPrms); /* expected FVID2_EALLOC; covers line 512 */
        if (NULL != plugSem1) { (void)SemaphoreP_delete(plugSem1); }
        if (NULL != plugSem2) { (void)SemaphoreP_delete(plugSem2); }
    }

    /* Release all extra semaphores so the pool is available for re-init. */
    for (semIdx = 0U; semIdx < numExtraSems; semIdx++)
    {
        (void)SemaphoreP_delete(extraSems[semIdx]);
    }

    /* The Fvid2_init above failed (FVID2_EALLOC); properly re-initialize
     * FVID2 now that the pool has sufficient space.                       */
    Fvid2InitPrms_init(&fvid2InitPrms);
    fvid2InitPrms.printFxn = &App_fvidPrint;
    Fvid2_init(&fvid2InitPrms);

    retVal = Csirx_init(&initPrms);
    if (FVID2_SOK != retVal)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  APP_NAME ": System Re-Init Failed!!!\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: Null Pointer Argument test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}



/*
 *  Key compound conditions targeted:
 *    - CsirxDrv_create: (drvId == CSIRX_CAPT_DRV_ID) && (instId < MAX)
 *    - CsirxDrv_queue:  (state == RUNNING) && (frmList != NULL) && (numFrames > 0)
 *    - CsirxDrv_dequeue: (state == RUNNING) && (frmList != NULL)
 *    - Csirx_dphyCfg:   (leftBandSpeed != 0) && (rightBandSpeed != 0)
 */
static int32_t CsirxCov_testCompoundConds(CsirxTestTaskObj *taskObj)
{
    int32_t retVal = FVID2_SOK;
    int32_t testResult = CSIRX_COV_TEST_PASS;
    Fvid2_Handle drvHandle = NULL;
    Csirx_EventPrms eventPrms;
    Csirx_DPhyCfg dphyCfg;
    Fvid2_CbParams cbPrms;
    uint32_t instId;
    CsirxDrv_InstObj *instObj;
    Csirx_InstStatus instStatus;
    Fvid2_TimeStampParams tsParams;


    Fvid2CbParams_init(&cbPrms);
    cbPrms.cbFxn = &CsirxCov_frameCompletionCb;
    cbPrms.appData = (Ptr)taskObj;
    instId  = taskObj->instObj.instCfgInfo->csiDrvInst;
    instObj = &gCsirxCommonObj.instObj[instId];

    drvHandle = gCovSharedHandle;
    if (NULL == drvHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: gCovSharedHandle is NULL\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    Csirx_eventPrmsInit(NULL);

    /* IOCTL: GET_INST_STATUS */
    if (NULL != drvHandle)
    {
        Csirx_instStatusInit(&instStatus);
        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_GET_INST_STATUS,
                               (void *)&instStatus,
                               NULL);

        /* GET_INST_STATUS with NULL cmdArgs */
        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_GET_INST_STATUS,
                               NULL,
                               NULL);
    }

    /* DPHY: vary left/right band speed independently */
    if (NULL != drvHandle)
    {
        /* leftBandSpeed=valid, rightBandSpeed=0 */
        Csirx_initDPhyCfg(&dphyCfg);
        dphyCfg.leftLaneBandSpeed  = 800U;
        dphyCfg.rightLaneBandSpeed = 0U;
        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_SET_DPHY_CONFIG,
                               (void *)&dphyCfg,
                               NULL);

        /* leftBandSpeed=0, rightBandSpeed=valid */
        Csirx_initDPhyCfg(&dphyCfg);
        dphyCfg.leftLaneBandSpeed  = 0U;
        dphyCfg.rightLaneBandSpeed = 800U;
        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_SET_DPHY_CONFIG,
                               (void *)&dphyCfg,
                               NULL);
    }

    if (NULL != drvHandle)
    {
        /* Register ERROR with only FIFO_OVERFLOW_FRONT bit set */
        Csirx_eventPrmsInit(&eventPrms);
        eventPrms.eventGroup = CSIRX_EVENT_GROUP_ERROR;
        eventPrms.eventMasks = CSIRX_EVENT_TYPE_ERR_FIFO_OVERFLOW_FRONT;
        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_REGISTER_EVENT,
                               &eventPrms,
                               NULL);
        if (FVID2_SOK != retVal)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }

        /* Unregister ERROR. */
        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_UNREGISTER_EVENT,
                               (void *)(uintptr_t)CSIRX_EVENT_GROUP_ERROR,
                               NULL);
        if (FVID2_SOK != retVal)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }
    }

    if (NULL != drvHandle)
    {
        tsParams.timeStampFxn = &CsirxCov_dummyTimestampFxn;
        tsParams.reserved     = 0U;
        retVal = Fvid2_control(drvHandle,
                               FVID2_REGISTER_TIMESTAMP_FXN,
                               (void *)&tsParams,
                               NULL);
        if (FVID2_SOK != retVal)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }

        /* Un-register by passing NULL function pointer */
        tsParams.timeStampFxn = NULL;
        tsParams.reserved     = 0U;
        retVal = Fvid2_control(drvHandle,
                               FVID2_REGISTER_TIMESTAMP_FXN,
                               (void *)&tsParams,
                               NULL);


        retVal = Fvid2_control(drvHandle,
                                   IOCTL_CSIRX_GET_INST_CH_NUM,
                                   &eventPrms,
                                   NULL);
        if (FVID2_SOK != retVal)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }
    }

    /* CsirxDrv_dphyrxPsoDisable */
    CsirxDrv_dphyrxPsoDisable(instObj->dPhyCoreAddr, 0U);
    CsirxDrv_dphyrxPsoDisable(instObj->dPhyCoreAddr, 1U);

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: Compound conditions test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

/* Test DMA Config */
static int32_t CsirxCov_testDmaCfgEdgeCases(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    int32_t retVal;
    Fvid2_Handle drvHandle = NULL;
    CsirxDrv_InstObj *instObj;
    CsirxDrv_ChObj *chObj;
    uint32_t instId;

    /* Saved state for restoration */
    Csirx_ChCfg *savedChCfg;
    uint32_t savedNumPixels;
    Udma_ChRxPrms *savedRxChParams;
    Udma_ChPrms savedChParams;
    uint8_t *savedTrpdMem;
    uint8_t *savedRxFqRingMem;
    uint8_t *savedRxCqRingMem;
    uint8_t *savedRxTdCqRingMem;
    CSL_CsirxDMAConfig savedDmaCfgParams;
    Udma_DrvHandle savedDrvHandle;
    Csirx_CreateParams createParams;
    CsirxDrv_CommonObj *captObj;

    /* Local config for sub-tests */
    Csirx_ChCfg localChCfg;

    /* Use the shared driver handle */
    drvHandle = gCovSharedHandle;
    if (NULL == drvHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: DmaCfg - gCovSharedHandle is NULL\r\n");
        return CSIRX_COV_TEST_FAIL;
    }

    instId = taskObj->instObj.instCfgInfo->csiDrvInst;
    instObj = &gCsirxCommonObj.instObj[instId];
    chObj = &instObj->chObj[0];
    captObj = chObj->instObj->commonObjRef;

    /* Save original state for all fields modified by CsirxDrv_getDMACfgParams */
    savedChCfg         = chObj->chCfg;
    savedNumPixels     = instObj->createParams.instCfg.numPixelsStrm0;
    savedRxChParams    = chObj->rxChParams;
    memcpy(&savedChParams, &chObj->chParams, sizeof(savedChParams));
    savedTrpdMem       = chObj->trpdMem;
    savedRxFqRingMem   = chObj->rxFqRingMem;
    savedRxCqRingMem   = chObj->rxCqRingMem;
    savedRxTdCqRingMem = chObj->rxTdCqRingMem;
    savedDrvHandle = captObj->initParams.drvHandle;
    memcpy(&savedDmaCfgParams, &chObj->dmaCfgParams, sizeof(savedDmaCfgParams));

    /* Initialise local config with sensible defaults */
    memset(&localChCfg, 0, sizeof(localChCfg));
    localChCfg.outFmt.ccsFormat = FVID2_CCSF_BITS12_UNPACKED16;

    /* Sub-test A: YUV422_8B with numPixelsStrm0 = 2 */
    localChCfg.inCsiDataType     = FVID2_CSI2_DF_YUV422_8B;
    localChCfg.outFmt.dataFormat = FVID2_DF_YUV422I_UYVY;
    chObj->chCfg = &localChCfg;
    createParams.instCfg.numPixelsStrm0 = 2U;
    chObj->instObj->createParams = createParams;

    retVal = CsirxDrv_getDMACfgParams(chObj);
    if (FVID2_EFAIL != retVal)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: DmaCfg Sub-A FAIL - expected EFAIL for"
                  " YUV422_8B numPixelsStrm0=2\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* RAW8 with numPixelsStrm0 = 2 */
    localChCfg.inCsiDataType     = FVID2_CSI2_DF_RAW8;
    localChCfg.outFmt.dataFormat = FVID2_DF_BGRX32_8888;
    chObj->chCfg = &localChCfg;
    instObj->createParams.instCfg.numPixelsStrm0 = 2U;

    retVal = CsirxDrv_getDMACfgParams(chObj);
    if (FVID2_SOK != retVal)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: DmaCfg Sub-B FAIL - expected SOK for"
                  " RAW8 numPixelsStrm0=2\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }
    else if ((CSL_CSIRX_DMA_DATA_SIZE_SHIFT_32BITS !=
              chObj->dmaCfgParams.dataSizeShift) ||
             (1U != chObj->dmaCfgParams.dualPkgEnable))
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: DmaCfg Sub-B FAIL - unexpected"
                  " dataSizeShift or dualPkgEnable\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }
    else
    {
        /* do nothing */
    }

    chObj->chCfg = &localChCfg;
    instObj->createParams.instCfg.numPixelsStrm0 = 0U;

    retVal = CsirxDrv_getDMACfgParams(chObj);

    chObj->chCfg = &localChCfg;
    instObj->createParams.instCfg.numPixelsStrm0 = 1U;

    retVal = CsirxDrv_getDMACfgParams(chObj);

    chObj->chCfg = &localChCfg;
    instObj->createParams.instCfg.numPixelsStrm0 = 3U;

    retVal = CsirxDrv_getDMACfgParams(chObj);

    /* Sub-test A: YUV422_8B with numPixelsStrm0 = 2 */
    localChCfg.inCsiDataType = FVID2_CSI2_DF_RAW12;
    localChCfg.outFmt.ccsFormat = FVID2_CCSF_BITS12_PACKED;
    localChCfg.outFmt.dataFormat = FVID2_DF_YUV422I_YUYV;
    chObj->chCfg = &localChCfg;

    retVal = CsirxDrv_getDMACfgParams(chObj);

    /* Sub-test A: YUV422_8B with numPixelsStrm0 = 2 */
    localChCfg.outFmt.ccsFormat = FVID2_CCSF_BITS12_UNPACKED16_MSB_ALIGNED;
    localChCfg.outFmt.dataFormat = FVID2_DF_YUV422I_YVYU;
    chObj->chCfg = &localChCfg;

    retVal = CsirxDrv_getDMACfgParams(chObj);

    /* Sub-test A: YUV422_8B with numPixelsStrm0 = 2 */
    localChCfg.inCsiDataType = FVID2_CSI2_DF_RAW20;
    localChCfg.outFmt.dataFormat = FVID2_DF_YUV422I_VYUY;
    chObj->chCfg = &localChCfg;

    retVal = CsirxDrv_getDMACfgParams(chObj);

    /* Sub-test A: YUV422_8B with numPixelsStrm0 = 2 */
    localChCfg.inCsiDataType = FVID2_CSI2_DF_YUV420_10B;
    instObj->createParams.instCfg.numPixelsStrm0 = 1U;
    chObj->chCfg = &localChCfg;

    retVal = CsirxDrv_getDMACfgParams(chObj);

    localChCfg.inCsiDataType = FVID2_CSI2_DF_YUV422_8B;
    chObj->chCfg = &localChCfg;

    retVal = CsirxDrv_getDMACfgParams(chObj);

    /* Sub-test: YUV422_8B with numPixelsStrm0=0 makes the (0U==numPixels)
     * condition at csirx_drvUdma.c line 222 the controlling TRUE term,
     * covering the gap at line 222-222 (Branch/Decision). */
    localChCfg.inCsiDataType = FVID2_CSI2_DF_YUV422_8B;
    localChCfg.outFmt.ccsFormat = FVID2_CCSF_BITS12_UNPACKED16;
    instObj->createParams.instCfg.numPixelsStrm0 = 0U;
    chObj->chCfg = &localChCfg;
    retVal = CsirxDrv_getDMACfgParams(chObj);

    /* Sub-tests: YUV422_10B, RAW10, RAW14, RAW16 each make their individual
     * sub-condition in the compound else-if at csirx_drvUdma.c lines 197-201
     * the controlling TRUE term, covering gaps 197-201, 198-201, 200-201. */
    instObj->createParams.instCfg.numPixelsStrm0 = 0U;

    localChCfg.inCsiDataType = FVID2_CSI2_DF_YUV422_10B;
    chObj->chCfg = &localChCfg;
    (void)CsirxDrv_getDMACfgParams(chObj);

    localChCfg.inCsiDataType = FVID2_CSI2_DF_RAW10;
    chObj->chCfg = &localChCfg;
    (void)CsirxDrv_getDMACfgParams(chObj);

    localChCfg.inCsiDataType = FVID2_CSI2_DF_RAW14;
    chObj->chCfg = &localChCfg;
    (void)CsirxDrv_getDMACfgParams(chObj);

    localChCfg.inCsiDataType = FVID2_CSI2_DF_RAW16;
    chObj->chCfg = &localChCfg;
    (void)CsirxDrv_getDMACfgParams(chObj);

    chObj->chCfg->chType = CSIRX_CH_TYPE_CAPT;

    /* Close existing UDMA resources before sub-tests.
     * Udma_chOpen does NOT check chInitDone and silently re-allocates RM
     * resources when called on an already-open handle, which orphans the
     * original channel in the RM and causes Udma_deinit to fail. */
    (void)CsirxDrv_clearUdmaParams(chObj);
    if (UDMA_INIT_DONE == captObj->masterEvent.eventInitDone)
    {
        (void)Udma_eventUnRegister(&captObj->masterEvent);
    }

    /* Sub-test: NULL drvHandle - Udma_chOpen fails immediately,
     * covers the error path at lines 288-293 in csirx_drvUdma.c */
    captObj->initParams.drvHandle = NULL;
    memset(&chObj->rxChObj, 0, sizeof(chObj->rxChObj));
    memset(&captObj->masterEvent, 0, sizeof(captObj->masterEvent));
    retVal = CsirxDrv_setChUdmaParams(chObj);

    /* Sub-test: valid drvHandle with zeroed masterEvent and rxChObj -
     * exercises the masterEvent registration path in CsirxDrv_setChUdmaParams */
    captObj->initParams.drvHandle = savedDrvHandle;
    memset(&chObj->rxChObj, 0, sizeof(chObj->rxChObj));
    memset(&captObj->masterEvent, 0, sizeof(captObj->masterEvent));
    retVal = CsirxDrv_setChUdmaParams(chObj);

    CsirxDrv_udmaCQEventCb(NULL, UDMA_EVENT_TYPE_DMA_COMPLETION, chObj);

    CsirxDrv_udmaCQEventCb(NULL, UDMA_EVENT_TYPE_MASTER, chObj);

#if defined(SOC_J721E)
    CsirxDrv_udmaTDCEventCb(NULL, UDMA_EVENT_TYPE_TEARDOWN_PACKET, chObj);
    CsirxDrv_udmaTDCEventCb(NULL, UDMA_EVENT_TYPE_MASTER, chObj);
#endif

    /* Close resources opened by the sub-test above */
    (void)CsirxDrv_clearUdmaParams(chObj);
    if (UDMA_INIT_DONE == captObj->masterEvent.eventInitDone)
    {
        (void)Udma_eventUnRegister(&captObj->masterEvent);
    }

    /* Restore all other state modified by CsirxDrv_getDMACfgParams */
    chObj->chCfg = savedChCfg;
    instObj->createParams.instCfg.numPixelsStrm0 = savedNumPixels;
    chObj->rxChParams    = savedRxChParams;
    memcpy(&chObj->chParams, &savedChParams, sizeof(chObj->chParams));
    chObj->trpdMem       = savedTrpdMem;
    chObj->rxFqRingMem   = savedRxFqRingMem;
    chObj->rxCqRingMem   = savedRxCqRingMem;
    chObj->rxTdCqRingMem = savedRxTdCqRingMem;
    memcpy(&chObj->dmaCfgParams, &savedDmaCfgParams, sizeof(chObj->dmaCfgParams));

    /* Re-open UDMA resources to restore driver to working state */
    memset(&chObj->rxChObj, 0, sizeof(chObj->rxChObj));
    memset(&captObj->masterEvent, 0, sizeof(captObj->masterEvent));
    retVal = CsirxDrv_setChUdmaParams(chObj);
    if (FVID2_SOK != retVal)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: DmaCfg - UDMA channel restore failed\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* Exercise CsirxDrv_getIcnt0 packed-RAW12 branch (lines 951-958):
     * Requires inCsiDataType==RAW12 AND outFmt.ccsFormat==BITS12_PACKED.
     * Use a temporary aligned buffer for the TRPD structure. */
    if (FVID2_SOK == retVal)
    {
        static uint8_t trpdBuf[256U] __attribute__((aligned(128)));
        Csirx_ChCfg raw12PackedCfg;

        memset(trpdBuf, 0, sizeof(trpdBuf));
        memset(&raw12PackedCfg, 0, sizeof(raw12PackedCfg));

        raw12PackedCfg.inCsiDataType    = FVID2_CSI2_DF_RAW12;
        raw12PackedCfg.outFmt.ccsFormat = FVID2_CCSF_BITS12_PACKED;
        raw12PackedCfg.outFmt.width     = CSIRX_COV_FRAME_WIDTH;
        raw12PackedCfg.outFmt.height    = CSIRX_COV_FRAME_HEIGHT;
        raw12PackedCfg.outFmt.pitch[0U] = CSIRX_COV_FRAME_WIDTH *
                                          CSIRX_COV_FRAME_BPP;
        raw12PackedCfg.chType           = CSIRX_CH_TYPE_CAPT;

        retVal = CsirxDrv_udmaRxTrpdInit(
            &chObj->rxChObj,
            trpdBuf,
            NULL,
            &raw12PackedCfg,
            (uint32_t)chObj->chId);

        if (FVID2_SOK != retVal)
        {
            GT_1trace(gAppTrace, GT_ERR,
                      " CSIRX_COV: DmaCfg - RAW12 packed TRPD init failed: %d\r\n",
                      retVal);
            testResult = CSIRX_COV_TEST_FAIL;
        }
    }

    /* FVID2_EINVALID_PARAMS when
     * icnt0 > dim1 (line width in bytes exceeds the configured pitch).
     * pitch[0]=0 forces dim1=0 while icnt0=1280 (640px * 2Bpp) → TRUE.
     * Uses the rxChObj already open from the CsirxDrv_setChUdmaParams
     * sub-test above (line 829). */
    {
        static uint8_t trpdBufBadPitch[256U] __attribute__((aligned(128)));
        Csirx_ChCfg    badPitchCfg;
        int32_t        trpdRet;

        memset(trpdBufBadPitch, 0, sizeof(trpdBufBadPitch));
        memset(&badPitchCfg, 0, sizeof(badPitchCfg));

        badPitchCfg.inCsiDataType    = FVID2_CSI2_DF_RAW12;
        badPitchCfg.outFmt.ccsFormat = FVID2_CCSF_BITS12_UNPACKED16;
        badPitchCfg.outFmt.width     = CSIRX_COV_FRAME_WIDTH;  /* icnt0 = 1280 B */
        badPitchCfg.outFmt.height    = CSIRX_COV_FRAME_HEIGHT;
        badPitchCfg.outFmt.pitch[0U] = 0U;  /* dim1 = 0 → icnt0 > dim1 */
        badPitchCfg.chType           = CSIRX_CH_TYPE_CAPT;

        trpdRet = CsirxDrv_udmaRxTrpdInit(
                      &chObj->rxChObj,
                      trpdBufBadPitch,
                      NULL,
                      &badPitchCfg,
                      (uint32_t)chObj->chId);
        if (FVID2_EINVALID_PARAMS != trpdRet)
        {
            GT_1trace(gAppTrace, GT_ERR,
                      " CSIRX_COV: DmaCfg - bad-pitch TRPD init expected"
                      " EINVALID_PARAMS, got: %d\r\n", trpdRet);
            testResult = CSIRX_COV_TEST_FAIL;
        }
    }

    /* CsirxDrv_getBpp / CsirxDrv_getStorageBpp with an unknown data type
     * exhaust the gRxDtInfo table → loopCnt >= size → if(loopCnt<size) FALSE.
     * 0xFF is not in gRxDtInfo, so both functions return bpp=0.            */
    {
        uint32_t bppVal;

        bppVal = CsirxDrv_getBpp(0xFFU);
        if (0U != bppVal)
        {
            GT_1trace(gAppTrace, GT_ERR,
                      " CSIRX_COV: DmaCfg - getBpp(0xFF) expected 0,"
                      " got %d\r\n", bppVal);
            testResult = CSIRX_COV_TEST_FAIL;
        }

        bppVal = CsirxDrv_getStorageBpp(0xFFU);
        if (0U != bppVal)
        {
            GT_1trace(gAppTrace, GT_ERR,
                      " CSIRX_COV: DmaCfg - getStorageBpp(0xFF) expected 0,"
                      " got %d\r\n", bppVal);
            testResult = CSIRX_COV_TEST_FAIL;
        }
    }

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: DMA Cfg edge cases test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

/* Direct call to the internal function with various UDMA error codes.*/
static int32_t CsirxCov_testUdmaErrorMap(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    int32_t retVal;

    retVal = CsirxDrv_udmaToFvid2ErrorMap(UDMA_SOK);
    if (FVID2_SOK != retVal)
    {
        GT_1trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: UdmaErrMap - UDMA_SOK mapped to %d"
                  " (expected FVID2_SOK)\r\n", retVal);
        testResult = CSIRX_COV_TEST_FAIL;
    }

    retVal = CsirxDrv_udmaToFvid2ErrorMap(UDMA_EFAIL);
    if (FVID2_EFAIL != retVal)
    {
        GT_1trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: UdmaErrMap - UDMA_EFAIL mapped to %d"
                  " (expected FVID2_EFAIL)\r\n", retVal);
        testResult = CSIRX_COV_TEST_FAIL;
    }

    retVal = CsirxDrv_udmaToFvid2ErrorMap(UDMA_EBADARGS);
    if (FVID2_EBADARGS != retVal)
    {
        GT_1trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: UdmaErrMap - UDMA_EBADARGS mapped to %d"
                  " (expected FVID2_EBADARGS)\r\n", retVal);
        testResult = CSIRX_COV_TEST_FAIL;
    }

    retVal = CsirxDrv_udmaToFvid2ErrorMap(UDMA_EINVALID_PARAMS);
    if (FVID2_EINVALID_PARAMS != retVal)
    {
        GT_1trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: UdmaErrMap - UDMA_EINVALID_PARAMS mapped to %d"
                  " (expected FVID2_EINVALID_PARAMS)\r\n", retVal);
        testResult = CSIRX_COV_TEST_FAIL;
    }

    retVal = CsirxDrv_udmaToFvid2ErrorMap(UDMA_ETIMEOUT);
    if (FVID2_ETIMEOUT != retVal)
    {
        GT_1trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: UdmaErrMap - UDMA_ETIMEOUT mapped to %d"
                  " (expected FVID2_ETIMEOUT)\r\n", retVal);
        testResult = CSIRX_COV_TEST_FAIL;
    }

    retVal = CsirxDrv_udmaToFvid2ErrorMap(UDMA_EALLOC);
    if (FVID2_EALLOC != retVal)
    {
        GT_1trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: UdmaErrMap - UDMA_EALLOC mapped to %d"
                  " (expected FVID2_EALLOC)\r\n", retVal);
        testResult = CSIRX_COV_TEST_FAIL;
    }

    retVal = CsirxDrv_udmaToFvid2ErrorMap(0x9999);
    if (FVID2_EFAIL != retVal)
    {
        GT_1trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: UdmaErrMap - unknown error mapped to %d"
                  " (expected FVID2_EFAIL default)\r\n", retVal);
        testResult = CSIRX_COV_TEST_FAIL;
    }

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: UDMA error map test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

/* Test: CsirxCov_testEventRegEdgeCases */
static int32_t CsirxCov_testEventRegEdgeCases(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    int32_t retVal;
    Fvid2_Handle drvHandle = NULL;
    Csirx_EventPrms eventPrms;
    uint32_t instId;
    CsirxDrv_InstObj *instObj;
    uint32_t savedInUse;
    uint32_t savedDrvInstId;
    uint32_t grpIdx;
    SemaphoreP_Handle savedLockSem;

    drvHandle = gCovSharedHandle;
    if (NULL == drvHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: EventReg - gCovSharedHandle is NULL\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    if (NULL != drvHandle)
    {
        instId = taskObj->instObj.instCfgInfo->csiDrvInst;
        instObj = &gCsirxCommonObj.instObj[instId];

        /* Register ERROR events (standard path) */
        Csirx_eventPrmsInit(&eventPrms);

        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_REGISTER_EVENT,
                               &eventPrms,
                               NULL);
        if(retVal != FVID2_SOK)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }

        /* Re-register ERROR with same params. */
        Csirx_eventPrmsInit(&eventPrms);
        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_REGISTER_EVENT,
                               &eventPrms,
                               NULL);
        if(retVal != FVID2_SOK)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }

        /* Re-register ERROR with different eventMasks. */
        Csirx_eventPrmsInit(&eventPrms);
        eventPrms.eventMasks = CSIRX_EVENT_TYPE_ERR_FIFO_OVERFLOW_FRONT;
        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_REGISTER_EVENT,
                               &eventPrms,
                               NULL);
        if(retVal != FVID2_WNON_RECOMMENDED_PARAMS)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }

        /* Register CP_INTD events. */
        Csirx_eventPrmsInit(&eventPrms);
        eventPrms.eventGroup = CSIRX_EVENT_GROUP_CP_INTD;
        eventPrms.eventMasks = CSIRX_EVENT_TYPE_CP_INTD_ALL;
        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_REGISTER_EVENT,
                               &eventPrms,
                               NULL);
        if(retVal != FVID2_SOK)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }

        /* Unregister CP_INTD. */
        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_UNREGISTER_EVENT,
                               (void *)(uintptr_t)CSIRX_EVENT_GROUP_CP_INTD,
                               NULL);
        if(retVal != FVID2_SOK)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }

        /* Register INFO events. */
        Csirx_eventPrmsInit(&eventPrms);
        eventPrms.eventGroup = CSIRX_EVENT_GROUP_INFO;
        eventPrms.eventMasks = CSIRX_EVENT_TYPE_INFO_ALL;
        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_REGISTER_EVENT,
                               &eventPrms,
                               NULL);
        if(retVal != FVID2_SOK)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }

        /* Unregister INFO. */
        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_UNREGISTER_EVENT,
                               (void *)(uintptr_t)CSIRX_EVENT_GROUP_INFO,
                               NULL);
        if(retVal != FVID2_SOK)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }

        /* Register with invalid params. */
        Csirx_eventPrmsInit(&eventPrms);
        eventPrms.eventGroup = CSIRX_EVENT_GROUP_MAX;
        eventPrms.eventMasks = 0U;
        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_REGISTER_EVENT,
                               &eventPrms,
                               NULL);
        if(retVal != FVID2_EBADARGS)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }

        /* Invalid eventGroup with non-zero eventMasks */
        Csirx_eventPrmsInit(&eventPrms);
        eventPrms.eventGroup = CSIRX_EVENT_GROUP_MAX + 1U;
        eventPrms.eventMasks = CSIRX_EVENT_TYPE_ERR_ALL;
        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_REGISTER_EVENT,
                               &eventPrms,
                               NULL);
        if(retVal != FVID2_EBADARGS)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }

        /* Register ERROR */
        Csirx_eventPrmsInit(&eventPrms);
        eventPrms.eventGroup = CSIRX_EVENT_GROUP_ERROR;
        eventPrms.eventMasks = CSIRX_EVENT_TYPE_ERR_ALL;

        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_REGISTER_EVENT,
                               &eventPrms,
                               NULL);
        if(retVal != FVID2_SOK)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }

        savedLockSem = instObj->lockSem;
        instObj->lockSem = (SemaphoreP_Handle)NULL;

        /* Unregister with lockSem=NULL */
        retVal = Fvid2_control(drvHandle,
                                IOCTL_CSIRX_UNREGISTER_EVENT,
                                (void *)(uintptr_t)CSIRX_EVENT_GROUP_ERROR,
                                NULL);
        instObj->lockSem = savedLockSem;

        /* Unregister ERROR */
        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_UNREGISTER_EVENT,
                               (void *)(uintptr_t)CSIRX_EVENT_GROUP_ERROR,
                               NULL);
        if(retVal != FVID2_SOK)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }

        instId = taskObj->instObj.instCfgInfo->csiDrvInst;
        instObj = &gCsirxCommonObj.instObj[instId];

        /*
         * Temporarily set inUse to NOT_USED so that the check at line 109
         * fails and retVal = FVID2_EFAIL at line 111 is executed.
         */
        savedInUse = instObj->inUse;
        instObj->inUse = CSIRX_DRV_USAGE_STATUS_NOT_USED;

        Csirx_eventPrmsInit(&eventPrms);
        eventPrms.eventGroup = CSIRX_EVENT_GROUP_ERROR;
        eventPrms.eventMasks = CSIRX_EVENT_TYPE_ERR_ALL;
        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_REGISTER_EVENT,
                               &eventPrms,
                               NULL);
        if (FVID2_EFAIL != retVal)
        {
            GT_1trace(gAppTrace, GT_INFO,
                  " CSIRX_COV: EventInvParams - inUse NOT_USED register"
                  " retVal=%d (expected EFAIL)\r\n", retVal);
            testResult = CSIRX_COV_TEST_FAIL;
        }

        instObj->inUse = savedInUse;

        /*
         * Temporarily change drvInstId to CSIRX_INSTANCE_ID_1 so that
         * CsirxDrv_eventGroupAllocResource enters the INSTANCE_ID_1.
         */
        savedDrvInstId = instObj->drvInstId;
        instObj->drvInstId = CSIRX_INSTANCE_ID_1;

        for (grpIdx = 0U; grpIdx < CSIRX_EVENT_GROUP_MAX; grpIdx++)
        {
            Csirx_eventPrmsInit(&eventPrms);
            eventPrms.eventGroup = grpIdx;
            /* Pick a non-zero eventMask appropriate for the group */
            if (CSIRX_EVENT_GROUP_ERROR == grpIdx)
            {
                eventPrms.eventMasks = CSIRX_EVENT_TYPE_ERR_ALL;
            }
            else if (CSIRX_EVENT_GROUP_CP_INTD == grpIdx)
            {
                eventPrms.eventMasks = CSIRX_EVENT_TYPE_CP_INTD_ALL;
            }
            else if (CSIRX_EVENT_GROUP_INFO == grpIdx)
            {
                eventPrms.eventMasks = CSIRX_EVENT_TYPE_INFO_ALL;
            }
            else
            {
                /* ESM groups */
                eventPrms.eventMasks =
                    CSIRX_EVENT_TYPE_ASF_PROTOCOL_ERR;
            }

            retVal = Fvid2_control(drvHandle,
                                   IOCTL_CSIRX_REGISTER_EVENT,
                                   &eventPrms,
                                   NULL);

            if (FVID2_SOK == retVal)
            {
                (void)Fvid2_control(drvHandle,
                                    IOCTL_CSIRX_UNREGISTER_EVENT,
                                    (void *)(uintptr_t)grpIdx,
                                    NULL);
            }

        }

        /* To hit 'default' in switch condition of CsirxDrv_eventGroupAllocResource */
        Csirx_eventPrmsInit(&eventPrms);
        eventPrms.eventGroup = grpIdx;
        retVal = Fvid2_control(drvHandle,
                                   IOCTL_CSIRX_REGISTER_EVENT,
                                   &eventPrms,
                                   NULL);
        if(retVal != FVID2_EBADARGS)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }

        retVal = CsirxDrv_eventEnable(drvHandle,
                                   eventPrms.eventGroup,
                                   eventPrms.eventMasks);
        if(retVal != FVID2_EBADARGS)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }

        retVal = CsirxDrv_eventDisable(drvHandle,
                                   eventPrms.eventGroup,
                                   eventPrms.eventMasks);
        if(retVal != FVID2_EBADARGS)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }

        /* To hit 'default' in switch condition of CsirxDrv_eventGroupAllocResource */
        instObj->drvInstId = instObj->drvInstId+2;
        retVal = Fvid2_control(drvHandle,
                                   IOCTL_CSIRX_REGISTER_EVENT,
                                   &eventPrms,
                                   NULL);
        if(retVal != FVID2_EBADARGS)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }

        instObj->drvInstId = savedDrvInstId;

#if defined(SOC_J784S4) || defined(SOC_J742S2)
        instObj->drvInstId = CSIRX_INSTANCE_ID_2;

        for (grpIdx = 0U; grpIdx < CSIRX_EVENT_GROUP_MAX; grpIdx++)
        {
            Csirx_eventPrmsInit(&eventPrms);
            eventPrms.eventGroup = grpIdx;
            if (CSIRX_EVENT_GROUP_ERROR == grpIdx)
            {
                eventPrms.eventMasks = CSIRX_EVENT_TYPE_ERR_ALL;
            }
            else if (CSIRX_EVENT_GROUP_CP_INTD == grpIdx)
            {
                eventPrms.eventMasks = CSIRX_EVENT_TYPE_CP_INTD_ALL;
            }
            else if (CSIRX_EVENT_GROUP_INFO == grpIdx)
            {
                eventPrms.eventMasks = CSIRX_EVENT_TYPE_INFO_ALL;
            }
            else
            {
                eventPrms.eventMasks = CSIRX_EVENT_TYPE_ASF_PROTOCOL_ERR;
            }

            retVal = Fvid2_control(drvHandle,
                                   IOCTL_CSIRX_REGISTER_EVENT,
                                   &eventPrms,
                                   NULL);
            if (FVID2_SOK == retVal)
            {
                (void)Fvid2_control(drvHandle,
                                    IOCTL_CSIRX_UNREGISTER_EVENT,
                                    (void *)(uintptr_t)grpIdx,
                                    NULL);
            }
            else
            {
                GT_1trace(gAppTrace, GT_ERR,
                          " CSIRX_COV: EventReg - INST_2 grp %d register"
                          " failed\r\n", grpIdx);
                testResult = CSIRX_COV_TEST_FAIL;
            }
        }

        /* Default sub-case (lines 372-373): invalid event group */
        Csirx_eventPrmsInit(&eventPrms);
        eventPrms.eventGroup = CSIRX_EVENT_GROUP_MAX;
        eventPrms.eventMasks = CSIRX_EVENT_TYPE_ERR_ALL;
        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_REGISTER_EVENT,
                               &eventPrms,
                               NULL);
        if (FVID2_EBADARGS != retVal)
        {
            GT_1trace(gAppTrace, GT_ERR,
                      " CSIRX_COV: EventReg - INST_2 default expected"
                      " FVID2_EBADARGS, got 0x%x\r\n", retVal);
            testResult = CSIRX_COV_TEST_FAIL;
        }

        instObj->drvInstId = savedDrvInstId;
#endif

        /* To hit Driver Not in Use condition in CsirxDrv_eventGroupUnRegister */
        instObj->inUse = CSIRX_DRV_USAGE_STATUS_NOT_USED;
        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_UNREGISTER_EVENT,
                               (void *)(uintptr_t)CSIRX_EVENT_GROUP_ERROR,
                               NULL);
        if(retVal != FVID2_EFAIL)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }

        instObj->inUse = CSIRX_DRV_USAGE_STATUS_IN_USE;
    }

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: Event registration edge cases - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

/* CsirxCov_testCheckDphyrxConfig */
static int32_t CsirxCov_testCheckDphyrxConfig(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    int32_t retVal;
    Csirx_DPhyCfg progCfg;
    Csirx_DPhyCfg newCfg;

    /* Matching configs */
    Csirx_initDPhyCfg(&progCfg);
    progCfg.inst               = 0U;
    progCfg.psmClkFreqDiv      = 83U;
    progCfg.leftLaneBandSpeed  = 800U;
    progCfg.rightLaneBandSpeed = 800U;
    progCfg.bandGapTimerVal    = 4U;

    Csirx_initDPhyCfg(&newCfg);
    newCfg.inst               = 0U;
    newCfg.psmClkFreqDiv      = 83U;
    newCfg.leftLaneBandSpeed  = 800U;
    newCfg.rightLaneBandSpeed = 800U;
    newCfg.bandGapTimerVal    = 4U;

    retVal = CsirxDrv_checkDphyrxConfig(&progCfg, &newCfg);
    if (FVID2_SOK != retVal)
    {
        GT_1trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: checkDphy - matching configs returned %d"
                  " (expected SOK)\r\n", retVal);
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* Different inst field */
    Csirx_initDPhyCfg(&newCfg);
    newCfg.inst               = 1U;
    newCfg.psmClkFreqDiv      = 83U;
    newCfg.leftLaneBandSpeed  = 800U;
    newCfg.rightLaneBandSpeed = 800U;
    newCfg.bandGapTimerVal    = 4U;

    retVal = CsirxDrv_checkDphyrxConfig(&progCfg, &newCfg);
    if (FVID2_WNON_RECOMMENDED_PARAMS != retVal)
    {
        GT_1trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: checkDphy - diff inst returned %d"
                  " (expected WNON_RECOMMENDED)\r\n", retVal);
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* Different psmClkFreqDiv */
    Csirx_initDPhyCfg(&newCfg);
    newCfg.inst               = 0U;
    newCfg.psmClkFreqDiv      = 100U;
    newCfg.leftLaneBandSpeed  = 800U;
    newCfg.rightLaneBandSpeed = 800U;
    newCfg.bandGapTimerVal    = 4U;

    retVal = CsirxDrv_checkDphyrxConfig(&progCfg, &newCfg);
    if (FVID2_WNON_RECOMMENDED_PARAMS != retVal)
    {
        GT_1trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: checkDphy - diff psmClk returned %d\r\n",
                  retVal);
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* Different leftLaneBandSpeed */
    Csirx_initDPhyCfg(&newCfg);
    newCfg.inst               = 0U;
    newCfg.psmClkFreqDiv      = 83U;
    newCfg.leftLaneBandSpeed  = 1600U;
    newCfg.rightLaneBandSpeed = 800U;
    newCfg.bandGapTimerVal    = 4U;

    retVal = CsirxDrv_checkDphyrxConfig(&progCfg, &newCfg);
    if (FVID2_WNON_RECOMMENDED_PARAMS != retVal)
    {
        GT_1trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: checkDphy - diff leftBand returned %d\r\n",
                  retVal);
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* Different rightLaneBandSpeed */
    Csirx_initDPhyCfg(&newCfg);
    newCfg.inst               = 0U;
    newCfg.psmClkFreqDiv      = 83U;
    newCfg.leftLaneBandSpeed  = 800U;
    newCfg.rightLaneBandSpeed = 1600U;
    newCfg.bandGapTimerVal    = 4U;

    retVal = CsirxDrv_checkDphyrxConfig(&progCfg, &newCfg);
    if (FVID2_WNON_RECOMMENDED_PARAMS != retVal)
    {
        GT_1trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: checkDphy - diff rightBand returned %d\r\n",
                  retVal);
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* Different bandGapTimerVal */
    Csirx_initDPhyCfg(&newCfg);
    newCfg.inst               = 0U;
    newCfg.psmClkFreqDiv      = 83U;
    newCfg.leftLaneBandSpeed  = 800U;
    newCfg.rightLaneBandSpeed = 800U;
    newCfg.bandGapTimerVal    = 8U;

    retVal = CsirxDrv_checkDphyrxConfig(&progCfg, &newCfg);
    if (FVID2_WNON_RECOMMENDED_PARAMS != retVal)
    {
        GT_1trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: checkDphy - diff bandGap returned %d\r\n",
                  retVal);
        testResult = CSIRX_COV_TEST_FAIL;
    }

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: checkDphyrxConfig test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

/* CsirxCov_testAsfEventGroups */
static int32_t CsirxCov_testAsfEventGroups(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    int32_t retVal;
    Fvid2_Handle drvHandle = NULL;
    Csirx_EventPrms eventPrms;
    uint32_t instId;
    CsirxDrv_InstObj *instObj;

    instId  = taskObj->instObj.instCfgInfo->csiDrvInst;
    instObj = &gCsirxCommonObj.instObj[instId];

    drvHandle = gCovSharedHandle;
    if (NULL == drvHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: AsfEvent - gCovSharedHandle is NULL\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    if (NULL != drvHandle)
    {
        /* Register ASF HI event group with all ASF event types */
        Csirx_eventPrmsInit(&eventPrms);
        eventPrms.eventGroup = CSIRX_ESM_HI_EVENT_GROUP_ASF;
        eventPrms.eventMasks = CSIRX_EVENT_TYPE_ASF_ALL;
        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_REGISTER_EVENT,
                               &eventPrms,
                               NULL);
        if (FVID2_SOK != retVal)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }

        /* Unregister ASF HI */
        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_UNREGISTER_EVENT,
                               (void *)(uintptr_t)CSIRX_ESM_HI_EVENT_GROUP_ASF,
                               NULL);
        if (FVID2_SOK != retVal)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }

        /* Register ASF LOW event group with all ASF event types. */
        Csirx_eventPrmsInit(&eventPrms);
        eventPrms.eventGroup = CSIRX_ESM_LOW_EVENT_GROUP_ASF;
        eventPrms.eventMasks = CSIRX_EVENT_TYPE_ASF_ALL;
        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_REGISTER_EVENT,
                               &eventPrms,
                               NULL);
        if (FVID2_SOK != retVal)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }

        if (FVID2_SOK == retVal)
        {
            (void)HwiP_post(
                instObj->eventObj[CSIRX_ESM_LOW_EVENT_GROUP_ASF].coreIntrNum);
        }

        /* Unregister ASF LOW */
        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_UNREGISTER_EVENT,
                               (void *)(uintptr_t)CSIRX_ESM_LOW_EVENT_GROUP_ASF,
                               NULL);
        if (FVID2_SOK != retVal)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }

        /* Register ASF CFG event group with all ASF event types */
        Csirx_eventPrmsInit(&eventPrms);
        eventPrms.eventGroup = CSIRX_ESM_CFG_EVENT_GROUP_ASF;
        eventPrms.eventMasks = CSIRX_EVENT_TYPE_ASF_ALL;
        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_REGISTER_EVENT,
                               &eventPrms,
                               NULL);
        if (FVID2_SOK != retVal)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }
        else if (CSIRX_INTR_INVALID ==
                 instObj->eventObj[CSIRX_ESM_CFG_EVENT_GROUP_ASF].coreIntrNum)
        {
            /* coreIntrNum was not set by registration - cannot trigger ISR */
            GT_0trace(gAppTrace, GT_ERR,
                      " CSIRX_COV: AsfEvent - coreIntrNum invalid after"
                      " CFG registration\r\n");
            testResult = CSIRX_COV_TEST_FAIL;
        }
        else
        {
            /* Software-trigger the registered ISR (CsirxDrv_asfEsmCfgEventIsrFxn)
             * via HwiP_post. The ISR fires with eventObj as arg; since
             * eventPrms.eventCb is NULL the callback branch is skipped. */
            (void)HwiP_post(
                instObj->eventObj[CSIRX_ESM_CFG_EVENT_GROUP_ASF].coreIntrNum);
        }

        /* Unregister ASF CFG */
        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_UNREGISTER_EVENT,
                               (void *)(uintptr_t)CSIRX_ESM_CFG_EVENT_GROUP_ASF,
                               NULL);
        if (FVID2_SOK != retVal)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }

        /* Register ASF CFG event group with all ASF event types */
        Csirx_eventPrmsInit(&eventPrms);
        eventPrms.eventGroup = CSIRX_EVENT_GROUP_CP_INTD;
        eventPrms.eventMasks = CSIRX_EVENT_TYPE_ASF_ALL;
        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_REGISTER_EVENT,
                               &eventPrms,
                               NULL);
        if (FVID2_SOK != retVal)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }

        if (FVID2_SOK == retVal)
        {
            (void)HwiP_post(
                instObj->eventObj[CSIRX_EVENT_GROUP_CP_INTD].coreIntrNum);
        }

        /* Unregister ASF CFG */
        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_UNREGISTER_EVENT,
                               (void *)(uintptr_t)CSIRX_EVENT_GROUP_CP_INTD,
                               NULL);
        if (FVID2_SOK != retVal)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }

        /*  Register ASF HI event group to set up event infrastructure */
        Csirx_eventPrmsInit(&eventPrms);
        eventPrms.eventGroup = CSIRX_ESM_HI_EVENT_GROUP_ASF;
        eventPrms.eventMasks = CSIRX_EVENT_TYPE_ASF_PROTOCOL_ERR;
        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_REGISTER_EVENT,
                               &eventPrms,
                               NULL);
        if (FVID2_SOK != retVal)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }

        /*  Unregister ASF HI event group to set up event infrastructure */
        retVal = Fvid2_control(drvHandle,
                               IOCTL_CSIRX_UNREGISTER_EVENT,
                               (void *)(uintptr_t)CSIRX_ESM_HI_EVENT_GROUP_ASF,
                               NULL);
        if (FVID2_SOK != retVal)
        {
            testResult = CSIRX_COV_TEST_FAIL;
        }
    }

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: ASF event groups test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

static int32_t CsirxCov_testDphyrxCoreLaneReady(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    uint32_t instId;
    CsirxDrv_InstObj *instObj;

    instId  = taskObj->instObj.instCfgInfo->csiDrvInst;
    instObj = &gCsirxCommonObj.instObj[instId];

    CsirxDrv_dphyrxCoreLaneReady(instObj->dPhyCoreAddr, 1U);

    /* case 2: CL + DL0 + DL1 — both physically active */
    CsirxDrv_dphyrxCoreLaneReady(instObj->dPhyCoreAddr, 2U);

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: dphyrxCoreLaneReady lane cases test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

/* break when Csirx_stopCh fails inside CsirxDrv_stopIoctl channel-stop loop */
static int32_t CsirxCov_testStopChFailBreak(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    Fvid2_Handle drvHandle = gCovSharedHandle;
    CsirxDrv_VirtContext *virtContext;
    CsirxDrv_InstObj *instObj;
    uint32_t instId;

    uint32_t savedNumCh, savedChId0, savedState, savedNumStarted;
    uint32_t savedPsilThread, savedChStatus, savedChType;
    Csirx_ChCfg *savedChCfg;

    if (NULL == drvHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: StopChFailBreak - shared handle NULL\r\n");
        return CSIRX_COV_TEST_FAIL;
    }

    instId = taskObj->instObj.instCfgInfo->csiDrvInst;
    instObj = &gCsirxCommonObj.instObj[instId];
    virtContext = &gCsirxCommonObj.virtContext[instId][0];

    /* Save original state */
    savedNumCh       = virtContext->numCh;
    savedChId0       = virtContext->chId[0];
    savedState       = virtContext->state;
    savedNumStarted  = instObj->numDrvInstStarted;
    savedPsilThread  = instObj->chObj[0].psilThreadId;
    savedChCfg       = instObj->chObj[0].chCfg;
    savedChStatus    = instObj->chObj[0].status;
    savedChType      = instObj->createParams.chCfg[0].chType;

    /* Set up one capture channel with an invalid psilThreadId. */
    virtContext->numCh       = 1U;
    virtContext->chId[0]     = 0U;
    virtContext->state       = CSIRX_DRV_STATE_RUNNING;
    instObj->numDrvInstStarted = 2U;
    instObj->chObj[0].chCfg  = &instObj->createParams.chCfg[0];
    instObj->createParams.chCfg[0].chType = CSIRX_CH_TYPE_CAPT;
    instObj->chObj[0].psilThreadId = 0xFFFFU;
    instObj->chObj[0].status = CSIRX_DRV_CH_STATE_RUNNING;

    /* Fvid2_stop -> CsirxDrv_stopIoctl -> Csirx_stopCh fails -> break */
    (void)Fvid2_stop(drvHandle, NULL);

    /* Restore all saved state */
    virtContext->numCh                    = savedNumCh;
    virtContext->chId[0]                  = savedChId0;
    virtContext->state                    = savedState;
    instObj->numDrvInstStarted            = savedNumStarted;
    instObj->chObj[0].psilThreadId        = savedPsilThread;
    instObj->chObj[0].chCfg               = savedChCfg;
    instObj->chObj[0].status              = savedChStatus;
    instObj->createParams.chCfg[0].chType = savedChType;

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: StopChFailBreak test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

/* Exercises CsirxDrv_infoHandler via the registered function pointer. */
static int32_t CsirxCov_testHandlers(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    uint32_t instId;
    CsirxDrv_InstObj *instObj;
    CSIRX_InfoIrqs InfoVal;
    CSIRX_MonitorIrqs MonitorVal;
    CSIRX_ErrorIrqs ErrorVal;
    CSIRX_DphyErrStatusIrq DphyVal;
    CSIRX_AsfIrqs AsfVal;

    instId  = taskObj->instObj.instCfgInfo->csiDrvInst;
    instObj = &gCsirxCommonObj.instObj[instId];

    memset(&InfoVal, 0, sizeof(InfoVal));
    memset(&MonitorVal, 0, sizeof(MonitorVal));
    memset(&ErrorVal, 0, sizeof(ErrorVal));
    memset(&DphyVal, 0, sizeof(DphyVal));
    memset(&AsfVal, 0, sizeof(AsfVal));

    if (NULL == instObj->cslObj.intrHandlers.infoHandler)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: InfoHandler - handler not registered\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }
    else
    {
        instObj->cslObj.intrHandlers.infoHandler(
            &instObj->cslObj.cslCfgData,
            &InfoVal);
    }

    if (NULL == instObj->cslObj.intrHandlers.monitorHandler)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: InfoHandler - handler not registered\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }
    else
    {
        instObj->cslObj.intrHandlers.monitorHandler(
            &instObj->cslObj.cslCfgData,
            &MonitorVal);
    }

    if (NULL == instObj->cslObj.intrHandlers.errorHandler)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: InfoHandler - handler not registered\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }
    else
    {
        instObj->cslObj.intrHandlers.errorHandler(
            &instObj->cslObj.cslCfgData,
            &ErrorVal);
    }

    if (NULL == instObj->cslObj.intrHandlers.dphyHandler)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: InfoHandler - handler not registered\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }
    else
    {
        instObj->cslObj.intrHandlers.dphyHandler(
            &instObj->cslObj.cslCfgData,
            &DphyVal);
    }

    if (NULL == instObj->cslObj.intrHandlers.asfEsmHandler)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: InfoHandler - handler not registered\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }
    else
    {
        instObj->cslObj.intrHandlers.asfEsmHandler(
            &instObj->cslObj.cslCfgData,
            &AsfVal);
    }

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: InfoHandler test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

/* Dummy callback used by CsirxCov_testErrorEventIsrPaths to cover the
 * eventCb invocation path (csirx_event.c lines 873-876). */
static void CsirxCov_errorEventCb(Csirx_EventStatus evtStatus, void *appData)
{
    (void)evtStatus;
    (void)appData;
}

static int32_t CsirxCov_testErrorEventIsrPaths(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    int32_t retVal;
    Fvid2_Handle drvHandle;
    Csirx_EventPrms eventPrms;
    uint32_t instId;
    CsirxDrv_InstObj *instObj;
    CSIRX_Regs fakeRegs;
    CSIRX_Regs *savedRegs;

    instId    = taskObj->instObj.instCfgInfo->csiDrvInst;
    instObj   = &gCsirxCommonObj.instObj[instId];
    drvHandle = gCovSharedHandle;

    if (NULL == drvHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: ErrIsr - gCovSharedHandle is NULL\r\n");
        return CSIRX_COV_TEST_FAIL;
    }

    /* Register ERROR event with a non-NULL callback so lines 873-876
     * (eventCb invocation) are reached when eventMasks becomes non-zero. */
    Csirx_eventPrmsInit(&eventPrms);
    eventPrms.eventGroup = CSIRX_EVENT_GROUP_ERROR;
    eventPrms.eventMasks = CSIRX_EVENT_TYPE_ERR_ALL;
    eventPrms.eventCb    = CsirxCov_errorEventCb;
    eventPrms.appData    = (void *)taskObj;
    retVal = Fvid2_control(drvHandle,
                           IOCTL_CSIRX_REGISTER_EVENT,
                           &eventPrms,
                           NULL);
    if (FVID2_SOK != retVal)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: ErrIsr - register event failed\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    if ((FVID2_SOK == retVal) &&
        (CSIRX_INTR_INVALID !=
         instObj->eventObj[CSIRX_EVENT_GROUP_ERROR].coreIntrNum))
    {
        /* Build a fake register bank with the error bits that exercise
         * each target branch in CsirxDrv_errorEventIsrFxn */
        memset(&fakeRegs, 0U, sizeof(fakeRegs));
        /* Include stream2/3 fifoOverflow bits (bits 18-19) so that
         * the fifoOverflowIrq[2] and fifoOverflowIrq[3] branches
         * (csirx_event.c lines 854-858, 864-868) are exercised. */
        fakeRegs.error_irqs = 0x000F03F1U |
            CSIRX__ERROR_IRQS__STREAM2_FIFO_OVERFLOW_IRQ_MASK |
            CSIRX__ERROR_IRQS__STREAM3_FIFO_OVERFLOW_IRQ_MASK;
        /* Set RUNNING bit (bit 31) for all four streams so that
         * CsirxDrv_resetStream enters the running-stream block
         * (csirx_event.c lines 1165-1223) for every stream reset.
         * readyState (bits 28-30) and streamFsm (bits 24-27) are left
         * zero so the idle-wait loop exits immediately. */
        fakeRegs.stream0_status = 0x80000000U;
        fakeRegs.stream1_status = 0x80000000U;
        fakeRegs.stream2_status = 0x80000000U;
        fakeRegs.stream3_status = 0x80000000U;

        savedRegs = instObj->cslObj.cslCfgData.regs;
        instObj->cslObj.cslCfgData.regs = &fakeRegs;

        /* HwiP_post fires the registered ISR (CsirxDrv_errorEventIsrFxn)
         * synchronously before returning, so the fake regs are in place
         * for the entire ISR execution. */
        (void)HwiP_post(
            instObj->eventObj[CSIRX_EVENT_GROUP_ERROR].coreIntrNum);

        instObj->cslObj.cslCfgData.regs = savedRegs;
    }
    else if (FVID2_SOK == retVal)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: ErrIsr - coreIntrNum invalid after"
                  " ERROR event registration\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* Unregister ERROR event */
    (void)Fvid2_control(drvHandle,
                        IOCTL_CSIRX_UNREGISTER_EVENT,
                        (void *)(uintptr_t)CSIRX_EVENT_GROUP_ERROR,
                        NULL);

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: Error event ISR paths test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

/* Covers all three if-branches and the callback in
 * CsirxDrv_cpIntdEventIsrFxn by redirecting instObj->cpIntdBaseAddr to
 * a fake CSL_csi_rx_intd_cfgRegs whose STATUS_REG_LEVEL_0 has all three
 * CP_INTD event bits set (0x00000007). */
static int32_t CsirxCov_testCpIntdEventIsrPaths(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    int32_t retVal;
    Fvid2_Handle drvHandle;
    Csirx_EventPrms eventPrms;
    uint32_t instId;
    CsirxDrv_InstObj *instObj;
    CSL_csi_rx_intd_cfgRegs fakeIntd;
    uint32_t savedCpIntdBaseAddr;

    instId    = taskObj->instObj.instCfgInfo->csiDrvInst;
    instObj   = &gCsirxCommonObj.instObj[instId];
    drvHandle = gCovSharedHandle;

    if (NULL == drvHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: CpIntdIsr - gCovSharedHandle is NULL\r\n");
        return CSIRX_COV_TEST_FAIL;
    }

    Csirx_eventPrmsInit(&eventPrms);
    eventPrms.eventGroup = CSIRX_EVENT_GROUP_CP_INTD;
    eventPrms.eventMasks = CSIRX_EVENT_TYPE_CP_INTD_ALL;
    eventPrms.eventCb    = CsirxCov_errorEventCb;
    eventPrms.appData    = (void *)taskObj;
    retVal = Fvid2_control(drvHandle,
                           IOCTL_CSIRX_REGISTER_EVENT,
                           &eventPrms,
                           NULL);
    if (FVID2_SOK != retVal)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: CpIntdIsr - register event failed\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    if ((FVID2_SOK == retVal) &&
        (CSIRX_INTR_INVALID !=
         instObj->eventObj[CSIRX_EVENT_GROUP_CP_INTD].coreIntrNum))
    {
        /* STATUS_REG_LEVEL_0 bits (CSIRX_INTD_INT_TYPE_LEVEL path):
         *   0x00000001 = FIFO_OVERFLOW
         *   0x00000002 = VP0_ERRLNFRM
         *   0x00000004 = VP1_ERRLNFRM                                    */
        memset(&fakeIntd, 0U, sizeof(fakeIntd));
        fakeIntd.STATUS_REG_LEVEL_0 = CSIRX_EVENT_TYPE_CP_INTD_ALL;

        savedCpIntdBaseAddr     = instObj->cpIntdBaseAddr;
        instObj->cpIntdBaseAddr = (uint32_t)(uintptr_t)&fakeIntd;

        (void)HwiP_post(
            instObj->eventObj[CSIRX_EVENT_GROUP_CP_INTD].coreIntrNum);

        instObj->cpIntdBaseAddr = savedCpIntdBaseAddr;
    }
    else if (FVID2_SOK == retVal)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: CpIntdIsr - coreIntrNum invalid after"
                  " CP_INTD event registration\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    (void)Fvid2_control(drvHandle,
                        IOCTL_CSIRX_UNREGISTER_EVENT,
                        (void *)(uintptr_t)CSIRX_EVENT_GROUP_CP_INTD,
                        NULL);

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: CP INTD event ISR paths test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

/* Covers the deskewEntryIrq branch and the callback in
 * CsirxDrv_infoEventIsrFxn by redirecting instObj->cslObj.cslCfgData.regs
 * to a fake CSIRX_Regs with info_irqs bit 5 (0x00000020) set. */
static int32_t CsirxCov_testInfoEventIsrPaths(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    int32_t retVal;
    Fvid2_Handle drvHandle;
    Csirx_EventPrms eventPrms;
    uint32_t instId;
    CsirxDrv_InstObj *instObj;
    CSIRX_Regs fakeRegs;
    CSIRX_Regs *savedRegs;

    instId    = taskObj->instObj.instCfgInfo->csiDrvInst;
    instObj   = &gCsirxCommonObj.instObj[instId];
    drvHandle = gCovSharedHandle;

    if (NULL == drvHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: InfoIsr - gCovSharedHandle is NULL\r\n");
        return CSIRX_COV_TEST_FAIL;
    }

    Csirx_eventPrmsInit(&eventPrms);
    eventPrms.eventGroup = CSIRX_EVENT_GROUP_INFO;
    eventPrms.eventMasks = CSIRX_EVENT_TYPE_INFO_ALL;
    eventPrms.eventCb    = CsirxCov_errorEventCb;
    eventPrms.appData    = (void *)taskObj;
    retVal = Fvid2_control(drvHandle,
                           IOCTL_CSIRX_REGISTER_EVENT,
                           &eventPrms,
                           NULL);
    if (FVID2_SOK != retVal)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: InfoIsr - register event failed\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    if ((FVID2_SOK == retVal) &&
        (CSIRX_INTR_INVALID !=
         instObj->eventObj[CSIRX_EVENT_GROUP_INFO].coreIntrNum))
    {
        /* info_irqs bit 5 (0x00000020) = deskewEntryIrq ->
         * covers the single branch in CsirxDrv_infoEventIsrFxn          */
        memset(&fakeRegs, 0U, sizeof(fakeRegs));
        fakeRegs.info_irqs = 0x00000020U;

        savedRegs = instObj->cslObj.cslCfgData.regs;
        instObj->cslObj.cslCfgData.regs = &fakeRegs;

        (void)HwiP_post(
            instObj->eventObj[CSIRX_EVENT_GROUP_INFO].coreIntrNum);

        instObj->cslObj.cslCfgData.regs = savedRegs;
    }
    else if (FVID2_SOK == retVal)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: InfoIsr - coreIntrNum invalid after"
                  " INFO event registration\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    (void)Fvid2_control(drvHandle,
                        IOCTL_CSIRX_UNREGISTER_EVENT,
                        (void *)(uintptr_t)CSIRX_EVENT_GROUP_INFO,
                        NULL);

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: Info event ISR paths test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

/* Covers all 7 ASF branches and the callback in
 * CsirxDrv_asfEsmHighEventIsrFxn.  The ISR loops over ALL instances, so
 * every instance's regs pointer is redirected to a fake CSIRX_Regs with
 * asf_int_status = 0x0000007F (all 7 ASF bits). */
static int32_t CsirxCov_testAsfEsmHighEventIsrPaths(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    int32_t retVal;
    Fvid2_Handle drvHandle;
    Csirx_EventPrms eventPrms;
    uint32_t instId;
    CsirxDrv_InstObj *instObj;
    CSIRX_Regs fakeRegs[CSIRX_INSTANCE_ID_MAX];
    CSIRX_Regs *savedRegs[CSIRX_INSTANCE_ID_MAX];
    uint32_t i;

    instId    = taskObj->instObj.instCfgInfo->csiDrvInst;
    instObj   = &gCsirxCommonObj.instObj[instId];
    drvHandle = gCovSharedHandle;

    if (NULL == drvHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: AsfHiIsr - gCovSharedHandle is NULL\r\n");
        return CSIRX_COV_TEST_FAIL;
    }

    Csirx_eventPrmsInit(&eventPrms);
    eventPrms.eventGroup = CSIRX_ESM_HI_EVENT_GROUP_ASF;
    eventPrms.eventMasks = CSIRX_EVENT_TYPE_ASF_ALL;
    eventPrms.eventCb    = CsirxCov_errorEventCb;
    eventPrms.appData    = (void *)taskObj;
    retVal = Fvid2_control(drvHandle,
                           IOCTL_CSIRX_REGISTER_EVENT,
                           &eventPrms,
                           NULL);
    if (FVID2_SOK != retVal)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: AsfHiIsr - register event failed\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    if ((FVID2_SOK == retVal) &&
        (CSIRX_INTR_INVALID !=
         instObj->eventObj[CSIRX_ESM_HI_EVENT_GROUP_ASF].coreIntrNum))
    {
        for (i = 0U; i < CSIRX_INSTANCE_ID_MAX; i++)
        {
            memset(&fakeRegs[i], 0U, sizeof(fakeRegs[i]));
            fakeRegs[i].asf_int_status = 0x0000007FU;
            savedRegs[i] = gCsirxCommonObj.instObj[i].cslObj.cslCfgData.regs;
            gCsirxCommonObj.instObj[i].cslObj.cslCfgData.regs = &fakeRegs[i];
        }

        (void)HwiP_post(
            instObj->eventObj[CSIRX_ESM_HI_EVENT_GROUP_ASF].coreIntrNum);

        for (i = 0U; i < CSIRX_INSTANCE_ID_MAX; i++)
        {
            gCsirxCommonObj.instObj[i].cslObj.cslCfgData.regs = savedRegs[i];
        }
    }
    else if (FVID2_SOK == retVal)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: AsfHiIsr - coreIntrNum invalid after"
                  " ASF HI event registration\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    (void)Fvid2_control(drvHandle,
                        IOCTL_CSIRX_UNREGISTER_EVENT,
                        (void *)(uintptr_t)CSIRX_ESM_HI_EVENT_GROUP_ASF,
                        NULL);

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: ASF ESM High event ISR paths test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

/* Mirrors CsirxCov_testAsfEsmHighEventIsrPaths for
 * CsirxDrv_asfEsmLowEventIsrFxn (CSIRX_ESM_LOW_EVENT_GROUP_ASF). */
static int32_t CsirxCov_testAsfEsmLowEventIsrPaths(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    int32_t retVal;
    Fvid2_Handle drvHandle;
    Csirx_EventPrms eventPrms;
    uint32_t instId;
    CsirxDrv_InstObj *instObj;
    CSIRX_Regs fakeRegs[CSIRX_INSTANCE_ID_MAX];
    CSIRX_Regs *savedRegs[CSIRX_INSTANCE_ID_MAX];
    uint32_t i;

    instId    = taskObj->instObj.instCfgInfo->csiDrvInst;
    instObj   = &gCsirxCommonObj.instObj[instId];
    drvHandle = gCovSharedHandle;

    if (NULL == drvHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: AsfLoIsr - gCovSharedHandle is NULL\r\n");
        return CSIRX_COV_TEST_FAIL;
    }

    Csirx_eventPrmsInit(&eventPrms);
    eventPrms.eventGroup = CSIRX_ESM_LOW_EVENT_GROUP_ASF;
    eventPrms.eventMasks = CSIRX_EVENT_TYPE_ASF_ALL;
    eventPrms.eventCb    = CsirxCov_errorEventCb;
    eventPrms.appData    = (void *)taskObj;
    retVal = Fvid2_control(drvHandle,
                           IOCTL_CSIRX_REGISTER_EVENT,
                           &eventPrms,
                           NULL);
    if (FVID2_SOK != retVal)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: AsfLoIsr - register event failed\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    if ((FVID2_SOK == retVal) &&
        (CSIRX_INTR_INVALID !=
         instObj->eventObj[CSIRX_ESM_LOW_EVENT_GROUP_ASF].coreIntrNum))
    {
        for (i = 0U; i < CSIRX_INSTANCE_ID_MAX; i++)
        {
            memset(&fakeRegs[i], 0U, sizeof(fakeRegs[i]));
            fakeRegs[i].asf_int_status = 0x0000007FU;
            savedRegs[i] = gCsirxCommonObj.instObj[i].cslObj.cslCfgData.regs;
            gCsirxCommonObj.instObj[i].cslObj.cslCfgData.regs = &fakeRegs[i];
        }

        (void)HwiP_post(
            instObj->eventObj[CSIRX_ESM_LOW_EVENT_GROUP_ASF].coreIntrNum);

        for (i = 0U; i < CSIRX_INSTANCE_ID_MAX; i++)
        {
            gCsirxCommonObj.instObj[i].cslObj.cslCfgData.regs = savedRegs[i];
        }
    }
    else if (FVID2_SOK == retVal)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: AsfLoIsr - coreIntrNum invalid after"
                  " ASF LOW event registration\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    (void)Fvid2_control(drvHandle,
                        IOCTL_CSIRX_UNREGISTER_EVENT,
                        (void *)(uintptr_t)CSIRX_ESM_LOW_EVENT_GROUP_ASF,
                        NULL);

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: ASF ESM Low event ISR paths test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

/* Covers the callback branch in CsirxDrv_asfEsmCfgEventIsrFxn. */
static int32_t CsirxCov_testAsfEsmCfgEventIsrPaths(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    int32_t retVal;
    Fvid2_Handle drvHandle;
    Csirx_EventPrms eventPrms;
    uint32_t instId;
    CsirxDrv_InstObj *instObj;

    instId    = taskObj->instObj.instCfgInfo->csiDrvInst;
    instObj   = &gCsirxCommonObj.instObj[instId];
    drvHandle = gCovSharedHandle;

    if (NULL == drvHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: AsfCfgIsr - gCovSharedHandle is NULL\r\n");
        return CSIRX_COV_TEST_FAIL;
    }

    Csirx_eventPrmsInit(&eventPrms);
    eventPrms.eventGroup = CSIRX_ESM_CFG_EVENT_GROUP_ASF;
    eventPrms.eventMasks = CSIRX_EVENT_TYPE_ASF_ALL;
    eventPrms.eventCb    = CsirxCov_errorEventCb;
    eventPrms.appData    = (void *)taskObj;
    retVal = Fvid2_control(drvHandle,
                           IOCTL_CSIRX_REGISTER_EVENT,
                           &eventPrms,
                           NULL);
    if (FVID2_SOK != retVal)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: AsfCfgIsr - register event failed\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    if ((FVID2_SOK == retVal) &&
        (CSIRX_INTR_INVALID !=
         instObj->eventObj[CSIRX_ESM_CFG_EVENT_GROUP_ASF].coreIntrNum))
    {
        /* No register reads in CsirxDrv_asfEsmCfgEventIsrFxn; triggering
         * with a non-NULL eventCb covers the callback branch.            */
        (void)HwiP_post(
            instObj->eventObj[CSIRX_ESM_CFG_EVENT_GROUP_ASF].coreIntrNum);
    }
    else if (FVID2_SOK == retVal)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: AsfCfgIsr - coreIntrNum invalid after"
                  " ASF CFG event registration\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    (void)Fvid2_control(drvHandle,
                        IOCTL_CSIRX_UNREGISTER_EVENT,
                        (void *)(uintptr_t)CSIRX_ESM_CFG_EVENT_GROUP_ASF,
                        NULL);

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: ASF ESM CFG event ISR paths test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

/* Covers IOCTL_CSIRX_CH_START and IOCTL_CSIRX_CH_STOP in csirx_drv.c */
static int32_t CsirxCov_testChStartStopIoctl(CsirxTestTaskObj *taskObj)
{
    int32_t retVal = FVID2_SOK;
    int32_t testResult = CSIRX_COV_TEST_PASS;
    Fvid2_Handle drvHandle = gCovSharedHandle;
    CsirxDrv_VirtContext *virtContext;
    CsirxDrv_InstObj *instObj;
    uint32_t instId;
    uint32_t chIdx;

    /* Saved state for restore */
    uint32_t savedNumCh;
    uint32_t savedChId0;
    uint32_t savedChStatus;
    uint32_t savedChType;
    Csirx_ChCfg *savedChCfg;

    if (NULL == drvHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: ChStartStop - gCovSharedHandle is NULL\r\n");
        return CSIRX_COV_TEST_FAIL;
    }

    instId      = taskObj->instObj.instCfgInfo->csiDrvInst;
    instObj     = &gCsirxCommonObj.instObj[instId];
    virtContext = &gCsirxCommonObj.virtContext[instId][0];

    /* Save original state */
    savedNumCh    = virtContext->numCh;
    savedChId0    = virtContext->chId[0];
    savedChCfg    = instObj->chObj[0].chCfg;
    savedChStatus = instObj->chObj[0].status;
    savedChType   = instObj->createParams.chCfg[0].chType;

    /* Test 1: IOCTL_CSIRX_CH_START with out-of-range channel index.
     * With numCh=1, any chIdx >= 1 takes the FVID2_EBADARGS branch
     * (csirx_drv.c lines 904-906). */
    virtContext->numCh = 1U;
    virtContext->chId[0] = 0U;
    chIdx = 1U;
    retVal = Fvid2_control(drvHandle, IOCTL_CSIRX_CH_START, &chIdx, NULL);
    if (FVID2_EBADARGS != retVal)
    {
        GT_1trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: ChStartStop - START bad-idx: expected"
                  " FVID2_EBADARGS, got 0x%x\r\n", retVal);
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* Test 2: IOCTL_CSIRX_CH_START with valid channel index.
     * Uses CSIRX_CH_TYPE_OTF so Csirx_startCh bypasses the UDMA ring-queue
     * path entirely (csirx_drv.c lines 910-911 + 2002 else-branch).      */
    instObj->chObj[0].chCfg = &instObj->createParams.chCfg[0];
    instObj->createParams.chCfg[0].chType = CSIRX_CH_TYPE_OTF;
    instObj->chObj[0].status = CSIRX_DRV_CH_STATE_CREATED;
    chIdx = 0U;
    retVal = Fvid2_control(drvHandle, IOCTL_CSIRX_CH_START, &chIdx, NULL);
    if (FVID2_SOK != retVal)
    {
        GT_1trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: ChStartStop - CH_START failed: 0x%x\r\n",
                  retVal);
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* Test 3: IOCTL_CSIRX_CH_STOP with out-of-range channel index
     * (csirx_drv.c lines 915-917). */
    chIdx = 1U;
    retVal = Fvid2_control(drvHandle, IOCTL_CSIRX_CH_STOP, &chIdx, NULL);
    if (FVID2_EBADARGS != retVal)
    {
        GT_1trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: ChStartStop - STOP bad-idx: expected"
                  " FVID2_EBADARGS, got 0x%x\r\n", retVal);
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* Test 4: IOCTL_CSIRX_CH_STOP with valid channel index.
     * Channel status is RUNNING after the successful start above.
     * Csirx_stopCh skips UDMA disable for non-CAPT channels
     * (csirx_drv.c lines 921-922). */
    chIdx = 0U;
    retVal = Fvid2_control(drvHandle, IOCTL_CSIRX_CH_STOP, &chIdx, NULL);
    if (FVID2_SOK != retVal)
    {
        GT_1trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: ChStartStop - CH_STOP failed: 0x%x\r\n",
                  retVal);
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* Restore all state so the shared handle cleanup works correctly */
    virtContext->numCh                    = savedNumCh;
    virtContext->chId[0]                  = savedChId0;
    instObj->chObj[0].chCfg               = savedChCfg;
    instObj->chObj[0].status              = savedChStatus;
    instObj->createParams.chCfg[0].chType = savedChType;

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: Channel Start/Stop IOCTL test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

static int32_t CsirxCov_testQueueDequeueEdgeCases(CsirxTestTaskObj *taskObj)
{
    int32_t retVal;
    int32_t testResult = CSIRX_COV_TEST_PASS;
    Fvid2_Handle drvHandle = gCovSharedHandle;
    CsirxDrv_VirtContext *virtContext;
    CsirxDrv_InstObj *instObj;
    uint32_t instId;
    uint32_t savedState;
    Fvid2_FrameList frmList;
    Fvid2_Frame frame;

    if (NULL == drvHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: QueueDequeue - gCovSharedHandle NULL\r\n");
        return CSIRX_COV_TEST_FAIL;
    }

    instId = taskObj->instObj.instCfgInfo->csiDrvInst;
    instObj = &gCsirxCommonObj.instObj[instId];

    virtContext = &gCsirxCommonObj.virtContext[instId][0U];
    savedState = virtContext->state;

    /* Build a minimal valid frame list */
    memset(&frmList, 0, sizeof(frmList));
    memset(&frame,   0, sizeof(frame));
    frmList.numFrames = 1U;
    frmList.frames[0] = &frame;

    /* Test 1 (line 556): queue on IDLE-state handle → FVID2_EFAIL */
    virtContext->state = CSIRX_DRV_STATE_IDLE;
    retVal = Fvid2_queue(drvHandle, &frmList, 0U);
    if (FVID2_EFAIL != retVal)
    {
        GT_1trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: Queue IDLE - expected FVID2_EFAIL, got 0x%x\r\n",
                  retVal);
        testResult = CSIRX_COV_TEST_FAIL;
    }
    virtContext->state = savedState;

    /* Test 2 (line 687): dequeue on IDLE-state handle → FVID2_EFAIL */
    virtContext->state = CSIRX_DRV_STATE_IDLE;
    retVal = Fvid2_dequeue(drvHandle, &frmList, 0U, FVID2_TIMEOUT_NONE);
    if (FVID2_EFAIL != retVal)
    {
        GT_1trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: Dequeue IDLE - expected FVID2_EFAIL, got 0x%x\r\n",
                  retVal);
        testResult = CSIRX_COV_TEST_FAIL;
    }
    virtContext->state = savedState;

    /* Test 3 (lines 830-833): dequeue on RUNNING handle with numCh=0 → FVID2_EAGAIN.
     * gCovSharedHandle has numCh=0, so doneQ loop runs 0 times, numFrames stays
     * 0, and state RUNNING → FVID2_EAGAIN (not CREATED/STOPPED). */
    virtContext->state = CSIRX_DRV_STATE_RUNNING;
    retVal = Fvid2_dequeue(drvHandle, &frmList, 0U, FVID2_TIMEOUT_NONE);
    if (FVID2_EAGAIN != retVal)
    {
        GT_1trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: Dequeue RUNNING - expected FVID2_EAGAIN,"
                  " got 0x%x\r\n", retVal);
        testResult = CSIRX_COV_TEST_FAIL;
    }
    virtContext->state = savedState;

    /* Test 4 (lines 591-594): queue when freeQ is exhausted → FVID2_EALLOC */
    {
        Fvid2_Handle captHandle = NULL;
        Csirx_CreateParams captCreateParams;
        Csirx_CreateStatus captCreateStatus;
        Fvid2_CbParams cbPrms;
        uint32_t chId;
        uint32_t qIdx;

        CsirxCov_initCreateParams(&captCreateParams, 1U,
                                  CSIRX_CH_TYPE_CAPT,
                                  FVID2_CSI2_DF_RAW12);
        Fvid2CbParams_init(&cbPrms);
        cbPrms.cbFxn   = &CsirxCov_frameCompletionCb;
        cbPrms.appData = (Ptr)taskObj;

        captHandle = Fvid2_create(CSIRX_CAPT_DRV_ID, instId,
                                  (void *)&captCreateParams,
                                  (void *)&captCreateStatus, &cbPrms);
        if (NULL == captHandle)
        {
            GT_0trace(gAppTrace, GT_ERR,
                      " CSIRX_COV: QueueEalloc - 1-ch handle creation FAILED\r\n");
            testResult = CSIRX_COV_TEST_FAIL;
        }
        else
        {
            /* captHandle is the second create for instId; slot 0 is taken
             * by gCovSharedHandle, so captHandle occupies slot 1.         */
            CsirxDrv_VirtContext *captVirt = &gCsirxCommonObj.virtContext[instId][1U];
            chId = captVirt->chId[0];

            /* Drain freeQ entirely: CSIRX_CAPT_QUEUE_DEPTH_PER_CH = 10 */
            for (qIdx = 0U; qIdx < CSIRX_CAPT_QUEUE_DEPTH_PER_CH; qIdx++)
            {
                (void)Fvid2Utils_dequeue(instObj->chObj[chId].bufManObj.freeQ);
            }

            /* Queue a frame: freeQ is empty → lines 591-594 hit */
            frame.chNum    = 0U;
            frmList.frames[0] = &frame;
            frmList.numFrames = 1U;

            retVal = Fvid2_queue(captHandle, &frmList, 0U);
            if (FVID2_EALLOC != retVal)
            {
                GT_1trace(gAppTrace, GT_ERR,
                          " CSIRX_COV: QueueEalloc - expected FVID2_EALLOC,"
                          " got 0x%x\r\n", retVal);
                testResult = CSIRX_COV_TEST_FAIL;
            }

            /* Cleanup: unregister cqEventObj before delete so masterEvent
             * has no child events when gCovSharedHandle cleanup later
             * calls Udma_eventUnRegister(&masterEvent).
             * Then mark channel IDLE so later tests can reuse it.           */
            (void)CsirxDrv_clearUdmaParams(&instObj->chObj[chId]);
            (void)Fvid2_delete(captHandle, NULL);
            captHandle = NULL;
            instObj->chObj[chId].status = CSIRX_DRV_CH_STATE_IDLE;
        }
    }

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: Queue/Dequeue edge cases test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

/* Redirect both cslCfgData.regs AND configParams.regBase on a
 * spare CSIRX instance to a fake register bank whose stream_status fields
 * have readyState and streamFsm both non-zero.  CSIRX_Init (called first
 * inside CsirxDrv_setCslCfgParams) reads configParams.regBase and assigns
 * it to pD->regs, so both pointers must be redirected - redirecting only
 * cslCfgData.regs is insufficient because CSIRX_Init overwrites it.
 * After CSIRX_Init sets pD->regs = &fakeRegs, every subsequent
 * CSIRX_GetStreamStatus call reads fakeRegs.stream0_status (non-idle) →
 * while-loop fires → times out → Fvid2_create returns NULL.
 *
 * Instance 1 is used to avoid corrupting the primary instance state.
 */
static int32_t CsirxCov_testStreamIdleWaitLoop(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    uint32_t instId = taskObj->instObj.instCfgInfo->csiDrvInst;
    uint32_t altInstId = (instId == 0U) ? 1U : 0U;
    CsirxDrv_InstObj *altInstObj = &gCsirxCommonObj.instObj[altInstId];
    CSIRX_Regs *savedRegs;
    CSIRX_Regs *savedRegBase;
    /* Fake register bank: zero-initialised; set stream status non-idle.
     * RUNNING=bit31, READY_STATE=bit8, STREAM_FSM=bits4-7                  */
    static CSIRX_Regs fakeRegs;
    Fvid2_Handle h;
    Csirx_CreateParams createParams;
    Csirx_CreateStatus createStatus;
    Fvid2_CbParams cbPrms;

    memset(&fakeRegs, 0, sizeof(fakeRegs));
    fakeRegs.stream0_status = 0x800001F0U;  /* running=1, readyState=1, FSM=0xF */
    fakeRegs.stream1_status = 0x800001F0U;

    /* Redirect BOTH the cached regs pointer AND the config base address so
     * that CSIRX_Init (which sets pD->regs = configParams.regBase) also
     * ends up pointing at fakeRegs after it runs.                         */
    savedRegs    = altInstObj->cslObj.cslCfgData.regs;
    savedRegBase = altInstObj->cslObj.configParams.regBase;
    altInstObj->cslObj.cslCfgData.regs       = &fakeRegs;
    altInstObj->cslObj.configParams.regBase  = &fakeRegs;

    CsirxCov_initCreateParams(&createParams, 0U,
                              CSIRX_CH_TYPE_CAPT,
                              FVID2_CSI2_DF_RAW12);
    Fvid2CbParams_init(&cbPrms);
    cbPrms.cbFxn   = &CsirxCov_frameCompletionCb;
    cbPrms.appData = (Ptr)taskObj;

    h = Fvid2_create(CSIRX_CAPT_DRV_ID, altInstId,
                     (void *)&createParams,
                     (void *)&createStatus, &cbPrms);

    /* Restore both real pointers before any assertions */
    altInstObj->cslObj.cslCfgData.regs      = savedRegs;
    altInstObj->cslObj.configParams.regBase = savedRegBase;

    if (NULL != h)
    {
        /* Create unexpectedly succeeded - stream status was already idle */
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: StreamIdleWait - Fvid2_create should have"
                  " returned NULL (ETIMEOUT) but succeeded\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
        (void)Fvid2_delete(h, NULL);
    }

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: Stream idle wait loop test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

static int32_t CsirxCov_testCreateEdgeCases(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    uint32_t instId = taskObj->instObj.instCfgInfo->csiDrvInst;
    CsirxDrv_InstObj *instObj = &gCsirxCommonObj.instObj[instId];
    Csirx_CreateParams createParams;
    Csirx_CreateStatus createStatus;
    Fvid2_CbParams cbPrms;
    Fvid2_Handle h;

    Fvid2CbParams_init(&cbPrms);
    cbPrms.cbFxn   = &CsirxCov_frameCompletionCb;
    cbPrms.appData = (Ptr)taskObj;

    /* Sub-test A (lines 2286-2291, 2312, else of 1900-1905):
     * validateCreateParams: inCsiDataType=0xFF → CsirxDrv_getBpp(0xFF)=0
     * → FVID2_EINVALID_PARAMS; breakFlag=TRUE → break (line 2312).       */
    CsirxCov_initCreateParams(&createParams, 1U,
                              CSIRX_CH_TYPE_CAPT,
                              (uint32_t)0xFFU);
    h = Fvid2_create(CSIRX_CAPT_DRV_ID, instId,
                     (void *)&createParams,
                     (void *)&createStatus, &cbPrms);
    if (NULL != h)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: CreateEdge A - expected NULL for invalid DT\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
        (void)Fvid2_delete(h, NULL);
    }

    /* Sub-test B (lines 2300-2308, 2312):
     * validateCreateParams: RAW12 (bpp=12) with numPixelsStrm0=2:
     * numPixel=32/12=2, (1<<2)=4 > 2 → else → FVID2_EINVALID_PARAMS.    */
    CsirxCov_initCreateParams(&createParams, 1U,
                              CSIRX_CH_TYPE_CAPT,
                              FVID2_CSI2_DF_RAW12);
    createParams.instCfg.numPixelsStrm0 = 2U;  /* (1<<2)=4 > numPixel=2 */
    h = Fvid2_create(CSIRX_CAPT_DRV_ID, instId,
                     (void *)&createParams,
                     (void *)&createStatus, &cbPrms);
    if (NULL != h)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: CreateEdge B - expected NULL for bad pixel rate\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
        (void)Fvid2_delete(h, NULL);
    }

    /* Sub-test C (lines 2256, 2262):
     * checkCreateParams: same instCfg scalars but different dataLanesMap[0]
     * → inner lane-loop breaks early (line 2256) → lane mismatch
     * → FVID2_WNON_RECOMMENDED_PARAMS (line 2262) → create returns NULL. */
    CsirxCov_initCreateParams(&createParams, 0U,
                              CSIRX_CH_TYPE_CAPT,
                              FVID2_CSI2_DF_RAW12);
    createParams.instCfg.dataLanesMap[0] =
        (instObj->createParams.instCfg.dataLanesMap[0] == 1U) ? 2U : 1U;
    h = Fvid2_create(CSIRX_CAPT_DRV_ID, instId,
                     (void *)&createParams,
                     (void *)&createStatus, &cbPrms);
    if (NULL != h)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: CreateEdge C - expected NULL for lane mismatch\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
        (void)Fvid2_delete(h, NULL);
    }

    /* Sub-test D (line 2268):
     * checkCreateParams: different numDataLanes → outer if FALSE → else
     * → FVID2_WNON_RECOMMENDED_PARAMS (line 2268) → create returns NULL. */
    CsirxCov_initCreateParams(&createParams, 0U,
                              CSIRX_CH_TYPE_CAPT,
                              FVID2_CSI2_DF_RAW12);
    createParams.instCfg.numDataLanes =
        (instObj->createParams.instCfg.numDataLanes == 2U) ? 4U : 2U;
    h = Fvid2_create(CSIRX_CAPT_DRV_ID, instId,
                     (void *)&createParams,
                     (void *)&createStatus, &cbPrms);
    if (NULL != h)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: CreateEdge D - expected NULL for numDataLanes mismatch\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
        (void)Fvid2_delete(h, NULL);
    }

    /* Sub-test E (lines 1123-1127):
     * enableErrbypass=TRUE path in CsirxDrv_setCslCfgParams.
     * CsirxDrv_checkCreateParams compares instObj->createParams with the
     * incoming createParams.  Temporarily set the stored enableErrbypass
     * field to BTRUE so both sides match, then restore it after the test. */
    {
        uint32_t savedErrBypass = instObj->createParams.instCfg.enableErrbypass;

        instObj->createParams.instCfg.enableErrbypass = (uint32_t)BTRUE;

        CsirxCov_initCreateParams(&createParams, 0U,
                                  CSIRX_CH_TYPE_CAPT,
                                  FVID2_CSI2_DF_RAW12);
        createParams.instCfg.enableErrbypass = (uint32_t)BTRUE;

        h = Fvid2_create(CSIRX_CAPT_DRV_ID, instId,
                         (void *)&createParams,
                         (void *)&createStatus, &cbPrms);

        instObj->createParams.instCfg.enableErrbypass = savedErrBypass;

        if (NULL == h)
        {
            GT_0trace(gAppTrace, GT_ERR,
                      " CSIRX_COV: CreateEdge E - enableErrbypass create FAILED\r\n");
            testResult = CSIRX_COV_TEST_FAIL;
        }
        else
        {
            (void)Fvid2_delete(h, NULL);
        }
    }

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: Create edge cases test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

static int32_t CsirxCov_testGetFreeChNumExhausted(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    uint32_t instId = taskObj->instObj.instCfgInfo->csiDrvInst;
    CsirxDrv_InstObj *instObj = &gCsirxCommonObj.instObj[instId];
    uint32_t savedStatus[CSIRX_NUM_CH];
    uint32_t chIdx;
    Fvid2_Handle h;
    Csirx_CreateParams createParams;
    Csirx_CreateStatus createStatus;
    Fvid2_CbParams cbPrms;

    /* Save and mark every channel non-IDLE */
    for (chIdx = 0U; chIdx < CSIRX_NUM_CH; chIdx++)
    {
        savedStatus[chIdx] = instObj->chObj[chIdx].status;
        instObj->chObj[chIdx].status = CSIRX_DRV_CH_STATE_CREATED;
    }

    CsirxCov_initCreateParams(&createParams, 1U,
                              CSIRX_CH_TYPE_CAPT,
                              FVID2_CSI2_DF_RAW12);
    Fvid2CbParams_init(&cbPrms);
    cbPrms.cbFxn   = &CsirxCov_frameCompletionCb;
    cbPrms.appData = (Ptr)taskObj;

    /* Fvid2_create: getFreeChNumIoctl → FVID2_EAGAIN (line 2199) for
     * the single channel requested; inner if-body is skipped; create
     * still succeeds returning a handle with 0 configured channels.      */
    h = Fvid2_create(CSIRX_CAPT_DRV_ID, instId,
                     (void *)&createParams,
                     (void *)&createStatus, &cbPrms);

    /* Restore all channel statuses */
    for (chIdx = 0U; chIdx < CSIRX_NUM_CH; chIdx++)
    {
        instObj->chObj[chIdx].status = savedStatus[chIdx];
    }

    if (NULL == h)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: GetFreeChNum - Fvid2_create unexpectedly failed\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }
    else
    {
        (void)Fvid2_delete(h, NULL);
    }

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: GetFreeChNum exhausted test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

static int32_t CsirxCov_testVirtContextExhausted(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    Fvid2_Handle handles[CSIRX_NUM_VIRTUAL_CONTEXT];
    Fvid2_Handle extraHandle;
    Csirx_CreateParams createParams;
    Csirx_CreateStatus createStatus;
    Fvid2_CbParams cbPrms;
    uint32_t instId;
    uint32_t idx;

    instId = taskObj->instObj.instCfgInfo->csiDrvInst;

    /* Use numCh=0 so no per-channel UDMA resources (rxChObj/cqEventObj)
     * are allocated - Fvid2_delete needs no special cleanup for these. */
    CsirxCov_initCreateParams(&createParams, 0U,
                              CSIRX_CH_TYPE_CAPT,
                              FVID2_CSI2_DF_RAW12);
    Fvid2CbParams_init(&cbPrms);
    cbPrms.cbFxn   = &CsirxCov_frameCompletionCb;
    cbPrms.appData = (Ptr)taskObj;

    /* Fill all 8 virtual-context slots */
    for (idx = 0U; idx < CSIRX_NUM_VIRTUAL_CONTEXT; idx++)
    {
        handles[idx] = Fvid2_create(CSIRX_CAPT_DRV_ID,
                                    instId,
                                    (void *)&createParams,
                                    (void *)&createStatus,
                                    &cbPrms);
        if (NULL == handles[idx])
        {
            GT_1trace(gAppTrace, GT_ERR,
                      " CSIRX_COV: VirtCtxExhaust - handle[%d] creation"
                      " FAILED unexpectedly\r\n", idx);
            testResult = CSIRX_COV_TEST_FAIL;
            /* Release whatever was opened so far before returning */
            while (idx > 0U)
            {
                idx--;
                (void)Fvid2_delete(handles[idx], NULL);
            }
            return testResult;
        }
    }

    /* 9th create must fail - hits csirx_drv.c line 256 */
    extraHandle = Fvid2_create(CSIRX_CAPT_DRV_ID,
                               instId,
                               (void *)&createParams,
                               (void *)&createStatus,
                               &cbPrms);
    if (NULL != extraHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: VirtCtxExhaust - 9th create should have"
                  " failed (all contexts in use) but succeeded!\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
        (void)Fvid2_delete(extraHandle, NULL);
    }

    /* Release all 8 slots */
    for (idx = 0U; idx < CSIRX_NUM_VIRTUAL_CONTEXT; idx++)
    {
        (void)Fvid2_delete(handles[idx], NULL);
    }

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: Virtual context exhausted test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

/* Covers NULL-handle error paths in delete/queue/dequeue/control
 */
static int32_t CsirxCov_testNullHandleApis(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    int32_t retVal;
    Fvid2_FrameList frmList;

    (void)taskObj;

    memset(&frmList, 0, sizeof(frmList));

    /* NULL handle to delete at FVID2 manager level */
    retVal = Fvid2_delete(NULL, NULL);
    if (FVID2_EBADARGS != retVal)
    {
        GT_1trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: NullHandle - delete expected EBADARGS, got 0x%x\r\n",
                  retVal);
        testResult = CSIRX_COV_TEST_FAIL;
    }

    retVal = CsirxDrv_delete(NULL, NULL);
    if (FVID2_EBADARGS != retVal)
    {
        GT_1trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: NullHandle - CsirxDrv_delete(NULL) expected"
                  " EBADARGS, got 0x%x\r\n", retVal);
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* NULL handle to queue */
    retVal = Fvid2_queue(NULL, &frmList, 0U);
    if (FVID2_EBADARGS != retVal)
    {
        GT_1trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: NullHandle - queue(NULL) expected EBADARGS, got 0x%x\r\n",
                  retVal);
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* NULL frmList to queue (valid handle, NULL list) */
    if (NULL != gCovSharedHandle)
    {
        retVal = Fvid2_queue(gCovSharedHandle, NULL, 0U);
        if (FVID2_EBADARGS != retVal)
        {
            GT_1trace(gAppTrace, GT_ERR,
                      " CSIRX_COV: NullHandle - queue(nullFrmList) expected EBADARGS,"
                      " got 0x%x\r\n", retVal);
            testResult = CSIRX_COV_TEST_FAIL;
        }
    }

    /* NULL handle to dequeue */
    retVal = Fvid2_dequeue(NULL, &frmList, 0U, FVID2_TIMEOUT_NONE);
    if (FVID2_EBADARGS != retVal)
    {
        GT_1trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: NullHandle - dequeue expected EBADARGS, got 0x%x\r\n",
                  retVal);
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* NULL handle to control */
    retVal = Fvid2_control(NULL, FVID2_START, NULL, NULL);
    if (FVID2_EBADARGS != retVal)
    {
        GT_1trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: NullHandle - control expected EBADARGS, got 0x%x\r\n",
                  retVal);
        testResult = CSIRX_COV_TEST_FAIL;
    }

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: Null handle API test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

/* Covers the "driver not in deletable state" error path in CsirxDrv_delete:
 * Injects RUNNING state into the shared handle, calls delete, and restores.
 */
static int32_t CsirxCov_testDeleteRunningHandle(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    int32_t retVal;
    uint32_t instId;
    CsirxDrv_VirtContext *virtContext;
    uint32_t savedState;

    if (NULL == gCovSharedHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: DeleteRunning - gCovSharedHandle is NULL\r\n");
        return CSIRX_COV_TEST_FAIL;
    }

    instId      = taskObj->instObj.instCfgInfo->csiDrvInst;
    virtContext = &gCsirxCommonObj.virtContext[instId][0U];
    savedState  = virtContext->state;

    virtContext->state = CSIRX_DRV_STATE_RUNNING;

    retVal = CsirxDrv_delete((Fdrv_Handle)virtContext, NULL);
    if (FVID2_EFAIL != retVal)
    {
        GT_1trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: DeleteRunning - expected FVID2_EFAIL, got 0x%x\r\n",
                  retVal);
        testResult = CSIRX_COV_TEST_FAIL;
    }

    virtContext->state = savedState;

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: Delete running handle test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

/* Covers the eventCb mismatch path in CsirxDrv_eventValidateParams:
 * Registers ERROR event with NULL callback, then re-registers the same
 * group with a non-NULL callback.  The mismatch triggers
 * FVID2_WNON_RECOMMENDED_PARAMS (a warning, not a hard error).
 */
static int32_t CsirxCov_testEventCbMismatch(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    int32_t retVal;
    Csirx_EventPrms eventPrms;

    if (NULL == gCovSharedHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: EventCbMismatch - gCovSharedHandle is NULL\r\n");
        return CSIRX_COV_TEST_FAIL;
    }

    /* Register with NULL callback */
    Csirx_eventPrmsInit(&eventPrms);
    eventPrms.eventGroup = CSIRX_EVENT_GROUP_ERROR;
    eventPrms.eventMasks = CSIRX_EVENT_TYPE_ERR_ALL;
    eventPrms.eventCb    = NULL;
    eventPrms.appData    = NULL;
    retVal = Fvid2_control(gCovSharedHandle,
                           IOCTL_CSIRX_REGISTER_EVENT,
                           &eventPrms,
                           NULL);
    if (FVID2_SOK != retVal)
    {
        GT_1trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: EventCbMismatch - first register failed: 0x%x\r\n",
                  retVal);
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* Re-register with a non-NULL callback → eventCb mismatch */
    Csirx_eventPrmsInit(&eventPrms);
    eventPrms.eventGroup = CSIRX_EVENT_GROUP_ERROR;
    eventPrms.eventMasks = CSIRX_EVENT_TYPE_ERR_ALL;
    eventPrms.eventCb    = CsirxCov_errorEventCb;
    eventPrms.appData    = NULL;
    retVal = Fvid2_control(gCovSharedHandle,
                           IOCTL_CSIRX_REGISTER_EVENT,
                           &eventPrms,
                           NULL);
    if ((FVID2_SOK != retVal) && (FVID2_WNON_RECOMMENDED_PARAMS != retVal))
    {
        GT_1trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: EventCbMismatch - re-register unexpected error: 0x%x\r\n",
                  retVal);
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* Unregister */
    (void)Fvid2_control(gCovSharedHandle,
                        IOCTL_CSIRX_UNREGISTER_EVENT,
                        (void *)(uintptr_t)CSIRX_EVENT_GROUP_ERROR,
                        NULL);

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: Event callback mismatch test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

/* Covers channel-count-limit paths in CsirxDrv_setChCfgParams:
 * Uses the already-IN_USE instance so Fvid2_create runs setChCfgParams.
 * The counter is pre-injected above the limit so the very first channel
 * creation attempt returns FVID2_EINVALID_PARAMS and the create returns NULL.
 */
static int32_t CsirxCov_testSetChCfgParamsCountLimit(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    uint32_t instId;
    CsirxDrv_InstObj *instObj;
    Csirx_CreateParams createParams;
    Csirx_CreateStatus createStatus;
    Fvid2_CbParams cbPrms;
    Fvid2_Handle h;
    uint32_t savedCaptCh, savedOtfCh, savedLpbkCh;

    if (NULL == gCovSharedHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: ChCfgLimit - gCovSharedHandle is NULL\r\n");
        return CSIRX_COV_TEST_FAIL;
    }

    instId  = taskObj->instObj.instCfgInfo->csiDrvInst;
    instObj = &gCsirxCommonObj.instObj[instId];

    savedCaptCh  = instObj->numCaptCh;
    savedOtfCh   = instObj->numOtfCh;
    savedLpbkCh  = instObj->numLpbkCh;

    Fvid2CbParams_init(&cbPrms);
    cbPrms.cbFxn   = &CsirxCov_frameCompletionCb;
    cbPrms.appData = (Ptr)taskObj;

    /* Sub-test A: CAPT channel count overflow */
    CsirxCov_initCreateParams(&createParams, 1U,
                              CSIRX_CH_TYPE_CAPT,
                              FVID2_CSI2_DF_RAW12);
    instObj->numCaptCh = CSIRX_NUM_CH_CAPT + 1U;
    h = Fvid2_create(CSIRX_CAPT_DRV_ID, instId,
                     (void *)&createParams,
                     (void *)&createStatus, &cbPrms);
    instObj->numCaptCh = savedCaptCh;
    if (NULL != h)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: ChCfgLimit - CAPT overflow should return NULL\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
        (void)Fvid2_delete(h, NULL);
    }

    /* Sub-test B: OTF channel count overflow */
    CsirxCov_initCreateParams(&createParams, 1U,
                              CSIRX_CH_TYPE_OTF,
                              FVID2_CSI2_DF_RAW12);
    instObj->numOtfCh = CSIRX_NUM_CH_OTF_MAX + 1U;
    h = Fvid2_create(CSIRX_CAPT_DRV_ID, instId,
                     (void *)&createParams,
                     (void *)&createStatus, &cbPrms);
    instObj->numOtfCh = savedOtfCh;
    if (NULL != h)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: ChCfgLimit - OTF overflow should return NULL\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
        (void)Fvid2_delete(h, NULL);
    }

    /* Sub-test C: LPBK channel count overflow */
    CsirxCov_initCreateParams(&createParams, 1U,
                              CSIRX_CH_TYPE_LPBK,
                              FVID2_CSI2_DF_RAW12);
    instObj->numLpbkCh = CSIRX_NUM_CH_LPBK_MAX + 1U;
    h = Fvid2_create(CSIRX_CAPT_DRV_ID, instId,
                     (void *)&createParams,
                     (void *)&createStatus, &cbPrms);
    instObj->numLpbkCh = savedLpbkCh;
    if (NULL != h)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: ChCfgLimit - LPBK overflow should return NULL\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
        (void)Fvid2_delete(h, NULL);
    }

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: Channel cfg params count limit test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

int32_t CsirxCov_runAllTests(CsirxTestTaskObj *taskObj)
{
    int32_t retVal = FVID2_SOK;
    int32_t testResult = CSIRX_COV_TEST_PASS;
    uint32_t passCount = 0U;
    uint32_t failCount = 0U;
    uint32_t totalTests = 30U;
    Fvid2_Handle sharedHandle = NULL;
    Csirx_CreateParams createParams;
    Csirx_CreateStatus createStatus;
    Fvid2_CbParams cbPrms;
    uint32_t instId;

    GT_0trace(gAppTrace, GT_INFO,
              "\r\n ===============================================\r\n");
    GT_0trace(gAppTrace, GT_INFO,
              " CSIRX Coverage Improvement Test Suite\r\n");
    GT_0trace(gAppTrace, GT_INFO,
              " ===============================================\r\n\r\n");

    instId = taskObj->instObj.instCfgInfo->csiDrvInst;
    CsirxCov_initCreateParams(&createParams, 0U,
                              CSIRX_CH_TYPE_CAPT,
                              FVID2_CSI2_DF_RAW12);
    Fvid2CbParams_init(&cbPrms);
    cbPrms.cbFxn = &CsirxCov_frameCompletionCb;
    cbPrms.appData = (Ptr)taskObj;

    sharedHandle = Fvid2_create(CSIRX_CAPT_DRV_ID,
                                instId,
                                (void *)&createParams,
                                (void *)&createStatus,
                                &cbPrms);
    if (NULL == sharedHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: Shared handle creation FAILED -"
                  " tests needing a handle will be skipped\r\n");
        return CSIRX_COV_TEST_FAIL;
    }

    gCovSharedHandle = sharedHandle;

    /* Invalid IOCTL command */
    GT_1trace(gAppTrace, GT_INFO,
              "\r\n [Test 1/%d] Invalid IOCTL Command\r\n", totalTests);
    retVal = CsirxCov_testInvalidCmd(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal) { passCount++; } else { failCount++; }

    /* Invalid Driver ID */
    GT_1trace(gAppTrace, GT_INFO,
              "\r\n [Test 2/%d] Invalid Driver ID\r\n", totalTests);
    retVal = CsirxCov_testInvalidDriver(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal) { passCount++; } else { failCount++; }

    /* DphyCfg error path */
    GT_1trace(gAppTrace, GT_INFO,
                "\r\n [Test 3/%d] DphyCfg Error Path\r\n", totalTests);
    retVal = CsirxCov_testDphyCfgErrorPath(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal) { passCount++; } else { failCount++; }

    /* Compound conditions */
    GT_1trace(gAppTrace, GT_INFO,
                "\r\n [Test 4/%d] Compound Conditions\r\n", totalTests);
    retVal = CsirxCov_testCompoundConds(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal) { passCount++; } else { failCount++; }

    /* DMA Cfg edge cases */
    GT_1trace(gAppTrace, GT_INFO,
                "\r\n [Test 5/%d] DMA Cfg Edge Cases\r\n", totalTests);
    retVal = CsirxCov_testDmaCfgEdgeCases(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal) { passCount++; } else { failCount++; }

    /* UDMA error map */
    GT_1trace(gAppTrace, GT_INFO,
              "\r\n [Test 6/%d] UDMA Error Map\r\n", totalTests);
    retVal = CsirxCov_testUdmaErrorMap(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal) { passCount++; } else { failCount++; }

    /* Event registration edge cases */
    GT_1trace(gAppTrace, GT_INFO,
                "\r\n [Test 7/%d] Event Registration Edge Cases\r\n", totalTests);
    retVal = CsirxCov_testEventRegEdgeCases(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal) { passCount++; } else { failCount++; }

    /* checkDphyrxConfig */
    GT_1trace(gAppTrace, GT_INFO,
              "\r\n [Test 8/%d] checkDphyrxConfig (SOC)\r\n", totalTests);
    retVal = CsirxCov_testCheckDphyrxConfig(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal) { passCount++; } else { failCount++; }

    /* ASF event groups */
    GT_1trace(gAppTrace, GT_INFO,
                "\r\n [Test 9/%d] ASF Event Groups\r\n", totalTests);
    retVal = CsirxCov_testAsfEventGroups(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal) { passCount++; } else { failCount++; }

    /* StopCh fail break */
    GT_1trace(gAppTrace, GT_INFO,
                "\r\n [Test 10/%d] StopCh Fail Break\r\n", totalTests);
    retVal = CsirxCov_testStopChFailBreak(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal) { passCount++; } else { failCount++; }

    /* Error event ISR paths */
    GT_1trace(gAppTrace, GT_INFO,
                "\r\n [Test 11/%d] Error Event ISR Paths\r\n", totalTests);
    retVal = CsirxCov_testErrorEventIsrPaths(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal) { passCount++; } else { failCount++; }

    GT_1trace(gAppTrace, GT_INFO,
                "\r\n [Test 12/%d] CP INTD Event ISR Paths\r\n", totalTests);
    retVal = CsirxCov_testCpIntdEventIsrPaths(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal) { passCount++; } else { failCount++; }

    GT_1trace(gAppTrace, GT_INFO,
                "\r\n [Test 13/%d] Info Event ISR Paths\r\n", totalTests);
    retVal = CsirxCov_testInfoEventIsrPaths(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal) { passCount++; } else { failCount++; }

    GT_1trace(gAppTrace, GT_INFO,
                "\r\n [Test 14/%d] ASF ESM High Event ISR Paths\r\n", totalTests);
    retVal = CsirxCov_testAsfEsmHighEventIsrPaths(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal) { passCount++; } else { failCount++; }

    GT_1trace(gAppTrace, GT_INFO,
                "\r\n [Test 15/%d] ASF ESM Low Event ISR Paths\r\n", totalTests);
    retVal = CsirxCov_testAsfEsmLowEventIsrPaths(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal) { passCount++; } else { failCount++; }

    GT_1trace(gAppTrace, GT_INFO,
                "\r\n [Test 16/%d] ASF ESM CFG Event ISR Paths\r\n", totalTests);
    retVal = CsirxCov_testAsfEsmCfgEventIsrPaths(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal) { passCount++; } else { failCount++; }

    GT_1trace(gAppTrace, GT_INFO,
                "\r\n [Test 17/%d] Channel Start/Stop IOCTL\r\n", totalTests);
    retVal = CsirxCov_testChStartStopIoctl(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal) { passCount++; } else { failCount++; }

    GT_1trace(gAppTrace, GT_INFO,
                "\r\n [Test 18/%d] Queue/Dequeue Edge Cases\r\n", totalTests);
    retVal = CsirxCov_testQueueDequeueEdgeCases(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal) { passCount++; } else { failCount++; }

    GT_1trace(gAppTrace, GT_INFO,
                "\r\n [Test 19/%d] Stream Idle Wait Loop\r\n", totalTests);
    retVal = CsirxCov_testStreamIdleWaitLoop(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal) { passCount++; } else { failCount++; }

    GT_1trace(gAppTrace, GT_INFO,
                "\r\n [Test 20/%d] Create Edge Cases\r\n", totalTests);
    retVal = CsirxCov_testCreateEdgeCases(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal) { passCount++; } else { failCount++; }

    GT_1trace(gAppTrace, GT_INFO,
                "\r\n [Test 21/%d] Null Handle APIs\r\n", totalTests);
    retVal = CsirxCov_testNullHandleApis(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal) { passCount++; } else { failCount++; }

    GT_1trace(gAppTrace, GT_INFO,
                "\r\n [Test 22/%d] Delete Running Handle\r\n", totalTests);
    retVal = CsirxCov_testDeleteRunningHandle(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal) { passCount++; } else { failCount++; }

    GT_1trace(gAppTrace, GT_INFO,
                "\r\n [Test 23/%d] Event Callback Mismatch\r\n", totalTests);
    retVal = CsirxCov_testEventCbMismatch(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal) { passCount++; } else { failCount++; }

    GT_1trace(gAppTrace, GT_INFO,
                "\r\n [Test 24/%d] Channel Cfg Params Count Limit\r\n", totalTests);
    retVal = CsirxCov_testSetChCfgParamsCountLimit(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal) { passCount++; } else { failCount++; }

    GT_1trace(gAppTrace, GT_INFO,
                "\r\n [Test 25/%d] dphyrxCoreLaneReady Lane Cases\r\n", totalTests);
    retVal = CsirxCov_testDphyrxCoreLaneReady(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal) { passCount++; } else { failCount++; }

    GT_1trace(gAppTrace, GT_INFO,
                "\r\n [Test 26/%d] NULL UDMA drvHandle on Capture Create\r\n",
                totalTests);
    retVal = CsirxCov_testNullDrvHandle(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal) { passCount++; } else { failCount++; }

    if (NULL != sharedHandle)
    {
        {
            CsirxDrv_InstObj   *cleanInstObj = &gCsirxCommonObj.instObj[instId];
            CsirxDrv_ChObj     *cleanChObj   = &cleanInstObj->chObj[0];
            CsirxDrv_CommonObj *cleanCaptObj = cleanChObj->instObj->commonObjRef;

            (void)CsirxDrv_clearUdmaParams(cleanChObj);
            if (UDMA_INIT_DONE == cleanCaptObj->masterEvent.eventInitDone)
            {
                (void)Udma_eventUnRegister(&cleanCaptObj->masterEvent);
            }
        }
        (void)Fvid2_delete(sharedHandle, NULL);
        sharedHandle     = NULL;
        gCovSharedHandle = NULL;
    }

    GT_1trace(gAppTrace, GT_INFO,
                "\r\n [Test 27/%d] Get Free Channel Num Exhausted\r\n", totalTests);
    retVal = CsirxCov_testGetFreeChNumExhausted(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal) { passCount++; } else { failCount++; }

    GT_1trace(gAppTrace, GT_INFO,
                "\r\n [Test 28/%d] Virtual Context Exhausted\r\n", totalTests);
    retVal = CsirxCov_testVirtContextExhausted(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal) { passCount++; } else { failCount++; }

    GT_1trace(gAppTrace, GT_INFO,
                "\r\n [Test 29/%d] Null Pointer Args\r\n", totalTests);
    retVal = CsirxCov_testNullPointerArgs(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal) { passCount++; } else { failCount++; }

    GT_1trace(gAppTrace, GT_INFO,
                "\r\n [Test 30/%d] Test Handlers in Csirx Core\r\n", totalTests);
    retVal = CsirxCov_testHandlers(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal) { passCount++; } else { failCount++; }

    /* Print summary */
    GT_0trace(gAppTrace, GT_INFO,
              "\r\n ===============================================\r\n");
    GT_0trace(gAppTrace, GT_INFO,
              " CSIRX Coverage Test Suite - SUMMARY\r\n");
    GT_0trace(gAppTrace, GT_INFO,
              " ===============================================\r\n");
    GT_2trace(gAppTrace, GT_INFO,
              "  Total : %d\r\n  Passed: %d\r\n",
              totalTests, passCount);
    GT_1trace(gAppTrace, GT_INFO,
              "  Failed: %d\r\n", failCount);
    GT_0trace(gAppTrace, GT_INFO,
              " ===============================================\r\n\r\n");

    if (failCount > 0U)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    return testResult;
}

/* ========================================================================== */
/*                       Internal Helper Functions                            */
/* ========================================================================== */

static int32_t CsirxCov_testNullDrvHandle(CsirxTestTaskObj *taskObj)
{
    int32_t           testResult    = CSIRX_COV_TEST_PASS;
    uint32_t          instId        = taskObj->instObj.instCfgInfo->csiDrvInst;
    CsirxDrv_InstObj *instObj       = &gCsirxCommonObj.instObj[instId];
    Csirx_ChCfg       tmpChCfg;
    Csirx_ChCfg      *savedChCfg;
    Udma_DrvHandle    savedDrvHandle;
    Csirx_CreateParams createParams;
    Csirx_CreateStatus createStatus;
    Fvid2_CbParams     cbPrms;
    Fvid2_Handle       h;

    /* Seed chObj[0].chCfg so the NULL-drvHandle guard is reachable
     * without a null-pointer dereference on the chType field. */
    Csirx_chCfgInit(&tmpChCfg);                   /* chType = CSIRX_CH_TYPE_CAPT */
    savedChCfg              = instObj->chObj[0U].chCfg;
    instObj->chObj[0U].chCfg = &tmpChCfg;

    savedDrvHandle                       = gCsirxCommonObj.initParams.drvHandle;
    gCsirxCommonObj.initParams.drvHandle = NULL_PTR;

    CsirxCov_initCreateParams(&createParams, 1U,
                              CSIRX_CH_TYPE_CAPT,
                              FVID2_CSI2_DF_RAW12);
    Fvid2CbParams_init(&cbPrms);
    cbPrms.cbFxn   = &CsirxCov_frameCompletionCb;
    cbPrms.appData = (Ptr)taskObj;

    h = Fvid2_create(CSIRX_CAPT_DRV_ID, instId,
                     (void *)&createParams,
                     (void *)&createStatus,
                     &cbPrms);

    /* Restore internal state before checking results */
    gCsirxCommonObj.initParams.drvHandle = savedDrvHandle;
    instObj->chObj[0U].chCfg             = savedChCfg;

    if ((NULL            != h) ||
        (FVID2_EBADARGS  != createStatus.retVal))
    {
        GT_2trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: NullDrvHandle - expected NULL/EBADARGS,"
                  " got handle 0x%x retVal 0x%x\r\n",
                  (uint32_t)(uintptr_t)h, createStatus.retVal);
        testResult = CSIRX_COV_TEST_FAIL;
        if (NULL != h)
        {
            (void)Fvid2_delete(h, NULL);
        }
    }

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: NULL UDMA drvHandle on Capture Create - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

/**
 *  \brief Initialize Csirx_CreateParams with test defaults.
 *
 *  \param createParams  [OUT] Pointer to create params to initialize
 *  \param numCh         [IN]  Number of channels
 *  \param chType        [IN]  Channel type (CAPT/OTF/LPBK)
 *  \param dataType      [IN]  CSI2 data type
 */
static void CsirxCov_initCreateParams(Csirx_CreateParams *createParams,
                                      uint32_t numCh,
                                      uint32_t chType,
                                      uint32_t dataType)
{
    uint32_t chIdx;

    Csirx_createParamsInit(createParams);

    createParams->numCh = numCh;
    createParams->frameDropBuf = (uint64_t)(uintptr_t)&gCovFrmDropBuf[0];
    createParams->frameDropBufLen = CSIRX_COV_FRAME_SIZE;

    /* Configure instance */
    createParams->instCfg.enableCsiv2p0Support = BFALSE;
    createParams->instCfg.numDataLanes = 2U;
    createParams->instCfg.dataLanesMap[0] = 1U;
    createParams->instCfg.dataLanesMap[1] = 2U;
    createParams->instCfg.enableErrbypass = BFALSE;

    for (chIdx = 0U; chIdx < numCh; chIdx++)
    {
        createParams->chCfg[chIdx].chId = chIdx;
        createParams->chCfg[chIdx].chType = chType;
        createParams->chCfg[chIdx].vcNum = chIdx;
        createParams->chCfg[chIdx].inCsiDataType = dataType;

        /* Output format */
        createParams->chCfg[chIdx].outFmt.width = CSIRX_COV_FRAME_WIDTH;
        createParams->chCfg[chIdx].outFmt.height = CSIRX_COV_FRAME_HEIGHT;
        createParams->chCfg[chIdx].outFmt.pitch[0] =
            CSIRX_COV_FRAME_WIDTH * CSIRX_COV_FRAME_BPP;
        createParams->chCfg[chIdx].outFmt.dataFormat =
            FVID2_DF_BGRX32_8888;
        createParams->chCfg[chIdx].outFmt.ccsFormat =
            FVID2_CCSF_BITS12_UNPACKED16;
    }
}

/**
 *  \brief Dummy timestamp function for coverage tests.
 */
static uint64_t CsirxCov_dummyTimestampFxn(void *args)
{
    return (uint64_t)0U;
}

/**
 *  \brief Frame completion callback for coverage tests.
 */
static int32_t CsirxCov_frameCompletionCb(Fvid2_Handle handle,
                                          Ptr appData)
{
    CsirxTestTaskObj *taskObj = (CsirxTestTaskObj *)appData;

    if (NULL != taskObj)
    {
        if (NULL != taskObj->instObj.lockSem)
        {
            (void)SemaphoreP_post(taskObj->instObj.lockSem);
        }
    }

    return FVID2_SOK;
}
