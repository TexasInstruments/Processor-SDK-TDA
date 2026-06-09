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
 *  \file main_baremetal.c
 *
 *  \brief DRU5 error handling and unsupported feature validation test.
 *
 *  This application validates DRU5 hardware error reporting by
 *  intentionally submitting Transfer Requests (TRs) with unsupported
 *  or invalid configurations and verifying the hardware response.
 *
 *  The test flow performed in main() is:
 *  1. Read DRU capabilities to determine supported TRs types and features
 *     (STATIC, EOL, AMODE, etc.).
 *  2. Configure  and  submit TR's with intentionally unsupported settings.
 *  3. Wait for TR completion and read the TR response written by hardware.
 *  4. Verify that returned STATUS_TYPE and STATUS_INFO match the expected
 *     hardware error response:
 *        - STATUS_TYPE Submission Error or Unsupported Feature
 *        - Correct STATUS_FIELD indicating the specific unsupported reason
 *
 *  This test checks if DRU correctly detects invalid TR configurations and
 *  reports precise error status information to software.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdio.h>
#include <stdint.h>
#include <ti/drv/udma/udma.h>
#include <ti/drv/uart/UART.h>
#include <ti/board/board.h>
#include <ti/drv/uart/UART_stdio.h>
#include <ti/drv/udma/examples/udma_apputils/udma_apputils.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/**< Number of bytes used for TR buffer allocation */
#define UDMA_TEST_APP_NUM_BYTES         (1000U)
/**< This ensures every channel memory is aligned */
#define UDMA_TEST_APP_NUM_BYTES_ALIGN   ((UDMA_TEST_APP_NUM_BYTES + UDMA_CACHELINE_ALIGNMENT) & ~(UDMA_CACHELINE_ALIGNMENT - 1U))

/**< Ring parameters */
/**< Number of ring entries */
#define UDMA_TEST_APP_RING_ENTRIES      (1U)
/**< Size (in bytes) of each ring entry (Size of pointer - 64-bit) */
#define UDMA_TEST_APP_RING_ENTRY_SIZE   (sizeof(uint64_t))
/**< Total ring memory */
#define UDMA_TEST_APP_RING_MEM_SIZE     (UDMA_TEST_APP_RING_ENTRIES * UDMA_TEST_APP_RING_ENTRY_SIZE)
/**< This ensures every channel memory is aligned */
#define UDMA_TEST_APP_RING_MEM_SIZE_ALIGN ((UDMA_TEST_APP_RING_MEM_SIZE + UDMA_CACHELINE_ALIGNMENT) & ~(UDMA_CACHELINE_ALIGNMENT - 1U))
/**
 *  \brief UDMA TR packet descriptor memory.
 *  This contains CSL_UdmapCppi5TRPD + Padding to sizeof(CSL_UdmapTR15) +
 *  one  Type_15 TR (CSL_UdmapTR15) + one  TR response of 4 bytes.  Since
 *  CSL_UdmapCppi5TRPD  is less than CSL_UdmapTR15,size is just two times
 *  CSL_UdmapTR15 for alignment.
 */
#define UDMA_TEST_APP_TRPD_SIZE         ((sizeof(CSL_UdmapTR15) * 2U) + 4U)
/**< This ensures every channel memory is aligned */
#define UDMA_TEST_APP_TRPD_SIZE_ALIGN   ((UDMA_TEST_APP_TRPD_SIZE + UDMA_CACHELINE_ALIGNMENT) & ~(UDMA_CACHELINE_ALIGNMENT - 1U))

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static int32_t App_unsupportedTrTest(Udma_ChHandle chHandle);
static int32_t App_submitUnsupportedTr(Udma_ChHandle chHandle, void *destBuf, void *srcBuf, uint32_t length);
static void App_udmaEventCb(Udma_EventHandle eventHandle, uint32_t eventType, void *appData);
static void App_print(const char *str);
static const char *App_trStatusToStr(uint32_t status);
static const char *App_trUnsupportedReasonToStr(uint32_t reason);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/**< UDMA driver objects */
static struct Udma_ChObj       gUdmaChObj;
static struct Udma_DrvObj      gUdmaDrvObj;
static struct Udma_EventObj    gUdmaCqEventObj;
static struct Udma_EventObj    gUdmaTdCqEventObj;

