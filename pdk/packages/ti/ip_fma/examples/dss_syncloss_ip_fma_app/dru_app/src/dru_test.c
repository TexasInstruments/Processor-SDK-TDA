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
 *  \file dru_test.c
 *
 *  \brief DRU memcpy sample application performing block copy using
 *  Type 15 Transfer Record (TR15) using Transfer Record Packet
 *  Descriptor (TRPD).
 *
 *  This file contains the DRU application that is the part of the DSS8
 *  safety diagnostic implementation. The application initializes the
 *  MSMC DRU engine for DMA transaction, sets up the DRU channels and opens
 *  them. Then it performs DMA transfers, as many as possible in a loop. It
 *  fills up the Free Queue (FQ) until it is full, an then drains the
 *  completion queue (CQ) once the transfer are finished.
 *
 *  This DRU application is required to generate heavy DMA traffic that
 *  competes with DSS for shared memory and interconnect bandwidth.
 *  Sustained bus contention delays DSS frame buffer read transactions,
 *  which can cause underflow in the display pipeline.
 *  Repeated display underflow events lead to loss of video synchronization
 *  (SYNCLOST), which is the fault condition detected by the DSS8 safety
 *  mechanism.
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
#define UDMA_TEST_APP_NUM_BYTES         (4U * 1024U * 1024U)  // 4 MB
/** \brief This ensures every channel memory is aligned */
#define UDMA_TEST_APP_NUM_BYTES_ALIGN   ((UDMA_TEST_APP_NUM_BYTES + UDMA_CACHELINE_ALIGNMENT) & ~(UDMA_CACHELINE_ALIGNMENT - 1U))
/** \brief Number of times DRU memcpy operation is done for each DRU channel */
#define DRU_APP_RUN_COUNT       ((uint32_t)20000000)

/*
 * Ring parameters
 */
/** \brief Number of ring entries - we can prime this much memcpy operations */
#define UDMA_TEST_APP_RING_ENTRIES      (16384U)

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

/** Number of DRU channels */
#define NUM_DRU_CHANNELS        (2U)

/* Number of TR packet descriptors per channel */
#define TRPDS_PER_CH            (8192U)

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static int32_t DruApp_UdmaMemcpy(uint32_t chIdx,
                                     void *destBuf,
                                     void *srcBuf,
                                     uint64_t length);                

static int32_t DruApp_Init(Udma_DrvHandle drvHandle);
static int32_t DruApp_Deinit(Udma_DrvHandle drvHandle);

static int32_t DruApp_Create(Udma_DrvHandle drvHandle, uint32_t chIdx);
static int32_t DruApp_Delete(Udma_DrvHandle drvHandle, uint32_t chIdx);

static void DruApp_UdmaTrpdInit(Udma_ChHandle chHandle,
                                    uint8_t *pTrpdMem,
                                    const void *destBuf,
                                    const void *srcBuf,
                                    uint64_t length);

static void DruApp_Print(const char *str);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/*
 * UDMA driver objects
 */
struct Udma_DrvObj      gUdmaDrvObj;
struct Udma_ChObj       gUdmaChObj[NUM_DRU_CHANNELS];
struct Udma_EventObj    gUdmaCqEventObj[NUM_DRU_CHANNELS];
struct Udma_EventObj    gUdmaTdCqEventObj[NUM_DRU_CHANNELS];

/*
 * UDMA Memories
 */
static uint8_t gDruRingMem[NUM_DRU_CHANNELS][UDMA_TEST_APP_RING_MEM_SIZE_ALIGN] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
static uint8_t gDruCompRingMem[NUM_DRU_CHANNELS][UDMA_TEST_APP_RING_MEM_SIZE_ALIGN] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
static uint8_t gDruTdCompRingMem[NUM_DRU_CHANNELS][UDMA_TEST_APP_RING_MEM_SIZE_ALIGN] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
static uint8_t gUdmaTrpdMem[NUM_DRU_CHANNELS][TRPDS_PER_CH][UDMA_TEST_APP_TRPD_SIZE_ALIGN] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));

