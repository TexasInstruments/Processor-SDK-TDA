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
 *  \file vhwa_m2mMscIntr.c
 *
 *  \brief Interrupt handling for MSC module
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "src/drv/vhwa_m2mMscPriv.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
extern Vhwa_M2mMscCommonObj   gM2mMscCommonObj;
extern const CSL_HtsHwaParams gCslHtsParams[CSL_HTS_HWA_MAX_SCH];

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static void vhwa_m2mMscHtsThreadStop(const CSL_MscHtsConfigReg *mscHtsRegs, const CSL_HtsSchConfig *cfg);

/**
 * \brief   ISR handler for MSC interrupt
 *          Registered to the BIOS for Instance 0
 *
 **/
static void vhwa_m2mMscIsr0(uintptr_t args);

/**
 * \brief   ISR handler for MSC interrupt
 *          Registered to the BIOS for Instance 1
 *
 **/
static void vhwa_m2mMscIsr1(uintptr_t args);

/**
 * \brief   Frame Complete ISR,
 *          It is called from #vhwa_m2mMscIsr0 when MSC completes a frame
 *          and HTS pipeline is done.
 *
 * \param   instObj             Instance object
 *
 **/
static void vhwa_m2mMscFrmDoneIsr(Vhwa_M2mMscInstObj *instObj,
                                    Vhwa_M2mMscCommonObj *comObj);

/**
 * \brief   Error Event ISR,
 *          It is called from #vhwa_m2mMscIsr0 when MSC generates error event.
 *
 * \param   instObj             Instance object
 * \param   errStat             Bit mask of occurred error events
 *
 **/
static void vhwa_m2mMscErrEventIsr(Vhwa_M2mMscInstObj *instObj,
                                    uint32_t errStat);
/**
 * \brief   WdTimer Error Event ISR,
 *          It is called from #vhwa_m2mMscIsr0 when MSC generates error event.
 *
 * \param   instObj             Instance object
 * \param   errStat             Bit mask of occurred error events
 *
 **/
