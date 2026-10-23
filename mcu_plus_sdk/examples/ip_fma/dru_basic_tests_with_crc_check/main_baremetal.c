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
 *  \brief DRU basic functionality test with information redundancy check.
 *
 *  Software test of the basic functionalities of the DRU (Data Routing Unit)
 *  combined with an information redundancy technique applied to the moved
 *  data. This application drives the DRU inside the VPAC0 module from
 *  main-r5fss0-0. Of the four DRU instances of this device, the two C7x local
 *  ones are reachable only from the C7x that owns them, and the DMPAC0 one
 *  answers reads but did not accept MMR writes from an R5F when this family
 *  of examples was brought up, so VPAC0 is the instance covered here. The
 *  DRU is exposed by the UDMA driver as a UTC instance of BCDMA_0 and is fed
 *  through its own per channel ring, which is the only TR submission path
 *  available to an R5F context.
 *
 *  Basic functionality and address line coverage:
 *      - A buffer is moved between memory locations chosen so that every
 *        address line is driven both low and high. The DDR window of this
 *        device is 0x8000_0000 to 0xFFFF_FFFF, so 0x5555_5555 is not a DDR
 *        address and 0x9555_5555 takes its place: it keeps the alternating
 *        0x5555_5555 pattern on address lines 0 to 29 and has bit 31 set.
 *        Paired with 0xAAAA_AAAA it drives lines 0 to 29 both ways
 *        (0x9555_5555 ^ 0xAAAA_AAAA = 0x3FFF_FFFF), but not lines 30 and 31,
 *        which both addresses hold at 0 and 1 respectively. Those two lines
 *        are driven by a second pair of transfers against a buffer in the
 *        MCU MSRAM (0x9555_5555 ^ 0x7910_2A00 = 0xEC45_7F55). The two pairs
 *        together cover all 32 address lines.
 *      - Each pair is transferred in both directions: the destination of a
 *        transfer becomes the source of the next one, and every destination
 *        buffer is cleared before its transfer, so data really has to be
 *        moved back and forth by the DRU.
 *      - The transfer size is twice the 64 byte burst size of the DRU.
 *      - Every destination buffer is followed by a guard region filled with
 *        a known pattern, which is verified after the transfer to prove that
 *        the DRU did not write past the programmed length. The region behind
 *        the data of a source buffer carries a different pattern, so that a
 *        transfer running past its length moves a value into the guard region
 *        that the check can tell apart from the guard pattern itself.
 *
 *  Information redundancy:
 *      - A CRC-32 signature is computed over the payload and embedded in the
 *        transferred buffer, right after the payload.
 *      - On the destination side, the payload and the embedded signature are
 *        unpacked, the signature is recomputed from the received payload and
 *        compared against the embedded one, for every transfer.
 *
 *  The addresses used for the test buffers must not be assigned to any other
 *  software in the system, the application writes to them directly.
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
/**< UTC instance under test - VPAC0 DRU on J722S non C7x builds */
#define APP_DRU_UTC_ID                  (UDMA_UTC_ID_MSMC_DRU0)
/**< Device that must be clocked before the DRU MMRs can be accessed */
#define APP_DRU_TISCI_DEV_ID            (TISCI_DEV_VPAC0)
/**< DRU scheduling queue used by the test channel */
#define APP_DRU_QUEUE_ID                (CSL_DRU_QUEUE_ID_0)

/**< DRU queue configuration used by the test channel */
#define APP_DRU_QUEUE_PRIORITY          ((uint64_t) 4U)
#define APP_DRU_QUEUE_QOS               ((uint64_t) 4U)
#define APP_DRU_QUEUE_ORDER_ID          ((uint64_t) 0U)
#define APP_DRU_QUEUE_CONSEC_TRANS      ((uint64_t) 0xFU)
#define APP_DRU_QUEUE_REARB_WAIT        ((uint64_t) 0xFU)

/**< TR type used for the block copy - the DRU supports TR8 to TR11 only */
#define APP_TR_TYPE                     (UDMA_TR_TYPE_9)
/**< CMD_ID stamped into the TR, it comes back in the TR response */
#define APP_TR_CMD_ID                   ((uint32_t) 0x25U)
/**< Bounded poll limit while waiting for the DRU reverse ring response */
#define APP_TR_RESP_TIMEOUT             ((uint32_t) 1000000U)

/**< Size of the payload protected by the CRC signature */
#define APP_PAYLOAD_SIZE                ((uint32_t) 124U)
/**< Size of the CRC signature embedded in the transferred buffer */
#define APP_CRC_SIZE                    ((uint32_t) 4U)
/**< Number of bytes moved by one transfer - twice the DRU burst size */
#define APP_XFER_SIZE                   (APP_PAYLOAD_SIZE + APP_CRC_SIZE)
/**< Size of the guard region placed after the transferred data */
#define APP_GUARD_SIZE                  ((uint32_t) 64U)
/**< Total size of one test buffer */
#define APP_BUF_SIZE                    (APP_XFER_SIZE + APP_GUARD_SIZE)
/**< Pattern written into the guard region of a destination buffer */
#define APP_GUARD_PATTERN               ((uint8_t) 0x5AU)
/**< Pattern written behind the data of a source buffer. It differs from
 *   #APP_GUARD_PATTERN so that a transfer running past its length carries a
 *   value into the guard region that the guard check can see */
#define APP_SRC_TAIL_PATTERN            ((uint8_t) 0xA5U)

