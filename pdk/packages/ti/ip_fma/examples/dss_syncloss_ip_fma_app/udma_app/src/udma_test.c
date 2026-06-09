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
 *  \file udma_test.c
 *
 *  \brief UDMA memory copy sample application performing block copy using
 *  Type 15 Transfer Record (TR15) using Transfer Record Packet
 *  Descriptor (TRPD).
 *
 *  This file contains the UDMA application that is the part of the DSS8 
 *  safety diagnostic implementation. The application initializes the 
 *  NAVSS internal UDMA engine for DMA transaction, sets up the UDMA channel 
 *  and opens it. Then it performs DMA transfers, on by one keeping track of 
 *  transfer with semaphor and completion/teardown callback functions.
 *
 *  This application is executed in parallel with the DSS display and DRU
 *  application on a separate core to generate additional interconnect
 *  and memory traffic. The resulting bus contention further delays DSS
 *  frame buffer accesses and helps to provoke hardware underflow and
 *  SYNCLOST conditions required for DSS8 safety mechanism verification.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdio.h>
#include <ti/drv/udma/udma.h>
#include <ti/drv/uart/UART.h>
#include <ti/drv/uart/UART_stdio.h>
#include <ti/drv/udma/examples/udma_apputils/udma_apputils.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/*
 * Application test parameters
 */
/** \brief Number of bytes to copy and buffer allocation */
#define UDMA_TEST_APP_NUM_BYTES         (1U * 20U * 1024U)  // 20 KB

/** \brief This ensures every channel memory is aligned */
#define UDMA_TEST_APP_NUM_BYTES_ALIGN    ((UDMA_TEST_APP_NUM_BYTES + UDMA_CACHELINE_ALIGNMENT) & ~(UDMA_CACHELINE_ALIGNMENT - 1U))

/** \brief Number of times to perform the memcpy operation */
#define UDMA_TEST_APP_LOOP_CNT                      (1000000U)

/*
 * Ring parameters
 */
/** \brief Number of ring entries - we can prime this much memcpy operations */
#define UDMA_TEST_APP_RING_ENTRIES      (1U)
/** \brief Size (in bytes) of each ring entry (Size of pointer - 64-bit) */
#define UDMA_TEST_APP_RING_ENTRY_SIZE   (sizeof(uint64_t))
/** \brief Total ring memory */
#define UDMA_TEST_APP_RING_MEM_SIZE     (UDMA_TEST_APP_RING_ENTRIES * UDMA_TEST_APP_RING_ENTRY_SIZE)
/** \brief This ensures every channel memory is aligned */
#define UDMA_TEST_APP_RING_MEM_SIZE_ALIGN ((UDMA_TEST_APP_RING_MEM_SIZE + UDMA_CACHELINE_ALIGNMENT) & ~(UDMA_CACHELINE_ALIGNMENT - 1U))
/**
 *  \brief UDMA TR packet descriptor memory.
 *  This contains the CSL_UdmapCppi5TRPD + Padding to sizeof(CSL_UdmapTR15) +
 *  one Type_15 TR (CSL_UdmapTR15) + one TR response of 4 bytes.
 *  Since CSL_UdmapCppi5TRPD is less than CSL_UdmapTR15, size is just two times
 *  CSL_UdmapTR15 for alignment.
 */
#define UDMA_TEST_APP_TRPD_SIZE         ((sizeof(CSL_UdmapTR15) * 2U) + 4U)
/** \brief This ensures every channel memory is aligned */
#define UDMA_TEST_APP_TRPD_SIZE_ALIGN   ((UDMA_TEST_APP_TRPD_SIZE + UDMA_CACHELINE_ALIGNMENT) & ~(UDMA_CACHELINE_ALIGNMENT - 1U))


/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static int32_t UdmaApp_MemcpyTest(Udma_ChHandle chHandle);
static int32_t UdmaApp_UdmaMemcpy(Udma_ChHandle chHandle,
                                      void *destBuf,
                                      void *srcBuf,
                                      uint32_t length);

static void UdmaApp_EventDmaCb(Udma_EventHandle eventHandle,
                                   uint32_t eventType,
                                   void *appData);

static void UdmaApp_EventTdCb(Udma_EventHandle eventHandle,
                                  uint32_t eventType,
                                  void *appData);

