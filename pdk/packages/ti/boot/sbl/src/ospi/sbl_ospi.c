/**
 *  \file    sbl_ospi.c
 *
 *  \brief   This file contains functions for OSPI read/write operations for SBL
 *
 */

/*
 * Copyright (C) 2018-2022 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of
 * its contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <stdint.h>
#include "string.h"

/* SBL Header files. */
#include "sbl_soc.h"
#include "sbl_rprc_parse.h"
#include "sbl_err_trap.h"
#include "sbl_sci_client.h"
#include "sbl_soc_cfg.h"

#if SBL_USE_DMA
#include "sbl_dma.h"
#endif

/* TI-RTOS Header files */
#include <ti/drv/spi/SPI.h>
#if SBL_USE_DMA
#include <ti/drv/udma/udma.h>
#endif

#include <ti/drv/spi/src/SPI_osal.h>
#include <ti/drv/uart/UART_stdio.h>
#include <ti/drv/spi/soc/SPI_soc.h>
#include <ti/drv/spi/src/v0/OSPI_v0.h>
#include <ti/drv/sciclient/sciclient.h>
#include <ti/csl/cslr_device.h>
#include <ti/csl/arch/csl_arch.h>
#include <ti/csl/arch/r5/csl_arm_r5.h>
#include <ti/csl/arch/r5/interrupt.h>
#include <ti/board/board_cfg.h>
#include <ti/board/src/flash/include/board_flash.h>
#include "sbl_ospi.h"

/* Macro to specify the counter number */
#define CNTR_NUMBER 0x1F

/* Initialize the OSPI driver and the controller. */
static void SBL_OSPI_Initialize(bool isNandBootEnabled);

void SBL_DCacheClean(void *addr, uint32_t size);

void SBL_SysFwLoad(void *dst, void *src, uint32_t size);

static void *boardHandle = NULL;

static OSPI_v0_HwAttrs ospi_cfg;

/* Global variable to check whether BUILD_XIP is defined or not */
bool isXIPEnable = BFALSE; 
/* Global variable to check whether OSPI needs to run on 133 MHZ or 166 MHz while booting an application in XIP mode */
uint32_t ospiFrequency;
/* Global variable to check whether OSPI_NAND_BOOT is defined or not */
bool gIsNandBootEnable = BFALSE;
/* Global variable to check whether combined ROM boot image format is used or not */
extern uint8_t combinedBootmode;

#if SBL_USE_DMA

/*
 * Ring parameters
 */
/** \brief Number of ring entries - we can prime this much memcpy operations */
#define UDMA_TEST_APP_RING_ENTRIES      (1U)
/** \brief Size (in bytes) of each ring entry (Size of pointer - 64-bit) */
#define UDMA_TEST_APP_RING_ENTRY_SIZE   (sizeof(uint64_t))
/** \brief Total ring memory */
/* To align the ring memory size with the cache line size, we multiply it by 4.
   This ensures that when invalidating gTxCompRingMem, no unintended data gets invalidated */
#define UDMA_TEST_APP_RING_MEM_SIZE     (UDMA_TEST_APP_RING_ENTRIES * \
                                         UDMA_TEST_APP_RING_ENTRY_SIZE * 4)
/**
 *  \brief UDMA TR packet descriptor memory.
 *  This contains the CSL_UdmapCppi5TRPD + Padding to sizeof(CSL_UdmapTR15) +
 *  one Type_15 TR (CSL_UdmapTR15) + one TR response of 4 bytes.
 *  Since CSL_UdmapCppi5TRPD is less than CSL_UdmapTR15, size is just two times
 *  CSL_UdmapTR15 for alignment.
 */
#define UDMA_TEST_APP_TRPD_SIZE         ((sizeof(CSL_UdmapTR15) * 2U) + 4U)

/*
 * UDMA driver objects
 */
struct Udma_DrvObj      gUdmaDrvObj;
struct Udma_ChObj       gUdmaChObj;
struct Udma_EventObj    gUdmaCqEventObj;

static Udma_DrvHandle          gDrvHandle = NULL;
/*
 * UDMA Memories
 */
