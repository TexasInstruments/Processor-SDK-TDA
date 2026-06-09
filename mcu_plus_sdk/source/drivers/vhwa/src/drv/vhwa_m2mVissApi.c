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
 *  \file vhwa_m2mVissApi.c
 *
 *  \brief API Implementation
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
static Vhwa_M2mVissHandleObj *vhwaM2mVissAllocHandleObj(
    const Vhwa_M2mVissInstObj *instObj);

/**
 * \brief   Local function to freeup allocated Handle Object and return it
 *          to the pool of free handle objects.
 *          No protection inside the function, Caller should protect
 *          the function call
 *
 * \param   hObj                Handle Object to be freed up.
 *
 **/
static void vhwaM2mVissFreeHandleObj(Vhwa_M2mVissHandleObj *hObj);

/**
 * \brief   Local function to check VISS Create params like Driver Id and Instance Id.
 *
 * \param   drvId               Driver ID
 * \param   drvInstId           Instance ID
 *
 *  \return FVID2_SOK if successful, else suitable error code
 **/
static int32_t vhwaM2mVissCheckCreatePrms(uint32_t drvId, uint32_t drvInstId);

/**
 * \brief   FVID2 Process Request function.
 *
 * \param   handle              FVID2 driver handle.
 *
 * \return  FVID2_SOK on success, else FVID2 error code
 *
 **/
Int32 vhwa_m2mVissProcessReq(Fdrv_Handle handle, Fvid2_FrameList *inFrmList,
    Fvid2_FrameList *outFrmList, uint32_t timeout);


/**
 * \brief   FVID2 Get Process Request function.
 *
 * \param   handle              FVID2 driver handle.
 *
 * \return  FVID2_SOK on success, else FVID2 error code
 *
 **/
Int32 vhwa_m2mVissGetProcessReq(Fdrv_Handle handle,
    Fvid2_FrameList *inFrmList, Fvid2_FrameList *outFrmList,
    UInt32 timeout);

int32_t vhwaVissCheckFrameList(Vhwa_M2mVissHandleObj *hObj,
    const Fvid2_FrameList *inFrmList, const Fvid2_FrameList *outFrmList);
static int32_t vhwaM2mVissSetParams(Vhwa_M2mVissInstObj *instObj,
    Vhwa_M2mVissHandleObj *hObj, Vhwa_M2mVissParams *vsPrms);
static int32_t vhwaM2mVissSubmitRequest(Vhwa_M2mVissInstObj *instObj,
    Vhwa_M2mVissQueueObj *qObj);
static int32_t vhwaM2mVissSetConfigInHW(Vhwa_M2mVissInstObj *instObj,
    const Vhwa_M2mVissQueueObj *qObj);
static void vhwaM2mVissCalcBlankParams(Vhwa_M2mVissHandleObj *hObj,
    const Vhwa_M2mVissParams *vsPrms);
static int32_t vhwaM2mVissCheckParams(Vhwa_M2mVissInstObj *instObj,
    Vhwa_M2mVissHandleObj *hObj, Vhwa_M2mVissParams *vsPrms);
static int32_t vhwaM2mVissCreateQueues(Vhwa_M2mVissInstObj *instObj);
static void vhwaM2mVissDeleteQueues(Vhwa_M2mVissInstObj *instObj);
static int32_t vhwaM2mVissAllocSl2(Vhwa_M2mVissInstObj *instObj,
    const Vhwa_M2mVissSl2Params *sl2AllocPrms);

static void Vhwa_m2mVissUpdateFcConnPrms(Vhwa_M2mVissHandleObj *hObj,
                                    const Vhwa_M2mVissFcConPrms *sl2FcPrms);
static void Vhwa_m2mVissUpdateFcPrms(Vhwa_M2mVissHandleObj *hObj,
                                    const Vhwa_M2mVissFcUpdatePrms *fcPrms);

void Vhwa_m2mVissGetSl2Prms(Vhwa_M2mVissHandleObj *hObj,
                            Vhwa_M2mVissSl2Prms *sl2Prms);

static int32_t vhwaM2mVissSubmitGlbceCtxSaveRequest(Vhwa_M2mVissInstObj *instObj, const Vhwa_M2mVissHandleObj *hObj);

static int32_t vhwaM2mVissSubmitGlbceCtxRestoreRequest(Vhwa_M2mVissInstObj *instObj, const Vhwa_M2mVissHandleObj *hObj);
#if !defined(VHWA_VPAC_IP_REV_VPAC3L)
int32_t Vhwa_M2mVissConfigRegMemCompare(const Vhwa_M2mVissHandleObj *hObj, const Vhwa_M2mVissInstObj *instObj);

int32_t Vhwa_M2mVissUpdateBuffers(const Vhwa_M2mVissHandleObj *hObj);

/**
 * @brief Validates the VISS (Vision Imaging Subsystem) status registers against expected values.
 *
 * This function compares the current VISS status register values with the expected values
 * provided in the 'expected' parameter. It uses the SoC-specific information from 'socInfo'
 * to perform the validation according to the hardware configuration.
 *
 * @param expected Pointer to a VissStatusRegisterGroup structure containing the expected
 *                 status register values.
 * @param socInfo  Pointer to a Viss_SocInfo structure containing SoC-specific information
 *                 required for validation.
 *
 * @return Returns 0 (FVID2_SOK) if the status registers match the expected values,
 *         or a negative error code if there is a mismatch or an error occurs.
 */
int32_t Vhwa_m2mVissStatusRegValidate(Vhwa_M2mVissHandleObj *hObj, const Viss_SocInfo *socInfo);

/**
 * \brief   Updates the VissStatusRegisterGroup structure in the handle object with expected status register values.
 *
 * This function calculates and populates the expected values for various status registers
 * based on the current VISS configuration. It updates:
 * - VISS ERR_STATUS and DEBUG_STATUS registers (expected to be 0 for no errors)
 * - HTS PROD_COUNT, PA_PRODCOUNT, and PROD_DMA_COUNT registers based on HTS configuration
 *
 * \param   hObj    Pointer to the Vhwa_M2mVissHandleObj whose statusRegs should be updated.
 *
 * \return  FVID2_SOK on success, else error code
 */
static int32_t vhwaM2mVissUpdateStatusRegGroup(Vhwa_M2mVissHandleObj *hObj);

/**
 * \brief   Reconfigure VISS MMR registers as needed for the current handle/queue.
 *
 * \param   instObj   Instance Object.
 * \param   hObj      Handle Object.
 * \param   qObj      Queue Object.
 *
 * \return  FVID2_SOK on success, else FVID2 error code.
 */
static int32_t Vhwa_m2mVissReconfigReinitReg(Vhwa_M2mVissInstObj *instObj,
                                             Vhwa_M2mVissHandleObj *hObj,
                                             const Vhwa_M2mVissQueueObj *qObj);
#endif

/* Implementation of FVID2 APIs */

/**
 * \brief   FVID2 Create Function.
 *
 * \param   instObj             Instance object.
 *
 * \return  FVID2 Driver Handle.
 *
 **/
Fdrv_Handle vhwa_m2mVissCreate(UInt32 drvId, UInt32 drvInstId,
    Ptr createArgs, Ptr createStatusArgs, const Fvid2_DrvCbParams *cbPrms);

/**
 * \brief   FVID2 Delete Function.
 *
 * \param   handle              FVID2 driver handle.
 *
 * \return  FVID2_SOK on success, else FVID2 error code
 *
 **/
Int32 vhwa_m2mVissDelete(Fdrv_Handle handle, Ptr deleteArgs);

/**
 * \brief   FVID2 Control Function.
 *
 * \param   handle              FVID2 driver handle.
 *
 * \return  FVID2_SOK on success, else FVID2 error code
 *
 **/
Int32 vhwa_m2mVissControl(Fdrv_Handle handle, UInt32 cmd, Ptr cmdArgs,
    Ptr cmdStatusArgs);

/**
 * \brief   Implementation of REGISTER_ERR_CB ioctl.
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   eePrms              Error Event Parameters
 *
 * \return  FVID2_SOK in case of sucess, Error otherwise
 *
 **/
static int32_t vhwa_m2mVissSetEeParams(const Vhwa_M2mVissInstObj *instObj,
    Vhwa_M2mVissHandleObj *hObj, const Viss_ErrEventParams *eePrms);

/**
 * \brief   Implementation of REGISTER_WDTIMER_ERR_CB ioctl.
 *
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   wdTimerEePrms       Watchdog timer Configuration Parameters
 *
 * \return  FVID2_SOK in case of sucess, Error otherwise
 *
 **/
static int32_t vhwa_m2mVissSetWdTimerEeParams(const Vhwa_M2mVissInstObj *instObj,
    Vhwa_M2mVissHandleObj *hObj, const Viss_WdTimerErrEventParams *wdTimerEePrms);

/**
 * \brief   Set VISS and WDTIMER interrupt enable/disable in INTD registers.
 *
 * This function configures the interrupt enable/disable settings in the VPAC INTD
 * registers for VISS and Watchdog Timer error events, based on the handle object's
 * error event parameters.
 *
 * \param   vhwaIrqNum      VHWA IRQ number to configure.
 * \param   intdRegs        Pointer to VPAC INTD register overlay.
 * \param   hObj            Pointer to the VISS handle object containing error event params.
 *
 * \return  FVID2_SOK on success, else FVID2 error code.
 */
static int32_t Vhwa_m2mVissSetIntrInHW(uint32_t vhwaIrqNum,
                                volatile CSL_vpac_intd_cfgRegs  *intdRegs,
                                const Vhwa_M2mVissHandleObj *hObj);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

Vhwa_M2mVissHandleObj gM2mVissHandleObj[VHWA_M2M_VISS_MAX_HANDLES];
Vhwa_M2mVissInstObj   gM2mVissInstObj[VHWA_M2M_VISS_DRV_MAX_INST];

Fvid2_DrvOps gM2mVissFvid2DrvOps = {
    FVID2_VHWA_M2M_VISS_DRV_ID,
    /**< Unique driver Id. */
    vhwa_m2mVissCreate,
    /**< FVID2 create function pointer. */
    vhwa_m2mVissDelete,
    /**< FVID2 delete function pointer. */
    vhwa_m2mVissControl,
    /**< FVID2 control function pointer. */
    NULL, NULL,
    /**< FVID2 queue function pointer. */
    vhwa_m2mVissProcessReq,
    /**< FVID2 process request function pointer. */
    vhwa_m2mVissGetProcessReq,
    /**< FVID2 get processed request function pointer. */
};


/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t Vhwa_m2mVissInit(Vhwa_M2mVissInitParams *initPrms)
{
    int32_t              status = FVID2_SOK;
    uint32_t             cnt;
    #if !defined(MCU_PLUS_SDK)
    SemaphoreP_Params   params;
    #endif
    Vhwa_M2mVissInstObj *instObj = NULL;

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the host kernel before the control reaches here.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
<justification end> */
    if (NULL != initPrms)
/* LDRA_JUSTIFY_END */
    {
        instObj = &gM2mVissInstObj[0U];

        /* Reset all instance object to 0x0 */
        Fvid2Utils_memset(instObj, 0U, sizeof (gM2mVissInstObj));

        /* Mark pool flags as free */
        for (cnt = 0U; cnt < VHWA_M2M_VISS_MAX_HANDLES; cnt++)
        {
            gM2mVissHandleObj[cnt].isUsed = (uint32_t) UFALSE;
        }

        /* Set HTS pipeline */
        instObj->pipeline = VHWA_M2M_VISS_HTS_PIPELINE;

        /* Initialize stalled flag to FALSE */
        instObj->isStalled = (uint32_t)UFALSE;

        Viss_getSocInfo(&instObj->socInfo);

        status = Fvid2_registerDriver(&gM2mVissFvid2DrvOps);
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
            GT_0trace(VhwaVissTrace, GT_ERR,
                "FVID2 driver registration failed!!\n");
            instObj->isRegistered = (uint32_t)UFALSE;
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
        /* Create Semaphores */
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
                GT_0trace(VhwaVissTrace, GT_ERR,
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
#if defined VHWA_M2M_VPAC_INSTANCE
#if (VHWA_M2M_VPAC_INSTANCE == 0)
            instObj->hwaLockIdx = VHWA_VPAC0_VISS_LOCK_IDX;
#elif defined(VHWA_M2M_VPAC_INSTANCE) && (VHWA_M2M_VPAC_INSTANCE == 1)
            instObj->hwaLockIdx = VHWA_VPAC1_VISS_LOCK_IDX;
#endif
#endif
            status = Vhwa_commonCreateHwaLock(instObj->hwaLockIdx);
        }

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start>
Rationale: The component level negative test framework and test applications cannot reach this portion.
This failure case is out of scope for the imaging test framework.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
However, due to the stated rationale, this is not tested.
<justification end> */
        /* Create free and request queues */
        if (FVID2_SOK == status)
/* LDRA_JUSTIFY_END */
        {
            status = vhwaM2mVissCreateQueues(instObj);
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
               required for VISS output */
            status = Vhwa_m2mVissUdmaInit(instObj, initPrms);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion as Vhwa_m2mVissUdmaInit() is calling with valid pointers of instObj and initPrms after the validation at the beginning of this function.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to print the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK != status)
            {
                GT_0trace(VhwaVissTrace, GT_ERR,
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
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
        <justification end> */
        /* Register ISR handler for the given irq number */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            instObj->irqNum = initPrms->irqInfo.irqNum;
            instObj->vhwaIrqNum = initPrms->irqInfo.vhwaIrqNum;

            status = Vhwa_m2mVissRegisterIsr(instObj);
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
        /* Init is done, copy init params locally,
           enable VPAC and VISS in VPAC Top */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            Fvid2Utils_memcpy(&instObj->initPrms, initPrms,
                sizeof(Vhwa_M2mVissInitParams));

            /* Enable VISS at VPAC Top*/
            CSL_vpacEnableModule(instObj->socInfo.vpacCntlRegs,
                VPAC_MODULE_VISS0, (uint32_t)UTRUE);

            /* Initialize Addresses of difference submodules */
            Vhwa_m2mVissInitAddresses(instObj);

            instObj->initDone = (uint32_t)UTRUE;
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
        /* Set up config UDMA to store register settings */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            bool saveConfig = true;

            instObj->initPrms.configThroughUdmaFlag =
                    initPrms->configThroughUdmaFlag;
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is NOT expected to result in an error.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (true == instObj->initPrms.configThroughUdmaFlag)
            /* LDRA_JUSTIFY_END */
            {
                /* Enable UDMA config channel */
                status = Vhwa_m2mVissStartConfigCh(instObj);
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This branch statement checks for errors accumulated in previous steps.
                Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
                Effect on this unit: The unit is NOT expected to result in an error. This behaviour is part of the application design.
                If the control reaches here, our code base is expected to print the error. However, due to the stated rationale, this is not tested.
                <justification end> */
                if (FVID2_SOK != status)
                {
                    GT_0trace(VhwaVissTrace, GT_ERR,
                            "UDMA enabling config channel failed !!\n");
                }
                /* LDRA_JUSTIFY_END */
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
                    /* Allocate UDMA config channel trmem */
                    status = Vhwa_m2mVissAllocConfigUdmaMem(instObj);
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This branch statement checks for errors accumulated in previous steps.
                Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
                Effect on this unit: The unit is NOT expected to result in an error. This behaviour is part of the application design.
                If the control reaches here, our code base is expected to print the error. However, due to the stated rationale, this is not tested.
                <justification end> */
                    if (FVID2_SOK != status)
                    {
                        GT_0trace(VhwaVissTrace, GT_ERR,
                                "UDMA config mem alloc failed !!\n");
                    }
                /* LDRA_JUSTIFY_END */
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
                if (FVID2_SOK == status)
                /* LDRA_JUSTIFY_END */
                {
                    /* before reading registers, enable glbce and nsf4 */
                    CSL_vissTopGlbceEnable(instObj->regAddrs.topRegs, UTRUE);
                    CSL_vissTopNsf4Enable(instObj->regAddrs.topRegs, UTRUE);

                    /* Save initial value of registers locally in instance object */
                    /* Initialize the buffer object to read registers */
                    Vhwa_m2mVissInitSaveBuffObject(instObj);

                    /* perform UDMA transfer to read all register data */
                    status = Vhwa_m2mVissSubmitSaveRestoreConfigUDMA(instObj,
                            NULL, saveConfig);
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    This branch statement checks for errors accumulated in previous steps.
                    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
                    Effect on this unit: The unit is NOT expected to result in an error. This behaviour is part of the application design.
                    If the control reaches here, our code base is expected to print the error. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if (FVID2_SOK != status)
                    {
                        GT_0trace(VhwaVissTrace, GT_ERR,
                                "UDMA config mem alloc failed !!\n");
                    }
                    /* LDRA_JUSTIFY_END */
                    /* disable glbce and nsf4 after reading data */
                    CSL_vissTopGlbceEnable(instObj->regAddrs.topRegs, UFALSE);
                    CSL_vissTopNsf4Enable(instObj->regAddrs.topRegs, UFALSE);
                }
            }
        }
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: The unit is NOT expected to result in an error. This behaviour is part of the application design.
    If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        status = FVID2_EBADARGS;
    }

    if (FVID2_SOK != status)
    {
        Vhwa_m2mVissDeInit();
    }
    /* LDRA_JUSTIFY_END */
    return (status);
}

void Vhwa_m2mVissDeInit(void)
{
    Vhwa_M2mVissInstObj *instObj = NULL;

    instObj = &gM2mVissInstObj[0U];

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> Rationale: The test framework and test apps cannot reach this portion.
            These parameters cannot be configured from test app or kernel directly and can only be overwritten by memory corruption. This is fail safe mechanism and cannot be removed.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
    if (instObj->openCnt > 0u)
    {
        GT_0trace(VhwaVissTrace, GT_ERR,
            "Warning: All driver handles are not closed!!\n");
    }
    /* LDRA_JUSTIFY_END */

    /* Stop UDMA channels */
    (void)Vhwa_m2mVissStopCh(instObj);

    Vhwa_m2mVissUnregisterIsr(instObj);

    (void)Vhwa_m2mVissUdmaDeInit(instObj);

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. An error print statement can be added in a future release if required.
    <justification end> */
    if ((uint32_t)UTRUE == instObj->isRegistered)
    /* LDRA_JUSTIFY_END */
    {
        (void)Fvid2_unRegisterDriver(&gM2mVissFvid2DrvOps);
    }

    vhwaM2mVissDeleteQueues(instObj);

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> Rationale: The test framework and test apps cannot reach this portion.
            These parameters cannot be configured from test app or kernel directly and can only be overwritten by memory corruption. This is fail safe mechanism and cannot be removed.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((uint32_t)UTRUE == instObj->isSl2AllocDone)
    /* LDRA_JUSTIFY_END */
    {
        Vhwa_m2mVissFreeSl2();
    }

    /* Delete the lock semaphore */
    #if defined(MCU_PLUS_SDK)
     SemaphoreP_destruct(&instObj->lock);
    #else
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> Rationale: The test framework and test apps cannot reach this portion.
            These parameters cannot be configured from test app or kernel directly and can only be overwritten by memory corruption. This is fail safe mechanism and cannot be removed.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != instObj->lock)
    /* LDRA_JUSTIFY_END */
    {
        (void)SemaphoreP_delete(instObj->lock);
        instObj->lock = NULL;
    }
    #endif

    Vhwa_commonDeleteHwaLock(instObj->hwaLockIdx);

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: The unit is NOT expected to result in an error. This behaviour is part of the application design.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if((bool)UTRUE == instObj->initPrms.copyGlbceCtxThroughBcdmaFlag)
    /* LDRA_JUSTIFY_END */
    {
        /* Delete the GLBCE context save/restore event semaphores */
        #if defined(MCU_PLUS_SDK)
        SemaphoreP_destruct(&instObj->glbceCqEventSemaphore);
        #else
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If the control reaches here, our code base is NOT expected to accumulate the error status.
        An else block can be added to catch and print the error status in a future release.
        <justification end> */
        if (NULL != instObj->glbceCqEventSemaphore)
        {
            (void)SemaphoreP_delete(instObj->glbceCqEventSemaphore);
            instObj->glbceCqEventSemaphore = NULL;
        }
        /* LDRA_JUSTIFY_END */
        #endif
    }

    /* Init all global variables to zero */
    Fvid2Utils_memset(instObj, 0U, sizeof (gM2mVissInstObj));

    instObj->initDone = (uint32_t)UFALSE;
}

