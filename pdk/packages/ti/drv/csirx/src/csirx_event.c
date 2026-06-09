/*
 *  Copyright (c) Texas Instruments Incorporated 2022
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
 *  \file csirx_event.c
 *
 *  \brief File containing the CSIRX driver event management functions.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <ti/drv/csirx/src/csirx_drvPriv.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
extern CsirxDrv_CommonObj gCsirxCommonObj;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */
static void CsirxDrv_errorEventIsrFxn(uintptr_t arg);
static void CsirxDrv_cpIntdEventIsrFxn(uintptr_t arg);
static void CsirxDrv_infoEventIsrFxn(uintptr_t arg);
static void CsirxDrv_asfEsmHighEventIsrFxn(uintptr_t arg);
static void CsirxDrv_asfEsmLowEventIsrFxn(uintptr_t arg);
static void CsirxDrv_asfEsmCfgEventIsrFxn(uintptr_t arg);
static int32_t CsirxDrv_eventValidateParams(const Csirx_EventPrms *eventPrms,
                                            CsirxDrv_InstObj *instObj);
static int32_t CsirxDrv_eventGroupAllocResource(CsirxDrv_InstObj *instObj,
                                                Csirx_EventPrms *eventPrms);
static int32_t CsirxDrv_resetStream(const CsirxDrv_InstObj *instObj,
                                    uint32_t strmIdx);
/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */
void Csirx_eventPrmsInit(Csirx_EventPrms *eventPrms)
{
    if (NULL != eventPrms)
    {
        eventPrms->eventGroup           = CSIRX_EVENT_GROUP_ERROR;
        eventPrms->eventMasks           = CSIRX_EVENT_TYPE_ERR_ALL;
        eventPrms->eventCb              = NULL;
        eventPrms->coreIntrNum          = CSIRX_INTR_INVALID;
        eventPrms->intrPriority         = 0U;
        eventPrms->appData              = NULL;
    }
}

int32_t CsirxDrv_eventGroupRegister(Fdrv_Handle drvHandle,
                                    Csirx_EventPrms *eventPrms)
{
    int32_t retVal = FVID2_SOK;
    CsirxDrv_InstObj *instObj = NULL;
    CsirxDrv_EventObj *eventObj;
    OsalRegisterIntrParams_t intrPrms;
    CsirxDrv_VirtContext *virtContext;

    /* Error Check: Parameters */
    GT_assert(CsirxTrace, (NULL_PTR != drvHandle));
    GT_assert(CsirxTrace, (NULL_PTR != eventPrms));

    virtContext = (CsirxDrv_VirtContext *) drvHandle;
    instObj = virtContext->instObj;
    if (CSIRX_DRV_USAGE_STATUS_IN_USE != instObj->inUse)
    {
        retVal = FVID2_EFAIL;
    }
    
    if (NULL != instObj->lockSem)
    {
        /* Pend on the instance semaphore */
        (void)SemaphoreP_pend(instObj->lockSem, SemaphoreP_WAIT_FOREVER);
    }

    if (FVID2_SOK == retVal)
    {
        /* Configure more events if not already configured during previous
           open instances, not needed currently as only Error types of
           events are supported */
        /* In case of configured events, check new params with
           configured one*/
        retVal = CsirxDrv_eventValidateParams(eventPrms, instObj);
    }

    if (FVID2_SOK == retVal)
    {
        eventObj = &instObj->eventObj[eventPrms->eventGroup];
        /* Only allow event registration on first call */
        /* Configure events only if not configured in previous open calls */
        if (UFALSE == eventObj->eventInitDone)
        {
            /* Copy event configuration parameters to driver object */
            switch (eventPrms->eventGroup)
            {
                case CSIRX_EVENT_GROUP_ERROR:
                    Fvid2Utils_memcpy(&eventObj->eventPrms,
                                      eventPrms,
                                      sizeof(Csirx_EventPrms));
                break;
                case CSIRX_EVENT_GROUP_CP_INTD:
                    Fvid2Utils_memcpy(&eventObj->eventPrms,
                                      eventPrms,
                                      sizeof(Csirx_EventPrms));
                break;
                case CSIRX_EVENT_GROUP_INFO:
                    Fvid2Utils_memcpy(&eventObj->eventPrms,
                                      eventPrms,
                                      sizeof(Csirx_EventPrms));
                break;
                case CSIRX_ESM_HI_EVENT_GROUP_ASF:
                    Fvid2Utils_memcpy(&eventObj->eventPrms,
                                      eventPrms,
                                      sizeof(Csirx_EventPrms));
                break;
                case CSIRX_ESM_LOW_EVENT_GROUP_ASF:
                    Fvid2Utils_memcpy(&eventObj->eventPrms,
                                      eventPrms,
                                      sizeof(Csirx_EventPrms));
                break;
                case CSIRX_ESM_CFG_EVENT_GROUP_ASF:
                    Fvid2Utils_memcpy(&eventObj->eventPrms,
                                      eventPrms,
                                      sizeof(Csirx_EventPrms));
                break;
                default:
                break;
            }
            /* Copy driver handle to event object */
            eventObj->drvHandle = drvHandle;
            /* Allocate resource to the event group */
            retVal = CsirxDrv_eventGroupAllocResource(instObj, eventPrms);
            if (FVID2_SOK == retVal)
            {
                /* Register interrupt */
                Osal_RegisterInterrupt_initParams(&intrPrms);
                /* Populate the interrupt parameters */
                intrPrms.corepacConfig.arg              =
                                            (uintptr_t) eventObj;
                if(CSIRX_EVENT_GROUP_ERROR == eventPrms->eventGroup)
                {
                    intrPrms.corepacConfig.isrRoutine       =
                                                CsirxDrv_errorEventIsrFxn;
                }
                else if (CSIRX_EVENT_GROUP_CP_INTD == eventPrms->eventGroup)
                {
                    intrPrms.corepacConfig.isrRoutine       =
                                                CsirxDrv_cpIntdEventIsrFxn;
                }
                else if (CSIRX_EVENT_GROUP_INFO == eventPrms->eventGroup)
                {
                    intrPrms.corepacConfig.isrRoutine       =
                                                CsirxDrv_infoEventIsrFxn;
                }
                else if (CSIRX_ESM_HI_EVENT_GROUP_ASF == eventPrms->eventGroup)
                {
                    intrPrms.corepacConfig.isrRoutine       =
                                                CsirxDrv_asfEsmHighEventIsrFxn;                    
                }
                else if (CSIRX_ESM_LOW_EVENT_GROUP_ASF == eventPrms->eventGroup)
                {
                    intrPrms.corepacConfig.isrRoutine       =
                                                CsirxDrv_asfEsmLowEventIsrFxn;                    
                }
                else if (CSIRX_ESM_CFG_EVENT_GROUP_ASF == eventPrms->eventGroup)
                {
                    intrPrms.corepacConfig.isrRoutine       =
                                                CsirxDrv_asfEsmCfgEventIsrFxn;                    
                }
                else
                {
                    //Do nothing
                }
                intrPrms.corepacConfig.priority         =
                                            eventPrms->intrPriority;
                intrPrms.corepacConfig.corepacEventNum  = 0U;
                intrPrms.corepacConfig.intVecNum        =
                                            eventObj->coreIntrNum;
                if (Osal_RegisterInterrupt(&intrPrms, &eventObj->hwiHandle) !=
                                            OSAL_INT_SUCCESS)
                {
                    eventObj->hwiHandle = NULL;
                    retVal = FVID2_EFAIL;
                }

            }
            if (FVID2_SOK == retVal)
            {
                /* Enable respective events */
                retVal = CsirxDrv_eventEnable(drvHandle,
                                              eventPrms->eventGroup,
                                              eventPrms->eventMasks);
            }
            if (FVID2_SOK == retVal)
            {
                eventObj->eventInitDone = 1U;
            }
        }
    }
    if (NULL != instObj->lockSem)
    {
        /* Post the instance semaphore */
        (void)SemaphoreP_post(instObj->lockSem);
    }

    return retVal;
}

