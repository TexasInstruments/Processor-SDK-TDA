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
 *  \file vhwa_m2mNfApi.c
 *
 *  \brief API Implementation
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "src/drv/vhwa_m2mNfPriv.h"


/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief   Create Queues, required for storing pending requests
 *
 * \param   instObj             Instance object.
 *
 * \return  FVID2_SOK in case of sucess, Error otherwise
 *
 **/
static int32_t Vhwa_nfCreateQueue(Vhwa_M2mNfInstObj *instObj);

/**
 * \brief   Delete Queues
 *
 * \param   instObj             Instance object.
 *
 * \return  FVID2_SOK in case of sucess, Error otherwise
 *
 **/
static int32_t Vhwa_nfDeleteQueue(Vhwa_M2mNfInstObj *instObj);

/**
 * \brief   Local Function to Allocate Handle Object from the
 *          pool of handle objects.
 *          No protection inside the function, Caller should protect
 *          the function call
 *
 * \param   instObj      Instance object.
 * \param   instId       Instance ID
 *
 * \return  pointer to handle object on success else NULL.
 *
 */
static Vhwa_M2mNfHandleObj *Vhwa_nfAllocHdlObj(const Vhwa_M2mNfInstObj *instObj);

/**
 * \brief   Local Function to free Handle Object
 *          No protection inside the function, Caller should protect
 *          the function call
 *
 * \param   instObj      Instance object.
 *
 */
static void Vhwa_nfFreeHandleObj(Vhwa_M2mNfHandleObj *hObj);

/**
 * \brief   Local function to Inititalize the handle object.
 *
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 *
 * \return  FVID2_SOK in case of sucess, Error otherwise
 *
 **/
static void Vhwa_nfInitHandleObj(Vhwa_M2mNfInstObj *instObj,
                                  Vhwa_M2mNfHandleObj *hObj);

/**
 * \brief   Local function to set the NF filter coefficients
 *
 *
 * \param   comObj             Common driver object
 * \param   pCoeffCfg          NF coeff parameters
 *
 * \return  FVID2_SOK in case of sucess, Error otherwise
 *
 **/
static int32_t Vhwa_nfSetFilterTables(Vhwa_M2mNfInstObj *instObj,
                                      const Nf_WgtTableConfig *pCoeffCfg);

/**
 * \brief   Implementation of SET_PARAMS ioctl.
 *          It uses #Vhwa_nfCheckNfCfg to validate the config.
 *          If it is valid, copies the config into handle object
 *          If it is invalid, it reverts LSE/HTS config to known valid config
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   comObj              Common driver Object
 * \param   nfPrms             NF configuration to be set
 *
 * \return  FVID2_SOK in case of sucess, Error otherwise
 *
 **/
static int32_t Vhwa_nfSetParams(Vhwa_M2mNfInstObj *instObj,
                                 Vhwa_M2mNfHandleObj *hObj,
                                 Vhwa_M2mNfConfig *nfPrms);

/**
 * \brief   Implementation of REGISTER_ERR_CB ioctl.
 *
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   eePrms              Error Parameters
 *
 * \return  FVID2_SOK in case of sucess, Error otherwise
 *
 **/
static int32_t Vhwa_nfSetEeParams(const Vhwa_M2mNfInstObj *instObj,
    Vhwa_M2mNfHandleObj *hObj, const Nf_ErrEventParams *eePrms);

/**
 * \brief   Implementation of REGISTER_WDTIMER_ERR_CB ioctl.
 *
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   wdTimerEePrms       Watchdog Timer Error Parameters
 *
 * \return  FVID2_SOK in case of sucess, Error otherwise
 *
 **/
static int32_t Vhwa_nfSetWdTimerEeParams(const Vhwa_M2mNfInstObj *instObj,
    Vhwa_M2mNfHandleObj *hObj, const Nf_WdTimerErrEventParams *wdTimerEePrms);

/**
 * \brief   Local function to set interrupt configuration in hardware
 *
 * \param   vhwaIrqNum          VHWA IRQ number
 * \param   intdRegs            Pointer to INTD registers
 * \param   hObj                Handle Object
 *
 * \return  FVID2_SOK in case of success, Error otherwise
 *
 **/
static int32_t Vhwa_m2mNfSetIntrInHW(uint32_t vhwaIrqNum,
                                volatile CSL_vpac_intd_cfgRegs  *intdRegs,
                                const Vhwa_M2mNfHandleObj *hObj);

/**
 * \brief   Based on the given NF config, it initializes HTS configuration.
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   nfPrms             NF Configuration
 *
 **/
static void Vhwa_nfSetHtsCfg(Vhwa_M2mNfInstObj *instObj,
                              Vhwa_M2mNfHandleObj *hObj);

/**
 * \brief   Based on the given NF config, it initializes LSE configuration.
 *
 * \param   instObj        Instance Object
 * \param   hObj           Handle Object
 * \param   nfPrms        NF Configuration
 *
 **/
static void Vhwa_nfSetLseCfg(const Vhwa_M2mNfInstObj *instObj,
                              Vhwa_M2mNfHandleObj *hObj);

/**
 * \brief   Local function to check the NF configuration
 *
 * \param   instObj             Instance Object
 * \param   nfPrms             NF Configuration
 *
 * \return  FVID2_SOK in case of correct configuration, Error otherwise
 *
 **/
static int32_t Vhwa_nfCheckNfCfg(Vhwa_M2mNfInstObj *instObj,
                                 Vhwa_M2mNfConfig *nfCfg);

/**
 * \brief   Local function to set CSL paramenter for Luma and Chroma based on
 *          NF configuration
 *
 * \param   hObj                Handle Object
 * \param   nfPrms             NF Configuration
 *
 **/
static void Vhwa_nfSetCfgParams(Vhwa_M2mNfHandleObj *hObj,
                                const Vhwa_M2mNfConfig *nfCfg);

/**
 * \brief   Local function to set HTS Bandwidth limiter
 *
 * \param   hObj                Handle Object
 * \param   bwLimit             Bandwidth limiter Configuration
 *
 **/
static void Vhwa_nfSetBwLimit(Vhwa_M2mNfHandleObj *hObj,
                              const Vhwa_HtsLimiter *bwLimit);

/**
 * \brief   Local function to check NF Create params like Driver Id and Instance Id.
 *
 * \param   drvId               Driver ID
 * \param   drvInstId           Instance ID
 *
 *  \return FVID2_SOK if successful, else suitable error code
 **/
static int32_t Vhwa_nfCheckCreatePrms(uint32_t drvId, uint32_t drvInstId);

/* Implementation of FVID2 APIs */
/**
 * \brief   FVID2 Create Function.
 *
 * \param   instObj             Instance object.
 *
 * \return  FVID2 Driver Handle.
 *
 **/
Fdrv_Handle Vhwa_m2mNfCreate(UInt32 drvId, UInt32 drvInstId,
                                     Ptr createArgs, Ptr createStatusArgs,
                                     const Fvid2_DrvCbParams *cbPrms);

/**
 * \brief   FVID2 Delete Function.
 *
 * \param   handle              FVID2 driver handle.
 *
 * \return  FVID2_SOK on success, else FVID2 error code
 *
 **/
Int32 Vhwa_m2mNfDelete(Fdrv_Handle handle, Ptr deleteArgs);

/**
 * \brief   FVID2 Control Function.
 *
 * \param   handle              FVID2 driver handle.
 *
 * \return  FVID2_SOK on success, else FVID2 error code
 *
 **/
Int32 Vhwa_m2mNfControl(Fdrv_Handle handle, UInt32 cmd,
                                  Ptr cmdArgs, Ptr cmdStatusArgs);

/**
 * \brief   FVID2 Process Request function.
 *
 * \param   handle              FVID2 driver handle.
 *
 * \return  FVID2_SOK on success, else FVID2 error code
 *
 **/
Int32 Vhwa_m2mNfProcessReq(Fdrv_Handle handle,
                                     Fvid2_FrameList *inFrmList,
                                     Fvid2_FrameList *outFrmList,
                                     uint32_t timeout);

/**
 * \brief   FVID2 Get Process Request function.
 *
 * \param   handle              FVID2 driver handle.
 *
 * \return  FVID2_SOK on success, else FVID2 error code
 *
 **/
Int32 Vhwa_m2mNfGetProcessReq(Fdrv_Handle handle,
                                        Fvid2_FrameList *inFrmList,
                                        Fvid2_FrameList *outFrmList,
                                        UInt32 timeout);

/**
 * \brief   Reconfigure NF MMR registers as needed for the current handle/queue.
 *
 * \param   instObj   Instance Object.
 * \param   hObj      Handle Object.
 * \param   qObj      Queue Object.
 *
 * \return  FVID2_SOK on success, else FVID2 error code.
 */
static int32_t Vhwa_m2mNfReconfigReinitReg(const Vhwa_M2mNfInstObj *instObj,
                                            const Vhwa_M2mNfHandleObj *hObj,
                                            const Vhwa_M2mNfQueueObj *qObj);


/**
 * @brief Validates the NF status registers against expected values.
 *
 * This function compares the current NF status register values with the expected values
 * provided in the 'expected' parameter. It uses the SoC-specific information from 'socInfo'
 * to perform the validation according to the hardware configuration.
 *
 * @param expected Pointer to a NfStatusRegisterGroup structure containing the expected
 * status register values.
 * @param socInfo  Pointer to a Nf_SocInfo structure containing SoC-specific information
 * required for validation.
 *
 * @return Returns 0 (FVID2_SOK) if the status registers match the expected values,
 * or a negative error code if there is a mismatch or an error occurs.
 */
int32_t Vhwa_m2mNfStatusRegValidate(const NfStatusRegisterGroup *expected,
                                    const Nf_SocInfo *socInfo);
/**
 * \brief   Updates the NfStatusRegisterGroup structure in the handle object with expected status register values.
 *
 * \param   hObj    Pointer to the Vhwa_M2mNfHandleObj whose statusRegs should be updated.
 *
 * \return  FVID2_SOK on success, error code otherwise.
 */
static int32_t vhwaM2mNfUpdateStatusRegGroup(Vhwa_M2mNfHandleObj *hObj);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
Vhwa_M2mNfHandleObj   gM2mNfHandleObj[VHWA_M2M_NF_MAX_HANDLES];
Vhwa_M2mNfInstObj     gM2mNfInstObj;

Fvid2_DrvOps gM2mNfFvid2DrvOps = {
    FVID2_VHWA_M2M_NF_DRV_ID,
    /**< Unique driver Id. */
    Vhwa_m2mNfCreate,
    /**< FVID2 create function pointer. */
    Vhwa_m2mNfDelete,
    /**< FVID2 delete function pointer. */
    Vhwa_m2mNfControl,
    /**< FVID2 control function pointer. */
    NULL, NULL,
    /**< FVID2 queue function pointer. */
    Vhwa_m2mNfProcessReq,
    /**< FVID2 process request function pointer. */
    Vhwa_m2mNfGetProcessReq,
    /**< FVID2 get processed request function pointer. */
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t Vhwa_m2mNfInit(Vhwa_M2mNfInitPrms *initPrms)
{
    int32_t             status = FVID2_SOK;
    uint32_t            cnt;
    #if !defined(MCU_PLUS_SDK)
    SemaphoreP_Params   params;
    #endif
    Vhwa_M2mNfInstObj *instObj = NULL;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != initPrms)
    /* LDRA_JUSTIFY_END */
    {
        instObj = &gM2mNfInstObj;

        /* Reset all instance object to 0x0 */
        Fvid2Utils_memset(instObj, 0U, sizeof (gM2mNfInstObj));

        /* Mark pool flags as free */
        for (cnt = 0U; cnt < VHWA_M2M_NF_MAX_HANDLES; cnt++)
        {
            gM2mNfHandleObj[cnt].isUsed = (uint32_t) UFALSE;
        }

        /* Set HTS pipeline */
        instObj->pipeline = VHWA_M2M_NF_HTS_PIPELINE;

        /* Initialize stalled flag to FALSE */
        instObj->isStalled = (uint32_t)UFALSE;

        Nf_getSocInfo(&instObj->socInfo);

        status = Fvid2_registerDriver(&gM2mNfFvid2DrvOps);
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
            instObj->isRegistered = (uint32_t)UTRUE;
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to print the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            GT_0trace(VhwaNfTrace, GT_ERR,
                "FVID2 driver registration failed!!\n");
            instObj->isRegistered = (uint32_t)UFALSE;
        }
        /* LDRA_JUSTIFY_END */

        /* Create Semaphores */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps.
        Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            /* Allocate lock semaphore */
            #if defined(MCU_PLUS_SDK)
            if(FVID2_SOK != SemaphoreP_constructBinary(&instObj->lock,1U))
            #else
            SemaphoreP_Params_init(&params);
            params.mode = SemaphoreP_Mode_BINARY;
            instObj->lock = SemaphoreP_create(1U, &params);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL == instObj->lock)
            #endif
            {
                GT_0trace(VhwaNfTrace, GT_ERR,
                    "Failed to allocate instance semaphore!!\n");
                status = FVID2_EALLOC;
            }
            /* LDRA_JUSTIFY_END */
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps.
        Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
#if defined VHWA_M2M_VPAC_INSTANCE
#if (VHWA_M2M_VPAC_INSTANCE == 0)
            instObj->hwaLockIdx = VHWA_VPAC0_NF_LOCK_IDX;
#elif defined(VHWA_M2M_VPAC_INSTANCE) && (VHWA_M2M_VPAC_INSTANCE == 1)
            instObj->hwaLockIdx = VHWA_VPAC1_NF_LOCK_IDX;
#endif
#endif
            /* Allocate lock semaphore */
            status = Vhwa_commonCreateHwaLock(instObj->hwaLockIdx);
        }

        /* Create free and request queues */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps.
        Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            status = Vhwa_nfCreateQueue(instObj);
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps.
        Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            /* Initialize UDMA, ie allocate and initialize channels
               required for NF output */
            status = Vhwa_m2mNfUdmaInit(instObj, initPrms);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to print the error.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK != status)
            {
                GT_0trace(VhwaNfTrace, GT_ERR,
                    "UDMA Initialization Failed !!\n");
            }
            /* LDRA_JUSTIFY_END */
        }

        /* Register ISR handler for the given irq number */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps.
        Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            instObj->irqNum = initPrms->irqInfo.irqNum;
            instObj->vhwaIrqNum = initPrms->irqInfo.vhwaIrqNum;

            status = Vhwa_m2mNfRegisterIsr(instObj);
        }

        /* Init is done, copy init params locally,
           enable VPAC and NF in VPAC Top */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps.
        Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            Fvid2Utils_memcpy(&instObj->initPrms, initPrms,
                sizeof(Vhwa_M2mNfInitPrms));

            /* Enable NF at VPAC Top*/
            CSL_vpacEnableModule(instObj->socInfo.vpacCntlRegs,
                VPAC_MODULE_NF, (uint32_t)UTRUE);

            instObj->initDone = (uint32_t)UTRUE;
            instObj->lastHndlObj = NULL;
        }
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK != status)
    {
        Vhwa_m2mNfDeInit();
    }

     /* LDRA_JUSTIFY_END */
    return (status);
}