int32_t Vhwa_m2mVissAllocSl2(const Vhwa_M2mVissSl2Params *sl2AllocPrms)
{
    int32_t              status = FVID2_SOK;
    Vhwa_M2mVissInstObj *instObj = NULL;

    instObj = &gM2mVissInstObj[0U];

    /* Cannot even lock, if init is not done */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> Rationale: The test framework and test apps cannot reach this portion.
            These parameters cannot be configured from test app or kernel directly and can only be overwritten by memory corruption. This is fail safe mechanism and cannot be removed.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((uint32_t)UFALSE == instObj->initDone)
    {
        GT_0trace(VhwaVissTrace, GT_ERR,
            "Driver init is not done!!\n");
        status = FVID2_EFAIL;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> Rationale: The test framework and test apps cannot reach this portion.
                The control cannot reach here without allocating the sl2 in the current test apps and kernel, therefore it is not possible to cover. Cannot be removed since this is a fail safe mechanism.
                Effect on this unit: If the control reaches here, it will resolve the sl2 allocation issue and keep the functionality integrated.
        <justification end> */
        if ((uint32_t)UTRUE == instObj->isSl2AllocDone)
        {
            GT_0trace(VhwaVissTrace, GT_ERR,
                "SL2 Memory is already allocated !!\n");
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */

        /* Still need to check if provided sl2AllocPrms is not null */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> Rationale: The test framework and test apps cannot reach this portion.
                These parameters cannot be configured from test app or kernel directly and can only be overwritten by memory corruption. This is fail safe mechanism and cannot be removed.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL == sl2AllocPrms)
        {
            GT_0trace(VhwaVissTrace, GT_ERR,
                "SL2 Params is null !!\n");
            status = FVID2_EBADARGS;
        }
        /* LDRA_JUSTIFY_END */
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> Rationale: The test framework and test apps cannot reach this portion.
            These parameters cannot be configured from test app or kernel directly and can only be overwritten by memory corruption. This is fail safe mechanism and cannot be removed.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Lock instance semaphore */
        (void)VhwaAl_SemaphoreP_pend(instObj->lock, SYSTEMP_WAIT_FOREVER);

        status = vhwaM2mVissAllocSl2(instObj, sl2AllocPrms);

        /* Release instance semaphore */
        (void)VhwaAl_SemaphoreP_post(instObj->lock);
    }

    return (status);
}

