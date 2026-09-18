/**
 *  \file   OSPI_v0.c
 *
 *  \brief  OSPI IP Version 0 specific driver APIs implementation.
 *
 *   This file contains the driver APIs for OSPI controller.
 */

/*
 * Copyright (C) 2017 - 2025 Texas Instruments Incorporated - http://www.ti.com/
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

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <ti/csl/arch/csl_arch.h>
#include <ti/drv/spi/SPI.h>
#include <ti/drv/spi/src/v0/OSPI_v0.h>
#include <ti/drv/spi/src/SPI_osal.h>
#include <ti/csl/src/ip/fss/V0/csl_fss.h>

#define OSPI_DAC_WRITE_TIMEOUT      (500000U)
#define OSPI_CHECK_IDLE_DELAY       (10U)
#define OSPI_CALIBRATE_DELAY        (20U)
#define OSPI_XIP_SETUP_DELAY        (250U)

/* Set the indirect trigger address offset at a non-cached location */
#if defined(SOC_J7200) || defined(SOC_AM64X) || defined(SOC_J721S2)
    #define OSPI_INDAC_TRIG_ADDR (0x3FC0000)
#else
    #define OSPI_INDAC_TRIG_ADDR (0x3FE0000)
#endif

/* OSPI AM57x functions */
static void       OSPI_close_v0(OSPI_Handle handle);
static void       OSPI_init_v0(OSPI_Handle handle);
static OSPI_Handle OSPI_open_v0(OSPI_Handle handle, const OSPI_Params *params);
static bool       OSPI_transfer_v0(OSPI_Handle handle,
                                   OSPI_Transaction *transaction);
static int32_t    OSPI_primeTransfer_v0(OSPI_Handle handle,
                                        OSPI_Transaction *transaction);
static void       OSPI_transferCallback_v0(OSPI_Handle handle,
                                           OSPI_Transaction *msg);
static int32_t    OSPI_control_v0(OSPI_Handle handle, uint32_t cmd, const void *arg);
static void       OSPI_transferCancel_v0(OSPI_Handle handle);
static void       OSPI_hwiFxn_v0(uintptr_t arg);
static int32_t    OSPI_waitIdle(OSPI_Handle handle, uint32_t timeOut);
static int32_t    OSPI_flashExecCmd(const CSL_ospi_flash_cfgRegs *pRegs);
static int32_t    OSPI_read_v0(OSPI_Handle handle, OSPI_Transaction *transaction);
static int32_t    OSPI_cmdRead(const CSL_ospi_flash_cfgRegs *pRegs,
                               uint32_t                      cmd,
                               uint8_t                      *rxBuf,
                               uint32_t                      rxLen);
static int32_t OSPI_cmdExtRead(const CSL_ospi_flash_cfgRegs *pRegs,
                               uint8_t                      *cmd,
                               uint32_t                      cmdLen,
                               uint8_t                      *rxBuf,
                               uint32_t                      rxLen,
                               uint32_t                      dummyCycles);
static int32_t    OSPI_write_v0(OSPI_Handle handle, OSPI_Transaction *transaction);
static int32_t    OSPI_cmdWrite(const CSL_ospi_flash_cfgRegs *pRegs,
                                const uint8_t                *cmdBuf,
                                uint32_t                      cmdLen,
                                const uint8_t                *txBuf,
                                uint32_t                      txLen);
static int32_t    OSPI_waitReadSramLvl(const CSL_ospi_flash_cfgRegs *pReg,
                                       uint32_t *rdLevel);
static int32_t    OSPI_ind_xfer_mode_read_v0(OSPI_Handle handle,
                                             OSPI_Transaction *transaction);
static int32_t    OSPI_waitIndWriteComplete(const CSL_ospi_flash_cfgRegs *pRegs);
static int32_t    OSPI_waitWriteSramLvl(const CSL_ospi_flash_cfgRegs *pReg,
                                        uint32_t *sramLvl);
static int32_t    OSPI_ind_xfer_mode_write_v0(OSPI_Handle handle,
                                              OSPI_Transaction *transaction);
static int32_t    OSPI_dac_xfer_mode_read_v0(OSPI_Handle handle,
                                             const OSPI_Transaction *transaction);
static int32_t    OSPI_dac_xfer_mode_write_v0(OSPI_Handle handle,
                                              const OSPI_Transaction *transaction);
static int32_t    OSPI_cmd_mode_write_v0(OSPI_Handle handle,
                                         const OSPI_Transaction *transaction);
static int32_t    OSPI_cmd_mode_read_v0(OSPI_Handle handle,
                                        OSPI_Transaction *transaction);

typedef enum OSPI_intrPollMode_s
{
    SPI_OPER_MODE_BLOCKING = 0U,  /*! Interrupt based blocking mode */
    SPI_OPER_MODE_POLLING,        /*! Non interrupt based blocking mode */
    SPI_OPER_MODE_CALLBACK        /*! Interrupt based call back mode */
} OSPI_intrPollMode;



/* SPI function table for OSPI AM57x implementation */
const OSPI_FxnTable OSPI_FxnTable_v0 =
{
    &OSPI_close_v0,
    &OSPI_control_v0,
    &OSPI_init_v0,
    &OSPI_open_v0,
    &OSPI_transfer_v0,
    &OSPI_transferCancel_v0,
    NULL
};

/*
 *  ======== OSPI_close_v0 ========
 */
static void OSPI_close_v0(OSPI_Handle handle)
{
    OSPI_v0_Object        *object = NULL;
    OSPI_v0_HwAttrs const *hwAttrs = NULL;

    if (NULL != handle)
    {
        /* Get the pointer to the object and hwAttrs */
        object = (OSPI_v0_Object *)handle->object;
        hwAttrs = (OSPI_v0_HwAttrs const *)handle->hwAttrs;

        /* disable the interrupts */
        CSL_ospiIntrEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                           CSL_OSPI_INTR_MASK_ALL,
                           UFALSE);

        /* Destruct the Hwi */
        if(NULL != object->hwi)
        {
            /* Destruct the Hwi */
            (void)SPI_osalHardwareIntDestruct(object->hwi, hwAttrs->eventId);
            object->hwi = NULL;
        }

        /* Destruct the instance lock */
        (void)SPI_osalDeleteBlockingLock(object->mutex);

        /* Destruct the transfer completion lock */
        if((uint32_t)SPI_OPER_MODE_BLOCKING == object->intrPollMode)
        {
            (void)SPI_osalDeleteBlockingLock(object->transferComplete);
        }

#ifdef SPI_DMA_ENABLE
        if (BTRUE == hwAttrs->dmaEnable)
        {
            OSPI_dmaFreeChannel(handle);
        }
#endif

        /* Open flag is set false */
        object->isOpen = BFALSE;
    }

    return;
}

/*
 *  ======== OSPI_hwiFxn_v0 ========
 *  Hwi interrupt handler to service the OSPI peripheral
 *
 *  The handler is a generic handler for a OSPI object.
 */