/**< DDR test buffer, 0x5555_5555 pattern on address lines 0 to 29 with
 *   bit 31 set so that the address falls inside the DDR window */
#define APP_BUF_ADDR_55                 ((uint32_t) 0x95555555U)
/**< DDR test buffer, inverse of #APP_BUF_ADDR_55 over address lines 0 to 29 */
#define APP_BUF_ADDR_AA                 ((uint32_t) 0xAAAAAAAAU)
/**< MCU MSRAM test buffer, used to cover address lines 30 and 31 */
#define APP_BUF_ADDR_MSRAM              ((uint32_t) (CSL_MCU_MSRAM_256K0_RAM_BASE + 0x2A00U))

/**< DRU exposed ring, holds 64-bit descriptor (TRPD) pointers */
#define APP_RING_ELEM_CNT               ((uint32_t) 8U)
#define APP_RING_MEM_SIZE               (APP_RING_ELEM_CNT * sizeof(uint64_t))
#define APP_RING_MEM_SIZE_ALIGN         (UDMA_ALIGN_SIZE(APP_RING_MEM_SIZE))
/**< TR packet descriptor memory size - one 64 byte TR plus its response */
#define APP_TRPD_SIZE                   (UDMA_GET_TRPD_TR15_SIZE(1U))

/**< CRC-32 (IEEE 802.3) parameters, reflected form */
#define APP_CRC32_POLY                  ((uint32_t) 0xEDB88320U)
#define APP_CRC32_INIT                  ((uint32_t) 0xFFFFFFFFU)
#define APP_CRC32_XOROUT                ((uint32_t) 0xFFFFFFFFU)

/**
 *  \brief One transfer of the test sequence.
 */
typedef struct
{
    uint32_t    srcAddr;
    /**< Address of the source buffer */
    uint32_t    destAddr;
    /**< Address of the destination buffer */
} App_TransferStep;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief   Enable the DRU host module clock so that its MMRs are accessible.
 *
 * \retval  UDMA_SOK    DRU register space is accessible.
 * \retval  UDMA_EFAIL  Module clock enable failed.
 */
static int32_t AppDru_enableModuleClock(void);

/**
 * \brief   Check that the DRU supports the TR type used by this test.
 *
 *          The DRU capabilities register lists the TR types and the
 *          addressing, element type and reformatting modes implemented by
 *          the instance. The block copy below relies on the TR type being
 *          present and on linear addressing with a one byte element type.
 *
 * \param   pDruRegs    DRU register overlay base.
 *
 * \retval  UDMA_SOK    The TR type used by this test is supported.
 * \retval  UDMA_EFAIL  Capabilities read failed or TR type not supported.
 */
static int32_t AppDru_checkCapabilities(const CSL_DRU_t *pDruRegs);

/**
 * \brief   Program the DRU scheduling queue used by the test channel.
 *
 *          The queue configuration is read back and compared against the
 *          value that was written. The CSL only validates its arguments, so
 *          a queue that is not implemented by this DRU instance still
 *          reports success and would leave the channel scheduled on a queue
 *          that never runs its transfers.
 *
 * \param   pDruRegs    DRU register overlay base.
 *
 * \retval  UDMA_SOK    Queue programmed and read back as expected.
 * \retval  UDMA_EFAIL  Queue configuration failed or did not take effect.
 */
static int32_t AppDru_configQueue(const CSL_DRU_t *pDruRegs);

/**
 * \brief   Open, configure and enable the DRU channel used for the test.
 *
 *          The channel is opened with ring memory, which makes the UDMA
 *          driver program the DRU per channel ring (CHRING_ADDR/SIZE). The
 *          UTC configuration is done in direct TR mode, the only mode whose
 *          UDMA driver path does not need an UDMAP channel to be configured
 *          by the system firmware, and the DRU side owner bit is then moved
 *          to UDMAC TR by a direct MMR write while the channel is still
 *          disabled, because the DRU consumes its forward ring only in that
 *          mode.
 *
 *          The channel to queue mapping is read back as well, the queue
 *          field of CHST_SCHED is narrower than the number of queues the CSL
 *          claims, so a queue number can be silently truncated.
 *
 * \param   drvHandle   UDMA driver handle.
 * \param   chHandle    UDMA channel handle to be opened.
 * \param   pChNum      Filled with the DRU relative channel number.
 *
 * \retval  UDMA_SOK    Channel is open and enabled.
 * \retval  others      Refer \ref Udma_ErrorCodes
 */
static int32_t AppDru_openChannel(Udma_DrvHandle drvHandle,
                                  Udma_ChHandle chHandle,
                                  uint32_t *pChNum);

/**
 * \brief   Disable and close the DRU channel.
 *
 * \param   chHandle    UDMA channel handle.
 * \param   pRetVal     Updated with the first error seen, if any.
 */
static void AppDru_closeChannel(Udma_ChHandle chHandle, int32_t *pRetVal);

/**
 * \brief   Log the real time state of a DRU channel.
 *
 *          Called when a transfer does not complete, so that the log carries
 *          the channel control and status registers, the ring occupancies
 *          and the error cause bits of the channel.
 *
 * \param   pDruRegs    DRU register overlay base.
 * \param   chNum       DRU relative channel number.
 */
static void AppDru_logChannelState(const CSL_DRU_t *pDruRegs, uint32_t chNum);

