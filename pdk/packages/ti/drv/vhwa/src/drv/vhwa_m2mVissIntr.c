/**
 *   Copyright (c) Texas Instruments Incorporated 2019
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
 */

/**
 *  \file vhwa_m2mVissIntr.c
 *
 *  \brief Implementation for Interrupt related APIs
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "src/drv/vhwa_m2mVissPriv.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief   ISR handler for VISS interrupt
 *          Registered to the BIOS
 *
 **/
static void vhwaM2mVissIsr(uintptr_t args);

/**
 * \brief   Frame Complete ISR,
 *          It is called from #vhwaM2mVissIsr when VISS completes a frame
 *          and HTS pipeline is done.
 *
 * \param   instObj             Instance object
 *
 **/
static void vhwaM2mVissFrmDoneIsr(Vhwa_M2mVissInstObj *instObj);

/**
 * \brief   Error Event ISR,
 *          It is called from #vhwaM2mVissIsr when VISS generates error event.
 *
 * \param   instObj             Instance object
 * \param   errStat             Bit mask of occurred error events
 *
 **/
static void vhwaM2mVissErrEventIsr(Vhwa_M2mVissInstObj *instObj,
    uint32_t errStat);

/**
 * \brief   WdTimer Error Event ISR,
 *          It is called from #vhwaM2mVissIsr when VISS generates error event.
 *
 * \param   instObj             Instance object
 * \param   errStat             Bit mask of occurred error events
 *
 **/
static void vhwaM2mVissWdTimerErrEventIsr(Vhwa_M2mVissInstObj *instObj,
                                    uint32_t wdTimer_errStat);

/**
 * \brief   Function to get status of the frame done event
 *
 * \param   instObj             Instance object
 * \param   errStat             Bit mask of occurred error events
 * \param   wdTimer_errStat     Status of the watchdog timer error event
 * \param   frmDoneStat         Status of the frame done event.
 *
 **/
static void vhwaM2mVissGetIntrStat(const Vhwa_M2mVissInstObj *instObj,
                                    uint32_t *errStat,
                                    uint32_t *wdTimer_errStat,
                                    uint32_t *frmDoneStat,
                                    uint32_t   isFlexConnect);


/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */



/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t Vhwa_m2mVissRegisterIsr(Vhwa_M2mVissInstObj *instObj)
{
    int32_t                          status = FVID2_SOK;
    uint32_t                         events;
    uint32_t                         vhwaIrqNum;
    HwiP_Params                      hwiParams;
    volatile CSL_vpac_intd_cfgRegs  *intdRegs = NULL;
    uint32_t                         wdtimer_events;

    /* Check for Null Pointer */
    GT_assert(VhwaVissTrace, (NULL != instObj));

    /* Clear out any pending interrupts */
    VhwaAl_HwiP_clearInterrupt(instObj->irqNum);

    /* Populate the interrupt parameters */
    HwiP_Params_init(&hwiParams);
    #if defined(MCU_PLUS_SDK)
    hwiParams.args = (void*) instObj;
    hwiParams.intNum = (uint32_t) instObj->irqNum;
    hwiParams.callback = (HwiP_FxnCallback) &vhwaM2mVissIsr;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(FVID2_SOK != HwiP_construct(&instObj->intrHndl, &hwiParams))
    #else
    hwiParams.arg = (uintptr_t) instObj;

    /* Register interrupt */
    instObj->intrHndl = HwiP_create(instObj->irqNum,
        &vhwaM2mVissIsr, &hwiParams);
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */    
    if (NULL == instObj->intrHndl)
    #endif
    {
        GT_0trace(VhwaVissTrace, GT_ERR,
            "Interrupt Registration Failed !!\n");
        status = FVID2_EFAIL;
    }
    /* LDRA_JUSTIFY_END */
    /* Enable Error Events */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        vhwaIrqNum  = instObj->vhwaIrqNum;
        intdRegs    = instObj->socInfo.vpacIntdRegs;

        /* All Error Events are by default enabled */
        events = VHWA_VISS_ALL_ERR_EVENTS;
        /* Enable HTS Interrupt in INTD Module */
        CSL_REG32_WR(&intdRegs->ENABLE_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_VISS_INTR_REG],
            events);

        /* All Wdtimer Error Events are by default disabled */
        wdtimer_events = VHWA_VISS_WDTIMER_ERR;
        /* Enable HTS Interrupt in INTD Module */
        CSL_REG32_WR(&intdRegs->ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_WDTIMER_INTR_REG],
            wdtimer_events);
    }
    return (status);
}