/*
 * Application Buffers
 */
static uint8_t gUdmaTestSrcBuf[NUM_DRU_CHANNELS][UDMA_TEST_APP_NUM_BYTES_ALIGN] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
static uint8_t gUdmaTestDestBuf[NUM_DRU_CHANNELS][UDMA_TEST_APP_NUM_BYTES_ALIGN] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));

/* Per channel circular submit index */
static uint16_t gTrpdSubmitIdx[NUM_DRU_CHANNELS] = {0};

/* Number of completed TRs */
static uint32_t gCqDoneCount[NUM_DRU_CHANNELS] = {0};
/* Increased when FQ is full but program is trying to fill another request to it */
static uint32_t gFqFullCount[NUM_DRU_CHANNELS] = {0};
/* Number of sucessful FQ attempts */
static uint32_t gSuccessQueue[NUM_DRU_CHANNELS] = {0};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/*
 * Application main
 */
int32_t DruApp_DruTest(void)
{
    int32_t         retVal;
    Udma_DrvHandle  drvHandle = &gUdmaDrvObj;

    DruApp_Print("UDMA - DRU application started...\n");

    retVal = DruApp_Init(drvHandle);
    if(UDMA_SOK != retVal)
    {
        DruApp_Print("[Error] DRU App init failed!!\n");
    }

    if(UDMA_SOK == retVal)
    {   
        for (uint8_t chIdx = 0; chIdx < NUM_DRU_CHANNELS; chIdx++)
        {
            retVal = DruApp_Create(drvHandle, chIdx);
            if(UDMA_SOK != retVal)
            {
                DruApp_Print("[Error] DRU App create failed!!\n");
            }
            else
            {
                UART_printf("[Success] Created DRU channel %d\n", chIdx);
            }
        }
    }

    uint64_t i = 0;
    uint8_t chIdx = 0;
    uint32_t loopCnt = 0;

    uint8_t *srcBuf[NUM_DRU_CHANNELS];
    uint8_t *destBuf[NUM_DRU_CHANNELS];

    for (chIdx = 0U; chIdx < NUM_DRU_CHANNELS; chIdx++)
    {
        srcBuf[chIdx]  = &gUdmaTestSrcBuf[chIdx][0U];
        destBuf[chIdx] = &gUdmaTestDestBuf[chIdx][0U];
    }

    for (chIdx = 0U; chIdx < NUM_DRU_CHANNELS; chIdx++)
    {
        /* Init buffers */
        for(i = 0U; i < UDMA_TEST_APP_NUM_BYTES; i++)
        {
            srcBuf[chIdx][i] = i;
            destBuf[chIdx][i] = 0U;
        }
    }
   
    while (loopCnt < DRU_APP_RUN_COUNT)
    {
        if (loopCnt % (DRU_APP_RUN_COUNT/10) == 0)
        {
            UART_printf("DRU iteration %d ...\n", loopCnt);
        }
        if(UDMA_SOK == retVal)
        {
            for (chIdx = 0; chIdx < NUM_DRU_CHANNELS; chIdx++)
            {
                /* Perform UDMA memcpy */
                retVal = DruApp_UdmaMemcpy(chIdx,
                                               destBuf[chIdx],
                                               srcBuf[chIdx],
                                               UDMA_TEST_APP_NUM_BYTES);
            }
        }

        for (chIdx = 0; chIdx < NUM_DRU_CHANNELS; chIdx++)
        {
            Udma_ChHandle chHandle = &gUdmaChObj[chIdx];
            uint64_t cqPtr;

            /* Drain all available completions without blocking */
            while (Udma_ringDequeueRaw(Udma_chGetCqRingHandle(chHandle), &cqPtr) == UDMA_SOK)
            {
                gCqDoneCount[chIdx]++;
            }
        }

        loopCnt++;
    }

    for (chIdx = 0; chIdx < NUM_DRU_CHANNELS; chIdx++)
    {
        Udma_ChHandle chHandle = &gUdmaChObj[chIdx];
        uint64_t cqPtr;
        uint32_t spin = 0;

        /* Drain until CQ completions catches up to the number of successful queues
        or until we hit a reasonable timeout to avoid infinite waits */
        while ((gCqDoneCount[chIdx] < gSuccessQueue[chIdx]) && (spin < 10000000U))
        {
            while (Udma_ringDequeueRaw(Udma_chGetCqRingHandle(chHandle), &cqPtr) == UDMA_SOK)
            {
                gCqDoneCount[chIdx]++;
            }
            spin++;
        }
    }

    for (chIdx = 0; chIdx < NUM_DRU_CHANNELS; chIdx++)
    {
        UART_printf("Channel %d\n", chIdx);
        UART_printf("--- FQ full attempts: %u, successful queues to FQ: %u, completed TRs: %u\n", gFqFullCount[chIdx], gSuccessQueue[chIdx], gCqDoneCount[chIdx]);
    }

    for (chIdx = 0; chIdx < NUM_DRU_CHANNELS; chIdx++)
    {
        retVal += DruApp_Delete(drvHandle, chIdx);
        if(UDMA_SOK != retVal)
        {
            DruApp_Print("[Error] DRU App delete failed!!\n");
        }
        else
        {
            UART_printf("[Success] Deleted DRU channel %d\n", chIdx);
        }
    }

    retVal += DruApp_Deinit(drvHandle);
    if(UDMA_SOK != retVal)
    {
        DruApp_Print("[Error] DRU App deinit failed!!\n");
    }

    if(UDMA_SOK == retVal)
    {
        DruApp_Print("DRU memcpy using TR15 block copy Passed!!\n");
        DruApp_Print("All tests have passed!!\n");
    }
    else
    {
        DruApp_Print("DRU memcpy using TR15 block copy Failed!!\n");
        DruApp_Print("Some tests have failed!!\n");
    }

    return retVal;
}