/**< UDMA Memories */
static uint8_t gDruRingMem[UDMA_TEST_APP_RING_MEM_SIZE_ALIGN] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
static uint8_t gDruCompRingMem[UDMA_TEST_APP_RING_MEM_SIZE_ALIGN] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
static uint8_t gDruTdCompRingMem[UDMA_TEST_APP_RING_MEM_SIZE_ALIGN] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
static uint8_t gUdmaTrpdMem[UDMA_TEST_APP_TRPD_SIZE_ALIGN] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));

/**< Application Buffers */
static uint8_t gUdmaTestSrcBuf[UDMA_TEST_APP_NUM_BYTES_ALIGN] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
static uint8_t gUdmaTestDestBuf[UDMA_TEST_APP_NUM_BYTES_ALIGN] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));

/**< Semaphore to indicate transfer completion */
static SemaphoreP_Handle gUdmaAppDoneSem = NULL;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t main(void)
{
    int32_t             retVal = UDMA_SOK;
    int32_t             tempRetVal;
    Udma_DrvHandle      drvHandle = &gUdmaDrvObj;
    Udma_ChHandle       chHandle = &gUdmaChObj;
    Board_initCfg       boardCfg;
    uint32_t            instId;
    Udma_InitPrms       initPrms;
    uint32_t            utcId;
    uint32_t            numQueue, queId;
    CSL_DruQueueConfig  queueCfg;
    uint32_t            chType;
    Udma_ChPrms         chPrms;
    Udma_ChUtcPrms      utcPrms;
    Udma_EventHandle    eventHandle;
    Udma_EventPrms      eventPrms;
    SemaphoreP_Params   semPrms;
    uint64_t            pDesc;

    uint32_t drvInitDone      = 0U;
    uint32_t chOpenDone       = 0U;
    uint32_t cqEventRegDone   = 0U;
    uint32_t tdEventRegDone   = 0U;
    uint32_t chEnableDone     = 0U;

    boardCfg = BOARD_INIT_MODULE_CLOCK  |
               BOARD_INIT_PINMUX_CONFIG |
               BOARD_INIT_UART_STDIO;

    Board_init(boardCfg);

    App_print("UTC Unsupported TR Check application started...\n");

    instId = UDMA_INST_ID_MAIN_0;
    UdmaInitPrms_init(instId, &initPrms);
    initPrms.printFxn = &App_print;

    retVal = Udma_init(drvHandle, &initPrms);
    if (UDMA_SOK != retVal)
    {
        App_print("[Error] UDMA init failed!!\n");
        goto cleanup;
    }
    drvInitDone = 1U;

    utcId = UDMA_UTC_ID_MSMC_DRU0;
    numQueue = Udma_druGetNumQueue(drvHandle, utcId);
    if (0U == numQueue)
    {
        App_print("[Error] Invalid queue number!!\n");
        retVal = UDMA_EFAIL;
        goto cleanup;
    }

    UdmaDruQueueConfig_init(&queueCfg);
    for (queId = CSL_DRU_QUEUE_ID_0; queId < numQueue; queId++)
    {
        retVal = Udma_druQueueConfig(drvHandle, utcId, queId, &queueCfg);
        if (UDMA_SOK != retVal)
        {
            App_print("[Error] DRU queue config failed!!\n");
            goto cleanup;
        }
    }

    SemaphoreP_Params_init(&semPrms);
    gUdmaAppDoneSem = SemaphoreP_create(0, &semPrms);
    if (NULL == gUdmaAppDoneSem)
    {
        App_print("[Error] Semaphore creation failed!!\n");
        retVal = UDMA_EFAIL;
        goto cleanup;
    }

    chType = UDMA_CH_TYPE_UTC;
    UdmaChPrms_init(&chPrms, chType);

    chPrms.utcId                = UDMA_UTC_ID_MSMC_DRU0;
    chPrms.fqRingPrms.ringMem   = &gDruRingMem[0U];
    chPrms.cqRingPrms.ringMem   = &gDruCompRingMem[0U];
    chPrms.tdCqRingPrms.ringMem = &gDruTdCompRingMem[0U];

    chPrms.fqRingPrms.ringMemSize   = UDMA_TEST_APP_RING_MEM_SIZE;
    chPrms.cqRingPrms.ringMemSize   = UDMA_TEST_APP_RING_MEM_SIZE;
    chPrms.tdCqRingPrms.ringMemSize = UDMA_TEST_APP_RING_MEM_SIZE;

    chPrms.fqRingPrms.elemCnt   = UDMA_TEST_APP_RING_ENTRIES;
    chPrms.cqRingPrms.elemCnt   = UDMA_TEST_APP_RING_ENTRIES;
    chPrms.tdCqRingPrms.elemCnt = UDMA_TEST_APP_RING_ENTRIES;

    retVal = Udma_chOpen(drvHandle, chHandle, chType, &chPrms);
    if (UDMA_SOK != retVal)
    {
        App_print("[Error] UDMA channel open failed!!\n");
        goto cleanup;
    }
    chOpenDone = 1U;

    UdmaChUtcPrms_init(&utcPrms);
    utcPrms.druOwner   = CSL_DRU_OWNER_UDMAC_TR;
    utcPrms.druQueueId = CSL_DRU_QUEUE_ID_3;

    retVal = Udma_chConfigUtc(chHandle, &utcPrms);
    if (UDMA_SOK != retVal)
    {
        App_print("[Error] UDMA UTC channel config failed!!\n");
        goto cleanup;
    }

    eventHandle = &gUdmaCqEventObj;
    UdmaEventPrms_init(&eventPrms);
    eventPrms.eventType         = UDMA_EVENT_TYPE_DMA_COMPLETION;
    eventPrms.eventMode         = UDMA_EVENT_MODE_SHARED;
    eventPrms.chHandle          = chHandle;
    eventPrms.masterEventHandle = Udma_eventGetGlobalHandle(drvHandle);
    eventPrms.eventCb           = &App_udmaEventCb;

    retVal = Udma_eventRegister(drvHandle, eventHandle, &eventPrms);
    if (UDMA_SOK != retVal)
    {
        App_print("[Error] UDMA CQ event register failed!!\n");
        goto cleanup;
    }
    cqEventRegDone = 1U;

    eventHandle = &gUdmaTdCqEventObj;
    UdmaEventPrms_init(&eventPrms);
    eventPrms.eventType         = UDMA_EVENT_TYPE_TEARDOWN_PACKET;
    eventPrms.eventMode         = UDMA_EVENT_MODE_SHARED;
    eventPrms.chHandle          = chHandle;
    eventPrms.masterEventHandle = Udma_eventGetGlobalHandle(drvHandle);
    eventPrms.eventCb           = &App_udmaEventCb;

    retVal = Udma_eventRegister(drvHandle, eventHandle, &eventPrms);
    if (UDMA_SOK != retVal)
    {
        App_print("[Error] UDMA Teardown CQ event register failed!!\n");
        goto cleanup;
    }
    tdEventRegDone = 1U;

    retVal = Udma_chEnable(chHandle);
    if (UDMA_SOK != retVal)
    {
        App_print("[Error] UDMA channel enable failed!!\n");
        goto cleanup;
    }
    chEnableDone = 1U;

    retVal = App_unsupportedTrTest(chHandle);
    if (UDMA_SOK != retVal)
    {
        App_print("[Error] Unsupported TR test failed!!\n");
        goto cleanup;
    }

cleanup:
    if (chEnableDone == 1U)
    {
        tempRetVal = Udma_chDisable(chHandle, UDMA_DEFAULT_CH_DISABLE_TIMEOUT);
        if ((UDMA_SOK != tempRetVal) && (UDMA_SOK == retVal))
        {
            App_print("[Error] UDMA channel disable failed!!\n");
            retVal = tempRetVal;
        }
    }

    if (chOpenDone == 1U)
    {
        while (1)
        {
            tempRetVal = Udma_ringFlushRaw(
                             Udma_chGetFqRingHandle(chHandle), &pDesc);
            if (UDMA_ETIMEOUT == tempRetVal)
            {
                break;
            }
        }
    }

    if (tdEventRegDone == 1U)
    {
        tempRetVal = Udma_eventUnRegister(&gUdmaTdCqEventObj);
        if ((UDMA_SOK != tempRetVal) && (UDMA_SOK == retVal))
        {
            App_print("[Error] UDMA teardown event unregister failed!!\n");
            retVal = tempRetVal;
        }
    }

    if (cqEventRegDone == 1U)
    {
        tempRetVal = Udma_eventUnRegister(&gUdmaCqEventObj);
        if ((UDMA_SOK != tempRetVal) && (UDMA_SOK == retVal))
        {
            App_print("[Error] UDMA CQ event unregister failed!!\n");
            retVal = tempRetVal;
        }
    }

    if (chOpenDone == 1U)
    {
        tempRetVal = Udma_chClose(chHandle);
        if ((UDMA_SOK != tempRetVal) && (UDMA_SOK == retVal))
        {
            App_print("[Error] UDMA channel close failed!!\n");
            retVal = tempRetVal;
        }
    }

    if (gUdmaAppDoneSem != NULL)
    {
        SemaphoreP_delete(gUdmaAppDoneSem);
        gUdmaAppDoneSem = NULL;
    }

    if (drvInitDone == 1U)
    {
        tempRetVal = Udma_deinit(drvHandle);
        if ((UDMA_SOK != tempRetVal) && (UDMA_SOK == retVal))
        {
            App_print("[Error] UDMA deinit failed!!\n");
            retVal = tempRetVal;
        }
    }

    App_print("UTC Unsupported TR Check application done\n");

    if (UDMA_SOK == retVal)
    {
        App_print("All tests have passed!!\n");
        return 0;
    }

    App_print("Test failed!!\n");
    return -1;
}

