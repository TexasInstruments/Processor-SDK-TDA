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
 *  \file vhwa_m2mSdeApi.c
 *
 *  \brief API Implementation
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "src/drv/vhwa_m2mSdePriv.h"


/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
/** Channel Indexes for HTS Configuration */
#define HTS_SDE_REFE_FRAME_CH_IDX           (0U)
#define HTS_SDE_BASE_FRAME_CH_IDX           (1U)
#define HTS_SDE_OUT_CH_IDX                  (0U)

#if defined (VHWA_TEST_NEGATIVE)
#define TEST_SDE_WDTIMER_ERR_INTERRUPT
#endif

#if defined (VHWA_TEST_NEGATIVE_FOCO)
#define TEST_SDE_FOCO_WDTIMER_ERR_INTERRUPT
#endif

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */


/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static void Vhwa_sdeSetFocoParams(Vhwa_M2mSdeHandleObj *hObj,
                                     const Vhwa_M2mSdePrms *sdePrms);

static int32_t Vhwa_sdeEnableFoco(const Vhwa_M2mSdeInstObj *instObj,
                                  const Vhwa_M2mSdeHandleObj *hObj);

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief   Local Function to Allocate Handle Object from the
 *          pool of handle objects.
 *          No protection inside the function, Caller should protect
 *          the function call
 *
 * \param   instObj             Instance object.
 *
 * \return  pointer to handle object on success else NULL.
 *
 **/
static Vhwa_M2mSdeHandleObj *Vhwa_sdeAllocHandleObj(
                                const Vhwa_M2mSdeInstObj *instObj);

/**
 * \brief   Create Queues, required for storing pending requests
 *
 * \param   instObj             Instance object.
 *
 * \return  pointer to handle object on success else NULL.
 *
 **/
static int32_t Vhwa_sdeCreateQueues(Vhwa_M2mSdeInstObj *instObj);

/**
 * \brief   Delete Queues
 *
 * \param   instObj             Instance object.
 *
 **/
static void Vhwa_sdeDeleteQueues(Vhwa_M2mSdeInstObj *instObj);

/**
 * \brief   Local function to freeup allocated Handle Object and return it
 *          to the pool of free handle objects.
 *          No protection inside the function, Caller should protect
 *          the function call
 *
 * \param   hObj                Handle Object to be freed up.
 *
 **/
static void Vhwa_sdeFreeHandleObj(Vhwa_M2mSdeHandleObj *hObj);

/**
 * \brief   Function to check given SDE configuration, used in
 *          SET_PARAMS ioctl it first initializes LSE and HTS
 *          config and then uses CSLFL of LSE and HTS to validate
 *          the configuration
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   sdePrms             SDE configuration to be validated
 *
 * \return  FVID2_SOK if given SDE cofiguration is valid, error code otherwise.
 *
 **/
static int32_t Vhwa_sdeCheckCfg(const Vhwa_M2mSdeInstObj *instObj,
                                Vhwa_M2mSdeHandleObj *hObj,
                                const Vhwa_M2mSdePrms *sdePrms);

/**
 * \brief   Based on the given SDE config, it initializes LSE configuration.
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   sdePrms              SDE Configuration
 *
 **/
static void Vhwa_sdeSetLseCfg(Vhwa_M2mSdeHandleObj *hObj,
                              Vhwa_M2mSdePrms *sdePrms);

/**
 * \brief   Based on the given SDE config, it initializes HTS configuration.
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   sdePrms             SDE Configuration
 *
 **/
static void Vhwa_sdeSetHtsCfg(Vhwa_M2mSdeInstObj *instObj,
    Vhwa_M2mSdeHandleObj *hObj, Vhwa_M2mSdePrms *sdePrms);

/**
 * \brief   Calculate and store the Sl2 buffer parameters based on SDE
 *          configuration
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   sdePrms             SDE configuration to be set
 *
 **/
static int32_t Vhwa_sdeSetSl2Prms(const Vhwa_M2mSdeInstObj *instObj,
                                  Vhwa_M2mSdeHandleObj *hObj,
                                  Vhwa_M2mSdePrms *sdePrms);

/**
 * \brief   Set the HTS bandwidth limiter parameters
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   bwLimit             BW limiter configuration
 *
 **/
static void Vhwa_sdeSetHtsLimitParams(Vhwa_M2mSdeHandleObj *hObj,
                                      const Vhwa_HtsLimiter *htsLimit);

/**
 * \brief   Implementation of SET_PARAMS ioctl.
 *          It uses CheckSdeCfg to validate the config.
 *          If it is valid, copies the config into handle object
 *          If it is invalid, it reverts LSE/HTS config to known valid config
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   sdeCfg              SDE configuration to be set
 *
 **/
static int32_t Vhwa_sdeSetParams(Vhwa_M2mSdeInstObj *instObj,
    Vhwa_M2mSdeHandleObj *hObj, Vhwa_M2mSdePrms *sdePrms);

/**
 * \brief   Sets the error event parameters
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   eePrms              Error Event parameters
 *
 **/
static int32_t Vhwa_sdeSetEeParams(const Vhwa_M2mSdeInstObj *instObj,
                                   Vhwa_M2mSdeHandleObj *hObj,
                                   const Sde_ErrEventParams *eePrms);

/**
 * \brief   Sets the error event parameters
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   wdTimerEePrms       Watchdog Timer Error Event parameters
 *
 **/
static int32_t Vhwa_sdeSetWdTimerEeParams(const Vhwa_M2mSdeInstObj *instObj,
                                   Vhwa_M2mSdeHandleObj *hObj,
                                   const Sde_WdTimerErrEventParams *wdTimerEePrms);

/**
 * \brief   Calculate the TR paramters for the disparity output
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 *
 **/
static void Vhwa_sdeCalOutTrParams(Vhwa_M2mSdeHandleObj *hObj);

/**
 * \brief   Set Sl2 buffer parameters for SDE for given configuration
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   sdePrms             SDE configuration to be set
 *
 **/
static void Vhwa_sdeSetBuffParams(Vhwa_M2mSdeHandleObj *hObj);


/**
 * \brief   Minimal function to submit request to hw and start SDE operation.
 *          It first sets address in the TR, Submits it to the ring and
 *          starts the pipeline.
 *          It does not start currently SDE, HTS schedulers.
 *
 * \param   instObj             Instance Object, used for getting base address
 * \param   qObj                Queue Object, used for getting handle object
 *
 **/
static int32_t Vhwa_sdeSubmitRequest(Vhwa_M2mSdeInstObj *instObj,
                                     Vhwa_M2mSdeQueueObj *qObj);

/**
 * \brief   Function to configure the hardware register for new request
 *
 * \param   instObj             Instance Object, used for getting base address
 * \param   qObj                Queue Object, used for getting handle object
 *
 **/
static int32_t Vhwa_m2mSdeSetConfigInHW(const Vhwa_M2mSdeInstObj *instObj,
                                        const Vhwa_M2mSdeQueueObj *qObj);

/**
 * \brief   Set SDE and WDTIMER interrupt enable/disable in INTD registers.
 *
 * This function configures the interrupt enable/disable settings in the DMPAC INTD
 * registers for SDE and Watchdog Timer error events, based on the handle object's
 * error event parameters.
 *
 * \param   vhwaIrqNum      VHWA IRQ number to configure.
 * \param   intdRegs        Pointer to DMPAC INTD register overlay.
 * \param   hObj            Pointer to the SDE handle object containing error event params.
 *
 * \return  FVID2_SOK on success, else FVID2 error code.
 */
static int32_t Vhwa_m2mSdeSetIntrInHW(uint32_t vhwaIrqNum,
                                volatile CSL_dmpac_intd_cfgRegs  *intdRegs,
                                const Vhwa_M2mSdeHandleObj *hObj);

/* Implementation of FVID2 APIs */

/**
 * \brief   FVID2 Create Function.
 *
 * \param   instObj             Instance object.
 *
 * \return  FVID2 Driver Handle.
 *
 **/
Fdrv_Handle vhwa_m2mSdeCreate(UInt32 drvId, UInt32 drvInstId,
    Ptr createArgs, Ptr createStatusArgs, const Fvid2_DrvCbParams *cbPrms);

/**
 * \brief   FVID2 Delete Function.
 *
 * \param   handle              FVID2 driver handle.
 *
 * \return  FVID2_SOK on success, else FVID2 error code
 *
 **/
Int32 vhwa_m2mSdeDelete(Fdrv_Handle handle, Ptr deleteArgs);

/**
 * \brief   FVID2 Control Function.
 *
 * \param   handle              FVID2 driver handle.
 *
 * \return  FVID2_SOK on success, else FVID2 error code
 *
 **/
Int32 vhwa_m2mSdeControl(Fdrv_Handle handle, UInt32 cmd, Ptr cmdArgs,
    Ptr cmdStatusArgs);

/**
 * \brief   FVID2 Process Request function.
 *
 * \param   handle              FVID2 driver handle.
 *
 * \return  FVID2_SOK on success, else FVID2 error code
 *
 **/
Int32 vhwa_m2mSdeProcessReq(Fdrv_Handle handle, Fvid2_FrameList *inFrmList,
    Fvid2_FrameList *outFrmList, uint32_t timeout);

/**
 * \brief   FVID2 Get Process Request function.
 *
 * \param   handle              FVID2 driver handle.
 *
 * \return  FVID2_SOK on success, else FVID2 error code
 *
 **/
Int32 vhwa_m2mSdeGetProcessReq(Fdrv_Handle handle,
    Fvid2_FrameList *inFrmList, Fvid2_FrameList *outFrmList,
    UInt32 timeout);

#if !defined(SOC_J722S)
/**
 * \brief   Reconfigure SDE MMR registers as needed for the current handle/queue.
 *
 * \param   instObj   Instance Object.
 * \param   hObj      Handle Object.
 * \param   qObj      Queue Object.
 *
 * \return  FVID2_SOK on success, else FVID2 error code.
 */
static int32_t Vhwa_m2mSdeReconfigReinitReg(const Vhwa_M2mSdeInstObj *instObj,
                                        const Vhwa_M2mSdeHandleObj *hObj,
                                        const Vhwa_M2mSdeQueueObj *qObj);

/**
 * @brief Validates the SDE (Stereo Disparity Engine) status registers against expected values.
 *
 * This function compares the current SDE status register values with the expected values
 * provided in the 'expected' parameter. It uses the SoC-specific information from 'socInfo'
 * to perform the validation according to the hardware configuration.
 *
 * @param expected Pointer to a SdeStatusRegisterGroup structure containing the expected
 *                 status register values.
 * @param socInfo  Pointer to a Sde_SocInfo structure containing SoC-specific information
 *                 required for validation.
 *
 * @return Returns 0 (FVID2_SOK) if the status registers match the expected values,
 *         or a negative error code if there is a mismatch or an error occurs.
 */
int32_t Vhwa_m2mSdeStatusRegValidate(const SdeStatusRegisterGroup *expected, const Sde_SocInfo *socInfo);

/**
 * \brief   Updates the SdeStatusRegisterGroup structure in the handle object with expected status register values.
 *
 * \param   hObj    Pointer to the Vhwa_M2mSdeHandleObj whose statusRegs should be updated.
 */
static int32_t VhwaM2mSdeUpdateStatusRegGroup(Vhwa_M2mSdeHandleObj *hObj);
/**
 * @brief Sets the default golden register memory values for the SDE (Stereo Disparity Engine) module.
 *
 * This function initializes the register memory values to their default "golden" state
 * for the specified SDE handle and instance object. It is typically used to ensure
 * that the hardware registers are set to known, validated values before operation.
 *
 * @param hObj Pointer to the SDE handle object.
 * @param instObj Pointer to the SDE instance object.
 *
 * @return Returns 0 on success, or a negative error code on failure.
 */
int32_t vhwaM2mSdeSetDefaultGoldenRegMemValues(const Vhwa_M2mSdeHandleObj *hObj, const Vhwa_M2mSdeInstObj *instObj);
#endif

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
Vhwa_M2mSdeHandleObj gM2mSdeHandleObj[VHWA_M2M_SDE_MAX_HANDLES];
Vhwa_M2mSdeInstObj   gM2mSdeInstObj;

Fvid2_DrvOps gM2mSdeFvid2DrvOps = {
    FVID2_VHWA_M2M_SDE_DRV_ID,
    /**< Unique driver Id. */
    vhwa_m2mSdeCreate,
    /**< FVID2 create function pointer. */
    vhwa_m2mSdeDelete,
    /**< FVID2 delete function pointer. */
    vhwa_m2mSdeControl,
    /**< FVID2 control function pointer. */
    NULL, NULL,
    /**< FVID2 queue function pointer. */
    vhwa_m2mSdeProcessReq,
    /**< FVID2 process request function pointer. */
    vhwa_m2mSdeGetProcessReq,
    /**< FVID2 get processed request function pointer. */
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t Vhwa_m2mSdeInit(Vhwa_M2mSdeInitParams *initPrms)
{
    int32_t             status = FVID2_SOK;
    uint32_t            cnt;
    #if !defined(MCU_PLUS_SDK)
    SemaphoreP_Params   params;
    #endif
    Vhwa_M2mSdeInstObj *instObj = NULL;

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
        instObj = &gM2mSdeInstObj;

        /* Reset all instance object to 0x0 */
        Fvid2Utils_memset(instObj, 0U, sizeof (gM2mSdeInstObj));

        /* Mark pool flags as free */
        for (cnt = 0U; cnt < VHWA_M2M_SDE_MAX_HANDLES; cnt++)
        {
            gM2mSdeHandleObj[cnt].isUsed = (uint32_t) UFALSE;
        }

        /* Set HTS pipeline */
        instObj->pipeline = VHWA_M2M_SDE_HTS_PIPELINE;

        /* Initialize stalled flag to FALSE */
        instObj->isStalled = (uint32_t)UFALSE;

        Sde_getSocInfo((void *)&instObj->socInfo);

        status = Fvid2_registerDriver(&gM2mSdeFvid2DrvOps);
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
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            GT_0trace(VhwaSdeTrace, GT_ERR,
                "FVID2 driver registration failed!!\n");
            instObj->isRegistered = (uint32_t)UFALSE;
        }
        /* LDRA_JUSTIFY_END */

        /* Create Semaphores */
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
            /* Allocate lock semaphore */
            #if defined(MCU_PLUS_SDK)
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
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
                GT_0trace(VhwaSdeTrace, GT_ERR,
                    "Failed to allocate instance semaphore!!\n");
                status = FVID2_EALLOC;
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
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            /* Allocate lock semaphore */
            status = Vhwa_commonCreateHwaLock(VHWA_DMPAC_SDE_LOCK_IDX);
        }

        /* Create free and request queues */
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
            status = Vhwa_sdeCreateQueues(instObj);
        }

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
            /* Initialize UDMA, ie allocate and initialize channels
               required for SDE output */
            status = Vhwa_m2mSdeUdmaInit(instObj, initPrms);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This branch statement checks for errors accumulated in previous steps.
            Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK != status)
            {
                GT_0trace(VhwaSdeTrace, GT_ERR,
                    "UDMA Initialization Failed !!\n");
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
        /* Register ISR handler for the given irq number */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            instObj->irqNum = initPrms->irqInfo.irqNum;
            instObj->vhwaIrqNum = initPrms->irqInfo.vhwaIrqNum;

            status = Vhwa_m2mSdeRegisterIsr(instObj);
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
        /* Init is done, copy init params locally,
           enable DMPAC and SDE in DMPAC Top */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            Fvid2Utils_memcpy(&instObj->initPrms, initPrms,
                sizeof(Vhwa_M2mSdeInitParams));

            /* Enable SDE at DMPAC Top*/
            CSL_dmpacEnableModule(instObj->socInfo.dmpacCntlRegs,
                DMPAC_MODULE_SDE, (uint32_t)UTRUE);

            instObj->initDone = (uint32_t)UTRUE;
            instObj->lastHndlObj = NULL;
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
        Vhwa_m2mSdeDeInit();
    }
     /* LDRA_JUSTIFY_END */
    return (status);
}

void Vhwa_m2mSdeDeInit(void)
{
    Vhwa_M2mSdeInstObj *instObj = NULL;
    int32_t status = FVID2_SOK;

    instObj = &gM2mSdeInstObj;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The negative test framework and test apps cannot reach this portion.
    The DeInit function cannot be called while any handles are open through a test configuration.
    Effect on this unit: User is warned and the channel is closed. This is not treated as an error.
    <justification end> */
    if (instObj->openCnt > 0u)
    {
        GT_0trace(VhwaSdeTrace, GT_ERR,
            "Warning: All driver handles are not closed!!\n");
    }
    /* LDRA_JUSTIFY_END */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. An error print statement can be added in a future release if required.
    <justification end> */
    if ((uint32_t)UTRUE == instObj->isSl2AllocDone)
    /* LDRA_JUSTIFY_END */
    {
        status = Vhwa_m2mSdeFreeSl2();
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(FVID2_SOK != status)
        {
            GT_0trace(VhwaSdeTrace, GT_ERR,
                "SL2 Free Failed !!\n");
        }        
        /* LDRA_JUSTIFY_END */
    }
    /* Stop UDMA channels */
    (void)Vhwa_m2mSdeStopCh(instObj);

    Vhwa_m2mSdeUnregisterIsr(instObj);

    (void)Vhwa_m2mSdeUdmaDeInit(instObj);

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The negative test framework and test apps cannot reach this portion.
    Vhwa_m2mSdeInit registers the driver for all test configurations.
    Effect on this unit: If the driver is not registered, then no action is required.
    <justification end> */
    if ((uint32_t)UTRUE == instObj->isRegistered)
    /* LDRA_JUSTIFY_END */
    {
        (void)Fvid2_unRegisterDriver(&gM2mSdeFvid2DrvOps);
    }

    /* Delete the lock semaphore */
    #if defined(MCU_PLUS_SDK)
     SemaphoreP_destruct(&instObj->lock);
    #else
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The negative test framework and test apps cannot reach this portion.
    The instance object is unlocked before the DeInit call is made.
    Effect on this unit: If lock is already NULL, then no action is required.
    <justification end> */
    if (NULL != instObj->lock)
    /* LDRA_JUSTIFY_END */
    {
        (void)SemaphoreP_delete(instObj->lock);
        instObj->lock = NULL;
    }

    /* Delete the lock semaphore */
    #endif

    Vhwa_commonDeleteHwaLock(VHWA_DMPAC_SDE_LOCK_IDX);

    Vhwa_sdeDeleteQueues(instObj);

    /* Init all global variables to zero */
    Fvid2Utils_memset(instObj, 0U, sizeof (gM2mSdeInstObj));

    instObj->initDone = (uint32_t)UFALSE;
}