static void vhwa_m2mMscWdTimerErrEventIsr(Vhwa_M2mMscInstObj *instObj,
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
static void vhwa_m2mMscGetIntrStat(const Vhwa_M2mMscInstObj *instObj,
                                   const Vhwa_M2mMscCommonObj *comObj,
                                    uint32_t instId,
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

int32_t Vhwa_m2mMscRegisterIsr(Vhwa_M2mMscInstObj *instObj,
                               const Vhwa_M2mMscCommonObj *comObj,
                               uint32_t instId)
{
    int32_t                          status = FVID2_SOK;
    uint32_t                         events;
    uint32_t                         vhwaIrqNum;
    HwiP_Params                      hwiParams;
    volatile CSL_vpac_intd_cfgRegs  *intdRegs = NULL;
    uint32_t                         wdtimer_events;

    GT_assert(VhwaMscTrace, (NULL != instObj));
    GT_assert(VhwaMscTrace, (NULL != comObj));

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale:The current test framework and applications cannot reach this portion.
    The instId has been passed from the imaging kernel, and it is limited to only two possible values: VPAC_MSC_INST_ID_0 and VPAC_MSC_INST_ID_1.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((instId != VPAC_MSC_INST_ID_0) &&
        (instId != VPAC_MSC_INST_ID_1))
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>Rationale:The current test framework and applications cannot reach this portion.
    The instId has been passed from the imaging kernel, and it is limited to only two possible values: VPAC_MSC_INST_ID_0 and VPAC_MSC_INST_ID_1.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Clear out any pending interrupts */
        VhwaAl_HwiP_clearInterrupt(comObj->mscInitPrms.irqInfo[instId].irqNum);

        /* Populate the interrupt parameters */
        HwiP_Params_init(&hwiParams);
        #if defined(MCU_PLUS_SDK)
        hwiParams.args = (void*) instObj;
        hwiParams.intNum = (uint32_t) comObj->mscInitPrms.irqInfo[instId].irqNum;
        #else
        hwiParams.arg = (uintptr_t) instObj;
        #endif
        if (VPAC_MSC_INST_ID_0 == instId)
        {
            /* Register interrupt */
        #if defined(MCU_PLUS_SDK)
            hwiParams.callback = (HwiP_FxnCallback) &vhwa_m2mMscIsr0;
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if(FVID2_SOK != HwiP_construct(&instObj->intrHandle, &hwiParams))
        #else
            instObj->intrHandle = HwiP_create(
                            comObj->mscInitPrms.irqInfo[instId].irqNum,
                            &vhwa_m2mMscIsr0, &hwiParams);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL == instObj->intrHandle)
        #endif
            {
                GT_0trace(VhwaMscTrace, GT_ERR,
                    "Interrupt Registration Failed !!\n");
                status = FVID2_EFAIL;
            }
            /* LDRA_JUSTIFY_END */
        }
        else
        {
            /* Register interrupt */
        #if defined(MCU_PLUS_SDK)
            hwiParams.callback = (HwiP_FxnCallback) &vhwa_m2mMscIsr1;
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if(FVID2_SOK != HwiP_construct(&instObj->intrHandle, &hwiParams))
        #else
            instObj->intrHandle = HwiP_create(
                            comObj->mscInitPrms.irqInfo[instId].irqNum,
                                    &vhwa_m2mMscIsr1, &hwiParams);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL == instObj->intrHandle)
        #endif
            {
                GT_0trace(VhwaMscTrace, GT_ERR,
                    "Interrupt Registration Failed !!\n");
                status = FVID2_EFAIL;
            }
            /* LDRA_JUSTIFY_END */
        }
    }

    /* Enable Error Events */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        vhwaIrqNum  = comObj->mscInitPrms.irqInfo[instId].vhwaIrqNum;
        intdRegs    = comObj->socInfo.vpacIntdRegs;

        /* All Error Events are by default enabled */
        events = VHWA_MSC_ALL_ERR_EVENTS;
        /* Enable HTS Interrupt in INTD Module */
        CSL_REG32_WR(&intdRegs->ENABLE_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_MSC_INTR_REG],
            events);

        if (instId == VPAC_MSC_INST_ID_0)
        {
            /* MSC0 Wdtimer Error Events are by default enabled */
            wdtimer_events = VHWA_MSC0_WDTIMER_ERR;
            /* Enable HTS Interrupt in INTD Module */
            CSL_REG32_WR(&intdRegs->ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_WDTIMER_INTR_REG],
                wdtimer_events);
        }
        else
        {
            /* MSC1 Wdtimer Error Events are by default enabled */
            wdtimer_events = VHWA_MSC1_WDTIMER_ERR;
            /* Enable HTS Interrupt in INTD Module */
            CSL_REG32_WR(&intdRegs->ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_WDTIMER_INTR_REG],
                wdtimer_events);
        }
    }
    return (status);
}

void Vhwa_m2mMscUnregisterIsr(Vhwa_M2mMscInstObj *instObj,
                              const  Vhwa_M2mMscCommonObj *comObj,
                               uint32_t instId)
{
    uint32_t                         events;
    uint32_t                         vhwaIrqNum;
    CSL_vpac_intd_cfgRegs           *intdRegs = NULL;
    uint32_t                         wdtimer_events;
     /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != instObj)
    /* LDRA_JUSTIFY_END */

    {
        vhwaIrqNum  = comObj->mscInitPrms.irqInfo[instId].vhwaIrqNum;
        intdRegs    = comObj->socInfo.vpacIntdRegs;

        /* If not already done, disable HTS Interrupt */
        Vhwa_disableHtsIntr(intdRegs, vhwaIrqNum, instObj->pipeline);

        /* All Error Events are by default enabled */
        events =
            VHWA_MSC_VBUSM_RD_ERR | VHWA_MSC_SL2_WR_ERR;;

        /* Disable HTS Interrupt in INTD Module */
        CSL_REG32_WR(&intdRegs->ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_MSC_INTR_REG],
            events);

        if (instId == VPAC_MSC_INST_ID_0)
        {
            /* MSC0 Wdtimer Error Events are by default enabled */
            wdtimer_events = VHWA_MSC0_WDTIMER_ERR;
            /* Enable HTS Interrupt in INTD Module */
            CSL_REG32_WR(&intdRegs->ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_WDTIMER_INTR_REG],
                wdtimer_events);
        }
        else
        {
            /* MSC1 Wdtimer Error Events are by default enabled */
            wdtimer_events = VHWA_MSC1_WDTIMER_ERR;
            /* Enable HTS Interrupt in INTD Module */
            CSL_REG32_WR(&intdRegs->ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_WDTIMER_INTR_REG],
                wdtimer_events);
        }


