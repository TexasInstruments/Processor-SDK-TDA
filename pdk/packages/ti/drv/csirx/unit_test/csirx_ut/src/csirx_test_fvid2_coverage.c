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
 *  \file csirx_test_fvid2_coverage.c
 *
 *  \brief CSIRX FVID2 Library coverage improvement tests.
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

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* Mirror of internal Fdm_Driver (fvid2_drvMgr.c) */
typedef struct
{
    const Fvid2_DrvOps *drvOps;
    uint32_t            numOpens;
    uint32_t            isUsed;
} LocalFdmDriver;

/* Mirror of internal Fdm_Channel (fvid2_drvMgr.c) */
typedef struct
{
    LocalFdmDriver *drv;
    Fdrv_Handle     drvHandle;
    Fvid2_CbParams  cbParams;
    uint32_t        isUsed;
} LocalFdmChannel;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

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


/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/* Test buffer for memsetw/memcmp tests */
static uint32_t gTestWordBuf[256];

/* Mock callback flags for driver callback testing */
static uint32_t gMockErrCbInvoked = 0U;
static uint32_t gMockTraceStCbInvoked = 0U;
static uint32_t gMockTraceEndCbInvoked = 0U;

/** \brief Frame buffer memory for coverage tests */
static uint8_t gCovFrmBuf[CSIRX_COV_NUM_FRAMES][CSIRX_COV_FRAME_SIZE]
    __attribute__((aligned(128), section(".data_buffer")));

/** \brief Frame drop buffer for coverage tests */
static uint8_t gCovFrmDropBuf[CSIRX_COV_FRAME_SIZE]
    __attribute__((aligned(128), section(".data_buffer")));

static void    CsirxCov_initCreateParams(Csirx_CreateParams *createParams,
                                         uint32_t numCh,
                                         uint32_t chType,
                                         uint32_t dataType);
static int32_t CsirxCov_frameCompletionCb(Fvid2_Handle handle,
                                          Ptr appData);
static int32_t testProcessRequest(Fdrv_Handle      handle,
                                           Fvid2_FrameList *inProcessList,
                                           Fvid2_FrameList *outProcessList,
                                           uint32_t         timeout)
{
    GT_0trace(gAppTrace, GT_INFO, "In Process Request Function\r\n");

    return FVID2_SOK;
}

static int32_t testgetProcessRequest(Fdrv_Handle      handle,
                                           Fvid2_FrameList *inProcessList,
                                           Fvid2_FrameList *outProcessList,
                                           uint32_t         timeout)
{
    GT_0trace(gAppTrace, GT_INFO, "In Get Process Request Function\r\n");

    return FVID2_SOK;
}

/**
 *  \brief Mock error callback for testing fdmDriverErrCbFxn
 */
static int32_t CsirxFvid2Cov_mockErrCallback(Fvid2_Handle handle,
                                             void *appData,
                                             void *errList)
{
    gMockErrCbInvoked++;
    GT_0trace(gAppTrace, GT_INFO1,
              " Mock error callback invoked\r\n");
    return FVID2_SOK;
}

/**
 *  \brief Mock trace start callback for testing fdmDriverTraceStCbFxn
 */
static int32_t CsirxFvid2Cov_mockTraceStCallback(Fvid2_Handle handle,
                                                 void *appData)
{
    gMockTraceStCbInvoked++;
    GT_0trace(gAppTrace, GT_INFO1,
              " Mock trace start callback invoked\r\n");
    return FVID2_SOK;
}

/**
 *  \brief Mock trace end callback for testing fdmDriverTraceEndCbFxn
 */
static int32_t CsirxFvid2Cov_mockTraceEndCallback(Fvid2_Handle handle,
                                                  void *appData)
{
    gMockTraceEndCbInvoked++;
    GT_0trace(gAppTrace, GT_INFO1,
              " Mock trace end callback invoked\r\n");
    return FVID2_SOK;
}

/* This test passes NULL pointers to all major FVID2 API entry points
 * to ensure the driver's parameter validation branches are exercised.
 * This test also executes Queue/Dequeue negative tests.
 */
