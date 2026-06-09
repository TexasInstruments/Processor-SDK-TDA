/*
 *  Copyright (c) Texas Instruments Incorporated 2026
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
 *  \file     ip_fma_sms.c
 *
 *  \brief    This file contains SMS library functions implementations
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <cslr.h>
#include "ip_fma_common.h"
#include "ip_fma_sms.h"

#if defined(SOC_J784S4)
#include <ti/csl/soc/j784s4/src/cslr_soc_baseaddress.h>
#endif
#if defined(SOC_J721S2)
#include <ti/csl/soc/j721s2/src/cslr_soc_baseaddress.h>
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* ========================================================================== */
/*                   Internal/Private Function Definitions                    */
/* ========================================================================== */

/**
 * \brief   This function returns the value of a DMTIMER register specified by the
 * given register offset and timer instance ID. The base address is selected
 * according to the provided ID (0–3).
 *
 * \param  offset     Register offset from the base address of the selected DMTIMER instance.
 * \param  id         DMTIMER instance identifier:
 *                     - 0 -> DMTIMER0
 *                     - 1 -> DMTIMER1
 *                     - 2 -> DMTIMER2
 *                     - 3 -> DMTIMER3
 *
 * \return  regValue  32-bit register value from the selected DMTIMER instance
 *
 */
static uint32_t IpFma_Sms_GetDmtimerRegVal(uint32_t offset, uint8_t id)
{
    uint32_t regValue = 0U;

    if (0 == id)
        regValue = CSL_REG32_RD(CSL_WKUP_SMS0_DMTIMER0_BASE + offset);
    else if (1 == id)
        regValue = CSL_REG32_RD(CSL_WKUP_SMS0_DMTIMER1_BASE + offset);
    else if (2 == id)
        regValue = CSL_REG32_RD(CSL_WKUP_SMS0_DMTIMER2_BASE + offset);
    else if (3 == id)
        regValue = CSL_REG32_RD(CSL_WKUP_SMS0_DMTIMER3_BASE + offset);

    return regValue;
}

/**
 * @brief  Reads a 32-bit RTI register value.
 *
 * This function returns the value of an RTI register based on the provided
 * register offset.
 *
 * @param  offset    Register offset from the base address of the selected RTI instance.
 *
 * @return regValue  32-bit value of the selected RTI register.
 */
static uint32_t IpFma_Sms_GetRtiRegValue(uint32_t offset)
{
    uint32_t regValue = 0U;

    regValue = CSL_REG32_RD(CSL_WKUP_SMS0_WDT_RTI_BASE + offset);

    return regValue;
}

/**
 * @brief Reads a 32-bit RTI MMR register value.
 *
 * @param  offset    Register offset from the base address.
 *
 * @return regValue  32-bit value of the selected RTI register.
 */
static uint32_t IpFma_Sms_GetRtiMmrRegValue(uint32_t offset)
{
    uint32_t regValue = 0U;

    regValue = CSL_REG32_RD(CSL_WKUP_SMS0_RTI_BASE + offset);

    return regValue;
}

/**
 * \brief   Reads a 32-bit value of TIFS PWR MMR register specified by the
 * given register offset.
 *
 * \param   offset    Register offset from the base address
 *
 * \return  regValue  32-bit register value
 */
static uint32_t IpFma_Sms_GetPwrMmrRegValue(uint32_t offset)
{
    uint32_t regValue = 0U;

    regValue = CSL_REG32_RD(CSL_WKUP_SMS0_PWR_BASE + offset);

    return regValue;
}

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