#if defined(MCU_PLUS_SDK)
        (void)HwiP_destruct(&instObj->intrHandle);
#else
         /* LDRA_JUSTIFY_START
        <metric start>  branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != instObj->intrHandle)
        /* LDRA_JUSTIFY_END */
        {
            /* Un-register interrupt */
            (void)HwiP_delete(instObj->intrHandle);
            instObj->intrHandle = NULL;
        }
#endif
    }
}

static void vhwa_m2mMscIsr0(uintptr_t args)
{
    uint32_t            errStat         = 0U;
    uint32_t            frmDoneStat     = 0U;
    uint32_t            wdTimer_errStat = 0U;
    uint32_t            instId;
    Vhwa_M2mMscInstObj *instObj = NULL;
    Vhwa_M2mMscCommonObj *comObj;
    Vhwa_M2mMscHandleObj            *hObj = NULL;

    GT_assert(VhwaMscTrace, (NULL != (Vhwa_M2mMscInstObj *)args));

    instObj = (Vhwa_M2mMscInstObj *)args;
    comObj = &gM2mMscCommonObj;
    instId = VPAC_MSC_INST_ID_0;
 /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if((NULL != instObj->activeQObj) && (NULL != instObj->activeQObj->hObj))
    /* LDRA_JUSTIFY_END */
    {
        hObj   = instObj->activeQObj->hObj;
    }

    instObj->totalIntrCnt ++;
 /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != hObj)
    /* LDRA_JUSTIFY_END */

    {
        vhwa_m2mMscGetIntrStat(instObj, comObj, instId, &errStat, &wdTimer_errStat, &frmDoneStat, hObj->fcStatus.isFlexConnect);
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component-level negative test framework and test applications cannot reach this portion. This branch is covered in the VHWA_NEGATIVE_TEST build.
    Effect on this unit: If control reaches here, the code base is expected to call the imaging callback function to return an error status to the application.
    <justification end> */
    if (0U != errStat)
    {
        /* First check if any of the registered error events has come,
           then first call the callback for the error events */
        vhwa_m2mMscErrEventIsr(instObj, errStat);
    }

    if (0U != wdTimer_errStat)
    {
        /* Set stalled flag to prevent further process requests */
        instObj->isStalled = (uint32_t)UTRUE;

        if(NULL != hObj)
        {
            /* Disable watchdog timer after its first trigger*/
            hObj->htsCfg[0].enableWdTimer = UFALSE;

            CSL_htsDisableWatchdogTimer(comObj->socInfo.htsRegs, &hObj->htsCfg[0]);

        }
        /* First check if any of the registered watchdog timer error events has come,
           then first call the callback for the error events */
        vhwa_m2mMscWdTimerErrEventIsr(instObj, wdTimer_errStat);
    }

    if (0U != frmDoneStat)
    {
        /* Now process the Frame Done ISR */
        vhwa_m2mMscFrmDoneIsr(instObj, comObj);
    }

    if((0u == errStat) && (0U == wdTimer_errStat) && (0U == frmDoneStat))
    {
        instObj->errIntrInvalidStatus ++;
    }
    /* LDRA_JUSTIFY_END */

}