static int32_t UdmaApp_Init(Udma_DrvHandle drvHandle);
static int32_t UdmaApp_Deinit(Udma_DrvHandle drvHandle);

static int32_t UdmaApp_Create(Udma_DrvHandle drvHandle, Udma_ChHandle chHandle);
static int32_t UdmaApp_Delete(Udma_DrvHandle drvHandle, Udma_ChHandle chHandle);

static void UdmaApp_TrpdInit(Udma_ChHandle chHandle,
                                 uint8_t *pTrpdMem,
                                 const void *destBuf,
                                 const void *srcBuf,
                                 uint32_t length);

static void UdmaApp_Print(const char *str);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/*
 * UDMA driver objects
 */
struct Udma_DrvObj      gUdmaDrvObj;
struct Udma_ChObj       gUdmaChObj;
struct Udma_EventObj    gUdmaCqEventObj;
struct Udma_EventObj    gUdmaTdCqEventObj;

/*
 * UDMA Memories
 */
static uint8_t gTxRingMem[UDMA_TEST_APP_RING_MEM_SIZE_ALIGN] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
static uint8_t gTxCompRingMem[UDMA_TEST_APP_RING_MEM_SIZE_ALIGN] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
static uint8_t gTxTdCompRingMem[UDMA_TEST_APP_RING_MEM_SIZE_ALIGN] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
static uint8_t gUdmaTrpdMem[UDMA_TEST_APP_TRPD_SIZE_ALIGN] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));

/*
 * Application Buffers
 */
static uint8_t gUdmaTestSrcBuf[UDMA_TEST_APP_NUM_BYTES_ALIGN] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
static uint8_t gUdmaTestDestBuf[UDMA_TEST_APP_NUM_BYTES_ALIGN] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));

/* Semaphore to indicate transfer completion */
static SemaphoreP_Handle gUdmaAppDoneSem = NULL;

/* Global test pass/fail flag */
static volatile int32_t gUdmaAppResult = UDMA_SOK;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/*
 * UDMA memcpy test
 */
int32_t UdmaApp_UdmaTest(void)
{
    int32_t         retVal;
    Udma_DrvHandle  drvHandle = &gUdmaDrvObj;
    Udma_ChHandle   chHandle = &gUdmaChObj;

    UdmaApp_Print("UDMA memcpy application started...\n");
    
    retVal = UdmaApp_Init(drvHandle);
    if(UDMA_SOK != retVal)
    {
        UdmaApp_Print("[Error] UDMA App init failed!!\n");
    }

    if(UDMA_SOK == retVal)
    {
        retVal = UdmaApp_Create(drvHandle, chHandle);
        if(UDMA_SOK != retVal)
        {
            UdmaApp_Print("[Error] UDMA App create failed!!\n");
        }
        else
        {
            UART_printf("[Success] Created UDMA channel \n");
        }
    }

    if(UDMA_SOK == retVal)
    {
        retVal = UdmaApp_MemcpyTest(chHandle);
        if(UDMA_SOK != retVal)
        {
            UdmaApp_Print("[Error] UDMA App memcpy test failed!!\n");
        }
    }

    retVal += UdmaApp_Delete(drvHandle, chHandle);
    if(UDMA_SOK != retVal)
    {
        UdmaApp_Print("[Error] UDMA App delete failed!!\n");
    }

    retVal += UdmaApp_Deinit(drvHandle);
    if(UDMA_SOK != retVal)
    {
        UdmaApp_Print("[Error] UDMA App deinit failed!!\n");
    }
    
    if((UDMA_SOK == retVal) && (UDMA_SOK == gUdmaAppResult))
    {
        UdmaApp_Print("UDMA memcpy using TR15 block copy Passed!!\n");
        UdmaApp_Print("All tests have passed!!\n");
    }
    else
    {
        UdmaApp_Print("UDMA memcpy using TR15 block copy Failed!!\n");
        UdmaApp_Print("Some tests have failed!!\n");
    }

    return (0);
}