int32_t CsirxCov_testNullAndNegArgs(CsirxTestTaskObj *taskObj)
{
    int32_t retVal = FVID2_SOK;
    int32_t testResult = CSIRX_COV_TEST_PASS;
    Fvid2_Handle drvHandle = NULL;
    Fvid2_FrameList frmList;
    Fvid2_Frame frm;
    Csirx_CreateParams createParams;
    Csirx_CreateStatus createStatus;
    Fvid2_CbParams cbPrms;
    uint32_t instId;
    Csirx_EventPrms eventPrms;
    Fvid2_FrameList frameList;

    Fvid2Utils_memset(&frameList, 0, sizeof(Fvid2_FrameList));
    frameList.numFrames = 0U;

    drvHandle = Fvid2_create(CSIRX_CAPT_DRV_ID,
                             0U,
                             NULL,     /* NULL createArgs */
                             NULL,     /* NULL createStatus */
                             NULL);    /* NULL cbPrms */
    if (NULL != drvHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: NULL - create with NULL args should fail!\r\n");
        (void)Fvid2_delete(drvHandle, NULL);
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* Fvid2_queue with NULL handle */
    Fvid2FrameList_init(&frmList);
    retVal = Fvid2_queue(NULL, &frmList, 0U);
    if (FVID2_SOK == retVal)
    {
        GT_0trace(gAppTrace, GT_INFO,
                  " CSIRX_COV: NULL - queue with NULL handle not rejected\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* Fvid2_dequeue with NULL handle */
    retVal = Fvid2_dequeue(NULL, &frmList, 0U, FVID2_TIMEOUT_NONE);
    if (FVID2_SOK == retVal)
    {
        GT_0trace(gAppTrace, GT_INFO,
                  " CSIRX_COV: NULL - dequeue with NULL handle not rejected\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* Fvid2_control with NULL handle */
    retVal = Fvid2_control(NULL, IOCTL_CSIRX_GET_INST_STATUS, NULL, NULL);
    if (FVID2_SOK == retVal)
    {
        GT_0trace(gAppTrace, GT_INFO,
                  " CSIRX_COV: NULL - control with NULL handle not rejected\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* Fvid2_start with NULL handle */
    retVal = Fvid2_start(NULL, NULL);
    if (FVID2_SOK == retVal)
    {
        GT_0trace(gAppTrace, GT_INFO,
                  " CSIRX_COV: NULL - start with NULL handle not rejected\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* Fvid2_stop with NULL handle */
    retVal = Fvid2_stop(NULL, NULL);
    if (FVID2_SOK == retVal)
    {
        GT_0trace(gAppTrace, GT_INFO,
                  " CSIRX_COV: NULL - stop with NULL handle not rejected\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* Fvid2_delete with NULL handle */
    retVal = Fvid2_delete(NULL, NULL);
    if (FVID2_SOK == retVal)
    {
        GT_0trace(gAppTrace, GT_INFO,
                  " CSIRX_COV: NULL - delete with NULL handle not rejected\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* FVID2 Utils Init */
    if (NULL != drvHandle)
    {
        retVal = Fvid2Utils_deInit(NULL);
        GT_1trace(gAppTrace, GT_INFO,
                  " CSIRX_COV: FVID2 Utils Deinit: %d\r\n",
                  retVal);
    }

    /* Negative Tests */
    Csirx_eventPrmsInit(&eventPrms);
    instId = taskObj->instObj.instCfgInfo->csiDrvInst;
    CsirxCov_initCreateParams(&createParams, 0U,
                              CSIRX_CH_TYPE_CAPT,
                              FVID2_CSI2_DF_RAW12);
    Fvid2CbParams_init(&cbPrms);
    cbPrms.cbFxn = &CsirxCov_frameCompletionCb;
    cbPrms.appData = (Ptr)taskObj;
    drvHandle = Fvid2_create(CSIRX_CAPT_DRV_ID,
                                instId,
                                (void *)&createParams,
                                (void *)&createStatus,
                                &cbPrms);;
    if (NULL == drvHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: QueueNeg - gCovSharedHandle is NULL\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* Queue with NULL frame pointer in the list. */
    if (NULL != drvHandle)
    {
        Fvid2FrameList_init(&frmList);
        frmList.numFrames = 1U;
        frmList.frames[0] = NULL;

        retVal = Fvid2_queue(drvHandle, &frmList, 0U);
        if(retVal != FVID2_EBADARGS)
        {
            GT_1trace(gAppTrace, GT_INFO,
                  " CSIRX_COV: QueueNeg - NULL frame returned: %d\r\n",
                  retVal);
            testResult = CSIRX_COV_TEST_FAIL;
        }
    }

    if (NULL != drvHandle)
    {
        Fvid2Frame_init(&frm);
        frm.addr[0] = (uint64_t)(uintptr_t)&gCovFrmBuf[0][0];
        frm.chNum = 0U;

        Fvid2FrameList_init(&frmList);
        frmList.numFrames = 1U;
        frmList.frames[0] = &frm;

        /* Queue with valid state but numFrames=0 */
        Fvid2FrameList_init(&frmList);
        frmList.numFrames = 0U;
        retVal = Fvid2_queue(drvHandle, &frmList, 0U);
        if(retVal != FVID2_EOUT_OF_RANGE)
        {
            GT_1trace(gAppTrace, GT_INFO,
                  " CSIRX_COV: MC/DC - queue with 0 frames: %d\r\n",
                  retVal);
            testResult = CSIRX_COV_TEST_FAIL;
        }
    }

    /* Queue with a valid frame but on invalid stream ID */
    if (NULL != drvHandle)
    {
        Fvid2Frame_init(&frm);
        frm.addr[0] = (uint64_t)(uintptr_t)&gCovFrmBuf[0][0];
        frm.chNum   = 0U;

        Fvid2FrameList_init(&frmList);
        frmList.numFrames = 1U;
        frmList.frames[0] = &frm;

        /* Use invalid stream ID (0xFFFF) */
        retVal = Fvid2_queue(drvHandle, &frmList, 0xFFFFU);
        if(retVal != FVID2_EFAIL)
        {
            GT_1trace(gAppTrace, GT_INFO,
                  " CSIRX_COV: QueueNeg - invalid streamId returned: %d\r\n",
                  retVal);
            testResult = CSIRX_COV_TEST_FAIL;
        }
    }

    /* Queue a valid frame before starting the driver. */
    if (NULL != drvHandle)
    {
        Fvid2Frame_init(&frm);
        frm.addr[0] = (uint64_t)(uintptr_t)&gCovFrmBuf[0][0];
        frm.chNum   = 0U;

        Fvid2FrameList_init(&frmList);
        frmList.numFrames = 1U;
        frmList.frames[0] = &frm;

        retVal = Fvid2_queue(drvHandle, &frmList, 0U);
        if(retVal != FVID2_EFAIL)
        {
            GT_1trace(gAppTrace, GT_INFO,
                  " CSIRX_COV: QueueNeg - queue before start returned: %d\r\n",
                  retVal);
            testResult = CSIRX_COV_TEST_FAIL;
        }
    }

    /* Dequeue with invalid stream ID */
    if (NULL != drvHandle)
    {
        Fvid2FrameList_init(&frmList);
        retVal = Fvid2_dequeue(drvHandle, &frmList, 0xFFFFU,
                               FVID2_TIMEOUT_NONE);
        if(retVal != FVID2_ENO_MORE_BUFFERS)
        {
            GT_1trace(gAppTrace, GT_INFO,
                  " CSIRX_COV: QueueNeg - dequeue invalid streamId"
                  " returned: %d\r\n", retVal);
            testResult = CSIRX_COV_TEST_FAIL;
        }
    }

    /* Test processRequest with NULL handle - should fail */
    retVal = Fvid2_processRequest(NULL, &frameList, &frameList, 0U);
    if (retVal == FVID2_SOK)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Fvid2_processRequest NULL handle should fail\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* Test getProcessedRequest with NULL handle - should fail */
    retVal = Fvid2_getProcessedRequest(NULL, &frameList, &frameList, 0U);
    if (retVal == FVID2_SOK)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Fvid2_getProcessedRequest NULL handle should fail\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* Clean up shared handle before returning */
    if (NULL != drvHandle)
    {
        (void)Fvid2_delete(drvHandle, NULL);
        drvHandle = NULL;
    }

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: NULL pointer args and Negative test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

/* To Test Edge Cases of Dequeue Operation */
int32_t CsirxCov_testDequeueEdgeCase(CsirxTestTaskObj *taskObj)
{
    int32_t retVal = FVID2_SOK;
    int32_t testResult = CSIRX_COV_TEST_PASS;
    Fvid2_Handle drvHandle = NULL;
    Fvid2_FrameList frmList;
    Csirx_CreateParams createParams;
    Csirx_CreateStatus createStatus;
    Fvid2_CbParams cbPrms;
    uint32_t instId;

    instId = taskObj->instObj.instCfgInfo->csiDrvInst;
    CsirxCov_initCreateParams(&createParams, 0U,
                              CSIRX_CH_TYPE_CAPT,
                              FVID2_CSI2_DF_RAW12);
    Fvid2CbParams_init(&cbPrms);
    cbPrms.cbFxn = &CsirxCov_frameCompletionCb;
    cbPrms.appData = (Ptr)taskObj;
    drvHandle = Fvid2_create(CSIRX_CAPT_DRV_ID,
                                instId,
                                (void *)&createParams,
                                (void *)&createStatus,
                                &cbPrms);
    if (NULL == drvHandle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: QueueNeg - gCovSharedHandle is NULL\r\n");
        testResult = CSIRX_COV_TEST_FAIL;
    }

    /* Dequeue on empty queue with TIMEOUT_NONE */
    if (NULL != drvHandle)
    {
        Fvid2FrameList_init(&frmList);
        retVal = Fvid2_dequeue(drvHandle,
                               &frmList,
                               0U,
                               FVID2_TIMEOUT_NONE);
        if (FVID2_SOK == retVal)
        {
            GT_1trace(gAppTrace, GT_INFO,
                      " CSIRX_COV: Dequeue - empty queue returned %d frames"
                      " (unexpected but OK for coverage)\r\n",
                      frmList.numFrames);
        }
    }

    /* Dequeue with NULL frame list pointer */
    if (NULL != drvHandle)
    {
        retVal = Fvid2_dequeue(drvHandle,
                               NULL,
                               0U,
                               FVID2_TIMEOUT_NONE);
        if (FVID2_SOK == retVal)
        {
            GT_0trace(gAppTrace, GT_INFO,
                      " CSIRX_COV: Dequeue - NULL frmList not rejected\r\n");
            testResult = CSIRX_COV_TEST_FAIL;
        }
    }

    /* Dequeue with per-channel dequeue mode */
    if (NULL != drvHandle)
    {
        /* Dequeue on per-channel mode with empty queue */
        Fvid2FrameList_init(&frmList);
        retVal = Fvid2_dequeue(drvHandle,
                               &frmList,
                               0U,
                               FVID2_TIMEOUT_NONE);
        if (FVID2_ENO_MORE_BUFFERS != retVal)
        {
            GT_1trace(gAppTrace, GT_INFO,
                      " CSIRX_COV: Dequeue - per-ch mode returned: %d\r\n", retVal);
            testResult = CSIRX_COV_TEST_FAIL;
        }
    }

    /* Clean up shared handle before returning */
    if (NULL != drvHandle)
    {
        (void)Fvid2_delete(drvHandle, NULL);
        drvHandle = NULL;
    }

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: Dequeue edge case test - %s\r\n",
              (CSIRX_COV_TEST_PASS == testResult) ? "PASS" : "FAIL");

    return testResult;
}

/* To Test Fvid2 Utility Functions */
static int32_t CsirxFvid2Cov_testUtils(void)
{
    int32_t retVal = CSIRX_COV_TEST_PASS;
    int32_t result;
    void *result_memsetw;
    uint32_t i, wordCount = 64U;
    uint32_t testWord = 0xDEADBEEFU;
    uint8_t buf1[128], buf2[128], buf3[128];
    const char *versionStr;
    uint32_t versionNum;
    Fvid2_ModeInfo modeInfo;
    const char *dataFmtStr;
    const char *stdStr;

    /* Test with valid standard */
    stdStr = Fvid2_getStandardString(FVID2_STD_1080P_60);
    if (NULL == stdStr)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Fvid2_getStandardString 1080P valid returned NULL\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Test with another valid standard */
    stdStr = Fvid2_getStandardString(FVID2_STD_720P_60);
    if (NULL == stdStr)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Fvid2_getStandardString 720P valid returned NULL\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Test with invalid standard - should return default string */
    stdStr = Fvid2_getStandardString(0xFFFFFFFFU);
    if (NULL == stdStr)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Fvid2_getStandardString invalid returned NULL\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Test with valid data format */
    dataFmtStr = Fvid2_getDataFmtString(FVID2_DF_RGB24_888);
    if (NULL == dataFmtStr)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Fvid2_getDataFmtString valid format returned NULL\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Test with another valid data format */
    dataFmtStr = Fvid2_getDataFmtString(FVID2_DF_YUV422I_UYVY);
    if (NULL == dataFmtStr)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Fvid2_getDataFmtString UYVY format returned NULL\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Test with invalid data format - should return default string */
    dataFmtStr = Fvid2_getDataFmtString(0xFFFFFFFFU);
    if (NULL == dataFmtStr)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Fvid2_getDataFmtString invalid format returned NULL\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Initialize mode info with default standard */
    Fvid2Utils_memset(&modeInfo, 0, sizeof(Fvid2_ModeInfo));
    modeInfo.standard = FVID2_STD_1080P_60;

    /* Test getModeInfo with valid standard */
    result = Fvid2_getModeInfo(&modeInfo);
    if (result != FVID2_SOK)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Fvid2_getModeInfo with valid standard FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Test getModeInfo with NULL pointer */
    result = Fvid2_getModeInfo(NULL);
    if (result == FVID2_SOK)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Fvid2_getModeInfo NULL test should fail\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Test getModeInfo with invalid standard */
    modeInfo.standard = 0xFFFFFFFFU;
    result = Fvid2_getModeInfo(&modeInfo);
    if (result == FVID2_SOK)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Fvid2_getModeInfo invalid standard should fail\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Test getVersionString */
    versionStr = Fvid2_getVersionString();
    if (NULL == versionStr)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Fvid2_getVersionString returned NULL\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Test getVersionNumber */
    versionNum = Fvid2_getVersionNumber();
    if (0U == versionNum)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Fvid2_getVersionNumber returned 0\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    result = Fvid2Utils_init();
    if (result != FVID2_SOK)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Fvid2Utils_init FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    result_memsetw = Fvid2Utils_memsetw(gTestWordBuf, testWord, wordCount);
    if (result_memsetw != (void *)gTestWordBuf)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Fvid2Utils_memsetw return value FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    for (i = 0U; i < wordCount; i++)
    {
        if (gTestWordBuf[i] != testWord)
        {
            GT_0trace(gAppTrace, GT_ERR,
                      " Fvid2Utils_memsetw content verification FAILED\r\n");
            retVal = CSIRX_COV_TEST_FAIL;
            break;
        }
    }

    Fvid2Utils_memset(buf1, 0xAB, 128);
    Fvid2Utils_memset(buf2, 0xAB, 128);
    Fvid2Utils_memset(buf3, 0xCD, 128);

    /* Compare identical buffers */
    result = Fvid2Utils_memcmp(buf1, buf2, 128);
    if (result != 0)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Fvid2Utils_memcmp identical FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Compare different buffers */
    result = Fvid2Utils_memcmp(buf1, buf3, 128);
    if (result == 0)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Fvid2Utils_memcmp different FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    result = Fvid2Utils_deInit(NULL);
    if (result != FVID2_SOK)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Fvid2Utils_deInit FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: FVID2 Utility Tests - %s\r\n",
              (CSIRX_COV_TEST_PASS == retVal) ? "PASS" : "FAIL");

    return retVal;
}

/* To test Fvid2Utils_constructLinkList and double linked list operations */
static int32_t CsirxFvid2Cov_testDoubleLinkList(void)
{
    int32_t retVal = CSIRX_COV_TEST_PASS;
    int32_t result;
    Fvid2UtilsLinkListObj llobj;
    Fvid2Utils_Node node1, node2;
    uint32_t data1 = 1U, data2 = 2U;
    Fvid2Utils_Node *pNode;

    /* Test constructLinkList with double list and BOTTOM mode */
    result = Fvid2Utils_constructLinkList(&llobj, FVID2UTILS_LLT_DOUBLE,
                                          FVID2UTILS_LAM_BOTTOM);
    if (result != FVID2_SOK)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Fvid2Utils_constructLinkList FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Test getNumNodes without any nodes */
    uint32_t numNodes = Fvid2Utils_getNumNodes((Fvid2Utils_Handle)&llobj);
    if (numNodes != 0U)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " getNumNodes FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Test linkNodeToHead */
    node1.data = (void *)&data1;
    Fvid2Utils_linkNodeToHead((Fvid2Utils_Handle)&llobj, &node1);

    if (Fvid2Utils_isListEmpty((Fvid2Utils_Handle)&llobj) == (uint32_t)TRUE)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " isListEmpty after linkNodeToHead FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Test getHeadNode */
    pNode = Fvid2Utils_getHeadNode((Fvid2Utils_Handle)&llobj);
    if (pNode != &node1)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " getHeadNode FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Test linkNodeToTail */
    node2.data = (void *)&data2;
    Fvid2Utils_linkNodeToTail((Fvid2Utils_Handle)&llobj, &node2);

    pNode = Fvid2Utils_getTailNode((Fvid2Utils_Handle)&llobj);
    if (pNode != &node2)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " getTailNode FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Test getNumNodes */
    numNodes = Fvid2Utils_getNumNodes((Fvid2Utils_Handle)&llobj);
    if (numNodes != 2U)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " getNumNodes FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Test unLinkNodeFromHead */
    pNode = Fvid2Utils_unLinkNodeFromHead((Fvid2Utils_Handle)&llobj);
    if (pNode != &node1)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " unLinkNodeFromHead FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Test unLinkNodeFromTail */
    pNode = Fvid2Utils_unLinkNodeFromTail((Fvid2Utils_Handle)&llobj);
    if (pNode != &node2)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " unLinkNodeFromTail FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    if (Fvid2Utils_isListEmpty((Fvid2Utils_Handle)&llobj) == (uint32_t)FALSE)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " isListEmpty after removal FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Ensure list is empty before destruction */
    Fvid2Utils_unLinkAllNodes((Fvid2Utils_Handle)&llobj);

    Fvid2Utils_destructLinkList(&llobj);

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: Test Operations on Double Linked List - %s\r\n",
              (CSIRX_COV_TEST_PASS == retVal) ? "PASS" : "FAIL");

    return retVal;
}

/* Test priority-based linked list operations */
static int32_t CsirxFvid2Cov_testPriorityLinkList(void)
{
    int32_t retVal = CSIRX_COV_TEST_PASS;
    int32_t result;
    Fvid2UtilsLinkListObj llobj;
    Fvid2Utils_Node node1, node2, node3;
    uint32_t data1 = 10U, data2 = 20U, data3 = 30U;
    Fvid2Utils_Node *pNode;

    /* Test constructLinkList with double list and PRIORITY mode */
    result = Fvid2Utils_constructLinkList(&llobj, FVID2UTILS_LLT_DOUBLE,
                                          FVID2UTILS_LAM_PRIORITY);
    if (result != FVID2_SOK)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Fvid2Utils_constructLinkList PRIORITY FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Add nodes with different priorities */
    node1.data = (void *)&data1;
    node1.priority = 2U;
    Fvid2Utils_linkNodePri((Fvid2Utils_Handle)&llobj, &node1, 2U);

    node2.data = (void *)&data2;
    node2.priority = 1U;
    Fvid2Utils_linkNodePri((Fvid2Utils_Handle)&llobj, &node2, 1U);

    node3.data = (void *)&data3;
    node3.priority = 3U;
    Fvid2Utils_linkNodePri((Fvid2Utils_Handle)&llobj, &node3, 3U);

    /* Verify order: node2 (pri 1), node1 (pri 2), node3 (pri 3) */
    pNode = Fvid2Utils_getHeadNode((Fvid2Utils_Handle)&llobj);
    if (pNode != &node2)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Priority order FAILED at head\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Test unLinkNodePri */
    Fvid2Utils_unLinkNodePri((Fvid2Utils_Handle)&llobj, &node2);

    pNode = Fvid2Utils_getHeadNode((Fvid2Utils_Handle)&llobj);
    if (pNode != &node1)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " unLinkNodePri FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Test unLinkNode */
    Fvid2Utils_unLinkNode((Fvid2Utils_Handle)&llobj, &node1);
    Fvid2Utils_unLinkNode((Fvid2Utils_Handle)&llobj, &node3);

    /* Ensure list is empty before destruction */
    Fvid2Utils_unLinkAllNodes((Fvid2Utils_Handle)&llobj);

    /* Test destructLinkList */
    Fvid2Utils_destructLinkList(&llobj);

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: Test Operations on Priority Double Linked List - %s\r\n",
              (CSIRX_COV_TEST_PASS == retVal) ? "PASS" : "FAIL");

    return retVal;
}

/**
 *  \brief Test unique priority linked list and unLinkAllNodes
 */
static int32_t CsirxFvid2Cov_testUniqueAndUnlinkAll(void)
{
    int32_t retVal = CSIRX_COV_TEST_PASS;
    int32_t result;
    Fvid2UtilsLinkListObj llobj;
    Fvid2Utils_Node node1, node2, node3;
    uint32_t data1 = 100U, data2 = 200U, data3 = 300U;
    uint32_t numNodes;

    /* Test constructLinkList with TOP mode for unique tests */
    result = Fvid2Utils_constructLinkList(&llobj, FVID2UTILS_LLT_DOUBLE,
                                          FVID2UTILS_LAM_TOP);
    if (result != FVID2_SOK)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Fvid2Utils_constructLinkList TOP FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    node1.data = (void *)&data1;
    node2.data = (void *)&data2;
    node3.data = (void *)&data3;

    /* Test linkUniqePriNode */
    result = Fvid2Utils_linkUniqePriNode((Fvid2Utils_Handle)&llobj, &node1, 0U);
    if (result != FVID2_SOK)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " linkUniqePriNode first FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    result = Fvid2Utils_linkUniqePriNode((Fvid2Utils_Handle)&llobj, &node2, 1U);
    if (result != FVID2_SOK)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " linkUniqePriNode second FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    numNodes = Fvid2Utils_getNodeCnt((Fvid2Utils_Handle)&llobj);
    if (numNodes != 2U)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " getNodeCnt FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Test unLinkAllNodes */
    result = Fvid2Utils_unLinkAllNodes((Fvid2Utils_Handle)&llobj);
    if (result != 0)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " unLinkAllNodes FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    if (Fvid2Utils_isListEmpty((Fvid2Utils_Handle)&llobj) == (uint32_t)FALSE)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " List not empty after unLinkAllNodes\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    Fvid2Utils_destructLinkList(&llobj);

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: Test Operations on Unique Priority Linked List - %s\r\n",
              (CSIRX_COV_TEST_PASS == retVal) ? "PASS" : "FAIL");

    return retVal;
}

/* To test Queue operations */
static int32_t CsirxFvid2Cov_testQueueOps(void)
{
    int32_t retVal = CSIRX_COV_TEST_PASS;
    int32_t result;
    Fvid2UtilsLinkListObj qobj;
    Fvid2Utils_QElem qelem1, qelem2, qelem3;
    uint32_t qdata1 = 111U, qdata2 = 222U, qdata3 = 333U;
    void *pData;
    uint32_t numElem;

    /* Test constructQ */
    result = Fvid2Utils_constructQ(&qobj);
    if (result != FVID2_SOK)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Fvid2Utils_constructQ FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Test isQEmpty on empty queue */
    if (Fvid2Utils_isQEmpty((Fvid2Utils_QHandle)&qobj) != (uint32_t)TRUE)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " isQEmpty on empty queue FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }
    else
    {
        void *peekedData;

        /* Call Fvid2Utils_peakHead on empty queue */
        peekedData = Fvid2Utils_peakHead((Fvid2Utils_QHandle)&qobj);

        /* Verify return value is NULL */
        if (peekedData != NULL)
        {
            GT_1trace(gAppTrace, GT_ERR,"ERROR: Expected NULL for empty queue, got %p\n", peekedData);
            return CSIRX_COV_TEST_FAIL;
        }

        peekedData = Fvid2Utils_peakTail((Fvid2Utils_QHandle)&qobj);
        /* Verify return value is NULL */
        if (peekedData != NULL)
        {
            GT_1trace(gAppTrace, GT_ERR,"ERROR: Expected NULL for empty queue, got %p\n", peekedData);
            return CSIRX_COV_TEST_FAIL;
        }
    }

    /* Test queue */
    Fvid2Utils_queue((Fvid2Utils_QHandle)&qobj, &qelem1, (void *)&qdata1);
    Fvid2Utils_queue((Fvid2Utils_QHandle)&qobj, &qelem2, (void *)&qdata2);
    Fvid2Utils_queue((Fvid2Utils_QHandle)&qobj, &qelem3, (void *)&qdata3);

    if (Fvid2Utils_isQEmpty((Fvid2Utils_QHandle)&qobj) != (uint32_t)FALSE)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " isQEmpty on non-empty queue FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Test getNumQElem */
    numElem = Fvid2Utils_getNumQElem((Fvid2Utils_QHandle)&qobj);
    if (numElem != 3U)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " getNumQElem FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Test peakHead */
    pData = Fvid2Utils_peakHead((Fvid2Utils_QHandle)&qobj);
    if (pData != (void *)&qdata1)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " peakHead FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Test peakTail */
    pData = Fvid2Utils_peakTail((Fvid2Utils_QHandle)&qobj);
    if (pData != (void *)&qdata3)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " peakTail FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Test dequeue */
    pData = Fvid2Utils_dequeue((Fvid2Utils_QHandle)&qobj);
    if (pData != (void *)&qdata1)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " dequeue first FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    pData = Fvid2Utils_dequeue((Fvid2Utils_QHandle)&qobj);
    if (pData != (void *)&qdata2)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " dequeue second FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    pData = Fvid2Utils_dequeue((Fvid2Utils_QHandle)&qobj);
    if (pData != (void *)&qdata3)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " dequeue third FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Ensure queue is empty before destruction */
    Fvid2Utils_unLinkAllNodes((Fvid2Utils_QHandle)&qobj);

    /* Test destructQ */
    Fvid2Utils_destructQ(&qobj);

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: Test Queue Operations - %s\r\n",
              (CSIRX_COV_TEST_PASS == retVal) ? "PASS" : "FAIL");

    return retVal;
}

/* To test queueBack operation */
static int32_t CsirxFvid2Cov_testQueueBack(void)
{
    int32_t retVal = CSIRX_COV_TEST_PASS;
    Fvid2UtilsLinkListObj qobj;
    Fvid2Utils_QElem qelem1, qelem2;
    uint32_t qdata1 = 444U, qdata2 = 555U;
    void *pData;

    Fvid2Utils_constructQ(&qobj);

    /* Queue first element */
    Fvid2Utils_queue((Fvid2Utils_QHandle)&qobj, &qelem1, (void *)&qdata1);

    /* Queue back (add to head) */
    Fvid2Utils_queueBack((Fvid2Utils_QHandle)&qobj, &qelem2, (void *)&qdata2);

    /* Verify qelem2 is now at head */
    pData = Fvid2Utils_peakHead((Fvid2Utils_QHandle)&qobj);
    if (pData != (void *)&qdata2)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " queueBack FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Dequeue and verify order */
    pData = Fvid2Utils_dequeue((Fvid2Utils_QHandle)&qobj);
    if (pData != (void *)&qdata2)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " queueBack dequeue order FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    pData = Fvid2Utils_dequeue((Fvid2Utils_QHandle)&qobj);
    if (pData != (void *)&qdata1)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " queueBack second dequeue FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Ensure queue is empty before destruction */
    Fvid2Utils_unLinkAllNodes((Fvid2Utils_QHandle)&qobj);

    Fvid2Utils_destructQ(&qobj);

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: Test QueueBack Operations - %s\r\n",
              (CSIRX_COV_TEST_PASS == retVal) ? "PASS" : "FAIL");

    return retVal;
}

/* To test circular linked list operations */
static int32_t CsirxFvid2Cov_testCircularLinkList(void)
{
    int32_t retVal = CSIRX_COV_TEST_PASS;
    int32_t result;
    Fvid2UtilsLinkListObj llobj;
    Fvid2Utils_Node node1, node2, node3;
    uint32_t data1 = 11U, data2 = 22U, data3 = 33U;

    /* Test circular list with non-priority add mode (should fail) */
    result = Fvid2Utils_constructLinkList(&llobj, FVID2UTILS_LLT_CIRCULAR,
                                          FVID2UTILS_LAM_TOP);
    if (result == FVID2_SOK)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Circular with TOP mode should have failed\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Test constructLinkList with circular list */
    result = Fvid2Utils_constructLinkList(&llobj, FVID2UTILS_LLT_CIRCULAR,
                                          FVID2UTILS_LAM_PRIORITY);
    if (result != FVID2_SOK)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Fvid2Utils_constructLinkList CIRCULAR FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    node1.data = (void *)&data1;
    node1.priority = 2U;
    Fvid2Utils_linkNodePri((Fvid2Utils_Handle)&llobj, &node1, 2U);

    node2.data = (void *)&data2;
    node2.priority = 1U;
    Fvid2Utils_linkNodePri((Fvid2Utils_Handle)&llobj, &node2, 1U);

    node3.data = (void *)&data3;
    node3.priority = 3U;
    Fvid2Utils_linkNodePri((Fvid2Utils_Handle)&llobj, &node3, 3U);

    /* Verify nodes are added */
    uint32_t numNodes = Fvid2Utils_getNumNodes((Fvid2Utils_Handle)&llobj);
    if (numNodes != 3U)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Circular list node count FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Test unlink operations on circular list */
    Fvid2Utils_unLinkNodePri((Fvid2Utils_Handle)&llobj, &node2);

    numNodes = Fvid2Utils_getNumNodes((Fvid2Utils_Handle)&llobj);
    if (numNodes != 2U)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Circular list after unlink count FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    Fvid2Utils_unLinkNodePri((Fvid2Utils_Handle)&llobj, &node1);
    Fvid2Utils_unLinkNodePri((Fvid2Utils_Handle)&llobj, &node3);

    /* Call getNumNodes on empty circular list: covers the false branch of
     * while (NULL != node) at fvid2_utils.c:665 (headNode == NULL). */
    numNodes = Fvid2Utils_getNumNodes((Fvid2Utils_Handle)&llobj);
    if (numNodes != 0U)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Circular list empty count FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Ensure list is empty before destruction */
    Fvid2Utils_unLinkAllNodes((Fvid2Utils_Handle)&llobj);

    Fvid2Utils_destructLinkList(&llobj);

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: Test Circular List Operations - %s\r\n",
              (CSIRX_COV_TEST_PASS == retVal) ? "PASS" : "FAIL");

    return retVal;
}

/**
 *  \brief Test Fvid2_checkFrameList and frame list utilities
 */
static int32_t CsirxFvid2Cov_testFrameListFuncs(void)
{
    int32_t retVal = CSIRX_COV_TEST_PASS;
    int32_t result;
    Fvid2_FrameList srcFrameList, destFrameList, dupFrameList;
    Fvid2_Frame frame1, frame2;
    uint8_t frameBuf1[256], frameBuf2[256];
    Fvid2_FrameList dqFrameList;
    Fvid2_FrameList frameList;
    Fvid2_Frame     dummyFrame;

    /* Initialize frame lists */
    Fvid2Utils_memset(&frameList, 0, sizeof(frameList));
    Fvid2Utils_memset(&dummyFrame, 0, sizeof(dummyFrame));
    Fvid2Utils_memset(&srcFrameList, 0, sizeof(Fvid2_FrameList));
    Fvid2Utils_memset(&destFrameList, 0, sizeof(Fvid2_FrameList));
    Fvid2Utils_memset(&dupFrameList, 0, sizeof(Fvid2_FrameList));
    Fvid2Utils_memset(&frame1, 0, sizeof(Fvid2_Frame));
    Fvid2Utils_memset(&frame2, 0, sizeof(Fvid2_Frame));

    /* Setup frames */
    frame1.addr[0] = (uint64_t)frameBuf1;
    frame2.addr[0] = (uint64_t)frameBuf2;

    /* Setup source frame list */
    srcFrameList.frames[0] = &frame1;
    srcFrameList.frames[1] = &frame2;
    srcFrameList.numFrames = 2U;

    /* Test checkFrameList with valid input */
    result = Fvid2_checkFrameList(&srcFrameList, 2U);
    if (result != FVID2_SOK)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Fvid2_checkFrameList FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Test checkFrameList with NULL frameList (should fail) */
    result = Fvid2_checkFrameList(NULL, 2U);
    if (result == FVID2_SOK)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Fvid2_checkFrameList NULL test should fail\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Test copyFrameList */
    Fvid2_copyFrameList(&destFrameList, &srcFrameList);
    if (destFrameList.numFrames != srcFrameList.numFrames)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Fvid2_copyFrameList failed\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Test duplicateFrameList */
    Fvid2_duplicateFrameList(&dupFrameList, &srcFrameList);
    if (dupFrameList.numFrames != srcFrameList.numFrames)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Fvid2_duplicateFrameList failed\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Initialize dequeue frame list */
    Fvid2Utils_memset(&dqFrameList, 0, sizeof(Fvid2_FrameList));
    dqFrameList.numFrames = 2U;

    /* Test checkDqFrameList */
    result = Fvid2_checkDqFrameList(&dqFrameList, 2U);
    if (result != FVID2_SOK)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Fvid2_checkDqFrameList FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Test with NULL pointer (should fail) */
    result = Fvid2_checkDqFrameList(NULL, 2U);
    if (result == FVID2_SOK)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Fvid2_checkDqFrameList NULL test should fail\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* numFrames > maxFrames */
    frameList.numFrames = 5U;
    frameList.frames[0] = &dummyFrame;
    frameList.frames[1] = &dummyFrame;
    frameList.frames[2] = &dummyFrame;
    frameList.frames[3] = &dummyFrame;
    frameList.frames[4] = &dummyFrame;

    result = Fvid2_checkFrameList(&frameList, 2U);
    if (FVID2_EOUT_OF_RANGE != result)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " checkFrameList: numFrames > maxFrames should fail\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    frameList.numFrames = FVID2_MAX_FRAME_PTR + 1U;
    result = Fvid2_checkFrameList(&frameList, FVID2_MAX_FRAME_PTR + 2U);
    if (FVID2_EOUT_OF_RANGE != result)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " checkFrameList: numFrames > MAX_FRAME_PTR should fail\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* checkDqFrameList maxFrames > FVID2_MAX_FRAME_PTR */
    frameList.numFrames = 2U;
    result = Fvid2_checkDqFrameList(&frameList, FVID2_MAX_FRAME_PTR + 1U);
    if (FVID2_EOUT_OF_RANGE != result)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " checkDqFrameList: maxFrames > MAX_FRAME_PTR "
                  "should fail\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: Test Frame list utilities - %s\r\n",
              (CSIRX_COV_TEST_PASS == retVal) ? "PASS" : "FAIL");

    return retVal;
}

/* Test GT_1trace through GT_7trace with non-INFO classes */
static int32_t CsirxFvid2Cov_testGTTraceFvid2(void)
{
    int32_t retVal = CSIRX_COV_TEST_PASS;
    uint32_t maskWithErr = (GT_ERR | GT_TraceState_Enable);
    uint32_t maxVal = 0xFFFFFFFFU;
    uint32_t minVal = 0x00000000U;
    uint32_t mixedVal = 0xDEADBEEFU;
    uint32_t maskDisabledErr   = GT_ERR;
    uint32_t maskDisabledInfo1 = GT_INFO1;
    uint32_t maskDisabledDebug = GT_DEBUG;
    uint32_t maskZero          = 0U;
    uint32_t maskWithInfo1 = (GT_INFO1 | GT_TraceState_Enable);
    uint32_t maskInfoClass     = (GT_INFO | GT_TraceState_Enable);
    uint32_t p0 = 10U, p1 = 20U, p2 = 30U, p3 = 40U, p4 = 50U, p5 = 60U, p6 = 70U;

    /* GT_trace1 with GT_ERR - executes fileName/lineNum printf */
    GT_trace1(maskWithErr, GT_ERR, __FILE__, __LINE__,
              "GT_trace1 with GT_ERR: p0=%d\r\n", p0);

    /* GT_trace2 with GT_ERR - executes fileName/lineNum printf */
    GT_trace2(maskWithErr, GT_ERR, __FILE__, __LINE__,
              "GT_trace2 with GT_ERR: p0=%d p1=%d\r\n", p0, p1);

    /* GT_trace3 with GT_ERR - executes fileName/lineNum printf */
    GT_trace3(maskWithErr, GT_ERR, __FILE__, __LINE__,
              "GT_trace3 with GT_ERR: p0=%d p1=%d p2=%d\r\n", p0, p1, p2);

    /* GT_trace4 with GT_ERR - executes fileName/lineNum printf */
    GT_trace4(maskWithErr, GT_ERR, __FILE__, __LINE__,
              "GT_trace4 with GT_ERR: p0=%d p1=%d p2=%d p3=%d\r\n",
              p0, p1, p2, p3);

    /* GT_trace5 with GT_ERR - executes fileName/lineNum printf */
    GT_trace5(maskWithErr, GT_ERR, __FILE__, __LINE__,
              "GT_trace5 with GT_ERR: p0=%d p1=%d p2=%d p3=%d p4=%d\r\n",
              p0, p1, p2, p3, p4);

    /* GT_trace6 with GT_ERR - executes fileName/lineNum printf */
    GT_trace6(maskWithErr, GT_ERR, __FILE__, __LINE__,
              "GT_trace6 with GT_ERR: p0=%d p1=%d p2=%d p3=%d p4=%d p5=%d\r\n",
              p0, p1, p2, p3, p4, p5);

    /* GT_trace7 with GT_ERR - executes fileName/lineNum printf */
    GT_trace7(maskWithErr, GT_ERR, __FILE__, __LINE__,
              "GT_trace7 with GT_ERR: p0=%d p1=%d p2=%d p3=%d p4=%d p5=%d p6=%d\r\n",
              p0, p1, p2, p3, p4, p5, p6);

    /* Test GT_trace7 with GT_ERR mask */
    GT_trace7((GT_ERR | GT_TraceState_Enable), GT_ERR, __FILE__, __LINE__,
              "GT_trace7 with GT_ERR: p0=%d p1=%d p2=%d p3=%d p4=%d p5=%d p6=%d\r\n",
              p0, p1, p2, p3, p4, p5, p6);

    /* Test GT_trace7 with GT_CRIT mask */
    GT_trace7((GT_CRIT | GT_TraceState_Enable), GT_CRIT, __FILE__, __LINE__,
              "GT_trace7 with GT_CRIT: p0=%d p1=%d p2=%d p3=%d p4=%d p5=%d p6=%d\r\n",
              p0, p1, p2, p3, p4, p5, p6);

    /* Test GT_trace7 with GT_INFO mask */
    GT_trace7((GT_INFO | GT_TraceState_Enable), GT_INFO, __FILE__, __LINE__,
              "GT_trace7 with GT_INFO: p0=%d p1=%d p2=%d p3=%d p4=%d p5=%d p6=%d\r\n",
              p0, p1, p2, p3, p4, p5, p6);

    /* Test GT_trace7 with GT_DEBUG mask */
    GT_trace7((GT_DEBUG | GT_TraceState_Enable), GT_DEBUG, __FILE__, __LINE__,
              "GT_trace7 with GT_DEBUG: p0=%d p1=%d p2=%d p3=%d p4=%d p5=%d p6=%d\r\n",
              p0, p1, p2, p3, p4, p5, p6);

    /* Test GT_trace7 with GT_ENTER mask */
    GT_trace7((GT_ENTER | GT_TraceState_Enable), GT_ENTER, __FILE__, __LINE__,
              "GT_trace7 with GT_ENTER: p0=%d p1=%d p2=%d p3=%d p4=%d p5=%d p6=%d\r\n",
              p0, p1, p2, p3, p4, p5, p6);

    /* Test GT_trace7 with GT_LEAVE mask */
    GT_trace7((GT_LEAVE | GT_TraceState_Enable), GT_LEAVE, __FILE__, __LINE__,
              "GT_trace7 with GT_LEAVE: p0=%d p1=%d p2=%d p3=%d p4=%d p5=%d p6=%d\r\n",
              p0, p1, p2, p3, p4, p5, p6);

    /* Test GT_trace0 with various strings */
    GT_trace0(maskWithInfo1, GT_INFO1, __FILE__, __LINE__,
              "\r\nEmpty next line below:\r\n");
    GT_trace0(maskWithInfo1, GT_INFO1, __FILE__, __LINE__, "");
    GT_trace0(maskWithInfo1, GT_INFO1, __FILE__, __LINE__,
              "\r\nSpecial chars: !@#$%^&*()");

    /* Test GT_trace1 with boundary values */
    GT_trace1(maskWithInfo1, GT_INFO1, __FILE__, __LINE__,
              "\r\nGT_trace1 maxVal=%x", maxVal);
    GT_trace1(maskWithInfo1, GT_INFO1, __FILE__, __LINE__,
              "\r\nGT_trace1 minVal=%x", minVal);

    /* Test GT_trace2 with mixed values */
    GT_trace2(maskWithInfo1, GT_INFO1, __FILE__, __LINE__,
              "\r\nGT_trace2 mixed=%x boundary=%x", mixedVal, maxVal);

    /* Test GT_trace3 with negative-like values (as unsigned) */
    GT_trace3(maskWithInfo1, GT_INFO1, __FILE__, __LINE__,
              "\r\nGT_trace3 val1=%d val2=%d val3=%d", minVal, maxVal, mixedVal);

    /* Test GT_trace4 with pointer-like values */
    GT_trace4(maskWithInfo1, GT_INFO1, __FILE__, __LINE__,
              "\r\nGT_trace4 ptr1=%x ptr2=%x ptr3=%x ptr4=%x",
              maxVal, minVal, mixedVal, 0x12345678U);

    /* Test GT_trace5 with string formatting variations */
    GT_trace5(maskWithInfo1, GT_INFO1, __FILE__, __LINE__,
              "\r\nGT_trace5 hex=%x dec=%d val2=%x val3=%u val4=%x",
              0xABCDEF12U, 12345670, 1234567890U, 987654321U, 0xF0F0F0F0U);

    /* Test GT_trace6 with mixed parameter types */
    GT_trace6(maskWithInfo1, GT_INFO1, __FILE__, __LINE__,
              "\r\nGT_trace6 signed=%d unsigned=%u hex=%x val4=%x val5=%x val6=%x",
              -12345, 54321U, 0xCAFEBABEU, 0xDEADBEEFU, 0xBEEFCAFEU, 0xFEEDBEEFU);

    /* Test GT_trace7 with all different conditions combined */
    GT_trace7(maskWithInfo1, GT_INFO1, __FILE__, __LINE__,
              "\r\nGT_trace7 comprehensive: max=%x min=%x mixed=%x v3=%x v4=%x v5=%x v6=%x",
              maxVal, minVal, mixedVal, 0xFFFF0000U, 42U, 0xAAAAAAAAU, 0x55555555U);

    GT_0trace(gAppTrace, GT_INFO1,
              "\r\nTesting GT_trace functions with tracing DISABLED (FALSE branch)\r\n");

    /* Test GT_trace0 with disabled mask - condition FALSE, no output */
    GT_trace0(maskDisabledErr, GT_ERR, __FILE__, __LINE__,
              "\r\nGT_trace0 DISABLED: This should NOT print");

    /* Test GT_trace1 with disabled mask - condition FALSE */
    GT_trace1(maskDisabledInfo1, GT_INFO1, __FILE__, __LINE__,
              "\r\nGT_trace1 DISABLED p0=%d: should NOT print", p0);

    /* Test GT_trace2 with disabled mask - condition FALSE */
    GT_trace2(maskDisabledDebug, GT_DEBUG, __FILE__, __LINE__,
              "\r\nGT_trace2 DISABLED p0=%d p1=%d: should NOT print", p0, p1);

    /* Test GT_trace3 with disabled mask - condition FALSE */
    GT_trace3(maskZero, GT_ERR, __FILE__, __LINE__,
              "\r\nGT_trace3 DISABLED p0=%d p1=%d p2=%d: should NOT print",
              p0, p1, p2);

    /* Test GT_trace4 with disabled mask - condition FALSE */
    GT_trace4(maskDisabledErr, GT_ERR, __FILE__, __LINE__,
              "\r\nGT_trace4 DISABLED p0=%d p1=%d p2=%d p3=%d: should NOT print",
              p0, p1, p2, p3);

    /* Test GT_trace5 with disabled mask - condition FALSE */
    GT_trace5(maskDisabledInfo1, GT_INFO1, __FILE__, __LINE__,
              "\r\nGT_trace5 DISABLED p0=%d p1=%d p2=%d p3=%d p4=%d: should NOT print",
              p0, p1, p2, p3, p4);

    /* Test GT_trace6 with disabled mask - condition FALSE */
    GT_trace6(maskDisabledDebug, GT_DEBUG, __FILE__, __LINE__,
              "\r\nGT_trace6 DISABLED p0=%d p1=%d p2=%d p3=%d p4=%d p5=%d: should NOT print",
              p0, p1, p2, p3, p4, p5);

    /* Test GT_trace7 with disabled mask - condition FALSE */
    GT_trace7(maskZero, GT_ERR, __FILE__, __LINE__,
              "\r\nGT_trace7 DISABLED p0=%d p1=%d p2=%d p3=%d p4=%d p5=%d p6=%d: should NOT print",
              p0, p1, p2, p3, p4, p5, p6);

    GT_trace0(maskWithErr, GT_INFO, __FILE__, __LINE__,
              "GT_trace0 masked: ERR class vs INFO classType (should NOT print)");
    GT_trace1(maskWithErr, GT_INFO, __FILE__, __LINE__,
              "GT_trace1 masked: ERR class vs INFO classType p0=%d (should NOT print)", p0);
    GT_trace2(maskWithErr, GT_INFO1, __FILE__, __LINE__,
              "GT_trace2 masked: ERR class vs INFO1 classType p0=%d p1=%d (should NOT print)",
              p0, p1);
    GT_trace3(maskInfoClass, GT_DEBUG, __FILE__, __LINE__,
              "GT_trace3 masked: INFO class vs DEBUG classType p0=%d p1=%d p2=%d (should NOT print)",
              p0, p1, p2);
    GT_trace4(maskWithErr, GT_DEBUG, __FILE__, __LINE__,
              "GT_trace4 masked: ERR class vs DEBUG classType p0=%d p1=%d p2=%d p3=%d (should NOT print)",
              p0, p1, p2, p3);
    GT_trace5(maskInfoClass, GT_DEBUG, __FILE__, __LINE__,
              "GT_trace5 masked: INFO class vs DEBUG classType p0=%d p1=%d p2=%d p3=%d p4=%d (should NOT print)",
              p0, p1, p2, p3, p4);
    GT_trace6(maskWithErr, GT_INFO1, __FILE__, __LINE__,
              "GT_trace6 masked: ERR class vs INFO1 classType p0=%d p1=%d p2=%d p3=%d p4=%d p5=%d (should NOT print)",
              p0, p1, p2, p3, p4, p5);
    GT_trace7(maskInfoClass, GT_DEBUG, __FILE__, __LINE__,
              "GT_trace7 masked: INFO class vs DEBUG classType p0=%d p1=%d p2=%d p3=%d p4=%d p5=%d p6=%d (should NOT print)",
              p0, p1, p2, p3, p4, p5, p6);

    /* Test Fvid2_printf with various format strings */
    Fvid2_printf("Fvid2_printf: Simple string test\n");
    Fvid2_printf("Fvid2_printf: Integer param = %d\n", 12345);
    Fvid2_printf("Fvid2_printf: Hex param = 0x%x\n", 0xDEADBEEF);
    Fvid2_printf("Fvid2_printf: Multiple params: %d, %x, %s\n", 100, 0xCAFE, "test");
    Fvid2_printf("Fvid2_printf: Float-like format %u\n", 999999U);

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: Test GTTrace with non-INFO classes - %s\r\n",
              (CSIRX_COV_TEST_PASS == retVal) ? "PASS" : "FAIL");

    return retVal;
}

/* Test fdmDriver callback mechanism */
static int32_t CsirxFvid2Cov_testFdmDriverCbFxns(CsirxTestTaskObj *taskObj)
{
    int32_t               retVal = CSIRX_COV_TEST_PASS;
    Fvid2_Handle          handle;
    Csirx_CreateParams    createParams;
    Csirx_CreateStatus    createStatus;
    Fvid2_CbParams        cbPrms;
    uint32_t              instId;
    /* Mirror of fvid2_drvMgr.c Fdm_Channel layout - order must match exactly */
    struct {
        void           *drv;
        void           *drvHandle;
        Fvid2_CbParams  cbParams;
        uint32_t        isUsed;
    } *fdmCh;
    CsirxDrv_VirtContext *virtCtx;

    instId = taskObj->instObj.instCfgInfo->csiDrvInst;

    CsirxCov_initCreateParams(&createParams, 0U,
                              CSIRX_CH_TYPE_CAPT,
                              FVID2_CSI2_DF_RAW12);

    Fvid2CbParams_init(&cbPrms);
    cbPrms.errCbFxn      = CsirxFvid2Cov_mockErrCallback;
    cbPrms.traceStCbFxn  = CsirxFvid2Cov_mockTraceStCallback;
    cbPrms.traceEndCbFxn = CsirxFvid2Cov_mockTraceEndCallback;

    handle = Fvid2_create(CSIRX_CAPT_DRV_ID,
                          instId,
                          (void *)&createParams,
                          (void *)&createStatus,
                          &cbPrms);
    if (NULL == handle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " CSIRX_COV: FdmCb - Fvid2_create failed\r\n");
        return CSIRX_COV_TEST_FAIL;
    }

    /* Extract CsirxDrv_VirtContext through the Fdm_Channel layout.
     * Fvid2_Handle is Fdm_Channel*; Fdm_Channel.drvHandle is the
     * CsirxDrv_VirtContext* returned by CsirxDrv_create. */
    fdmCh   = (void *)handle;
    virtCtx = (CsirxDrv_VirtContext *)fdmCh->drvHandle;

    /* Invoke fdmDriverErrCbFxn via its stored function pointer */
    gMockErrCbInvoked = 0U;
    if (NULL != virtCtx->fdmCbParams.fdmErrCbFxn)
    {
        (void)virtCtx->fdmCbParams.fdmErrCbFxn(virtCtx->fdmCbParams.fdmData,
                                               NULL);
    }
    if (gMockErrCbInvoked != 1U)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " fdmDriverErrCbFxn did not invoke application callback\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Invoke fdmDriverTraceStCbFxn via its stored function pointer */
    gMockTraceStCbInvoked = 0U;
    if (NULL != virtCtx->fdmCbParams.fdmTraceStartCbFxn)
    {
        (void)virtCtx->fdmCbParams.fdmTraceStartCbFxn(
                                        virtCtx->fdmCbParams.fdmData);
    }
    if (gMockTraceStCbInvoked != 1U)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " fdmDriverTraceStCbFxn did not invoke trace start callback\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Invoke fdmDriverTraceEndCbFxn via its stored function pointer */
    gMockTraceEndCbInvoked = 0U;
    if (NULL != virtCtx->fdmCbParams.fdmTraceEndCbFxn)
    {
        (void)virtCtx->fdmCbParams.fdmTraceEndCbFxn(
                                        virtCtx->fdmCbParams.fdmData);
    }
    if (gMockTraceEndCbInvoked != 1U)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " fdmDriverTraceEndCbFxn did not invoke trace end callback\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: Test fdmDriver callback mechanism - %s\r\n",
              (CSIRX_COV_TEST_PASS == retVal) ? "PASS" : "FAIL");

    (void)Fvid2_delete(handle, NULL);

    return retVal;
}

/* Test linkUniqePriNode */
static int32_t CsirxFvid2Cov_testLinkUniqePri(void)
{
    int32_t retVal = CSIRX_COV_TEST_PASS;
    int32_t result;
    Fvid2UtilsLinkListObj llobj;
    Fvid2Utils_Node node1, node2, node3;
    uint32_t data1 = 10U, data2 = 20U, data3 = 30U;

    /* Construct double list with BOTTOM addMode */
    result = Fvid2Utils_constructLinkList(&llobj, FVID2UTILS_LLT_DOUBLE,
                                          FVID2UTILS_LAM_BOTTOM);
    if (result != FVID2_SOK)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " constructLinkList BOTTOM FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    node1.data = (void *)&data1;
    node2.data = (void *)&data2;

    /* linkUniqePriNode with BOTTOM mode - triggers lines 382-385 */
    result = Fvid2Utils_linkUniqePriNode((Fvid2Utils_Handle)&llobj, &node1, 0U);
    if (result != FVID2_SOK)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " linkUniqePriNode BOTTOM first FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    result = Fvid2Utils_linkUniqePriNode((Fvid2Utils_Handle)&llobj, &node2, 0U);
    if (result != FVID2_SOK)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " linkUniqePriNode BOTTOM second FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Cleanup */
    Fvid2Utils_unLinkAllNodes((Fvid2Utils_Handle)&llobj);
    Fvid2Utils_destructLinkList(&llobj);

    /* Construct circular list with PRIORITY addMode */
    result = Fvid2Utils_constructLinkList(&llobj, FVID2UTILS_LLT_CIRCULAR,
                                          FVID2UTILS_LAM_PRIORITY);
    if (result != FVID2_SOK)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " constructLinkList CIRCULAR PRIORITY FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    node1.data = (void *)&data1;
    node2.data = (void *)&data2;
    node3.data = (void *)&data3;

    /* linkUniqePriNode with circular list */
    result = Fvid2Utils_linkUniqePriNode((Fvid2Utils_Handle)&llobj, &node1, 1U);
    if (result != FVID2_SOK)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " linkUniqePriNode CIRCULAR first FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    result = Fvid2Utils_linkUniqePriNode((Fvid2Utils_Handle)&llobj, &node2, 3U);
    if (result != FVID2_SOK)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " linkUniqePriNode CIRCULAR second FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    result = Fvid2Utils_linkUniqePriNode((Fvid2Utils_Handle)&llobj, &node3, 2U);
    if (result != FVID2_SOK)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " linkUniqePriNode CIRCULAR third FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Unlink node that is not the Head Node*/
    Fvid2Utils_unLinkNodePri((Fvid2Utils_Handle)&llobj, &node3);

    /* Verify node3 was removed: node1->next should be node2 */
    if (node1.next != &node2)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Unlink non-head node FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Cleanup */
    Fvid2Utils_unLinkAllNodes((Fvid2Utils_Handle)&llobj);
    Fvid2Utils_destructLinkList(&llobj);

/* Construct double list with PRIORITY addMode */
    result = Fvid2Utils_constructLinkList(&llobj, FVID2UTILS_LLT_DOUBLE,
                                          FVID2UTILS_LAM_PRIORITY);
    if (result != FVID2_SOK)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " constructLinkList PRIORITY FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    node1.data = (void *)&data1;
    node2.data = (void *)&data2;

    /* Add first node with priority 5 */
    result = Fvid2Utils_linkUniqePriNode((Fvid2Utils_Handle)&llobj, &node1, 5U);
    if (result != FVID2_SOK)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " linkUniqePriNode first node FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Add second node with same priority 5 - should fail with FVID2_EBADARGS */
    result = Fvid2Utils_linkUniqePriNode((Fvid2Utils_Handle)&llobj, &node2, 5U);
    if (result != FVID2_EBADARGS)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Duplicate priority should have returned EBADARGS\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Cleanup */
    Fvid2Utils_unLinkAllNodes((Fvid2Utils_Handle)&llobj);
    Fvid2Utils_destructLinkList(&llobj);

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: Test linkUniqePriNode - %s\r\n",
              (CSIRX_COV_TEST_PASS == retVal) ? "PASS" : "FAIL");

    return retVal;
}

/* Test dutilsUnLinkDoublePri */
static int32_t CsirxFvid2Cov_testUnLinkDoublePriNonHead(void)
{
    int32_t retVal = CSIRX_COV_TEST_PASS;
    int32_t result;
    Fvid2UtilsLinkListObj llobj;
    Fvid2Utils_Node node1, node2, node3;
    uint32_t data1 = 10U, data2 = 20U, data3 = 30U;

    /* Construct double list with PRIORITY addMode */
    result = Fvid2Utils_constructLinkList(&llobj, FVID2UTILS_LLT_DOUBLE,
                                          FVID2UTILS_LAM_PRIORITY);
    if (result != FVID2_SOK)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " constructLinkList PRIORITY FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    node1.data = (void *)&data1;
    node2.data = (void *)&data2;
    node3.data = (void *)&data3;

    /* Add 3 nodes: node1 (pri 1) -> node2 (pri 2) -> node3 (pri 3) */
    Fvid2Utils_linkNodePri((Fvid2Utils_Handle)&llobj, &node1, 1U);
    Fvid2Utils_linkNodePri((Fvid2Utils_Handle)&llobj, &node2, 2U);
    Fvid2Utils_linkNodePri((Fvid2Utils_Handle)&llobj, &node3, 3U);

    /* Unlink middle node (non-head, non-tail) - covers line 1102 */
    Fvid2Utils_unLinkNodePri((Fvid2Utils_Handle)&llobj, &node2);

    /* Verify node2 was removed: node1->next should be node3 */
    if (node1.next != &node3)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Unlink non-head node FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Unlink tail node (node3, whose next is NULL) - covers line 1111 */
    Fvid2Utils_unLinkNodePri((Fvid2Utils_Handle)&llobj, &node3);

    /* Unlink remaining head node */
    Fvid2Utils_unLinkNodePri((Fvid2Utils_Handle)&llobj, &node1);

    /* Cleanup */
    Fvid2Utils_unLinkAllNodes((Fvid2Utils_Handle)&llobj);
    Fvid2Utils_destructLinkList(&llobj);

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: Test dutilsUnLinkDoublePri - %s\r\n",
              (CSIRX_COV_TEST_PASS == retVal) ? "PASS" : "FAIL");

    return retVal;
}

/* Test unLinkNodeFromTail with multiple nodes in list */
static int32_t CsirxFvid2Cov_testUnLinkFromTailMultiNode(void)
{
    int32_t retVal = CSIRX_COV_TEST_PASS;
    Fvid2UtilsLinkListObj qobj;
    Fvid2Utils_QElem qelem1, qelem2, qelem3;
    uint32_t data1 = 111U, data2 = 222U, data3 = 333U;
    Fvid2Utils_Node *pNode;

    Fvid2Utils_constructQ(&qobj);

    /* Add 3 elements to queue (uses dutilsLinkDouble which tracks numElements) */
    Fvid2Utils_queue((Fvid2Utils_QHandle)&qobj, &qelem1, (void *)&data1);
    Fvid2Utils_queue((Fvid2Utils_QHandle)&qobj, &qelem2, (void *)&data2);
    Fvid2Utils_queue((Fvid2Utils_QHandle)&qobj, &qelem3, (void *)&data3);

    /* Unlink from tail with 3 nodes - covers lines 1170-1182 */
    pNode = Fvid2Utils_unLinkNodeFromTail((Fvid2Utils_Handle)&qobj);
    if ((pNode == NULL) || (pNode->data != (void *)&data3))
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " unLinkNodeFromTail 3 nodes FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Unlink from tail with 2 nodes - covers same path again */
    pNode = Fvid2Utils_unLinkNodeFromTail((Fvid2Utils_Handle)&qobj);
    if ((pNode == NULL) || (pNode->data != (void *)&data2))
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " unLinkNodeFromTail 2 nodes FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    pNode = Fvid2Utils_unLinkNodeFromTail((Fvid2Utils_Handle)&qobj);
    if ((pNode == NULL) || (pNode->data != (void *)&data1))
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " unLinkNodeFromTail last node FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Verify list is now empty (both head and tail are NULL) */
    if ((qobj.headNode != NULL) || (qobj.tailNode != NULL))
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " unLinkNodeFromTail final state FAILED - list not empty\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Cleanup */
    Fvid2Utils_unLinkAllNodes((Fvid2Utils_QHandle)&qobj);
    Fvid2Utils_destructQ(&qobj);

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: Test unLinkNodeFromTail with multiple nodes in list - %s\r\n",
              (CSIRX_COV_TEST_PASS == retVal) ? "PASS" : "FAIL");

    return retVal;
}

