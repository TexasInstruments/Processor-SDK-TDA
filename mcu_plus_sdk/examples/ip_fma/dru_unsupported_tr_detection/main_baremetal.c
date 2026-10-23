/*
 * Copyright (c) 2026 Texas Instruments Incorporated
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
 *  \file main_baremetal.c
 *
 *  \brief UTC unsupported-TR check for VPAC0 on J722S (R5F).
 *
 *  Intentionally submits one transfer request with an unsupported TR type
 *  (3D_DATA_MOVE) and verifies the hardware TR response.
 *
 *  On the main R5F the DRU (MSMC_DRU0) is reached through BCDMA_0's UTC path.
 *  That path does NOT expose a NavSS ring; the DRU has its own per-channel ring
 *  in its CHNRT/CHRT MMRs, driven directly via the Udma_chRing*Raw family:
 *    Udma_chRingQueueRaw()  -> forward ring, Udma_chRingRingDbRaw() -> submit,
 *    CHRT[ch].RING_RCVS_OCC -> poll, Udma_chRingDeQueueRaw()/RvrDbRaw() -> pop.
 *  The channel is opened DIRECT_TR (config succeeds on R5F) and its owner bit
 *  is then forced to UDMAC_TR by direct MMR while disabled, since the DRU only
 *  consumes its forward ring in that mode (see AppDru_openDruChannel).
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/SystemP.h>
#include <kernel/dpl/CacheP.h>
#include <drivers/udma.h>
#include <drivers/soc.h>
#include <drivers/sciclient.h>
#include <drivers/hw_include/cslr64.h>
#include "ti_drivers_config.h"
#include "ti_board_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/**< UDMA instance carrying the VPAC0 DRU UTC instance */
#define APP_DRU_UDMA_INST_ID            (UDMA_INST_ID_BCDMA_0)
/**< UTC instance under test - VPAC0 DRU on J722S non-C7x builds */
#define APP_DRU_UTC_ID                  (UDMA_UTC_ID_MSMC_DRU0)
/**< Device that must be clocked before the DRU MMRs can be accessed */
#define APP_DRU_TISCI_DEV_ID            (TISCI_DEV_VPAC0)
/**< CMD_ID stamped into the TR; it comes back in the TR response */
#define APP_TR_CMD_ID                   ((uint8_t) 0x25U)
/**< Number of bytes moved by the (legal fields of the) test TR */
#define APP_TR_NUM_BYTES                (1000U)
/**< Bounded poll limit while waiting for the DRU reverse-ring response */
#define APP_TR_RESP_TIMEOUT             (1000000U)

/**< The illegal TR type under test */
#define APP_ILLEGAL_TR_TYPE             (CSL_UDMAP_TR_FLAGS_TYPE_3D_DATA_MOVE)

/**< DRU exposed ring: holds 64-bit descriptor (TRPD) pointers */
#define APP_RING_ELEM_CNT               (8U)
#define APP_RING_MEM_SIZE               (APP_RING_ELEM_CNT * sizeof(uint64_t))
#define APP_ALIGN(x)                    (((x) + UDMA_CACHELINE_ALIGNMENT) & ~(UDMA_CACHELINE_ALIGNMENT - 1U))
#define APP_RING_MEM_SIZE_ALIGN         (APP_ALIGN(APP_RING_MEM_SIZE))
#define APP_NUM_BYTES_ALIGN             (APP_ALIGN(APP_TR_NUM_BYTES))

/**
 *  \brief TRPD memory: CSL_UdmapCppi5TRPD (padded to sizeof(CSL_UdmapTR15)) +
 *  one Type-15 TR + a 4-byte TR response.
 */
#define APP_TRPD_SIZE                   ((sizeof(CSL_UdmapTR15) * 2U) + 4U)
#define APP_TRPD_SIZE_ALIGN             (APP_ALIGN(APP_TRPD_SIZE))

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static int32_t AppDru_enableModuleClock(void);
static void    App_print(const char *str);
static void    App_printDruCapabilities(const CSL_DRU_t *pDruRegs);
static void    App_buildTr(CSL_UdmapTR15 *pTr, const void *srcBuf,
                           const void *dstBuf, uint32_t length);