void Vhwa_m2mVissFreeSl2(void)
{
    Vhwa_M2mVissInstObj *instObj = NULL;

    instObj = &gM2mVissInstObj[0U];

    /* Lock instance semaphore */
    (void)VhwaAl_SemaphoreP_pend(instObj->lock, SYSTEMP_WAIT_FOREVER);

    Vhwa_FreeSl2(VHWA_SL2_INST_VPAC);

    instObj->sl2Addr = (uint64_t)NULL;
    instObj->sl2Size = 0x0U;

    /* Mark Sl2 Allocation flag to UFALSE */
    instObj->isSl2AllocDone = (uint32_t)UFALSE;

    /* Release instance semaphore */
    (void)VhwaAl_SemaphoreP_post(instObj->lock);
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
Rationale: The test framework and test apps cannot reach this portion
The function is unused because of an un-implemented debug feature, this code base is not being used/enabled with the current kernel support.
Effect on this unit: None; Unused feature, cannot be enabled, control cannot reach to the false condition.
<justification end> */
Vhwa_M2mVissHandleObj *Vhwa_m2mVissGetHandleObj(uint32_t cnt)
{
    return &gM2mVissHandleObj[cnt];
}
/* LDRA_JUSTIFY_END */

#if !defined(VHWA_VPAC_IP_REV_VPAC3L)
int32_t Vhwa_m2mVissReInit(void)
{
    int32_t                status = FVID2_SOK;
    Vhwa_M2mVissInstObj   *instObj = NULL;

    /** No lock is required, as reinit is called only when HW is not accessed by any other
     *  handle, so it is ok to access the members here. */
    instObj = &gM2mVissInstObj[0u];

    /** If isregistered is set, then Vhwa_m2mDofUdmaInit is called and UDMA
     * channels are initialized, so reinit is required. */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: The unit is NOT expected to result in an error. This behaviour is part of the application design.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((uint32_t)UTRUE == instObj->initDone)
    /* LDRA_JUSTIFY_END */
    {
        status = Vhwa_m2mVissUdmaUtcDeInit(instObj);

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
            /* Need to unregister ISR, so it can be re-registered in next step */
            Vhwa_m2mVissUnregisterIsr(instObj);
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
            /* Now reinitialized channels */
            status = Vhwa_m2mVissUdmaUtcInit(instObj, &instObj->initPrms);
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
            status = Vhwa_m2mVissRegisterIsr(instObj);
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
            /* Enable VISS at VPAC Top*/
            CSL_vpacEnableModule(instObj->socInfo.vpacCntlRegs,
                VPAC_MODULE_VISS0, (uint32_t)UTRUE);
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
            status = Vhwa_m2mVissStartUtcCh(instObj);
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
            Vhwa_enableHtsIntr(instObj->socInfo.vpacIntdRegs,
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
/*                          FVID2 Function implementation                     */
/* ========================================================================== */

Fdrv_Handle vhwa_m2mVissCreate(UInt32 drvId, UInt32 drvInstId,
    Ptr createArgs, Ptr createStatusArgs, const Fvid2_DrvCbParams *cbPrms)
{
    int32_t                 status = FVID2_SOK;
    Vhwa_M2mVissInstObj    *instObj = NULL;
    Vhwa_M2mVissHandleObj  *hObj = NULL;
    Vhwa_M2mVissCreateArgs *vissCreateArgs = NULL;
    Fdrv_Handle             handle = NULL;
    Viss_SocInfo           *socInfo = NULL;

    instObj = &gM2mVissInstObj[0U];
    socInfo = &instObj->socInfo;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    /* Check for errors */
    if ((NULL == createArgs) ||
        (NULL == cbPrms))
    {
        GT_0trace(VhwaVissTrace, GT_ERR, "NULL Pointer !!\n");
        status = FVID2_EBADARGS;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        status = vhwaM2mVissCheckCreatePrms(drvId, drvInstId);
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
    if (FVID2_SOK == status)
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
        /* Not allowed if init not called */
        if ((uint32_t)UFALSE == instObj->initDone)
        {
            GT_0trace(VhwaVissTrace, GT_ERR, "Init Not Done\n");
            status  = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */
        vissCreateArgs = (Vhwa_M2mVissCreateArgs *)createArgs;
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
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Take the instance semaphore */
        (void)VhwaAl_SemaphoreP_pend(instObj->lock, SYSTEMP_WAIT_FOREVER);

        /* Allocate Handle Object */
        hObj = vhwaM2mVissAllocHandleObj(instObj);
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
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the caller before the control reaches here.
                Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                <justification end> */
                /* Allocate SL2 Memory if not already allocated */
                if ((uint32_t)UFALSE == instObj->isSl2AllocDone)
                {
                    /* Initialize SL2 parameters with the defaul values */
                    Vhwa_m2mVissInitSl2Params(&instObj->sl2AllocPrms);

                    /* Allocate SL2 Parameters */
                    status = vhwaM2mVissAllocSl2(instObj,
                        &instObj->sl2AllocPrms);
                }
                /* LDRA_JUSTIFY_END */

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This branch statement checks for errors accumulated in previous steps.
                Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
                Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                <justification end> */
                /* On the first open, enable interrupts in HTS and
                 * start all UTC channels.
                 * Even if all UTC channels are enabled, only channels enabled
                 * in HTS will be used for the transfer */
                if (FVID2_SOK == status)
                /* LDRA_JUSTIFY_END */
                {
                    /* Start UDMA Channels on the first handle Create */
                    status = Vhwa_m2mVissStartCh(instObj);
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
                if (FVID2_SOK == status)
                /* LDRA_JUSTIFY_END */
                {
                    /* Enable Pipeline interrupt in INTD */
                    Vhwa_enableHtsIntr(socInfo->vpacIntdRegs,
                        instObj->vhwaIrqNum, instObj->pipeline);
                }
            }

            /* Incrementing openCnt, so that in case of error,
             * delete function can be called directly. */
            instObj->openCnt ++;
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This branch statement checks for errors accumulated in previous steps.
            Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
            <justification end> */
            /* Create doneQ, Done Q is specific to handle so that
             * FVID2_Dequeue returns correct request for that handle. */
            if (FVID2_SOK == status)
            /* LDRA_JUSTIFY_END */
            {
                status = Fvid2Utils_constructQ(&hObj->doneQObj);
                GT_assert(VhwaVissTrace, (FVID2_SOK == status));

                hObj->doneQ = &hObj->doneQObj;
            }

            /*
             * initialize appBuffInitDone and configInHwDone to false,
             * should be set after receiving valid buffer
             */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is NOT expected to result in an error.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (true == instObj->initPrms.configThroughUdmaFlag)
            /* LDRA_JUSTIFY_END */
            {
                hObj->appBuffInitDone = false;
                hObj->configInHwDone = false;
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
            if (FVID2_SOK == status)
            /* LDRA_JUSTIFY_END */
            {
                /* Allocate Descriptor, Ring memories */
                status = Vhwa_m2mVissAllocUdmaMem(instObj, hObj);
            }
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This branch statement checks for errors accumulated in previous steps.
            Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
            Effect on this unit: If the control reaches here, our code base is NOT expected to accumulate the error status.
            An else block can be added to catch and print the error status in a future release.
            <justification end> */
            if (FVID2_SOK == status)
            /* LDRA_JUSTIFY_END */
            {
                if(instObj->initPrms.copyGlbceCtxThroughBcdmaFlag == (bool)UTRUE)
                {
                    /* Allocate Descriptor, Ring memories for GLBCE cntx save/restore */
                    status = Vhwa_m2mVissAllocGlbceCntxSaveRestoreUdmaMem(instObj, hObj);
                }
            }
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This branch statement checks for errors accumulated in previous steps.
            Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
            Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK == status)
            /* LDRA_JUSTIFY_END */
            {
                Fvid2Utils_memcpy(&hObj->createArgs, vissCreateArgs,
                    sizeof(Vhwa_M2mVissCreateArgs));

                Fvid2Utils_memcpy(&hObj->cbPrms, cbPrms,
                    sizeof (hObj->cbPrms));
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
                vhwaM2mVissFreeHandleObj(hObj);
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
            status = FVID2_EALLOC;
        }
        /* LDRA_JUSTIFY_END */
        /* Posting here, so that delete can be called */
        (void)VhwaAl_SemaphoreP_post(instObj->lock);

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps.
        Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            handle = (Fdrv_Handle)hObj;
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
            /* No Need to check return status */
            (void)vhwa_m2mVissDelete((Fdrv_Handle)hObj, NULL);
        }
        /* LDRA_JUSTIFY_END */
    }

    return (handle);
}

Int32 vhwa_m2mVissDelete(Fdrv_Handle handle, Ptr deleteArgs)
{
    int32_t                 status = FVID2_SOK;
    Vhwa_M2mVissInstObj     *instObj = NULL;
    Vhwa_M2mVissHandleObj   *hObj = NULL;
    Viss_SocInfo            *socInfo = NULL;

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
        instObj = &gM2mVissInstObj[0U];
        hObj    = (Vhwa_M2mVissHandleObj *)handle;
        socInfo = &instObj->socInfo;

        /* Take the instance semaphore */
        (void)VhwaAl_SemaphoreP_pend(instObj->lock, SYSTEMP_WAIT_FOREVER);

        /* Check if all handles are already closed */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> Rationale: The negative test framework and test apps cannot reach this portion.
            These parameters cannot be configured from test app or kernel directly and can only be overwritten by memory corruption. This is fail safe mechanism and cannot be removed.
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
        <justification start> Rationale: The negative test framework and test apps cannot reach this portion.
            These parameters cannot be configured from test app or kernel directly and can only be overwritten by memory corruption. This is fail safe mechanism and cannot be removed.
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
        <justification start> Rationale: The negative test framework and test apps cannot reach this portion.
            These parameters cannot be configured from test app or kernel directly and can only be overwritten by memory corruption. This is fail safe mechanism and cannot be removed.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            /* Delete the Done queue */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> Rationale: The negative test framework and test apps cannot reach this portion.
                These parameters cannot be configured from test app or kernel directly and can only be overwritten by memory corruption. This is fail safe mechanism and cannot be removed.
            Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL != hObj->doneQ)
            /* LDRA_JUSTIFY_END */
            {
                Fvid2Utils_destructQ(hObj->doneQ);
                hObj->doneQ = NULL;
            }

            Vhwa_m2mVissStopModules(instObj, hObj);

            vhwaM2mVissFreeHandleObj(hObj);

            instObj->openCnt --;

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

Int32 vhwa_m2mVissControl(Fdrv_Handle handle, UInt32 cmd, Ptr cmdArgs,
    Ptr cmdStatusArgs)
{
    int32_t                  status = FVID2_SOK;
    Vhwa_M2mVissInstObj     *instObj = NULL;
    Vhwa_M2mVissHandleObj   *hObj = NULL;
    Vhwa_M2mVissParams      *vsPrms = NULL;
    Vhwa_HtsLimiter         *htsLimit = NULL;
    Viss_ErrEventParams     *eePrms = NULL;
    Viss_WdTimerErrEventParams *wdTimerEePrms = NULL;
#if !defined(VHWA_VPAC_IP_REV_VPAC3L)
    uint32_t                cookie;
#endif

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
    else
    /* LDRA_JUSTIFY_END */
    {
        instObj = &gM2mVissInstObj[0U];
        hObj = (Vhwa_M2mVissHandleObj *)handle;
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
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Take the instance semaphore */
        (void)VhwaAl_SemaphoreP_pend(instObj->lock, SYSTEMP_WAIT_FOREVER);

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. An error print statement can be added in a future release if required.
        <justification end> */
        switch (cmd)
        {
        /* LDRA_JUSTIFY_END */
            /* Set VISS Parameters */
            case IOCTL_VHWA_M2M_VISS_SET_PARAMS:
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
                    vsPrms = (Vhwa_M2mVissParams *)cmdArgs;
                    status = vhwaM2mVissSetParams(instObj, hObj, vsPrms);
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
                    GT_0trace(VhwaVissTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_VHWA_M2M_VISS_SET_PARAMS !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            /* SET HTS Limiter config */
            case IOCTL_VHWA_M2M_VISS_SET_HTS_LIMIT:
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
                    htsLimit = (Vhwa_HtsLimiter *)cmdArgs;

                    /* Setting directly in the HTS configuration, so that
                     * it will be configured from the next request. */
                    hObj->htsCfg.enableBwLimit   = htsLimit->enableBwLimit;
                    hObj->htsCfg.cycleCnt        = htsLimit->cycleCnt;
                    hObj->htsCfg.tokenCnt        = htsLimit->tokenCnt;
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
                    GT_0trace(VhwaVissTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_VHWA_M2M_VISS_SET_HTS_LIMIT !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }
            /* Get buffer details to write configuration through UDMA */
            case IOCTL_VHWA_M2M_VISS_GET_BUFF_INFO:
            {
                Vhwa_M2mVissConfigAppBuff *appBuff = NULL;
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != cmdArgs)
                /* LDRA_JUSTIFY_END */
                {
                    appBuff = (Vhwa_M2mVissConfigAppBuff*) cmdArgs;
                    /* calculate the buffer size needed */
                    hObj->appBufInfo.length = vhwa_m2mCalAppBufSize();
                    appBuff->length = hObj->appBufInfo.length;
                    appBuff->isCacheWriteBackPolicy = true;
                    appBuff->configThroughUdmaFlag =
                            instObj->initPrms.configThroughUdmaFlag;
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
                    GT_0trace(VhwaVissTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_VHWA_M2M_VISS_GET_BUFF_INFO !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            /* Set buffer details to write configuration through UDMA */
            case IOCTL_VHWA_M2M_VISS_SET_BUFF_INFO:
            {
                Vhwa_M2mVissConfigAppBuff *appBuff = NULL;
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the caller before the control reaches here.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((NULL != cmdArgs)
                    && (instObj->initPrms.configThroughUdmaFlag == true))
                /* LDRA_JUSTIFY_END */
                {
                    appBuff = (Vhwa_M2mVissConfigAppBuff*) cmdArgs;
                    /* copy the buffer pointer to the handle object */
                    hObj->appBufInfo.bufferPtr = appBuff->bufferPtr;
                    hObj->appBufInfo.isCacheWriteBackPolicy = appBuff->isCacheWriteBackPolicy;
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    The parameters are pre-validated by the caller before the control reaches here.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    /* validate the application buffer */
                    if ((hObj->appBufInfo.length == appBuff->length)
                        && (NULL != hObj->appBufInfo.bufferPtr))
                    /* LDRA_JUSTIFY_END */
                    {
                        /* process the received buffer */
                        status=Vhwa_m2mVissProcessAppBuf(instObj, hObj);
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
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This branch statement checks for errors accumulated in previous steps.
                Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
                Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaVissTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_VHWA_M2M_VISS_SET_BUFF_INFO !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            /* Ioctl to set RAW FE Configuration */
            case IOCTL_RFE_SET_CONFIG:
            {
                Rfe_Control *rfeCtrl = NULL;

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != cmdArgs)
                /* LDRA_JUSTIFY_END */
                {
                    rfeCtrl = (Rfe_Control *)cmdArgs;
                    status = Vhwa_m2mVissSetRfeConfig(instObj, hObj, rfeCtrl);
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
                    GT_0trace(VhwaVissTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_RFE_SET_CONFIG !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            /* Ioctl to Set FlexCC/FlexCFA configuration */
            case IOCTL_FCP_SET_CONFIG:
            {
                Fcp_Control *fcpCtrl = NULL;

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != cmdArgs)
                /* LDRA_JUSTIFY_END */
                {
                    fcpCtrl = (Fcp_Control *)cmdArgs;
                    status = Vhwa_m2mVissSetFcpConfig(instObj, hObj, fcpCtrl);
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
                    GT_0trace(VhwaVissTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_FCP_SET_CONFIG !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

        #if defined VHWA_VPAC_IP_REV_VPAC3
            /* Ioctl to Set FlexCC/FlexCFA configuration */
            case IOCTL_FCP2_SET_CONFIG:
            {
                Fcp_Control *fcpCtrl = NULL;

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != cmdArgs)
                /* LDRA_JUSTIFY_END */
                {
                    fcpCtrl = (Fcp_Control *)cmdArgs;
                    status = Vhwa_m2mVissSetFcp2Config(instObj, hObj, fcpCtrl);
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
                    GT_0trace(VhwaVissTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_FCP2_SET_CONFIG !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }
        #endif

            case IOCTL_FCP_GET_HISTOGRAM:
            {
                Fcp_HistData *histData = NULL;

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the caller before the control reaches here.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != cmdArgs)
                /* LDRA_JUSTIFY_END */
                {
                    histData = (Fcp_HistData *)cmdArgs;
                    status = Vhwa_m2mVissReadHistogram(instObj, hObj, histData);
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
                    GT_0trace(VhwaVissTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_FCP_GET_HISTOGRAM !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            /* Ioctl to set GLBCE Configuration */
            case IOCTL_GLBCE_SET_CONFIG:
            {
                Glbce_Control *glbceCtrl = NULL;

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != cmdArgs)
                /* LDRA_JUSTIFY_END */
                {
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    The parameters are pre-validated by the caller before the control reaches here.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if ((uint32_t)UTRUE == hObj->vsPrms.enableGlbce)
                    /* LDRA_JUSTIFY_END */
                    {
                        glbceCtrl = (Glbce_Control *)cmdArgs;
                        status = Vhwa_m2mVissSetGlbceConfig(instObj,
                            hObj, glbceCtrl);
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
                        status = FVID2_EINVALID_PARAMS;
                        GT_0trace(VhwaVissTrace, GT_ERR,
                            "GLBCE is disabled in SET_PARAMS!!\n");
                    }
                    /* LDRA_JUSTIFY_END */
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
                    GT_0trace(VhwaVissTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_GLBCE_SET_CONFIG !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            /* Ioctl to get GLBCE Cntxt save/restore external memory address */
            case IOCTL_GLBCE_SET_EXT_CNTXT_PTR:
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the caller before the control reaches here.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != cmdArgs)
                /* LDRA_JUSTIFY_END */
                {
                    hObj->glbceStatsExtMem = *(uint64_t *) cmdArgs;
                    hObj->glbceStatsMem = instObj->regAddrs.glbceStatsMem;
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
                    GT_0trace(VhwaVissTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_GLBCE_SET_EXT_CNTXT_PTR !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            /* Ioctl to get GLBCE Configuration */
            case IOCTL_GLBCE_GET_CONFIG:
            {
                Glbce_Control *glbceCtrl = NULL;

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != cmdArgs)
                /* LDRA_JUSTIFY_END */
                {
                    glbceCtrl = (Glbce_Control *)cmdArgs;
                    status = Vhwa_m2mVissGetGlbceConfig(instObj,
                        hObj, glbceCtrl);
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
                    GT_0trace(VhwaVissTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_GLBCE_GET_CONFIG !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            /* Ioctl to set GLBCE Configuration */
            case IOCTL_NSF4_SET_CONFIG:
            {
                Nsf4v_Config *nsf4Cfg = NULL;

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != cmdArgs)
                /* LDRA_JUSTIFY_END */
                {
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    The parameters are pre-validated by the caller before the control reaches here.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if ((uint32_t)UTRUE == hObj->vsPrms.enableNsf4)
                    /* LDRA_JUSTIFY_END */
                    {
                        nsf4Cfg = (Nsf4v_Config *)cmdArgs;
                        status = Vhwa_m2mVissSetNsf4Config(instObj,
                            hObj, nsf4Cfg);
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
                        status = FVID2_EINVALID_PARAMS;
                        GT_0trace(VhwaVissTrace, GT_ERR,
                            "NSF4 is disabled in SET_PARAMS!!\n");
                    }
                    /* LDRA_JUSTIFY_END */
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
                    GT_0trace(VhwaVissTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_NSF4_SET_CONFIG !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

        #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
            /* Ioctl to get NSF4 histogram */
            case IOCTL_NSF4_GET_HISTOGRAM:
            {
                Nsf4_Histogram *nsf4Hist = NULL;

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the caller before the control reaches here.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != cmdArgs)
                /* LDRA_JUSTIFY_END */
                {
                    nsf4Hist = (Nsf4_Histogram *)cmdArgs;
                    status = Vhwa_m2mVissGetNsf4Histogram(instObj,
                                        hObj, nsf4Hist);
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
                    GT_0trace(VhwaVissTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_NSF4_GET_HISTOGRAM !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }
        #endif

        #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
            /* Ioctl to set RAW histogram application pointer */
            case IOCTL_SET_RAW_HIST_APP_PTR:
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != cmdArgs)
                /* LDRA_JUSTIFY_END */
                {
                        hObj->nsf4HistExtAddr = (uint32_t *)cmdArgs;
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
                    GT_0trace(VhwaVissTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_SET_RAW_HIST_APP_PTR !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }
        #endif

            /* Ioctl to set H3A Configuration */
            case IOCTL_H3A_SET_CONFIG:
            {
                H3a_Config *h3aCfg = NULL;

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != cmdArgs)
                /* LDRA_JUSTIFY_END */
                {
                    /* No need to check if h3a is enabled in SET_PARAMS here,
                     * as it is checked in Vhwa_m2mVissSetH3aParams */
                    h3aCfg = (H3a_Config *)cmdArgs;
                    status = Vhwa_m2mVissSetH3aParams(instObj, hObj, h3aCfg);
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
                    GT_0trace(VhwaVissTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_H3A_SET_CONFIG !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

        #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
            /* Ioctl to set CAC Configuration */
            case IOCTL_CAC_SET_CONFIG:
            {
                Cac_Config *cacCfg = NULL;

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != cmdArgs)
                /* LDRA_JUSTIFY_END */
                {
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    The parameters are pre-validated by the caller before the control reaches here.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if ((uint32_t)UTRUE == hObj->vsPrms.enableCac)
                    /* LDRA_JUSTIFY_END */
                    {
                        cacCfg = (Cac_Config *)cmdArgs;
                        status = Vhwa_m2mVissSetCacConfig(instObj,
                            hObj, cacCfg);
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
                        status = FVID2_EINVALID_PARAMS;
                        GT_0trace(VhwaVissTrace, GT_ERR,
                            "GLBCE is disabled in SET_PARAMS!!\n");
                    }
                    /* LDRA_JUSTIFY_END */
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
                    GT_0trace(VhwaVissTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_CAC_SET_CONFIG !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }
        #endif /* #if defined VHWA_VPAC_IP_REV_VPAC3 or VPAC3L */

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> Rationale: The test framework and test apps cannot reach this portion.
                    The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
                    Effect on this unit: None;Unused feature, cannot be enabled, control cannot reach here.
            <justification end> */
            case IOCTL_VHWA_M2M_VISS_GET_PERFORMANCE:
            {
                uint32_t *perf = NULL;
                if (NULL != cmdArgs)
                {
                    perf = (uint32_t *)cmdArgs;
                    *perf = (uint32_t)hObj->perf;
                }
                else
                {
                    GT_0trace(VhwaVissTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_VHWA_M2M_VISS_GET_PERFORMANCE !!\n");
                    status = FVID2_EBADARGS;
                }
                break;
            }
            /* LDRA_JUSTIFY_END */

            case IOCTL_VHWA_M2M_VISS_SYNC_START:
            {
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start> Rationale: The test framework and test apps cannot reach this portion.
                        The function is unused because of an un-implemented debug feature, this code base is not being used/enabled with the current kernel support.
                        Effect on this unit: None;Unused feature, cannot be enabled, control cannot reach here.
                <justification end> */
                if (NULL != hObj->createArgs.getTimeStamp)
                {
                    hObj->perf = hObj->createArgs.getTimeStamp();
                }
                /* LDRA_JUSTIFY_END */
                status = CSL_htsPipelineStart(
                                    instObj->regAddrs.htsRegs, &hObj->htsCfg);
                break;
            }

            #if defined (VHWA_VPAC_IP_REV_VPAC3L)
            case IOCTL_PCID_SET_CONFIG:
            {
                Pcid_Cfg *pcidCfg = NULL;

                if (NULL != cmdArgs)
                {
                    if ((uint32_t)TRUE == hObj->vsPrms.enablePcid)
                    {
                        pcidCfg = (Pcid_Cfg *)cmdArgs;
                        status = Vhwa_m2mVissSetPcidConfig(instObj,
                            hObj, pcidCfg);
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
                        status = FVID2_EINVALID_PARAMS;
                        GT_0trace(VhwaVissTrace, GT_ERR,
                            "PCID is disabled in SET_PARAMS!!\n");
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
                    GT_0trace(VhwaVissTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_PCID_SET_CONFIG !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }
            #endif /* SOC_AM62AX, SOC_J722S*/
            case IOCTL_VHWA_VISS_FC_CONN_PARAMS:
            {
                Vhwa_M2mVissFcConPrms *pFcSl2Prms = NULL;
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
                    pFcSl2Prms = (Vhwa_M2mVissFcConPrms *)cmdArgs;
                    Vhwa_m2mVissUpdateFcConnPrms(hObj, pFcSl2Prms);
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
                    GT_0trace(VhwaVissTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_VHWA_VISS_FC_CONN_PARAMS !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                  break;
            }

            case IOCTL_VHWA_VISS_FC_UPDATE_PARAMS:
            {
                Vhwa_M2mVissFcUpdatePrms *pFcPrms = NULL;
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
                    pFcPrms = (Vhwa_M2mVissFcUpdatePrms *)cmdArgs;
                    Vhwa_m2mVissUpdateFcPrms(hObj, pFcPrms);
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
                    GT_0trace(VhwaVissTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_VHWA_VISS_FC_UPDATE_PARAMS !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                  break;
            }

            case IOCTL_VHWA_VISS_GET_SL2_PARAMS:
            {
                Vhwa_M2mVissSl2Prms *pSl2Prms = NULL;
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
                    pSl2Prms = (Vhwa_M2mVissSl2Prms *)cmdArgs;
                    Vhwa_m2mVissGetSl2Prms(hObj, pSl2Prms);

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
                    GT_0trace(VhwaVissTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_VHWA_VISS_GET_SL2_PARAMS !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                  break;
            }
            /* Enable Register error events callback */
            case IOCTL_VHWA_M2M_VISS_REGISTER_ERR_CB:
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != cmdArgs)
                /* LDRA_JUSTIFY_END */
                {
                    eePrms = (Viss_ErrEventParams *)cmdArgs;
                    status = vhwa_m2mVissSetEeParams(instObj, hObj, eePrms);
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
                    GT_0trace(VhwaVissTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_VHWA_M2M_VISS_REGISTER_ERR_CB !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }
            /* Configure watchdog timer */
            case IOCTL_VHWA_M2M_VISS_REGISTER_WDTIMER_ERR_CB:
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
                    wdTimerEePrms = (Viss_WdTimerErrEventParams *)cmdArgs;
                    status = vhwa_m2mVissSetWdTimerEeParams(instObj, hObj, wdTimerEePrms);
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
                    GT_0trace(VhwaVissTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_VHWA_M2M_VISS_REGISTER_WDTIMER_ERR_CB !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }
            /* Get PSA signature values for enabled outputs */
            case VHWA_M2M_IOCTL_VISS_GET_PSA_SIGN:
            {
                Vhwa_M2mVissPsaSign *psa = NULL;

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
                    psa = (Vhwa_M2mVissPsaSign *)cmdArgs;
                    *psa = hObj->psa;
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
                    GT_0trace(VhwaVissTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_VISS_GET_PSA_SIGN !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }
#if !defined(VHWA_VPAC_IP_REV_VPAC3L)
            case VHWA_M2M_IOCTL_VISS_ENABLE_RECONFIG_REINIT_REG:
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
                    GT_0trace(VhwaVissTrace, GT_ERR,
                        "handle/cmdArgs cannot be NULL for VHWA_M2M_IOCTL_VISS_ENABLE_RECONFIG_REINIT_REG !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case VHWA_M2M_IOCTL_VISS_ENABLE_STATUS_REG_VALIDATE:
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

                    /* Disable Interrupt since the below flag is accessed in the ISR */
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
                <metric start> statement <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaVissTrace, GT_ERR,
                        "handle/cmdArgs cannot be NULL for VHWA_M2M_IOCTL_VISS_ENABLE_STATUS_REG_VALIDATE !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case VHWA_M2M_IOCTL_VISS_VALIDATE_REG:
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
                    if (UTRUE == hObj->enableStatusRegValidate)
                    {
                        status = Vhwa_m2mVissStatusRegValidate(hObj, &instObj->socInfo);
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
                            GT_0trace(VhwaVissTrace, GT_ERR, "VISS: StatusRegValidate failed!\n");
                        }
                        /* LDRA_JUSTIFY_END */
                    }
                    if (UTRUE == hObj->enableConfigRegValidate)
                    {
                        /* Readback config registers */
                        status = Vhwa_m2mVissConfigRegReadback(hObj, instObj);
                    }

                    /* Release the hardware lock after the configReg readback or statusReg */
                    if ((UTRUE == hObj->enableStatusRegValidate) ||
                        (UTRUE == hObj->enableConfigRegValidate))
                    {
                        Vhwa_commonHwaLockRelease(VHWA_VPAC0_VISS_LOCK_IDX);
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
                        status = Vhwa_M2mVissUpdateBuffers(hObj);

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
                            status = Vhwa_M2mVissConfigRegMemCompare(hObj, instObj);
                            
                            /* LDRA_JUSTIFY_START
                            <metric start> statement branch <metric end>
                            <justification start>
                            Rationale: The component level negative test framework and test applications cannot reach this portion.
                            This failure case is out of scope for the imaging test framework.
                            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                            However, due to the stated rationale, this is not tested.
                            <justification end> */
                            if (FVID2_SOK != status)
                            {
                                GT_0trace(VhwaVissTrace, GT_ERR, "VISS: Vhwa_M2mVissConfigRegMemCompare failed!\n");
                            }
                            /* LDRA_JUSTIFY_END */
                        }
                        else
                        {
                            GT_0trace(VhwaVissTrace, GT_ERR, "VISS: Vhwa_M2mVissUpdateBuffers failed!\n");
                        }

                        /* Disable Interrupt since the below flag is accessed in the ISR and reset the configReg validate flag */
                        cookie = HwiP_disable();
                        hObj->enableConfigRegValidate = (uint32_t)UFALSE;
                        HwiP_restore(cookie);
                    }
                }
                else
                {
                    GT_0trace(VhwaVissTrace, GT_ERR,
                        "handle cannot be NULL for VHWA_M2M_IOCTL_VISS_VALIDATE_REG !!\n");
                    status = FVID2_EBADARGS;
                }
                break;
            }
            case VHWA_M2M_IOCTL_VISS_ENABLE_CONFIG_REG_READBACK:
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

                    /* Disable Interrupt since the below flag is accessed in the ISR */
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
                <metric start> statement <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaVissTrace, GT_ERR,
                        "handle/cmdArgs cannot be NULL for VHWA_M2M_IOCTL_VISS_ENABLE_CONFIG_REG_READBACK !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case VHWA_M2M_IOCTL_VISS_GET_READBACK_SIZE:
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
                    uint32_t *readbackSize = (uint32_t *)cmdArgs;
                    *readbackSize = vhwa_m2mCalAppBufSize();
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
                    GT_0trace(VhwaVissTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_VISS_GET_READBACK_SIZE !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case VHWA_M2M_IOCTL_VISS_SET_CONFIG_REG_PTRS:
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
                    Vhwa_M2mVissConfigRegMemParams *reg_ptrs = (Vhwa_M2mVissConfigRegMemParams *)cmdArgs;

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
                        GT_0trace(VhwaVissTrace, GT_ERR,
                            "readback_ptr, golden_ptr, cbFxn cannot be NULL for VHWA_M2M_IOCTL_VISS_SET_CONFIG_REG_PTRS !!\n");
                        status = FVID2_EBADARGS;
                    }
                    /* LDRA_JUSTIFY_END */
                    else
                    {
                        hObj->configRegMemPrms.configRegReadbackPtr = reg_ptrs->readback_ptr;
                        hObj->configRegMemPrms.configGoldenRegPtr = reg_ptrs->golden_ptr;
                        hObj->configRegMemPrms.cbFxn = reg_ptrs->cbFxn;
                        hObj->configRegMemPrms.appData = reg_ptrs->appData;

                        /* copy the buffer pointer to the handle object */
                        hObj->readbackBufferInfo.bufferPtr = reg_ptrs->readback_ptr;
                        hObj->readbackBufferInfo.isCacheWriteBackPolicy = true;
                        hObj->readbackBufferInfo.length = vhwa_m2mCalAppBufSize();

                        /* Partition the buffer address for the modules */
                        Vhwa_m2mVissInitReadbackBuffAddresses(hObj, hObj->readbackBufferInfo.bufferPtr);

                        /* Initialize buffer object for the handle */
                        Vhwa_m2mVissInitReadbackBuffObject(instObj, hObj, hObj->readbackBufferObjHolder);
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
                    GT_0trace(VhwaVissTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_VISS_SET_CONFIG_REG_PTRS !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }            
#endif
            /* Default Case */
            default:
            {
                status = FVID2_EUNSUPPORTED_CMD;
                break;
            }
        }

        (void)VhwaAl_SemaphoreP_post(instObj->lock);
    }

    return (status);
}

Int32 vhwa_m2mVissProcessReq(Fdrv_Handle handle, Fvid2_FrameList *inFrmList,
    Fvid2_FrameList *outFrmList, uint32_t timeout)
{
    int32_t                status = FVID2_SOK;
    uint32_t               semTimeout;
    Vhwa_M2mVissInstObj   *instObj = NULL;
    Vhwa_M2mVissHandleObj *hObj = NULL;
    Vhwa_M2mVissQueueObj  *qObj = NULL;

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
    else
    /* LDRA_JUSTIFY_END */
    {
        instObj = &gM2mVissInstObj[0U];
        hObj = (Vhwa_M2mVissHandleObj *)handle;

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
            GT_0trace(VhwaVissTrace, GT_ERR,
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
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        status = vhwaVissCheckFrameList(hObj, inFrmList, outFrmList);
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
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /** Steps
         *  1, lock the HW semaphore
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
        The test framework does not support the configuration required to trigger this error scenario.
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
        The test framework does not support the configuration required to trigger this error scenario.
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
         * number of pending request counter */
        hObj->numPendReq ++;

        /* Take the instance semaphore, so that no other
         * handle can submit request from the task context. */
        status = Vhwa_commonHwaLockAquire(instObj->hwaLockIdx, semTimeout);

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
            qObj = (Vhwa_M2mVissQueueObj *) Fvid2Utils_dequeue(instObj->freeQ);
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
                GT_0trace(VhwaVissTrace, GT_ERR,
                    "Failed to Free Queue Object!!\n");
                status = FVID2_EALLOC;
            }
            else
            /* LDRA_JUSTIFY_END */
            {
                qObj->hObj = hObj;
                /* Copy the application's process list to request
                 * objects lists */
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
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            /* HW is free, submit request to the hardware */
            /* If previous handle and current handles are same, */
            if (instObj->lastHndlObj == hObj)
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is NOT expected to result in an error.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (true == instObj->initPrms.configThroughUdmaFlag)
                /* LDRA_JUSTIFY_END */
                {
                    /* Update only dirty buffer object */
                    hObj->sameAsPrevHandle = true;
                }
            }
            else
            {
                /** Last handle was not same as new handle,
                 *  so require to recofigure all HW IPs */
                status = vhwaM2mVissSetConfigInHW(instObj, qObj);
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
                    if (true == instObj->initPrms.configThroughUdmaFlag)
                    {
                        /* Update only dirty buffer object */
                        hObj->sameAsPrevHandle = false;
                    }
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
            if (FVID2_SOK == status)
            /* LDRA_JUSTIFY_END */
            {
                /* Enable Selected HW Interrupts*/
                status = Vhwa_m2mVissSetIntrInHW(instObj->vhwaIrqNum, instObj->socInfo.vpacIntdRegs, hObj);
            }
#if !defined(VHWA_VPAC_IP_REV_VPAC3L)
            /* Invoke the reconfig-MMR if enableReconfigReinitReg enabled for the current handle */
            if (UTRUE == hObj->enableReconfigReinitReg)
            {
                status = Vhwa_m2mVissReconfigReinitReg(instObj, hObj, qObj);
                hObj->enableReconfigReinitReg = (uint32_t)UFALSE;
            }
#endif
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
                /** Set the addresses, Submit the TR
                 *  Start the pipeline */
                status = vhwaM2mVissSubmitRequest(instObj, qObj);
            }
        }
    }

    return (status);
}

/** \brief Typedef for FVID2 get processed frames function pointer. */
Int32 vhwa_m2mVissGetProcessReq(Fdrv_Handle handle,
    Fvid2_FrameList *inFrmList, Fvid2_FrameList *outFrmList,
    UInt32 timeout)
{
    int32_t                status = FVID2_SOK;
    uint32_t               cookie;
    Vhwa_M2mVissInstObj    *instObj = NULL;
    Vhwa_M2mVissHandleObj  *hObj = NULL;
    Vhwa_M2mVissQueueObj   *qObj = NULL;

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
    else
    /* LDRA_JUSTIFY_END */
    {
        instObj = &gM2mVissInstObj[0U];
        hObj = (Vhwa_M2mVissHandleObj *)handle;
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
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Take the instance semaphore, so that no other
         * handle can submit request from the task context. */
        (void)VhwaAl_SemaphoreP_pend(instObj->lock, SYSTEMP_WAIT_FOREVER);
        #if defined (MCU_PLUS_SDK)
        uint32_t               statusQ;
        /* Wait until completed request queue is populated.
         * Failure to exit this loop indicates that the ISR never hit.*/
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

        if(hObj->vsPrms.enableGlbceSaveRestoreCtx == UTRUE)
        {
            status = vhwaM2mVissSubmitGlbceCtxSaveRequest(instObj, hObj);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This branch statement checks for errors accumulated in previous steps.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if(FVID2_SOK != status)
            {
                GT_0trace(VhwaVissTrace, GT_ERR,
                    "Failed to submit GLBCE context save request!!\n");
            }
            /* LDRA_JUSTIFY_END */
        }

        /* Disable interrupts before accessing queue */
        cookie = HwiP_disable();

        /* Dequeue the completed request from done queue */
        qObj = (Vhwa_M2mVissQueueObj *) Fvid2Utils_dequeue(hObj->doneQ);

        /* Restore interrupts after updating node list */
        HwiP_restore(cookie);

        #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
        if ((FVID2_SOK == status) && (NULL != hObj->nsf4HistExtAddr))
        {
            /* Get the NSF4 Histogram */
            status = Vhwa_m2mVissGetNsf4Histogram(instObj, hObj, (Nsf4_Histogram *)hObj->nsf4HistExtAddr);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This branch statement checks for errors accumulated in previous steps.
            Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. If the control reaches here, our code base is expected to print the error.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if(FVID2_SOK != status)
            {
                GT_0trace(VhwaVissTrace, GT_ERR, "Failed to get NSF4 Histogram\n");
            }
            /* LDRA_JUSTIFY_END */
        }
        #endif

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        /* No buffers in the output queue */
        if (NULL == qObj)
        {
            /* Check if requests are pending with driver */
            if (0U == hObj->numPendReq)
            {
                /* Nothing is queued */
                GT_0trace(VhwaVissTrace, GT_DEBUG,
                    "Nothing to dequeue. No request pending with driver!!\r\n");
                status = FVID2_ENO_MORE_BUFFERS;
            }
            else
            {
                /* If no request have completed, return try again */
                GT_0trace(VhwaVissTrace, GT_DEBUG,
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

            /* Return back the queue object to the free queue,
             * No need to protect from ISR, as it is not accessed in ISR */
            Fvid2Utils_queue(instObj->freeQ, &qObj->qElem, qObj);

            /* Dequeue from Rings */
            status = Vhwa_m2mVissPopRings(instObj, hObj);

            /* Decrement the pending request count. */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (0U != hObj->numPendReq)
            /* LDRA_JUSTIFY_END */
            {
                hObj->numPendReq--;
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
                GT_0trace(VhwaVissTrace, GT_DEBUG,
                    "Done queue of the handle is not empty when there are is no pending request on the handle!!\r\n");
                status = FVID2_EFAIL;
            }
            /* LDRA_JUSTIFY_END */
        }

        if (UFALSE == hObj->fcStatus.isFlexConnect)
        {
            Vhwa_commonHwaLockRelease(instObj->hwaLockIdx);
        }

        (void)VhwaAl_SemaphoreP_post(instObj->lock);
    }

    return (status);
}

/* ========================================================================== */
/*                           Local Functions                                  */
/* ========================================================================== */

static Vhwa_M2mVissHandleObj *vhwaM2mVissAllocHandleObj(
    const Vhwa_M2mVissInstObj *instObj)
{
    uint32_t cnt;
    Vhwa_M2mVissHandleObj *hObj = NULL;

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
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: This portion can only be reached by multi-camera application using 8 cameras.
        Effect on this unit: The code is expected to allocate memory for all the handles. However, due to the stated rationale, this is not tested.
        <justification end> */
        for (cnt = 0U; cnt < VHWA_M2M_VISS_MAX_HANDLES; cnt ++)
        /* LDRA_JUSTIFY_END */
        {
            if ((uint32_t)UFALSE == gM2mVissHandleObj[cnt].isUsed)
            {
                hObj = &gM2mVissHandleObj[cnt];
                Fvid2Utils_memset(hObj, 0x0, sizeof(Vhwa_M2mVissHandleObj));
                gM2mVissHandleObj[cnt].isUsed = (uint32_t)UTRUE;
                hObj->hIdx = cnt;
                break;
            }
        }
    }
    return (hObj);
}

/* Function to freeup allocated Handle Object */
static void vhwaM2mVissFreeHandleObj(Vhwa_M2mVissHandleObj *hObj)
{
    uint32_t cnt;

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
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: This portion can only be reached by multi-camera application using 8 cameras.
        Effect on this unit: The code is expected to allocate memory for all the handles. However, due to the stated rationale, this is not tested.
        <justification end> */
        for (cnt = 0U; cnt < VHWA_M2M_VISS_MAX_HANDLES; cnt ++)
        /* LDRA_JUSTIFY_END */
        {
            if (hObj == &gM2mVissHandleObj[cnt])
            {
                hObj->isUsed = (uint32_t)UFALSE;;
                break;
            }
        }
    }
}

static int32_t vhwaM2mVissSetParams(Vhwa_M2mVissInstObj *instObj,
    Vhwa_M2mVissHandleObj *hObj, Vhwa_M2mVissParams *vsPrms)
{
    int32_t status = FVID2_EBADARGS;

    /* Check for Null Pointer */
    GT_assert(VhwaVissTrace, (NULL != instObj));
    GT_assert(VhwaVissTrace, (NULL != hObj));
    GT_assert(VhwaVissTrace, (NULL != vsPrms));

    {
        /* Calculate Blanking information */
        vhwaM2mVissCalcBlankParams(hObj, vsPrms);

        status = vhwaM2mVissCheckParams(instObj, hObj, vsPrms);

        if (FVID2_SOK != status)
        {
            /* Revert Blanking information */
            vhwaM2mVissCalcBlankParams(hObj, &hObj->vsPrms);

            /* Revert SL2 Size */
            hObj->totalSl2Size = 0x0U;

            /* Reset VISS Config to old working config */
            (void)Vhwa_m2mVissSetInChParams(instObj, hObj, &hObj->vsPrms);
            /* Reset VISS Config to old working config */
            (void)Vhwa_m2mVissSetOutChParams(instObj, hObj, &hObj->vsPrms);
            /* Reset LSE config based on local correct viss config */
            Vhwa_m2mVissSetLseCfg(instObj, hObj, &hObj->vsPrms);
            /* Reset HTS config based on local correct viss config */
            Vhwa_m2mVissSetHtsCfg(instObj, hObj, &hObj->vsPrms);
        }
        else
        {
            /* Copy Parameters in handle object */
            (void)memcpy(&hObj->vsPrms, vsPrms, sizeof(Vhwa_M2mVissParams));

            /* Setup TR Descriptor */
            Vhwa_m2mVissSetTrDesc(instObj, hObj);

            hObj->isPrmsSet = (uint32_t)UTRUE;

            hObj->fcpCfg.width = vsPrms->inFmt.width;
            hObj->fcpCfg.height = vsPrms->inFmt.height;
            hObj->rfeCfg.width = vsPrms->inFmt.width;
            hObj->rfeCfg.height = vsPrms->inFmt.height;
            hObj->nsf4vCfg.width = vsPrms->inFmt.width;
            hObj->nsf4vCfg.height = vsPrms->inFmt.height;
            hObj->eeCfg.width = vsPrms->inFmt.width;
            hObj->eeCfg.height = vsPrms->inFmt.height;
        #if defined (VHWA_VPAC_IP_REV_VPAC3L)
            hObj->pcidFrameCfg.width = vsPrms->inFmt.width;
            hObj->pcidFrameCfg.height = vsPrms->inFmt.height;
        #endif
        #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
            hObj->cacCfg.width = vsPrms->inFmt.width;
            hObj->cacCfg.height = vsPrms->inFmt.height;
        #endif
        #if defined VHWA_VPAC_IP_REV_VPAC3
            hObj->fcp2Cfg.width = vsPrms->inFmt.width;
            hObj->fcp2Cfg.height = vsPrms->inFmt.height;
            hObj->fcp2EeCfg.width = vsPrms->inFmt.width;
            hObj->fcp2EeCfg.height = vsPrms->inFmt.height;
        #endif

            /* Do GLBCE init sequence, so that GLBCE registers are
             * accessible */
            Vhwa_m2mVissGlbceInit(instObj, hObj);
            if (((uint32_t)UTRUE == hObj->vsPrms.enableGlbceSaveRestoreCtx) && ((uint32_t)UTRUE == hObj->vsPrms.enableGlbce))
            {
                Vhwa_m2mVissSetGlbceCntxtSaveRestoreTrDesc(instObj, hObj);
            }
            /* Similarly enable NSF4 at VISS Top, so that
             * NSF4v registers are accessible, for set_config */
            Vhwa_m2mVissNsf4Init(instObj, hObj);

        #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
            if ((uint32_t)UTRUE == hObj->vsPrms.enableCac)
            {
                Vhwa_m2mVissCacInit(instObj, hObj);
            }
        #endif
        }
            #if defined (VHWA_VPAC_IP_REV_VPAC3L)
            if ((uint32_t)TRUE == hObj->vsPrms.enablePcid)
            {
                Vhwa_m2mVissPcidInit(instObj, hObj);
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
        if ((FVID2_SOK == status)
                && (true == instObj->initPrms.configThroughUdmaFlag))
        /* LDRA_JUSTIFY_END */
        {
            /* configuration has been changed */
            hObj->configInHwDone = false;
        }
    }

    return (status);
}

static int32_t vhwa_m2mVissSetEeParams(const Vhwa_M2mVissInstObj *instObj,
    Vhwa_M2mVissHandleObj *hObj, const Viss_ErrEventParams *eePrms)
{
    int32_t status = FVID2_SOK;
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
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
            GT_0trace(VhwaVissTrace, GT_ERR,
                "Cannot change error events while handle is active !!\n");
            status = FVID2_EAGAIN;
        }
        /* LDRA_JUSTIFY_END */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else if (0U != (eePrms->errEvents & (~VHWA_VISS_ALL_ERR_AND_INFO_EVENTS)))
        {
            GT_1trace(VhwaVissTrace, GT_ERR,
                "Attempt to enable a non-existent error event: 0x%08x !!\n",
                (eePrms->errEvents & (~VHWA_VISS_ALL_ERR_AND_INFO_EVENTS)));
            status = FVID2_EINVALID_PARAMS;
        }
        /* LDRA_JUSTIFY_END */
        else
        {
            Fvid2Utils_memcpy(&hObj->eePrms, eePrms, sizeof(Viss_ErrEventParams));
            status = FVID2_SOK;
        }
    }
    else
    {
        status = FVID2_EBADARGS;
    }

    return (status);
}

static int32_t vhwa_m2mVissSetWdTimerEeParams(const Vhwa_M2mVissInstObj *instObj,
    Vhwa_M2mVissHandleObj *hObj, const Viss_WdTimerErrEventParams *wdTimerEePrms)
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
    if ((NULL != instObj) && (NULL != hObj) && (NULL != wdTimerEePrms))
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
            GT_0trace(VhwaVissTrace, GT_ERR,
                "Cannot change watchdog timer error events while handle is active !!\n");
            status = FVID2_EAGAIN;
        }
        /* LDRA_JUSTIFY_END */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else if (0U != (wdTimerEePrms->wdTimerErrEvents & (~VHWA_VISS_WDTIMER_ERR)))
        {
            GT_1trace(VhwaVissTrace, GT_ERR,
                "Attempt to enable a non-existent watchdog timer error event: 0x%08x !!\n",
                (wdTimerEePrms->wdTimerErrEvents & (~VHWA_VISS_WDTIMER_ERR)));
            status = FVID2_EINVALID_PARAMS;
        }
        /* LDRA_JUSTIFY_END */
        else
        {
            Fvid2Utils_memcpy(&hObj->wdTimerEePrms, wdTimerEePrms, sizeof(Viss_WdTimerErrEventParams));

            /* Enable watchdog timer in HTS if WDTIMER_ERR event is registered */
            if (0U != (wdTimerEePrms->wdTimerErrEvents & VHWA_VISS_WDTIMER_ERR))
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

static int32_t Vhwa_m2mVissSetIntrInHW(uint32_t vhwaIrqNum,
                                volatile CSL_vpac_intd_cfgRegs  *intdRegs,
                                const Vhwa_M2mVissHandleObj *hObj)
{
    int32_t status = FVID2_SOK;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
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
        CSL_REG32_WR(&intdRegs->ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_VISS_INTR_REG], VHWA_VISS_ALL_ERR_AND_INFO_EVENTS);
        CSL_REG32_WR(&intdRegs->ENABLE_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_VISS_INTR_REG],
            hObj->eePrms.errEvents);

        CSL_REG32_WR(&intdRegs->ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_WDTIMER_INTR_REG], VHWA_VISS_WDTIMER_ERR);
        CSL_REG32_WR(&intdRegs->ENABLE_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_WDTIMER_INTR_REG],
            hObj->wdTimerEePrms.wdTimerErrEvents);
    }

    return (status);
}

static void vhwaM2mVissCalcBlankParams(Vhwa_M2mVissHandleObj *hObj,
    const Vhwa_M2mVissParams *vsPrms)
{
    uint32_t            latency;
    #if defined VHWA_VPAC_IP_REV_VPAC3
        uint32_t            mvLatency;
    #endif
    #if defined VHWA_VPAC_IP_REV_VPAC3L
    uint32_t outChEnables, outChIrOut, irOnlyFlag, cnt;
    #endif

    /* Check For Null pointers */
    GT_assert(VhwaVissTrace, (NULL != hObj));
    GT_assert(VhwaVissTrace, (NULL != vsPrms));

    latency = 0u;
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL != hObj) && (NULL != vsPrms))
    /* LDRA_JUSTIFY_END */
    {
        #if defined (VHWA_VPAC_IP_REV_VPAC3L)
        if ((uint32_t)TRUE == vsPrms->enableDpc)
        {
            latency = latency + CSL_RAWFE_HORZ_LATENCY_4;
        }
        #else
        latency = latency + CSL_RAWFE_HORZ_LATENCY; /* For RAW FE */
        #endif
        latency = latency + CSL_FLEX_CFA_HORZ_LATENCY; /* For CFA */
        latency = latency + CSL_FLEX_CC_HORZ_LATENCY; /* For FCC */

        #if defined VHWA_VPAC_IP_REV_VPAC3
            mvLatency = latency;
        #endif


        if ((uint32_t)UTRUE == vsPrms->enableGlbce)
        {
            latency = latency + CSL_GLBCE_HORZ_LATENCY;
        }
        if ((uint32_t)UTRUE == vsPrms->enableNsf4)
        {
            latency = latency + CSL_NSF4V_HORZ_LATENCY;
        }
    #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
        if ((uint32_t)UTRUE == vsPrms->enableCac)
        {
            latency = latency + CSL_CAC_HORZ_LATENCY;
        }
    #endif
        #if defined (VHWA_VPAC_IP_REV_VPAC3L)
        if((uint32_t)TRUE == vsPrms->enablePcid)
        {
            latency += CSL_PCID_HORZ_LATENCY_BAYER_OUT;
        }
        #endif

        if ((0U != (VHWA_M2M_VISS_EE_ON_FCP0_LUMA12 & vsPrms->edgeEnhancerMode)) ||
            (0U != (VHWA_M2M_VISS_EE_ON_FCP0_LUMA8 & vsPrms->edgeEnhancerMode)))
        {
            latency = latency + CSL_FLEX_EE_HORZ_LATENCY;
        }

        /* Horizontal blanking is max of all enabled modules */
        hObj->maxLseHorzBlanking  = VHWA_VISS_MAX_HORZ_BLANKING;

        /* Calculate the threshold value */
        hObj->thr = Vhwa_ceil(latency,
                (vsPrms->inFmt.width + hObj->maxLseHorzBlanking));

    #if defined VHWA_VPAC_IP_REV_VPAC3
        if(UTRUE == vsPrms->enableMVPipe)
        {
            if (VHWA_VISS_MV_PIPE_INPUT_GLBC == vsPrms->mvPipeInSel)
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the caller before the control reaches here.
                Effect on this unit: If the control reaches here, our code base is expected to print the error.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((uint32_t)UTRUE == vsPrms->enableGlbce)
                /* LDRA_JUSTIFY_END */
                {
                    mvLatency = mvLatency + CSL_GLBCE_HORZ_LATENCY;
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the caller before the control reaches here.
                Effect on this unit: If the control reaches here, our code base is expected to print the error.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaVissTrace, GT_ERR, "Invalid MV Pipe Input Selection while GLBCE is off!!\n");
                }
                /* LDRA_JUSTIFY_END */
                if ((uint32_t)UTRUE == vsPrms->enableNsf4)
                {
                    mvLatency = mvLatency + CSL_NSF4V_HORZ_LATENCY;
                }
                if ((uint32_t)UTRUE == vsPrms->enableCac)
                {
                    mvLatency = mvLatency + CSL_CAC_HORZ_LATENCY;
                }
            }
            else if (VHWA_VISS_MV_PIPE_INPUT_NSF4 == vsPrms->mvPipeInSel)
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the caller before the control reaches here.
                Effect on this unit: If the control reaches here, our code base is expected to print the error.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((uint32_t)UTRUE == vsPrms->enableNsf4)
                /* LDRA_JUSTIFY_END */
                {
                    mvLatency = mvLatency + CSL_NSF4V_HORZ_LATENCY;
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the caller before the control reaches here.
                Effect on this unit: If the control reaches here, our code base is expected to print the error.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaVissTrace, GT_ERR, "Invalid MV Pipe Input Selection while NSF4 is off!!\n");
                }
                /* LDRA_JUSTIFY_END */
                if ((uint32_t)UTRUE == vsPrms->enableCac)
                {
                    mvLatency = mvLatency + CSL_CAC_HORZ_LATENCY;
                }
            }
            else if (VHWA_VISS_MV_PIPE_INPUT_CAC == vsPrms->mvPipeInSel)
            {
                if ((uint32_t)UTRUE == vsPrms->enableCac)
                {
                    mvLatency = mvLatency + CSL_CAC_HORZ_LATENCY;
                }
                else
                {
                    GT_0trace(VhwaVissTrace, GT_ERR, "Invalid MV Pipe Input Selection while CAC is off!!\n");
                }
            }
            else
            {
              /*Do Nothing*/
            }

            if ((0U != (VHWA_M2M_VISS_EE_ON_FCP1_LUMA12 & vsPrms->edgeEnhancerMode)) ||
                (0U != (VHWA_M2M_VISS_EE_ON_FCP1_LUMA8 & vsPrms->edgeEnhancerMode)))
            {
                mvLatency = mvLatency + CSL_FLEX_EE_HORZ_LATENCY;
            }

        }
        if(latency > mvLatency)
        {
            hObj->mvPipeDelay = latency - mvLatency;
        }
        else
        {
            hObj->mvPipeDelay = 0U;
        }
    #endif /* #if defined VHWA_VPAC_IP_REV_VPAC3 */

/* Vertical blanking is sum of all enabled modules vertical blanking Calculate vertical blanking */
        hObj->maxLseVertBlanking = 0;
    #if defined VHWA_VPAC_IP_REV_VPAC3
        hObj->maxLseVertBlankingMV = 0;
    #endif

    /* HV Vertical Blanking for J721E, J721S2, J784S4 */
#if defined VHWA_VPAC_IP_REV_VPAC || defined VHWA_VPAC_IP_REV_VPAC3
        hObj->maxLseVertBlanking += CSL_FLEX_CC_VERT_BLANKING; /* For FCC */
        if (UTRUE == vsPrms->enableDpc)
        {
            hObj->maxLseVertBlanking += CSL_RAWFE_DPC_VERT_BLANKING;
        }
        if (UTRUE == vsPrms->enableGlbce)
        {
            hObj->maxLseVertBlanking += CSL_GLBCE_VERT_BLANKING;
        }
        if (UTRUE == vsPrms->enableNsf4)
        {
            hObj->maxLseVertBlanking += CSL_NSF4V_VERT_BLANKING;
        }
    #if defined VHWA_VPAC_IP_REV_VPAC3
        if (UTRUE == vsPrms->enableCac)
        {
            hObj->maxLseVertBlanking += CSL_CAC_VERT_BLANKING;
        }
    #endif
#endif

    /* HV Vertical Blanking for J722S and AM62A */
#if defined (VHWA_VPAC_IP_REV_VPAC3L)
        irOnlyFlag = 0;
        /* Vertical blanking calculation for an IR only output is different */
        /* Vertical blanking calculation for IR path*/
        hObj->maxLseVertBlankingIR = CSL_PCID_VERT_BLANKING;
        if (TRUE == vsPrms->enableDpc)
        {
            hObj->maxLseVertBlankingIR += CSL_RAWFE_DPC_VERT_BLANKING_4;
        }

        outChEnables = 0;
        outChIrOut = 0;

        for (cnt = 0U; cnt < VHWA_M2M_VISS_MAX_OUTPUTS; cnt ++)
        {
            if(TRUE == vsPrms->outPrms[cnt].enable && cnt != 5) /*Do not check for H3A out*/
            {
                outChEnables += 1;
                outChIrOut += vsPrms->outPrms[cnt].isIrOut;
            }
        }
        if(outChEnables != 1 || outChIrOut != 1)
        {
            if(vsPrms->cfa_mode != RFE_CFA_CFG_MODE_2_LEGACY)
            {
                hObj->maxLseVertBlanking += (CSL_FLEX_CFA_VERT_LATENCY - CSL_FLEX_CFA_VERT_BLANKING); /* Diff b/w CFA blanking and latency */
            }
            hObj->maxLseVertBlanking += CSL_FLEX_CFA_VERT_BLANKING; /* For CFA */
            hObj->maxLseVertBlanking += CSL_FLEX_CC_VERT_BLANKING; /* For FCC */
        }
        else
        {
            irOnlyFlag = 1;
        }

        if (TRUE == vsPrms->enableDpc)
        {
            hObj->maxLseVertBlanking += CSL_RAWFE_DPC_VERT_BLANKING_4;
        }
        if(TRUE == vsPrms->enablePcid)
        {
            hObj->maxLseVertBlanking += CSL_PCID_VERT_BLANKING;
        }

        if (TRUE == vsPrms->enableGlbce && FALSE == irOnlyFlag)
        {
            hObj->maxLseVertBlanking += CSL_GLBCE_VERT_BLANKING;
        }
        if (TRUE == vsPrms->enableNsf4 && FALSE == irOnlyFlag)
        {
            hObj->maxLseVertBlanking += CSL_NSF4V_VERT_BLANKING;
        }
        if (TRUE == vsPrms->enableCac && FALSE == irOnlyFlag)
        {
            hObj->maxLseVertBlanking += CSL_CAC_VERT_BLANKING;
        }
#endif

    #if defined VHWA_VPAC_IP_REV_VPAC3
    /* MV Vertical Blanking */

        if(UTRUE == vsPrms->enableMVPipe)
        {
            /*Flex CC blanking values are added. */
            hObj->maxLseVertBlankingMV += CSL_FLEX_CC_VERT_BLANKING; /* For FCC */

            /*If DPC enabled then blanking values are added. */
            if (UTRUE == vsPrms->enableDpc)
            {
                hObj->maxLseVertBlankingMV += CSL_RAWFE_DPC_VERT_BLANKING;
            }

            /*For individual modules that are enabled, blanking values are calculated. */
            if (VHWA_VISS_MV_PIPE_INPUT_GLBC == vsPrms->mvPipeInSel)
            {
            /*If GLBCE enabled then blanking values are added.  */
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the caller before the control reaches here.
                Effect on this unit: If the control reaches here, our code base is expected to print the error.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((uint32_t)UTRUE == vsPrms->enableGlbce)
                /* LDRA_JUSTIFY_END */
                {
                    hObj->maxLseVertBlankingMV += CSL_GLBCE_VERT_BLANKING;
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the caller before the control reaches here.
                Effect on this unit: If the control reaches here, our code base is expected to print the error.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaVissTrace, GT_ERR, "Invalid MV Pipe Input Selection while GLBCE is off!!\n");
                }
                /* LDRA_JUSTIFY_END */
            /*If NSF4V enabled then blanking values are added. */

                if ((uint32_t)UTRUE == vsPrms->enableNsf4)
                {
                    hObj->maxLseVertBlankingMV += CSL_NSF4V_VERT_BLANKING;
                }
            /*If CAC enabled then blanking values are added. */

                if ((uint32_t)UTRUE == vsPrms->enableCac)
                {
                    hObj->maxLseVertBlankingMV += CSL_CAC_VERT_BLANKING;
                }
            }
            else if (VHWA_VISS_MV_PIPE_INPUT_NSF4 == vsPrms->mvPipeInSel)
            {
            /*If NSF4V enabled then blanking values are added. */
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the caller before the control reaches here.
                Effect on this unit: If the control reaches here, our code base is expected to print the error.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((uint32_t)UTRUE == vsPrms->enableNsf4)
                /* LDRA_JUSTIFY_END */
                {
                    hObj->maxLseVertBlankingMV += CSL_NSF4V_VERT_BLANKING;
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the caller before the control reaches here.
                Effect on this unit: If the control reaches here, our code base is expected to print the error.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaVissTrace, GT_ERR, "Invalid MV Pipe Input Selection while NSF4 is off!!\n");
                }
                /* LDRA_JUSTIFY_END */
            /*If CAC enabled then blanking values are added. */

                if ((uint32_t)UTRUE == vsPrms->enableCac)
                {
                    hObj->maxLseVertBlankingMV += CSL_CAC_VERT_BLANKING;
                }
            }
            else if (VHWA_VISS_MV_PIPE_INPUT_CAC == vsPrms->mvPipeInSel)
            {
            /*If CAC enabled then blanking values are added. */
                if ((uint32_t)UTRUE == vsPrms->enableCac)
                {
                    hObj->maxLseVertBlankingMV += CSL_CAC_VERT_BLANKING;
                }
                else
                {
                    GT_0trace(VhwaVissTrace, GT_ERR, "Invalid MV Pipe Input Selection while CAC is off!!\n");
                }
            }
            else
            {
              /*Do Nothing*/
            }
        }
    #endif /* #if defined VHWA_VPAC_IP_REV_VPAC3 */
    }
}

static int32_t vhwaM2mVissCheckParams(Vhwa_M2mVissInstObj *instObj,
    Vhwa_M2mVissHandleObj *hObj, Vhwa_M2mVissParams *vsPrms)
{
    int32_t         status = FVID2_SOK;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL != instObj) && (NULL != hObj) && (NULL != vsPrms))
    /* LDRA_JUSTIFY_END */
    {
        hObj->totalSl2Size = 0x0U;

        status = Vhwa_m2mVissSetInChParams(instObj, hObj, vsPrms);

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps.
        Effect on this unit: If the control reaches here, our code base is NOT expected to accumulate the error status.
        An else block can be added to catch and print the error status in a future release.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            status = Vhwa_m2mVissSetOutChParams(instObj, hObj, vsPrms);
        }

        if (FVID2_SOK == status)
        {
            /* Initialize LSE configuration based on the new VISS Params */
            Vhwa_m2mVissSetLseCfg(instObj, hObj, vsPrms);
            /* Initialize HTS configuration based on the new VISS Params */
            Vhwa_m2mVissSetHtsCfg(instObj, hObj, vsPrms);
        }

        if (FVID2_SOK == status)
        {
            /* Check New HTS Configuration */
            status = CSL_htsCheckThreadConfig(&hObj->htsCfg);
        }
        if (FVID2_SOK == status)
        {
            /* Check LSE HTS Configuration */
            status = CSL_lseCheckConfig(&hObj->lseCfg);
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

    return (status);
}

static int32_t vhwaM2mVissSetConfigInHW(Vhwa_M2mVissInstObj *instObj,
    const Vhwa_M2mVissQueueObj *qObj)
{
    int32_t status = FVID2_SOK;
    Vhwa_M2mVissRegAddr *regAddrs = NULL;
    Vhwa_M2mVissHandleObj *hObj = NULL;
    /* enable buffer object if used */
    bool enableBufObj = false;

    /* Null pointer check */
    GT_assert(VhwaVissTrace, (NULL != instObj));
    GT_assert(VhwaVissTrace, (NULL != qObj));
    GT_assert(VhwaVissTrace, (NULL != qObj->hObj));

    hObj = qObj->hObj;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is NOT expected to result in an error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (true == instObj->initPrms.configThroughUdmaFlag)
     /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (true == hObj->appBuffInitDone)
        /* LDRA_JUSTIFY_END */
        {
            /* we will be writing configuration to buffer object */
            regAddrs = &hObj->buffRegAddrs;
            enableBufObj = true;
            hObj->configInHwDone = false;
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
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is NOT expected to result in an error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    /* LDRA_JUSTIFY_END */
    {
        regAddrs = &instObj->regAddrs;
    }

    /*
     * perform config HW if,
     * 1) configTroughUDMA is false. 2) configInHwDone is false.
     */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((false == enableBufObj) || (false == hObj->configInHwDone))
    /* LDRA_JUSTIFY_END */
    {
        /* Step-1, Configure HWA Core,
         * This changes the frame size and paths, as required by this handle */
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
            status = Vhwa_memVissSetVissSizeAndPath(instObj, hObj);
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
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            /* Step-2, Configure HWA Wrapper LSE */
            status = CSL_lseSetConfig(regAddrs->lseRegs, &hObj->lseCfg);
            /* Enable used buffer object */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This branch statement checks for errors accumulated in previous steps.
            Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((FVID2_SOK == status) && (true == enableBufObj))
            /* LDRA_JUSTIFY_END */
            {
                hObj->bufferObjHolder[BUFF_ID_LSE_REGS].isModified = true;
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
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            /* Step-3, Configure HWA Common Wrapper HTS */
            status = CSL_htsSetThreadConfig(regAddrs->htsRegs, &hObj->htsCfg);
            /* Enable used buffer object */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This branch statement checks for errors accumulated in previous steps.
            Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((FVID2_SOK == status) && (true == enableBufObj))
            /* LDRA_JUSTIFY_END */
            {
                hObj->bufferObjHolder[BUFF_ID_HTS_HWA0_REGS].isModified = true;
                hObj->bufferObjHolder[BUFF_ID_HTS_DMA_0_4_REGS].isModified
                            = true;
                hObj->bufferObjHolder[BUFF_ID_HTS_DMA_240_245_REGS].isModified
                            = true;
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
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            /* Step-5, Start HTS Channels */
            status = CSL_htsThreadStart(regAddrs->htsRegs, &hObj->htsCfg);
            /* Enable used buffer object */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This branch statement checks for errors accumulated in previous steps.
            Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((FVID2_SOK == status) && (true == enableBufObj))
            /* LDRA_JUSTIFY_END */
            {
                hObj->bufferObjHolder[BUFF_ID_HTS_HWA0_REGS].isModified = true;
                hObj->bufferObjHolder[BUFF_ID_HTS_DMA_0_4_REGS].isModified
                            = true;
                hObj->bufferObjHolder[BUFF_ID_HTS_DMA_240_245_REGS].isModified
                            = true;
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
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this branch.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
            <justification end> */
            if (true == enableBufObj)
            /* LDRA_JUSTIFY_END */
            {
                /*
                 * if configThroughUDMA is true,
                 * no need to config HW every time
                 */
                hObj->configInHwDone = true;
            }
        }
    }

    return (status);
}

static int32_t vhwaM2mVissSubmitRequest(Vhwa_M2mVissInstObj *instObj,
    Vhwa_M2mVissQueueObj *qObj)
{
    int32_t status = FVID2_SOK;
    Vhwa_M2mVissHandleObj *hObj = NULL;

    /* Null pointer check */
    GT_assert(VhwaVissTrace, (NULL != instObj));
    GT_assert(VhwaVissTrace, (NULL != qObj));
    GT_assert(VhwaVissTrace, (NULL != qObj->hObj));

    hObj = qObj->hObj;

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
        /* Start the core */
        status = Vhwa_memVissSetVissStartModules(instObj, hObj);
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
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /*
         * before submitting the request to VISS, write configuration
         * if configThroughUdmaFlag is true
         */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. An error print statement can be added in a future release if required.
        <justification end> */
        if (true == instObj->initPrms.configThroughUdmaFlag)
        /* LDRA_JUSTIFY_END */
        {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the caller before the control reaches here.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (true == hObj->appBuffInitDone)
            /* LDRA_JUSTIFY_END */
            {
                /*
                 * Before submitting the VISS input buffers write the
                 * config UDMA buffer to registers through UDMA channel.
                 */
                status = Vhwa_m2mVissSubmitConfUDMABuf(instObj, hObj);
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (FVID2_SOK != status)
                {
                    status = FVID2_EFAIL;
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
                status = FVID2_EFAIL;
            }
            /* LDRA_JUSTIFY_END */
        }
    }

    /* Submit Request for Enqueue Restore GLBCE ctx channel */
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
        if(hObj->vsPrms.enableGlbceSaveRestoreCtx == UTRUE)
        {
            status = vhwaM2mVissSubmitGlbceCtxRestoreRequest(instObj, hObj);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This branch statement checks for errors accumulated in previous steps.
            Effect on this unit: If the control reaches here, our code base is expected to print the error status.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if(UDMA_SOK != status)
            {
                GT_0trace(VhwaVissTrace, GT_ERR, "Failed to submit GLBCE context restore request!!\n\n");
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
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Configure Buffer Addresses in TR Record */
        Vhwa_m2mVissSetAddress(hObj);

        /* Submit Rings to the Ring Accelerator */
        status = Vhwa_m2mVissSubmitRing(instObj, hObj);
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
        /* Better to set Active object to this q object, so that if
         * interrupt comes immediately, actQObj would be set..
         * If pipeline start fails, it would be set to NULL.*/
        instObj->actQObj = qObj;

        instObj->totalReqCnt++;

        #ifndef VHWA_USE_PIPELINE_COMMON_ENABLE
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != hObj->createArgs.getTimeStamp)
        {
            hObj->perf = hObj->createArgs.getTimeStamp();
        }
        /* LDRA_JUSTIFY_END */
        /* Before starting pipeline make sure, config UDMA is complete */

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. An error print statement can be added in a future release if required.
        <justification end> */
        if (true == instObj->initPrms.configThroughUdmaFlag)
        /* LDRA_JUSTIFY_END */
        {
            status = Vhwa_m2mVissCheckConfUDMAComp(instObj, hObj);
        }
#if !defined(VHWA_VPAC_IP_REV_VPAC3L)
        /* Update the VissStatusRegisterGroup with valid status register values for dynamic registers */
        if((FVID2_SOK == status) && ((uint32_t)UTRUE == hObj->enableStatusRegValidate))
        {
            status = vhwaM2mVissUpdateStatusRegGroup(hObj);
        }
#endif
        /* Start HTS pipeline */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((FVID2_SOK == status) && (UFALSE == hObj->fcStatus.isFlexConnect))
        /* LDRA_JUSTIFY_END */
        {
            status = CSL_htsPipelineStart(
                    instObj->regAddrs.htsRegs, &hObj->htsCfg);
        }
        #endif
        /* Start HTS pipeline */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps.
        Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: The unit is NOT expected to result in an error. This behaviour is part of the application design.
        If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != status)
        {
            instObj->actQObj = NULL;
            status = FVID2_EFAIL;
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            status = FVID2_SOK;
        }
    }

    return (status);
}

int32_t vhwaVissCheckFrameList(Vhwa_M2mVissHandleObj *hObj,
    const Fvid2_FrameList *inFrmList, const Fvid2_FrameList *outFrmList)
{
    int32_t                 status = FVID2_SOK;
    uint32_t                cnt;
    uint32_t                frmIdx;
    uint32_t                frmAddrIdx;
    Vhwa_M2mVissChParams   *chPrms = NULL;

    GT_assert(VhwaVissTrace, (NULL != hObj));
    GT_assert(VhwaVissTrace, (NULL != inFrmList));
    GT_assert(VhwaVissTrace, (NULL != outFrmList));

    /* Cannot continue if parameters are not set */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((uint32_t)UFALSE == hObj->isPrmsSet)
    {
        status = FVID2_EINVALID_PARAMS;
    }
    /* LDRA_JUSTIFY_END */

    /* Input Buffer Addresses cannot be null */
    for (cnt = 0U; cnt < VHWA_M2M_VISS_MAX_INPUTS; cnt ++)
    {
        chPrms = &hObj->inChPrms[cnt];
        if ((uint32_t)UTRUE == chPrms->isEnabled)
        {
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the caller before the control reaches here.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            /* Both Frame and frame address must not be null */
            if ((NULL == inFrmList->frames[cnt]) ||
                (0U == inFrmList->frames[cnt]->addr[0U]))
            {
                status = FVID2_EBADARGS;
                break;
            }
            else
            /* LDRA_JUSTIFY_END */
            {
                /* Storing Buffer address here, so does not have to run
                 * this loop again */
                chPrms->bufAddr = inFrmList->frames[cnt]->addr[0U];
            }
        }
    }

    /* Output Buffer Addresses cannot be null */
    for (cnt = 0U; cnt < VHWA_M2M_VISS_MAX_OUTPUTS; cnt ++)
    {
        chPrms     = &hObj->outChPrms[cnt];
        frmIdx     = chPrms->frmIdx;
        frmAddrIdx = chPrms->frmAddrIdx;

        if ((uint32_t)UTRUE == chPrms->isEnabled)
        {
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the caller before the control reaches here.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            /* Both Frame and frame address must not be null */
            if (((NULL == outFrmList->frames[frmIdx]) ||
                 (0U == outFrmList->frames[frmIdx]->addr[frmAddrIdx])) &&
                 (UFALSE == hObj->fcStatus.isFlexConnect))
            {
                status = FVID2_EBADARGS;
                break;
            }
            else
            /* LDRA_JUSTIFY_END */
            {
                /* Storing Buffer address here, so does not have to run
                 * this loop again */
                chPrms->bufAddr = outFrmList->frames[frmIdx]->addr[frmAddrIdx];
            }
        }
    }

    return (status);
}


static int32_t vhwaM2mVissCreateQueues(Vhwa_M2mVissInstObj *instObj)
{
    int32_t              status = FVID2_SOK;
    uint32_t             qCnt;
    Vhwa_M2mVissQueueObj *qObj;

    /* NULL pointer check */
    GT_assert(VhwaVissTrace, (NULL != instObj));

    instObj->reqQ  = NULL;
    instObj->freeQ = NULL;

    /* Create Free queue */
    status = Fvid2Utils_constructQ(&instObj->freeQObj);
    GT_assert(VhwaVissTrace, (FVID2_SOK == status));

    instObj->freeQ = &instObj->freeQObj;

    /* Create input queue */
    status = Fvid2Utils_constructQ(&instObj->reqQObj);
    GT_assert(VhwaVissTrace, (FVID2_SOK == status));

    instObj->reqQ = &instObj->reqQObj;

    /* Initialize and queue the allocate queue object to free Q */
    for(qCnt = 0U; qCnt < VHWA_M2M_VISS_UDMA_RING_ENTRIES; qCnt ++)
    {
        qObj = &instObj->vissQObj[qCnt];

        Fvid2Utils_memset(qObj, 0x0, sizeof(Vhwa_M2mVissQueueObj));

        Fvid2Utils_queue(instObj->freeQ, &qObj->qElem, qObj);
    }

    return (status);
}

static void vhwaM2mVissDeleteQueues(Vhwa_M2mVissInstObj *instObj)
{
    Vhwa_M2mVissQueueObj *qObj = NULL;

    /* NULL pointer check */
    GT_assert(VhwaVissTrace, (NULL != instObj));

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(NULL != instObj->freeQ)
    /* LDRA_JUSTIFY_END */
    {
        /* Free-up all the queued free queue objects */
        do
        {
            qObj = (Vhwa_M2mVissQueueObj *) Fvid2Utils_dequeue(instObj->freeQ);
        } while (NULL != qObj);

        /* Delete the free Q */
        Fvid2Utils_destructQ(instObj->freeQ);
        instObj->freeQ = NULL;
    }

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(NULL != instObj->reqQ)
    /* LDRA_JUSTIFY_END */
    {
        /* Free-up all object from input queue */
        do
        {
            qObj = (Vhwa_M2mVissQueueObj *) Fvid2Utils_dequeue(instObj->reqQ);
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The negative test framework and test apps cannot reach this portion.
        These parameters cannot be configured from test app or kernel directly and can only be overwritten by memory corruption. This is fail safe mechanism and cannot be removed.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        } while (NULL != qObj);
        /* LDRA_JUSTIFY_END */

        /* Delete the input Q */
        Fvid2Utils_destructQ(instObj->reqQ);
        instObj->reqQ = NULL;
    }
}

static int32_t vhwaM2mVissAllocSl2(Vhwa_M2mVissInstObj *instObj,
    const Vhwa_M2mVissSl2Params *sl2AllocPrms)
{
    int32_t              status = FVID2_SOK;
    uint32_t             cnt;
    uint32_t             sl2MemSize;
    uint64_t             sl2Addr;
    uint32_t             width;

    /* Error Checking */
    for (cnt = 0U; cnt < VHWA_M2M_VISS_MAX_INPUTS; cnt ++)
    {
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        /* Check min width for input */
        if ((0U != sl2AllocPrms->maxInWidth[cnt]) &&
            (VHWA_M2M_VISS_MIN_INPUT_DEPTH > sl2AllocPrms->inDepth))
        {
            GT_2trace(VhwaVissTrace, GT_ERR,
                "Incorrect Depth %d for input %d !!\n",
                sl2AllocPrms->inDepth, cnt);
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */
    }
    for (cnt = 0U; cnt < VHWA_M2M_VISS_MAX_OUTPUTS; cnt ++)
    {
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        /* Check min width for output */
        if ((0U != sl2AllocPrms->maxOutWidth[cnt]) &&
            (VHWA_M2M_VISS_MIN_OUTPUT_DEPTH > sl2AllocPrms->outDepth[cnt]))
        {
            GT_2trace(VhwaVissTrace, GT_ERR,
                "Incorrect Depth %d for output %d !!\n",
                sl2AllocPrms->outDepth[cnt], cnt);
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
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
    <justification end> */
    /* Check min width for output */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        sl2MemSize = 0U;
        /* Calculate the total size required */
        for (cnt = 0U; cnt < VHWA_M2M_VISS_MAX_INPUTS; cnt ++)
        {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the caller before the control reaches here.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (0U != sl2AllocPrms->maxInWidth[cnt])
            /* LDRA_JUSTIFY_END */
            {
                width = Vhwa_calcHorzSizeInBytes(
                    sl2AllocPrms->maxInWidth[cnt],
                    sl2AllocPrms->inCcsf);

                /* Align block width in bytes to SL2 pitch ie 64byte aligned */
                width = Vhwa_calcSl2Pitch(width);

                /* calculate size based on the depth required */
                sl2MemSize = sl2MemSize +
                    (width * sl2AllocPrms->inDepth);
            }
        }
        /* Calculate the total size required for output */
        for (cnt = 0U; cnt < VHWA_M2M_VISS_MAX_OUTPUTS; cnt ++)
        {
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the caller before the control reaches here.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (0U != sl2AllocPrms->maxOutWidth[cnt])
            /* LDRA_JUSTIFY_END */
            {
                width = Vhwa_calcHorzSizeInBytes(
                    sl2AllocPrms->maxOutWidth[cnt],
                    sl2AllocPrms->outCcsf[cnt]);

                /* Align block width in bytes to SL2 pitch ie 64byte aligned */
                width = Vhwa_calcSl2Pitch(width);

                /* calculate size based on the depth required */
                sl2MemSize = sl2MemSize +
                    (width * sl2AllocPrms->outDepth[cnt]);
            }
        }

        /* Apply 16KB alignment for size and address if requested for firewall support */
        if ((uint32_t)TRUE == sl2AllocPrms->enable16KBAlignment)
        {
            sl2MemSize = ((sl2MemSize + 0x3FFFu) & ~0x3FFFu);
            sl2Addr = Vhwa_allocateSl2Aligned(sl2MemSize, VHWA_SL2_INST_VPAC, 0x4000u);
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
            sl2Addr = Vhwa_allocateSl2(sl2MemSize, VHWA_SL2_INST_VPAC);
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (0U == sl2Addr)
        {
            GT_1trace(VhwaVissTrace, GT_ERR,
                "SL2 aligned allocation failed for output%d (check alignment=0x4000 is power of 2 and sufficient SL2 memory available)\n", cnt);
            status = FVID2_EALLOC;
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            /* Store the allocated size (already aligned if firewall enabled) */
            instObj->sl2Size = sl2MemSize;
            instObj->sl2Addr = sl2Addr;
        }
    }

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK != status)
    {
        /* Error in memory allocation, open will not be allowed. */
        instObj->isSl2AllocDone = (uint32_t)UFALSE;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        /* Memory is allocated successfully, now open is allowed. */
        instObj->isSl2AllocDone = (uint32_t)UTRUE;
        Fvid2Utils_memcpy(&instObj->sl2AllocPrms, sl2AllocPrms,
            sizeof(Vhwa_M2mVissSl2Params));
    }

    return (status);
}
void Vhwa_m2mVissConfgUdmaEvenCb(Udma_EventHandle eventHandle,
                               uint32_t eventType,
                               void *appData)
{
    Vhwa_M2mVissInstObj *instObj = &gM2mVissInstObj[0U];

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (UDMA_EVENT_TYPE_DMA_COMPLETION == eventType)
    /* LDRA_JUSTIFY_END */
    {
        (void)VhwaAl_SemaphoreP_post(instObj->configCqEventSemaphore);
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
        GT_0trace(VhwaVissTrace, GT_ERR,"Vhwa_m2mVissConfgUdmaEvenCb fail !!\n");
    }
    /* LDRA_JUSTIFY_END */

    return;
}

void Vhwa_m2mVissGetSl2Prms(Vhwa_M2mVissHandleObj *hObj,
                            Vhwa_M2mVissSl2Prms *sl2Prms)
{
    uint32_t cnt;
    Vhwa_M2mVissChParams   *chPrms = NULL;
    Vhwa_M2mVissInstObj    *instObj = &gM2mVissInstObj[0U];

    sl2Prms->sl2MemSize = instObj->sl2Size;

    for (cnt=0; cnt < VHWA_M2M_VISS_MAX_IN_DMA_CH; cnt++)
    {
        chPrms = &hObj->inChPrms[cnt];

        sl2Prms->inSl2Addr[cnt] = chPrms->sl2Addr;
        sl2Prms->inWidthInBytes[cnt] = chPrms->sl2Pitch;
        sl2Prms->inSl2BuffDepth[cnt] = chPrms->sl2Depth;

    }

    for (cnt=0; cnt < VHWA_M2M_VISS_MAX_OUT_DMA_CH; cnt++)
    {
        chPrms = &hObj->outChPrms[cnt];

        sl2Prms->outSl2Addr[cnt] = chPrms->sl2Addr;
        sl2Prms->outWidthInBytes[cnt] = chPrms->sl2Pitch;
        sl2Prms->outSl2BuffDepth[cnt] = chPrms->sl2Depth;

    }
}

void Vhwa_m2mVissUpdatePipeline(uint32_t htsPipeline)
{
    Vhwa_M2mVissInstObj *instObj = NULL;

    instObj = &gM2mVissInstObj[0U];

    /* Disable HTS Interrupt */
    Vhwa_disableHtsIntr(instObj->socInfo.vpacIntdRegs,
                instObj->vhwaIrqNum, instObj->pipeline);

    instObj->fcPipeline = htsPipeline;

    /* Enable Pipeline interrupt in INTD */
    Vhwa_enableHtsIntr(instObj->socInfo.vpacIntdRegs,
                instObj->vhwaIrqNum, instObj->pipeline);

    Vhwa_enableHtsIntr(instObj->socInfo.vpacIntdRegs,
                instObj->vhwaIrqNum, instObj->fcPipeline);
}

static void Vhwa_m2mVissUpdateFcPrms(Vhwa_M2mVissHandleObj *hObj,
                                    const Vhwa_M2mVissFcUpdatePrms *fcPrms)
{
    hObj->htsCfg.dmaConsCfg[0].enable = fcPrms->dmaConsEnable[0];
    hObj->htsCfg.dmaConsCfg[1].enable = fcPrms->dmaConsEnable[1];
    hObj->htsCfg.dmaConsCfg[2].enable = fcPrms->dmaConsEnable[2];
    hObj->htsCfg.dmaConsCfg[3].enable = fcPrms->dmaConsEnable[3];
    hObj->htsCfg.dmaConsCfg[4].enable = fcPrms->dmaConsEnable[4];

    hObj->fcStatus.outDmaEnable[0] = fcPrms->dmaConsEnable[0];
    hObj->fcStatus.outDmaEnable[1] = fcPrms->dmaConsEnable[1];
    hObj->fcStatus.outDmaEnable[2] = fcPrms->dmaConsEnable[2];
    hObj->fcStatus.outDmaEnable[3] = fcPrms->dmaConsEnable[3];
    hObj->fcStatus.outDmaEnable[4] = fcPrms->dmaConsEnable[4];
    hObj->fcStatus.outDmaEnable[5] = fcPrms->dmaConsEnable[5];

    /* DMA related Parameters */
    hObj->fcStatus.isFlexConnect = fcPrms->isFlexConnect;
}


static void Vhwa_m2mVissUpdateFcConnPrms(Vhwa_M2mVissHandleObj *hObj,
                                    const Vhwa_M2mVissFcConPrms *sl2FcPrms)
{
    uint8_t              *pTrMem = NULL;
    CSL_UdmapTR9          *pTr9;

    hObj->htsCfg.prodCfg[sl2FcPrms->ctrlIdx].enable = UTRUE;
    hObj->htsCfg.prodCfg[sl2FcPrms->ctrlIdx].isMaskSelect =
                                                sl2FcPrms->enableMaskSel;
    hObj->htsCfg.prodCfg[sl2FcPrms->ctrlIdx].maskSelect = sl2FcPrms->maskSel;

    hObj->htsCfg.prodCfg[sl2FcPrms->ctrlIdx].consId = sl2FcPrms->htsConsId;
    hObj->htsCfg.prodCfg[sl2FcPrms->ctrlIdx].depth = sl2FcPrms->sl2Depth;

    hObj->htsCfg.prodCfg[sl2FcPrms->ctrlIdx].threshold =
                                                sl2FcPrms->htsThreshold;
    hObj->htsCfg.prodCfg[sl2FcPrms->ctrlIdx].cntPreLoad =
                                                sl2FcPrms->htsDmaProdPreLoad;
    hObj->htsCfg.prodCfg[sl2FcPrms->ctrlIdx].cntPostLoad =
                                                sl2FcPrms->htsDmaProdPostLoad;
    hObj->htsCfg.prodCfg[sl2FcPrms->ctrlIdx].countDec  =
                                                sl2FcPrms->htsDmaProdCntDec;

    hObj->htsCfg.prodCfg[sl2FcPrms->outputIdx].depth = sl2FcPrms->sl2Depth;

    hObj->htsCfg.prodCfg[sl2FcPrms->outputIdx].paCfg.enable =
                        sl2FcPrms->paEnable;
    hObj->htsCfg.prodCfg[sl2FcPrms->outputIdx].paCfg.enableBufCtrl =
                        sl2FcPrms->paEnableBufCtrl;

    hObj->htsCfg.prodCfg[sl2FcPrms->outputIdx].paCfg.psMaxCnt =
                            sl2FcPrms->paPsMaxCnt;
    hObj->htsCfg.prodCfg[sl2FcPrms->outputIdx].paCfg.csMaxCnt =
                        sl2FcPrms->paCsMaxCnt;

    hObj->htsCfg.prodCfg[sl2FcPrms->outputIdx].paCfg.enableDecCtrl =
                        sl2FcPrms->paEnableDecCtrl;
    hObj->htsCfg.prodCfg[sl2FcPrms->outputIdx].paCfg.countDec =
                        sl2FcPrms->paCountDec;

    hObj->outChPrms[sl2FcPrms->outputIdx].sl2Addr = sl2FcPrms->sl2Addr;
    hObj->outChPrms[sl2FcPrms->outputIdx].sl2Depth = sl2FcPrms->sl2Depth;
    hObj->outChPrms[sl2FcPrms->outputIdx].sl2Pitch = sl2FcPrms->sl2Pitch;

    hObj->lseCfg.outChCfg[sl2FcPrms->outputIdx].bufAddr = (uint32_t)sl2FcPrms->sl2Addr;
    hObj->lseCfg.outChCfg[sl2FcPrms->outputIdx].circBufSize = sl2FcPrms->sl2Depth;
    hObj->lseCfg.outChCfg[sl2FcPrms->outputIdx].lineOffset = sl2FcPrms->sl2Pitch;


    pTrMem = hObj->outChPrms[sl2FcPrms->outputIdx].trMem;
    pTr9 = (CSL_UdmapTR9 *)((uint32_t)pTrMem + sizeof(CSL_UdmapTR9));
    VhwaAl_Cache_Inv(pTrMem, VHWA_M2M_VISS_UDMA_TRPD_SIZE, CacheP_TYPE_L1D);

    pTr9->addr    = sl2FcPrms->sl2Addr;
    pTr9->icnt1   = (uint16_t)sl2FcPrms->sl2Depth;
    pTr9->icnt2   =
        (uint16_t)(hObj->outChPrms[sl2FcPrms->outputIdx].height/sl2FcPrms->sl2Depth);

    pTr9->dim1 = (int32_t)sl2FcPrms->sl2Pitch;
    hObj->outChPrms[sl2FcPrms->outputIdx].sl2Pitch = sl2FcPrms->sl2Pitch;
    hObj->lseCfg.outChCfg[sl2FcPrms->outputIdx].lineOffset = sl2FcPrms->sl2Pitch;

    if ((pTr9->icnt2 * pTr9->icnt1) != hObj->outChPrms[sl2FcPrms->outputIdx].height)
    {
        pTr9->icnt2 ++;
    }

    pTr9->dim1 = (int32_t)sl2FcPrms->sl2Pitch;

    VhwaAl_Cache_Wb(pTrMem, VHWA_M2M_VISS_UDMA_TRPD_SIZE, CacheP_TYPE_L1D);

}

static int32_t vhwaM2mVissCheckCreatePrms(uint32_t drvId, uint32_t drvInstId)
{
    int32_t     status = FVID2_SOK;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_VHWA_M2M_VISS_DRV_ID != drvId)
    {
        GT_0trace(VhwaVissTrace, GT_ERR, "Invalid Driver ID !!\n");
        status = FVID2_EINVALID_PARAMS;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        /* Check for correct instance ID */
#if defined VHWA_M2M_VPAC_INSTANCE
#if (VHWA_M2M_VPAC_INSTANCE == 0)
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (VHWA_M2M_VPAC_0_VISS_DRV_INST_ID_0 != drvInstId)
        {
            status = FVID2_EINVALID_PARAMS;
            GT_0trace(VhwaVissTrace, GT_ERR, "Invalid/unsupported Instance Id\n");
        }
        /* LDRA_JUSTIFY_END */
#endif
#endif

#if defined VHWA_M2M_VPAC_INSTANCE
#if (VHWA_M2M_VPAC_INSTANCE == 1)
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (VHWA_M2M_VPAC_1_VISS_DRV_INST_ID_0 != drvInstId)
        {
            status = FVID2_EINVALID_PARAMS;
            GT_0trace(VhwaVissTrace, GT_ERR, "Invalid/unsupported Instance Id\n");
        }
        /* LDRA_JUSTIFY_END */
#endif
#endif
    }

    return (status);
}

void Vhwa_m2mDisableFcVissHtsIntr(void)
{
    Vhwa_M2mVissInstObj    *instObj = NULL;
    instObj = &gM2mVissInstObj[0U];

    /* Disable HTS Interrupt */
    Vhwa_disableHtsIntr(instObj->socInfo.vpacIntdRegs, instObj->vhwaIrqNum, instObj->fcPipeline);
}

void Vhwa_m2mVissGlbceCtxSaveRestoreEventCb(Udma_EventHandle eventHandle,
                                 uint32_t eventType,
                                 void *appData)
{
    Vhwa_M2mVissInstObj *instObj = &gM2mVissInstObj[0U];

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is NOT expected to accumulate the error status.
    An else block can be added to catch and print the error status in a future release.
    <justification end> */
    if (UDMA_EVENT_TYPE_DMA_COMPLETION == eventType)
    /* LDRA_JUSTIFY_END */
    {
        (void)VhwaAl_SemaphoreP_post(instObj->glbceCqEventSemaphore);
    }

    return;
}
static int32_t vhwaM2mVissSubmitGlbceCtxRestoreRequest(Vhwa_M2mVissInstObj *instObj, const Vhwa_M2mVissHandleObj *hObj)
{
    int32_t status = FVID2_SOK;
    int32_t semStatus = SYSTEMP_SUCCESS;
    uint8_t *pTrMem = hObj->glbceCntxtTrpd[VHWA_M2M_VISS_GLBCE_CTX_RESTORE_TRPD];
    uint64_t ringPopVal;
    const uint32_t *pTrResp;
    uint32_t trRespStatus;

    /* Submit TRPD to the ring */
    status = Udma_ringQueueRaw(instObj->glbceFqRingHndl, (uint64_t) pTrMem);
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (UDMA_SOK != status)
    {
        GT_0trace(VhwaVissTrace, GT_ERR, "Ring queue failed for GLBCE context restore\n");
        status = FVID2_EFAIL;
    }
    /* LDRA_JUSTIFY_END */

    /* Wait for the semaphore to be posted by the callback */
    if (FVID2_SOK == status)
    {
        semStatus = VhwaAl_SemaphoreP_pend(instObj->glbceCqEventSemaphore, SYSTEMP_WAIT_FOREVER);

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (SYSTEMP_SUCCESS != semStatus)
        {
            GT_0trace(VhwaVissTrace, GT_ERR, "GLBCE context restore timeout failure\n");
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        status = Udma_ringDequeueRaw(instObj->glbceCqRingHndl, &ringPopVal);
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Check TR response status */
        VhwaAl_Cache_Inv(pTrMem, VHWA_M2M_VISS_UDMA_GLBCE_TRPD_SIZE, CacheP_TYPE_L1D);
        pTrResp = (const uint32_t *) (pTrMem +
                             (sizeof(CSL_UdmapTR15) * 2U));
        trRespStatus = CSL_FEXT(*pTrResp, UDMAP_TR_RESPONSE_STATUS_TYPE);

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (CSL_UDMAP_TR_RESPONSE_STATUS_COMPLETE != trRespStatus)
        {
            GT_0trace(VhwaVissTrace, GT_ERR, "TR Response status is 'Not Complete' for GLBCE context restore\n");
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        GT_0trace(VhwaVissTrace, GT_ERR, "Ring dequeue failed for GLBCE context restore\n");
        status = FVID2_EFAIL;
    }
    /* LDRA_JUSTIFY_END */

    return status;
}

static int32_t vhwaM2mVissSubmitGlbceCtxSaveRequest(Vhwa_M2mVissInstObj *instObj, const Vhwa_M2mVissHandleObj *hObj)
{
    int32_t status = FVID2_SOK;
    int32_t semStatus = SYSTEMP_SUCCESS;
    uint8_t *pTrMem = hObj->glbceCntxtTrpd[VHWA_M2M_VISS_GLBCE_CTX_SAVE_TRPD];
    uint64_t ringPopVal;
    const uint32_t *pTrResp;
    uint32_t trRespStatus;
    /* Submit TRPD to the ring */
    status = Udma_ringQueueRaw(instObj->glbceFqRingHndl, (uint64_t) pTrMem);
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (UDMA_SOK != status)
    {
        GT_0trace(VhwaVissTrace, GT_ERR, "Ring queue failed for GLBCE context save\n");
        status = FVID2_EFAIL;
    }
    /* LDRA_JUSTIFY_END */

    /* Wait for the semaphore to be posted by the callback */
    if (FVID2_SOK == status)
    {
        semStatus = VhwaAl_SemaphoreP_pend(instObj->glbceCqEventSemaphore, SYSTEMP_WAIT_FOREVER);
 
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (SYSTEMP_SUCCESS != semStatus)
        {
            GT_0trace(VhwaVissTrace, GT_ERR, "GLBCE context save timeout failure\n");
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. An error print statement can be added in a future release if required.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        status = Udma_ringDequeueRaw(instObj->glbceCqRingHndl, &ringPopVal);
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Check TR response status */
        VhwaAl_Cache_Inv(pTrMem, VHWA_M2M_VISS_UDMA_GLBCE_TRPD_SIZE, CacheP_TYPE_L1D);
        pTrResp = (const uint32_t *) (pTrMem +
                             (sizeof(CSL_UdmapTR15) * 2U));
        trRespStatus = CSL_FEXT(*pTrResp, UDMAP_TR_RESPONSE_STATUS_TYPE);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (CSL_UDMAP_TR_RESPONSE_STATUS_COMPLETE != trRespStatus)
        {
            GT_0trace(VhwaVissTrace, GT_ERR, "TR Response status is 'Not Complete' for GLBCE context save\n");
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        GT_0trace(VhwaVissTrace, GT_ERR, "Ring dequeue failed for GLBCE context save\n");
    }
    /* LDRA_JUSTIFY_END */

    return status;
}

#if !defined(VHWA_VPAC_IP_REV_VPAC3L)
static int32_t Vhwa_m2mVissReconfigReinitReg(Vhwa_M2mVissInstObj *instObj,
                                             Vhwa_M2mVissHandleObj *hObj,
                                             const Vhwa_M2mVissQueueObj *qObj)
{
    int32_t status = FVID2_SOK;

    /* Null pointer check */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to prevent undefined behaviour due to dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == instObj) || (NULL == hObj) || (NULL == qObj))
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        /* GUARD: Configuration through UDMA must be enabled for this safety mechanism */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The safety mechanism requires UDMA configuration mode. This parameter is enabled by default during initialisation.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (true != instObj->initPrms.configThroughUdmaFlag)
        {
            GT_0trace(VhwaVissTrace, GT_ERR,
                "configThroughUdmaFlag must be enabled for register reconfiguration mechanism!!\n");
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */

        /* GUARD: Application buffer must be initialized */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((FVID2_SOK == status) && (true != hObj->appBuffInitDone))
        {
            GT_0trace(VhwaVissTrace, GT_ERR,
                "Application buffer not initialized for UDMA config transfer!!\n");
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */

        if (FVID2_SOK == status)
        {
            /* Re-enable VISS module in VPAC top */
            CSL_vpacEnableModule(instObj->socInfo.vpacCntlRegs,
                                 VPAC_MODULE_VISS0, (uint32_t)UTRUE);

            /* GLBCE Re-initialization */
            Vhwa_m2mVissGlbceInit(instObj, hObj);

            /* NSF4 Re-initialization */
            Vhwa_m2mVissNsf4Init(instObj, hObj);

            #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
            if ((uint32_t)UTRUE == hObj->vsPrms.enableCac)
            {
                Vhwa_m2mVissCacInit(instObj, hObj);
            }
            #endif

            #if defined (VHWA_VPAC_IP_REV_VPAC3L)
            if ((uint32_t)TRUE == hObj->vsPrms.enablePcid)
            {
                Vhwa_m2mVissPcidInit(instObj, hObj);
            }
            #endif

            /* Re-Init Pipeline interrupt in INTD */
            Vhwa_enableHtsIntr(instObj->socInfo.vpacIntdRegs,
                instObj->vhwaIrqNum, instObj->pipeline);

            if (instObj->lastHndlObj == qObj->hObj)
            {
                status = vhwaM2mVissSetConfigInHW(instObj, qObj);
                /* Enabling this flag results in reconfiguration of all .isUsed
                 * buffers during submit request, instead of just .isModified */
                hObj->sameAsPrevHandle = false;
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK != status)
            {
                GT_0trace(VhwaVissTrace, GT_ERR, "Vhwa_m2mVissReconfigReinitReg: vhwaM2mVissSetConfigInHW failed!\n");
            }
            /* LDRA_JUSTIFY_END */
        }
    }

    return status;
}

static int32_t vhwaM2mVissUpdateStatusRegGroup(Vhwa_M2mVissHandleObj *hObj)
{
    VissStatusRegisterGroup *statusRegs;
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

        /* Update the expected values for DST_BUF_CFG_CH_STATUS[5] for VISS LSE
         * VISS has 5 output channels
         */
        for (i = 1U; i < 5U; i++)
        {
            /* For VISS, we validate that channels are properly configured
             * The CH_DISABLED bit should match the output enable configuration
             */
            expected_val = 0U;
            /* Read the previous LSE DST BUF_CFG register value */
            uint32_t reg_val = CSL_REG32_RD(&gM2mVissInstObj[0].regAddrs.lseRegs->DST[i - 1U].BUF_CFG);

            /* Check if YUV422_OUT_EN is set using CSL_FEXT */
            if (1U == CSL_FEXT(reg_val, LSE_DST_BUF_CFG_YUV422_OUT_EN))
            {
                /* If YUV422_OUT_EN is set, set CH_DISABLED to true (1) for current LSE config */
                CSL_FINS(expected_val, LSE_DST_BUF_CFG_CH_DISABLED, (uint32_t)1U);
            }
            else
            {
                CSL_FINS(expected_val, LSE_DST_BUF_CFG_CH_DISABLED, (uint32_t)0U);
            }

            statusRegs->vissLseStatus.DST_BUF_CFG_CH_STATUS[i] = expected_val;
        }

        /* Update PROD_COUNT for VISS HWA producers */
        for (i = 0; i < CSL_HTS_NUM_VISS_PRODUCERS; i++)
        {
            const CSL_HtsSchConfig *cfg = &hObj->htsCfg;
            const CSL_HtsProdConfig *prodCfg = &cfg->prodCfg[i];

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((prodCfg != NULL) && (prodCfg->enable == UTRUE))
            /* LDRA_JUSTIFY_END */
            {
                expected_val = 0U;
                CSL_FINS(expected_val, HTS_HWA0_PROD0_COUNT_COUNT_POSTLOAD, prodCfg->cntPostLoad);
                statusRegs->vissHtsStatus.PROD_COUNT[i] = expected_val;
            }
            else
            {
                statusRegs->vissHtsStatus.PROD_COUNT[i] = 0U;
            }
        }

#if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
        /* Update HWA_HOP for VPAC3/VPAC3L */
        for (i = 0; i < CSL_HTS_NUM_VISS_PIPELINES; i++)
        {
            const CSL_HtsSchConfig *cfg = &hObj->htsCfg;
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((cfg != NULL) && (cfg->tdoneCntEnable == UTRUE))
            /* LDRA_JUSTIFY_END */
            {
                expected_val = 0U;
                CSL_FINS(expected_val, HTS_HWA0_HOP_TDONE_COUNT, cfg->numHop);
                statusRegs->vissHtsStatus.HWA_HOP[i] = expected_val;
            }
            else
            {
                statusRegs->vissHtsStatus.HWA_HOP[i] = 0U;
            }
        }
#endif

        /* Update PROD_DMA_HOP and PROD_DMA_COUNT for DMA producers */
        for (i = 0; i < CSL_HTS_NUM_VISS_PRODUCERS_DMA; i++)
        {
            const CSL_HtsSchConfig *cfg = &hObj->htsCfg;
            const CSL_HtsDmaProdConfig *dmaProdCfg = &cfg->dmaProdCfg[i];

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((dmaProdCfg != NULL) && (dmaProdCfg->enable == UTRUE))
            /* LDRA_JUSTIFY_END */
            {
                expected_val = 0U;
                CSL_FINS(expected_val, HTS_DMA0_PROD0_COUNT_COUNT_POSTLOAD, dmaProdCfg->cntPostLoad);
                statusRegs->vissHtsStatus.PROD_DMA_COUNT[i] = expected_val;

#if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
                expected_val = 0U;
                CSL_FINS(expected_val, HTS_DMA0_HOP_TDONE_COUNT, dmaProdCfg->numHop);
                statusRegs->vissHtsStatus.PROD_DMA_HOP[i] = expected_val;
#endif
            }
            else
            {
                statusRegs->vissHtsStatus.PROD_DMA_COUNT[i] = 0U;
#if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
                statusRegs->vissHtsStatus.PROD_DMA_HOP[i] = 0U;
#endif
            }
        }
    }

    return status;
}

int32_t Vhwa_m2mVissStatusRegValidate(Vhwa_M2mVissHandleObj *hObj, const Viss_SocInfo *socInfo)
{
    int32_t status = FVID2_SOK;
    VissStatusRegisterGroup *expected = NULL;
    const CSL_HtsHwaParams *htsPrms = NULL;
    expected = &hObj->statusRegs;
    htsPrms = GetVpacHtsParams(CSL_HTS_HWA_SCH_VISS0);
    uint32_t regVal;
    uint32_t vissBaseAddr;
    CSL_viss_topRegs *vissTopRegs = NULL;
    CSL_lseRegs *lseRegs = NULL;

    /* Null pointer check */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
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
        /* Validate VPAC PID */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)VHWA_HW_PID_VPAC != CSL_REG32_RD(&socInfo->vpacCntlRegs->PID))
        {
            GT_0trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: vpacCntlRegs->PID mismatch\n");
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
        /* Get VISS module register base addresses */
        vissBaseAddr = socInfo->vissBaseAddr;
        vissTopRegs = (CSL_viss_topRegs *)(vissBaseAddr + CSL_VPAC_VISS_VISS_TOP_REGS_BASE);
        lseRegs = (CSL_lseRegs *)(vissBaseAddr + CSL_VPAC_VISS_LSE_REGS_BASE);

        /* Validate VISS PID */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)VHWA_HW_PID_VISS != CSL_REG32_RD(&vissTopRegs->REVISION_REG))
        {
            GT_0trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: vissTopRegs->REVISION_REG mismatch\n");
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
        if(UTRUE == hObj->vsPrms.enableGlbce)
        {
            regVal = CSL_REG32_RD(&vissTopRegs->GLBCE_INT_STAT);

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((uint32_t)VHWA_M2M_VISS_GLBCE_INT_STAT != regVal)
            {
                GT_1trace(VhwaVissTrace, GT_ERR,
                    "StatusRegValidate: vissTopRegs->GLBCE_INT_STAT is 0x%x (expected 0x0 after clear)\n",
                    regVal);
                status = FVID2_EFAIL;
            }
            /* LDRA_JUSTIFY_END */
        }
    }

#if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
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
        /* Validate NSF4V_INT_STAT in VISS Top - should be 0 (no pending interrupts) */
        regVal = CSL_REG32_RD(&vissTopRegs->NSF4V_INT_STAT);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)FVID2_SOK != regVal)
        {
            GT_1trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: vissTopRegs->NSF4V_INT_STAT is 0x%x\n", regVal);
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */
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
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        /* Validate VISS_FUSE_STATUS - Fuse control status register
         * Bit 0 GLBCE_DISABLE: 0 = GLBCE HW available, 1 = not available
         * Bit 1 NIKON_DISABLE: 0 = Nikon features available, 1 = not available
         * Expected value: 0x0 (both features available)
         */
        regVal = CSL_REG32_RD(&vissTopRegs->VISS_FUSE_STATUS);

        /* Check GLBCE_DISABLE bit - should be 0 (GLBCE available) */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)FVID2_SOK != (regVal & CSL_VISS_TOP_VISS_FUSE_STATUS_GLBCE_DISABLE_MASK))
        {
            GT_1trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: VISS_FUSE_STATUS GLBCE_DISABLE is 0x%x (GLBCE not available)\n",
                (regVal & CSL_VISS_TOP_VISS_FUSE_STATUS_GLBCE_DISABLE_MASK));
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */

        /* Check NIKON_DISABLE bit - should be 0 (Nikon features available) */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)FVID2_SOK != (regVal & CSL_VISS_TOP_VISS_FUSE_STATUS_NIKON_DISABLE_MASK))
        {
            GT_1trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: VISS_FUSE_STATUS NIKON_DISABLE is 0x%x (Nikon features not available)\n",
                      (regVal & CSL_VISS_TOP_VISS_FUSE_STATUS_NIKON_DISABLE_MASK));
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
        /* Validate VISS_LINEMEM_SIZE - Maximum Line memory Size supported in VISS
         * Bits 13:0 LINEMEM_SZ: Read-only, reset value = 0x1000 (4096 pixels per line)
         * This is a read-only capability register indicating HW configuration
         * Expected value: 0x1000
         */
        regVal = CSL_REG32_RD(&vissTopRegs->VISS_LINEMEM_SIZE) & CSL_VISS_TOP_VISS_LINEMEM_SIZE_LINEMEM_SZ_MASK;
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (VHWA_M2M_VISS_MAX_WIDTH != regVal)
        {
            GT_1trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: vissTopRegs->VISS_LINEMEM_SIZE LINEMEM_SZ is 0x%x (expected 0x1000)\n",
                      regVal);
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */
    }

    #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
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
        /* Validate FREEPCLK_CFG bit 1 (PCLKFREE_STATE - GLBCE free running clock state) */
        regVal = CSL_REG32_RD(&vissTopRegs->FREEPCLK_CFG);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)FVID2_SOK != (regVal & CSL_VISS_TOP_FREEPCLK_CFG_PCLKFREE_STATE_MASK))
        {
            GT_1trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: vissTopRegs->FREEPCLK_CFG PCLKFREE_STATE is 0x%x\n",
                      (regVal & CSL_VISS_TOP_FREEPCLK_CFG_PCLKFREE_STATE_MASK));
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
    if ((status == FVID2_SOK) && (UTRUE == hObj->vsPrms.enableCac))
    /* LDRA_JUSTIFY_END */
    {
        /* Validate CAC INT_STAT - should be 0 (no errors) */
        CSL_cacRegs *cacRegs = (CSL_cacRegs *)(vissBaseAddr + CSL_VPAC_VISS_CAC_REGS_BASE);
        regVal = CSL_REG32_RD(&cacRegs->INT_STAT);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)FVID2_SOK != regVal)
        {
            GT_1trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: cacRegs->INT_STAT is 0x%x\n", regVal);
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */
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
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        /* Validate RAWFE INT_STAT - should be 0 (no errors) */
        CSL_rawfe_cfgRegs *rfeRegs = (CSL_rawfe_cfgRegs *)(vissBaseAddr + CSL_VPAC_VISS_RAWFE_CFG_REGS_BASE);
        regVal = CSL_REG32_RD(&rfeRegs->RAWFE_INT_STAT);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)FVID2_SOK != regVal)
        {
            GT_1trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: rfeRegs->INT_STAT is 0x%x\n", regVal);
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
    if ((status == FVID2_SOK) && (UTRUE == hObj->vsPrms.enableGlbce))
    /* LDRA_JUSTIFY_END */
    {
        /* Validate GLBCE registers */
        CSL_glbceRegs *glbceRegs = (CSL_glbceRegs *)(vissBaseAddr + CSL_VPAC_VISS_GLBCE_CFG_REGS_BASE);

        /* Validate GLBCE_CONTROL0 (GLBCE Control Register 0) */
        regVal = CSL_REG32_RD(&glbceRegs->CONTROL0);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)FVID2_SOK != (regVal & CSL_GLBCE_CONTROL0_CCTL_MASK))
        {
            GT_1trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: glbceRegs->CONTROL0 CCTL bit is 0x%x (expected 0x0)\n",
                      (regVal & CSL_GLBCE_CONTROL0_CCTL_MASK));
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */

        /* Validate GLBCE_FORMAT_CONTROL_REG0 */
        regVal = CSL_REG32_RD(&glbceRegs->FORMAT_CONTROL_REG0);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)FVID2_SOK != regVal)
        {
            GT_1trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: glbceRegs->FORMAT_CONTROL_REG0 is 0x%x\n", regVal);
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */

        /* Validate GLBCE_FORMAT_CONTROL_REG1
         * This register has non-zero reset value:
         * Bit 6 AUTOPOS = 1 (read-only, always 1 in this HW version)
         * Expected default value = CSL_GLBCE_FORMAT_CONTROL_REG1_AUTOPOS_MASK
         */
        regVal = CSL_REG32_RD(&glbceRegs->FORMAT_CONTROL_REG1);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (CSL_GLBCE_FORMAT_CONTROL_REG1_AUTOPOS_MASK != regVal)
        {
            GT_2trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: glbceRegs->FORMAT_CONTROL_REG1 is 0x%x (expected 0x%x)\n",
                      regVal, CSL_GLBCE_FORMAT_CONTROL_REG1_AUTOPOS_MASK);
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */

        /* Validate GLBCE OUTPUT_FLAGS */
        regVal = CSL_REG32_RD(&glbceRegs->OUTPUT_FLAGS);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)FVID2_SOK != regVal)
        {
            GT_1trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: glbceRegs->OUTPUT_FLAGS is 0x%x\n", regVal);
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
        /* Validate FLEXCC INT_STATUS - should be 0 (no errors) */
        CSL_flexcc_cfgRegs *fcpRegs = (CSL_flexcc_cfgRegs *)(vissBaseAddr + CSL_VPAC_VISS_FLEXCC_CFG_REGS_BASE);
        regVal = CSL_REG32_RD(&fcpRegs->FLEXCC_INT_STATUS);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)FVID2_SOK != regVal)
        {
            GT_1trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: fcpRegs->INT_STATUS is 0x%x\n", regVal);
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
        /* Validate FLEXCFA INT_STATUS - should be 0 (no errors) */
        CSL_flexcfaRegs *cfaRegs = (CSL_flexcfaRegs *)(vissBaseAddr + CSL_VPAC_VISS_FLEXCFA_REGS_BASE);
        regVal = CSL_REG32_RD(&cfaRegs->INT_STATUS);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)FVID2_SOK != regVal)
        {
            GT_1trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: cfaRegs->INT_STATUS is 0x%x\n", regVal);
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
        /* Validate FLEXEE INT_STATUS - should be 0 (no errors) */
        CSL_flexeeRegs *eeRegs = (CSL_flexeeRegs *)(vissBaseAddr + CSL_VPAC_VISS_FLEXEE_REGS_BASE);
        regVal = CSL_REG32_RD(&eeRegs->INT_STATUS);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)FVID2_SOK != regVal)
        {
            GT_1trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: eeRegs->INT_STATUS is 0x%x\n", regVal);
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */
    }

    #if defined VHWA_VPAC_IP_REV_VPAC3
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    /* VPAC3 has two FCP pipelines - validate FCP2 registers if enabled */
    if ((status == FVID2_SOK) && (UTRUE == hObj->vsPrms.enableMVPipe))
    /* LDRA_JUSTIFY_END */
    {
        /* Validate FLEXCC FCP2 INT_STATUS - should be 0 (no errors) */
        CSL_flexcc_cfgRegs *fcp2Regs = (CSL_flexcc_cfgRegs *)(vissBaseAddr + CSL_VPAC_VISS_FLEXCC_CFG_REGS_1_BASE);
        regVal = CSL_REG32_RD(&fcp2Regs->FLEXCC_INT_STATUS);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)FVID2_SOK != regVal)
        {
            GT_1trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: fcp2Regs->INT_STATUS is 0x%x\n", regVal);
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
    if ((status == FVID2_SOK) && (UTRUE == hObj->vsPrms.enableMVPipe))
    /* LDRA_JUSTIFY_END */
    {
        /* Validate FLEXCFA FCP2 INT_STATUS - should be 0 (no errors) */
        CSL_flexcfaRegs *cfa2Regs = (CSL_flexcfaRegs *)(vissBaseAddr + CSL_VPAC_VISS_FLEXCFA_REGS_1_BASE);
        regVal = CSL_REG32_RD(&cfa2Regs->INT_STATUS);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)FVID2_SOK != regVal)
        {
            GT_1trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: cfa2Regs->INT_STATUS is 0x%x\n", regVal);
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
    if ((status == FVID2_SOK) && (UTRUE == hObj->vsPrms.enableMVPipe))
    /* LDRA_JUSTIFY_END */
    {
        /* Validate FLEXEE FCP2 INT_STATUS - should be 0 (no errors) */
        CSL_flexeeRegs *ee2Regs = (CSL_flexeeRegs *)(vissBaseAddr + CSL_VPAC_VISS_FLEXEE_REGS_1_BASE);
        regVal = CSL_REG32_RD(&ee2Regs->INT_STATUS);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)FVID2_SOK != regVal)
        {
            GT_1trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: ee2Regs->INT_STATUS is 0x%x\n", regVal);
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */
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
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        /* Validate H3A PID and status */
        CSL_rawfe_h3a_cfgRegs *h3aRegs = (CSL_rawfe_h3a_cfgRegs *)(vissBaseAddr + CSL_VPAC_VISS_RAWFE_H3A_CFG_REGS_BASE);
        regVal = CSL_REG32_RD(&h3aRegs->PID);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)VHWA_HW_PID_H3A != regVal)
        {
            GT_1trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: h3aRegs->PID is 0x%x\n", regVal);
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
        /* Validate H3A_PCR register status bits
         * Bit 15 BUSYAF: Read-only busy bit for AF - should be 0 when not busy
         * Bit 18 BUSYAEAWB: Read-only busy bit for AE/AWB - should be 0 when not busy
         * Bit 21 OVF: rw1tc (H3A module overflow status) - should be 0 if no overflow
         */
        CSL_rawfe_h3a_cfgRegs *h3aRegs = (CSL_rawfe_h3a_cfgRegs *)(vissBaseAddr + CSL_VPAC_VISS_RAWFE_H3A_CFG_REGS_BASE);
        regVal = CSL_REG32_RD(&h3aRegs->PCR);

        /* Check BUSYAF bit (bit 15) */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)FVID2_SOK != (regVal & CSL_RAWFE_H3A_CFG_PCR_BUSYAF_MASK))
        {
            GT_1trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: h3aRegs->PCR BUSYAF bit is 0x%x\n",
                      (regVal & CSL_RAWFE_H3A_CFG_PCR_BUSYAF_MASK));
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */

        /* Check BUSYAEAWB bit (bit 18) */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)FVID2_SOK != (regVal & CSL_RAWFE_H3A_CFG_PCR_BUSYAEAWB_MASK))
        {
            GT_1trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: h3aRegs->PCR BUSYAEAWB bit is 0x%x\n",
                      (regVal & CSL_RAWFE_H3A_CFG_PCR_BUSYAEAWB_MASK));
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */

        /* Check OVF bit (bit 21) */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)FVID2_SOK != (regVal & CSL_RAWFE_H3A_CFG_PCR_OVF_MASK))
        {
            GT_1trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: h3aRegs->PCR OVF bit is 0x%x\n",
                      (regVal & CSL_RAWFE_H3A_CFG_PCR_OVF_MASK));
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
        /* Validate H3A_ADVANCED register */
        CSL_rawfe_h3a_cfgRegs *h3aRegs = (CSL_rawfe_h3a_cfgRegs *)(vissBaseAddr + CSL_VPAC_VISS_RAWFE_H3A_CFG_REGS_BASE);
        regVal = CSL_REG32_RD(&h3aRegs->ADVANCED);
        /* Check that ID field (bits 31:16) is 0x0 */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (0U != (regVal & CSL_RAWFE_H3A_CFG_ADVANCED_ID_MASK))
        {
            GT_1trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: h3aRegs->ADVANCED ID field is 0x%x (expected 0x0)\n",
                      ((regVal & CSL_RAWFE_H3A_CFG_ADVANCED_ID_MASK) >> CSL_RAWFE_H3A_CFG_ADVANCED_ID_SHIFT));
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */
    }

    #if defined VHWA_VPAC_IP_REV_VPAC3L
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
        /* Validate PCID INT_STAT - should be 0 (no errors) */
        CSL_pcidRegs *pcidRegs = (CSL_pcidRegs *)(vissBaseAddr + CSL_VPAC_VISS_PCID_REGS_BASE);
        regVal = CSL_REG32_RD(&pcidRegs->INT_STAT);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)FVID2_SOK != regVal)
        {
            GT_1trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: pcidRegs->INT_STAT is 0x%x\n", regVal);
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
        /* Validate VISS_INT_STAT
         * Bit 0 IROUT_OVF_ERR (rw1tc): IR output overflow error - should be 0
         */
        regVal = CSL_REG32_RD(&vissTopRegs->VISS_INT_STAT);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)FVID2_SOK != regVal)
        {
            GT_1trace(VhwaVissTrace, GT_ERR,
                "StatusRegValidate: vissTopRegs->VISS_INT_STAT is 0x%x\n", regVal);
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */
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
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        /* Validate LSE STATUS_ERROR - should be 0 (no errors) */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)FVID2_SOK != CSL_REG32_RD(&lseRegs->STATUS_ERROR))
        {
            GT_0trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: lseRegs->STATUS_ERROR mismatch\n");
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
        /* Validate LSE STATUS_IDLE_MODE */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)FVID2_SOK != CSL_REG32_RD(&lseRegs->STATUS_IDLE_MODE))
        {
            GT_0trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: lseRegs->STATUS_IDLE_MODE mismatch\n");
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
        /* Validate DST_BUF_CFG_CH_STATUS for 5 VISS output channels */
        for (uint32_t i = 0; i < 5U; i++)
        {
            uint32_t expected_val = (expected->vissLseStatus.DST_BUF_CFG_CH_STATUS[i] & CSL_LSE_DST_BUF_CFG_CH_DISABLED_MASK);
            uint32_t reg_val = (CSL_REG32_RD(&lseRegs->DST[i].BUF_CFG) & CSL_LSE_DST_BUF_CFG_CH_DISABLED_MASK);
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
                GT_1trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: lseRegs->DST[%d].BUF_CFG mismatch\n", i);
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
        /* VISS uses HWA0 scheduler */
        uint32_t scheduler_base_addr = (uint32_t)socInfo->htsRegs + htsPrms->schOffset;

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        /* Validate HTS SCHEDULER_CONTROL for HWA0 (VISS0) */
        if ((uint32_t)FVID2_SOK !=
            (CSL_REG32_RD(scheduler_base_addr) & CSL_HTS_HWA0_SCHEDULER_CONTROL_STATE_MASK))
        {
            GT_0trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: htsRegs->HWA0_SCHEDULER_CONTROL mismatch\n");
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
            /* Validate HTS WD_TIMER status for HWA0 */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((uint32_t)FVID2_SOK !=
                (CSL_REG32_RD(scheduler_base_addr + HWA_WDTIMER_OFFSET) & CSL_HTS_HWA0_WDTIMER_WDTIMER_STATUS_MASK))
            {
                GT_0trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: htsRegs->HWA0_WDTIMER mismatch\n");
                status = FVID2_EFAIL;
            }
            /* LDRA_JUSTIFY_END */
        }

        #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
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
            /* Validate HTS HWA HOP register */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((expected->vissHtsStatus.HWA_HOP[0] & CSL_HTS_HWA0_HOP_TDONE_COUNT_MASK) !=
                    (CSL_REG32_RD(scheduler_base_addr + HWA_HOP_OFFSET) & CSL_HTS_HWA0_HOP_TDONE_COUNT_MASK))
            {
                GT_0trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: htsRegs->HWA0_HOP mismatch\n");
                status = FVID2_EFAIL;
            }
            /* LDRA_JUSTIFY_END */
        }
        #endif

        /* Validate HTS PROD_COUNT and PA_PRODCOUNT */
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
            uint32_t prod_count_addr;
            CSL_HtsSchConfig *cfg = &hObj->htsCfg;

            for (uint32_t i = 0; i < CSL_HTS_NUM_VISS_PRODUCERS; i++)
            {
                CSL_HtsProdConfig *prodCfg = &cfg->prodCfg[i];
                if ((prodCfg != NULL) && (prodCfg->enable == UTRUE))
                {
                    prod_count_addr = (uint32_t)socInfo->htsRegs + htsPrms->prodOffset[i];

                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    The test framework does not support the configuration required to trigger this error scenario.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if ((expected->vissHtsStatus.PROD_COUNT[i] & CSL_HTS_HWA0_PROD0_COUNT_COUNT_MASK) !=
                        (CSL_REG32_RD(prod_count_addr + HWA_PROD_COUNT_OFFSET) & CSL_HTS_HWA0_PROD0_COUNT_COUNT_MASK))
                    {
                        GT_1trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: htsRegs->VISS0_PROD_COUNT[%d] mismatch\n", i);
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
                    if ((uint32_t)FVID2_SOK != (CSL_REG32_RD(prod_count_addr + HWA_PROD_PA_PROD_COUNT_OFFSET) & CSL_HTS_HWA0_PA0_PRODCOUNT_PA_PSCOUNT_MASK))
                    {
                        GT_1trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: htsRegs->VISS0_PA_PROD_COUNT[%d] mismatch\n", i);
                        status = FVID2_EFAIL;
                    }
                    /* LDRA_JUSTIFY_END */
                }
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
            for (uint32_t i = 0; i < CSL_HTS_NUM_VISS_PRODUCERS_DMA; i++)
            {
                CSL_HtsSchConfig *cfg = &hObj->htsCfg;
                CSL_HtsDmaProdConfig *dmaProdCfg = &cfg->dmaProdCfg[i];

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((dmaProdCfg != NULL) && (dmaProdCfg->enable == UTRUE))
                /* LDRA_JUSTIFY_END */
                {
                    uint32_t prod_dma_addr = (uint32_t)socInfo->htsRegs + htsPrms->dmaProdOffset[i];
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    The test framework does not support the configuration required to trigger this error scenario.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if ((uint32_t)FVID2_SOK !=
                        (CSL_REG32_RD(prod_dma_addr) & CSL_HTS_DMA0_SCHEDULER_CONTROL_STATE_MASK))
                    {
                        GT_1trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: htsRegs->VISS0_PROD_DMA_SCHEDULER_CONTROL[%d] mismatch\n", i);
                        status = FVID2_EFAIL;
                    }
                    /* LDRA_JUSTIFY_END */
                    /* Validate PROD_DMA_COUNT */
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    The test framework does not support the configuration required to trigger this error scenario.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if ((expected->vissHtsStatus.PROD_DMA_COUNT[i] & CSL_HTS_DMA0_PROD0_COUNT_COUNT_MASK) !=
                        (CSL_REG32_RD(prod_dma_addr + HWA_DMA_PROD_COUNT_OFFSET) & CSL_HTS_DMA0_PROD0_COUNT_COUNT_MASK))
                    {
                        GT_1trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: htsRegs->PROD_DMA_COUNT[%d] mismatch\n", i);
                        status = FVID2_EFAIL;
                    }
                    /* LDRA_JUSTIFY_END */

                    #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    The test framework does not support the configuration required to trigger this error scenario.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    /* Validate DMA producer HOP register */
                    if ((expected->vissHtsStatus.PROD_DMA_HOP[i] & CSL_HTS_DMA0_HOP_TDONE_COUNT_MASK) !=
                            (CSL_REG32_RD(prod_dma_addr + HWA_DMA_PROD_HOP_OFFSET) & CSL_HTS_DMA0_HOP_TDONE_COUNT_MASK))
                    {
                        GT_1trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: htsRegs->VISS0_PROD_DMA_HOP[%d] mismatch\n", i);
                        status = FVID2_EFAIL;
                    }
                    /* LDRA_JUSTIFY_END */
                    #endif
                }
            }
        }

        /* Validate DMA consumer scheduler control status */
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
            for (uint32_t i = 0; i < CSL_HTS_NUM_VISS_CONSUMERS_DMA; i++)
            {
                uint32_t cons_dma_addr = (uint32_t)socInfo->htsRegs + htsPrms->dmaConsOffset[i];

                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((uint32_t)FVID2_SOK !=
                    (CSL_REG32_RD(cons_dma_addr) & CSL_HTS_DMA240_SCHEDULER_CONTROL_STATE_MASK))
                {
                    GT_1trace(VhwaVissTrace, GT_ERR, "StatusRegValidate: htsRegs->VISS0_CONSUMER_DMA_SCHEDULER_CONTROL[%d] mismatch\n", i);
                    status = FVID2_EFAIL;
                }
                /* LDRA_JUSTIFY_END */
            }
        }
    }
    return status;
}