/* Test unLinkNodeFromHead with 2-node list */
static int32_t CsirxFvid2Cov_testUnLinkFromHeadTwoNode(void)
{
    int32_t retVal = CSIRX_COV_TEST_PASS;
    Fvid2UtilsLinkListObj qobj;
    Fvid2Utils_QElem qelem1, qelem2;
    uint32_t data1 = 101U, data2 = 102U;
    Fvid2Utils_Node *pNode;

    Fvid2Utils_constructQ(&qobj);

    /* Add 2 elements to queue */
    Fvid2Utils_queue((Fvid2Utils_QHandle)&qobj, &qelem1, (void *)&data1);
    Fvid2Utils_queue((Fvid2Utils_QHandle)&qobj, &qelem2, (void *)&data2);

    /* Unlink from head with 2 nodes - should NOT trigger line 1165 yet */
    pNode = Fvid2Utils_unLinkNodeFromHead((Fvid2Utils_Handle)&qobj);
    if ((pNode == NULL) || (pNode->data != (void *)&data1))
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " unLinkNodeFromHead first FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Verify qelem2 is now head and tail */
    if ((qobj.headNode != (Fvid2Utils_Node *)&qelem2) ||
        (qobj.tailNode != (Fvid2Utils_Node *)&qelem2))
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " unLinkNodeFromHead middle state FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    pNode = Fvid2Utils_unLinkNodeFromHead((Fvid2Utils_Handle)&qobj);
    if ((pNode == NULL) || (pNode->data != (void *)&data2))
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " unLinkNodeFromHead last node FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Verify list is now empty (both head and tail are NULL) */
    if ((qobj.headNode != NULL) || (qobj.tailNode != NULL))
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " unLinkNodeFromHead final state FAILED - list not empty\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Cleanup */
    Fvid2Utils_unLinkAllNodes((Fvid2Utils_QHandle)&qobj);
    Fvid2Utils_destructQ(&qobj);

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: Test unLinkNodeFromHead with 2-node list - %s\r\n",
              (CSIRX_COV_TEST_PASS == retVal) ? "PASS" : "FAIL");

    return retVal;
}