static int32_t DruApp_UdmaMemcpy(uint32_t chIdx,
                                     void *destBuf,
                                     void *srcBuf,
                                     uint64_t length)
{
    uint32_t retVal = UDMA_SOK;
    Udma_ChHandle chHandle = &gUdmaChObj[chIdx];

    /* Pick next TRPD in the circular pool */
    uint16_t idx = gTrpdSubmitIdx[chIdx];
    uint8_t *trpdMem = &gUdmaTrpdMem[chIdx][idx][0U];

    /* Update TR packet descriptor */
    DruApp_UdmaTrpdInit(chHandle, trpdMem, destBuf, srcBuf, length);

    /* Submit TRPD to channel */
    retVal = Udma_ringQueueRaw(Udma_chGetFqRingHandle(chHandle), (uint64_t) trpdMem);

    if (retVal == UDMA_SOK)
    {
        /* Advance circular index only on successful queue */
        gTrpdSubmitIdx[chIdx] = (uint16_t)((idx + 1U) % TRPDS_PER_CH);

        gSuccessQueue[chIdx]++;
    }
    else
    {
        /* FQ full right now – normal under load; increment counter and continue */
        gFqFullCount[chIdx]++;
    }

    /* Return immediately — CPU is free to continue */
    return UDMA_SOK;
}