static int32_t App_unsupportedTrTest(Udma_ChHandle chHandle)
{
    int32_t             retVal = UDMA_SOK;
    uint32_t            i;
    uint8_t            *srcBuf = &gUdmaTestSrcBuf[0U];
    uint8_t            *destBuf = &gUdmaTestDestBuf[0U];

    /* Init buffers */
    for (i = 0U; i < UDMA_TEST_APP_NUM_BYTES; i++)
    {
        srcBuf[i] = i;
        destBuf[i] = 0U;
    }
    /* Writeback source and destination buffer */
    Udma_appUtilsCacheWb(&gUdmaTestSrcBuf[0U], UDMA_TEST_APP_NUM_BYTES);
    Udma_appUtilsCacheWb(&gUdmaTestDestBuf[0U], UDMA_TEST_APP_NUM_BYTES);

    /* Submit unsupported TR and verify hardware response */
    retVal = App_submitUnsupportedTr(
                    chHandle,
                    destBuf,
                    srcBuf,
                    UDMA_TEST_APP_NUM_BYTES);

    return (retVal);
}

static void CSL_druPrintCapabilities(const CSL_DruCapabilities *caps)
{
    uint32_t i;

    if (caps == NULL)
    {
        UART_printf("DRU capabilities: NULL pointer\n");
        return;
    }

    UART_printf("==== DRU CAPABILITIES ====\n");

    /* TR Types */
    UART_printf("Supported TR Types: ");
    for (i = 0; i < 16; i++)
    {
        if (caps->trTypeSupported & (1U << i))
        {
            UART_printf("%u ", i);
        }
    }
    UART_printf("\n");

    /* Optional features */
    UART_printf("STATIC support       : %s\n",
           caps->staticSupported ? "YES" : "NO");

    UART_printf("EOL support          : %s\n",
           caps->eolSupported ? "YES" : "NO");

    UART_printf("Local trigger        : %s\n",
           caps->localTriggerSupported ? "YES" : "NO");

    UART_printf("Global trigger       : %s\n",
           caps->globalTriggerSupported ? "YES" : "NO");

    /* AMODE */
    UART_printf("Supported AMODEs     : ");
    if (caps->amode == 0U)
    {
        UART_printf("NONE");
    }
    else
    {
        for (i = 0; i < 8; i++)
        {
            if (caps->amode & (1U << i))
            {
                UART_printf("%u ", i);
            }
        }
    }
    UART_printf("\n");

    /* ELTYPE */
    UART_printf("Supported ELTYPEs    : ");
    if (caps->elType == 0U)
    {
        UART_printf("NONE");
    }
    else
    {
        for (i = 0; i < 8; i++)
        {
            if (caps->elType & (1U << i))
            {
                UART_printf("%u ", i);
            }
        }
    }
    UART_printf("\n");

    /* DFMT */
    UART_printf("Supported DFMTs      : ");
    if (caps->dfmt == 0U)
    {
        UART_printf("NONE");
    }
    else
    {
        for (i = 0; i < 8; i++)
        {
            if (caps->dfmt & (1U << i))
            {
                UART_printf("%u ", i);
            }
        }
    }
    UART_printf("\n");

    /* SECTR */
    UART_printf("Secure transfer      : %s\n",
           caps->sectr ? "YES" : "NO");

    UART_printf("==========================\n");
}