uint8_t gTxRingMem[UDMA_TEST_APP_RING_MEM_SIZE] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
uint8_t gTxCompRingMem[UDMA_TEST_APP_RING_MEM_SIZE] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
uint8_t gTxTdCompRingMem[UDMA_TEST_APP_RING_MEM_SIZE] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
uint8_t gUdmaTprdMem[UDMA_TEST_APP_TRPD_SIZE] __attribute__((aligned(UDMA_CACHELINE_ALIGNMENT)));
OSPI_dmaInfo gUdmaInfo;

static int32_t Ospi_udma_deinit(void);
static int32_t Ospi_udma_init(OSPI_v0_HwAttrs *cfg);

static int32_t Ospi_udma_init(OSPI_v0_HwAttrs *cfg)
{
    int32_t         retVal = UDMA_SOK;
    Udma_InitPrms   initPrms;
    uint32_t        instId;

    if (gDrvHandle == (Udma_DrvHandle)uint32_to_void_ptr(0U))
    {
        /* UDMA driver init */
        instId = UDMA_INST_ID_MCU_0;

        UdmaInitPrms_init(instId, &initPrms);
        retVal = Udma_init(&gUdmaDrvObj, &initPrms);
        if(UDMA_SOK == retVal)
        {
            gDrvHandle = &gUdmaDrvObj;
        }
    }

    if(gDrvHandle)
    {
        gUdmaInfo.drvHandle      = (void *)gDrvHandle;
        gUdmaInfo.chHandle       = (void *)&gUdmaChObj;
        gUdmaInfo.ringMem        = (void *)&gTxRingMem[0];
        gUdmaInfo.cqRingMem      = (void *)&gTxCompRingMem[0];
        gUdmaInfo.tdCqRingMem    = (void *)&gTxTdCompRingMem[0];
        gUdmaInfo.tprdMem        = (void *)&gUdmaTprdMem[0];
        gUdmaInfo.eventHandle    = (void *)&gUdmaCqEventObj;
        cfg->dmaInfo             = &gUdmaInfo;
    }

    return (retVal);
}

static int32_t Ospi_udma_deinit(void)
{
    int32_t         retVal = UDMA_SOK;

    if (gDrvHandle)
    {
        retVal = Udma_deinit(gDrvHandle);
        if(UDMA_SOK == retVal)
        {
            gDrvHandle = NULL;
        }
    }

    return (retVal);
}
#endif