static int32_t CsirxDrv_eventValidateParams(const Csirx_EventPrms *eventPrms,
                                            CsirxDrv_InstObj *instObj)
{
    int32_t retVal = FVID2_SOK;
    CsirxDrv_EventObj *eventObj;

    if ((0U == eventPrms->eventMasks) &&
        (CSIRX_EVENT_GROUP_MAX <= eventPrms->eventGroup))
    {
        retVal = FVID2_EBADARGS;
    }
    if (FVID2_SOK == retVal)
    {
        eventObj = &instObj->eventObj[eventPrms->eventGroup];
        if (UTRUE == eventObj->eventInitDone)
        {
            /* check against configured parameters */
            if ((eventObj->eventPrms.eventGroup == eventPrms->eventGroup) &&
                (eventObj->eventPrms.eventMasks == eventPrms->eventMasks) &&
                (eventObj->eventPrms.eventCb == eventPrms->eventCb) &&
                (eventObj->eventPrms.intrPriority == eventPrms->intrPriority))
            {
                /* Configuration matches, do nothing */
            }
            else
            {
                retVal = FVID2_WNON_RECOMMENDED_PARAMS;
            }
        }
    }
    else
    {
        /* Do nothing */
    }

    return retVal;
}

static int32_t CsirxDrv_eventGroupAllocResource(CsirxDrv_InstObj *instObj,
                                                Csirx_EventPrms *eventPrms)
{
    int32_t retVal = FVID2_SOK;

    eventPrms->coreIntrNum = CSIRX_INTR_INVALID;
    switch (instObj->drvInstId)
    {
        case CSIRX_INSTANCE_ID_0:
            switch (eventPrms->eventGroup)
            {
                case CSIRX_EVENT_GROUP_ERROR:
                    eventPrms->coreIntrNum = CSIRX_CORE_INTR_NUM_MOD_0_ERR_INTR;
                break;
                case CSIRX_EVENT_GROUP_CP_INTD:
                    eventPrms->coreIntrNum = CSIRX_CORE_INTR_NUM_MOD_0_LEVEL_INTR;
                break;
                case CSIRX_EVENT_GROUP_INFO:
                    eventPrms->coreIntrNum = CSIRX_CORE_INTR_NUM_MOD_0_INFO_INTR;
                break;
                case CSIRX_ESM_HI_EVENT_GROUP_ASF:
                    eventPrms->coreIntrNum = CSIRX_CORE_INTR_MAIN_ESM_HIGH_LVL_INTR;
                break;
                case CSIRX_ESM_LOW_EVENT_GROUP_ASF:
                    eventPrms->coreIntrNum = CSIRX_CORE_INTR_MAIN_ESM_LOW_LVL_INTR;
                break;
                case CSIRX_ESM_CFG_EVENT_GROUP_ASF:
                    eventPrms->coreIntrNum = CSIRX_CORE_INTR_MAIN_ESM_CONFIG_LVL_INTR;
                break;
                default:
                    retVal = FVID2_EBADARGS;
                break;
            }
        break;
        case CSIRX_INSTANCE_ID_1:
            switch (eventPrms->eventGroup)
            {
                case CSIRX_EVENT_GROUP_ERROR:
                    eventPrms->coreIntrNum = CSIRX_CORE_INTR_NUM_MOD_1_ERR_INTR;
                break;
                case CSIRX_EVENT_GROUP_CP_INTD:
                    eventPrms->coreIntrNum = CSIRX_CORE_INTR_NUM_MOD_1_LEVEL_INTR;
                break;
                case CSIRX_EVENT_GROUP_INFO:
                    eventPrms->coreIntrNum = CSIRX_CORE_INTR_NUM_MOD_1_INFO_INTR;
                break;
                case CSIRX_ESM_HI_EVENT_GROUP_ASF:
                    eventPrms->coreIntrNum = CSIRX_CORE_INTR_MAIN_ESM_HIGH_LVL_INTR;
                break;
                case CSIRX_ESM_LOW_EVENT_GROUP_ASF:
                    eventPrms->coreIntrNum = CSIRX_CORE_INTR_MAIN_ESM_LOW_LVL_INTR;
                break;
                case CSIRX_ESM_CFG_EVENT_GROUP_ASF:
                    eventPrms->coreIntrNum = CSIRX_CORE_INTR_MAIN_ESM_CONFIG_LVL_INTR;
                break;
                default:
                    retVal = FVID2_EBADARGS;
                break;
            }
        break;
#if defined (SOC_J784S4) || defined (SOC_J742S2)
        case CSIRX_INSTANCE_ID_2:
            switch (eventPrms->eventGroup)
            {
                case CSIRX_EVENT_GROUP_ERROR:
                    eventPrms->coreIntrNum = CSIRX_CORE_INTR_NUM_MOD_2_ERR_INTR;
                break;
                case CSIRX_EVENT_GROUP_CP_INTD:
                    eventPrms->coreIntrNum = CSIRX_CORE_INTR_NUM_MOD_2_LEVEL_INTR;
                break;
                case CSIRX_EVENT_GROUP_INFO:
                    eventPrms->coreIntrNum = CSIRX_CORE_INTR_NUM_MOD_2_INFO_INTR;
                break;
                case CSIRX_ESM_HI_EVENT_GROUP_ASF:
                    eventPrms->coreIntrNum = CSIRX_CORE_INTR_MAIN_ESM_HIGH_LVL_INTR;
                break;
                case CSIRX_ESM_LOW_EVENT_GROUP_ASF:
                    eventPrms->coreIntrNum = CSIRX_CORE_INTR_MAIN_ESM_LOW_LVL_INTR;
                break;
                case CSIRX_ESM_CFG_EVENT_GROUP_ASF:
                    eventPrms->coreIntrNum = CSIRX_CORE_INTR_MAIN_ESM_CONFIG_LVL_INTR;
                break;
                default:
                    retVal = FVID2_EBADARGS;
                break;
            }
        break;
#endif
        default:
            retVal = FVID2_EBADARGS;
        break;
    }
    instObj->eventObj[eventPrms->eventGroup].coreIntrNum =
                                                    eventPrms->coreIntrNum;

    return retVal;
}

