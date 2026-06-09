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
 *  \file   udma_transm_redund_test.c
 *
 *  \brief  This file contains code that performs transmission redundancy test.
 *
*/

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdio.h>
#include <ti/drv/udma/udma.h>
#include <ti/csl/soc.h>
#include <ti/csl/csl_crc.h>
#include <ti/drv/uart/UART.h>
#include <ti/drv/uart/UART_stdio.h>
#include <ti/drv/udma/examples/udma_apputils/udma_apputils.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/*
 * Application test parameters
 */
/** \brief Number of times to perform the CRC operation */
#define UDMA_TEST_APP_LOOP_CNT          (3U)

/*
 * Ring parameters
 */
/** \brief Number of ring entries - we can prime this much CRC operations */
#define UDMA_TEST_APP_RING_ENTRIES      (1U)
/** \brief Size (in bytes) of each ring entry (Size of pointer - 64-bit) */
#define UDMA_TEST_APP_RING_ENTRY_SIZE   (sizeof(uint64_t))
/** \brief Total ring memory */
#define UDMA_TEST_APP_RING_MEM_SIZE     (UDMA_TEST_APP_RING_ENTRIES * \
                                         UDMA_TEST_APP_RING_ENTRY_SIZE)
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

/* Pre-calculated crc signature value for given data pattern */
#define APP_CRC_REFERENCE_SIGN_VAL_L    (0x83A8C73AU)
#define APP_CRC_REFERENCE_SIGN_VAL_H    (0x18633761U)

/* Frame details - used as reference data */
#define APP_FRAME_HEIGHT                ((uint32_t) 200U)
#define APP_FRAME_WIDTH                 ((uint32_t) 100U)
#define APP_FRAME_SIZE                  (APP_FRAME_HEIGHT * APP_FRAME_WIDTH)
#define APP_FRAME_SIZE_ALIGN            ((APP_FRAME_SIZE + UDMA_CACHELINE_ALIGNMENT) & ~(UDMA_CACHELINE_ALIGNMENT - 1U))

/* CRC channel parameters */
#define APP_CRC_CHANNEL                 (CRC_CHANNEL_1)
#define APP_CRC_CH_CCITENR_MASK         (CRC_INTR_CH1_CCITENR_MASK)
#define APP_CRC_WATCHDOG_PRELOAD_VAL    ((uint32_t) 0U)
#define APP_CRC_BLOCK_PRELOAD_VAL       ((uint32_t) 0U)

/* CRC size parameters */
#define APP_CRC_PATTERN_SIZE            ((uint32_t) 4U)
#define APP_CRC_SECT_CNT                ((uint32_t) 1U)

/* Use MCU NAVSS/peripherals for MCU domain cores. */
#define APP_CRC_BASE                    (CSL_MCU_NAVSS0_MCRC_BASE)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static int32_t Udma3App_CrcTest(Udma_ChHandle chHandle);
static int32_t Udma3App_UdmaCrc(Udma_ChHandle chHandle,
                           void *srcBuf,
                           uint32_t length,
                           crcSignature_t *signatureVal_);

static void Udma3App_UdmaEventCb(Udma_EventHandle eventHandle,
                            uint32_t eventType,
                            void *appData);

static int32_t Udma3App_Init(Udma_DrvHandle drvHandle);
static int32_t Udma3App_Deinit(Udma_DrvHandle drvHandle);

static int32_t Udma3App_Create(Udma_DrvHandle drvHandle, Udma_ChHandle chHandle);
static int32_t Udma3App_Delete(Udma_DrvHandle drvHandle, Udma_ChHandle chHandle);

static void Udma3App_UdmaTrpdInit(Udma_ChHandle chHandle,
                             uint8_t *pTrpdMem,
                             const void *srcBuf,
                             const void *destBuf,
                             uint32_t length);
static void Udma3App_CrcInit(void);

static void Udma3App_Print(const char *str);

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
static uint8_t gCrcSrcBufA[APP_FRAME_SIZE_ALIGN] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT), section(".data"))) = {1U};
static uint8_t gCrcSrcBufB[APP_FRAME_SIZE_ALIGN] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT), section(".data"))) = {1U};

/* Semaphore to indicate transfer completion */
static SemaphoreP_Handle gUdmaAppDoneSem = NULL;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/*
 * Application main
 */