void Vhwa_m2mNfDeInit(void)
{
    Vhwa_M2mNfInstObj *instObj = NULL;

    instObj = &gM2mNfInstObj;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale:
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (instObj->openCnt > 0u)
    {
        GT_0trace(VhwaNfTrace, GT_ERR,
            "Warning: All driver handles are not closed!!\n");
    }
    /* LDRA_JUSTIFY_END */

    /* Stop UDMA channels */
    (void)Vhwa_m2mNfStopCh(instObj);

    Vhwa_m2mNfUnregisterIsr(instObj);

    (void)Vhwa_m2mNfUdmaDeInit(instObj);

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((uint32_t)UTRUE == instObj->isRegistered)
    /* LDRA_JUSTIFY_END */
    {
        (void)Fvid2_unRegisterDriver(&gM2mNfFvid2DrvOps);
    }

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale:
    The negative test framework and test apps cannot reach this portion.
    These parameters cannot be configured from test app or kernel directly and can only be overwritten by memory corruption.
    This is fail safe mechanism and cannot be removed
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((uint32_t)UTRUE == instObj->isSl2AllocDone)
    {
        Vhwa_m2mNfFreeSl2();
    }
    /* LDRA_JUSTIFY_END */

    /* Delete the lock semaphore */
    #if defined(MCU_PLUS_SDK)
    SemaphoreP_destruct(&instObj->lock);
    #else
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != instObj->lock)
    /* LDRA_JUSTIFY_END */
    {
        (void)SemaphoreP_delete(instObj->lock);
        instObj->lock = NULL;
    }
    #endif

    /* Delete the lock semaphore */
    Vhwa_commonDeleteHwaLock(instObj->hwaLockIdx);

    (void)Vhwa_nfDeleteQueue(instObj);

    /* Init all global variables to zero */
    Fvid2Utils_memset(instObj, 0U, sizeof (gM2mNfInstObj));

    instObj->initDone = (uint32_t)UFALSE;
}

Fdrv_Handle Vhwa_m2mNfCreate(UInt32 drvId, UInt32 drvInstId,
    Ptr createArgs, Ptr createStatusArgs, const Fvid2_DrvCbParams *cbPrms)
{
    int32_t                 status = FVID2_SOK;
    Vhwa_M2mNfInstObj     *instObj = NULL;
    Vhwa_M2mNfHandleObj   *hObj = NULL;
    Vhwa_M2mNfCreatePrms  *nfCreatePrms = NULL;
    Fdrv_Handle             handle = NULL;
    Nf_SocInfo            *socInfo = NULL;
    Vhwa_M2mNfSl2AllocPrms sl2AllocPrms;

    instObj = &gM2mNfInstObj;
    socInfo = &instObj->socInfo;

    /* Check for errors */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == createArgs) ||
        (NULL == cbPrms))
    {
        GT_0trace(VhwaNfTrace, GT_ERR, "NULL Pointer !!\n");
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        status = Vhwa_nfCheckCreatePrms(drvId, drvInstId);
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Open not allowed if init is not done */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)UFALSE == instObj->initDone)
        {
            GT_0trace(VhwaNfTrace, GT_ERR,
                "Vhwa_m2mNfInit is not called\n");
            status  = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */

        nfCreatePrms = (Vhwa_M2mNfCreatePrms *)createArgs;
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Take the instance semaphore */
        (void)VhwaAl_SemaphoreP_pend(instObj->lock, SYSTEMP_WAIT_FOREVER);

        /* Allocate Handle Object */
        hObj = Vhwa_nfAllocHdlObj(instObj);

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != hObj)
        /* LDRA_JUSTIFY_END */
        {
            if (0U == instObj->openCnt)
            {
                /* On the first open, enable interrupts in HTS and
                 * start all UTC channels.
                 * Even if all UTC channels are enabled, only channels enabled
                 * in HTS will be used for the transfer */

                /* Check if SL2 is allocated, Allocate the SL2 for default
                   if not allocated */
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameter is expected to be pre-validated in the imaging layer before reaching vhwa.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((uint32_t)UFALSE == instObj->isSl2AllocDone)
                {
                    GT_0trace(VhwaNfTrace, GT_DEBUG,
                    "Vhwa_m2mNfAllocSl2 is not called, allocating for default\n");

                    Vhwa_M2mNfSl2AllocPrmsInit(&sl2AllocPrms);
                    status = Vhwa_m2mNfAllocSl2(&sl2AllocPrms);
                }
                /* LDRA_JUSTIFY_END */

                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This branch statement checks for errors accumulated in previous steps.
                Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (FVID2_SOK == status)
                {
                    /* Start UDMA Channels on the first handle Create */
                    status = Vhwa_m2mNfStartCh(instObj);
                }
                /* LDRA_JUSTIFY_END */

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This branch statement checks for errors accumulated in previous steps.
                Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (FVID2_SOK == status)
                /* LDRA_JUSTIFY_END */
                {
                    /* Enable Pipeline interrupt in INTD */
                    Vhwa_enableHtsIntr(socInfo->vpacIntdRegs,
                        instObj->vhwaIrqNum, instObj->pipeline);
                }
            }

            /* Create doneQ, Done Q is specific to handle so that
             * FVID2_Dequeue returns correct request for that handle. */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This branch statement checks for errors accumulated in previous steps.
            Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK == status)
            /* LDRA_JUSTIFY_END */
            {
                status = Fvid2Utils_constructQ(&hObj->doneQObj);
            }
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This branch statement checks for errors accumulated in previous steps.
            Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK == status)
            /* LDRA_JUSTIFY_END */
            {
                hObj->doneQ = &hObj->doneQObj;

                /* Allocate Descriptor, Ring memories */
                status = Vhwa_m2mNfAllocUdmaMem(hObj);
            }

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This branch statement checks for errors accumulated in previous steps.
            Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK == status)
            /* LDRA_JUSTIFY_END */
            {
                Fvid2Utils_memcpy(&hObj->createPrms, nfCreatePrms,
                    sizeof(Vhwa_M2mNfCreatePrms));

                Fvid2Utils_memcpy(&hObj->cbPrms, cbPrms,
                    sizeof (hObj->cbPrms));

                Vhwa_nfInitHandleObj(instObj, hObj);

                instObj->openCnt ++;
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This branch statement checks for errors accumulated in previous steps.
            Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
            Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
             However, due to the stated rationale, this is not tested.
            <justification end> */
            else
            {
                /* Some error, so free up the handle object */
                Vhwa_nfFreeHandleObj(hObj);
            }
            /* LDRA_JUSTIFY_END */
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The number of handles is expected to be pre-validated in the imaging layer before reaching vhwa.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            status = FVID2_EALLOC;
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps.
        Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            handle = (Fdrv_Handle)hObj;
        }

        (void)VhwaAl_SemaphoreP_post(instObj->lock);
    }

    return (handle);
}

Int32 Vhwa_m2mNfDelete(Fdrv_Handle handle, Ptr deleteArgs)
{
    int32_t                 status = FVID2_SOK;
    Vhwa_M2mNfInstObj     *instObj = NULL;
    Vhwa_M2mNfHandleObj   *hObj = NULL;
    Nf_SocInfo            *socInfo = NULL;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated in the imaging layer before reaching vhwa.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != handle)
    /* LDRA_JUSTIFY_END */
    {
        instObj = &gM2mNfInstObj;
        hObj    = (Vhwa_M2mNfHandleObj *)handle;
        socInfo = &instObj->socInfo;

        /* Take the instance semaphore */
        (void)VhwaAl_SemaphoreP_pend(instObj->lock, SYSTEMP_WAIT_FOREVER);

        /* Check if all handles are already closed */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (0U == instObj->openCnt)
        {
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps.
        Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((FVID2_SOK == status) &&
            (0U != hObj->numPendReq))
        {
            /* Clear All pending requests first */
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps.
        Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            Vhwa_nfFreeHandleObj(hObj);

            instObj->openCnt --;

            /* Delete the Done queue */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the caller before the control reaches here.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL != hObj->doneQ)
            {
                Fvid2Utils_destructQ(hObj->doneQ);
                hObj->doneQ = NULL;
            }
            /* LDRA_JUSTIFY_END */

            /* for the last close, stop all channels and
             * disable HTS interrupt in INTD */
            if (0U == instObj->openCnt)
            {
                /* Disable HTS Interrupt */
                Vhwa_disableHtsIntr(socInfo->vpacIntdRegs,
                            instObj->vhwaIrqNum, instObj->pipeline);

                /* No active objects and last handle object */
                instObj->actQObj = NULL;
                instObj->lastHndlObj = NULL;
            }
        }

        (void)VhwaAl_SemaphoreP_post(instObj->lock);
    }

    return (status);
}