Fdrv_Handle vhwa_m2mSdeCreate(UInt32 drvId, UInt32 drvInstId,
    Ptr createArgs, Ptr createStatusArgs, const Fvid2_DrvCbParams *cbPrms)
{
    int32_t                 status = FVID2_SOK;
    Vhwa_M2mSdeInstObj     *instObj = NULL;
    Vhwa_M2mSdeHandleObj   *hObj = NULL;
    Vhwa_M2mSdeCreateArgs  *sdeCreateArgs = NULL;
    Fdrv_Handle             handle = NULL;
    Sde_SocInfo            *socInfo = NULL;
    Vhwa_M2mSdeSl2AllocPrms sl2AllocPrms;

    instObj = &gM2mSdeInstObj;
    socInfo = &instObj->socInfo;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    /* Check for errors */
    if ((FVID2_VHWA_M2M_SDE_DRV_ID != drvId) ||
        (VHWA_M2M_SDE_DRV_INST_ID != drvInstId) ||
        (NULL == createArgs) ||
        (NULL == cbPrms))
    {
        GT_0trace(VhwaSdeTrace, GT_ERR, "NULL Pointer !!\n");
        status = FVID2_EBADARGS;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        /* Open not allowed if init is not done */
        if ((uint32_t)UFALSE == instObj->initDone)
        {
            GT_0trace(VhwaSdeTrace, GT_ERR,
                "Vhwa_m2mSdeInit is not called\n");
            status  = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */
        sdeCreateArgs = (Vhwa_M2mSdeCreateArgs *)createArgs;
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
        hObj = Vhwa_sdeAllocHandleObj(instObj);

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The number of handles is expected to be pre-validated in the imaging layer before reaching vhwa.
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
                    GT_0trace(VhwaSdeTrace, GT_DEBUG,
                    "Vhwa_m2mSdeAllocSl2 is not called, allocating for default\n");

                    Vhwa_M2mSdeSl2AllocPrmsInit(&sl2AllocPrms);

                    status = Vhwa_m2mSdeAllocSl2(&sl2AllocPrms);
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
                    /* Start UDMA Channels on the first handle Create */
                    status = Vhwa_m2mSdeStartCh(instObj);
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
                    /* Enable Pipeline interrupt in INTD */
                    Vhwa_enableDmpacHtsIntr(socInfo->dmpacIntdRegs,
                        instObj->vhwaIrqNum, instObj->pipeline);
                }
            }

            hObj->sl2Prms.sl2OutBuffDepth = VHWA_M2M_SDE_MIN_OUT_BUFF_DEPTH;

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This branch statement checks for errors accumulated in previous steps.
            Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            /* Create doneQ, Done Q is specific to handle so that
             * FVID2_Dequeue returns correct request for that handle. */
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
                status = Vhwa_m2mSdeAllocUdmaMem(hObj);
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
                Fvid2Utils_memcpy(&hObj->createArgs, sdeCreateArgs,
                    sizeof(Vhwa_M2mSdeCreateArgs));

                Fvid2Utils_memcpy(&hObj->cbPrms, cbPrms,
                    sizeof (hObj->cbPrms));

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
                Vhwa_sdeFreeHandleObj(hObj);
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

Int32 vhwa_m2mSdeDelete(Fdrv_Handle handle, Ptr deleteArgs)
{
    int32_t                 status = FVID2_SOK;
    Vhwa_M2mSdeInstObj     *instObj = NULL;
    Vhwa_M2mSdeHandleObj   *hObj = NULL;
    Sde_SocInfo            *socInfo = NULL;

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
        instObj = &gM2mSdeInstObj;
        hObj    = (Vhwa_M2mSdeHandleObj *)handle;
        socInfo = &instObj->socInfo;

        /* Take the instance semaphore */
        (void)VhwaAl_SemaphoreP_pend(instObj->lock, SYSTEMP_WAIT_FOREVER);

        /* Check if all handles are already closed */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The negative test framework and test apps cannot reach this portion.
        The application will not reach the Delete function without any open handle through the existing test framework.
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
        Rationale: The negative test framework and test apps cannot reach this portion.
        The application will not reach the Delete function without any open handle through the existing test framework. This will result in the status always being FVID2_SOK.
        The application will not reach the Delete function with any pending request on the handle through the existing test framework.
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
        Rationale: The negative test framework and test apps cannot reach this portion.
        The application will not reach the Delete function without any open handle through the existing test framework. This will result in the status always being FVID2_SOK.
        The application will not reach the Delete function with any pending request on the handle through the existing test framework.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            Vhwa_sdeFreeHandleObj(hObj);

            instObj->openCnt --;

            /* Delete the Done queue */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The negative test framework and test apps cannot reach this portion.
            No handle will have a NULL doneQ.
            Effect on this unit: If the queue is already NULL, no action is required.
            <justification end> */
            if (NULL != hObj->doneQ)
            /* LDRA_JUSTIFY_END */
            {
                Fvid2Utils_destructQ(hObj->doneQ);
                hObj->doneQ = NULL;
            }

            /* for the last close, stop all channels and
             * disable HTS interrupt in INTD */
            if (0U == instObj->openCnt)
            {
                /* Disable HTS Interrupt */
                Vhwa_disableDmpacHtsIntr(socInfo->dmpacIntdRegs,
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

Int32 vhwa_m2mSdeControl(Fdrv_Handle handle, UInt32 cmd, Ptr cmdArgs,
    Ptr cmdStatusArgs)
{
    int32_t                 status = FVID2_SOK;
    Vhwa_M2mSdeInstObj     *instObj = NULL;
    Vhwa_M2mSdeHandleObj   *hObj = NULL;
    Vhwa_M2mSdePrms        *sdePrms = NULL;
    Sde_ErrEventParams     *eePrms = NULL;
    Vhwa_HtsLimiter        *htsLimit = NULL;
    Sde_WdTimerErrEventParams *wdTimerEePrms = NULL;
#if !defined(SOC_J722S)
    uint32_t                cookie;
#endif

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
    else
    /* LDRA_JUSTIFY_END */
    {
        instObj = &gM2mSdeInstObj;
        hObj = (Vhwa_M2mSdeHandleObj *)handle;
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
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameter is expected to be pre-validated in the imaging layer before reaching vhwa.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        switch (cmd)
        {
        /* LDRA_JUSTIFY_END */
            /* Set SDE Parameters */
            case VHWA_M2M_IOCTL_SDE_SET_PARAMS:
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
                    sdePrms = (Vhwa_M2mSdePrms *)cmdArgs;
                    status = Vhwa_sdeSetParams(instObj, hObj, sdePrms);
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
                    GT_0trace(VhwaSdeTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_SDE_SET_PARAMS !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            /* Enable Register error events callback */
            case VHWA_M2M_IOCTL_SDE_REGISTER_ERR_CB:
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
                    eePrms = (Sde_ErrEventParams *)cmdArgs;
                    status = Vhwa_sdeSetEeParams(instObj, hObj, eePrms);
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
                    GT_0trace(VhwaSdeTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_SDE_REGISTER_ERR_CB !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            /* SET HTS Limiter config */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications CAN reach this portion.
                        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                    However, due to the stated rationale, this is not tested.
            <justification end> */
            case VHWA_M2M_IOCTL_SDE_SET_HTS_LIMIT:
            {
                if (NULL != cmdArgs)
                {
                    htsLimit = (Vhwa_HtsLimiter *)cmdArgs;

                    Vhwa_sdeSetHtsLimitParams(hObj, htsLimit);
                }
                else
                {
                    GT_0trace(VhwaSdeTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_SDE_SET_HTS_LIMIT !!\n");
                    status = FVID2_EBADARGS;
                }
                break;
            }
            /* LDRA_JUSTIFY_END */

            /* Get PSA Signature */
            case VHWA_M2M_IOCTL_SDE_GET_PSA_SIGN:
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
                    status = CSL_sdeGetPsa(instObj->socInfo.sdeRegs, (uint32_t *)cmdArgs);
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
                    GT_0trace(VhwaSdeTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_SDE_GET_PSA_SIGN !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            /* Get Histogram */
            case VHWA_M2M_IOCTL_SDE_GET_HISTOGRAM:
            {
                uint32_t *csHistogram = NULL;
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
                    csHistogram = (uint32_t *)cmdArgs;
                    Fvid2Utils_memcpy(csHistogram, hObj->hist, sizeof(uint32_t) * DMPAC_SDE_MAX_HISTOGRAM_SIZE);
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
                    GT_0trace(VhwaSdeTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_SDE_GET_HISTOGRAM !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            /* Get performance */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications CAN reach this portion.
                        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                    However, due to the stated rationale, this is not tested.
            <justification end> */
            case VHWA_M2M_IOCTL_SDE_GET_PERFORMANCE:
            {
                uint32_t *perf = NULL;
                if (NULL != cmdArgs)
                {
                    perf = (uint32_t *)cmdArgs;
                    *perf = (uint32_t)hObj->perfNum;
                }
                else
                {
                    GT_0trace(VhwaSdeTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_SDE_GET_PERFORMANCE !!\n");
                    status = FVID2_EBADARGS;
                }
                break;
            }

            case VHWA_M2M_IOCTL_SDE_SYNC_START:
            {
                if (NULL != hObj->createArgs.getTimeStamp)
                {
                    hObj->perfNum = hObj->createArgs.getTimeStamp();
                }

                status = CSL_dmpacHtsPipelineStart(instObj->socInfo.htsRegs, &hObj->htsCfg);
                break;
            }
            /* LDRA_JUSTIFY_END */
            /* Enable Register Wdtimer error events callback */
            case VHWA_M2M_IOCTL_SDE_REGISTER_WDTIMER_ERR_CB:
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
                    wdTimerEePrms = (Sde_WdTimerErrEventParams *)cmdArgs;
                    status = Vhwa_sdeSetWdTimerEeParams(instObj, hObj, wdTimerEePrms);
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
                    GT_0trace(VhwaSdeTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_SDE_REGISTER_WDTIMER_ERR_CB !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }
#if !defined(SOC_J722S)
            case VHWA_M2M_IOCTL_SDE_ENABLE_RECONFIG_REINIT_REG:
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
                    uint32_t modeVal = *((uint32_t *)cmdArgs);
                    if (modeVal > VHWA_SAFETY_MODE_CONTINUOUS)
                    {
                        GT_1trace(VhwaSdeTrace, GT_ERR,
                            "Invalid safety mode value %u for IOCTL_VHWA_M2M_SDE_ENABLE_RECONFIG_REG !!\n", modeVal);
                        status = FVID2_EBADARGS;
                    }
                    else
                    {
                        hObj->enableReconfigReinitReg = modeVal;
                        status = FVID2_SOK;
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
                    GT_0trace(VhwaSdeTrace, GT_ERR,
                        "handle/cmdArgs cannot be NULL for IOCTL_VHWA_M2M_SDE_ENABLE_RECONFIG_REG !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case VHWA_M2M_IOCTL_SDE_ENABLE_STATUS_REG_VALIDATE:
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
                    uint32_t modeVal = *((uint32_t *)cmdArgs);
                    if (modeVal > VHWA_SAFETY_MODE_CONTINUOUS)
                    {
                        GT_1trace(VhwaSdeTrace, GT_ERR,
                            "Invalid safety mode value %u for VHWA_M2M_IOCTL_SDE_ENABLE_STATUS_REG_VALIDATE !!\n", modeVal);
                        status = FVID2_EBADARGS;
                    }
                    else
                    {
                        /* Disable Interrupt since the below flag is accessed in the ISR */
                        cookie = HwiP_disable();
                        hObj->enableStatusRegValidate = modeVal;
                        HwiP_restore(cookie);
                        status = FVID2_SOK;
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
                    GT_0trace(VhwaSdeTrace, GT_ERR,
                        "handle/cmdArgs cannot be NULL for VHWA_M2M_IOCTL_SDE_ENABLE_STATUS_REG_VALIDATE !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case VHWA_M2M_IOCTL_SDE_VALIDATE_REG:
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the caller before the control reaches here.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != handle)
                /* LDRA_JUSTIFY_END */
                {
                    if (VHWA_SAFETY_MODE_DISABLED != hObj->enableStatusRegValidate)
                    {
                        status = Vhwa_m2mSdeStatusRegValidate(&hObj->statusRegs, &instObj->socInfo);
                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        The parameters are pre-validated by the caller before the control reaches here.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if (FVID2_SOK != status)
                        {
                            GT_0trace(VhwaSdeTrace, GT_ERR, "DMPAC-10: StatusRegValidate failed!\n");
                        }
                        /* LDRA_JUSTIFY_END */
                    }
                    if (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate)
                    {
                        status = Vhwa_m2mSdeConfigRegReadback(hObj, instObj);
                    }

                    /* Release the hardware lock after the configReg readback or statusReg */
                    if ((VHWA_SAFETY_MODE_DISABLED != hObj->enableStatusRegValidate) ||
                        (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate))
                    {
                        Vhwa_commonHwaLockRelease(VHWA_DMPAC_SDE_LOCK_IDX);
                    }

                    /* Reset the statusReg validate flag only in one-shot mode */
                    if (VHWA_SAFETY_MODE_ONESHOT == hObj->enableStatusRegValidate)
                    {
                        cookie = HwiP_disable();
                        hObj->enableStatusRegValidate = (uint32_t)VHWA_SAFETY_MODE_DISABLED;
                        HwiP_restore(cookie);
                    }

                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    This failure case is out of scope for the imaging test framework.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if ((VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate) && (FVID2_SOK == status))
                    /* LDRA_JUSTIFY_END */
                    {
                        /* Callback with goldenReg and ReadBackReg pointer to application for memory comparison */
                        if((NULL != hObj->configRegMemPrms.cbFxn) &&
                            (NULL != hObj->configRegMemPrms.configGoldenRegPtr) &&
                            (NULL != hObj->configRegMemPrms.configRegReadbackPtr))
                        {
                            Vhwa_M2mSdeConfigRegMemParams configRegMemPtrs;
                            configRegMemPtrs.golden_ptr = hObj->configRegMemPrms.configGoldenRegPtr;
                            configRegMemPtrs.readback_ptr = hObj->configRegMemPrms.configRegReadbackPtr;
                            configRegMemPtrs.readback_mem_size = sizeof(VhwaDmpacSdeSocReadBack);
                            configRegMemPtrs.appData = (void *)hObj->configRegMemPrms.appData;

                            status = hObj->configRegMemPrms.cbFxn((Fvid2_Handle)hObj, &configRegMemPtrs);
                        }
                        else
                        {
                            GT_0trace(VhwaSdeTrace, GT_ERR, "ConfigRegMemParams are NULL!\n");
                            status = FVID2_EFAIL;
                        }

                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        This failure case is out of scope for the imaging test framework.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        /* Get status from callback for memory comparison */
                        if (FVID2_SOK != status)
                        {
                            GT_0trace(VhwaSdeTrace, GT_ERR, "DMPAC-7: ConfigRegReadbackCallback Memory Comparison failed!\n");
                        }
                        /* LDRA_JUSTIFY_END */

                        /* Reset the configReg validate flag only in one-shot mode */
                        if (VHWA_SAFETY_MODE_ONESHOT == hObj->enableConfigRegValidate)
                        {
                            cookie = HwiP_disable();
                            hObj->enableConfigRegValidate = (uint32_t)VHWA_SAFETY_MODE_DISABLED;
                            HwiP_restore(cookie);
                        }
                    }
                }
                else
                {
                    GT_0trace(VhwaSdeTrace, GT_ERR,
                        "handle cannot be NULL for VHWA_M2M_IOCTL_SDE_VALIDATE_REG !!\n");
                    status = FVID2_EBADARGS;
                }
                break;
            }

            case VHWA_M2M_IOCTL_SDE_ENABLE_CONFIG_REG_READBACK:
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the caller before the control reaches here.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((NULL != handle) && (NULL != cmdArgs))
                /* LDRA_JUSTIFY_END */
                {
                    uint32_t modeVal = *((uint32_t *)cmdArgs);
                    if (modeVal > VHWA_SAFETY_MODE_CONTINUOUS)
                    {
                        GT_1trace(VhwaSdeTrace, GT_ERR,
                            "Invalid safety mode value %u for VHWA_M2M_IOCTL_SDE_ENABLE_CONFIG_REG_READBACK !!\n", modeVal);
                        status = FVID2_EBADARGS;
                    }
                    else
                    {
                        cookie = HwiP_disable();
                        hObj->enableConfigRegValidate = modeVal;
                        HwiP_restore(cookie);
                        status = FVID2_SOK;
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
                    GT_0trace(VhwaSdeTrace, GT_ERR,
                        "handle/cmdArgs cannot be NULL for VHWA_M2M_IOCTL_SDE_ENABLE_CONFIG_REG_READBACK !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case VHWA_M2M_IOCTL_SDE_GET_READBACK_SIZE:
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the caller before the control reaches here.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((NULL != handle) && (NULL != cmdArgs))
                /* LDRA_JUSTIFY_END */
                {
                    uint32_t *readbackSize = (uint32_t *)cmdArgs;
                    *readbackSize = (uint32_t)sizeof(VhwaDmpacSdeSocReadBack);
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
                    GT_0trace(VhwaSdeTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_SDE_GET_READBACK_SIZE !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case VHWA_M2M_IOCTL_SDE_SET_CONFIG_REG_PTRS:
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the caller before the control reaches here.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((NULL != handle) && (NULL != cmdArgs))
                /* LDRA_JUSTIFY_END */
                {
                    Vhwa_M2mSdeConfigRegMemParams *reg_ptrs = (Vhwa_M2mSdeConfigRegMemParams *)cmdArgs;
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    The parameters are pre-validated by the caller before the control reaches here.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if ((reg_ptrs->readback_ptr == NULL) ||
                        (reg_ptrs->golden_ptr == NULL) ||
                        (reg_ptrs->cbFxn == NULL))
                    {
                        GT_0trace(VhwaSdeTrace, GT_ERR,
                            "readback_ptr, golden_ptr, cbFxn cannot be NULL for VHWA_M2M_IOCTL_SDE_SET_REG_READBACK_PTR !!\n");
                        status = FVID2_EBADARGS;
                    }
                    /* LDRA_JUSTIFY_END */
                    else
                    {
                        hObj->configRegMemPrms.configRegReadbackPtr = reg_ptrs->readback_ptr;
                        hObj->configRegMemPrms.configGoldenRegPtr = reg_ptrs->golden_ptr;
                        hObj->configRegMemPrms.cbFxn = reg_ptrs->cbFxn;
                        hObj->configRegMemPrms.appData = reg_ptrs->appData;
                        status = vhwaM2mSdeSetDefaultGoldenRegMemValues(hObj, instObj);
                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        This failure case is out of scope for the imaging test framework.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if(FVID2_SOK != status)
                        {
                            GT_0trace(VhwaSdeTrace, GT_ERR,
                                "Failed to set default goldenReg mem values for VHWA_M2M_IOCTL_SDE_SET_REG_READBACK_PTR !!\n");
                        }
                        /* LDRA_JUSTIFY_END */
                    }
                }
                else
                {
                    GT_0trace(VhwaSdeTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_SDE_SET_REG_READBACK_PTR !!\n");
                    status = FVID2_EBADARGS;
                }
                break;
            }
#endif
            /* Default Case */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameter is expected to be pre-validated in the imaging layer before reaching vhwa.
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

Int32 vhwa_m2mSdeProcessReq(Fdrv_Handle handle, Fvid2_FrameList *inFrmList,
    Fvid2_FrameList *outFrmList, uint32_t timeout)
{
    int32_t               status = FVID2_SOK;
    uint32_t              semTimeout;
    Vhwa_M2mSdeInstObj   *instObj = NULL;
    Vhwa_M2mSdeHandleObj *hObj = NULL;
    Vhwa_M2mSdeQueueObj  *qObj = NULL;
    VhwaDmpacSdeSocReadBack *goldenRegVal = NULL;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated in the imaging layer before reaching vhwa.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == handle) || (NULL == inFrmList) || (NULL == outFrmList))
    {
        status = FVID2_EBADARGS;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        instObj = &gM2mSdeInstObj;
        hObj = (Vhwa_M2mSdeHandleObj *)handle;

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        /* Check if IP is in stalled state due to error interrupt or watchdog timeout */
        if ((uint32_t)UTRUE == instObj->isStalled)
        {
            status = FVID2_EFAIL;
            GT_0trace(VhwaSdeTrace, GT_ERR,
                "Process request rejected: IP is in stalled state due to previous error\n");
        }
        /* LDRA_JUSTIFY_END */
        goldenRegVal = hObj->configRegMemPrms.configGoldenRegPtr;
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
        The parameter is expected to be pre-validated in the imaging layer before reaching vhwa.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((NULL == inFrmList->frames[SDE_INPUT_REFERENCE_IMG]) ||
            (0U == inFrmList->frames[SDE_INPUT_REFERENCE_IMG]->addr[0U]) ||
            (NULL == inFrmList->frames[SDE_INPUT_BASE_IMG]) ||
            (0U == inFrmList->frames[SDE_INPUT_BASE_IMG]->addr[0U]) ||
            (NULL == outFrmList->frames[0U]) ||
            (0U == outFrmList->frames[0u]->addr[0U]))
        {
            status = FVID2_EBADARGS;
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
        Timeout is always FVID2_TIMEOUT_FOREVER, pre-validated from target tivxDmpacSdeProcess function
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
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
        Timeout is always FVID2_TIMEOUT_FOREVER, pre-validated from target tivxDmpacSdeProcess function
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
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
        status = Vhwa_commonHwaLockAquire(VHWA_DMPAC_SDE_LOCK_IDX, semTimeout);

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
            /* Get a queue object from the free queue,
             * No need to protect from ISR as it is not accessed from ISR */
            qObj = (Vhwa_M2mSdeQueueObj *) Fvid2Utils_dequeue(instObj->freeQ);

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
                GT_0trace(VhwaSdeTrace, GT_ERR,
                    "Failed to Free Queue Object!!\n");
                status = FVID2_EALLOC;
            }
            else
            /* LDRA_JUSTIFY_END */
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
            Vhwa_m2mSdeSetAddress(qObj->hObj, &qObj->inFrmList,
                                    &qObj->outFrmList);
            /* HW is free, submit request to the hardware */
            /* If previous handle and current handles are same, */
            if (instObj->lastHndlObj != qObj->hObj)
            {
                /** Last handle was not same as new handle,
                 *  so Buffer Parameters needs to be configured */
                status = CSL_sdeSetBufParam(instObj->socInfo.sdeRegs,
                                                &hObj->sdeBuffPrms);

                if ((VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate) && (NULL != goldenRegVal))
                {
                    (void)CSL_sdeSetBufParam(&(goldenRegVal->sdeRegs), &hObj->sdeBuffPrms);
                }
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The negative test framework and test apps cannot reach this portion.
                The CSL call returns failure if and only if either of the arguments passed is NULL.
                We cannot make the arguments being passed here NULL through the test framework.
                Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if(FVID2_SOK == status)
                /* LDRA_JUSTIFY_END */
                {
                    status = Vhwa_m2mSdeSetConfigInHW(instObj, qObj);
                }
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
                /* Enable Selected HW Interrupts*/
                status = Vhwa_m2mSdeSetIntrInHW(instObj->vhwaIrqNum, instObj->socInfo.dmpacIntdRegs, hObj);
            }

#if !defined(SOC_J722S)
            /* Invoke the reconfig-MMR if enableReconfigReinitReg enabled for the current handle */
            if (VHWA_SAFETY_MODE_DISABLED != hObj->enableReconfigReinitReg)
            {
                status = Vhwa_m2mSdeReconfigReinitReg(instObj, hObj, qObj);
                /* Reset only in one-shot mode */
                if (VHWA_SAFETY_MODE_ONESHOT == hObj->enableReconfigReinitReg)
                {
                    hObj->enableReconfigReinitReg = (uint32_t)VHWA_SAFETY_MODE_DISABLED;
                }
            }
#endif

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The negative test framework and test apps cannot reach this portion
            The test framework does not have access to instObj and qObj to make this call return failure
            Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK == status)
            /* LDRA_JUSTIFY_END */
            {
                /** Set the addresses, Configure SDE/HTS, Submit the TR
                 *  Start the pipeline */
                status = Vhwa_sdeSubmitRequest(instObj, qObj);
            }
        }
    }

    return (status);
}

/** \brief Typedef for FVID2 get processed frames function pointer. */
Int32 vhwa_m2mSdeGetProcessReq(Fdrv_Handle handle,
    Fvid2_FrameList *inFrmList, Fvid2_FrameList *outFrmList,
    UInt32 timeout)
{
    int32_t                status = FVID2_SOK;
    uint32_t               cookie;
    Vhwa_M2mSdeInstObj    *instObj = NULL;
    Vhwa_M2mSdeHandleObj  *hObj = NULL;
    Vhwa_M2mSdeQueueObj   *qObj = NULL;

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
    else
    /* LDRA_JUSTIFY_END */
    {
        instObj = &gM2mSdeInstObj;
        hObj = (Vhwa_M2mSdeHandleObj *)handle;
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated in the imaging layer before reaching vhwa.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Take the instance semaphore, so that no other
         * handle can submit request from the task context. */
        (void)VhwaAl_SemaphoreP_pend(instObj->lock, SYSTEMP_WAIT_FOREVER);
#if defined (MCU_PLUS_SDK)
        uint32_t               statusQ;
        do
        {
            statusQ = Fvid2Utils_isQEmpty(hObj->doneQ);
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is NOT expected to result in an error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        while(statusQ == TRUE);
        /* LDRA_JUSTIFY_END */
#endif
        /* Disable interrupts before accessing queue */
        cookie = HwiP_disable();

        /* Dequeue the completed request from done queue */
        qObj = (Vhwa_M2mSdeQueueObj *) Fvid2Utils_dequeue(hObj->doneQ);

        /* Restore interrupts after updating node list */
        HwiP_restore(cookie);

        /* No buffers in the output queue */
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
            /* Check if requests are pending with driver */
            if (0U == hObj->numPendReq)
            {
                /* Nothing is queued */
                GT_0trace(VhwaSdeTrace, GT_DEBUG,
                    "Nothing to dequeue. No request pending with driver!!\r\n");
                status = FVID2_ENO_MORE_BUFFERS;
            }
            else
            {
                /* If no request have completed, return try again */
                GT_0trace(VhwaSdeTrace, GT_DEBUG,
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

            /* Dequeue from Rings */
#if !defined (MCU_PLUS_SDK)
            status = Vhwa_m2mSdePopRings(instObj, hObj);
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if(FVID2_SOK != status)
            {
                GT_0trace(VhwaSdeTrace, GT_ERR,
                    "Failed to dequeue from Rings!!\n");
            }
            /* LDRA_JUSTIFY_END */
#endif
            /* Return back the queue object to the free queue,
             * No need to protect from ISR, as it is not accessed in ISR */
            Fvid2Utils_queue(instObj->freeQ, &qObj->qElem, qObj);

            /* Read Histogram and locally store it in handle object */
            status = CSL_sdeGetCsHistogram(instObj->socInfo.sdeRegs, &hObj->hist[0u]);

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if(FVID2_SOK != status)
            {
                GT_0trace(VhwaSdeTrace, GT_ERR,
                    "Failed to get histogram from HW!!\n");
            }
            /* LDRA_JUSTIFY_END */

            /* Decrement the pending request count. */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The negative test framework and test apps cannot reach this portion.
            No test case configuration will lead to there being an object in the done queue of a handle when the handle has no pending request.
            Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
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
                GT_0trace(VhwaSdeTrace, GT_DEBUG,
                    "Done queue of the handle is not empty when there are is no pending request on the handle!!\r\n");
                status = FVID2_EFAIL;
            }
            /* LDRA_JUSTIFY_END */
        }
        /* New request can now be submitted to the SDE IP for non-readBack of StatusRegisters/ConfigurationRegisters enabled frames */
        if ((VHWA_SAFETY_MODE_DISABLED == hObj->enableStatusRegValidate) &&
            (VHWA_SAFETY_MODE_DISABLED == hObj->enableConfigRegValidate))
        {
            Vhwa_commonHwaLockRelease(VHWA_DMPAC_SDE_LOCK_IDX);
        }

        (void)VhwaAl_SemaphoreP_post(instObj->lock);
    }

    return (status);
}

int32_t Vhwa_m2mSdeAllocSl2(const Vhwa_M2mSdeSl2AllocPrms *sl2AllocPrms)
{
    int32_t status = FVID2_SOK;
    uint32_t pitchSl2;
    uint32_t imgMem, outBuffMem, rcBuffMem, focoBuffMem, tMemReq;
    Vhwa_M2mSdeInstObj *instObj = NULL;

    instObj = &gM2mSdeInstObj;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated in the imaging layer before reaching vhwa.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    /* Cannot even lock, if init is not done */
    if ((uint32_t)UFALSE == instObj->initDone)
    {
        GT_0trace(VhwaSdeTrace, GT_ERR,
            "Driver init is not done!!\n");
        status = FVID2_EFAIL;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The negative test framework and test apps cannot reach this portion.
        The values of sl2AllocPrms and instObj->isSl2AllocDone are determined by the application and are not directly controllable in automated tests.
            Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
            However, due to the stated rationale, this is not tested.
            <justification end> */
        /* Still need to check if provided sl2AllocPrms is not null and SL2 is
         * not allocated */
        if ((NULL == sl2AllocPrms) || (UTRUE == instObj->isSl2AllocDone))
        {
            status = FVID2_EBADARGS;
        }
        /* LDRA_JUSTIFY_END */
    }

    /* Lock instance semaphore */
    (void)VhwaAl_SemaphoreP_pend(instObj->lock, SYSTEMP_WAIT_FOREVER);

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The negative test framework and test apps cannot reach this portion.
    The values of sl2AllocPrms and instObj->isSl2AllocDone are determined by the application and are not directly controllable in automated tests.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Ref and Base Memory Required */
        /* Image data in SL2 should always starts at a 64 byte aligned boundary*/
        /* If width is multiple of 256 to alivate peak SL2 usage Line
           staggering is required */
        pitchSl2 = Vhwa_calcHorzSizeInBytes(sl2AllocPrms->maxImgWidth,
                                            FVID2_CCSF_BITS12_PACKED);
        pitchSl2 = Vhwa_calcSl2Pitch(pitchSl2);

        imgMem = pitchSl2 * SDE_SL2_REF_BASE_IMG_CIR_BUFF_SIZE;

        /* Set Out buffer Parameters */
        outBuffMem = SDE_PER_OUT_BUFF_SIZE * sl2AllocPrms->disBuffDepth;

        /* Set Row cost Buffer Parameters */
        rcBuffMem= (sl2AllocPrms->maxImgWidth/8u)*2u*8u*8u;

        if(sl2AllocPrms->searchRange == SDE_SR_192)
        {
            rcBuffMem = rcBuffMem * 3u;
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The negative test framework and test apps cannot reach this portion. The search range is hardcoded to achieve maximum memory allocation.
        Effect on this unit: Unimplemented feature. The SL2 memory allocation might shift from maximum allocation to flexible allocation based on input format.
        <justification end> */
        else if(sl2AllocPrms->searchRange == SDE_SR_128)
        {
            rcBuffMem = rcBuffMem * 2u;
        }
        else
        {
            /* Nothing to do here */
        }
        /* LDRA_JUSTIFY_END */

        /* Set FOCO Parameters */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The negative test framework and test apps cannot reach this portion. The input format is hardcoded to achieve maximum memory allocation.
        Effect on this unit: Unimplemented feature. The SL2 memory allocation might shift from maximum allocation to flexible allocation based on input format.
        <justification end> */
        if(FVID2_CCSF_BITS12_PACKED == sl2AllocPrms->inCcsf)
        {
            focoBuffMem = 0;
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            focoBuffMem = Vhwa_calcHorzSizeInBytes(sl2AllocPrms->maxImgWidth,
                                                   sl2AllocPrms->inCcsf);
            focoBuffMem = Vhwa_calcSl2Pitch(focoBuffMem);
            focoBuffMem = focoBuffMem * 2u;
        }

        tMemReq = (imgMem*2u) + outBuffMem + rcBuffMem + (focoBuffMem*2u);

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        /* Apply 16KB alignment for size and address if requested for firewall support */
        if ((uint32_t)TRUE == sl2AllocPrms->enable16KBAlignment)
        {
            tMemReq = ((tMemReq + 0x3FFFu) & ~0x3FFFu);
            instObj->sl2StartAddr = Vhwa_allocateSl2Aligned(tMemReq, VHWA_SL2_INST_DMPAC, 0x4000u);
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
            instObj->sl2StartAddr = Vhwa_allocateSl2(tMemReq, VHWA_SL2_INST_DMPAC);
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
        if(0u == instObj->sl2StartAddr)
        {
            GT_0trace(VhwaSdeTrace, GT_ERR,
                "SL2 aligned allocation failed (check alignment=0x4000 is power of 2 and sufficient SL2 memory available)\n");
            status = FVID2_EALLOC;
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            instObj->sl2Size = tMemReq;
            status = FVID2_SOK;
        }
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The negative test framework and test apps cannot reach this portion.
    The maximum memory allocation cannot be exceeded because all SL2 memory parameters are internally managed and not exposed to external configuration.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        instObj->isSl2AllocDone = (uint32_t)UTRUE;
    }

    /* Release instance semaphore */
    (void)VhwaAl_SemaphoreP_post(instObj->lock);

    return (status);
}


int32_t Vhwa_m2mSdeFreeSl2(void)
{
    int32_t retVal = FVID2_SOK;
    Vhwa_M2mSdeInstObj *instObj = NULL;

    instObj = &gM2mSdeInstObj;
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. An error print statement can be added in a future release if required.
    <justification end> */
    if (UTRUE == instObj->isSl2AllocDone)
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. An error print statement can be added in a future release if required.
        <justification end> */
        if(0U == instObj->openCnt)
        /* LDRA_JUSTIFY_END */
        {
            /* Lock instance semaphore */
            (void)VhwaAl_SemaphoreP_pend(instObj->lock, SYSTEMP_WAIT_FOREVER);

            Vhwa_FreeSl2(VHWA_SL2_INST_DMPAC);

            instObj->isSl2AllocDone = (uint32_t)UFALSE;
            instObj->sl2Size        = 0u;
            instObj->sl2StartAddr   = 0u;

            /* Release instance semaphore */
            (void)VhwaAl_SemaphoreP_post(instObj->lock);
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
            retVal = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */
    }

    return (retVal);
}

#if !defined(SOC_J722S)
int32_t Vhwa_m2mSdeReInit(void)
{
    int32_t                status = FVID2_SOK;
    Vhwa_M2mSdeInstObj    *instObj = NULL;

    /** No lock is required, as reinit is called only when HW is not accessed by any other
     *  handle, so it is ok to access the members here. */
    instObj = &gM2mSdeInstObj;

    /** If isregistered is set, then Vhwa_m2mDofUdmaInit is called and UDMA
     * channels are initialized, so reinit is required. */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((uint32_t)UTRUE == instObj->isRegistered)
    /* LDRA_JUSTIFY_END */
    {
        status = Vhwa_m2mSdeStopCh(instObj);

        if(FVID2_SOK == status)
        {        
            status = Vhwa_m2mSdeUdmaDeInit(instObj);
        }

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
            /* Need to unregister ISR, so it can be re-registered in next step */
            Vhwa_m2mSdeUnregisterIsr(instObj);
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(UDMA_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            /* Now reinitialized channels */
            status = Vhwa_m2mSdeUdmaInit(instObj, &instObj->initPrms);
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            /* INTD module also requires to be reinitialized */
            status = Vhwa_m2mSdeRegisterIsr(instObj);
        }

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
            /* Enable DOF at DMPAC Top*/
            CSL_dmpacEnableModule(instObj->socInfo.dmpacCntlRegs,
                DMPAC_MODULE_SDE, (uint32_t)UTRUE);
        }

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
            /* Now start all channels */
            status = Vhwa_m2mSdeStartCh(instObj);
        }

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
            /* Enable Pipeline interrupt in INTD */
            Vhwa_enableDmpacHtsIntr(instObj->socInfo.dmpacIntdRegs,
                instObj->vhwaIrqNum, instObj->pipeline);
        }
        /* Settting last handle to false/null, so that reconfiguration is done */
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
            instObj->lastHndlObj = NULL;
        }
    }

    return (status);
}
#endif
/* ========================================================================== */
/*                           Local Functions                                  */
/* ========================================================================== */

static int32_t Vhwa_sdeSetEeParams(const Vhwa_M2mSdeInstObj *instObj,
                                   Vhwa_M2mSdeHandleObj *hObj,
                                   const Sde_ErrEventParams *eePrms)
{
    int32_t status = FVID2_SOK;
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated in the imaging layer before reaching vhwa.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL != instObj) && (NULL != hObj) && (NULL != eePrms))
    /* LDRA_JUSTIFY_END */
    {
        /* Check if this handle is already being processed */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((NULL != instObj->actQObj) && (instObj->actQObj->hObj == hObj))
        {
            GT_0trace(VhwaSdeTrace, GT_ERR,
                "Cannot change error events while handle is active !!\n");
            status = FVID2_EAGAIN;
        }
        else if (0U != (eePrms->errEvents & (~VHWA_SDE_ALL_ERR_EVENTS)))
        {
            GT_1trace(VhwaSdeTrace, GT_ERR,
                "Attempt to enable a non-existent error event: 0x%08x !!\n",
                (eePrms->errEvents & (~VHWA_SDE_ALL_ERR_EVENTS)));
            status = FVID2_EINVALID_PARAMS;
        }
        /* LDRA_JUSTIFY_END */
        else
        {
            /* Copy the input params to handle object */
            Fvid2Utils_memcpy(&hObj->eePrms, eePrms, sizeof(Sde_ErrEventParams));
            status = FVID2_SOK;
        }
    }
    else
    {
        status = FVID2_EBADARGS;
    }

    return (status);
}

static int32_t Vhwa_sdeSetWdTimerEeParams(const Vhwa_M2mSdeInstObj *instObj,
                                   Vhwa_M2mSdeHandleObj *hObj,
                                   const Sde_WdTimerErrEventParams *wdTimerEePrms)
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
    if ((NULL != instObj) && (NULL != hObj) && (NULL != wdTimerEePrms))
    /* LDRA_JUSTIFY_END */
    {
        /* Check if this handle is already being processed */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((NULL != instObj->actQObj) && (instObj->actQObj->hObj == hObj))
        {
            GT_0trace(VhwaSdeTrace, GT_ERR,
                "Cannot change watchdog timer error events while handle is active !!\n");
            status = FVID2_EAGAIN;
        }
        else if (0U != (wdTimerEePrms->wdTimerErrEvents & (~VHWA_SDE_ALL_WDTIMER_ERR_EVENTS)))
        {
            GT_1trace(VhwaSdeTrace, GT_ERR,
                "Attempt to enable a non-existent watchdog timer error event: 0x%08x !!\n",
                (wdTimerEePrms->wdTimerErrEvents & (~VHWA_SDE_ALL_WDTIMER_ERR_EVENTS)));
            status = FVID2_EINVALID_PARAMS;
        }
        /* LDRA_JUSTIFY_END */
        else
        {
            /* Copy the input params to handle object */
            Fvid2Utils_memcpy(&hObj->wdTimerEePrms, wdTimerEePrms, sizeof(Sde_WdTimerErrEventParams));

            /* Update all HTS configurations (SDE can have multiple configurations) */
            /* Configure SDE WDTimer - enable if WDTIMER_ERR event is registered */
            if (0U != (wdTimerEePrms->wdTimerErrEvents & VHWA_SDE_WDTIMER_ERR))
            {
                hObj->htsCfg.enableWdTimer = (uint32_t)UTRUE;
                if (VHWA_WDTIMEOUT_64K_CYCLES == wdTimerEePrms->wdTimeoutCycles)
                {
                    hObj->htsCfg.isWdTimer128KMode = (uint32_t)UFALSE;
                }
                else
                {
                    hObj->htsCfg.isWdTimer128KMode = (uint32_t)UTRUE;
                }
            }
            else
            {
                hObj->htsCfg.enableWdTimer = (uint32_t)UFALSE;
            }

            /* Configure FOCO WDTimer - enable if FOCO1 WDTIMER_ERR event is registered */
            /* Only configure if FOCO is actually being used */
            if (UTRUE == hObj->isFocoUsed)
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (0U != (wdTimerEePrms->wdTimerErrEvents & VHWA_SDE_FOCO1_WDTIMER_ERR))
                /* LDRA_JUSTIFY_END */
                {
                    hObj->focoHtsCfg.enableWdTimer = (uint32_t)UTRUE;
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    This failure case is out of scope for the imaging test framework.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if (VHWA_WDTIMEOUT_64K_CYCLES == wdTimerEePrms->focoWdTimeoutCycles)
                    {
                        hObj->focoHtsCfg.isWdTimer128KMode = (uint32_t)UFALSE;
                    }
                    /* LDRA_JUSTIFY_END */
                    else
                    {
                        hObj->focoHtsCfg.isWdTimer128KMode = (uint32_t)UTRUE;
                    }
                }
                else
                {
                    hObj->focoHtsCfg.enableWdTimer = (uint32_t)UFALSE;
                }
            }
            status = FVID2_SOK;
        }
    }
    else
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */

    return (status);
}

static void Vhwa_sdeCalOutTrParams(Vhwa_M2mSdeHandleObj *hObj)
{
    Vhwa_M2mSdeOutChPrms *outChPrms = NULL;
    uint32_t numVerBlk,numBlkInRow;
    uint32_t outBuffPitch, numOfMidBlkLeft;
    uint32_t blkRwDone, trIdx, tNumOfBuff, rowDone;
    uint32_t blkLeftInRow, sl2BufIdx, sl2BufLeft;

    GT_assert(VhwaSdeTrace, (NULL != hObj));

    outBuffPitch = hObj->sdePrms.inOutImgFmt[SDE_OUTPUT].pitch[0U];
    outChPrms = &hObj->chPrms.outPrams;
    tNumOfBuff = hObj->sl2Prms.sl2OutBuffDepth;
    trIdx = 0;
    blkRwDone = 0;

    /* Calculate Number of bloacks and TR's for the output */
    numVerBlk = (hObj->sdePrms.sdeCfg.height/8U) - 1U;

    numBlkInRow = hObj->sdePrms.sdeCfg.width / SDE_OUTPUT_BLOCK_WIDTH_IN_PIX;

    if((hObj->sdePrms.sdeCfg.width % SDE_OUTPUT_BLOCK_WIDTH_IN_PIX) != 0u)
    {
        numBlkInRow++;
    }

    outChPrms->numBlkInRow = numBlkInRow;
    outChPrms->numVerBlk = numVerBlk;

    /* Calculate TR params for first block row */
    outChPrms->sicnt1[trIdx] = SDE_OUT_FIRST_LAST_BLK_HEIGHT;
    outChPrms->sicnt2[trIdx] = tNumOfBuff;
    outChPrms->sicnt3[trIdx] = numBlkInRow/tNumOfBuff;
    if((outChPrms->sicnt3[trIdx] * tNumOfBuff) != numBlkInRow)
    {
        outChPrms->sicnt3[trIdx]++;
    }
    outChPrms->dicnt2[trIdx] = numBlkInRow;
    outChPrms->dicnt3[trIdx] = 1U;

    outChPrms->sl2AddrOff[trIdx] = 0;
    outChPrms->bufAddrOff[trIdx] = 0;

    sl2BufIdx = numBlkInRow % tNumOfBuff;
    trIdx++;
    blkRwDone++;
    rowDone = SDE_OUT_FIRST_LAST_BLK_HEIGHT;

    /* Calculate TR params for middle block rows */
    while((sl2BufIdx > 0u) && (blkRwDone < (numVerBlk - 1u)))
    {
        sl2BufLeft = tNumOfBuff - sl2BufIdx;
        /* The TR source address is not aligned with the SL2 output
         * Buffer index split TR into 2 TR's */
        /* TR to transfer remaining blocks to align the SL2 buff index with
         * source buffer */
        outChPrms->sicnt1[trIdx] = SDE_OUT_MIDDLE_BLK_HEIGHT;
        outChPrms->sicnt2[trIdx] = sl2BufLeft;
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                However, due to the stated rationale, this is not tested.
        <justification end> */
        if(outChPrms->sicnt2[trIdx] > numBlkInRow)
        {
           outChPrms->sicnt2[trIdx] = numBlkInRow;
        }
        /* LDRA_JUSTIFY_END */
        outChPrms->sicnt3[trIdx] = 1U;

        outChPrms->dicnt2[trIdx] = outChPrms->sicnt2[trIdx];
        outChPrms->dicnt3[trIdx] = 1U;

        outChPrms->sl2AddrOff[trIdx] = SDE_PER_OUT_BUFF_SIZE * sl2BufIdx;
        outChPrms->bufAddrOff[trIdx] = rowDone * outBuffPitch;

        sl2BufIdx = sl2BufLeft - outChPrms->sicnt2[trIdx];
        blkLeftInRow = numBlkInRow - outChPrms->sicnt2[trIdx];
        trIdx++;

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                However, due to the stated rationale, this is not tested.
        <justification end> */
        if(blkLeftInRow > 0u)
        /* LDRA_JUSTIFY_END */
        {
            /* TR to transfer the rest of the blocks in row */
            outChPrms->sicnt1[trIdx] = SDE_OUT_MIDDLE_BLK_HEIGHT;
            outChPrms->sicnt2[trIdx] = tNumOfBuff;
            outChPrms->sicnt3[trIdx] = blkLeftInRow/tNumOfBuff;
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications CAN reach this portion.
                        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                    However, due to the stated rationale, this is not tested.
            <justification end> */
            if((outChPrms->sicnt3[trIdx] * tNumOfBuff) != blkLeftInRow)
            {
                outChPrms->sicnt3[trIdx]++;
            }
            /* LDRA_JUSTIFY_END */
            outChPrms->dicnt2[trIdx] = blkLeftInRow;
            outChPrms->dicnt3[trIdx] = 1U;

            outChPrms->sl2AddrOff[trIdx] = 0U;
            outChPrms->bufAddrOff[trIdx] = (rowDone * outBuffPitch) +
                            (sl2BufLeft * SDE_OUTPUT_BLOCK_WIDTH_IN_BYTE);

            sl2BufIdx = blkLeftInRow % tNumOfBuff;
            trIdx++;
        }
        blkRwDone++;
        rowDone += SDE_OUT_MIDDLE_BLK_HEIGHT;
    }

    /* Transfer reset of the middle rows blocks in one TR */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
                No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
            However, due to the stated rationale, this is not tested.
    <justification end> */
    if(blkRwDone < (numVerBlk - 1u))
    /* LDRA_JUSTIFY_END */
    {
        numOfMidBlkLeft = (numBlkInRow * (numVerBlk - blkRwDone - 1u));
        outChPrms->sicnt1[trIdx] = SDE_OUT_MIDDLE_BLK_HEIGHT;
        outChPrms->sicnt2[trIdx] = tNumOfBuff;
        outChPrms->sicnt3[trIdx] = numOfMidBlkLeft/tNumOfBuff;
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                However, due to the stated rationale, this is not tested.
        <justification end> */
        if((outChPrms->sicnt3[trIdx] * tNumOfBuff) != numOfMidBlkLeft)
        {
            outChPrms->sicnt3[trIdx]++;
        }
        /* LDRA_JUSTIFY_END */
        outChPrms->dicnt2[trIdx] = numBlkInRow;
        outChPrms->dicnt3[trIdx] = (numVerBlk - blkRwDone - 1u);

        outChPrms->sl2AddrOff[trIdx] = 0;
        outChPrms->bufAddrOff[trIdx] = outBuffPitch * rowDone;

        sl2BufIdx = numOfMidBlkLeft % tNumOfBuff;
        rowDone += SDE_OUT_MIDDLE_BLK_HEIGHT * (numVerBlk - blkRwDone - 1u);
        trIdx++;
    }

    /* Calculate TR params for last block row */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
                No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
            However, due to the stated rationale, this is not tested.
    <justification end> */
    if(sl2BufIdx > 0u)
    {
        sl2BufLeft = tNumOfBuff - sl2BufIdx;
        /* The TR source address is not aligned with the SL2 output
         * Buffer index split TR into 2 TR's */
        /* TR to transfer remaining blocks to align the SL2 buff index with
         * source buffer */
        outChPrms->sicnt1[trIdx] = SDE_OUT_FIRST_LAST_BLK_HEIGHT;
        outChPrms->sicnt2[trIdx] = sl2BufLeft;
        if(outChPrms->sicnt2[trIdx] > numBlkInRow)
        {
           outChPrms->sicnt2[trIdx] = numBlkInRow;
        }
        outChPrms->sicnt3[trIdx] = 1U;

        outChPrms->dicnt2[trIdx] = outChPrms->sicnt2[trIdx];
        outChPrms->dicnt3[trIdx] = 1U;

        outChPrms->sl2AddrOff[trIdx] = SDE_PER_OUT_BUFF_SIZE * sl2BufIdx;
        outChPrms->bufAddrOff[trIdx] = rowDone * outBuffPitch;

        blkLeftInRow = numBlkInRow - outChPrms->sicnt2[trIdx];
        trIdx++;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        blkLeftInRow = numBlkInRow;
        sl2BufLeft = 0;
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
                No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
            However, due to the stated rationale, this is not tested.
    <justification end> */
    if(blkLeftInRow > 0u)
    /* LDRA_JUSTIFY_END */
    {
        /* TR to transfer the rest of the blocks in row */
        outChPrms->sicnt1[trIdx] = SDE_OUT_FIRST_LAST_BLK_HEIGHT;
        outChPrms->sicnt2[trIdx] = tNumOfBuff;
        outChPrms->sicnt3[trIdx] = blkLeftInRow/tNumOfBuff;
        if((outChPrms->sicnt3[trIdx] * tNumOfBuff) != blkLeftInRow)
        {
            outChPrms->sicnt3[trIdx]++;
        }
        outChPrms->dicnt2[trIdx] = blkLeftInRow;
        outChPrms->dicnt3[trIdx] = 1U;

        outChPrms->sl2AddrOff[trIdx] = 0U;
        outChPrms->bufAddrOff[trIdx] = (rowDone * outBuffPitch) +
                            (sl2BufLeft * SDE_OUTPUT_BLOCK_WIDTH_IN_BYTE);

        trIdx++;
    }

    outChPrms->numTr = trIdx;
}

static Vhwa_M2mSdeHandleObj *Vhwa_sdeAllocHandleObj(
                                    const Vhwa_M2mSdeInstObj *instObj)
{
    uint32_t cnt;
    Vhwa_M2mSdeHandleObj *hObj = NULL;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated in the imaging layer before reaching vhwa.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != instObj)
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. An error print statement can be added in a future release if required.
        <justification end> */
        for (cnt = 0U; cnt < VHWA_M2M_SDE_MAX_HANDLES; cnt ++)
        /* LDRA_JUSTIFY_END */
        {
            if ((uint32_t)UFALSE == gM2mSdeHandleObj[cnt].isUsed)
            {
                hObj = &gM2mSdeHandleObj[cnt];
                Fvid2Utils_memset(hObj, 0x0, sizeof(Vhwa_M2mSdeHandleObj));
                gM2mSdeHandleObj[cnt].isUsed = (uint32_t)UTRUE;
                gM2mSdeHandleObj[cnt].enableReconfigReinitReg = (uint32_t)VHWA_SAFETY_MODE_DISABLED;
                gM2mSdeHandleObj[cnt].enableStatusRegValidate = (uint32_t)VHWA_SAFETY_MODE_DISABLED;
                hObj->hIdx = cnt;
                break;
            }
        }
    }
    return (hObj);
}

static void Vhwa_sdeFreeHandleObj(Vhwa_M2mSdeHandleObj *hObj)
{
    uint32_t cnt;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated in the imaging layer before reaching vhwa.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != hObj)
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. An error print statement can be added in a future release if required.
        <justification end> */
        for (cnt = 0U; cnt < VHWA_M2M_SDE_MAX_HANDLES; cnt ++)
        /* LDRA_JUSTIFY_END */
        {
            if (hObj == &gM2mSdeHandleObj[cnt])
            {
                hObj->isUsed = (uint32_t)UFALSE;;
                break;
            }
        }
    }
}

static void Vhwa_sdeSetLseCfg(Vhwa_M2mSdeHandleObj *hObj,
                              Vhwa_M2mSdePrms *sdePrms)
{
    Fvid2_ColorCompStorageFmt ccsFormat;
    CSL_LseConfig *lseCfg = NULL;
    Sde_Config    *sdeCfg = NULL;
    uint32_t      lineOffset;

    /* NULL Pointer check */
    GT_assert(VhwaSdeTrace, (NULL != hObj));
    GT_assert(VhwaSdeTrace, (NULL != sdePrms));

    lseCfg = &hObj->lseCfg;
    ccsFormat = sdePrms->inOutImgFmt[SDE_INPUT_BASE_IMG].ccsFormat;
    sdeCfg = &sdePrms->sdeCfg;

    /* Initialize LSE configuration with the default values */
    CSL_lseConfigInit(lseCfg);

    lineOffset = Vhwa_calcHorzSizeInBytes(sdeCfg->width, ccsFormat);

    lineOffset = ((lineOffset + 63u) & 0xFFFFFFC0u);

    lseCfg->numInCh = 1;
    lseCfg->numOutCh = 2;

    lseCfg->enableFixedArbMode = 1;

    /* Reference image */
    lseCfg->inChCfg[0U].enable = (uint32_t)UTRUE;
    lseCfg->inChCfg[0U].frameWidth = sdeCfg->width;

    lseCfg->inChCfg[0U].frameHeight = sdeCfg->height;

    lseCfg->inChCfg[0U].enableAddrIncrBy2 = (uint32_t)UFALSE;

    lseCfg->inChCfg[0U].ccsf = ccsFormat;
    lseCfg->inChCfg[0U].startOffset = 0U;

    lseCfg->inChCfg[0U].lineOffset = lineOffset;
    lseCfg->inChCfg[0U].circBufSize = SDE_FOCO_SL2_BUFF_DEPTH;
    lseCfg->inChCfg[0U].bufAddr[0U] =
                        (uint32_t)hObj->sl2Prms.sl2Addr[SDE_INPUT_FOCO_REF_IMG];
    lseCfg->inChCfg[0U].bufAddr[1U] =
                        (uint32_t)hObj->sl2Prms.sl2Addr[SDE_INPUT_FOCO_BASE_IMG];
    lseCfg->inChCfg[0U].numInBuff = 2U;

    lseCfg->inChCfg[0U].enableMultiTapKnlConfig = UFALSE;

    lseCfg->outChCfg[0U].enable = (uint32_t)UTRUE;
    lseCfg->outChCfg[0U].ccsf = FVID2_CCSF_BITS12_PACKED;
    lseCfg->outChCfg[0U].lineOffset =
                                hObj->sl2Prms.sl2Pitch[SDE_INPUT_REFERENCE_IMG];
    lseCfg->outChCfg[0U].circBufSize = SDE_SL2_REF_BASE_IMG_CIR_BUFF_SIZE;
    lseCfg->outChCfg[0U].bufAddr =
                        (uint32_t)hObj->sl2Prms.sl2Addr[SDE_INPUT_REFERENCE_IMG];

    /* Current image */
    lseCfg->outChCfg[1U].enable = (uint32_t)UTRUE;
    lseCfg->outChCfg[1U].ccsf = FVID2_CCSF_BITS12_PACKED;
    lseCfg->outChCfg[1U].lineOffset = hObj->sl2Prms.sl2Pitch[SDE_INPUT_BASE_IMG];
    lseCfg->outChCfg[1U].circBufSize = SDE_SL2_REF_BASE_IMG_CIR_BUFF_SIZE;
    lseCfg->outChCfg[1U].bufAddr =
                        (uint32_t)hObj->sl2Prms.sl2Addr[SDE_INPUT_BASE_IMG];
}

static void Vhwa_sdeSetHtsCfg(Vhwa_M2mSdeInstObj *instObj,
    Vhwa_M2mSdeHandleObj *hObj, Vhwa_M2mSdePrms *sdePrms)
{
    uint32_t psMaxCnt;
    CSL_DmpacHtsSchConfig *htsCfg = NULL;
    CSL_DmpacHtsSchConfig *focoHtsCfg = NULL;
    Sde_Config *sdeCfg = NULL;

    /* NULL Pointer check */
    GT_assert(VhwaSdeTrace, (NULL != instObj));
    GT_assert(VhwaSdeTrace, (NULL != hObj));
    GT_assert(VhwaSdeTrace, (NULL != sdePrms));

    htsCfg = &hObj->htsCfg;
    sdeCfg = &sdePrms->sdeCfg;
    focoHtsCfg = &hObj->focoHtsCfg;

    CSL_dmpacHtsSchConfigInit(htsCfg);

    htsCfg->schId = CSL_HTS_HWA_SCH_SDE;

    htsCfg->pipeline = instObj->pipeline;
    htsCfg->enableStream = (uint32_t)UFALSE;
    htsCfg->enableHop = (uint32_t)UFALSE;
    htsCfg->enableWdTimer = (uint32_t)UFALSE;
    htsCfg->isWdTimer128KMode = (uint32_t)UFALSE;

    htsCfg->enableBwLimit = UFALSE;

    /* Default configuration for FOCO HTS */
    CSL_dmpacHtsSchConfigInit(focoHtsCfg);
    focoHtsCfg->schId = CSL_HTS_HWA_SCH_FOCO2;

    if(hObj->isFocoUsed == UTRUE)
    {
        focoHtsCfg->pipeline = instObj->pipeline;
        focoHtsCfg->enableStream = (uint32_t)UFALSE;
        focoHtsCfg->enableHop = (uint32_t)UFALSE;
        focoHtsCfg->enableWdTimer = UFALSE;
        focoHtsCfg->isWdTimer128KMode = UFALSE;
        focoHtsCfg->enableBwLimit = (uint32_t)UFALSE;

        /* Reference frame config */
        focoHtsCfg->consCfg[HTS_SDE_REFE_FRAME_CH_IDX].enable = (uint32_t)UTRUE;
        focoHtsCfg->consCfg[HTS_SDE_REFE_FRAME_CH_IDX].prodId =
                                            CSL_HTS_PROD_IDX_UDMA;

        focoHtsCfg->dmaProdCfg[HTS_SDE_REFE_FRAME_CH_IDX].enable = (uint32_t)UTRUE;
        focoHtsCfg->dmaProdCfg[HTS_SDE_REFE_FRAME_CH_IDX].dmaChNum =
                    Udma_chGetNum(instObj->utcChHndl[SDE_INPUT_REFERENCE_IMG]);
        focoHtsCfg->dmaProdCfg[HTS_SDE_REFE_FRAME_CH_IDX].pipeline =
                                            instObj->pipeline;
        focoHtsCfg->dmaProdCfg[HTS_SDE_REFE_FRAME_CH_IDX].consId =
                                            CSL_HTS_CONS_IDX_UDMA;
        /* Default mapping for DMA */

        focoHtsCfg->dmaProdCfg[HTS_SDE_REFE_FRAME_CH_IDX].threshold = 1;
        focoHtsCfg->dmaProdCfg[HTS_SDE_REFE_FRAME_CH_IDX].cntPreLoad = 0;

        focoHtsCfg->dmaProdCfg[HTS_SDE_REFE_FRAME_CH_IDX].cntPostLoad = 0;

        focoHtsCfg->dmaProdCfg[HTS_SDE_REFE_FRAME_CH_IDX].enableHop =
                                                (uint32_t)UTRUE;
        focoHtsCfg->dmaProdCfg[HTS_SDE_REFE_FRAME_CH_IDX].numHop =
                                                sdeCfg->height;
        focoHtsCfg->dmaProdCfg[HTS_SDE_REFE_FRAME_CH_IDX].countDec = 1U;
        focoHtsCfg->dmaProdCfg[HTS_SDE_REFE_FRAME_CH_IDX].depth =
                                                SDE_FOCO_SL2_BUFF_DEPTH;

        focoHtsCfg->prodCfg[HTS_SDE_REFE_FRAME_CH_IDX].enable = (uint32_t)UTRUE;
        focoHtsCfg->prodCfg[HTS_SDE_REFE_FRAME_CH_IDX].consId =
                                                CSL_HTS_CONS_IDX_SDE_REF;

        focoHtsCfg->prodCfg[HTS_SDE_REFE_FRAME_CH_IDX].threshold =
                                        SDE_SL2_REF_BASE_IMG_BUFF_HEIGHT * 2U;
        focoHtsCfg->prodCfg[HTS_SDE_REFE_FRAME_CH_IDX].cntPreLoad = 0;
        focoHtsCfg->prodCfg[HTS_SDE_REFE_FRAME_CH_IDX].cntPostLoad = 0;


        focoHtsCfg->prodCfg[HTS_SDE_REFE_FRAME_CH_IDX].depth =
                                        SDE_SL2_REF_BASE_IMG_BUFF_HEIGHT * 3U;
        focoHtsCfg->prodCfg[HTS_SDE_REFE_FRAME_CH_IDX].countDec =
                                        SDE_SL2_REF_BASE_IMG_BUFF_HEIGHT;

        /* Enable PA */
        focoHtsCfg->prodCfg[HTS_SDE_REFE_FRAME_CH_IDX].paCfg.enable = (uint32_t)UTRUE;
        focoHtsCfg->prodCfg[HTS_SDE_REFE_FRAME_CH_IDX].paCfg.enableBufCtrl = 0U;

        psMaxCnt = sdeCfg->width/SDE_PROCESS_BLOCK_WIDTH;
        if((SDE_PROCESS_BLOCK_WIDTH * psMaxCnt) != sdeCfg->width)
        {
            psMaxCnt++;
        }
        focoHtsCfg->prodCfg[HTS_SDE_REFE_FRAME_CH_IDX].paCfg.psMaxCnt = psMaxCnt;
        focoHtsCfg->prodCfg[HTS_SDE_REFE_FRAME_CH_IDX].paCfg.csMaxCnt =
                                        SDE_SL2_REF_BASE_IMG_BUFF_HEIGHT * 2U;
        focoHtsCfg->prodCfg[HTS_SDE_REFE_FRAME_CH_IDX].paCfg.enableDecCtrl = 1U;
        focoHtsCfg->prodCfg[HTS_SDE_REFE_FRAME_CH_IDX].paCfg.countDec = 1U;


        /* Base frame config */
        focoHtsCfg->consCfg[HTS_SDE_BASE_FRAME_CH_IDX].enable = (uint32_t)UTRUE;
        focoHtsCfg->consCfg[HTS_SDE_BASE_FRAME_CH_IDX].prodId =
                                            CSL_HTS_PROD_IDX_UDMA;

        focoHtsCfg->dmaProdCfg[HTS_SDE_BASE_FRAME_CH_IDX].enable = (uint32_t)UTRUE;
        focoHtsCfg->dmaProdCfg[HTS_SDE_BASE_FRAME_CH_IDX].dmaChNum =
                        Udma_chGetNum(instObj->utcChHndl[SDE_INPUT_BASE_IMG]);
        focoHtsCfg->dmaProdCfg[HTS_SDE_BASE_FRAME_CH_IDX].pipeline =
                                            instObj->pipeline;
        focoHtsCfg->dmaProdCfg[HTS_SDE_BASE_FRAME_CH_IDX].consId =
                                            CSL_HTS_CONS_IDX_UDMA;
        /* Default mapping for DMA */

        focoHtsCfg->dmaProdCfg[HTS_SDE_BASE_FRAME_CH_IDX].threshold = 1;
        focoHtsCfg->dmaProdCfg[HTS_SDE_BASE_FRAME_CH_IDX].cntPreLoad = 0;
        focoHtsCfg->dmaProdCfg[HTS_SDE_BASE_FRAME_CH_IDX].cntPostLoad = 0;

        focoHtsCfg->dmaProdCfg[HTS_SDE_BASE_FRAME_CH_IDX].enableHop = (uint32_t)UTRUE;
        focoHtsCfg->dmaProdCfg[HTS_SDE_BASE_FRAME_CH_IDX].numHop = sdeCfg->height;
        focoHtsCfg->dmaProdCfg[HTS_SDE_BASE_FRAME_CH_IDX].countDec = 1U;
        focoHtsCfg->dmaProdCfg[HTS_SDE_BASE_FRAME_CH_IDX].depth =
                                                    SDE_FOCO_SL2_BUFF_DEPTH;


        focoHtsCfg->prodCfg[HTS_SDE_BASE_FRAME_CH_IDX].enable = (uint32_t)UTRUE;
        focoHtsCfg->prodCfg[HTS_SDE_BASE_FRAME_CH_IDX].consId =
                                            CSL_HTS_CONS_IDX_SDE_BASE;

        focoHtsCfg->prodCfg[HTS_SDE_BASE_FRAME_CH_IDX].threshold =
                                        SDE_SL2_REF_BASE_IMG_BUFF_HEIGHT * 2U;

        focoHtsCfg->prodCfg[HTS_SDE_BASE_FRAME_CH_IDX].cntPreLoad = 0;
        focoHtsCfg->prodCfg[HTS_SDE_BASE_FRAME_CH_IDX].cntPostLoad = 0;


        focoHtsCfg->prodCfg[HTS_SDE_BASE_FRAME_CH_IDX].depth =
                                        SDE_SL2_REF_BASE_IMG_BUFF_HEIGHT * 3U;
        focoHtsCfg->prodCfg[HTS_SDE_BASE_FRAME_CH_IDX].countDec =
                                        SDE_SL2_REF_BASE_IMG_BUFF_HEIGHT;

        /* Enable PA */
        focoHtsCfg->prodCfg[HTS_SDE_BASE_FRAME_CH_IDX].paCfg.enable = (uint32_t)UTRUE;
        focoHtsCfg->prodCfg[HTS_SDE_BASE_FRAME_CH_IDX].paCfg.enableBufCtrl = 0U;

        focoHtsCfg->prodCfg[HTS_SDE_BASE_FRAME_CH_IDX].paCfg.psMaxCnt = psMaxCnt;
        focoHtsCfg->prodCfg[HTS_SDE_BASE_FRAME_CH_IDX].paCfg.csMaxCnt =
                                        SDE_SL2_REF_BASE_IMG_BUFF_HEIGHT * 2U;
        focoHtsCfg->prodCfg[HTS_SDE_BASE_FRAME_CH_IDX].paCfg.enableDecCtrl = 1U;
        focoHtsCfg->prodCfg[HTS_SDE_BASE_FRAME_CH_IDX].paCfg.countDec = 1U;


        /* HWA0 Consumer control for RFGW */
        htsCfg->consCfg[HTS_SDE_REFE_FRAME_CH_IDX].enable = (uint32_t)UTRUE;
        htsCfg->consCfg[HTS_SDE_REFE_FRAME_CH_IDX].prodId = CSL_HTS_PROD_IDX_FOCO1_0;

        /* HWA0 Consumer control for CFGW */
        htsCfg->consCfg[HTS_SDE_BASE_FRAME_CH_IDX].enable = (uint32_t)UTRUE;
        htsCfg->consCfg[HTS_SDE_BASE_FRAME_CH_IDX].prodId = CSL_HTS_PROD_IDX_FOCO1_1;

    }
    else
    {
        /* Reference Frame Config */
        htsCfg->consCfg[HTS_SDE_REFE_FRAME_CH_IDX].enable = (uint32_t)UTRUE;
        htsCfg->consCfg[HTS_SDE_REFE_FRAME_CH_IDX].prodId = CSL_HTS_PROD_IDX_UDMA;

        htsCfg->dmaProdCfg[HTS_SDE_REFE_FRAME_CH_IDX].enable = (uint32_t)UTRUE;
        htsCfg->dmaProdCfg[HTS_SDE_REFE_FRAME_CH_IDX].dmaChNum =
                    Udma_chGetNum(instObj->utcChHndl[SDE_INPUT_REFERENCE_IMG]);
        htsCfg->dmaProdCfg[HTS_SDE_REFE_FRAME_CH_IDX].pipeline = instObj->pipeline;
        htsCfg->dmaProdCfg[HTS_SDE_REFE_FRAME_CH_IDX].consId = CSL_HTS_CONS_IDX_UDMA;
        /* Default mapping for DMA */


        htsCfg->dmaProdCfg[HTS_SDE_REFE_FRAME_CH_IDX].threshold = 2U;
        htsCfg->dmaProdCfg[HTS_SDE_REFE_FRAME_CH_IDX].cntPreLoad = 0;
        htsCfg->dmaProdCfg[HTS_SDE_REFE_FRAME_CH_IDX].cntPostLoad = 0;

        htsCfg->dmaProdCfg[HTS_SDE_REFE_FRAME_CH_IDX].enableHop = (uint32_t)UTRUE;
        htsCfg->dmaProdCfg[HTS_SDE_REFE_FRAME_CH_IDX].numHop = sdeCfg->height/SDE_SL2_REF_BASE_IMG_BUFF_HEIGHT;
        htsCfg->dmaProdCfg[HTS_SDE_REFE_FRAME_CH_IDX].countDec = 1U;
        htsCfg->dmaProdCfg[HTS_SDE_REFE_FRAME_CH_IDX].depth = SDE_SL2_REF_BASE_IMG_BUFF_COUNT;

        /* Enable PA */
        htsCfg->dmaProdCfg[HTS_SDE_REFE_FRAME_CH_IDX].paCfg.enable = (uint32_t)UTRUE;
        htsCfg->dmaProdCfg[HTS_SDE_REFE_FRAME_CH_IDX].paCfg.enableBufCtrl = 0U;

        psMaxCnt = sdeCfg->width/SDE_PROCESS_BLOCK_WIDTH;
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                However, due to the stated rationale, this is not tested.
        <justification end> */
        if((SDE_PROCESS_BLOCK_WIDTH * psMaxCnt) != sdeCfg->width)
        {
            psMaxCnt++;
        }
        /* LDRA_JUSTIFY_END */
        htsCfg->dmaProdCfg[HTS_SDE_REFE_FRAME_CH_IDX].paCfg.psMaxCnt = psMaxCnt;
        htsCfg->dmaProdCfg[HTS_SDE_REFE_FRAME_CH_IDX].paCfg.csMaxCnt = 2U;
        htsCfg->dmaProdCfg[HTS_SDE_REFE_FRAME_CH_IDX].paCfg.enableDecCtrl = 1U;
        htsCfg->dmaProdCfg[HTS_SDE_REFE_FRAME_CH_IDX].paCfg.countDec = 1U;

        /* Base Frame Config */
        htsCfg->consCfg[HTS_SDE_BASE_FRAME_CH_IDX].enable = (uint32_t)UTRUE;
        htsCfg->consCfg[HTS_SDE_BASE_FRAME_CH_IDX].prodId = CSL_HTS_PROD_IDX_UDMA;

        htsCfg->dmaProdCfg[HTS_SDE_BASE_FRAME_CH_IDX].enable = (uint32_t)UTRUE;
        htsCfg->dmaProdCfg[HTS_SDE_BASE_FRAME_CH_IDX].dmaChNum =
                        Udma_chGetNum(instObj->utcChHndl[SDE_INPUT_BASE_IMG]);
        htsCfg->dmaProdCfg[HTS_SDE_BASE_FRAME_CH_IDX].pipeline = instObj->pipeline;
        htsCfg->dmaProdCfg[HTS_SDE_BASE_FRAME_CH_IDX].consId = CSL_HTS_CONS_IDX_UDMA;
        /* Default mapping for DMA */

        htsCfg->dmaProdCfg[HTS_SDE_BASE_FRAME_CH_IDX].threshold = 2;
        htsCfg->dmaProdCfg[HTS_SDE_BASE_FRAME_CH_IDX].cntPreLoad = 0;
        htsCfg->dmaProdCfg[HTS_SDE_BASE_FRAME_CH_IDX].cntPostLoad = 0;

        htsCfg->dmaProdCfg[HTS_SDE_BASE_FRAME_CH_IDX].enableHop = (uint32_t)UTRUE;
        htsCfg->dmaProdCfg[HTS_SDE_BASE_FRAME_CH_IDX].numHop = sdeCfg->height/SDE_SL2_REF_BASE_IMG_BUFF_HEIGHT;
        htsCfg->dmaProdCfg[HTS_SDE_BASE_FRAME_CH_IDX].countDec = 1U;
        htsCfg->dmaProdCfg[HTS_SDE_BASE_FRAME_CH_IDX].depth = SDE_SL2_REF_BASE_IMG_BUFF_COUNT;

        /* Enable PA */
        htsCfg->dmaProdCfg[HTS_SDE_BASE_FRAME_CH_IDX].paCfg.enable = (uint32_t)UTRUE;
        htsCfg->dmaProdCfg[HTS_SDE_BASE_FRAME_CH_IDX].paCfg.enableBufCtrl = 0U;
        htsCfg->dmaProdCfg[HTS_SDE_BASE_FRAME_CH_IDX].paCfg.psMaxCnt = psMaxCnt;
        htsCfg->dmaProdCfg[HTS_SDE_BASE_FRAME_CH_IDX].paCfg.csMaxCnt = 2U;
        htsCfg->dmaProdCfg[HTS_SDE_BASE_FRAME_CH_IDX].paCfg.enableDecCtrl = 1U;
        htsCfg->dmaProdCfg[HTS_SDE_BASE_FRAME_CH_IDX].paCfg.countDec = 1U;
    }

    /* Output flow config */
    htsCfg->prodCfg[HTS_SDE_OUT_CH_IDX].enable = (uint32_t)UTRUE;
    htsCfg->prodCfg[HTS_SDE_OUT_CH_IDX].consId = CSL_HTS_CONS_IDX_UDMA;

    htsCfg->prodCfg[HTS_SDE_OUT_CH_IDX].threshold = 1u;
    htsCfg->prodCfg[HTS_SDE_OUT_CH_IDX].cntPreLoad = 0u;
    htsCfg->prodCfg[HTS_SDE_OUT_CH_IDX].cntPostLoad = 0u;

    /* Output 2 lines per dma transfer*/
    htsCfg->prodCfg[HTS_SDE_OUT_CH_IDX].depth = hObj->sl2Prms.sl2OutBuffDepth;
    htsCfg->prodCfg[HTS_SDE_OUT_CH_IDX].countDec = 1U;

    htsCfg->dmaConsCfg[HTS_SDE_OUT_CH_IDX].enable = (uint32_t)UTRUE;
    htsCfg->dmaConsCfg[HTS_SDE_OUT_CH_IDX].dmaChNum =
                                Udma_chGetNum(instObj->utcChHndl[SDE_OUTPUT]);
    htsCfg->dmaConsCfg[HTS_SDE_OUT_CH_IDX].pipeline = instObj->pipeline;
    htsCfg->dmaConsCfg[HTS_SDE_OUT_CH_IDX].enableStream = (uint32_t)UFALSE;
    htsCfg->dmaConsCfg[HTS_SDE_OUT_CH_IDX].prodId = CSL_HTS_PROD_IDX_UDMA;
}

static void Vhwa_sdeSetFocoParams(Vhwa_M2mSdeHandleObj *hObj,
                                     const Vhwa_M2mSdePrms *sdePrms)
{
    Dmpac_FocoConfig *focoCfg = &hObj->focoCfg;
    #ifndef TEST_SDE_FOCO_WDTIMER_ERR_INTERRUPT
    Sde_Config       *sdeCfg = &hObj->sdePrms.sdeCfg;
    #endif

    /* NULL Pointer check */
    GT_assert(VhwaSdeTrace, (NULL != hObj));
    GT_assert(VhwaSdeTrace, (NULL != sdePrms));

    if(sdePrms->focoPrms.shiftM1 != 0u)
    {
        focoCfg->shiftEnable = UTRUE;
        focoCfg->shiftM1 = sdePrms->focoPrms.shiftM1 - 1u;
    }
    else
    {
        focoCfg->shiftEnable = UFALSE;
        focoCfg->shiftM1 = 0U;
    }
    focoCfg->dir = sdePrms->focoPrms.dir;
    focoCfg->round = sdePrms->focoPrms.round;
    focoCfg->channelEnable = UTRUE;
    focoCfg->mask = 0xffff;
    #ifdef TEST_SDE_FOCO_WDTIMER_ERR_INTERRUPT
    focoCfg->trig = 0;
    #else
    focoCfg->trig = sdeCfg->height;
    #endif

    focoCfg->preload = 0;
    /* Multiple of 2 */
    focoCfg->postload = 0;

}

static int32_t Vhwa_sdeCheckCfg(const Vhwa_M2mSdeInstObj *instObj,
                                Vhwa_M2mSdeHandleObj *hObj,
                                const Vhwa_M2mSdePrms *sdePrms)
{
    int32_t status = FVID2_SOK;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated in the imaging layer before reaching vhwa.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == instObj) || (NULL == hObj) || (NULL == sdePrms))
    {
        status = FVID2_EINVALID_PARAMS;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
        <justification end> */
        if((sdePrms->sdeCfg.width < SDE_MIN_IMAGE_WIDTH) ||
           (sdePrms->sdeCfg.height < SDE_MIN_IMAGE_HEIGHT) ||
           (sdePrms->sdeCfg.width > SDE_MAX_IMAGE_WIDTH) ||
           (sdePrms->sdeCfg.height > SDE_MAX_IMAGE_HEIGHT) ||
           ((sdePrms->sdeCfg.width & 0xFu) != 0u) ||
           ((sdePrms->sdeCfg.height & 0xFu) != 0u))
        /* LDRA_JUSTIFY_END */
        {
            status = FVID2_EBADARGS;
        }

        if((sdePrms->sdeCfg.minDisparity != 0u) &&
           (sdePrms->sdeCfg.minDisparity != 1u))
        {
            status = FVID2_EBADARGS;

        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
        <justification end> */
        if((sdePrms->sdeCfg.searchRange != SDE_SR_64) &&
           (sdePrms->sdeCfg.searchRange != SDE_SR_128) &&
           (sdePrms->sdeCfg.searchRange != SDE_SR_192))
        {
            status = FVID2_EBADARGS;

        }
        /* LDRA_JUSTIFY_END */
        if(sdePrms->inOutImgFmt[SDE_INPUT_BASE_IMG].ccsFormat == FVID2_CCSF_BITS12_PACKED)
        {
            hObj->isFocoUsed = UFALSE;
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
        <justification end> */
        else if((sdePrms->inOutImgFmt[SDE_INPUT_BASE_IMG].ccsFormat ==
                                    FVID2_CCSF_BITS12_UNPACKED16) ||
                (sdePrms->inOutImgFmt[SDE_INPUT_BASE_IMG].ccsFormat ==
                                    FVID2_CCSF_BITS8_PACKED) ||
                (sdePrms->inOutImgFmt[SDE_INPUT_BASE_IMG].ccsFormat ==
                                    FVID2_CCSF_BITS12_UNPACKED16_MSB_ALIGNED))
        /* LDRA_JUSTIFY_END */
        {
            hObj->isFocoUsed = UTRUE;
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
            hObj->isFocoUsed = UFALSE;
            status = FVID2_EBADARGS;
        }
        /* LDRA_JUSTIFY_END */
    }

    return status;
}

static int32_t Vhwa_sdeSetSl2Prms(const Vhwa_M2mSdeInstObj *instObj,
                                  Vhwa_M2mSdeHandleObj *hObj,
                                  Vhwa_M2mSdePrms *sdePrms)
{
    int32_t status = FVID2_SOK;
    uint32_t pitchSl2;
    uint32_t imgMem, outBuffMem, rcBuffMem, focoBuffMem, tMemReq;
    uint64_t sl2StartAddr;
    Sde_Config         *sdeCfg = NULL;
    Vhwa_M2mSdeSl2Prms *sl2Prms = NULL;

    GT_assert(VhwaSdeTrace, (NULL != instObj));
    GT_assert(VhwaSdeTrace, (NULL != hObj));
    GT_assert(VhwaSdeTrace, (NULL != sdePrms));

    sdeCfg = &sdePrms->sdeCfg;
    sl2Prms = &hObj->sl2Prms;

    pitchSl2 = Vhwa_calcHorzSizeInBytes(sdeCfg->width,
                                        FVID2_CCSF_BITS12_PACKED);
    pitchSl2 = Vhwa_calcSl2Pitch(pitchSl2);

    imgMem = pitchSl2 * SDE_SL2_REF_BASE_IMG_CIR_BUFF_SIZE;

    /* Set Out buffer Parameters */
    outBuffMem = SDE_PER_OUT_BUFF_SIZE * sl2Prms->sl2OutBuffDepth;

    /* Set Row cost Buffer Parameters */
    rcBuffMem= (sdeCfg->width/8u)*2u*8u*8u;

    if(sdeCfg->searchRange == SDE_SR_192)
    {
        rcBuffMem = rcBuffMem * 3u;
    }
    else if(sdeCfg->searchRange == SDE_SR_128)
    {
        rcBuffMem = rcBuffMem * 2u;
    }
    else
    {
        /* Nothing to do here */
    }

    /* Set FOCO Parameters */
    if(FVID2_CCSF_BITS12_PACKED ==
                            sdePrms->inOutImgFmt[SDE_INPUT_BASE_IMG].ccsFormat)
    {
        focoBuffMem = 0;
    }
    else
    {
        focoBuffMem = Vhwa_calcHorzSizeInBytes(sdeCfg->width,
                           sdePrms->inOutImgFmt[SDE_INPUT_BASE_IMG].ccsFormat);
        focoBuffMem = Vhwa_calcSl2Pitch(focoBuffMem);
        focoBuffMem = focoBuffMem * 2u;
    }

    tMemReq = (imgMem*2u) + outBuffMem + rcBuffMem + (focoBuffMem*2u);

    if(tMemReq > instObj->sl2Size)
    {
        status = FVID2_EALLOC;
    }
    else
    {
        sl2StartAddr = instObj->sl2StartAddr;

        sl2Prms->sl2Addr[SDE_INPUT_REFERENCE_IMG] = sl2StartAddr;
        sl2Prms->sl2Pitch[SDE_INPUT_REFERENCE_IMG] = pitchSl2;
        sl2StartAddr = sl2StartAddr + imgMem;
        sl2Prms->sl2Addr[SDE_INPUT_BASE_IMG] = sl2StartAddr;
        sl2Prms->sl2Pitch[SDE_INPUT_BASE_IMG] = pitchSl2;
        sl2StartAddr = sl2StartAddr + imgMem;

        sl2Prms->sl2Addr[SDE_OUTPUT] = sl2StartAddr;
        sl2StartAddr = sl2StartAddr + outBuffMem;

        sl2Prms->sl2RwCostBuffAddr = sl2StartAddr;
        sl2StartAddr = sl2StartAddr + rcBuffMem;

        if(focoBuffMem > 0u)
        {
            sl2Prms->sl2Addr[SDE_INPUT_FOCO_REF_IMG] = sl2StartAddr;
            sl2StartAddr = sl2StartAddr + focoBuffMem;
            sl2Prms->sl2Addr[SDE_INPUT_FOCO_BASE_IMG] = sl2StartAddr;
        }
        else
        {
            sl2Prms->sl2Addr[SDE_INPUT_FOCO_REF_IMG] = 0;
            sl2Prms->sl2Addr[SDE_INPUT_FOCO_BASE_IMG] = 0;
        }
    }

    return (status);
}

static int32_t Vhwa_sdeSetParams(Vhwa_M2mSdeInstObj *instObj,
    Vhwa_M2mSdeHandleObj *hObj, Vhwa_M2mSdePrms *sdePrms)
{
    int32_t status = FVID2_EBADARGS;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated in the imaging layer before reaching vhwa.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL != instObj) && (NULL != hObj) && (NULL != sdePrms))
    /* LDRA_JUSTIFY_END */
    {
        status = Vhwa_sdeCheckCfg(instObj, hObj, sdePrms);

        if(FVID2_SOK == status)
        {
            status = Vhwa_sdeSetSl2Prms(instObj, hObj, sdePrms);
        }

        if (FVID2_SOK == status)
        {
            Fvid2Utils_memcpy(&hObj->sdePrms, sdePrms, sizeof(Vhwa_M2mSdePrms));

            /* Set SDE buff params */
            Vhwa_sdeSetBuffParams(hObj);

            /* Calculate TR information for output channel */
            Vhwa_sdeCalOutTrParams(hObj);

            /* HTS config */
            Vhwa_sdeSetHtsCfg(instObj, hObj, &hObj->sdePrms);

            if(hObj->isFocoUsed == UTRUE)
            {
                /* LSE config */
                Vhwa_sdeSetLseCfg(hObj, &hObj->sdePrms);

                Vhwa_sdeSetFocoParams(hObj, &hObj->sdePrms);
            }

            /* Setup TR Descriptor */
            Vhwa_m2mSdeSetTrDesc(instObj, hObj);
        }
        else
        {
            /* Reset HTS config based on local correct LDC config */
            Vhwa_sdeSetHtsCfg(instObj, hObj, &hObj->sdePrms);
            /* Reset LSE config based on local correct LDC config */
            Vhwa_sdeSetLseCfg(hObj, &hObj->sdePrms);
        }
    }

    return (status);
}

static void Vhwa_sdeSetBuffParams(Vhwa_M2mSdeHandleObj *hObj)
{
    CSL_SdeBufParams    *sdeBuffParams;

    sdeBuffParams = &hObj->sdeBuffPrms;

    sdeBuffParams->baseImageAddress =
                    (uint32_t)hObj->sl2Prms.sl2Addr[SDE_INPUT_REFERENCE_IMG];
    sdeBuffParams->baseImageWidth =
                    (uint32_t)hObj->sl2Prms.sl2Pitch[SDE_INPUT_REFERENCE_IMG];
    sdeBuffParams->refImageAddress =
                    (uint32_t)hObj->sl2Prms.sl2Addr[SDE_INPUT_BASE_IMG];
    sdeBuffParams->refImageWidth =
                    (uint32_t)hObj->sl2Prms.sl2Pitch[SDE_INPUT_BASE_IMG];
    sdeBuffParams->disparityAddress =
                    (uint32_t)hObj->sl2Prms.sl2Addr[SDE_OUTPUT];
    sdeBuffParams->numOutputBufs = (uint32_t)hObj->sl2Prms.sl2OutBuffDepth;
    sdeBuffParams->rowCostBufferAddress =
                    (uint32_t)hObj->sl2Prms.sl2RwCostBuffAddr;
}

static int32_t Vhwa_sdeEnableFoco(const Vhwa_M2mSdeInstObj *instObj,
                                  const Vhwa_M2mSdeHandleObj *hObj)
{
    int32_t status = FVID2_SOK;
    CSL_dmpac_foco_coreRegs *focoRegs = instObj->socInfo.dmpacFocoRegs;

    VhwaDmpacSdeSocReadBack *goldenRegVal = hObj->configRegMemPrms.configGoldenRegPtr;

    /* Enable channel 0 for Reference Frame */
    status = CSL_dmpacFocoSetConfig(focoRegs, DMPAC_FOCO_CHANNEL_0,
                                    &hObj->focoCfg);

    /* If configRegValidate is enabled, also set the golden register values */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (((FVID2_SOK == status) && (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate)) && (NULL != goldenRegVal))
    /* LDRA_JUSTIFY_END */
    {
        status = CSL_dmpacFocoSetConfig(&(goldenRegVal->dmpacFocoRegs), DMPAC_FOCO_CHANNEL_0,
                    &hObj->focoCfg);
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The negative test framework and test apps cannot reach this portion.
    For the previously called CSL function, the status is returned as fail if and only if either of the passed arguments is NULL.
    None of the arguments are accessible to us though the test framework.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    /* Enable channel 1 for Current Frame */
    if(CSL_PASS == status)
    /* LDRA_JUSTIFY_END */
    {
        status = CSL_dmpacFocoSetConfig(focoRegs, DMPAC_FOCO_CHANNEL_1,
                                        &hObj->focoCfg);
    }
    /* If configRegValidate is enabled, also set the golden register values */

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (((FVID2_SOK == status) && (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate)) && (NULL != goldenRegVal))
    /* LDRA_JUSTIFY_END */
    {
        status = CSL_dmpacFocoSetConfig(&(goldenRegVal->dmpacFocoRegs), DMPAC_FOCO_CHANNEL_1,
                    &hObj->focoCfg);
    }
    return status;
}

static int32_t Vhwa_m2mSdeSetConfigInHW(const Vhwa_M2mSdeInstObj *instObj,
                                        const Vhwa_M2mSdeQueueObj *qObj)
{
    int32_t               status = FVID2_SOK;
    Vhwa_M2mSdeHandleObj *hObj = NULL;
    const Sde_SocInfo          *socInfo = NULL;

    /* Null pointer check */
    GT_assert(VhwaSdeTrace, (NULL != instObj));
    GT_assert(VhwaSdeTrace, (NULL != qObj));
    GT_assert(VhwaSdeTrace, (NULL != qObj->hObj));

    hObj = qObj->hObj;
    socInfo = &instObj->socInfo;

    VhwaDmpacSdeSocReadBack *goldenRegVal = hObj->configRegMemPrms.configGoldenRegPtr;

    /* Configure SL2 Buffer parameters */
    status = CSL_sdeSetBufParam(socInfo->sdeRegs, &hObj->sdeBuffPrms);

    /* If configRegValidate is enabled, also set the golden register values */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (((FVID2_SOK == status) && (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate)) && (NULL != goldenRegVal))
    /* LDRA_JUSTIFY_END */
    {
        status = CSL_sdeSetBufParam(&(goldenRegVal->sdeRegs), &hObj->sdeBuffPrms);
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The negative test framework and test apps cannot reach this portion.
    For the previously called CSL function, the status is returned as fail if and only if either of the passed arguments is NULL.
    None of the arguments are accessible to us though the test framework.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {

        status = CSL_sdeSetConfig(socInfo->sdeRegs,
                                        &hObj->sdePrms.sdeCfg);
    }
    /* If configRegValidate is enabled, also set the golden register values */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (((FVID2_SOK == status) && (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate)) && (NULL != goldenRegVal))
    /* LDRA_JUSTIFY_END */
    {
        status = CSL_sdeSetConfig(&(goldenRegVal->sdeRegs),
                                        &hObj->sdePrms.sdeCfg);
    }

    CSL_sdeEnablePsa(socInfo->sdeRegs, hObj->createArgs.enablePsa);

    /* If configRegValidate is enabled, also set the golden register values */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (((FVID2_SOK == status) && (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate)) && (NULL != goldenRegVal))
    /* LDRA_JUSTIFY_END */
    {
        CSL_sdeEnablePsa(&(goldenRegVal->sdeRegs), hObj->createArgs.enablePsa);
    }

    if(hObj->isFocoUsed == UTRUE)
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The negative test framework and test apps cannot reach this portion.
        For the previously called CSL function, the status is returned as fail if and only if either of the passed arguments is NULL.
        None of the arguments are accessible to us though the test framework.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            /* Step-2, Configure HWA Common Wrapper LSE */
            status = Vhwa_sdeEnableFoco(instObj, hObj);
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The negative test framework and test apps cannot reach this portion.
        For Vhwa_sdeEnableFoco, the status is returned as fail if and only if either of the passed arguments is NULL.
        None of the arguments are accessible to us though the test framework.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            /* Step-2, Configure HWA Common Wrapper LSE */
            status = CSL_lseSetConfig(socInfo->lseRegs, &hObj->lseCfg);
        }
        /* If configRegValidate is enabled, also set the golden register values */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (((FVID2_SOK == status) && (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate)) && (NULL != goldenRegVal))
        /* LDRA_JUSTIFY_END */
        {
            status = CSL_lseSetConfig(&(goldenRegVal->lseRegs), &hObj->lseCfg);
        }
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The negative test framework and test apps cannot reach this portion.
    For the previously called CSL function, the status is returned as fail if and only if either of the passed arguments is NULL.
    None of the arguments are accessible to us though the test framework.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Configure HTS */
        status = CSL_dmpacHtsSetThreadConfig(socInfo->htsRegs, &hObj->htsCfg);
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The negative test framework and test apps cannot reach this portion.
    For the previously called CSL function, the status is returned as fail if and only if either of the passed arguments is NULL.
    None of the arguments are accessible to us though the test framework.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if((hObj->isFocoUsed == UTRUE) && (FVID2_SOK == status))
    /* LDRA_JUSTIFY_END */
    {
        status = CSL_dmpacHtsSetThreadConfig(socInfo->htsRegs,
                                        &hObj->focoHtsCfg);
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The negative test framework and test apps cannot reach this portion.
    For the previously called CSL function, the status is returned as fail if and only if either of the passed arguments is NULL.
    None of the arguments are accessible to us though the test framework.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Start HTS Scheduler */
        status = CSL_dmpacHtsThreadStart(socInfo->htsRegs, &hObj->htsCfg);

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The negative test framework and test apps cannot reach this portion.
        For the previously called CSL function, the status is returned as fail if and only if either of the passed arguments is NULL.
        None of the arguments are accessible to us though the test framework.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if((hObj->isFocoUsed == UTRUE) && (FVID2_SOK == status))
        /* LDRA_JUSTIFY_END */
        {
            status = CSL_dmpacHtsThreadStart(socInfo->htsRegs, &hObj->focoHtsCfg);
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The negative test framework and test apps cannot reach this portion.
        For the previously called CSL function, the status is returned as fail if and only if either of the passed arguments is NULL.
        None of the arguments are accessible to us though the test framework.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            (void)CSL_dmpacHtsThreadStopAll(socInfo->htsRegs, &hObj->focoHtsCfg);
        }
        /* LDRA_JUSTIFY_END */

    }

    return (status);
}

static int32_t Vhwa_sdeSubmitRequest(Vhwa_M2mSdeInstObj *instObj,
                                     Vhwa_M2mSdeQueueObj *qObj)
{
    int32_t status = FVID2_SOK;
    Vhwa_M2mSdeHandleObj *hObj = NULL;

    GT_assert(VhwaSdeTrace, (NULL != instObj));
    GT_assert(VhwaSdeTrace, (NULL != qObj));
    GT_assert(VhwaSdeTrace, (NULL != qObj->hObj));

    hObj = qObj->hObj;
#if !defined(SOC_J722S)
    VhwaDmpacSdeSocReadBack *goldenRegVal = hObj->configRegMemPrms.configGoldenRegPtr;

    /* Submit Rings to the Ring Accelerator */

    /* Update the SdeStatusRegisterGroup with valid status register values for dynamic registers */
    if(VHWA_SAFETY_MODE_DISABLED != hObj->enableStatusRegValidate)
    {
        status = VhwaM2mSdeUpdateStatusRegGroup(hObj);
    }
#endif
    if (FVID2_SOK == status)
    {
        status = Vhwa_m2mSdeSubmitRing(instObj, hObj);
    }

#if !defined(SOC_J722S)
    /* Update the SdeConfigRegisterGroup with config register values for frame specific Static Config, INTD and HTS registers */
    if(VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate)
    {
        status = vhwaM2mSdeUpdateConfigRegGroup(goldenRegVal, instObj, hObj);
    }
#endif
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (CSL_PASS == status)
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
        Rationale:
        Effect on this unit:
        <justification end> */
        if (NULL != hObj->createArgs.getTimeStamp)
        {
            hObj->perfNum = hObj->createArgs.getTimeStamp();
        }
        /* LDRA_JUSTIFY_END */

        /* Start HTS pipeline */
        status = CSL_dmpacHtsPipelineStart(instObj->socInfo.htsRegs, &hObj->htsCfg);
        #ifdef TEST_SDE_WDTIMER_ERR_INTERRUPT
        uint32_t delay_cnt = 0;
        volatile uint32_t   *sdeCtrlReg = (volatile uint32_t *)(CSL_DMPAC0_PAR_PAR_SDE_S_VBUSP_MMR_VBUSP_MMR_BASE + 0x4);
        while(instObj->socInfo.htsRegs->PIPELINE_CONTROL[(uint32_t)hObj->htsCfg.pipeline] & 0x1u)
        {
            delay_cnt++;
            if(delay_cnt > 2000)
            {
                uint32_t regVal = CSL_REG32_RD(sdeCtrlReg);
                regVal &= ~0x2U;
                CSL_REG32_WR(sdeCtrlReg, regVal);
            break;
            }
        }
        #endif
        #endif
    }

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The negative test framework and test apps cannot reach this portion.
    CSL_dmpacHtsPipelineStart returns a failure status only if it receives NULL arguments, which cannot occur through any valid test configuration.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (CSL_PASS != status)
    {
        instObj->actQObj = NULL;
        status = FVID2_EFAIL;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        status = FVID2_SOK;
    }

    return (status);
}

static int32_t Vhwa_m2mSdeSetIntrInHW(uint32_t vhwaIrqNum,
                                volatile CSL_dmpac_intd_cfgRegs  *intdRegs,
                                const Vhwa_M2mSdeHandleObj *hObj)
{
    int32_t status = FVID2_SOK;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
	Rationale: The component level negative test framework and test applications cannot reach this portion.
	The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == intdRegs) || (NULL == hObj))
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        /* Enable Selected HTS Interrupt in INTD Module */
        CSL_REG32_WR(&intdRegs->ENABLE_CLR_REG_LEVEL_DMPAC_OUT[vhwaIrqNum][VHWA_DMPAC_INTD_SDE_INTR_REG], VHWA_SDE_ALL_ERR_EVENTS);
        CSL_REG32_WR(&intdRegs->ENABLE_REG_LEVEL_DMPAC_OUT[vhwaIrqNum][VHWA_DMPAC_INTD_SDE_INTR_REG],
            hObj->eePrms.errEvents);

        CSL_REG32_WR(&intdRegs->ENABLE_CLR_REG_LEVEL_DMPAC_OUT[vhwaIrqNum][VHWA_DMPAC_INTD_WDTIMER_INTR_REG], VHWA_SDE_ALL_WDTIMER_ERR_EVENTS);
        CSL_REG32_WR(&intdRegs->ENABLE_REG_LEVEL_DMPAC_OUT[vhwaIrqNum][VHWA_DMPAC_INTD_WDTIMER_INTR_REG],
            hObj->wdTimerEePrms.wdTimerErrEvents);
    }

    return status;
}

static int32_t Vhwa_sdeCreateQueues(Vhwa_M2mSdeInstObj *instObj)
{
    int32_t              status = FVID2_SOK;
    uint32_t             qCnt;
    Vhwa_M2mSdeQueueObj *qObj;

    /* NULL pointer check */
    GT_assert(VhwaSdeTrace, (NULL != instObj));

    instObj->freeQ = NULL;

    /* Create Free queue */
    status = Fvid2Utils_constructQ(&instObj->freeQObj);
    GT_assert(VhwaSdeTrace, (FVID2_SOK == status));

    instObj->freeQ = &instObj->freeQObj;

    /* Initialize and queue the allocate queue object to free Q */
    for(qCnt = 0U; qCnt < VHWA_M2M_SDE_UDMA_RING_ENTRIES; qCnt ++)
    {
        qObj = &instObj->sdeQObj[qCnt];

        Fvid2Utils_memset(qObj, 0x0, sizeof(Vhwa_M2mSdeQueueObj));

        Fvid2Utils_queue(instObj->freeQ, &qObj->qElem, qObj);
    }

    return (status);
}

static void Vhwa_sdeDeleteQueues(Vhwa_M2mSdeInstObj *instObj)
{
    Vhwa_M2mSdeQueueObj *qObj = NULL;

    /* NULL pointer check */
    GT_assert(VhwaSdeTrace, (NULL != instObj));

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The negative test framework and test apps cannot reach this portion.
    No app configuration will result in call to Vhwa_sdeDeleteQueues if the queue is already empty.
    Effect on this unit: If the queue is already NULL, no action is needed.
    <justification end> */
    if(NULL != instObj->freeQ)
    /* LDRA_JUSTIFY_END */
    {
        /* Free-up all the queued free queue objects */
        do
        {
            qObj = (Vhwa_M2mSdeQueueObj *) Fvid2Utils_dequeue(instObj->freeQ);
        } while (NULL != qObj);

        /* Delete the free Q */
        Fvid2Utils_destructQ(instObj->freeQ);
        instObj->freeQ = NULL;
    }
}

/* LDRA_JUSTIFY_START
<metric start> statement <metric end>
<justification start>
Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
        However, due to the stated rationale, this is not tested.
<justification end> */
static void Vhwa_sdeSetHtsLimitParams(Vhwa_M2mSdeHandleObj *hObj,
                                      const Vhwa_HtsLimiter *htsLimit)
{
    /* Setting directly in the HTS configuration, so that
     * it will be configured from the next request. */
    hObj->htsCfg.enableBwLimit   = htsLimit->enableBwLimit;
    hObj->htsCfg.cycleCnt        = htsLimit->cycleCnt;
    hObj->htsCfg.tokenCnt        = htsLimit->tokenCnt;
}
/* LDRA_JUSTIFY_END */

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
Rationale: The test framework and test apps cannot reach this portion.
Used only in Vhwa_m2mSdePopRings to handle out-of-order UDMA interrupts, for which no test configuration exists yet.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
Vhwa_M2mSdeHandleObj *Vhwa_m2mSdeGetHandleObj(uint32_t cnt)
{
    return &gM2mSdeHandleObj[cnt];
}
/* LDRA_JUSTIFY_END */

#if !defined(SOC_J722S)
static int32_t Vhwa_m2mSdeReconfigReinitReg(const Vhwa_M2mSdeInstObj *instObj, const Vhwa_M2mSdeHandleObj *hObj, const Vhwa_M2mSdeQueueObj *qObj)
{
    int32_t status = FVID2_SOK;

    /* Null pointer check */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
	Rationale: The component level negative test framework and test applications cannot reach this portion.
	The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == instObj) || (NULL == hObj) || (NULL == qObj))
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        VhwaDmpacSdeSocReadBack *goldenRegVal = hObj->configRegMemPrms.configGoldenRegPtr;

        if (instObj->lastHndlObj == qObj->hObj)
        {
            /** Last handle was same as new handle,
             *  so Buffer Parameters have already been reconfigured */
            status = CSL_sdeSetBufParam(instObj->socInfo.sdeRegs,
                                            &hObj->sdeBuffPrms);
            /* If configRegValidate is enabled, also set the golden register values */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((FVID2_SOK == status) && (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate) && (NULL != goldenRegVal))
            /* LDRA_JUSTIFY_END */
            {
                status = CSL_sdeSetBufParam(&(goldenRegVal->sdeRegs),
                                            &hObj->sdeBuffPrms);
            }

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if(FVID2_SOK == status)
            /* LDRA_JUSTIFY_END */
            {
                status = Vhwa_m2mSdeSetConfigInHW((const Vhwa_M2mSdeInstObj *)instObj, qObj);
            }
        }
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
            CSL_dmpacEnableModule(instObj->socInfo.dmpacCntlRegs,
                DMPAC_MODULE_SDE, (uint32_t)UTRUE);
        }
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
            /* Re-Init Pipeline interrupt in INTD */
            Vhwa_enableDmpacHtsIntr(instObj->socInfo.dmpacIntdRegs,
                instObj->vhwaIrqNum, instObj->pipeline);
        }
    }
    return status;
}