static void OSPI_hwiFxn_v0(uintptr_t arg)
{
    uint32_t               intrStatus;
    OSPI_v0_Object        *object = NULL;
    OSPI_v0_HwAttrs const *hwAttrs = NULL;
    uint32_t               sramLevel, rdBytes, wrBytes;

    /* Get the pointer to the object and hwAttrs */
    object = (OSPI_v0_Object *)(((OSPI_Handle)arg)->object);
    hwAttrs = (OSPI_v0_HwAttrs const *)(((OSPI_Handle)arg)->hwAttrs);

    /* Read the interrupt status register */
    intrStatus = CSL_ospiIntrStatus((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr));

    if (NULL_PTR != object->readBufIdx)
    {
		/* Indirect read operation */
        if (0U != (intrStatus & CSL_OSPI_INTR_MASK_IND_XFER))
        {
            if(0U != object->readCountIdx)
            {
                while (BTRUE)
                {
                    sramLevel = CSL_ospiGetSramLvl((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), 1U);
                    if (0U == sramLevel)
                    {
                        break;
                    }
                    rdBytes = sramLevel * CSL_OSPI_FIFO_WIDTH;
                    rdBytes = (rdBytes > object->readCountIdx) ? object->readCountIdx : rdBytes;

                    /* Read data from FIFO */
                    CSL_ospiReadFifoData(hwAttrs->dataAddr + OSPI_INDAC_TRIG_ADDR, object->readBufIdx, rdBytes);

                    object->readBufIdx += rdBytes;
                    object->readCountIdx -= rdBytes;
                }

                if ((0U < object->readCountIdx) &&
                    ((CSL_OSPI_SRAM_WARERMARK_RD_LVL * CSL_OSPI_FIFO_WIDTH) > object->readCountIdx))
                {
                    while(BTRUE)
                    {
                        sramLevel = CSL_ospiGetSramLvl((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), 1U);
                        rdBytes = sramLevel * CSL_OSPI_FIFO_WIDTH;
                        if (rdBytes >= object->readCountIdx)
                        {
                            break;
                        }
                    }
                    rdBytes = object->readCountIdx;
                    CSL_ospiReadFifoData(hwAttrs->dataAddr + OSPI_INDAC_TRIG_ADDR, object->readBufIdx, rdBytes);
                    object->readBufIdx += rdBytes;
                    object->readCountIdx -= rdBytes;
                }
            }

            if((0U == object->readCountIdx) || (0U != (intrStatus & CSL_OSPI_INTR_MASK_IND_OP_DONE)))
            {
                /* Clear indirect read operation complete status */
                CSL_ospiClrIndReadComplete((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr));

                /* disable and clear the interrupts */
                CSL_ospiIntrEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                    CSL_OSPI_INTR_MASK_ALL,
                                    UFALSE);
                CSL_ospiIntrClear((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                  CSL_OSPI_INTR_MASK_ALL);

                /* Call the call back function */
                object->ospiParams.transferCallbackFxn((OSPI_Handle)arg, NULL);
            }
            else
            {
                /* Clear interrupt status */
                CSL_ospiIntrClear((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), intrStatus);
            }
        }
        else
        {
            /* Clear interrupt status */
            CSL_ospiIntrClear((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), intrStatus);
        }
    }
    else
    {
		/* Indirect write operation */
        if (0U != (intrStatus & CSL_OSPI_INTR_MASK_IND_XFER))
        {
            if (0U != object->writeCountIdx)
            {
                sramLevel = CSL_OSPI_SRAM_PARTITION_WR - \
                            CSL_ospiGetSramLvl((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), 0U);

                wrBytes = sramLevel * CSL_OSPI_FIFO_WIDTH;
                wrBytes = (wrBytes > object->writeCountIdx) ? object->writeCountIdx : wrBytes;

                /* Write data to FIFO */
                CSL_ospiWriteFifoData(hwAttrs->dataAddr + OSPI_INDAC_TRIG_ADDR, object->writeBufIdx, wrBytes);

                object->writeBufIdx += wrBytes;
                object->writeCountIdx -= wrBytes;

                sramLevel = CSL_OSPI_SRAM_PARTITION_WR - \
                            CSL_ospiGetSramLvl((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), 0U);

                if ((0U < object->writeCountIdx) &&
                    (object->writeCountIdx <= (sramLevel * CSL_OSPI_FIFO_WIDTH)))
                {
                    wrBytes = object->writeCountIdx;
                    CSL_ospiWriteFifoData(hwAttrs->dataAddr + OSPI_INDAC_TRIG_ADDR, object->writeBufIdx, wrBytes);
                    object->writeBufIdx += wrBytes;
                    object->writeCountIdx -= wrBytes;
                }
            }

            if (0U != (intrStatus & CSL_OSPI_INTR_MASK_IND_OP_DONE))
            {
                /* Clear indirect write operation complete status */
                CSL_ospiClrIndWriteComplete((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr));

                /* disable and clear the interrupts */
                CSL_ospiIntrEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                    CSL_OSPI_INTR_MASK_ALL,
                                    UFALSE);
                CSL_ospiIntrClear((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                  CSL_OSPI_INTR_MASK_ALL);

                /* Call the call back function */
                object->ospiParams.transferCallbackFxn((OSPI_Handle)arg, NULL);
            }
            else
            {
                /* Clear interrupt status */
                CSL_ospiIntrClear((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), intrStatus);
            }
        }
        else
        {
            /* Clear interrupt status */
            CSL_ospiIntrClear((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), intrStatus);
        }
    }
}

/*
 *  ======== OSPI_init_v0 ========
 */
static void OSPI_init_v0(OSPI_Handle handle)
{
    if (NULL != handle)
    {
        /* Mark the object as available */
        ((OSPI_v0_Object *)(handle->object))->isOpen = BFALSE;
    }
}

static void OSPI_delay(uint32_t delay);
static void OSPI_delay(uint32_t delay)
{
    volatile uint32_t i = delay;

    while (i > 0U)
    {
        i = i - 1U;
    }
}

static int32_t OSPI_waitIdle(OSPI_Handle handle, uint32_t timeOut)
{
    OSPI_v0_HwAttrs const *hwAttrs;      /* OSPI hardware attributes */
    uint32_t               retry = 0U;
    int32_t                retVal = SPI_STATUS_ERROR;
    uint32_t               timeOutVal = timeOut;

    /* Get the pointer to the object and hwAttrs */
    hwAttrs = (OSPI_v0_HwAttrs const *)handle->hwAttrs;

    while (0U != timeOutVal)
    {
        if (0U != CSL_ospiIsIdle((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr)))
        {
            retry++;
            if (3U == retry)
            {
                retVal = SPI_STATUS_SUCCESS;
                break;
            }
        }
        else
        {
            retry = 0U;
        }
        OSPI_delay(OSPI_CHECK_IDLE_DELAY);
        timeOutVal--;
    }

    return (retVal);
}

/*
 *  ======== OSPI_open_v0 ========
 */