Int32 Vhwa_m2mNfControl(Fdrv_Handle handle, UInt32 cmd, Ptr cmdArgs,
    Ptr cmdStatusArgs)
{
    int32_t                 status = FVID2_SOK;
    Vhwa_M2mNfInstObj     *instObj = NULL;
    Vhwa_M2mNfHandleObj   *hObj = NULL;
    Vhwa_M2mNfConfig      *nfCfg = NULL;
    Nf_ErrEventParams     *eePrms = NULL;
    Vhwa_HtsLimiter       *bwLimit = NULL;
    Nf_WgtTableConfig     *filterPrms = NULL;
    Nf_WdTimerErrEventParams *wdTimerEePrms = NULL;
    uint32_t                cookie;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated in the imaging layer before reaching vhwa.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL == handle)
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        instObj = &gM2mNfInstObj;
        hObj = (Vhwa_M2mNfHandleObj *)handle;
    }
    /* LDRA_JUSTIFY_START
    <metric start>branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Take the instance semaphore */
        (void)VhwaAl_SemaphoreP_pend(instObj->lock, SYSTEMP_WAIT_FOREVER);

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameter is expected to be pre-validated in the imaging layer before reaching vhwa.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        switch (cmd)
        {
        /* LDRA_JUSTIFY_END */
            /* Set NF Parameters */
            case IOCTL_VHWA_M2M_NF_SET_PARAMS:
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, then our code base is expected to prevent undefined behaviour due to dereferencing a NULL pointer.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != cmdArgs)
                /* LDRA_JUSTIFY_END */
                {
                    nfCfg = (Vhwa_M2mNfConfig *)cmdArgs;
                    status = Vhwa_nfSetParams(instObj, hObj, nfCfg);
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaNfTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_VHWA_M2M_NF_SET_PARAMS !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            /* Set NF Parameters */
            case IOCTL_VHWA_M2M_NF_SET_FILTER_COEFF:
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, then our code base is expected to prevent undefined behaviour due to dereferencing a NULL pointer.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != cmdArgs)
                /* LDRA_JUSTIFY_END */
                {
                    filterPrms = (Nf_WgtTableConfig *)cmdArgs;
                    status = Vhwa_nfSetFilterTables(instObj, filterPrms);
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application. This behaviour is part of the application design. An error print statement can be added in a future release if required.
                    <justification end> */
                    /* Update golden memory if config register validation is enabled */

                    if ((FVID2_SOK == status) && ((uint32_t)UTRUE == hObj->enableConfigRegValidate))
                    /* LDRA_JUSTIFY_END */
                    {
                        VhwaVpacNfSocReadBack *goldenRegVal = hObj->configRegMemPrms.configGoldenRegPtr;
                        /* LDRA_JUSTIFY_START
                        <metric start> branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion. The parameters are pre-validated by the caller before the control reaches here.
                        Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer. However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if (NULL != goldenRegVal)
                        /* LDRA_JUSTIFY_END */
                        {
                            status = CSL_nfSetWgtTableConfig(&goldenRegVal->nfRegs, filterPrms);
                        }
                    }
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaNfTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_VHWA_M2M_NF_SET_FILTER_COEFF !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            /* Enable Register error events callback */
            case IOCTL_VHWA_M2M_NF_REGISTER_ERR_CB:
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, then our code base is expected to prevent undefined behaviour due to dereferencing a NULL pointer.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != cmdArgs)
                /* LDRA_JUSTIFY_END */
                {
                    eePrms = (Nf_ErrEventParams *)cmdArgs;
                    status = Vhwa_nfSetEeParams(instObj, hObj, eePrms);
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaNfTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_VHWA_M2M_NF_REGISTER_ERR_CB !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            /* SET HTS Limiter config */
            case IOCTL_VHWA_M2M_NF_SET_HTS_LIMIT:
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, then our code base is expected to prevent undefined behaviour due to dereferencing a NULL pointer.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != cmdArgs)
                /* LDRA_JUSTIFY_END */
                {
                    bwLimit = (Vhwa_HtsLimiter *)cmdArgs;
                    Vhwa_nfSetBwLimit(hObj, bwLimit);
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaNfTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_VHWA_M2M_NF_SET_HTS_LIMIT !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            /* Get PSA Signature */
            case IOCTL_VHWA_M2M_NF_GET_PSA_SIGN:
            {
                Vhwa_M2mNfPsaSign *psa = NULL;

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, then our code base is expected to prevent undefined behaviour due to dereferencing a NULL pointer.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != cmdArgs)
                /* LDRA_JUSTIFY_END */
                {
                    psa = (Vhwa_M2mNfPsaSign *)cmdArgs;
                    *psa = hObj->psa;
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaNfTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_VHWA_M2M_NF_GET_PSA_SIGN !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            /* Get IP Performance */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The test framework and test apps cannot reach this portion
            The IOCTL is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit:
            None;Unused feature, cannot be enabled, control cannot reach here.
            <justification end> */
            case IOCTL_VHWA_M2M_NF_GET_PERFORMANCE:
            {
                Vhwa_M2mNfPerf *perf = NULL;

                if (NULL != cmdArgs)
                {
                    perf = (Vhwa_M2mNfPerf *)cmdArgs;
                    perf->perf[0u] = (uint32_t)hObj->perfNum[0u];
                    perf->perf[1u] = (uint32_t)hObj->perfNum[1u];
                }
                else
                {
                    GT_0trace(VhwaNfTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_VHWA_M2M_NF_GET_PERFORMANCE !!\n");
                    status = FVID2_EBADARGS;
                }
                break;
            }

            case IOCTL_VHWA_M2M_NF_SYNC_START:
            {
                if (NULL != hObj->createPrms.getTimeStamp)
                {
                    hObj->perfNum[0u] = hObj->createPrms.getTimeStamp();
                }

                status = CSL_htsPipelineStart(instObj->socInfo.htsRegs, &hObj->htsCfg[0]);
                break;
            }
            /* LDRA_JUSTIFY_END */
            /* Enable Register error events callback */
            case VHWA_M2M_IOCTL_NF_REGISTER_WDTIMER_ERR_CB:
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, then our code base is expected to prevent undefined behaviour due to dereferencing a NULL pointer.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != cmdArgs)
                /* LDRA_JUSTIFY_END */
                {
                    wdTimerEePrms = (Nf_WdTimerErrEventParams *)cmdArgs;
                    status = Vhwa_nfSetWdTimerEeParams(instObj, hObj, wdTimerEePrms);
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaNfTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_NF_REGISTER_WDTIMER_ERR_CB !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case VHWA_M2M_IOCTL_NF_ENABLE_RECONFIG_REINIT_REG:
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, then our code base is expected to prevent undefined behaviour due to dereferencing a NULL pointer.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((NULL != handle) && (NULL != cmdArgs))
                /* LDRA_JUSTIFY_END */
                {
                    uint32_t *enableFlag = (uint32_t *)cmdArgs;
                    if ((uint32_t)1U == *enableFlag)
                    {
                        hObj->enableReconfigReinitReg = 1U;
                    }
                    else
                    {
                        hObj->enableReconfigReinitReg = 0U;
                    }
                    status = FVID2_SOK;
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaNfTrace, GT_ERR,
                        "handle/cmdArgs cannot be NULL for VHWA_M2M_IOCTL_NF_ENABLE_RECONFIG_REINIT_REG !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case VHWA_M2M_IOCTL_NF_ENABLE_STATUS_REG_VALIDATE:
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The parameters are pre-validated by the caller before the control reaches here.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((NULL != handle) && (NULL != cmdArgs))
                /* LDRA_JUSTIFY_END */
                {
                    uint32_t *enableFlag = (uint32_t *)cmdArgs;
                    cookie = HwiP_disable();
                    if ((uint32_t)1U == *enableFlag)
                    {
                        hObj->enableStatusRegValidate = (uint32_t)UTRUE;
                    }
                    else
                    {
                        hObj->enableStatusRegValidate = (uint32_t)UFALSE;
                    }
                    HwiP_restore(cookie);
                    status = FVID2_SOK;
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case VHWA_M2M_IOCTL_NF_VALIDATE_REG:
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The parameters are pre-validated by the caller before the control reaches here.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != handle)
                /* LDRA_JUSTIFY_END */
                {
                    if (UTRUE == hObj->enableStatusRegValidate)
                    {
                        status = Vhwa_m2mNfStatusRegValidate(&hObj->nfStatusRegs, &instObj->socInfo);
                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                        Effect on this unit: If the control reaches here, our code base is expected to print the error. However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if (FVID2_SOK != status)
                        {
                            GT_0trace(VhwaNfTrace, GT_ERR, "VPAC-10: StatusRegValidate failed!\n");
                        }
                        /* LDRA_JUSTIFY_END */
                    }
                    if (UTRUE == hObj->enableConfigRegValidate)
                    {
                        /* Readback config registers */
                        status = Vhwa_m2mNfConfigRegReadback(hObj, instObj);
                    }

                    /* Release the hardware lock after the configReg readback or statusReg */
                    if ((UTRUE == hObj->enableStatusRegValidate) ||
                        (UTRUE == hObj->enableConfigRegValidate))
                    {
                        Vhwa_commonHwaLockRelease(instObj->hwaLockIdx);
                    }

                    if (UTRUE == hObj->enableStatusRegValidate)
                    {
                        /* Reset the statusReg validate flag */
                        cookie = HwiP_disable();
                        hObj->enableStatusRegValidate = (uint32_t)UFALSE;
                        HwiP_restore(cookie);
                    }

                    if ((UTRUE == hObj->enableConfigRegValidate) && (FVID2_SOK == status))
                    {
                        /* Callback with goldenReg and ReadBackReg pointer to application for memory comparison */
                        /* LDRA_JUSTIFY_START
                        <metric start> branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion. The parameters are pre-validated by the caller before the control reaches here.
                        Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer. However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if((NULL != hObj->configRegMemPrms.cbFxn) &&
                            (NULL != hObj->configRegMemPrms.configGoldenRegPtr) &&
                            (NULL != hObj->configRegMemPrms.configRegReadbackPtr))
                        /* LDRA_JUSTIFY_END */
                        {
                            Vhwa_M2mNfConfigRegMemParams configRegMemPtrs;
                            configRegMemPtrs.golden_ref_ptr = hObj->configRegMemPrms.configGoldenRegPtr;
                            configRegMemPtrs.readback_ptr = hObj->configRegMemPrms.configRegReadbackPtr;
                            configRegMemPtrs.readback_mem_size = sizeof(VhwaVpacNfSocReadBack);
                            configRegMemPtrs.appData = (void *)hObj->configRegMemPrms.appData;

                            status = hObj->configRegMemPrms.cbFxn((Fvid2_Handle)hObj, &configRegMemPtrs);
                        }
                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                        <justification end> */
                        else
                        {
                            GT_0trace(VhwaNfTrace, GT_ERR, "ConfigRegMemParams are NULL!\n");
                            status = FVID2_EFAIL;
                        }
                        /* LDRA_JUSTIFY_END */

                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                        <justification end> */
                        /* Get status from callback for memory comparison */
                        if (FVID2_SOK != status)
                        {
                            GT_0trace(VhwaNfTrace, GT_ERR, "VPAC-7: ConfigRegReadbackCallback Memory Comparison failed!\n");
                        }
                        /* LDRA_JUSTIFY_END */

                        /* Disable Interrupt since the below flag is accessed in the ISR and reset the configReg validate flag */
                        cookie = HwiP_disable();
                        hObj->enableConfigRegValidate = (uint32_t)UFALSE;
                        HwiP_restore(cookie);
                    }
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaNfTrace, GT_ERR,
                        "handle cannot be NULL for VHWA_M2M_IOCTL_NF_VALIDATE_REG !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case VHWA_M2M_IOCTL_NF_ENABLE_CONFIG_REG_READBACK:
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The parameters are pre-validated by the caller before the control reaches here.
                Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer. However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((NULL != handle) && (NULL != cmdArgs))
                /* LDRA_JUSTIFY_END */
                {
                    uint32_t *enableFlag = (uint32_t *)cmdArgs;
                    cookie = HwiP_disable();
                    if ((uint32_t)1U == *enableFlag)
                    {
                        hObj->enableConfigRegValidate = (uint32_t)UTRUE;
                    }
                    else
                    {
                        hObj->enableConfigRegValidate = (uint32_t)UFALSE;
                    }
                    HwiP_restore(cookie);
                    status = FVID2_SOK;
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaNfTrace, GT_ERR,
                        "handle/cmdArgs cannot be NULL for VHWA_M2M_IOCTL_NF_ENABLE_CONFIG_REG_READBACK !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case VHWA_M2M_IOCTL_NF_GET_READBACK_SIZE:
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The parameter is expected to be pre-validated in the imaging layer before reaching vhwa.
                Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer. However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((NULL != handle) && (NULL != cmdArgs))
                /* LDRA_JUSTIFY_END */
                {
                    uint32_t *readbackSize = (uint32_t *)cmdArgs;
                    *readbackSize = (uint32_t)sizeof(VhwaVpacNfSocReadBack);
                }
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaNfTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_NF_GET_READBACK_SIZE !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case VHWA_M2M_IOCTL_NF_SET_CONFIG_REG_PTRS:
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The parameter is expected to be pre-validated in the imaging layer before reaching vhwa.
                Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer. However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((NULL != handle) && (NULL != cmdArgs))
                /* LDRA_JUSTIFY_END */
                {
                    Vhwa_M2mNfConfigRegMemParams *reg_ptrs = (Vhwa_M2mNfConfigRegMemParams *)cmdArgs;
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if ((reg_ptrs->readback_ptr == NULL) ||
                        (reg_ptrs->golden_ref_ptr == NULL) ||
                        (reg_ptrs->cbFxn == NULL))
                    {
                        GT_0trace(VhwaNfTrace, GT_ERR,
                            "readback_ptr, golden_ref_ptr, cbFxn cannot be NULL for VHWA_M2M_IOCTL_NF_SET_CONFIG_REG_PTRS !!\n");
                        status = FVID2_EBADARGS;
                    }
                    /* LDRA_JUSTIFY_END */
                    else
                    {
                        hObj->configRegMemPrms.configRegReadbackPtr = reg_ptrs->readback_ptr;
                        hObj->configRegMemPrms.configGoldenRegPtr = reg_ptrs->golden_ref_ptr;
                        hObj->configRegMemPrms.cbFxn = reg_ptrs->cbFxn;
                        hObj->configRegMemPrms.appData = reg_ptrs->appData;

                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if(FVID2_SOK != status)
                        {
                            GT_0trace(VhwaNfTrace, GT_ERR,
                                "Failed to set default goldenReg mem values for VHWA_M2M_IOCTL_NF_SET_CONFIG_REG_PTRS !!\n");
                        }
                        /* LDRA_JUSTIFY_END */
                    }
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaNfTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_NF_SET_CONFIG_REG_PTRS !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            /* Default Case */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            default:
            {
                status = FVID2_EUNSUPPORTED_CMD;
                break;
            }
            /* LDRA_JUSTIFY_END */
        }

        (void)VhwaAl_SemaphoreP_post(instObj->lock);
    }

    return (status);
}

Int32 Vhwa_m2mNfProcessReq(Fdrv_Handle handle, Fvid2_FrameList *inFrmList,
    Fvid2_FrameList *outFrmList, uint32_t timeout)
{
    int32_t             status = FVID2_SOK;
    uint32_t            semTimeout;
    Vhwa_M2mNfInstObj   *instObj = NULL;
    Vhwa_M2mNfHandleObj *hObj = NULL;
    Vhwa_M2mNfQueueObj  *qObj = NULL;
    Vhwa_M2mNfConfig    *nfCfg = NULL;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == handle) || (NULL == inFrmList) || (NULL == outFrmList))
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        instObj = &gM2mNfInstObj;
        hObj = (Vhwa_M2mNfHandleObj *)handle;
        nfCfg = &hObj->nfCfg[0];

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        /* Check if IP is in stalled state due to error interrupt or watchdog timeout */
        if ((uint32_t)UTRUE == instObj->isStalled)
        {
            status = FVID2_EFAIL;
            GT_0trace(VhwaNfTrace, GT_ERR,
                "Process request rejected: IP is in stalled state due to previous error\n");
        }
        /* LDRA_JUSTIFY_END */
    }

    /* Check for null pointers */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Input Buffer Addresses cannot be null */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((NULL == inFrmList->frames[0U]) ||
            (0U == inFrmList->frames[0u]->addr[0U]) ||
            (NULL == outFrmList->frames[0U]) ||
            (0U == outFrmList->frames[0u]->addr[0U]))
        {
            status = FVID2_EBADARGS;
        }
        /* LDRA_JUSTIFY_END */
        else
        {
            /* For YUV420 input, chroma buffer cannot be null */
            if ((FVID2_DF_YUV420SP_UV == nfCfg->inFmt.dataFormat) ||
                (FVID2_DF_YUV420SP_VU == nfCfg->inFmt.dataFormat))
            {
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (0U == inFrmList->frames[0u]->addr[1U])
                {
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
            }
            if ((FVID2_DF_YUV420SP_UV == nfCfg->outFmt.dataFormat) ||
                (FVID2_DF_YUV420SP_VU == nfCfg->outFmt.dataFormat))
            {
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (0U == outFrmList->frames[0u]->addr[1U])
                {
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
            }
        }
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /** Steps
         *  1, Get a queue object from the free queue.
         *  2, Copy frame list into queue object and set
         *     buffer addresses in the TR.
         *  3, Check to see if the curActive Handle is null or not
         *  4, If it is not null, move the queue object to reqQ
         *  5, If it is null, ie HW is free, check to see
         *     if prevHandle is same as this handle.
         *  6, If they are same ,
         *     submit the TR without any configuration
         *  7, If they are not same,
         *     set the config and then submit the TR.
         *
         */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameter is expected to be pre-validated in the imaging layer before reaching vhwa.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_TIMEOUT_FOREVER == timeout)
        /* LDRA_JUSTIFY_END */
        {
            semTimeout = SYSTEMP_WAIT_FOREVER;
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameter is expected to be pre-validated in the imaging layer before reaching vhwa.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. However, due to the stated rationale, this is not tested
        <justification end> */
        else if (FVID2_TIMEOUT_NONE == timeout)
        {
            semTimeout = SYSTEMP_NO_WAIT;
        }
        else
        {
            semTimeout = timeout;
        }
        /* LDRA_JUSTIFY_END */

        /* This request is going to be submitted, so increment
         * number of pending request counter,
         * */
        hObj->numPendReq ++;

        /* Take the instance semaphore, so that no other
         * handle can submit request from the task context. */
        status = Vhwa_commonHwaLockAquire(instObj->hwaLockIdx, semTimeout);

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps.
        Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            /* Get a queue object from the free queue,
             * No need to protect from ISR as it is not accessed from ISR */
            qObj = (Vhwa_M2mNfQueueObj *) Fvid2Utils_dequeue(instObj->freeQ);

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL == qObj)
            {
                hObj->numPendReq --;

                GT_0trace(VhwaNfTrace, GT_ERR,
                    "Failed to Free Queue Object!!\n");
                status = FVID2_EALLOC;
            }
            /* LDRA_JUSTIFY_END */
            else
            {
                qObj->hObj = hObj;
                /* Copy the application's process list to request objects lists */
                Fvid2_copyFrameList(&qObj->inFrmList, inFrmList);
                Fvid2_copyFrameList(&qObj->outFrmList, outFrmList);
            }
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            /* Failed, so reducing number of pending request */
            hObj->numPendReq --;
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps.
        Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            Vhwa_m2mNfSetTRAddress(qObj->hObj, &qObj->inFrmList,
                                    &qObj->outFrmList);

            /* HW is free, submit request to the hardware */
            /* If previous handle and current handles are same, */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (instObj->lastHndlObj == qObj->hObj)
            {
                /** Last handle was same as new handle,
                 *  so require to recofigure all HW IPs */
                status = Vhwa_m2mNfSetFrameSize(instObj, qObj, 0U);
            }
            /* LDRA_JUSTIFY_END */
            else
            {
                /** Last handle was not same as new handle,
                 *  so require to recofigure all HW IPs */
                status = Vhwa_m2mNfSetConfigInHW(instObj, qObj, 0U);
            }

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This branch statement checks for errors accumulated in previous steps.
            Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
            <justification end> */
            if(FVID2_SOK == status)
            /* LDRA_JUSTIFY_END */
            {
                status = Vhwa_m2mNfSetIntrInHW(instObj->vhwaIrqNum,
                            instObj->socInfo.vpacIntdRegs, qObj->hObj);
            }

            /* Invoke the reconfig-MMR if enableReconfigReinitReg enabled for the current handle */
            if (UTRUE == hObj->enableReconfigReinitReg)
            {
                status = Vhwa_m2mNfReconfigReinitReg(instObj, hObj, qObj);
                hObj->enableReconfigReinitReg = (uint32_t)UFALSE;
            }

            /* Update the NfStatusRegisterGroup with valid status register values for dynamic registers */
            if ((uint32_t)UTRUE == hObj->enableStatusRegValidate)
            {
                status = vhwaM2mNfUpdateStatusRegGroup(hObj);
            }

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK == status)
            /* LDRA_JUSTIFY_END */
            {
                /** Set the addresses, Submit the TR
                 *  Start the pipeline */
                status = Vhwa_m2mNfSubmitRequest(instObj, qObj, 0U);
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            else
            {
                hObj->numPendReq --;
            }
            /* LDRA_JUSTIFY_END */
        }
    }

    return (status);
}

