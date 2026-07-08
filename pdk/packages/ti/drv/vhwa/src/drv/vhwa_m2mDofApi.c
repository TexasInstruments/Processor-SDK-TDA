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
 *  \file vhwa_m2mDofApi.c
 *
 *  \brief API Implementation
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "src/drv/vhwa_m2mDofPriv.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/** Channel Indexes for HTS Configuration */
#define HTS_DOF_CUR_FRAME_CH_IDX        (1U)
#define HTS_DOF_REFE_FRAME_CH_IDX       (0U)
#define HTS_DOF_TEM_PRE_CH_IDX          (2U)
#define HTS_DOF_PYR_PRE_CH_IDX          (3U)
#define HTS_DOF_SOF_CH_IDX              (4U)
#define HTS_DOF_OUT_CH_IDX              (0U)

#if defined (VHWA_TEST_NEGATIVE)
#define TEST_DOF_WDTIMER_ERR_INTERRUPT
#endif

#if defined (VHWA_TEST_NEGATIVE_FOCO)
#define TEST_DOF_FOCO_WDTIMER_ERR_INTERRUPT
#endif

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
static Vhwa_M2mDofHandleObj *vhwaM2mDofAllocHandleObj(
    const Vhwa_M2mDofInstObj *instObj);

/**
 * \brief   Create Queues, required for storing pending requests
 *
 * \param   instObj             Instance object.
 *
 * \return  pointer to handle object on success else NULL.
 *
 **/
static int32_t vhwaM2mDofCreateQueues(Vhwa_M2mDofInstObj *instObj);

/**
 * \brief   Delete Queues
 *
 * \param   instObj             Instance object.
 *
 **/
static void vhwaM2mDofDeleteQueues(Vhwa_M2mDofInstObj *instObj);

/**
 * \brief   Local function to freeup allocated Handle Object and return it
 *          to the pool of free handle objects.
 *          No protection inside the function, Caller should protect
 *          the function call
 *
 * \param   hObj                Handle Object to be freed up.
 *
 **/
static void vhwaM2mDofFreeHandleObj(Vhwa_M2mDofHandleObj *hObj);

/**
 * \brief   Function to check given DOF configuration, used in
 *          SET_PARAMS ioctl it first initializes LSE and HTS
 *          config and then uses CSLFL of LSE and HTS to validate
 *          the configuration
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   dofPrms              DOF configuration to be validated
 *
 * \return  FVID2_SOK if given DOF cofiguration is valid, error code otherwise.
 *
 **/
static int32_t vhwaM2mDofCheckDofCfg(const Vhwa_M2mDofInstObj *instObj,
    Vhwa_M2mDofHandleObj *hObj, Vhwa_M2mDofPrms *dofPrms);

/**
 * \brief   Based on the given DOF config, it initializes LSE configuration.
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   dofPrms              DOF Configuration
 *
 **/
static void vhwaM2mDofSetLseCfg(Vhwa_M2mDofHandleObj *hObj,
                                const Vhwa_M2mDofPrms *dofPrms);

/**
 * \brief   Based on the given DOF config, it initializes HTS configuration.
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   dofPrms             DOF Configuration
 *
 **/
static void vhwaM2mDofSetHtsCfg(Vhwa_M2mDofInstObj *instObj,
    Vhwa_M2mDofHandleObj *hObj, const Vhwa_M2mDofPrms *dofPrms);

/**
 * \brief   Set the DMPAC FOCO prameters for reference and current buffers
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   dofPrms             DOF Configuration
 *
 **/
static void vhwaM2mDofSetFocoParams(Vhwa_M2mDofHandleObj *hObj,
                                       Vhwa_M2mDofPrms *dofPrms);

/**
 * \brief   Set the HTS bandwidth limiter parameters
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   bwLimit             BW limiter configuration
 *
 **/
static void vhwaM2mDofSetBwLimitParams(Vhwa_M2mDofHandleObj *hObj,
                                       const Vhwa_HtsLimiter *bwLimit);

/*  */
/**
 * \brief   Implementation of SET_PARAMS ioctl.
 *          It uses CheckDofCfg to validate the config.
 *          If it is valid, copies the config into handle object
 *          If it is invalid, it reverts LSE/HTS config to known valid config
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   dofCfg              DOF configuration to be set
 *
 **/
static int32_t vhwaM2mDofSetParams(Vhwa_M2mDofInstObj *instObj,
    Vhwa_M2mDofHandleObj *hObj, Vhwa_M2mDofPrms *dofPrms);

/**
 * \brief   Sets the error event parameters
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   eePrms              Error Event parameters
 *
 **/
static int32_t vhwaM2mDofSetEeParams(const Vhwa_M2mDofInstObj *instObj,
    Vhwa_M2mDofHandleObj *hObj, const Dof_ErrEventParams *eePrms);

/**
 * \brief   Sets the watchdog timer error event parameters
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   wdTimerEePrms       Watchdog Timer Error Event parameters
 *
 **/
static int32_t vhwaM2mDofSetWdTimerEeParams(const Vhwa_M2mDofInstObj *instObj,
    Vhwa_M2mDofHandleObj *hObj, const Dof_WdTimerErrEventParams *wdTimerEePrms);

/**
 * \brief   Calculate and store the Sl2 buffer parameters based on Dof
 *          configuration
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   dofPrms             DOF configuration to be set
 *
 **/
static int32_t vhwaM2mDofSetSl2Params(const Vhwa_M2mDofInstObj *instObj,
        Vhwa_M2mDofHandleObj *hObj, Vhwa_M2mDofPrms *dofPrms);

/**
 * \brief   Calculate and get the Dof parameters for all pyramid level based on
 *          Base pyramid level configuration and store in internal structure
 *
 * \param   hObj                Handle Object
 *
 **/
static void vhwaM2mDofCalPyrmPrms(Vhwa_M2mDofHandleObj *hObj);

/**
 * \brief   Set Sl2 buffer parameters for DOF for given configuration
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   dofPrms             DOF configuration to be set
 *
 **/
static void vhwaM2mDofSetBuffParams(Vhwa_M2mDofHandleObj *hObj,
                                    Vhwa_M2mDofPrms *dofPrms);

/**
 * \brief   Enable the FOCO engine. This function called while submitting the
 *          new request
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 *
 **/
static int32_t vhwaM2mDofEnableFoco(const Vhwa_M2mDofInstObj *instObj,
    const Vhwa_M2mDofHandleObj *hObj);


/**
 * \brief   Minimal function to submit request to hw and start DOF operation.
 *          It first sets address in the TR, Submits it to the ring and
 *          starts the pipeline.
 *          It does not start currently DOF, HTS schedulers.
 *
 * \param   instObj             Instance Object, used for getting base address
 * \param   qObj                Queue Object, used for getting handle object
 *
 **/
static int32_t vhwaM2mDofSubmitRequest(Vhwa_M2mDofInstObj *instObj,
    Vhwa_M2mDofQueueObj *qObj);

/**
 * \brief   Set DOF and WDTIMER interrupt enable/disable in INTD registers.
 *
 * This function configures the interrupt enable/disable settings in the DMPAC INTD
 * registers for DOF and Watchdog Timer error events, based on the handle object's
 * error event parameters.
 *
 * \param   vhwaIrqNum      VHWA IRQ number to configure.
 * \param   intdRegs        Pointer to DMPAC INTD register overlay.
 * \param   hObj            Pointer to the DOF handle object containing error event params.
 *
 * \return  FVID2_SOK on success, else FVID2 error code.
 */
static int32_t Vhwa_m2mDofSetIntrInHW(uint32_t vhwaIrqNum,
                                volatile CSL_dmpac_intd_cfgRegs  *intdRegs,
                                const Vhwa_M2mDofHandleObj *hObj);

/* Implementation of FVID2 APIs */

/**
 * \brief   FVID2 Create Function.
 *
 * \param   instObj             Instance object.
 *
 * \return  FVID2 Driver Handle.
 *
 **/
Fdrv_Handle vhwa_m2mDofCreate(UInt32 drvId, UInt32 drvInstId,
    Ptr createArgs, Ptr createStatusArgs, const Fvid2_DrvCbParams *cbPrms);

/**
 * \brief   FVID2 Delete Function.
 *
 * \param   handle              FVID2 driver handle.
 *
 * \return  FVID2_SOK on success, else FVID2 error code
 *
 **/
Int32 vhwa_m2mDofDelete(Fdrv_Handle handle, Ptr deleteArgs);

/**
 * \brief   FVID2 Control Function.
 *
 * \param   handle              FVID2 driver handle.
 *
 * \return  FVID2_SOK on success, else FVID2 error code
 *
 **/
Int32 vhwa_m2mDofControl(Fdrv_Handle handle, UInt32 cmd, Ptr cmdArgs,
    Ptr cmdStatusArgs);

/**
 * \brief   FVID2 Process Request function.
 *
 * \param   handle              FVID2 driver handle.
 *
 * \return  FVID2_SOK on success, else FVID2 error code
 *
 **/
Int32 vhwa_m2mDofProcessReq(Fdrv_Handle handle, Fvid2_FrameList *inFrmList,
    Fvid2_FrameList *outFrmList, uint32_t timeout);

/**
 * \brief   FVID2 Get Process Request function.
 *
 * \param   handle              FVID2 driver handle.
 *
 * \return  FVID2_SOK on success, else FVID2 error code
 *
 **/
Int32 vhwa_m2mDofGetProcessReq(Fdrv_Handle handle,
    Fvid2_FrameList *inFrmList, Fvid2_FrameList *outFrmList,
    UInt32 timeout);

static void Vhwa_m2mDofCalGwPrms(const Dof_Config *dofConfig, uint32_t *refBot,
                                 uint32_t *refTop, uint32_t *curBot,
                                 uint32_t *curTop);

#if !defined(SOC_J722S)
/**
 * \brief   Reconfigure DOF MMR registers as needed for the current handle/queue.
 *
 * \param   instObj   Instance Object.
 * \param   hObj      Handle Object.
 * \param   qObj      Queue Object.
 *
 * \return  FVID2_SOK on success, else FVID2 error code.
 */
static int32_t Vhwa_m2mDofReconfigReinitReg(const Vhwa_M2mDofInstObj *instObj,
                                      const Vhwa_M2mDofHandleObj *hObj,
                                      const Vhwa_M2mDofQueueObj *qObj);


/**
 * @brief Validates the DOF (Dense Optical Flow) status registers against expected values.
 *
 * This function compares the current DOF status register values with the expected values
 * provided in the 'expected' parameter. It uses the SoC-specific information from 'socInfo'
 * to perform the validation according to the hardware configuration.
 *
 * @param expected Pointer to a DofStatusRegisterGroup structure containing the expected
 *                 status register values.
 * @param socInfo  Pointer to a Dof_SocInfo structure containing SoC-specific information
 *                 required for validation.
 *
 * @return Returns 0 (FVID2_SOK) if the status registers match the expected values,
 *         or a negative error code if there is a mismatch or an error occurs.
 */
int32_t Vhwa_m2mDofStatusRegValidate(const DofStatusRegisterGroup *expected, const Dof_SocInfo *socInfo);


/**
 * \brief   Updates the DofStatusRegisterGroup structure in the handle object with expected status register values.
 *
 * \param   hObj    Pointer to the Vhwa_M2mDofHandleObj whose statusRegs should be updated.
 */
static int32_t vhwaM2mDofUpdateStatusRegGroup(Vhwa_M2mDofHandleObj *hObj);

/**
 * @brief Sets the default golden register memory values for the DOF (Dense Optical Flow) module.
 *
 * This function initializes the register memory values to their default "golden" state
 * for the specified DOF handle and instance object. It is typically used to ensure
 * that the hardware registers are set to known, validated values before operation.
 *
 * @param hObj Pointer to the DOF handle object.
 * @param instObj Pointer to the DOF instance object.
 *
 * @return Returns 0 on success, or a negative error code on failure.
 */
int32_t vhwaM2mDofSetDefaultGoldenRegMemValues(const Vhwa_M2mDofHandleObj *hObj, const Vhwa_M2mDofInstObj *instObj);
#endif

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
/* Default Confidence score configuration */
Dof_ConfScoreParam gConfScoreParam[] = {
    {
        224,
        {
            {{6, 7, 0},{ 14,  13,  0},
                {846,  (uint32_t)-846, (uint32_t)-846, (uint32_t)-846}},
            {{6, 5, 4},{ 30,  178, 33064},
                {(uint32_t)-450,  450, (uint32_t)-450,  450}},
            {{5, 6, 7},{ 499, 3,   28},
                {258,  (uint32_t)-258,  258, (uint32_t)-258}},
            {{5, 6, 0},{ 38,  14,  692},
                {(uint32_t)-152,  152,  152, (uint32_t)-152}},
            {{6, 6, 7},{ 14,  8,   22},
                {191,  (uint32_t)-191,  191, (uint32_t)-191}},
            {{6, 7, 7},{ 14,  13,  52},
                {(uint32_t)-158,  158,  158, (uint32_t)-158}},
            {{5, 7, 0},{ 625, 1,   632},
                {150,  (uint32_t)-150, (uint32_t)-150,  150}},
            {{0, 4, 7},{ 90,  4197,7},
                {(uint32_t)-91,   91,   91,  (uint32_t)-91}},
            {{4, 1, 4},{ 1126,70,  307},
                {(uint32_t)-5120, 5120, 5120, 5120}},
            {{4, 0, 0},{ 1126,0,   0},
                {5028,  5028,(uint32_t)-5028,(uint32_t)-5028}},
            {{6, 6, 5},{ 30,  3,   88},
                {123,  (uint32_t)-123, (uint32_t)-123,  123}},
            {{6, 4, 7},{ 19,  512, 28},
                {(uint32_t)-105,  105, (uint32_t)-105,  105}},
            {{6, 7, 5},{ 14,  13,  499},
                {(uint32_t)-107,  107,  107, (uint32_t)-107}},
            {{5, 5, 6},{ 562, 178, 47},
                {93,   (uint32_t)-93,   93,  (uint32_t)-93}},
            {{5, 0, 7},{ 79,  2234,34},
                {(uint32_t)-71,   71,   71,  (uint32_t)-71}},
            {{5, 5, 5},{ 163, 163, 734},
                {(uint32_t)-16384,0,    0,   (uint32_t)-16384}},

        }
    },
};

Vhwa_M2mDofHandleObj gM2mDofHandleObj[VHWA_M2M_DOF_MAX_HANDLES];
Vhwa_M2mDofInstObj   gM2mDofInstObj;

Fvid2_DrvOps gM2mDofFvid2DrvOps = {
    FVID2_VHWA_M2M_DOF_DRV_ID,
    /**< Unique driver Id. */
    vhwa_m2mDofCreate,
    /**< FVID2 create function pointer. */
    vhwa_m2mDofDelete,
    /**< FVID2 delete function pointer. */
    vhwa_m2mDofControl,
    /**< FVID2 control function pointer. */
    NULL, NULL,
    /**< FVID2 queue function pointer. */
    vhwa_m2mDofProcessReq,
    /**< FVID2 process request function pointer. */
    vhwa_m2mDofGetProcessReq,
    /**< FVID2 get processed request function pointer. */
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t Vhwa_m2mDofInit(Vhwa_M2mDofInitParams *initPrms)
{
    int32_t             status = FVID2_SOK;
    uint32_t            cnt;
    #if !defined(MCU_PLUS_SDK)
    SemaphoreP_Params   params;
    #endif
    Vhwa_M2mDofInstObj *instObj = NULL;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The test framework and test apps cannot reach this portion. initPrms always passing as non NULL value from appVhwaVpacInit()
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != initPrms)
    /* LDRA_JUSTIFY_END */
    {
        instObj = &gM2mDofInstObj;

        /* Reset all instance object to 0x0 */
        Fvid2Utils_memset(instObj, 0U, sizeof (gM2mDofInstObj));

        /* Mark pool flags as free */
        for (cnt = 0U; cnt < VHWA_M2M_DOF_MAX_HANDLES; cnt++)
        {
            gM2mDofHandleObj[cnt].isUsed = (uint32_t) UFALSE;
        }

        /* Initialize stalled flag to FALSE */
        instObj->isStalled = (uint32_t)UFALSE;

        /* Set HTS pipeline */
        instObj->pipeline = VHWA_M2M_DOF_HTS_PIPELINE;

        Dof_getSocInfo(&instObj->socInfo);

        status = Fvid2_registerDriver(&gM2mDofFvid2DrvOps);

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
            GT_0trace(VhwaDofTrace, GT_ERR,
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
                GT_0trace(VhwaDofTrace, GT_ERR,
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
            status = Vhwa_commonCreateHwaLock(VHWA_DMPAC_DOF_LOCK_IDX);
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
            status = vhwaM2mDofCreateQueues(instObj);
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
               required for DOF output */
            status = Vhwa_m2mDofUdmaInit(instObj, initPrms);
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
                GT_0trace(VhwaDofTrace, GT_ERR,
                    "UDMA Initialization Failed !!\n");
            }
            /* LDRA_JUSTIFY_END */
        }

        /* Register ISR handler for the given irq number */
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
            instObj->irqNum = initPrms->irqInfo.irqNum;
            instObj->vhwaIrqNum = initPrms->irqInfo.vhwaIrqNum;

            status = Vhwa_m2mDofRegisterIsr(instObj);
        }

        /* Init is done, copy init params locally,
           enable DMPAC and DOF in DMPAC Top */

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
                sizeof(Vhwa_M2mDofInitParams));

            /* Enable DOF at DMPAC Top*/
            CSL_dmpacEnableModule(instObj->socInfo.dmpacCntlRegs,
                DMPAC_MODULE_DOF, (uint32_t)UTRUE);

            instObj->initDone = (uint32_t)UTRUE;
            instObj->lastHndlObj = NULL;
        }
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
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
        Vhwa_m2mDofDeInit();
    }
    /* LDRA_JUSTIFY_END */
    return (status);
}

void Vhwa_m2mDofDeInit(void)
{
    Vhwa_M2mDofInstObj *instObj = NULL;
    int32_t status = FVID2_SOK;

    instObj = &gM2mDofInstObj;
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> Rationale: The negative test framework and test apps cannot reach this portion.
    Effect on this unit: An else statement has no effect on these units since the deinit function is
    supposed to do nothing if the above params are already NULL, which is consistent with the current behaviour
    <justification end> */
    if (instObj->openCnt > 0u)
    /* LDRA_JUSTIFY_END */
    {
        GT_0trace(VhwaDofTrace, GT_ERR,
            "Warning: All driver handles are not closed!!\n");
    }
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
        status = Vhwa_m2mDofFreeSl2();
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
            GT_0trace(VhwaDofTrace, GT_ERR,
                "SL2 Free Failed !!\n");
        }
        /* LDRA_JUSTIFY_END */
    }
    /* Stop UDMA Channels */
    (void)Vhwa_m2mDofStopCh(instObj);

    Vhwa_m2mDofUnregisterIsr(instObj);

    (void)Vhwa_m2mDofUdmaDeInit(instObj);
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> Rationale: The negative test framework and test apps cannot reach this portion.
    Effect on this unit: An else statement has no effect on these units since the deinit function is
    supposed to do nothing if the above params are already NULL, which is consistent with the current behaviour
    <justification end> */
    if ((uint32_t)UTRUE == instObj->isRegistered)
    /* LDRA_JUSTIFY_END */
    {
        (void)Fvid2_unRegisterDriver(&gM2mDofFvid2DrvOps);
    }

    /* Delete the lock semaphore */
    #if defined(MCU_PLUS_SDK)
     SemaphoreP_destruct(&instObj->lock);
    #else
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> Rationale: The negative test framework and test apps cannot reach this portion.
    Effect on this unit: An else statement has no effect on these units since the deinit function is
    supposed to do nothing if the above params are already NULL, which is consistent with the current behaviour
    <justification end> */
    if (NULL != instObj->lock)
    /* LDRA_JUSTIFY_END */
    {
        (void)SemaphoreP_delete(instObj->lock);
        instObj->lock = NULL;
    }
    #endif

    Vhwa_commonDeleteHwaLock(VHWA_DMPAC_DOF_LOCK_IDX);

    vhwaM2mDofDeleteQueues(instObj);

    /* Init all global variables to zero */
    Fvid2Utils_memset(instObj, 0U, sizeof (gM2mDofInstObj));

    instObj->initDone = (uint32_t)UFALSE;
}