static OSPI_Handle OSPI_open_v0(OSPI_Handle handle, const OSPI_Params *params)
{
    SemaphoreP_Params        semParams;
    uintptr_t                key;
    OSPI_Handle              retHandle = handle;
    OSPI_v0_Object          *object = NULL;
    OSPI_v0_HwAttrs const   *hwAttrs = NULL;
	OsalRegisterIntrParams_t interruptRegParams;
    uint32_t                 retFlag = UFALSE;
    uint32_t                 numAddrBytes;

    if (NULL != handle)
    {
        /* Get the pointer to the object and hwAttrs */
        object = (OSPI_v0_Object *)handle->object;
        hwAttrs = (OSPI_v0_HwAttrs const *)handle->hwAttrs;

        /* Determine if the device index was already opened */
        key = SPI_osalHardwareIntDisable();
    }

    if ((NULL == handle) || (BTRUE == object->isOpen))
    {
        if (NULL != handle)
        {
            SPI_osalHardwareIntRestore(key);
            retHandle = NULL;
        }
    }
    else
    {
        /* Mark the handle as being used */
        object->isOpen = BTRUE;
        SPI_osalHardwareIntRestore(key);

        /* Store the OSPI parameters */
        if (NULL == params) {
            /* No params passed in, so use the defaults */
            OSPI_Params_init(&(object->ospiParams));
        }
        else {
            /* Copy the params contents */
            object->ospiParams = *params;
        }

        /* Copy the controller mode from hardware attributes to object */
        object->ospiMode  = (uint32_t)hwAttrs->operMode;
        object->xferLines = (uint32_t)hwAttrs->xferLines;
        object->hwi       = NULL;
        object->extRdCmdLen = 0U;

        /* Extract OSPI operating mode based on hwAttrs and input parameters */
        if(OSPI_MODE_BLOCKING == object->ospiParams.transferMode)
        {
            if ((BTRUE == hwAttrs->intrEnable) && (BFALSE == hwAttrs->dacEnable))
            {
                /* interrupt is only used in indirect access mode */
                object->intrPollMode = (uint32_t)SPI_OPER_MODE_BLOCKING;
            }
#ifdef SPI_DMA_ENABLE
            else if ((BTRUE == hwAttrs->dmaEnable) && (BTRUE == hwAttrs->dacEnable))
            {
                /* DMA mode is only used in direct access mode */
                object->intrPollMode = (uint32_t)SPI_OPER_MODE_BLOCKING;
            }
#endif
            else
            {
                object->intrPollMode = (uint32_t)SPI_OPER_MODE_POLLING;
            }
        }
        else
        {
            object->intrPollMode = (uint32_t)SPI_OPER_MODE_CALLBACK;
        }

        /* interrupt mode only enabled in indirect access controller mode */
        if ((BTRUE == hwAttrs->intrEnable) && (BFALSE == hwAttrs->dacEnable))
        {
            Osal_RegisterInterrupt_initParams(&interruptRegParams);

            interruptRegParams.corepacConfig.name = NULL;
#if ((__ARM_ARCH == 7) && (__ARM_ARCH_PROFILE == 'R') && defined(__ARM_FEATURE_IDIV))
            interruptRegParams.corepacConfig.priority = 0x8U;
#else
            interruptRegParams.corepacConfig.priority = 0x20U;
#endif
            interruptRegParams.corepacConfig.corepacEventNum = hwAttrs->eventId;
            interruptRegParams.corepacConfig.intVecNum = hwAttrs->intrNum; /* Host Interrupt vector */
            interruptRegParams.corepacConfig.isrRoutine = (void (*)(uintptr_t))(&OSPI_hwiFxn_v0);
            interruptRegParams.corepacConfig.arg = (uintptr_t)handle;

            (void)SPI_osalRegisterInterrupt(&interruptRegParams,&(object->hwi));
            if (NULL == object->hwi) {
                OSPI_close_v0(handle);
                retHandle = NULL;
                retFlag = UTRUE;
            }
        }

        if(UFALSE == retFlag)
        {
            /*
             * Construct thread safe handles for this OSPI peripheral
             * Semaphore to provide exclusive access to the OSPI peripheral
             */
            SPI_osalSemParamsInit(&semParams);
            semParams.mode = SemaphoreP_Mode_BINARY;
            object->mutex = SPI_osalCreateBlockingLock(1U, &semParams);

            /*
             * Store a callback function that posts the transfer complete
             * semaphore for synchronous mode
             */
            if ((uint32_t)SPI_OPER_MODE_BLOCKING == object->intrPollMode)
            {
                /*
                 * Semaphore to cause the waiting task to block for the OSPI
                 * to finish
                 */
                object->transferComplete = SPI_osalCreateBlockingLock(0, &semParams);

                /* Store internal callback function */
                object->ospiParams.transferCallbackFxn = &OSPI_transferCallback_v0;
            }
            if((uint32_t)SPI_OPER_MODE_CALLBACK == object->intrPollMode)
            {
                /* Check to see if a callback function was defined for async mode */
                if (NULL == object->ospiParams.transferCallbackFxn)
                {
                    OSPI_close_v0(handle);
                    retHandle = NULL;
                    retFlag = UTRUE;
                }
            }

            if(UFALSE == retFlag)
            {
                object->transaction = NULL;

    #ifdef SPI_DMA_ENABLE
                if (BTRUE == hwAttrs->dmaEnable)
                {
                    (void)OSPI_dmaConfig(handle);
                }
    #endif
                /* Disable DAC */
                CSL_ospiDacEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), UFALSE);

                /* Disable DTR protocol */
                CSL_ospiDtrEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), UFALSE);

                if (BFALSE == hwAttrs->xipEnable)
                {
                    /* Disable XIP */
                    CSL_ospiXipEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), UFALSE);
                }

                /* Disable OSPI controller */
                CSL_ospiEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), UFALSE);

                /* Wait until Serial Interface and OSPI pipeline is IDLE. */
                if (SPI_STATUS_SUCCESS != OSPI_waitIdle(handle, OSPI_DAC_WRITE_TIMEOUT))
                {
                    OSPI_close_v0(handle);
                    retHandle = NULL;
                    retFlag = UTRUE;
                }
            }
        }

        if(UFALSE == retFlag)
        {
            /* Chip select setup */
            CSL_ospiSetChipSelect((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                  hwAttrs->chipSelect,
                                  CSL_OSPI_DECODER_SELECT4);

            /* Set clock mode */
            CSL_ospiSetClkMode((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                               hwAttrs->frmFmt);

            /* Disable the adapted loopback clock circuit */
            CSL_ospiLoopbackClkEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                      UFALSE);

            /* Delay Setup */
            CSL_ospiSetDevDelay((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                hwAttrs->devDelays);

            if (hwAttrs->baudRateDiv)
            {
                CSL_ospiSetPreScaler((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                     CSL_OSPI_BAUD_RATE_DIVISOR(hwAttrs->baudRateDiv));
            }
            else
            {
                /* Set default baud rate divider value */
                if (hwAttrs->phyEnable)
                {
                    CSL_ospiSetPreScaler((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                         CSL_OSPI_BAUD_RATE_DIVISOR(2U));
                }
                else
                {
                    /* Disable high speed mode when PHY is disabled */
                    CSL_ospiSetPreScaler((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                         CSL_OSPI_BAUD_RATE_DIVISOR_DEFAULT);
                }
            }

            if (hwAttrs->phyEnable)
            {
                /* Enable PHY mode */
                CSL_ospiPhyEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), UTRUE);
            }
            else
            {
                /* Disable PHY mode */
                CSL_ospiPhyEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), UFALSE);
            }
            /* Disable PHY pipeline mode */
            CSL_ospiPipelinePhyEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), UFALSE);

            /* numAddrBytes holds the explicit byte count (1-4) set in the SOC
             * config or overridden by the flash driver. The CSL register field
             * encodes (byte count - 1). If numAddrBytes is 0 (not set by SOC
             * config — older SOCs), fall back to dtrEnable-based selection. */
            if (0U != hwAttrs->numAddrBytes)
            {
                numAddrBytes = hwAttrs->numAddrBytes - 1U;
            }
            else if (hwAttrs->dtrEnable)
            {
                numAddrBytes = CSL_OSPI_MEM_MAP_NUM_ADDR_BYTES_4;
            }
            else
            {
                numAddrBytes = CSL_OSPI_MEM_MAP_NUM_ADDR_BYTES_3;
            }
            /* Set device size cofigurations */
            CSL_ospiSetDevSize((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                               numAddrBytes,
                               hwAttrs->pageSize,
                               hwAttrs->blkSize);

            /* Set indirect trigger address register */
            if (hwAttrs->dacEnable)
            {
                CSL_ospiSetIndTrigAddr((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                        0x4000000);
            }
            else
            {
                CSL_ospiSetIndTrigAddr((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                       OSPI_INDAC_TRIG_ADDR);
            }

        #if defined(SOC_J721E)
            /* Enable write completion auto polling */
            CSL_ospiSetWrCompAutoPolling((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                         CSL_OSPI_WRITE_COMP_AUTO_POLLING_ENABLE);
        #else
            /* Disable write completion auto polling */
            CSL_ospiSetWrCompAutoPolling((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                         CSL_OSPI_WRITE_COMP_AUTO_POLLING_DISABLE);
        #endif

            /* Set SRAM partition configuration */
            CSL_ospiSetSramPartition((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                     CSL_OSPI_SRAM_PARTITION_DEFAULT);

            /* disable and clear the interrupts */
            CSL_ospiIntrEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                CSL_OSPI_INTR_MASK_ALL,
                                UFALSE);
            CSL_ospiIntrClear((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                              CSL_OSPI_INTR_MASK_ALL);

            if (hwAttrs->dacEnable)
            {
                CSL_ospiDacEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), UTRUE);
            }
            else
            {
                CSL_ospiDacEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), UFALSE);
            }

            CSL_ospiSetDataReadCapDelay((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                        hwAttrs->rdDataCapDelay);
            CSL_ospiSetCsSotDelay((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                  hwAttrs->csSotDelay);
            /* Enable OSPI controller */
            CSL_ospiEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), UTRUE);
        }
    }
    return(retHandle);
}

/*
 *  ======== OSPI_primeTransfer_v0 =======
 */
static int32_t OSPI_primeTransfer_v0(OSPI_Handle handle,
                                     OSPI_Transaction *transaction)
{
    OSPI_v0_Object        *object = NULL;
    OSPI_v0_HwAttrs const *hwAttrs = NULL;
    int32_t                retVal = SPI_STATUS_SUCCESS;

    /* Get the pointer to the object and hwAttrs */
    object = (OSPI_v0_Object *)handle->object;
    hwAttrs = (OSPI_v0_HwAttrs const *)handle->hwAttrs;

    /* Disable and clear the interrupts */
    CSL_ospiIntrEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                        CSL_OSPI_INTR_MASK_ALL,
                        UFALSE);
    CSL_ospiIntrClear((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                      CSL_OSPI_INTR_MASK_ALL);

    /* Interrupt mode */
    if((uint32_t)SPI_OPER_MODE_POLLING != object->intrPollMode)
    {
        CSL_ospiIntrEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                           CSL_OSPI_INTR_MASK_ALL,
                           UTRUE);
    }

    /* Identify the direction of transfer (whether read/write) */
    if((uint32_t)SPI_TRANSACTION_TYPE_READ == object->transactionType)
    {
        retVal = (OSPI_read_v0(handle, transaction));
    }
    else if((uint32_t)SPI_TRANSACTION_TYPE_WRITE == object->transactionType)
    {
        retVal = (OSPI_write_v0(handle, transaction));
    }
    else
    {
        transaction->status = OSPI_TRANSFER_CANCELED;
		retVal = SPI_STATUS_ERROR;
    }
    return(retVal);
}

