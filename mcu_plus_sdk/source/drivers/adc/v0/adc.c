/*
 * Copyright (C) 2021-2025 Texas Instruments Incorporated
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 *
 *   Neither the name of Texas Instruments Incorporated nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
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
 */

/**
 *  \file   adc.c
 *
 *  \brief  This file contains the implementation of the APIs present in the
 *          device abstraction layer file of ADC.
 *          This also contains some related macros.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <drivers/adc.h>
#include <drivers/hw_include/hw_types.h>
#include <drivers/hw_include/csl_types.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                 Internal Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void ADC_powerUp(uint32_t baseAddr, uint32_t powerUp)
{
    if (0U != powerUp)
    {
        /* Power up Analog Front End */
        HW_WR_FIELD32(baseAddr + ADC_CTRL, ADC_CTRL_POWER_DOWN,
                      ADC_CTRL_POWER_DOWN_AFEPOWERUP);
    }
    else
    {
        /* Power down Analog Front End */
        HW_WR_FIELD32(baseAddr + ADC_CTRL, ADC_CTRL_POWER_DOWN,
                      ADC_CTRL_POWER_DOWN_AFEPOWERDOWN);
    }
}

uint32_t Adc_isPoweredUp(uint32_t baseAddr)
{
    uint32_t powerUp;
    uint32_t state = FALSE;

    powerUp = HW_RD_FIELD32(baseAddr + ADC_CTRL, ADC_CTRL_POWER_DOWN);

    if(powerUp == 0U)
    {
        state = (uint32_t)TRUE;
    }
    else
    {
        state = (uint32_t)FALSE;
    }

    return state;
}

int32_t ADC_setClkDivider(uint32_t baseAddr, uint32_t clkDivider)
{
    int32_t status;

    /* Clock divider is not supported  */
    status = CSL_PASS;

    return status;
}

void ADC_init(uint32_t baseAddr,
             uint32_t errCorrection,
             uint32_t errOffset,
             uint32_t calibration)
{
    uint32_t initMask = 0x0U;

    /* Calculate mask used for ADC init */
    initMask = (calibration << ADC_MISC_AFE_SPARE_INPUT_SHIFT);

    /* ADC Initialization */
    HW_WR_FIELD32(baseAddr + ADC_MISC, ADC_MISC_AFE_SPARE_INPUT, initMask);
}

void ADC_FIFODMAAccessEnable(uint32_t baseAddr,
                            uint32_t fifoNum,
                            uint32_t dmaLineEnable)
{
    if (ADC_FIFO_NUM_0 == fifoNum)
    {
        if (TRUE == dmaLineEnable)
        {
            /* Enable DMA access */
            HW_WR_FIELD32(baseAddr + ADC_DMAENABLE_SET, ADC_DMAENABLE_SET_EN_0,
                          ADC_DMAENABLE_SET_EN_0_ENABLE);
        }
        else
        {
            /* Disable DMA access */
            HW_WR_FIELD32(baseAddr + ADC_DMAENABLE_CLR, ADC_DMAENABLE_CLR_EN_0,
                          ADC_DMAENABLE_CLR_EN_0_DISABLE);
        }
    }
    if (ADC_FIFO_NUM_1 == fifoNum)
    {
        if (TRUE == dmaLineEnable)
        {
            /* Enable DMA access */
            HW_WR_FIELD32(baseAddr + ADC_DMAENABLE_SET, ADC_DMAENABLE_SET_EN_1,
                          ADC_DMAENABLE_SET_EN_1_ENABLE);
        }
        else
        {
            /* Disable DMA access */
            HW_WR_FIELD32(baseAddr + ADC_DMAENABLE_CLR, ADC_DMAENABLE_CLR_EN_1,
                          ADC_DMAENABLE_CLR_EN_1_DISABLE);
        }
    }
}