int32_t Vhwa_m2mDofAllocSl2(const Vhwa_M2mDofSl2AllocPrms *sl2AllocPrms)
{
    int32_t status = FVID2_SOK;
    uint32_t sl2MemReq = 0;
    uint32_t pitchSl2, sofPitch, focoBuffPitch;
    Vhwa_M2mDofInstObj *instObj = NULL;

    instObj = &gM2mDofInstObj;

    /* Cannot even lock, if init is not done */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> Rationale: The negative test framework and test apps cannot reach this portion.
    sl2AllocPrms is created and passed at the driver level during app init. It's used to calculate the sl2 memory requirements.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((uint32_t)UFALSE == instObj->initDone)
    {
        GT_0trace(VhwaDofTrace, GT_ERR,
            "Driver init is not done!!\n");
        status = FVID2_EFAIL;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        /* Still need to check if provided sl2AllocPrms is not null and SL2 is
         * not allocated */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> Rationale: The negative test framework and test apps cannot reach this portion.
        sl2AllocPrms is created and passed at the driver level during app init. It's used to calculate the sl2 memory requirements.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
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
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Ref and Base Memory Required */
        /* Image data in SL2 should always starts at a 64 byte aligned boundary*/
        /* Since if width is multiple of 256 to alivate peak SL2 usage Line
           staggering is required */
        pitchSl2 = Vhwa_calcHorzSizeInBytes(sl2AllocPrms->maxImgWidth,
                            FVID2_CCSF_BITS12_PACKED);
        pitchSl2 = Vhwa_calcSl2Pitch(pitchSl2);

        /* RFGW SL2 Memory required */
        sl2MemReq += pitchSl2 * (((sl2AllocPrms->topSR + 7u) & 0xFFFFFFFEu)
                            + 2u + ((sl2AllocPrms->botSR + 10u) & 0xFFFFFFFEu)
                            + sl2AllocPrms->refBuffDepth);

        /* CFGW SL2 Memory required */
        sl2MemReq += pitchSl2 * (6u + 2u + 8u + sl2AllocPrms->currBuffDepth);

        /* Calculate Temporal Predictor memory requirement */
        sl2MemReq += CSL_lseMakePitchAligned(sl2AllocPrms->maxImgWidth * 4U) *
                        DOF_TEMPORAL_PRED_SL2_DEPTH;

        /* Calculate Pyramidal Predictor memory requirement */
        sl2MemReq += CSL_lseMakePitchAligned(sl2AllocPrms->maxImgWidth) *
                        DOF_PYRAM_PRED_SL2_DEPTH;

        /* Calculate SOF memory requirement */
        sofPitch = sl2AllocPrms->maxImgWidth/8u;

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> Rationale: The test framework and test apps cannot reach this portion. Covering in NEGATIVE_TEST build.
        Effect on this unit: The pitch is expected to be incremented by 1 if the width is not a multiple of 8.
        This is to ensure that the SOF buffer is aligned to 8 bytes.
        <justification end> */
        if((sofPitch * 8u) != sl2AllocPrms->maxImgWidth)
        {
            sofPitch++;
        }
        /* LDRA_JUSTIFY_END */
        sofPitch = CSL_lseMakePitchAligned(sofPitch);

        sl2MemReq += sofPitch * DOF_SOF_SL2_DEPTH;

        /* Output Buffer Memory Required */
        sl2MemReq += CSL_lseMakePitchAligned(sl2AllocPrms->maxImgWidth * 4U) *
                        sl2AllocPrms->fvBuffDepth;


        /* Memory required for FOCO for Ref and Curr buffer */
        focoBuffPitch = Vhwa_calcHorzSizeInBytes(sl2AllocPrms->maxImgWidth,
                            sl2AllocPrms->inCcsf);
        focoBuffPitch = CSL_lseMakePitchAligned(focoBuffPitch);
        sl2MemReq += focoBuffPitch * DOF_FOCO_SL2_DEPTH * 2u;

        /* Apply 16KB alignment for size and address if requested for firewall support */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: The unit is NOT expected to result in an error. This behaviour is part of the application design.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)TRUE == sl2AllocPrms->enable16KBAlignment)
        /* LDRA_JUSTIFY_END */
        {
            /* DOF TWO-REGION ALLOCATION (uses DMPAC firewall regions 0 and 1):
             *
             * Region 0 (MAIN/DMPAC_FWL_REGION_DOF_MAIN):
             *   Contents: Current image + FOCO current + Predictors + Flow output
             *   Permissions: R5(213), FOCO(211), DOF(210)
             *
             * Region 1 (REFERENCE/DMPAC_FWL_REGION_DOF_REFERENCE):
             *   Contents: Reference image + FOCO reference
             *   Permissions: R5(213), FOCO(211), DOF_RD(209)
             *
             * Memory Layout: [Region0: MAIN][Region1: REFERENCE]
             * Both regions are 16KB-aligned (firewall requirement)
             */

            /* Calculate component sizes */
            /* RFGW: Reference image working buffer */
            uint32_t rfgwSize = pitchSl2 * (((sl2AllocPrms->topSR + 7u) & 0xFFFFFFFEu)
                                + 2u + ((sl2AllocPrms->botSR + 10u) & 0xFFFFFFFEu)
                                + sl2AllocPrms->refBuffDepth);
            /* CFGW: Current image working buffer */
            uint32_t cfgwSize = pitchSl2 * (6u + 2u + 8u + sl2AllocPrms->currBuffDepth);

            /* FOCO buffer size (used for both current and reference preprocessing) */
            uint32_t focoBuffMem = Vhwa_calcHorzSizeInBytes(sl2AllocPrms->maxImgWidth,
                                sl2AllocPrms->inCcsf);
            focoBuffMem = CSL_lseMakePitchAligned(focoBuffMem) * DOF_FOCO_SL2_DEPTH;

            /* Predictors size: Temporal + Pyramid + SOF (already calculated above in sl2MemReq) */
            uint32_t predictorsRegionSize = (CSL_lseMakePitchAligned(sl2AllocPrms->maxImgWidth * 4U) *
                                DOF_TEMPORAL_PRED_SL2_DEPTH) +
                                (CSL_lseMakePitchAligned(sl2AllocPrms->maxImgWidth) *
                                DOF_PYRAM_PRED_SL2_DEPTH) +
                                (sofPitch * DOF_SOF_SL2_DEPTH);

            /* Output buffer size */
            uint32_t outputRegionSize = CSL_lseMakePitchAligned(sl2AllocPrms->maxImgWidth * 4U) *
                                sl2AllocPrms->fvBuffDepth;

            /* Calculate Region 0 (MAIN) size: Current + FOCO_cur + Predictors + Output */
            uint32_t region0Size = cfgwSize + focoBuffMem + predictorsRegionSize + outputRegionSize;
            uint32_t region0SizeAligned = (region0Size + 0x3FFFu) & ~0x3FFFu;  /* 16KB aligned */

            /* Calculate Region 1 (REFERENCE) size: Reference + FOCO_ref */
            uint32_t region1Size = rfgwSize + focoBuffMem;
            uint32_t region1SizeAligned = (region1Size + 0x3FFFu) & ~0x3FFFu;  /* 16KB aligned */

            /* Total physical allocation (both regions contiguous) */
            uint32_t totalSize = region0SizeAligned + region1SizeAligned;

            /* Allocate ONE contiguous 16KB-aligned block for both regions */
            instObj->startAddr = Vhwa_allocateSl2Aligned(totalSize, VHWA_SL2_INST_DMPAC, 0x4000u);

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (instObj->startAddr != 0u)
            /* LDRA_JUSTIFY_END */
            {
                /* Region 0 (MAIN) - maps to DMPAC_FWL_REGION_DOF_MAIN */
                instObj->mainRegionStartAddr = instObj->startAddr;
                instObj->mainRegionSl2Size = region0SizeAligned;

                /* Region 1 (REFERENCE) - maps to DMPAC_FWL_REGION_DOF_REFERENCE */
                instObj->refRegionStartAddr = instObj->startAddr + region0SizeAligned;
                instObj->refRegionSl2Size = region1SizeAligned;

                /* Total SL2 size */
                instObj->sl2Size = totalSize;
                instObj->isSl2AllocDone = (uint32_t)UTRUE;
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
                GT_0trace(VhwaDofTrace, GT_ERR,
                    "SL2 aligned allocation failed (check alignment=0x4000 is power of 2 and sufficient SL2 memory available)\n");
                status = FVID2_EALLOC;
                instObj->isSl2AllocDone = (uint32_t)UFALSE;
            }
            /* LDRA_JUSTIFY_END */
        }
        else
        {
            /* Backward compatibility: single contiguous allocation when firewall not enabled */
            instObj->startAddr = Vhwa_allocateSl2(sl2MemReq, VHWA_SL2_INST_DMPAC);

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The test framework and test apps cannot reach this portion. instObj->startAddr couldnot be zero, assigning a default address in Vhwa_allocateSl2
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if(instObj->startAddr == 0u)
            {
                status = FVID2_EALLOC;
                instObj->isSl2AllocDone = (uint32_t)UFALSE;
            }
            else
            /* LDRA_JUSTIFY_END */
            {
                /* For non-firewall mode, no partition info needed (no firewall to configure) */
                instObj->sl2Size = sl2MemReq;
                instObj->mainRegionStartAddr = 0u;
                instObj->refRegionStartAddr = 0u;
                instObj->isSl2AllocDone = (uint32_t)UTRUE;
            }
        }
    }

    /* Release instance semaphore */
    (void)VhwaAl_SemaphoreP_post(instObj->lock);

    return (status);
}

int32_t Vhwa_m2mDofFreeSl2(void)
{
    int32_t retVal = FVID2_SOK;
    Vhwa_M2mDofInstObj *instObj = NULL;

    instObj = &gM2mDofInstObj;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> Rationale: The negative test framework and test apps cannot reach this portion.
    initObj→isSl2AllocDone is used to check whether sl2 buffer has already been allocated before allocating it.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (UTRUE == instObj->isSl2AllocDone)
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> Rationale: Vhwa_m2mDofFreeSl2 is called to free allocated SL2. It is not being called
        in any test case at the vhwa or imaging level.
        Effect on this unit: It is unused code at the imaging and LLD tests level
        <justification end> */
        if(0U == instObj->openCnt)
        {
            /* Lock instance semaphore */
            (void)VhwaAl_SemaphoreP_pend(instObj->lock, SYSTEMP_WAIT_FOREVER);

            Vhwa_FreeSl2(VHWA_SL2_INST_DMPAC);

            instObj->isSl2AllocDone = (uint32_t)UFALSE;
            instObj->sl2Size        = 0u;
            instObj->startAddr      = 0u;

            /* Release instance semaphore */
            (void)VhwaAl_SemaphoreP_post(instObj->lock);
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            retVal = FVID2_EFAIL;
        }
    }

    return (retVal);
}

int32_t Vhwa_m2mDofGetSl2Info(Vhwa_M2mDofSl2Info *sl2Info)
{
    int32_t retVal = FVID2_SOK;
    Vhwa_M2mDofInstObj *instObj = &gM2mDofInstObj;

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
        GT_0trace(VhwaDofTrace, GT_ERR,
            "SL2 Memory is not allocated. Call Vhwa_m2mDofAllocSl2() first.\n");
        retVal = FVID2_EFAIL;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        /* Populate extended info with 2-region details for firewall use cases */
        sl2Info->sl2Addr = instObj->startAddr;
        sl2Info->sl2Size = instObj->sl2Size;

        sl2Info->mainRegionSl2Addr = instObj->mainRegionStartAddr;
        sl2Info->mainRegionSl2Size = instObj->mainRegionSl2Size;

        sl2Info->refRegionSl2Addr = instObj->refRegionStartAddr;
        sl2Info->refRegionSl2Size = instObj->refRegionSl2Size;
    }

    return retVal;
}