static int32_t UdmaApp_MemcpyTest(Udma_ChHandle chHandle)
{
    int32_t             retVal = UDMA_SOK;
    uint32_t            loopCnt = 0U;
    uint8_t            *srcBuf = &gUdmaTestSrcBuf[0U];
    uint8_t            *destBuf = &gUdmaTestDestBuf[0U];
    uint32_t            i;

    /* Init buffers */
    for(i = 0U; i < UDMA_TEST_APP_NUM_BYTES; i++)
    {
        srcBuf[i] = i;
        destBuf[i] = 0U;
    }
    /* Writeback source and destination buffer */
    Udma_appUtilsCacheWb(&gUdmaTestSrcBuf[0U], UDMA_TEST_APP_NUM_BYTES);
    Udma_appUtilsCacheWb(&gUdmaTestDestBuf[0U], UDMA_TEST_APP_NUM_BYTES);

    while(loopCnt < UDMA_TEST_APP_LOOP_CNT)
    {
        if (loopCnt % (UDMA_TEST_APP_LOOP_CNT/10) == 0)
        {
            UART_printf("UDMA iteration %d ...\n", loopCnt);
        }
        /* Perform UDMA memcpy */
        retVal = UdmaApp_UdmaMemcpy(chHandle,
                                        destBuf,
                                        srcBuf,
                                        UDMA_TEST_APP_NUM_BYTES);

        if(UDMA_SOK != retVal)
        {
            break;
        }

        loopCnt++;
    }

    return (retVal);
}

static int32_t UdmaApp_UdmaMemcpy(Udma_ChHandle chHandle,
                                      void *destBuf,
                                      void *srcBuf,
                                      uint32_t length)
{
    int32_t     retVal = UDMA_SOK;
    uint32_t   *pTrResp, trRespStatus;
    uint64_t    pDesc = 0;
    uint8_t    *trpdMem = &gUdmaTrpdMem[0U];

    /* Update TR packet descriptor */
    UdmaApp_TrpdInit(chHandle, trpdMem, destBuf, srcBuf, length);

    /* Submit TRPD to channel */
    retVal = Udma_ringQueueRaw(
                 Udma_chGetFqRingHandle(chHandle), (uint64_t) Udma_appVirtToPhyFxn(trpdMem, 0U, NULL));
    if(UDMA_SOK != retVal)
    {
        UdmaApp_Print("[Error] Channel queue failed!!\n");
    }

    if(UDMA_SOK == retVal)
    {
        /* Wait for return descriptor in completion ring - this marks the
         * transfer completion */
        SemaphoreP_pend(gUdmaAppDoneSem, SemaphoreP_WAIT_FOREVER);

        /* Response received in completion queue */
        retVal = Udma_ringDequeueRaw(Udma_chGetCqRingHandle(chHandle), &pDesc);
        if(UDMA_SOK != retVal)
        {
            UdmaApp_Print("[Error] No descriptor after callback!!\n");
            retVal = UDMA_EFAIL;
        }
    }

    if(UDMA_SOK == retVal)
    {
        /*
         * Sanity check
         */
        /* Check returned descriptor pointer */
        if(((uint64_t) Udma_appPhyToVirtFxn(pDesc, 0U, NULL)) != ((uint64_t) trpdMem))
        {
            UdmaApp_Print("[Error] TR descriptor pointer returned doesn't "
                   "match the submitted address!!\n");
            retVal = UDMA_EFAIL;
        }
    }

    if(UDMA_SOK == retVal)
    {
        /* Invalidate cache */
        Udma_appUtilsCacheInv(&gUdmaTrpdMem[0U], UDMA_TEST_APP_TRPD_SIZE);

        /* check TR response status */
        pTrResp = (uint32_t *) (trpdMem + (sizeof(CSL_UdmapTR15) * 2U));
        trRespStatus = CSL_FEXT(*pTrResp, UDMAP_TR_RESPONSE_STATUS_TYPE);
        if(trRespStatus != CSL_UDMAP_TR_RESPONSE_STATUS_COMPLETE)
        {
            UdmaApp_Print("[Error] TR Response not completed!!\n");
            retVal = UDMA_EFAIL;
        }
    }

    return (retVal);
}

static void UdmaApp_EventDmaCb(Udma_EventHandle eventHandle,
                                   uint32_t eventType,
                                   void *appData)
{
    if(UDMA_EVENT_TYPE_DMA_COMPLETION == eventType)
    {
        SemaphoreP_post(gUdmaAppDoneSem);
    }
    else
    {
        gUdmaAppResult = UDMA_EFAIL;
    }

    return;
}

