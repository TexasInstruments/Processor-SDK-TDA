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
 *  \file csirx_test_csl_coverage.c
 *
 *  \brief CSL-CSIRX driver coverage improvement tests.
 *
 *  This file exercises uncovered CSL functions in csirx.c by calling
 *  them directly with a valid CSIRX_PrivateData obtained from the
 *  driver's internal structures after Fvid2_create().
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

/** \brief Stream number used for stream-based CSL API tests */
#define CSIRX_COV_STREAM_NUM                    ((uint32_t)0U)
#define CSIRX_COV_STREAM_ERR                    ((uint32_t)5U)

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static void CsirxCovCsl_initCreateParams(Csirx_CreateParams *createParams,
                                         uint32_t numCh,
                                         uint32_t chType,
                                         uint32_t dataType);

/* CSL-level coverage test functions */
static int32_t CsirxCovCsl_testStreamControlFuncs(CsirxTestTaskObj *taskObj);
static int32_t CsirxCovCsl_testIrqFunctions(CsirxTestTaskObj *taskObj);
static int32_t CsirxCovCsl_testDphyFunctions(CsirxTestTaskObj *taskObj);
static int32_t CsirxCovCsl_testAsfFunctions(CsirxTestTaskObj *taskObj);
static int32_t CsirxCovCsl_testCfg(CsirxTestTaskObj *taskObj);
static int32_t CsirxCovCsl_testProbe(CsirxTestTaskObj *taskObj);
static int32_t CsirxCovCsl_testIfRegFunctions(CsirxTestTaskObj *taskObj);
static int32_t CsirxCovCsl_testIntdFunctions(CsirxTestTaskObj *taskObj);
static int32_t CsirxCovCsl_testIsrFunction(CsirxTestTaskObj *taskObj);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

extern uint32_t gAppTrace;
extern CsirxDrv_CommonObj gCsirxCommonObj;
extern uint32_t CSIRX_AsfFatalNonFatalSelectSF(const CSIRX_AsfFatalNonFatalSelect *obj);
extern uint32_t CSIRX_MonitorIrqsSF(const CSIRX_MonitorIrqs *obj);
extern uint32_t CSIRX_MonitorIrqsMaskCfgSF(const CSIRX_MonitorIrqsMaskCfg *obj);
extern uint32_t CSIRX_InfoIrqsSF(const CSIRX_InfoIrqs *obj);
extern uint32_t CSIRX_InfoIrqsMaskCfgSF(const CSIRX_InfoIrqsMaskCfg *obj);
extern uint32_t CSIRX_ErrorIrqsSF(const CSIRX_ErrorIrqs *obj);
extern uint32_t CSIRX_ErrorIrqsMaskCfgSF(const CSIRX_ErrorIrqsMaskCfg *obj);
extern uint32_t CSIRX_DphyLaneControlSF(const CSIRX_DphyLaneControl *obj);
extern uint32_t CSIRX_DphyErrStatusIrqSF(const CSIRX_DphyErrStatusIrq *obj);
extern uint32_t CSIRX_DphyErrIrqMaskCfgSF(const CSIRX_DphyErrIrqMaskCfg *obj);
extern uint32_t CSIRX_StaticCfgSF(const CSIRX_StaticCfg *obj);
extern uint32_t CSIRX_StreamCfgSF(const CSIRX_StreamCfg *obj);
extern uint32_t CSIRX_StreamDataCfgSF(const CSIRX_StreamDataCfg *obj);
extern uint32_t CSIRX_StreamMonitorCtrlSF(const CSIRX_StreamMonitorCtrl *obj);
extern uint32_t CSIRX_StreamTimerSF(const CSIRX_StreamTimer *obj);
extern uint32_t CSIRX_StreamFccCtrlSF(const CSIRX_StreamFccCtrl *obj);
extern uint32_t CSIRX_StreamFifoFillLvlSF(const CSIRX_StreamFifoFillLvl *obj);
extern uint32_t CSIRX_StreamCtrlSF(const CSIRX_StreamCtrl *obj);
extern uint32_t CSIRX_ErrorBypassCfgSF(const CSIRX_ErrorBypassCfg *obj);
extern uint32_t CSIRX_AsfIrqsSF(const CSIRX_AsfIrqs *obj);
extern uint32_t CSIRX_AsfIrqMaskCfgSF(const CSIRX_AsfIrqMaskCfg *obj);
extern uint32_t CSIRX_AsfIrqTestSF(const CSIRX_AsfIrqTest *obj);
extern uint32_t CSIRX_AsfTransToCtrlSF(const CSIRX_AsfTransToCtrl *obj);
extern uint32_t CSIRX_AsfTransToFaultMaskSF(const CSIRX_AsfTransToFaultMask *obj);
extern uint32_t CSIRX_AsfTransToFaultStatusSF(const CSIRX_AsfTransToFaultStatus *obj);

/** \brief Frame drop buffer for coverage tests */
static uint8_t gCovFrmDropBuf[CSIRX_COV_FRAME_SIZE]
    __attribute__((aligned(128), section(".data_buffer")));

/** \brief Fake register buffer for checkMagicNumber failure test */
static uint32_t gFakeCsirxRegBuf[1024U]
    __attribute__((aligned(128)));

/** \brief Shared driver handle for all CSL tests */
static Fvid2_Handle gCovCslSharedHandle = NULL;

/** \brief Instance ID used for the shared driver */
static uint32_t gCovCslInstId = 0U;

/**
 *  \brief Frame completion callback for coverage tests.
 */