int32_t CsirxDrv_eventEnable(Fdrv_Handle drvHandle,
                             uint32_t eventGroup,
                             uint32_t eventType)
{
    int32_t retVal = FVID2_SOK;
    CsirxDrv_InstObj *instObj;
    CSIRX_ErrorIrqsMaskCfg errMask;
    CSIRX_InfoIrqsMaskCfg infoMask;
    CSIRX_AsfIrqMaskCfg asfMask;
    CSIRX_AsfTransToFaultMask asfTransToFaultMsk;
    CSIRX_CpIntd_cfg config;
    CsirxDrv_VirtContext *virtContext;

    virtContext = (CsirxDrv_VirtContext *) drvHandle;
    instObj = virtContext->instObj;
    switch (eventGroup)
    {
        case CSIRX_EVENT_GROUP_ERROR:
            if (CDN_EOK != CSIRX_GetErrorIrqsMaskCfg(&instObj->cslObj.cslCfgData,
                                          &errMask))
            {
                retVal = FVID2_EFAIL;
            }
            if (FVID2_SOK == retVal)
            {
                errMask.frontFifoOverflowIrqm  |= (uint8_t)
                        ((eventType & CSIRX_EVENT_TYPE_ERR_FIFO_OVERFLOW_FRONT) >>
                         CSL_CSIRX_ERROR_IRQS_FRONT_FIFO_OVERFLOW_IRQ_SHIFT);
                errMask.payloadCrcIrqm         |= (uint8_t)
                        ((eventType & CSIRX_EVENT_TYPE_ERR_PAYLOAD_CRC) >>
                         CSL_CSIRX_ERROR_IRQS_PAYLOAD_CRC_IRQ_SHIFT);
                errMask.headerEccIrqm          |= (uint8_t)
                        ((eventType & CSIRX_EVENT_TYPE_ERR_HEADER_ECC) >>
                         CSL_CSIRX_ERROR_IRQS_HEADER_ECC_IRQ_SHIFT);
                errMask.headerCorrectedEccIrqm |= (uint8_t)
                        ((eventType & CSIRX_EVENT_TYPE_ERR_HEADER_CORRECTED_ECC) >>
                         CSL_CSIRX_ERROR_IRQS_HEADER_CORRECTED_ECC_IRQ_SHIFT);
                errMask.dataIdIrqm             |= (uint8_t)
                        ((eventType & CSIRX_EVENT_TYPE_ERR_DATA_ID) >>
                         CSL_CSIRX_ERROR_IRQS_DATA_ID_IRQ_SHIFT);
                errMask.invalidAccessIrqm      |= (uint8_t)
                        ((eventType & CSIRX_EVENT_TYPE_ERR_INVALID_ACCESS) >>
                         CSL_CSIRX_ERROR_IRQS_INVALID_ACCESS_IRQ_SHIFT);
                errMask.spInvalidRcvdIrqm      |= (uint8_t)
                        ((eventType & CSIRX_EVENT_TYPE_ERR_INVALID_SHORT_PACKET) >>
                         CSL_CSIRX_ERROR_IRQS_SP_INVALID_RCVD_IRQ_SHIFT);
                errMask.fifoOverflowIrqm[0U]   |= (uint8_t)
                        ((eventType & CSIRX_EVENT_TYPE_ERR_FIFO_OVERFLOW_STRM0) >>
                         CSL_CSIRX_ERROR_IRQS_STREAM0_FIFO_OVERFLOW_IRQ_SHIFT);
                errMask.fifoOverflowIrqm[1U]   |= (uint8_t)
                        ((eventType & CSIRX_EVENT_TYPE_ERR_FIFO_OVERFLOW_STRM1) >>
                         CSL_CSIRX_ERROR_IRQS_STREAM1_FIFO_OVERFLOW_IRQ_SHIFT);
                errMask.fifoOverflowIrqm[2U]   |= (uint8_t)
                        ((eventType & CSIRX_EVENT_TYPE_ERR_FIFO_OVERFLOW_STRM2) >>
                         CSL_CSIRX_ERROR_IRQS_STREAM2_FIFO_OVERFLOW_IRQ_SHIFT);
                errMask.fifoOverflowIrqm[3U]   |= (uint8_t)
                        ((eventType & CSIRX_EVENT_TYPE_ERR_FIFO_OVERFLOW_STRM3) >>
                         CSL_CSIRX_ERROR_IRQS_STREAM3_FIFO_OVERFLOW_IRQ_SHIFT);

                if (CDN_EOK != CSIRX_SetErrorIrqsMaskCfg(&instObj->cslObj.cslCfgData,
                                              &errMask))
                {
                    retVal = FVID2_EFAIL;
                }
            }
        break;
        case CSIRX_EVENT_GROUP_CP_INTD:
            /* In TDA4 SoCs, interrupt from INTD is mapped for level interrupts only */
            config.type = CSIRX_INTD_INT_TYPE_LEVEL ;
            if (FVID2_SOK != CSL_getCsirxIntdIntEnable((CSL_csi_rx_intd_cfgRegs*)instObj->cpIntdBaseAddr,
                                          &config))
            {
                retVal = FVID2_EFAIL;
            }
            config.intr_mask |= eventType;
            if (FVID2_SOK == retVal)
            {
                if (FVID2_SOK != CSL_setCsirxIntdIntEnable((CSL_csi_rx_intd_cfgRegs*)instObj->cpIntdBaseAddr,
                                              &config))
                {
                    retVal = FVID2_EFAIL;
                }
            }
        break;
        case CSIRX_EVENT_GROUP_INFO:
            if (CDN_EOK != CSIRX_GetInfoIrqsMaskCfg(&instObj->cslObj.cslCfgData,
                                          &infoMask))
            {
                retVal = FVID2_EFAIL;
            }
            if (FVID2_SOK == retVal)
            {
                infoMask.deskewEntryIrqm  |= (uint8_t)
                        ((eventType & CSIRX_EVENT_TYPE_INFO_DESKEW_ENTRY) >>
                         CSL_CSIRX_INFO_IRQS_MASK_CFG_DESKEW_ENTRY_IRQM_SHIFT);

                if (CDN_EOK != CSIRX_SetInfoIrqsMaskCfg(&instObj->cslObj.cslCfgData,
                                              &infoMask))
                {
                    retVal = FVID2_EFAIL;
                }
            }
        break;
        case CSIRX_ESM_HI_EVENT_GROUP_ASF:
        case CSIRX_ESM_LOW_EVENT_GROUP_ASF:
        case CSIRX_ESM_CFG_EVENT_GROUP_ASF:
            if (CDN_EOK != CSIRX_GetAsfIrqMaskCfg(&instObj->cslObj.cslCfgData, 
                                        &asfMask))
            {
                retVal = FVID2_EFAIL;
            }
            if (FVID2_SOK == retVal)
            {
                asfMask.asfIntegrityErrIrqm &= (uint8_t)
                        (~((eventType & CSIRX_EVENT_TYPE_ASF_INTEGRITY_ERR) >>
                        CSL_CSIRX_ASF_INT_STATUS_ASF_INTEGRITY_ERR_SHIFT));
                asfMask.asfProtocolErrIrqm &= (uint8_t)
                        (~((eventType & CSIRX_EVENT_TYPE_ASF_PROTOCOL_ERR) >>
                        CSL_CSIRX_ASF_INT_STATUS_ASF_PROTOCOL_ERR_SHIFT));
                asfMask.asfTransToErrIrqm &= (uint8_t)
                        (~((eventType & CSIRX_EVENT_TYPE_ASF_TRANS_TO_ERR) >>
                        CSL_CSIRX_ASF_INT_STATUS_ASF_TRANS_TO_ERR_SHIFT));
                asfMask.asfCsrErrIrqm &= (uint8_t)
                        (~((eventType & CSIRX_EVENT_TYPE_ASF_CSR_ERR) >>
                        CSL_CSIRX_ASF_INT_STATUS_ASF_CSR_ERR_SHIFT));
                asfMask.asfDapErrIrqm &= (uint8_t)
                        (~((eventType & CSIRX_EVENT_TYPE_ASF_DAP_ERR) >>
                        CSL_CSIRX_ASF_INT_STATUS_ASF_DAP_ERR_SHIFT));
                asfMask.asfSramUncorrErrIrqm &= (uint8_t)
                        (~((eventType & CSIRX_EVENT_TYPE_ASF_SRAM_UNCORR_ERR) >>
                        CSL_CSIRX_ASF_INT_STATUS_ASF_SRAM_UNCORR_ERR_SHIFT));
                asfMask.asfSramCorrErrIrqm &= (uint8_t)
                        (~((eventType & CSIRX_EVENT_TYPE_ASF_SRAM_CORR_ERR) >>
                        CSL_CSIRX_ASF_INT_STATUS_ASF_SRAM_CORR_ERR_SHIFT));

                if (CDN_EOK != CSIRX_SetAsfIrqMaskCfg(&instObj->cslObj.cslCfgData,
                                              &asfMask))
                {
                    retVal = FVID2_EFAIL;
                }

                if(((eventType & CSIRX_EVENT_TYPE_ASF_TRANS_TO_ERR) >>
                        CSL_CSIRX_ASF_INT_STATUS_ASF_TRANS_TO_ERR_SHIFT) == 1U)
                {
                    CSIRX_GetAsfTransToFaultMask(&instObj->cslObj.cslCfgData, &asfTransToFaultMsk);
                    
                    asfTransToFaultMsk.asfTransToFaultMask &= (uint8_t)
                            (~((1U & CSIRX_EVENT_TYPE_ASF_TRANS_TO_FAULT) >>
                            CSL_CSIRX_ASF_TRANS_TO_FAULT_MASK_ASF_TRANS_TO_FAULT_0_MASK_SHIFT));

                    if(CDN_EOK != CSIRX_SetAsfTransToFaultMask(&instObj->cslObj.cslCfgData, &asfTransToFaultMsk))
                    {
                        retVal = FVID2_EFAIL;
                    }
                }
            }
        break;
        default:
            retVal = FVID2_EBADARGS;
        break;
    }

    return retVal;
}