static void UdmaApp_EventTdCb(Udma_EventHandle eventHandle,
                                  uint32_t eventType,
                                  void *appData)
{
    int32_t             retVal;
    CSL_UdmapTdResponse tdResp;

    if(UDMA_EVENT_TYPE_TEARDOWN_PACKET == eventType)
    {
        /* Response received in Teardown completion queue */
        retVal = Udma_chDequeueTdResponse(&gUdmaChObj, &tdResp);
        if(UDMA_SOK != retVal)
        {
            /* [Error] No TD response after callback!! */
            gUdmaAppResult = UDMA_EFAIL;
        }
    }
    else
    {
        gUdmaAppResult = UDMA_EFAIL;
    }

    return;
}

static int32_t UdmaApp_Init(Udma_DrvHandle drvHandle)
{
    int32_t         retVal;
    Udma_InitPrms   initPrms;
    uint32_t        instId;

    /* Use MCU NAVSS for MCU domain cores. Rest cores all uses Main NAVSS */
#if defined (BUILD_MCU1_0) || defined (BUILD_MCU1_1)
    instId = UDMA_INST_ID_MCU_0;
#else
    instId = UDMA_INST_ID_MAIN_0;
#endif


    /* UDMA driver init */
    retVal = UdmaInitPrms_init(instId, &initPrms);
    if(UDMA_SOK != retVal)
    {
        UdmaApp_Print("[Error] UDMA init prms init failed!!\n");
    }
    else
    {
        initPrms.virtToPhyFxn   = &Udma_appVirtToPhyFxn;
        initPrms.phyToVirtFxn   = &Udma_appPhyToVirtFxn;
        initPrms.printFxn       = &UdmaApp_Print;
        retVal = Udma_init(drvHandle, &initPrms);
        if(UDMA_SOK != retVal)
        {
            UdmaApp_Print("[Error] UDMA init failed!!\n");
        }
    }

    return (retVal);
}

static int32_t UdmaApp_Deinit(Udma_DrvHandle drvHandle)
{
    int32_t     retVal;

    retVal = Udma_deinit(drvHandle);
    if(UDMA_SOK != retVal)
    {
        UdmaApp_Print("[Error] UDMA deinit failed!!\n");
    }

    return (retVal);
}