int32_t SBL_ReadSysfwImage(void **pBuffer, uint32_t num_bytes)
{
#if !defined(SBL_BYPASS_OSPI_DRIVER) && !defined(SBL_BYPASS_OSPI_DRIVER_FOR_SYSFW_DOWNLOAD)
    Board_flashHandle flashHandle;

    /* Init SPI driver */
    OSPI_init();

    /* Get default OSPI cfg */
    OSPI_socGetInitCfg(BOARD_OSPI_DOMAIN, BOARD_OSPI_NOR_INSTANCE, &ospi_cfg);

    ospi_cfg.funcClk = OSPI_MODULE_CLK_200M;

    /* OSPI clock is set to 200MHz by RBL on J7200, J721S2, J784S4 & J742S2 platforms.
     * PHY mode cannot be used until sysfw is loaded and OSPI clock is
     * configured to 133MHz.
     */
#if defined(SOC_J721E) || defined(SOC_J7200) || defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)
    ospi_cfg.phyEnable = BFALSE;
    ospi_cfg.baudRateDiv = 8;
#endif

    /* Set the default SPI init configurations */
    if (gIsNandBootEnable == BTRUE)
    {
        ospi_cfg.cacheEnable = 1;
        ospi_cfg.dtrEnable = false;
        ospi_cfg.baudRateDiv = 4;
    }
    OSPI_socSetInitCfg(BOARD_OSPI_DOMAIN, BOARD_OSPI_NOR_INSTANCE, &ospi_cfg);

#if defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)
    if (gIsNandBootEnable == BTRUE)
    {
        flashHandle = Board_flashOpen(BOARD_FLASH_ID_W35N01JWTBAG,
                            BOARD_OSPI_NOR_INSTANCE, NULL);
    }
    else
    {
        flashHandle = Board_flashOpen(BOARD_FLASH_ID_S28HS512T,
                            BOARD_OSPI_NOR_INSTANCE, NULL);
    }
#elif defined(SOC_J7200)
    flashHandle = Board_flashOpen(BOARD_FLASH_ID_S28HS512T,
                        BOARD_OSPI_NOR_INSTANCE, NULL);
#else
    flashHandle = Board_flashOpen(BOARD_FLASH_ID_MT35XU512ABA1G12,
                        BOARD_OSPI_NOR_INSTANCE, NULL);
#endif

    if (flashHandle)
    {

        /* Disable PHY pipeline mode */
        CSL_ospiPipelinePhyEnable((const CSL_ospi_flash_cfgRegs *)(ospi_cfg.baseAddr), UFALSE);

#if defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)
        /* Until OSPI PHY + DMA is enabled at this early stage, the
         * ROM can more efficiently load the SYSFW directly from xSPI flash */
        if(pBuffer)
        {
            if (gIsNandBootEnable == BTRUE)
            {
                if (Board_flashRead(flashHandle, SBL_OSPI_OFFSET_SYSFW, (uint8_t *) *pBuffer, SBL_SYSFW_MAX_SIZE, NULL))
                {
                    SBL_log(SBL_LOG_ERR, "Board_flashRead failed in SBL_ReadSysfwImage \n");
                    SblErrLoop(__FILE__, __LINE__);
                }
            }
            else
            {
                /* Set up ROM to load system firmware */
                *pBuffer = (void *)(ospi_cfg.dataAddr + SBL_OSPI_OFFSET_SYSFW);
            }   /* (gIsNandBootEnable == BTRUE) && defined(SOC_J721S2) */
        }
#elif defined(SOC_J7200)
        if(pBuffer)
        {
            /* Set up ROM to load system firmware */
            *pBuffer = (void *)(ospi_cfg.dataAddr + SBL_OSPI_OFFSET_SYSFW);
        }
#else
        /* Optimized CPU copy loop - can be removed once ROM load is working */
        SBL_SysFwLoad((void *)(*pBuffer), (void *)(ospi_cfg.dataAddr + SBL_OSPI_OFFSET_SYSFW), num_bytes);
#endif

        /* Update handle for later use*/
        boardHandle = (void *)flashHandle;
    }
    else
    {
        SBL_log(SBL_LOG_ERR, "Board_flashOpen failed in SBL_ReadSysfwImage \n");
        SblErrLoop(__FILE__, __LINE__);
    }

    return CSL_PASS;
#else

     /* Get default OSPI cfg */
    OSPI_socGetInitCfg(BOARD_OSPI_DOMAIN, BOARD_OSPI_NOR_INSTANCE, &ospi_cfg);

   if(pBuffer)
   {
      /* Set up ROM to load system firmware */
      *pBuffer = (void *)(ospi_cfg.dataAddr + SBL_OSPI_OFFSET_SYSFW);
   }

   return CSL_PASS;
#endif

}