static int32_t VhwaM2mSdeUpdateStatusRegGroup(Vhwa_M2mSdeHandleObj *hObj)
{
    SdeStatusRegisterGroup *statusRegs;
    int32_t status = FVID2_SOK;
    uint32_t expected_val = 0U;
    uint32_t i;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
	Rationale: The component level negative test framework and test applications cannot reach this portion.
	The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL == hObj)
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        statusRegs = &hObj->statusRegs;

        /* Update the expected values for SDE STATUS */
        statusRegs->sdeStatus.STATUS = VHWA_DMPAC_SDE_STATUS_IDLE;

        /* Update only the count related values in the HtsStatusRead structure */
        /* These are: PROD_COUNT, PROD_DMA_COUNT */
        for (i = 0; i < CSL_HTS_NUM_SDE_FOCO1_PRODUCERS; i++)
        {
            CSL_DmpacHtsSchConfig *cfg = NULL;
            CSL_DmpacHtsProdConfig *prodCfg = NULL;

            if(i < CSL_HTS_NUM_SDE_PRODUCERS)
            {
                /* SDE producers */
                cfg = &hObj->htsCfg;
                prodCfg = &cfg->prodCfg[i];
            }
            else
            {
                /* FOCO0 producers */
                uint32_t foco_idx = i - CSL_HTS_NUM_SDE_PRODUCERS;
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (hObj->isFocoUsed == UTRUE)
                /* LDRA_JUSTIFY_END */
                {
                    cfg = &hObj->focoHtsCfg;
                    prodCfg = &cfg->prodCfg[foco_idx];
                }
            }

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((cfg != NULL) && (prodCfg != NULL) && (prodCfg->enable == UTRUE))
            /* LDRA_JUSTIFY_END */
            {
                expected_val = 0U;
                CSL_FINS(expected_val, HTS_HWA1_PROD0_COUNT_COUNT_POSTLOAD, prodCfg->cntPostLoad);
                statusRegs->sdeHtsStatus.PROD_COUNT[i] = expected_val;
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
                statusRegs->sdeHtsStatus.PROD_COUNT[i] = 0U;
            }
            /* LDRA_JUSTIFY_END */
        }

        for (i = 0; i < CSL_HTS_NUM_SDE_FOCO1_PRODUCERS_DMA; i++)
        {
            const CSL_DmpacHtsSchConfig *cfg = NULL;
            const CSL_DmpacHtsDmaProdConfig *dmaProdCfg = NULL;

            if (i < CSL_HTS_NUM_SDE_PRODUCERS_DMA)
            {
                /* SDE DMA producers */
                cfg = &hObj->htsCfg;
                dmaProdCfg = &cfg->dmaProdCfg[i];
            }
            else
            {
                /* FOCO1 DMA producers */
                uint32_t foco_dma_idx = i - CSL_HTS_NUM_SDE_PRODUCERS_DMA;
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (hObj->isFocoUsed == UTRUE)
                /* LDRA_JUSTIFY_END */
                {
                    cfg = &hObj->focoHtsCfg;
                    dmaProdCfg = &cfg->dmaProdCfg[foco_dma_idx];
                }
            }

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((cfg != NULL) && (dmaProdCfg != NULL) && (dmaProdCfg->enable == UTRUE))
            /* LDRA_JUSTIFY_END */
            {
                expected_val = 0U;
                CSL_FINS(expected_val, HTS_DMA8_PROD0_COUNT_COUNT_POSTLOAD, dmaProdCfg->cntPostLoad);
                statusRegs->sdeHtsStatus.PROD_DMA_COUNT[i] = expected_val;
            }
            else
            {
                statusRegs->sdeHtsStatus.PROD_DMA_COUNT[i] = 0U;
            }
        }

    }

    return status;
}