int32_t Udma3App_CrcTrRdTest(void)
{
    int32_t         retVal = UDMA_SOK;
    int32_t         status = UDMA_SOK;
    Udma_DrvHandle  drvHandle = &gUdmaDrvObj;
    Udma_ChHandle   chHandle = &gUdmaChObj;

    int32_t initDone = 0;
    int32_t createDone = 0;

    Udma3App_Print("UDMA CRC application started...\n");

    retVal = Udma3App_Init(drvHandle);
    if(UDMA_SOK != retVal)
    {
        Udma3App_Print("[Error] UDMA App init failed!!\n");
    }
    else
    {
        initDone = 1;

        retVal = Udma3App_Create(drvHandle, chHandle);
        if(UDMA_SOK != retVal)
        {
            Udma3App_Print("[Error] UDMA App create failed!!\n");
        }
        else
        {
            createDone = 1;

            retVal = Udma3App_CrcTest(chHandle);
            if(UDMA_SOK != retVal)
            {
                Udma3App_Print("[Error] UDMA App CRC test failed!!\n");
            }
        }
    }

    if (createDone)
    {
        status = Udma3App_Delete(drvHandle, chHandle);
        if(UDMA_SOK != status)
        {
            Udma3App_Print("[Error] UDMA App delete failed!!\n");

            if (UDMA_SOK == retVal)
            {
                retVal = status;
            }
        }
    }

    if(initDone)
    {
        status = Udma3App_Deinit(drvHandle);
        if(UDMA_SOK != status)
        {
            Udma3App_Print("[Error] UDMA App deinit failed!!\n");

            if (UDMA_SOK == retVal)
            {
                retVal = status;
            }
        }
    }
    
    if(UDMA_SOK == retVal)
    {
        Udma3App_Print("UDMA CRC Test Passed!!\n");
        Udma3App_Print("All tests have passed!!\n");
    }
    else
    {
        Udma3App_Print("UDMA CRC Test Failed!!\n");
        Udma3App_Print("Some tests have failed!!\n");
    }

    return retVal;
}

static int32_t Udma3App_CrcTest(Udma_ChHandle chHandle)
{
    int32_t     retVal = UDMA_SOK;
    uint32_t    i;
    uint32_t    loopCnt = 0U;
    uint8_t    *srcBufA = &gCrcSrcBufA[0U];
    uint8_t    *srcBufB = &gCrcSrcBufB[0U];

    uint32_t   *srcBufA32 = (uint32_t *) srcBufA;
    uint32_t   *srcBufB32 = (uint32_t *) srcBufB;
    

    /* Init buffers */
    for(i = 0U; i < (APP_FRAME_SIZE / sizeof(uint32_t)); i++)
    {
        srcBufA32[i] = i;
        srcBufB32[i] = i;
    }
    /* Writeback source buffer */
    Udma_appUtilsCacheWb(&gCrcSrcBufA[0U], APP_FRAME_SIZE);
    Udma_appUtilsCacheWb(&gCrcSrcBufB[0U], APP_FRAME_SIZE);
    
    crcSignature_t signatureValue_A;
    crcSignature_t signatureValue_B;

    while(loopCnt < UDMA_TEST_APP_LOOP_CNT)
    {
        Udma3App_Print("\nTest\n");
        Udma3App_Print("***************************************************************************\n");

        /* Perform UDMA CRC for srcBufA */
        Udma3App_Print("-- srcBufA transfer start\n");
        retVal = Udma3App_UdmaCrc(chHandle, srcBufA, APP_FRAME_SIZE, &signatureValue_A);
        if(UDMA_SOK != retVal)
        {
            break;
        }
        Udma3App_Print("-- srcBufA transfer finished\n");

        /* Perform UDMA CRC for srcBufB */
        Udma3App_Print("-- srcBufB transfer start\n");
        retVal = Udma3App_UdmaCrc(chHandle, srcBufB, APP_FRAME_SIZE, &signatureValue_B);
        if(UDMA_SOK != retVal)
        {
            break;
        }
        Udma3App_Print("-- srcBufB transfer finished\n");

        if((signatureValue_A.regH == signatureValue_B.regH) &&
           (signatureValue_A.regL == signatureValue_B.regL))
        {
            Udma3App_Print("---- Signature Values A and B MATCH!\n\n");
        }
        else
        {
            Udma3App_Print("---- Signature Values A and B DO NOT MATCH!\n\n");
            retVal = UDMA_EFAIL;
        }

        loopCnt++;
    }

    return (retVal);
}