int32_t CsirxDrv_eventDisable(Fdrv_Handle drvHandle,
                              uint32_t eventGroup,
                              uint32_t eventType)
{
    int32_t retVal = FVID2_SOK;
    CsirxDrv_InstObj *instObj;
    CSIRX_ErrorIrqsMaskCfg errMask;
    CSIRX_InfoIrqsMaskCfg infoMask;
    CSIRX_AsfIrqMaskCfg asfMask;
    CSIRX_AsfTransToFaultMask asfTransToFaultMsk;
    CSIRX_CpIntd_cfg config;
    CsirxDrv_VirtContext *virtContext;

    virtContext = (CsirxDrv_VirtContext *) drvHandle;
    instObj = virtContext->instObj;
    switch (eventGroup)
    {
        case CSIRX_EVENT_GROUP_ERROR:
            if (CDN_EOK != CSIRX_GetErrorIrqsMaskCfg(&instObj->cslObj.cslCfgData,
                                          &errMask))
            {
                retVal = FVID2_EFAIL;
            }
            if (FVID2_SOK == retVal)
            {
                errMask.frontFifoOverflowIrqm  &= (uint8_t)
                        (~((eventType & CSIRX_EVENT_TYPE_ERR_FIFO_OVERFLOW_FRONT) >>
                         CSL_CSIRX_ERROR_IRQS_FRONT_FIFO_OVERFLOW_IRQ_SHIFT));
                errMask.payloadCrcIrqm         &= (uint8_t)
                        (~((eventType & CSIRX_EVENT_TYPE_ERR_PAYLOAD_CRC) >>
                         CSL_CSIRX_ERROR_IRQS_PAYLOAD_CRC_IRQ_SHIFT));
                errMask.headerEccIrqm          &= (uint8_t)
                        (~((eventType & CSIRX_EVENT_TYPE_ERR_HEADER_ECC) >>
                         CSL_CSIRX_ERROR_IRQS_HEADER_ECC_IRQ_SHIFT));
                errMask.headerCorrectedEccIrqm &= (uint8_t)
                        (~((eventType & CSIRX_EVENT_TYPE_ERR_HEADER_CORRECTED_ECC) >>
                         CSL_CSIRX_ERROR_IRQS_HEADER_CORRECTED_ECC_IRQ_SHIFT));
                errMask.dataIdIrqm             &= (uint8_t)
                        (~((eventType & CSIRX_EVENT_TYPE_ERR_DATA_ID) >>
                         CSL_CSIRX_ERROR_IRQS_DATA_ID_IRQ_SHIFT));
                errMask.invalidAccessIrqm      &= (uint8_t)
                        (~((eventType & CSIRX_EVENT_TYPE_ERR_INVALID_ACCESS) >>
                         CSL_CSIRX_ERROR_IRQS_INVALID_ACCESS_IRQ_SHIFT));
                errMask.spInvalidRcvdIrqm      &= (uint8_t)
                        (~((eventType & CSIRX_EVENT_TYPE_ERR_INVALID_SHORT_PACKET) >>
                         CSL_CSIRX_ERROR_IRQS_SP_INVALID_RCVD_IRQ_SHIFT));
                errMask.fifoOverflowIrqm[0U]   &= (uint8_t)
                        (~((eventType & CSIRX_EVENT_TYPE_ERR_FIFO_OVERFLOW_STRM0) >>
                         CSL_CSIRX_ERROR_IRQS_STREAM0_FIFO_OVERFLOW_IRQ_SHIFT));
                errMask.fifoOverflowIrqm[1U]   &= (uint8_t)
                        (~((eventType & CSIRX_EVENT_TYPE_ERR_FIFO_OVERFLOW_STRM1) >>
                         CSL_CSIRX_ERROR_IRQS_STREAM1_FIFO_OVERFLOW_IRQ_SHIFT));
                errMask.fifoOverflowIrqm[2U]   &= (uint8_t)
                        (~((eventType & CSIRX_EVENT_TYPE_ERR_FIFO_OVERFLOW_STRM2) >>
                         CSL_CSIRX_ERROR_IRQS_STREAM2_FIFO_OVERFLOW_IRQ_SHIFT));
                errMask.fifoOverflowIrqm[3U]   &= (uint8_t)
                        (~((eventType & CSIRX_EVENT_TYPE_ERR_FIFO_OVERFLOW_STRM3) >>
                         CSL_CSIRX_ERROR_IRQS_STREAM3_FIFO_OVERFLOW_IRQ_SHIFT));

                if (CDN_EOK != CSIRX_SetErrorIrqsMaskCfg(&instObj->cslObj.cslCfgData,
                                              &errMask))
                {
                    retVal = FVID2_EFAIL;
                }
            }
        break;
        case CSIRX_EVENT_GROUP_CP_INTD:
            /* In TDA4 SoCs, interrupt from INTD is mapped for level interrupts only */
            config.type = CSIRX_INTD_INT_TYPE_LEVEL ;
            if (FVID2_SOK !=
                CSL_getCsirxIntdIntEnable((CSL_csi_rx_intd_cfgRegs*)instObj->cpIntdBaseAddr,
                &config))
            {
                retVal = FVID2_EFAIL;
            }
            config.intr_mask &= ~eventType;
            if (FVID2_SOK == retVal)
            {
                if (FVID2_SOK != CSL_setCsirxIntdIntEnableClear((CSL_csi_rx_intd_cfgRegs*)instObj->cpIntdBaseAddr,
                                              &config))
                {
                    retVal = FVID2_EFAIL;
                }
            }
        break;
        case CSIRX_EVENT_GROUP_INFO:
            if (CDN_EOK != CSIRX_GetInfoIrqsMaskCfg(&instObj->cslObj.cslCfgData,
                                          &infoMask))
            {
                retVal = FVID2_EFAIL;
            }
            if (FVID2_SOK == retVal)
            {
                infoMask.deskewEntryIrqm  &= (uint8_t)
                        (~((eventType & CSIRX_EVENT_TYPE_INFO_DESKEW_ENTRY) >>
                         CSL_CSIRX_INFO_IRQS_MASK_CFG_DESKEW_ENTRY_IRQM_SHIFT));

                if (CDN_EOK != CSIRX_SetInfoIrqsMaskCfg(&instObj->cslObj.cslCfgData,
                                              &infoMask))
                {
                    retVal = FVID2_EFAIL;
                }
            }
        break;
        case CSIRX_ESM_HI_EVENT_GROUP_ASF:
        case CSIRX_ESM_LOW_EVENT_GROUP_ASF:
        case CSIRX_ESM_CFG_EVENT_GROUP_ASF:
            if (CDN_EOK != CSIRX_GetAsfIrqMaskCfg(&instObj->cslObj.cslCfgData, 
                                        &asfMask))
            {
                retVal = FVID2_EFAIL;
            }
            if (FVID2_SOK == retVal)
            {
                asfMask.asfIntegrityErrIrqm |= (uint8_t)
                        ((eventType & CSIRX_EVENT_TYPE_ASF_INTEGRITY_ERR) >>
                        CSL_CSIRX_ASF_INT_STATUS_ASF_INTEGRITY_ERR_SHIFT);
                asfMask.asfProtocolErrIrqm |= (uint8_t)
                        ((eventType & CSIRX_EVENT_TYPE_ASF_PROTOCOL_ERR) >>
                        CSL_CSIRX_ASF_INT_STATUS_ASF_PROTOCOL_ERR_SHIFT);
                asfMask.asfTransToErrIrqm |= (uint8_t)
                        ((eventType & CSIRX_EVENT_TYPE_ASF_TRANS_TO_ERR) >>
                        CSL_CSIRX_ASF_INT_STATUS_ASF_TRANS_TO_ERR_SHIFT);
                asfMask.asfCsrErrIrqm |= (uint8_t)
                        ((eventType & CSIRX_EVENT_TYPE_ASF_CSR_ERR) >>
                        CSL_CSIRX_ASF_INT_STATUS_ASF_CSR_ERR_SHIFT);
                asfMask.asfDapErrIrqm |= (uint8_t)
                        ((eventType & CSIRX_EVENT_TYPE_ASF_DAP_ERR) >>
                        CSL_CSIRX_ASF_INT_STATUS_ASF_DAP_ERR_SHIFT);
                asfMask.asfSramUncorrErrIrqm |= (uint8_t)
                        ((eventType & CSIRX_EVENT_TYPE_ASF_SRAM_UNCORR_ERR) >>
                        CSL_CSIRX_ASF_INT_STATUS_ASF_SRAM_UNCORR_ERR_SHIFT);
                asfMask.asfSramCorrErrIrqm |= (uint8_t)
                        ((eventType & CSIRX_EVENT_TYPE_ASF_SRAM_CORR_ERR) >>
                        CSL_CSIRX_ASF_INT_STATUS_ASF_SRAM_CORR_ERR_SHIFT);

                if (CDN_EOK != CSIRX_SetAsfIrqMaskCfg(&instObj->cslObj.cslCfgData,
                                              &asfMask))
                {
                    retVal = FVID2_EFAIL;
                }

                if(((eventType & CSIRX_EVENT_TYPE_ASF_TRANS_TO_ERR) >>
                        CSL_CSIRX_ASF_INT_STATUS_ASF_TRANS_TO_ERR_SHIFT) == 1U)
                {
                    CSIRX_GetAsfTransToFaultMask(&instObj->cslObj.cslCfgData, &asfTransToFaultMsk);
                    
                    asfTransToFaultMsk.asfTransToFaultMask |= (uint8_t)
                            ((1U & CSIRX_EVENT_TYPE_ASF_TRANS_TO_FAULT) >>
                            CSL_CSIRX_ASF_TRANS_TO_FAULT_MASK_ASF_TRANS_TO_FAULT_0_MASK_SHIFT);

                    if(CDN_EOK != CSIRX_SetAsfTransToFaultMask(&instObj->cslObj.cslCfgData, &asfTransToFaultMsk))
                    {
                        retVal = FVID2_EFAIL;
                    }
                }
            }
        break;
        default:
            retVal = FVID2_EBADARGS;
        break;
    }

    return retVal;
}