/**
 * \brief   Compute a CRC-32 (IEEE 802.3) signature over a buffer.
 *
 *          Reflected, bitwise implementation of the CRC-32 used by Ethernet
 *          and zlib, polynomial 0x04C11DB7 (0xEDB88320 in reflected form),
 *          initial value 0xFFFFFFFF and final XOR 0xFFFFFFFF.
 *
 * \param   buf         Buffer to compute the signature over.
 * \param   length      Number of bytes to process.
 *
 * \return  CRC-32 signature of the buffer.
 */
static uint32_t App_swCrc32(const void *buf, uint32_t length);

/**
 * \brief   Fill the payload buffer used as reference data for all transfers.
 */
static void App_initPayload(void);

/**
 * \brief   Build the initial source buffer, payload followed by its CRC.
 *
 * \param   srcBuf      Source buffer of the first transfer.
 * \param   crcVal      CRC signature to embed after the payload.
 */
static void App_initSrcBuf(uint8_t *srcBuf, uint32_t crcVal);

/**
 * \brief   Write the tail pattern behind the data of a source buffer.
 *
 *          A buffer that has been a destination carries the guard pattern
 *          behind its data. Refreshing the tail before the buffer is used as a
 *          source keeps the two patterns distinct, which is what lets the guard
 *          check see a transfer that runs past its length.
 *
 * \param   srcBuf      Source buffer of the next transfer.
 */
static void App_initSrcTail(uint8_t *srcBuf);

/**
 * \brief   Clear a destination buffer and initialize its guard region.
 *
 * \param   destBuf     Destination buffer of the next transfer.
 */
static void App_initDestBuf(uint8_t *destBuf);

/**
 * \brief   Check that the guard region after the transferred data is intact.
 *
 * \param   destBuf     Destination buffer of the completed transfer.
 *
 * \retval  UDMA_SOK    Guard region unchanged, no overrun.
 * \retval  UDMA_EFAIL  Guard region modified by the transfer.
 */
static int32_t App_checkGuard(const uint8_t *destBuf);

/**
 * \brief   Compare the received payload against the reference payload.
 *
 * \param   destBuf     Destination buffer of the completed transfer.
 *
 * \retval  UDMA_SOK    Received payload matches the reference payload.
 * \retval  UDMA_EFAIL  Payload mismatch.
 */
static int32_t App_checkPayload(const uint8_t *destBuf);

/**
 * \brief   Unpack payload and CRC signature and verify the signature.
 *
 *          The signature embedded in the received buffer is extracted, a new
 *          signature is computed over the received payload and both values
 *          are compared.
 *
 * \param   destBuf     Destination buffer of the completed transfer.
 *
 * \retval  UDMA_SOK    Embedded and recomputed signatures match.
 * \retval  UDMA_EFAIL  Signature mismatch.
 */
static int32_t App_checkCrc(const uint8_t *destBuf);

/**
 * \brief   Initialize the TR packet descriptor for one linear block copy.
 *
 * \param   chHandle    UDMA channel handle.
 * \param   trpdMem     TRPD memory to be populated.
 * \param   destAddr    Address of the destination buffer.
 * \param   srcAddr     Address of the source buffer.
 * \param   length      Number of bytes to move.
 */
static void App_initTrpd(Udma_ChHandle chHandle,
                         uint8_t *trpdMem,
                         uint32_t destAddr,
                         uint32_t srcAddr,
                         uint32_t length);

/**
 * \brief   Run one block copy on the DRU and wait for its completion.
 *
 *          The TRPD is queued into the DRU forward ring and the forward
 *          doorbell is rung. Completion is detected by polling the reverse
 *          ring occupancy of the channel, the returned descriptor pointer
 *          and the TR response status are both checked.
 *
 * \param   chHandle    UDMA channel handle.
 * \param   pDruRegs    DRU register overlay base.
 * \param   chNum       DRU relative channel number.
 * \param   destAddr    Address of the destination buffer.
 * \param   srcAddr     Address of the source buffer.
 * \param   length      Number of bytes to move.
 *
 * \retval  UDMA_SOK    Transfer completed without error.
 * \retval  others      Refer \ref Udma_ErrorCodes
 */
static int32_t AppDru_memcpy(Udma_ChHandle chHandle,
                             const CSL_DRU_t *pDruRegs,
                             uint32_t chNum,
                             uint32_t destAddr,
                             uint32_t srcAddr,
                             uint32_t length);

/**
 * \brief   Run one transfer of the test sequence and validate its result.
 *
 *          Prepares the destination buffer, performs the transfer and then
 *          checks the guard region, the payload and the CRC signature.
 *
 * \param   chHandle    UDMA channel handle.
 * \param   pDruRegs    DRU register overlay base.
 * \param   chNum       DRU relative channel number.
 * \param   step        Source and destination addresses of the transfer.
 *
 * \retval  UDMA_SOK    Transfer and all checks passed.
 * \retval  others      Refer \ref Udma_ErrorCodes
 */
static int32_t App_transferTest(Udma_ChHandle chHandle,
                                const CSL_DRU_t *pDruRegs,
                                uint32_t chNum,
                                const App_TransferStep *step);

/**
 * \brief   Run the complete test sequence on the given DRU channel.
 *
 * \param   chHandle    UDMA channel handle.
 * \param   pDruRegs    DRU register overlay base.
 * \param   chNum       DRU relative channel number.
 *
 * \retval  UDMA_SOK    All transfers and checks passed.
 * \retval  others      Refer \ref Udma_ErrorCodes
 */
static int32_t App_druTest(Udma_ChHandle chHandle,
                           const CSL_DRU_t *pDruRegs,
                           uint32_t chNum);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