void Vhwa_m2mVissUnregisterIsr(Vhwa_M2mVissInstObj *instObj)
{
    uint32_t                        events;
    uint32_t                        vhwaIrqNum;
    CSL_vpac_intd_cfgRegs           *intdRegs = NULL;
    uint32_t                         wdtimer_events;

    /* Check for Null Pointer */
    GT_assert(VhwaVissTrace, (NULL != instObj));
	
    /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end> 
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != instObj)
    /* LDRA_JUSTIFY_END */
    {
        vhwaIrqNum  = instObj->vhwaIrqNum;
        intdRegs    = instObj->socInfo.vpacIntdRegs;

        /* If not already done, disable HTS Interrupt */
        Vhwa_disableHtsIntr(intdRegs, instObj->vhwaIrqNum, instObj->pipeline);

        /* All Error Events are by default enabled */
        events = VHWA_VISS_ALL_ERR_EVENTS;
        /* Disable HTS Interrupt in INTD Module */
        CSL_REG32_WR(&intdRegs->ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_VISS_INTR_REG],
            events);

        /* All Wdtimer Error Events are by default enabled */
        wdtimer_events = VHWA_VISS_WDTIMER_ERR;
         /* Disable Watchdog Timer Interrupt in INTD Module */
        CSL_REG32_WR(&intdRegs->ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_WDTIMER_INTR_REG],
            wdtimer_events);

#if defined(MCU_PLUS_SDK)
        (void)HwiP_destruct(&instObj->intrHndl);
#else
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> Rationale: The negative test framework and test apps cannot reach this portion.
         These parameters cannot be configured from test app or kernel directly and can only be overwritten by memory corruption. This is fail safe mechanism and cannot be removed.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */   
        if (NULL != instObj->intrHndl)
        /* LDRA_JUSTIFY_END */
        {
            /* Un-register interrupt */
            (void)HwiP_delete(instObj->intrHndl);
            instObj->intrHndl = NULL;
        }
#endif
    }
}