void Vhwa_m2mConfScoreParamInit(Dof_ConfScoreParam *csPrms)
{
    (void)memcpy(csPrms, &gConfScoreParam[0], sizeof(Dof_ConfScoreParam));
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
<metric start> statement <metric end>
<justification start>
Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
<justification end> */
Vhwa_M2mDofHandleObj *Vhwa_m2mDofGetHandleObj(uint32_t cnt)
{
    return &gM2mDofHandleObj[cnt];
}
/* LDRA_JUSTIFY_END */

#if !defined(SOC_J722S)
int32_t Vhwa_m2mDofReInit(void)
{
    int32_t                status = FVID2_SOK;
    Vhwa_M2mDofInstObj    *instObj = NULL;

    /** No lock is required, as reinit is called only when HW is not accessed by any other
     *  handle, so it is ok to access the members here. */
    instObj = &gM2mDofInstObj;

    /** If initDone is set, then Vhwa_m2mDofUdmaInit is called and UDMA
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
        /* First close all channels, so that they can be reopend */

        status = Vhwa_m2mDofStopCh(instObj);

        if(FVID2_SOK == status)
        {
            status = Vhwa_m2mDofUdmaDeInit(instObj);
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
            /* Need to unregister ISR, so it can be re-registered in next step */
            Vhwa_m2mDofUnregisterIsr(instObj);
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
            status = Vhwa_m2mDofUdmaInit(instObj, &instObj->initPrms);
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
            status = Vhwa_m2mDofRegisterIsr(instObj);
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
                DMPAC_MODULE_DOF, (uint32_t)UTRUE);
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
            status = Vhwa_m2mDofStartCh(instObj);
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

        /* Setting last handle to false/null, so that reconfiguration is done */
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

Fdrv_Handle vhwa_m2mDofCreate(UInt32 drvId, UInt32 drvInstId,
    Ptr createArgs, Ptr createStatusArgs, const Fvid2_DrvCbParams *cbPrms)
{
    int32_t                 status = FVID2_SOK;
    Vhwa_M2mDofInstObj     *instObj = NULL;
    Vhwa_M2mDofHandleObj   *hObj = NULL;
    Vhwa_M2mDofCreateArgs  *dofCreateArgs = NULL;
    Fdrv_Handle             handle = NULL;
    Dof_SocInfo            *socInfo = NULL;
    Vhwa_M2mDofSl2AllocPrms sl2AllocPrms;

    instObj = &gM2mDofInstObj;
    socInfo = &instObj->socInfo;

    /* Check for errors */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> Rationale: The negative test framework and test apps cannot reach this portion.
    The Create function at the level above fvid2 creates and populates cbPrms and createArgs before passing
    it down to the vhwa create function through fvid2.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((FVID2_VHWA_M2M_DOF_DRV_ID != drvId) ||
        (VHWA_M2M_DOF_DRV_INST_ID != drvInstId) ||
        (NULL == createArgs) ||
        (NULL == cbPrms))
    {
        GT_0trace(VhwaDofTrace, GT_ERR, "NULL Pointer !!\n");
        status = FVID2_EBADARGS;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        /* Open not allowed if init is not done */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> Rationale: instObj→initDone is checked to be TRUE at Vhwa_m2mDofInit.
        Therefore, the statement is not hit in create function. The negative test framework and test apps cannot reach this portion.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)UFALSE == instObj->initDone)
        {
            GT_0trace(VhwaDofTrace, GT_ERR,
                "Vhwa_m2mDofInit is not called\n");
            status  = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */

        dofCreateArgs = (Vhwa_M2mDofCreateArgs *)createArgs;
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> Rationale: The test framework and test apps cannot reach this portion. Passing only success case to here
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Take the instance semaphore */
        (void)VhwaAl_SemaphoreP_pend(instObj->lock, SYSTEMP_WAIT_FOREVER);

        /* Allocate Handle Object */
        hObj = vhwaM2mDofAllocHandleObj(instObj);

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
                <justification start> Rationale: sl2AllocDone should be true if there is an open instance of the hardware.
                The negative test framework and test apps cannot reach this portion.
                Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((uint32_t)UFALSE == instObj->isSl2AllocDone)
                {
                    GT_0trace(VhwaDofTrace, GT_DEBUG,
                    "Vhwa_m2mDofAllocSl2 is not called, allocating for default\n");

                    Vhwa_M2mDofSl2AllocPrmsInit(&sl2AllocPrms);
                    status = Vhwa_m2mDofAllocSl2(&sl2AllocPrms);
                }
                /* LDRA_JUSTIFY_END */
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This branch statement checks for errors accumulated in previous steps.
                Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested..
                <justification end> */
                if (FVID2_SOK == status)
                /* LDRA_JUSTIFY_END */
                {
                    /* Start UDMA Channels on the first handle Create */
                    status = Vhwa_m2mDofStartCh(instObj);
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

            /* Fill Sl2 Depth Params with default */
            hObj->sl2Prms.sl2NumLines[DOF_INPUT_REFERENCE_IMG] =
                                                DOF_REF_IMG_SL2_DEPTH;
            hObj->sl2Prms.sl2NumLines[DOF_INPUT_CURRENT_IMG] =
                                                DOF_CURR_IMG_SL2_DEPTH;
            hObj->sl2Prms.sl2NumLines[DOF_INPUT_TEMPORAL_PRED] =
                                                DOF_TEMPORAL_PRED_SL2_DEPTH;
            hObj->sl2Prms.sl2NumLines[DOF_INPUT_PYRAMID_PRED] =
                                                DOF_PYRAM_PRED_SL2_DEPTH;
            hObj->sl2Prms.sl2NumLines[DOF_INPUT_SOF] =
                                                DOF_SOF_SL2_DEPTH;
            hObj->sl2Prms.sl2NumLines[DOF_OUTPUT] =
                                                DOF_OUTPUT_SL2_DEPTH;
            hObj->sl2Prms.sl2NumLines[DOF_INPUT_FOCO_REF_IMG] =
                                                DOF_FOCO_SL2_DEPTH;
            hObj->sl2Prms.sl2NumLines[DOF_INPUT_FOCO_CURR_IMG] =
                                                DOF_FOCO_SL2_DEPTH;


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
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK == status)
            /* LDRA_JUSTIFY_END */
            {
                hObj->doneQ = &hObj->doneQObj;

                /* Allocate Descriptor, Ring memories */
                status = Vhwa_m2mDofAllocUdmaMem(hObj);
            }
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> Rationale: The test framework and test apps cannot reach this portion. Passing only success case to here
            Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK == status)
            /* LDRA_JUSTIFY_END */
            {
                Fvid2Utils_memcpy(&hObj->createArgs, dofCreateArgs,
                    sizeof(Vhwa_M2mDofCreateArgs));

                Fvid2Utils_memcpy(&hObj->cbPrms, cbPrms,
                    sizeof (hObj->cbPrms));

                instObj->openCnt ++;
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> Rationale: The test framework and test apps cannot reach this portion. Passing only success case to here
            Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            else
            {
                /* Some error, so free up the handle object */
                vhwaM2mDofFreeHandleObj(hObj);
            }
            /* LDRA_JUSTIFY_END */
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameter is expected to be pre-validated in the imaging layer before reaching vhwa.
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

Int32 vhwa_m2mDofDelete(Fdrv_Handle handle, Ptr deleteArgs)
{
    int32_t                 status = FVID2_SOK;
    Vhwa_M2mDofInstObj     *instObj = NULL;
    Vhwa_M2mDofHandleObj   *hObj = NULL;
    Dof_SocInfo            *socInfo = NULL;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The function vhwa_m2mDofDelete() is only invoked from the imaging target file after the handle has been pre-validated for NULL.
    This pre-validation is performed in both existing call sites: tivxDmpacDofCreate() and tivxDmpacDofDelete().
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != handle)
    /* LDRA_JUSTIFY_END */
    {
        instObj = &gM2mDofInstObj;
        hObj    = (Vhwa_M2mDofHandleObj *)handle;
        socInfo = &instObj->socInfo;

        /* Take the instance semaphore */
        (void)VhwaAl_SemaphoreP_pend(instObj->lock, SYSTEMP_WAIT_FOREVER);

        /* Check if all handles are already closed */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> Rationale: The negative test framework and test apps cannot reach this portion.
        The application won't reach Delete function without any open handle through the existing test framework
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (0U == instObj->openCnt)
        {
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> Rationale: The negative test framework and test apps cannot reach this portion.
        The application can't be made to call a delete function when there is a pending request through existing test framework.
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
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            vhwaM2mDofFreeHandleObj(hObj);

            instObj->openCnt --;

            /* Delete the Done queue */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> Rationale: The negative test framework and test apps cannot reach this portion.
            The application can't be made to delete a non-empty done queue in the existing test framework.
            Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
            However, due to the stated rationale, this is not tested.
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

Int32 vhwa_m2mDofControl(Fdrv_Handle handle, UInt32 cmd, Ptr cmdArgs,
    Ptr cmdStatusArgs)
{
    int32_t                 status = FVID2_SOK;
    Vhwa_M2mDofInstObj     *instObj = NULL;
    Vhwa_M2mDofHandleObj   *hObj = NULL;
    Vhwa_M2mDofPrms      *dofPrms = NULL;
    Dof_ConfScoreParam     *confScoreCfg = NULL;
    Dof_ErrEventParams     *eePrms = NULL;
    Vhwa_HtsLimiter        *bwLimit = NULL;
    Dof_WdTimerErrEventParams *wdTimerEePrms = NULL;
#if !defined(SOC_J722S)
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
        instObj = &gM2mDofInstObj;
        hObj = (Vhwa_M2mDofHandleObj *)handle;
    }
    /* LDRA_JUSTIFY_START
    <metric start>branch <metric end>
    <justification start> Rationale: The test framework and test apps cannot reach this portion. As handle is not NULL, passing only success case to here
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
        <justification start> Rationale: The test framework and test apps cannot reach this portion. prevalidating in imaging tivxDmpacDofControl function
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        switch (cmd)
        {
        /* LDRA_JUSTIFY_END */
            /* Set DOF Parameters */
            case VHWA_M2M_IOCTL_DOF_SET_PARAMS:
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
                    dofPrms = (Vhwa_M2mDofPrms *)cmdArgs;
                    status = vhwaM2mDofSetParams(instObj, hObj, dofPrms);
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
                    GT_0trace(VhwaDofTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_DOF_SET_PARAMS !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }
            /* Set DOF Parameters */
            case VHWA_M2M_IOCTL_DOF_SET_CONF_SCORE_PARAMS:
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
                    confScoreCfg = (Dof_ConfScoreParam *)cmdArgs;
                    Fvid2Utils_memcpy(&hObj->confScoreCfg, confScoreCfg,
                                    sizeof(Dof_ConfScoreParam));
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
                    GT_0trace(VhwaDofTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_DOF_SET_CONF_SCORE_PARAMS !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            /* Enable Register error events callback */
            case VHWA_M2M_IOCTL_DOF_REGISTER_ERR_CB:
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
                    eePrms = (Dof_ErrEventParams *)cmdArgs;
                    status = vhwaM2mDofSetEeParams(instObj, hObj, eePrms);
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
                    GT_0trace(VhwaDofTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_DOF_REGISTER_ERR_CB !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            /* SET HTS Limiter config */
            case VHWA_M2M_IOCTL_DOF_SET_HTS_LIMIT:
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

                    vhwaM2mDofSetBwLimitParams(hObj, bwLimit);
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
                    GT_0trace(VhwaDofTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_DOF_SET_HTS_LIMIT !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            /* SET Next pyramid level to be processed */
            case VHWA_M2M_IOCTL_DOF_SET_NEXT_PYR:
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
                    uint32_t nextPyrLvl = *((uint32_t *)cmdArgs);
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    The test framework does not support the configuration required to trigger this error scenario.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                        However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if (nextPyrLvl < hObj->dofPrms.tPrmdLvl)
                    /* LDRA_JUSTIFY_END */
                    {
                        hObj->nextPyrLvl = nextPyrLvl;
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
                        GT_0trace(VhwaDofTrace, GT_ERR,
                            "nextPyrLvl exceeds tPrmdLvl !!\n");
                        status = FVID2_EINVALID_PARAMS;
                    }
                    /* LDRA_JUSTIFY_END */
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
                    GT_0trace(VhwaDofTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_DOF_SET_NEXT_PYR !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            /* Get PSA Signature */
            case VHWA_M2M_IOCTL_DOF_GET_PSA_SIGN:
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
                    status = CSL_dofGetPsa(instObj->socInfo.dofRegs, (uint32_t *)cmdArgs);
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement <metric end>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaDofTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_DOF_GET_PSA_SIGN !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            /* Get Histogram */
            case VHWA_M2M_IOCTL_DOF_GET_HISTOGRAM:
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
                    Fvid2Utils_memcpy(csHistogram, hObj->hist, sizeof(uint32_t)*DOF_MAX_HISTOGRAM_SIZE);
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
                    GT_0trace(VhwaDofTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_DOF_GET_HISTOGRAM !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> Rationale: The negative test framework and test apps cannot reach this portion.
            The Ioctl is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: None; Unused feature, cannot be enabled, control cannot reach here.
            <justification end> */
            case VHWA_M2M_IOCTL_DOF_GET_PERFORMANCE:
            {
                uint32_t *perf = NULL;
                if (NULL != cmdArgs)
                {
                    perf = (uint32_t *)cmdArgs;
                    *perf = (uint32_t)hObj->perfNum;
                }
                else
                {
                    GT_0trace(VhwaDofTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_DOF_GET_PERFORMANCE !!\n");
                    status = FVID2_EBADARGS;
                }
                break;
            }
            case VHWA_M2M_IOCTL_DOF_SYNC_START:
            {
                uint32_t *pyrLvl = NULL;
                if (NULL != cmdArgs)
                {
                    pyrLvl = (uint32_t *)cmdArgs;
                    if (NULL != hObj->createArgs.getTimeStamp)
                    {
                        hObj->perfNum = hObj->createArgs.getTimeStamp();
                    }
                    status = CSL_dmpacHtsPipelineStart(instObj->socInfo.htsRegs,
                                                    &hObj->htsCfg[*pyrLvl]);
                }
                else
                {
                    GT_0trace(VhwaDofTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_DOF_SYNC_START !!\n");
                    status = FVID2_EBADARGS;
                }
                break;
            }
            /* LDRA_JUSTIFY_END */

            /* Enable Register Wdtimer error events callback */
            case VHWA_M2M_IOCTL_DOF_REGISTER_WDTIMER_ERR_CB:
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
                    wdTimerEePrms = (Dof_WdTimerErrEventParams *)cmdArgs;
                    status = vhwaM2mDofSetWdTimerEeParams(instObj, hObj, wdTimerEePrms);
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
                    GT_0trace(VhwaDofTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_DOF_REGISTER_WDTIMER_ERR_CB !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }
#if !defined(SOC_J722S)
            case VHWA_M2M_IOCTL_DOF_ENABLE_RECONFIG_REINIT_REG:
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
                        GT_1trace(VhwaDofTrace, GT_ERR,
                            "Invalid safety mode value %u for IOCTL_VHWA_M2M_DOF_ENABLE_RECONFIG_REG !!\n", modeVal);
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
                    GT_0trace(VhwaDofTrace, GT_ERR,
                        "handle/cmdArgs cannot be NULL for IOCTL_VHWA_M2M_DOF_ENABLE_RECONFIG_REG !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case VHWA_M2M_IOCTL_DOF_ENABLE_STATUS_REG_VALIDATE:
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
                        GT_1trace(VhwaDofTrace, GT_ERR,
                            "Invalid safety mode value %u for VHWA_M2M_IOCTL_DOF_ENABLE_STATUS_REG_VALIDATE !!\n", modeVal);
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
                    GT_0trace(VhwaDofTrace, GT_ERR,
                        "handle/cmdArgs cannot be NULL for VHWA_M2M_IOCTL_DOF_ENABLE_STATUS_REG_VALIDATE !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case VHWA_M2M_IOCTL_DOF_VALIDATE_REG:
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
                        status = Vhwa_m2mDofStatusRegValidate(&hObj->statusRegs, &instObj->socInfo);
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
                            GT_0trace(VhwaDofTrace, GT_ERR, "DMPAC-10: StatusRegValidate failed!\n");
                        }
                        /* LDRA_JUSTIFY_END */
                    }
                    if (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate)
                    {
                        /* Readback config registers */
                        status = Vhwa_m2mDofConfigRegReadback(hObj, instObj);
                    }

                    /* Release the hardware lock after the configReg readback or statusReg */
                    if ((VHWA_SAFETY_MODE_DISABLED != hObj->enableStatusRegValidate) ||
                        (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate))
                    {
                        Vhwa_commonHwaLockRelease(VHWA_DMPAC_DOF_LOCK_IDX);
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
                            Vhwa_M2mDofConfigRegMemParams configRegMemPtrs;
                            configRegMemPtrs.golden_ptr = hObj->configRegMemPrms.configGoldenRegPtr;
                            configRegMemPtrs.readback_ptr = hObj->configRegMemPrms.configRegReadbackPtr;
                            configRegMemPtrs.readback_mem_size = sizeof(VhwaDmpacDofSocReadBack);
                            configRegMemPtrs.appData = (void *)hObj->configRegMemPrms.appData;

                            status = hObj->configRegMemPrms.cbFxn((Fvid2_Handle)hObj, &configRegMemPtrs);
                        }
                        else
                        {
                            GT_0trace(VhwaDofTrace, GT_ERR, "ConfigRegMemParams are NULL!\n");
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
                            GT_0trace(VhwaDofTrace, GT_ERR, "DMPAC-7: ConfigRegReadbackCallback Memory Comparison failed!\n");
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
                    GT_0trace(VhwaDofTrace, GT_ERR,
                        "handle cannot be NULL for VHWA_M2M_IOCTL_DOF_VALIDATE_REG !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case VHWA_M2M_IOCTL_DOF_ENABLE_CONFIG_REG_READBACK:
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
                        GT_1trace(VhwaDofTrace, GT_ERR,
                            "Invalid safety mode value %u for VHWA_M2M_IOCTL_DOF_ENABLE_CONFIG_REG_READBACK !!\n", modeVal);
                        status = FVID2_EBADARGS;
                    }
                    else
                    {
                        /* Disable Interrupt since the below flag is accessed in the ISR */
                        cookie = HwiP_disable();
                        hObj->enableConfigRegValidate = modeVal;
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
                    GT_0trace(VhwaDofTrace, GT_ERR,
                        "handle/cmdArgs cannot be NULL for VHWA_M2M_IOCTL_DOF_ENABLE_CONFIG_REG_READBACK !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case VHWA_M2M_IOCTL_DOF_GET_READBACK_SIZE:
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
                    *readbackSize = (uint32_t)sizeof(VhwaDmpacDofSocReadBack);
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
                    GT_0trace(VhwaDofTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_DOF_GET_READBACK_SIZE !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case VHWA_M2M_IOCTL_DOF_SET_CONFIG_REG_PTRS:
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
                    Vhwa_M2mDofConfigRegMemParams *reg_ptrs = (Vhwa_M2mDofConfigRegMemParams *)cmdArgs;
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    This failure case is out of scope for the imaging test framework.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if ((reg_ptrs->readback_ptr == NULL) &&
                        (reg_ptrs->golden_ptr == NULL) &&
                        (reg_ptrs->cbFxn == NULL))
                    {
                        GT_0trace(VhwaDofTrace, GT_ERR,
                            "readback_ptr, golden_ptr, cbFxn cannot be NULL for VHWA_M2M_IOCTL_DOF_SET_REG_READBACK_PTR !!\n");
                        status = FVID2_EBADARGS;
                    }
                    /* LDRA_JUSTIFY_END */
                    else
                    {
                        hObj->configRegMemPrms.configRegReadbackPtr = reg_ptrs->readback_ptr;
                        hObj->configRegMemPrms.configGoldenRegPtr = reg_ptrs->golden_ptr;
                        hObj->configRegMemPrms.cbFxn = reg_ptrs->cbFxn;
                        hObj->configRegMemPrms.appData = reg_ptrs->appData;

                        /* Invoke the API to set the non-zero default values in goldenReg memory */
                        status = vhwaM2mDofSetDefaultGoldenRegMemValues(hObj, instObj);

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
                            GT_0trace(VhwaDofTrace, GT_ERR,
                                "Failed to set default goldenReg mem values for VHWA_M2M_IOCTL_DOF_SET_REG_READBACK_PTR !!\n");
                        }
                        /* LDRA_JUSTIFY_END */
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
                    GT_0trace(VhwaDofTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_DOF_SET_REG_READBACK_PTR !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }
#endif
            /* Default Case */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> Rationale: The test framework and test apps cannot reach this portion. prevalidating in imaging tivxDmpacDofControl function
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

Int32 vhwa_m2mDofProcessReq(Fdrv_Handle handle, Fvid2_FrameList *inFrmList,
    Fvid2_FrameList *outFrmList, uint32_t timeout)
{
    int32_t               status = FVID2_SOK;
    uint32_t              semTimeout;
    Vhwa_M2mDofInstObj   *instObj = NULL;
    Vhwa_M2mDofHandleObj *hObj = NULL;
    Vhwa_M2mDofQueueObj  *qObj = NULL;
    VhwaDmpacDofSocReadBack *goldenRegVal = NULL;

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
        instObj = &gM2mDofInstObj;
        hObj = (Vhwa_M2mDofHandleObj *)handle;

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
            GT_0trace(VhwaDofTrace, GT_ERR,
                "Process request rejected: IP is in stalled state due to previous error\n");
        }
        /* LDRA_JUSTIFY_END */
        goldenRegVal = hObj->configRegMemPrms.configGoldenRegPtr;
    }

    /* Check for null pointers */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> Rationale: The test framework and test apps cannot reach this portion. Passing only success case to here
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Input Buffer Addresses cannot be null */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> Rationale: The test framework and test applications cannot reach this portion. Input buffer addresses are always validated in the target function tivxDmpacDofProcess before reaching this code.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((NULL == inFrmList->frames[0U]) ||
            (0U == inFrmList->frames[0u]->addr[0U]))
        {
            status = FVID2_EBADARGS;
        }
        /* LDRA_JUSTIFY_END */
    }

    /* Input Buffer Addresses cannot be null */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> Rationale: The test framework and test apps cannot reach this portion. Passing only success case to here as the hObj,inFrmList,outFrmList configuring from imaging target function tivxDmpacDofProcess
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
        <justification start> Rationale: The test framework and test apps cannot reach this portion. timeout always FVID2_TIMEOUT_FOREVER, prevallidating from target tivxDmpacDofProcess function
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested..
        <justification end> */
        if (FVID2_TIMEOUT_FOREVER == timeout)
        /* LDRA_JUSTIFY_END */
        {
            semTimeout = SYSTEMP_WAIT_FOREVER;
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> Rationale: The test framework and test apps cannot reach this portion. timeout always FVID2_TIMEOUT_FOREVER, prevallidating from target tivxDmpacDofProcess function
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
        status = Vhwa_commonHwaLockAquire(VHWA_DMPAC_DOF_LOCK_IDX, semTimeout);

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
            qObj = (Vhwa_M2mDofQueueObj *) Fvid2Utils_dequeue(instObj->freeQ);

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
                GT_0trace(VhwaDofTrace, GT_ERR,
                    "Failed to Free Queue Object!!\n");
                status = FVID2_EALLOC;
            }
            else
            /* LDRA_JUSTIFY_END */
            {
                /* Get the current pyramid level */
                hObj->curPyrLvl = hObj->nextPyrLvl;

                qObj->hObj = hObj;
                qObj->pyrLvl = hObj->curPyrLvl;
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
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            /* HW is free, submit request to the hardware */
            /* If previous handle and current handles are same, */
            if (instObj->lastHndlObj != qObj->hObj)
            {
                /** Last handle was not same as new handle,
                 *  so Buffer Parameters needs to be configured */
                 (void)CSL_dofSetBufParam(instObj->socInfo.dofRegs,
                                                &hObj->dofBuffPrams);

                if ((VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate) && (NULL != goldenRegVal))
                {
                    (void)CSL_dofSetBufParam(&(goldenRegVal->dofRegs),
                                                                &hObj->dofBuffPrams);
                }
            }

#if !defined(SOC_J722S)
            /* Invoke the reconfig-MMR if enableReconfigReinitReg enabled for the current handle */
            if (VHWA_SAFETY_MODE_DISABLED != hObj->enableReconfigReinitReg)
            {
                status = Vhwa_m2mDofReconfigReinitReg(instObj, hObj, qObj);
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
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if(FVID2_SOK == status)
            /* LDRA_JUSTIFY_END */
            {
                /* Enable Selected HW Interrupts*/
                status = Vhwa_m2mDofSetIntrInHW(instObj->vhwaIrqNum, instObj->socInfo.dmpacIntdRegs, hObj);
            }

            /** Set the addresses, Configure DOF/HTS, Submit the TR
             *  Start the pipeline */
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
                status = vhwaM2mDofSubmitRequest(instObj, qObj);
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
                GT_0trace(VhwaDofTrace, GT_ERR,
                    "Request cannot be submitted!!\n");
            }
            /* LDRA_JUSTIFY_END */
        }
    }

    return (status);
}

/** \brief Typedef for FVID2 get processed frames function pointer. */
Int32 vhwa_m2mDofGetProcessReq(Fdrv_Handle handle,
    Fvid2_FrameList *inFrmList, Fvid2_FrameList *outFrmList,
    UInt32 timeout)
{
    int32_t                status = FVID2_SOK;
    uint32_t               cookie;
    Vhwa_M2mDofInstObj    *instObj = NULL;
    Vhwa_M2mDofHandleObj  *hObj = NULL;
    Vhwa_M2mDofQueueObj   *qObj = NULL;

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
        instObj = &gM2mDofInstObj;
        hObj = (Vhwa_M2mDofHandleObj *)handle;
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
        qObj = (Vhwa_M2mDofQueueObj *) Fvid2Utils_dequeue(hObj->doneQ);

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
                GT_0trace(VhwaDofTrace, GT_DEBUG,
                    "Nothing to dequeue. No request pending with driver!!\r\n");
                status = FVID2_ENO_MORE_BUFFERS;
            }
            else
            {
                /* If no request have completed, return try again */
                GT_0trace(VhwaDofTrace, GT_DEBUG,
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
#if !defined(MCU_PLUS_SDK)
            status = Vhwa_m2mDofPopRings(instObj, hObj, qObj->pyrLvl);
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
                GT_0trace(VhwaDofTrace, GT_ERR,
                    "Failed to dequeue from Rings!!\n");
            }
            /* LDRA_JUSTIFY_END */
#endif

            /* Return back the queue object to the free queue,
             * No need to protect from ISR, as it is not accessed in ISR */
            Fvid2Utils_queue(instObj->freeQ, &qObj->qElem, qObj);

            status = CSL_dofGetCsHistogram(instObj->socInfo.dofRegs, &hObj->hist[0]);

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
                GT_0trace(VhwaDofTrace, GT_ERR,
                    "Failed to get histogram from HW!!\n");
            }
            /* LDRA_JUSTIFY_END */

            /* Decrement the pending request count. */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> Rationale: The negative test framework and test apps cannot reach this portion.
            Getprocessreq won't be called with any test configuration without a pending request from the handle.
            Effect on this unit: No action taken by application if hobj→numPendReq is already 0.
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
                GT_0trace(VhwaDofTrace, GT_DEBUG,
                    "Done queue of the handle is not empty when there are is no pending request on the handle!!\r\n");
                status = FVID2_EFAIL;
            }
            /* LDRA_JUSTIFY_END */
        }
        /* New request can now be submitted to the DOF IP for non-readBack of StatusRegisters/ConfigurationRegisters enabled frames */
        if ((VHWA_SAFETY_MODE_DISABLED == hObj->enableStatusRegValidate) &&
            (VHWA_SAFETY_MODE_DISABLED == hObj->enableConfigRegValidate))
        {
            Vhwa_commonHwaLockRelease(VHWA_DMPAC_DOF_LOCK_IDX);
        }

        (void)VhwaAl_SemaphoreP_post(instObj->lock);
    }

    return (status);
}

static void Vhwa_m2mDofCalGwPrms(const Dof_Config *dofConfig, uint32_t *refBot,
                                 uint32_t *refTop, uint32_t *curBot,
                                 uint32_t *curTop)
{
    *refTop = dofConfig->topSearchRange + 6u; /* 6 = census tr + hamming dist */
    *refTop = (*refTop + 1u) & 0xFFFFFFFEu;    /* should be multiple of 2 */
    *refBot = dofConfig->bottomSearchRange + 6u + 2u + 1u; /* 6 = census tr + hamming dist; 2=prefetch; 1=adv-pred */
    *refBot = (*refBot + 1u) & 0xFFFFFFFEu;    /* should be multiple of 2 */

    *curTop = 6u; /* 6=census tr + hamming dist */
    *curBot = 8u; /* 6=census tr + hamming dist; 1=adv-pred; 1=round-even */
}

/* ========================================================================== */
/*                           Local Functions                                  */
/* ========================================================================== */

/* Function to Allocate Handle Object */
static Vhwa_M2mDofHandleObj *vhwaM2mDofAllocHandleObj(
    const Vhwa_M2mDofInstObj *instObj)
{
    uint32_t cnt;
    Vhwa_M2mDofHandleObj *hObj = NULL;
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> Rationale: The test framework and test apps cannot reach this portion. instObj can't be NULL, prevalidating in vhwa_m2mDofCreate
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
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
        for (cnt = 0U; cnt < VHWA_M2M_DOF_MAX_HANDLES; cnt ++)
        /* LDRA_JUSTIFY_END */
        {
            if ((uint32_t)UFALSE == gM2mDofHandleObj[cnt].isUsed)
            {
                hObj = &gM2mDofHandleObj[cnt];
                Fvid2Utils_memset(hObj, 0x0, sizeof(Vhwa_M2mDofHandleObj));
                gM2mDofHandleObj[cnt].isUsed = (uint32_t)UTRUE;
                gM2mDofHandleObj[cnt].enableReconfigReinitReg = (uint32_t)VHWA_SAFETY_MODE_DISABLED;
                gM2mDofHandleObj[cnt].enableStatusRegValidate = (uint32_t)VHWA_SAFETY_MODE_DISABLED;
                hObj->hIdx = cnt;
                break;
            }
        }
    }
    return (hObj);
}

/* Function to freeup allocated Handle Object */
static void vhwaM2mDofFreeHandleObj(Vhwa_M2mDofHandleObj *hObj)
{
    uint32_t cnt;
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> Rationale: The test framework and test apps cannot reach this portion. hObj can't be NULL, prevalidating in vhwa_m2mDofCreate
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
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
        for (cnt = 0U; cnt < VHWA_M2M_DOF_MAX_HANDLES; cnt ++)
        /* LDRA_JUSTIFY_END */
        {
            if (hObj == &gM2mDofHandleObj[cnt])
            {
                hObj->isUsed = (uint32_t)UFALSE;;
                break;
            }
        }
    }
}


static int32_t vhwaM2mDofCheckDofCfg(const Vhwa_M2mDofInstObj *instObj,
    Vhwa_M2mDofHandleObj *hObj, Vhwa_M2mDofPrms *dofPrms)
{
    int32_t     status = FVID2_SOK;
    uint32_t    pyrLvl, tHeight, tWidth;
    Dof_Config  *dofCfg;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> Rationale: The test framework and test apps cannot reach this portion. instObj, hObj and dofPrms are pre-validated from vhwaM2mDofSetParams
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL != instObj) && (NULL != hObj) && (NULL != dofPrms))
    /* LDRA_JUSTIFY_END */
    {
        dofCfg = &dofPrms->coreCfg;
        for(pyrLvl = 0u; pyrLvl < dofPrms->tPrmdLvl; pyrLvl++)
        {
            if(dofPrms->inOutImgFmt[pyrLvl][DOF_INPUT_CURRENT_IMG].ccsFormat ==
                                                    FVID2_CCSF_BITS12_PACKED)
            {
                hObj->isFocoUsed[pyrLvl] = UFALSE;
            }
            else if((dofPrms->inOutImgFmt[pyrLvl][DOF_INPUT_CURRENT_IMG].ccsFormat
                                    == FVID2_CCSF_BITS12_UNPACKED16) ||
                    (dofPrms->inOutImgFmt[pyrLvl][DOF_INPUT_CURRENT_IMG].ccsFormat
                                    == FVID2_CCSF_BITS8_PACKED) ||
                    (dofPrms->inOutImgFmt[pyrLvl][DOF_INPUT_CURRENT_IMG].ccsFormat
                                    == FVID2_CCSF_BITS12_UNPACKED16_MSB_ALIGNED))
            {
                hObj->isFocoUsed[pyrLvl] = UTRUE;
            }
            else
            {
                hObj->isFocoUsed[pyrLvl] = UFALSE;
                status = FVID2_EINVALID_PARAMS;
            }
        }

        if((DOF_PREDICTOR_TEMPORAL == dofPrms->tPredictor) ||
           (DOF_PREDICTOR_PYR_LEFT == dofPrms->tPredictor) ||
           (DOF_PREDICTOR_PYR_COLOCATED == dofPrms->tPredictor) ||
           (DOF_PREDICTOR_TEMPORAL == dofPrms->mPredictor1) ||
           (DOF_PREDICTOR_TEMPORAL == dofPrms->mPredictor2))
        {
            status = FVID2_EINVALID_PARAMS;
        }

        if(dofPrms->flowVectorHeight > dofCfg->height)
        {
            status = FVID2_EINVALID_PARAMS;
        }

        if((dofCfg->height > DOF_MAXIMUM_HEIGHT) ||
           (dofCfg->height < DOF_MINIMUM_HEIGHT) ||
           (dofCfg->width > DOF_MAXIMUM_WIDTH) ||
           (dofCfg->width < DOF_MINIMUM_WIDTH))
        {
            status = FVID2_EINVALID_PARAMS;
        }

        pyrLvl = dofPrms->tPrmdLvl;

        tWidth = dofCfg->width;
        tHeight = dofCfg->height;

        while(pyrLvl-- > 0u)
        {
            if(((tHeight & 0x1u) != 0U) ||
            ((tWidth & 0x1u) != 0U))
            {
                status = FVID2_EINVALID_PARAMS;
            }

            tWidth = tWidth/2u;
            tHeight = tHeight/2u;
        }

        if((dofCfg->horizontalSearchRange > DOF_MAXIMUM_HSR) ||
           (dofCfg->topSearchRange > DOF_MAXIMUM_VSR) ||
           (dofCfg->bottomSearchRange > DOF_MAXIMUM_VSR))
        {
            status = FVID2_EINVALID_PARAMS;
        }
        if((DOF_MAXIMUM_TVSR_WTIH_191_HSR <
           (dofCfg->topSearchRange + dofCfg->bottomSearchRange)) &&
           ((dofCfg->horizontalSearchRange > DOF_MAXIMUM_HSR_WTIH_124_VSR)))
        {
            status = FVID2_EINVALID_PARAMS;
        }
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> Rationale: The test framework and test apps cannot reach this portion. instObj, hObj and dofPrms are pre-validated from vhwaM2mDofSetParams
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

static int32_t vhwaM2mDofSetParams(Vhwa_M2mDofInstObj *instObj,
    Vhwa_M2mDofHandleObj *hObj, Vhwa_M2mDofPrms *dofPrms)
{
    int32_t status = FVID2_EBADARGS;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> Rationale: The test framework and test apps cannot reach this portion. instObj, hObj and dofPrms are pre-validated from vhwa_m2mDofControl
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL != instObj) && (NULL != hObj) && (NULL != dofPrms))
    /* LDRA_JUSTIFY_END */
    {
        status = vhwaM2mDofCheckDofCfg(instObj, hObj, dofPrms);

        /* DOF configuration is verified */
        if (FVID2_SOK == status)
        {
            status = vhwaM2mDofSetSl2Params(instObj, hObj, dofPrms);

            if(FVID2_SOK == status)
            {
                Fvid2Utils_memcpy(&hObj->dofPrms, dofPrms,
                                    sizeof(Vhwa_M2mDofPrms));

                /* Calculate the pyramidal paramters from the input
                    configuration */
                vhwaM2mDofCalPyrmPrms(hObj);


                /* Set Sl2 buffer parameters for DOF */
                vhwaM2mDofSetBuffParams(hObj, &hObj->dofPrms);

                /* Reset HTS config based on local correct dof config */
                vhwaM2mDofSetHtsCfg(instObj, hObj, &hObj->dofPrms);

                /* Reset LSE config based on local correct dof config - Required
                   only in case of FOCO */
                vhwaM2mDofSetLseCfg(hObj, &hObj->dofPrms);

                vhwaM2mDofSetFocoParams(hObj, &hObj->dofPrms);

                /* Setup TR Descriptor */
                Vhwa_m2mDofSetTrDesc(instObj, hObj);
            }
        }

        if(FVID2_SOK != status)
        {
            /* Reset HTS config based on local correct DOF config */
            vhwaM2mDofSetHtsCfg(instObj, hObj, &hObj->dofPrms);
            /* Reset LSE config based on local correct DOF config */
            vhwaM2mDofSetLseCfg(hObj, &hObj->dofPrms);
        }
    }

    return (status);
}

static int32_t vhwaM2mDofSetEeParams(const Vhwa_M2mDofInstObj *instObj,
    Vhwa_M2mDofHandleObj *hObj, const Dof_ErrEventParams *eePrms)
{
    int32_t status = FVID2_SOK;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> Rationale: The test framework and test apps cannot reach this portion. instObj, hObj and eePrms are pre-validated from vhwa_m2mDofControl
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
            GT_0trace(VhwaDofTrace, GT_ERR,
                "Cannot change error events while handle is active !!\n");
            status = FVID2_EAGAIN;
        }
        else if (0U != (eePrms->errEvents & (~VHWA_DOF_ALL_ERR_EVENTS)))
        {
            GT_1trace(VhwaDofTrace, GT_ERR,
                "Attempt to enable a non-existent error event: 0x%08x !!\n", (eePrms->errEvents & (~VHWA_DOF_ALL_ERR_EVENTS)));
            status = FVID2_EINVALID_PARAMS;
        }
        /* LDRA_JUSTIFY_END */
        else
        {
            Fvid2Utils_memcpy(&hObj->eePrms, eePrms, sizeof(Dof_ErrEventParams));
            status = FVID2_SOK;
        }
    }
    else
    {
        status = FVID2_EBADARGS;
    }

    return (status);
}

static int32_t vhwaM2mDofSetWdTimerEeParams(const Vhwa_M2mDofInstObj *instObj,
    Vhwa_M2mDofHandleObj *hObj, const Dof_WdTimerErrEventParams *wdTimerEePrms)
{
    int32_t status = FVID2_SOK;
    uint32_t pyrLvl;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> Rationale: The test framework and test apps cannot reach this portion. instObj, hObj and wdTimerEePrms are pre-validated from vhwa_m2mDofControl
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL != instObj) && (NULL != hObj) && (NULL != wdTimerEePrms))
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
            GT_0trace(VhwaDofTrace, GT_ERR,
                "Cannot change watchdog timer error events while handle is active !!\n");
            status = FVID2_EAGAIN;
        }
        else if (0U != (wdTimerEePrms->wdTimerErrEvents & (~VHWA_DOF_ALL_WDTIMER_ERR_EVENTS)))
        {
            GT_1trace(VhwaDofTrace, GT_ERR,
                "Attempt to enable a non-existent watchdog timer error event: 0x%08x !!\n",
                (wdTimerEePrms->wdTimerErrEvents & (~VHWA_DOF_ALL_WDTIMER_ERR_EVENTS)));
            status = FVID2_EINVALID_PARAMS;
        }
        /* LDRA_JUSTIFY_END */
        else
        {
            Fvid2Utils_memcpy(&hObj->wdTimerEePrms, wdTimerEePrms, sizeof(Dof_WdTimerErrEventParams));

            /* Update all HTS configurations (DOF has multiple pyramid levels) */
            /* Configure DOF and FOCO WDTimer for all pyramid levels */
            for (pyrLvl = 0u; pyrLvl < DOF_MAX_PYR_LVL_SUPPORTED; pyrLvl++)
            {
                /* Configure DOF WDTimer - enable if WDTIMER_ERR event is registered */
                if (0U != (wdTimerEePrms->wdTimerErrEvents & VHWA_DOF_WDTIMER_ERR))
                {
                    hObj->htsCfg[pyrLvl].enableWdTimer = (uint32_t)UTRUE;
                    if (VHWA_WDTIMEOUT_64K_CYCLES == wdTimerEePrms->wdTimeoutCycles)
                    {
                        hObj->htsCfg[pyrLvl].isWdTimer128KMode = (uint32_t)UFALSE;
                    }
                    else
                    {
                        hObj->htsCfg[pyrLvl].isWdTimer128KMode = (uint32_t)UTRUE;
                    }
                }
                else
                {
                    hObj->htsCfg[pyrLvl].enableWdTimer = (uint32_t)UFALSE;
                }

                /* Configure FOCO WDTimer - enable if FOCO WDTIMER_ERR event is registered */
                /* Only configure FOCO watchdog timer if FOCO is used for this pyramid level */
                if (UTRUE == hObj->isFocoUsed[pyrLvl])
                {
                    if (0U != (wdTimerEePrms->wdTimerErrEvents & VHWA_DOF_FOCO0_WDTIMER_ERR))
                    {
                        hObj->focoHtsCfg[pyrLvl].enableWdTimer = (uint32_t)UTRUE;
                        if (VHWA_WDTIMEOUT_64K_CYCLES == wdTimerEePrms->focoWdTimeoutCycles)
                        {
                            hObj->focoHtsCfg[pyrLvl].isWdTimer128KMode = (uint32_t)UFALSE;
                        }
                        else
                        {
                            hObj->focoHtsCfg[pyrLvl].isWdTimer128KMode = (uint32_t)UTRUE;
                        }
                    }
                    else
                    {
                        hObj->focoHtsCfg[pyrLvl].enableWdTimer = (uint32_t)UFALSE;
                    }
                }
            }
        }
    }
    else
    {
        status = FVID2_EBADARGS;
    }

    return (status);
}