static int32_t OSPI_waitReadSramLvl(const CSL_ospi_flash_cfgRegs *pReg,
                                    uint32_t *rdLevel)
{
    uint32_t retry = OSPI_DAC_WRITE_TIMEOUT;
    uint32_t sramLevel;
    int32_t  retVal = SPI_STATUS_SUCCESS;
    while(0U != retry)
    {
        sramLevel = CSL_ospiGetSramLvl(pReg, 1U);
        if (0U != sramLevel)
        {
            *rdLevel = sramLevel;
            break;
        }
        OSPI_delay(OSPI_CHECK_IDLE_DELAY);
        retry--;
    }

    if (0U != retry)
    {
        retVal = SPI_STATUS_SUCCESS;
    }
    else
    {
        retVal = SPI_STATUS_ERROR;
    }
    return(retVal);
}

static int32_t OSPI_waitIndReadComplete(const CSL_ospi_flash_cfgRegs *pRegs)
{
    uint32_t retry = OSPI_DAC_WRITE_TIMEOUT;
    int32_t  retVal;

    /* Check flash indirect read controller status */
    while (0U != retry)
    {
        if (UTRUE == CSL_ospiIndReadComplete(pRegs))
        {
            break;
        }
        OSPI_delay(OSPI_CHECK_IDLE_DELAY);
        retry--;
    }

    if (0U != retry)
    {
        /* Clear indirect completion status */
        CSL_ospiClrIndReadComplete(pRegs);
        retVal = SPI_STATUS_SUCCESS;
    }
    else
    {
        retVal = SPI_STATUS_ERROR;
    }
    return(retVal);
}

static int32_t OSPI_ind_xfer_mode_read_v0(OSPI_Handle handle,
                                          OSPI_Transaction *transaction)
{
    OSPI_v0_HwAttrs const *hwAttrs;      /* OSPI hardware attributes */
    OSPI_v0_Object        *object;       /* OSPI object */
    uint8_t               *pDst;         /* Destination address */
    uint32_t               count;        /* transaction length */
    uint32_t               offset;       /* OSPI flash offset address */
    uint32_t               remaining;
    uint32_t               flashcount;
    uint32_t               sramLevel = 0U, rdBytes = 0U;
    uint32_t               retFlag = UFALSE;
    int32_t                retVal = SPI_STATUS_SUCCESS;

    /* Copy flash transaction parameters to local variables */
    offset = (uint32_t)((uintptr_t)transaction->arg); /* NOR Flash offset address to read */
    pDst = (uint8_t *)transaction->rxBuf;
    count = (uint32_t)transaction->count;

    flashcount = count;
    if(count % 2 != 0){
        flashcount++;
    }

    /* Get the pointer to the object and hwAttrs */
    hwAttrs = (OSPI_v0_HwAttrs const *)handle->hwAttrs;
    object = (OSPI_v0_Object *)handle->object;

    /* Disable DAC mode */
    CSL_ospiDacEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), UFALSE);

    if (BTRUE == hwAttrs->phyEnable)
    {
        /* Enable PHY pipeline mode for read */
        CSL_ospiPipelinePhyEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), UTRUE);
    }

    /* Set read address in indirect mode */
    CSL_ospiIndSetStartAddr((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                            offset,
                            UTRUE);
    CSL_ospiIndReadExecute((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), flashcount);

    if ((uint32_t)SPI_OPER_MODE_POLLING == object->intrPollMode)
    {
        remaining = count;
        while(0U < remaining)
        {
            /* Wait indirect read SRAM fifo has data */
            if (SPI_STATUS_SUCCESS != OSPI_waitReadSramLvl((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                     &sramLevel))
            {
                retFlag = UTRUE;
                retVal = SPI_STATUS_ERROR;
                transaction->status = OSPI_TRANSFER_FAILED;
                break;
            }

            rdBytes = sramLevel * CSL_OSPI_FIFO_WIDTH;
            rdBytes = (rdBytes > remaining) ? remaining : rdBytes;

            /* Read data from FIFO */
            CSL_ospiReadFifoData(hwAttrs->dataAddr + OSPI_INDAC_TRIG_ADDR, pDst, rdBytes);

            pDst += rdBytes;
            remaining -= rdBytes;
        }
        if(UFALSE == retFlag)
        {
            if(SPI_STATUS_SUCCESS != OSPI_waitIndReadComplete((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr)))
            {
                retFlag = UTRUE;
                retVal = SPI_STATUS_ERROR;
                transaction->status = OSPI_TRANSFER_FAILED;
            }
        }
    }
#if defined (__aarch64__) || ((__ARM_ARCH == 7) && (__ARM_ARCH_PROFILE == 'R'))
    CSL_archMemoryFence();
#endif

    return (retVal);
}

static uint8_t OSPI_getDeviceStatus(OSPI_Handle handle);
static uint8_t OSPI_getDeviceStatus(OSPI_Handle handle)
{
    OSPI_v0_HwAttrs const *hwAttrs; /* OSPI hardware attributes */
    OSPI_v0_Object        *object;  /* OSPI object */
    uint8_t                status = 0xFF;
    uint8_t                cmd[3];
    uint32_t               rx_lines;

    hwAttrs = (OSPI_v0_HwAttrs const *)handle->hwAttrs;
    object = (OSPI_v0_Object *)handle->object;
    rx_lines = hwAttrs->xferLines;

    if(object->extRdCmdLen)
    {
        if (OSPI_XFER_LINES_OCTAL == rx_lines)
        {
            cmd[0] = object->rdStatusCmd;
            cmd[1] = object->rdStatusAddr;
            cmd[2] = object->rdStatusAddr;
            OSPI_cmdExtRead((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                            (uint8_t *)cmd,
                            3,
                            &status,
                            1,
                            7);
        }
        else
        {
            cmd[0] = object->rdStatusCmd;
            cmd[1] = object->rdStatusAddr;
            OSPI_cmdExtRead((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                            (uint8_t *)cmd,
                            2,
                            &status,
                            1,
                            0);
        }
    }
    else
    {
        cmd[0] = object->rdStatusCmd;
        OSPI_cmdRead((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                    (uint32_t)cmd[0],
                    &status,
                    1);
    }

    return (status);
}

static bool OSPI_waitDeviceReady(OSPI_Handle handle, uint32_t timeOut);
static bool OSPI_waitDeviceReady(OSPI_Handle handle, uint32_t timeOut)
{
    bool         retVal = BFALSE;
    uint8_t      status;
    uint32_t     timeOutVal = timeOut;

    while (0U != timeOutVal)
    {
        status = OSPI_getDeviceStatus(handle);
        if (0U == (status & 1U))
        {
            retVal = BTRUE;
            break;
        }
        timeOutVal--;
        OSPI_delay(OSPI_CHECK_IDLE_DELAY);
    }
    return (retVal);
}

static int32_t OSPI_dac_xfer_mode_read_v0(OSPI_Handle handle,
                                          const OSPI_Transaction *transaction)
{
    OSPI_v0_HwAttrs const *hwAttrs;      /* OSPI hardware attributes */
    uint8_t               *pSrc;         /* Source address */
    uint8_t               *pDst;         /* Destination address */
    uint32_t               offset;       /* OSPI flash offset address */
    uint32_t               i;
    uint32_t               size;
    uint32_t               remainSize;

    /* Copy flash transaction parameters to local variables */
    offset = (uint32_t)((uintptr_t)transaction->arg); /* OSPI Flash offset address to read */
    pDst = (uint8_t *)transaction->rxBuf;

    /* Get the pointer to the object and hwAttrs */
    hwAttrs = (OSPI_v0_HwAttrs const *)handle->hwAttrs;

#ifdef SPI_DMA_ENABLE
    if (BTRUE == hwAttrs->dmaEnable)
    {
        if (BTRUE == hwAttrs->phyEnable)
        {
            /* Enable PHY pipeline mode for read */
            CSL_ospiPipelinePhyEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), UTRUE);
        }
        OSPI_dmaTransfer(handle, transaction);
    }
    else
#endif
    {
        if (BTRUE == hwAttrs->cacheEnable)
        {
            if (BTRUE == hwAttrs->phyEnable)
            {
                /* Enable PHY pipeline mode for read */
                CSL_ospiPipelinePhyEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), UTRUE);
            }
        }
        pSrc = (uint8_t *)(hwAttrs->dataAddr + offset);
        remainSize = (uint32_t)transaction->count & 3U;
        size = (uint32_t)transaction->count - remainSize;
        /* Transfer the data in 32-bit size */
        for (i = 0U; i < size; i += 4U)
        {
            CSL_REG32_WR(pDst + i, CSL_REG32_RD(pSrc + i));
#if defined (__aarch64__)
            CSL_archMemoryFence();
#endif
        }
        /* Transfer the remaining data in 8-bit size */
        for (i = 0U; i < remainSize; i++)
        {
            CSL_REG8_WR(pDst + size + i, CSL_REG8_RD(pSrc + size + i));
#if defined (__aarch64__)
            CSL_archMemoryFence();
#endif
        }
        if (BTRUE == hwAttrs->cacheEnable)
        {
            CacheP_wbInv((void *)(hwAttrs->dataAddr + offset), transaction->count);
        }
    }

    return (SPI_STATUS_SUCCESS);
}

