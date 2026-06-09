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
 *  \file     esm.c
 *
 *  \brief    This file implements ESM(Error Signaling Module) code.
 */

/*===========================================================================*/
/*                            Include files                                  */
/*===========================================================================*/

#include <stdint.h>

#include <ti/osal/osal.h>
#include "esm.h"

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* Interrupt handlers */
HwiP_Handle gEsmHiHwiPHandle;
HwiP_Handle gEsmLoHwiPHandle;

/* Data used to check if the correct interrupt was caused. */
extern volatile Bool gDdrParityInterruptInvoked;
extern uint32_t gRamId;
extern uint32_t gParityInterruptId;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static int32_t cslAppClearESMIntrStatus(uint32_t baseAddr, int32_t intNum);
static int32_t cslAppEnableIntr(uint32_t baseAddr, int32_t intNum);
static int32_t cslAppSetPri(uint32_t baseAddr, uint32_t pri, int32_t intNum);
static int32_t cslAppEnableEsmGlobalIntr(uint32_t baseAddr);
static int32_t cslAppEsmSetupHighPriHandler(uint32_t esm_hi_pri_evt);
static int32_t cslAppEsmSetupLowPriHandler(uint32_t esm_lo_pri_evt);
static void    cslAppEsmHighInterruptHandler(uintptr_t arg);
static void    cslAppEsmLowInterruptHandler(uintptr_t arg);
static void    cslAppEsmInterruptHandler (esmIntrPriorityLvl_t esmIntrPriorityLvlType,
                                          uintptr_t arg );
static void    cslAppEsmProcessInterruptSource(uintptr_t arg, uint32_t intSrc);
static void    cslAppChkIsExpectedEvent(uint32_t eventId);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

static void cslAppEsmProcessInterruptSource(uintptr_t arg, uint32_t intSrc)
{
    if (intSrc != NO_EVENT_VALUE)
    {
        /* Clear this error */
        (void)ESMClearIntrStatus(((uint32_t) ESM_CFG_BASE), intSrc);
    }
    return;
}

static void cslAppEsmInterruptHandler(esmIntrPriorityLvl_t esmIntrPriorityLvlType, uintptr_t arg)
{
    uint32_t             intSrc1, intSrc2;
    esmGroupIntrStatus_t localEsmGroupIntrStatus;

    /* Check if the interrupt is caused by the expected RAM ID */
    cslAppChkIsExpectedEvent(arg);

    /* Check on the highest priority event and handle it */
    do {

        (void)ESMGetGroupIntrStatus(((uint32_t) ESM_CFG_BASE), (uint32_t)esmIntrPriorityLvlType, &localEsmGroupIntrStatus);

        intSrc1 = localEsmGroupIntrStatus.highestPendPlsIntNum;
        cslAppEsmProcessInterruptSource(arg, intSrc1);
        if (intSrc1 == (uint32_t) arg)
        {
            break;
        }

        intSrc2 = localEsmGroupIntrStatus.highestPendLvlIntNum;
        cslAppEsmProcessInterruptSource(arg, intSrc2);
        if (intSrc2 == (uint32_t) arg)
        {
            break;
        }
    } while ((intSrc1 != (uint32_t)(NO_EVENT_VALUE)) || (intSrc2 != (uint32_t)(NO_EVENT_VALUE)));

    return;
}

void cslAppEsmHighInterruptHandler(uintptr_t arg)
{
    uint32_t ecc_err_evt = (uint32_t) arg;

    /* Call common Interrupt handler. */
     cslAppEsmInterruptHandler(ESM_INTR_PRIORITY_LEVEL_HIGH, ecc_err_evt);

    /* Write end of interrupt. */
    (void)ESMClearIntrStatus(((uint32_t) ESM_CFG_BASE), ecc_err_evt);

    return;
}

void cslAppEsmLowInterruptHandler(uintptr_t arg)
{
    uint32_t ecc_err_evt = (uint32_t) arg;

    /* Call common Interrupt handler. */
    cslAppEsmInterruptHandler(ESM_INTR_PRIORITY_LEVEL_LOW, ecc_err_evt);

    /* Write end of interrupt. */
    (void)ESMClearIntrStatus(((uint32_t) ESM_CFG_BASE), ecc_err_evt);

    return;
}


static int32_t cslAppEsmSetupHighPriHandler(uint32_t esm_hi_pri_evt)
{
    HwiP_Params       hwiParams;
    uint32_t          intNumHi = ESM_HI_INT;

    HwiP_disableInterrupt(intNumHi);
    HwiP_Params_init(&hwiParams);
    hwiParams.arg = esm_hi_pri_evt;
    hwiParams.enableIntr = FALSE;
    /* Register the call back function for ESM Hi interrupt */
    gEsmHiHwiPHandle = HwiP_create(intNumHi,
                                   (HwiP_Fxn) cslAppEsmHighInterruptHandler,
                                   (void *)&hwiParams);
    HwiP_enableInterrupt(intNumHi);

    return (CSL_PASS);
}