static int32_t UdmaApp_Create(Udma_DrvHandle drvHandle, Udma_ChHandle chHandle)
{
    int32_t             retVal = UDMA_SOK;
    uint32_t            chType;
    Udma_ChPrms         chPrms;
    Udma_ChTxPrms       txPrms;
    Udma_ChRxPrms       rxPrms;
    Udma_EventHandle    tdCqEventHandle;
    Udma_EventPrms      tdCqEventPrms;
    Udma_EventHandle    cqEventHandle;
    Udma_EventPrms      cqEventPrms;
    SemaphoreP_Params   semPrms;

    SemaphoreP_Params_init(&semPrms);
    gUdmaAppDoneSem = SemaphoreP_create(0, &semPrms);
    if(NULL == gUdmaAppDoneSem)
    {
        UdmaApp_Print("[Error] Sem create failed!!\n");
        retVal = UDMA_EFAIL;
    }

    if(UDMA_SOK == retVal)
    {
        /* Init channel parameters */
        chType = UDMA_CH_TYPE_TR_BLK_COPY;
        UdmaChPrms_init(&chPrms, chType);
        chPrms.fqRingPrms.ringMem       = &gTxRingMem[0U];
        chPrms.fqRingPrms.ringMemSize   = UDMA_TEST_APP_RING_MEM_SIZE;
        chPrms.fqRingPrms.elemCnt       = UDMA_TEST_APP_RING_ENTRIES;
        chPrms.cqRingPrms.ringMem       = &gTxCompRingMem[0U];
        chPrms.cqRingPrms.ringMemSize   = UDMA_TEST_APP_RING_MEM_SIZE;
        chPrms.cqRingPrms.elemCnt       = UDMA_TEST_APP_RING_ENTRIES;
        chPrms.tdCqRingPrms.ringMem     = &gTxTdCompRingMem[0U];
        chPrms.tdCqRingPrms.ringMemSize = UDMA_TEST_APP_RING_MEM_SIZE;
        chPrms.tdCqRingPrms.elemCnt     = UDMA_TEST_APP_RING_ENTRIES;

        /* Open channel for block copy */
        retVal = Udma_chOpen(drvHandle, chHandle, chType, &chPrms);
        if(UDMA_SOK != retVal)
        {
            UdmaApp_Print("[Error] UDMA channel open failed!!\n");
        }
    }

    if(UDMA_SOK == retVal)
    {
        /* Config TX channel */
        UdmaChTxPrms_init(&txPrms, chType);
        retVal = Udma_chConfigTx(chHandle, &txPrms);
        if(UDMA_SOK != retVal)
        {
            UdmaApp_Print("[Error] UDMA TX channel config failed!!\n");
        }
    }

    if(UDMA_SOK == retVal)
    {
        /* Config RX channel - which is implicitly paired to TX channel in
         * block copy mode */
        UdmaChRxPrms_init(&rxPrms, chType);
        retVal = Udma_chConfigRx(chHandle, &rxPrms);
        if(UDMA_SOK != retVal)
        {
            UdmaApp_Print("[Error] UDMA RX channel config failed!!\n");
        }
    }

    if(UDMA_SOK == retVal)
    {
        /* Register ring completion callback */
        cqEventHandle = &gUdmaCqEventObj;
        UdmaEventPrms_init(&cqEventPrms);
        cqEventPrms.eventType         = UDMA_EVENT_TYPE_DMA_COMPLETION;
        cqEventPrms.eventMode         = UDMA_EVENT_MODE_SHARED;
        cqEventPrms.chHandle          = chHandle;
        cqEventPrms.masterEventHandle = Udma_eventGetGlobalHandle(drvHandle);
        cqEventPrms.eventCb           = &UdmaApp_EventDmaCb;
        retVal = Udma_eventRegister(drvHandle, cqEventHandle, &cqEventPrms);
        if(UDMA_SOK != retVal)
        {
            UdmaApp_Print("[Error] UDMA CQ event register failed!!\n");
        }
    }

    if(UDMA_SOK == retVal)
    {
        /* Register teardown ring completion callback */
        tdCqEventHandle = &gUdmaTdCqEventObj;
        UdmaEventPrms_init(&tdCqEventPrms);
        tdCqEventPrms.eventType         = UDMA_EVENT_TYPE_TEARDOWN_PACKET;
        tdCqEventPrms.eventMode         = UDMA_EVENT_MODE_SHARED;
        tdCqEventPrms.chHandle          = chHandle;
        tdCqEventPrms.masterEventHandle = Udma_eventGetGlobalHandle(drvHandle);
        tdCqEventPrms.eventCb           = &UdmaApp_EventTdCb;
        retVal = Udma_eventRegister(drvHandle, tdCqEventHandle, &tdCqEventPrms);
        if(UDMA_SOK != retVal)
        {
            UdmaApp_Print("[Error] UDMA Teardown CQ event register failed!!\n");
        }
    }

    if(UDMA_SOK == retVal)
    {
        /* Channel enable */
        retVal = Udma_chEnable(chHandle);
        if(UDMA_SOK != retVal)
        {
            UdmaApp_Print("[Error] UDMA channel enable failed!!\n");
        }
    }

    return (retVal);
}