static int32_t Udma3App_UdmaCrc(Udma_ChHandle chHandle,
                           void *srcBuf,
                           uint32_t length,
                           crcSignature_t *signatureVal_)
{
    int32_t               retVal = UDMA_SOK;
    uint32_t             *pTrResp, trRespStatus;
    uint64_t              pDesc = 0;
    uint8_t              *trpdMem = &gUdmaTrpdMem[0U];
    crcSignature_t        sectSignVal;
    crcSignatureRegAddr_t psaSignRegAddr;
    uint32_t              patternCnt;

    sectSignVal.regL = 0U;
    sectSignVal.regH = 0U;
    patternCnt = length / APP_CRC_PATTERN_SIZE;

    /* Get CRC PSA signature register address */
    CRCGetPSASigRegAddr(APP_CRC_BASE, APP_CRC_CHANNEL, &psaSignRegAddr);
    CRCChannelReset(APP_CRC_BASE, APP_CRC_CHANNEL);
    CRCConfigure(APP_CRC_BASE, APP_CRC_CHANNEL, patternCnt, APP_CRC_SECT_CNT, CRC_OPERATION_MODE_SEMICPU);

    /* Update TR packet descriptor */
    Udma3App_UdmaTrpdInit(chHandle, trpdMem, srcBuf, (void *)(uintptr_t) psaSignRegAddr.regL, length);

    /* Submit TRPD to channel */
    retVal = Udma_ringQueueRaw(Udma_chGetFqRingHandle(chHandle), (uint64_t) Udma_appVirtToPhyFxn(trpdMem, 0U, NULL));
    if(UDMA_SOK != retVal)
    {
        Udma3App_Print("[Error] Channel queue failed!!\n");
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
            Udma3App_Print("[Error] No descriptor after callback!!\n");
            retVal = UDMA_EFAIL;
        }
    }

    if(UDMA_SOK == retVal)
    {
        /*
         * Sanity check
         */
        /* Check returned descriptor pointer */
        if (((uint64_t) Udma_appPhyToVirtFxn(pDesc, 0U, NULL)) != ((uint64_t) trpdMem))
        {
            Udma3App_Print("[Error] TR descriptor pointer returned doesn't "
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
            Udma3App_Print("[Error] TR Response not completed!!\n");
            retVal = UDMA_EFAIL;
        }
    }

    if(UDMA_SOK == retVal)
    {
        uint32_t intrStatus;

        while (1U)
        {
            CRCGetIntrStatus(APP_CRC_BASE, APP_CRC_CHANNEL, &intrStatus);
            if((intrStatus & APP_CRC_CH_CCITENR_MASK) == 0x1U)
            {
                break;
            }
            /* Wait here till CRC compression complete is set. */
        }

        CRCGetPSASectorSig(APP_CRC_BASE, APP_CRC_CHANNEL, &sectSignVal);
        /* Compare CRC signature value against reference CRC signature */
        if((sectSignVal.regH == APP_CRC_REFERENCE_SIGN_VAL_H) &&
           (sectSignVal.regL == APP_CRC_REFERENCE_SIGN_VAL_L))
        {
            /* Sector signature matches - Passed */
            Udma3App_Print("---- Sector signature match with pre-calculated value.\n");
        }
        else
        {
            Udma3App_Print("---- Sector signature does not match with pre-calculated value.\n");
            retVal = UDMA_EFAIL;
        }
        
        /* Update the signature value passed as function argument */
        signatureVal_->regH = sectSignVal.regH;
        signatureVal_->regL = sectSignVal.regL;

        CRCClearIntr(APP_CRC_BASE, APP_CRC_CHANNEL, CRC_CHANNEL_IRQSTATUS_RAW_MAIN_ALL);
    }

    return (retVal);
}

static void Udma3App_UdmaEventCb(Udma_EventHandle eventHandle,
                            uint32_t eventType,
                            void *appData)
{
    int32_t         retVal;
    CSL_UdmapTdResponse tdResp;

    if(UDMA_EVENT_TYPE_DMA_COMPLETION == eventType)
    {
        SemaphoreP_post(gUdmaAppDoneSem);
    }
    if(UDMA_EVENT_TYPE_TEARDOWN_PACKET == eventType)
    {
        /* Response received in Teardown completion queue */
        retVal = Udma_chDequeueTdResponse(&gUdmaChObj, &tdResp);
        if(UDMA_SOK != retVal)
        {
            /* [Error] No TD response after callback!! */
        }
    }

    return;
}

static int32_t Udma3App_Init(Udma_DrvHandle drvHandle)
{
    int32_t         retVal;
    Udma_InitPrms   initPrms;
    uint32_t        instId;

    /* Use MCU NAVSS for MCU domain cores. Rest all cores uses Main NAVSS */
    instId = UDMA_INST_ID_MCU_0;

    /* UDMA driver init */
    UdmaInitPrms_init(instId, &initPrms);
    initPrms.printFxn = &Udma3App_Print;
    retVal = Udma_init(drvHandle, &initPrms);
    if(UDMA_SOK != retVal)
    {
        Udma3App_Print("[Error] UDMA init failed!!\n");
    }

    return (retVal);
}