static int32_t OSPI_flashExecCmd(const CSL_ospi_flash_cfgRegs *pRegs)
{
    uint32_t retry = OSPI_DAC_WRITE_TIMEOUT;
    int32_t  retVal = SPI_STATUS_SUCCESS;
    uint32_t idleFlag = UFALSE;

    while (UFALSE == idleFlag)
    {
        idleFlag = CSL_ospiIsIdle(pRegs);
    }

    /* Start to execute flash read/write command */
    CSL_ospiFlashExecCmd(pRegs);

    while (0U != retry)
    {
        /* Check the command execution status */
        if (UTRUE == CSL_ospiFlashExecCmdComplete(pRegs))
        {
            break;
        }
        OSPI_delay(OSPI_CHECK_IDLE_DELAY);
        retry--;
    }

    if (0U == retry)
    {
        retVal = SPI_STATUS_ERROR;
    }

    idleFlag = UFALSE;
    while (UFALSE == idleFlag)
    {
        idleFlag = CSL_ospiIsIdle(pRegs);
    }

    return (retVal);
}

static int32_t OSPI_cmdRead(const CSL_ospi_flash_cfgRegs *pRegs,
                            uint32_t                      cmd,
                            uint8_t                      *rxBuf,
                            uint32_t                      rxLen)
{
    uint32_t regVal;
    uint32_t rdLen;
    int32_t  retVal;
    uint8_t *pBuf = rxBuf;

    (void)CSL_ospiCmdRead(pRegs, cmd, rxLen);
    retVal = OSPI_flashExecCmd(pRegs);
    if (SPI_STATUS_SUCCESS == retVal)
    {
        regVal = CSL_REG32_RD(&pRegs->FLASH_RD_DATA_LOWER_REG);

        /* Put the read value into rxBuf */
        rdLen = (rxLen > 4U) ? 4U : rxLen;
        (void)memcpy((void *)pBuf, (void *)(&regVal), rdLen);
        pBuf += rdLen;

        if (rxLen > 4U) {
            regVal = CSL_REG32_RD(&pRegs->FLASH_RD_DATA_UPPER_REG);
            rdLen = rxLen - rdLen;
            (void)memcpy((void *)pBuf, (void *)(&regVal), rdLen);
        }
    }
    return (retVal);
}

static int32_t OSPI_cmdExtRead(const CSL_ospi_flash_cfgRegs *pRegs,
                               uint8_t                      *cmd,
                               uint32_t                     cmdLen,
                               uint8_t                      *rxBuf,
                               uint32_t                     rxLen,
                               uint32_t                     dummyCycles)
{
    uint32_t regVal;
    uint32_t rdLen;
    int32_t  retVal;
    uint8_t *pBuf = rxBuf;

    (void)CSL_ospiCmdExtRead(pRegs, cmd, cmdLen, rxLen, dummyCycles);
    retVal = OSPI_flashExecCmd(pRegs);
    if (SPI_STATUS_SUCCESS == retVal)
    {
        regVal = CSL_REG32_RD(&pRegs->FLASH_RD_DATA_LOWER_REG);

        /* Put the read value into rxBuf */
        rdLen = (rxLen > 4U) ? 4U : rxLen;
        (void)memcpy((void *)pBuf, (void *)(&regVal), rdLen);
        pBuf += rdLen;

        if (rxLen > 4U) {
            regVal = CSL_REG32_RD(&pRegs->FLASH_RD_DATA_UPPER_REG);
            rdLen = rxLen - rdLen;
            (void)memcpy((void *)pBuf, (void *)(&regVal), rdLen);
        }
    }
    return (retVal);
}