/* Test dutilsUnLinkDouble with specific node removal */
static int32_t CsirxFvid2Cov_testUnLinkSpecificNode(void)
{
    int32_t retVal = CSIRX_COV_TEST_PASS;
    Fvid2UtilsLinkListObj llobj;
    Fvid2Utils_Node node1, node2, node3, node4;
    uint32_t data1 = 1U, data2 = 2U, data3 = 3U, data4 = 4U;

    /* Construct double list */
    Fvid2Utils_constructLinkList(&llobj, FVID2UTILS_LLT_DOUBLE,
                                 FVID2UTILS_LAM_BOTTOM);

    node1.data = (void *)&data1;
    node2.data = (void *)&data2;
    node3.data = (void *)&data3;
    node4.data = (void *)&data4;

    /* Add 4 nodes using linkNodeToTail (uses dutilsLinkDouble, increments numElements) */
    Fvid2Utils_linkNodeToTail((Fvid2Utils_Handle)&llobj, &node1);
    Fvid2Utils_linkNodeToTail((Fvid2Utils_Handle)&llobj, &node2);
    Fvid2Utils_linkNodeToTail((Fvid2Utils_Handle)&llobj, &node3);
    Fvid2Utils_linkNodeToTail((Fvid2Utils_Handle)&llobj, &node4);

    /* Remove middle node (node2) - covers lines 1219-1223 */
    Fvid2Utils_unLinkNode((Fvid2Utils_Handle)&llobj, &node2);

    /* Verify node1->next is now node3 */
    if (node1.next != &node3)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Unlink middle node FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Remove tail node (node4) - covers lines 1214-1218 */
    Fvid2Utils_unLinkNode((Fvid2Utils_Handle)&llobj, &node4);

    /* Verify node3 is now tail */
    if (llobj.tailNode != &node3)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Unlink tail node FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Remove head node (node1) - covers lines 1209-1213 */
    Fvid2Utils_unLinkNode((Fvid2Utils_Handle)&llobj, &node1);

    /* Verify node3 is now head */
    if (llobj.headNode != &node3)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Unlink head node FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Remove only remaining node (node3) - covers lines 1203-1208 */
    Fvid2Utils_unLinkNode((Fvid2Utils_Handle)&llobj, &node3);

    /* Verify list is empty */
    if ((llobj.headNode != NULL) || (llobj.tailNode != NULL))
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " Unlink only node FAILED\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Cleanup */
    Fvid2Utils_unLinkAllNodes((Fvid2Utils_Handle)&llobj);
    Fvid2Utils_destructLinkList(&llobj);

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: Test dutilsUnLinkDouble with specific node removal - %s\r\n",
              (CSIRX_COV_TEST_PASS == retVal) ? "PASS" : "FAIL");

    return retVal;
}