static void CsirxDrv_errorEventIsrFxn(uintptr_t arg)
{
    CsirxDrv_InstObj *instObj;
    CsirxDrv_EventObj *eventObj;
    Csirx_EventStatus status;
    CSIRX_ErrorIrqs errStatus;
    CSIRX_SoftReset resetType;
    CSIRX_ErrorDebug errInfo;
    uint32_t strmIdx, cookie;
    CsirxDrv_VirtContext *virtContext;

    eventObj = (CsirxDrv_EventObj *)arg;
    virtContext = (CsirxDrv_VirtContext *) eventObj->drvHandle;
    instObj = virtContext->instObj;
    status.eventGroup = CSIRX_EVENT_GROUP_ERROR;
    status.eventMasks = 0U;
    status.drvHandle  = eventObj->drvHandle;
    /* Disable HW interrupts here */
    cookie = HwiP_disable();
    /* Check of event status */
    GT_assert(CsirxTrace,
             (CDN_EOK == CSIRX_GetErrorIrqs(&instObj->cslObj.cslCfgData,
                                            &errStatus)));
    /* Clear Event status */
    GT_assert(CsirxTrace,
            (CDN_EOK == CSIRX_SetErrorIrqs(&instObj->cslObj.cslCfgData,
                                           &errStatus)));

#if (CSIRX_DRV_ENABLE_DEBUG == 1U)
    uint32_t oldEventCnt;

    oldEventCnt = gCsirxCommonObj.eventCnt;
    gCsirxCommonObj.eventInstId[gCsirxCommonObj.eventCnt] = instObj->drvInstId;
    /* Read register status directly */
    gCsirxCommonObj.eventStatus[gCsirxCommonObj.eventCnt] =
            CSL_REG32_RD(&instObj->cslObj.configParams.regBase->error_irqs);
    if (NULL != gCsirxCommonObj.getTimeStamp)
    {
        gCsirxCommonObj.eventTS[gCsirxCommonObj.eventCnt] =
                    (uint32_t)(gCsirxCommonObj.getTimeStamp(NULL) / 1000U);
    }
    gCsirxCommonObj.eventCnt = (gCsirxCommonObj.eventCnt + 1U) %
                              CSIRX_DRV_LOG_CNT;
    if (oldEventCnt > gCsirxCommonObj.eventCnt)
    {
        gCsirxCommonObj.eventCntOverflow = UTRUE;
    }
#endif

    /* Service pending events */
    if (1U == errStatus.frontFifoOverflowIrq)
    {
        /* Front FIFO Overflow */
        /* Reset front */
        resetType.protocol = 0U;
        resetType.front    = 1U;
        GT_assert(CsirxTrace,
                 (CDN_EOK == CSIRX_SetSoftReset(&instObj->cslObj.cslCfgData,
                                                &resetType)));
        status.eventMasks |= CSIRX_EVENT_TYPE_ERR_FIFO_OVERFLOW_FRONT;
        instObj->status.frontFIFOOvflCount++;
    }
    if (1U == errStatus.payloadCrcIrq)
    {
        /* Get vc and dt info */
        GT_assert(CsirxTrace,
                (CDN_EOK == CSIRX_GetErrorDebug(&instObj->cslObj.cslCfgData,
                                                &errInfo)));
        status.eventMasks |= CSIRX_EVENT_TYPE_ERR_PAYLOAD_CRC;
        instObj->status.crcCount++;
    }
    if (1U == errStatus.headerEccIrq)
    {
        for (strmIdx = 0U ; strmIdx < CSIRX_NUM_STREAM ; strmIdx++)
        {
            /* Reset only enabled streams */
            GT_assert(CsirxTrace,
                      (FVID2_SOK == CsirxDrv_resetStream(instObj, strmIdx)));
        }
        status.eventMasks |= CSIRX_EVENT_TYPE_ERR_HEADER_ECC;
        instObj->status.eccCount++;
    }
    if (1U == errStatus.headerCorrectedEccIrq)
    {
        /* Nothing to be done here as it is corrected ECC Error */
        status.eventMasks |= CSIRX_EVENT_TYPE_ERR_HEADER_CORRECTED_ECC;
        instObj->status.correctedEccCount++;
    }
    if (1U == errStatus.dataIdIrq)
    {
        /* Nothing to be done here as it is invalid data type has received.
           This will not affect current capture. */
        status.eventMasks |= CSIRX_EVENT_TYPE_ERR_DATA_ID;
        instObj->status.dataIdErrorCount++;
    }
    if (1U == errStatus.invalidAccessIrq)
    {
        /* Nothing to be done here */
        status.eventMasks |= CSIRX_EVENT_TYPE_ERR_INVALID_ACCESS;
        instObj->status.invalidAccessCount++;
    }
    if (1U == errStatus.spInvalidRcvdIrq)
    {
        /* Nothing to be done here */
        status.eventMasks |= CSIRX_EVENT_TYPE_ERR_INVALID_SHORT_PACKET;
        instObj->status.invalidSpCount++;
    }
    if (1U == errStatus.fifoOverflowIrq[0U])
    {
        GT_assert(CsirxTrace,
                      (FVID2_SOK == CsirxDrv_resetStream(instObj, 0U)));
        status.eventMasks |= CSIRX_EVENT_TYPE_ERR_FIFO_OVERFLOW_STRM0;
        instObj->status.strmFIFOOvflCount[0U]++;
    }
    if (1U == errStatus.fifoOverflowIrq[1U])
    {
        GT_assert(CsirxTrace,
                      (FVID2_SOK == CsirxDrv_resetStream(instObj, 1U)));
        status.eventMasks |= CSIRX_EVENT_TYPE_ERR_FIFO_OVERFLOW_STRM1;
        if (CSIRX_NUM_STREAM > 1U)
        {
            instObj->status.strmFIFOOvflCount[1U]++;
        }
    }
    if (1U == errStatus.fifoOverflowIrq[2U])
    {
        GT_assert(CsirxTrace,
                      (FVID2_SOK == CsirxDrv_resetStream(instObj, 2U)));
        status.eventMasks |= CSIRX_EVENT_TYPE_ERR_FIFO_OVERFLOW_STRM2;
        if (2U < CSIRX_NUM_STREAM)
        {
            instObj->status.strmFIFOOvflCount[2U]++;
        }
    }
    if (1U == errStatus.fifoOverflowIrq[3U])
    {
        GT_assert(CsirxTrace,
                      (FVID2_SOK == CsirxDrv_resetStream(instObj, 3U)));
        status.eventMasks |= CSIRX_EVENT_TYPE_ERR_FIFO_OVERFLOW_STRM3;
        if (3U < CSIRX_NUM_STREAM)
        {
            instObj->status.strmFIFOOvflCount[3U]++;
        }
    }

    /** Enable HW interrupts here */
    HwiP_restore(cookie);

    if ((NULL != eventObj->eventPrms.eventCb) && (0U != status.eventMasks))
    {
        /* Issue a CB to application */
        eventObj->eventPrms.eventCb(status, eventObj->eventPrms.appData);
    }
}
static void CsirxDrv_cpIntdEventIsrFxn(uintptr_t arg)
{
    CsirxDrv_InstObj *instObj;
    CsirxDrv_EventObj *eventObj;
    Csirx_EventStatus status;
    uint32_t cookie;
    CsirxDrv_VirtContext *virtContext;
    CSIRX_CpIntd_cfg config;

    eventObj = (CsirxDrv_EventObj *)arg;
    virtContext = (CsirxDrv_VirtContext *) eventObj->drvHandle;
    instObj = virtContext->instObj;
    status.eventGroup = CSIRX_EVENT_GROUP_CP_INTD;
    status.eventMasks = 0U;
    status.drvHandle  = eventObj->drvHandle;
    /* Disable HW interrupts here */
    cookie = HwiP_disable();
    /* In TDA4 SoCs, interrupt from INTD is mapped for level interrupts only */
    config.type = CSIRX_INTD_INT_TYPE_LEVEL ;
    /* Check of event status */
    GT_assert(CsirxTrace,
             (FVID2_SOK == CSL_getCsirxIntdStatus((CSL_csi_rx_intd_cfgRegs*)instObj->cpIntdBaseAddr,
                                            &config)));
    /* Clear Event status */
    GT_assert(CsirxTrace,
            (FVID2_SOK == CSL_setCsirxIntdStatusClear((CSL_csi_rx_intd_cfgRegs*)instObj->cpIntdBaseAddr,
                                           &config)));

    /* Set End of Interrupt register to allow next interrupt */
    CSL_csirxIntdsetEOI((CSL_csi_rx_intd_cfgRegs*)instObj->cpIntdBaseAddr,
                                           config.intr_mask);

    /* Service pending events */
    if (CSIRX_EVENT_TYPE_CP_INTD_VP1_ERRINLNFRM ==
	(CSIRX_EVENT_TYPE_CP_INTD_VP1_ERRINLNFRM & config.intr_mask))
    {
        status.eventMasks |= CSIRX_EVENT_TYPE_CP_INTD_VP1_ERRINLNFRM;
        instObj->status.vp1ErrLnFrm++;
    }
    if (CSIRX_EVENT_TYPE_CP_INTD_VP0_ERRINLNFRM ==
	     (CSIRX_EVENT_TYPE_CP_INTD_VP0_ERRINLNFRM & config.intr_mask))
    {
        status.eventMasks |= CSIRX_EVENT_TYPE_CP_INTD_VP0_ERRINLNFRM;
        instObj->status.vp0ErrLnFrm++;
    }
    if (CSIRX_EVENT_TYPE_CP_INTD_FIFO_OVERFLOW ==
	     (CSIRX_EVENT_TYPE_CP_INTD_FIFO_OVERFLOW & config.intr_mask))
    {
        status.eventMasks |= CSIRX_EVENT_TYPE_CP_INTD_FIFO_OVERFLOW;
        instObj->status.psilFifoOverflow++;
    }
    /** Enable HW interrupts here */
    HwiP_restore(cookie);

    if ((NULL != eventObj->eventPrms.eventCb) && (0U != status.eventMasks))
    {
        /* Issue a CB to application */
        eventObj->eventPrms.eventCb(status, eventObj->eventPrms.appData);
    }
}