Udma_DrvObject gUdmaDrvObj;
Udma_ChObject  gUdmaChObj;

/**< DRU exposed ring memory and the TR packet descriptor */
static uint8_t gDruRingMem[APP_RING_MEM_SIZE_ALIGN] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
static uint8_t gUdmaTrpdMem[APP_TRPD_SIZE] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));

/**< Reference payload, the CRC signature is computed over this data */
static uint8_t gAppPayload[APP_PAYLOAD_SIZE];

/**< Transfer sequence, each destination becomes the next source */
static const App_TransferStep gAppTransferSteps[] =
{
    { APP_BUF_ADDR_55,    APP_BUF_ADDR_AA    },
    { APP_BUF_ADDR_AA,    APP_BUF_ADDR_55    },
    { APP_BUF_ADDR_55,    APP_BUF_ADDR_MSRAM },
    { APP_BUF_ADDR_MSRAM, APP_BUF_ADDR_55    },
};

#define APP_NUM_TRANSFER_STEPS  (sizeof(gAppTransferSteps) / \
                                 sizeof(gAppTransferSteps[0U]))

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int main(void)
{
    int32_t             status = SystemP_FAILURE;
    int32_t             retVal = UDMA_EFAIL;
    int32_t             tempRetVal;
    Udma_DrvHandle      drvHandle = &gUdmaDrvObj;
    Udma_ChHandle       chHandle = &gUdmaChObj;
    const CSL_DRU_t    *pDruRegs = (const CSL_DRU_t *) UDMA_UTC_BASE_DRU0;
    Udma_InitPrms       initPrms;
    uint32_t            chNum = 0U;
    bool                initDone = false;
    bool                chOpenDone = false;

    System_init();
    Board_init();

    Drivers_open();
    status = Board_driversOpen();
    DebugP_assert(status == SystemP_SUCCESS);

    DebugP_log("DRU basic tests with CRC check\r\n");

    retVal = AppDru_enableModuleClock();

    if(UDMA_SOK == retVal)
    {
        retVal = UdmaInitPrms_init(APP_DRU_UDMA_INST_ID, &initPrms);
        if(UDMA_SOK != retVal)
        {
            DebugP_log("[Error] UDMA init params init failed!!\r\n");
        }
    }

    if(UDMA_SOK == retVal)
    {
        initPrms.enableUtc = (uint8_t) TRUE;

        retVal = Udma_init(drvHandle, &initPrms);
        if(UDMA_SOK != retVal)
        {
            DebugP_log("[Error] UDMA init failed!!\r\n");
        }
        else
        {
            initDone = true;
        }
    }

    if(UDMA_SOK == retVal)
    {
        retVal = AppDru_checkCapabilities(pDruRegs);
    }

    if(UDMA_SOK == retVal)
    {
        retVal = AppDru_configQueue(pDruRegs);
    }

    if(UDMA_SOK == retVal)
    {
        retVal = AppDru_openChannel(drvHandle, chHandle, &chNum);
        if(UDMA_SOK == retVal)
        {
            chOpenDone = true;
        }
    }

    if(UDMA_SOK == retVal)
    {
        retVal = App_druTest(chHandle, pDruRegs, chNum);
    }

    if(chOpenDone)
    {
        AppDru_closeChannel(chHandle, &retVal);
    }

    if(initDone)
    {
        tempRetVal = Udma_deinit(drvHandle);
        if(UDMA_SOK != tempRetVal)
        {
            DebugP_log("[Error] UDMA deinit failed!!\r\n");
            if(UDMA_SOK == retVal)
            {
                retVal = tempRetVal;
            }
        }
    }

    if(UDMA_SOK == retVal)
    {
        DebugP_log("All tests have passed!!\r\n");
    }
    else
    {
        DebugP_log("Some tests have failed!!\r\n");
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
        DebugP_log("[Error] DRU host module clock enable failed!!\r\n");
        retVal = UDMA_EFAIL;
    }

    return (retVal);
}

static int32_t AppDru_checkCapabilities(const CSL_DRU_t *pDruRegs)
{
    int32_t             retVal = UDMA_SOK;
    int32_t             status;
    CSL_DruCapabilities druCap;
    uint32_t            trTypeMask = ((uint32_t) 1U << ((uint32_t) APP_TR_TYPE));

    status = CSL_druGetCapabilities(pDruRegs, &druCap);
    if(SystemP_SUCCESS != status)
    {
        DebugP_log("[Error] DRU capabilities read failed!!\r\n");
        retVal = UDMA_EFAIL;
    }
    else
    {
        if(0U == (((uint32_t) druCap.trTypeSupported) & trTypeMask))
        {
            DebugP_log("[Error] TR type %u is not supported by this DRU, "
                       "supported TR type mask is 0x%04x!!\r\n",
                       (uint32_t) APP_TR_TYPE,
                       (uint32_t) druCap.trTypeSupported);
            retVal = UDMA_EFAIL;
        }
    }

    return (retVal);
}