int32_t Vhwa_m2mVissConfigRegReadback(Vhwa_M2mVissHandleObj *hObj, Vhwa_M2mVissInstObj *instObj)
{
    int32_t status = FVID2_SOK;
    uint32_t *readback = NULL;
    CSL_htsRegs *htsRegs = NULL;

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
        readback = hObj->configRegMemPrms.configRegReadbackPtr;
        htsRegs = instObj->socInfo.htsRegs;
    }

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
	Rationale: The component level negative test framework and test applications cannot reach this portion.
	The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == readback) || (NULL == htsRegs))
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        /* Read current values from hardware registers */
        /* submit udma ring */
        status = Vhwa_m2mVissSubmitReadbackUDMABuf(instObj, hObj);
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
            GT_0trace(VhwaVissTrace, GT_ERR,
                "Vhwa_m2mVissSubmitReadbackUDMABuf Submit Failed !!\n");
        }
        /* LDRA_JUSTIFY_END */

    }

    return status;
}
#endif

int32_t Vhwa_m2mVissGetSl2Info(Vhwa_M2mVissSl2Info *sl2Info)
{
    int32_t retVal = FVID2_SOK;
    Vhwa_M2mVissInstObj *instObj = &gM2mVissInstObj[0U];

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
    /* LDRA_JUSTIFY_END */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else if ((uint32_t)UFALSE == instObj->isSl2AllocDone)
    {
        /* SL2 memory not allocated yet */
        GT_0trace(VhwaVissTrace, GT_ERR,
            "SL2 Memory is not allocated. Call Vhwa_m2mVissAllocSl2() first.\n");
        retVal = FVID2_EFAIL;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        sl2Info->sl2Addr = instObj->sl2Addr;
        sl2Info->sl2Size = instObj->sl2Size;
    }

    return retVal;
}