static int32_t DruApp_Init(Udma_DrvHandle drvHandle)
{
    int32_t             retVal;
    Udma_InitPrms       initPrms;
    uint32_t            instId;
    uint32_t            utcId;
    uint32_t            numQueue, queId;
    CSL_DruQueueConfig  queueCfg;

    /* Note: There is no external channel support in MCU NAVSS. So always use
     * main NAVSS even for MCU builds */
    /* UDMA driver init */
    instId = UDMA_INST_ID_MAIN_0;
    UdmaInitPrms_init(instId, &initPrms);
    initPrms.printFxn = &DruApp_Print;
    retVal = Udma_init(drvHandle, &initPrms);
    if(UDMA_SOK != retVal)
    {
        DruApp_Print("[Error] UDMA init failed!!\n");
    }

    /* Init all DRU queue */
    utcId = UDMA_UTC_ID_MSMC_DRU0;
    numQueue = Udma_druGetNumQueue(drvHandle, utcId);
    if(0U == numQueue)
    {
        DruApp_Print("[Error] Invalid queue number!!\n");
    }
    UdmaDruQueueConfig_init(&queueCfg);
    for(queId = CSL_DRU_QUEUE_ID_0; queId < numQueue; queId++)
    {
        retVal = Udma_druQueueConfig(drvHandle, utcId, queId, &queueCfg);
        if(UDMA_SOK != retVal)
        {
            DruApp_Print("[Error] DRU queue config failed!!\n");
            break;
        }
    }

    return (retVal);
}

static int32_t DruApp_Deinit(Udma_DrvHandle drvHandle)
{
    int32_t             retVal;

    retVal = Udma_deinit(drvHandle);
    if(UDMA_SOK != retVal)
    {
        DruApp_Print("[Error] UDMA deinit failed!!\n");
    }

    return (retVal);
}

static int32_t DruApp_Create(Udma_DrvHandle drvHandle, uint32_t chIdx)
{

    int32_t             retVal = UDMA_SOK;
    uint32_t            chType;
    Udma_ChPrms         chPrms;
    Udma_ChHandle chHandle  = &gUdmaChObj[chIdx];
    Udma_ChUtcPrms      utcPrms;
    
    if(UDMA_SOK == retVal)
    {
        /* Init channel parameters */
        chType = UDMA_CH_TYPE_UTC;
        UdmaChPrms_init(&chPrms, chType);
        chPrms.utcId                = UDMA_UTC_ID_MSMC_DRU0;
        chPrms.fqRingPrms.ringMem   = &gDruRingMem[chIdx][0U];
        chPrms.cqRingPrms.ringMem   = &gDruCompRingMem[chIdx][0U];
        chPrms.tdCqRingPrms.ringMem = &gDruTdCompRingMem[chIdx][0U];
        chPrms.fqRingPrms.ringMemSize   = UDMA_TEST_APP_RING_MEM_SIZE;
        chPrms.cqRingPrms.ringMemSize   = UDMA_TEST_APP_RING_MEM_SIZE;
        chPrms.tdCqRingPrms.ringMemSize = UDMA_TEST_APP_RING_MEM_SIZE;
        chPrms.fqRingPrms.elemCnt   = UDMA_TEST_APP_RING_ENTRIES;
        chPrms.cqRingPrms.elemCnt   = UDMA_TEST_APP_RING_ENTRIES;
        chPrms.tdCqRingPrms.elemCnt = UDMA_TEST_APP_RING_ENTRIES;

        /* Open channel for DRU */
        retVal = Udma_chOpen(drvHandle, chHandle, chType, &chPrms);
        if(UDMA_SOK != retVal)
        {
            DruApp_Print("[Error] UDMA channel open failed!!\n");
        }
    }

    if(UDMA_SOK == retVal)
    {
        /* Config UTC channel */
        UdmaChUtcPrms_init(&utcPrms);
        utcPrms.druOwner    = CSL_DRU_OWNER_UDMAC_TR;
        if (chIdx == 0)
        {
            utcPrms.druQueueId  = CSL_DRU_QUEUE_ID_1;
        } 
        else if (chIdx == 1)
        {
            utcPrms.druQueueId = CSL_DRU_QUEUE_ID_2;
        }

        retVal = Udma_chConfigUtc(chHandle, &utcPrms);
        if(UDMA_SOK != retVal)
        {
            DruApp_Print("[Error] UDMA UTC channel config failed!!\n");
        }
    }

    if(UDMA_SOK == retVal)
    {
        /* Channel enable */
        retVal = Udma_chEnable(chHandle);
        if(UDMA_SOK != retVal)
        {
            DruApp_Print("[Error] UDMA channel enable failed!!\n");
        }
    }

    return (retVal);
}