void OSPI_configClk(uint32_t freq)
{
    OSPI_v0_HwAttrs ospi_cfg;
	int32_t retVal;
    uint64_t ospi_rclk_freq;
    uint32_t parClk;
    uint32_t clkID[] = {
                           TISCI_DEV_MCU_FSS0_OSPI_0_OSPI_RCLK_CLK,
                           TISCI_DEV_MCU_FSS0_OSPI_1_OSPI_RCLK_CLK
    };
    uint32_t devID[] = {
                           TISCI_DEV_MCU_FSS0_OSPI_0,
	                       TISCI_DEV_MCU_FSS0_OSPI_1
    };

    /* Get the default SPI init configurations */
    OSPI_socGetInitCfg(BOARD_OSPI_DOMAIN, BOARD_OSPI_NOR_INSTANCE, &ospi_cfg);

    retVal = Sciclient_pmModuleClkRequest(devID[BOARD_OSPI_NOR_INSTANCE],
                                          clkID[BOARD_OSPI_NOR_INSTANCE],
                                          TISCI_MSG_VALUE_CLOCK_SW_STATE_REQ,
                                          TISCI_MSG_FLAG_AOP,
                                          SCICLIENT_SERVICE_WAIT_FOREVER);
    if (retVal != CSL_PASS)
    {
        SBL_log(SBL_LOG_MAX, "Sciclient_pmModuleClkRequest failed \n");
    }

    /* Max clocks */
    if (freq == OSPI_MODULE_CLK_166M)
    {
        parClk = TISCI_DEV_MCU_FSS0_OSPI_0_OSPI_RCLK_CLK_PARENT_HSDIV4_16FFT_MCU_2_HSDIVOUT4_CLK;

        retVal = Sciclient_pmSetModuleClkParent(devID[BOARD_OSPI_NOR_INSTANCE],
                                                clkID[BOARD_OSPI_NOR_INSTANCE],
                                                parClk,
                                                SCICLIENT_SERVICE_WAIT_FOREVER);
    }
    else
    {
        parClk = TISCI_DEV_MCU_FSS0_OSPI_0_OSPI_RCLK_CLK_PARENT_HSDIV4_16FFT_MCU_1_HSDIVOUT4_CLK;
        retVal = Sciclient_pmSetModuleClkParent(devID[BOARD_OSPI_NOR_INSTANCE],
                                                clkID[BOARD_OSPI_NOR_INSTANCE],
                                                parClk,
                                                SCICLIENT_SERVICE_WAIT_FOREVER);
    }

    if (retVal != CSL_PASS)
    {
        SBL_log(SBL_LOG_MAX, "Sciclient_pmSetModuleClkParent failed \n");
    }

	ospi_cfg.funcClk = freq;
    OSPI_socSetInitCfg(BOARD_OSPI_DOMAIN, BOARD_OSPI_NOR_INSTANCE, &ospi_cfg);

    ospi_rclk_freq = (uint64_t)freq;
    retVal = Sciclient_pmSetModuleClkFreq(devID[BOARD_OSPI_NOR_INSTANCE],
                                          clkID[BOARD_OSPI_NOR_INSTANCE],
                                          ospi_rclk_freq,
                                          TISCI_MSG_FLAG_AOP,
                                          SCICLIENT_SERVICE_WAIT_FOREVER);

    if (retVal != CSL_PASS)
    {
        SBL_log(SBL_LOG_MAX, "Sciclient_pmSetModuleClkFreq failed \n");
    }

	ospi_rclk_freq = 0;
    retVal = Sciclient_pmGetModuleClkFreq(devID[BOARD_OSPI_NOR_INSTANCE],
                                          clkID[BOARD_OSPI_NOR_INSTANCE],
                                          &ospi_rclk_freq,
                                          SCICLIENT_SERVICE_WAIT_FOREVER);
    if (retVal != CSL_PASS)
    {
        SBL_log(SBL_LOG_MAX, "Sciclient_pmGetModuleClkFreq failed \n");
    }

    SBL_log(SBL_LOG_MAX, "OSPI RCLK running at %d MHz. \n", (uint32_t)ospi_rclk_freq);

}

int32_t SBL_ospiInit(void *handle, bool isNandBootEnabled)
{
#if (!defined(SBL_BYPASS_OSPI_DRIVER) \
     /* In simulation, we must ALWAYS bypass the OSPI driver regardless of what */ \
     /* .. bypass option is requested. REMOVE WHEN SIMULATION IS IRRELEVANT. */ \
     && !(defined(SBL_BYPASS_OSPI_DRIVER_FOR_SYSFW_DOWNLOAD) && defined(SIM_BUILD)))

    Board_flashHandle flashHandle = *(Board_flashHandle *) handle;
    static uint32_t enableTuning = UTRUE;

    if (flashHandle)
    {
        Board_flashClose(flashHandle);

    }

    /* Get default OSPI cfg */
    OSPI_socGetInitCfg(BOARD_OSPI_DOMAIN, BOARD_OSPI_NOR_INSTANCE, &ospi_cfg);

#if !defined(SBL_SKIP_BRD_CFG_PM) && !defined(SBL_SKIP_SYSFW_INIT)
    {
        /* System Firmware reconfigures the OSPI clock to
         * unsupported values. This is a workaround until
         * that issue is fixed
         */
        uint64_t ospiFunClk;
        if(isXIPEnable == BTRUE)
        {
            if(ospiFrequency == 166)
            {
                ospiFunClk = (uint64_t)(OSPI_MODULE_CLK_166M);
            }
            else
            {
                ospiFunClk = (uint64_t)(OSPI_MODULE_CLK_133M);
            }
        }
        else
        {
            #if defined(SOC_J721E) || defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)
                    ospiFunClk = (uint64_t)(OSPI_MODULE_CLK_166M);
            #else
                    ospiFunClk = (uint64_t)(OSPI_MODULE_CLK_133M);
            #endif
        }
        ospi_cfg.funcClk = ospiFunClk;
        ospi_cfg.baudRateDiv = 8;
        OSPI_configClk(ospiFunClk);
        ospi_cfg.cacheEnable = true;
#if SBL_USE_DMA
        ospi_cfg.phyEnable = true;
#else
        ospi_cfg.phyEnable = false;
#endif
    }