static int32_t OSPI_cmd_mode_read_v0(OSPI_Handle handle,
                                     OSPI_Transaction *transaction)
{
    OSPI_v0_HwAttrs const *hwAttrs;    /* OSPI hardware attributes */
    OSPI_v0_Object        *object;  /* OSPI object */
    uint8_t               *cmd;
    int32_t               retVal;

    cmd = (uint8_t *)transaction->txBuf;
    /* Get the pointer to the hwAttrs */
    hwAttrs = (OSPI_v0_HwAttrs const *)handle->hwAttrs;
    object  = (OSPI_v0_Object *)handle->object;

    if(object->extRdCmdLen)
    {
        retVal = OSPI_cmdExtRead((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                 (uint8_t *)cmd,
                                 (uint32_t)object->extRdCmdLen,
                                 (uint8_t *)transaction->rxBuf,
                                 (uint32_t)transaction->count - object->extRdCmdLen,
                                 (uint32_t)object->extRdDummyClks);
    }
    else if(CSL_ospiGetDualByteOpcodeMode((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr)))
    {
        retVal = OSPI_cmdExtRead((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                 (uint8_t *)cmd,
                                 (uint32_t)CSL_OSPI_CMD_LEN_EXTENDED,
                                 (uint8_t *)transaction->rxBuf,
                                 (uint32_t)transaction->count - CSL_OSPI_CMD_LEN_EXTENDED,
                                 (uint32_t)object->extRdDummyClks);
    }
    else
    {
        retVal = OSPI_cmdRead((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                              (uint32_t)cmd[0],
                              (uint8_t *)transaction->rxBuf,
                              (uint32_t)transaction->count - CSL_OSPI_CMD_LEN_DEFAULT);
    }

    return (retVal);
}


static int32_t OSPI_read_v0(OSPI_Handle handle, OSPI_Transaction *transaction)
{
    OSPI_v0_Object *object;
    int32_t         retVal = SPI_STATUS_ERROR;

    object = (OSPI_v0_Object *)handle->object;

    if((uint32_t)OSPI_OPER_MODE_IND_XFER == object->ospiMode)
    {
        retVal = (OSPI_ind_xfer_mode_read_v0(handle, transaction));
    }
    else if((uint32_t)OSPI_OPER_MODE_DAC_XFER == object->ospiMode)
    {
        retVal = (OSPI_dac_xfer_mode_read_v0(handle, transaction));
    }
    else if((uint32_t)OSPI_OPER_MODE_CFG == object->ospiMode)
    {
        retVal = (OSPI_cmd_mode_read_v0(handle, transaction));
    }
    else
    {
        transaction->status = OSPI_TRANSFER_CANCELED;
    }

    return(retVal);
}

static int32_t OSPI_waitWriteSramLvl(const CSL_ospi_flash_cfgRegs *pReg,
                                     uint32_t *sramLvl)
{
    uint32_t retry = CSL_OSPI_REG_RETRY;
    uint32_t sramLevel;
    int32_t  retVal = SPI_STATUS_SUCCESS;

    while(0U != retry)
    {
        sramLevel = CSL_ospiGetSramLvl(pReg, 0);
        if (sramLevel <= CSL_OSPI_SRAM_WATERMARK_WR_LVL)
        {
			*sramLvl = sramLevel;
            break;
        }
        (void)SPI_osalDelay(CSL_OSPI_POLL_IDLE_DELAY);
        retry--;
    }

    if (0U != retry)
    {
        retVal = SPI_STATUS_SUCCESS;
    }
    else
    {
        retVal = SPI_STATUS_ERROR;
    }
    return(retVal);
}

static int32_t OSPI_waitIndWriteComplete(const CSL_ospi_flash_cfgRegs *pRegs)
{
    uint32_t retry = OSPI_DAC_WRITE_TIMEOUT;
    int32_t  retVal;

    /* Check flash indirect write controller status */
    while (0U != retry)
    {
        if (UTRUE == CSL_ospiIsIndWriteComplete(pRegs))
        {
            break;
        }
        OSPI_delay(OSPI_CHECK_IDLE_DELAY);
        retry--;
    }

    if (0U != retry)
    {
        /* Clear indirect completion status */
        CSL_ospiClrIndWriteComplete(pRegs);
        retVal = SPI_STATUS_SUCCESS;
    }
    else
    {
        retVal = SPI_STATUS_ERROR;
    }
    return(retVal);
}

static int32_t OSPI_ind_xfer_mode_write_v0(OSPI_Handle handle,
                                           OSPI_Transaction *transaction)
{
    OSPI_v0_HwAttrs const *hwAttrs;      /* OSPI hardware attributes */
    OSPI_v0_Object        *object;       /* OSPI object */
    uint8_t               *pSrc;         /* Destination address */
    uint32_t               count;        /* transaction length */
    uint32_t               offset;       /* OSPI flash offset address */
    uint32_t               remaining;
    uint32_t               sramLevel, wrBytes;
    uint32_t               retFlag = UFALSE;
    int32_t                retVal = SPI_STATUS_SUCCESS;

    object = (OSPI_v0_Object *)handle->object;

    /* Copy flash transaction parameters to local variables */
    offset = (uint32_t)((uintptr_t)transaction->arg); /* NOR Flash offset address to write */
    pSrc = (uint8_t*)transaction->txBuf;
    count = (uint32_t)transaction->count;

    /* Get the pointer to the object and hwAttrs */
    hwAttrs = (OSPI_v0_HwAttrs const *)handle->hwAttrs;
    object = (OSPI_v0_Object *)handle->object;

    /* Disable DAC mode */
    CSL_ospiDacEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), UFALSE);

    /* Set write address in indirect mode */
    CSL_ospiIndSetStartAddr((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                            offset,
                            UFALSE);
    CSL_ospiIndWriteExecute((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), count);

    if ((uint32_t)SPI_OPER_MODE_POLLING == object->intrPollMode)
    {
        /* Wait Indirect Write  SRAM fill level below the threshold */
        if (SPI_STATUS_SUCCESS != OSPI_waitWriteSramLvl((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                  &sramLevel))
        {
            retFlag = UTRUE;
            retVal = (int32_t)(-1);
            transaction->status = OSPI_TRANSFER_FAILED;
        }
        else
        {
            remaining = count;
            while(0U < remaining)
            {
                /* Wait indirect write SRAM fifo level below watermark */
                if (SPI_STATUS_SUCCESS != OSPI_waitWriteSramLvl((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                          &sramLevel))
                {
                    retFlag = UTRUE;
                    retVal = SPI_STATUS_ERROR;
                    break;
                }

                wrBytes = (CSL_OSPI_SRAM_PARTITION_WR - sramLevel) * CSL_OSPI_FIFO_WIDTH;
                wrBytes = (wrBytes > remaining) ? remaining : wrBytes;

                /* Write data to FIFO */
                CSL_ospiWriteFifoData(hwAttrs->dataAddr + OSPI_INDAC_TRIG_ADDR, pSrc, wrBytes);

                pSrc += wrBytes;
                remaining -= wrBytes;
            }

            if(UFALSE == retFlag)
            {
                if (OSPI_waitIndWriteComplete((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr)) != (int32_t)0U)
                {
                    retFlag = UTRUE;
                    retVal = SPI_STATUS_ERROR;
                }
            }
        }
    }

    if(UTRUE == retFlag)
    {
        transaction->status = OSPI_TRANSFER_FAILED;
        CSL_ospiIndWriteCancel((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr));
    }
    return (retVal);
}

static int32_t OSPI_dac_xfer_mode_write_v0(OSPI_Handle handle,
                                           const OSPI_Transaction *transaction)
{
    OSPI_v0_HwAttrs const *hwAttrs;      /* OSPI hardware attributes */
    uint8_t               *pSrc;         /* Source address */
    uint32_t               offset;       /* OSPI flash offset address */
    uint32_t               i;
    uint8_t               *pDst;         /* Destination address */
    uint32_t               size;
    uint32_t               remainSize;
    int32_t                retVal = SPI_STATUS_SUCCESS;
    uint32_t               wrWord;
    uint32_t               wrByte;

    /* Copy flash transaction parameters to local variables */
    offset = (uint32_t)((uintptr_t)transaction->arg); /* OSPI Flash offset address to write */
    pSrc = (uint8_t *)transaction->txBuf;

    /* Get the pointer to the object and hwAttrs */
    hwAttrs = (OSPI_v0_HwAttrs const *)handle->hwAttrs;

    /* Disable PHY pipeline mode */
    CSL_ospiPipelinePhyEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), UFALSE);

#ifdef SPI_DMA_ENABLE
    if (BTRUE == hwAttrs->dmaEnable)
    {
        OSPI_dmaTransfer(handle, transaction);
    }
    else
#endif
    {
        pDst = (uint8_t *)(hwAttrs->dataAddr + offset);
        remainSize = (uint32_t)transaction->count & 3U;
        size = (uint32_t)transaction->count - remainSize;
        /* Transfer the data in 32 bit size (R5 word size) to ensure that chip select does not drop during writes. */
        for (i = 0U; i < size; i += 4U)
        {
            wrWord = CSL_REG32_RD(pSrc + i);
            CSL_REG32_WR(pDst + i, wrWord);
            if (OSPI_waitDeviceReady(handle, OSPI_DAC_WRITE_TIMEOUT))
            {
                retVal = SPI_STATUS_SUCCESS;
            }
            else
            {
                retVal = SPI_STATUS_ERROR;
            }
        }
        if (SPI_STATUS_SUCCESS == retVal)
        {
            /* Transfer the remaining data in 8-bit size */
            for (i = 0U; i < remainSize; i++)
            {
                wrByte = CSL_REG8_RD(pSrc + size + i);
                CSL_REG8_WR(pDst + size + i, wrByte);
                if (OSPI_waitDeviceReady(handle, OSPI_DAC_WRITE_TIMEOUT))
                {
                    retVal = SPI_STATUS_SUCCESS;
                }
                else
                {
                    retVal = SPI_STATUS_ERROR;
                }
            }
        }
    }
    if (BTRUE == hwAttrs->cacheEnable)
    {
        CacheP_wbInv((void *)(hwAttrs->dataAddr + offset), transaction->count);
    }
    return (retVal);
}

static int32_t OSPI_cmdWrite(const CSL_ospi_flash_cfgRegs *pRegs,
                             const uint8_t                *cmdBuf,
                             uint32_t                      cmdLen,
                             const uint8_t                *txBuf,
                             uint32_t                      txLen)
{
    (void)CSL_ospiCmdWrite(pRegs, cmdBuf, cmdLen, txBuf, txLen);
    return (OSPI_flashExecCmd(pRegs));
}

static int32_t OSPI_cmd_mode_write_v0(OSPI_Handle handle,
                                      const OSPI_Transaction *transaction)
{
    OSPI_v0_HwAttrs const *hwAttrs; /* OSPI hardware attributes */
    uint8_t               *txBuf;
    uint32_t               dataLen;
    uint32_t               cmdLen;
    int32_t                retVal = SPI_STATUS_SUCCESS;

    txBuf = (uint8_t *)transaction->txBuf;
    dataLen = (uint32_t)((uintptr_t)transaction->arg);
    cmdLen = (uint32_t)transaction->count - dataLen;
    /* Get the pointer to the object and hwAttrs */
    hwAttrs = (OSPI_v0_HwAttrs const *)handle->hwAttrs;

    if (0U != dataLen)
    {
        retVal = (OSPI_cmdWrite((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                 txBuf,
                                 cmdLen,
                                 &txBuf[cmdLen],
                                 dataLen));
    }
    else
    {
        retVal = (OSPI_cmdWrite((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                 txBuf,
                                 cmdLen,
                                 NULL,
                                 0));
    }
    return (retVal);
}


static int32_t OSPI_write_v0(OSPI_Handle handle, OSPI_Transaction *transaction)
{
    OSPI_v0_Object  *object;
    int32_t          retVal = SPI_STATUS_ERROR;

    /* Get the pointer to the object and hwAttrs */
    object = (OSPI_v0_Object *)handle->object;

    if((uint32_t)OSPI_OPER_MODE_IND_XFER == object->ospiMode)
    {
        retVal = (OSPI_ind_xfer_mode_write_v0(handle, transaction));
    }
    else if((uint32_t)OSPI_OPER_MODE_DAC_XFER == object->ospiMode)
    {
        retVal = (OSPI_dac_xfer_mode_write_v0(handle, transaction));
    }
    else if((uint32_t)OSPI_OPER_MODE_CFG == object->ospiMode)
    {
        retVal = (OSPI_cmd_mode_write_v0(handle, transaction));
    }
    else
    {
        transaction->status = OSPI_TRANSFER_CANCELED;
    }
    return (retVal);
}


/*
 *  ======== OSPI_transfer_v0 ========
 */
static bool OSPI_transfer_v0(OSPI_Handle handle, OSPI_Transaction *transaction)
{
    bool                  ret = BFALSE; /* return value */
    OSPI_v0_HwAttrs const *hwAttrs;     /* OSPI hardware attributes */
    OSPI_v0_Object        *object;      /* OSPI object */
    uintptr_t              key;
    int32_t                xferRet;

    /* Check if anything needs to be written or read */
    if ((NULL != handle) && (NULL != transaction) && (0U != (uint32_t)transaction->count))
    {
        /* Get the pointer to the object and hwAttrs */
        object = (OSPI_v0_Object *)handle->object;
        hwAttrs = (OSPI_v0_HwAttrs const *)handle->hwAttrs;

        /* Check if a transfer is in progress */
        key = SPI_osalHardwareIntDisable();
        if (NULL != object->transaction)
        {
            SPI_osalHardwareIntRestore(key);
            /* Transfer is in progress */
            transaction->status = OSPI_TRANSFER_CANCELED;
        }
        else
        {
            /* Save the pointer to the transaction */
            object->transaction = transaction;

            /* Acquire the lock for this particular I2C handle */
            (void)SPI_osalPendLock(object->mutex, SemaphoreP_WAIT_FOREVER);

            /* Book keeping of transmit and receive buffers. */
            object->writeBufIdx = (uint8_t *)transaction->txBuf;
            object->writeCountIdx = (uint32_t)transaction->count;
            object->readBufIdx =  (uint8_t *)transaction->rxBuf;
            object->readCountIdx = (uint32_t)transaction->count;

             /*
             * OSPI_primeTransfer_v0 is a longer process and
             * protection is needed from the OSPI interrupt
             */
            if ((uint32_t)SPI_OPER_MODE_POLLING != object->intrPollMode)
            {
                SPI_osalHardwareIntrEnable(hwAttrs->intrNum, hwAttrs->eventId);
            }

            xferRet = OSPI_primeTransfer_v0(handle, transaction);
            SPI_osalHardwareIntRestore(key);

            if (SPI_STATUS_SUCCESS == xferRet)
            {
                if ((uint32_t)SPI_OPER_MODE_BLOCKING == object->intrPollMode)
                {
                    ret = BTRUE;
                    if ((uint32_t)OSPI_OPER_MODE_IND_XFER == object->ospiMode)
                    {
                        /* In indirect transfer mode, wait for the lock
                           posted form the word completion interrupt */
                        (void)SPI_osalPendLock(object->transferComplete, SemaphoreP_WAIT_FOREVER);
                    }
#ifdef SPI_DMA_ENABLE
                    if (((uint32_t)OSPI_OPER_MODE_DAC_XFER == object->ospiMode) && (BTRUE == hwAttrs->dmaEnable))
                    {
                        /* in direct transfer mode with DMA enabled, wait for the lock
                           posted form the word completion interrupt */
                        (void)SPI_osalPendLock(object->transferComplete, SemaphoreP_WAIT_FOREVER);
                        if((uint32_t)SPI_TRANSACTION_TYPE_WRITE == object->transactionType)
                        {
                            ret = OSPI_waitDeviceReady(handle, OSPI_DAC_WRITE_TIMEOUT);
                        }
                    }
#endif
                    /* transfer is completed and semaphore is posted. */
                }
                else
                {
                    /* Always return BTRUE if in Asynchronous mode */
                    ret = BTRUE;
                }
                transaction->status = OSPI_TRANSFER_COMPLETED;
                /* Release the lock for this particular SPI handle */

                (void)SPI_osalPostLock(object->mutex);
            }
            else
            {
                transaction->status = OSPI_TRANSFER_FAILED;
            }

            if ((uint32_t)SPI_OPER_MODE_CALLBACK != object->intrPollMode)
            {
                object->transaction = NULL;
            }
        }
    }
    else
    {
        if (NULL != transaction)
        {
            transaction->status = OSPI_TRANSFER_CANCELED;
        }
    }

    /* Return the number of bytes transferred by the I2C */
    return (ret);
}

/*
 *  ======== OSPI_transferCallback_v0 ========
 */
static void OSPI_transferCallback_v0(OSPI_Handle handle, OSPI_Transaction *msg)
{
    OSPI_v0_Object        *object;  /* OSPI object */

    if (NULL != handle)
    {
        /* Get the pointer to the object */
        object = (OSPI_v0_Object *)handle->object;

        /* Indicate transfer complete */
        (void)SPI_osalPostLock(object->transferComplete);
    }

}

static int32_t OSPI_configDdr(OSPI_Handle handle, uint32_t cmd, uint32_t addr, uint32_t data);
static int32_t OSPI_configDdr(OSPI_Handle handle, uint32_t cmd, uint32_t addr, uint32_t data)
{
    OSPI_v0_HwAttrs const *hwAttrs; /* OSPI hardware attributes */
    int32_t                retVal;

    hwAttrs = (OSPI_v0_HwAttrs const *)handle->hwAttrs;
    CSL_ospiFlashStig((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), cmd, addr, data);
    retVal = OSPI_flashExecCmd((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr));
    if (SPI_STATUS_SUCCESS == retVal)
    {
        if (hwAttrs->dacEnable)
        {
            CSL_ospiDacEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), UTRUE);
        }
        else
        {
            CSL_ospiDacEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), UFALSE);
        }

        /* Enable DTR protocol */
        if (BTRUE == hwAttrs->dtrEnable)
        {
            CSL_ospiDtrEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), UTRUE);
        }
        else
        {
            CSL_ospiDtrEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), UFALSE);
        }
    }

    return (retVal);
}