/** \brief Typedef for FVID2 get processed frames function pointer. */
Int32 Vhwa_m2mNfGetProcessReq(Fdrv_Handle handle,
    Fvid2_FrameList *inFrmList, Fvid2_FrameList *outFrmList,
    UInt32 timeout)
{
    int32_t                status = FVID2_SOK;
    uint32_t               cookie, cnt;
    Vhwa_M2mNfInstObj    *instObj = NULL;
    Vhwa_M2mNfHandleObj  *hObj = NULL;
    Vhwa_M2mNfQueueObj   *qObj = NULL;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL == handle)
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        instObj = &gM2mNfInstObj;
        hObj = (Vhwa_M2mNfHandleObj *)handle;
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Take the instance semaphore, so that no other
         * handle can submit request from the task context. */
        (void)VhwaAl_SemaphoreP_pend(instObj->lock, SYSTEMP_WAIT_FOREVER);

        /* Disable interrupts before accessing queue */
        cookie = HwiP_disable();

        /* Dequeue the completed request from done queue */
        qObj = (Vhwa_M2mNfQueueObj *) Fvid2Utils_dequeue(hObj->doneQ);

        /* Restore interrupts after updating node list */
        HwiP_restore(cookie);

        /* No buffers in the output queue */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL == qObj)
        {
            /* Check if requests are pending with driver */
            if (0U == hObj->numPendReq)
            {
                /* Nothing is queued */
                GT_0trace(VhwaNfTrace, GT_DEBUG,
                    "Nothing to dequeue. No request pending with driver!!\r\n");
                status = FVID2_ENO_MORE_BUFFERS;
            }
            else
            {
                /* If no request have completed, return try again */
                GT_0trace(VhwaNfTrace, GT_DEBUG,
                    "Nothing to dequeue. Try again!!\r\n");
                status = FVID2_EAGAIN;
            }
        }
        else /* OutQ has buffer to be returned */
        /* LDRA_JUSTIFY_END */
        {
            /* Copy the driver's process list to application's process list */
            Fvid2_copyFrameList(inFrmList, &qObj->inFrmList);
            Fvid2_copyFrameList(outFrmList, &qObj->outFrmList);

            for (cnt = 0; cnt < hObj->numIter; cnt++)
            {
                /* Dequeue from Rings */
                status = Vhwa_m2mNfPopRings(instObj, hObj, cnt);
            }

            /* Return back the queue object to the free queue,
             * No need to protect from ISR, as it is not accessed in ISR */
            Fvid2Utils_queue(instObj->freeQ, &qObj->qElem, qObj);

            /* Decrement the pending request count. */
            /* LDRA_JUSTIFY_START
            <metric start>  branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (0U != hObj->numPendReq)
            /* LDRA_JUSTIFY_END */
            {
                hObj->numPendReq--;
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            else
            {
                GT_0trace(VhwaNfTrace, GT_DEBUG,
                    "Done queue of the handle is not empty when there are is no pending request on the handle!!\r\n");
                status = FVID2_EFAIL;
            }
            /* LDRA_JUSTIFY_END */
        }
        /* New request can now be submitted to the NF IP for non-readBack of StatusRegisters/ConfigurationRegisters enabled frames */
        if ((UFALSE == hObj->enableStatusRegValidate) &&
            (UFALSE == hObj->enableConfigRegValidate))
        {
            Vhwa_commonHwaLockRelease(instObj->hwaLockIdx);
        }

        (void)VhwaAl_SemaphoreP_post(instObj->lock);
    }

    return (status);
}


int32_t Vhwa_m2mNfAllocSl2(const Vhwa_M2mNfSl2AllocPrms *sl2AllocPrms)
{
    int32_t retVal = FVID2_SOK;
    uint32_t sl2MemReq = 0;
    uint32_t inPitchSl2,outPitchSl2;
    uint64_t sl2Addr;
    Vhwa_M2mNfInstObj *instObj = NULL;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(NULL == sl2AllocPrms)
    {
        GT_0trace(VhwaNfTrace, GT_ERR, "Bad Arguments\n");
        retVal = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(FVID2_SOK == retVal)
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if((sl2AllocPrms->inBuffDepth < VHWA_M2M_NF_MIN_RD_BUFFER_DEPTH) ||
           (sl2AllocPrms->outBuffDepth < VHWA_M2M_NF_MIN_WR_BUFFER_DEPTH))
        {
            GT_0trace(VhwaNfTrace, GT_ERR, "Bad Arguments\n");
            retVal = FVID2_EBADARGS;
        }
        /* LDRA_JUSTIFY_END */
        instObj = &gM2mNfInstObj;
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(FVID2_SOK == retVal)
    /* LDRA_JUSTIFY_END */
    {
        /* Calculate Sl2 buffer size required for input */
        inPitchSl2 = Vhwa_calcHorzSizeInBytes(sl2AllocPrms->maxImgWidth,
                                                sl2AllocPrms->inCcsf);
        inPitchSl2 = Vhwa_calcSl2Pitch(inPitchSl2);

        sl2MemReq += inPitchSl2 * sl2AllocPrms->inBuffDepth;

        /* Calculate Sl2 buffer size required for output */
        outPitchSl2 = Vhwa_calcHorzSizeInBytes(sl2AllocPrms->maxImgWidth,
                                                sl2AllocPrms->outCcsf);
        outPitchSl2 = Vhwa_calcSl2Pitch(outPitchSl2);

        sl2MemReq += outPitchSl2 * sl2AllocPrms->outBuffDepth;

        /* Apply 16KB alignment for size and address if requested for firewall support */
        if ((uint32_t)TRUE == sl2AllocPrms->enable16KBAlignment)
        {
            sl2MemReq = ((sl2MemReq + 0x3FFFu) & ~0x3FFFu);
            sl2Addr = Vhwa_allocateSl2Aligned(sl2MemReq, VHWA_SL2_INST_VPAC, 0x4000u);
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application. This behaviour is part of the application design. An error print statement can be added in a future release if required.
        <justification end> */
        else
        {
            sl2Addr = Vhwa_allocateSl2(sl2MemReq, VHWA_SL2_INST_VPAC);
        }
        /* LDRA_JUSTIFY_END */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(0u == sl2Addr)
        {
            GT_0trace(VhwaNfTrace, GT_ERR,
                "SL2 aligned allocation failed (check alignment=0x4000 is power of 2 and sufficient SL2 memory available)\n");
            retVal = FVID2_EALLOC;
        }
        /* LDRA_JUSTIFY_END */
        else
        {
            instObj->sl2Prms.sl2StartAddr = sl2Addr;
            instObj->sl2Prms.sl2MemSize = sl2MemReq;
        }
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(FVID2_SOK == retVal)
    /* LDRA_JUSTIFY_END */
    {
        /* Calculate Sl2 buffer size required for input */
        instObj->sl2Prms.sl2Addr[0] = sl2Addr;
        instObj->sl2Prms.sl2NumLines[0] = sl2AllocPrms->inBuffDepth;

        sl2Addr += (uint64_t)inPitchSl2 * (uint64_t)sl2AllocPrms->inBuffDepth;

        instObj->sl2Prms.sl2Addr[1] = sl2Addr;
        instObj->sl2Prms.sl2NumLines[1] = sl2AllocPrms->outBuffDepth;

        instObj->isSl2AllocDone = UTRUE;
    }

    return retVal;
}


void Vhwa_m2mNfFreeSl2(void)
{
    Vhwa_M2mNfInstObj *instObj = &gM2mNfInstObj;

    Vhwa_FreeSl2(VHWA_SL2_INST_VPAC);

    instObj->isSl2AllocDone = UFALSE;
}

/**
 * \brief   Returns handle object for the requested handle count.
 *
 * \param   cnt              count.
 *
 * \return  reference to the handle object.
 *
 **/
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
<justification end> */
Vhwa_M2mNfHandleObj *Vhwa_m2mNfGetHandleObj(uint32_t cnt)
{
    return &gM2mNfHandleObj[cnt];
}
/* LDRA_JUSTIFY_END */

int32_t Vhwa_m2mNfReInit(void)
{
    int32_t                status = FVID2_SOK;
    Vhwa_M2mNfInstObj     *instObj = NULL;

    /** No lock is required, as reinit is called only when HW is not accessed by any other
     *  handle, so it is ok to access the members here. */
    instObj = &gM2mNfInstObj;

    /** If isregistered is set, then Vhwa_m2mDofUdmaInit is called and UDMA
     * channels are initialized, so reinit is required. */
    /* LDRA_JUSTIFY_START_DISABLED
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations. However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((uint32_t)UTRUE == instObj->isRegistered)
    /* LDRA_JUSTIFY_END_DISABLED */
    {
        status = Vhwa_m2mNfUdmaDeInit(instObj);

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if(FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            /* Need to unregister ISR, so it can be re-registered in next step */
            Vhwa_m2mNfUnregisterIsr(instObj);
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if(FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            /* Now reinitialized channels */
            status = Vhwa_m2mNfUdmaInit(instObj, &instObj->initPrms);
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if(FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            /* INTD module also requires to be reinitialized */
            status = Vhwa_m2mNfRegisterIsr(instObj);
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            /* Enable LDC at VPAC Top*/
            CSL_vpacEnableModule(instObj->socInfo.vpacCntlRegs,
                VPAC_MODULE_NF, (uint32_t)UTRUE);
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            /* Now start all channels */
            status = Vhwa_m2mNfStartCh(instObj);
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            /* Enable Pipeline interrupt in INTD */
            Vhwa_enableHtsIntr(instObj->socInfo.vpacIntdRegs,
                instObj->vhwaIrqNum, instObj->pipeline);
        }

        /* Settting last handle to false/null, so that reconfiguration is done */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            instObj->lastHndlObj = NULL;
        }
    }

    return (status);
}

/* ========================================================================== */
/*                           Local Functions                                  */
/* ========================================================================== */
static Vhwa_M2mNfHandleObj *Vhwa_nfAllocHdlObj(const Vhwa_M2mNfInstObj *instObj)
{
    uint32_t cnt;
    Vhwa_M2mNfHandleObj *hObj = NULL;

    /* Check for Null pointer */
    GT_assert(VhwaNfTrace, (NULL != instObj));

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
    <justification end> */
    for (cnt = 0U; cnt < VHWA_M2M_NF_MAX_HANDLES; cnt ++)
    /* LDRA_JUSTIFY_END */
    {
        if ((uint32_t)UFALSE == gM2mNfHandleObj[cnt].isUsed)
        {
            /* Allocate Handle Object */
            hObj = &gM2mNfHandleObj[cnt];
            Fvid2Utils_memset(hObj, 0x0, sizeof(Vhwa_M2mNfHandleObj));
            gM2mNfHandleObj[cnt].isUsed = (uint32_t)UTRUE;
            gM2mNfHandleObj[cnt].enableReconfigReinitReg = (uint32_t)UFALSE;
            gM2mNfHandleObj[cnt].enableStatusRegValidate = (uint32_t)UFALSE;
            hObj->hIdx = cnt;
            break;
        }
    }

    return (hObj);
}

static int32_t Vhwa_nfCreateQueue(Vhwa_M2mNfInstObj *instObj)
{
    int32_t retVal = FVID2_SOK;
    uint32_t qCnt;
    Vhwa_M2mNfQueueObj *qObj;

    /* NULL pointer check */
    GT_assert(VhwaNfTrace, (NULL != instObj));

    instObj->actQObj = NULL;
    instObj->freeQ = NULL;

    /* Create Free queue */
    retVal = Fvid2Utils_constructQ(&instObj->freeQObj);
    GT_assert(VhwaNfTrace, (retVal == FVID2_SOK));

    instObj->freeQ = &instObj->freeQObj;


    /* Initialize and queue the allocate queue object to free Q */
    for(qCnt=0U; qCnt < VHWA_NF_QUEUE_LEN_PER_INST; qCnt++)
    {
        qObj = &instObj->nfQObj[qCnt];
        qObj->hObj = NULL;
        Fvid2Utils_queue(instObj->freeQ, &qObj->qElem, qObj);
    }

    return retVal;
}

static int32_t Vhwa_nfDeleteQueue(Vhwa_M2mNfInstObj *instObj)
{
    int32_t retVal = FVID2_SOK;
    Vhwa_M2mNfQueueObj *qObj;

    /* NULL pointer check */
    GT_assert(VhwaNfTrace, (NULL != instObj));

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The negative test framework and test apps cannot reach this portion.
    These parameters cannot be configured from test app or kernel directly and can only be overwritten by memory corruption. This is fail safe mechanism and cannot be removed
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(NULL != instObj->freeQ)
    /* LDRA_JUSTIFY_END */
    {
        /* Free-up all the queued free queue objects */
        do
        {
            qObj = (Vhwa_M2mNfQueueObj *) Fvid2Utils_dequeue(instObj->freeQ);
        } while (NULL != qObj);

        /* Delete the free Q */
        Fvid2Utils_destructQ(instObj->freeQ);
        instObj->freeQ = NULL;
    }

    return (retVal);
}

static void Vhwa_nfFreeHandleObj(Vhwa_M2mNfHandleObj *hObj)
{
    uint32_t cnt;

    /* Check for Null pointer */
    GT_assert(VhwaNfTrace, (NULL != hObj));

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
    <justification end> */
    for (cnt = 0U; cnt < VHWA_M2M_NF_MAX_HANDLES; cnt ++)
    /* LDRA_JUSTIFY_END */
    {
        /* Freeup allocated Handle Object */
        if (hObj == &gM2mNfHandleObj[cnt])
        {
            hObj->isUsed = (uint32_t)UFALSE;;
            break;
        }
    }

}

static void Vhwa_nfInitHandleObj(Vhwa_M2mNfInstObj *instObj,
    Vhwa_M2mNfHandleObj *hObj)
{
    /* Check for Null pointer */
    GT_assert(VhwaNfTrace, (NULL != hObj));
    GT_assert(VhwaNfTrace, (NULL != instObj));

    hObj->numIter = 1U;
    /* Initialize NF with Default configuration */
    Vhwa_M2mNfConfigInit(&hObj->nfCfg[0]);

    /* Set HTS Configuration for default NF configuration */
    Vhwa_nfSetHtsCfg(instObj, hObj);
    /* Set LSE Configuration for default NF configuration */
    Vhwa_nfSetLseCfg(instObj, hObj);
}

static int32_t Vhwa_nfSetFilterTables(Vhwa_M2mNfInstObj *instObj,
                                      const Nf_WgtTableConfig *pCoeffCfg)
{
    int32_t retVal = FVID2_SOK;

    /* Check for Null pointer */
    GT_assert(VhwaNfTrace, (NULL != pCoeffCfg));

    retVal = CSL_nfSetWgtTableConfig(instObj->socInfo.nfRegs, pCoeffCfg);

    return (retVal);
}

static int32_t Vhwa_nfSetParams(Vhwa_M2mNfInstObj *instObj,
    Vhwa_M2mNfHandleObj *hObj, Vhwa_M2mNfConfig *nfPrms)
{
    int32_t retVal = FVID2_SOK;
    uint32_t cnt;

    /* Check for Null pointer */
    GT_assert(VhwaNfTrace, (NULL != hObj));
    GT_assert(VhwaNfTrace, (NULL != instObj));
    GT_assert(VhwaNfTrace, (NULL != nfPrms));

    retVal = Vhwa_nfCheckNfCfg(instObj, nfPrms);

    if (FVID2_SOK == retVal)
    {
        /* Set the CSL parameters */
        Vhwa_nfSetCfgParams(hObj, nfPrms);

        /* Initialize HTS configuration based on the NF Config */
        Vhwa_nfSetHtsCfg(instObj, hObj);
        /* Initialize LSE configuration based on the NF Config */
        Vhwa_nfSetLseCfg(instObj, hObj);

        for(cnt = 0u; cnt < hObj->numIter; cnt++)
        {
            if(FVID2_SOK != retVal)
            {
                break;
            }
            /* Verify HTS Configuration */
            retVal = CSL_htsCheckThreadConfig(&hObj->htsCfg[cnt]);
        }
        for(cnt = 0u; cnt < hObj->numIter; cnt++)
        {
            if(FVID2_SOK != retVal)
            {
                break;
            }
            /* Verify LSE Configuration */
            retVal = CSL_lseCheckConfig(&hObj->lseCfg[cnt]);
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps.
        Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(FVID2_SOK == retVal)
        /* LDRA_JUSTIFY_END */
        {
            /* Setup TR Descriptor */
            Vhwa_m2mNfSetTrDesc(instObj, hObj);
        }
    }

    if(FVID2_SOK != retVal)
    {
        hObj->numIter = 1;
        /* Initialize NF with Default configuration */
        Vhwa_M2mNfConfigInit(&hObj->nfCfg[0]);
        /* Set HTS Configuration for default NF configuration */
        Vhwa_nfSetHtsCfg(instObj, hObj);
        /* Set LSE Configuration for default NF configuration */
        Vhwa_nfSetLseCfg(instObj, hObj);
    }

    return (retVal);
}

static int32_t Vhwa_nfSetEeParams(const Vhwa_M2mNfInstObj *instObj,
    Vhwa_M2mNfHandleObj *hObj, const Nf_ErrEventParams *eePrms)
{
    int32_t status = FVID2_SOK;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL != instObj) && (NULL != hObj) && (NULL != eePrms))
    /* LDRA_JUSTIFY_END */
    {
        /* Check if this handle is already being processed */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((NULL != instObj->actQObj) && (instObj->actQObj->hObj == hObj))
        {
            GT_0trace(VhwaNfTrace, GT_ERR,
                "Cannot change error events while handle is active !!\n");
            status = FVID2_EAGAIN;
        }
        else if (0U != (eePrms->errEvents & (~VHWA_NF_ALL_ERR_EVENTS)))
        {
            GT_1trace(VhwaNfTrace, GT_ERR,
                "Attempt to enable a non-existent error event: 0x%08x !!\n",
                (eePrms->errEvents & (~VHWA_NF_ALL_ERR_EVENTS)));
            status = FVID2_EINVALID_PARAMS;
        }
        /* LDRA_JUSTIFY_END */
        else
        {
            Fvid2Utils_memcpy(&hObj->eePrms, eePrms, sizeof(Nf_ErrEventParams));
            status = FVID2_SOK;
        }
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */

    return (status);
}

static int32_t Vhwa_nfSetWdTimerEeParams(const Vhwa_M2mNfInstObj *instObj,
    Vhwa_M2mNfHandleObj *hObj, const Nf_WdTimerErrEventParams *wdTimerEePrms)
{
    int32_t status = FVID2_SOK;
    uint32_t itrCnt;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL != instObj) && (NULL != hObj) && (NULL != wdTimerEePrms))
    /* LDRA_JUSTIFY_END */
    {
        /* Check if this handle is already being processed */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((NULL != instObj->actQObj) && (instObj->actQObj->hObj == hObj))
        {
            GT_0trace(VhwaNfTrace, GT_ERR,
                "Cannot change watchdog timer error events while handle is active !!\n");
            status = FVID2_EAGAIN;
        }
        else if (0U != (wdTimerEePrms->wdTimerErrEvents & (~VHWA_NF_WDTIMER_ERR)))
        {
            GT_1trace(VhwaNfTrace, GT_ERR,
                "Attempt to enable a non-existent watchdog timer error event: 0x%08x !!\n",
                (wdTimerEePrms->wdTimerErrEvents & (~VHWA_NF_WDTIMER_ERR)));
            status = FVID2_EINVALID_PARAMS;
        }
        /* LDRA_JUSTIFY_END */
        else
        {
            Fvid2Utils_memcpy(&hObj->wdTimerEePrms, wdTimerEePrms, sizeof(Nf_WdTimerErrEventParams));
            /* Update all HTS configurations (NF has multiple iterations) */
            /* Enable watchdog timer in HTS if WDTIMER_ERR event is registered */
            for (itrCnt = 0u; itrCnt < hObj->numIter; itrCnt++)
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The branch is being covered in VHWA_NEGATIVE_TEST build.
                Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application. This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                <justification end> */
                if (0U != (wdTimerEePrms->wdTimerErrEvents & VHWA_NF_WDTIMER_ERR))
                {
                    hObj->htsCfg[itrCnt].enableWdTimer = (uint32_t)UTRUE;
                    if (VHWA_WDTIMEOUT_64K_CYCLES == wdTimerEePrms->wdTimeoutCycles)
                    {
                        hObj->htsCfg[itrCnt].isWdTimer128KMode = (uint32_t)UFALSE;
                    }
                    else
                    {
                        hObj->htsCfg[itrCnt].isWdTimer128KMode = (uint32_t)UTRUE;
                    }
                }
                /* LDRA_JUSTIFY_END */
                else
                {
                    hObj->htsCfg[itrCnt].enableWdTimer = (uint32_t)UFALSE;
                }
            }
            status = FVID2_SOK;
        }
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */

    return status;
}

static int32_t Vhwa_m2mNfSetIntrInHW(uint32_t vhwaIrqNum,
                                volatile CSL_vpac_intd_cfgRegs  *intdRegs,
                                const Vhwa_M2mNfHandleObj *hObj)
{
    int32_t status = FVID2_SOK;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == intdRegs) || (NULL == hObj))
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        CSL_REG32_WR(&intdRegs->ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_NF_INTR_REG], VHWA_NF_ALL_ERR_EVENTS);
        CSL_REG32_WR(&intdRegs->ENABLE_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_NF_INTR_REG],
            hObj->eePrms.errEvents);

        CSL_REG32_WR(&intdRegs->ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_WDTIMER_INTR_REG], VHWA_NF_WDTIMER_ERR);
        CSL_REG32_WR(&intdRegs->ENABLE_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_WDTIMER_INTR_REG],
            hObj->wdTimerEePrms.wdTimerErrEvents);
    }

    return status;
}