static void vhwaM2mDofSetFocoParams(Vhwa_M2mDofHandleObj *hObj,
                                       Vhwa_M2mDofPrms *dofPrms)
{
    Dmpac_FocoConfig *focoRefCfg = NULL;
    Dmpac_FocoConfig *focoCurrCfg = NULL;
    Dof_Config       *dofCfg = NULL;
    Vhwa_FocoPrms    *focoPrms = NULL;
    uint32_t         refBot, refTop, curBot, curTop;
    uint32_t         pyrLvl;

    /* Null pointer check */
    GT_assert(VhwaDofTrace, (NULL != hObj));
    GT_assert(VhwaDofTrace, (NULL != dofPrms));

    focoPrms = &dofPrms->focoPrms;

    for(pyrLvl = 0; pyrLvl < dofPrms->tPrmdLvl; pyrLvl++)
    {
        if(UTRUE == hObj->isFocoUsed[pyrLvl])
        {
            focoRefCfg = &hObj->focoCfg[pyrLvl][DOF_INPUT_REFERENCE_IMG];
            focoCurrCfg = &hObj->focoCfg[pyrLvl][DOF_INPUT_CURRENT_IMG];
            dofCfg = &hObj->dofCfg[pyrLvl];

            Vhwa_m2mDofCalGwPrms(dofCfg, &refBot, &refTop, &curBot, &curTop);

            if(focoPrms->shiftM1 != 0u)
            {
                focoRefCfg->shiftEnable = UTRUE;
                focoCurrCfg->shiftEnable = UTRUE;

                focoRefCfg->shiftM1 = focoPrms->shiftM1 - 1U;
                focoCurrCfg->shiftM1 = focoPrms->shiftM1 - 1U;
            }
            else
            {
                focoRefCfg->shiftEnable = UFALSE;
                focoCurrCfg->shiftEnable = UFALSE;

                focoRefCfg->shiftM1 = 0U;
                focoCurrCfg->shiftM1 = 0U;
            }
            focoRefCfg->dir = focoPrms->dir;
            focoRefCfg->round = focoPrms->round;
            focoRefCfg->channelEnable = UTRUE;
            focoRefCfg->mask = 0xffffU;
            if(((refTop + refBot + 2U) > dofCfg->height) &&
                    ((refBot + 2U) >= dofCfg->height))
            {
                focoRefCfg->trig = ((2U * dofCfg->height) - 2U - curBot);
                focoCurrCfg->preload = dofCfg->height - 2U - curBot;
            }
            else
            {
                focoRefCfg->trig = dofCfg->height + (refBot - curBot);
                focoCurrCfg->preload = refBot - curBot;
            }

            focoCurrCfg->dir = focoPrms->dir;
            focoCurrCfg->round = focoPrms->round;
            focoCurrCfg->channelEnable = UTRUE;
            focoCurrCfg->mask = 0xffff;
            focoCurrCfg->trig = dofCfg->height;

            focoRefCfg->preload = 0u;
            focoRefCfg->postload = 0u;
            focoCurrCfg->postload = 0u;
        }
        #ifdef TEST_DOF_FOCO_WDTIMER_ERR_INTERRUPT
        focoCurrCfg->channelEnable = UTRUE;
        focoCurrCfg->trig = 0;
        focoRefCfg->channelEnable = UTRUE;
        focoRefCfg->trig = 0;
        #endif
    }

}

static void vhwaM2mDofCalPyrmPrms(Vhwa_M2mDofHandleObj *hObj)
{
    uint32_t pyrLvl, cnt;
    Dof_Config *dofCfg;

    GT_assert(VhwaDofTrace, (NULL != hObj));

    for (pyrLvl = 0u; pyrLvl < DOF_MAX_PYR_LVL_SUPPORTED; pyrLvl++)
    {
        for (cnt = 0u; cnt < VHWA_M2M_DOF_MAX_DMA_CH; cnt ++)
        {
            hObj->chPrms[pyrLvl][cnt].isEnabled = UFALSE;
        }
    }

    for (pyrLvl = 0u; pyrLvl < hObj->dofPrms.tPrmdLvl; pyrLvl++)
    {
        dofCfg = &hObj->dofCfg[pyrLvl];

        Fvid2Utils_memcpy(dofCfg, &hObj->dofPrms.coreCfg,
                        sizeof(Dof_Config));

        dofCfg->pyramidalTopColPred = UFALSE;
        dofCfg->pyramidalTopLeftPred = UFALSE;
        dofCfg->temporalPred = UFALSE;
        dofCfg->delayedLeftPred = UFALSE;

        if(pyrLvl == 0u)
        {
            /* Base Layer */
            if((hObj->dofPrms.bPredictor1 == DOF_PREDICTOR_TEMPORAL) ||
               (hObj->dofPrms.bPredictor2 == DOF_PREDICTOR_TEMPORAL))
            {
                dofCfg->temporalPred = UTRUE;
            }
            if((hObj->dofPrms.bPredictor1 == DOF_PREDICTOR_PYR_COLOCATED) ||
               (hObj->dofPrms.bPredictor2 == DOF_PREDICTOR_PYR_COLOCATED))
            {
                dofCfg->pyramidalTopColPred = UTRUE;
            }
            if((hObj->dofPrms.bPredictor1 == DOF_PREDICTOR_PYR_LEFT) ||
               (hObj->dofPrms.bPredictor2 == DOF_PREDICTOR_PYR_LEFT))
            {
                dofCfg->pyramidalTopLeftPred = UTRUE;
            }
            if((hObj->dofPrms.bPredictor1 == DOF_PREDICTOR_DELEY_LEFT) ||
               (hObj->dofPrms.bPredictor2 == DOF_PREDICTOR_DELEY_LEFT))
            {
                dofCfg->delayedLeftPred      = UTRUE;
            }
            if(dofCfg->temporalPred == (uint32_t)UTRUE)
            {
                hObj->isTempPred = UTRUE;
                hObj->chPrms[pyrLvl][DOF_INPUT_TEMPORAL_PRED].isEnabled = UTRUE;
            }
            else
            {
                hObj->isTempPred = UFALSE;
            }
            if(dofCfg->enableSof == (uint32_t)UTRUE)
            {
                hObj->isSof = UTRUE;
                hObj->chPrms[pyrLvl][DOF_INPUT_SOF].isEnabled = UTRUE;
            }
            else
            {
                hObj->isSof = UFALSE;
            }
        }
        else if(pyrLvl == (hObj->dofPrms.tPrmdLvl - 1u))
        {
            /* Top Layer */
            if(hObj->dofPrms.tPredictor == DOF_PREDICTOR_DELEY_LEFT)
            {
                dofCfg->delayedLeftPred = UTRUE;
            }

            dofCfg->lkConfidanceScore = UFALSE;
            dofCfg->enableSof = UFALSE;
        }
        else
        {
            /* Middle Pyramid layers */
            if((hObj->dofPrms.mPredictor1 == DOF_PREDICTOR_PYR_COLOCATED) ||
               (hObj->dofPrms.mPredictor2 == DOF_PREDICTOR_PYR_COLOCATED))
            {
                dofCfg->pyramidalTopColPred = UTRUE;
            }
            if((hObj->dofPrms.mPredictor1 == DOF_PREDICTOR_PYR_LEFT) ||
               (hObj->dofPrms.mPredictor2 == DOF_PREDICTOR_PYR_LEFT))
            {
                dofCfg->pyramidalTopLeftPred = UTRUE;
            }
            if((hObj->dofPrms.mPredictor1 == DOF_PREDICTOR_DELEY_LEFT) ||
               (hObj->dofPrms.mPredictor2 == DOF_PREDICTOR_DELEY_LEFT))
            {
                dofCfg->delayedLeftPred      = UTRUE;
            }
            dofCfg->lkConfidanceScore = UFALSE;
            dofCfg->enableSof = UFALSE;
        }

        if (((uint32_t)UTRUE == dofCfg->pyramidalTopColPred) ||
            ((uint32_t)UTRUE == dofCfg->pyramidalTopLeftPred))
        {
            hObj->isPyrPred[pyrLvl] = UTRUE;
            hObj->chPrms[pyrLvl][DOF_INPUT_PYRAMID_PRED].isEnabled = UTRUE;
        }
        else
        {
            hObj->isPyrPred[pyrLvl] = UFALSE;
        }

        cnt = pyrLvl;

        while(cnt-- > 0u)
        {
            dofCfg->width   = dofCfg->width/2u;
            dofCfg->height  = dofCfg->height/2u;
        }

        hObj->chPrms[pyrLvl][DOF_OUTPUT].isEnabled = UTRUE;

        if(UTRUE == hObj->isFocoUsed[pyrLvl])
        {
            hObj->chPrms[pyrLvl][DOF_INPUT_FOCO_REF_IMG].isEnabled = UTRUE;
            hObj->chPrms[pyrLvl][DOF_INPUT_FOCO_CURR_IMG].isEnabled = UTRUE;
        }
        else
        {
            hObj->chPrms[pyrLvl][DOF_INPUT_REFERENCE_IMG].isEnabled = UTRUE;
            hObj->chPrms[pyrLvl][DOF_INPUT_CURRENT_IMG].isEnabled = UTRUE;
        }
    }
}

static int32_t vhwaM2mDofEnableFoco(const Vhwa_M2mDofInstObj *instObj,
    const Vhwa_M2mDofHandleObj *hObj)
{
    int32_t status = FVID2_SOK;

    /* Null pointer check */
    GT_assert(VhwaDofTrace, (NULL != instObj));
    GT_assert(VhwaDofTrace, (NULL != hObj));

    CSL_dmpac_foco_coreRegs *focoRegs = instObj->socInfo.dmpacFocoRegs;

    VhwaDmpacDofSocReadBack *goldenRegVal = hObj->configRegMemPrms.configGoldenRegPtr;

    /* Enable channel 0 for Reference Frame */
    status = CSL_dmpacFocoSetConfig(focoRegs, DMPAC_FOCO_CHANNEL_0,
                    &hObj->focoCfg[hObj->curPyrLvl][DOF_INPUT_REFERENCE_IMG]);

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
                    &hObj->focoCfg[hObj->curPyrLvl][DOF_INPUT_REFERENCE_IMG]);
    }
    /* Enable channel 1 for Current Frame */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(FVID2_SOK == status)
    {
        status = CSL_dmpacFocoSetConfig(focoRegs, DMPAC_FOCO_CHANNEL_1,
                    &hObj->focoCfg[hObj->curPyrLvl][DOF_INPUT_CURRENT_IMG]);
    }
    if (((FVID2_SOK == status) && (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate)) && (NULL != goldenRegVal))
    {
        status = CSL_dmpacFocoSetConfig(&(goldenRegVal->dmpacFocoRegs), DMPAC_FOCO_CHANNEL_1,
                    &hObj->focoCfg[hObj->curPyrLvl][DOF_INPUT_CURRENT_IMG]);
    }
    /* LDRA_JUSTIFY_END */
    return status;
}