static int32_t AppDru_openDruChannel(Udma_DrvHandle drvHandle,
                                     Udma_ChHandle chHandle, uint32_t *pChNum);
static void    AppDru_closeDruChannel(Udma_ChHandle chHandle, int32_t *pRetVal);
static int32_t App_submitUnsupportedTr(Udma_ChHandle chHandle,
                                       const CSL_DRU_t *pDruRegs, uint32_t chNum);
static const char *App_trStatusToStr(uint32_t status);
static const char *App_trUnsupportedReasonToStr(uint32_t reason);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

Udma_DrvObject gUdmaDrvObj;
Udma_ChObject  gUdmaChObj;

/**< DRU exposed-ring memory (holds TRPD pointers) and the TRPD itself */
static uint8_t gDruRingMem[APP_RING_MEM_SIZE_ALIGN] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
static uint8_t gUdmaTrpdMem[APP_TRPD_SIZE_ALIGN]    __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));

/**< Application Buffers */
static uint8_t gUdmaTestSrcBuf[APP_NUM_BYTES_ALIGN]  __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
static uint8_t gUdmaTestDestBuf[APP_NUM_BYTES_ALIGN] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int main(void)
{
    int32_t             status  = SystemP_SUCCESS;
    int32_t             retVal  = UDMA_SOK;
    int32_t             tempRetVal;
    Udma_DrvHandle      drvHandle = &gUdmaDrvObj;
    Udma_ChHandle       chHandle  = &gUdmaChObj;
    const CSL_DRU_t    *pDruRegs  = (const CSL_DRU_t *) UDMA_UTC_BASE_DRU0;
    Udma_InitPrms       initPrms;
    uint32_t            chNum = 0U;

    uint32_t drvInitDone  = 0U;
    uint32_t chOpenDone   = 0U;

    System_init();
    Board_init();

    status = Board_driversOpen();
    DebugP_assert(status == SystemP_SUCCESS);
    Drivers_open();

    App_print("UTC Unsupported TR Check application started...\r\n");

    /* VPAC0 must be clocked before the DRU MMRs are accessible. */
    retVal = AppDru_enableModuleClock();
    if (UDMA_SOK != retVal)
    {
        goto cleanup;
    }

    UdmaInitPrms_init(APP_DRU_UDMA_INST_ID, &initPrms);
    initPrms.enableUtc = (uint8_t) TRUE;
    retVal = Udma_init(drvHandle, &initPrms);
    if (UDMA_SOK != retVal)
    {
        App_print("[Error] UDMA init failed!!\r\n");
        goto cleanup;
    }
    drvInitDone = 1U;

    retVal = AppDru_openDruChannel(drvHandle, chHandle, &chNum);
    if (UDMA_SOK != retVal)
    {
        goto cleanup;
    }
    chOpenDone = 1U;

    retVal = App_submitUnsupportedTr(chHandle, pDruRegs, chNum);
    if (UDMA_SOK != retVal)
    {
        App_print("[Error] Unsupported TR test failed!!\r\n");
        goto cleanup;
    }

cleanup:
    if (chOpenDone == 1U)
    {
        AppDru_closeDruChannel(chHandle, &retVal);
    }

    if (drvInitDone == 1U)
    {
        tempRetVal = Udma_deinit(drvHandle);
        if ((UDMA_SOK != tempRetVal) && (UDMA_SOK == retVal))
        {
            App_print("[Error] UDMA deinit failed!!\r\n");
            retVal = tempRetVal;
        }
    }

    App_print("UTC Unsupported TR Check application done\r\n");

    if (UDMA_SOK == retVal)
    {
        App_print("All tests have passed!!\r\n");
    }
    else
    {
        App_print("Test failed!!\r\n");
    }

    Board_driversClose();
    Drivers_close();

    Board_deinit();
    System_deinit();

    return (retVal);
}