static int32_t AppDru_configQueue(const CSL_DRU_t *pDruRegs)
{
    int32_t             retVal = UDMA_SOK;
    int32_t             status;
    CSL_DruQueueConfig  queueCfg;
    uint64_t            expectedCfg;
    uint32_t            actualCfg;

    queueCfg.priority         = APP_DRU_QUEUE_PRIORITY;
    queueCfg.qos              = APP_DRU_QUEUE_QOS;
    queueCfg.orderId          = APP_DRU_QUEUE_ORDER_ID;
    queueCfg.consecuitveTrans = APP_DRU_QUEUE_CONSEC_TRANS;
    queueCfg.rearbWait        = APP_DRU_QUEUE_REARB_WAIT;

    expectedCfg  = CSL_FMK(DRU_QUEUE_CFG_PRI, queueCfg.priority);
    expectedCfg |= CSL_FMK(DRU_QUEUE_CFG_QOS, queueCfg.qos);
    expectedCfg |= CSL_FMK(DRU_QUEUE_CFG_ORDERID, queueCfg.orderId);
    expectedCfg |= CSL_FMK(DRU_QUEUE_CFG_CONSECUTIVE_TRANS,
                           queueCfg.consecuitveTrans);
    expectedCfg |= CSL_FMK(DRU_QUEUE_CFG_REARB_WAIT, queueCfg.rearbWait);

    status = CSL_druQueueConfig(pDruRegs, APP_DRU_QUEUE_ID, &queueCfg);
    if(SystemP_SUCCESS != status)
    {
        DebugP_log("[Error] DRU queue %u config failed!!\r\n",
                   (uint32_t) APP_DRU_QUEUE_ID);
        retVal = UDMA_EFAIL;
    }

    if(UDMA_SOK == retVal)
    {
        actualCfg = (uint32_t) CSL_REG64_RD(
                        &pDruRegs->DRUQueues.CFG[APP_DRU_QUEUE_ID]);
        if(((uint32_t) expectedCfg) != actualCfg)
        {
            DebugP_log("[Error] DRU queue %u CFG reads back 0x%08x, "
                       "expected 0x%08x!!\r\n",
                       (uint32_t) APP_DRU_QUEUE_ID,
                       actualCfg,
                       (uint32_t) expectedCfg);
            retVal = UDMA_EFAIL;
        }
    }

    return (retVal);
}

static int32_t AppDru_openChannel(Udma_DrvHandle drvHandle,
                                  Udma_ChHandle chHandle,
                                  uint32_t *pChNum)
{
    int32_t         retVal;
    uint32_t        chType;
    uint32_t        chNum = 0U;
    Udma_ChPrms     chPrms;
    Udma_ChUtcPrms  utcPrms;
    CSL_DRU_t      *pDruRegs = (CSL_DRU_t *) UDMA_UTC_BASE_DRU0;
    uint64_t        chCfg;
    uint64_t        chSched;
    uint32_t        schedQueueId;
    bool            chOpened = false;

    chType = UDMA_CH_TYPE_UTC;
    UdmaChPrms_init(&chPrms, chType);
    chPrms.utcId                  = APP_DRU_UTC_ID;
    chPrms.fqRingPrms.ringMem     = &gDruRingMem[0U];
    chPrms.fqRingPrms.ringMemSize = APP_RING_MEM_SIZE;
    chPrms.fqRingPrms.elemCnt     = APP_RING_ELEM_CNT;

    retVal = Udma_chOpen(drvHandle, chHandle, chType, &chPrms);
    if(UDMA_SOK != retVal)
    {
        DebugP_log("[Error] UDMA channel open failed!!\r\n");
    }
    else
    {
        chOpened = true;
        chNum = Udma_chGetNum(chHandle);
    }

    if(UDMA_SOK == retVal)
    {
        UdmaChUtcPrms_init(&utcPrms);
        utcPrms.druOwner   = CSL_DRU_OWNER_DIRECT_TR;
        utcPrms.druQueueId = APP_DRU_QUEUE_ID;

        retVal = Udma_chConfigUtc(chHandle, &utcPrms);
        if(UDMA_SOK != retVal)
        {
            DebugP_log("[Error] UDMA UTC channel config failed!!\r\n");
        }
    }

    if(UDMA_SOK == retVal)
    {
        chSched = CSL_REG64_RD(&pDruRegs->CHNRT[chNum].CHST_SCHED);
        schedQueueId = (uint32_t) CSL_REG64_FEXT(&chSched,
                                                DRU_CHNRT_CHST_SCHED_QUEUE);
        if(schedQueueId != ((uint32_t) APP_DRU_QUEUE_ID))
        {
            DebugP_log("[Error] DRU channel is scheduled on queue %u instead "
                       "of queue %u!!\r\n",
                       schedQueueId,
                       (uint32_t) APP_DRU_QUEUE_ID);
            retVal = UDMA_EFAIL;
        }
    }

    if(UDMA_SOK == retVal)
    {
        chCfg  = CSL_REG64_RD(&pDruRegs->CHNRT[chNum].CFG);
        chCfg |= (uint64_t) CSL_DRU_CHNRT_CFG_CHAN_TYPE_OWNER_MASK;
        CSL_REG64_WR(&pDruRegs->CHNRT[chNum].CFG, chCfg);

        if(0U == (CSL_REG64_RD(&pDruRegs->CHNRT[chNum].CFG) &
                  (uint64_t) CSL_DRU_CHNRT_CFG_CHAN_TYPE_OWNER_MASK))
        {
            DebugP_log("[Error] DRU channel owner could not be set!!\r\n");
            retVal = UDMA_EFAIL;
        }
    }

    if(UDMA_SOK == retVal)
    {
        retVal = Udma_chEnable(chHandle);
        if(UDMA_SOK != retVal)
        {
            DebugP_log("[Error] UDMA channel enable failed!!\r\n");
        }
        else
        {
            *pChNum = chNum;
        }
    }

    if((UDMA_SOK != retVal) && chOpened)
    {
        (void) Udma_chClose(chHandle);
    }

    return (retVal);
}