#if !defined(VHWA_VPAC_IP_REV_VPAC3L)
int32_t Vhwa_m2mVissSubmitReadbackUDMABuf(Vhwa_M2mVissInstObj *instObj,
    Vhwa_M2mVissHandleObj *hObj)
{
    int32_t status = FVID2_SOK;
    int32_t semStatus = FVID2_SOK;
    uint8_t *pTrMem = NULL;
    CSL_UdmapCppi5TRPD *pTrpd = NULL;
    uint32_t packetInfoSize;
    uint32_t numOfTr = 0;
    uint32_t buffObjectCnt = 0;
    Vhwa_M2mVissConfigBuffobj *bufferObjHolder = NULL;
    CSL_UdmapTR15 *pTrLocal = NULL;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
	Rationale: The component level negative test framework and test applications cannot reach this portion.
	The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((instObj == NULL) || (hObj == NULL))
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        VhwaAl_Cache_Wb(hObj->readbackBufferInfo.bufferPtr, hObj->readbackBufferInfo.length, CacheP_TYPE_L1D);

        /* Prepare UDMA TRPD for register readback */
        packetInfoSize = sizeof(CSL_UdmapTR15);
        pTrMem = instObj->configTxTrMem;
        pTrpd = (CSL_UdmapCppi5TRPD*) pTrMem;

        /* Invalidate cache before changing contents */
        VhwaAl_Cache_Inv(pTrMem, VHWA_M2M_VISS_UDMA_CONFIG_TRPD_SIZE, CacheP_TYPE_L1D);

        /* Setup buffer object holder for readback */
        bufferObjHolder = hObj->readbackBufferObjHolder;
        Vhwa_m2mVissUpdateReadbackBufObj(hObj);
        numOfTr = Vhwa_m2mVissCalcNumOfTrsForReadback(hObj);
        pTrLocal = (CSL_UdmapTR15*) ((uint32_t) pTrMem + packetInfoSize);

        /* Make TRPD */
        Vhwa_m2mVissmakeTrpd(pTrpd, UDMA_TR_TYPE_15, numOfTr, instObj->configCqRingNum);

        instObj->configTxTrRespMem = (uint8_t*) ((uint32_t) pTrMem + (packetInfoSize * (numOfTr + 1U)));

        /* Setup Transfer Records for each buffer object */
        for (buffObjectCnt = 0; buffObjectCnt < (uint32_t)BUFF_ID_MAXBUFID; buffObjectCnt++)
        {
            if (true == bufferObjHolder[buffObjectCnt].isUsed)
            {
                pTrLocal->flags =
                CSL_FMK(UDMAP_TR_FLAGS_TYPE, (uint32_t)15U) |
                CSL_FMK(UDMAP_TR_FLAGS_STATIC, (uint32_t)0U) |
                CSL_FMK(UDMAP_TR_FLAGS_EOL, (uint32_t)0U) |
                CSL_FMK(UDMAP_TR_FLAGS_EVENT_SIZE, CSL_UDMAP_TR_FLAGS_EVENT_SIZE_COMPLETION) |
                CSL_FMK(UDMAP_TR_FLAGS_TRIGGER0, CSL_UDMAP_TR_FLAGS_TRIGGER_NONE) |
                CSL_FMK(UDMAP_TR_FLAGS_TRIGGER0_TYPE, CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ALL) |
                CSL_FMK(UDMAP_TR_FLAGS_TRIGGER1, CSL_UDMAP_TR_FLAGS_TRIGGER_NONE) |
                CSL_FMK(UDMAP_TR_FLAGS_TRIGGER1_TYPE, CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ALL) |
                CSL_FMK(UDMAP_TR_FLAGS_CMD_ID, (uint32_t)0x25U) |
                CSL_FMK(UDMAP_TR_FLAGS_SA_INDIRECT, (uint32_t)0U) |
                CSL_FMK(UDMAP_TR_FLAGS_DA_INDIRECT, (uint32_t)0U) |
                CSL_FMK(UDMAP_TR_FLAGS_EOP, (uint32_t)1U);

                pTrLocal->icnt0 = (uint16_t)bufferObjHolder[buffObjectCnt].length;
                pTrLocal->icnt1 = 1U;
                pTrLocal->icnt2 = 1U;
                pTrLocal->icnt3 = 1U;
                pTrLocal->dim1 = (int32_t)pTrLocal->icnt0;
                pTrLocal->dim2 = 0;
                pTrLocal->dim3 = 0;
                pTrLocal->addr = (uint64_t)bufferObjHolder[buffObjectCnt].srcPtr;
                pTrLocal->fmtflags = 0x00000000U; /* Linear addressing, 1 byte per elem */
                pTrLocal->dicnt0 = (uint16_t)bufferObjHolder[buffObjectCnt].length;
                pTrLocal->dicnt1 = 1U;
                pTrLocal->dicnt2 = 1U;
                pTrLocal->dicnt3 = 1U;
                pTrLocal->ddim1 = (int32_t)pTrLocal->dicnt0;
                pTrLocal->ddim2 = 0;
                pTrLocal->ddim3 = 0;
                pTrLocal->daddr = (uint64_t)bufferObjHolder[buffObjectCnt].dstPtr;

                pTrLocal++;
            }
        }

        /* Clear TR response memory */
        *(uint32_t*) instObj->configTxTrRespMem = 0xFFFFFFFFU;

        /* Write back contents into memory after changing */
        VhwaAl_Cache_Wb(pTrMem, VHWA_M2M_VISS_UDMA_CONFIG_TRPD_SIZE, CacheP_TYPE_L1D);

        /* Submit linked list of UDMA TR's to ring queue */
        status = Vhwa_m2mVissSubmitConfigUDMARing(instObj, instObj->configTxTrMem);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != status)
        {
            status = FVID2_EFAIL;
            GT_0trace(VhwaVissTrace, GT_ERR, "Vhwa_m2mVissSubmitConfigUDMARing failed!!\n");
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
            semStatus = VhwaAl_SemaphoreP_pend(instObj->configCqEventSemaphore,
                SYSTEMP_WAIT_FOREVER);
            if (SYSTEMP_SUCCESS == semStatus)
            {
                status = FVID2_SOK;
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
                status = FVID2_EFAIL;
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
            /*
            * semaphore post informs transfer complete,
            * check the completion queue
            */
            status = Vhwa_m2mVissPopConfigRings(instObj);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK != status)
            {
                status = FVID2_EFAIL;
            }
            /* LDRA_JUSTIFY_END */
        }
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
            VhwaAl_Cache_Inv(hObj->readbackBufferInfo.bufferPtr,
                hObj->readbackBufferInfo.length, CacheP_TYPE_L1D);
        }
    }

    return status;
}