static void App_printDruCapabilities(Udma_ChHandle chHandle, uint32_t utcId)
{
    if (chHandle == NULL)
    {
        UART_printf("chHandle is NULL\n");
        return;
    }

    if (chHandle->utcInfo == NULL)
    {
        UART_printf("Channel has no UTC info (not a UTC channel)\n");
        return;
    }

    if (chHandle->utcInfo[utcId].utcType != UDMA_UTC_TYPE_DRU)
    {
        UART_printf("UTC is not DRU-based\n");
        return;
    }

    if (chHandle->utcInfo[utcId].druRegs == NULL)
    {
        UART_printf("DRU registers not present\n");
        return;
    }

    CSL_DruCapabilities caps;
    if (CSL_druGetCapabilities(chHandle->utcInfo[utcId].druRegs, &caps) != CSL_PASS)
    {
        UART_printf("CSL_druGetCapabilities failed\n");
        return;
    }

    UART_printf("DRU Capabilities:\n");
    CSL_druPrintCapabilities(&caps);
}

static int32_t App_submitUnsupportedTr(Udma_ChHandle chHandle, void *destBuf, void *srcBuf, uint32_t length)
{
    int32_t     retVal = UDMA_SOK;
    uint32_t   *pTrResp, trRespStatus;
    uint64_t    pDesc = 0;
    uint8_t    *trpdMem = &gUdmaTrpdMem[0U];

    /* Update TR packet descriptor */
    CSL_UdmapCppi5TRPD *pTrpd = (CSL_UdmapCppi5TRPD *) trpdMem;
    CSL_UdmapTR15 *pTr = (CSL_UdmapTR15 *)UdmaUtils_getTrpdTr1Pointer(trpdMem, 0U);
    pTrResp = (uint32_t *) (trpdMem + (sizeof(CSL_UdmapTR15) * 2U));
    uint32_t cqRingNum = Udma_chGetCqRingNum(chHandle);

    /* Make TRPD */
    UdmaUtils_makeTrpd(pTrpd, UDMA_TR_TYPE_9, 1U, cqRingNum);

    /* Setup TR
     *
     * Intentionally program an unsupported TR type (3D Data Move).
     * DRU5 does not advertise support for CSL_UDMAP_TR_FLAGS_TYPE_3D_DATA_MOVE
     * in its capabilities register. Submitting this TR is expected to be
     * rejected by hardware and result in:
     *   STATUS_TYPE  = CSL_UDMAP_TR_RESPONSE_STATUS_UNSUPPORTED_ERR
     *   STATUS_INFO = CSL_UDMAP_TR_RESPONSE_STATUS_UNSUPPORTED_TR_TYPE
     *
     * This is used to validate DRU hardware error detection and reporting.
     */
    pTr->flags    = CSL_FMK(UDMAP_TR_FLAGS_TYPE, CSL_UDMAP_TR_FLAGS_TYPE_3D_DATA_MOVE) | // CSL_UDMAP_TR_FLAGS_TYPE_3D_DATA_MOVE)         |
                    CSL_FMK(UDMAP_TR_FLAGS_STATIC, 0U)                                          |
                    CSL_FMK(UDMAP_TR_FLAGS_EOL, 0U)                                             |   /* NA */
                    CSL_FMK(UDMAP_TR_FLAGS_EVENT_SIZE, CSL_UDMAP_TR_FLAGS_EVENT_SIZE_COMPLETION)|
                    CSL_FMK(UDMAP_TR_FLAGS_TRIGGER0, CSL_UDMAP_TR_FLAGS_TRIGGER_NONE)           |
                    CSL_FMK(UDMAP_TR_FLAGS_TRIGGER0_TYPE, CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ALL)  |
                    CSL_FMK(UDMAP_TR_FLAGS_TRIGGER1, CSL_UDMAP_TR_FLAGS_TRIGGER_NONE)           |
                    CSL_FMK(UDMAP_TR_FLAGS_TRIGGER1_TYPE, CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ALL)  |
                    CSL_FMK(UDMAP_TR_FLAGS_CMD_ID, 0x25U)                                       |   /* This will come back in TR response */
                    CSL_FMK(UDMAP_TR_FLAGS_SA_INDIRECT, 0U)                                     |
                    CSL_FMK(UDMAP_TR_FLAGS_DA_INDIRECT, 0U)                                     |
                    CSL_FMK(UDMAP_TR_FLAGS_EOP, 1U);
    pTr->icnt0    = length;
    pTr->icnt1    = 1U;
    pTr->icnt2    = 1U;
    pTr->icnt3    = 1U;
    pTr->dim1     = pTr->icnt0;
    pTr->dim2     = (pTr->icnt0 * pTr->icnt1);
    pTr->dim3     = (pTr->icnt0 * pTr->icnt1 * pTr->icnt2);
    pTr->addr     = (uint64_t) Udma_appVirtToPhyFxn(srcBuf, 0U, NULL);
    pTr->fmtflags = 0x00000000U;        /* Linear addressing, 1 byte per elem.
                                           Replace with CSL-FL API */
    pTr->dicnt0   = length;
    pTr->dicnt1   = 1U;
    pTr->dicnt2   = 1U;
    pTr->dicnt3   = 1U;
    pTr->ddim1    = pTr->dicnt0;
    pTr->ddim2    = (pTr->dicnt0 * pTr->dicnt1);
    pTr->ddim3    = (pTr->dicnt0 * pTr->dicnt1 * pTr->dicnt2);
    pTr->daddr    = (uint64_t) Udma_appVirtToPhyFxn(destBuf, 0U, NULL);

    /* Clear TR response memory */
    *pTrResp = 0xFFFFFFFFU;

    /* Writeback cache */
    Udma_appUtilsCacheWb(trpdMem, UDMA_TEST_APP_TRPD_SIZE);

    /* Submit TRPD to channel */
    retVal = Udma_ringQueueRaw(Udma_chGetFqRingHandle(chHandle), (uint64_t) Udma_appVirtToPhyFxn(trpdMem, 0U, NULL));
    if (UDMA_SOK != retVal)
    {
        App_print("[Error] Channel queue failed!!\n");
    }

    if (UDMA_SOK == retVal)
    {
        /* Wait for return descriptor in completion ring - this marks the
         * transfer completion */
        SemaphoreP_pend(gUdmaAppDoneSem, SemaphoreP_WAIT_FOREVER);

        /* Response received in completion queue */
        retVal = Udma_ringDequeueRaw(Udma_chGetCqRingHandle(chHandle), &pDesc);
        if (UDMA_SOK != retVal)
        {
            App_print("[Error] No descriptor after callback!!\n");
            retVal = UDMA_EFAIL;
        }
    }

    if (UDMA_SOK == retVal)
    {
        /*
         * Sanity check
         */
        /* Check returned descriptor pointer */
        if (pDesc != ((uint64_t) trpdMem))
        {
            App_print("[Error] TR descriptor pointer returned doesn't "
                   "match the submitted address!!\n");
            retVal = UDMA_EFAIL;
        }
    }

    if (UDMA_SOK == retVal)
    {
        /* Invalidate cache */
        Udma_appUtilsCacheInv(&gUdmaTrpdMem[0U], UDMA_TEST_APP_TRPD_SIZE);

        /* check TR response status */
        pTrResp = (uint32_t *) (trpdMem + (sizeof(CSL_UdmapTR15) * 2U));
        trRespStatus = CSL_FEXT(*pTrResp, UDMAP_TR_RESPONSE_STATUS_TYPE);

        App_printDruCapabilities(chHandle, UDMA_UTC_ID_MSMC_DRU0);

        if (trRespStatus == CSL_UDMAP_TR_RESPONSE_STATUS_COMPLETE)
        {
            UART_printf("[Error] Invalid TR should NOT complete successfully!\n");
            retVal = UDMA_EFAIL;
        }
        else if (trRespStatus == CSL_UDMAP_TR_RESPONSE_STATUS_UNSUPPORTED_ERR)
        {
            uint32_t unsupportedReason =
                CSL_FEXT(*pTrResp, UDMAP_TR_RESPONSE_STATUS_INFO);
        
            UART_printf("TR Status: %s (%u)\n",
                        App_trStatusToStr(trRespStatus),
                        trRespStatus);
            
            UART_printf("Unsupported reason: %s (%u)\n",
                        App_trUnsupportedReasonToStr(unsupportedReason),
                        unsupportedReason);
            
            /* This is EXPECTED behavior */
            retVal = UDMA_SOK;
        }
        else
        {
            UART_printf("[Error] Unexpected TR status: %s (%u)\n",
                        App_trStatusToStr(trRespStatus),
                        trRespStatus);
            
            retVal = UDMA_EFAIL;
        }
    }

    return (retVal);
}