IpFma_Status IpFma_Sms_GetRtiRegs(IpFma_SmsRtiRegs* reg)
{
    IpFma_Status status = IPFMA_OK;

    if (NULL_PTR == reg)
    {
        status = IPFMA_E_PARAM;
    }

    if (IPFMA_OK == status)
    {
        reg->gctrl          = IpFma_Sms_GetRtiRegValue(RTI_GCTRL);
        reg->tbctrl         = IpFma_Sms_GetRtiRegValue(RTI_TBCTRL);
        reg->capctrl        = IpFma_Sms_GetRtiRegValue(RTI_CAPCTRL);
        reg->compctrl       = IpFma_Sms_GetRtiRegValue(RTI_COMPCTRL);
        reg->frc0           = IpFma_Sms_GetRtiRegValue(RTI_FRC0);
        reg->uc0            = IpFma_Sms_GetRtiRegValue(RTI_UC0);
        reg->cpuc0          = IpFma_Sms_GetRtiRegValue(RTI_CPUC0);
        reg->cafrc0         = IpFma_Sms_GetRtiRegValue(RTI_CAFRC0);
        reg->cauc0          = IpFma_Sms_GetRtiRegValue(RTI_CAUC0);
        reg->frc1           = IpFma_Sms_GetRtiRegValue(RTI_FRC1);
        reg->uc1            = IpFma_Sms_GetRtiRegValue(RTI_UC1);
        reg->cpuc1          = IpFma_Sms_GetRtiRegValue(RTI_CPUC1);
        reg->cafrc1         = IpFma_Sms_GetRtiRegValue(RTI_CAFRC1);
        reg->cauc1          = IpFma_Sms_GetRtiRegValue(RTI_CAUC1);
        reg->comp0          = IpFma_Sms_GetRtiRegValue(RTI_COMP0);
        reg->udcp0          = IpFma_Sms_GetRtiRegValue(RTI_UDCP0);
        reg->comp1          = IpFma_Sms_GetRtiRegValue(RTI_COMP1);
        reg->udcp1          = IpFma_Sms_GetRtiRegValue(RTI_UDCP1);
        reg->comp2          = IpFma_Sms_GetRtiRegValue(RTI_COMP2);
        reg->udcp2          = IpFma_Sms_GetRtiRegValue(RTI_UDCP2);
        reg->comp3          = IpFma_Sms_GetRtiRegValue(RTI_COMP3);
        reg->udcp3          = IpFma_Sms_GetRtiRegValue(RTI_UDCP3);
        reg->tblcomp        = IpFma_Sms_GetRtiRegValue(RTI_TBLCOMP);
        reg->tbhcomp        = IpFma_Sms_GetRtiRegValue(RTI_TBHCOMP);
        reg->setint         = IpFma_Sms_GetRtiRegValue(RTI_SETINT);
        reg->clearint       = IpFma_Sms_GetRtiRegValue(RTI_CLEARINT);
        reg->intflag        = IpFma_Sms_GetRtiRegValue(RTI_INTFLAG);
        reg->dwdctrl        = IpFma_Sms_GetRtiRegValue(RTI_DWDCTRL);
        reg->dwdprld        = IpFma_Sms_GetRtiRegValue(RTI_DWDPRLD);
        reg->wdstatus       = IpFma_Sms_GetRtiRegValue(RTI_WDSTATUS);
        reg->wdkey          = IpFma_Sms_GetRtiRegValue(RTI_WDKEY);
        reg->dwwdrxnctrl    = IpFma_Sms_GetRtiRegValue(RTI_DWWDRXNCTRL);
        reg->dwwdsizectrl   = IpFma_Sms_GetRtiRegValue(RTI_DWWDSIZECTRL);
        reg->intclrenable   = IpFma_Sms_GetRtiRegValue(RTI_INTCLRENABLE);
        reg->comp0clr       = IpFma_Sms_GetRtiRegValue(RTI_COMP0CLR);
        reg->comp1clr       = IpFma_Sms_GetRtiRegValue(RTI_COMP1CLR);
        reg->comp2clr       = IpFma_Sms_GetRtiRegValue(RTI_COMP2CLR);
        reg->comp3clr       = IpFma_Sms_GetRtiRegValue(RTI_COMP3CLR);
    }

    return status;
}