static int32_t AppDru_enableModuleClock(void)
{
    int32_t retVal = UDMA_SOK;
    int32_t status;

    status = SOC_moduleClockEnable(APP_DRU_TISCI_DEV_ID, 1U);
    if(SystemP_SUCCESS != status)
    {
        App_print("[Error] DRU host module clock enable failed!!\r\n");
        retVal = UDMA_EFAIL;
    }

    return (retVal);
}

static void App_print(const char *str)
{
    DebugP_log("%s", str);
    return;
}

static void CSL_druPrintCapabilities(const CSL_DruCapabilities *caps)
{
    uint32_t i;

    if (caps == NULL)
    {
        DebugP_log("DRU capabilities: NULL pointer\r\n");
        return;
    }

    DebugP_log("==== DRU CAPABILITIES ====\r\n");

    /* TR Types */
    DebugP_log("Supported TR Types: ");
    for (i = 0; i < 16; i++)
    {
        if (caps->trTypeSupported & (1U << i))
        {
            DebugP_log("%u ", i);
        }
    }
    DebugP_log("\r\n");

    /* Optional features */
    DebugP_log("STATIC support       : %s\r\n",
               caps->staticSupported ? "YES" : "NO");
    DebugP_log("EOL support          : %s\r\n",
               caps->eolSupported ? "YES" : "NO");
    DebugP_log("Local trigger        : %s\r\n",
               caps->localTriggerSupported ? "YES" : "NO");
    DebugP_log("Global trigger       : %s\r\n",
               caps->globalTriggerSupported ? "YES" : "NO");

    /* AMODE */
    DebugP_log("Supported AMODEs     : ");
    if (caps->amode == 0U)
    {
        DebugP_log("NONE");
    }
    else
    {
        for (i = 0; i < 8; i++)
        {
            if (caps->amode & (1U << i))
            {
                DebugP_log("%u ", i);
            }
        }
    }
    DebugP_log("\r\n");

    /* ELTYPE */
    DebugP_log("Supported ELTYPEs    : ");
    if (caps->elType == 0U)
    {
        DebugP_log("NONE");
    }
    else
    {
        for (i = 0; i < 8; i++)
        {
            if (caps->elType & (1U << i))
            {
                DebugP_log("%u ", i);
            }
        }
    }
    DebugP_log("\r\n");

    /* DFMT */
    DebugP_log("Supported DFMTs      : ");
    if (caps->dfmt == 0U)
    {
        DebugP_log("NONE");
    }
    else
    {
        for (i = 0; i < 8; i++)
        {
            if (caps->dfmt & (1U << i))
            {
                DebugP_log("%u ", i);
            }
        }
    }
    DebugP_log("\r\n");

    /* SECTR */
    DebugP_log("Secure transfer      : %s\r\n",
               caps->sectr ? "YES" : "NO");

    DebugP_log("==========================\r\n");
}

static void App_printDruCapabilities(const CSL_DRU_t *pDruRegs)
{
    CSL_DruCapabilities caps;

    if (CSL_druGetCapabilities(pDruRegs, &caps) != CSL_PASS)
    {
        DebugP_log("CSL_druGetCapabilities failed\r\n");
        return;
    }

    DebugP_log("DRU Capabilities:\r\n");
    CSL_druPrintCapabilities(&caps);
}