static int32_t vhwaM2mDofSubmitRequest(Vhwa_M2mDofInstObj *instObj,
    Vhwa_M2mDofQueueObj *qObj)
{
    int32_t               status = FVID2_SOK;
    uint32_t              pyrLvl;
    Dof_SocInfo          *socInfo = NULL;
    Vhwa_M2mDofHandleObj *hObj = NULL;

    /* Null pointer check */
    GT_assert(VhwaDofTrace, (NULL != instObj));
    GT_assert(VhwaDofTrace, (NULL != qObj));
    GT_assert(VhwaDofTrace, (NULL != qObj->hObj));

    socInfo = &instObj->socInfo;
    hObj = qObj->hObj;
    pyrLvl = hObj->curPyrLvl;

    VhwaDmpacDofSocReadBack *goldenRegVal = hObj->configRegMemPrms.configGoldenRegPtr;

    /* Configure DOF Confidence score parameters */
    (void)CSL_dofSetConfScoreParam(instObj->socInfo.dofRegs, &hObj->confScoreCfg);

    /* If configRegValidate is enabled, also set the golden register values */
    if ((VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate) && (NULL != goldenRegVal))
    {
        (void)CSL_dofSetConfScoreParam(&(goldenRegVal->dofRegs), &hObj->confScoreCfg);
    }

    /* Configure DOF Core */
    status = CSL_dofSetConfig(socInfo->dofRegs, &hObj->dofCfg[pyrLvl]);
    /* If configRegValidate is enabled, also set the golden register values */
    if (((FVID2_SOK == status) && (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate)) && (NULL != goldenRegVal))
    {
        (void)CSL_dofSetConfig(&(goldenRegVal->dofRegs), &hObj->dofCfg[pyrLvl]);
    }

    CSL_dofEnablePsa(socInfo->dofRegs, hObj->createArgs.enablePsa);

    /* If configRegValidate is enabled, also set the golden register values */
    if ((VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate) && (NULL != goldenRegVal))
    {
        CSL_dofEnablePsa(&(goldenRegVal->dofRegs), hObj->createArgs.enablePsa);
    }

    /* If foco is used set foco paramenters */
    if(hObj->isFocoUsed[pyrLvl] == UTRUE)
    {
        if (FVID2_SOK == status)
        {
            /* Step-2, Configure HWA Common Wrapper LSE */
            status = vhwaM2mDofEnableFoco(instObj, hObj);
        }

        if (FVID2_SOK == status)
        {
            /* Step-2, Configure HWA Common Wrapper LSE */
            status = CSL_lseSetConfig(socInfo->lseRegs, &hObj->lseCfg[pyrLvl]);
        }

        if ((FVID2_SOK == status) && ((VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate)))
        {
            /* Update the golden LSE config values for readback */
            status = CSL_lseSetConfig(&(goldenRegVal->lseRegs), &hObj->lseCfg[pyrLvl]);
        }
    }

    if (FVID2_SOK == status)
    {
        /* Configure HTS */
        status = CSL_dmpacHtsSetThreadConfig(socInfo->htsRegs, &hObj->htsCfg[pyrLvl]);
    }

    if((hObj->isFocoUsed[pyrLvl] == UTRUE) && (FVID2_SOK == status))
    {
        status = CSL_dmpacHtsSetThreadConfig(socInfo->htsRegs,
                                        &hObj->focoHtsCfg[pyrLvl]);
    }

    if (FVID2_SOK == status)
    {
        /* Start HTS Scheduler */
        status = CSL_dmpacHtsThreadStart(socInfo->htsRegs, &hObj->htsCfg[pyrLvl]);

        if((hObj->isFocoUsed[pyrLvl] == UTRUE) && (FVID2_SOK == status))
        {
            status = CSL_dmpacHtsThreadStart(socInfo->htsRegs, &hObj->focoHtsCfg[pyrLvl]);
        }
        else
        {
            (void)CSL_dmpacHtsThreadStopAll(socInfo->htsRegs, &hObj->focoHtsCfg[pyrLvl]);
        }

        /* Update the DofStatusRegisterGroup with valid status register values for frame specific registers */
#if !defined(SOC_J722S)
        if(VHWA_SAFETY_MODE_DISABLED != hObj->enableStatusRegValidate)
        {
            status = vhwaM2mDofUpdateStatusRegGroup(hObj);
        }
#endif
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        {
            /* Set Input and output addresses */
            vhwaM2mDofSetTRAddress(hObj, &qObj->inFrmList,
                        qObj->outFrmList.frames[0U]->addr[0]);

            /* Submit Rings to the Ring Accelerator */
            status = Vhwa_m2mDofSubmitRing(instObj, hObj);
        }
        /* LDRA_JUSTIFY_END */
    }

#if !defined(SOC_J722S)
    /* Update the DofConfigRegisterGroup with config register values for frame specific Static Config, INTD and HTS registers */
    if(VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate)
    {
        status = vhwaM2mDofUpdateConfigRegGroup(goldenRegVal, instObj, hObj);
    }
#endif
    if (FVID2_SOK == status)
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
        status = CSL_dmpacHtsPipelineStart(socInfo->htsRegs, &hObj->htsCfg[pyrLvl]);
        #ifdef TEST_DOF_WDTIMER_ERR_INTERRUPT
        uint32_t delay_cnt = 0;
        volatile uint32_t   *dofCtrlReg = (volatile uint32_t *)(CSL_DMPAC0_PAR_DOF_CFG_VP_MMR_VBUSP_DOFCORE_BASE + 0x4);
        volatile uint32_t   *dofStatReg = (volatile uint32_t *)(CSL_DMPAC0_PAR_DOF_CFG_VP_MMR_VBUSP_DOFCORE_BASE + 0x8);
        if(0 == pyrLvl)
        {
            while(socInfo->htsRegs->PIPELINE_CONTROL[(uint32_t)hObj->htsCfg[pyrLvl].pipeline] & 0x1u)
            {
                delay_cnt++;
                if((delay_cnt > 2000) && (((CSL_REG32_RD(dofStatReg)) & 0x1U) == 1u))
                {
                    uint32_t regVal = CSL_REG32_RD(dofCtrlReg);
                    regVal &= ~0x1U;
                    CSL_REG32_WR(dofCtrlReg, regVal);
                    break;
                }
            }
        }
        #endif
        #endif
    }

    if (FVID2_SOK != status)
    {
        instObj->actQObj = NULL;
        status = FVID2_EFAIL;
    }
    else
    {
        status = FVID2_SOK;
    }

    return (status);
}

static void vhwaM2mDofSetLseCfg(Vhwa_M2mDofHandleObj *hObj,
                                const Vhwa_M2mDofPrms *dofPrms)
{
    Fvid2_ColorCompStorageFmt ccsFormat;
    Vhwa_M2mDofSl2Prms *sl2Prms = NULL;
    CSL_LseConfig *lseCfg = NULL;
    Dof_Config    *coreCfg = NULL;
    uint32_t      refBot, refTop, curBot, curTop;
    uint32_t      lineOffset, pyrLvl;

    GT_assert(VhwaDofTrace, (NULL != hObj));
    GT_assert(VhwaDofTrace, (NULL != dofPrms));

    for(pyrLvl = 0; pyrLvl < dofPrms->tPrmdLvl; pyrLvl++)
    {
        if(UTRUE == hObj->isFocoUsed[pyrLvl])
        {
            lseCfg = &hObj->lseCfg[pyrLvl];
            sl2Prms = &hObj->sl2Prms;
            ccsFormat = (Fvid2_ColorCompStorageFmt)dofPrms->inOutImgFmt[pyrLvl]
                                    [DOF_INPUT_CURRENT_IMG].ccsFormat;
            coreCfg = &hObj->dofCfg[pyrLvl];

            Vhwa_m2mDofCalGwPrms(coreCfg, &refBot, &refTop, &curBot, &curTop);

            /* Initialize LSE configuration with the default values */
            CSL_lseConfigInit(lseCfg);

            lineOffset = Vhwa_calcHorzSizeInBytes(coreCfg->width, ccsFormat);

            lineOffset = (lineOffset + 63u) & 0xFFFFFFC0u;

            lseCfg->numInCh = 1;
            lseCfg->numOutCh = 2;

            lseCfg->enableFixedArbMode = 1;

            /* Reference image */
            lseCfg->inChCfg[0U].enable = (uint32_t)UTRUE;
            lseCfg->inChCfg[0U].frameWidth = coreCfg->width;

            if(((refTop + refBot + 2u) > coreCfg->height) &&
                    ((refBot + 2u) >= coreCfg->height))
            {
                lseCfg->inChCfg[0U].frameHeight =
                                    ((2u * coreCfg->height) - 2u - curBot);
            }
            else
            {
                lseCfg->inChCfg[0U].frameHeight =
                                    coreCfg->height + (refBot - curBot);
            }

            lseCfg->inChCfg[0U].enableAddrIncrBy2 = (uint32_t)UFALSE;

            lseCfg->inChCfg[0U].ccsf = ccsFormat;
            lseCfg->inChCfg[0U].startOffset = 0U;

            lseCfg->inChCfg[0U].lineOffset = lineOffset;
            lseCfg->inChCfg[0U].circBufSize = DOF_FOCO_SL2_DEPTH;
            lseCfg->inChCfg[0U].bufAddr[0U] =
                        (uint32_t)sl2Prms->sl2Addr[DOF_INPUT_FOCO_REF_IMG];
            lseCfg->inChCfg[0U].bufAddr[1U] =
                        (uint32_t)sl2Prms->sl2Addr[DOF_INPUT_FOCO_CURR_IMG];
            lseCfg->inChCfg[0U].numInBuff = 2U;

            lseCfg->inChCfg[0U].enableMultiTapKnlConfig = UFALSE;

            lseCfg->outChCfg[0U].enable = (uint32_t)UTRUE;
            lseCfg->outChCfg[0U].ccsf = FVID2_CCSF_BITS12_PACKED;
            lseCfg->outChCfg[0U].lineOffset = sl2Prms->sl2RefCurPitch;
            lseCfg->outChCfg[0U].circBufSize = refTop + refBot + 2u +
                                sl2Prms->sl2NumLines[DOF_INPUT_CURRENT_IMG];
            lseCfg->outChCfg[0U].bufAddr =
                        (uint32_t)sl2Prms->sl2Addr[DOF_INPUT_REFERENCE_IMG];

            /* Current image */
            lseCfg->outChCfg[1U].enable = (uint32_t)UTRUE;
            lseCfg->outChCfg[1U].ccsf = FVID2_CCSF_BITS12_PACKED;
            lseCfg->outChCfg[1U].lineOffset = sl2Prms->sl2RefCurPitch;
            lseCfg->outChCfg[1U].circBufSize = curTop + curBot + 2u +
                                sl2Prms->sl2NumLines[DOF_INPUT_CURRENT_IMG];
            lseCfg->outChCfg[1U].bufAddr =
                            (uint32_t)sl2Prms->sl2Addr[DOF_INPUT_CURRENT_IMG];
        }
    }
}

static void vhwaM2mDofSetHtsCfg(Vhwa_M2mDofInstObj *instObj,
    Vhwa_M2mDofHandleObj *hObj, const Vhwa_M2mDofPrms *dofPrms)
{
    uint32_t pyrLvl;
    CSL_DmpacHtsSchConfig *htsCfg = NULL;
    CSL_DmpacHtsSchConfig *focoHtsCfg = NULL;
    Dof_Config *dofConfig = NULL;
    uint32_t   refTop,refBot,curTop,curBot;

    GT_assert(VhwaDofTrace, (NULL != instObj));
    GT_assert(VhwaDofTrace, (NULL != hObj));
    GT_assert(VhwaDofTrace, (NULL != dofPrms));

    for(pyrLvl = 0u; pyrLvl < dofPrms->tPrmdLvl; pyrLvl++)
    {
        htsCfg = &hObj->htsCfg[pyrLvl];
        focoHtsCfg = &hObj->focoHtsCfg[pyrLvl];
        dofConfig = &hObj->dofCfg[pyrLvl];

        CSL_dmpacHtsSchConfigInit(htsCfg);

        htsCfg->schId = CSL_HTS_HWA_SCH_DOF;

        htsCfg->pipeline = instObj->pipeline;
        htsCfg->enableStream = (uint32_t)UFALSE;
        htsCfg->enableHop = (uint32_t)UFALSE;
        htsCfg->enableWdTimer = UFALSE;
        htsCfg->isWdTimer128KMode = (uint32_t)UFALSE;
        htsCfg->enableBwLimit = (uint32_t)UFALSE;

        htsCfg->enableBwLimit = UFALSE;

        Vhwa_m2mDofCalGwPrms(dofConfig, &refBot, &refTop, &curBot, &curTop);

        CSL_dmpacHtsSchConfigInit(focoHtsCfg);
        focoHtsCfg->schId = CSL_HTS_HWA_SCH_FOCO1;

        if(hObj->isFocoUsed[pyrLvl] == UTRUE)
        {
            focoHtsCfg->pipeline = instObj->pipeline;
            focoHtsCfg->enableStream = (uint32_t)UFALSE;
            focoHtsCfg->enableHop = (uint32_t)UFALSE;
            focoHtsCfg->enableWdTimer = UFALSE;
            focoHtsCfg->isWdTimer128KMode = (uint32_t)UFALSE;
            focoHtsCfg->enableBwLimit = (uint32_t)UFALSE;

            /* RFGW Config */
            focoHtsCfg->consCfg[HTS_DOF_REFE_FRAME_CH_IDX].enable = (uint32_t)UTRUE;
            focoHtsCfg->consCfg[HTS_DOF_REFE_FRAME_CH_IDX].prodId = CSL_HTS_PROD_IDX_UDMA;

            focoHtsCfg->dmaProdCfg[HTS_DOF_REFE_FRAME_CH_IDX].enable = (uint32_t)UTRUE;
            focoHtsCfg->dmaProdCfg[HTS_DOF_REFE_FRAME_CH_IDX].dmaChNum =
                                Udma_chGetNum(instObj->utcChHndl[DOF_INPUT_FOCO_REF_IMG]);
            focoHtsCfg->dmaProdCfg[HTS_DOF_REFE_FRAME_CH_IDX].pipeline = instObj->pipeline;
            focoHtsCfg->dmaProdCfg[HTS_DOF_REFE_FRAME_CH_IDX].consId = CSL_HTS_CONS_IDX_UDMA;
            /* Default mapping for DMA */

            focoHtsCfg->dmaProdCfg[HTS_DOF_REFE_FRAME_CH_IDX].threshold = 1u;
            focoHtsCfg->dmaProdCfg[HTS_DOF_REFE_FRAME_CH_IDX].cntPreLoad = 0u;

            if(((refTop + refBot + 2u) > dofConfig->height) &&
                ((refBot + 2u) >= dofConfig->height))
            {
                focoHtsCfg->dmaProdCfg[HTS_DOF_REFE_FRAME_CH_IDX].cntPostLoad =
                            dofConfig->height - 2u - curBot;
            }
            else
            {
                focoHtsCfg->dmaProdCfg[HTS_DOF_REFE_FRAME_CH_IDX].cntPostLoad =
                            refBot - curBot;
            }

            focoHtsCfg->dmaProdCfg[HTS_DOF_REFE_FRAME_CH_IDX].enableHop = (uint32_t)UTRUE;
            focoHtsCfg->dmaProdCfg[HTS_DOF_REFE_FRAME_CH_IDX].numHop = dofConfig->height;
            focoHtsCfg->dmaProdCfg[HTS_DOF_REFE_FRAME_CH_IDX].countDec = 1U;
            focoHtsCfg->dmaProdCfg[HTS_DOF_REFE_FRAME_CH_IDX].depth = DOF_FOCO_SL2_DEPTH;


            focoHtsCfg->prodCfg[HTS_DOF_REFE_FRAME_CH_IDX].enable = (uint32_t)UTRUE;
            focoHtsCfg->prodCfg[HTS_DOF_REFE_FRAME_CH_IDX].consId = CSL_HTS_CONS_IDX_DOF_REF;

            if((refTop + refBot + 2u) > dofConfig->height)
            {
                focoHtsCfg->prodCfg[HTS_DOF_REFE_FRAME_CH_IDX].threshold =
                            dofConfig->height;
                if((refBot + 2u) < dofConfig->height)
                {
                    focoHtsCfg->prodCfg[HTS_DOF_REFE_FRAME_CH_IDX].cntPreLoad =
                           (dofConfig->height - refBot - 2u);
                }
                else
                {
                    focoHtsCfg->prodCfg[HTS_DOF_REFE_FRAME_CH_IDX].cntPreLoad = 0u;
                }

                focoHtsCfg->prodCfg[HTS_DOF_REFE_FRAME_CH_IDX].cntPostLoad =
                        curBot + (2u * dofConfig->medianFilter);
            }
            else
            {
                focoHtsCfg->prodCfg[HTS_DOF_REFE_FRAME_CH_IDX].threshold =
                                 (refTop + refBot + 2u);
                focoHtsCfg->prodCfg[HTS_DOF_REFE_FRAME_CH_IDX].cntPreLoad = refTop;
                focoHtsCfg->prodCfg[HTS_DOF_REFE_FRAME_CH_IDX].cntPostLoad =
                                    curBot + (2u * dofConfig->medianFilter);
            }

            focoHtsCfg->prodCfg[HTS_DOF_REFE_FRAME_CH_IDX].depth =
                        (refTop + refBot + 2u +
                         hObj->sl2Prms.sl2NumLines[DOF_INPUT_REFERENCE_IMG]);
            focoHtsCfg->prodCfg[HTS_DOF_REFE_FRAME_CH_IDX].countDec = 2U;


            /* CFGW Config */
            focoHtsCfg->consCfg[HTS_DOF_CUR_FRAME_CH_IDX].enable = (uint32_t)UTRUE;
            focoHtsCfg->consCfg[HTS_DOF_CUR_FRAME_CH_IDX].prodId = CSL_HTS_PROD_IDX_UDMA;

            focoHtsCfg->dmaProdCfg[HTS_DOF_CUR_FRAME_CH_IDX].enable = (uint32_t)UTRUE;
            focoHtsCfg->dmaProdCfg[HTS_DOF_CUR_FRAME_CH_IDX].dmaChNum =
                            Udma_chGetNum(instObj->utcChHndl[DOF_INPUT_FOCO_CURR_IMG]);
            focoHtsCfg->dmaProdCfg[HTS_DOF_CUR_FRAME_CH_IDX].pipeline = instObj->pipeline;
            focoHtsCfg->dmaProdCfg[HTS_DOF_CUR_FRAME_CH_IDX].consId = CSL_HTS_CONS_IDX_UDMA;
            /* Default mapping for DMA */

            focoHtsCfg->dmaProdCfg[HTS_DOF_CUR_FRAME_CH_IDX].threshold = 1u;
            focoHtsCfg->dmaProdCfg[HTS_DOF_CUR_FRAME_CH_IDX].cntPreLoad =
                           focoHtsCfg->dmaProdCfg[HTS_DOF_REFE_FRAME_CH_IDX].cntPostLoad;
            focoHtsCfg->dmaProdCfg[HTS_DOF_CUR_FRAME_CH_IDX].cntPostLoad = 0u;

            focoHtsCfg->dmaProdCfg[HTS_DOF_CUR_FRAME_CH_IDX].enableHop = (uint32_t)UTRUE;
            focoHtsCfg->dmaProdCfg[HTS_DOF_CUR_FRAME_CH_IDX].numHop = dofConfig->height;
            focoHtsCfg->dmaProdCfg[HTS_DOF_CUR_FRAME_CH_IDX].countDec = 1U;
            focoHtsCfg->dmaProdCfg[HTS_DOF_CUR_FRAME_CH_IDX].depth = DOF_FOCO_SL2_DEPTH;


            focoHtsCfg->prodCfg[HTS_DOF_CUR_FRAME_CH_IDX].enable = (uint32_t)UTRUE;
            focoHtsCfg->prodCfg[HTS_DOF_CUR_FRAME_CH_IDX].consId = CSL_HTS_CONS_IDX_DOF_CURR;

            focoHtsCfg->prodCfg[HTS_DOF_CUR_FRAME_CH_IDX].threshold =
                                        (curTop + curBot + 2u);

            focoHtsCfg->prodCfg[HTS_DOF_CUR_FRAME_CH_IDX].cntPreLoad = curTop;
            focoHtsCfg->prodCfg[HTS_DOF_CUR_FRAME_CH_IDX].cntPostLoad =
                                        curBot + (2u * dofConfig->medianFilter);


            focoHtsCfg->prodCfg[HTS_DOF_CUR_FRAME_CH_IDX].depth =
                         (curTop + curBot + 2u +
                         hObj->sl2Prms.sl2NumLines[DOF_INPUT_CURRENT_IMG]);
            focoHtsCfg->prodCfg[HTS_DOF_CUR_FRAME_CH_IDX].countDec = 2U;

            /* Consumer control for RFGW */
            htsCfg->consCfg[HTS_DOF_REFE_FRAME_CH_IDX].enable = (uint32_t)UTRUE;
            htsCfg->consCfg[HTS_DOF_REFE_FRAME_CH_IDX].prodId = CSL_HTS_PROD_IDX_FOCO0_0;

            /* Consumer control for CFGW */
            htsCfg->consCfg[HTS_DOF_CUR_FRAME_CH_IDX].enable = (uint32_t)UTRUE;
            htsCfg->consCfg[HTS_DOF_CUR_FRAME_CH_IDX].prodId = CSL_HTS_PROD_IDX_FOCO0_1;

        }
        else
        {
            /* RFGW Config */
            htsCfg->consCfg[HTS_DOF_REFE_FRAME_CH_IDX].enable = (uint32_t)UTRUE;
            htsCfg->consCfg[HTS_DOF_REFE_FRAME_CH_IDX].prodId = CSL_HTS_PROD_IDX_UDMA;

            htsCfg->dmaProdCfg[HTS_DOF_REFE_FRAME_CH_IDX].enable = (uint32_t)UTRUE;
            htsCfg->dmaProdCfg[HTS_DOF_REFE_FRAME_CH_IDX].dmaChNum =
                        Udma_chGetNum(instObj->utcChHndl[DOF_INPUT_REFERENCE_IMG]);
            htsCfg->dmaProdCfg[HTS_DOF_REFE_FRAME_CH_IDX].pipeline = instObj->pipeline;
            htsCfg->dmaProdCfg[HTS_DOF_REFE_FRAME_CH_IDX].consId = CSL_HTS_CONS_IDX_UDMA;
            /* Default mapping for DMA */

            if((refTop + refBot + 2u) > dofConfig->height)
            {
                htsCfg->dmaProdCfg[HTS_DOF_REFE_FRAME_CH_IDX].threshold =
                            dofConfig->height/2U;
                if((refBot + 2u) < dofConfig->height)
                {
                    htsCfg->dmaProdCfg[HTS_DOF_REFE_FRAME_CH_IDX].cntPreLoad =
                           (dofConfig->height - refBot - 2u)/2U;
                }
                else
                {
                    htsCfg->dmaProdCfg[HTS_DOF_REFE_FRAME_CH_IDX].cntPreLoad = 0;
                }
                if(refBot < dofConfig->height)
                {
                    htsCfg->dmaProdCfg[HTS_DOF_REFE_FRAME_CH_IDX].cntPostLoad =
                            (refBot/2U)
                            + dofConfig->medianFilter;
                }
                else
                {
                    htsCfg->dmaProdCfg[HTS_DOF_REFE_FRAME_CH_IDX].cntPostLoad =
                            ((dofConfig->height - 2u)/2U)
                            + dofConfig->medianFilter;
                }
            }
            else
            {
                htsCfg->dmaProdCfg[HTS_DOF_REFE_FRAME_CH_IDX].threshold =
                            (refTop + refBot + 2u)/2U;
                htsCfg->dmaProdCfg[HTS_DOF_REFE_FRAME_CH_IDX].cntPreLoad =
                            refTop/2U;
                htsCfg->dmaProdCfg[HTS_DOF_REFE_FRAME_CH_IDX].cntPostLoad =
                            (refBot/2U) + dofConfig->medianFilter/*1*/;
            }

            htsCfg->dmaProdCfg[HTS_DOF_REFE_FRAME_CH_IDX].enableHop = (uint32_t)UTRUE;
            htsCfg->dmaProdCfg[HTS_DOF_REFE_FRAME_CH_IDX].numHop =
                            dofConfig->height/2U;
            htsCfg->dmaProdCfg[HTS_DOF_REFE_FRAME_CH_IDX].countDec = 1U;
            htsCfg->dmaProdCfg[HTS_DOF_REFE_FRAME_CH_IDX].depth =
                        (refTop + refBot + 2u +
                         hObj->sl2Prms.sl2NumLines[DOF_INPUT_REFERENCE_IMG])/2U;


            /* CFGW Config */
            htsCfg->consCfg[HTS_DOF_CUR_FRAME_CH_IDX].enable = (uint32_t)UTRUE;
            htsCfg->consCfg[HTS_DOF_CUR_FRAME_CH_IDX].prodId = CSL_HTS_PROD_IDX_UDMA;

            htsCfg->dmaProdCfg[HTS_DOF_CUR_FRAME_CH_IDX].enable = (uint32_t)UTRUE;
            htsCfg->dmaProdCfg[HTS_DOF_CUR_FRAME_CH_IDX].dmaChNum =
                        Udma_chGetNum(instObj->utcChHndl[DOF_INPUT_CURRENT_IMG]);
            htsCfg->dmaProdCfg[HTS_DOF_CUR_FRAME_CH_IDX].pipeline = instObj->pipeline;
            htsCfg->dmaProdCfg[HTS_DOF_CUR_FRAME_CH_IDX].consId = CSL_HTS_CONS_IDX_UDMA;
            /* Default mapping for DMA */

            htsCfg->dmaProdCfg[HTS_DOF_CUR_FRAME_CH_IDX].threshold =
                        (curTop + curBot + 2U)/2U;
            htsCfg->dmaProdCfg[HTS_DOF_CUR_FRAME_CH_IDX].cntPreLoad = curTop/2U;
            htsCfg->dmaProdCfg[HTS_DOF_CUR_FRAME_CH_IDX].cntPostLoad =
                            (curBot/2U) + dofConfig->medianFilter;

            htsCfg->dmaProdCfg[HTS_DOF_CUR_FRAME_CH_IDX].enableHop = (uint32_t)UTRUE;
            htsCfg->dmaProdCfg[HTS_DOF_CUR_FRAME_CH_IDX].numHop = dofConfig->height/2U;
            htsCfg->dmaProdCfg[HTS_DOF_CUR_FRAME_CH_IDX].countDec = 1U;
            htsCfg->dmaProdCfg[HTS_DOF_CUR_FRAME_CH_IDX].depth =
                        htsCfg->dmaProdCfg[HTS_DOF_CUR_FRAME_CH_IDX].threshold +
                        (hObj->sl2Prms.sl2NumLines[DOF_INPUT_CURRENT_IMG]/2U);
        }

        if((UTRUE == hObj->isTempPred) && (0u == pyrLvl))
        {
            /* Temporal predictor config */
            htsCfg->consCfg[HTS_DOF_TEM_PRE_CH_IDX].enable = (uint32_t)UTRUE;
            htsCfg->consCfg[HTS_DOF_TEM_PRE_CH_IDX].prodId = CSL_HTS_PROD_IDX_UDMA;

            htsCfg->dmaProdCfg[HTS_DOF_TEM_PRE_CH_IDX].enable = (uint32_t)UTRUE;
            htsCfg->dmaProdCfg[HTS_DOF_TEM_PRE_CH_IDX].dmaChNum =
                Udma_chGetNum(instObj->utcChHndl[DOF_INPUT_TEMPORAL_PRED]);
            htsCfg->dmaProdCfg[HTS_DOF_TEM_PRE_CH_IDX].pipeline = instObj->pipeline;
            htsCfg->dmaProdCfg[HTS_DOF_TEM_PRE_CH_IDX].consId = CSL_HTS_CONS_IDX_UDMA;
            /* Default mapping for DMA */

            htsCfg->dmaProdCfg[HTS_DOF_TEM_PRE_CH_IDX].threshold = 1u;
            htsCfg->dmaProdCfg[HTS_DOF_TEM_PRE_CH_IDX].cntPreLoad = 0u;
            htsCfg->dmaProdCfg[HTS_DOF_TEM_PRE_CH_IDX].cntPostLoad = dofConfig->medianFilter;

            htsCfg->dmaProdCfg[HTS_DOF_TEM_PRE_CH_IDX].enableHop = (uint32_t)UTRUE;
            htsCfg->dmaProdCfg[HTS_DOF_TEM_PRE_CH_IDX].numHop = dofConfig->height/2u;
            htsCfg->dmaProdCfg[HTS_DOF_TEM_PRE_CH_IDX].countDec = 1U;
            htsCfg->dmaProdCfg[HTS_DOF_TEM_PRE_CH_IDX].depth =
                                       DOF_TEMPORAL_PRED_SL2_DEPTH;
        }

        if (UTRUE == hObj->isPyrPred[pyrLvl])
        {
            /* Pyramidal predictor config */
            htsCfg->consCfg[HTS_DOF_PYR_PRE_CH_IDX].enable = (uint32_t)UTRUE;
            htsCfg->consCfg[HTS_DOF_PYR_PRE_CH_IDX].prodId = CSL_HTS_PROD_IDX_UDMA;


            htsCfg->dmaProdCfg[HTS_DOF_PYR_PRE_CH_IDX].enable = (uint32_t)UTRUE;
            htsCfg->dmaProdCfg[HTS_DOF_PYR_PRE_CH_IDX].dmaChNum =
                    Udma_chGetNum(instObj->utcChHndl[DOF_INPUT_PYRAMID_PRED]);
            htsCfg->dmaProdCfg[HTS_DOF_PYR_PRE_CH_IDX].pipeline = instObj->pipeline;
            htsCfg->dmaProdCfg[HTS_DOF_PYR_PRE_CH_IDX].consId = CSL_HTS_CONS_IDX_UDMA;
            /* Default mapping for DMA */

            htsCfg->dmaProdCfg[HTS_DOF_PYR_PRE_CH_IDX].threshold = 1U;
            htsCfg->dmaProdCfg[HTS_DOF_PYR_PRE_CH_IDX].cntPreLoad = 0u;
            htsCfg->dmaProdCfg[HTS_DOF_PYR_PRE_CH_IDX].cntPostLoad = dofConfig->medianFilter;

            htsCfg->dmaProdCfg[HTS_DOF_PYR_PRE_CH_IDX].enableHop = (uint32_t)UTRUE;
            htsCfg->dmaProdCfg[HTS_DOF_PYR_PRE_CH_IDX].numHop = dofConfig->height/2u;
            htsCfg->dmaProdCfg[HTS_DOF_PYR_PRE_CH_IDX].countDec = 1U;
            htsCfg->dmaProdCfg[HTS_DOF_PYR_PRE_CH_IDX].depth =
                                       DOF_PYRAM_PRED_SL2_DEPTH;
        }

        if((UTRUE == hObj->isSof) && (0u == pyrLvl))
        {
            /* SOF config */
            htsCfg->consCfg[HTS_DOF_SOF_CH_IDX].enable = (uint32_t)UTRUE;
            htsCfg->consCfg[HTS_DOF_SOF_CH_IDX].prodId = CSL_HTS_PROD_IDX_UDMA;

            htsCfg->dmaProdCfg[HTS_DOF_SOF_CH_IDX].enable = (uint32_t)UTRUE;
            htsCfg->dmaProdCfg[HTS_DOF_SOF_CH_IDX].dmaChNum =
                    Udma_chGetNum(instObj->utcChHndl[DOF_INPUT_SOF]);
            htsCfg->dmaProdCfg[HTS_DOF_SOF_CH_IDX].pipeline = instObj->pipeline;
            htsCfg->dmaProdCfg[HTS_DOF_SOF_CH_IDX].consId = CSL_HTS_CONS_IDX_UDMA;
            /* Default mapping for DMA */

            htsCfg->dmaProdCfg[HTS_DOF_SOF_CH_IDX].threshold = 1U;
            htsCfg->dmaProdCfg[HTS_DOF_SOF_CH_IDX].cntPreLoad = 0u;
            htsCfg->dmaProdCfg[HTS_DOF_SOF_CH_IDX].cntPostLoad =
                                                dofConfig->medianFilter;

            htsCfg->dmaProdCfg[HTS_DOF_SOF_CH_IDX].enableHop = (uint32_t)UTRUE;
            htsCfg->dmaProdCfg[HTS_DOF_SOF_CH_IDX].numHop = dofConfig->height/2u;
            htsCfg->dmaProdCfg[HTS_DOF_SOF_CH_IDX].countDec = 1U;
            if(dofConfig->medianFilter == UTRUE)
            {
                htsCfg->dmaProdCfg[HTS_DOF_SOF_CH_IDX].depth =
                                       (DOF_SOF_SL2_DEPTH/2u) - 1U;
            }
            else
            {
                htsCfg->dmaProdCfg[HTS_DOF_SOF_CH_IDX].depth =
                                       DOF_SOF_SL2_DEPTH/2u;
            }
        }

        /* Output flow config */
        htsCfg->prodCfg[HTS_DOF_OUT_CH_IDX].enable = (uint32_t)UTRUE;
        htsCfg->prodCfg[HTS_DOF_OUT_CH_IDX].consId = CSL_HTS_CONS_IDX_UDMA;

        htsCfg->prodCfg[HTS_DOF_OUT_CH_IDX].threshold = 1u;
        htsCfg->prodCfg[HTS_DOF_OUT_CH_IDX].cntPreLoad = 0u;
        htsCfg->prodCfg[HTS_DOF_OUT_CH_IDX].cntPostLoad = 0u;

        /* Output 2 lines per dma transfer*/
        htsCfg->prodCfg[HTS_DOF_OUT_CH_IDX].depth =
                hObj->sl2Prms.sl2NumLines[DOF_OUTPUT]/2U;
        htsCfg->prodCfg[HTS_DOF_OUT_CH_IDX].countDec = 1U;

        htsCfg->dmaConsCfg[HTS_DOF_OUT_CH_IDX].enable = (uint32_t)UTRUE;
        htsCfg->dmaConsCfg[HTS_DOF_OUT_CH_IDX].dmaChNum =
                Udma_chGetNum(instObj->utcChHndl[DOF_OUTPUT]);
        htsCfg->dmaConsCfg[HTS_DOF_OUT_CH_IDX].pipeline = instObj->pipeline;
        htsCfg->dmaConsCfg[HTS_DOF_OUT_CH_IDX].enableStream = (uint32_t)UFALSE;
        htsCfg->dmaConsCfg[HTS_DOF_OUT_CH_IDX].prodId = CSL_HTS_PROD_IDX_UDMA;
    }
}