static uint32_t gDummyDrvHandleVal;

/** \brief Dummy create function – returns a non-NULL Fdrv_Handle */
static Fdrv_Handle CsirxFvid2Cov_dummyCreateFxn(
    uint32_t              drvId,
    uint32_t              instanceId,
    void                 *createArgs,
    void                 *createStatusArgs,
    const Fvid2_DrvCbParams *fdmCbParams)
{
    (void)drvId;
    (void)instanceId;
    (void)createArgs;
    (void)createStatusArgs;
    (void)fdmCbParams;
    return (Fdrv_Handle)&gDummyDrvHandleVal;
}

/** \brief Dummy mock callbacks matching Fvid2_CbFxn / Fvid2_ErrCbFxn */
static int32_t CsirxFvid2Cov_dummyCbFxn(Fvid2_Handle handle, void *appData)
{
    (void)handle;
    (void)appData;
    return FVID2_SOK;
}

static int32_t CsirxFvid2Cov_dummyErrCbFxn(Fvid2_Handle handle,
                                             void        *appData,
                                             void        *errList)
{
    (void)handle;
    (void)appData;
    (void)errList;
    return FVID2_SOK;
}

/* Test FVID2 driver manager – NULL ops, unsupported ops, and
 *         callback pointer branches in Fvid2_create.
 */