int32_t Vhwa_m2mSdeStatusRegValidate(const SdeStatusRegisterGroup *expected, const Sde_SocInfo *socInfo)
{

    int32_t status = FVID2_SOK;
    uint32_t i;

    /* Null pointer check */
    /* LDRA_JUSTIFY_START
    <metric start> statementbranch <metric end>
    <justification start>
	Rationale: The component level negative test framework and test applications cannot reach this portion.
	The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((expected == NULL) || (socInfo == NULL))
    {
        status = FVID2_EBADARGS;
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
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        /* Validate PID values in CSL_dmpacRegs */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)VHWA_HW_PID_DMPAC != CSL_REG32_RD(&socInfo->dmpacCntlRegs->PID))
        {
            GT_0trace(VhwaSdeTrace, GT_ERR, "StatusRegValidate: dmpacCntlRegs->PID mismatch\n");
            status = FVID2_EFAIL;
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
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        /* Validate PID values in CSL_sdeRegs */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)VHWA_HW_PID_SDE != CSL_REG32_RD(&socInfo->sdeRegs->PID))
        {
            GT_0trace(VhwaSdeTrace, GT_ERR, "StatusRegValidate: sdeRegs->PID mismatch\n");
            status = FVID2_EFAIL;
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
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        /* Validate SDE status */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (expected->sdeStatus.STATUS != CSL_REG32_RD(&socInfo->sdeRegs->STATUS))
        {
            GT_0trace(VhwaSdeTrace, GT_ERR, "StatusRegValidate: sdeRegs->STATUS mismatch\n");
            status = FVID2_EFAIL;
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
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        /* Validate LSE status directly from registers using CSL_RD */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)FVID2_SOK != CSL_REG32_RD(&socInfo->lseRegs->STATUS_ERROR))
        {
            GT_0trace(VhwaSdeTrace, GT_ERR, "StatusRegValidate: lseRegs->STATUS_ERROR mismatch\n");
            status = FVID2_EFAIL;
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
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        /* Validate LSE status idle mode */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)FVID2_SOK != CSL_REG32_RD(&socInfo->lseRegs->STATUS_IDLE_MODE))
        {
            GT_0trace(VhwaSdeTrace, GT_ERR, "StatusRegValidate: lseRegs->STATUS_IDLE_MODE mismatch\n");
            status = FVID2_EFAIL;
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
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        /* Validate DST_BUF_CFG_CH_STATUS[4] of sdeFocoLseStatus using mask for CSL_LSE_DST_BUF_CFG_CH_DISABLED_MASK */
        for (i = 0; i < 4U; i++)
        {
            uint32_t expected_val = (expected->sdeFocoLseStatus.DST_BUF_CFG_CH_STATUS[i] & CSL_LSE_DST_BUF_CFG_CH_DISABLED_MASK);
            uint32_t reg_val = (CSL_REG32_RD(&socInfo->lseRegs->DST[i].BUF_CFG) & CSL_LSE_DST_BUF_CFG_CH_DISABLED_MASK);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (expected_val != reg_val)
            {
                GT_1trace(VhwaSdeTrace, GT_ERR, "StatusRegValidate: lseRegs->DST[%d].BUF_CFG mismatch\n", i);
                status = FVID2_EFAIL;
            }
            /* LDRA_JUSTIFY_END */
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
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {

        /* Validate HTS status directly from registers using CSL_RD */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        /* Validate HTS SCHEDULER_CONTROL for HWA1 (SDE) and HWA8 (FOCO1) */
        if (socInfo->htsRegs != NULL)
        /* LDRA_JUSTIFY_END */
        {
            uint32_t scheduler_base_addr;
            const CSL_DmpacHtsHwaParams *htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_SDE);
            scheduler_base_addr = (uint32_t)socInfo->htsRegs + htsPrms->schOffset;

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            /* Validate HTS SCHEDULER_CONTROL for HWA1 (SDE) */
            if ((uint32_t)FVID2_SOK !=
                (CSL_REG32_RD(scheduler_base_addr) & CSL_HTS_HWA1_SCHEDULER_CONTROL_STATE_MASK))
            {
                GT_0trace(VhwaSdeTrace, GT_ERR, "StatusRegValidate: htsRegs->HWA1_SCHEDULER_CONTROL mismatch\n");
                status = FVID2_EFAIL;
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
            if (status == FVID2_SOK)
            /* LDRA_JUSTIFY_END */
            {
                /* Validate HTS WD_TIMER status for HWA1 (SDE) */
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((uint32_t)FVID2_SOK !=
                    (CSL_REG32_RD(scheduler_base_addr + HWA_WDTIMER_OFFSET) & CSL_HTS_HWA1_WDTIMER_WDTIMER_STATUS_MASK))
                {
                    GT_0trace(VhwaSdeTrace, GT_ERR, "StatusRegValidate: htsRegs->HWA1_WDTIMER mismatch\n");
                    status = FVID2_EFAIL;
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
            if (status == FVID2_SOK)
            /* LDRA_JUSTIFY_END */
            {
                htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_FOCO2);
                scheduler_base_addr = (uint32_t)socInfo->htsRegs + htsPrms->schOffset;
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                /* Validate HTS SCHEDULER_CONTROL for HWA8 (FOCO1) */
                if ((uint32_t)FVID2_SOK !=
                    (CSL_REG32_RD(scheduler_base_addr) & CSL_HTS_HWA8_SCHEDULER_CONTROL_STATE_MASK))
                {
                    GT_0trace(VhwaSdeTrace, GT_ERR, "StatusRegValidate: htsRegs->HWA8_SCHEDULER_CONTROL mismatch\n");
                    status = FVID2_EFAIL;
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
            if (status == FVID2_SOK)
            /* LDRA_JUSTIFY_END */
            {
                /* Validate HTS WD_TIMER status for HWA8 (FOCO1) */
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((uint32_t)FVID2_SOK !=
                    (CSL_REG32_RD(scheduler_base_addr + HWA_WDTIMER_OFFSET) & CSL_HTS_HWA8_WDTIMER_WDTIMER_STATUS_MASK))
                {
                    GT_0trace(VhwaSdeTrace, GT_ERR, "StatusRegValidate: htsRegs->HWA8_WDTIMER mismatch\n");
                    status = FVID2_EFAIL;
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
            /* Validate HTS PROD_COUNT and PA_PRODCOUNT for all SDE_FOCO1 producers in a single loop */
            if (status == FVID2_SOK)
            /* LDRA_JUSTIFY_END */
            {
                uint32_t prod_count_addr;

                for (i = 0; i < CSL_HTS_NUM_SDE_FOCO1_PRODUCERS; i++)
                {
                    if (i < CSL_HTS_NUM_SDE_PRODUCERS)
                    {
                        /* SDE producers */
                        htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_SDE);
                        prod_count_addr = (uint32_t)socInfo->htsRegs + htsPrms->prodOffset[i];
                    }
                    else
                    {
                        /* FOCO1 producers */
                        htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_FOCO2);
                        uint32_t foco_idx = i - CSL_HTS_NUM_SDE_PRODUCERS;
                        prod_count_addr = (uint32_t)socInfo->htsRegs + htsPrms->prodOffset[foco_idx];
                    }

                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    The test framework does not support the configuration required to trigger this error scenario.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if ((expected->sdeHtsStatus.PROD_COUNT[i] & CSL_HTS_HWA1_PROD0_COUNT_COUNT_MASK) !=
                        (CSL_REG32_RD(prod_count_addr + HWA_PROD_COUNT_OFFSET) & CSL_HTS_HWA1_PROD0_COUNT_COUNT_MASK))
                    {
                        if (i < CSL_HTS_NUM_SDE_PRODUCERS)
                        {
                            GT_1trace(VhwaSdeTrace, GT_ERR, "StatusRegValidate: htsRegs->SDE_PROD_COUNT[%d] mismatch\n", i);
                        }
                        else
                        {
                            GT_1trace(VhwaSdeTrace, GT_ERR, "StatusRegValidate: htsRegs->FOCO1_PROD_COUNT[%d] mismatch\n", i - CSL_HTS_NUM_SDE_PRODUCERS);
                        }
                        status = FVID2_EFAIL;
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
                    if ((uint32_t)FVID2_SOK != (CSL_REG32_RD(prod_count_addr + HWA_PROD_PA_PROD_COUNT_OFFSET) & CSL_HTS_HWA1_PA0_PRODCOUNT_PA_PSCOUNT_MASK))
                    {
                        if (i < CSL_HTS_NUM_SDE_PRODUCERS)
                        {
                            GT_1trace(VhwaSdeTrace, GT_ERR, "StatusRegValidate: htsRegs->SDE_PA_PROD_COUNT[%d] mismatch\n", i);
                        }
                        else
                        {
                            GT_1trace(VhwaSdeTrace, GT_ERR, "StatusRegValidate: htsRegs->FOCO1_PA_PROD_COUNT[%d] mismatch\n", i - CSL_HTS_NUM_SDE_PRODUCERS);
                        }
                        status = FVID2_EFAIL;
                    }
                    /* LDRA_JUSTIFY_END */
                }
            }

            /* Validate PROD_DMA_SCHEDULER_CONTROL and PROD_DMA_COUNT */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This branch statement checks for errors accumulated in previous steps.
            Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (status == FVID2_SOK)
            /* LDRA_JUSTIFY_END */
            {
                for (i = 0; i < CSL_HTS_NUM_SDE_FOCO1_PRODUCERS_DMA; i++)
                {
                    uint32_t prod_dma_addr;
                    if (i < CSL_HTS_NUM_SDE_PRODUCERS_DMA)
                    {
                        /* SDE DMA producers */
                        htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_SDE);
                        prod_dma_addr = (uint32_t)socInfo->htsRegs + htsPrms->dmaProdOffset[i];
                    }
                    else
                    {
                        /* FOCO1 DMA producers */
                        htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_FOCO2);
                        uint32_t foco_dma_idx = i - CSL_HTS_NUM_SDE_PRODUCERS_DMA;
                        prod_dma_addr = (uint32_t)socInfo->htsRegs + htsPrms->dmaProdOffset[foco_dma_idx];
                    }

                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    The test framework does not support the configuration required to trigger this error scenario.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if ((uint32_t)FVID2_SOK !=
                        (CSL_REG32_RD(prod_dma_addr) & CSL_HTS_DMA8_SCHEDULER_CONTROL_STATE_MASK))
                    {
                        if (i < CSL_HTS_NUM_SDE_PRODUCERS_DMA)
                        {
                            GT_1trace(VhwaSdeTrace, GT_ERR, "StatusRegValidate: htsRegs->SDE_PROD_DMA_SCHEDULER_CONTROL[%d] mismatch\n", i);
                        }
                        else
                        {
                            GT_1trace(VhwaSdeTrace, GT_ERR, "StatusRegValidate: htsRegs->FOCO1_PROD_DMA_SCHEDULER_CONTROL[%d] mismatch\n", i - CSL_HTS_NUM_SDE_PRODUCERS_DMA);
                        }
                        status = FVID2_EFAIL;
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
                    if ((expected->sdeHtsStatus.PROD_DMA_COUNT[i] & CSL_HTS_DMA8_PROD0_COUNT_COUNT_MASK) !=
                        (CSL_REG32_RD(prod_dma_addr + HWA_DMA_PROD_COUNT_OFFSET) & CSL_HTS_DMA8_PROD0_COUNT_COUNT_MASK))
                    {
                        if (i < CSL_HTS_NUM_SDE_PRODUCERS_DMA)
                        {
                            GT_1trace(VhwaSdeTrace, GT_ERR, "StatusRegValidate: htsRegs->SDE_PROD_DMA_COUNT[%d] mismatch\n", i);
                        }
                        else
                        {
                            GT_1trace(VhwaSdeTrace, GT_ERR, "StatusRegValidate: htsRegs->FOCO1_PROD_DMA_COUNT[%d] mismatch\n", i - CSL_HTS_NUM_SDE_PRODUCERS_DMA);
                        }
                        status = FVID2_EFAIL;
                    }
                    /* LDRA_JUSTIFY_END */
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
            /* Validate for DMA consumer scheduler control status */
            if (status == FVID2_SOK)
            /* LDRA_JUSTIFY_END */
            {
                for (i = 0; i < CSL_HTS_NUM_SDE_FOCO1_CONSUMERS_DMA; i++)
                {
                    uint32_t cons_dma_addr;
                    if (i < CSL_HTS_NUM_SDE_CONSUMERS_DMA)
                    {
                        /* SDE DMA consumers */
                        htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_SDE);
                        cons_dma_addr = (uint32_t)socInfo->htsRegs + htsPrms->dmaConsOffset[i];
                    }
                    else
                    {
                        /* FOCO1 DMA consumers */
                        htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_FOCO2);
                        uint32_t foco_dma_idx = i - CSL_HTS_NUM_SDE_CONSUMERS_DMA;
                        cons_dma_addr = (uint32_t)socInfo->htsRegs + htsPrms->dmaConsOffset[foco_dma_idx];
                    }

                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    The test framework does not support the configuration required to trigger this error scenario.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if ((uint32_t)FVID2_SOK !=
                        (CSL_REG32_RD(cons_dma_addr) & CSL_HTS_DMA256_SCHEDULER_CONTROL_STATE_MASK))
                    {
                        if (i < CSL_HTS_NUM_SDE_CONSUMERS_DMA)
                        {
                            GT_1trace(VhwaSdeTrace, GT_ERR, "StatusRegValidate: htsRegs->SDE_CONSUMER_DMA_SCHEDULER_CONTROL[%d] mismatch\n", i);
                        }
                        else
                        {
                            GT_1trace(VhwaSdeTrace, GT_ERR, "StatusRegValidate: htsRegs->FOCO1_CONSUMER_DMA_SCHEDULER_CONTROL[%d] mismatch\n", i - CSL_HTS_NUM_SDE_CONSUMERS_DMA);
                        }
                        status = FVID2_EFAIL;
                    }
                    /* LDRA_JUSTIFY_END */
                }
            }
        }
    }

    return status;
}

/**
 * @brief Sets the default golden register memory values for the SDE (Stereo Disparity Estimation) module.
 *
 * This function initializes the register memory values to their default "golden" state
 * for the specified SDE handle and instance object. It is typically used to ensure
 * that the hardware registers are set to known, validated values before operation.
 *
 * @param hObj Pointer to the SDE handle object.
 * @param instObj Pointer to the SDE instance object.
 *
 * @return Returns 0 on success, or a negative error code on failure.
 */
int32_t vhwaM2mSdeSetDefaultGoldenRegMemValues(const Vhwa_M2mSdeHandleObj *hObj, const Vhwa_M2mSdeInstObj *instObj)
{
    VhwaDmpacSdeSocReadBack *goldenReg = NULL;
    int32_t status = FVID2_SOK;
    uint32_t reset_val = 0U;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == hObj) || (NULL == instObj))
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */

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
        goldenReg = hObj->configRegMemPrms.configGoldenRegPtr;
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL != goldenReg) && (FVID2_SOK == status))
    /* LDRA_JUSTIFY_END */
    {
        /* Foco CTRL registers are have non-zero reset values which are to be copied to both goldenReg memory. */
        CSL_dmpac_foco_coreRegs   *focoRegs  = instObj->socInfo.dmpacFocoRegs;
        for (uint32_t i = 0; i < DMPAC_MAX_FOCO_CHANNELS; i++)
        {
            reset_val = CSL_REG32_RD(&focoRegs->FOCO_CTRL[i].CH_CTRL);
            goldenReg->dmpacFocoRegs.FOCO_CTRL[i].CH_CTRL = reset_val;
        }
    }
    else
    {
        status = FVID2_EBADARGS;
    }

    return status;
}