static int32_t Vhwa_nfCheckNfCfg(Vhwa_M2mNfInstObj *instObj,
    Vhwa_M2mNfConfig *nfCfg)
{
    int32_t retVal = FVID2_SOK;
    uint32_t minPitch;
    Fvid2_Format *inFmt = NULL;
    Fvid2_Format *outFmt = NULL;

    /* Check for Null pointer */
    GT_assert(VhwaNfTrace, (NULL != instObj));
    GT_assert(VhwaNfTrace, (NULL != nfCfg));

    inFmt = &nfCfg->inFmt;

    /* Check for Valid data format */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((FVID2_DF_LUMA_ONLY != inFmt->dataFormat) &&
        (FVID2_DF_CHROMA_ONLY != inFmt->dataFormat) &&
        (FVID2_DF_YUV420SP_UV != inFmt->dataFormat) &&
        (FVID2_DF_YUV420SP_VU != inFmt->dataFormat))
    /* LDRA_JUSTIFY_END */
    {
        retVal = FVID2_EINVALID_PARAMS;
    }

    if((inFmt->width > VHWA_M2M_NF_MAX_WIDTH_HEIGHT) ||
        (inFmt->height > VHWA_M2M_NF_MAX_WIDTH_HEIGHT))
    {
        retVal = FVID2_EINVALID_PARAMS;
    }

    minPitch = Vhwa_calcHorzSizeInBytes(inFmt->width, inFmt->ccsFormat);
    if (inFmt->pitch[0] < minPitch)
    {
        retVal = FVID2_EINVALID_PARAMS;
    }
    else
    {
        instObj->sl2Prms.sl2Pitch[0] = Vhwa_calcSl2Pitch(minPitch);
    }

    outFmt = &nfCfg->outFmt;
    /* check for Input and output Width/height */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if((0u == nfCfg->nfCfg.skipMode) &&
       ((outFmt->width != inFmt->width) ||
        (outFmt->height != inFmt->height)))
    /* LDRA_JUSTIFY_END */
    {
        retVal = FVID2_EINVALID_PARAMS;
    }

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if((0u != nfCfg->nfCfg.skipMode) &&
       ((outFmt->width != (inFmt->width/2u)) ||
        (outFmt->height != (inFmt->height/2u))))
    /* LDRA_JUSTIFY_END */
    {
        retVal = FVID2_EINVALID_PARAMS;
    }

    minPitch = Vhwa_calcHorzSizeInBytes(outFmt->width,
                                        outFmt->ccsFormat);
    if (outFmt->pitch[0] < minPitch)
    {
        retVal = FVID2_EINVALID_PARAMS;
    }
    else
    {
        instObj->sl2Prms.sl2Pitch[1] = Vhwa_calcSl2Pitch(minPitch);
    }

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(outFmt->dataFormat != inFmt->dataFormat)
    {
        retVal = FVID2_EINVALID_PARAMS;
    }
    /* LDRA_JUSTIFY_END */
    return (retVal);
}

static void Vhwa_nfSetLseCfg(const Vhwa_M2mNfInstObj *instObj,
                             Vhwa_M2mNfHandleObj *hObj)
{
    Vhwa_M2mNfCreatePrms *createPrms = NULL;
    CSL_LseConfig *lseCfg = NULL;
    Vhwa_M2mNfConfig *nfCfg = NULL;
    uint32_t itrCnt;

    /* Check for Null pointer */
    GT_assert(VhwaNfTrace, (NULL != hObj));
    GT_assert(VhwaNfTrace, (NULL != instObj));

    for (itrCnt = 0; itrCnt < hObj->numIter; itrCnt++)
    {
        nfCfg = &hObj->nfCfg[itrCnt];
        lseCfg = &hObj->lseCfg[itrCnt];

        createPrms = &hObj->createPrms;

        /* Initialize LSE configuration with the default values */
        CSL_lseConfigInit(lseCfg);

        /* For NF, number of input and output channels are 1 */
        lseCfg->numInCh = 1U;
        lseCfg->numOutCh = 1U;

        lseCfg->enablePsa = createPrms->enablePsa;
        lseCfg->inChCfg[0U].enable = (uint32_t)UTRUE;
        lseCfg->inChCfg[0U].frameWidth = nfCfg->inFmt.width;
        lseCfg->inChCfg[0U].frameHeight = nfCfg->inFmt.height;
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
        <justification end> */
        if(nfCfg->outFmt.height == nfCfg->inFmt.height)
        /* LDRA_JUSTIFY_END */
        {
            lseCfg->inChCfg[0U].enableAddrIncrBy2 = (uint32_t)UFALSE;
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            lseCfg->inChCfg[0U].enableAddrIncrBy2 = (uint32_t)UTRUE;
       }
        /* LDRA_JUSTIFY_END */
        lseCfg->inChCfg[0U].ccsf =
            (Fvid2_ColorCompStorageFmt)nfCfg->inFmt.ccsFormat;
        lseCfg->inChCfg[0U].startOffset = 0U;
        lseCfg->inChCfg[0U].lineOffset = instObj->sl2Prms.sl2Pitch[0];
        lseCfg->inChCfg[0U].circBufSize = instObj->sl2Prms.sl2NumLines[0];
        lseCfg->inChCfg[0U].bufAddr[0U] = (uint32_t)instObj->sl2Prms.sl2Addr[0];
        lseCfg->inChCfg[0U].numInBuff = 1U;

        /* Assumes 5 tap filter, requires change once config is set */
        lseCfg->inChCfg[0U].enableMultiTapKnlConfig = UTRUE;
        lseCfg->inChCfg[0U].knTopPadding = 2U;
        lseCfg->inChCfg[0U].knLineOffset = 0U;
        lseCfg->inChCfg[0U].knHeight = 5U;
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (nfCfg->outFmt.height == nfCfg->inFmt.height)
        /* LDRA_JUSTIFY_END */
        {
            lseCfg->inChCfg[0U].knBottomPadding = 2U;
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            lseCfg->inChCfg[0U].knBottomPadding = 1U;
        }
        /* LDRA_JUSTIFY_END */

        lseCfg->outChCfg[0U].enable = (uint32_t)UTRUE;
        lseCfg->outChCfg[0U].ccsf =
            (Fvid2_ColorCompStorageFmt)nfCfg->outFmt.ccsFormat;
        lseCfg->outChCfg[0U].lineOffset = instObj->sl2Prms.sl2Pitch[1];
        lseCfg->outChCfg[0U].circBufSize = instObj->sl2Prms.sl2NumLines[1];
        lseCfg->outChCfg[0U].bufAddr = (uint32_t)instObj->sl2Prms.sl2Addr[1];
    }
}