static void vhwa_m2mMscIsr1(uintptr_t args)
{
    uint32_t            errStat         = 0U;
    uint32_t            frmDoneStat     = 0U;
    uint32_t            wdTimer_errStat = 0U;
    uint32_t            instId;
    Vhwa_M2mMscInstObj  *instObj = NULL;
    Vhwa_M2mMscCommonObj *comObj;
    Vhwa_M2mMscHandleObj            *hObj = NULL;

    GT_assert(VhwaMscTrace, (NULL != (Vhwa_M2mMscInstObj *)args));

    instObj = (Vhwa_M2mMscInstObj *)args;
    comObj = &gM2mMscCommonObj;
    instId = VPAC_MSC_INST_ID_1;
 /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if((NULL != instObj->activeQObj) && (NULL != instObj->activeQObj->hObj))
        /* LDRA_JUSTIFY_END */

    {
        hObj   = instObj->activeQObj->hObj;
    }

    instObj->totalIntrCnt ++;
    /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != hObj)
        /* LDRA_JUSTIFY_END */

    {
        vhwa_m2mMscGetIntrStat(instObj, comObj, instId, &errStat, &wdTimer_errStat, &frmDoneStat, hObj->fcStatus.isFlexConnect);
    }

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component-level negative test framework and test applications cannot reach this portion. This branch is covered in the VHWA_NEGATIVE_TEST build.
    Effect on this unit: If control reaches here, the code base is expected to call the imaging callback function to return an error status to the application.
    <justification end> */
    if (0U != errStat)
    {
        /* First check if any of the registered error events has come,
           then first call the callback for the error events */
        vhwa_m2mMscErrEventIsr(instObj, errStat);
    }

    if (0U != wdTimer_errStat)
    {
        /* Set stalled flag to prevent further process requests */
        instObj->isStalled = (uint32_t)UTRUE;

        if(NULL != hObj)
        {
            /* Disable watchdog timer after its first trigger*/
            hObj->htsCfg[0].enableWdTimer = UFALSE;

            CSL_htsDisableWatchdogTimer(comObj->socInfo.htsRegs, &hObj->htsCfg[0]);
        }
        /* First check if any of the registered watchdog timer error events has come,
           then first call the callback for the error events */
        vhwa_m2mMscWdTimerErrEventIsr(instObj, wdTimer_errStat);
    }

    if (0U != frmDoneStat)
    {
        /* Now process the Frame Done ISR */
        vhwa_m2mMscFrmDoneIsr(instObj, comObj);
    }
    if((0u == errStat) && (0U == wdTimer_errStat) && (0U == frmDoneStat))
    {
        instObj->errIntrInvalidStatus ++;
    }
    /* LDRA_JUSTIFY_END */

}


/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: The component-level negative test framework and test applications cannot reach this portion. This branch is covered in the VHWA_NEGATIVE_TEST build.
    Effect on this unit: If control reaches here, the code base is expected to call the imaging callback function to return an error status to the application.