static int32_t OSPI_enableXip (OSPI_Handle handle, uint32_t cmd, uint32_t addr, uint32_t data)
{
    OSPI_v0_HwAttrs const *hwAttrs; /* OSPI hardware attributes */
    int32_t                retVal;

    hwAttrs = (OSPI_v0_HwAttrs const *)handle->hwAttrs;

    /* Disable Direct Access Controller */
    CSL_ospiDacEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), UFALSE);

    /* Configure Flash Command Control Register to issue VCR write to FLASH memory to enable/disable XIP mode */
    CSL_ospiFlashStig((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), cmd, addr, data);
    retVal = OSPI_flashExecCmd((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr));
    if (SPI_STATUS_SUCCESS == retVal)
    {
        OSPI_delay(OSPI_XIP_SETUP_DELAY);

        /* Clear the XIP mode bits in the Mode Bit Configuration Register. */
        CSL_ospiSetModeBits((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), 0x00);

        /* Enable the local controllers XIP mode */
        CSL_ospiXipEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), UTRUE);
    }

    /* Re-enable the Direct Access Controller */
    CSL_ospiDacEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), UTRUE);
    return (retVal);
}

static int32_t OSPI_configDummyCycle(OSPI_Handle handle, uint32_t cmd, uint32_t addr, uint32_t data);
static int32_t OSPI_configDummyCycle(OSPI_Handle handle, uint32_t cmd, uint32_t addr, uint32_t data)
{
    OSPI_v0_HwAttrs const *hwAttrs; /* OSPI hardware attributes */
    int32_t                retVal;

    hwAttrs = (OSPI_v0_HwAttrs const *)handle->hwAttrs;
    CSL_ospiFlashStig((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), cmd, addr, data);
    retVal = OSPI_flashExecCmd((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr));

    return (retVal);
}

/*
 *  ======== OSPI_control_v0 ========
 */
