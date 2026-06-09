/**
 *   Copyright (c) Texas Instruments Incorporated 2018
 *   All rights reserved.
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
 *
 */

/**
 *  \file csl_dmpacTop.c
 *
 *  \brief File implementing common APIs, functionality of DOF/SDE module
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <src/csl/include/csl_dmpactop.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
#define DMPAC_MODULE_ENABLE_BIT    (0U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void CSL_dmpacEnableModule(CSL_dmpacRegs *dmpacTopRegs,
                           uint32_t module,
                           uint32_t enable)
{
    volatile uint32_t regVal;
	
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> 
Rationale: The negative test framework and test apps cannot reach this portion. 
                      'dmpacTopRegs' is passed using Fvid2Control from Imaging kernel layer along with pointer to instObj which holds the pointer to dmpacTopRegs register, instObj is defined as a global gM2mVissInstObj.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
    if (NULL != dmpacTopRegs)
/* LDRA_JUSTIFY_END */
    {
        regVal = CSL_REG32_RD(&dmpacTopRegs->ENABLE);
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> 
Rationale: The negative test framework and test apps cannot reach this portion for false condition, as the function is calling from sdeand dof Api.c file with 'enable' value as UTRUE only.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
        if ((uint32_t)UTRUE == enable)
/* LDRA_JUSTIFY_END */
        {
            /* Enable DOF/SDE module */
            regVal |= ((uint32_t)0x1U << (uint32_t)DMPAC_MODULE_ENABLE_BIT);
            regVal |= ((uint32_t)0x1U << (uint32_t)module);
        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> 
Rationale: The negative test framework and test apps cannot reach this portion as the function is calling from sdeand dof Api.c file with 'enable' value as UTRUE only.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
        else
        {
            /* Disable DOF/SDE module */
            regVal &= ~(0x1U << (uint32_t)DMPAC_MODULE_ENABLE_BIT);
            regVal &= ~(0x1U << (uint32_t)module);
        }
/* LDRA_JUSTIFY_END */
        CSL_REG32_WR(&dmpacTopRegs->ENABLE, regVal);
    }
}

int32_t CSL_dmpacFocoSetConfig(CSL_dmpac_foco_coreRegs *focoRegs,
                               uint32_t channel,
                               const Dmpac_FocoConfig *cfg)
{
    volatile uint32_t regVal;
    int32_t  status = CSL_PASS;
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> 
Rationale: The negative test framework and test apps cannot reach this portion. 
           'focoRegs' and 'cfg' are passed using Fvid2Control from Imaging kernel layer along with pointer to instObj which holds the pointer to focoRegs register, instObj is defined as a global gM2mVissInstObj.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
    if ((NULL == focoRegs) || (NULL == cfg))
    {
        status = CSL_EBADARGS;
    }
    if(status == CSL_PASS)
/* LDRA_JUSTIFY_END */
    {
        /* Configure the DMPAC FOCO Module */
        regVal = CSL_REG32_RD(&focoRegs->FOCO_CTRL[channel].CH_COUNT);
        CSL_FINS(
              regVal,
              DMPAC_FOCO_CORE_FOCO_CTRL_CH_COUNT_PRELOAD,
              cfg->preload);
        CSL_FINS(
              regVal,
              DMPAC_FOCO_CORE_FOCO_CTRL_CH_COUNT_POSTLOAD,
              cfg->postload);
        CSL_FINS(
              regVal,
              DMPAC_FOCO_CORE_FOCO_CTRL_CH_COUNT_TRIG,
              cfg->trig);
        CSL_REG32_WR(&focoRegs->FOCO_CTRL[channel].CH_COUNT, regVal);

        regVal = CSL_REG32_RD(&focoRegs->FOCO_CTRL[channel].CH_CTRL);
        CSL_FINS(
              regVal,
              DMPAC_FOCO_CORE_FOCO_CTRL_CH_CTRL_CH_EN,
              cfg->channelEnable);
        CSL_FINS(
              regVal,
              DMPAC_FOCO_CORE_FOCO_CTRL_CH_CTRL_SHIFT_EN,
              cfg->shiftEnable);
        CSL_FINS(
              regVal,
              DMPAC_FOCO_CORE_FOCO_CTRL_CH_CTRL_SHIFT_M1,
              cfg->shiftM1);
        CSL_FINS(
              regVal,
              DMPAC_FOCO_CORE_FOCO_CTRL_CH_CTRL_DIR,
              cfg->dir);
        CSL_FINS(
              regVal,
              DMPAC_FOCO_CORE_FOCO_CTRL_CH_CTRL_ROUND,
              cfg->round);
        CSL_FINS(
              regVal,
              DMPAC_FOCO_CORE_FOCO_CTRL_CH_CTRL_MASK,
              cfg->mask);
        CSL_REG32_WR(&focoRegs->FOCO_CTRL[channel].CH_CTRL, regVal);
    }

    return status;
}