<justification end> */
static void vhwa_m2mMscErrEventIsr(Vhwa_M2mMscInstObj *instObj,
                                    uint32_t errStat)
{
    uint32_t                        errStatus;
    Msc_ErrEventParams              *eePrms = NULL;
    Vhwa_M2mMscHandleObj            *hObj = NULL;

    if (NULL != instObj)
    {
        if ((NULL == instObj->activeQObj) || (NULL == instObj->activeQObj->hObj))
        {
            instObj->errNoActQObj ++;
            if (NULL != instObj->lastHndlObj)
            {
                hObj = instObj->lastHndlObj;
                eePrms = &hObj->eePrms;
            }
            else
            {
                hObj = NULL;
                instObj->errNoLastHndlObj ++;
            }
        }
        else
        {
            hObj   = instObj->activeQObj->hObj;
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
Rationale: The component-level negative test framework and test applications cannot reach this portion. This branch is covered in the VHWA_NEGATIVE_TEST build.
    Effect on this unit: If control reaches here, the code base is expected to call the imaging callback function to return an error status to the application.
<justification end> */
static void vhwa_m2mMscWdTimerErrEventIsr(Vhwa_M2mMscInstObj *instObj,
                                    uint32_t wdTimer_errStat)
{
    uint32_t                        errStatus;
    Msc_WdTimerErrEventParams       *wdTimerEePrms = NULL;
    Vhwa_M2mMscHandleObj            *hObj = NULL;

    if (NULL != instObj)
    {
        if ((NULL == instObj->activeQObj) || (NULL == instObj->activeQObj->hObj))
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
            hObj   = instObj->activeQObj->hObj;
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

static void vhwa_m2mMscFrmDoneIsr(Vhwa_M2mMscInstObj *instObj,
                                    Vhwa_M2mMscCommonObj *comObj)
{
    int32_t               retVal = FVID2_SOK;
    uint32_t              cnt;
    Vhwa_M2mMscHandleObj *hObj = NULL;
    Vhwa_M2mMscQueueObj  *qObj = NULL;
    VhwaVpacMscSocReadBack *goldenRegVal = NULL;

 /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated in the imaging layer before reaching vhwa.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
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
        GT_assert(VhwaMscTrace, (NULL != instObj->activeQObj));

        qObj = instObj->activeQObj;
         /* LDRA_JUSTIFY_START
        <metric start>  branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != qObj)
        /* LDRA_JUSTIFY_END */

        {
            /* Handle Object in QObject must not be null */
            GT_assert(VhwaMscTrace, (NULL != qObj->hObj));

            hObj = qObj->hObj;
             /* LDRA_JUSTIFY_START
            <metric start>  branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if(NULL != hObj)
            /* LDRA_JUSTIFY_END */

            {
                goldenRegVal = hObj->configRegMemPrms.configGoldenRegPtr;
                 /* LDRA_JUSTIFY_START
                <metric start>  branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                <justification end> */
                if (0u == CSL_htsPipelineStatus(comObj->socInfo.htsRegs,
                        &hObj->htsCfg[hObj->curIterCnt]))
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
                    if (NULL != hObj->createPrms.getTimeStamp)
                    {
                        hObj->perf[hObj->curIterCnt] =
                            hObj->createPrms.getTimeStamp() -
                            hObj->perf[hObj->curIterCnt];
                    }
                    /* LDRA_JUSTIFY_END */

                    if ((uint32_t)UTRUE == hObj->lseCfg[hObj->curIterCnt].enablePsa)
                    {
                        cnt = hObj->lseCfg[hObj->curIterCnt].numOutCh;
                        hObj->lseCfg[hObj->curIterCnt].numOutCh = MSC_MAX_OUTPUT;

                        CSL_lseGetPsaSign(comObj->socInfo.lseRegs,
                        &hObj->lseCfg[hObj->curIterCnt],
                        hObj->psa.psaSign[hObj->curIterCnt]);

                        hObj->lseCfg[hObj->curIterCnt].numOutCh = cnt;

                    }

                    /**
                     * Steps
                     * 1, Clear the interrupt
                     * 2, Move the queue object to doneQ
                     * 3, Mark Active Queue Object to NULL and lastHandle to
                     *    completed handle
                     * 4, If the reqQ is not empty, Remove one object from reqQ
                     * 5, if the last handle is same as this handle, submit the TR
                     * 6, If they are not same, set the config and then
                     *    submit the TR
                     * 7, move the new object to actQ
                     * 8, Call the application callback
                     */

                    hObj->curIterCnt++;
                    if (hObj->curIterCnt < hObj->numIter)
                    {
                        /* TODO Add seperate submit function for chroma */
                        /* Process Chroma */
                        retVal = Vhwa_m2mMscSetFrameSize(qObj, comObj,
                                                        hObj->curIterCnt);
                        /* LDRA_JUSTIFY_START
                        <metric start>branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        The test framework does not support the configuration required to trigger this error scenario.
                        Effect on this unit: If the control reaches here, our code base is expected to print the error status.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if (FVID2_SOK == retVal)
                        /* LDRA_JUSTIFY_END */

                        {
                            retVal = Vhwa_m2mMscSubmitRequest(instObj, qObj, comObj,
                                                            hObj->curIterCnt);
                        }
                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        The test framework does not support the configuration required to trigger this error scenario.
                        Effect on this unit: If the control reaches here, our code base is expected to print the error status.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if (FVID2_SOK != retVal)
                        {
                            GT_0trace(VhwaMscTrace, GT_ERR,
                                "ISR0 Submiit request failed\n");
                        }
                        /* LDRA_JUSTIFY_END */

                    }
                    else
                    {
                        hObj->curIterCnt = 0;

                        /* Move completed qObject to done queue */
                        Fvid2Utils_queue(hObj->outQ, &qObj->qElem, qObj);

                        instObj->lastHndlObj = hObj;
                        instObj->activeQObj = NULL;

                        /* Free the output scalar */
                        for (cnt = 0U; cnt < MSC_MAX_OUTPUT; cnt ++)
                        {
                            if (0u != (hObj->scalarUsed & ((uint32_t)1u << cnt)))
                            {
                                (void)VhwaAl_SemaphoreP_post(comObj->scLockSem[cnt]);
                            }
                        }

                        /* Call Application callback, for the completed handle object */
                        /* LDRA_JUSTIFY_START
                        <metric start>  branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        The test framework does not support the configuration required to trigger this error scenario.
                        Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if (NULL != hObj->fdmCbPrms.fdmCbFxn)
                        /* LDRA_JUSTIFY_END */
                        {
                            (void)hObj->fdmCbPrms.fdmCbFxn(hObj->fdmCbPrms.fdmData);
                        }

                        CSL_lseStopChannels(comObj->socInfo.lseRegs, &hObj->lseCfg[0]);

                        /* For sequential processing, the LSE config is updated in golden config registers. */
                        if ((uint32_t)VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate)
                        {
                            /* LDRA_JUSTIFY_START
                            <metric start> branch <metric end>
                            <justification start>
                            Rationale: The component level negative test framework and test applications cannot reach this portion.
                            The test framework does not support the configuration required to trigger this error scenario.
                            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                            This behaviour is part of the application design. An error print statement can be added in a future release if required.
                            <justification end> */
                            if (NULL != goldenRegVal)
                            /* LDRA_JUSTIFY_END */
                            {
                                CSL_lseStopChannels(&goldenRegVal->lseRegs, &hObj->lseCfg[0]);
                            }
                        }

                        CSL_htsThreadStop(comObj->socInfo.htsRegs, &hObj->htsCfg[0]);

                        if ((uint32_t)VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate)
                        {
                            /* LDRA_JUSTIFY_START
                            <metric start> branch <metric end>
                            <justification start>
                            Rationale: The component level negative test framework and test applications cannot reach this portion.
                            The test framework does not support the configuration required to trigger this error scenario.
                            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                            This behaviour is part of the application design. An error print statement can be added in a future release if required.
                            <justification end> */
                            if (NULL != goldenRegVal)
                            /* LDRA_JUSTIFY_END */
                            {
                                vhwa_m2mMscHtsThreadStop(&goldenRegVal->mscHtsRegs, &hObj->htsCfg[0]);
                            }
                        }
                    }
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    instObj->errInvalidHtsPipelineStatus ++;
                }
                /* LDRA_JUSTIFY_END */

            }
        }
        /* LDRA_JUSTIFY_START
        <metric start>  statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            instObj->errIntrRepeateIntr ++;
        }
        /* LDRA_JUSTIFY_END */

    }
}

static void vhwa_m2mMscGetIntrStat(const Vhwa_M2mMscInstObj *instObj,
                                   const Vhwa_M2mMscCommonObj *comObj,
                                    uint32_t instId,
                                    uint32_t *errStat,
                                    uint32_t *wdTimer_errStat,
                                    uint32_t *frmDoneStat,
                                    uint32_t   isFlexConnect)
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
    <metric start>  branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated in the imaging layer before reaching vhwa.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != instObj)
    /* LDRA_JUSTIFY_END */

    {
        vhwaIrqNum  = comObj->mscInitPrms.irqInfo[instId].vhwaIrqNum;
        intdRegs    = comObj->socInfo.vpacIntdRegs;

        vhwa_GetErrIntrStat(&intdRegs->STATUS_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_MSC_INTR_REG],
            &intdRegs->STATUS_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_MSC_INTR_REG], errStat, VHWA_VPAC_INTD_MSC_ERR_MASK);

        if(VPAC_MSC_INST_ID_0 == instId)
        {
            vhwa_GetWdtimerErrIntrStat(&intdRegs->STATUS_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_WDTIMER_INTR_REG],
                &intdRegs->STATUS_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_WDTIMER_INTR_REG], wdTimer_errStat, VHWA_VPAC_INTD_MSC0_WDTIMER_ERR_BIT);
        }
        else
        {
            vhwa_GetWdtimerErrIntrStat(&intdRegs->STATUS_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_WDTIMER_INTR_REG],
                &intdRegs->STATUS_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_WDTIMER_INTR_REG], wdTimer_errStat, VHWA_VPAC_INTD_MSC1_WDTIMER_ERR_BIT);
        }

        regVal = CSL_REG32_RD(
            &intdRegs->STATUS_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_PIPE_INTR_REG]);
        *frmDoneStat = 0U;
        /* LDRA_JUSTIFY_START
        <metric start>  branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. An error print statement can be added in a future release if required.
        <justification end> */
        if (0U != (regVal & ((uint32_t)1u << htsPipeline)))
        /* LDRA_JUSTIFY_END */

        {
            /* Clear the interrupt */
            regVal = (uint32_t)1u << htsPipeline;
            CSL_REG32_WR(
                &intdRegs->STATUS_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_PIPE_INTR_REG],
                regVal);

            *frmDoneStat = 1U;
        }
    }
}