IpFma_Status IpFma_Sms_GetDmtimerRegs(IpFma_SmsDmtimerRegs* reg, uint8_t id)
{
    IpFma_Status status = IPFMA_OK;

    if (NULL_PTR == reg)
    {
        status = IPFMA_E_PARAM;
    }

    if (IPFMA_OK == status)
    {
        reg->tidr          = IpFma_Sms_GetDmtimerRegVal(DMTIMER_TIDR, id);
        reg->tiocp_cfg     = IpFma_Sms_GetDmtimerRegVal(DMTIMER_TIOCP_CFG, id);
        reg->irq_eoi       = IpFma_Sms_GetDmtimerRegVal(DMTIMER_IRQ_EOI, id);
        reg->irqstatus_raw = IpFma_Sms_GetDmtimerRegVal(DMTIMER_IRQSTATUS_RAW, id);
        reg->irqstatus     = IpFma_Sms_GetDmtimerRegVal(DMTIMER_IRQSTATUS, id);
        reg->irqstatus_set = IpFma_Sms_GetDmtimerRegVal(DMTIMER_IRQSTATUS_SET, id);
        reg->irqstatus_clr = IpFma_Sms_GetDmtimerRegVal(DMTIMER_IRQSTATUS_CLR, id);
        reg->irqwakeen     = IpFma_Sms_GetDmtimerRegVal(DMTIMER_IRQWAKEEN, id);
        reg->tclr          = IpFma_Sms_GetDmtimerRegVal(DMTIMER_TCLR, id);
        reg->tcrr          = IpFma_Sms_GetDmtimerRegVal(DMTIMER_TCRR, id);
        reg->tldr          = IpFma_Sms_GetDmtimerRegVal(DMTIMER_TLDR, id);
        reg->ttgr          = IpFma_Sms_GetDmtimerRegVal(DMTIMER_TTGR, id);
        reg->twps          = IpFma_Sms_GetDmtimerRegVal(DMTIMER_TWPS, id);
        reg->tmar          = IpFma_Sms_GetDmtimerRegVal(DMTIMER_TMAR, id);
        reg->tcar1         = IpFma_Sms_GetDmtimerRegVal(DMTIMER_TCAR1, id);
        reg->tsicr         = IpFma_Sms_GetDmtimerRegVal(DMTIMER_TSICR, id);
        reg->tcar2         = IpFma_Sms_GetDmtimerRegVal(DMTIMER_TCAR2, id);
        reg->tpir          = IpFma_Sms_GetDmtimerRegVal(DMTIMER_TPIR, id);
        reg->tnir          = IpFma_Sms_GetDmtimerRegVal(DMTIMER_TNIR, id);
        reg->tcvr          = IpFma_Sms_GetDmtimerRegVal(DMTIMER_TCVR, id);
        reg->tocr          = IpFma_Sms_GetDmtimerRegVal(DMTIMER_TOCR, id);
        reg->towr          = IpFma_Sms_GetDmtimerRegVal(DMTIMER_TOWR, id);
    }

    return status;
}

IpFma_Status IpFma_Sms_GetRtiMmrRegs(IpFma_SmsRtiMmrRegs* reg)
{
    IpFma_Status status = IPFMA_OK;

    if (NULL_PTR == reg)
    {
        status = IPFMA_E_PARAM;
    }

    if (IPFMA_OK == status)
    {
        reg->lock0_kick0      = IpFma_Sms_GetRtiMmrRegValue(RTI_MMR_LOCK0_KICK0);
        reg->lock0_kick1      = IpFma_Sms_GetRtiMmrRegValue(RTI_MMR_LOCK0_KICK1);
        reg->wwrti_ctrl       = IpFma_Sms_GetRtiMmrRegValue(RTI_MMR_WWRTI_CTRL);
        reg->wwrti_clock_ctrl = IpFma_Sms_GetRtiMmrRegValue(RTI_MMR_WWRTI_CLOCK_CTRL);
    }

    return status;
}