static int32_t cslAppEsmSetupLowPriHandler(uint32_t esm_lo_pri_evt)
{
    HwiP_Params       hwiParams;
    uint32_t          intNumLo = ESM_LO_INT;

    HwiP_disableInterrupt(intNumLo);
    HwiP_Params_init(&hwiParams);
    hwiParams.arg = esm_lo_pri_evt;
    hwiParams.enableIntr = FALSE;
    /* register the call back function for ESM Lo interrupt */
    gEsmLoHwiPHandle = HwiP_create(intNumLo,
                                   (HwiP_Fxn) cslAppEsmLowInterruptHandler,
                                   (void *)&hwiParams);
    HwiP_enableInterrupt(intNumLo);
    return (CSL_PASS);
}

/* This function clears the ESM interrrupt status */
static int32_t cslAppClearESMIntrStatus(uint32_t baseAddr, int32_t intNum)
{
    int32_t   cslRet;
    uint32_t  intStatus;

    /* Clear interrupt status, so that we start with clean state */
    cslRet = ESMClearIntrStatus(baseAddr, intNum);

    if (cslRet == CSL_PASS)
    {
        cslRet = ESMGetIntrStatus(baseAddr, intNum, &intStatus);
    }
    if (cslRet == CSL_PASS)
    {
        if (intStatus != ((uint32_t)0U))
        {
            cslRet = CSL_EFAIL;
        }
    }
    return (cslRet);
}

/* Enable the ESM event */
static int32_t cslAppEnableIntr(uint32_t baseAddr, int32_t intNum)
{
    int32_t   cslRet;
    uint32_t  intStatus;

    /* Enable interrupt and verify if interrupt status is enabled */
    cslRet = ESMEnableIntr(baseAddr, intNum);

    if (cslRet == CSL_PASS)
    {
        cslRet = ESMIsEnableIntr(baseAddr, intNum, &intStatus);
    }
    if (cslRet == CSL_PASS)
    {
        if (intStatus != ((uint32_t)1U))
        {
            cslRet = CSL_EFAIL;
        }
    }
    return (cslRet);
}


/* Set the ESM Pri for that event */
static int32_t cslAppSetPri(uint32_t baseAddr, uint32_t pri, int32_t intNum)
{
    int32_t    cslRet;
    esmIntrPriorityLvl_t intrPriorityLvlWr, intrPriorityLvlRd;

    intrPriorityLvlWr = pri;

    cslRet = ESMSetIntrPriorityLvl(baseAddr, intNum, intrPriorityLvlWr);

    if (cslRet == CSL_PASS)
    {
        cslRet = ESMGetIntrPriorityLvl(baseAddr,
                                       intNum,
                                       &intrPriorityLvlRd);
    }
    if (cslRet == CSL_PASS)
    {
        if (intrPriorityLvlWr != intrPriorityLvlRd)
        {
            cslRet = CSL_EFAIL;
        }
    }
    return (cslRet);
}

/* Enable the global interrupt */
static int32_t cslAppEnableEsmGlobalIntr(uint32_t baseAddr)
{
    int32_t     cslRet;
    uint32_t    intStatus;

    /* Enable Global interrupt and verify if global interrupt is enabled for ESM */
    cslRet = ESMEnableGlobalIntr(baseAddr);

    if (cslRet == CSL_PASS)
    {
        cslRet = ESMGetGlobalIntrEnabledStatus(baseAddr, &intStatus);
    }
    if (cslRet == CSL_PASS)
    {
        if (intStatus != CSL_TEST_ESM_EN_KEY_ENABLE_VAL)
        {
            cslRet = CSL_EFAIL;
        }
    }
    return (cslRet);
}

/* ---------------------------------------------------- */
void cslAppChkIsExpectedEvent(uint32_t eventId)
{
    int32_t cslRet = CSL_PASS;
    bool isPend = FALSE;

    if (eventId == gParityInterruptId)
    {
        gDdrParityInterruptInvoked = TRUE;


        /**
         * Check if the interrupt is caused by the expected RAM ID.
         * Parity Errors are always Non Correctable errors so they are
         * reported as DED interrupt even though they are one bit.
         */
        if (cslRet == CSL_PASS)
        {
            cslRet = CSL_ecc_aggrIsEDCInterconnectIntrPending((CSL_ecc_aggrRegs*) DDR_ECC_AGGR_REGION_LOCAL_BASE, \
                                                              gRamId, \
                                                              CSL_ECC_AGGR_INTR_SRC_DOUBLE_BIT,
                                                              &isPend);
        }

        if ((cslRet == CSL_PASS) && (isPend == true))
        {
            cslRet = CSL_ecc_aggrClrEDCInterconnectNIntrPending((CSL_ecc_aggrRegs*) DDR_ECC_AGGR_REGION_LOCAL_BASE, \
                                                                gRamId, \
                                                                CSL_ECC_AGGR_INTR_SRC_DOUBLE_BIT, \
                                                                CSL_ECC_AGGR_ERROR_SUBTYPE_INJECT, \
                                                                1U);
            if (cslRet == CSL_PASS)
            {
                CSL_ecc_aggrAckIntr((CSL_ecc_aggrRegs*) DDR_ECC_AGGR_REGION_LOCAL_BASE, CSL_ECC_AGGR_INTR_SRC_DOUBLE_BIT);

            }

        }
    }
    else
    {
        UART_printf("\n ERROR: Wrong interrupt invoked: %u", eventId);
    }

    return;
}