static int32_t CsirxFvid2Cov_testDrvMgrNullOpsAndCallbacks(void)
{
    int32_t        retVal     = CSIRX_COV_TEST_PASS;
    int32_t        result;
    Fvid2_Handle   handle;
    Fvid2_DrvOps   drvOps1;
    Fvid2_DrvOps   drvOps2;
    Fvid2_CbParams cbPrms;
    Fvid2_FrameList frameList;
    Fvid2_DrvOps    fakeDrvOps;
    LocalFdmDriver  fakeDriver;
    LocalFdmChannel fakeChannel;

    /* Driver: createFxn set, all other ops NULL */
    Fvid2Utils_memset(&drvOps1, 0, sizeof(drvOps1));
    drvOps1.drvId     = 0xFFF0U;
    drvOps1.createFxn = &CsirxFvid2Cov_dummyCreateFxn;

    result = Fvid2_registerDriver(&drvOps1);
    if (FVID2_SOK != result)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " DrvMgr: register driver1 failed\r\n");
        return CSIRX_COV_TEST_FAIL;
    }

    /* Create with full callback set */
    Fvid2CbParams_init(&cbPrms);
    cbPrms.cbFxn       = &CsirxFvid2Cov_dummyCbFxn;
    cbPrms.errCbFxn    = &CsirxFvid2Cov_dummyErrCbFxn;
    cbPrms.traceStCbFxn  = &CsirxFvid2Cov_dummyCbFxn;
    cbPrms.traceEndCbFxn = &CsirxFvid2Cov_dummyCbFxn;

    handle = Fvid2_create(0xFFF0U, 0U, NULL, NULL, &cbPrms);
    if (NULL == handle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " DrvMgr: Fvid2_create with dummy driver failed\r\n");
        (void)Fvid2_unRegisterDriver(&drvOps1);
        return CSIRX_COV_TEST_FAIL;
    }

    Fvid2Utils_memset(&frameList, 0, sizeof(frameList));

    result = Fvid2_control(handle, 0U, NULL, NULL);
    if (FVID2_EUNSUPPORTED_OPS != result)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " DrvMgr: control should return EUNSUPPORTED_OPS\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    result = Fvid2_queue(handle, &frameList, 0U);
    if (FVID2_EUNSUPPORTED_OPS != result)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " DrvMgr: queue should return EUNSUPPORTED_OPS\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    result = Fvid2_dequeue(handle, &frameList, 0U, 0U);
    if (FVID2_EUNSUPPORTED_OPS != result)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " DrvMgr: dequeue should return EUNSUPPORTED_OPS\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    result = Fvid2_processRequest(handle, &frameList, &frameList, 0U);
    if (FVID2_EUNSUPPORTED_OPS != result)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " DrvMgr: processRequest should return EUNSUPPORTED_OPS\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    drvOps1.processRequestFxn = &testProcessRequest;
    result = Fvid2_processRequest(handle, &frameList, &frameList, 0U);
    if (FVID2_SOK != result)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " DrvMgr: processRequest should return FVID2_SOK\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    result = Fvid2_getProcessedRequest(handle, &frameList, &frameList, 0U);
    if (FVID2_EUNSUPPORTED_OPS != result)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " DrvMgr: getProcessedRequest should return "
                  "EUNSUPPORTED_OPS\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    drvOps1.getProcessedRequestFxn = &testgetProcessRequest;
    result = Fvid2_getProcessedRequest(handle, &frameList, &frameList, 0U);
    if (FVID2_SOK != result)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " DrvMgr: processRequest should return FVID2_SOK\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Cover fdmFreeChannelObject line 1526 */
        Fvid2Utils_memset(&fakeDrvOps,  0, sizeof(fakeDrvOps));
    Fvid2Utils_memset(&fakeDriver,  0, sizeof(fakeDriver));
    Fvid2Utils_memset(&fakeChannel, 0, sizeof(fakeChannel));

    /* Non-NULL drv and drvOps so OSAL_Assert checks pass in Fvid2_delete.
        * deleteFxn is NULL so the "not supported" else-branch is taken
        * (which is fine; we only care about reaching fdmFreeChannelObject). */
    fakeDriver.drvOps   = &fakeDrvOps;
    fakeDriver.numOpens = 1U;

    fakeChannel.drv    = &fakeDriver;
    fakeChannel.isUsed = (uint32_t)FALSE; /* triggers else at line 1526 */

    (void)Fvid2_delete((Fvid2_Handle)&fakeChannel, NULL);

    result = Fvid2_delete(handle, NULL);

    (void)Fvid2_unRegisterDriver(&drvOps1);

    Fvid2Utils_memset(&drvOps2, 0, sizeof(drvOps2));
    drvOps2.drvId = 0xFFF1U;

    result = Fvid2_registerDriver(&drvOps2);
    if (FVID2_SOK != result)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " DrvMgr: register driver2 failed\r\n");
        return CSIRX_COV_TEST_FAIL;
    }

    handle = Fvid2_create(0xFFF1U, 0U, NULL, NULL, NULL);
    if (NULL != handle)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " DrvMgr: create with NULL createFxn should return NULL\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    (void)Fvid2_unRegisterDriver(&drvOps2);

    GT_1trace(gAppTrace, GT_INFO,
              " CSIRX_COV: Test FVID2 driver manager - %s\r\n",
              (CSIRX_COV_TEST_PASS == retVal) ? "PASS" : "FAIL");

    return retVal;
}