IpFma_Status IpFma_Sms_GetPwrMmrRegs(IpFma_SmsPwrMmrRegs* reg)
{
    IpFma_Status status = IPFMA_OK;

    if (NULL_PTR == reg)
    {
        status = IPFMA_E_PARAM;
    }

    if (IPFMA_OK == status)
    {
        reg->lock0_kick0     = IpFma_Sms_GetPwrMmrRegValue(PWR_MMR_LOCK0_KICK0);
        reg->lock0_kick1     = IpFma_Sms_GetPwrMmrRegValue(PWR_MMR_LOCK0_KICK1);
        reg->pmctrl_sys      = IpFma_Sms_GetPwrMmrRegValue(PWR_MMR_PMCTRL_SYS);
        reg->pmctrl_mosc     = IpFma_Sms_GetPwrMmrRegValue(PWR_MMR_PMCTRL_MOSC);
        reg->pmctrl_dmsc     = IpFma_Sms_GetPwrMmrRegValue(PWR_MMR_PMCTRL_DMSC);
        reg->cpu_stcalib     = IpFma_Sms_GetPwrMmrRegValue(PWR_MMR_CPU_STCALIB);
        reg->pm_dmtimer_ctrl = IpFma_Sms_GetPwrMmrRegValue(PWR_MMR_PM_DMTIMER_CTRL);
        reg->pm_permission   = IpFma_Sms_GetPwrMmrRegValue(PWR_MMR_PM_PERMISSION);
    }

    return status;
}

IpFma_Status IpFma_Sms_VerifyDmtimerRegs(const IpFma_SmsDmtimerRegs* reg, uint8_t id)
{
    uint32_t regData = 0U, mismatch = 0U;
    IpFma_Status status = IPFMA_OK;

    if (NULL_PTR == reg)
    {
        status = IPFMA_E_PARAM;
    }

    if (IPFMA_OK == status)
    {
        regData = IpFma_Sms_GetDmtimerRegVal(DMTIMER_TIDR, id);
        mismatch |= reg->tidr ^ regData;

        regData = IpFma_Sms_GetDmtimerRegVal(DMTIMER_TIOCP_CFG, id);
        mismatch |= reg->tiocp_cfg ^ regData;

        regData = IpFma_Sms_GetDmtimerRegVal(DMTIMER_IRQ_EOI, id);
        mismatch |= reg->irq_eoi ^ regData;

        regData = IpFma_Sms_GetDmtimerRegVal(DMTIMER_IRQSTATUS_RAW, id);
        mismatch |= reg->irqstatus_raw ^ regData;

        regData = IpFma_Sms_GetDmtimerRegVal(DMTIMER_IRQSTATUS, id);
        mismatch |= reg->irqstatus ^ regData;

        regData = IpFma_Sms_GetDmtimerRegVal(DMTIMER_IRQSTATUS_SET, id);
        mismatch |= reg->irqstatus_set ^ regData;

        regData = IpFma_Sms_GetDmtimerRegVal(DMTIMER_IRQSTATUS_CLR, id);
        mismatch |= reg->irqstatus_clr ^ regData;

        regData = IpFma_Sms_GetDmtimerRegVal(DMTIMER_IRQWAKEEN, id);
        mismatch |= reg->irqwakeen ^ regData;

        regData = IpFma_Sms_GetDmtimerRegVal(DMTIMER_TCLR, id);
        mismatch |= reg->tclr ^ regData;

        regData = IpFma_Sms_GetDmtimerRegVal(DMTIMER_TCRR, id);
        mismatch |= reg->tcrr ^ regData;

        regData = IpFma_Sms_GetDmtimerRegVal(DMTIMER_TLDR, id);
        mismatch |= reg->tldr ^ regData;

        regData = IpFma_Sms_GetDmtimerRegVal(DMTIMER_TTGR, id);
        mismatch |= reg->ttgr ^ regData;

        regData = IpFma_Sms_GetDmtimerRegVal(DMTIMER_TWPS, id);
        mismatch |= reg->twps ^ regData;

        regData = IpFma_Sms_GetDmtimerRegVal(DMTIMER_TMAR, id);
        mismatch |= reg->tmar ^ regData;

        regData = IpFma_Sms_GetDmtimerRegVal(DMTIMER_TCAR1, id);
        mismatch |= reg->tcar1 ^ regData;

        regData = IpFma_Sms_GetDmtimerRegVal(DMTIMER_TSICR, id);
        mismatch |= reg->tsicr ^ regData;

        regData = IpFma_Sms_GetDmtimerRegVal(DMTIMER_TCAR2, id);
        mismatch |= reg->tcar2 ^ regData;

        regData = IpFma_Sms_GetDmtimerRegVal(DMTIMER_TPIR, id);
        mismatch |= reg->tpir ^ regData;

        regData = IpFma_Sms_GetDmtimerRegVal(DMTIMER_TNIR, id);
        mismatch |= reg->tnir ^ regData;

        regData = IpFma_Sms_GetDmtimerRegVal(DMTIMER_TCVR, id);
        mismatch |= reg->tcvr ^ regData;

        regData = IpFma_Sms_GetDmtimerRegVal(DMTIMER_TOCR, id);
        mismatch |= reg->tocr ^ regData;

        regData = IpFma_Sms_GetDmtimerRegVal(DMTIMER_TOWR, id);
        mismatch |= reg->towr ^ regData;
    }

    if (mismatch != 0U)
    {
        status = IPFMA_E_MISMATCH;
    }

    return status;
}