static void App_buildTr(CSL_UdmapTR15 *pTr, const void *srcBuf,
                        const void *dstBuf, uint32_t length)
{
    /*
     * Intentionally program an unsupported TR type (3D Data Move).  The DRU
     * does not advertise support for it in its capabilities register, so the
     * TR is expected to be rejected with:
     *   STATUS_TYPE = CSL_UDMAP_TR_RESPONSE_STATUS_UNSUPPORTED_ERR
     *   STATUS_INFO = CSL_UDMAP_TR_RESPONSE_STATUS_UNSUPPORTED_TR_TYPE
     */
    pTr->flags    = CSL_FMK(UDMAP_TR_FLAGS_TYPE, APP_ILLEGAL_TR_TYPE)                    |
                    CSL_FMK(UDMAP_TR_FLAGS_STATIC, 0U)                                    |
                    CSL_FMK(UDMAP_TR_FLAGS_EOL, 0U)                                       |
                    CSL_FMK(UDMAP_TR_FLAGS_EVENT_SIZE, CSL_UDMAP_TR_FLAGS_EVENT_SIZE_COMPLETION) |
                    CSL_FMK(UDMAP_TR_FLAGS_TRIGGER0, CSL_UDMAP_TR_FLAGS_TRIGGER_NONE)     |
                    CSL_FMK(UDMAP_TR_FLAGS_TRIGGER0_TYPE, CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ALL) |
                    CSL_FMK(UDMAP_TR_FLAGS_TRIGGER1, CSL_UDMAP_TR_FLAGS_TRIGGER_NONE)     |
                    CSL_FMK(UDMAP_TR_FLAGS_TRIGGER1_TYPE, CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ALL) |
                    CSL_FMK(UDMAP_TR_FLAGS_CMD_ID, (uint32_t) APP_TR_CMD_ID)              |
                    CSL_FMK(UDMAP_TR_FLAGS_SA_INDIRECT, 0U)                               |
                    CSL_FMK(UDMAP_TR_FLAGS_DA_INDIRECT, 0U)                               |
                    CSL_FMK(UDMAP_TR_FLAGS_EOP, 1U);
    pTr->icnt0    = (uint16_t) length;
    pTr->icnt1    = 1U;
    pTr->icnt2    = 1U;
    pTr->icnt3    = 1U;
    pTr->dim1     = (int32_t) pTr->icnt0;
    pTr->dim2     = (int32_t) ((uint32_t) pTr->icnt0 * pTr->icnt1);
    pTr->dim3     = (int32_t) ((uint32_t) pTr->icnt0 * pTr->icnt1 * pTr->icnt2);
    pTr->addr     = (uint64_t) Udma_defaultVirtToPhyFxn(srcBuf, 0U, NULL);
    pTr->fmtflags = 0x00000000U;    /* Linear addressing, 1 byte per elem. */
    pTr->dicnt0   = (uint16_t) length;
    pTr->dicnt1   = 1U;
    pTr->dicnt2   = 1U;
    pTr->dicnt3   = 1U;
    pTr->ddim1    = (int32_t) pTr->dicnt0;
    pTr->ddim2    = (int32_t) ((uint32_t) pTr->dicnt0 * pTr->dicnt1);
    pTr->ddim3    = (int32_t) ((uint32_t) pTr->dicnt0 * pTr->dicnt1 * pTr->dicnt2);
    pTr->daddr    = (uint64_t) Udma_defaultVirtToPhyFxn(dstBuf, 0U, NULL);

    return;
}