static int32_t Udma3App_Deinit(Udma_DrvHandle drvHandle)
{
    int32_t     retVal;

    retVal = Udma_deinit(drvHandle);
    if(UDMA_SOK != retVal)
    {
        Udma3App_Print("[Error] UDMA deinit failed!!\n");
    }

    return (retVal);
}

static int32_t Udma3App_Create(Udma_DrvHandle drvHandle, Udma_ChHandle chHandle)
{
    int32_t             retVal = UDMA_SOK;
    uint32_t            chType;
    Udma_ChPrms         chPrms;
    Udma_ChTxPrms       txPrms;
    Udma_ChRxPrms       rxPrms;
    Udma_EventHandle    eventHandle;
    Udma_EventPrms      eventPrms;
    SemaphoreP_Params   semPrms;

    Udma3App_CrcInit();

    SemaphoreP_Params_init(&semPrms);
    gUdmaAppDoneSem = SemaphoreP_create(0, &semPrms);
    if(NULL == gUdmaAppDoneSem)
    {
        Udma3App_Print("[Error] Sem create failed!!\n");
        retVal = UDMA_EFAIL;
    }

    if(UDMA_SOK == retVal)
    {
        /* Init channel parameters */
        chType = UDMA_CH_TYPE_TR_BLK_COPY;
        UdmaChPrms_init(&chPrms, chType);
        chPrms.fqRingPrms.ringMem   = &gTxRingMem[0U];
        chPrms.cqRingPrms.ringMem   = &gTxCompRingMem[0U];
        chPrms.tdCqRingPrms.ringMem = &gTxTdCompRingMem[0U];
        chPrms.fqRingPrms.ringMemSize   = UDMA_TEST_APP_RING_MEM_SIZE;
        chPrms.cqRingPrms.ringMemSize   = UDMA_TEST_APP_RING_MEM_SIZE;
        chPrms.tdCqRingPrms.ringMemSize = UDMA_TEST_APP_RING_MEM_SIZE;
        chPrms.fqRingPrms.elemCnt   = UDMA_TEST_APP_RING_ENTRIES;
        chPrms.cqRingPrms.elemCnt   = UDMA_TEST_APP_RING_ENTRIES;
        chPrms.tdCqRingPrms.elemCnt = UDMA_TEST_APP_RING_ENTRIES;

        /* Open channel for block copy */
        retVal = Udma_chOpen(drvHandle, chHandle, chType, &chPrms);
        if(UDMA_SOK != retVal)
        {
            Udma3App_Print("[Error] UDMA channel open failed!!\n");
        }
    }

    if(UDMA_SOK == retVal)
    {
        /* Config TX channel */
        UdmaChTxPrms_init(&txPrms, chType);
        retVal = Udma_chConfigTx(chHandle, &txPrms);
        if(UDMA_SOK != retVal)
        {
            Udma3App_Print("[Error] UDMA TX channel config failed!!\n");
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
            Udma3App_Print("[Error] UDMA RX channel config failed!!\n");
        }
    }

    if(UDMA_SOK == retVal)
    {
        /* Register ring completion callback */
        eventHandle = &gUdmaCqEventObj;
        UdmaEventPrms_init(&eventPrms);
        eventPrms.eventType         = UDMA_EVENT_TYPE_DMA_COMPLETION;
        eventPrms.eventMode         = UDMA_EVENT_MODE_SHARED;
        eventPrms.chHandle          = chHandle;
        eventPrms.masterEventHandle = Udma_eventGetGlobalHandle(drvHandle);
        eventPrms.eventCb           = &Udma3App_UdmaEventCb;
        retVal = Udma_eventRegister(drvHandle, eventHandle, &eventPrms);
        if(UDMA_SOK != retVal)
        {
            Udma3App_Print("[Error] UDMA CQ event register failed!!\n");
        }
    }

    if(UDMA_SOK == retVal)
    {
        /* Register teardown ring completion callback */
        eventHandle = &gUdmaTdCqEventObj;
        UdmaEventPrms_init(&eventPrms);
        eventPrms.eventType         = UDMA_EVENT_TYPE_TEARDOWN_PACKET;
        eventPrms.eventMode         = UDMA_EVENT_MODE_SHARED;
        eventPrms.chHandle          = chHandle;
        eventPrms.masterEventHandle = Udma_eventGetGlobalHandle(drvHandle);
        eventPrms.eventCb           = &Udma3App_UdmaEventCb;
        retVal = Udma_eventRegister(drvHandle, eventHandle, &eventPrms);
        if(UDMA_SOK != retVal)
        {
            Udma3App_Print("[Error] UDMA Teardown CQ event register failed!!\n");
        }
    }

    if(UDMA_SOK == retVal)
    {
        /* Channel enable */
        retVal = Udma_chEnable(chHandle);
        if(UDMA_SOK != retVal)
        {
            Udma3App_Print("[Error] UDMA channel enable failed!!\n");
        }
    }

    return (retVal);
}

