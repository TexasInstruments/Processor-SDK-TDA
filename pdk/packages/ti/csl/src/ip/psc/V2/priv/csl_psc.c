/**
 *   @file  csl_psc.h
 *
 *   @brief API implementation file for PSC CSL
 *
 *  \par
 *  ============================================================================
 *  @n   (C) Copyright 2002-2019 Texas Instruments, Inc.
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
#include <stdint.h>
#include <stdbool.h>
#include <ti/csl/csl_psc.h>

uint32_t CSL_PSC_getVersionInfo(const CSL_PscRegs *pPscRegs)
{
    return pPscRegs->PID;
}

uint8_t CSL_PSC_getVoltageControl(const CSL_PscRegs *pPscRegs)
{
    return (uint8_t)0U;
}

void CSL_PSC_setModuleNextState( CSL_PscRegs *pPscRegs, uint32_t moduleNum, CSL_PSC_MODSTATE state )
{
    CSL_FINS( pPscRegs->MDCTL[moduleNum], PSC_MDCTL_NEXT, state );
    RB_CHECK_FIELD32_VAL(pPscRegs->MDCTL[moduleNum], PSC_MDCTL_NEXT, state, CSL_PSC_readbackErr);
    return;
}

CSL_PSC_MODSTATE CSL_PSC_getModuleNextState( const CSL_PscRegs *pPscRegs, uint32_t moduleNum )
{
    return (CSL_PSC_MODSTATE)CSL_FEXT( pPscRegs->MDCTL[moduleNum], PSC_MDCTL_NEXT );
}

void CSL_PSC_setModuleLocalReset( CSL_PscRegs *pPscRegs, uint32_t moduleNum, CSL_PSC_MDLRST resetState )
{
    CSL_FINS( pPscRegs->MDCTL[moduleNum], PSC_MDCTL_LRST, resetState );
    RB_CHECK_FIELD32_VAL(pPscRegs->MDCTL[moduleNum], PSC_MDCTL_LRST, resetState, CSL_PSC_readbackErr);
    return;
}

CSL_PSC_MDLRST CSL_PSC_getModuleLocalReset( const CSL_PscRegs *pPscRegs, uint32_t moduleNum )
{
    return (CSL_PSC_MDLRST)CSL_FEXT( pPscRegs->MDCTL[moduleNum], PSC_MDCTL_LRST );
}

void CSL_PSC_enableModuleResetIsolation( CSL_PscRegs *pPscRegs, uint32_t moduleNum )
{
    CSL_FINST( pPscRegs->MDCTL[moduleNum], PSC_MDCTL_RSTISO, ENABLE );
    RB_CHECK_FIELD32_T(pPscRegs->MDCTL[moduleNum], PSC_MDCTL_RSTISO, ENABLE, CSL_PSC_readbackErr);
    return;
}

void CSL_PSC_disableModuleResetIsolation( CSL_PscRegs *pPscRegs, uint32_t moduleNum )
{
    CSL_FINST( pPscRegs->MDCTL[moduleNum], PSC_MDCTL_RSTISO, DISABLE );
    RB_CHECK_FIELD32_T(pPscRegs->MDCTL[moduleNum], PSC_MDCTL_RSTISO, DISABLE, CSL_PSC_readbackErr);
    return;
}

bool CSL_PSC_isModuleResetIsolationEnabled( const CSL_PscRegs *pPscRegs, uint32_t moduleNum )
{
    return CSL_FEXT( pPscRegs->MDCTL[moduleNum], PSC_MDCTL_RSTISO ) ? (bool)true : (bool)false;
}

CSL_PSC_MODSTATE CSL_PSC_getModuleState( const CSL_PscRegs *pPscRegs, uint32_t moduleNum )
{
    return (CSL_PSC_MODSTATE)CSL_FEXT( pPscRegs->MDSTAT[moduleNum], PSC_MDSTAT_STATE );
}

CSL_PSC_MDLRST CSL_PSC_getModuleLocalResetStatus( const CSL_PscRegs *pPscRegs, uint32_t moduleNum )
{
    return (CSL_PSC_MDLRST)CSL_FEXT( pPscRegs->MDSTAT[moduleNum], PSC_MDSTAT_LRST );
}

bool CSL_PSC_isModuleLocalResetDone( const CSL_PscRegs *pPscRegs, uint32_t moduleNum )
{
    return CSL_FEXT( pPscRegs->MDSTAT[moduleNum], PSC_MDSTAT_LRSTDONE ) ? (bool)true : (bool)false;
}

CSL_PSC_MDRST CSL_PSC_getModuleResetStatus( const CSL_PscRegs *pPscRegs, uint32_t moduleNum )
{
    return (CSL_PSC_MDRST)CSL_FEXT( pPscRegs->MDSTAT[moduleNum], PSC_MDSTAT_MRST );
}

bool CSL_PSC_isModuleResetDone( const CSL_PscRegs *pPscRegs, uint32_t moduleNum )
{
    return CSL_FEXT( pPscRegs->MDSTAT[moduleNum], PSC_MDSTAT_MRSTDONE ) ? (bool)true : (bool)false;
}

bool CSL_PSC_isModuleClockOn( const CSL_PscRegs *pPscRegs, uint32_t moduleNum )
{
    return CSL_FEXT( pPscRegs->MDSTAT[moduleNum], PSC_MDSTAT_MCKOUT ) ? (bool)true : (bool)false;
}

void CSL_PSC_enablePowerDomain( CSL_PscRegs *pPscRegs, uint32_t pwrDmnNum )
{
    CSL_FINST( pPscRegs->PDCTL[pwrDmnNum], PSC_PDCTL_NEXT, ON );
    RB_CHECK_FIELD32_T(pPscRegs->PDCTL[pwrDmnNum], PSC_PDCTL_NEXT, ON, CSL_PSC_readbackErr);
    return;
}

void CSL_PSC_disablePowerDomain( CSL_PscRegs *pPscRegs, uint32_t pwrDmnNum )
{
    CSL_FINST( pPscRegs->PDCTL[pwrDmnNum], PSC_PDCTL_NEXT, OFF );
    RB_CHECK_FIELD32_T(pPscRegs->PDCTL[pwrDmnNum], PSC_PDCTL_NEXT, OFF, CSL_PSC_readbackErr);
    return;
}

CSL_PSC_PDSTATE CSL_PSC_getPowerDomainState( const CSL_PscRegs *pPscRegs, uint32_t pwrDmnNum )
{
    return (CSL_PSC_PDSTATE)CSL_FEXT( pPscRegs->PDSTAT[pwrDmnNum], PSC_PDSTAT_STATE );
}

void CSL_PSC_startStateTransition( CSL_PscRegs *pPscRegs, uint32_t pwrDmnNum )
{
    uint32_t  pwrDmnGrp      = pwrDmnNum >> 5U;
    uint32_t  pwrDmnNumInGrp = pwrDmnNum & 0x1FU;
    pPscRegs->PTCMD[pwrDmnGrp] = (1 << pwrDmnNumInGrp);
    /* This is a pseudo-command register with no actual storage. Reads return 0.
     * According J784S4_Registers_Public_20250808.xlsx */
    RB_CHECK_REG32(&pPscRegs->PTCMD[pwrDmnGrp], (uint32_t)0U, RB_MASK_ALL_BITS_32, CSL_PSC_readbackErr);
    return;
}

uint32_t CSL_PSC_isStateTransitionDone( const CSL_PscRegs *pPscRegs, uint32_t pwrDmnNum )
{
    uint32_t  pdTransStatus;

    uint32_t  pwrDmnGrp      = pwrDmnNum >> 5U;
    uint32_t  pwrDmnNumInGrp = pwrDmnNum & 0x1FU;
    pdTransStatus = CSL_FEXTR( pPscRegs->PTSTAT[pwrDmnGrp], pwrDmnNumInGrp, pwrDmnNumInGrp );
    return pdTransStatus ? (uint32_t)0U : (uint32_t)1U;
}

__attribute__((weak)) void CSL_PSC_readbackErr(uint32_t expVal, uint32_t readVal)
{
    // this is a default empty function       
}