int32_t ADC_setStepParams(uint32_t               baseAddr,
                         uint32_t               stepId,
                         const adcStepConfig_t *configParams)
{
    int32_t status = CSL_PASS;

    if ((ADC_OPENDELAY_MAX >= configParams->openDelay) &&
        (ADC_SAMPLEDELAY_MAX >= configParams->sampleDelay))
    {
        HW_WR_FIELD32(baseAddr + ADC_STEPCONFIG(stepId), ADC_STEPCONFIG_MODE,
                      configParams->mode);
        HW_WR_FIELD32(baseAddr + ADC_STEPCONFIG(stepId),
                      ADC_STEPCONFIG_SEL_INP_SWC,
                      configParams->channel);
        HW_WR_FIELD32(baseAddr + ADC_STEPDELAY(stepId), ADC_STEPDELAY_OPENDELAY,
                      configParams->openDelay);
        HW_WR_FIELD32(baseAddr + ADC_STEPDELAY(stepId),
                      ADC_STEPDELAY_SAMPLEDELAY,
                      configParams->sampleDelay);
        HW_WR_FIELD32(baseAddr + ADC_STEPCONFIG(stepId),
                      ADC_STEPCONFIG_AVERAGING,
                      configParams->averaging);
        HW_WR_FIELD32(baseAddr + ADC_STEPCONFIG(stepId),
                      ADC_STEPCONFIG_FIFO_SELECT,
                      configParams->fifoNum);
        HW_WR_FIELD32(baseAddr + ADC_STEPCONFIG(stepId),
                      ADC_STEPCONFIG_RANGE_CHECK,
                      configParams->rangeCheckEnable);
    }
    else
    {
        status = CSL_EFAIL;
    }
    return status;
}

void ADC_stepEnable(uint32_t baseAddr,
                   uint32_t stepId,
                   uint32_t stepEnable)
{
    uint32_t regData;

    regData = HW_RD_REG32(baseAddr + ADC_STEPENABLE);
    if (TRUE == stepEnable)
    {
        regData |= ((uint32_t) 0x1U << (stepId + 1U));
    }
    else
    {
        regData &= ~((uint32_t) 0x1U << (stepId + 1U));
    }
    HW_WR_REG32(baseAddr + ADC_STEPENABLE, regData);
}

void ADC_clearAllSteps(uint32_t baseAddr)
{
    HW_WR_REG32(baseAddr + ADC_STEPENABLE, 0x00U);
}

void ADC_start(uint32_t baseAddr, uint32_t adcEnable)
{
    HW_WR_FIELD32(baseAddr + ADC_CTRL, ADC_CTRL_EN, adcEnable);
}

void ADC_stepIdTagEnable(uint32_t baseAddr, uint32_t stepIdTag)
{
    HW_WR_FIELD32(baseAddr + ADC_CTRL, ADC_CTRL_STEP_ID_TAG, stepIdTag);
}

uint32_t ADC_getFIFOData(uint32_t baseAddr, uint32_t fifoNum)
{
    uint32_t data = 0U;

    data = HW_RD_REG32(baseAddr + ADC_FIFODATA(fifoNum));
    return data;
}

uint32_t ADC_getFIFOWordCount(uint32_t baseAddr, uint32_t fifoNum)
{
    return (HW_RD_REG32(baseAddr + ADC_FIFOCOUNT(fifoNum)));
}

int32_t ADC_setCPUFIFOThresholdLevel(uint32_t baseAddr,
                                    uint32_t fifoNum,
                                    uint32_t threshold)
{
    int32_t status = CSL_EFAIL;

    if (ADC_FIFO_SIZE >= threshold)
    {
        HW_WR_FIELD32(baseAddr + ADC_FIFOTHRESHOLD(fifoNum),
                      ADC_FIFOTHRESHOLD_FIFO_THR_LEVEL, threshold - 1U);
        status = CSL_PASS;
    }
    return status;
}

uint32_t ADC_getCPUFIFOThresholdLevel(uint32_t baseAddr, uint32_t fifoNum)
{
    return (HW_RD_REG32(baseAddr + ADC_FIFOTHRESHOLD(fifoNum)) + 1U);
}

int32_t ADC_setDMAFIFOThresholdLevel(uint32_t baseAddr,
                                    uint32_t fifoNum,
                                    uint32_t threshold)
{
    int32_t status = CSL_EFAIL;

    if (ADC_FIFO_SIZE >= threshold)
    {
        HW_WR_FIELD32(baseAddr + ADC_DMAREQ(fifoNum),
                      ADC_DMAREQ_DMA_REQUEST_LEVEL, threshold - 1U);
        status = CSL_PASS;
    }
    return status;
}