static void vhwaM2mVissIsr(uintptr_t args)
{
    uint32_t             errStat        = 0U;
    uint32_t             frmDoneStat    = 0U;
    uint32_t            wdTimer_errStat = 0U;
    Vhwa_M2mVissInstObj *instObj = NULL;
    Vhwa_M2mVissHandleObj            *hObj = NULL;

    GT_assert(VhwaVissTrace, (NULL != (Vhwa_M2mVissInstObj *)args));

    instObj = (Vhwa_M2mVissInstObj *)args;

    instObj->totalIntrCnt ++;
	
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL != instObj->actQObj) && (NULL != instObj->actQObj->hObj))
    {
        hObj = instObj->actQObj->hObj;
    }

    if(NULL != hObj)
    /* LDRA_JUSTIFY_END */
    {
        vhwaM2mVissGetIntrStat(instObj, &errStat, &wdTimer_errStat, &frmDoneStat, hObj->fcStatus.isFlexConnect);
    }
	
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The test framework and test apps cannot reach this portion as this requires a separate negative build.
    However, this portion is covered in VHWA_TEST_NEGATIVE build.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    This has been already tested out in the Negative test build.
    <justification end> */
    if (0U != errStat)
    {
        /* Check if the error is a stalling error interrupt.
         * Only stalling errors cause IP to stop processing and require setting
         * the isStalled flag to prevent further process requests. */
        if (0U != (errStat & VHWA_VISS_STALLING_ERR_MASK))
        {
            /* Set stalled flag to prevent further process requests */
            instObj->isStalled = (uint32_t)UTRUE;
        }

        /* First check if any of the registered error events has come,
           then first call the callback for the error events */
        vhwaM2mVissErrEventIsr(instObj, errStat);
    }
    /* LDRA_JUSTIFY_END */
 
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The test framework and test apps cannot reach this portion as this requires a separate negative build.
    However, this portion is covered in VHWA_TEST_NEGATIVE build.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    This has been already tested out in the Negative test build.
    <justification end> */
    if (0U != wdTimer_errStat)
    {
        /* Set stalled flag to prevent further process requests */
        instObj->isStalled = (uint32_t)UTRUE;

        if(NULL != hObj)
        {
            /* Disable watchdog timer after its first trigger*/
            hObj->htsCfg.enableWdTimer = UFALSE;

            CSL_htsDisableWatchdogTimer(instObj->socInfo.htsRegs, &hObj->htsCfg);
        }
        /* First check if any of the registered error events has come,
           then first call the callback for the error events */
        vhwaM2mVissWdTimerErrEventIsr(instObj, wdTimer_errStat);
    }
    /* LDRA_JUSTIFY_END */

    /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end> 
    <justification start>
    Rationale: This is covered in a test framework of type negative build since this requires altering the source code macros to trigger the error.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (0U != frmDoneStat)
    /* LDRA_JUSTIFY_END */
    {
        /* Now process the Frame Done ISR */
        vhwaM2mVissFrmDoneIsr(instObj);
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end> 
	<justification start>
    <justification start>
    Rationale: This is covered in a test framework of type negative build since this requires altering the source code macros to trigger the error.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if((0u == errStat) && (0U == wdTimer_errStat) && (0U == frmDoneStat))
    {
        instObj->errIntrInvalidStatus ++;
    }
    /* LDRA_JUSTIFY_END */
}

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: The test framework and test apps cannot reach this portion as this requires a separate negative build.
However, this portion is covered in VHWA_TEST_NEGATIVE build.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
This has been already tested out in the Negative test build.
<justification end> */
static void vhwaM2mVissErrEventIsr(Vhwa_M2mVissInstObj *instObj,
    uint32_t errStat)
{
    Viss_ErrEventParams              *eePrms = NULL;
    Vhwa_M2mVissHandleObj            *hObj = NULL;
    uint32_t                         errStatus;


    if (NULL != instObj)
    {
        if ((NULL == instObj->actQObj) || (NULL == instObj->actQObj->hObj))
        {
            instObj->errNoActQObj ++;
            if (NULL != instObj->lastHndlObj)
            {
                hObj = instObj->lastHndlObj;
                eePrms = &hObj->eePrms;
            }
            else
            {
                hObj = instObj->lastHndlObj;
                instObj->errNoLastHndlObj ++;
            }
        }
        else
        {
            hObj   = instObj->actQObj->hObj;
            eePrms = &hObj->eePrms;
        }


        if(NULL != hObj)
        {
            /* Check if any of the error event has occurred */
            errStatus = errStat & eePrms->errEvents;
            /* Process only if error events are enabled for this handle */
            if (0U != errStatus)
            {
                /* Call Application callback, for the completed handle object */
                if (NULL != eePrms->cbFxn)
                {
                    eePrms->cbFxn((Fvid2_Handle)hObj, errStatus, eePrms->appData);
                }
            }
        }
    }
}
/* LDRA_JUSTIFY_END */

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: The test framework and test apps cannot reach this portion as this requires a separate negative build.
However, this portion is covered in VHWA_TEST_NEGATIVE build.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
This has been already tested out in the Negative test build.
<justification end> */
static void vhwaM2mVissWdTimerErrEventIsr(Vhwa_M2mVissInstObj *instObj,
                                    uint32_t wdTimer_errStat)
{
    uint32_t                          errStatus;
    Viss_WdTimerErrEventParams       *wdTimerEePrms = NULL;
    Vhwa_M2mVissHandleObj            *hObj = NULL;

    if (NULL != instObj)
    {
        if ((NULL == instObj->actQObj) || (NULL == instObj->actQObj->hObj))
        {
            instObj->errNoActQObj ++;
            if (NULL != instObj->lastHndlObj)
            {
                hObj = instObj->lastHndlObj;
                wdTimerEePrms = &hObj->wdTimerEePrms;
            }
            else
            {
                hObj = NULL;
                instObj->errNoLastHndlObj ++;
            }
        }
        else
        {
            hObj   = instObj->actQObj->hObj;
            wdTimerEePrms = &hObj->wdTimerEePrms;
        }

        if(NULL != hObj)
        {
            /* Check if any of the error event has occurred */
            errStatus = wdTimer_errStat & wdTimerEePrms->wdTimerErrEvents;
            /* Process only if error events are enabled for this handle */
            if (0U != errStatus)
            {
                /* Call Application callback, for the completed handle object */
                if (NULL != wdTimerEePrms->cbFxn)
                {
                    wdTimerEePrms->cbFxn((Fvid2_Handle)hObj, errStatus, wdTimerEePrms->appData);
                }
            }
        }
    }
}
/* LDRA_JUSTIFY_END */
static void vhwaM2mVissFrmDoneIsr(Vhwa_M2mVissInstObj *instObj)
{
    Vhwa_M2mVissHandleObj *doneHndlObj = NULL;
    Vhwa_M2mVissQueueObj  *qObj = NULL;

   /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != instObj)
    /* LDRA_JUSTIFY_END */
    {
        /**
         * Steps
         * 1, Clear the interrupt
         * 2, Move the queue object to doneQ
         * 3, Mark Active Queue Object to NULL and lastHandle to
         *    completed handle
         * 4, If the reqQ is not empty, Remove one object from reqQ
         * 5, if the last handle is same as this handle, submit the TR
         * 6, If they are not same, set the config and then submit the TR
         * 7, move the new object to actQ
         * 8, Call the application callback
         */
        /* Active Handle Object must not be null */
        GT_assert(VhwaVissTrace, (NULL != instObj->actQObj));

        qObj = instObj->actQObj;

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != qObj)
        /* LDRA_JUSTIFY_END */
        {
            /* Handle Object in QObject must not be null */
            GT_assert(VhwaVissTrace, (NULL != qObj->hObj));

            doneHndlObj = qObj->hObj;
			
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> 
            Rationale: The negative test framework and test apps cannot reach this portion.
            These parameters cannot be configured from test app or kernel directly and can only be overwritten by memory corruption. This is fail safe mechanism and cannot be removed
            Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (0u == CSL_htsPipelineStatus(instObj->regAddrs.htsRegs,
                &doneHndlObj->htsCfg))
            /* LDRA_JUSTIFY_END */
            {
                /* Get the timestmap for performance numbers */
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the caller before the control reaches here.
                Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != doneHndlObj->createArgs.getTimeStamp)
                {
                    doneHndlObj->perf =
                        doneHndlObj->createArgs.getTimeStamp() -
                        doneHndlObj->perf;
                }
                /* LDRA_JUSTIFY_END */

                /* Move completed qObject to done queue */
                Fvid2Utils_queue(doneHndlObj->doneQ,
                    &qObj->qElem, qObj);

                instObj->lastHndlObj = doneHndlObj;
                instObj->actQObj = NULL;
                
                if ((uint32_t)UTRUE == doneHndlObj->createArgs.enablePsa)
                {
                    CSL_lseGetPsaSign(instObj->regAddrs.lseRegs,
                                      &doneHndlObj->lseCfg,
                                      &doneHndlObj->psa.psaSign[0U]);
                }

                /* Call Application callback, for the completed handle object */
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start> Rationale: These parameters cannot be configured from test app or kernel directly and can only be overwritten by memory corruption. 
                                                This is fail safe mechanism and cannot be removed
                Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if(NULL != doneHndlObj->cbPrms.fdmCbFxn)
                /* LDRA_JUSTIFY_END */
                {
                    (void)doneHndlObj->cbPrms.fdmCbFxn(
                        doneHndlObj->cbPrms.fdmData);
                }
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the caller before the control reaches here.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            else
            {
                instObj->errInvalidHtsPipelineStatus ++;
            }
            /* LDRA_JUSTIFY_END */
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            instObj->errIntrRepeateIntr ++;
        }
         /* LDRA_JUSTIFY_END */
    }
}