IpFma_Status IpFma_Sms_VerifyRtiRegs(const IpFma_SmsRtiRegs* reg)
{
    uint32_t regData = 0U, mismatch = 0U;
    IpFma_Status status = IPFMA_OK;

    if (NULL_PTR == reg)
    {
        status = IPFMA_E_PARAM;
    }

    if (IPFMA_OK == status)
    {
        regData = IpFma_Sms_GetRtiRegValue(RTI_GCTRL);
        mismatch |= reg->gctrl ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_TBCTRL);
        mismatch |= reg->tbctrl ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_CAPCTRL);
        mismatch |= reg->capctrl ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_COMPCTRL);
        mismatch |= reg->compctrl ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_FRC0);
        mismatch |= reg->frc0 ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_UC0);
        mismatch |= reg->uc0 ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_CPUC0);
        mismatch |= reg->cpuc0 ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_CAFRC0);
        mismatch |= reg->cafrc0 ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_CAUC0);
        mismatch |= reg->cauc0 ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_FRC1);
        mismatch |= reg->frc1 ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_UC1);
        mismatch |= reg->uc1 ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_CPUC1);
        mismatch |= reg->cpuc1 ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_CAFRC1);
        mismatch |= reg->cafrc1 ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_CAUC1);
        mismatch |= reg->cauc1 ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_COMP0);
        mismatch |= reg->comp0 ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_UDCP0);
        mismatch |= reg->udcp0 ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_COMP1);
        mismatch |= reg->comp1 ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_UDCP1);
        mismatch |= reg->udcp1 ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_COMP2);
        mismatch |= reg->comp2 ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_UDCP2);
        mismatch |= reg->udcp2 ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_COMP3);
        mismatch |= reg->comp3 ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_UDCP3);
        mismatch |= reg->udcp3 ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_TBLCOMP);
        mismatch |= reg->tblcomp ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_TBHCOMP);
        mismatch |= reg->tbhcomp ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_SETINT);
        mismatch |= reg->setint ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_CLEARINT);
        mismatch |= reg->clearint ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_INTFLAG);
        mismatch |= reg->intflag ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_DWDCTRL);
        mismatch |= reg->dwdctrl ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_DWDPRLD);
        mismatch |= reg->dwdprld ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_WDSTATUS);
        mismatch |= reg->wdstatus ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_WDKEY);
        mismatch |= reg->wdkey ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_DWWDRXNCTRL);
        mismatch |= reg->dwwdrxnctrl ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_DWWDSIZECTRL);
        mismatch |= reg->dwwdsizectrl ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_INTCLRENABLE);
        mismatch |= reg->intclrenable ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_COMP0CLR);
        mismatch |= reg->comp0clr ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_COMP1CLR);
        mismatch |= reg->comp1clr ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_COMP2CLR);
        mismatch |= reg->comp2clr ^ regData;

        regData = IpFma_Sms_GetRtiRegValue(RTI_COMP3CLR);
        mismatch |= reg->comp3clr ^ regData;
    }

    if (mismatch != 0U)
    {
        status = IPFMA_E_MISMATCH;
    }

    return status;
}