static void CsirxDrv_infoEventIsrFxn(uintptr_t arg)
{
    CsirxDrv_InstObj *instObj;
    CsirxDrv_EventObj *eventObj;
    Csirx_EventStatus status;
    CSIRX_InfoIrqs infoStatus;
    uint32_t cookie;
    CsirxDrv_VirtContext *virtContext;

    eventObj = (CsirxDrv_EventObj *)arg;
    virtContext = (CsirxDrv_VirtContext *) eventObj->drvHandle;
    instObj = virtContext->instObj;
    status.eventGroup = CSIRX_EVENT_GROUP_INFO;
    status.eventMasks = 0U;
    status.drvHandle  = eventObj->drvHandle;
    /* Disable HW interrupts here */
    cookie = HwiP_disable();
    /* Check of event status */
    GT_assert(CsirxTrace,
             (CDN_EOK == CSIRX_GetInfoIrqs(&instObj->cslObj.cslCfgData,
                                            &infoStatus)));
    /* Clear Event status */
    GT_assert(CsirxTrace,
            (CDN_EOK == CSIRX_SetInfoIrqs(&instObj->cslObj.cslCfgData,
                                           &infoStatus)));

    /* Service pending events */
    if (1U == infoStatus.deskewEntryIrq)
    {
        status.eventMasks |= CSIRX_EVENT_TYPE_INFO_DESKEW_ENTRY;
        instObj->status.deSkewEntryCount++;
    }

    /** Enable HW interrupts here */
    HwiP_restore(cookie);

    if ((NULL != eventObj->eventPrms.eventCb) && (0U != status.eventMasks))
    {
        /* Issue a CB to application */
        eventObj->eventPrms.eventCb(status, eventObj->eventPrms.appData);
    }
}