#endif

#if SBL_USE_DMA
    ospi_cfg.dmaEnable = BTRUE;
    Ospi_udma_init(&ospi_cfg);
    SBL_udmaInit(gDrvHandle);
#endif

    /* Set the default SPI init configurations */
    if (isNandBootEnabled == BTRUE)
    {
        ospi_cfg.blkSize = 16;
        ospi_cfg.dtrEnable = false;
    }
    OSPI_socSetInitCfg(BOARD_OSPI_DOMAIN, BOARD_OSPI_NOR_INSTANCE, &ospi_cfg);

    /* Currently there is a dependence for Board_flashClose() to be called before SBL_ospiInit() to reset
     * OSPI_FLASH_CFG_RD_DATA_CAPTURE_REG_SAMPLE_EDGE_SEL_FLD. In the combined boot case, where SBL_ReadSysfwImage
     * is not called, setting this bit before opening the flash is necessary for clock frequencies of 166 MHz  */
    if(combinedBootmode == 1)
    {
        const CSL_ospi_flash_cfgRegs *pRegs = (const CSL_ospi_flash_cfgRegs *)(ospi_cfg.baseAddr);
        CSL_REG32_FINS(&pRegs->RD_DATA_CAPTURE_REG, OSPI_FLASH_CFG_RD_DATA_CAPTURE_REG_SAMPLE_EDGE_SEL_FLD, 0);

    }

#if defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)
    if (isNandBootEnabled == BTRUE)
    {
        flashHandle = Board_flashOpen(BOARD_FLASH_ID_W35N01JWTBAG,
                            BOARD_OSPI_NOR_INSTANCE, (void *)(enableTuning));
    }
    else
    {
        flashHandle = Board_flashOpen(BOARD_FLASH_ID_S28HS512T,
                            BOARD_OSPI_NOR_INSTANCE, (void *)(enableTuning));
    }
#elif defined(SOC_J7200)
    flashHandle = Board_flashOpen(BOARD_FLASH_ID_S28HS512T,
                            BOARD_OSPI_NOR_INSTANCE, (void *)(enableTuning));
#else
    flashHandle = Board_flashOpen(BOARD_FLASH_ID_MT35XU512ABA1G12,
                            BOARD_OSPI_NOR_INSTANCE, (void *)(enableTuning));
#endif
    if (flashHandle)
    {
        *(Board_flashHandle *) handle = flashHandle;
        /* Update the static handle as well, for later use */
        boardHandle = (void *)flashHandle;
    }
    else
    {
        SBL_log(SBL_LOG_ERR, "Board_flashOpen failed in SBL_ospiInit \n");
        SblErrLoop(__FILE__, __LINE__);
    }

    return 0;
#else
    return 0;
#endif
}

int32_t SBL_ospiFlashRead(const void *handle, uint8_t *dst, uint32_t length,
    uint32_t offset)
{
    uint32_t start_time = CSL_armR5PmuReadCntr(CNTR_NUMBER);
    uint32_t end_time = 0;

#if (!defined(SBL_BYPASS_OSPI_DRIVER) \
     /* In simulation, we must ALWAYS bypass the OSPI driver regardless of what */ \
     /* .. bypass option is requested. REMOVE WHEN SIMULATION IS IRRELEVANT. */ \
     && !(defined(SBL_BYPASS_OSPI_DRIVER_FOR_SYSFW_DOWNLOAD) && defined(SIM_BUILD)))

#if SBL_USE_DMA
    Board_flashHandle h = *(const Board_flashHandle *) handle;
    uint32_t ioMode = OSPI_FLASH_OCTAL_READ;

    if (length > 4 * 1024)
    {
        /* split transfer if not reading from 16 byte aligned flash offset */
        uint32_t dma_offset  = (offset + 0xF) & (~0xF);
        uint32_t non_aligned_bytes = dma_offset - offset;
        uint8_t *dma_dst = (dst + non_aligned_bytes);
        uint32_t dma_len = length - non_aligned_bytes;

        SBL_DCacheClean((void *)dst, length);

        if ((non_aligned_bytes) && (Board_flashRead(h, offset, dst, non_aligned_bytes, (void *)(&ioMode))))
        {
            SBL_log(SBL_LOG_ERR, "Board_flashRead failed!\n");
            SblErrLoop(__FILE__, __LINE__);
        }

        if (Board_flashRead(h, dma_offset, dma_dst, dma_len, (void *)(&ioMode)))
        {
            SBL_log(SBL_LOG_ERR, "Board_flashRead failed!\n");
            SblErrLoop(__FILE__, __LINE__);
        }
    }
    else
    {
        SBL_DCacheClean((void *)dst, length);
        Board_flashRead(h, offset, dst, length, (void *)(&ioMode));
    }