int32_t Vhwa_m2mSdeConfigRegReadback(const Vhwa_M2mSdeHandleObj *hObj, const Vhwa_M2mSdeInstObj *instObj)
{
    int32_t status = FVID2_SOK;
    CSL_dmpac_sdeRegs *sdeRegs = NULL;
    VhwaDmpacSdeSocReadBack *readback = NULL;
    CSL_dmpac_foco_coreRegs   *focoRegs  = NULL;
    CSL_lseRegs               *lseRegs   = NULL;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
	Rationale: The component level negative test framework and test applications cannot reach this portion.
	The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((hObj == NULL) || (instObj == NULL))
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */

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
        sdeRegs = instObj->socInfo.sdeRegs;
        readback = hObj->configRegMemPrms.configRegReadbackPtr;
        focoRegs  = instObj->socInfo.dmpacFocoRegs;
        lseRegs   = instObj->socInfo.lseRegs;
    }

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
	Rationale: The component level negative test framework and test applications cannot reach this portion.
	The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == readback) || (NULL == sdeRegs) || (NULL == focoRegs) || (NULL == lseRegs))
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {

        // Read back SDE core registers
        readback->sdeRegs.CTRL          = CSL_REG32_RD(&sdeRegs->CTRL);
        readback->sdeRegs.IMGRES        = CSL_REG32_RD(&sdeRegs->IMGRES);
        readback->sdeRegs.SRCHRNG       = CSL_REG32_RD(&sdeRegs->SRCHRNG);
        readback->sdeRegs.LRCHCK        = CSL_REG32_RD(&sdeRegs->LRCHCK);
        readback->sdeRegs.TXTFLT        = CSL_REG32_RD(&sdeRegs->TXTFLT);
        readback->sdeRegs.PNLTY         = CSL_REG32_RD(&sdeRegs->PNLTY);
        readback->sdeRegs.CONFMAPG0     = CSL_REG32_RD(&sdeRegs->CONFMAPG0);
        readback->sdeRegs.CONFMAPG1     = CSL_REG32_RD(&sdeRegs->CONFMAPG1);
        readback->sdeRegs.CONFMAPG2     = CSL_REG32_RD(&sdeRegs->CONFMAPG2);
        readback->sdeRegs.CONFMAPG3     = CSL_REG32_RD(&sdeRegs->CONFMAPG3);
        readback->sdeRegs.BASEIMGADDR   = CSL_REG32_RD(&sdeRegs->BASEIMGADDR);
        readback->sdeRegs.BASEIMGWD     = CSL_REG32_RD(&sdeRegs->BASEIMGWD);
        readback->sdeRegs.REFIMGADDR    = CSL_REG32_RD(&sdeRegs->REFIMGADDR);
        readback->sdeRegs.REFIMGWD      = CSL_REG32_RD(&sdeRegs->REFIMGWD);
        readback->sdeRegs.DISPBUFCFG    = CSL_REG32_RD(&sdeRegs->DISPBUFCFG);
        readback->sdeRegs.IRCBUF        = CSL_REG32_RD(&sdeRegs->IRCBUF);
        readback->sdeRegs.PSA_CTRL      = CSL_REG32_RD(&sdeRegs->PSA_CTRL);


        /* Read back FOCO core registers */
        for (uint32_t i = 0; i < DMPAC_MAX_FOCO_CHANNELS; i++)
        {
            readback->dmpacFocoRegs.FOCO_CTRL[i].CH_CTRL  = CSL_REG32_RD(&focoRegs->FOCO_CTRL[i].CH_CTRL);
            readback->dmpacFocoRegs.FOCO_CTRL[i].CH_COUNT = CSL_REG32_RD(&focoRegs->FOCO_CTRL[i].CH_COUNT);
            /* Reserved bytes are not read (Resv_16) */
        }

        /* Readback of LSE registers */
        {
            readback->lseRegs.CFG_LSE          = CSL_REG32_RD(&lseRegs->CFG_LSE);

            for (uint32_t i = 0U; i < CSL_LSE_SDE_MAX_INPUT_CH; i++)
            {
                readback->lseRegs.SRC[i].CFG        = CSL_REG32_RD(&lseRegs->SRC[i].CFG);
                readback->lseRegs.SRC[i].FRAME_SIZE = CSL_REG32_RD(&lseRegs->SRC[i].FRAME_SIZE);
                readback->lseRegs.SRC[i].BUF_ATTR   = CSL_REG32_RD(&lseRegs->SRC[i].BUF_ATTR);
                for (uint32_t j = 0U; j < CSL_LSE_SDE_MAX_OUTPUT_CH; j++)
                {
                    readback->lseRegs.SRC[i].BUF_BA[j] = CSL_REG32_RD(&lseRegs->SRC[i].BUF_BA[j]);
                }
            }

            for (uint32_t i = 0U; i < CSL_LSE_SDE_MAX_OUTPUT_CH; i++)
            {
                /* Read BUF_CFG ignoring the CSL_LSE_DST_BUF_CFG_CH_DISABLED_MASK bit */
                readback->lseRegs.DST[i].BUF_CFG = CSL_REG32_RD(&lseRegs->DST[i].BUF_CFG) & ~CSL_LSE_DST_BUF_CFG_CH_DISABLED_MASK;
                readback->lseRegs.DST[i].BUF_ATTR0  = CSL_REG32_RD(&lseRegs->DST[i].BUF_ATTR0);
                readback->lseRegs.DST[i].BUF_BA     = CSL_REG32_RD(&lseRegs->DST[i].BUF_BA);
            }
        }
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
	Rationale: The component level negative test framework and test applications cannot reach this portion.
	The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((readback != NULL) && (instObj != NULL))
    /* LDRA_JUSTIFY_END */
    {
        status = vhwaM2mSdeUpdateConfigRegGroup(readback, instObj, hObj);
    }

    return status;
}