static void CsirxDrv_asfEsmHighEventIsrFxn(uintptr_t arg)
{
    CsirxDrv_InstObj *instObj;
    CsirxDrv_EventObj *eventObj;
    Csirx_EventStatus status[CSIRX_INSTANCE_ID_MAX] = {0U};
    CSIRX_AsfIrqs asfStatus;
    uint32_t instIdx;
    uint32_t cookie;

    eventObj = (CsirxDrv_EventObj *)arg;

    /* Disable HW interrupts here */
    cookie = HwiP_disable();

    for(instIdx = 0U; instIdx < CSIRX_INSTANCE_ID_MAX; instIdx++)
    {
        instObj = (CsirxDrv_InstObj *)&gCsirxCommonObj.instObj[instIdx];
        status[instIdx].eventGroup = CSIRX_ESM_HI_EVENT_GROUP_ASF;
        status[instIdx].eventMasks = 0U;
        status[instIdx].drvHandle  = instObj->eventObj->drvHandle;
        
        /* Check of event status */
        GT_assert(CsirxTrace,
                (CDN_EOK == CSIRX_GetAsfIrqs(&instObj->cslObj.cslCfgData,
                                                &asfStatus)));
        /* Clear Event status */
        GT_assert(CsirxTrace,
                (CDN_EOK == CSIRX_SetAsfIrqs(&instObj->cslObj.cslCfgData,
                                            &asfStatus)));
        
        /* Service pending events */
        if (1U == asfStatus.asfIntegrityErrIrq)
        {
            status[instIdx].eventMasks |= CSIRX_EVENT_TYPE_ASF_INTEGRITY_ERR;
        }
        if (1U == asfStatus.asfProtocolErrIrq)
        {
            status[instIdx].eventMasks |= CSIRX_EVENT_TYPE_ASF_PROTOCOL_ERR;
        }
        if (1U == asfStatus.asfTransToErrIrq)
        {
            status[instIdx].eventMasks |= CSIRX_EVENT_TYPE_ASF_TRANS_TO_ERR;
            instObj->status.asfTransToEvent++;
        }
        if (1U == asfStatus.asfCsrErrIrq)
        {
            status[instIdx].eventMasks |= CSIRX_EVENT_TYPE_ASF_CSR_ERR;
        }
        if (1U == asfStatus.asfDapErrIrq)
        {
            status[instIdx].eventMasks |= CSIRX_EVENT_TYPE_ASF_DAP_ERR;
        }
        if (1U == asfStatus.asfSramUncorrErrIrq)
        {
            status[instIdx].eventMasks |= CSIRX_EVENT_TYPE_ASF_SRAM_UNCORR_ERR;
        }
        if (1U == asfStatus.asfSramCorrErrIrq)
        {
            status[instIdx].eventMasks |= CSIRX_EVENT_TYPE_ASF_SRAM_CORR_ERR;
        }

        if ((NULL != eventObj->eventPrms.eventCb) && (0U != status[instIdx].eventMasks))
        {
            /* Issue a CB to application */
            eventObj->eventPrms.eventCb(status[instIdx], eventObj->eventPrms.appData);
        }
    }

    /** Enable HW interrupts here */
    HwiP_restore(cookie);
}