static void AppDru_closeChannel(Udma_ChHandle chHandle, int32_t *pRetVal)
{
    int32_t status;

    status = Udma_chDisable(chHandle, UDMA_DEFAULT_CH_DISABLE_TIMEOUT);
    if(UDMA_SOK != status)
    {
        DebugP_log("[Error] UDMA channel disable failed!!\r\n");
        if(UDMA_SOK == *pRetVal)
        {
            *pRetVal = status;
        }
    }

    status = Udma_chClose(chHandle);
    if(UDMA_SOK != status)
    {
        DebugP_log("[Error] UDMA channel close failed!!\r\n");
        if(UDMA_SOK == *pRetVal)
        {
            *pRetVal = status;
        }
    }

    return;
}

static void AppDru_logChannelState(const CSL_DRU_t *pDruRegs, uint32_t chNum)
{
    uint64_t    ctl = CSL_REG64_RD(&pDruRegs->CHRT[chNum].CHRT_CTL);
    uint64_t    det = CSL_REG64_RD(&pDruRegs->CHRT[chNum].CHRT_STATUS_DET);
    uint64_t    fwdOcc = CSL_REG64_RD(&pDruRegs->CHRT[chNum].RING_FWD_OCC);
    uint64_t    rvrsOcc = CSL_REG64_RD(&pDruRegs->CHRT[chNum].RING_RCVS_OCC);
    uint64_t    cause = CSL_REG64_RD(&pDruRegs->CAUSE.CAUSE.CAUSE[chNum / 16U]);

    DebugP_log("  DRU channel %u state:\r\n", chNum);
    DebugP_log("    CHRT_CTL        = 0x%08x%08x\r\n",
               (uint32_t) (ctl >> 32U), (uint32_t) ctl);
    DebugP_log("    CHRT_STATUS_DET = 0x%08x%08x\r\n",
               (uint32_t) (det >> 32U), (uint32_t) det);
    DebugP_log("    RING_FWD_OCC    = 0x%08x%08x\r\n",
               (uint32_t) (fwdOcc >> 32U), (uint32_t) fwdOcc);
    DebugP_log("    RING_RCVS_OCC   = 0x%08x%08x\r\n",
               (uint32_t) (rvrsOcc >> 32U), (uint32_t) rvrsOcc);
    DebugP_log("    CAUSE[%u]        = 0x%08x%08x\r\n",
               chNum / 16U,
               (uint32_t) (cause >> 32U), (uint32_t) cause);

    return;
}

static uint32_t App_swCrc32(const void *buf, uint32_t length)
{
    const uint8_t  *pData = (const uint8_t *) buf;
    uint32_t        crc = APP_CRC32_INIT;
    uint32_t        i;
    uint32_t        bit;

    for(i = 0U; i < length; i++)
    {
        crc ^= (uint32_t) pData[i];
        for(bit = 0U; bit < 8U; bit++)
        {
            if(0U != (crc & 1U))
            {
                crc = (crc >> 1U) ^ APP_CRC32_POLY;
            }
            else
            {
                crc = (crc >> 1U);
            }
        }
    }

    return (crc ^ APP_CRC32_XOROUT);
}

static void App_initPayload(void)
{
    uint32_t i;

    for(i = 0U; i < APP_PAYLOAD_SIZE; i++)
    {
        gAppPayload[i] = (uint8_t) i;
    }

    return;
}

static void App_initSrcBuf(uint8_t *srcBuf, uint32_t crcVal)
{
    (void) memcpy(&srcBuf[0U], &gAppPayload[0U], APP_PAYLOAD_SIZE);
    (void) memcpy(&srcBuf[APP_PAYLOAD_SIZE], &crcVal, APP_CRC_SIZE);
    (void) memset(&srcBuf[APP_XFER_SIZE], APP_SRC_TAIL_PATTERN, APP_GUARD_SIZE);

    CacheP_wb(srcBuf, APP_BUF_SIZE, CacheP_TYPE_ALLD);

    return;
}

static void App_initSrcTail(uint8_t *srcBuf)
{
    (void) memset(&srcBuf[APP_XFER_SIZE], APP_SRC_TAIL_PATTERN, APP_GUARD_SIZE);

    CacheP_wb(srcBuf, APP_BUF_SIZE, CacheP_TYPE_ALLD);

    return;
}

static void App_initDestBuf(uint8_t *destBuf)
{
    (void) memset(&destBuf[0U], 0, APP_XFER_SIZE);
    (void) memset(&destBuf[APP_XFER_SIZE], APP_GUARD_PATTERN, APP_GUARD_SIZE);

    CacheP_wb(destBuf, APP_BUF_SIZE, CacheP_TYPE_ALLD);

    return;
}

static int32_t App_checkGuard(const uint8_t *destBuf)
{
    int32_t     retVal = UDMA_SOK;
    uint32_t    i;

    for(i = 0U; i < APP_GUARD_SIZE; i++)
    {
        if(APP_GUARD_PATTERN != destBuf[APP_XFER_SIZE + i])
        {
            DebugP_log("\r\n[Error] Overrun detected at offset %u, "
                       "expected 0x%02x got 0x%02x!!\r\n",
                       APP_XFER_SIZE + i,
                       (uint32_t) APP_GUARD_PATTERN,
                       (uint32_t) destBuf[APP_XFER_SIZE + i]);
            retVal = UDMA_EFAIL;
            break;
        }
    }

    return (retVal);
}