uint32_t ADC_getDMAFIFOThresholdLevel(uint32_t baseAddr, uint32_t fifoNum)
{
    return (HW_RD_REG32(baseAddr + ADC_DMAREQ(fifoNum)) + 1U);
}

void ADC_enableIntr(uint32_t baseAddr, uint32_t intrMask)
{
    HW_WR_REG32(baseAddr + ADC_IRQENABLE_SET, intrMask);
}

void ADC_disableIntr(uint32_t baseAddr, uint32_t intrMask)
{
    HW_WR_REG32(baseAddr + ADC_IRQENABLE_CLR, intrMask);
}

void ADC_writeEOI(uint32_t baseAddr)
{
    HW_WR_FIELD32(baseAddr + ADC_IRQ_EOI, ADC_IRQ_EOI_LINE_NUMBER,
                  ADC_IRQ_EOI_LINE_NUMBER_EOI);
}

uint32_t ADC_getIntrStatus(uint32_t baseAddr)
{
    return (HW_RD_REG32(baseAddr + ADC_IRQSTATUS));
}

void ADC_clearIntrStatus(uint32_t baseAddr, uint32_t intrMask)
{
    HW_WR_REG32(baseAddr + ADC_IRQSTATUS, intrMask);
}

uint32_t ADC_getIntrRawStatus(uint32_t baseAddr)
{
    return (HW_RD_REG32(baseAddr + ADC_IRQSTATUS_RAW));
}

int32_t ADC_setRange(uint32_t baseAddr, uint32_t highRange, uint32_t lowRange)
{
    int32_t status = CSL_EFAIL;

    if ((ADC_RANGE_MAX >= highRange) &&
        (highRange >= lowRange))
    {
        HW_WR_FIELD32(baseAddr + ADC_ADCRANGE, ADC_ADCRANGE_HIGH_RANGE_DATA,
                      highRange);
        HW_WR_FIELD32(baseAddr + ADC_ADCRANGE, ADC_ADCRANGE_LOW_RANGE_DATA,
                      lowRange);
        status = CSL_PASS;
    }

    return status;
}

void ADC_getRange(uint32_t  baseAddr,
                 uint32_t *highRange,
                 uint32_t *lowRange)
{
    *highRange = HW_RD_FIELD32(baseAddr + ADC_ADCRANGE, ADC_ADCRANGE_HIGH_RANGE_DATA);
    *lowRange = HW_RD_FIELD32(baseAddr + ADC_ADCRANGE, ADC_ADCRANGE_LOW_RANGE_DATA);
}

void ADC_getSequencerStatus(uint32_t              baseAddr,
                           adcSequencerStatus_t *status)
{
    status->afeBusy = HW_RD_FIELD32(baseAddr + ADC_ADCSTAT, ADC_ADCSTAT_AFE_BUSY);
    status->fsmBusy = HW_RD_FIELD32(baseAddr + ADC_ADCSTAT, ADC_ADCSTAT_FSM_BUSY);
    status->stepId = HW_RD_FIELD32(baseAddr + ADC_ADCSTAT, ADC_ADCSTAT_STEP_ID);
}

void ADC_setIdleMode(uint32_t baseAddr, uint32_t idleMode)
{
    /* Not supported  */
    return;
}

void ADC_getRevisionId(uint32_t baseAddr, adcRevisionId_t *revId)
{
    /* Get the scheme */
    revId->scheme = HW_RD_FIELD32(baseAddr + ADC_REVISION, ADC_REVISION_SCHEME);
    /* Get the Functional release number*/
    revId->func = HW_RD_FIELD32(baseAddr + ADC_REVISION, ADC_REVISION_FUNC);
    /* Get the Design Release Number */
    revId->rtlRev = HW_RD_FIELD32(baseAddr + ADC_REVISION, ADC_REVISION_R_RTL);
    /* Get the Major Revision Number */
    revId->major = HW_RD_FIELD32(baseAddr + ADC_REVISION, ADC_REVISION_X_MAJOR);
    /* Get a special version of the module */
    revId->custom = HW_RD_FIELD32(baseAddr + ADC_REVISION, ADC_REVISION_CUSTOM);
    /* Get the Minor revision number */
    revId->minor = HW_RD_FIELD32(baseAddr + ADC_REVISION, ADC_REVISION_Y_MINOR);
}