int32_t vhwaM2mSdeUpdateConfigRegGroup(VhwaDmpacSdeSocReadBack *RegVal, const Vhwa_M2mSdeInstObj *instObj, const Vhwa_M2mSdeHandleObj *hObj)
{
    int32_t status = FVID2_SOK;
    volatile CSL_dmpac_intd_cfgRegs *intdRegs = NULL;
    volatile CSL_htsRegs *htsRegs = NULL;
    uint32_t              reg_val;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
	Rationale: The component level negative test framework and test applications cannot reach this portion.
	The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == RegVal) || (NULL == instObj) || (NULL == hObj))
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        /* Accumulate any static configuration registers not updated after init */
        /* If configRegValidate is enabled, also set the golden register values */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != instObj->socInfo.dmpacCntlRegs)
        /* LDRA_JUSTIFY_END */
        {
            CSL_dmpacEnableModule(&(RegVal->dmpacCntlRegs),
                DMPAC_MODULE_SDE, (uint32_t)UTRUE);
        }

        /* Accumulate INTD registers */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != instObj->socInfo.dmpacIntdRegs)
        /* LDRA_JUSTIFY_END */
        {
            intdRegs = instObj->socInfo.dmpacIntdRegs;
            uint32_t vhwaIrqNum = instObj->vhwaIrqNum;
            uint32_t counter = 0U;
            /* NOTE: Do not make this a loop, since the INTR macros (VHWA_DMPAC_INTD_PIPE_INTR_REG,
             * VHWA_DMPAC_INTD_SDE_INTR_REG, VHWA_DMPAC_INTD_WDTIMER_INTR_REG) can change in future.
             * Keeping this as explicit assignments ensures maintainability if the macros or their order change.
             */

            /* 0 -> VHWA_DMPAC_INTD_SDE_INTR_REG */
            RegVal->dmpacSdeIntd.ENABLE_REG_LEVEL_DMPAC_OUT[vhwaIrqNum][counter] =
            CSL_REG32_RD(&intdRegs->ENABLE_REG_LEVEL_DMPAC_OUT[vhwaIrqNum][VHWA_DMPAC_INTD_SDE_INTR_REG]);
            RegVal->dmpacSdeIntd.ENABLE_CLR_REG_LEVEL_DMPAC_OUT[vhwaIrqNum][counter] =
            CSL_REG32_RD(&intdRegs->ENABLE_CLR_REG_LEVEL_DMPAC_OUT[vhwaIrqNum][VHWA_DMPAC_INTD_SDE_INTR_REG]);
            counter += 1U;

            /* 1 -> VHWA_DMPAC_INTD_PIPE_INTR_REG */
            RegVal->dmpacSdeIntd.ENABLE_REG_LEVEL_DMPAC_OUT[vhwaIrqNum][counter] =
            CSL_REG32_RD(&intdRegs->ENABLE_REG_LEVEL_DMPAC_OUT[vhwaIrqNum][VHWA_DMPAC_INTD_PIPE_INTR_REG]);
            RegVal->dmpacSdeIntd.ENABLE_CLR_REG_LEVEL_DMPAC_OUT[vhwaIrqNum][counter] =
            CSL_REG32_RD(&intdRegs->ENABLE_CLR_REG_LEVEL_DMPAC_OUT[vhwaIrqNum][VHWA_DMPAC_INTD_PIPE_INTR_REG]);
            counter += 1U;

            /* 2 -> VHWA_DMPAC_INTD_WDTIMER_INTR_REG */
            RegVal->dmpacSdeIntd.ENABLE_REG_LEVEL_DMPAC_OUT[vhwaIrqNum][counter] =
            CSL_REG32_RD(&intdRegs->ENABLE_REG_LEVEL_DMPAC_OUT[vhwaIrqNum][VHWA_DMPAC_INTD_WDTIMER_INTR_REG]);
            RegVal->dmpacSdeIntd.ENABLE_CLR_REG_LEVEL_DMPAC_OUT[vhwaIrqNum][counter] =
            CSL_REG32_RD(&intdRegs->ENABLE_CLR_REG_LEVEL_DMPAC_OUT[vhwaIrqNum][VHWA_DMPAC_INTD_WDTIMER_INTR_REG]);
        }


        /* Accumulate HTS registers */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (instObj->socInfo.htsRegs != NULL)
        /* LDRA_JUSTIFY_END */
        {
            htsRegs = instObj->socInfo.htsRegs;
            const CSL_DmpacHtsHwaParams *htsPrms = NULL;
            uint32_t scheduler_base_addr;

            /* Read the HTS pipeline registers for SDE and FOCO2 */
            RegVal->sdeHts.PIPELINE_CONTROL[0U] = CSL_REG32_RD(&htsRegs->PIPELINE_CONTROL[(uint32_t)hObj->htsCfg.pipeline]);
            RegVal->sdeHts.PIPELINE_CONTROL[1U] = CSL_REG32_RD(&htsRegs->PIPELINE_CONTROL[(uint32_t)hObj->focoHtsCfg.pipeline]);

            /* Read the HTS HWA scheduler registers for SDE and FOCO2 */
            for (uint32_t schIdx = 0; schIdx < CSL_HTS_NUM_SDE_SCHEDULERS; schIdx++)
            {
                if (schIdx == 0U)
                {
                    htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_SDE);
                }
                else
                {
                    htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_FOCO2);
                }
                scheduler_base_addr = (uint32_t)instObj->socInfo.htsRegs + htsPrms->schOffset;

                reg_val = CSL_REG32_RD(scheduler_base_addr + HWA_SCH_CONTROL_OFFSET);
                RegVal->sdeHts.SCHEDULER_CONTROL[schIdx] =
                    (reg_val & CSL_HTS_HWA1_SCHEDULER_CONTROL_PAUSE_MASK) |
                    (reg_val & CSL_HTS_HWA1_SCHEDULER_CONTROL_STRM_EN_MASK) |
                    (reg_val & CSL_HTS_HWA1_SCHEDULER_CONTROL_PIPELINE_NUM_MASK) |
                    (reg_val & CSL_HTS_HWA1_SCHEDULER_CONTROL_SCH_EN_MASK) |
                    (reg_val & CSL_HTS_HWA1_SCHEDULER_CONTROL_DEBUG_RDY_MASK);

                RegVal->sdeHts.HOP[schIdx] = CSL_REG32_RD(scheduler_base_addr + HWA_HOP_OFFSET);

                reg_val = CSL_REG32_RD(scheduler_base_addr + HWA_WDTIMER_OFFSET);
                RegVal->sdeHts.WDTIMER[schIdx] =
                    (reg_val & CSL_HTS_HWA1_WDTIMER_WDTIMER_EN_MASK) |
                    (reg_val & CSL_HTS_HWA1_WDTIMER_WDTIMER_MODE_MASK);

                RegVal->sdeHts.BW_LIMITER[schIdx] = CSL_REG32_RD(scheduler_base_addr + HWA_BW_LIMITER_OFFSET);
            }

            /* Readback for consumer control */
            uint32_t cons_ctrl_addr;
            for (uint32_t i = 0; i < CSL_HTS_NUM_SDE_FOCO1_CONSUMERS; i++)
            {
                if (i < CSL_HTS_NUM_SDE_CONSUMERS)
                {
                    htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_SDE);
                    cons_ctrl_addr = (uint32_t)instObj->socInfo.htsRegs + htsPrms->consOffset[i];
                }
                else
                {
                    htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_FOCO2);
                    uint32_t foco_idx = i - CSL_HTS_NUM_SDE_CONSUMERS;
                    cons_ctrl_addr = (uint32_t)instObj->socInfo.htsRegs + htsPrms->consOffset[foco_idx];
                }
                RegVal->sdeHts.CONS_CONTROL[i] = CSL_REG32_RD(cons_ctrl_addr + HWA_CONS_CONTROL_OFFSET);
            }

            /* Readback for producers control */
            uint32_t prod_ctrl_addr;
            for (uint32_t i = 0; i < CSL_HTS_NUM_SDE_FOCO1_PRODUCERS; i++)
            {
                if (i < CSL_HTS_NUM_SDE_PRODUCERS)
                {
                    htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_SDE);
                    prod_ctrl_addr = (uint32_t)instObj->socInfo.htsRegs + htsPrms->prodOffset[i];
                }
                else
                {
                    htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_FOCO2);
                    uint32_t foco_idx = i - CSL_HTS_NUM_SDE_PRODUCERS;
                    prod_ctrl_addr = (uint32_t)instObj->socInfo.htsRegs + htsPrms->prodOffset[foco_idx];
                }
                RegVal->sdeHts.PROD_CONTROL[i] = CSL_REG32_RD(prod_ctrl_addr + HWA_PROD_CONTROL_OFFSET);
                RegVal->sdeHts.PROD_BUF_CONTROL[i] = CSL_REG32_RD(prod_ctrl_addr + HWA_PROD_BUF_CONTROL_OFFSET);

                reg_val = CSL_REG32_RD(prod_ctrl_addr + HWA_PROD_COUNT_OFFSET);
                RegVal->sdeHts.PROD_COUNT[i] =
                    (reg_val & CSL_HTS_HWA1_PROD0_COUNT_COUNT_POSTLOAD_MASK) |
                    (reg_val & CSL_HTS_HWA1_PROD0_COUNT_COUNT_PRELOAD_MASK);

                RegVal->sdeHts.PA_CONTROL[i] = CSL_REG32_RD(prod_ctrl_addr + HWA_PROD_PA_CONTROL_OFFSET);

                reg_val = CSL_REG32_RD(prod_ctrl_addr + HWA_PROD_PA_PROD_COUNT_OFFSET);
                RegVal->sdeHts.PA_PRODCOUNT[i] = (reg_val & CSL_HTS_HWA1_PA0_PRODCOUNT_PA_COUNT_DEC_MASK);
            }

            /* Readback producers DMA */
            uint32_t prod_dma_ctrl_addr;
            for (uint32_t i = 0; i < CSL_HTS_NUM_SDE_FOCO1_PRODUCERS_DMA; i++)
            {
                if (i < CSL_HTS_NUM_SDE_PRODUCERS_DMA)
                {
                    htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_SDE);
                    prod_dma_ctrl_addr = (uint32_t)instObj->socInfo.htsRegs + htsPrms->dmaProdOffset[i];
                }
                else
                {
                    htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_FOCO2);
                    uint32_t foco_dma_idx = i - CSL_HTS_NUM_SDE_PRODUCERS_DMA;
                    prod_dma_ctrl_addr = (uint32_t)instObj->socInfo.htsRegs + htsPrms->dmaProdOffset[foco_dma_idx];
                }
                reg_val = CSL_REG32_RD(prod_dma_ctrl_addr + HWA_DMA_PROD_SCH_CONTROL_OFFSET);
                RegVal->sdeHts.PROD_DMA_SCHEDULER_CONTROL[i] =
                    (reg_val & CSL_HTS_DMA8_SCHEDULER_CONTROL_DMA_CHANNEL_NO_MASK) |
                    (reg_val & CSL_HTS_DMA8_SCHEDULER_CONTROL_DEBUG_RDY_MASK) |
                    (reg_val & CSL_HTS_DMA8_SCHEDULER_CONTROL_PAUSE_MASK) |
                    (reg_val & CSL_HTS_DMA8_SCHEDULER_CONTROL_STRM_EN_MASK) |
                    (reg_val & CSL_HTS_DMA8_SCHEDULER_CONTROL_PIPELINE_NUM_MASK) |
                    (reg_val & CSL_HTS_DMA8_SCHEDULER_CONTROL_SCH_EN_MASK);

                RegVal->sdeHts.PROD_DMA_HOP[i] = CSL_REG32_RD(prod_dma_ctrl_addr + HWA_DMA_PROD_HOP_OFFSET);
                RegVal->sdeHts.PROD_CONTROL_DMA[i] = CSL_REG32_RD(prod_dma_ctrl_addr + HWA_DMA_PROD_CONTROL_OFFSET);
                RegVal->sdeHts.PROD_BUF_CONTROL_DMA[i] = CSL_REG32_RD(prod_dma_ctrl_addr + HWA_DMA_PROD_BUF_CONTROL_OFFSET);

                reg_val = CSL_REG32_RD(prod_dma_ctrl_addr + HWA_DMA_PROD_COUNT_OFFSET);
                RegVal->sdeHts.PROD_COUNT_DMA[i] =
                    (reg_val & CSL_HTS_DMA8_PROD0_COUNT_COUNT_POSTLOAD_MASK) |
                    (reg_val & CSL_HTS_DMA8_PROD0_COUNT_COUNT_PRELOAD_MASK);
            }

            /* Readback of consumers DMA */
            uint32_t cons_dma_ctrl_addr;
            for (uint32_t i = 0; i < CSL_HTS_NUM_SDE_FOCO1_CONSUMERS_DMA; i++)
            {
                if (i < CSL_HTS_NUM_SDE_CONSUMERS_DMA)
                {
                    htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_SDE);
                    cons_dma_ctrl_addr = (uint32_t)instObj->socInfo.htsRegs + htsPrms->dmaConsOffset[i];
                }
                else
                {
                    htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_FOCO2);
                    uint32_t foco_dma_idx = i - CSL_HTS_NUM_SDE_CONSUMERS_DMA;
                    cons_dma_ctrl_addr = (uint32_t)instObj->socInfo.htsRegs + htsPrms->dmaConsOffset[foco_dma_idx];
                }
                reg_val = CSL_REG32_RD(cons_dma_ctrl_addr + HWA_DMA_CONS_SCH_CONTROL_OFFSET);
                RegVal->sdeHts.CONS_DMA_SCHEDULER_CONTROL[i] =
                    (reg_val & CSL_HTS_DMA256_SCHEDULER_CONTROL_DMA_CHANNEL_NO_MASK) |
                    (reg_val & CSL_HTS_DMA256_SCHEDULER_CONTROL_DEBUG_RDY_MASK) |
                    (reg_val & CSL_HTS_DMA256_SCHEDULER_CONTROL_PAUSE_MASK) |
                    (reg_val & CSL_HTS_DMA256_SCHEDULER_CONTROL_STRM_EN_MASK) |
                    (reg_val & CSL_HTS_DMA256_SCHEDULER_CONTROL_PIPELINE_NUM_MASK) |
                    (reg_val & CSL_HTS_DMA256_SCHEDULER_CONTROL_SCH_EN_MASK);

                RegVal->sdeHts.CONS_CONTROL_DMA[i] = CSL_REG32_RD(cons_dma_ctrl_addr + HWA_DMA_CONS_CONTROL_OFFSET);
            }
        }
    }

    return status;
}
#endif
int32_t Vhwa_m2mSdeGetSl2Info(Vhwa_M2mSdeSl2Info *sl2Info)
{
    int32_t retVal = FVID2_SOK;
    Vhwa_M2mSdeInstObj *instObj = &gM2mSdeInstObj;

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
        GT_0trace(VhwaSdeTrace, GT_ERR,
            "SL2 Memory is not allocated. Call Vhwa_m2mSdeAllocSl2() first.\n");
        retVal = FVID2_EFAIL;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        sl2Info->sl2Addr = instObj->sl2StartAddr;
        sl2Info->sl2Size = instObj->sl2Size;
    }

    return retVal;
}