static int32_t vhwaM2mDofSetSl2Params(const Vhwa_M2mDofInstObj *instObj,
        Vhwa_M2mDofHandleObj *hObj, Vhwa_M2mDofPrms *dofPrms)
{
    int32_t status = FVID2_SOK;
    uint32_t pitchSl2, width, pyrLvl, isFocoUsed;
    uint32_t rfgw, cfgw, sofMem, pyrMem, tempMem, focoBuffMem, outMem, tMemReq;
    uint64_t sl2StartAddr;
    Dof_Config          *coreCfg;;
    Vhwa_M2mDofSl2Prms  *sl2Prms;

    GT_assert(VhwaDofTrace, (NULL != instObj));
    GT_assert(VhwaDofTrace, (NULL != hObj));
    GT_assert(VhwaDofTrace, (NULL != dofPrms));

    coreCfg = &dofPrms->coreCfg;
    sl2Prms = &hObj->sl2Prms;
    sl2StartAddr = instObj->startAddr;

    isFocoUsed = UFALSE;

    width = coreCfg->width;

    /* Set ref and Base buffer Parameters */
    pitchSl2 = Vhwa_calcHorzSizeInBytes(width,
                        FVID2_CCSF_BITS12_PACKED);
    pitchSl2 = Vhwa_calcSl2Pitch(pitchSl2);

    /* RFGW */
    rfgw = pitchSl2 * (((coreCfg->topSearchRange + 7u) & 0xFFFFFFFEu)
                    + 2u + ((coreCfg->bottomSearchRange + 10u) & 0xFFFFFFFEu)
                    + sl2Prms->sl2NumLines[DOF_INPUT_REFERENCE_IMG]);

    /* CFGW */
    cfgw = pitchSl2 * (6u + 2u + 8u + sl2Prms->sl2NumLines[DOF_INPUT_REFERENCE_IMG]);

    /* Calculate Temporal Predictor memory requirement */
    tempMem = CSL_lseMakePitchAligned(width * 4U) *
                DOF_TEMPORAL_PRED_SL2_DEPTH;

    /* Calculate Pyramidal Predictor memory requirement */
    pyrMem = CSL_lseMakePitchAligned(width) * DOF_PYRAM_PRED_SL2_DEPTH;

    /* Calculate SOF memory requirement */
    sofMem = width/8u;
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: There is no test case with the required test configuration to reach this portion.
    Effect on this unit: The code base is expected to ensure that enough sl2 memory is allocated
    for sof map when the width is not a multiple of 8.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if((sofMem * 8u) != width)
    {
        sofMem++;
    }
    /* LDRA_JUSTIFY_END */
    sofMem = CSL_lseMakePitchAligned(sofMem) * DOF_SOF_SL2_DEPTH;

    /* Output Buffer Memory Required */
    outMem = CSL_lseMakePitchAligned(width * 4U) *
                sl2Prms->sl2NumLines[DOF_OUTPUT];

    for (pyrLvl = 0u; pyrLvl < dofPrms->tPrmdLvl; pyrLvl++)
    {
        if(FVID2_CCSF_BITS12_PACKED !=
                dofPrms->inOutImgFmt[pyrLvl][DOF_INPUT_CURRENT_IMG].ccsFormat)
        {
            isFocoUsed = UTRUE;
            break;
        }
    }
    /* Set FOCO Parameters */
    if(UFALSE == isFocoUsed)
    {
        focoBuffMem = 0u;
    }
    else
    {
        focoBuffMem = Vhwa_calcHorzSizeInBytes(width,
                dofPrms->inOutImgFmt[pyrLvl][DOF_INPUT_CURRENT_IMG].ccsFormat);
        focoBuffMem = CSL_lseMakePitchAligned(focoBuffMem) * DOF_FOCO_SL2_DEPTH;
    }

    tMemReq = rfgw + cfgw + sofMem + pyrMem + tempMem +
              (focoBuffMem * 2u) + outMem;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    /* Check if 2-region allocation (firewall enabled) */
    if (instObj->mainRegionStartAddr != 0u)
    /* LDRA_JUSTIFY_END */
    {
        /* DOF Two-region mode: validate buffer requirements fit within allocated regions */
        uint32_t region0MemReq = cfgw + focoBuffMem + sofMem + pyrMem + tempMem + outMem;  /* MAIN region */
        uint32_t region1MemReq = rfgw + focoBuffMem;  /* REFERENCE region */

        /* Validate Region 0 (MAIN/DMPAC_FWL_REGION_DOF_MAIN) buffers fit */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (region0MemReq > instObj->mainRegionSl2Size)
        {
            GT_2trace(VhwaDofTrace, GT_ERR,
                "Region 0 (MAIN) buffers required (0x%x) exceed allocated size (0x%x)\n",
                region0MemReq, instObj->mainRegionSl2Size);
            status = FVID2_EFAIL;
        }
        /* Validate Region 1 (REFERENCE/DMPAC_FWL_REGION_DOF_REFERENCE) buffers fit */
        else if (region1MemReq > instObj->refRegionSl2Size)
        {
            GT_2trace(VhwaDofTrace, GT_ERR,
                "Region 1 (REFERENCE) buffers required (0x%x) exceed allocated size (0x%x)\n",
                region1MemReq, instObj->refRegionSl2Size);
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */
        else
        {
            /* DOF TWO-REGION LAYOUT (DMPAC firewall regions 0 and 1):
             * Region 0 (MAIN/DMPAC_FWL_REGION_DOF_MAIN): Current + FOCO_CUR + Predictors + Output
             *   Permissions: R5(213), FOCO(211), DOF(210)
             * Region 1 (REFERENCE/DMPAC_FWL_REGION_DOF_REFERENCE): Reference + FOCO_REF
             *   Permissions: R5(213), FOCO(211), DOF_RD(209) */

            /* ===== Region 0 (MAIN) Buffer Layout ===== */
            uint64_t region0Addr = instObj->mainRegionStartAddr;

            /* Buffer 1: Current image (CFGW) */
            sl2Prms->sl2Addr[DOF_INPUT_CURRENT_IMG] = region0Addr;
            region0Addr += cfgw;
            sl2Prms->sl2RefCurPitch = pitchSl2;

            /* Buffer 2: FOCO preprocessed current */
            sl2Prms->sl2Addr[DOF_INPUT_FOCO_CURR_IMG] = region0Addr;
            region0Addr += focoBuffMem;

            /* Buffer 3: Temporal predictor */
            sl2Prms->sl2Addr[DOF_INPUT_TEMPORAL_PRED] = region0Addr;
            region0Addr += tempMem;

            /* Buffer 4: Pyramid predictor */
            sl2Prms->sl2Addr[DOF_INPUT_PYRAMID_PRED] = region0Addr;
            region0Addr += pyrMem;

            /* Buffer 5: SOF predictor */
            sl2Prms->sl2Addr[DOF_INPUT_SOF] = region0Addr;
            region0Addr += sofMem;

            /* Buffer 6: Flow vector output */
            sl2Prms->sl2Addr[DOF_OUTPUT] = region0Addr;

            /* ===== Region 1 (REFERENCE) Buffer Layout ===== */
            uint64_t region1Addr = instObj->refRegionStartAddr;

            /* Buffer 1: Reference image (RFGW) */
            sl2Prms->sl2Addr[DOF_INPUT_REFERENCE_IMG] = region1Addr;
            region1Addr += rfgw;

            /* Buffer 2: FOCO preprocessed reference */
            sl2Prms->sl2Addr[DOF_INPUT_FOCO_REF_IMG] = region1Addr;
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
    else if (tMemReq <= instObj->sl2Size)
    /* LDRA_JUSTIFY_END */
    {
        /* Single-region mode: Place all buffers sequentially (order matches allocation) */
        sl2Prms->sl2Addr[DOF_INPUT_REFERENCE_IMG] = sl2StartAddr;
        sl2StartAddr = sl2StartAddr + rfgw;

        sl2Prms->sl2Addr[DOF_INPUT_CURRENT_IMG] = sl2StartAddr;
        sl2Prms->sl2RefCurPitch = pitchSl2;
        sl2StartAddr = sl2StartAddr + cfgw;

        sl2Prms->sl2Addr[DOF_INPUT_TEMPORAL_PRED] = sl2StartAddr;
        sl2StartAddr = sl2StartAddr + tempMem;

        sl2Prms->sl2Addr[DOF_INPUT_PYRAMID_PRED] = sl2StartAddr;
        sl2StartAddr = sl2StartAddr + pyrMem;

        sl2Prms->sl2Addr[DOF_INPUT_SOF] = sl2StartAddr;
        sl2StartAddr = sl2StartAddr + sofMem;

        sl2Prms->sl2Addr[DOF_INPUT_FOCO_REF_IMG] = sl2StartAddr;
        sl2StartAddr = sl2StartAddr + focoBuffMem;

        sl2Prms->sl2Addr[DOF_INPUT_FOCO_CURR_IMG] = sl2StartAddr;
        sl2StartAddr = sl2StartAddr + focoBuffMem;

        /* OUTPUT buffer follows FOCO buffers */
        sl2Prms->sl2Addr[DOF_OUTPUT] = sl2StartAddr;
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
        GT_2trace(VhwaDofTrace, GT_ERR,
            "Total memory (0x%x) exceeds SL2 size (0x%x)\n",
            tMemReq, instObj->sl2Size);
        status = FVID2_EFAIL;
    }
    /* LDRA_JUSTIFY_END */
    return status;
}


static void vhwaM2mDofSetBuffParams(Vhwa_M2mDofHandleObj *hObj,
                                    Vhwa_M2mDofPrms *dofPrms)
{
    uint32_t curTop, curBot, refTop, refBot;
    Dof_Config *coreCfg;

    GT_assert(VhwaDofTrace, (NULL != hObj));
    GT_assert(VhwaDofTrace, (NULL != dofPrms));

    coreCfg = &dofPrms->coreCfg;

    Vhwa_m2mDofCalGwPrms(coreCfg, &refBot, &refTop, &curBot, &curTop);

    /* Current Frame Buffer config */
    hObj->dofBuffPrams.currImageAddress =
                    (uint32_t)hObj->sl2Prms.sl2Addr[DOF_INPUT_CURRENT_IMG];
    hObj->dofBuffPrams.currBufWidth = hObj->sl2Prms.sl2RefCurPitch;
    /* line level staggering is done while organizing the growing window */
    hObj->dofBuffPrams.currBufHeight = (curTop + curBot + 2u +
                        hObj->sl2Prms.sl2NumLines[DOF_INPUT_CURRENT_IMG]);

    /* Reference Frame buffer config */
    hObj->dofBuffPrams.refImageAddress =
                    (uint32_t)hObj->sl2Prms.sl2Addr[DOF_INPUT_REFERENCE_IMG];
    hObj->dofBuffPrams.refBufWidth = hObj->sl2Prms.sl2RefCurPitch;

    hObj->dofBuffPrams.refBufHeight =
                        refTop + refBot + 2u +
                        hObj->sl2Prms.sl2NumLines[DOF_INPUT_REFERENCE_IMG];

    hObj->dofBuffPrams.spatialPredAddress =
                    (uint32_t)hObj->sl2Prms.sl2Addr[DOF_INPUT_PYRAMID_PRED];
    hObj->dofBuffPrams.spatialPredDepth = DOF_PYRAM_PRED_SL2_DEPTH;
    hObj->dofBuffPrams.temporalPredAddress =
                    (uint32_t)hObj->sl2Prms.sl2Addr[DOF_INPUT_TEMPORAL_PRED];
    hObj->dofBuffPrams.temporalPredDepth = DOF_TEMPORAL_PRED_SL2_DEPTH;
    hObj->dofBuffPrams.binaryMapAddress =
                    (uint32_t)hObj->sl2Prms.sl2Addr[DOF_INPUT_SOF];
    hObj->dofBuffPrams.binaryMapDepth = DOF_SOF_SL2_DEPTH;
    hObj->dofBuffPrams.outFlowAddress =
                    (uint32_t)hObj->sl2Prms.sl2Addr[DOF_OUTPUT];
    hObj->dofBuffPrams.outFlowpDepth = hObj->sl2Prms.sl2NumLines[DOF_OUTPUT];

}

static int32_t vhwaM2mDofCreateQueues(Vhwa_M2mDofInstObj *instObj)
{
    int32_t              status = FVID2_SOK;
    uint32_t             qCnt;
    Vhwa_M2mDofQueueObj *qObj;

    /* NULL pointer check */
    GT_assert(VhwaDofTrace, (NULL != instObj));

    instObj->freeQ = NULL;

    /* Create Free queue */
    status = Fvid2Utils_constructQ(&instObj->freeQObj);
    GT_assert(VhwaDofTrace, (FVID2_SOK == status));

    instObj->freeQ = &instObj->freeQObj;

    /* Initialize and queue the allocate queue object to free Q */
    for(qCnt = 0U; qCnt < VHWA_M2M_DOF_UDMA_RING_ENTRIES; qCnt ++)
    {
        qObj = &instObj->dofQObj[qCnt];

        Fvid2Utils_memset(qObj, 0x0, sizeof(Vhwa_M2mDofQueueObj));

        Fvid2Utils_queue(instObj->freeQ, &qObj->qElem, qObj);
    }

    return (status);
}

static void vhwaM2mDofDeleteQueues(Vhwa_M2mDofInstObj *instObj)
{
    Vhwa_M2mDofQueueObj *qObj = NULL;

    /* NULL pointer check */
    GT_assert(VhwaDofTrace, (NULL != instObj));
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> Rationale: The negative test framework and test apps cannot reach this portion.
    No app configuration will result in call to vhwaM2mDofDeleteQueues if the queue is already empty.
    Effect on this unit: If the queue is already NULL, not action is needed.
    <justification end> */
    if(NULL != instObj->freeQ)
    /* LDRA_JUSTIFY_END */
    {
        /* Free-up all the queued free queue objects */
        do
        {
            qObj = (Vhwa_M2mDofQueueObj *) Fvid2Utils_dequeue(instObj->freeQ);
        } while (NULL != qObj);

        /* Delete the free Q */
        Fvid2Utils_destructQ(instObj->freeQ);
        instObj->freeQ = NULL;
    }
}

static void vhwaM2mDofSetBwLimitParams(Vhwa_M2mDofHandleObj *hObj,
                                       const Vhwa_HtsLimiter *bwLimit)
{
    uint32_t pyrLvl;

    for (pyrLvl = 0u; pyrLvl < hObj->dofPrms.tPrmdLvl; pyrLvl++)
    {
        /* Setting directly in the HTS configuration, so that
         * it will be configured from the next request. */
        hObj->htsCfg[pyrLvl].enableBwLimit   = bwLimit->enableBwLimit;
        hObj->htsCfg[pyrLvl].cycleCnt        = bwLimit->cycleCnt;
        hObj->htsCfg[pyrLvl].tokenCnt        = bwLimit->tokenCnt;
    }
}

#if !defined(SOC_J722S)
static int32_t Vhwa_m2mDofReconfigReinitReg(const Vhwa_M2mDofInstObj *instObj, const Vhwa_M2mDofHandleObj *hObj, const Vhwa_M2mDofQueueObj *qObj)
{
    int32_t status = FVID2_SOK;

    /* Null pointer check */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated in the imaging layer before reaching vhwa.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == instObj) || (NULL == hObj) || (NULL == qObj))
    /* LDRA_JUSTIFY_END */
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        VhwaDmpacDofSocReadBack *goldenRegVal = hObj->configRegMemPrms.configGoldenRegPtr;

        /* Reconfigure the DOF MMR registers as needed.
           This is a placeholder for actual reconfiguration logic */

        /* If last handle is same as current handle, reconfigure the non-static configuration MMRs */
        if (instObj->lastHndlObj == qObj->hObj)
        {
            /* Example: Reconfigure DOF buffer parameters for the given handle's config */
            status = CSL_dofSetBufParam(instObj->socInfo.dofRegs,
                                        &hObj->dofBuffPrams);

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
                status = CSL_dofSetBufParam(&(goldenRegVal->dofRegs), &hObj->dofBuffPrams);
            }
        }

        /* Reinitialise the MMRs that are having static configurations */
        /* Example: ENABLE register of CSL_dmpacRegs */
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
                DMPAC_MODULE_DOF, (uint32_t)UTRUE);

            /* If configRegValidate is enabled, also set the golden register values */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate) && (NULL != goldenRegVal))
            /* LDRA_JUSTIFY_END */
            {
                CSL_dmpacEnableModule(&(goldenRegVal->dmpacCntlRegs),
                    DMPAC_MODULE_DOF, (uint32_t)UTRUE);
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
            /* Re-Init Pipeline interrupt in INTD */
            Vhwa_enableDmpacHtsIntr(instObj->socInfo.dmpacIntdRegs,
                instObj->vhwaIrqNum, instObj->pipeline);
        }
    }

    return status;
}
#endif