int32_t Vhwa_M2mVissConfigRegMemCompare(const Vhwa_M2mVissHandleObj *hObj, const Vhwa_M2mVissInstObj *instObj)
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
    if((NULL == hObj) || (NULL == instObj))
    {
        status = FVID2_EBADARGS;
        GT_0trace(VhwaVissTrace, GT_ERR, "ConfigRegMemCompare: Invalid arguments\n");
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
    if(status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(1U != CSL_FEXT(CSL_REG32_RD(&instObj->socInfo.vpacCntlRegs->ENABLE), VPAC_CNTL_ENABLE_VISS0_ENABLE))
        {
            GT_0trace(VhwaVissTrace, GT_ERR,
                "ConfigRegMemCompare: VISS enable mismatch (VPAC_CNTL_ENABLE_VISS0_ENABLE is not set)\n");
            status = FVID2_EFAIL;
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
    if(status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        for (uint32_t buffId = 0; buffId < (uint32_t)BUFF_ID_MAXBUFID; buffId++)
        {
            if((bool)UTRUE == hObj->readbackBufferObjHolder[buffId].isUsed)
            {
                Vhwa_M2mVissConfigRegMemParams configRegMemPtrs;
                const Vhwa_M2mVissConfigBuffobj *readbackObj = &hObj->readbackBufferObjHolder[buffId];
                const Vhwa_M2mVissConfigBuffobj *goldenObj = &hObj->bufferObjHolder[buffId];
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if(NULL != hObj->configRegMemPrms.cbFxn)
                /* LDRA_JUSTIFY_END */
                {
                    /* Only process if both objects are used and have valid pointers/length */
                    if ((readbackObj->dstPtr != NULL) && (goldenObj->srcPtr != NULL) &&
                        (readbackObj->length > 0U) && (goldenObj->length > 0U))
                    {
                        configRegMemPtrs.golden_ptr = goldenObj->srcPtr;
                        configRegMemPtrs.readback_ptr = readbackObj->dstPtr;
                        configRegMemPtrs.readback_mem_size = readbackObj->length;
                        configRegMemPtrs.appData = (void *)hObj->configRegMemPrms.appData;
                        
                        int32_t cbStatus = hObj->configRegMemPrms.cbFxn((const Fvid2_Handle)hObj, &configRegMemPtrs);

                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        This failure case is out of scope for the imaging test framework.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if (cbStatus != FVID2_SOK)
                        {
                            GT_1trace(VhwaVissTrace, GT_ERR, "ConfigRegReadbackCallback failed for BUFF_ID %d!\n", buffId);
                            status = cbStatus;
                        }
                        /* LDRA_JUSTIFY_END */
                    }
                }
                else
                {
                    status = FVID2_EBADARGS;
                    GT_0trace(VhwaVissTrace, GT_ERR, "ConfigRegMemCompare: Invalid arguments\n");
                }
            }
        }
    }
    return status;
}

int32_t Vhwa_M2mVissUpdateBuffers(const Vhwa_M2mVissHandleObj *hObj)
{
    int32_t status = FVID2_SOK;
    uint32_t i;
    Vhwa_M2mVissRegAddr readbackRegAddrs;

    if ((hObj != NULL))
    {
        readbackRegAddrs = hObj->readbackRegAddrs;
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
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(status == FVID2_SOK)
    {
        
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (readbackRegAddrs.topRegs != NULL) 
        /* LDRA_JUSTIFY_END */
        {
            CSL_REG32_WR(&readbackRegAddrs.topRegs->GLBCE_INT_STAT, 0U);
        }
        if(UTRUE == hObj->vsPrms.enableGlbce)
        {
            if (readbackRegAddrs.glbceRegs != NULL) 
            {
                /* Set CONTROL0 to ON for glbce, if glbce is ON; this is set to off by HWA*/
                CSL_REG32_FINS(&readbackRegAddrs.glbceRegs->CONTROL0, GLBCE_CONTROL0_ONOFF, 1U);
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
        if (readbackRegAddrs.rfeRegs != NULL) 
        /* LDRA_JUSTIFY_END */
        {
            readbackRegAddrs.rfeRegs->DBG_STAT2 = hObj->buffRegAddrs.rfeRegs->DBG_STAT2;
            
            readbackRegAddrs.rfeRegs->DBG_STAT3 = hObj->buffRegAddrs.rfeRegs->DBG_STAT3;
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (readbackRegAddrs.htsRegs != NULL)
        /* LDRA_JUSTIFY_END */
        {

#if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
            CSL_REG32_FINS(&readbackRegAddrs.htsRegs->HWA0_HOP, HTS_HWA0_HOP_TDONE_COUNT, 0U);

            CSL_REG32_FINS(&readbackRegAddrs.htsRegs->DMA0_HOP, HTS_DMA0_HOP_TDONE_COUNT, 0U);
            CSL_REG32_FINS(&readbackRegAddrs.htsRegs->DMA1_HOP, HTS_DMA1_HOP_TDONE_COUNT, 0U);
            CSL_REG32_FINS(&readbackRegAddrs.htsRegs->DMA2_HOP, HTS_DMA2_HOP_TDONE_COUNT, 0U);
            CSL_REG32_FINS(&readbackRegAddrs.htsRegs->DMA3_HOP, HTS_DMA3_HOP_TDONE_COUNT, 0U);
            CSL_REG32_FINS(&readbackRegAddrs.htsRegs->DMA4_HOP, HTS_DMA4_HOP_TDONE_COUNT, 0U);
#endif

        }

        for (i = 1U; i < 5U; i++)
        {
            /* For VISS, we validate that channels are properly configured
             * The CH_DISABLED bit should match the output enable configuration
             */
            /* Read the previous LSE DST BUF_CFG register value */
            uint32_t reg_val = CSL_REG32_RD(&gM2mVissInstObj[0].regAddrs.lseRegs->DST[i - 1U].BUF_CFG);

            /* Check if YUV422_OUT_EN is set using CSL_FEXT */
            if (1U == CSL_FEXT(reg_val, LSE_DST_BUF_CFG_YUV422_OUT_EN))
            {
                /* If YUV422_OUT_EN is set, set CH_DISABLED to true (1) for current LSE config */
                CSL_REG32_FINS(&readbackRegAddrs.lseRegs->DST[i].BUF_CFG, LSE_DST_BUF_CFG_CH_DISABLED, 0U);
            }
            /* Clear PSA_SIGNATURE register (set to 0) */
            Fvid2Utils_memset((void *)&readbackRegAddrs.lseRegs->PSA_SIGNATURE, 0, sizeof(readbackRegAddrs.lseRegs->PSA_SIGNATURE));
        }

#if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (readbackRegAddrs.nsf4Regs != NULL)
        /* LDRA_JUSTIFY_END */
        {
            CSL_REG32_FINS(&readbackRegAddrs.nsf4Regs->HIST_CTRL, NSF4V_HIST_CTRL_BANK, 0U);
        }
#endif
    }

    return status;
}
#endif