static int32_t AppDru_openDruChannel(Udma_DrvHandle drvHandle,
                                     Udma_ChHandle chHandle, uint32_t *pChNum)
{
    int32_t         retVal = UDMA_SOK;
    uint32_t        chType;
    uint32_t        chNum = 0U;
    Udma_ChPrms     chPrms;
    Udma_ChUtcPrms  utcPrms;

    chType = UDMA_CH_TYPE_UTC;
    UdmaChPrms_init(&chPrms, chType);
    chPrms.utcId = APP_DRU_UTC_ID;
    /* Providing fqRingPrms.ringMem makes chOpen configure the DRU's own ring
     * (CHRING_ADDR) via Udma_utcChRingCfg. */
    chPrms.fqRingPrms.ringMem     = &gDruRingMem[0U];
    chPrms.fqRingPrms.ringMemSize = APP_RING_MEM_SIZE;
    chPrms.fqRingPrms.elemCnt     = APP_RING_ELEM_CNT;

    retVal = Udma_chOpen(drvHandle, chHandle, chType, &chPrms);
    if (UDMA_SOK != retVal)
    {
        App_print("[Error] UDMA channel open failed!!\r\n");
    }
    else
    {
        chNum = Udma_chGetNum(chHandle);
    }

    if (UDMA_SOK == retVal)
    {
        /* DIRECT_TR so Udma_chConfigUtc succeeds on the R5F (no SYSFW TX-ch
         * config). It leaves the channel disabled. */
        UdmaChUtcPrms_init(&utcPrms);
        utcPrms.druOwner   = CSL_DRU_OWNER_DIRECT_TR;
        utcPrms.druQueueId = (CSL_DRU_QUEUE_ID_3 < CSL_DRU_NUM_QUEUE) ?
                             CSL_DRU_QUEUE_ID_3 : (CSL_DRU_NUM_QUEUE - 1U);
        retVal = Udma_chConfigUtc(chHandle, &utcPrms);
        if (UDMA_SOK != retVal)
        {
            App_print("[Error] UDMA UTC channel config failed!!\r\n");
        }
    }

    if (UDMA_SOK == retVal)
    {
        /* The DRU consumes its CHNRT forward ring only in UDMAC_TR owner mode.
         * The DRU-side owner bit is plain MMR; set it directly while the channel
         * is still disabled (CHNRT_CFG rejects writes when enabled). The ring
         * was already configured by chOpen. */
        CSL_DRU_t *pDruW = (CSL_DRU_t *) UDMA_UTC_BASE_DRU0;
        uint64_t   cfg;

        cfg  = CSL_REG64_RD(&pDruW->CHNRT[chNum].CFG);
        cfg |= (uint64_t) CSL_DRU_CHNRT_CFG_CHAN_TYPE_OWNER_MASK;
        CSL_REG64_WR(&pDruW->CHNRT[chNum].CFG, cfg);
    }

    if (UDMA_SOK == retVal)
    {
        retVal = Udma_chEnable(chHandle);
        if (UDMA_SOK != retVal)
        {
            App_print("[Error] UDMA channel enable failed!!\r\n");
        }
        else
        {
            *pChNum = chNum;
        }
    }

    return (retVal);
}

static void AppDru_closeDruChannel(Udma_ChHandle chHandle, int32_t *pRetVal)
{
    int32_t status;

    status = Udma_chDisable(chHandle, UDMA_DEFAULT_CH_DISABLE_TIMEOUT);
    if ((UDMA_SOK != status) && (UDMA_SOK == *pRetVal))
    {
        App_print("[Error] UDMA channel disable failed!!\r\n");
        *pRetVal = status;
    }

    status = Udma_chClose(chHandle);
    if ((UDMA_SOK != status) && (UDMA_SOK == *pRetVal))
    {
        App_print("[Error] UDMA channel close failed!!\r\n");
        *pRetVal = status;
    }

    return;
}