static void vhwaM2mVissGetIntrStat(const Vhwa_M2mVissInstObj *instObj,
                                    uint32_t *errStat,
                                    uint32_t *wdTimer_errStat,
                                    uint32_t *frmDoneStat,
                                    uint32_t  isFlexConnect)
{
    uint32_t                        vhwaIrqNum;
    volatile uint32_t               regVal;
    volatile CSL_vpac_intd_cfgRegs *intdRegs = NULL;
    uint32_t htsPipeline;

    if(0U == isFlexConnect)
    {
        htsPipeline = instObj->pipeline;
    }
    else
    {
        htsPipeline = instObj->fcPipeline;
    }
	
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, then our code base is expected to prevent undefined behaviour due to dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != instObj)
    /* LDRA_JUSTIFY_END */
    {
        vhwaIrqNum  = instObj->vhwaIrqNum;
        intdRegs    = instObj->socInfo.vpacIntdRegs;

        vhwa_GetErrIntrStat(&intdRegs->STATUS_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_VISS_INTR_REG],
                        &intdRegs->STATUS_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_VISS_INTR_REG], errStat, VHWA_VPAC_INTD_VISS_ERR_MASK);

        vhwa_GetWdtimerErrIntrStat(&intdRegs->STATUS_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_WDTIMER_INTR_REG],
                        &intdRegs->STATUS_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_WDTIMER_INTR_REG], wdTimer_errStat, VHWA_VPAC_INTD_VISS_WDTIMER_ERR_BIT);

        regVal = CSL_REG32_RD(
            &intdRegs->STATUS_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_PIPE_INTR_REG]);

        *frmDoneStat = 0U;
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is NOT expected to accumulate the error status.
        An else block can added to catch and print the error status in a future release.
        <justification end> */
        if (0U != (regVal & ((uint32_t)1u << htsPipeline)))
        /* LDRA_JUSTIFY_END */
        {
            /* Clear the interrupt */
            regVal = ((uint32_t)1u << htsPipeline);
            CSL_REG32_WR(
                &intdRegs->STATUS_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_PIPE_INTR_REG],
                regVal);

            *frmDoneStat = 1U;
        }
    }
}