#else
    Board_flashHandle h = *(const Board_flashHandle *) handle;
    uint32_t ioMode = OSPI_FLASH_OCTAL_READ;
    SBL_DCacheClean((void *)dst, length);
    int32_t status = CSL_PASS;
    status = Board_flashRead(h, offset, dst, length, (void *)(&ioMode));
    if (status != CSL_PASS)
    {
        SBL_log(SBL_LOG_ERR, "Board flash Read failed !! ");
    }

#endif /* #if SBL_USE_DMA */

#else
    memcpy((void *)dst, (void *)(ospi_cfg.dataAddr + offset), length);
#endif /* #if !defined(SBL_BYPASS_OSPI_DRIVER) */

    end_time = CSL_armR5PmuReadCntr(CNTR_NUMBER);

    SBL_log(SBL_LOG_MAX, "Ospi Read speed for 0x%x bytes from offset 0x%x = %d Mbytes per sec\n", length, offset, ((400000000 / (end_time-start_time)) * length)/0x100000);

    return 0;
}

int32_t SBL_ospiClose(const void *handle)
{
#if (!defined(SBL_BYPASS_OSPI_DRIVER) \
     /* In simulation, we must ALWAYS bypass the OSPI driver regardless of what */ \
     /* .. bypass option is requested. REMOVE WHEN SIMULATION IS IRRELEVANT. */ \
     && !(defined(SBL_BYPASS_OSPI_DRIVER_FOR_SYSFW_DOWNLOAD) && defined(SIM_BUILD)))

    Board_flashHandle h = *(const Board_flashHandle *) handle;

    SBL_log(SBL_LOG_MAX, "SBL_ospiClose called\n");
    Board_flashClose(h);
#if SBL_USE_DMA
    SBL_udmaDeInit();
    Ospi_udma_deinit();
#endif
#else
#endif
    return 0;
}

#if defined(SBL_HLOS_OWNS_FLASH) && !defined(SBL_USE_MCU_DOMAIN_ONLY)
/* Only put OSPI flash back into SPI mode if we're going to directly boot ATF/U-boot/Linux from SBL */
int32_t SBL_ospiLeaveConfigSPI()
{
    int32_t retVal = E_PASS;
    Board_flashHandle h;
    Board_FlashInfo *flashInfo;

    /* Get default OSPI cfg */
    OSPI_socGetInitCfg(BOARD_OSPI_DOMAIN, BOARD_OSPI_NOR_INSTANCE, &ospi_cfg);

    ospi_cfg.funcClk = OSPI_MODULE_CLK_133M;
    /* Configure the flash for SPI mode */
    ospi_cfg.xferLines = OSPI_XFER_LINES_SINGLE;
    /* Put controller in DAC mode so flash ID can be read directly */
    ospi_cfg.dacEnable = BTRUE;
    /* Disable PHY in legacy SPI mode (1-1-1) */
    ospi_cfg.phyEnable = BFALSE;
    ospi_cfg.dtrEnable = BFALSE;
    ospi_cfg.xipEnable = BFALSE;

    /* Set the default SPI init configurations */
    OSPI_socSetInitCfg(BOARD_OSPI_DOMAIN, BOARD_OSPI_NOR_INSTANCE, &ospi_cfg);

#if defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)
    if (gIsNandBootEnable == BTRUE)
    {
        h = Board_flashOpen(BOARD_FLASH_ID_W35N01JWTBAG,
                            BOARD_OSPI_NOR_INSTANCE, NULL);
    }
    else
    {
        h = Board_flashOpen(BOARD_FLASH_ID_S28HS512T,
                            BOARD_OSPI_NOR_INSTANCE, NULL);
    }
#elif defined(SOC_J7200)
        h = Board_flashOpen(BOARD_FLASH_ID_S28HS512T,
                            BOARD_OSPI_NOR_INSTANCE, NULL);
#else
    h = Board_flashOpen(BOARD_FLASH_ID_MT35XU512ABA1G12,
                        BOARD_OSPI_NOR_INSTANCE, NULL);
#endif

    if (h)
    {
        SBL_log(SBL_LOG_MAX, "OSPI flash left configured in Legacy SPI mode.\n");
        flashInfo = (Board_FlashInfo *)h;
        SBL_log(SBL_LOG_MAX, "\n OSPI NOR device ID: 0x%x, manufacturer ID: 0x%x \n",
                flashInfo->device_id, flashInfo->manufacturer_id);
        Board_flashClose(h);
    }
    else
    {
        SBL_log(SBL_LOG_ERR, "Board_flashOpen failed in SPI mode!!\n");
        retVal = E_FAIL;
    }


    return(retVal);
}
#endif