static void App_udmaEventCb(Udma_EventHandle eventHandle, uint32_t eventType, void *appData)
{
    int32_t         retVal;
    CSL_UdmapTdResponse tdResp;

    if (UDMA_EVENT_TYPE_DMA_COMPLETION == eventType)
    {
        SemaphoreP_post(gUdmaAppDoneSem);
    }

    if (UDMA_EVENT_TYPE_TEARDOWN_PACKET == eventType)
    {
        /* Response received in Teardown completion queue */
        retVal = Udma_chDequeueTdResponse(&gUdmaChObj, &tdResp);
        if (UDMA_SOK != retVal)
        {
            /* [Error] No TD response after callback!! */
        }
    }

    return;
}

static void App_print(const char *str)
{
    UART_printf("%s", str);

    if (UTRUE == Udma_appIsPrintSupported())
    {
        printf("%s", str);
    }

    return;
}

static const char *App_trStatusToStr(uint32_t status)
{
    switch (status)
    {
        case CSL_UDMAP_TR_RESPONSE_STATUS_COMPLETE:
            return "Complete";
        case CSL_UDMAP_TR_RESPONSE_STATUS_TRANSFER_ERR:
            return "Transfer Error";
        case CSL_UDMAP_TR_RESPONSE_STATUS_ABORTED_ERR:
            return "Aborted Error";
        case CSL_UDMAP_TR_RESPONSE_STATUS_SUBMISSION_ERR:
            return "Submission Error";
        case CSL_UDMAP_TR_RESPONSE_STATUS_UNSUPPORTED_ERR:
            return "Unsupported Feature";
        case CSL_UDMAP_TR_RESPONSE_STATUS_TRANSFER_EXCEP_ERR:
            return "Transfer Exception";
        default:
            return "Unknown Status";
    }
}