static int32_t Vhwa_m2mDofSetIntrInHW(uint32_t vhwaIrqNum,
                                volatile CSL_dmpac_intd_cfgRegs  *intdRegs,
                                const Vhwa_M2mDofHandleObj *hObj)
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
        CSL_REG32_WR(&intdRegs->ENABLE_CLR_REG_LEVEL_DMPAC_OUT[vhwaIrqNum][VHWA_DMPAC_INTD_DOF_INTR_REG], VHWA_DOF_ALL_ERR_EVENTS);
        CSL_REG32_WR(&intdRegs->ENABLE_REG_LEVEL_DMPAC_OUT[vhwaIrqNum][VHWA_DMPAC_INTD_DOF_INTR_REG],
            hObj->eePrms.errEvents);

        CSL_REG32_WR(&intdRegs->ENABLE_CLR_REG_LEVEL_DMPAC_OUT[vhwaIrqNum][VHWA_DMPAC_INTD_WDTIMER_INTR_REG], VHWA_DOF_ALL_WDTIMER_ERR_EVENTS);
        CSL_REG32_WR(&intdRegs->ENABLE_REG_LEVEL_DMPAC_OUT[vhwaIrqNum][VHWA_DMPAC_INTD_WDTIMER_INTR_REG],
            hObj->wdTimerEePrms.wdTimerErrEvents);
    }

    return status;
}