static void Vhwa_nfSetHtsCfg(Vhwa_M2mNfInstObj *instObj,
                                Vhwa_M2mNfHandleObj *hObj)
{
    CSL_HtsSchConfig *htsCfg = NULL;
    Vhwa_M2mNfConfig *nfCfg = NULL;
    uint32_t itrCnt;

    /* Check for Null pointer */
    GT_assert(VhwaNfTrace, (NULL != hObj));
    GT_assert(VhwaNfTrace, (NULL != instObj));

    for (itrCnt = 0; itrCnt < hObj->numIter; itrCnt++)
    {
        htsCfg = &hObj->htsCfg[itrCnt];
        nfCfg = &hObj->nfCfg[itrCnt];

        CSL_htsSchConfigInit(htsCfg);

        htsCfg->schId = CSL_HTS_HWA_SCH_NF;
        htsCfg->pipeline = instObj->pipeline;
        htsCfg->enableStream = (uint32_t)UFALSE;
        htsCfg->enableHop = (uint32_t)UFALSE;
        htsCfg->enableWdTimer = UFALSE;
        htsCfg->isWdTimer128KMode = (uint32_t)UFALSE;

        htsCfg->enableBwLimit = UFALSE;

        htsCfg->consCfg[0U].enable = (uint32_t)UTRUE;
        htsCfg->consCfg[0U].prodId = CSL_HTS_PROD_IDX_UDMA;

        htsCfg->prodCfg[0U].enable = (uint32_t)UTRUE;
        htsCfg->prodCfg[0U].consId = CSL_HTS_CONS_IDX_UDMA;

        htsCfg->prodCfg[0U].threshold = 1u;
        htsCfg->prodCfg[0U].cntPreLoad = 0u;
        htsCfg->prodCfg[0U].cntPostLoad = 0u;

        htsCfg->prodCfg[0U].depth = instObj->sl2Prms.sl2NumLines[1];
        htsCfg->prodCfg[0U].countDec  =1U;

        htsCfg->dmaConsCfg[0U].enable = (uint32_t)UTRUE;
        htsCfg->dmaConsCfg[0U].dmaChNum = Udma_chGetNum(instObj->utcChHndl[1]);
        htsCfg->dmaConsCfg[0U].pipeline = instObj->pipeline;
        htsCfg->dmaConsCfg[0U].enableStream = (uint32_t)UFALSE;
        htsCfg->dmaConsCfg[0U].prodId = CSL_HTS_PROD_IDX_UDMA;
            /* Default mapping for DMA */

        htsCfg->dmaProdCfg[0U].enable = (uint32_t)UTRUE;
        htsCfg->dmaProdCfg[0U].dmaChNum = Udma_chGetNum(instObj->utcChHndl[0]);
        htsCfg->dmaProdCfg[0U].pipeline = instObj->pipeline;
        htsCfg->dmaProdCfg[0U].consId = CSL_HTS_CONS_IDX_UDMA;
            /* Default mapping for DMA */


        htsCfg->dmaProdCfg[0U].threshold = 5U;
        htsCfg->dmaProdCfg[0U].cntPreLoad = 2U;
        htsCfg->dmaProdCfg[0U].cntPostLoad = 2U;

        htsCfg->dmaProdCfg[0U].enableHop = (uint32_t)UTRUE;
        htsCfg->dmaProdCfg[0U].numHop = nfCfg->inFmt.height;

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (nfCfg->outFmt.height == nfCfg->inFmt.height)
        /* LDRA_JUSTIFY_END */
        {
            htsCfg->dmaProdCfg[0U].countDec = 1U;
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            htsCfg->dmaProdCfg[0U].countDec = 2U;
        }
        /* LDRA_JUSTIFY_END */

        htsCfg->dmaProdCfg[0U].depth = instObj->sl2Prms.sl2NumLines[0];
    }
}

static void Vhwa_nfSetCfgParams(Vhwa_M2mNfHandleObj *hObj,
                                const Vhwa_M2mNfConfig *nfCfg)
{
    uint32_t itrCnt;
    Vhwa_M2mNfConfig *nfConfig = NULL;


    /* Check for Null pointer */
    GT_assert(VhwaNfTrace, (NULL != hObj));
    GT_assert(VhwaNfTrace, (NULL != nfCfg));


    if ((FVID2_DF_YUV420SP_UV == nfCfg->inFmt.dataFormat) ||
        (FVID2_DF_YUV420SP_VU == nfCfg->inFmt.dataFormat))
    {
        /* Both Luma and Chroma Needs to be processed */
        hObj->numIter = 2U;
    }
    else
    {
        /* Only Chroma/Luma needs to be processed */
        hObj->numIter = 1U;
    }

    for (itrCnt = 0; itrCnt < hObj->numIter; itrCnt++)
    {
        nfConfig = &hObj->nfCfg[itrCnt];

        Fvid2Utils_memcpy(nfConfig, nfCfg, sizeof(Vhwa_M2mNfConfig));

        if(((FVID2_DF_YUV420SP_UV == nfConfig->inFmt.dataFormat) ||
            (FVID2_DF_YUV420SP_VU == nfConfig->inFmt.dataFormat)) && (itrCnt == 1U))
        {
            /* Chroma */
            nfConfig->nfCfg.interleaveMode = UTRUE;

            /* Height will be 1/2 for Chroma */
            nfConfig->inFmt.height =  nfConfig->inFmt.height/2U;
            nfConfig->outFmt.height =  nfConfig->outFmt.height/2U;
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
        <justification end> */
        else if(FVID2_DF_CHROMA_ONLY == nfConfig->inFmt.dataFormat)
        {
            nfConfig->nfCfg.interleaveMode = UTRUE;
        }
        /* LDRA_JUSTIFY_END */
        else
        {
            nfConfig->nfCfg.interleaveMode = UFALSE;
        }
    }
}

static void Vhwa_nfSetBwLimit(Vhwa_M2mNfHandleObj *hObj,
                              const Vhwa_HtsLimiter *bwLimit)
{
    uint32_t cnt;
    CSL_HtsSchConfig *htsCfg = NULL;

    /* Check for Null pointer */
    GT_assert(VhwaNfTrace, (NULL != hObj));
    GT_assert(VhwaNfTrace, (NULL != bwLimit));

    for (cnt = 0; cnt < hObj->numIter; cnt++)
    {
        htsCfg = &hObj->htsCfg[cnt];

        /* Configure  HTS BW limiter */
        htsCfg->enableBwLimit = bwLimit->enableBwLimit;
        htsCfg->cycleCnt      = bwLimit->cycleCnt;
        htsCfg->tokenCnt      = bwLimit->tokenCnt;
    }
}

int32_t Vhwa_m2mNfSetConfigInHW(const Vhwa_M2mNfInstObj *instObj,
                                const Vhwa_M2mNfQueueObj *qObj,
                                uint32_t itrCnt)
{
    int32_t              status = FVID2_SOK;
    Vhwa_M2mNfHandleObj *hObj = NULL;

    /* Null pointer check */
    GT_assert(VhwaNfTrace, (NULL != instObj));
    GT_assert(VhwaNfTrace, (NULL != qObj));
    GT_assert(VhwaNfTrace, (NULL != qObj->hObj));

    hObj = qObj->hObj;
    VhwaVpacNfSocReadBack *goldenRegVal = hObj->configRegMemPrms.configGoldenRegPtr;

    /* Configure NF Core */
    CSL_nfSetConfig(instObj->socInfo.nfRegs,
                &hObj->nfCfg[itrCnt].nfCfg);
    if (((uint32_t)UTRUE == hObj->enableConfigRegValidate) && (NULL != goldenRegVal))
    {
        CSL_nfSetConfig(&goldenRegVal->nfRegs, &hObj->nfCfg[itrCnt].nfCfg);
    }

    /* Configure HWA Common Wrapper LSE */
    status = CSL_lseSetConfig(instObj->socInfo.lseRegs, &hObj->lseCfg[itrCnt]);

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application. This behaviour is part of the application design. An error print statement can be added in a future release if required.
        <justification end> */
        if (((uint32_t)UTRUE == hObj->enableConfigRegValidate) && (NULL != goldenRegVal))
        /* LDRA_JUSTIFY_END */
        {
            status = CSL_lseSetConfig(&goldenRegVal->nfLseRegs, &hObj->lseCfg[itrCnt]);
        }
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Configure HWA Common Wrapper HTS */
        status = CSL_htsSetThreadConfig(instObj->socInfo.htsRegs, &hObj->htsCfg[itrCnt]);
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Start HTS Channels */
        status = CSL_htsThreadStart(instObj->socInfo.htsRegs,
                                    &hObj->htsCfg[itrCnt]);
    }

    return (status);
}

int32_t Vhwa_m2mNfSetFrameSize(Vhwa_M2mNfInstObj *instObj,
                               const Vhwa_M2mNfQueueObj *qObj,
                               uint32_t itrCnt)
{
    int32_t               status = FVID2_SOK;
    Vhwa_M2mNfHandleObj *hObj = NULL;

    /* Null pointer check */
    GT_assert(VhwaNfTrace, (NULL != instObj));
    GT_assert(VhwaNfTrace, (NULL != qObj));
    GT_assert(VhwaNfTrace, (NULL != qObj->hObj));

    hObj = qObj->hObj;

    /* Configure NF Core */
    CSL_nfUpdateConfig(instObj->socInfo.nfRegs,
                &hObj->nfCfg[itrCnt].nfCfg);
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (((uint32_t)UTRUE == hObj->enableConfigRegValidate) && (NULL != hObj->configRegMemPrms.configGoldenRegPtr))
    /* LDRA_JUSTIFY_END */
    {
        CSL_nfUpdateConfig(&hObj->configRegMemPrms.configGoldenRegPtr->nfRegs, &hObj->nfCfg[itrCnt].nfCfg);
    }

    /* Configure HWA Common Wrapper LSE */
    status = CSL_lseSetUpdateConfig(instObj->socInfo.lseRegs,
        &hObj->lseCfg[itrCnt]);
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (((uint32_t)UTRUE == hObj->enableConfigRegValidate) && (NULL != hObj->configRegMemPrms.configGoldenRegPtr))
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if(status == FVID2_SOK)
        /* LDRA_JUSTIFY_END */
        {
            status = CSL_lseSetUpdateConfig(&hObj->configRegMemPrms.configGoldenRegPtr->nfLseRegs,
                &hObj->lseCfg[itrCnt]);
        }
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Configure HWA Common Wrapper HTS */
        status = CSL_htsSetThreadUpdateConfig(instObj->socInfo.htsRegs,
            &hObj->htsCfg[itrCnt]);
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Start HTS Channels */
        status = CSL_htsThreadStart(instObj->socInfo.htsRegs,
                                    &hObj->htsCfg[itrCnt]);
    }

    return (status);
}

int32_t Vhwa_m2mNfSubmitRequest(Vhwa_M2mNfInstObj *instObj,
                                    Vhwa_M2mNfQueueObj *qObj,
                                    uint32_t itrCnt)
{
    int32_t retVal = FVID2_SOK;
    Vhwa_M2mNfHandleObj *hObj = NULL;

    GT_assert(VhwaNfTrace, (NULL != instObj));
    GT_assert(VhwaNfTrace, (NULL != qObj));
    GT_assert(VhwaNfTrace, (NULL != qObj->hObj));

    hObj = qObj->hObj;
    VhwaVpacNfSocReadBack *goldenRegVal = hObj->configRegMemPrms.configGoldenRegPtr;
    /* Submit Rings to the Ring Accelerator */
    retVal = Vhwa_m2mNfSubmitRing(instObj, hObj, itrCnt);

    /* Update the NfConfigRegisterGroup with config register values for frame specific Static Config, INTD and HTS registers */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (((uint32_t)UTRUE == hObj->enableConfigRegValidate) && (goldenRegVal != NULL))
    /* LDRA_JUSTIFY_END */
    {
        retVal = vhwaM2mNfUpdateConfigRegGroup(goldenRegVal, instObj, hObj);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to print the error status. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != retVal)
        {
            GT_0trace(VhwaNfTrace, GT_ERR, "Failed to set default goldenReg mem values!\n");
            retVal = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */

    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == retVal)
    /* LDRA_JUSTIFY_END */
    {
        /* Better to set Active object to this q object, so that if
             * interrupt comes immediately, actQObj would be set..
             * If pipeline start fails, it would be set to NULL.*/
        instObj->actQObj = qObj;

        instObj->totalReqCnt ++;

        #ifndef VHWA_USE_PIPELINE_COMMON_ENABLE
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != hObj->createPrms.getTimeStamp)
        {
            hObj->perfNum[itrCnt] = hObj->createPrms.getTimeStamp();
        }
        /* LDRA_JUSTIFY_END */
        /* Start HTS pipeline */
        retVal = CSL_htsPipelineStart(instObj->socInfo.htsRegs,
                                      &hObj->htsCfg[itrCnt]);
        #endif
        /* LDRA_JUSTIFY_START
        <metric start>statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps.
        Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != retVal)
        {
            instObj->actQObj = NULL;
            retVal = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */
        else
        {
            retVal = FVID2_SOK;
        }
    }

    return (retVal);
}

static int32_t Vhwa_nfCheckCreatePrms(uint32_t drvId, uint32_t drvInstId)
{
    int32_t     status = FVID2_SOK;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_VHWA_M2M_NF_DRV_ID != drvId)
    {
        GT_0trace(VhwaNfTrace, GT_ERR, "Invalid Driver ID !!\n");
        status = FVID2_EINVALID_PARAMS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        /* Check for correct instance ID */
#if defined VHWA_M2M_VPAC_INSTANCE
#if (VHWA_M2M_VPAC_INSTANCE == 0)
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (VHWA_M2M_VPAC_0_NF_DRV_INST_ID_0 != drvInstId)
        {
            status = FVID2_EINVALID_PARAMS;
            GT_0trace(VhwaNfTrace, GT_ERR, "Invalid/unsupported Instance Id\n");
        }
        /* LDRA_JUSTIFY_END */
#endif
#endif

#if defined VHWA_M2M_VPAC_INSTANCE
#if (VHWA_M2M_VPAC_INSTANCE == 1)
        if (VHWA_M2M_VPAC_1_NF_DRV_INST_ID_0 != drvInstId)
        {
            status = FVID2_EINVALID_PARAMS;
            GT_0trace(VhwaNfTrace, GT_ERR, "Invalid/unsupported Instance Id\n");
        }
#endif
#endif
    }

    return (status);
}

static int32_t Vhwa_m2mNfReconfigReinitReg(const Vhwa_M2mNfInstObj *instObj,
                                           const Vhwa_M2mNfHandleObj *hObj,
                                           const Vhwa_M2mNfQueueObj *qObj)
{
    int32_t status = FVID2_SOK;
    VhwaVpacNfSocReadBack *goldenRegVal = NULL;

    /* Null pointer check */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer. However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == instObj) || (NULL == hObj) || (NULL == qObj))
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        goldenRegVal = hObj->configRegMemPrms.configGoldenRegPtr;

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application. This behaviour is part of the application design. An error print statement can be added in a future release if required.
        <justification end> */
        if (instObj->lastHndlObj == qObj->hObj)
        /* LDRA_JUSTIFY_END */
        {
            /* Last handle was same as new handle,
             * so hardware has not been reconfigured */
            status = Vhwa_m2mNfSetConfigInHW((const Vhwa_M2mNfInstObj *)instObj, qObj, 0U);
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            CSL_vpacEnableModule(instObj->socInfo.vpacCntlRegs,
                VPAC_MODULE_NF, (uint32_t)UTRUE);
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The parameters are pre-validated by the caller before the control reaches here.
            Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer. However, due to the stated rationale, this is not tested.
            <justification end> */
            if(((uint32_t)UTRUE == hObj->enableConfigRegValidate) && (NULL != goldenRegVal))
            /* LDRA_JUSTIFY_END */
            {
                CSL_vpacEnableModule(&goldenRegVal->vpacCntlRegs,
                    VPAC_MODULE_NF, (uint32_t)UTRUE);
            }
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            /* Re-Init Pipeline interrupt in INTD */
            Vhwa_enableHtsIntr(instObj->socInfo.vpacIntdRegs,
                instObj->vhwaIrqNum, instObj->pipeline);
        }
    }
    return status;
}