static int32_t DruApp_Delete(Udma_DrvHandle drvHandle, uint32_t chIdx)
{
    int32_t             retVal, tempRetVal;
    uint64_t            pDesc;

    Udma_ChHandle chHandle = &gUdmaChObj[chIdx];

    retVal = Udma_chDisable(chHandle, UDMA_DEFAULT_CH_DISABLE_TIMEOUT);
    if(UDMA_SOK != retVal)
    {
        DruApp_Print("[Error] UDMA channel disable failed!!\n");
    }

    /* Flush any pending request from the free queue */
    while(1)
    {
        tempRetVal = Udma_ringFlushRaw(Udma_chGetFqRingHandle(chHandle), &pDesc);
        if(UDMA_ETIMEOUT == tempRetVal)
        {
            break;
        }
    }

    retVal += Udma_chClose(chHandle);
    if(UDMA_SOK != retVal)
    {
        DruApp_Print("[Error] UDMA channel close failed!!\n");
    }

    return (retVal);
}

static void DruApp_UdmaTrpdInit(Udma_ChHandle chHandle,
                                    uint8_t *pTrpdMem,
                                    const void *destBuf,
                                    const void *srcBuf,
                                    uint64_t length)
{
    CSL_UdmapCppi5TRPD *pTrpd = (CSL_UdmapCppi5TRPD *) pTrpdMem;
    CSL_UdmapTR15 *pTr = (CSL_UdmapTR15 *)(pTrpdMem + sizeof(CSL_UdmapTR15));
    uint32_t *pTrResp = (uint32_t *) (pTrpdMem + (sizeof(CSL_UdmapTR15) * 2U));
    uint32_t cqRingNum = Udma_chGetCqRingNum(chHandle);

    /* Make TRPD */
    UdmaUtils_makeTrpd(pTrpd, UDMA_TR_TYPE_9, 1U, cqRingNum);

    /* Setup TR */
    pTr->flags    = CSL_FMK(UDMAP_TR_FLAGS_TYPE, CSL_UDMAP_TR_FLAGS_TYPE_4D_BLOCK_MOVE)         |
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
    
    const uint32_t lineSize = 256;                  // 256 B
    const uint32_t linesPerRow = 16;                // 16 * 256 = 4096 = 4 KB
    const uint32_t rows = length / (lineSize * linesPerRow);

    pTr->icnt0 = lineSize;          // 256 B
    pTr->icnt1 = linesPerRow;       // 16
    pTr->icnt2 = rows;              
    pTr->icnt3 = 1;

    pTr->dim1  = pTr->icnt0;                      // next 256 B
    pTr->dim2  = pTr->icnt0 * pTr->icnt1;         // 4 KB
    pTr->dim3  = pTr->dim2;

    pTr->addr     = (uint64_t) srcBuf;
    pTr->fmtflags = 0x00000000U;        /* Linear addressing, 1 byte per elem.
                                           Replace with CSL-FL API */

    pTr->dicnt0 = pTr->icnt0;
    pTr->dicnt1 = pTr->icnt1;
    pTr->dicnt2 = 1;
    pTr->dicnt3 = 1;

    pTr->ddim1 = pTr->dim1;
    pTr->ddim2 = pTr->dim2;
    pTr->ddim3 = pTr->dim3;

    pTr->daddr    = (uint64_t) destBuf;

    /* Clear TR response memory */
    *pTrResp = 0xFFFFFFFFU;

    /* Writeback cache */
    Udma_appUtilsCacheWb(pTrpdMem, UDMA_TEST_APP_TRPD_SIZE);

    return;
}

static void DruApp_Print(const char *str)
{
    UART_printf("%s", str);

    if(UTRUE == Udma_appIsPrintSupported())
    {
        printf("%s", str);
    }

    return;
}