IpFma_Status IpFma_Sms_VerifyRtiMmrRegs(const IpFma_SmsRtiMmrRegs* reg)
{
    uint32_t regData = 0U, mismatch = 0U;
    IpFma_Status status = IPFMA_OK;

    if (NULL_PTR == reg)
    {
        status = IPFMA_E_PARAM;
    }

    if (IPFMA_OK == status)
    {
        regData = IpFma_Sms_GetRtiMmrRegValue(RTI_MMR_LOCK0_KICK0);
        mismatch |= reg->lock0_kick0 ^ regData;

        regData = IpFma_Sms_GetRtiMmrRegValue(RTI_MMR_LOCK0_KICK1);
        mismatch |= reg->lock0_kick1 ^ regData;

        regData = IpFma_Sms_GetRtiMmrRegValue(RTI_MMR_WWRTI_CTRL);
        mismatch |= reg->wwrti_ctrl ^ regData;

        regData = IpFma_Sms_GetRtiMmrRegValue(RTI_MMR_WWRTI_CLOCK_CTRL);
        mismatch |= reg->wwrti_clock_ctrl ^ regData;
    }

    if (mismatch != 0U)
    {
        status = IPFMA_E_MISMATCH;
    }

    return status;
}

IpFma_Status IpFma_Sms_VerifyPwrMmrRegs(const IpFma_SmsPwrMmrRegs* reg)
{
    uint32_t regData = 0U, mismatch = 0U;
    IpFma_Status status = IPFMA_OK;

    if (NULL_PTR == reg)
    {
        status = IPFMA_E_PARAM;
    }

    if (IPFMA_OK == status)
    {
        regData = IpFma_Sms_GetPwrMmrRegValue(PWR_MMR_LOCK0_KICK0);
        mismatch |= reg->lock0_kick0 ^ regData;

        regData = IpFma_Sms_GetPwrMmrRegValue(PWR_MMR_LOCK0_KICK1);
        mismatch |= reg->lock0_kick1 ^ regData;

        regData = IpFma_Sms_GetPwrMmrRegValue(PWR_MMR_PMCTRL_SYS);
        mismatch |= reg->pmctrl_sys ^ regData;

        regData = IpFma_Sms_GetPwrMmrRegValue(PWR_MMR_PMCTRL_MOSC);
        mismatch |= reg->pmctrl_mosc ^ regData;

        regData = IpFma_Sms_GetPwrMmrRegValue(PWR_MMR_PMCTRL_DMSC);
        mismatch |= reg->pmctrl_dmsc ^ regData;

        regData = IpFma_Sms_GetPwrMmrRegValue(PWR_MMR_CPU_STCALIB);
        mismatch |= reg->cpu_stcalib ^ regData;

        regData = IpFma_Sms_GetPwrMmrRegValue(PWR_MMR_PM_DMTIMER_CTRL);
        mismatch |= reg->pm_dmtimer_ctrl ^ regData;

        regData = IpFma_Sms_GetPwrMmrRegValue(PWR_MMR_PM_PERMISSION);
        mismatch |= reg->pm_permission ^ regData;
    }

    if (mismatch != 0U)
    {
        status = IPFMA_E_MISMATCH;
    }

    return status;
}

/** @} */