/** \brief Maximum dummy drivers for slot exhaustion test */
#define CSIRX_COV_MAX_DUMMY_DRIVERS     (FVID2_CFG_FDM_NUM_DRV_OBJS)

/** \brief Array of dummy driver ops for slot exhaustion */
static Fvid2_DrvOps gDummyDrvOpsArray[CSIRX_COV_MAX_DUMMY_DRIVERS];

/* Test FVID2 driver manager registration edge cases */
static int32_t CsirxFvid2Cov_testDrvMgrRegisterEdgeCases(void)
{
    int32_t      retVal          = CSIRX_COV_TEST_PASS;
    int32_t      result;
    uint32_t     i;
    uint32_t     registeredCount = 0U;
    Fvid2_DrvOps dupOps;
    Fvid2_DrvOps nonRegOps;
    Fvid2_Handle handle;

    /* Duplicate drvId registration */
    Fvid2Utils_memset(&dupOps, 0, sizeof(dupOps));
    dupOps.drvId     = 0xFFE0U;
    dupOps.createFxn = &CsirxFvid2Cov_dummyCreateFxn;

    result = Fvid2_registerDriver(&dupOps);
    if (FVID2_SOK != result)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " DrvMgr: first registration failed\r\n");
        return CSIRX_COV_TEST_FAIL;
    }

    result = Fvid2_registerDriver(&dupOps);
    if (FVID2_EDRIVER_INUSE != result)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " DrvMgr: duplicate registration should fail\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* --- 2. Unregister while in use → lines 1468-1471 --- */
    handle = Fvid2_create(0xFFE0U, 0U, NULL, NULL, NULL);
    if (NULL != handle)
    {
        result = Fvid2_unRegisterDriver(&dupOps);
        if (FVID2_EDEVICE_INUSE != result)
        {
            GT_0trace(gAppTrace, GT_ERR,
                      " DrvMgr: unregister-in-use should return "
                      "EDEVICE_INUSE\r\n");
            retVal = CSIRX_COV_TEST_FAIL;
        }

        /* Delete to release numOpens */
        (void)Fvid2_delete(handle, NULL);
    }

    (void)Fvid2_unRegisterDriver(&dupOps);

    /* --- 3. Unregister non-registered driver → line 1478 --- */
    Fvid2Utils_memset(&nonRegOps, 0, sizeof(nonRegOps));
    nonRegOps.drvId = 0xFFDFU;

    result = Fvid2_unRegisterDriver(&nonRegOps);
    if (FVID2_EFAIL != result)
    {
        GT_0trace(gAppTrace, GT_ERR,
                  " DrvMgr: unregister non-registered should return EFAIL\r\n");
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* --- 4. Exhaust driver object pool → lines 632-634 --- */
    for (i = 0U; i < CSIRX_COV_MAX_DUMMY_DRIVERS; i++)
    {
        Fvid2Utils_memset(&gDummyDrvOpsArray[i], 0, sizeof(Fvid2_DrvOps));
        gDummyDrvOpsArray[i].drvId = 0xFF00U + i;

        result = Fvid2_registerDriver(&gDummyDrvOpsArray[i]);
        if (FVID2_SOK == result)
        {
            registeredCount++;
        }
        else
        {
            /* Either FVID2_EALLOC or other error –
               both confirm the pool is full. */
            break;
        }
    }

    /* If all slots registered without error, try one more to trigger 632 */
    if (i == CSIRX_COV_MAX_DUMMY_DRIVERS)
    {
        Fvid2_DrvOps extraOps;
        Fvid2Utils_memset(&extraOps, 0, sizeof(extraOps));
        extraOps.drvId = 0xFF00U + CSIRX_COV_MAX_DUMMY_DRIVERS;

        result = Fvid2_registerDriver(&extraOps);
        if (FVID2_EALLOC != result)
        {
            GT_0trace(gAppTrace, GT_ERR,
                      " DrvMgr: pool exhaustion should return EALLOC\r\n");
            retVal = CSIRX_COV_TEST_FAIL;
        }
    }

    /* Cleanup: unregister all dummy drivers */
    for (i = 0U; i < registeredCount; i++)
    {
        (void)Fvid2_unRegisterDriver(&gDummyDrvOpsArray[i]);
    }

    GT_1trace(gAppTrace, GT_INFO,
              " FVID2: Test Frame list utilities test - %s\r\n",
              (CSIRX_COV_TEST_PASS == retVal) ? "PASS" : "FAIL");

    return retVal;
}