static void CsirxDrv_asfEsmLowEventIsrFxn(uintptr_t arg)
{
    CsirxDrv_InstObj *instObj;
    CsirxDrv_EventObj *eventObj;
    Csirx_EventStatus status[CSIRX_INSTANCE_ID_MAX] = {0U};
    CSIRX_AsfIrqs asfStatus;
    uint32_t instIdx;
    uint32_t cookie;

    eventObj = (CsirxDrv_EventObj *)arg;

    /* Disable HW interrupts here */
    cookie = HwiP_disable();

    for(instIdx = 0U; instIdx < CSIRX_INSTANCE_ID_MAX; instIdx++)
    {
        instObj = (CsirxDrv_InstObj *)&gCsirxCommonObj.instObj[instIdx];
        status[instIdx].eventGroup = CSIRX_ESM_HI_EVENT_GROUP_ASF;
        status[instIdx].eventMasks = 0U;
        status[instIdx].drvHandle  = instObj->eventObj->drvHandle;
        
        /* Check of event status */
        GT_assert(CsirxTrace,
                (CDN_EOK == CSIRX_GetAsfIrqs(&instObj->cslObj.cslCfgData,
                                                &asfStatus)));
        /* Clear Event status */
        GT_assert(CsirxTrace,
                (CDN_EOK == CSIRX_SetAsfIrqs(&instObj->cslObj.cslCfgData,
                                            &asfStatus)));
        
        /* Service pending events */
        if (1U == asfStatus.asfIntegrityErrIrq)
        {
            status[instIdx].eventMasks |= CSIRX_EVENT_TYPE_ASF_INTEGRITY_ERR;
        }
        if (1U == asfStatus.asfProtocolErrIrq)
        {
            status[instIdx].eventMasks |= CSIRX_EVENT_TYPE_ASF_PROTOCOL_ERR;
        }
        if (1U == asfStatus.asfTransToErrIrq)
        {
            status[instIdx].eventMasks |= CSIRX_EVENT_TYPE_ASF_TRANS_TO_ERR;
            instObj->status.asfTransToEvent++;
        }
        if (1U == asfStatus.asfCsrErrIrq)
        {
            status[instIdx].eventMasks |= CSIRX_EVENT_TYPE_ASF_CSR_ERR;
        }
        if (1U == asfStatus.asfDapErrIrq)
        {
            status[instIdx].eventMasks |= CSIRX_EVENT_TYPE_ASF_DAP_ERR;
        }
        if (1U == asfStatus.asfSramUncorrErrIrq)
        {
            status[instIdx].eventMasks |= CSIRX_EVENT_TYPE_ASF_SRAM_UNCORR_ERR;
        }
        if (1U == asfStatus.asfSramCorrErrIrq)
        {
            status[instIdx].eventMasks |= CSIRX_EVENT_TYPE_ASF_SRAM_CORR_ERR;
        }

        if ((NULL != eventObj->eventPrms.eventCb) && (0U != status[instIdx].eventMasks))
        {
            /* Issue a CB to application */
            eventObj->eventPrms.eventCb(status[instIdx], eventObj->eventPrms.appData);
        }
    }

    /** Enable HW interrupts here */
    HwiP_restore(cookie);
}

static void CsirxDrv_asfEsmCfgEventIsrFxn(uintptr_t arg)
{
    CsirxDrv_EventObj *eventObj;
    Csirx_EventStatus status;

    eventObj = (CsirxDrv_EventObj *)arg;
    status.eventGroup = CSIRX_ESM_CFG_EVENT_GROUP_ASF;
    status.eventMasks = 0U;
    status.drvHandle  = eventObj->drvHandle;

    /** Interrupt due to ESM Module Configuration Error */
    if ((NULL != eventObj->eventPrms.eventCb))
    {
        /* Issue a CB to application */
        eventObj->eventPrms.eventCb(status, eventObj->eventPrms.appData);
    }
}

static int32_t CsirxDrv_resetStream(const CsirxDrv_InstObj *instObj,
                                    uint32_t strmIdx)
{
    int32_t retVal = FVID2_SOK;
    CSIRX_StreamStatus strmStatus;
    CSIRX_StreamCtrl strmCtrlParams;
    uint32_t status, currTimeout = 0U;

    /* check if stream is enabled */
    if (CDN_EOK != CSIRX_GetStreamStatus(&instObj->cslObj.cslCfgData,
                              &strmStatus,
                              strmIdx))
    {
        retVal = FVID2_EFAIL;
    }
    /* Reset stream */
    if (FVID2_SOK == retVal)
    {
        if (1U == strmStatus.running)
        {
            /* Stream is enabled */
            strmCtrlParams.softRst = 1U;
            strmCtrlParams.abrt    = 0U;
            strmCtrlParams.stop    = 0U;
            strmCtrlParams.start   = 0U;
            status = CSIRX_SetStreamCtrl(&instObj->cslObj.cslCfgData,
                                         &strmCtrlParams,
                                         strmIdx);
            if (CDN_EOK != status)
            {
                retVal = FVID2_EBADARGS;
            }
            currTimeout = 0U;
            status = CSIRX_GetStreamStatus(&instObj->cslObj.cslCfgData,
                                           &strmStatus,
                                           strmIdx);
            if (CDN_EOK != status)
            {
                retVal = FVID2_EBADARGS;
            }
            /* Wait for stream to become IDLE */
            while ((0x0U != strmStatus.readyState) &&
                   (0x0U != strmStatus.streamFsm)  &&
                   (FVID2_SOK == retVal))
            {
                 status = CSIRX_GetStreamStatus(&instObj->cslObj.cslCfgData,
                                           &strmStatus,
                                           strmIdx);
                if (CDN_EOK != status)
                {
                    retVal = FVID2_EBADARGS;
                    break;
                }
                if (CSIRX_TIMEOUT_VAL_MS < currTimeout)
                {
                    retVal = FVID2_ETIMEOUT;
                }
                else
                {
                    (void)Osal_delay(1U);
                    currTimeout++;
                }
            }
            /* Re-start stream */
            if (FVID2_SOK == retVal)
            {
                strmCtrlParams.softRst = 0U;
                strmCtrlParams.abrt    = 0U;
                strmCtrlParams.stop    = 0U;
                strmCtrlParams.start   = 1U;
                status = CSIRX_SetStreamCtrl(&instObj->cslObj.cslCfgData,
                                            &strmCtrlParams,
                                            strmIdx);
                if (CDN_EOK != status)
                {
                    retVal = FVID2_EBADARGS;
                }
            }
        }
    }

    return retVal;
}

int32_t CsirxDrv_eventGroupUnRegister(Fdrv_Handle drvHandle,
                                      uint32_t eventGroup)
{
    int32_t retVal = FVID2_SOK;
    CsirxDrv_InstObj *instObj = NULL;
    CsirxDrv_EventObj *eventObj;
    CsirxDrv_VirtContext *virtContext;

    /* Error Check: Parameters */
    GT_assert(CsirxTrace, (NULL_PTR != drvHandle));
    virtContext = (CsirxDrv_VirtContext *) drvHandle;
    instObj = virtContext->instObj;
    eventObj = &instObj->eventObj[eventGroup];
    if (CSIRX_DRV_USAGE_STATUS_IN_USE != instObj->inUse)
    {
        retVal = FVID2_EFAIL;
    }
    if (NULL != instObj->lockSem)
    {
        /* Pend on the instance semaphore */
        (void)SemaphoreP_pend(instObj->lockSem, SemaphoreP_WAIT_FOREVER);
    }
    if (drvHandle == eventObj->drvHandle)
    {
        /* Only let instance which registered events to unregister them, for
           others this will be empty call */
        if (FVID2_SOK == retVal)
        {
            /* Disable All HW events for the given group first */
            retVal = CsirxDrv_eventDisable(drvHandle, eventGroup,
                                           eventObj->eventPrms.eventMasks);
        }
        if (FVID2_SOK == retVal)
        {
            /* Disable Core interrupts */
            /* Delete interrupts: TODO: 0U in following call should be core based */
            (void) Osal_DeleteInterrupt((HwiP_Handle) eventObj->hwiHandle, 0);
            eventObj = &instObj->eventObj[eventGroup];
            eventObj->coreIntrNum   = CSIRX_INTR_INVALID;
            eventObj->eventInitDone = 0U;
            eventObj->hwiHandle     = NULL;
        }

    }
    if (NULL != instObj->lockSem)
    {
        /* Post the instance semaphore */
        (void)SemaphoreP_post(instObj->lockSem);
    }

    return retVal;
}