static int32_t OSPI_control_v0(OSPI_Handle handle, uint32_t cmd, const void *arg)
{
    OSPI_v0_HwAttrs       *hwAttrs; /* OSPI hardware attributes */
    OSPI_v0_Object        *object;  /* OSPI object */
    uint32_t               phyOpMode;
    int32_t                retVal = SPI_STATUS_ERROR;
    const uint32_t        *ctrlData = (const uint32_t *)arg;
    uint32_t               nvcrCmd;
    uint32_t               addr;
    uint32_t               data;
    uint32_t               extOpcodeLo;
    uint32_t               extOpcodeUp;
    CSL_FssCfg             fssCfg;

    if (NULL != handle)
    {
        /* Get the pointer to the object */
        object = (OSPI_v0_Object *)handle->object;
        hwAttrs = (OSPI_v0_HwAttrs *)handle->hwAttrs;

        switch (cmd)
        {
            case OSPI_V0_CMD_XFER_OPCODE:
            {
                object->transferCmd = *ctrlData;
                ctrlData++;

                if(hwAttrs->dtrEnable == true)
                {
                    /* If dtr enable is true program the controller in 8D-8D-8D mode */
                    CSL_ospiConfigRead((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                    object->transferCmd,
                                    CSL_OSPI_CFG_XFER_MODE_8D_8D_8D,
                                    object->rdDummyClks);
                }
                else if(hwAttrs->xferLines == OSPI_XFER_LINES_OCTAL)
                {
                    if(hwAttrs->numAddrLines == OSPI_XFER_LINES_OCTAL)
                    {
                        /* Program the controller in 1S-8S-8S mode */
                        CSL_ospiConfigRead((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                        object->transferCmd,
                                        CSL_OSPI_CFG_XFER_MODE_1S_8S_8S,
                                        object->rdDummyClks);
                    }
                    else
                    {
                        /* Program the controller in 1S-1S-8S mode */
                        CSL_ospiConfigRead((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                        object->transferCmd,
                                        CSL_OSPI_CFG_XFER_MODE_1S_1S_8S,
                                        object->rdDummyClks);
                    }
                }
                else
                {
                    /* Program the controller in 1S-1S-1S mode */
                    CSL_ospiConfigRead((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                    object->transferCmd,
                                    CSL_OSPI_CFG_XFER_MODE_1S_1S_1S,
                                    object->rdDummyClks);
                }


                object->transferCmd = *ctrlData;
                ctrlData++;
                CSL_ospiWriteSetup((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                   object->transferCmd,
                                   object->xferLines);
                object->rdStatusCmd = *ctrlData;
                ctrlData++;
                object->rdStatusAddr = *ctrlData;
                retVal = SPI_STATUS_SUCCESS;
                break;
            }

            case OSPI_V0_CMD_XFER_OPCODE_EXT:
            {
                extOpcodeLo  = *ctrlData++; /* EXT_STIG_OPCODE_FLD */
                extOpcodeLo |= (*ctrlData++ << 8); /* EXT_POLL_OPCODE_FLD */
                extOpcodeLo |= (*ctrlData++ << 16); /* EXT_WRITE_OPCODE_FLD */
                extOpcodeLo |= (*ctrlData++ << 24); /* EXT_READ_OPCODE_FLD */

                extOpcodeUp  = (*ctrlData++ << 16); /* EXT_WEL_OPCODE_FLD */
                extOpcodeUp |= (*ctrlData << 24); /* WEL_OPCODE_FLD */

                CSL_ospiExtOpcodeSet((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                     extOpcodeLo,
                                     extOpcodeUp);
                CSL_ospiSetDualByteOpcodeMode((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                              UTRUE);
                retVal = SPI_STATUS_SUCCESS;
                break;
            }

            case OSPI_V0_CMD_XFER_MODE_RW:
            {
                object->transactionType = *ctrlData;
                retVal = SPI_STATUS_SUCCESS;
                break;
            }

            case OSPI_V0_CMD_SET_CFG_MODE:
            {
                object->ospiMode = (uint32_t)OSPI_OPER_MODE_CFG;
                retVal = SPI_STATUS_SUCCESS;
                break;
            }

            case OSPI_V0_CMD_SET_XFER_MODE:
            {
                if (hwAttrs->dacEnable)
                {
                    object->ospiMode = (uint32_t)OSPI_OPER_MODE_DAC_XFER;
                }
                else
                {
                    object->ospiMode = (uint32_t)OSPI_OPER_MODE_IND_XFER;
                }
                retVal = SPI_STATUS_SUCCESS;
                break;
            }

            case OSPI_V0_CMD_CFG_POLL_DUMMY_CYCLE:
            {
                CSL_ospiSetPollingDummyCycles((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                                   *ctrlData);
                break;
            }

            case OSPI_V0_CMD_SET_XFER_LINES:
            {
                uint32_t numAddrBytes;
                object->xferLines = *ctrlData;
                /* numAddrBytes holds the explicit byte count (1-4). If 0
                 * (not set — older SOCs), fall back to dtrEnable-based selection. */
                if (0U != hwAttrs->numAddrBytes)
                {
                    numAddrBytes = hwAttrs->numAddrBytes - 1U;
                }
                else if (hwAttrs->dtrEnable)
                {
                    numAddrBytes = CSL_OSPI_MEM_MAP_NUM_ADDR_BYTES_4;
                }
                else
                {
                    numAddrBytes = CSL_OSPI_MEM_MAP_NUM_ADDR_BYTES_3;
                }
                /* Set device size cofigurations */
                CSL_ospiSetDevSize((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                   numAddrBytes,
                                   hwAttrs->pageSize,
                                   hwAttrs->blkSize);
                retVal = SPI_STATUS_SUCCESS;
                break;
            }

            case OSPI_V0_CMD_RD_DUMMY_CLKS:
            {
                object->rdDummyClks = *ctrlData;
                retVal = SPI_STATUS_SUCCESS;
                break;
            }

            case OSPI_V0_CMD_EXT_RD_DUMMY_CLKS:
            {
                object->extRdDummyClks = *ctrlData;
                retVal = SPI_STATUS_SUCCESS;
                break;
            }

            case OSPI_V0_CMD_EXT_RD_CMD_LEN:
            {
                object->extRdCmdLen = *ctrlData;
                retVal = SPI_STATUS_SUCCESS;
                break;
            }

            case OSPI_V0_CMD_CFG_PHY:
            {
                uint32_t phyEnable = *ctrlData++;
                if (UTRUE == phyEnable)
                {
                    if (BFALSE == hwAttrs->phyEnable)
                    {
                        CSL_ospiSetPreScaler((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                             CSL_OSPI_BAUD_RATE_DIVISOR(2U));
                        /* Enable PHY mode */
                        CSL_ospiPhyEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), UTRUE);
                        hwAttrs->phyEnable = BTRUE;
                    }
                    uint32_t txDelay = *ctrlData++;
                    uint32_t rxDelay = *ctrlData;
                    uint32_t funcClk = hwAttrs->funcClk;
                    phyOpMode        = hwAttrs->phyOpMode;
                    uint32_t halfClkLock = hwAttrs->phyLockCycle;
                    CSL_ospiConfigPhyDLL((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                         txDelay, rxDelay, phyOpMode, halfClkLock, funcClk);
                }
                else
                {
                    if (BTRUE == hwAttrs->phyEnable)
                    {
                        /* Disable high speed mode when PHY is disabled */
                        CSL_ospiSetPreScaler((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                             CSL_OSPI_BAUD_RATE_DIVISOR_DEFAULT);
                        CSL_ospiPhyEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), UFALSE);
                        hwAttrs->phyEnable = BFALSE;
                    }
                }
                retVal = SPI_STATUS_SUCCESS;
                break;
            }

            case OSPI_V0_CMD_ENABLE_DDR:
            case OSPI_V0_CMD_ENABLE_SDR:
            {
                nvcrCmd = *ctrlData;
                ctrlData++;
                addr   = *ctrlData;
                ctrlData++;
                data   = *ctrlData;
                retVal = OSPI_configDdr(handle, nvcrCmd, addr, data);
                break;
            }

            case OSPI_V0_CMD_CFG_XIP:
            {
                if (BTRUE == hwAttrs->xipEnable) {
                    nvcrCmd = *ctrlData;
                    ctrlData++;
                    addr = *ctrlData;
                    ctrlData++;
                    data = *ctrlData;

                    (void)OSPI_enableXip(handle, nvcrCmd , addr, data);
                }
                /* Enable PHY pipeline mode for read */
                CSL_ospiPipelinePhyEnable((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), UTRUE);
                break;
            }
            case OSPI_V0_CMD_ENABLE_XIP_PREFETCH:
            {
                uint32_t xipPrefetchEnable = *ctrlData;
                /**
                 * The FSS prefetches 32 bytes ahead of time, and if the flash data is updated,
                 * the FSS will still return the prefetched data instead of initiating a new read request.
                 * XIP prefetch should be enabled only if the controller is in XIP mode else or when reads
                 * are contiguous without any update in flash memory.
                */
                fssCfg.pFsasRegs = (CSL_fss_fsas_genregsRegs *)CSL_MCU_FSS0_FSAS_CFG_BASE;
                CSL_fssOspiSetXipPrefetchEnable(&fssCfg, CSL_FSS_FSAS_INTERFACE_PATH_SELECT_OSPI0, xipPrefetchEnable);
            }

            case OSPI_V0_CMD_CFG_RD_DELAY:
            {
                uint32_t rdDelay = *ctrlData;
                CSL_ospiSetDataReadCapDelay((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                                            rdDelay);
                retVal = SPI_STATUS_SUCCESS;
                break;
            }

            case OSPI_V0_CMD_CFG_DUMMY_CYCLE:
            {
                nvcrCmd = *ctrlData;
                ctrlData++;
                addr   = *ctrlData;
                ctrlData++;
                data   = *ctrlData;
                retVal = OSPI_configDummyCycle(handle, nvcrCmd, addr, data);
                break;
            }

            default:
                retVal = SPI_STATUS_UNDEFINEDCMD;
            break;
        }
    }

    return retVal;
}


/*
 *  ======== OSPI_transferCancel_v0 ========
 */
static void OSPI_transferCancel_v0(OSPI_Handle handle)
{
    (void)handle;
    return;
}