#if !defined(SOC_J722S)
static int32_t vhwaM2mDofUpdateStatusRegGroup(Vhwa_M2mDofHandleObj *hObj)
{
    DofStatusRegisterGroup *statusRegs;
    int32_t status = FVID2_SOK;
    uint32_t pyrLvl;
    uint32_t expected_val = 0U;
    uint32_t curpaxaddrx_sts, curpaxaddry_sts;

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
        pyrLvl  = hObj->curPyrLvl;

        /* Update the current paxel address for DOF STAT */
        /* Prepare expected_val for DOF STAT register using width and height */
        /* Use CSL_FINS macros to set expected_val for DOF STAT register */
        expected_val = 0U;
        curpaxaddrx_sts = (hObj->dofCfg[pyrLvl].width / 2U ) - 1U;
        curpaxaddry_sts = (hObj->dofCfg[pyrLvl].height / 2U ) - 1U;
        CSL_FINS(expected_val, DMPAC_DOF_STAT_CURPAXADDRX_STS, curpaxaddrx_sts);
        CSL_FINS(expected_val, DMPAC_DOF_STAT_CURPAXADDRY_STS, curpaxaddry_sts);

        statusRegs->dofStatus.STAT = expected_val;

        /* Update only the count related values in the HtsStatusRead structure */
        /* These are: PROD_COUNT, PROD_DMA_COUNT */

        /* Assuming CSL_HTS_NUM_DOF_FOCO0_PRODUCERS and CSL_HTS_NUM_DOF_FOCO0_PRODUCERS_DMA are defined */
        uint32_t i;

        /* Update PROD_COUNT */
        for (i = 0; i < CSL_HTS_NUM_DOF_FOCO0_PRODUCERS; i++)
        {
            CSL_DmpacHtsSchConfig *cfg = NULL;
            CSL_DmpacHtsProdConfig *prodCfg = NULL;

            if (i < CSL_HTS_NUM_DOF_PRODUCERS)
            {
                /* DOF producers */
                cfg = &hObj->htsCfg[pyrLvl];
                prodCfg = &cfg->prodCfg[i];
            }
            else
            {
            /* FOCO0 producers */
            uint32_t foco_idx = i - CSL_HTS_NUM_DOF_PRODUCERS;
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (hObj->isFocoUsed[pyrLvl] == UTRUE)
            /* LDRA_JUSTIFY_END */
            {
                cfg = &hObj->focoHtsCfg[pyrLvl];
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
                CSL_FINS(expected_val, HTS_HWA0_PROD0_COUNT_COUNT_POSTLOAD, prodCfg->cntPostLoad);
                statusRegs->htsStatus.PROD_COUNT[i] = expected_val;
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
                statusRegs->htsStatus.PROD_COUNT[i] = 0U;
            }
            /* LDRA_JUSTIFY_END */
        }

        /* Update PROD_DMA_COUNT */
        for (i = 0; i < CSL_HTS_NUM_DOF_FOCO0_PRODUCERS_DMA; i++)
        {
            /* Update PROD_DMA_COUNT */
            const CSL_DmpacHtsSchConfig *cfg = NULL;
            const CSL_DmpacHtsDmaProdConfig *dmaProdCfg = NULL;

            if (i < CSL_HTS_NUM_DOF_PRODUCERS_DMA)
            {
                /* DOF DMA producers */
                cfg = &hObj->htsCfg[pyrLvl];
                dmaProdCfg = &cfg->dmaProdCfg[i];
            }
            else
            {
                /* FOCO0 DMA producers */
                uint32_t foco_dma_idx = i - CSL_HTS_NUM_DOF_PRODUCERS_DMA;
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (hObj->isFocoUsed[pyrLvl] == UTRUE)
                /* LDRA_JUSTIFY_END */
                {
                    cfg = &hObj->focoHtsCfg[pyrLvl];
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
                CSL_FINS(expected_val, HTS_DMA0_PROD0_COUNT_COUNT_POSTLOAD, dmaProdCfg->cntPostLoad);
                statusRegs->htsStatus.PROD_DMA_COUNT[i] = expected_val;
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
                statusRegs->htsStatus.PROD_DMA_COUNT[i] = 0U;
            }
            /* LDRA_JUSTIFY_END */
        }
    }

    return status;
}

int32_t Vhwa_m2mDofStatusRegValidate(const DofStatusRegisterGroup *expected, const Dof_SocInfo *socInfo)
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
            GT_0trace(VhwaDofTrace, GT_ERR, "StatusRegValidate: dmpacCntlRegs->PID mismatch\n");
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
            /* Validate PID values in CSL_dofRegs */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((uint32_t)VHWA_HW_PID_DOF != CSL_REG32_RD(&socInfo->dofRegs->PID))
            {
                GT_0trace(VhwaDofTrace, GT_ERR, "StatusRegValidate: dofRegs->PID mismatch\n");
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
            /* Validate DOF status */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (expected->dofStatus.STAT != CSL_REG32_RD(&socInfo->dofRegs->STAT))
            {
                GT_0trace(VhwaDofTrace, GT_ERR, "StatusRegValidate: dofRegs->STAT mismatch\n");
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
                GT_0trace(VhwaDofTrace, GT_ERR, "StatusRegValidate: lseRegs->STATUS_ERROR mismatch\n");
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
                GT_0trace(VhwaDofTrace, GT_ERR, "StatusRegValidate: lseRegs->STATUS_IDLE_MODE mismatch\n");
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
            /* Validate DST_BUF_CFG_CH_STATUS[4] of focoLseStatus using mask for CSL_LSE_DST_BUF_CFG_CH_DISABLED_MASK */
            /* The Channel disabled field of BUF CFG will be set to 1 in case of YUV interleaving, this doesnt apply for DMPAC but applicable for VPAC modules.
             * The expected values/channel disabled status is to be calculated based on the data format of frame submitted and the outputs enabled for the LSE out config.
            */
            for (uint32_t i = 0; i < 4U; i++)
            {
                uint32_t expected_val = (expected->focoLseStatus.DST_BUF_CFG_CH_STATUS[i] & CSL_LSE_DST_BUF_CFG_CH_DISABLED_MASK);
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
                    GT_1trace(VhwaDofTrace, GT_ERR, "StatusRegValidate: lseRegs->DST[%d].BUF_CFG mismatch\n", i);
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
            uint32_t scheduler_base_addr;

            /* Validate HTS status directly from registers using CSL_RD */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            /* Validate HTS SCHEDULER_CONTROL for HWA0 and HWA7 */
            if (socInfo->htsRegs != NULL)
            /* LDRA_JUSTIFY_END */
            {
                const CSL_DmpacHtsHwaParams *htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_DOF);
                scheduler_base_addr = (uint32_t)socInfo->htsRegs + htsPrms->schOffset;

                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                /* Validate HTS SCHEDULER_CONTROL for HWA0 */
                if ((uint32_t)FVID2_SOK !=
                (CSL_REG32_RD(scheduler_base_addr) & CSL_HTS_HWA0_SCHEDULER_CONTROL_STATE_MASK))
                {
                    GT_0trace(VhwaDofTrace, GT_ERR, "StatusRegValidate: htsRegs->HWA0_SCHEDULER_CONTROL mismatch\n");
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
                            GT_0trace(VhwaDofTrace, GT_ERR, "StatusRegValidate: htsRegs->HWA0_WDTIMER mismatch\n");
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
                htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_FOCO1);
                scheduler_base_addr = (uint32_t)socInfo->htsRegs + htsPrms->schOffset;
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                /* Validate HTS SCHEDULER_CONTROL for HWA7 */
                if ((uint32_t)FVID2_SOK !=
                    (CSL_REG32_RD(scheduler_base_addr) & CSL_HTS_HWA7_SCHEDULER_CONTROL_STATE_MASK))
                    {
                        GT_0trace(VhwaDofTrace, GT_ERR, "StatusRegValidate: htsRegs->HWA7_SCHEDULER_CONTROL mismatch\n");
                        status = FVID2_EFAIL;
                    }
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
                    /* Validate HTS WD_TIMER status for HWA7 */
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    The test framework does not support the configuration required to trigger this error scenario.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if ((uint32_t)FVID2_SOK !=
                        (CSL_REG32_RD(scheduler_base_addr + HWA_WDTIMER_OFFSET) & CSL_HTS_HWA7_WDTIMER_WDTIMER_STATUS_MASK))
                        {
                            GT_0trace(VhwaDofTrace, GT_ERR, "StatusRegValidate: htsRegs->HWA7_WDTIMER mismatch\n");
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
                /* Validate HTS PROD_COUNT and PA_PRODCOUNT for all DOF_FOCO0 producers in a single loop */
                if (status == FVID2_SOK)
                /* LDRA_JUSTIFY_END */
                {
                    uint32_t prod_count_addr;

                    for (uint32_t i = 0; i < CSL_HTS_NUM_DOF_FOCO0_PRODUCERS; i++)
                    {
                        if (i < CSL_HTS_NUM_DOF_PRODUCERS)
                        {
                            /* DOF producers */
                            htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_DOF);
                            prod_count_addr = (uint32_t)socInfo->htsRegs + htsPrms->prodOffset[i];
                        }
                        else
                        {
                            /* FOCO0 producers */
                            htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_FOCO1);
                            /* For FOCO0, index offset from CSL_HTS_NUM_DOF_PRODUCERS */
                            uint32_t foco_idx = i - CSL_HTS_NUM_DOF_PRODUCERS;
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
                        if ((expected->htsStatus.PROD_COUNT[i] & CSL_HTS_HWA0_PROD0_COUNT_COUNT_MASK) !=
                            (CSL_REG32_RD(prod_count_addr + HWA_PROD_COUNT_OFFSET) & CSL_HTS_HWA0_PROD0_COUNT_COUNT_MASK))
                        {
                            if (i < CSL_HTS_NUM_DOF_PRODUCERS)
                            {
                                GT_1trace(VhwaDofTrace, GT_ERR, "StatusRegValidate: htsRegs->DOF_PROD_COUNT[%d] mismatch\n", i);
                            }
                            else
                            {
                                GT_1trace(VhwaDofTrace, GT_ERR, "StatusRegValidate: htsRegs->FOCO0_PROD_COUNT[%d] mismatch\n", i);
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
                        if ((uint32_t)FVID2_SOK != (CSL_REG32_RD(prod_count_addr + HWA_PROD_PA_PROD_COUNT_OFFSET) & CSL_HTS_HWA0_PA0_PRODCOUNT_PA_PSCOUNT_MASK))
                        {
                            if (i < CSL_HTS_NUM_DOF_PRODUCERS)
                            {
                                GT_1trace(VhwaDofTrace, GT_ERR, "StatusRegValidate: htsRegs->DOF_PA_PROD_COUNT[%d] mismatch\n", i);
                            }
                            else
                            {
                                GT_1trace(VhwaDofTrace, GT_ERR, "StatusRegValidate: htsRegs->FOCO0_PA_PROD_COUNT[%d] mismatch\n", i);
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
                    /* Validate PROD_DMA_SCHEDULER_CONTROL and PROD_DMA_COUNT for all DOF_FOCO0_PRODUCERS_DMA in a loop */
                    for (uint32_t i = 0; i < CSL_HTS_NUM_DOF_FOCO0_PRODUCERS_DMA; i++)
                    {
                        uint32_t prod_dma_addr;
                        if (i < CSL_HTS_NUM_DOF_PRODUCERS_DMA)
                        {
                            /* DOF DMA producers */
                            htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_DOF);
                            prod_dma_addr = (uint32_t)socInfo->htsRegs + htsPrms->dmaProdOffset[i];
                        }
                        else
                        {
                            /* FOCO0 DMA producers */
                            htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_FOCO1);
                            uint32_t foco_dma_idx = i - CSL_HTS_NUM_DOF_PRODUCERS_DMA;
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
                            (CSL_REG32_RD(prod_dma_addr) & CSL_HTS_DMA0_SCHEDULER_CONTROL_STATE_MASK))
                        {
                            if (i < CSL_HTS_NUM_DOF_PRODUCERS_DMA)
                            {
                                GT_1trace(VhwaDofTrace, GT_ERR, "StatusRegValidate: htsRegs->DOF_PROD_DMA_SCHEDULER_CONTROL[%d] mismatch\n", i);
                            }
                            else
                            {
                                GT_1trace(VhwaDofTrace, GT_ERR, "StatusRegValidate: htsRegs->FOCO0_PROD_DMA_SCHEDULER_CONTROL[%d] mismatch\n", i - CSL_HTS_NUM_DOF_PRODUCERS_DMA);
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
                        if ((expected->htsStatus.PROD_DMA_COUNT[i] & CSL_HTS_DMA0_PROD0_COUNT_COUNT_MASK) !=
                            (CSL_REG32_RD(prod_dma_addr + HWA_DMA_PROD_COUNT_OFFSET) & CSL_HTS_DMA0_PROD0_COUNT_COUNT_MASK))
                        {
                            if (i < CSL_HTS_NUM_DOF_PRODUCERS_DMA)
                            {
                                GT_1trace(VhwaDofTrace, GT_ERR, "StatusRegValidate: htsRegs->DOF_PROD_DMA_COUNT[%d] mismatch\n", i);
                            }
                            else
                            {
                                GT_1trace(VhwaDofTrace, GT_ERR, "StatusRegValidate: htsRegs->FOCO0_PROD_DMA_COUNT[%d] mismatch\n", i - CSL_HTS_NUM_DOF_PRODUCERS_DMA);
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
                    /* Validate CONSUMER_DMA_SCHEDULER_CONTROL and CONSUMER_DMA_COUNT for all DOF_FOCO0_CONSUMERS_DMA in a loop */
                    for (uint32_t i = 0; i < CSL_HTS_NUM_DOF_FOCO0_CONSUMERS_DMA; i++)
                    {
                        uint32_t cons_dma_addr;
                        if (i < CSL_HTS_NUM_DOF_CONSUMERS_DMA)
                        {
                            /* DOF DMA consumers */
                            htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_DOF);
                            cons_dma_addr = (uint32_t)socInfo->htsRegs + htsPrms->dmaConsOffset[i];
                        }
                        else
                        {
                            /* FOCO0 DMA consumers */
                            htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_FOCO1);
                            uint32_t foco_dma_idx = i - CSL_HTS_NUM_DOF_CONSUMERS_DMA;
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
                            (CSL_REG32_RD(cons_dma_addr) & CSL_HTS_DMA240_SCHEDULER_CONTROL_STATE_MASK))
                        {
                            if (i < CSL_HTS_NUM_DOF_CONSUMERS_DMA)
                            {
                                GT_1trace(VhwaDofTrace, GT_ERR, "StatusRegValidate: htsRegs->DOF_CONSUMER_DMA_SCHEDULER_CONTROL[%d] mismatch\n", i);
                            }
                            else
                            {
                                GT_1trace(VhwaDofTrace, GT_ERR, "StatusRegValidate: htsRegs->FOCO0_CONSUMER_DMA_SCHEDULER_CONTROL[%d] mismatch\n", i - CSL_HTS_NUM_DOF_CONSUMERS_DMA);
                            }
                            status = FVID2_EFAIL;
                        }
                        /* LDRA_JUSTIFY_END */
                    }
                }
            }
        }
    }

    return status;
}

int32_t vhwaM2mDofSetDefaultGoldenRegMemValues(const Vhwa_M2mDofHandleObj *hObj, const Vhwa_M2mDofInstObj *instObj)
{
    VhwaDmpacDofSocReadBack *goldenReg = NULL;
    int32_t status = FVID2_SOK;
    uint32_t reset_val = 0U;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
	Rationale: The component level negative test framework and test applications cannot reach this portion.
	The parameters are pre-validated by the caller before the control reaches here.
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

    return status;
}

int32_t Vhwa_m2mDofConfigRegReadback(const Vhwa_M2mDofHandleObj *hObj, const Vhwa_M2mDofInstObj *instObj)
{
    int32_t status = FVID2_SOK;
    CSL_dmpac_dofRegs         *dofRegs   = NULL;
    VhwaDmpacDofSocReadBack   *readback  = NULL;
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
        dofRegs   = instObj->socInfo.dofRegs;
        readback  = hObj->configRegMemPrms.configRegReadbackPtr;
        focoRegs  = instObj->socInfo.dmpacFocoRegs;
        lseRegs   = instObj->socInfo.lseRegs;
    }

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == readback) || (NULL == dofRegs) || (NULL == focoRegs) || (NULL == lseRegs))
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        /* Read back DOF core registers.
         * Note: Reserved fields (Resv_*) are not read back as they are not used or required for configuration validation.
         */
        readback->dofRegs.RES         = CSL_REG32_RD(&dofRegs->RES);
        readback->dofRegs.SR          = CSL_REG32_RD(&dofRegs->SR);
        readback->dofRegs.CR          = CSL_REG32_RD(&dofRegs->CR);
        readback->dofRegs.SOF         = CSL_REG32_RD(&dofRegs->SOF);
        readback->dofRegs.MSFR        = CSL_REG32_RD(&dofRegs->MSFR);
        readback->dofRegs.CSCFGR      = CSL_REG32_RD(&dofRegs->CSCFGR);
        readback->dofRegs.CFGWBASE    = CSL_REG32_RD(&dofRegs->CFGWBASE);
        readback->dofRegs.CFGWWIDTH   = CSL_REG32_RD(&dofRegs->CFGWWIDTH);
        readback->dofRegs.CFGWHEIGHT  = CSL_REG32_RD(&dofRegs->CFGWHEIGHT);
        readback->dofRegs.RFGWBASE    = CSL_REG32_RD(&dofRegs->RFGWBASE);
        readback->dofRegs.RFGWWIDTH   = CSL_REG32_RD(&dofRegs->RFGWWIDTH);
        readback->dofRegs.RFGWHEIGHT  = CSL_REG32_RD(&dofRegs->RFGWHEIGHT);
        readback->dofRegs.SPBUFBASE   = CSL_REG32_RD(&dofRegs->SPBUFBASE);
        readback->dofRegs.TPBUFBASE   = CSL_REG32_RD(&dofRegs->TPBUFBASE);
        readback->dofRegs.BMBUFBASE   = CSL_REG32_RD(&dofRegs->BMBUFBASE);
        readback->dofRegs.FVBUFBASE   = CSL_REG32_RD(&dofRegs->FVBUFBASE);
        readback->dofRegs.PSA_CTRL    = CSL_REG32_RD(&dofRegs->PSA_CTRL);

        /* Reserved bytes (Resv_*) in CSL_dmpac_dofRegs are intentionally not read. */

        for (uint32_t i = 0; i < DOF_NUM_DECISION_TREES; i++)
        {
            readback->dofRegs.TREE[i].TH0   = CSL_REG32_RD(&dofRegs->TREE[i].TH0);
            readback->dofRegs.TREE[i].TH10  = CSL_REG32_RD(&dofRegs->TREE[i].TH10);
            readback->dofRegs.TREE[i].TH11  = CSL_REG32_RD(&dofRegs->TREE[i].TH11);
            readback->dofRegs.TREE[i].WT00  = CSL_REG32_RD(&dofRegs->TREE[i].WT00);
            readback->dofRegs.TREE[i].WT01  = CSL_REG32_RD(&dofRegs->TREE[i].WT01);
            readback->dofRegs.TREE[i].WT10  = CSL_REG32_RD(&dofRegs->TREE[i].WT10);
            readback->dofRegs.TREE[i].WT11  = CSL_REG32_RD(&dofRegs->TREE[i].WT11);
            readback->dofRegs.TREE[i].FIDS  = CSL_REG32_RD(&dofRegs->TREE[i].FIDS);
            /* Reserved bytes in TREE struct are not read. */
        }

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

            for (uint32_t i = 0U; i < CSL_LSE_DOF_MAX_INPUT_CH; i++)
            {
                readback->lseRegs.SRC[i].CFG        = CSL_REG32_RD(&lseRegs->SRC[i].CFG);
                readback->lseRegs.SRC[i].FRAME_SIZE = CSL_REG32_RD(&lseRegs->SRC[i].FRAME_SIZE);
                readback->lseRegs.SRC[i].BUF_ATTR   = CSL_REG32_RD(&lseRegs->SRC[i].BUF_ATTR);
                for (uint32_t j = 0U; j < CSL_LSE_DOF_MAX_OUTPUT_CH; j++)
                {
                    readback->lseRegs.SRC[i].BUF_BA[j] = CSL_REG32_RD(&lseRegs->SRC[i].BUF_BA[j]);
                }
            }

            for (uint32_t i = 0U; i < CSL_LSE_DOF_MAX_OUTPUT_CH; i++)
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
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((readback != NULL) && (instObj != NULL))
    /* LDRA_JUSTIFY_END */
    {
        status = vhwaM2mDofUpdateConfigRegGroup(readback, instObj, hObj);
    }

    return status;
}

int32_t vhwaM2mDofUpdateConfigRegGroup(VhwaDmpacDofSocReadBack *RegVal, const Vhwa_M2mDofInstObj *instObj, const Vhwa_M2mDofHandleObj *hObj)
{
    int32_t status = FVID2_SOK;
    volatile CSL_dmpac_intd_cfgRegs *intdRegs = NULL;
    volatile CSL_htsRegs *htsRegs = NULL;
    uint32_t              pyrLvl;
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
        pyrLvl = hObj->curPyrLvl;

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
                DMPAC_MODULE_DOF, (uint32_t)UTRUE);
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
             * VHWA_DMPAC_INTD_DOF_INTR_REG, VHWA_DMPAC_INTD_WDTIMER_INTR_REG) can change in future.
             * Keeping this as explicit assignments ensures maintainability if the macros or their order change.
             */

            /* 0 -> VHWA_DMPAC_INTD_DOF_INTR_REG */
            RegVal->dmpacDofIntd.ENABLE_REG_LEVEL_DMPAC_OUT[vhwaIrqNum][counter] =
                CSL_REG32_RD(&intdRegs->ENABLE_REG_LEVEL_DMPAC_OUT[vhwaIrqNum][VHWA_DMPAC_INTD_DOF_INTR_REG]);
            RegVal->dmpacDofIntd.ENABLE_CLR_REG_LEVEL_DMPAC_OUT[vhwaIrqNum][counter] =
                CSL_REG32_RD(&intdRegs->ENABLE_CLR_REG_LEVEL_DMPAC_OUT[vhwaIrqNum][VHWA_DMPAC_INTD_DOF_INTR_REG]);
            counter += 1U;

            /* 1 -> VHWA_DMPAC_INTD_PIPE_INTR_REG */
            RegVal->dmpacDofIntd.ENABLE_REG_LEVEL_DMPAC_OUT[vhwaIrqNum][counter] =
                CSL_REG32_RD(&intdRegs->ENABLE_REG_LEVEL_DMPAC_OUT[vhwaIrqNum][VHWA_DMPAC_INTD_PIPE_INTR_REG]);
            RegVal->dmpacDofIntd.ENABLE_CLR_REG_LEVEL_DMPAC_OUT[vhwaIrqNum][counter] =
                CSL_REG32_RD(&intdRegs->ENABLE_CLR_REG_LEVEL_DMPAC_OUT[vhwaIrqNum][VHWA_DMPAC_INTD_PIPE_INTR_REG]);
            counter += 1U;

            /* 2 -> VHWA_DMPAC_INTD_WDTIMER_INTR_REG */
            RegVal->dmpacDofIntd.ENABLE_REG_LEVEL_DMPAC_OUT[vhwaIrqNum][counter] =
                CSL_REG32_RD(&intdRegs->ENABLE_REG_LEVEL_DMPAC_OUT[vhwaIrqNum][VHWA_DMPAC_INTD_WDTIMER_INTR_REG]);
            RegVal->dmpacDofIntd.ENABLE_CLR_REG_LEVEL_DMPAC_OUT[vhwaIrqNum][counter] =
                CSL_REG32_RD(&intdRegs->ENABLE_CLR_REG_LEVEL_DMPAC_OUT[vhwaIrqNum][VHWA_DMPAC_INTD_WDTIMER_INTR_REG]);
        }

        /* Accumulate HTS registers */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (instObj->socInfo.htsRegs != NULL)
        /* LDRA_JUSTIFY_END */
        {
            uint32_t scheduler_base_addr;
            htsRegs = instObj->socInfo.htsRegs;
            const CSL_DmpacHtsHwaParams *htsPrms = NULL;

            /* Read the HTS pipeline registers */
            RegVal->dofHts.PIPELINE_CONTROL[0U] = CSL_REG32_RD(&htsRegs->PIPELINE_CONTROL[(uint32_t)hObj->htsCfg[pyrLvl].pipeline]);
            RegVal->dofHts.PIPELINE_CONTROL[1U]  = CSL_REG32_RD(&htsRegs->PIPELINE_CONTROL[(uint32_t)hObj->focoHtsCfg[pyrLvl].pipeline]);

            /* Read the HTS HWA scheduler registers */
            /* Loop for both DOF and FOCO1 HWA schedulers */
            for (uint32_t schIdx = 0; schIdx < CSL_HTS_NUM_DOF_SCHEDULERS; schIdx++)
            {
                if (schIdx == 0U)
                {
                    htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_DOF);
                }
                else
                {
                    htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_FOCO1);
                }
                scheduler_base_addr = (uint32_t)instObj->socInfo.htsRegs + htsPrms->schOffset;

                {
                    reg_val = CSL_REG32_RD(scheduler_base_addr + HWA_SCH_CONTROL_OFFSET);
                    RegVal->dofHts.SCHEDULER_CONTROL[schIdx] =
                        (reg_val & CSL_HTS_HWA0_SCHEDULER_CONTROL_PAUSE_MASK) |
                        (reg_val & CSL_HTS_HWA0_SCHEDULER_CONTROL_STRM_EN_MASK) |
                        (reg_val & CSL_HTS_HWA0_SCHEDULER_CONTROL_PIPELINE_NUM_MASK) |
                        (reg_val & CSL_HTS_HWA0_SCHEDULER_CONTROL_SCH_EN_MASK) |
                        (reg_val & CSL_HTS_HWA0_SCHEDULER_CONTROL_DEBUG_RDY_MASK);
                }

                RegVal->dofHts.HOP[schIdx] = CSL_REG32_RD(scheduler_base_addr + HWA_HOP_OFFSET);

                /* Only keep enable and mode bits from WD_TIMER register */
                {
                    reg_val = CSL_REG32_RD(scheduler_base_addr + HWA_WDTIMER_OFFSET);
                    RegVal->dofHts.WDTIMER[schIdx] =
                        (reg_val & CSL_HTS_HWA0_WDTIMER_WDTIMER_EN_MASK) |
                        (reg_val & CSL_HTS_HWA0_WDTIMER_WDTIMER_MODE_MASK);
                }

                RegVal->dofHts.BW_LIMITER[schIdx] = CSL_REG32_RD(scheduler_base_addr + HWA_BW_LIMITER_OFFSET);
            }

            /* Readback for consumer control */
            uint32_t cons_ctrl_addr;

            for (uint32_t i = 0; i < CSL_HTS_NUM_DOF_FOCO0_CONSUMERS; i++)
            {
                if (i < CSL_HTS_NUM_DOF_CONSUMERS)
                {
                    /* DOF consumers */
                    htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_DOF);
                    cons_ctrl_addr = (uint32_t)instObj->socInfo.htsRegs + htsPrms->consOffset[i];
                }
                else
                {
                    /* FOCO0 consumers */
                    htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_FOCO1);
                    /* For FOCO0, index offset from CSL_HTS_NUM_DOF_CONSUMERS */
                    uint32_t foco_idx = i - CSL_HTS_NUM_DOF_CONSUMERS;
                    cons_ctrl_addr = (uint32_t)instObj->socInfo.htsRegs + htsPrms->consOffset[foco_idx];
                }

                RegVal->dofHts.CONS_CONTROL[i] = CSL_REG32_RD(cons_ctrl_addr + HWA_CONS_CONTROL_OFFSET);
            }

            /* Readback for producers control */
            uint32_t prod_ctrl_addr;

            for (uint32_t i = 0; i < CSL_HTS_NUM_DOF_FOCO0_PRODUCERS; i++)
            {
                if (i < CSL_HTS_NUM_DOF_PRODUCERS)
                {
                    /* DOF producers */
                    htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_DOF);
                    prod_ctrl_addr = (uint32_t)instObj->socInfo.htsRegs + htsPrms->prodOffset[i];
                }
                else
                {
                    /* FOCO0 producers */
                    htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_FOCO1);
                    /* For FOCO0, index offset from CSL_HTS_NUM_DOF_PRODUCERS */
                    uint32_t foco_idx = i - CSL_HTS_NUM_DOF_PRODUCERS;
                    prod_ctrl_addr = (uint32_t)instObj->socInfo.htsRegs + htsPrms->prodOffset[foco_idx];
                }

                RegVal->dofHts.PROD_CONTROL[i] = CSL_REG32_RD(prod_ctrl_addr + HWA_PROD_CONTROL_OFFSET);
                RegVal->dofHts.PROD_BUF_CONTROL[i] = CSL_REG32_RD(prod_ctrl_addr + HWA_PROD_BUF_CONTROL_OFFSET);

                /* Only keep postload and preload bits from PROD_COUNT register */
                reg_val = CSL_REG32_RD(prod_ctrl_addr + HWA_PROD_COUNT_OFFSET);
                RegVal->dofHts.PROD_COUNT[i] =
                    (reg_val & CSL_HTS_HWA0_PROD0_COUNT_COUNT_POSTLOAD_MASK) |
                    (reg_val & CSL_HTS_HWA0_PROD0_COUNT_COUNT_PRELOAD_MASK);

                RegVal->dofHts.PA_CONTROL[i] = CSL_REG32_RD(prod_ctrl_addr + HWA_PROD_PA_CONTROL_OFFSET);

                /* Only keep PA_COUNT_DEC bits from PA_PRODCOUNT register */
                reg_val = CSL_REG32_RD(prod_ctrl_addr + HWA_PROD_PA_PROD_COUNT_OFFSET);
                RegVal->dofHts.PA_PRODCOUNT[i] = (reg_val & CSL_HTS_HWA0_PA0_PRODCOUNT_PA_COUNT_DEC_MASK);
            }

            /* Readback producers DMA */
            uint32_t prod_dma_ctrl_addr;

            for (uint32_t i = 0; i < CSL_HTS_NUM_DOF_FOCO0_PRODUCERS_DMA; i++)
            {
                if (i < CSL_HTS_NUM_DOF_PRODUCERS_DMA)
                {
                    /* DOF DMA producers */
                    htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_DOF);
                    prod_dma_ctrl_addr = (uint32_t)instObj->socInfo.htsRegs + htsPrms->dmaProdOffset[i];
                }
                else
                {
                    /* FOCO0 DMA producers */
                    htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_FOCO1);
                    uint32_t foco_dma_idx = i - CSL_HTS_NUM_DOF_PRODUCERS_DMA;
                    prod_dma_ctrl_addr = (uint32_t)instObj->socInfo.htsRegs + htsPrms->dmaProdOffset[foco_dma_idx];
                }

                /* Read PROD_DMA_SCHEDULER_CONTROL */
                {
                    reg_val = CSL_REG32_RD(prod_dma_ctrl_addr + HWA_DMA_PROD_SCH_CONTROL_OFFSET);
                    RegVal->dofHts.PROD_DMA_SCHEDULER_CONTROL[i] =
                        (reg_val & CSL_HTS_DMA0_SCHEDULER_CONTROL_DMA_CHANNEL_NO_MASK) |
                        (reg_val & CSL_HTS_DMA0_SCHEDULER_CONTROL_DEBUG_RDY_MASK) |
                        (reg_val & CSL_HTS_DMA0_SCHEDULER_CONTROL_PAUSE_MASK) |
                        (reg_val & CSL_HTS_DMA0_SCHEDULER_CONTROL_STRM_EN_MASK) |
                        (reg_val & CSL_HTS_DMA0_SCHEDULER_CONTROL_PIPELINE_NUM_MASK) |
                        (reg_val & CSL_HTS_DMA0_SCHEDULER_CONTROL_SCH_EN_MASK);
                }

                /* Read PROD_DMA_HOP */
                RegVal->dofHts.PROD_DMA_HOP[i] = CSL_REG32_RD(prod_dma_ctrl_addr + HWA_DMA_PROD_HOP_OFFSET);

                /* Read PROD_CONTROL_DMA */
                RegVal->dofHts.PROD_CONTROL_DMA[i] = CSL_REG32_RD(prod_dma_ctrl_addr + HWA_DMA_PROD_CONTROL_OFFSET);

                /* Read PROD_BUF_CONTROL_DMA */
                RegVal->dofHts.PROD_BUF_CONTROL_DMA[i] = CSL_REG32_RD(prod_dma_ctrl_addr + HWA_DMA_PROD_BUF_CONTROL_OFFSET);

                /* Only keep postload and preload bits from PROD_DMA_COUNT register */
                reg_val = CSL_REG32_RD(prod_dma_ctrl_addr + HWA_DMA_PROD_COUNT_OFFSET);
                RegVal->dofHts.PROD_COUNT_DMA[i] =
                    (reg_val & CSL_HTS_DMA0_PROD0_COUNT_COUNT_POSTLOAD_MASK) |
                    (reg_val & CSL_HTS_DMA0_PROD0_COUNT_COUNT_PRELOAD_MASK);
            }

            /* Readback of consumers DMA */
            uint32_t cons_dma_ctrl_addr;

            for (uint32_t i = 0; i < CSL_HTS_NUM_DOF_FOCO0_CONSUMERS_DMA; i++)
            {
                if (i < CSL_HTS_NUM_DOF_CONSUMERS_DMA)
                {
                    /* DOF DMA consumers */
                    htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_DOF);
                    cons_dma_ctrl_addr = (uint32_t)instObj->socInfo.htsRegs + htsPrms->dmaConsOffset[i];
                }
                else
                {
                    /* FOCO0 DMA consumers */
                    htsPrms = GetDmpacHtsParams(CSL_HTS_HWA_SCH_FOCO1);
                    uint32_t foco_dma_idx = i - CSL_HTS_NUM_DOF_CONSUMERS_DMA;
                    cons_dma_ctrl_addr = (uint32_t)instObj->socInfo.htsRegs + htsPrms->dmaConsOffset[foco_dma_idx];
                }

                /* Read CONSUMER_DMA_SCHEDULER_CONTROL */
                {
                    reg_val = CSL_REG32_RD(cons_dma_ctrl_addr + HWA_DMA_CONS_SCH_CONTROL_OFFSET);
                    RegVal->dofHts.CONS_DMA_SCHEDULER_CONTROL[i] =
                        (reg_val & CSL_HTS_DMA240_SCHEDULER_CONTROL_DMA_CHANNEL_NO_MASK) |
                        (reg_val & CSL_HTS_DMA240_SCHEDULER_CONTROL_DEBUG_RDY_MASK) |
                        (reg_val & CSL_HTS_DMA240_SCHEDULER_CONTROL_PAUSE_MASK) |
                        (reg_val & CSL_HTS_DMA240_SCHEDULER_CONTROL_STRM_EN_MASK) |
                        (reg_val & CSL_HTS_DMA240_SCHEDULER_CONTROL_PIPELINE_NUM_MASK) |
                        (reg_val & CSL_HTS_DMA240_SCHEDULER_CONTROL_SCH_EN_MASK);
                }

                /* Read CONSUMER_CONTROL_DMA */
                RegVal->dofHts.CONS_CONTROL_DMA[i] = CSL_REG32_RD(cons_dma_ctrl_addr + HWA_DMA_CONS_CONTROL_OFFSET);
            }
        }
    }

    return status;
}
#endif