static int32_t Udma3App_Delete(Udma_DrvHandle drvHandle, Udma_ChHandle chHandle)
{
    int32_t             retVal, tempRetVal;
    uint64_t            pDesc;
    Udma_EventHandle    eventHandle;

    retVal = Udma_chDisable(chHandle, UDMA_DEFAULT_CH_DISABLE_TIMEOUT);
    if(UDMA_SOK != retVal)
    {
        Udma3App_Print("[Error] UDMA channel disable failed!!\n");
    }

    /* Flush any pending request from the free queue */
    while (1)
    {
        tempRetVal = Udma_ringFlushRaw(
                         Udma_chGetFqRingHandle(chHandle), &pDesc);
        if (UDMA_ETIMEOUT == tempRetVal)
        {
            break;
        }
    }

    /* Unregister all events */
    eventHandle = &gUdmaTdCqEventObj;
    retVal += Udma_eventUnRegister(eventHandle);
    eventHandle = &gUdmaCqEventObj;
    retVal += Udma_eventUnRegister(eventHandle);
    if(UDMA_SOK != retVal)
    {
        Udma3App_Print("[Error] UDMA event unregister failed!!\n");
    }

    retVal += Udma_chClose(chHandle);
    if(UDMA_SOK != retVal)
    {
        Udma3App_Print("[Error] UDMA channel close failed!!\n");
    }

    if(gUdmaAppDoneSem != NULL)
    {
        SemaphoreP_delete(gUdmaAppDoneSem);
        gUdmaAppDoneSem = NULL;
    }

    return (retVal);
}

static void Udma3App_UdmaTrpdInit(Udma_ChHandle chHandle,
                             uint8_t *pTrpdMem,
                             const void *srcBuf,
                             const void *destBuf,
                             uint32_t length)
{
    CSL_UdmapCppi5TRPD *pTrpd = (CSL_UdmapCppi5TRPD *) pTrpdMem;
    CSL_UdmapTR15 *pTr = (CSL_UdmapTR15 *)UdmaUtils_getTrpdTr1Pointer(pTrpdMem, 0U);
    uint32_t *pTrResp = (uint32_t *) (pTrpdMem + (sizeof(CSL_UdmapTR15) * 2U));
    uint32_t cqRingNum = Udma_chGetCqRingNum(chHandle);
    uint32_t cCnt;

    /* Make TRPD */
    UdmaUtils_makeTrpd(pTrpd, UDMA_TR_TYPE_15, 1U, cqRingNum);

    /* Setup TR */
    cCnt = 1;
    while ((length / cCnt) > 0x7FFFU)
    {
        cCnt = cCnt * 2;
    }
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
    pTr->dicnt0   = APP_CRC_PATTERN_SIZE;
    pTr->dicnt1   = (length / pTr->dicnt0) / cCnt;
    pTr->dicnt2   = cCnt;
    pTr->dicnt3   = 1U;
    pTr->ddim1    = 0U;
    pTr->ddim2    = 0U;
    pTr->ddim3    = 0U;
    pTr->daddr    = (uint64_t) Udma_appVirtToPhyFxn(destBuf, 0U, NULL);

    /* Clear TR response memory */
    *pTrResp = 0xFFFFFFFFU;

    /* Writeback cache */
    Udma_appUtilsCacheWb(pTrpdMem, UDMA_TEST_APP_TRPD_SIZE);

    return;
}

static void Udma3App_CrcInit(void)
{
    /* Configure CRC channel */
    CRCInitialize(
        APP_CRC_BASE,
        APP_CRC_CHANNEL,
        APP_CRC_WATCHDOG_PRELOAD_VAL,
        APP_CRC_BLOCK_PRELOAD_VAL);

    return;
}

static void Udma3App_Print(const char *str)
{
    UART_printf("%s", str);

    if(UTRUE == Udma_appIsPrintSupported())
    {
        printf("%s", str);
    }

    return;
}