static int32_t App_checkPayload(const uint8_t *destBuf)
{
    int32_t     retVal = UDMA_SOK;
    uint32_t    i;

    for(i = 0U; i < APP_PAYLOAD_SIZE; i++)
    {
        if(gAppPayload[i] != destBuf[i])
        {
            DebugP_log("\r\n[Error] Payload mismatch at offset %u, "
                       "expected 0x%02x got 0x%02x!!\r\n",
                       i,
                       (uint32_t) gAppPayload[i],
                       (uint32_t) destBuf[i]);
            retVal = UDMA_EFAIL;
            break;
        }
    }

    return (retVal);
}

static int32_t App_checkCrc(const uint8_t *destBuf)
{
    int32_t     retVal = UDMA_SOK;
    uint32_t    embeddedCrc;
    uint32_t    calcCrc;

    (void) memcpy(&embeddedCrc, &destBuf[APP_PAYLOAD_SIZE], APP_CRC_SIZE);
    calcCrc = App_swCrc32(&destBuf[0U], APP_PAYLOAD_SIZE);

    if(embeddedCrc != calcCrc)
    {
        DebugP_log("\r\n[Error] CRC check failed, embedded 0x%08x "
                   "calculated 0x%08x!!\r\n",
                   embeddedCrc,
                   calcCrc);
        retVal = UDMA_EFAIL;
    }

    return (retVal);
}

static void App_initTrpd(Udma_ChHandle chHandle,
                         uint8_t *trpdMem,
                         uint32_t destAddr,
                         uint32_t srcAddr,
                         uint32_t length)
{
    CSL_UdmapTR15  *pTr;
    uint32_t        cqRingNum = Udma_chGetCqRingNum(chHandle);

    UdmaUtils_makeTrpd(trpdMem, APP_TR_TYPE, 1U, cqRingNum);

    pTr           = UdmaUtils_getTrpdTr15Pointer(trpdMem, 0U);
    pTr->flags    = CSL_FMK(UDMAP_TR_FLAGS_TYPE, APP_TR_TYPE);
    pTr->flags   |= CSL_FMK(UDMAP_TR_FLAGS_STATIC, 0U);
    pTr->flags   |= CSL_FMK(UDMAP_TR_FLAGS_EOL, 0U);
    pTr->flags   |= CSL_FMK(UDMAP_TR_FLAGS_EVENT_SIZE,
                            CSL_UDMAP_TR_FLAGS_EVENT_SIZE_COMPLETION);
    pTr->flags   |= CSL_FMK(UDMAP_TR_FLAGS_TRIGGER0,
                            CSL_UDMAP_TR_FLAGS_TRIGGER_NONE);
    pTr->flags   |= CSL_FMK(UDMAP_TR_FLAGS_TRIGGER0_TYPE,
                            CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ALL);
    pTr->flags   |= CSL_FMK(UDMAP_TR_FLAGS_TRIGGER1,
                            CSL_UDMAP_TR_FLAGS_TRIGGER_NONE);
    pTr->flags   |= CSL_FMK(UDMAP_TR_FLAGS_TRIGGER1_TYPE,
                            CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ALL);
    pTr->flags   |= CSL_FMK(UDMAP_TR_FLAGS_CMD_ID, APP_TR_CMD_ID);
    pTr->flags   |= CSL_FMK(UDMAP_TR_FLAGS_SA_INDIRECT, 0U);
    pTr->flags   |= CSL_FMK(UDMAP_TR_FLAGS_DA_INDIRECT, 0U);
    pTr->flags   |= CSL_FMK(UDMAP_TR_FLAGS_EOP, 1U);
    pTr->icnt0    = (uint16_t) length;
    pTr->icnt1    = 1U;
    pTr->icnt2    = 1U;
    pTr->icnt3    = 1U;
    pTr->dim1     = (int32_t) pTr->icnt0;
    pTr->dim2     = (int32_t) ((uint32_t) pTr->icnt0 * (uint32_t) pTr->icnt1);
    pTr->dim3     = (int32_t) ((uint32_t) pTr->icnt0 * (uint32_t) pTr->icnt1 *
                               (uint32_t) pTr->icnt2);
    pTr->addr     = (uint64_t) srcAddr;
    pTr->fmtflags = 0x00000000U;
    pTr->dicnt0   = (uint16_t) length;
    pTr->dicnt1   = 1U;
    pTr->dicnt2   = 1U;
    pTr->dicnt3   = 1U;
    pTr->ddim1    = (int32_t) pTr->dicnt0;
    pTr->ddim2    = (int32_t) ((uint32_t) pTr->dicnt0 * (uint32_t) pTr->dicnt1);
    pTr->ddim3    = (int32_t) ((uint32_t) pTr->dicnt0 * (uint32_t) pTr->dicnt1 *
                               (uint32_t) pTr->dicnt2);
    pTr->daddr    = (uint64_t) destAddr;

    CacheP_wb(trpdMem, APP_TRPD_SIZE, CacheP_TYPE_ALLD);

    return;
}