static int32_t UdmaApp_Delete(Udma_DrvHandle drvHandle, Udma_ChHandle chHandle)
{
    int32_t             retVal, tempRetVal;
    uint64_t            pDesc;
    Udma_EventHandle    cqEventHandle;
    Udma_EventHandle    tdCqEventHandle;

    retVal = Udma_chDisable(chHandle, UDMA_DEFAULT_CH_DISABLE_TIMEOUT);
    if(UDMA_SOK != retVal)
    {
        UdmaApp_Print("[Error] UDMA channel disable failed!!\n");
    }

    /* Flush any pending request from the free queue */
    while(1)
    {
        tempRetVal = Udma_ringFlushRaw(
                         Udma_chGetFqRingHandle(chHandle), &pDesc);
        if(UDMA_ETIMEOUT == tempRetVal)
        {
            break;
        }
    }

    /* Unregister all events */
    cqEventHandle = &gUdmaCqEventObj;
    retVal += Udma_eventUnRegister(cqEventHandle);
    if(UDMA_SOK != retVal)
    {
        UdmaApp_Print("[Error] UDMA event unregister failed!!\n");
    }
    
    tdCqEventHandle = &gUdmaTdCqEventObj;
    retVal += Udma_eventUnRegister(tdCqEventHandle);
    if(UDMA_SOK != retVal)
    {
        UdmaApp_Print("[Error] UDMA event unregister failed!!\n");
    }

    retVal += Udma_chClose(chHandle);
    if(UDMA_SOK != retVal)
    {
        UdmaApp_Print("[Error] UDMA channel close failed!!\n");
    }

    if(gUdmaAppDoneSem != NULL)
    {
        SemaphoreP_delete(gUdmaAppDoneSem);
        gUdmaAppDoneSem = NULL;
    }

    return (retVal);
}

static void UdmaApp_TrpdInit(Udma_ChHandle chHandle,
                                 uint8_t *pTrpdMem,
                                 const void *destBuf,
                                 const void *srcBuf,
                                 uint32_t length)
{
    CSL_UdmapCppi5TRPD *pTrpd = (CSL_UdmapCppi5TRPD *) pTrpdMem;
    CSL_UdmapTR15 *pTr = (CSL_UdmapTR15 *)UdmaUtils_getTrpdTr1Pointer(pTrpdMem, 0U);
    uint32_t *pTrResp = (uint32_t *) (pTrpdMem + (sizeof(CSL_UdmapTR15) * 2U));
    uint32_t cqRingNum = Udma_chGetCqRingNum(chHandle);

    /* Make TRPD */
    UdmaUtils_makeTrpd(pTrpd, UDMA_TR_TYPE_15, 1U, cqRingNum);

    /* Choose a line size that fits in 16 bits */
    uint32_t lineSize = 1024U;          /* bytes per line */
    uint32_t lineCnt  = length / lineSize;

    /* Sanity: ensure length is multiple of lineSize and fits icnt1(16-bit) */
    if ((length % lineSize) != 0U || (lineCnt > 0xFFFFU))
    {
        /* fall back or assert; for now just clamp */
        lineSize = 4096U;
        lineCnt  = length / lineSize;
    }

    /* Setup TR */
    pTr->flags    = CSL_FMK(UDMAP_TR_FLAGS_TYPE, 15)                                            |
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
    pTr->icnt0    = (uint16_t)lineSize;
    pTr->icnt1    = lineCnt;
    pTr->icnt2    = 1U;
    pTr->icnt3    = 1U;
    pTr->dim1     = pTr->icnt0;
    pTr->dim2     = (pTr->icnt0 * pTr->icnt1);
    pTr->dim3     = (pTr->icnt0 * pTr->icnt1 * pTr->icnt2);
    pTr->addr     = (uint64_t) Udma_appVirtToPhyFxn(srcBuf, 0U, NULL);
    pTr->fmtflags = 0x00000000U;        /* Linear addressing, 1 byte per elem.
                                           Replace with CSL-FL API */
    pTr->dicnt0   = pTr->icnt0;
    pTr->dicnt1   = pTr->icnt1;
    pTr->dicnt2   = 1U;
    pTr->dicnt3   = 1U;
    pTr->ddim1    = pTr->dicnt0;
    pTr->ddim2    = (pTr->dicnt0 * pTr->dicnt1);
    pTr->ddim3    = (pTr->dicnt0 * pTr->dicnt1 * pTr->dicnt2);
    pTr->daddr    = (uint64_t) Udma_appVirtToPhyFxn(destBuf, 0U, NULL);

    /* Clear TR response memory */
    *pTrResp = 0xFFFFFFFFU;

    /* Writeback cache */
    Udma_appUtilsCacheWb(pTrpdMem, UDMA_TEST_APP_TRPD_SIZE);

    return;
}

static void UdmaApp_Print(const char *str)
{
    UART_printf("%s", str);
    if(UTRUE == Udma_appIsPrintSupported())
    {
        printf("%s", str);
    }

    return;
}