/* Initialize ESM */
int32_t cslAppEsmSetup(CSL_esm_app_R5_cfg* cfg)
{
    int32_t     cslRet;

    /* ESM reset and configure */
    cslRet = ESMReset((uint32_t) ESM_CFG_BASE);
    if (cslRet != CSL_PASS)
    {
        UART_printf( "\r\nESM reset failed...");
    }
    if (cslRet == CSL_PASS)
    {
        cslRet = ESMReset((uint32_t) ESM_CFG_BASE);
        if (cslRet != CSL_PASS)
        {
            UART_printf( "\r\nMain ESM reset failed...");
        }
    }

    /* Remove current high priority interrupt handler. */
    if (gEsmHiHwiPHandle != NULL) {
        HwiP_delete(gEsmHiHwiPHandle);
        gEsmHiHwiPHandle = NULL;
    }

    /* Remove current low priority interrupt handler. */
    if (gEsmLoHwiPHandle != NULL) {
        HwiP_delete(gEsmLoHwiPHandle);
        gEsmLoHwiPHandle = NULL;
    }

    if (cslRet == CSL_PASS)
    {
        cslRet = cslAppClearESMIntrStatus((uint32_t) ESM_CFG_BASE, cfg->hi_pri_evt);
        if (cslRet != CSL_PASS)
        {
            UART_printf( "\r\n cslAppClearESMIntrStatus hi pri event failed...");
        }
    }

    if (cslRet == CSL_PASS)
    {
        cslRet = cslAppClearESMIntrStatus((uint32_t) ESM_CFG_BASE, cfg->lo_pri_evt);
        if (cslRet != CSL_PASS)
        {
            UART_printf( "\r\n cslAppClearESMIntrStatus lo pri event failed...");
        }
    }

     /* Enable interrupt and verify if interrupt status is enabled */
     if (cslRet == CSL_PASS)
    {
        cslRet = cslAppEnableIntr(((uint32_t) ESM_CFG_BASE), cfg->hi_pri_evt);
        if (cslRet != CSL_PASS)
        {
            UART_printf( "\r\nError in ESM Intr Enable for hi pri Event...");
        }
    }

    if (cslRet == CSL_PASS)
    {
         cslRet = cslAppEnableIntr(((uint32_t) ESM_CFG_BASE), cfg->lo_pri_evt);
         if (cslRet != CSL_PASS)
         {
            UART_printf( "\r\nError in ESM Intr Enable for lo pri Event...");
         }
    }

    /* Assign the priority for the events
     * Single bit errors can be corrected, so they are assigned to lo pri
     * Double bit errors can not be corrected, so they are assigned to hi pri
     */
    if (cslRet == CSL_PASS)
    {
        cslRet = cslAppSetPri(((uint32_t) ESM_CFG_BASE), ESM_INTR_PRIORITY_LEVEL_HIGH, cfg->hi_pri_evt);
        if (cslRet != CSL_PASS)
        {
            UART_printf( "\r\nError in setting Pri for hi pri Event...");
        }
    }

     if (cslRet == CSL_PASS)
     {
        cslRet = cslAppSetPri(((uint32_t) ESM_CFG_BASE), ESM_INTR_PRIORITY_LEVEL_LOW, cfg->lo_pri_evt);
        if (cslRet != CSL_PASS)
        {
            UART_printf( "\r\nError in setting Pri for lo pri Event...");
        }
     }

     if (cslRet == CSL_PASS)
     {
        cslRet = cslAppEnableEsmGlobalIntr((uint32_t) ESM_CFG_BASE);
        if (cslRet != CSL_PASS)
        {
            UART_printf( "\r\nESM Enable Global Interrupt Failed...");
        }
     }

    if (cslRet == CSL_PASS)
    {
        cslRet = cslAppEsmSetupHighPriHandler(cfg->hi_pri_evt);
        if (cslRet != CSL_PASS)
        {
            UART_printf( "\r\nESM High Priority handler setup Failed...");
        }
    }

    if (cslRet == CSL_PASS)
    {
        cslRet = cslAppEsmSetupLowPriHandler(cfg->lo_pri_evt);
        if (cslRet != CSL_PASS)
        {
            UART_printf( "\r\nESM Lo Priority handler setup Failed...");
        }
    }

    if (cslRet == CSL_PASS)
    {
        UART_printf( "\r\n cslAppEsmInit...Done");
    }
    return (cslRet);
}