static void vhwa_m2mMscHtsThreadStop(const CSL_MscHtsConfigReg *mscHtsRegs, const CSL_HtsSchConfig *cfg)
{
    uint32_t cnt;
    const CSL_HtsHwaParams *htsPrms = NULL;

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start>
Rationale: The negative test framework and test apps cannot reach the false case of this condition.
           'mscHtsRegs and cfg' are passed during safety configuration validation.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
    if ((NULL != mscHtsRegs) && (NULL != cfg))
/* LDRA_JUSTIFY_END */
    {
        htsPrms = &gCslHtsParams[cfg->schId];

        /* Stop scheduler */
        CSL_REG32_FINS(&mscHtsRegs->SCHEDULER_CONTROL[0],
            HTS_HWA0_SCHEDULER_CONTROL_SCH_EN, 0U);

        /* Stop consumers */
        for (cnt = 0U; cnt < htsPrms->maxConsumer; cnt ++)
        {
            if ((uint32_t)UFALSE == cfg->dmaProdCfg[cnt].bypass)
            {
                if ((uint32_t)UTRUE == cfg->consCfg[cnt].enable)
                {
                    CSL_REG32_FINS(&mscHtsRegs->CONS_CONTROL[cnt],
                        HTS_HWA0_CONS0_CONTROL_CONS_EN, 0U);
                }

                if ((uint32_t)UTRUE == cfg->dmaProdCfg[cnt].enable)
                {
                    CSL_REG32_FINS(&mscHtsRegs->PROD_CONTROL_DMA[cnt],
                        HTS_DMA0_PROD0_CONTROL_PROD_EN, 0U);
                    CSL_REG32_FINS(&mscHtsRegs->PROD_DMA_SCHEDULER_CONTROL[cnt],
                        HTS_DMA0_SCHEDULER_CONTROL_SCH_EN, 0U);

#if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
                    if ((uint32_t)UTRUE == htsPrms->isDmaProdPaSupported[cnt])
                    {
                        if (UTRUE == cfg->dmaProdCfg[cnt].paCfg.enable)
                        {
                            CSL_REG32_FINS(&mscHtsRegs->PA_CONTROL_DMA[cnt],
                                HTS_DMA8_PA0_CONTROL_PA_ENABLE, 0U);
                        }
                    }
#endif
                }
            }
        }

        /* Stop producers */
        for (cnt = 0U; cnt < htsPrms->maxProducer; cnt ++)
        {
            if ((uint32_t)UFALSE == cfg->dmaConsCfg[cnt].bypass)
            {
                if ((uint32_t)UTRUE == cfg->prodCfg[cnt].enable)
                {
                    CSL_REG32_FINS(&mscHtsRegs->PROD_CONTROL[cnt],
                        HTS_HWA0_PROD0_CONTROL_PROD_EN, 0U);

#if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
                    if ((uint32_t)UTRUE == htsPrms->isPaSupported[cnt])
                    {
                        if (UTRUE == cfg->prodCfg[cnt].paCfg.enable)
                        {
                            CSL_REG32_FINS(&mscHtsRegs->PA_CONTROL[cnt],
                                HTS_HWA0_PA0_CONTROL_PA_ENABLE, 0U);
                        }
                    }
#endif
                }

                if ((uint32_t)UTRUE == cfg->dmaConsCfg[cnt].enable)
                {
                    CSL_REG32_FINS(&mscHtsRegs->CONS_CONTROL_DMA[cnt],
                        HTS_DMA240_CONS0_CONTROL_CONS_EN, 0U);
                    CSL_REG32_FINS(&mscHtsRegs->CONS_DMA_SCHEDULER_CONTROL[cnt],
                        HTS_DMA240_SCHEDULER_CONTROL_SCH_EN, 0U);
                }
            }
        }
    }
}