static int32_t AppDru_memcpy(Udma_ChHandle chHandle,
                             const CSL_DRU_t *pDruRegs,
                             uint32_t chNum,
                             uint32_t destAddr,
                             uint32_t srcAddr,
                             uint32_t length)
{
    int32_t     retVal;
    uint8_t    *trpdMem = &gUdmaTrpdMem[0U];
    uint64_t    trpdPhys = (uint64_t) Udma_defaultVirtToPhyFxn(trpdMem, 0U, NULL);
    uint64_t    respElem = 0U;
    uint64_t    occ = 0U;
    uint64_t    occMask = (uint64_t) CSL_DRU_CHRT_CHRT_RING_RCVS_OCC_CNT_MASK |
                          (uint64_t) CSL_DRU_CHRT_CHRT_RING_RCVS_OCC_COMP_MASK;
    uint32_t    timeoutCnt = APP_TR_RESP_TIMEOUT;
    uint32_t    trRespStatus;

    App_initTrpd(chHandle, trpdMem, destAddr, srcAddr, length);

    retVal = Udma_chRingQueueRaw(chHandle, (uint8_t *) (uintptr_t) trpdPhys, 1U);
    if(UDMA_SOK != retVal)
    {
        DebugP_log("[Error] DRU ring queue failed!!\r\n");
    }

    if(UDMA_SOK == retVal)
    {
        retVal = Udma_chRingRingDbRaw(chHandle, 1U);
        if(UDMA_SOK != retVal)
        {
            DebugP_log("[Error] DRU ring doorbell failed!!\r\n");
        }
    }

    if(UDMA_SOK == retVal)
    {
        do
        {
            occ = CSL_REG64_RD(&pDruRegs->CHRT[chNum].RING_RCVS_OCC);
            timeoutCnt--;
        } while((0ULL == (occ & occMask)) && (timeoutCnt > 0U));

        if(0ULL == (occ & occMask))
        {
            DebugP_log("[Error] Timeout waiting for the DMA transfer to "
                       "complete!!\r\n");
            AppDru_logChannelState(pDruRegs, chNum);
            retVal = UDMA_ETIMEOUT;
        }
        else
        {
            (void) Udma_chRingDeQueueRaw(chHandle, 1U, &respElem);
            (void) Udma_chRingRingRvrDbRaw(chHandle, 1U);

            if(respElem != trpdPhys)
            {
                DebugP_log("[Error] Returned descriptor 0x%08x does not match "
                           "the submitted one 0x%08x!!\r\n",
                           (uint32_t) respElem,
                           (uint32_t) trpdPhys);
                retVal = UDMA_EFAIL;
            }
        }
    }

    if(UDMA_SOK == retVal)
    {
        CacheP_inv(trpdMem, APP_TRPD_SIZE, CacheP_TYPE_ALLD);

        trRespStatus = UdmaUtils_getTrpdTr15Response(trpdMem, 1U, 0U);
        if(CSL_UDMAP_TR_RESPONSE_STATUS_COMPLETE != trRespStatus)
        {
            DebugP_log("[Error] TR response status is %u, transfer did not "
                       "complete!!\r\n",
                       trRespStatus);
            AppDru_logChannelState(pDruRegs, chNum);
            retVal = UDMA_EFAIL;
        }
    }

    return (retVal);
}

static int32_t App_transferTest(Udma_ChHandle chHandle,
                                const CSL_DRU_t *pDruRegs,
                                uint32_t chNum,
                                const App_TransferStep *step)
{
    int32_t     retVal;
    uint8_t    *destBuf = (uint8_t *) step->destAddr;

    DebugP_log("\r\nDMA transfer of buffer from address 0x%08x to "
               "address 0x%08x\r\n",
               step->srcAddr,
               step->destAddr);

    App_initSrcTail((uint8_t *) step->srcAddr);

    /* Init destination to all zeros */
    App_initDestBuf(destBuf);

    retVal = AppDru_memcpy(chHandle,
                           pDruRegs,
                           chNum,
                           step->destAddr,
                           step->srcAddr,
                           APP_XFER_SIZE);
    if(UDMA_SOK == retVal)
    {
        CacheP_inv(destBuf, APP_BUF_SIZE, CacheP_TYPE_ALLD);

        DebugP_log("Running tests for Guard .. ");
        retVal = App_checkGuard(destBuf);
        if(UDMA_SOK == retVal)
        {
            DebugP_log("OK\r\n");
        }
    }

    if(UDMA_SOK == retVal)
    {
        DebugP_log("Running tests for Payload .. ");
        retVal = App_checkPayload(destBuf);
        if(UDMA_SOK == retVal)
        {
            DebugP_log("OK\r\n");
        }
    }

    if(UDMA_SOK == retVal)
    {
        DebugP_log("Running tests for CRC .. ");
        retVal = App_checkCrc(destBuf);
        if(UDMA_SOK == retVal)
        {
            DebugP_log("OK\r\n");
        }
    }

    return (retVal);
}

static int32_t App_druTest(Udma_ChHandle chHandle,
                           const CSL_DRU_t *pDruRegs,
                           uint32_t chNum)
{
    int32_t     retVal = UDMA_SOK;
    uint32_t    crcVal;
    uint32_t    i;

    App_initPayload();
    crcVal = App_swCrc32(&gAppPayload[0U], APP_PAYLOAD_SIZE);

    DebugP_log("Initial CRC calculation based on the payload data\r\n");
    DebugP_log("... CRC value: 0x%08x\r\n", crcVal);

    App_initSrcBuf((uint8_t *) gAppTransferSteps[0U].srcAddr, crcVal);

    for(i = 0U; i < APP_NUM_TRANSFER_STEPS; i++)
    {
        retVal = App_transferTest(chHandle,
                                  pDruRegs,
                                  chNum,
                                  &gAppTransferSteps[i]);
        if(UDMA_SOK != retVal)
        {
            break;
        }
    }

    return (retVal);
}