static int32_t vhwaM2mNfUpdateStatusRegGroup(Vhwa_M2mNfHandleObj *hObj)
{
    NfStatusRegisterGroup *statusRegs;
    int32_t status = FVID2_SOK;
    uint32_t expected_val = 0U;
    uint32_t i;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL == hObj)
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        statusRegs = &hObj->nfStatusRegs;

        uint32_t numProducers    = CSL_HTS_NUM_NF_PRODUCERS;
        #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
        uint32_t numSchedulers   = CSL_HTS_NUM_NF_SCHEDULERS;
        #endif
        uint32_t numDmaProducers = CSL_HTS_NUM_NF_PRODUCERS_DMA;

        /* Update PROD_COUNT for all producers (HWA6) */
        for (i = 0; i < numProducers; i++)
        {
            CSL_HtsSchConfig *cfg = &hObj->htsCfg[0];
            CSL_HtsProdConfig *prodCfg = &cfg->prodCfg[i];

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The parameters are pre-validated by the caller before the control reaches here.
            Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer. However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((prodCfg != NULL) && (prodCfg->enable == UTRUE))
            /* LDRA_JUSTIFY_END */
            {
                expected_val = 0U;
                CSL_FINS(expected_val, HTS_HWA6_PROD0_COUNT_COUNT_POSTLOAD, prodCfg->cntPostLoad);
                statusRegs->nfHtsStatus.PROD_COUNT[i] = expected_val;
            }
            else
            {
                statusRegs->nfHtsStatus.PROD_COUNT[i] = 0U;
            }
        }

        #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
        /* Update HWA_HOP for all schedulers (HWA6) */
        for (i = 0; i < numSchedulers; i++)
        {
            CSL_HtsSchConfig *cfg = &hObj->htsCfg[0];

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The parameters are pre-validated by the caller before the control reaches here.
            Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer. However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((cfg != NULL) && (cfg->tdoneCntEnable == UTRUE))
            /* LDRA_JUSTIFY_END */
            {
                expected_val = 0U;
                CSL_FINS(expected_val, HTS_HWA6_HOP_TDONE_COUNT, hObj->nfCfg->inFmt.height);
                statusRegs->nfHtsStatus.HWA_HOP[i] = expected_val;
            }
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale:
            Effect on this unit:
            <justification end> */
            else
            {
                statusRegs->nfHtsStatus.HWA_HOP[i] = 0U;
            }
            /* LDRA_JUSTIFY_END */
        }
        #endif

        /* Update PROD_DMA_COUNT and PROD_DMA_HOP for all DMA producers (DMA48) */
        for (i = 0; i < numDmaProducers; i++)
        {
            CSL_HtsSchConfig *cfg = &hObj->htsCfg[0];
            CSL_HtsDmaProdConfig *dmaProdCfg = &cfg->dmaProdCfg[i];

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The parameters are pre-validated by the caller before the control reaches here.
            Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer. However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((dmaProdCfg != NULL) && (dmaProdCfg->enable == UTRUE))
            /* LDRA_JUSTIFY_END */
            {
                expected_val = 0U;
                CSL_FINS(expected_val, HTS_DMA48_PROD0_COUNT_COUNT_POSTLOAD, dmaProdCfg->cntPostLoad);
                statusRegs->nfHtsStatus.PROD_DMA_COUNT[i] = expected_val;
                #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
                expected_val = 0U;
                CSL_FINS(expected_val, HTS_DMA48_HOP_TDONE_COUNT, dmaProdCfg->numHop);
                statusRegs->nfHtsStatus.PROD_DMA_HOP[i] = expected_val;
                #endif
            }
            else
            {
                statusRegs->nfHtsStatus.PROD_DMA_COUNT[i] = 0U;
                #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
                statusRegs->nfHtsStatus.PROD_DMA_HOP[i] = 0U;
                #endif
            }
        }
    }

    return status;
}

int32_t Vhwa_m2mNfStatusRegValidate(const NfStatusRegisterGroup *expected, const Nf_SocInfo *socInfo)
{
    int32_t status = FVID2_SOK;
    uint32_t i;

    /* Null pointer check */
    if ((expected == NULL) || (socInfo == NULL))
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        /* Validate PID values in CSL_vpacCntlRegs */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)VHWA_HW_PID_VPAC != CSL_REG32_RD(&socInfo->vpacCntlRegs->PID))
        {
            GT_0trace(VhwaNfTrace, GT_ERR, "StatusRegValidate: vpacCntlRegs->PID mismatch\n");
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        /* Validate PID values in CSL_nfRegs */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)VHWA_HW_PID_NF != CSL_REG32_RD(&socInfo->nfRegs->REVISION))
        {
            GT_0trace(VhwaNfTrace, GT_ERR, "StatusRegValidate: nfRegs->PID mismatch\n");
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        /* Validate LSE status directly from registers using CSL_RD */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)FVID2_SOK != CSL_REG32_RD(&socInfo->lseRegs->STATUS_ERROR))
        {
            GT_0trace(VhwaNfTrace, GT_ERR, "StatusRegValidate: lseRegs->STATUS_ERROR mismatch\n");
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        /* Validate LSE status idle mode */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)FVID2_SOK != CSL_REG32_RD(&socInfo->lseRegs->STATUS_IDLE_MODE))
        {
            GT_0trace(VhwaNfTrace, GT_ERR, "StatusRegValidate: lseRegs->STATUS_IDLE_MODE mismatch\n");
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        /* Validate HTS status directly from registers using CSL_RD */
        if (socInfo->htsRegs != NULL)
        {
            uint32_t numProducers           = CSL_HTS_NUM_NF_PRODUCERS;
            uint32_t numDmaProducers        = CSL_HTS_NUM_NF_PRODUCERS_DMA;
            uint32_t numDmaConsumers        = CSL_HTS_NUM_NF_CONSUMERS_DMA;
            #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
            uint32_t numSchedulers          = CSL_HTS_NUM_NF_SCHEDULERS;
            #endif
            const CSL_HtsHwaParams *htsPrms = GetVpacHtsParams(CSL_HTS_HWA_SCH_NF);
            /* Validate HTS SCHEDULER_CONTROL and WD_TIMER for NF instance (HWA6) */
            uint32_t scheduler_base_addr = (uint32_t)socInfo->htsRegs + htsPrms->schOffset;

            /* Validate HTS SCHEDULER_CONTROL */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((uint32_t)FVID2_SOK !=
            (CSL_REG32_RD(scheduler_base_addr) & CSL_HTS_HWA6_SCHEDULER_CONTROL_STATE_MASK))
            {
                GT_0trace(VhwaNfTrace, GT_ERR, "StatusRegValidate: htsRegs->SCHEDULER_CONTROL mismatch\n");
                status = FVID2_EFAIL;
            }
            /* LDRA_JUSTIFY_END */

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
            /* Validate HTS WD_TIMER status */
            if (status == FVID2_SOK)
            /* LDRA_JUSTIFY_END */
            {
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((uint32_t)FVID2_SOK !=
                    (CSL_REG32_RD(scheduler_base_addr + HWA_WDTIMER_OFFSET) & CSL_HTS_HWA6_WDTIMER_WDTIMER_STATUS_MASK))
                {
                    GT_0trace(VhwaNfTrace, GT_ERR, "StatusRegValidate: htsRegs->WDTIMER mismatch\n");
                    status = FVID2_EFAIL;
                }
                /* LDRA_JUSTIFY_END */
            }
            #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
            /* Validate HTS HWA_HOP for all schedulers */
            if (status == FVID2_SOK)
            /* LDRA_JUSTIFY_END */
            {
                for (i = 0; i < numSchedulers; i++)
                {
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if ((expected->nfHtsStatus.HWA_HOP[i] & CSL_HTS_HWA6_HOP_TDONE_COUNT_MASK) !=
                        (CSL_REG32_RD(scheduler_base_addr + HWA_HOP_OFFSET) & CSL_HTS_HWA6_HOP_TDONE_COUNT_MASK))
                    {
                        GT_1trace(VhwaNfTrace, GT_ERR, "StatusRegValidate: htsRegs->HWA_HOP[%d] mismatch\n", i);
                        status = FVID2_EFAIL;
                    }
                    /* LDRA_JUSTIFY_END */
                }
            }
            #endif
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
            /* Validate HTS PROD_COUNT for all producers */
            if (status == FVID2_SOK)
            /* LDRA_JUSTIFY_END */
            {
                for (i = 0; i < numProducers; i++)
                {
                    uint32_t prod_count_addr = (uint32_t)socInfo->htsRegs + htsPrms->prodOffset[i];
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if ((expected->nfHtsStatus.PROD_COUNT[i] & CSL_HTS_HWA6_PROD0_COUNT_COUNT_MASK) !=
                        (CSL_REG32_RD(prod_count_addr + HWA_PROD_COUNT_OFFSET) & CSL_HTS_HWA6_PROD0_COUNT_COUNT_MASK))
                    {
                        GT_1trace(VhwaNfTrace, GT_ERR, "StatusRegValidate: htsRegs->PROD_COUNT[%d] mismatch\n", i);
                        status = FVID2_EFAIL;
                    }
                    #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
                    if ((uint32_t)FVID2_SOK != (CSL_REG32_RD(prod_count_addr + HWA_PROD_PA_PROD_COUNT_OFFSET) & CSL_HTS_HWA6_PA0_PRODCOUNT_PA_PSCOUNT_MASK))
                    {
                        GT_1trace(VhwaNfTrace, GT_ERR, "StatusRegValidate: htsRegs->PA_PROD_COUNT[%d] mismatch\n", i);
                        status = FVID2_EFAIL;
                    }
                    #endif
                    /* LDRA_JUSTIFY_END */
                }
            }
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
            /* Validate PROD_DMA_SCHEDULER_CONTROL, PROD_DMA_COUNT, PROD_DMA_HOP for DMA producers (DMA48) */
            if (status == FVID2_SOK)
            /* LDRA_JUSTIFY_END */
            {
                for (i = 0; i < numDmaProducers; i++)
                {
                    uint32_t prod_dma_addr = (uint32_t)socInfo->htsRegs + htsPrms->dmaProdOffset[i];
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if ((uint32_t)FVID2_SOK !=
                        (CSL_REG32_RD(prod_dma_addr) & CSL_HTS_DMA48_SCHEDULER_CONTROL_STATE_MASK))
                    {
                        GT_1trace(VhwaNfTrace, GT_ERR, "StatusRegValidate: htsRegs->PROD_DMA_SCHEDULER_CONTROL[%d] mismatch\n", i);
                        status = FVID2_EFAIL;
                    }
                    if ((expected->nfHtsStatus.PROD_DMA_COUNT[i] & CSL_HTS_DMA48_PROD0_COUNT_COUNT_MASK) !=
                        (CSL_REG32_RD(prod_dma_addr + HWA_DMA_PROD_COUNT_OFFSET) & CSL_HTS_DMA48_PROD0_COUNT_COUNT_MASK))
                    {
                        GT_1trace(VhwaNfTrace, GT_ERR, "StatusRegValidate: htsRegs->PROD_DMA_COUNT[%d] mismatch\n", i);
                        status = FVID2_EFAIL;
                    }
                    #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
                    if ((expected->nfHtsStatus.PROD_DMA_HOP[i] & CSL_HTS_DMA48_HOP_TDONE_COUNT_MASK) !=
                        (CSL_REG32_RD(prod_dma_addr + HWA_DMA_PROD_HOP_OFFSET) & CSL_HTS_DMA48_HOP_TDONE_COUNT_MASK))
                    {
                        GT_1trace(VhwaNfTrace, GT_ERR, "StatusRegValidate: htsRegs->PROD_DMA_HOP[%d] mismatch\n", i);
                        status = FVID2_EFAIL;
                    }
                    #endif
                    /* LDRA_JUSTIFY_END */
                }
            }
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
            /* Validate for DMA consumer scheduler control status (NF uses DMA336) */
            if (status == FVID2_SOK)
            /* LDRA_JUSTIFY_END */
            {
                for (i = 0; i < numDmaConsumers; i++)
                {
                    uint32_t cons_dma_addr = (uint32_t)socInfo->htsRegs + htsPrms->dmaConsOffset[i];
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if ((uint32_t)FVID2_SOK !=
                        (CSL_REG32_RD(cons_dma_addr) & CSL_HTS_DMA336_SCHEDULER_CONTROL_STATE_MASK))
                    {
                        GT_1trace(VhwaNfTrace, GT_ERR, "StatusRegValidate: htsRegs->CONSUMER_DMA_SCHEDULER_CONTROL[%d] mismatch\n", i);
                        status = FVID2_EFAIL;
                    }
                    /* LDRA_JUSTIFY_END */
                }
            }
        }
    }

    return status;
}

int32_t Vhwa_m2mNfConfigRegReadback(const Vhwa_M2mNfHandleObj *hObj, const Vhwa_M2mNfInstObj *instObj)
{
    int32_t status = FVID2_SOK;

    VhwaVpacNfSocReadBack *readback = NULL;
    CSL_vpac_nfRegs            *nfRegs   = NULL;
    CSL_lseRegs           *lseRegs  = NULL;
    CSL_htsRegs           *htsRegs  = NULL;
    CSL_vpac_cntlRegs      *vpacCntlRegs = NULL;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((hObj == NULL) || (instObj == NULL))
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        readback = hObj->configRegMemPrms.configRegReadbackPtr;
        nfRegs   = instObj->socInfo.nfRegs;
        lseRegs  = instObj->socInfo.lseRegs;
        htsRegs  = instObj->socInfo.htsRegs;
        vpacCntlRegs = instObj->socInfo.vpacCntlRegs;
    }

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == readback) || (NULL == nfRegs) || (NULL == lseRegs) || (NULL == htsRegs) || (NULL == vpacCntlRegs))
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {

        /* Read back NF core registers */
        readback->nfRegs.CONTROL       = CSL_REG32_RD(&nfRegs->CONTROL);
        readback->nfRegs.CENTER_WEIGHT = CSL_REG32_RD(&nfRegs->CENTER_WEIGHT);

        for(uint32_t i =0; i < VHWA_VPAC_NF_WEIGHT_LUT_SIZE; i++)
        {
            readback->nfRegs.WEIGHT_LUT[i] = CSL_REG32_RD(&nfRegs->WEIGHT_LUT[i]);
        }
        /* Read back CFG_LSE register */
        readback->nfLseRegs.CFG_LSE = CSL_REG32_RD(&lseRegs->CFG_LSE);

        for (uint32_t i = 0U; i < 1u ; i++)
        {
            readback->nfLseRegs.SRC[i].CFG        = CSL_REG32_RD(&lseRegs->SRC[i].CFG);
            readback->nfLseRegs.SRC[i].FRAME_SIZE = CSL_REG32_RD(&lseRegs->SRC[i].FRAME_SIZE);
            readback->nfLseRegs.SRC[i].BUF_ATTR   = CSL_REG32_RD(&lseRegs->SRC[i].BUF_ATTR);
            for (uint32_t j = 0U; j < 1u ; j++)
            {
                readback->nfLseRegs.SRC[i].BUF_BA[j] = CSL_REG32_RD(&lseRegs->SRC[i].BUF_BA[j]);
            }
        }

        for (uint32_t i = 0U; i < 1u ; i++)
        {
            /* Read BUF_CFG ignoring the CSL_LSE_DST_BUF_CFG_CH_DISABLED_MASK bit */
            readback->nfLseRegs.DST[i].BUF_CFG    = CSL_REG32_RD(&lseRegs->DST[i].BUF_CFG) & ~CSL_LSE_DST_BUF_CFG_CH_DISABLED_MASK;
            readback->nfLseRegs.DST[i].BUF_ATTR0  = CSL_REG32_RD(&lseRegs->DST[i].BUF_ATTR0);
            readback->nfLseRegs.DST[i].BUF_BA     = CSL_REG32_RD(&lseRegs->DST[i].BUF_BA);
        }
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer. However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((readback != NULL) && (instObj != NULL))
    /* LDRA_JUSTIFY_END */
    {
        status = vhwaM2mNfUpdateConfigRegGroup(readback, instObj, hObj);
    }

    return status;
}