static const char *App_trUnsupportedReasonToStr(uint32_t reason)
{
    switch (reason)
    {
        case CSL_UDMAP_TR_RESPONSE_STATUS_UNSUPPORTED_TR_TYPE:
            return "TR TYPE not supported";
        case CSL_UDMAP_TR_RESPONSE_STATUS_UNSUPPORTED_STATIC:
            return "STATIC not supported";
        case CSL_UDMAP_TR_RESPONSE_STATUS_UNSUPPORTED_EOL:
            return "EOL not supported";
        case CSL_UDMAP_TR_RESPONSE_STATUS_UNSUPPORTED_AMODE:
            return "AMODE not supported";
        case CSL_UDMAP_TR_RESPONSE_STATUS_UNSUPPORTED_ELTYPE:
            return "ELTYPE not supported";
        case CSL_UDMAP_TR_RESPONSE_STATUS_UNSUPPORTED_DFMT:
            return "DFMT not supported";
        case CSL_UDMAP_TR_RESPONSE_STATUS_UNSUPPORTED_SECTR:
            return "SECTR not supported";
        case CSL_UDMAP_TR_RESPONSE_STATUS_UNSUPPORTED_AMODE_SPECIFIC:
            return "AMODE-specific field not supported";
        default:
            return "Unknown unsupported reason";
    }
}