static int32_t CsirxCovCsl_frameCompletionCb(Fvid2_Handle handle,
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

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t CsirxCovCsl_runAllTests(CsirxTestTaskObj *taskObj)
{
    int32_t retVal = FVID2_SOK;
    int32_t testResult = CSIRX_COV_TEST_PASS;
    Csirx_CreateParams createParams;
    Csirx_CreateStatus createStatus;
    Fvid2_CbParams cbPrms;
    uint32_t passCount = 0U;
    uint32_t failCount = 0U;
    uint32_t totalTests = 9U;

    GT_0trace(gAppTrace, GT_INFO,
              "\r\n ===============================================\r\n");
    GT_0trace(gAppTrace, GT_INFO,
              " CSIRX CSL Coverage Improvement Test Suite\r\n");
    GT_0trace(gAppTrace, GT_INFO,
              " ===============================================\r\n\r\n");

    gCovCslInstId = taskObj->instObj.instCfgInfo->csiDrvInst;
    CsirxCovCsl_initCreateParams(&createParams, 0U, CSIRX_CH_TYPE_CAPT,
                                 FVID2_CSI2_DF_RAW12);
    Fvid2CbParams_init(&cbPrms);
    cbPrms.cbFxn = &CsirxCovCsl_frameCompletionCb;
    cbPrms.appData = (Ptr)taskObj;

    gCovCslSharedHandle = Fvid2_create(CSIRX_CAPT_DRV_ID, gCovCslInstId,
                                       (void *)&createParams, &createStatus,
                                       &cbPrms);
    if ((Fvid2_Handle)NULL == gCovCslSharedHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV_CSL: Shared driver create FAILED\r\n");
    }

    /* Run CSL-level coverage test functions */
    GT_0trace(gAppTrace, GT_INFO,
              "\r\n [Test 1/9] CSL-level Stream Control functions\r\n");
    retVal = CsirxCovCsl_testStreamControlFuncs(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal)
    {
        passCount++;
    }
    else
    {
        failCount++;
    }

    GT_0trace(gAppTrace, GT_INFO,
              "\r\n [Test 2/9] CSL-level Test IRQ functions\r\n");
    retVal = CsirxCovCsl_testIrqFunctions(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal)
    {
        passCount++;
    }
    else
    {
        failCount++;
    }

    GT_0trace(gAppTrace, GT_INFO,
              "\r\n [Test 3/9] CSL-level Test DPHY functions\r\n");
    retVal = CsirxCovCsl_testDphyFunctions(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal)
    {
        passCount++;
    }
    else
    {
        failCount++;
    }

    GT_0trace(gAppTrace, GT_INFO,
              "\r\n [Test 4/9] CSL-level Test ASF functions\r\n");
    retVal = CsirxCovCsl_testAsfFunctions(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal)
    {
        passCount++;
    }
    else
    {
        failCount++;
    }

    GT_0trace(gAppTrace, GT_INFO,
              "\r\n [Test 5/9] CSL-level Test Magic Number / Probe\r\n");
    retVal = CsirxCovCsl_testProbe(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal)
    {
        passCount++;
    }
    else
    {
        failCount++;
    }

    GT_0trace(gAppTrace, GT_INFO,
              "\r\n [Test 6/9] CSL-level Static Config & Debug\r\n");
    retVal = CsirxCovCsl_testCfg(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal)
    {
        passCount++;
    }
    else
    {
        failCount++;
    }

    GT_0trace(gAppTrace, GT_INFO,
              "\r\n [Test 7/9] CSL-level IF Register functions\r\n");
    retVal = CsirxCovCsl_testIfRegFunctions(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal)
    {
        passCount++;
    }
    else
    {
        failCount++;
    }

    GT_0trace(gAppTrace, GT_INFO,
              "\r\n [Test 8/9] CSL-level INTD functions\r\n");
    retVal = CsirxCovCsl_testIntdFunctions(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal)
    {
        passCount++;
    }
    else
    {
        failCount++;
    }

    GT_0trace(gAppTrace, GT_INFO,
              "\r\n [Test 9/9] ISR function\r\n");
    retVal = CsirxCovCsl_testIsrFunction(taskObj);
    if (CSIRX_COV_TEST_PASS == retVal)
    {
        passCount++;
    }
    else
    {
        failCount++;
    }

    /* Print summary */
    GT_0trace(gAppTrace, GT_INFO,
              "\r\n ===============================================\r\n");
    GT_0trace(gAppTrace, GT_INFO,
              " CSIRX CSL Coverage Test Suite - SUMMARY\r\n");
    GT_0trace(gAppTrace, GT_INFO,
              " ===============================================\r\n");
    GT_1trace(gAppTrace, GT_INFO,
              " Total Tests: %d\r\n", totalTests);
    GT_1trace(gAppTrace, GT_INFO,
              " Passed: %d\r\n", passCount);
    GT_1trace(gAppTrace, GT_INFO,
              " Failed: %d\r\n", failCount);
    GT_0trace(gAppTrace, GT_INFO,
              " ===============================================\r\n\r\n");

    if (failCount > 0U)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* Clean up shared handle before returning */
    if (NULL != gCovCslSharedHandle)
    {
        (void)Fvid2_delete(gCovCslSharedHandle, NULL);
        gCovCslSharedHandle = NULL;
    }

    return testResult;
}

/* ========================================================================== */
/*                       Internal Helper Functions                            */
/* ========================================================================== */

static void CsirxCovCsl_initCreateParams(Csirx_CreateParams *createParams,
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

/* ========================================================================== */
/*                         CSL Coverage Test Functions                        */
/* ========================================================================== */

/**
 *  \brief CSL coverage test for stream related functions.
 *
 *  Calls uncovered stream getter/setter CSL functions:
 *  CSIRX_GetStreamCtrl, CSIRX_SetStreamCtrl, CSIRX_GetStreamStatus,
 *  CSIRX_GetStreamCfg, CSIRX_SetStreamCfg, CSIRX_GetStreamDataCfg,
 *  CSIRX_SetStreamDataCfg, CSIRX_GetStreamMonitorCtrl, CSIRX_SetStreamMonitorCtrl,
 *  CSIRX_GetStreamMonitorFrame, CSIRX_GetStreamMonitorLb, CSIRX_SetStreamMonitorLb,
 *  CSIRX_GetStreamTimer, CSIRX_SetStreamTimer, CSIRX_GetStreamFccCfg,
 *  CSIRX_SetStreamFccCfg, CSIRX_GetStreamFccCtrl, CSIRX_SetStreamFccCtrl,
 *  CSIRX_GetStreamFifoFillLvl, CSIRX_SetStreamFifoFillLvl
 */
static int32_t CsirxCovCsl_testStreamControlFuncs(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    uint32_t status;
    CSIRX_PrivateData *pD;
    CSIRX_StreamCtrl streamCtrl;
    CSIRX_StreamStatus streamStatus;
    CSIRX_StreamCfg streamCfg;
    CSIRX_StreamDataCfg streamDataCfg;
    CSIRX_StreamMonitorCtrl monCtrl;
    CSIRX_StreamMonitorFrame monFrame;
    CSIRX_StreamMonitorLb monLb;
    CSIRX_StreamTimer streamTimer;
    CSIRX_StreamFccCfg fccCfg;
    CSIRX_StreamFccCtrl fccCtrl;
    CSIRX_StreamFifoFillLvl fifoFillLvl;

    if ((Fvid2_Handle)NULL == gCovCslSharedHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV_CSL: No shared handle, skipping\r\n");
        return CSIRX_COV_TEST_FAIL;
    }

    pD = &gCsirxCommonObj.instObj[gCovCslInstId].cslObj.cslCfgData;

    (void)memset(&streamCtrl, 0, sizeof(streamCtrl));
    (void)memset(&streamStatus, 0, sizeof(streamStatus));
    (void)memset(&streamCfg, 0, sizeof(streamCfg));
    (void)memset(&streamDataCfg, 0, sizeof(streamDataCfg));
    (void)memset(&monCtrl, 0, sizeof(monCtrl));
    (void)memset(&monFrame, 0, sizeof(monFrame));
    (void)memset(&monLb, 0, sizeof(monLb));
    (void)memset(&streamTimer, 0, sizeof(streamTimer));
    (void)memset(&fccCfg, 0, sizeof(fccCfg));
    (void)memset(&fccCtrl, 0, sizeof(fccCtrl));
    (void)memset(&fifoFillLvl, 0, sizeof(fifoFillLvl));

    /* CSIRX_GetStreamCtrl */
    status = CSIRX_GetStreamCtrl(pD, &streamCtrl, CSIRX_COV_STREAM_NUM);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamCtrl(pD, &streamCtrl, 1U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamCtrl(pD, &streamCtrl, 2U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamCtrl(pD, &streamCtrl, 3U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamCtrl(NULL, &streamCtrl, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamCtrl(pD, NULL, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamCtrl(pD, &streamCtrl, CSIRX_COV_STREAM_ERR);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_SetStreamCtrl */
    status = CSIRX_SetStreamCtrl(pD, &streamCtrl, CSIRX_COV_STREAM_NUM);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamCtrl(pD, &streamCtrl, 1U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamCtrl(pD, &streamCtrl, 2U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamCtrl(pD, &streamCtrl, 3U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamCtrl(NULL, &streamCtrl, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamCtrl(pD, NULL, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamCtrl(pD, &streamCtrl, CSIRX_COV_STREAM_ERR);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_GetStreamStatus */
    status = CSIRX_GetStreamStatus(pD, &streamStatus, CSIRX_COV_STREAM_NUM);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamStatus(pD, &streamStatus, 1U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamStatus(pD, &streamStatus, 2U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamStatus(pD, &streamStatus, 3U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamStatus(NULL, &streamStatus, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamStatus(pD, NULL, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamStatus(pD, &streamStatus, CSIRX_COV_STREAM_ERR);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_GetStreamCfg */
    status = CSIRX_GetStreamCfg(pD, &streamCfg, CSIRX_COV_STREAM_NUM);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamCfg(pD, &streamCfg, 1U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamCfg(pD, &streamCfg, 2U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamCfg(pD, &streamCfg, 3U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamCfg(NULL, &streamCfg, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamCfg(pD, NULL, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamCfg(pD, &streamCfg, CSIRX_COV_STREAM_ERR);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_SetStreamCfg */
    status = CSIRX_SetStreamCfg(pD, &streamCfg, CSIRX_COV_STREAM_NUM);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamCfg(pD, &streamCfg, 1U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamCfg(pD, &streamCfg, 2U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamCfg(pD, &streamCfg, 3U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamCfg(NULL, &streamCfg, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamCfg(pD, NULL, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamCfg(pD, &streamCfg, CSIRX_COV_STREAM_ERR);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_GetStreamDataCfg */
    status = CSIRX_GetStreamDataCfg(pD, &streamDataCfg, CSIRX_COV_STREAM_NUM);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamDataCfg(pD, &streamDataCfg, 1U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamDataCfg(pD, &streamDataCfg, 2U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamDataCfg(pD, &streamDataCfg, 3U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamDataCfg(NULL, &streamDataCfg, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamDataCfg(pD, NULL, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamDataCfg(pD, &streamDataCfg, CSIRX_COV_STREAM_ERR);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_SetStreamDataCfg */
    status = CSIRX_SetStreamDataCfg(pD, &streamDataCfg, CSIRX_COV_STREAM_NUM);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamDataCfg(pD, &streamDataCfg, 1U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamDataCfg(pD, &streamDataCfg, 2U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamDataCfg(pD, &streamDataCfg, 3U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamDataCfg(NULL, &streamDataCfg, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamDataCfg(pD, NULL, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamDataCfg(pD, &streamDataCfg, CSIRX_COV_STREAM_ERR);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_GetStreamMonitorCtrl */
    status = CSIRX_GetStreamMonitorCtrl(pD, &monCtrl, CSIRX_COV_STREAM_NUM);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamMonitorCtrl(pD, &monCtrl, 1U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamMonitorCtrl(pD, &monCtrl, 2U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamMonitorCtrl(pD, &monCtrl, 3U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamMonitorCtrl(NULL, &monCtrl, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamMonitorCtrl(pD, NULL, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamMonitorCtrl(pD, &monCtrl, CSIRX_COV_STREAM_ERR);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_SetStreamMonitorCtrl */
    status = CSIRX_SetStreamMonitorCtrl(pD, &monCtrl, CSIRX_COV_STREAM_NUM);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamMonitorCtrl(pD, &monCtrl, 1U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamMonitorCtrl(pD, &monCtrl, 2U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamMonitorCtrl(pD, &monCtrl, 3U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamMonitorCtrl(NULL, &monCtrl, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamMonitorCtrl(pD, NULL, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamMonitorCtrl(pD, &monCtrl, CSIRX_COV_STREAM_ERR);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_GetStreamMonitorFrame */
    status = CSIRX_GetStreamMonitorFrame(pD, &monFrame, CSIRX_COV_STREAM_NUM);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamMonitorFrame(pD, &monFrame, 1U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamMonitorFrame(pD, &monFrame, 2U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamMonitorFrame(pD, &monFrame, 3U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamMonitorFrame(NULL, &monFrame, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamMonitorFrame(pD, NULL, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamMonitorFrame(pD, &monFrame, CSIRX_COV_STREAM_ERR);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_GetStreamMonitorLb */
    status = CSIRX_GetStreamMonitorLb(pD, &monLb, CSIRX_COV_STREAM_NUM);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamMonitorLb(pD, &monLb, 1U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamMonitorLb(pD, &monLb, 2U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamMonitorLb(pD, &monLb, 3U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamMonitorLb(NULL, &monLb, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamMonitorLb(pD, NULL, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamMonitorLb(pD, &monLb, CSIRX_COV_STREAM_ERR);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_SetStreamMonitorLb */
    status = CSIRX_SetStreamMonitorLb(pD, &monLb, CSIRX_COV_STREAM_NUM);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamMonitorLb(pD, &monLb, 1U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamMonitorLb(pD, &monLb, 2U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamMonitorLb(pD, &monLb, 3U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamMonitorLb(NULL, &monLb, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamMonitorLb(pD, NULL, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamMonitorLb(pD, &monLb, CSIRX_COV_STREAM_ERR);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_GetStreamTimer */
    status = CSIRX_GetStreamTimer(pD, &streamTimer, CSIRX_COV_STREAM_NUM);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamTimer(pD, &streamTimer, 1U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamTimer(pD, &streamTimer, 2U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamTimer(pD, &streamTimer, 3U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamTimer(NULL, &streamTimer, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamTimer(pD, NULL, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamTimer(pD, &streamTimer, CSIRX_COV_STREAM_ERR);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_SetStreamTimer */
    status = CSIRX_SetStreamTimer(pD, &streamTimer, CSIRX_COV_STREAM_NUM);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamTimer(pD, &streamTimer, 1U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamTimer(pD, &streamTimer, 2U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamTimer(pD, &streamTimer, 3U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamTimer(NULL, &streamTimer, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamTimer(pD, NULL, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamTimer(pD, &streamTimer, CSIRX_COV_STREAM_ERR);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_GetStreamFccCfg */
    status = CSIRX_GetStreamFccCfg(pD, &fccCfg, CSIRX_COV_STREAM_NUM);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamFccCfg(pD, &fccCfg, 1U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamFccCfg(pD, &fccCfg, 2U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamFccCfg(pD, &fccCfg, 3U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamFccCfg(NULL, &fccCfg, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamFccCfg(pD, NULL, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamFccCfg(pD, &fccCfg, CSIRX_COV_STREAM_ERR);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_SetStreamFccCfg */
    status = CSIRX_SetStreamFccCfg(pD, &fccCfg, CSIRX_COV_STREAM_NUM);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamFccCfg(pD, &fccCfg, 1U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamFccCfg(pD, &fccCfg, 2U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamFccCfg(pD, &fccCfg, 3U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamFccCfg(NULL, &fccCfg, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamFccCfg(pD, NULL, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamFccCfg(pD, &fccCfg, CSIRX_COV_STREAM_ERR);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_GetStreamFccCtrl */
    status = CSIRX_GetStreamFccCtrl(pD, &fccCtrl, CSIRX_COV_STREAM_NUM);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamFccCtrl(pD, &fccCtrl, 1U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamFccCtrl(pD, &fccCtrl, 2U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamFccCtrl(pD, &fccCtrl, 3U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamFccCtrl(NULL, &fccCtrl, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamFccCtrl(pD, NULL, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamFccCtrl(pD, &fccCtrl, CSIRX_COV_STREAM_ERR);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_SetStreamFccCtrl */
    status = CSIRX_SetStreamFccCtrl(pD, &fccCtrl, CSIRX_COV_STREAM_NUM);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamFccCtrl(pD, &fccCtrl, 1U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamFccCtrl(pD, &fccCtrl, 2U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamFccCtrl(pD, &fccCtrl, 3U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamFccCtrl(NULL, &fccCtrl, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamFccCtrl(pD, NULL, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamFccCtrl(pD, &fccCtrl, CSIRX_COV_STREAM_ERR);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_GetStreamFifoFillLvl */
    status = CSIRX_GetStreamFifoFillLvl(pD, &fifoFillLvl, CSIRX_COV_STREAM_NUM);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamFifoFillLvl(pD, &fifoFillLvl, 1U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamFifoFillLvl(pD, &fifoFillLvl, 2U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamFifoFillLvl(pD, &fifoFillLvl, 3U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamFifoFillLvl(NULL, &fifoFillLvl, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamFifoFillLvl(pD, NULL, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStreamFifoFillLvl(pD, &fifoFillLvl, CSIRX_COV_STREAM_ERR);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_SetStreamFifoFillLvl */
    status = CSIRX_SetStreamFifoFillLvl(pD, &fifoFillLvl, CSIRX_COV_STREAM_NUM);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamFifoFillLvl(pD, &fifoFillLvl, 1U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamFifoFillLvl(pD, &fifoFillLvl, 2U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamFifoFillLvl(pD, &fifoFillLvl, 3U);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamFifoFillLvl(NULL, &fifoFillLvl, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamFifoFillLvl(pD, NULL, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamFifoFillLvl(pD, &fifoFillLvl, CSIRX_COV_STREAM_ERR);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    streamDataCfg.vcSelect = 0xFFU;
    uint8_t originVCXConfig = pD -> deviceConfig.vcxConfig;
    pD -> deviceConfig.vcxConfig = 0U;

    status = CSIRX_SetStreamDataCfg(pD, &streamDataCfg, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
     pD -> deviceConfig.vcxConfig = originVCXConfig;

    (void)memset(&streamCtrl, 0xFF, sizeof(streamCtrl));
    (void)memset(&streamCfg, 0xFF, sizeof(streamCfg));
    (void)memset(&streamDataCfg, 0xFF, sizeof(streamDataCfg));
    (void)memset(&monCtrl, 0xFF, sizeof(monCtrl));
    (void)memset(&fccCtrl, 0xFF, sizeof(fccCtrl));
    (void)memset(&fifoFillLvl, 0xFF, sizeof(fifoFillLvl));

    status = CSIRX_StreamCtrlSF(&streamCtrl);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_StreamCfgSF */
    status = CSIRX_StreamCfgSF(&streamCfg);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_StreamDataCfgSF */
    status = CSIRX_StreamDataCfgSF(&streamDataCfg);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStreamDataCfg(pD, &streamDataCfg, CSIRX_COV_STREAM_NUM);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_StreamMonitorCtrlSF */
    status = CSIRX_StreamMonitorCtrlSF(&monCtrl);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_StreamMonitorCtrlSF */
    streamTimer.count = 0xFFFF0000U;
    status = CSIRX_StreamTimerSF(&streamTimer);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_StreamFccCtrlSF */
    status = CSIRX_StreamFccCtrlSF(&fccCtrl);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_StreamFifoFillLvlSF */
    status = CSIRX_StreamFifoFillLvlSF(&fifoFillLvl);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV_CSL: Stream control functions test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

/**
 *  \brief CSL coverage test for IRQ mask and status functions.
 *
 *  Calls IRQ-related CSL functions:
 *  CSIRX_GetInfoIrqsMaskCfg, CSIRX_SetInfoIrqsMaskCfg,
 *  CSIRX_GetInfoIrqs, CSIRX_SetInfoIrqs,
 *  CSIRX_GetMonitorIrqsMaskCfg, CSIRX_SetMonitorIrqsMaskCfg,
 *  CSIRX_GetErrorIrqsMaskCfg, CSIRX_SetErrorIrqsMaskCfg,
 *  CSIRX_GetErrorIrqs, CSIRX_SetErrorIrqs,
 *  CSIRX_GetDphyErrIrqMaskCfg, CSIRX_SetDphyErrIrqMaskCfg,
 *  CSIRX_GetMonitorIrqs, CSIRX_SetMonitorIrqs,
 *  CSIRX_GetDphyErrStatusIrq, CSIRX_SetDphyErrStatusIrq,
 *  CSIRX_GetAsfIrqs, CSIRX_GetAsfIrqs,
 *  CSIRX_GetAsfIrqMaskCfg, CSIRX_GetAsfIrqMaskCfg,
 *  CSIRX_GetAsfIrqTest, CSIRX_GetAsfIrqTest
 */
static int32_t CsirxCovCsl_testIrqFunctions(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    uint32_t status;
    CSIRX_PrivateData *pD;
    CSIRX_InfoIrqsMaskCfg infoIrqsMaskCfg;
    CSIRX_ErrorIrqsMaskCfg errIrqsMaskCfg;
    CSIRX_InfoIrqs infoIrqs;
    CSIRX_ErrorIrqs errIrqs;
    CSIRX_AsfIrqs asfIrqs;
    CSIRX_AsfIrqMaskCfg asfIrqMask;
    CSIRX_AsfIrqTest asfIrqTest;
    CSIRX_MonitorIrqsMaskCfg monIrqMask;
    CSIRX_DphyErrIrqMaskCfg dphyErrIrqMask;
    CSIRX_MonitorIrqs monIrqs;
    CSIRX_DphyErrStatusIrq dphyErrStatus;

    if ((Fvid2_Handle)NULL == gCovCslSharedHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV_CSL: No shared handle, skipping\r\n");
        return CSIRX_COV_TEST_FAIL;
    }

    pD = &gCsirxCommonObj.instObj[gCovCslInstId].cslObj.cslCfgData;

    (void)memset(&monIrqMask, 0, sizeof(monIrqMask));
    (void)memset(&dphyErrIrqMask, 0, sizeof(dphyErrIrqMask));
    (void)memset(&monIrqs, 0, sizeof(monIrqs));
    (void)memset(&dphyErrStatus, 0, sizeof(dphyErrStatus));
    (void)memset(&infoIrqsMaskCfg, 0, sizeof(infoIrqsMaskCfg));
    (void)memset(&infoIrqs, 0, sizeof(infoIrqs));
    (void)memset(&errIrqs, 0, sizeof(errIrqs));
    (void)memset(&errIrqsMaskCfg, 0, sizeof(errIrqsMaskCfg));
    (void)memset(&asfIrqs, 0, sizeof(asfIrqs));
    (void)memset(&asfIrqMask, 0, sizeof(asfIrqMask));
    (void)memset(&asfIrqTest, 0, sizeof(asfIrqTest));

    /* CSIRX_GetInfoIrqsMaskCfg */
    status = CSIRX_GetInfoIrqsMaskCfg(pD, &infoIrqsMaskCfg);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetInfoIrqsMaskCfg(pD, NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetInfoIrqsMaskCfg(NULL, &infoIrqsMaskCfg);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_SetInfoIrqsMaskCfg */
    status = CSIRX_SetInfoIrqsMaskCfg(pD, &infoIrqsMaskCfg);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetInfoIrqsMaskCfg(pD, NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetInfoIrqsMaskCfg(NULL, &infoIrqsMaskCfg);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_GetMonitorIrqsMaskCfg */
    status = CSIRX_GetMonitorIrqsMaskCfg(pD, &monIrqMask);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetMonitorIrqsMaskCfg(pD, NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetMonitorIrqsMaskCfg(NULL, &monIrqMask);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_SetMonitorIrqsMaskCfg */
    status = CSIRX_SetMonitorIrqsMaskCfg(pD, &monIrqMask);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetMonitorIrqsMaskCfg(pD, NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetMonitorIrqsMaskCfg(NULL, &monIrqMask);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_GetErrorIrqsMaskCfg */
    status = CSIRX_GetErrorIrqsMaskCfg(pD, &errIrqsMaskCfg);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetErrorIrqsMaskCfg(pD, NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetErrorIrqsMaskCfg(NULL, &errIrqsMaskCfg);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_SetErrorIrqsMaskCfg */
    status = CSIRX_SetErrorIrqsMaskCfg(pD, &errIrqsMaskCfg);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetErrorIrqsMaskCfg(pD, NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetErrorIrqsMaskCfg(NULL, &errIrqsMaskCfg);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_GetDphyErrIrqMaskCfg */
    status = CSIRX_GetDphyErrIrqMaskCfg(pD, &dphyErrIrqMask);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetDphyErrIrqMaskCfg(pD, NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetDphyErrIrqMaskCfg(NULL, &dphyErrIrqMask);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_SetDphyErrIrqMaskCfg */
    status = CSIRX_SetDphyErrIrqMaskCfg(pD, &dphyErrIrqMask);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetDphyErrIrqMaskCfg(pD, NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetDphyErrIrqMaskCfg(NULL, &dphyErrIrqMask);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_GetInfoIrqs */
    status = CSIRX_GetInfoIrqs(pD, &infoIrqs);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetInfoIrqs(pD, NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetInfoIrqs(NULL, &infoIrqs);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_SetInfoIrqs */
    status = CSIRX_SetInfoIrqs(pD, &infoIrqs);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetInfoIrqs(pD, NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetInfoIrqs(NULL, &infoIrqs);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_GetMonitorIrqs */
    status = CSIRX_GetMonitorIrqs(pD, &monIrqs);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetMonitorIrqs(pD, NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetMonitorIrqs(NULL, &monIrqs);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_SetMonitorIrqs */
    status = CSIRX_SetMonitorIrqs(pD, &monIrqs);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetMonitorIrqs(pD, NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetMonitorIrqs(NULL, &monIrqs);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_GetErrorIrqs */
    status = CSIRX_GetErrorIrqs(pD, &errIrqs);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetErrorIrqs(pD, NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetErrorIrqs(NULL, &errIrqs);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_SetErrorIrqs */
    status = CSIRX_SetErrorIrqs(pD, &errIrqs);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetErrorIrqs(pD, NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetErrorIrqs(NULL, &errIrqs);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_GetDphyErrStatusIrq */
    status = CSIRX_GetDphyErrStatusIrq(pD, &dphyErrStatus);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetDphyErrStatusIrq(pD, NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetDphyErrStatusIrq(NULL, &dphyErrStatus);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_SetDphyErrStatusIrq */
    status = CSIRX_SetDphyErrStatusIrq(pD, &dphyErrStatus);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetDphyErrStatusIrq(pD, NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetDphyErrStatusIrq(NULL, &dphyErrStatus);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_GetAsfIrqs */
    status = CSIRX_GetAsfIrqs(pD, &asfIrqs);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetAsfIrqs(pD, NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetAsfIrqs(NULL, &asfIrqs);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_SetAsfIrqs */
    status = CSIRX_SetAsfIrqs(pD, &asfIrqs);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetAsfIrqs(pD, NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetAsfIrqs(NULL, &asfIrqs);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    (void)memset(&asfIrqs, 0xFF, sizeof(asfIrqs));
    status = CSIRX_SetAsfIrqs(pD, &asfIrqs);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_GetAsfIrqMaskCfg */
    status = CSIRX_GetAsfIrqMaskCfg(pD, &asfIrqMask);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetAsfIrqMaskCfg(pD, NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetAsfIrqMaskCfg(NULL, &asfIrqMask);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_SetAsfIrqMaskCfg */
    status = CSIRX_SetAsfIrqMaskCfg(pD, &asfIrqMask);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetAsfIrqMaskCfg(pD, NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetAsfIrqMaskCfg(NULL, &asfIrqMask);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    (void)memset(&asfIrqMask, 0xFF, sizeof(asfIrqMask));
    status = CSIRX_SetAsfIrqMaskCfg(pD, &asfIrqMask);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_GetTestAsfIrqs */
    status = CSIRX_GetTestAsfIrqs(pD, &asfIrqTest);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetTestAsfIrqs(pD, NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetTestAsfIrqs(NULL, &asfIrqTest);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_SetTestAsfIrqs */
    status = CSIRX_SetTestAsfIrqs(pD, &asfIrqTest);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetTestAsfIrqs(pD, NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetTestAsfIrqs(NULL, &asfIrqTest);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    (void)memset(&asfIrqTest, 0xFF, sizeof(asfIrqTest));
    status = CSIRX_SetTestAsfIrqs(pD, &asfIrqTest);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    (void)memset(&monIrqs, 0xFF, sizeof(monIrqs));
    (void)memset(&monIrqMask, 0xFF, sizeof(monIrqMask));
    (void)memset(&infoIrqs, 0xFF, sizeof(infoIrqs));
    (void)memset(&infoIrqsMaskCfg, 0xFF, sizeof(infoIrqsMaskCfg));
    (void)memset(&errIrqs, 0xFF, sizeof(errIrqs));
    (void)memset(&errIrqsMaskCfg, 0xFF, sizeof(errIrqsMaskCfg));

    /*  CSIRX_AsfIrqsSF */
    status = CSIRX_AsfIrqsSF(NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /*  CSIRX_AsfIrqMaskCfgSF */
    status = CSIRX_AsfIrqMaskCfgSF(NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /*  CSIRX_AsfIrqTestSF */
    status = CSIRX_AsfIrqTestSF(NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /*  CSIRX_MonitorIrqsSF */
    status = CSIRX_MonitorIrqsSF(&monIrqs);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /*  CSIRX_MonitorIrqsMaskCfgSF */
    status = CSIRX_MonitorIrqsMaskCfgSF(&monIrqMask);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /*  CSIRX_InfoIrqsSF */
    status = CSIRX_InfoIrqsSF(&infoIrqs);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /*  CSIRX_InfoIrqsMaskCfgSF */
    status = CSIRX_InfoIrqsMaskCfgSF(&infoIrqsMaskCfg);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /*  CSIRX_ErrorIrqsSF */
    status = CSIRX_ErrorIrqsSF(&errIrqs);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /*  CSIRX_ErrorIrqsMaskCfgSF */
    status = CSIRX_ErrorIrqsMaskCfgSF(&errIrqsMaskCfg);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV_CSL: IRQ functions test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

/**
 *  \brief CSL coverage test for D-PHY functions:
 *  CSIRX_GetDphyLaneControl, CSIRX_GetDphyStatus
 */
static int32_t CsirxCovCsl_testDphyFunctions(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    uint32_t status;
    CSIRX_PrivateData *pD;
    CSIRX_DphyLaneControl dphyLaneCtrl;
    CSIRX_DphyStatus dphyStatus;
    CSIRX_DphyErrStatusIrq dphyErrStatus;
    CSIRX_DphyErrIrqMaskCfg dphyErrIrqMask;

    if ((Fvid2_Handle)NULL == gCovCslSharedHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV_CSL: No shared handle, skipping\r\n");
        return CSIRX_COV_TEST_FAIL;
    }

    pD = &gCsirxCommonObj.instObj[gCovCslInstId].cslObj.cslCfgData;

    (void)memset(&dphyLaneCtrl, 0, sizeof(dphyLaneCtrl));
    (void)memset(&dphyStatus, 0, sizeof(dphyStatus));

    /* CSIRX_GetDphyLaneControl */
    status = CSIRX_GetDphyLaneControl(pD, &dphyLaneCtrl);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetDphyLaneControl((CSIRX_PrivateData*)NULL, &dphyLaneCtrl);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetDphyLaneControl(pD, (CSIRX_DphyLaneControl*)NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_SetDphyLaneControl */
    status = CSIRX_SetDphyLaneControl(pD, &dphyLaneCtrl);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetDphyLaneControl((CSIRX_PrivateData*)NULL, &dphyLaneCtrl);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetDphyLaneControl(pD, (CSIRX_DphyLaneControl*)NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_GetDphyStatus */
    status = CSIRX_GetDphyStatus(pD, &dphyStatus);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetDphyStatus((CSIRX_PrivateData*)NULL, &dphyStatus);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetDphyStatus(pD, (CSIRX_DphyStatus*)NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    (void)memset(&dphyLaneCtrl, 0xFF, sizeof(dphyLaneCtrl));
    (void)memset(&dphyErrStatus, 0xFF, sizeof(dphyErrStatus));
    (void)memset(&dphyErrIrqMask, 0xFF, sizeof(dphyErrIrqMask));

    /* CSIRX_DphyLaneControlSF */
    status = CSIRX_DphyLaneControlSF(&dphyLaneCtrl);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_DphyLaneControlSF */
    status = CSIRX_DphyErrStatusIrqSF(&dphyErrStatus);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_DphyErrIrqMaskCfgSF */
    status = CSIRX_DphyErrIrqMaskCfgSF(&dphyErrIrqMask);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV_CSL: D-PHY functions test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

/**
 *  \brief CSL coverage test for ASF functions. */
static int32_t CsirxCovCsl_testAsfFunctions(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    uint32_t status;
    CSIRX_PrivateData *pD;
    CSIRX_AsfInfo asfInfo;
    CSIRX_AsfFatalNonFatalSelect asfFatalNonfatal;
    CSIRX_AsfTransToFaultStatus asfFaultStatus;
    CSIRX_AsfTransToFaultMask asfFaultMask;
    CSIRX_AsfTransToCtrl asfTransToCtrl;

    if ((Fvid2_Handle)NULL == gCovCslSharedHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV_CSL: No shared handle, skipping\r\n");
        return CSIRX_COV_TEST_FAIL;
    }

    pD = &gCsirxCommonObj.instObj[gCovCslInstId].cslObj.cslCfgData;

    (void)memset(&asfInfo, 0, sizeof(asfInfo));
    (void)memset(&asfFatalNonfatal, 0, sizeof(asfFatalNonfatal));
    (void)memset(&asfFaultStatus, 0, sizeof(asfFaultStatus));
    (void)memset(&asfFaultMask, 0, sizeof(asfFaultMask));
    (void)memset(&asfTransToCtrl, 0, sizeof(asfTransToCtrl));

    /* CSIRX_AsfTransToCtrlSF */
    status = CSIRX_AsfTransToCtrlSF( NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_AsfTransToFaultMaskSF */
    status = CSIRX_AsfTransToFaultMaskSF( NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_AsfTransToFaultStatusSF */
    status = CSIRX_AsfTransToFaultStatusSF( &asfFaultStatus);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_AsfTransToFaultStatusSF( NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_AsfFatalNonFatalSelectSF */
    status = CSIRX_AsfFatalNonFatalSelectSF( NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_GetAsfInfo */
    status = CSIRX_GetAsfInfo(pD, &asfInfo);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetAsfInfo((CSIRX_PrivateData *)NULL, &asfInfo);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetAsfInfo(pD, (CSIRX_AsfInfo*)NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    uint8_t originConfig = pD->deviceConfig.asfConfig;
    pD->deviceConfig.asfConfig = 0U;
    status = CSIRX_GetAsfInfo(pD, &asfInfo);
    if (CDN_ENOTSUP != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    pD->deviceConfig.asfConfig = originConfig;

    /* CSIRX_GetAsfFatalNonfatal */
    status = CSIRX_GetAsfFatalNonfatal(pD, &asfFatalNonfatal);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetAsfFatalNonfatal((CSIRX_PrivateData *)NULL, &asfFatalNonfatal);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetAsfFatalNonfatal(pD, NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_SetAsfFatalNonfatal */
    status = CSIRX_SetAsfFatalNonfatal(pD, &asfFatalNonfatal);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetAsfFatalNonfatal((CSIRX_PrivateData *)NULL, &asfFatalNonfatal);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetAsfFatalNonfatal(pD, NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    (void)memset(&asfFatalNonfatal, 0xFF, sizeof(asfFatalNonfatal));
    status = CSIRX_SetAsfFatalNonfatal(pD, &asfFatalNonfatal);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_GetAsfTransToFaultStatus */
    status = CSIRX_GetAsfTransToFaultStatus(pD, &asfFaultStatus);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetAsfTransToFaultStatus((CSIRX_PrivateData *)NULL, &asfFaultStatus);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetAsfTransToFaultStatus(pD, NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_GetAsfTransToFaultMask */
    status = CSIRX_GetAsfTransToFaultMask(pD, &asfFaultMask);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetAsfTransToFaultMask((CSIRX_PrivateData *)NULL, &asfFaultMask);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetAsfTransToFaultMask(pD, NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_SetAsfTransToFaultMask */
    status = CSIRX_SetAsfTransToFaultMask(pD, &asfFaultMask);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetAsfTransToFaultMask((CSIRX_PrivateData *)NULL, &asfFaultMask);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetAsfTransToFaultMask(pD, NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    (void)memset(&asfFaultMask, 0xFF, sizeof(asfFaultMask));
    status = CSIRX_SetAsfTransToFaultMask(pD, &asfFaultMask);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_SetAsfTransMonToCtrl */
    status = CSIRX_SetAsfTransMonToCtrl(pD, &asfTransToCtrl);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetAsfTransMonToCtrl((CSIRX_PrivateData *)NULL, &asfTransToCtrl);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetAsfTransMonToCtrl(pD, NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    (void)memset(&asfTransToCtrl, 0xFF, sizeof(asfTransToCtrl));
    status = CSIRX_SetAsfTransMonToCtrl(pD, &asfTransToCtrl);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    status = CSIRX_AsfTransToCtrlSF( &asfTransToCtrl);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_AsfTransToFaultStatusSF */
    (void)memset(&asfFaultStatus, 0xFF, sizeof(asfFaultStatus));
    status = CSIRX_AsfTransToFaultStatusSF( &asfFaultStatus);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV_CSL: ASF functions test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

/**
 *  \brief CSL coverage test for Probe and Destroy functions. */
static int32_t CsirxCovCsl_testProbe(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    uint32_t status;
    CSIRX_SysReq sysReq;
    CSIRX_Config config;

    if ((Fvid2_Handle)NULL == gCovCslSharedHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV_CSL: No shared handle, skipping\r\n");
        return CSIRX_COV_TEST_FAIL;
    }

    (void)memset(&sysReq, 0, sizeof(sysReq));
    (void)memset(&config, 0, sizeof(config));

    /* Use the same config params that were used during driver init */
    config.regBase =
        gCsirxCommonObj.instObj[gCovCslInstId].cslObj.configParams.regBase;

    /* CSIRX_Probe */
    status = CSIRX_Probe(&config, &sysReq);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_Probe(&config, (CSIRX_SysReq*)NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_Probe((CSIRX_Config *)NULL, &sysReq);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    CSIRX_Config fakeConfig;
    (void)memset(&fakeConfig, 0, sizeof(fakeConfig));
    (void)memset(gFakeCsirxRegBuf, 0, sizeof(gFakeCsirxRegBuf));
    fakeConfig.regBase = (CSIRX_Regs *)gFakeCsirxRegBuf;

    status = CSIRX_Probe(&fakeConfig, &sysReq);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_Destroy */
    CSIRX_Destroy();

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV_CSL: Probe/Destroy test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

/**
 *  \brief CSL coverage test for static config and debug getter functions.
 *
 *  Calls uncovered CSL functions:
 *  CSIRX_GetSoftReset, CSIRX_SetSoftReset,
 *  CSIRX_GetStaticCfg, CSIRX_GetStaticCfg,
 *  CSIRX_GetErrorBypassCfg, CSIRX_SetErrorBypassCfg,
 *  CSIRX_GetIntegrationDebug, CSIRX_GetErrorDebug, CSIRX_GetTestGeneric
 */
static int32_t CsirxCovCsl_testCfg(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    uint32_t status;
    CSIRX_PrivateData *pD;
    CSIRX_SoftReset softReset;
    CSIRX_StaticCfg staticCfg;
    CSIRX_ErrorBypassCfg errBypassCfg;
    CSIRX_IntegrationDebug intDebug;
    CSIRX_ErrorDebug errDebug;
    CSIRX_TestGeneric testGeneric;
    CSIRX_DeviceConfig devCfg;
    CSIRX_Config config;

    if ((Fvid2_Handle)NULL == gCovCslSharedHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV_CSL: No shared handle, skipping\r\n");
        return CSIRX_COV_TEST_FAIL;
    }

    pD = &gCsirxCommonObj.instObj[gCovCslInstId].cslObj.cslCfgData;

    (void)memset(&softReset, 0, sizeof(softReset));
    (void)memset(&staticCfg, 0, sizeof(staticCfg));
    (void)memset(&errBypassCfg, 0, sizeof(errBypassCfg));
    (void)memset(&intDebug, 0, sizeof(intDebug));
    (void)memset(&errDebug, 0, sizeof(errDebug));
    (void)memset(&testGeneric, 0, sizeof(testGeneric));
    (void)memset(&devCfg, 0, sizeof(devCfg));
    (void)memset(&config, 0, sizeof(config));

    /* Call with NULL pD */
    status = CSIRX_Init(NULL, NULL, NULL);
    if (CDN_EINVAL != status)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Expected CDN_EINVAL for NULL Init params\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_Init(pD, NULL, NULL);
    if (CDN_EINVAL != status)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Expected CDN_EINVAL for NULL Init params\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_Init(pD, &config, NULL);
    if (CDN_EINVAL != status)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Expected CDN_EINVAL for NULL Init params\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_GetDeviceConfig */
    status = CSIRX_GetDeviceConfig(pD, &devCfg);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetDeviceConfig((CSIRX_PrivateData*)NULL, &devCfg);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetDeviceConfig(pD, (CSIRX_DeviceConfig*)NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_GetSoftReset */
    status = CSIRX_GetSoftReset(pD, &softReset);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetSoftReset((CSIRX_PrivateData*)NULL, &softReset);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetSoftReset(pD, (CSIRX_SoftReset*)NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_SetSoftReset */
    status = CSIRX_SetSoftReset(pD, &softReset);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetSoftReset((CSIRX_PrivateData*)NULL, &softReset);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetSoftReset(pD, (CSIRX_SoftReset*)NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    (void)memset(&softReset, 0xFF, sizeof(softReset));
    status = CSIRX_SetSoftReset(pD, &softReset);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_GetStaticCfg */
    status = CSIRX_GetStaticCfg(pD, &staticCfg);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStaticCfg((CSIRX_PrivateData*)NULL, &staticCfg);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetStaticCfg(pD, (CSIRX_StaticCfg*)NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_SetStaticCfg */
    status = CSIRX_SetStaticCfg(pD, &staticCfg);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStaticCfg((CSIRX_PrivateData*)NULL, &staticCfg);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetStaticCfg(pD, (CSIRX_StaticCfg*)NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    (void)memset(&staticCfg, 0xFF, sizeof(staticCfg));
    status = CSIRX_SetStaticCfg(pD, &staticCfg);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_GetErrorBypassCfg */
    status = CSIRX_GetErrorBypassCfg(pD, &errBypassCfg);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetErrorBypassCfg((CSIRX_PrivateData*)NULL, &errBypassCfg);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetErrorBypassCfg(pD, (CSIRX_ErrorBypassCfg*)NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_SetErrorBypassCfg */
    status = CSIRX_SetErrorBypassCfg(pD, &errBypassCfg);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetErrorBypassCfg((CSIRX_PrivateData*)NULL, &errBypassCfg);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_SetErrorBypassCfg(pD, (CSIRX_ErrorBypassCfg*)NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_GetIntegrationDebug */
    status = CSIRX_GetIntegrationDebug(pD, &intDebug);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetIntegrationDebug((CSIRX_PrivateData*)NULL, &intDebug);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetIntegrationDebug(pD, (CSIRX_IntegrationDebug*)NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_GetErrorDebug */
    status = CSIRX_GetErrorDebug(pD, &errDebug);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetErrorDebug((CSIRX_PrivateData*)NULL, &errDebug);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetErrorDebug(pD, (CSIRX_ErrorDebug*)NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSIRX_GetTestGeneric */
    status = CSIRX_GetTestGeneric(pD, &testGeneric);
    if (CDN_EOK != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetTestGeneric((CSIRX_PrivateData*)NULL, &testGeneric);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSIRX_GetTestGeneric(pD, (CSIRX_TestGeneric*)NULL);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    (void)memset(&errBypassCfg, 0xFF, sizeof(errBypassCfg));

    /* CSIRX_ErrorBypassCfgSF */
    status = CSIRX_ErrorBypassCfgSF(&errBypassCfg);
    if (CDN_EINVAL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV_CSL: Static config & debug test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

/**
 *  \brief CSL coverage test for shim IF register functions:
 *  CSL_csirxGetRevisionId, CSL_csirxEnableVP, CSL_csirxConfigVP,
 *  CSL_csirxAssertPixelIfReset, CSL_csirxConfigDMA
 */
static int32_t CsirxCovCsl_testIfRegFunctions(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    int32_t status;
    CsirxDrv_InstObj *instObj;
    CSL_csi_rx_ifRegs *ifRegisters;
    CSL_CsirxRevisionId revId;
    CSL_CsirxVPConfig vpCfg;
    CSL_CsirxDMAConfig dmaCfg;

    if ((Fvid2_Handle)NULL == gCovCslSharedHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV_CSL: No shared handle, skipping\r\n");
        return CSIRX_COV_TEST_FAIL;
    }

    instObj = &gCsirxCommonObj.instObj[gCovCslInstId];
    ifRegisters = (CSL_csi_rx_ifRegs *)(uintptr_t)instObj->shimBaseAddr;

    /* CSL_csirxGetRevisionId */
    (void)memset(&revId, 0, sizeof(revId));
    CSL_csirxGetRevisionId(ifRegisters, &revId);
    GT_1trace(gAppTrace, GT_INFO,
              " CSL_csirxGetRevisionId: major=%d\r\n", revId.major);

    /* --- CSL_csirxEnableVP: 5 calls to cover all branches --- */
    status = CSL_csirxEnableVP(ifRegisters, CSL_CSIRX_VP_INSTANCE_0, UTRUE);
    if (CSL_PASS != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSL_csirxEnableVP(ifRegisters, CSL_CSIRX_VP_INSTANCE_0, UFALSE);
    if (CSL_PASS != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSL_csirxEnableVP(ifRegisters, CSL_CSIRX_VP_INSTANCE_1, UTRUE);
    if (CSL_PASS != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSL_csirxEnableVP(ifRegisters, CSL_CSIRX_VP_INSTANCE_1, UFALSE);
    if (CSL_PASS != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    status = CSL_csirxEnableVP(ifRegisters, 99U, UTRUE);
    if (CSL_EFAIL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    (void)memset(&vpCfg, 0, sizeof(vpCfg));
    vpCfg.instance = CSL_CSIRX_VP_INSTANCE_0;
    status = CSL_csirxConfigVP(ifRegisters, &vpCfg);
    if (CSL_PASS != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    vpCfg.instance = CSL_CSIRX_VP_INSTANCE_1;
    status = CSL_csirxConfigVP(ifRegisters, &vpCfg);
    if (CSL_PASS != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    vpCfg.instance = 99U;
    status = CSL_csirxConfigVP(ifRegisters, &vpCfg);
    if (CSL_EFAIL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSL_csirxAssertPixelIfReset */
    CSL_csirxAssertPixelIfReset(ifRegisters, UTRUE);
    CSL_csirxAssertPixelIfReset(ifRegisters, UFALSE);

    /* CSL_csirxConfigDMA */
    (void)memset(&dmaCfg, 0, sizeof(dmaCfg));
    dmaCfg.chNum = 0U;
    status = CSL_csirxConfigDMA(ifRegisters, &dmaCfg);
    if (CSL_PASS != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }
    dmaCfg.chNum = CSL_CSIRX_PSI_L_THREAD_NUM_MAX;
    status = CSL_csirxConfigDMA(ifRegisters, &dmaCfg);
    if (CSL_EFAIL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV_CSL: IF register functions test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

/**
 *  \brief CSL coverage test for interrupt distributor (INTD) functions.
 *
 *  Calls uncovered CSL functions that operate on CSL_csi_rx_intd_cfgRegs:
 *  CSL_setCsirxIntdIntEnable, CSL_getCsirxIntdIntEnable,
 *  CSL_setCsirxIntdIntEnableClear, CSL_setCsirxIntdStatusClear,
 *  CSL_getCsirxIntdStatus, CSL_csirxIntdsetEOI
 *
 *  Each branching function is called with LEVEL type, PULSE type, and
 *  an invalid type to cover all 3 paths.
 */
static int32_t CsirxCovCsl_testIntdFunctions(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    int32_t status;
    CsirxDrv_InstObj *instObj;
    CSL_csi_rx_intd_cfgRegs *intdRegs;
    CSIRX_CpIntd_cfg intdCfg;

    if ((Fvid2_Handle)NULL == gCovCslSharedHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV_CSL: No shared handle, skipping\r\n");
        return CSIRX_COV_TEST_FAIL;
    }

    instObj = &gCsirxCommonObj.instObj[gCovCslInstId];
    intdRegs = (CSL_csi_rx_intd_cfgRegs *)(uintptr_t)instObj->cpIntdBaseAddr;

    /* CSL_setCsirxIntdIntEnable */
    (void)memset(&intdCfg, 0, sizeof(intdCfg));
    intdCfg.intr_mask = 0U;
    intdCfg.type = CSIRX_INTD_INT_TYPE_LEVEL;
    status = CSL_setCsirxIntdIntEnable(intdRegs, &intdCfg);
    if (CSL_PASS != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    intdCfg.type = CSIRX_INTD_INT_TYPE_PULSE;
    status = CSL_setCsirxIntdIntEnable(intdRegs, &intdCfg);
    if (CSL_PASS != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    intdCfg.type = 99U;
    status = CSL_setCsirxIntdIntEnable(intdRegs, &intdCfg);
    if (CSL_EFAIL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSL_getCsirxIntdIntEnable */
    intdCfg.type = CSIRX_INTD_INT_TYPE_LEVEL;
    status = CSL_getCsirxIntdIntEnable(intdRegs, &intdCfg);
    if (CSL_PASS != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    intdCfg.type = CSIRX_INTD_INT_TYPE_PULSE;
    status = CSL_getCsirxIntdIntEnable(intdRegs, &intdCfg);
    if (CSL_PASS != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    intdCfg.type = 99U;
    status = CSL_getCsirxIntdIntEnable(intdRegs, &intdCfg);
    if (CSL_EFAIL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSL_setCsirxIntdIntEnableClear */
    intdCfg.type = CSIRX_INTD_INT_TYPE_LEVEL;
    status = CSL_setCsirxIntdIntEnableClear(intdRegs, &intdCfg);
    if (CSL_PASS != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    intdCfg.type = CSIRX_INTD_INT_TYPE_PULSE;
    status = CSL_setCsirxIntdIntEnableClear(intdRegs, &intdCfg);
    if (CSL_PASS != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    intdCfg.type = 99U;
    status = CSL_setCsirxIntdIntEnableClear(intdRegs, &intdCfg);
    if (CSL_EFAIL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSL_setCsirxIntdStatusClear */
    intdCfg.type = CSIRX_INTD_INT_TYPE_LEVEL;
    status = CSL_setCsirxIntdStatusClear(intdRegs, &intdCfg);
    if (CSL_PASS != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    intdCfg.type = CSIRX_INTD_INT_TYPE_PULSE;
    status = CSL_setCsirxIntdStatusClear(intdRegs, &intdCfg);
    if (CSL_PASS != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    intdCfg.type = 99U;
    status = CSL_setCsirxIntdStatusClear(intdRegs, &intdCfg);
    if (CSL_EFAIL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* --- CSL_getCsirxIntdStatus: LEVEL / PULSE / invalid --- */
    intdCfg.type = CSIRX_INTD_INT_TYPE_LEVEL;
    status = CSL_getCsirxIntdStatus(intdRegs, &intdCfg);
    if (CSL_PASS != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    intdCfg.type = CSIRX_INTD_INT_TYPE_PULSE;
    status = CSL_getCsirxIntdStatus(intdRegs, &intdCfg);
    if (CSL_PASS != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    intdCfg.type = 99U;
    status = CSL_getCsirxIntdStatus(intdRegs, &intdCfg);
    if (CSL_EFAIL != status)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* CSL_csirxIntdsetEOI */
    CSL_csirxIntdsetEOI(intdRegs, 0U);

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV_CSL: INTD functions test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

/* Dummy ISR handler callbacks for exercising ISR branches with fake regs */
static void CsirxCovCsl_dummyInfoHandler(CSIRX_PrivateData *pD,
                                          CSIRX_InfoIrqs *val)
{
    (void)pD;
    (void)val;
}

static void CsirxCovCsl_dummyMonitorHandler(CSIRX_PrivateData *pD,
                                             CSIRX_MonitorIrqs *val)
{
    (void)pD;
    (void)val;
}

static void CsirxCovCsl_dummyDphyHandler(CSIRX_PrivateData *pD,
                                          CSIRX_DphyErrStatusIrq *val)
{
    (void)pD;
    (void)val;
}

/**
 * Test CSIRX_Isr function */
static int32_t CsirxCovCsl_testIsrFunction(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    CSIRX_PrivateData *pD;

    /* Avoid unused parameter warning */
    (void)taskObj;

    if ((Fvid2_Handle)NULL == gCovCslSharedHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV_CSL: No shared handle, skipping\r\n");
        return CSIRX_COV_TEST_FAIL;
    }

    pD = &gCsirxCommonObj.instObj[gCovCslInstId].cslObj.cslCfgData;

    /* Call ISR with valid pD */
    CSIRX_Isr(pD);

    /* Call ISR with NULL pD */
    CSIRX_Isr(NULL);

    /*
     * Exercise ISR branches where IRQ status registers are non-zero.
     * Redirect pD->regs to the fake register buffer with info_irqs,
     * monitor_irqs and dphy_err_status_irq set to non-zero, and install
     * dummy handler callbacks so the function pointer calls succeed.
     */
    {
        CSIRX_Regs *origRegs           = pD->regs;
        CSIRX_InfoHandler origInfoH    = pD->infoHandler;
        CSIRX_MonitorHandler origMonH  = pD->monitorHandler;
        CSIRX_DphyHandler origDphyH    = pD->dphyErrorHandler;
        CSIRX_Regs *fakeRegs           = (CSIRX_Regs *)(void *)gFakeCsirxRegBuf;

        /* Zero out fake buffer, then set IRQ registers to non-zero */
        (void)memset(gFakeCsirxRegBuf, 0, sizeof(gFakeCsirxRegBuf));
        fakeRegs->info_irqs            = 1U;
        fakeRegs->monitor_irqs         = 1U;
        fakeRegs->dphy_err_status_irq  = 1U;

        /* Install dummy handlers and redirect regs */
        pD->infoHandler      = &CsirxCovCsl_dummyInfoHandler;
        pD->monitorHandler   = &CsirxCovCsl_dummyMonitorHandler;
        pD->dphyErrorHandler = &CsirxCovCsl_dummyDphyHandler;
        pD->regs             = fakeRegs;

        CSIRX_Isr(pD);

        /* Restore originals */
        pD->regs             = origRegs;
        pD->infoHandler      = origInfoH;
        pD->monitorHandler   = origMonH;
        pD->dphyErrorHandler = origDphyH;
    }

    /*
     * Exercise ISR branches where IRQ status registers are non-zero.
     * Redirect pD->regs to the fake register buffer with info_irqs,
     * monitor_irqs and dphy_err_status_irq set to non-zero, and install
     * dummy handler callbacks so the function pointer calls succeed.
     */
    {
        CSIRX_Regs *origRegs           = pD->regs;
        CSIRX_InfoHandler origInfoH    = pD->infoHandler;
        CSIRX_MonitorHandler origMonH  = pD->monitorHandler;
        CSIRX_DphyHandler origDphyH    = pD->dphyErrorHandler;
        CSIRX_Regs *fakeRegs           = (CSIRX_Regs *)(void *)gFakeCsirxRegBuf;

        /* Zero out fake buffer, then set IRQ registers to non-zero */
        (void)memset(gFakeCsirxRegBuf, 0, sizeof(gFakeCsirxRegBuf));
        fakeRegs->info_irqs            = 0U;
        fakeRegs->monitor_irqs         = 1U;
        fakeRegs->dphy_err_status_irq  = 0U;

        /* Install dummy handlers and redirect regs */
        pD->infoHandler      = &CsirxCovCsl_dummyInfoHandler;
        pD->monitorHandler   = &CsirxCovCsl_dummyMonitorHandler;
        pD->dphyErrorHandler = &CsirxCovCsl_dummyDphyHandler;
        pD->regs             = fakeRegs;

        CSIRX_Isr(pD);

        /* Restore originals */
        pD->regs             = origRegs;
        pD->infoHandler      = origInfoH;
        pD->monitorHandler   = origMonH;
        pD->dphyErrorHandler = origDphyH;
    }

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV_CSL: ISR function test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}