int32_t SBL_OSPIBootImage(sblEntryPoint_t *pEntry)
{
    int32_t retVal;
    uint32_t offset = SBL_OSPI_OFFSET_SI;
    bool isNandBootEnabled = BFALSE;
    /* Profile point after Board init Clocks and before OSPI init */
    SBL_ADD_PROFILE_POINT;
    if (BTRUE == gIsNandBootEnable)
    {
        isNandBootEnabled = BTRUE;
    }
    /* Initialization of the driver. */
    SBL_OSPI_Initialize(isNandBootEnabled);

#if defined(SBL_ENABLE_HLOS_BOOT) && (defined(SOC_J721E) || defined(SOC_J7200) || defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2))
    retVal =  SBL_MulticoreImageParse((void *) &offset, SBL_OSPI_OFFSET_SI, pEntry, SBL_SKIP_BOOT_AFTER_COPY);
#else
    /* Profile point after OSPI init and before phy tuning */
    SBL_ADD_PROFILE_POINT;
    retVal =  SBL_MulticoreImageParse((void *) &offset, SBL_OSPI_OFFSET_SI, pEntry, SBL_BOOT_AFTER_COPY);
#endif

    SBL_ospiClose(&boardHandle);

if(isXIPEnable == BTRUE)
{
    #if (defined(SOC_J7200) || defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2))
        /* These fields are reset by Nor_xspiClose but are required for XIP */

        const CSL_ospi_flash_cfgRegs *pRegs = (const CSL_ospi_flash_cfgRegs *)(ospi_cfg.baseAddr);

        CSL_REG32_FINS(&pRegs->RD_DATA_CAPTURE_REG,
                        OSPI_FLASH_CFG_RD_DATA_CAPTURE_REG_SAMPLE_EDGE_SEL_FLD,
                        1);

        CSL_REG32_FINS(&pRegs->RD_DATA_CAPTURE_REG,
                        OSPI_FLASH_CFG_RD_DATA_CAPTURE_REG_DQS_ENABLE_FLD,
                        1);
    #endif
}

#if defined(SBL_HLOS_OWNS_FLASH) && !defined(SBL_USE_MCU_DOMAIN_ONLY)
/* Only put OSPI flash back into SPI mode if we're going to directly boot ATF/U-boot/Linux from SBL */
    SBL_ospiLeaveConfigSPI();
#endif

    return retVal;
}