int32_t vhwaM2mNfUpdateConfigRegGroup(VhwaVpacNfSocReadBack *reg_buf_ptr, const Vhwa_M2mNfInstObj *instObj, const Vhwa_M2mNfHandleObj *hObj)
{
    int32_t status = FVID2_SOK;
    volatile CSL_htsRegs *htsRegs = NULL;
    uint32_t reg_val;
    uint32_t scheduler_base_addr;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == reg_buf_ptr) || (NULL == instObj) || (NULL == hObj))
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {

        /* Accumulate any static configuration registers not updated after init */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != instObj->socInfo.vpacCntlRegs)
        /* LDRA_JUSTIFY_END */
        {
            reg_buf_ptr->vpacCntlRegs.ENABLE = CSL_REG32_RD(&instObj->socInfo.vpacCntlRegs->ENABLE);
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer. However, due to the stated rationale, this is not tested.
        <justification end> */
        /* Accumulate INTD registers for NF */
        if (NULL != instObj->socInfo.vpacIntdRegs)
        /* LDRA_JUSTIFY_END */
        {
            volatile CSL_vpac_intd_cfgRegs *intdRegs = instObj->socInfo.vpacIntdRegs;
            uint32_t vhwaIrqNum = instObj->vhwaIrqNum;
            uint32_t counter = 0U;

            /* NOTE: Do not make this a loop, since the INTR macros (VHWA_VPAC_INTD_PIPE_INTR_REG,
             * VHWA_VPAC_INTD_NF_INTR_REG, VHWA_VPAC_INTD_WDTIMER_INTR_REG) can change in future.
             * Keeping this as explicit assignments ensures maintainability if the macros or their order change.
             */

            /* 0 -> VHWA_VPAC_INTD_NF_INTR_REG */
            reg_buf_ptr->vpacNfIntd.ENABLE_REG_LEVEL_VPAC_OUT[vhwaIrqNum][counter] =
            CSL_REG32_RD(&intdRegs->ENABLE_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_NF_INTR_REG]);
            reg_buf_ptr->vpacNfIntd.ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][counter] =
            CSL_REG32_RD(&intdRegs->ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_NF_INTR_REG]);
            counter += 1U;

            /* 1 -> VHWA_VPAC_INTD_PIPE_INTR_REG */
            reg_buf_ptr->vpacNfIntd.ENABLE_REG_LEVEL_VPAC_OUT[vhwaIrqNum][counter] =
            CSL_REG32_RD(&intdRegs->ENABLE_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_PIPE_INTR_REG]);
            reg_buf_ptr->vpacNfIntd.ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][counter] =
            CSL_REG32_RD(&intdRegs->ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_PIPE_INTR_REG]);
            counter += 1U;

            /* 2 -> VHWA_VPAC_INTD_WDTIMER_INTR_REG */
            reg_buf_ptr->vpacNfIntd.ENABLE_REG_LEVEL_VPAC_OUT[vhwaIrqNum][counter] =
            CSL_REG32_RD(&intdRegs->ENABLE_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_WDTIMER_INTR_REG]);
            reg_buf_ptr->vpacNfIntd.ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][counter] =
            CSL_REG32_RD(&intdRegs->ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_WDTIMER_INTR_REG]);
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer. However, due to the stated rationale, this is not tested.
        <justification end> */
        /* Accumulate HTS registers for NF */
        if (instObj->socInfo.htsRegs != NULL)
        /* LDRA_JUSTIFY_END */
        {
            htsRegs = instObj->socInfo.htsRegs;
            const CSL_HtsHwaParams *htsPrms = GetVpacHtsParams(CSL_HTS_HWA_SCH_NF);

            /* Read the HTS pipeline control register */
            reg_buf_ptr->nfHtsRegs.PIPELINE_CONTROL[0] = CSL_REG32_RD(&htsRegs->PIPELINE_CONTROL[(uint32_t)hObj->htsCfg[0].pipeline]);

            for (uint32_t schIdx = 0; schIdx < CSL_HTS_NUM_NF_SCHEDULERS; schIdx++)
            {
                scheduler_base_addr = (uint32_t)instObj->socInfo.htsRegs + htsPrms->schOffset;
                {
                    reg_val = CSL_REG32_RD(scheduler_base_addr + HWA_SCH_CONTROL_OFFSET);
                    reg_buf_ptr->nfHtsRegs.SCHEDULER_CONTROL[schIdx] =
                        (reg_val & CSL_HTS_HWA6_SCHEDULER_CONTROL_PAUSE_MASK) |
                        (reg_val & CSL_HTS_HWA6_SCHEDULER_CONTROL_STRM_EN_MASK) |
                        (reg_val & CSL_HTS_HWA6_SCHEDULER_CONTROL_PIPELINE_NUM_MASK) |
                        (reg_val & CSL_HTS_HWA6_SCHEDULER_CONTROL_SCH_EN_MASK) |
                        (reg_val & CSL_HTS_HWA6_SCHEDULER_CONTROL_DEBUG_RDY_MASK);
                }

                #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
                reg_val = CSL_REG32_RD(scheduler_base_addr + HWA_HOP_OFFSET);
                reg_buf_ptr->nfHtsRegs.HOP[schIdx] =
                        (reg_val & CSL_HTS_HWA6_HOP_TDONE_COUNT_EN_MASK) |
                        (reg_val & CSL_HTS_HWA6_HOP_HOP_MASK) |
                        (reg_val & CSL_HTS_HWA6_HOP_HOP_THREAD_COUNT_MASK);
                #endif

                /* Only keep enable and mode bits from WD_TIMER register */
                {
                    reg_val = CSL_REG32_RD(scheduler_base_addr + HWA_WDTIMER_OFFSET);
                    reg_buf_ptr->nfHtsRegs.WDTIMER[schIdx] =
                        (reg_val & CSL_HTS_HWA6_WDTIMER_WDTIMER_EN_MASK) |
                        (reg_val & CSL_HTS_HWA6_WDTIMER_WDTIMER_MODE_MASK);
                }

                reg_buf_ptr->nfHtsRegs.BW_LIMITER[schIdx] = CSL_REG32_RD(scheduler_base_addr + HWA_BW_LIMITER_OFFSET);
            }
            /* Readback for consumer control */
            for (uint32_t i = 0; i < CSL_HTS_NUM_NF_CONSUMERS; i++)
            {
                uint32_t cons_ctrl_addr = (uint32_t)instObj->socInfo.htsRegs + htsPrms->consOffset[i];
                reg_buf_ptr->nfHtsRegs.CONS_CONTROL[i] = CSL_REG32_RD(cons_ctrl_addr + HWA_CONS_CONTROL_OFFSET);
            }

            /* Readback for producers control */
            for (uint32_t i = 0; i < CSL_HTS_NUM_NF_PRODUCERS; i++)
            {
                uint32_t prod_ctrl_addr = (uint32_t)instObj->socInfo.htsRegs + htsPrms->prodOffset[i];

                reg_buf_ptr->nfHtsRegs.PROD_CONTROL[i] = CSL_REG32_RD(prod_ctrl_addr + HWA_PROD_CONTROL_OFFSET);
                reg_buf_ptr->nfHtsRegs.PROD_BUF_CONTROL[i] = CSL_REG32_RD(prod_ctrl_addr + HWA_PROD_BUF_CONTROL_OFFSET);

                /* Only keep postload and preload bits from PROD_COUNT register */
                reg_val = CSL_REG32_RD(prod_ctrl_addr + HWA_PROD_COUNT_OFFSET);
                reg_buf_ptr->nfHtsRegs.PROD_COUNT[i] =
                    (reg_val & CSL_HTS_HWA6_PROD0_COUNT_COUNT_POSTLOAD_MASK) |
                    (reg_val & CSL_HTS_HWA6_PROD0_COUNT_COUNT_PRELOAD_MASK);

                #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
                reg_buf_ptr->nfHtsRegs.PA_CONTROL[i] = CSL_REG32_RD(prod_ctrl_addr + HWA_PROD_PA_CONTROL_OFFSET);

                /* Only keep PA_COUNT_DEC bits from PA_PRODCOUNT register */
                reg_val = CSL_REG32_RD(prod_ctrl_addr + HWA_PROD_PA_PROD_COUNT_OFFSET);
                reg_buf_ptr->nfHtsRegs.PA_PRODCOUNT[i] = (reg_val & CSL_HTS_HWA6_PA0_PRODCOUNT_PA_COUNT_DEC_MASK);
                #endif
            }

            /* Readback for DMA producers */
            for (uint32_t i = 0; i < CSL_HTS_NUM_NF_PRODUCERS_DMA; i++)
            {
            uint32_t prod_dma_ctrl_addr = (uint32_t)instObj->socInfo.htsRegs + htsPrms->dmaProdOffset[i];

            /* PROD_DMA_SCHEDULER_CONTROL */
            reg_val = CSL_REG32_RD(prod_dma_ctrl_addr + HWA_DMA_PROD_SCH_CONTROL_OFFSET);
            reg_buf_ptr->nfHtsRegs.PROD_DMA_SCHEDULER_CONTROL[i] =
                (reg_val & CSL_HTS_DMA48_SCHEDULER_CONTROL_DMA_CHANNEL_NO_MASK) |
                (reg_val & CSL_HTS_DMA48_SCHEDULER_CONTROL_DEBUG_RDY_MASK) |
                (reg_val & CSL_HTS_DMA48_SCHEDULER_CONTROL_PAUSE_MASK) |
                (reg_val & CSL_HTS_DMA48_SCHEDULER_CONTROL_STRM_EN_MASK) |
                (reg_val & CSL_HTS_DMA48_SCHEDULER_CONTROL_PIPELINE_NUM_MASK) |
                (reg_val & CSL_HTS_DMA48_SCHEDULER_CONTROL_SCH_EN_MASK);

            #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
            /* PROD_DMA_HOP */
            reg_val = CSL_REG32_RD(prod_dma_ctrl_addr + HWA_DMA_PROD_HOP_OFFSET);
            reg_buf_ptr->nfHtsRegs.PROD_DMA_HOP[i] =
                (reg_val & CSL_HTS_DMA48_HOP_HOP_MASK) |
                (reg_val & CSL_HTS_DMA48_HOP_HOP_THREAD_COUNT_MASK) |
                (reg_val & CSL_HTS_DMA48_HOP_TDONE_COUNT_EN_MASK);
            #endif

            /* PROD_CONTROL_DMA */
            reg_buf_ptr->nfHtsRegs.PROD_CONTROL_DMA[i] = CSL_REG32_RD(prod_dma_ctrl_addr + HWA_DMA_PROD_CONTROL_OFFSET);

            /* PROD_BUF_CONTROL_DMA */
            reg_buf_ptr->nfHtsRegs.PROD_BUF_CONTROL_DMA[i] = CSL_REG32_RD(prod_dma_ctrl_addr + HWA_DMA_PROD_BUF_CONTROL_OFFSET);

            /* Only keep postload and preload bits from PROD_DMA_COUNT register */
            reg_val = CSL_REG32_RD(prod_dma_ctrl_addr + HWA_DMA_PROD_COUNT_OFFSET);
            reg_buf_ptr->nfHtsRegs.PROD_COUNT_DMA[i] =
                (reg_val & CSL_HTS_DMA48_PROD0_COUNT_COUNT_POSTLOAD_MASK) |
                (reg_val & CSL_HTS_DMA48_PROD0_COUNT_COUNT_PRELOAD_MASK);
            }

            /* Readback for consumers DMA */
            for (uint32_t i = 0; i < CSL_HTS_NUM_NF_CONSUMERS_DMA; i++)
            {
                uint32_t cons_dma_ctrl_addr = (uint32_t)instObj->socInfo.htsRegs + htsPrms->dmaConsOffset[i];

                /* CONSUMER_DMA_SCHEDULER_CONTROL */
                reg_val = CSL_REG32_RD(cons_dma_ctrl_addr + HWA_DMA_CONS_SCH_CONTROL_OFFSET);
                reg_buf_ptr->nfHtsRegs.CONS_DMA_SCHEDULER_CONTROL[i] =
                    (reg_val & CSL_HTS_DMA336_SCHEDULER_CONTROL_DMA_CHANNEL_NO_MASK) |
                    (reg_val & CSL_HTS_DMA336_SCHEDULER_CONTROL_DEBUG_RDY_MASK) |
                    (reg_val & CSL_HTS_DMA336_SCHEDULER_CONTROL_PAUSE_MASK) |
                    (reg_val & CSL_HTS_DMA336_SCHEDULER_CONTROL_STRM_EN_MASK) |
                    (reg_val & CSL_HTS_DMA336_SCHEDULER_CONTROL_PIPELINE_NUM_MASK) |
                    (reg_val & CSL_HTS_DMA336_SCHEDULER_CONTROL_SCH_EN_MASK);

                /* CONSUMER_CONTROL_DMA */
                reg_buf_ptr->nfHtsRegs.CONS_CONTROL_DMA[i] = CSL_REG32_RD(cons_dma_ctrl_addr + HWA_DMA_CONS_CONTROL_OFFSET);
            }
        }
    }

    return status;
}
int32_t Vhwa_m2mNfGetSl2Info(Vhwa_M2mNfSl2Info *sl2Info)
{
    int32_t retVal = FVID2_SOK;
    Vhwa_M2mNfInstObj *instObj = &gM2mNfInstObj;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */  
    if ((NULL == instObj) || (NULL == sl2Info))
    {
        retVal = FVID2_EBADARGS;
    }
    else if ((uint32_t)UFALSE == instObj->isSl2AllocDone)
    {
        /* SL2 memory not allocated yet */
        GT_0trace(VhwaNfTrace, GT_ERR,
            "SL2 Memory is not allocated. Call Vhwa_m2mNfAllocSl2() first.\n");
        retVal = FVID2_EFAIL;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        sl2Info->sl2Addr = instObj->sl2Prms.sl2StartAddr;
        sl2Info->sl2Size = instObj->sl2Prms.sl2MemSize;
    }

    return retVal;
}