/*  Test: CsirxFvid2Cov_testUdmaErrorMap */
static int32_t CsirxFvid2Cov_testUdmaErrorMap(void)
{
    int32_t retVal = CSIRX_COV_TEST_PASS;
    int32_t result;

    /* Maps UDMA_SOK to FVID2_SOK */
    result = CsirxDrv_udmaToFvid2ErrorMap(UDMA_SOK);
    if (result != FVID2_SOK)
    {
        GT_2trace(gAppTrace, GT_INFO,
              " FVID2: ErrorMap - UDMA_SOK result=%d (expect %d)\r\n",
              result, FVID2_SOK);
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Maps UDMA_EFAIL to FVID2_EFAIL */
    result = CsirxDrv_udmaToFvid2ErrorMap(UDMA_EFAIL);
    if (result != FVID2_EFAIL)
    {
        GT_2trace(gAppTrace, GT_INFO,
              " FVID2: ErrorMap - UDMA_EFAIL result=%d (expect %d)\r\n",
              result, FVID2_EFAIL);
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Maps UDMA_EBADARGS to FVID2_EBADARGS */
    result = CsirxDrv_udmaToFvid2ErrorMap(UDMA_EBADARGS);
    if (result != FVID2_EBADARGS)
    {
        GT_2trace(gAppTrace, GT_INFO,
              " FVID2: ErrorMap - UDMA_EBADARGS result=%d (expect %d)\r\n",
              result, FVID2_EBADARGS);
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Maps UDMA_EINVALID_PARAMS to FVID2_EINVALID_PARAMS */
    result = CsirxDrv_udmaToFvid2ErrorMap(UDMA_EINVALID_PARAMS);
    if (result != FVID2_EINVALID_PARAMS)
    {
        GT_2trace(gAppTrace, GT_INFO,
              " FVID2: ErrorMap - UDMA_EINVALID_PARAMS result=%d (expect %d)\r\n",
              result, FVID2_EINVALID_PARAMS);
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Maps UDMA_ETIMEOUT to FVID2_ETIMEOUT */
    result = CsirxDrv_udmaToFvid2ErrorMap(UDMA_ETIMEOUT);
    if (result != FVID2_ETIMEOUT)
    {
        GT_2trace(gAppTrace, GT_INFO,
              " FVID2: ErrorMap - UDMA_ETIMEOUT result=%d (expect %d)\r\n",
              result, FVID2_ETIMEOUT);
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Maps UDMA_EALLOC to FVID2_EALLOC */
    result = CsirxDrv_udmaToFvid2ErrorMap(UDMA_EALLOC);
    if (result != FVID2_EALLOC)
    {
        GT_2trace(gAppTrace, GT_INFO,
              " FVID2: ErrorMap - UDMA_EALLOC result=%d (expect %d)\r\n",
              result, FVID2_EALLOC);
        retVal = CSIRX_COV_TEST_FAIL;
    }

    /* Tests the false condition - any invalid UDMA error code maps to FVID2_EFAIL */
    result = CsirxDrv_udmaToFvid2ErrorMap(0xDEADBEEF);  /* Invalid error code */
    if (result != FVID2_EFAIL)
    {
        GT_2trace(gAppTrace, GT_INFO,
              " FVID2: ErrorMap - Invalid code result=%d (expect %d)\r\n",
              result, FVID2_EFAIL);
        retVal = CSIRX_COV_TEST_FAIL;
    }

    GT_1trace(gAppTrace, GT_INFO,
              " FVID2: UDMA Error mapping test - %s\r\n",
              (CSIRX_COV_TEST_PASS == retVal) ? "PASS" : "FAIL");

    return retVal;
}

int32_t CsirxFvid2_runAllTests(CsirxTestTaskObj *taskObj)
{
    int32_t testResult = CSIRX_COV_TEST_PASS;
    uint32_t passCount = 0U;
    uint32_t failCount = 0U;
    uint32_t skipCount = 0U;
    uint32_t totalTests = 20U;

    GT_0trace(gAppTrace, GT_INFO,
              "\r\n ===============================================\r\n");
    GT_0trace(gAppTrace, GT_INFO,
              " FVID2 Utils Coverage Improvement Test Suite\r\n");
    GT_0trace(gAppTrace, GT_INFO,
              " ===============================================\r\n\r\n");

    GT_1trace(gAppTrace, GT_INFO,
              "\r\n [Test 1/%d] NULL Pointer Arguments\r\n",totalTests);
    if (CSIRX_COV_TEST_PASS == CsirxCov_testNullAndNegArgs(taskObj))
    {
        passCount++;
    }
    else
    {
        failCount++;
    }

    GT_1trace(gAppTrace, GT_INFO,
                "\r\n [Test 2/%d] Dequeue Edge Case\r\n", totalTests);
    if (CSIRX_COV_TEST_PASS == CsirxCov_testDequeueEdgeCase(taskObj))
    {
        passCount++;
    }
    else
    {
        failCount++;
    }

    GT_1trace(gAppTrace, GT_INFO,
              "\r\n [Test 3/%d] FVID2 Utils functions\r\n", totalTests);
    if (CsirxFvid2Cov_testUtils() == CSIRX_COV_TEST_PASS)
    {
        passCount++;
    }
    else
    {
        failCount++;
    }

    GT_1trace(gAppTrace, GT_INFO,
              "\r\n [Test 4/%d] Test Operations on Double Linked List\r\n", totalTests);
    if (CsirxFvid2Cov_testDoubleLinkList() == CSIRX_COV_TEST_PASS)
    {
        passCount++;
    }
    else
    {
        failCount++;
    }

    GT_1trace(gAppTrace, GT_INFO,
              "\r\n [Test 5/%d] Test Operations on Priority Linked List\r\n", totalTests);
    if (CsirxFvid2Cov_testPriorityLinkList() == CSIRX_COV_TEST_PASS)
    {
        passCount++;
    }
    else
    {
        failCount++;
    }

    GT_1trace(gAppTrace, GT_INFO,
              "\r\n [Test 6/%d] Test Operations on Unique Priority Linked List\r\n", totalTests);
    if (CsirxFvid2Cov_testUniqueAndUnlinkAll() == CSIRX_COV_TEST_PASS)
    {
        passCount++;
    }
    else
    {
        failCount++;
    }

    GT_1trace(gAppTrace, GT_INFO,
              "\r\n [Test 7/%d] Test Queue Operations\r\n", totalTests);
    if (CsirxFvid2Cov_testQueueOps() == CSIRX_COV_TEST_PASS)
    {
        passCount++;
    }
    else
    {
        failCount++;
    }

    GT_1trace(gAppTrace, GT_INFO,
              "\r\n [Test 8/%d] Test QueueBack Operations\r\n", totalTests);
    if (CsirxFvid2Cov_testQueueBack() == CSIRX_COV_TEST_PASS)
    {
        passCount++;
    }
    else
    {
        failCount++;
    }

    GT_1trace(gAppTrace, GT_INFO,
              "\r\n [Test 9/%d] Test Circular List Operations\r\n", totalTests);
    if (CsirxFvid2Cov_testCircularLinkList() == CSIRX_COV_TEST_PASS)
    {
        passCount++;
    }
    else
    {
        failCount++;
    }

    GT_1trace(gAppTrace, GT_INFO,
              "\r\n [Test 10/%d] Test Frame list utilities\r\n", totalTests);
    if (CsirxFvid2Cov_testFrameListFuncs() == CSIRX_COV_TEST_PASS)
    {
        passCount++;
    }
    else
    {
        failCount++;
    }

    GT_1trace(gAppTrace, GT_INFO,
              "\r\n [Test 11/%d] Test GTTrace with non-INFO classes\r\n", totalTests);
    if (CsirxFvid2Cov_testGTTraceFvid2() == CSIRX_COV_TEST_PASS)
    {
        passCount++;
    }
    else
    {
        failCount++;
    }

    GT_1trace(gAppTrace, GT_INFO,
              "\r\n [Test 12/%d] Test fdmDriver callback mechanism\r\n", totalTests);
    if (CsirxFvid2Cov_testFdmDriverCbFxns(taskObj) == CSIRX_COV_TEST_PASS)
    {
        passCount++;
    }
    else
    {
        failCount++;
    }

    GT_1trace(gAppTrace, GT_INFO,
              "\r\n [Test 13/%d] Test linkUniqePriNode\r\n", totalTests);
    if (CsirxFvid2Cov_testLinkUniqePri() == CSIRX_COV_TEST_PASS)
    {
        passCount++;
    }
    else
    {
        failCount++;
    }

    GT_1trace(gAppTrace, GT_INFO,
              "\r\n [Test 14/%d] Test dutilsUnLinkDoublePri\r\n", totalTests);
    if (CsirxFvid2Cov_testUnLinkDoublePriNonHead() == CSIRX_COV_TEST_PASS)
    {
        passCount++;
    }
    else
    {
        failCount++;
    }

    GT_1trace(gAppTrace, GT_INFO,
              "\r\n [Test 15/%d] Test unLinkNodeFromTail with multiple nodes in list\r\n", totalTests);
    if (CsirxFvid2Cov_testUnLinkFromTailMultiNode() == CSIRX_COV_TEST_PASS)
    {
        passCount++;
    }
    else
    {
        failCount++;
    }

    GT_1trace(gAppTrace, GT_INFO,
              "\r\n [Test 16/%d] Test unLinkNodeFromHead with 2-node list\r\n", totalTests);
    if (CsirxFvid2Cov_testUnLinkFromHeadTwoNode() == CSIRX_COV_TEST_PASS)
    {
        passCount++;
    }
    else
    {
        failCount++;
    }

    GT_1trace(gAppTrace, GT_INFO,
              "\r\n [Test 17/%d] Test dutilsUnLinkDouble with specific node removal\r\n", totalTests);
    if (CsirxFvid2Cov_testUnLinkSpecificNode() == CSIRX_COV_TEST_PASS)
    {
        passCount++;
    }
    else
    {
        failCount++;
    }

    GT_1trace(gAppTrace, GT_INFO,
              "\r\n [Test 18/%d] Test FVID2 driver manager\r\n", totalTests);
    if (CsirxFvid2Cov_testDrvMgrNullOpsAndCallbacks() == CSIRX_COV_TEST_PASS)
    {
        passCount++;
    }
    else
    {
        failCount++;
    }

    GT_1trace(gAppTrace, GT_INFO,
              "\r\n [Test 19/%d] Test Frame list utilities\r\n", totalTests);
    if (CsirxFvid2Cov_testDrvMgrRegisterEdgeCases() == CSIRX_COV_TEST_PASS)
    {
        passCount++;
    }
    else
    {
        failCount++;
    }

    GT_1trace(gAppTrace, GT_INFO,
              "\r\n [Test 20/%d] UDMA Error mapping test\r\n", totalTests);
    if (CsirxFvid2Cov_testUdmaErrorMap() == CSIRX_COV_TEST_PASS)
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
              " CSIRX Coverage Test Suite - SUMMARY\r\n");
    GT_0trace(gAppTrace, GT_INFO,
              " ===============================================\r\n");
    GT_2trace(gAppTrace, GT_INFO,
              "  Total : %d\r\n  Passed: %d\r\n",
              totalTests, passCount);
    GT_1trace(gAppTrace, GT_INFO,
              "  Failed: %d\r\n", failCount);
    GT_1trace(gAppTrace, GT_INFO,
              "  Skipped: %d\r\n", skipCount);
    GT_0trace(gAppTrace, GT_INFO,
              " ===============================================\r\n\r\n");

    if (failCount > 0U)
    {
        testResult = CSIRX_COV_TEST_FAIL;
    }

    return testResult;
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