int32_t SBL_ospiCopyHsmImage(uint8_t** dstAddr, uint32_t srcOffsetAddr, uint32_t numBytes)
{
    int32_t retVal = CSL_PASS;
    Board_flashHandle h = (Board_flashHandle) boardHandle;
    /* In combined boot SBL doesn't load tifs, So SBL doesn't configure OSPI.
       Configure OSPI to load hsm.bin in combined boot */
    if (1 == combinedBootmode)
    {
        /* Init SPI driver */
        OSPI_init();

        /* Get default OSPI cfg */
        OSPI_socGetInitCfg(BOARD_OSPI_DOMAIN, BOARD_OSPI_NOR_INSTANCE, &ospi_cfg);
        ospi_cfg.funcClk = OSPI_MODULE_CLK_200M;
        ospi_cfg.dtrEnable = BTRUE;
        ospi_cfg.phyEnable = BFALSE;
        ospi_cfg.baudRateDiv = 8;
        if (gIsNandBootEnable == BTRUE)
        {
            ospi_cfg.cacheEnable = BTRUE;
        }
        OSPI_socSetInitCfg(BOARD_OSPI_DOMAIN, BOARD_OSPI_NOR_INSTANCE, &ospi_cfg);
#if defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)
        if (gIsNandBootEnable == BTRUE)
        {
            h = Board_flashOpen(BOARD_FLASH_ID_W35N01JWTBAG,
                                BOARD_OSPI_NOR_INSTANCE, NULL);
        }
        else
        {
            h = Board_flashOpen(BOARD_FLASH_ID_S28HS512T,
                                BOARD_OSPI_NOR_INSTANCE, NULL);
        }
#elif defined(SOC_J7200)
        h = Board_flashOpen(BOARD_FLASH_ID_S28HS512T,
                                BOARD_OSPI_NOR_INSTANCE, NULL);
#else
        h = Board_flashOpen(BOARD_FLASH_ID_MT35XU512ABA1G12,
                            BOARD_OSPI_NOR_INSTANCE, NULL);
#endif
        if (h)
        {
            /* Disable PHY pipeline mode */
            CSL_ospiPipelinePhyEnable((const CSL_ospi_flash_cfgRegs *)(ospi_cfg.baseAddr), UFALSE);

            /* Update handle for later use*/
            boardHandle = (void *)h;
        }
        else
        {
            SBL_log(SBL_LOG_ERR, "Board_flashOpen failed in SBL_ospiCopyHsmImage \n");
        }
    }
    else
    {
        uint64_t ospiFunClk = (uint64_t)(OSPI_MODULE_CLK_200M);
        /* OSPI clock has been reconfigured by PM Init API. Set clock to 200MHz to match settings while reading SYSFW. */
        OSPI_configClk(ospiFunClk);
    }

    if (gIsNandBootEnable == BTRUE)
    {
        /* In case of OSPI NAND, first hsm.bin should be copied to OCMC memory and then 
        pointer to OCMC memory should be passed to Sciclient_procBootAuthAndStart() API */
        retVal = Board_flashRead(h, srcOffsetAddr, *dstAddr, numBytes, NULL);
        if (retVal != CSL_PASS)
        {
            SBL_log(SBL_LOG_ERR, "Board_flashRead failed in SBL_ospiCopyHsmImage !! \n");
        }
    }
    else
    {
        /* In case of OSPI NOR, Pointer to OSPI NOR can be directly passed to 
           Sciclient_procBootAuthAndStart() API */
        *dstAddr = (uint8_t *) (ospi_cfg.dataAddr + SBL_OSPI_OFFSET_HSM);
    }

    /* HS/HSFS Device: Check if HSM binary is present or not based on SBL_HSM_HEADER.
       GP Device: The HSM binary is not signed so skip checking SBL_HSM_HEADER.
                  Always copy the image from HSM image offset.
    */
    if (SBL_IsAuthReq() != SBL_NEVER_AUTH_APP)
    {
        /* Check if HSM binary is present or not */
        uint8_t* ptr = (uint8_t *) *dstAddr;
        if (*ptr != SBL_HSM_HEADER)
        {
            retVal = SBL_HSM_IMG_NOT_FOUND;
        }
    }
    return retVal;
}

static void SBL_OSPI_Initialize(bool isNandBootEnabled)
{
    SBL_ospiInit(&boardHandle, isNandBootEnabled);

    /* Initialize the function pointers to parse through the RPRC format. */
    fp_readData = &SBL_OSPI_ReadSectors;
    fp_seek     = &SBL_OSPI_seek;
}

void SBL_SPI_init()
{
    OSPI_init();
}

int32_t SBL_OSPI_ReadSectors(void *dstAddr,
                             void *srcOffsetAddr,
                             uint32_t length)
{
    int32_t ret;
    ret = SBL_ospiFlashRead(&boardHandle, (uint8_t *) dstAddr, length,
        *((uint32_t *) srcOffsetAddr));
    *((uint32_t *) srcOffsetAddr) += length;
    return ret;
}

void SBL_OSPI_seek(void *srcAddr, uint32_t location)
{
    *((uint32_t *) srcAddr) = location;
}

void SBL_enableXIPMode(uint32_t freq)
{
    isXIPEnable = BTRUE;
    ospiFrequency = freq;
}

void SBL_enableNandBoot()
{
    gIsNandBootEnable = BTRUE;
}