static int32_t App_submitUnsupportedTr(Udma_ChHandle chHandle,
                                       const CSL_DRU_t *pDruRegs, uint32_t chNum)
{
    int32_t         retVal = UDMA_SOK;
    uint32_t        *pTrResp;
    uint32_t        trRespStatus;
    uint32_t        i;
    uint8_t         *srcBuf = &gUdmaTestSrcBuf[0U];
    uint8_t         *destBuf = &gUdmaTestDestBuf[0U];
    uint8_t         *trpdMem = &gUdmaTrpdMem[0U];
    CSL_UdmapTR15   *pTr;
    uint32_t        cqRingNum;
    uint64_t        trpdPhys;
    uint64_t        respElem = 0U;
    uint64_t        occ = 0U;
    uint32_t        timeoutCnt;

    /* Init buffers */
    for (i = 0U; i < APP_TR_NUM_BYTES; i++)
    {
        srcBuf[i]  = (uint8_t) i;
        destBuf[i] = 0U;
    }
    CacheP_wb(srcBuf,  APP_TR_NUM_BYTES, CacheP_TYPE_ALL);
    CacheP_wb(destBuf, APP_TR_NUM_BYTES, CacheP_TYPE_ALL);

    /* Update TR packet descriptor */
    pTr       = UdmaUtils_getTrpdTr15Pointer(trpdMem, 0U);
    pTrResp   = (uint32_t *) (trpdMem + (sizeof(CSL_UdmapTR15) * 2U));
    cqRingNum = Udma_chGetCqRingNum(chHandle);

    UdmaUtils_makeTrpd(trpdMem, UDMA_TR_TYPE_9, 1U, cqRingNum);
    App_buildTr(pTr, srcBuf, destBuf, APP_TR_NUM_BYTES);

    /* Clear TR response memory */
    *pTrResp = 0xFFFFFFFFU;

    /* Writeback cache */
    CacheP_wb(trpdMem, APP_TRPD_SIZE, CacheP_TYPE_ALL);

    trpdPhys = (uint64_t) Udma_defaultVirtToPhyFxn(trpdMem, 0U, NULL);

    /* Submit TRPD to the DRU forward ring and ring the forward doorbell. */
    retVal = Udma_chRingQueueRaw(chHandle, (uint8_t *)(uintptr_t) trpdPhys, 1U);
    if (UDMA_SOK != retVal)
    {
        App_print("[Error] Channel queue failed!!\r\n");
    }

    if (UDMA_SOK == retVal)
    {
        retVal = Udma_chRingRingDbRaw(chHandle, 1U);
        if (UDMA_SOK != retVal)
        {
            App_print("[Error] Channel doorbell failed!!\r\n");
        }
    }

    if (UDMA_SOK == retVal)
    {
        /* Wait for the DRU to write the response to its reverse ring. */
        timeoutCnt = APP_TR_RESP_TIMEOUT;
        do
        {
            occ = CSL_REG64_RD(&pDruRegs->CHRT[chNum].RING_RCVS_OCC);
            timeoutCnt--;
        } while ((((occ & CSL_DRU_CHRT_CHRT_RING_RCVS_OCC_CNT_MASK) == 0ULL) &&
                  ((occ & CSL_DRU_CHRT_CHRT_RING_RCVS_OCC_COMP_MASK) == 0ULL)) &&
                 (timeoutCnt > 0U));

        if (((occ & CSL_DRU_CHRT_CHRT_RING_RCVS_OCC_CNT_MASK) == 0ULL) &&
            ((occ & CSL_DRU_CHRT_CHRT_RING_RCVS_OCC_COMP_MASK) == 0ULL))
        {
            App_print("[Error] No descriptor after submission!!\r\n");
            retVal = UDMA_EFAIL;
        }
        else
        {
            (void) Udma_chRingDeQueueRaw(chHandle, 1U, &respElem);
            (void) Udma_chRingRingRvrDbRaw(chHandle, 1U);

            /* Check returned descriptor pointer */
            if (respElem != trpdPhys)
            {
                App_print("[Error] TR descriptor pointer returned doesn't "
                          "match the submitted address!!\r\n");
                retVal = UDMA_EFAIL;
            }
        }
    }

    if (UDMA_SOK == retVal)
    {
        /* Invalidate cache */
        CacheP_inv(trpdMem, APP_TRPD_SIZE, CacheP_TYPE_ALL);

        /* check TR response status */
        pTrResp = (uint32_t *) (trpdMem + (sizeof(CSL_UdmapTR15) * 2U));
        trRespStatus = CSL_FEXT(*pTrResp, UDMAP_TR_RESPONSE_STATUS_TYPE);

        App_printDruCapabilities(pDruRegs);

        if (trRespStatus == CSL_UDMAP_TR_RESPONSE_STATUS_COMPLETE)
        {
            DebugP_log("[Error] Invalid TR should NOT complete successfully!\r\n");
            retVal = UDMA_EFAIL;
        }
        else if (trRespStatus == CSL_UDMAP_TR_RESPONSE_STATUS_UNSUPPORTED_ERR)
        {
            uint32_t unsupportedReason =
                CSL_FEXT(*pTrResp, UDMAP_TR_RESPONSE_STATUS_INFO);

            DebugP_log("TR Status: %s (%u)\r\n",
                       App_trStatusToStr(trRespStatus), trRespStatus);
            DebugP_log("Unsupported reason: %s (%u)\r\n",
                       App_trUnsupportedReasonToStr(unsupportedReason),
                       unsupportedReason);

            /* This is EXPECTED behavior */
            retVal = UDMA_SOK;
        }
        else
        {
            DebugP_log("[Error] Unexpected TR status: %s (%u)\r\n",
                       App_trStatusToStr(trRespStatus), trRespStatus);
            retVal = UDMA_EFAIL;
        }
    }

    return (retVal);
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
