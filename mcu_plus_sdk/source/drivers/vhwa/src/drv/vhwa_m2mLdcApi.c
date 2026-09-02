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
 *  \file vhwa_m2mLdcApi.c
 *
 *  \brief API Implementation
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include "src/drv/vhwa_m2mLdcPriv.h"

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
static Vhwa_M2mLdcHandleObj *vhwaM2mLdcAllocHandleObj(
    const Vhwa_M2mLdcInstObj *instObj);

/**
 * \brief   Create Queues, required for storing pending requests
 *
 * \param   instObj             Instance object.
 *
 * \return  pointer to handle object on success else NULL.
 *
 **/
static int32_t vhwaM2mLdcCreateQueues(Vhwa_M2mLdcInstObj *instObj);

/**
 * \brief   Delete Queues
 *
 * \param   instObj             Instance object.
 *
 **/
static void vhwaM2mLdcDeleteQueues(Vhwa_M2mLdcInstObj *instObj);

/**
 * \brief   Local function to freeup allocated Handle Object and return it
 *          to the pool of free handle objects.
 *          No protection inside the function, Caller should protect
 *          the function call
 *
 * \param   hObj                Handle Object to be freed up.
 *
 **/
static void vhwaM2mLdcFreeHandleObj(Vhwa_M2mLdcHandleObj *hObj);

/**
 * \brief   Function to check given LDC configuration, used in
 *          SET_PARAMS ioctl it first initializes LSE and HTS
 *          config and then uses CSLFL of LSE and HTS to validate
 *          the configuration
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   ldcCfg              LDC configuration to be validated
 *
 * \return  FVID2_SOK if given LDC cofiguration is valid, error code otherwise.
 *
 **/
static int32_t vhwaM2mLdcCheckLdcCfg(const Vhwa_M2mLdcInstObj *instObj,
    Vhwa_M2mLdcHandleObj *hObj, const Ldc_Config *ldcCfg);

/**
 * \brief   Function to calculate required block size for the
 *          given LDC configuration
 *
 * \param   ldcCfg              LDC configuration to be validated
 * \param   outId               Output ID
 *
 * \return  required block size in bytes.
 *
 **/
static uint32_t vhwaM2mLdcCalcSl2MemSize(const Ldc_Config *cfg,
                                    uint32_t outId);

/**
 * \brief   Function to get block width and height for the given
 *          LDC configuration
 *
 * \param   ldcCfg              LDC configuration, input parameter
 * \param   blockWidth          calculated block width, output parameter
 * \param   blockHeight         calculated block height, output parameter
 *
 * \return  required block size in bytes.
 *
 **/
static void vhwaM2mLdcGetBlockSize(const Ldc_Config *cfg,
                                   uint32_t *blockWidth,
                                   uint32_t *blockHeight);

/**
 * \brief   Function to initialize Region parameters from the given channel
 *
 * \param   chPrms              Channel params,
 *                              Region params from this are updated
 * \param   ldcCfg              LDC Configuration
 *
 **/
static void vhwaM2mLdcInitRegParams(Vhwa_M2mLdcChParams *chPrms,
    Ldc_Config *ldcCfg);

/**
 * \brief   Function to initialize Region parameters from the
 *          given chroma channel. Created a separate function so that
 *          height division is clear
 *
 * \param   chPrms              Channel params,
 *                              Region params from this are updated
 * \param   ldcCfg              LDC Configuration
 *
 **/
static void vhwaM2mLdcInitRegParamsForChroma(Vhwa_M2mLdcChParams *chPrms,
    Ldc_Config *ldcCfg);

/**
 * \brief   Function to calculate maximum block pitch out of enabled regions
 *          in the given channel.
 *          The output is stored in the channel parameters.
 *
 * \param   chPrms              Channel params,
 *
 **/
static void vhwaM2mLdcCalcMaxBlockPitch(Vhwa_M2mLdcChParams *chPrms);

/**
 * \brief   Function to calculate possible number of blocks,
 *          given the number of SL2 lines and block height in each region.
 *          It stored the number of blocks in the channel parameters.
 *
 * \param   chPrms              Channel params
 *
 **/
static void vhwaM2mLdcCalcNumSl2Blocks(Vhwa_M2mLdcChParams *chPrms);

/**
 * \brief   Function to calculate transfer record parameters for each channel.
 *          It internally call another API to calculate TR
 *          parameters for each region
 *
 * \param   instObj             Instance Object
 * \param   chPrms              Channel params
 *
 **/
static void vhwaM2mLdcCalcChTrParams(const Vhwa_M2mLdcInstObj *instObj,
    Vhwa_M2mLdcHandleObj *hObj);

/**
 * \brief   Function to calculate transfer record parameters for
 *          each region. Calculates icnts, dims etc. so that it can
 *          be directly used in the TR creation.
 *          It also calculates the number of TRs required for each region.
 *
 * \param   chPrms              Channel params
 *
 **/
static void vhwaM2mLdcCalcRegionTrParams(Vhwa_M2mLdcChParams *chPrms);

/**
 * \brief   Based on the given LDC config, it initializes LSE configuration.
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   ldcCfg              LDC Configuration
 *
 **/
static void vhwaM2mLdcSetLseCfg(const Vhwa_M2mLdcInstObj *instObj,
    Vhwa_M2mLdcHandleObj *hObj, const Ldc_Config *ldcCfg);

/**
 * \brief   Based on the given LDC config, it initializes HTS configuration.
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   ldcCfg              LDC Configuration
 *
 **/
static void vhwaM2mLdcSetHtsCfg(const Vhwa_M2mLdcInstObj *instObj,
    Vhwa_M2mLdcHandleObj *hObj, const Ldc_Config *ldcCfg);

/*  */
/**
 * \brief   Implementation of SET_PARAMS ioctl.
 *          It uses CheckLdcCfg to validate the config.
 *          If it is valid, copies the config into handle object
 *          If it is invalid, it reverts LSE/HTS config to known valid config
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   ldcCfg              LDC configuration to be set
 *
 **/
static int32_t vhwaM2mLdcSetParams(const Vhwa_M2mLdcInstObj *instObj,
    Vhwa_M2mLdcHandleObj *hObj, const Ldc_Config *ldcCfg);

/**
 * \brief   Function to configure LDS, LSE and HTS in the HW.
 *
 * \param   instObj             Instance Object, used for getting base address
 * \param   qObj                Queue Object, used for getting handle object
 *
 **/
static int32_t vhwaM2mLdcSetConfigInHW(const Vhwa_M2mLdcInstObj *instObj,
    const Vhwa_M2mLdcQueueObj *qObj);

/**
 * \brief   Minimal function to submit request to hw and start LDC operation.
 *          It first sets address in the TR, Submits it to the ring and
 *          starts the pipeline.
 *          It does not start currently LDC, HTS schedulers.
 *
 * \param   instObj             Instance Object, used for getting base address
 * \param   qObj                Queue Object, used for getting handle object
 *
 **/
static int32_t vhwaM2mLdcSubmitRequest(Vhwa_M2mLdcInstObj *instObj,
    Vhwa_M2mLdcQueueObj *qObj);

/**
 * \brief   Set LDC and WDTIMER interrupt enable/disable in INTD registers.
 *
 * This function configures the interrupt enable/disable settings in the VPAC INTD
 * registers for LDC and Watchdog Timer error events, based on the handle object's
 * error event parameters.
 *
 * \param   vhwaIrqNum      VHWA IRQ number to configure.
 * \param   intdRegs        Pointer to VPAC INTD register overlay.
 * \param   hObj            Pointer to the LDC handle object containing error event params.
 *
 * \return  FVID2_SOK on success, else FVID2 error code.
 */
static int32_t Vhwa_m2mLdcSetIntrInHW(uint32_t vhwaIrqNum,
                                volatile CSL_vpac_intd_cfgRegs  *intdRegs,
                                const Vhwa_M2mLdcHandleObj *hObj);

/**
 * \brief   Function to set input and output addresses in LDC and in TR
 *          It uses buffer addresses from the input and output frame lists
 *          set the them in the enable output channels.
 *
 * \param   instObj             Instance Object, used for getting base address
 * \param   hObj                Handle Object
 * \param   socInfo             Soc Information, used to get base
 *                              address for LDC
 * \param   inFrmList           Input Frame List
 * \param   outFrmList          Output Frame List
 *
 **/
static void vhwaM2mLdcSetAddress(Vhwa_M2mLdcHandleObj *hObj,
    Ldc_SocInfo *socInfo, const Fvid2_FrameList *inFrmList,
    const Fvid2_FrameList *outFrmList);

static int32_t vhwaM2mLdcAllocSl2(Vhwa_M2mLdcInstObj *instObj,
    const Vhwa_M2mLdcSl2AllocPrms *sl2AllocPrms);

/**
 * \brief   Local function to check LDC Create params like Driver Id and Instance Id.
 *
 * \param   drvId               Driver ID
 * \param   drvInstId           Instance ID
 *
 *  \return FVID2_SOK if successful, else suitable error code
 **/
static int32_t vhwaM2mLdcCheckCreatePrms(uint32_t drvId, uint32_t drvInstId);
static int32_t vhwa_m2mLdcSetEeParams(const Vhwa_M2mLdcInstObj *instObj,
    Vhwa_M2mLdcHandleObj *hObj, const Ldc_ErrEventParams *eePrms);
static int32_t vhwa_m2mLdcSetWdTimerEeParams(const Vhwa_M2mLdcInstObj *instObj,
    Vhwa_M2mLdcHandleObj *hObj, const Ldc_WdTimerErrEventParams *wdTimerEePrms);

/**
 * @brief Validates the LDC (Lens Distortion Correction) status registers against expected values.
 *
 * This function compares the current LDC status register values with the expected values
 * provided in the 'expected' parameter. It uses the SoC-specific information from 'socInfo'
 * to perform the validation according to the hardware configuration.
 *
 * @param expected Pointer to a LdcStatusRegisterGroup structure containing the expected
 *                 status register values.
 * @param socInfo  Pointer to a Ldc_SocInfo structure containing SoC-specific information
 *                 required for validation.
 *
 * @return Returns 0 (FVID2_SOK) if the status registers match the expected values,
 *         or a negative error code if there is a mismatch or an error occurs.
 */
int32_t Vhwa_m2mLdcStatusRegValidate(Vhwa_M2mLdcHandleObj *hObj, const Ldc_SocInfo *socInfo);

/**
 * \brief   Updates the LdcStatusRegisterGroup structure in the handle object with expected status register values.
 *
 * This function calculates and populates the expected values for various status registers
 * based on the current LDC configuration. It updates:
 * - LDC ERR_STATUS and DEBUG_STATUS registers (expected to be 0 for no errors)
 * - HTS PROD_COUNT, PA_PRODCOUNT, and PROD_DMA_COUNT registers based on HTS configuration
 *
 * \param   hObj    Pointer to the Vhwa_M2mLdcHandleObj whose statusRegs should be updated.
 *
 * \return  FVID2_SOK on success, else error code
 */
static int32_t vhwaM2mLdcUpdateStatusRegGroup(Vhwa_M2mLdcHandleObj *hObj);

/* Implementation of FVID2 APIs */

/**
 * \brief   FVID2 Create Function.
 *
 * \param   instObj             Instance object.
 *
 * \return  FVID2 Driver Handle.
 *
 **/
Fdrv_Handle vhwa_m2mLdcCreate(UInt32 drvId, UInt32 drvInstId,
    Ptr createArgs, Ptr createStatusArgs, const Fvid2_DrvCbParams *cbPrms);

/**
 * \brief   FVID2 Delete Function.
 *
 * \param   handle              FVID2 driver handle.
 *
 * \return  FVID2_SOK on success, else FVID2 error code
 *
 **/
Int32 vhwa_m2mLdcDelete(Fdrv_Handle handle, Ptr deleteArgs);

/**
 * \brief   FVID2 Control Function.
 *
 * \param   handle              FVID2 driver handle.
 *
 * \return  FVID2_SOK on success, else FVID2 error code
 *
 **/
Int32 vhwa_m2mLdcControl(Fdrv_Handle handle, UInt32 cmd, Ptr cmdArgs,
    Ptr cmdStatusArgs);

/**
 * \brief   FVID2 Process Request function.
 *
 * \param   handle              FVID2 driver handle.
 *
 * \return  FVID2_SOK on success, else FVID2 error code
 *
 **/
Int32 vhwa_m2mLdcProcessReq(Fdrv_Handle handle, Fvid2_FrameList *inFrmList,
    Fvid2_FrameList *outFrmList, uint32_t timeout);

/**
 * \brief   FVID2 Get Process Request function.
 *
 * \param   handle              FVID2 driver handle.
 *
 * \return  FVID2_SOK on success, else FVID2 error code
 *
 **/
Int32 vhwa_m2mLdcGetProcessReq(Fdrv_Handle handle,
    Fvid2_FrameList *inFrmList, Fvid2_FrameList *outFrmList,
    UInt32 timeout);

static void vhwaM2mLdcCalcHtsDepth(Vhwa_M2mLdcHandleObj *hObj);

static void vhwaM2mLdcInitChParamsFromLdcConfig(const Vhwa_M2mLdcInstObj *instObj,
    Vhwa_M2mLdcHandleObj *hObj);

static void vhwaM2mLdcCalcChSl2Params(const Vhwa_M2mLdcInstObj *instObj,
    Vhwa_M2mLdcHandleObj *hObj);

/**
 * \brief   Reconfigure LDC MMR registers as needed for the current handle/queue.
 *
 * \param   instObj   Instance Object.
 * \param   hObj      Handle Object.
 * \param   qObj      Queue Object.
 *
 * \return  FVID2_SOK on success, else FVID2 error code.
 */
static int32_t Vhwa_m2mLdcReconfigReinitReg(Vhwa_M2mLdcInstObj *instObj,
                                            const Vhwa_M2mLdcHandleObj *hObj,
                                            const Vhwa_M2mLdcQueueObj *qObj);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

Vhwa_M2mLdcHandleObj gM2mLdcHandleObj[VHWA_M2M_LDC_MAX_HANDLES];
Vhwa_M2mLdcInstObj   gM2mLdcInstObj;

Fvid2_DrvOps gM2mLdcFvid2DrvOps = {
    FVID2_VHWA_M2M_LDC_DRV_ID,
    /**< Unique driver Id. */
    vhwa_m2mLdcCreate,
    /**< FVID2 create function pointer. */
    vhwa_m2mLdcDelete,
    /**< FVID2 delete function pointer. */
    vhwa_m2mLdcControl,
    /**< FVID2 control function pointer. */
    NULL, NULL,
    /**< FVID2 queue function pointer. */
    vhwa_m2mLdcProcessReq,
    /**< FVID2 process request function pointer. */
    vhwa_m2mLdcGetProcessReq,
    /**< FVID2 get processed request function pointer. */
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t Vhwa_m2mLdcInit(Vhwa_M2mLdcInitParams *initPrms)
{
    int32_t             status = FVID2_SOK;
    uint32_t            cnt;
    #if !defined(MCU_PLUS_SDK)
    SemaphoreP_Params   params;
    #endif
    Vhwa_M2mLdcInstObj *instObj = NULL;

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
        instObj = &gM2mLdcInstObj;

        /* Reset all instance object to 0x0 */
        Fvid2Utils_memset(instObj, 0U, sizeof (gM2mLdcInstObj));

        /* Mark pool flags as free */
        for (cnt = 0U; cnt < VHWA_M2M_LDC_MAX_HANDLES; cnt++)
        {
            gM2mLdcHandleObj[cnt].isUsed = (uint32_t) UFALSE;
        }

        /* Set HTS pipeline */
        instObj->pipeline = VHWA_M2M_LDC_HTS_PIPELINE;

        /* Initialize stalled flag to FALSE */
        instObj->isStalled = (uint32_t)UFALSE;

        Ldc_getSocInfo(&instObj->socInfo);

        status = Fvid2_registerDriver(&gM2mLdcFvid2DrvOps);
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
        Effect on this unit: If the control reaches here, our code base is expected to print the error status.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            GT_0trace(VhwaLdcTrace, GT_ERR,
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
            /* LDRA_JUSTIFY_END */
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
                GT_0trace(VhwaLdcTrace, GT_ERR,
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
#if defined VHWA_M2M_VPAC_INSTANCE
#if (VHWA_M2M_VPAC_INSTANCE == 0)
            instObj->hwaLockIdx = VHWA_VPAC0_LDC_LOCK_IDX;
#elif defined(VHWA_M2M_VPAC_INSTANCE) && (VHWA_M2M_VPAC_INSTANCE == 1)
            instObj->hwaLockIdx = VHWA_VPAC1_LDC_LOCK_IDX;
#endif
#endif
            status = Vhwa_commonCreateHwaLock(instObj->hwaLockIdx);
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
            status = vhwaM2mLdcCreateQueues(instObj);
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
               required for LDC output */
            status = Vhwa_m2mLdcUdmaInit(instObj, initPrms);
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
                GT_0trace(VhwaLdcTrace, GT_ERR,
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

            status = Vhwa_m2mLdcRegisterIsr(instObj);
        }

        /* Init is done, copy init params locally,
           enable VPAC and LDC in VPAC Top */
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
            Fvid2Utils_memcpy(&instObj->initPrms, initPrms,
                sizeof(Vhwa_M2mLdcInitParams));

            /* Enable LDC at VPAC Top*/
            CSL_vpacEnableModule(instObj->socInfo.vpacCntlRegs,
                VPAC_MODULE_LDC, (uint32_t)UTRUE);

            instObj->initDone = (uint32_t)UTRUE;
        }
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
        status = FVID2_EBADARGS;
    }

    if (FVID2_SOK != status)
    {
        Vhwa_m2mLdcDeInit();
    }
    /* LDRA_JUSTIFY_END */

    return (status);
}

void Vhwa_m2mLdcDeInit(void)
{
    Vhwa_M2mLdcInstObj *instObj = NULL;

    instObj = &gM2mLdcInstObj;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to print the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (instObj->openCnt > 0u)
    {
        GT_0trace(VhwaLdcTrace, GT_ERR,
            "Warning: All driver handles are not closed!!\n");
    }
    /* LDRA_JUSTIFY_END */

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((uint32_t)UTRUE == instObj->isSl2AllocDone)
    /* LDRA_JUSTIFY_END */
    {
        Vhwa_m2mLdcFreeSl2();
    }

    /* Stop UDMA channels */
    (void)Vhwa_m2mLdcStopCh(instObj);

    Vhwa_m2mLdcUnregisterIsr(instObj);

    (void)Vhwa_m2mLdcUdmaDeInit(instObj);

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
        (void)Fvid2_unRegisterDriver(&gM2mLdcFvid2DrvOps);
    }

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

    vhwaM2mLdcDeleteQueues(instObj);

    /* Init all global variables to zero */
    Fvid2Utils_memset(instObj, 0U, sizeof (gM2mLdcInstObj));

    instObj->initDone = (uint32_t)UFALSE;
}

int32_t Vhwa_m2mLdcAllocSl2(const Vhwa_M2mLdcSl2AllocPrms *sl2AllocPrms)
{
    int32_t             status = FVID2_SOK;
    Vhwa_M2mLdcInstObj *instObj = NULL;

    instObj = &gM2mLdcInstObj;

    /* Cannot even lock, if init is not done */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((uint32_t)UFALSE == instObj->initDone)
    {
        GT_0trace(VhwaLdcTrace, GT_ERR,
            "Driver init is not done!!\n");
        status = FVID2_EFAIL;
    }
    else
    {
    /* LDRA_JUSTIFY_END */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)UTRUE == instObj->isSl2AllocDone)
        {
            GT_0trace(VhwaLdcTrace, GT_ERR,
                "SL2 Memory is already allocated !!\n");
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */

        /* Still need to check if provided sl2AllocPrms is not null */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL == sl2AllocPrms)
        {
            GT_0trace(VhwaLdcTrace, GT_ERR,
                "SL2 Params is null !!\n");
            status = FVID2_EBADARGS;
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
        /* Lock instance semaphore */
        (void)VhwaAl_SemaphoreP_pend(instObj->lock, SYSTEMP_WAIT_FOREVER);

        status = vhwaM2mLdcAllocSl2(instObj, sl2AllocPrms);

        /* Release instance semaphore */
        (void)VhwaAl_SemaphoreP_post(instObj->lock);
    }

    return (status);
}

void Vhwa_m2mLdcFreeSl2(void)
{
    uint32_t            cnt;
    Vhwa_M2mLdcInstObj *instObj = NULL;

    instObj = &gM2mLdcInstObj;

    /* Lock instance semaphore */
    (void)VhwaAl_SemaphoreP_pend(instObj->lock, SYSTEMP_WAIT_FOREVER);

    for (cnt = 0U; cnt < LDC_MAX_OUTPUT; cnt ++)
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (0U != instObj->sl2Size[cnt])
        /* LDRA_JUSTIFY_END */
        {
            Vhwa_FreeSl2(VHWA_SL2_INST_VPAC);
            instObj->sl2Size[cnt] = 0U;
        }
    }

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
Rationale: The component level negative test framework and test applications CAN reach this portion.
No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
<justification end> */
Vhwa_M2mLdcHandleObj *Vhwa_m2mLdcGetHandleObj(uint32_t cnt)
{
    return &gM2mLdcHandleObj[cnt];
}
/* LDRA_JUSTIFY_END */

int32_t Vhwa_m2mLdcReInit(void)
{
    int32_t                status = FVID2_SOK;
    Vhwa_M2mLdcInstObj    *instObj = NULL;

    /** No lock is required, as reinit is called only when HW is not accessed by any other
     *  handle, so it is ok to access the members here. */
    instObj = &gM2mLdcInstObj;

    /** If isregistered is set, then Vhwa_m2mDofUdmaInit is called and UDMA
     * channels are initialized, so reinit is required. */
    if ((uint32_t)UTRUE == instObj->isRegistered)
    {
        status = Vhwa_m2mLdcStopCh(instObj);

        if(FVID2_SOK == status)
        {
            status = Vhwa_m2mLdcUdmaDeInit(instObj);
        }

        if(FVID2_SOK == status)
        {
            /* Need to unregister ISR, so it can be re-registered in next step */
            Vhwa_m2mLdcUnregisterIsr(instObj);
        }

        if(FVID2_SOK == status)
        {
            /* Now reinitialized channels */
            status = Vhwa_m2mLdcUdmaInit(instObj, &instObj->initPrms);
        }

        if(FVID2_SOK == status)
        {
            /* INTD module also requires to be reinitialized */
            status = Vhwa_m2mLdcRegisterIsr(instObj);
        }

        if (FVID2_SOK == status)
        {
            /* Enable LDC at VPAC Top*/
            CSL_vpacEnableModule(instObj->socInfo.vpacCntlRegs,
                VPAC_MODULE_LDC, (uint32_t)UTRUE);
        }

        if (FVID2_SOK == status)
        {
            /* Now start all channels */
            status = Vhwa_m2mLdcStartCh(instObj);
        }

        if (FVID2_SOK == status)
        {
            /* Enable Pipeline interrupt in INTD */
            Vhwa_enableHtsIntr(instObj->socInfo.vpacIntdRegs,
                instObj->vhwaIrqNum, instObj->pipeline);
        }
        /* Setting last handle to false/null, so that reconfiguration is done */
        if (FVID2_SOK == status)
        {
            instObj->lastHndlObj = NULL;
        }
    }

    return (status);
}


/* ========================================================================== */
/*                          FVID2 Function implementation                     */
/* ========================================================================== */

Fdrv_Handle vhwa_m2mLdcCreate(UInt32 drvId, UInt32 drvInstId,
    Ptr createArgs, Ptr createStatusArgs, const Fvid2_DrvCbParams *cbPrms)
{
    int32_t                 status = FVID2_SOK;
    Vhwa_M2mLdcInstObj     *instObj = NULL;
    Vhwa_M2mLdcHandleObj   *hObj = NULL;
    Vhwa_M2mLdcCreateArgs  *ldcCreateArgs = NULL;
    Fdrv_Handle             handle = NULL;
    Ldc_SocInfo            *socInfo = NULL;

    instObj = &gM2mLdcInstObj;
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
        GT_0trace(VhwaLdcTrace, GT_ERR, "NULL Pointer !!\n");
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        status = vhwaM2mLdcCheckCreatePrms(drvId, drvInstId);
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
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
            GT_0trace(VhwaLdcTrace, GT_ERR,
                "Vhwa_m2mLdcInit is not called\n");
            status  = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */

        ldcCreateArgs = (Vhwa_M2mLdcCreateArgs *)createArgs;
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
        hObj = vhwaM2mLdcAllocHandleObj(instObj);

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> Rationale:
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
                /* Allocate SL2 Memory if not already allocated */
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
                    /* Initialize SL2 parameters with the defaul values */
                    Vhwa_M2mLdcSl2AllocPrmsInit(&instObj->sl2AllocPrms);

                    /* Allocate SL2 Parameters */
                    status = vhwaM2mLdcAllocSl2(instObj,
                        &instObj->sl2AllocPrms);
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
                    /* On the first open, enable interrupts in HTS and
                     * start all UTC channels.
                     * Even if all UTC channels are enabled,
                     * only channels enabled
                     * in HTS will be used for the transfer */

                    /* Start UDMA Channels on the first handle Create */
                    status = Vhwa_m2mLdcStartCh(instObj);
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
                status = Vhwa_m2mLdcAllocUdmaMem(hObj);
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
                Fvid2Utils_memcpy(&hObj->createArgs, ldcCreateArgs,
                    sizeof(Vhwa_M2mLdcCreateArgs));

                Fvid2Utils_memcpy(&hObj->cbPrms, cbPrms,
                    sizeof (hObj->cbPrms));

                instObj->openCnt ++;

                /* Setting it to UTRUE so that next request will update in HW. */
                hObj->isCfgUpdated = 1u;

                Fvid2Utils_memset(&hObj->chromaLutCfg, 0, sizeof(Ldc_RemapLutCfg));
                Fvid2Utils_memset(&hObj->lumaLutCfg, 0, sizeof(Ldc_RemapLutCfg));
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
                vhwaM2mLdcFreeHandleObj(hObj);
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
        {
            handle = (Fdrv_Handle)hObj;
        }
        /* LDRA_JUSTIFY_END */

        (void)VhwaAl_SemaphoreP_post(instObj->lock);
    }

    return (handle);
}

Int32 vhwa_m2mLdcDelete(Fdrv_Handle handle, Ptr deleteArgs)
{
    int32_t                 status = FVID2_SOK;
    Vhwa_M2mLdcInstObj     *instObj = NULL;
    Vhwa_M2mLdcHandleObj   *hObj = NULL;
    Ldc_SocInfo            *socInfo = NULL;

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
        instObj = &gM2mLdcInstObj;
        hObj    = (Vhwa_M2mLdcHandleObj *)handle;
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
            vhwaM2mLdcFreeHandleObj(hObj);

            instObj->openCnt --;

            /* Delete the Done queue */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the caller before the control reaches here.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
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
                #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
                /* Stop LDC - clear LDC_EN bit
                 * Only needed in continuous mode, as hardware doesn't automatically
                 * clear LDC_EN bit at end of frame in continuous mode.
                 * In one-shot mode, hardware automatically clears LDC_EN. */
                /* LDRA_JUSTIFY_START
                <metric start>statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications CAN reach this portion.
                No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                This behaviour is part of the application design. An error print statement can be added in a future release if required.
                <justification end> */
                if ((NULL != socInfo->ldcRegs) &&
                    (hObj->ldcCfg.continuousMode == VHWA_LDC_CNTU_MODE_CONTINUOUS))
                {
                    (void)CSL_ldcStop(socInfo->ldcRegs);
                }
                /* LDRA_JUSTIFY_END */
                #endif

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

Int32 vhwa_m2mLdcControl(Fdrv_Handle handle, UInt32 cmd, Ptr cmdArgs,
    Ptr cmdStatusArgs)
{
    int32_t                 status = FVID2_SOK;
    Vhwa_M2mLdcInstObj     *instObj = NULL;
    Vhwa_M2mLdcHandleObj   *hObj = NULL;
    Ldc_Config             *ldcCfg = NULL;
    Ldc_ErrEventParams     *eePrms = NULL;
    Ldc_RdBwLimitConfig    *rdBwLtCfg = NULL;
    Vhwa_HtsLimiter        *htsLimit = NULL;
    Ldc_RemapLutCfg        *lutCfg = NULL;
    Ldc_WdTimerErrEventParams *wdTimerEePrms = NULL;
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
        instObj = &gM2mLdcInstObj;
        hObj = (Vhwa_M2mLdcHandleObj *)handle;
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
            /* Set LDC Parameters */
            case IOCTL_VHWA_M2M_LDC_SET_PARAMS:
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
                    ldcCfg = (Ldc_Config *)cmdArgs;
                    status = vhwaM2mLdcSetParams(instObj, hObj, ldcCfg);
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the caller before the control reaches here.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaLdcTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_VHWA_M2M_LDC_SET_PARAMS !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            /* Enable Register error events callback */
            case IOCTL_VHWA_M2M_LDC_REGISTER_ERR_CB:
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
                    eePrms = (Ldc_ErrEventParams *)cmdArgs;
                    status = vhwa_m2mLdcSetEeParams(instObj, hObj, eePrms);
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the caller before the control reaches here.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaLdcTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_VHWA_M2M_LDC_REGISTER_ERR_CB !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            /* SET Read BW limit config */
            case IOCTL_VHWA_M2M_LDC_SET_RD_BW_LIMIT:
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
                    rdBwLtCfg = (Ldc_RdBwLimitConfig *)cmdArgs;
                    if((VHWA_SAFETY_MODE_DISABLED != hObj->enableReconfigReinitReg) || (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate))
                    {
                        /* Keep a copy of the LUT in hObj to reconfigure the registers*/
                        Fvid2Utils_memcpy(&hObj->ldcCfg.rdBwLimitCfg, rdBwLtCfg, sizeof(Ldc_RdBwLimitConfig));
                    }
                    status = CSL_ldcSetBwLimitConfig(
                        instObj->socInfo.ldcRegs, rdBwLtCfg);

                    /* If configRegValidate is enabled, also update golden memory to reuse CSL code */
                    if ((FVID2_SOK == status) && (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate))
                    {
                        VhwaVpacLdcSocReadBack *goldenRegVal = hObj->configRegMemPrms.configGoldenRegPtr;
                        if (NULL != goldenRegVal)
                        {
                            status = CSL_ldcSetBwLimitConfig(&goldenRegVal->ldcRegs, rdBwLtCfg);
                        }
                    }
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the caller before the control reaches here.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaLdcTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_VHWA_M2M_LDC_SET_RD_BW_LIMIT !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            /* SET HTS Limiter config */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The test framework and test apps cannot reach this portion
            The IOCTL is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit
            None;Unused feature, cannot be enabled, control cannot reach here.
            <justification end> */
            case IOCTL_VHWA_M2M_LDC_SET_HTS_LIMIT:
            {
                if (NULL != cmdArgs)
                {
                    htsLimit = (Vhwa_HtsLimiter *)cmdArgs;

                    /* Setting directly in the HTS configuration, so that
                     * it will be configured from the next request. */
                    hObj->htsCfg.enableBwLimit   = htsLimit->enableBwLimit;
                    hObj->htsCfg.cycleCnt        = htsLimit->cycleCnt;
                    hObj->htsCfg.tokenCnt        = htsLimit->tokenCnt;
                }
                else
                {
                    GT_0trace(VhwaLdcTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_VHWA_M2M_LDC_SET_HTS_LIMIT !!\n");
                    status = FVID2_EBADARGS;
                }
                break;
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
            case IOCTL_VHWA_M2M_LDC_GET_PERFORMANCE:
            {
                uint32_t *perf = NULL;
                if (NULL != cmdArgs)
                {
                    perf = (uint32_t *)cmdArgs;
                    *perf = (uint32_t)hObj->perfNum;
                }
                else
                {
                    GT_0trace(VhwaLdcTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_VHWA_M2M_LDC_GET_PERFORMANCE !!\n");
                    status = FVID2_EBADARGS;
                }
                break;
            }

            case IOCTL_VHWA_M2M_LDC_SYNC_START:
            {
                if (NULL != hObj->createArgs.getTimeStamp)
                {
                    hObj->perfNum = hObj->createArgs.getTimeStamp();
                }

                status = CSL_htsPipelineStart(instObj->socInfo.htsRegs, &hObj->htsCfg);
                break;
            }
            /* LDRA_JUSTIFY_END */
            /* LDRA_JUSTIFY_START
            <metric start>statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. An error print statement can be added in a future release if required.
            <justification end> */
            case IOCTL_LDC_SET_LUMA_TONEMAP_LUT_CFG:
            {
                lutCfg = (Ldc_RemapLutCfg *)cmdArgs;
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != lutCfg)
                {
                    /* Keep a copy of the LUT in hObj to reconfigure the registers*/
                    Fvid2Utils_memcpy(&hObj->lumaLutCfg, lutCfg, sizeof(Ldc_RemapLutCfg));

                    status = CSL_ldcSetLumaToneMapLutCfg(
                        instObj->socInfo.ldcRegs,
                        instObj->socInfo.lumaLutBaseAddr, lutCfg);

                    /* If config register validation is enabled, also write LUT to golden memory */
                    if ((FVID2_SOK == status) && (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate))
                    {
                        VhwaVpacLdcSocReadBack *goldenRegVal = hObj->configRegMemPrms.configGoldenRegPtr;
                        if (NULL != goldenRegVal)
                        {
                            status = CSL_ldcSetLumaToneMapLutCfg(
                                &goldenRegVal->ldcRegs,
                                (volatile uint32_t *)goldenRegVal->ldcLumaLut.LUT, lutCfg);
                            if (FVID2_SOK != status)
                            {
                                GT_0trace(VhwaLdcTrace, GT_ERR,
                                    "LDC LUMA LUT: Failed to write to golden memory\n");
                            }
                        }
                        else
                        {
                            GT_0trace(VhwaLdcTrace, GT_ERR,
                                "goldenRegVal is NULL, cannot write LUMA LUT to golden memory !!\n");
                            GT_0trace(VhwaLdcTrace, GT_ERR,
                                "This indicates buffers were not allocated by VHWA_M2M_IOCTL_LDC_SET_CONFIG_REG_PTRS\n");
                            status = FVID2_EFAIL;
                        }
                    }
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the caller before the control reaches here.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaLdcTrace, GT_ERR,
                        "cmdArgs cannot be NULL for CSL_ldcSetLumaToneMapLutCfg !!\n");
                    status = FVID2_EBADARGS;
                }
                break;
            }
            /* LDRA_JUSTIFY_END */

            /* LDRA_JUSTIFY_START
            <metric start>statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. An error print statement can be added in a future release if required.
            <justification end> */
            case IOCTL_LDC_SET_CHROMA_TONEMAP_LUT_CFG:
            {
                lutCfg = (Ldc_RemapLutCfg *)cmdArgs;
                if (NULL != lutCfg)
                {
                    /* Keep a copy of the LUT in hObj to reconfigure the registers*/
                    Fvid2Utils_memcpy(&hObj->chromaLutCfg, lutCfg, sizeof(Ldc_RemapLutCfg));

                    status = CSL_ldcSetChromaToneMapLutCfg(
                        instObj->socInfo.ldcRegs,
                        instObj->socInfo.chromaLutBaseAddr, lutCfg);

                    /* If config register validation is enabled, also write LUT to golden memory */
                    if ((FVID2_SOK == status) && (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate))
                    {
                        VhwaVpacLdcSocReadBack *goldenRegVal = hObj->configRegMemPrms.configGoldenRegPtr;
                        if (NULL != goldenRegVal)
                        {
                            status = CSL_ldcSetChromaToneMapLutCfg(
                                &goldenRegVal->ldcRegs,
                                (volatile uint32_t *)goldenRegVal->ldcChromaLut.LUT, lutCfg);
                        }
                        else
                        {
                            GT_0trace(VhwaLdcTrace, GT_ERR,
                                "goldenRegVal is NULL, cannot write CHROMA LUT to golden memory !!\n");
                            status = FVID2_EFAIL;
                        }
                    }
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the caller before the control reaches here.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaLdcTrace, GT_ERR,
                        "cmdArgs cannot be NULL for CSL_ldcSetChromaToneMapLutCfg !!\n");
                    status = FVID2_EBADARGS;
                }
                break;
            }
            /* LDRA_JUSTIFY_END */
            /* Configure watchdog timer */

            case VHWA_M2M_IOCTL_LDC_REGISTER_WDTIMER_ERR_CB:
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
                    wdTimerEePrms = (Ldc_WdTimerErrEventParams *)cmdArgs;
                    status = vhwa_m2mLdcSetWdTimerEeParams(instObj, hObj, wdTimerEePrms);
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the caller before the control reaches here.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaLdcTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_LDC_REGISTER_WDTIMER_ERR_CB !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }
            /* Get PSA signature values for enabled outputs */
            case VHWA_M2M_IOCTL_LDC_GET_PSA_SIGN:
            {
                Vhwa_M2mLdcPsaSign *psa = NULL;

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
                    psa = (Vhwa_M2mLdcPsaSign *)cmdArgs;
                    *psa = hObj->psa;
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement <metric end>
                <justification start>
                RRationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the caller before the control reaches here.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaLdcTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_LDC_GET_PSA_SIGN !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case VHWA_M2M_IOCTL_LDC_ENABLE_RECONFIG_REINIT_REG:
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
                        GT_1trace(VhwaLdcTrace, GT_ERR,
                            "Invalid safety mode value %u for VHWA_M2M_IOCTL_LDC_ENABLE_RECONFIG_REINIT_REG !!\n", modeVal);
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
                    GT_0trace(VhwaLdcTrace, GT_ERR,
                        "handle/cmdArgs cannot be NULL for VHWA_M2M_IOCTL_LDC_ENABLE_RECONFIG_REINIT_REG !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case VHWA_M2M_IOCTL_LDC_ENABLE_STATUS_REG_VALIDATE:
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
                        GT_1trace(VhwaLdcTrace, GT_ERR,
                            "Invalid safety mode value %u for VHWA_M2M_IOCTL_LDC_ENABLE_STATUS_REG_VALIDATE !!\n", modeVal);
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
                    GT_0trace(VhwaLdcTrace, GT_ERR,
                        "handle/cmdArgs cannot be NULL for VHWA_M2M_IOCTL_LDC_ENABLE_STATUS_REG_VALIDATE !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case VHWA_M2M_IOCTL_LDC_VALIDATE_REG:
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
                        status = Vhwa_m2mLdcStatusRegValidate(hObj, &instObj->socInfo);
                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        The test framework does not support the configuration required to trigger this error scenario.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if (FVID2_SOK != status)
                        {
                            GT_0trace(VhwaLdcTrace, GT_ERR, "VPAC-10: StatusRegValidate failed!\n");
                        }
                        /* LDRA_JUSTIFY_END */
                    }
                    if (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate)
                    {
                        /* Readback config registers */
                        status = Vhwa_m2mLdcConfigRegReadback(hObj, instObj);
                    }

                    /* Release the hardware lock after the configReg readback or statusReg */
                    if ((VHWA_SAFETY_MODE_DISABLED != hObj->enableStatusRegValidate) ||
                        (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate))
                    {
                        Vhwa_commonHwaLockRelease(instObj->hwaLockIdx);
                    }

                    /* Reset the statusReg validate flag only in one-shot mode */
                    if (VHWA_SAFETY_MODE_ONESHOT == hObj->enableStatusRegValidate)
                    {
                        cookie = HwiP_disable();
                        hObj->enableStatusRegValidate = (uint32_t)VHWA_SAFETY_MODE_DISABLED;
                        HwiP_restore(cookie);
                    }

                    if ((VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate) && (FVID2_SOK == status))
                    {
                        /* Callback with goldenReg and ReadBackReg pointer to application for memory comparison */
                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        The parameters are pre-validated by the caller before the control reaches here.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if((NULL != hObj->configRegMemPrms.cbFxn) &&
                            (NULL != hObj->configRegMemPrms.configGoldenRegPtr) &&
                            (NULL != hObj->configRegMemPrms.configRegReadbackPtr))
                        /* LDRA_JUSTIFY_END */
                        {
                            status = Vhwa_M2mLdcConfigRegMemCompare(hObj);
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
                            GT_0trace(VhwaLdcTrace, GT_ERR, "ConfigRegMemParams are NULL!\n");
                            status = FVID2_EFAIL;
                        }
                        /* LDRA_JUSTIFY_END */

                        /* Get status from callback for memory comparison */
                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        The test framework does not support the configuration required to trigger this error scenario.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if (FVID2_SOK != status)
                        {
                            GT_0trace(VhwaLdcTrace, GT_ERR, "VPAC-7: ConfigRegReadbackCallback Memory Comparison failed!\n");
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
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaLdcTrace, GT_ERR,
                        "handle cannot be NULL for VHWA_M2M_IOCTL_LDC_VALIDATE_REG !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case VHWA_M2M_IOCTL_LDC_ENABLE_CONFIG_REG_READBACK:
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
                        GT_1trace(VhwaLdcTrace, GT_ERR,
                            "Invalid safety mode value %u for VHWA_M2M_IOCTL_LDC_ENABLE_CONFIG_REG_READBACK !!\n", modeVal);
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
                    GT_0trace(VhwaLdcTrace, GT_ERR,
                        "handle/cmdArgs cannot be NULL for VHWA_M2M_IOCTL_LDC_ENABLE_CONFIG_REG_READBACK !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case VHWA_M2M_IOCTL_LDC_GET_READBACK_SIZE:
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
                    *readbackSize = (uint32_t)sizeof(VhwaVpacLdcSocReadBack);
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
                    GT_0trace(VhwaLdcTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_LDC_GET_READBACK_SIZE !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case VHWA_M2M_IOCTL_LDC_SET_CONFIG_REG_PTRS:
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
                    Vhwa_M2mLdcConfigRegMemParams *reg_ptrs = (Vhwa_M2mLdcConfigRegMemParams *)cmdArgs;

                    /* LDRA_JUSTIFY_START
                    <metric start> statement <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    The test framework does not support the configuration required to trigger this error scenario.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if ((reg_ptrs->readback_ptr == NULL) ||
                        (reg_ptrs->golden_ptr == NULL) ||
                        (reg_ptrs->cbFxn == NULL))
                    {
                        GT_0trace(VhwaLdcTrace, GT_ERR,
                            "readback_ptr, golden_ptr, cbFxn cannot be NULL for VHWA_M2M_IOCTL_LDC_SET_CONFIG_REG_PTRS !!\n");
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
                        status = vhwaM2mLdcSetDefaultGoldenRegMemValues(hObj, instObj);

                        /* LDRA_JUSTIFY_START
                        <metric start> statement <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        The test framework does not support the configuration required to trigger this error scenario.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if(FVID2_SOK != status)
                        {
                            GT_0trace(VhwaLdcTrace, GT_ERR,
                                "Failed to set default goldenReg mem values for VHWA_M2M_IOCTL_LDC_SET_CONFIG_REG_PTRS !!\n");
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
                    GT_0trace(VhwaLdcTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_LDC_SET_CONFIG_REG_PTRS !!\n");
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


Int32 vhwa_m2mLdcProcessReq(Fdrv_Handle handle, Fvid2_FrameList *inFrmList,
    Fvid2_FrameList *outFrmList, uint32_t timeout)
{
    int32_t               status = FVID2_SOK;
    uint32_t              chId;
    uint32_t              outCnt;
    uint32_t              chCnt;
    uint32_t              semTimeout;
    Vhwa_M2mLdcInstObj   *instObj = NULL;
    Vhwa_M2mLdcHandleObj *hObj = NULL;
    Vhwa_M2mLdcQueueObj  *qObj = NULL;
    Ldc_Config           *ldcCfg = NULL;
    Fvid2_Frame          *frm = NULL;

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
        instObj = &gM2mLdcInstObj;
        hObj = (Vhwa_M2mLdcHandleObj *)handle;
        ldcCfg = &hObj->ldcCfg;

        /* Check if IP is in stalled state due to error interrupt or watchdog timeout */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)UTRUE == instObj->isStalled)
        {
            status = FVID2_EFAIL;
            GT_0trace(VhwaLdcTrace, GT_ERR,
                "Process request rejected: IP is in stalled state due to previous error\n");
        }
        /* LDRA_JUSTIFY_END */
    }

    /* Check for null pointers */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    // check  (only branch )
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
            (0U == inFrmList->frames[0u]->addr[0U]))
        {
            status = FVID2_EBADARGS;
        }
        /* LDRA_JUSTIFY_END */
        else
        {
            /* For YUV420 input, chroma buffer cannot be null */
            if ((FVID2_DF_YUV420SP_UV == ldcCfg->inFmt.dataFormat) ||
                (FVID2_DF_YUV422SP_UV == ldcCfg->inFmt.dataFormat))
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
        }

        chId = 0u;
        for (outCnt = 0u; outCnt < LDC_MAX_OUTPUT; outCnt ++)
        {
            frm = outFrmList->frames[outCnt];
            if ((uint32_t)UTRUE == ldcCfg->enableOutput[outCnt])
            {
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL == frm)
                {
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                else
                {
                    for (chCnt = 0U; chCnt < VHWA_MAX_CH_PER_OUTPUT; chCnt ++)
                    {
                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        The test framework does not support the configuration required to trigger this error scenario.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if (((uint32_t)UTRUE == hObj->chPrms[chId].isEnabled) &&
                            (0U == frm->addr[chCnt]))
                        {
                            status = FVID2_EBADARGS;
                            break;
                        }
                        /* LDRA_JUSTIFY_END */

                        chId ++;
                    }
                }
            }
            else
            {
                chId = chId + VHWA_MAX_CH_PER_OUTPUT;
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK != status)
            {
                break;
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
            qObj = (Vhwa_M2mLdcQueueObj *) Fvid2Utils_dequeue(instObj->freeQ);

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
                GT_0trace(VhwaLdcTrace, GT_ERR,
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
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected decreament number of pending requests.
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
            /* HW is free, submit request to the hardware */
            /* If previous handle and current handles are same, */
            if ((instObj->lastHndlObj != qObj->hObj) ||
                (1u == hObj->isCfgUpdated))
            {
                /** Last handle was not same as new handle,
                 *  so require to recofigure all HW IPs */
                status = vhwaM2mLdcSetConfigInHW(instObj, qObj);

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
                    /* Cfg is successfully set in the HW, so
                     * resetting this flag */
                    hObj->isCfgUpdated = 0u;
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
                status = Vhwa_m2mLdcSetIntrInHW(instObj->vhwaIrqNum, instObj->socInfo.vpacIntdRegs, hObj);
            }

            /* Invoke the reconfig-MMR if enableReconfigReinitReg enabled for the current handle */
            if (VHWA_SAFETY_MODE_DISABLED != hObj->enableReconfigReinitReg)
            {
                status = Vhwa_m2mLdcReconfigReinitReg(instObj, hObj, qObj);
                /* Reset only in one-shot mode */
                if (VHWA_SAFETY_MODE_ONESHOT == hObj->enableReconfigReinitReg)
                {
                    hObj->enableReconfigReinitReg = (uint32_t)VHWA_SAFETY_MODE_DISABLED;
                }
                hObj->isCfgUpdated = 0u;
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
                status = vhwaM2mLdcSubmitRequest(instObj, qObj);
            }
        }
    }

    return (status);
}

/** \brief Typedef for FVID2 get processed frames function pointer. */
Int32 vhwa_m2mLdcGetProcessReq(Fdrv_Handle handle,
    Fvid2_FrameList *inFrmList, Fvid2_FrameList *outFrmList,
    UInt32 timeout)
{
    int32_t                status = FVID2_SOK;
    uint32_t               cookie;
    Vhwa_M2mLdcInstObj    *instObj = NULL;
    Vhwa_M2mLdcHandleObj  *hObj = NULL;
    Vhwa_M2mLdcQueueObj   *qObj = NULL;

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
        instObj = &gM2mLdcInstObj;
        hObj = (Vhwa_M2mLdcHandleObj *)handle;
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
        /* Wait until completed request queue is populated.
         * Failure to exit this loop indicates that the ISR never hit.*/
        do
        {
            statusQ = Fvid2Utils_isQEmpty(hObj->doneQ);
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
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
        qObj = (Vhwa_M2mLdcQueueObj *) Fvid2Utils_dequeue(hObj->doneQ);

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
                GT_0trace(VhwaLdcTrace, GT_DEBUG,
                    "Nothing to dequeue. No request pending with driver!!\r\n");
                status = FVID2_ENO_MORE_BUFFERS;
            }
            else
            {
                /* If no request have completed, return try again */
                GT_0trace(VhwaLdcTrace, GT_DEBUG,
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
#if !defined (MCU_PLUS_SDK)
            /* Dequeue from Rings */
            status = Vhwa_m2mLdcPopRings(instObj, hObj);
#endif
            /* Decrement the pending request count. */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
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
                GT_0trace(VhwaLdcTrace, GT_DEBUG,
                    "Done queue of the handle is not empty when there are is no pending request on the handle!!\r\n");
                status = FVID2_EFAIL;
            }
            /* LDRA_JUSTIFY_END */
        }
        if ((VHWA_SAFETY_MODE_DISABLED == hObj->enableStatusRegValidate) &&
        (VHWA_SAFETY_MODE_DISABLED == hObj->enableConfigRegValidate))
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

static void vhwaM2mLdcCalcHtsDepth(Vhwa_M2mLdcHandleObj *hObj)
{
    uint32_t chCnt, hCnt, vCnt;
    Vhwa_M2mLdcChParams *chPrms = NULL;
    Vhwa_M2mLdcRegionParams *regPrms = NULL;

    for (chCnt = 0u; chCnt < VHWA_M2M_LDC_OUT_DMA_CH; chCnt ++)
    {
        chPrms = &hObj->chPrms[chCnt];
        if ((uint32_t)UTRUE == chPrms->isEnabled)
        {
            chPrms->minHtsDepth = 0xFFFFFFFFu;
            for (vCnt = 0u; vCnt < LDC_MAX_VERT_REGIONS; vCnt ++)
            {
                for (hCnt = 0u; hCnt < LDC_MAX_HORZ_REGIONS; hCnt ++)
                {
                    regPrms = &chPrms->regPrms[vCnt][hCnt];
                    if ((uint32_t)UTRUE == regPrms->isEnabled)
                    {
                        if (chPrms->minHtsDepth > regPrms->numSl2Blocks)
                        {
                            chPrms->minHtsDepth = regPrms->numSl2Blocks;
                        }
                    }
                }
            }
        }
    }
}

static void vhwaM2mLdcInitRegParams(Vhwa_M2mLdcChParams *chPrms,
    Ldc_Config *ldcCfg)
{
    uint32_t                 hCnt;
    uint32_t                 vCnt;
    Ldc_RegionConfig        *regCfg = NULL;
    Vhwa_M2mLdcRegionParams *regPrms = NULL;

    /* By default, all regions are disabled */
    for (vCnt = 0u; vCnt < LDC_MAX_VERT_REGIONS; vCnt ++)
    {
        for (hCnt = 0u; hCnt < LDC_MAX_HORZ_REGIONS; hCnt ++)
        {
            regPrms = &chPrms->regPrms[vCnt][hCnt];
            regPrms->isEnabled = (uint32_t)UFALSE;
        }
    }

    if ((uint32_t)UFALSE == ldcCfg->enableMultiRegions)
    {
        /* For non-multi region mode, regParams[0][0] is used */
        regPrms = &chPrms->regPrms[0u][0u];

        regPrms->width       = ldcCfg->outputFrameWidth;
        regPrms->height      = ldcCfg->outputFrameHeight;
        regPrms->blockWidth  = ldcCfg->outputBlockWidth;
        regPrms->blockHeight = ldcCfg->outputBlockHeight;

        /* Calculate pitch required for block */
        regPrms->blockWidthInBytes =
            Vhwa_m2mLdcCalcHorzSizeInBytes(regPrms->blockWidth,
                chPrms->ccsf, chPrms->dataFmt);

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
        <justification end> */
        if(FVID2_DF_RGI_B == chPrms->dataFmt)
        {
            regPrms->blockWidthInBytes = regPrms->blockWidthInBytes*2u;
        }
        /* LDRA_JUSTIFY_END */

        regPrms->isEnabled = (uint32_t)UTRUE;
    }
    else
    {
        for (vCnt = 0u; vCnt < LDC_MAX_VERT_REGIONS; vCnt ++)
        {
            for (hCnt = 0u; hCnt < LDC_MAX_HORZ_REGIONS; hCnt ++)
            {
                regPrms = &chPrms->regPrms[vCnt][hCnt];
                regCfg = &ldcCfg->regCfg;

                regPrms->width       = regCfg->width[hCnt];
                regPrms->height      = regCfg->height[vCnt];
                if ((uint32_t)UTRUE == regCfg->enable[vCnt][hCnt])
                {
                    regPrms->blockWidth  = regCfg->blockWidth[vCnt][hCnt];
                    regPrms->blockHeight = regCfg->blockHeight[vCnt][hCnt];

                    /* Calculate pitch required for block */
                    regPrms->blockWidthInBytes =
                        Vhwa_m2mLdcCalcHorzSizeInBytes(regPrms->blockWidth,
                            chPrms->ccsf, chPrms->dataFmt);
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if(FVID2_DF_RGI_B == chPrms->dataFmt)
                    {
                        regPrms->blockWidthInBytes = regPrms->blockWidthInBytes*2u;
                    }
                    /* LDRA_JUSTIFY_END */

                    regPrms->isEnabled = (uint32_t)UTRUE;
                }
            }
        }
    }
}

static void vhwaM2mLdcInitRegParamsForChroma(Vhwa_M2mLdcChParams *chPrms,
    Ldc_Config *ldcCfg)
{
    uint32_t                 hCnt;
    uint32_t                 vCnt;
    Ldc_RegionConfig        *regCfg = NULL;
    Vhwa_M2mLdcRegionParams *regPrms = NULL;

    /* By default, all regions are disabled. */
    for (vCnt = 0u; vCnt < LDC_MAX_VERT_REGIONS; vCnt ++)
    {
        for (hCnt = 0u; hCnt < LDC_MAX_HORZ_REGIONS; hCnt ++)
        {
            regPrms = &chPrms->regPrms[vCnt][hCnt];
            regPrms->isEnabled = (uint32_t)UFALSE;
        }
    }

    if ((uint32_t)UFALSE == ldcCfg->enableMultiRegions)
    {
        /* For non-multi region mode, regParams[0][0] is used */
        regPrms              = &chPrms->regPrms[0u][0u];

        regPrms->width       = ldcCfg->outputFrameWidth;
        regPrms->blockWidth  = ldcCfg->outputBlockWidth;
        if(FVID2_DF_YUV420SP_UV == chPrms->dataFmt)
        /* LDRA_JUSTIFY_END */
        {
            regPrms->height      = ldcCfg->outputFrameHeight / 2u;
            regPrms->blockHeight = ldcCfg->outputBlockHeight / 2u;
        }
        else
        {
            regPrms->height      = ldcCfg->outputFrameHeight;
            regPrms->blockHeight = ldcCfg->outputBlockHeight;
        }
        /* LDRA_JUSTIFY_END */

        /* Calculate pitch required for block */
        regPrms->blockWidthInBytes =
            Vhwa_m2mLdcCalcHorzSizeInBytes(regPrms->blockWidth,
                chPrms->ccsf, chPrms->dataFmt);

        regPrms->isEnabled = (uint32_t)UTRUE;
    }
    else
    {
        for (vCnt = 0u; vCnt < LDC_MAX_VERT_REGIONS; vCnt ++)
        {
            for (hCnt = 0u; hCnt < LDC_MAX_HORZ_REGIONS; hCnt ++)
            {
                regPrms = &chPrms->regPrms[vCnt][hCnt];
                regCfg = &ldcCfg->regCfg;

                regPrms->width       = regCfg->width[hCnt];
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications CAN reach this portion.
                No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                <justification end> */
                if(FVID2_DF_YUV420SP_UV == chPrms->dataFmt)
                /* LDRA_JUSTIFY_END */
                {
                    regPrms->height      = regCfg->height[vCnt] / 2u;
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
                    regPrms->height      = regCfg->height[vCnt];
                }
                /* LDRA_JUSTIFY_END */

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications CAN reach this portion.
                No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((uint32_t)UTRUE == regCfg->enable[vCnt][hCnt])
                /* LDRA_JUSTIFY_END */
                {
                    regPrms->blockWidth  = regCfg->blockWidth[vCnt][hCnt];

                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if(FVID2_DF_YUV420SP_UV == chPrms->dataFmt)
                    /* LDRA_JUSTIFY_END */
                    {
                        regPrms->blockHeight = regCfg->blockHeight[vCnt][hCnt] / 2u;
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
                        regPrms->blockHeight = regCfg->blockHeight[vCnt][hCnt];
                    }
                    /* LDRA_JUSTIFY_END */

                    /* Calculate pitch required for block */
                    regPrms->blockWidthInBytes =
                        Vhwa_m2mLdcCalcHorzSizeInBytes(regPrms->blockWidth,
                            chPrms->ccsf, chPrms->dataFmt);

                    regPrms->isEnabled = (uint32_t)UTRUE;
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
                    regPrms->isEnabled = (uint32_t)UFALSE;
                }
                /* LDRA_JUSTIFY_END */
            }
        }
    }
}

static void vhwaM2mLdcInitChParamsFromLdcConfig(const Vhwa_M2mLdcInstObj *instObj,
    Vhwa_M2mLdcHandleObj *hObj)
{
    uint32_t             outCnt;
    uint32_t             chId;
    uint32_t             sl2MemSize;
    Ldc_Config          *ldcCfg = NULL;
    Vhwa_M2mLdcChParams *chPrms = NULL;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL != instObj) && (NULL != hObj))
    /* LDRA_JUSTIFY_END */
    {
        ldcCfg = &hObj->ldcCfg;

        /* Enable the channel which are required */
        chId = 0u;
        for (outCnt = 0u; outCnt < LDC_MAX_OUTPUT; outCnt ++)
        {
            /* Enable channels only if output is enabled */
            if ((uint32_t)UTRUE == ldcCfg->enableOutput[outCnt])
            {
                sl2MemSize = vhwaM2mLdcCalcSl2MemSize(ldcCfg, outCnt);

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications CAN reach this portion.
                No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                <justification end> */
                switch(ldcCfg->outFmt[outCnt].dataFormat)
                {
                /* LDRA_JUSTIFY_END */
                    case FVID2_DF_YUV420SP_UV:
                    {
                        /* For YUV420 output, two channels are required */
                        chPrms              = &hObj->chPrms[chId];
                        chPrms->isEnabled   = (uint32_t)UTRUE;
                        chPrms->pitch       = ldcCfg->outFmt[outCnt].pitch[0u];
                        chPrms->ccsf        = ldcCfg->outFmt[outCnt].ccsFormat;
                        chPrms->dataFmt     = ldcCfg->outFmt[outCnt].dataFormat;
                        chPrms->sl2Addr     = instObj->sl2Addr[outCnt];
                        chPrms->sl2Size     = sl2MemSize;
                        vhwaM2mLdcInitRegParams(chPrms, ldcCfg);

                        /* Enable Channel for Chroma */
                        chPrms              = &hObj->chPrms[chId + 1u];
                        chPrms->isEnabled   = (uint32_t)UTRUE;
                        chPrms->pitch       = ldcCfg->outFmt[outCnt].pitch[1u];
                        if(UTRUE == ldcCfg->indChPrms.enable)
                        {
                            chPrms->ccsf        = ldcCfg->indOutChCcsf[outCnt];
                        }
                        else
                        {
                            chPrms->ccsf        = ldcCfg->outFmt[outCnt].ccsFormat;
                        }
                        chPrms->dataFmt     = ldcCfg->outFmt[outCnt].dataFormat;
                        chPrms->sl2Addr     = instObj->sl2Addr[outCnt] +
                                                                sl2MemSize;
                        chPrms->sl2Size     = sl2MemSize / 2U;
                        vhwaM2mLdcInitRegParamsForChroma(chPrms, ldcCfg);
                        break;
                    }
                    case FVID2_DF_YUV422SP_UV:
                    {
                        /* For YUV422 output, two channels are required */
                        chPrms              = &hObj->chPrms[chId];
                        chPrms->isEnabled   = (uint32_t)UTRUE;
                        chPrms->pitch       = ldcCfg->outFmt[outCnt].pitch[0u];
                        chPrms->ccsf        = ldcCfg->outFmt[outCnt].ccsFormat;
                        chPrms->dataFmt     = ldcCfg->outFmt[outCnt].dataFormat;
                        chPrms->sl2Addr     = instObj->sl2Addr[outCnt];
                        chPrms->sl2Size     = sl2MemSize;
                        vhwaM2mLdcInitRegParams(chPrms, ldcCfg);

                        /* Enable Channel for Chroma */
                        chPrms              = &hObj->chPrms[chId + 1u];
                        chPrms->isEnabled   = (uint32_t)UTRUE;
                        chPrms->pitch       = ldcCfg->outFmt[outCnt].pitch[1u];
                        if(UTRUE == ldcCfg->indChPrms.enable)
                        {
                            chPrms->ccsf        = ldcCfg->indOutChCcsf[outCnt];
                        }
                        else
                        {
                            chPrms->ccsf        = ldcCfg->outFmt[outCnt].ccsFormat;
                        }
                        chPrms->dataFmt     = ldcCfg->outFmt[outCnt].dataFormat;
                        chPrms->sl2Addr     = instObj->sl2Addr[outCnt] +
                                                                sl2MemSize;
                        chPrms->sl2Size     = sl2MemSize;
                        vhwaM2mLdcInitRegParamsForChroma(chPrms, ldcCfg);
                        break;
                    }
                    case FVID2_DF_2PLANES:
                    {
                        /* For Y1 + Y2 output, two channels are required */
                        chPrms              = &hObj->chPrms[chId];
                        chPrms->isEnabled   = (uint32_t)UTRUE;
                        chPrms->pitch       = ldcCfg->outFmt[outCnt].pitch[0u];
                        chPrms->ccsf        = ldcCfg->outFmt[outCnt].ccsFormat;
                        chPrms->dataFmt     = ldcCfg->outFmt[outCnt].dataFormat;
                        chPrms->sl2Addr     = instObj->sl2Addr[outCnt];
                        chPrms->sl2Size     = sl2MemSize;
                        vhwaM2mLdcInitRegParams(chPrms, ldcCfg);

                        /* Enable Channel for Y2 */
                        chPrms              = &hObj->chPrms[chId + 1u];
                        chPrms->isEnabled   = (uint32_t)UTRUE;
                        chPrms->pitch       = ldcCfg->outFmt[outCnt].pitch[1u];
                        if(UTRUE == ldcCfg->indChPrms.enable)
                        {
                            chPrms->ccsf        = ldcCfg->indOutChCcsf[outCnt];
                        }
                        else
                        {
                            chPrms->ccsf        = ldcCfg->outFmt[outCnt].ccsFormat;
                        }
                        chPrms->dataFmt     = ldcCfg->outFmt[outCnt].dataFormat;
                        chPrms->sl2Addr     = instObj->sl2Addr[outCnt] +
                                                                sl2MemSize;
                        chPrms->sl2Size     = sl2MemSize;
                        vhwaM2mLdcInitRegParams(chPrms, ldcCfg);
                        break;
                    }
                    case FVID2_DF_RGB:
                    {
                        /* For R + GB output, channel 0/1/3 channels are required */
                        chPrms              = &hObj->chPrms[chId];
                        chPrms->isEnabled   = (uint32_t)UTRUE;
                        chPrms->pitch       = ldcCfg->outFmt[outCnt].pitch[0u];
                        chPrms->ccsf        = ldcCfg->outFmt[outCnt].ccsFormat;
                        chPrms->dataFmt     = ldcCfg->outFmt[outCnt].dataFormat;
                        chPrms->sl2Addr     = instObj->sl2Addr[outCnt];
                        chPrms->sl2Size     = sl2MemSize;
                        vhwaM2mLdcInitRegParams(chPrms, ldcCfg);

                        /* Enable Channel for Y2 */
                        chPrms              = &hObj->chPrms[chId+1u];
                        chPrms->isEnabled   = (uint32_t)UTRUE;
                        chPrms->pitch       = ldcCfg->outFmt[outCnt].pitch[1u];
                        /* LDRA_JUSTIFY_START
                        <metric start> branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications CAN reach this portion.
                        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                        This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if(UTRUE == ldcCfg->indChPrms.enable)
                        /* LDRA_JUSTIFY_END */
                        {
                            chPrms->ccsf        = ldcCfg->indOutChCcsf[outCnt];
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
                            chPrms->ccsf        = ldcCfg->outFmt[outCnt].ccsFormat;
                        }
                        /* LDRA_JUSTIFY_END */
                        chPrms->dataFmt     = ldcCfg->outFmt[outCnt].dataFormat;
                        chPrms->sl2Addr     = instObj->sl2Addr[outCnt] +
                                                sl2MemSize;
                        chPrms->sl2Size     = sl2MemSize;
                        vhwaM2mLdcInitRegParams(chPrms, ldcCfg);

                        /* Enable Channel for Y3 */
                        chPrms              = &hObj->chPrms[chId+3u];
                        chPrms->isEnabled   = (uint32_t)UTRUE;
                        chPrms->pitch       = ldcCfg->outFmt[outCnt].pitch[2u];
                        /* LDRA_JUSTIFY_START
                        <metric start> branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications CAN reach this portion.
                        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                        This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if(UTRUE == ldcCfg->indChPrms.enable)
                        /* LDRA_JUSTIFY_END */
                        {
                            chPrms->ccsf        = ldcCfg->indOutChCcsf[outCnt];
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
                            chPrms->ccsf        = ldcCfg->outFmt[outCnt].ccsFormat;
                        }
                        /* LDRA_JUSTIFY_END */
                        chPrms->dataFmt     = ldcCfg->outFmt[outCnt].dataFormat;
                        chPrms->sl2Addr     = instObj->sl2Addr[outCnt] +
                                                      ((uint64_t)sl2MemSize * (uint64_t)2u);
                        chPrms->sl2Size     = sl2MemSize;
                        vhwaM2mLdcInitRegParams(chPrms, ldcCfg);
                        break;
                    }
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    case FVID2_DF_RGI_B:
                    {
                        /* For RG+B output, channel 0 and 3 channels are required */
                        chPrms              = &hObj->chPrms[chId];
                        chPrms->isEnabled   = (uint32_t)UTRUE;
                        chPrms->pitch       = ldcCfg->outFmt[outCnt].pitch[0u];
                        chPrms->ccsf        = ldcCfg->outFmt[outCnt].ccsFormat;
                        chPrms->dataFmt     = ldcCfg->outFmt[outCnt].dataFormat;
                        chPrms->sl2Addr     = instObj->sl2Addr[outCnt];
                        chPrms->sl2Size     = sl2MemSize * 2u;
                        vhwaM2mLdcInitRegParams(chPrms, ldcCfg);

                        /* Enable Channel 3 for Y3 */
                        chPrms              = &hObj->chPrms[chId + 3u];
                        chPrms->isEnabled   = (uint32_t)UTRUE;
                        chPrms->pitch       = ldcCfg->outFmt[outCnt].pitch[1u];
                        if(UTRUE == ldcCfg->indChPrms.enable)
                        {
                            chPrms->ccsf        = ldcCfg->indOutChCcsf[outCnt];
                        }
                        else
                        {
                            chPrms->ccsf        = ldcCfg->outFmt[outCnt].ccsFormat;
                        }
                        chPrms->dataFmt     = ldcCfg->outFmt[outCnt].dataFormat;
                        chPrms->sl2Addr     = instObj->sl2Addr[outCnt] +
                                                        ((uint64_t)sl2MemSize * (uint64_t)2u);
                        chPrms->sl2Size     = sl2MemSize;
                        vhwaM2mLdcInitRegParamsForChroma(chPrms, ldcCfg);
                        break;
                    }
                    /* LDRA_JUSTIFY_END */
                    case FVID2_DF_YUV422I_YUYV:
                    case FVID2_DF_YUV422I_UYVY:
                    {
                        chPrms              = &hObj->chPrms[chId];
                        chPrms->isEnabled   = (uint32_t)UTRUE;
                        chPrms->pitch       = ldcCfg->outFmt[outCnt].pitch[0u];
                        chPrms->ccsf        = ldcCfg->outFmt[outCnt].ccsFormat;
                        chPrms->dataFmt     = ldcCfg->outFmt[outCnt].dataFormat;
                        chPrms->sl2Addr     = instObj->sl2Addr[outCnt];
                        chPrms->sl2Size     = sl2MemSize;
                        vhwaM2mLdcInitRegParams(chPrms, ldcCfg);
                        break;
                    }
                    case FVID2_DF_LUMA_ONLY:
                    {
                        /* For chroma only mode, only one channel
                           needs to be enabled */
                        chPrms              = &hObj->chPrms[chId];
                        chPrms->isEnabled   = (uint32_t)UTRUE;
                        chPrms->pitch       = ldcCfg->outFmt[outCnt].pitch[0u];
                        chPrms->ccsf        = ldcCfg->outFmt[outCnt].ccsFormat;
                        chPrms->dataFmt     = ldcCfg->outFmt[outCnt].dataFormat;
                        chPrms->sl2Addr     = instObj->sl2Addr[outCnt];
                        chPrms->sl2Size     = sl2MemSize;
                        vhwaM2mLdcInitRegParams(chPrms, ldcCfg);
                        break;
                    }
                    case FVID2_DF_CHROMA_ONLY:
                    {
                        /* For chroma only mode, only second channel
                           needs to be enabled */
                        chPrms              = &hObj->chPrms[chId + 1u];
                        chPrms->isEnabled   = (uint32_t)UTRUE;
                        /* PS: Pitch is taken from offset1 in outFmt */
                        chPrms->pitch       = ldcCfg->outFmt[outCnt].pitch[1u];
                        chPrms->ccsf        = ldcCfg->outFmt[outCnt].ccsFormat;
                        chPrms->dataFmt     = ldcCfg->outFmt[outCnt].dataFormat;
                        chPrms->sl2Addr     = instObj->sl2Addr[outCnt];
                        chPrms->sl2Size     = sl2MemSize;
                        /* Chroma only mode, so no need to divide height by 2
                           as Application has provided correct height
                           for Chroma */
                        vhwaM2mLdcInitRegParams(chPrms, ldcCfg);
                        break;
                    }
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    default:
                    {
                        /* Nothing to do here */
                        break;
                    }
                    /* LDRA_JUSTIFY_END */
                }
            }

            chId += VHWA_MAX_CH_PER_OUTPUT;
        }
    }
}

static void vhwaM2mLdcCalcChSl2Params(const Vhwa_M2mLdcInstObj *instObj,
    Vhwa_M2mLdcHandleObj *hObj)
{
    uint32_t cnt;
    Vhwa_M2mLdcChParams *chPrms = NULL;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL != hObj) && (NULL != instObj))
     /* LDRA_JUSTIFY_END */
    {
        for (cnt = 0U; cnt < VHWA_M2M_LDC_OUT_DMA_CH; cnt ++)
        {
            chPrms = &hObj->chPrms[cnt];
            if ((uint32_t)UTRUE == chPrms->isEnabled)
            {
                /* Calculated max block size */
                vhwaM2mLdcCalcMaxBlockPitch(chPrms);

                /* Calculate the number of lines available in SL2 */
                chPrms->sl2NumLines = chPrms->sl2Size /
                    chPrms->maxSl2BlockPitch;

                /* Calculate the number of SL2 blocks */
                vhwaM2mLdcCalcNumSl2Blocks(chPrms);
            }
        }
    }
}

/* Function to Allocate Handle Object */
static Vhwa_M2mLdcHandleObj *vhwaM2mLdcAllocHandleObj(
    const Vhwa_M2mLdcInstObj *instObj)
{
    uint32_t cnt;
    Vhwa_M2mLdcHandleObj *hObj = NULL;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != instObj)
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
        <justification end> */
        for (cnt = 0U; cnt < VHWA_M2M_LDC_MAX_HANDLES; cnt ++)
        /* LDRA_JUSTIFY_END */
        {
            if ((uint32_t)UFALSE == gM2mLdcHandleObj[cnt].isUsed)
            {
                hObj = &gM2mLdcHandleObj[cnt];
                Fvid2Utils_memset(hObj, 0x0, sizeof(Vhwa_M2mLdcHandleObj));
                gM2mLdcHandleObj[cnt].isUsed = (uint32_t)UTRUE;
                gM2mLdcHandleObj[cnt].enableReconfigReinitReg = (uint32_t)VHWA_SAFETY_MODE_DISABLED;
                gM2mLdcHandleObj[cnt].enableStatusRegValidate = (uint32_t)VHWA_SAFETY_MODE_DISABLED;
                hObj->hIdx = cnt;
                break;
            }
        }
    }
    return (hObj);
}

/* Function to freeup allocated Handle Object */
static void vhwaM2mLdcFreeHandleObj(Vhwa_M2mLdcHandleObj *hObj)
{
    uint32_t cnt;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != hObj)
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
        <justification end> */
        for (cnt = 0U; cnt < VHWA_M2M_LDC_MAX_HANDLES; cnt ++)
        /* LDRA_JUSTIFY_END */
        {
            if (hObj == &gM2mLdcHandleObj[cnt])
            {
                hObj->isUsed = (uint32_t)UFALSE;;
                break;
            }
        }
    }
}


static int32_t vhwaM2mLdcCheckLdcCfg(const Vhwa_M2mLdcInstObj *instObj,
    Vhwa_M2mLdcHandleObj *hObj, const Ldc_Config *ldcCfg)
{
    int32_t     status = FVID2_SOK;
    uint32_t    outId;
    uint32_t    sl2Size;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL != instObj) && (NULL != hObj) && (NULL != ldcCfg))
    /* LDRA_JUSTIFY_END */
    {
        /* Check if SL2 has sufficient size to run this block size */
        for (outId = 0U; outId < LDC_MAX_OUTPUT; outId ++)
        {
            if ((uint32_t)UTRUE == ldcCfg->enableOutput[outId])
            {
                sl2Size = vhwaM2mLdcCalcSl2MemSize(ldcCfg, outId);

                /* Above API does not provide sl2 size for YUV420 data */
                if ((FVID2_DF_YUV420SP_UV == ldcCfg->outFmt[outId].dataFormat))
                {
                    sl2Size = sl2Size * 3U / 2U;
                }
                else if ((FVID2_DF_YUV422SP_UV == ldcCfg->outFmt[outId].dataFormat) ||
                         (FVID2_DF_2PLANES == ldcCfg->outFmt[outId].dataFormat))
                {
                    sl2Size = sl2Size * 2U;
                }
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications CAN reach this portion.
                No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                <justification end> */
                else if ((FVID2_DF_RGI_B == ldcCfg->outFmt[outId].dataFormat) ||
                         (FVID2_DF_RGB == ldcCfg->outFmt[outId].dataFormat))
                /* LDRA_JUSTIFY_END */
                {
                    /* 2nd Channel will required twice the SL2 size */
                    sl2Size = sl2Size * 3U;
                }
                else
                {
                    /*Do Nothing*/
                }

                /* SL2 size must be greater than the required for the
                 * max block size */
                if (instObj->sl2Size[outId] < sl2Size)
                {
                    GT_2trace(VhwaLdcTrace, GT_ERR,
                        "SL2 size insufficient for output %d: required=%u\n", outId, sl2Size);
                    status = FVID2_EALLOC;
                }
            }
        }

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (0U != hObj->numPendReq)
        {
            GT_1trace(VhwaLdcTrace, GT_ERR,
                "Handle has %u pending requests, device in use\n", hObj->numPendReq);
            status = FVID2_EDEVICE_INUSE;
        }
        /* LDRA_JUSTIFY_END */

        if (FVID2_SOK == status)
        {
            /* Initialize HTS configuration based on the new NF Config */
            vhwaM2mLdcSetHtsCfg(instObj, hObj, ldcCfg);
            /* Initialize LSE configuration based on the new NF Config */
            vhwaM2mLdcSetLseCfg(instObj, hObj, ldcCfg);
        }

        if (FVID2_SOK == status)
        {
            /* Check New HTS Configuration */
            status = CSL_htsCheckThreadConfig(&hObj->htsCfg);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK != status)
            {
                GT_0trace(VhwaLdcTrace, GT_ERR,
                    "CSL_htsCheckThreadConfig failed\n");
            }
            /* LDRA_JUSTIFY_END */
        }
        if (FVID2_SOK == status)
        {
            /* Check LSE HTS Configuration */
            status = CSL_lseCheckConfig(&hObj->lseCfg);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK != status)
            {
                GT_0trace(VhwaLdcTrace, GT_ERR,
                    "CSL_lseCheckConfig failed\n");
            }
            /* LDRA_JUSTIFY_END */
        }
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
        GT_0trace(VhwaLdcTrace, GT_ERR,
            "Invalid arguments: instObj/hObj/ldcCfg is NULL\n");
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */

    return (status);
}

static int32_t vhwaM2mLdcSetParams(const Vhwa_M2mLdcInstObj *instObj,
    Vhwa_M2mLdcHandleObj *hObj, const Ldc_Config *ldcCfg)
{
    int32_t status = FVID2_EBADARGS;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL != instObj) && (NULL != hObj) && (NULL != ldcCfg))
    /* LDRA_JUSTIFY_END */
    {
        status = vhwaM2mLdcCheckLdcCfg(instObj, hObj, ldcCfg);

        /* LDC configuration is verified */
        if (FVID2_SOK == status)
        {
            Fvid2Utils_memcpy(&hObj->ldcCfg, ldcCfg, sizeof(Ldc_Config));

            /* Initialize Channel parameters, based on the output format */
            vhwaM2mLdcInitChParamsFromLdcConfig(instObj, hObj);

            /* Calculate the SL2 information for each channel */
            vhwaM2mLdcCalcChSl2Params(instObj, hObj);

            /* Calculate the HTS depth,
             * HTS depth should be configured with
             * minimum of all region depth/Sl2 number of blocks.
             * Number of blocks for each regions are already calculated in
             * vhwaM2mLdcCalcChSl2Params. This API calculates min of
             * them all. */
            vhwaM2mLdcCalcHtsDepth(hObj);

            /* Calculate TR information for each channel */
            vhwaM2mLdcCalcChTrParams(instObj, hObj);

            /* Now set the HTS and LSE configuration, based on
             * the calculated parameters */

            /* Again set the HTS Configuration,
             * Based on the calculated parameters. */
            vhwaM2mLdcSetHtsCfg(instObj, hObj, ldcCfg);

            /* Set the LSE Configuration,
             * Based on the calculated parameters. */
            vhwaM2mLdcSetLseCfg(instObj, hObj, ldcCfg);

            /* Now create TR and initialized it from the calculated parameters
             * */
            Vhwa_m2mLdcSetTrDesc(instObj, hObj);

            /* Now set flag so that params will be loaded in HW next time
             * */
            hObj->isCfgUpdated = 1u;

        }
        else /* incorrect LDC configuration, restore HTS/LSE config */
        {
            /* Reset HTS config based on local correct LDC config */
            vhwaM2mLdcSetHtsCfg(instObj, hObj, &hObj->ldcCfg);
            /* Reset LSE config based on local correct LDC config */
            vhwaM2mLdcSetLseCfg(instObj, hObj, &hObj->ldcCfg);
        }
    }

    return (status);
}

static int32_t vhwaM2mLdcSetConfigInHW(const Vhwa_M2mLdcInstObj *instObj,
    const Vhwa_M2mLdcQueueObj *qObj)
{
    int32_t               status = FVID2_SOK;
    const Ldc_SocInfo    *socInfo = NULL;
    Vhwa_M2mLdcHandleObj *hObj = NULL;
    VhwaVpacLdcSocReadBack *goldenRegVal = NULL;

    /* Null pointer check */
    GT_assert(VhwaLdcTrace, (NULL != instObj));
    GT_assert(VhwaLdcTrace, (NULL != qObj));
    GT_assert(VhwaLdcTrace, (NULL != qObj->hObj));

    socInfo = &instObj->socInfo;
    hObj = qObj->hObj;
    goldenRegVal = hObj->configRegMemPrms.configGoldenRegPtr;

    /* Configure LDC Core */
    status = CSL_ldcSetConfig(socInfo->ldcRegs, &hObj->ldcCfg);

    /* If configRegValidate is enabled, also set the golden register values to reuse CSL code */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((FVID2_SOK == status) && (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate) && (NULL != goldenRegVal))
    /* LDRA_JUSTIFY_END */
    {
        status = CSL_ldcSetConfig(&(goldenRegVal->ldcRegs), &hObj->ldcCfg);
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
        /* Configure HWA Common Wrapper LSE */
        status = CSL_lseSetConfig(socInfo->lseRegs, &hObj->lseCfg);

        /* If configRegValidate is enabled, also set the golden register values to reuse CSL code */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps.
        Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((FVID2_SOK == status) && (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate) && (NULL != goldenRegVal))
        /* LDRA_JUSTIFY_END */
        {
            status = CSL_lseSetConfig(&(goldenRegVal->lseRegs), &hObj->lseCfg);
        }

    }
    /* Disable Luma Tone Mapping LUT in hardware if it is not enabled for the current handle */
    if((FVID2_SOK == status) && (0U == hObj->lumaLutCfg.enable))
    {
        status = CSL_ldcSetLumaToneMapLutCfg(
                        instObj->socInfo.ldcRegs,
                        instObj->socInfo.lumaLutBaseAddr, &hObj->lumaLutCfg);
        if ((FVID2_SOK == status) && (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate) && (NULL != goldenRegVal))
        {
            status = CSL_ldcSetLumaToneMapLutCfg(
                                &goldenRegVal->ldcRegs,
                                (volatile uint32_t *)goldenRegVal->ldcLumaLut.LUT, &hObj->lumaLutCfg);
        }
    }
    /* Disable Chroma Tone Mapping LUT in hardware if it is not enabled for the current handle */
    if((FVID2_SOK == status) && (0U == hObj->chromaLutCfg.enable))
    {
        status = CSL_ldcSetChromaToneMapLutCfg(
                        instObj->socInfo.ldcRegs,
                        instObj->socInfo.chromaLutBaseAddr, &hObj->chromaLutCfg);
        if ((FVID2_SOK == status) && (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate) && (NULL != goldenRegVal))
        {
            status = CSL_ldcSetChromaToneMapLutCfg(
                                &goldenRegVal->ldcRegs,
                                (volatile uint32_t *)goldenRegVal->ldcChromaLut.LUT, &hObj->chromaLutCfg);
        }
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
        /* Configure HTS */
        status = CSL_htsSetThreadConfig(socInfo->htsRegs, &hObj->htsCfg);
    }
    /* Update the LdcConfigRegisterGroup with config register values for frame specific Static Config, HTS registers */
    if(VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate)
    {
        status = vhwaM2mLdcUpdateConfigRegGroup(goldenRegVal, instObj, hObj);
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
        /* Start HTS Scheduler */
        status = CSL_htsThreadStart(socInfo->htsRegs, &hObj->htsCfg);
    }

    return (status);
}

static int32_t vhwaM2mLdcSubmitRequest(Vhwa_M2mLdcInstObj *instObj,
    Vhwa_M2mLdcQueueObj *qObj)
{
    int32_t               status = FVID2_SOK;
    Ldc_SocInfo          *socInfo = NULL;
    Vhwa_M2mLdcHandleObj *hObj = NULL;

    /* Null pointer check */
    GT_assert(VhwaLdcTrace, (NULL != instObj));
    GT_assert(VhwaLdcTrace, (NULL != qObj));
    GT_assert(VhwaLdcTrace, (NULL != qObj->hObj));

    socInfo = &instObj->socInfo;
    hObj = qObj->hObj;

    /* Update the LdcStatusRegisterGroup with valid status register values for dynamic registers */
    if(VHWA_SAFETY_MODE_DISABLED != hObj->enableStatusRegValidate)
    {
        status = vhwaM2mLdcUpdateStatusRegGroup(hObj);
    }
    else
    {
        status = FVID2_SOK;
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
        /* Set Input and output addresses */
        vhwaM2mLdcSetAddress(hObj, socInfo, &qObj->inFrmList,
            &qObj->outFrmList);

        /* Submit Rings to the Ring Accelerator */
        status = Vhwa_m2mLdcSubmitRing(instObj, hObj);
    }

    /* Update the LdcConfigRegisterGroup with config register values for frame-specific static config, INTD and HTS registers */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((FVID2_SOK == status) && (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate))
    /* LDRA_JUSTIFY_END */
    {
        VhwaVpacLdcSocReadBack *goldenRegVal = hObj->configRegMemPrms.configGoldenRegPtr;
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
            status = vhwaM2mLdcUpdateConfigRegGroup(goldenRegVal, instObj, hObj);
        }
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
        /* Start the core */
        status = CSL_ldcStart(socInfo->ldcRegs);

        /* If configRegValidate is enabled, also start on golden memory to reuse CSL code */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((FVID2_SOK == status) && (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate))
        /* LDRA_JUSTIFY_END */
        {
            VhwaVpacLdcSocReadBack *goldenRegVal = hObj->configRegMemPrms.configGoldenRegPtr;
            if (NULL != goldenRegVal)
            {
                status = CSL_ldcStart(&goldenRegVal->ldcRegs);
            }
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
            /* Better to set Active object to this q object, so that if
             * interrupt comes immediately, actQObj would be set..
             * If pipeline start fails, it would be set to NULL.*/
            instObj->actQObj = qObj;

            instObj->totalReqCnt ++;

            #ifndef VHWA_USE_PIPELINE_COMMON_ENABLE
            if (NULL != hObj->createArgs.getTimeStamp)
            {
                hObj->perfNum = hObj->createArgs.getTimeStamp();
            }
            /* Start HTS pipeline */
            status = CSL_htsPipelineStart(socInfo->htsRegs, &hObj->htsCfg);
            #endif
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
        if (FVID2_SOK != status)
        {
            instObj->actQObj = NULL;
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */
        else
        {
            status = FVID2_SOK;
        }
    }

    return (status);
}

static int32_t vhwa_m2mLdcSetEeParams(const Vhwa_M2mLdcInstObj *instObj,
    Vhwa_M2mLdcHandleObj *hObj, const Ldc_ErrEventParams *eePrms)
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
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((NULL != instObj->actQObj) && (instObj->actQObj->hObj == hObj))
        {
            GT_0trace(VhwaLdcTrace, GT_ERR,
                "Cannot change error events while handle is active !!\n");
            status = FVID2_EAGAIN;
        }
        else if (0U != (eePrms->errEvents & (~VHWA_LDC_ALL_ERR_EVENTS)))
        {
            GT_1trace(VhwaLdcTrace, GT_ERR,
                "Attempt to enable a non-existent error event: 0x%08x !!\n",
                (eePrms->errEvents & (~VHWA_LDC_ALL_ERR_EVENTS)));
            status = FVID2_EINVALID_PARAMS;
        }
        /* LDRA_JUSTIFY_END */
        else
        {
            /* Copy Error Events in Handle object, so if the error
             * comes on the next frame, it will be processed */
            Fvid2Utils_memcpy(&hObj->eePrms, eePrms,
                sizeof(Ldc_ErrEventParams));
            status = FVID2_SOK;
            hObj->isCfgUpdated = 1u;
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

static int32_t vhwa_m2mLdcSetWdTimerEeParams(const Vhwa_M2mLdcInstObj *instObj,
    Vhwa_M2mLdcHandleObj *hObj, const Ldc_WdTimerErrEventParams *wdTimerEePrms)
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
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((NULL != instObj->actQObj) && (instObj->actQObj->hObj == hObj))
        {
            GT_0trace(VhwaLdcTrace, GT_ERR,
                "Cannot change watchdog timer error events while handle is active !!\n");
            status = FVID2_EAGAIN;
        }
        else if (0U != (wdTimerEePrms->wdTimerErrEvents & (~VHWA_LDC_WDTIMER_ERR)))
        {
            GT_1trace(VhwaLdcTrace, GT_ERR,
                "Attempt to enable a non-existent watchdog timer error event: 0x%08x !!\n",
                (wdTimerEePrms->wdTimerErrEvents & (~VHWA_LDC_WDTIMER_ERR)));
            status = FVID2_EINVALID_PARAMS;
        }
        /* LDRA_JUSTIFY_END */
        else
        {
            /* Copy Error Events in Handle object, so if the error
             * comes on the next frame, it will be processed */
            Fvid2Utils_memcpy(&hObj->wdTimerEePrms, wdTimerEePrms,
                sizeof(Ldc_WdTimerErrEventParams));

            /* Enable watchdog timer in HTS if WDTIMER_ERR event is registered */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
            <justification end> */
            if (0U != (wdTimerEePrms->wdTimerErrEvents & VHWA_LDC_WDTIMER_ERR))
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
            /* LDRA_JUSTIFY_END */
            else
            {
                hObj->htsCfg.enableWdTimer = (uint32_t)UFALSE;
            }
            hObj->isCfgUpdated = 1u;
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

static int32_t Vhwa_m2mLdcSetIntrInHW(uint32_t vhwaIrqNum,
                                volatile CSL_vpac_intd_cfgRegs  *intdRegs,
                                const Vhwa_M2mLdcHandleObj *hObj)
{
    int32_t status = FVID2_SOK;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
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
        CSL_REG32_WR(&intdRegs->ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_LDC_INTR_REG], VHWA_LDC_ALL_ERR_EVENTS);
        CSL_REG32_WR(&intdRegs->ENABLE_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_LDC_INTR_REG],
            hObj->eePrms.errEvents);

        CSL_REG32_WR(&intdRegs->ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_WDTIMER_INTR_REG], VHWA_LDC_WDTIMER_ERR);
        CSL_REG32_WR(&intdRegs->ENABLE_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_WDTIMER_INTR_REG],
            hObj->wdTimerEePrms.wdTimerErrEvents);
    }

    return status;
}

static uint32_t vhwaM2mLdcCalcSl2MemSize(const Ldc_Config *cfg,
                                    uint32_t outId)
{
    uint32_t blockWidth;
    uint32_t blockHeight;
    uint32_t blockSize;

    GT_assert(VhwaLdcTrace, (NULL != cfg));
    GT_assert(VhwaLdcTrace, (outId < LDC_MAX_OUTPUT));

    /* Size has to be calculated from max of block width and
     * max of block height,
     * this is because pitch is common for all regions,
     * so, for example, 128x16 block size > 16x64, but
     * for 16x64 block size, sl2 memory required is 128x64.
     * so it is calculated from max of block width and max of block height */
    vhwaM2mLdcGetBlockSize(cfg, &blockWidth, &blockHeight);

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((FVID2_DF_YUV420SP_UV == cfg->outFmt[outId].dataFormat) ||
        (FVID2_DF_YUV422SP_UV == cfg->outFmt[outId].dataFormat) ||
        (FVID2_DF_2PLANES == cfg->outFmt[outId].dataFormat) ||
        (FVID2_DF_RGI_B == cfg->outFmt[outId].dataFormat) ||
        (FVID2_DF_RGB == cfg->outFmt[outId].dataFormat) ||
        (FVID2_DF_YUV422I_UYVY == cfg->outFmt[outId].dataFormat) ||
        (FVID2_DF_LUMA_ONLY == cfg->outFmt[outId].dataFormat) ||
        (FVID2_DF_CHROMA_ONLY == cfg->outFmt[outId].dataFormat))
    /* LDRA_JUSTIFY_END */
    {
        if (FVID2_DF_YUV422I_UYVY == cfg->outFmt[outId].dataFormat)
        {
            blockWidth = blockWidth * 2U;
        }
        if (FVID2_CCSF_BITS12_PACKED == cfg->outFmt[outId].ccsFormat)
        {
            blockWidth = blockWidth * 3U /2U;
        }
        else if (FVID2_CCSF_BITS12_UNPACKED16 ==
                    cfg->outFmt[outId].ccsFormat)
        {
            blockWidth = blockWidth * 2U;
        }
        else /* 8bit packed */
        {
            /* Nothing to do here */
        }
    }
    else
    {
    }

    /* Make block width aligned to SL2 pitch requirement */
    blockWidth = Vhwa_calcSl2Pitch(blockWidth);

    /* TODO: Add support for Depth more than 2 */
    blockSize = blockWidth * blockHeight * 2U;

    return (blockSize);
}

static void vhwaM2mLdcGetBlockSize(const Ldc_Config *cfg,
                                   uint32_t *blockWidth,
                                   uint32_t *blockHeight)
{
    uint32_t cnt0, cnt1, width, height;

    GT_assert(VhwaLdcTrace, (NULL != cfg));
    GT_assert(VhwaLdcTrace, (NULL != blockWidth));
    GT_assert(VhwaLdcTrace, (NULL != blockHeight));

    width = 0U;
    height = 0U;

    if (UTRUE == cfg->enableMultiRegions)
    {
        for (cnt0 = 0; cnt0 < LDC_MAX_VERT_REGIONS; cnt0 ++)
        {
            for (cnt1 = 0; cnt1 < LDC_MAX_HORZ_REGIONS; cnt1 ++)
            {
                if (UTRUE == cfg->regCfg.enable[cnt0][cnt1])
                {
                    if (width < cfg->regCfg.blockWidth[cnt0][cnt1])
                    {
                        width = cfg->regCfg.blockWidth[cnt0][cnt1];
                    }
                    if (height < cfg->regCfg.blockHeight[cnt0][cnt1])
                    {
                        height = cfg->regCfg.blockHeight[cnt0][cnt1];
                    }
                }
            }
        }
    }
    else
    {
        width = cfg->outputBlockWidth;
        height = cfg->outputBlockHeight;
    }

    *blockWidth = width;
    *blockHeight = height;
}

static void vhwaM2mLdcCalcNumSl2Blocks(Vhwa_M2mLdcChParams *chPrms)
{
    uint32_t                 hCnt;
    uint32_t                 vCnt;
    Vhwa_M2mLdcRegionParams *regPrms = NULL;

    for (vCnt = 0u; vCnt < LDC_MAX_VERT_REGIONS; vCnt ++)
    {
        for (hCnt = 0u; hCnt < LDC_MAX_HORZ_REGIONS; hCnt ++)
        {
            regPrms = &chPrms->regPrms[vCnt][hCnt];
            if ((uint32_t)UTRUE == regPrms->isEnabled)
            {
                regPrms->numSl2Blocks = chPrms->sl2NumLines /
                    regPrms->blockHeight;
            }
        }
    }
}

static void vhwaM2mLdcCalcMaxBlockPitch(Vhwa_M2mLdcChParams *chPrms)
{
    uint32_t                 hCnt;
    uint32_t                 vCnt;
    uint32_t                 maxBlockWidth = 0u;
    Vhwa_M2mLdcRegionParams *regPrms = NULL;

    for (vCnt = 0u; vCnt < LDC_MAX_VERT_REGIONS; vCnt ++)
    {
        for (hCnt = 0u; hCnt < LDC_MAX_HORZ_REGIONS; hCnt ++)
        {
            regPrms = &chPrms->regPrms[vCnt][hCnt];

            if ((uint32_t)UTRUE == regPrms->isEnabled)
            {
                if (regPrms->blockWidthInBytes > maxBlockWidth)
                {
                    maxBlockWidth = regPrms->blockWidthInBytes;
                }
            }
        }
    }

    /* Block pitch requires to be LSE pitch aligned/64 bytes */
    chPrms->maxSl2BlockPitch = CSL_lseMakePitchAligned(maxBlockWidth);
}

static void vhwaM2mLdcCalcChTrParams(const Vhwa_M2mLdcInstObj *instObj,
    Vhwa_M2mLdcHandleObj *hObj)
{
    uint32_t             chcnt;
    Vhwa_M2mLdcChParams *chPrms = NULL;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL != hObj) && (NULL != instObj))
    /* LDRA_JUSTIFY_END */
    {
        /* Calculate Number of TRs for each region and channels and
           calculate the address offset for each channel for each TR */
        for (chcnt = 0u; chcnt < VHWA_M2M_LDC_OUT_DMA_CH; chcnt ++)
        {
            chPrms = &hObj->chPrms[chcnt];

            if ((uint32_t)UTRUE == chPrms->isEnabled)
            {
                vhwaM2mLdcCalcRegionTrParams(chPrms);
            }
        }
    }
}

static void vhwaM2mLdcCalcRegionTrParams(Vhwa_M2mLdcChParams *chPrms)
{
    uint32_t                 hCnt;
    uint32_t                 vCnt;
    uint32_t                 numTr;
    uint32_t                 numHorzBlocks;
    uint32_t                 numVertBlocks;
    uint32_t                 totalBlocks;
    uint32_t                 remBlocks;
    uint32_t                 remHorzBlocks;
    uint32_t                 width = 0U;
    uint32_t                 height = 0U;
    uint32_t                 startAddrOffset;
    Vhwa_M2mLdcRegionParams *regPrms = NULL;

    /* This function calculates the total number of TRs required in
     * each region and the parameters for each TRs, like icnt, dims,
     * start offsets etc.
     * It uses below logic,
     * 1, Minimum 1 TR is required for each region.
     * 2, Second TR is required if frame height is not
     *    multiple of region height. In this case, first TR is
     *    used to transfer all block rows, except last row and
     *    second TR is used to transfer last row.
     * 3, Third TR is required if the last row of blocks does not
     *    start from SL2 start address ie second last row does not
     *    end on the last SL2 block. In this case,
     *    second TR is used to transfer one or two block such that
     *    start address of the next TR is SL2 start address. Then third
     *    TR is used to transfer remaining blocks of the last row.
     * */

    chPrms->totalTRs = 0u;
    for (vCnt = 0u; vCnt < LDC_MAX_VERT_REGIONS; vCnt ++)
    {
        width = 0u;

        /* Calculate the total height,
           used in calculating start offset */
        if (vCnt > 0u)
        {
            height = height + chPrms->regPrms[vCnt - 1u][0u].height;
        }

        for (hCnt = 0u; hCnt < LDC_MAX_HORZ_REGIONS; hCnt ++)
        {
            /* Calculate the total width
               Used in calculating start offset */
            if (hCnt > 0u)
            {
                width = width + chPrms->regPrms[0u][hCnt - 1u].width;
            }

            regPrms = &chPrms->regPrms[vCnt][hCnt];

            /* This is the start address offset for each region */
            startAddrOffset = (height * chPrms->pitch) +
                Vhwa_m2mLdcCalcHorzSizeInBytes(width, chPrms->ccsf,
                    chPrms->dataFmt);

            numTr = 0u;
            if ((uint32_t)UTRUE == regPrms->isEnabled)
            {
                /* Calculate total number of 2D Blocks in a frame,
                 * On horizontal side, make sure pitch is multiple
                 * of block width, even through frame width is non
                 * multiple of block width */
                numHorzBlocks = Vhwa_ceil(regPrms->width, regPrms->blockWidth);
                numVertBlocks = regPrms->height / regPrms->blockHeight;
                totalBlocks = numHorzBlocks * numVertBlocks;

                regPrms->sicnt2[numTr] = regPrms->numSl2Blocks;
                regPrms->sicnt3[numTr] =
                    Vhwa_ceil(totalBlocks, regPrms->numSl2Blocks);
                regPrms->dicnt2[numTr] = numHorzBlocks;
                regPrms->dicnt3[numTr] = numVertBlocks;

                /* Calculate the buffer offset in the output buffer */
                regPrms->bufAddrOff[numTr] = startAddrOffset;

                /* SL2 Address for the first block is always start address */
                regPrms->sl2Addr[numTr] = chPrms->sl2Addr;

                /* Minimum 1 TR is required for this transfer */
                numTr ++;

                /* More than one TR is required if output frame
                   height is not multiple of block height */
                if ((numVertBlocks * regPrms->blockHeight) != regPrms->height)
                {
                    /* Calculate the address offset in the output buffer */
                    regPrms->bufAddrOff[numTr] = startAddrOffset +
                        (chPrms->pitch * (numVertBlocks *
                            regPrms->blockHeight));

                    remBlocks = totalBlocks % regPrms->numSl2Blocks;
                    if (0u == remBlocks)
                    {
                        /* Since the last row blocks starts from the
                           SL2 start address, only one TR is required to
                           transfer last row of blocks */
                        regPrms->sicnt2[numTr] = regPrms->numSl2Blocks;
                        regPrms->sicnt3[numTr] =
                            Vhwa_ceil(numHorzBlocks, regPrms->numSl2Blocks);
                        regPrms->dicnt2[numTr] = numHorzBlocks;
                        regPrms->dicnt3[numTr] = 1u;

                        /* SL2 Offset is still 0, since last blocks of row
                           starts at start address of sl2 */
                        regPrms->sl2Addr[numTr] = chPrms->sl2Addr;

                        numTr ++;
                    }
                    else
                    {
                        /* Since the last row blocks does not start from the
                           SL2 start address, one TR is required to
                           bring TR start address to SL2 start address
                           and another TR require for the remaining
                           blocks of the last row. */

                        /* SL2 address is where the last block of the last row
                           ended in SL2 */
                        regPrms->sl2Addr[numTr] = chPrms->sl2Addr +
                                        ((uint64_t)chPrms->maxSl2BlockPitch *
                                         (uint64_t)regPrms->blockHeight *
                                         (uint64_t)remBlocks);

                        remBlocks = regPrms->numSl2Blocks - remBlocks;

                        regPrms->sicnt2[numTr] = remBlocks;
                        regPrms->sicnt3[numTr] = 1u;
                        regPrms->dicnt2[numTr] = remBlocks;
                        regPrms->dicnt3[numTr] = 1u;

                        /* One TR for flushing remainder of blocks to bring back
                           to start address of SL2 */
                        numTr ++;

                        /* Last TR required only if there are remaining
                           blocks to be transferred */
                        remHorzBlocks = numHorzBlocks - remBlocks;
                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start>
                        Rationale:
                        The value of remHorzBlocks is determined by the relationship between numHorzBlocks and numSl2Blocks,
                        and the preceding logic ensures it is never exactly zero for valid configurations.
                        As a result, the condition remHorzBlocks == 0 is not encountered during normal execution.
                        Effect on this unit:
                        The code always follows the path for remHorzBlocks != 0, making the zero-remainder branch unreachable.
                        This guarantees consistent handling of remaining horizontal blocks and prevents execution of the special case for zero remainder.
                        <justification end> */
                        if (0u != remHorzBlocks)
                        {
                            regPrms->sicnt2[numTr] = regPrms->numSl2Blocks;
                            regPrms->sicnt3[numTr] =
                                Vhwa_ceil(remHorzBlocks, regPrms->numSl2Blocks);
                            regPrms->dicnt2[numTr] = remHorzBlocks;
                            regPrms->dicnt3[numTr] = 1u;

                            regPrms->bufAddrOff[numTr] =
                                regPrms->bufAddrOff[numTr - 1u] +
                                (remBlocks * regPrms->blockWidthInBytes);

                            /* SL2 address for this region is start of
                             * SL2 address as extra TR is added to start
                             * from the first location. */
                            regPrms->sl2Addr[numTr] = chPrms->sl2Addr;

                            numTr ++;
                        }
                        /* LDRA_JUSTIFY_END */
                    }
                }
            }

            regPrms->numTr = numTr;
            chPrms->totalTRs += numTr;
        }
    }
}

static void vhwaM2mLdcSetLseCfg(const Vhwa_M2mLdcInstObj *instObj,
    Vhwa_M2mLdcHandleObj *hObj, const Ldc_Config *ldcCfg)
{
    uint32_t               chCnt;
    Vhwa_M2mLdcCreateArgs *createArgs = NULL;
    CSL_LseConfig         *lseCfg = NULL;
    Vhwa_M2mLdcChParams   *chPrms = NULL;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL != instObj) && (NULL != hObj) && (NULL != ldcCfg))
    /* LDRA_JUSTIFY_END */
    {
        lseCfg = &hObj->lseCfg;
        createArgs = &hObj->createArgs;

        /* Initialize LSE configuration with the default values */
        CSL_lseConfigInit(lseCfg);

        /* For LDC, Max number of output channels are 4 */
        lseCfg->numOutCh = VHWA_M2M_LDC_OUT_DMA_CH;

        /* Enable PSA if it is enabled in create params */
        lseCfg->enablePsa = createArgs->enablePsa;

        /* Enable the channel which are required */
        for (chCnt = 0u; chCnt < VHWA_M2M_LDC_OUT_DMA_CH; chCnt ++)
        {
            chPrms = &hObj->chPrms[chCnt];
            if ((uint32_t)UTRUE == chPrms->isEnabled)
            {
                lseCfg->outChCfg[chCnt].enable = (uint32_t)UTRUE;
                lseCfg->outChCfg[chCnt].ccsf =
                    (Fvid2_ColorCompStorageFmt)chPrms->ccsf;
                lseCfg->outChCfg[chCnt].lineOffset = chPrms->maxSl2BlockPitch;

                lseCfg->outChCfg[chCnt].circBufSize = chPrms->sl2NumLines;
                lseCfg->outChCfg[chCnt].bufAddr = (uint32_t)chPrms->sl2Addr;
                lseCfg->outChCfg[chCnt].useMultiBprParams = (uint32_t)UFALSE;
                lseCfg->outChCfg[chCnt].enableTDoneEoBPR = (uint32_t)UTRUE;
                lseCfg->outChCfg[chCnt].bpr = 1u;
                lseCfg->outChCfg[chCnt].enableVertWrap = (uint32_t)UFALSE;

                if ((FVID2_DF_YUV422I_UYVY ==
                        (Fvid2_DataFormat)chPrms->dataFmt) ||
                    (FVID2_DF_YUV422I_YUYV ==
                    (Fvid2_DataFormat)chPrms->dataFmt))
                {
                    lseCfg->outChCfg[chCnt].enableYuv422Out = (uint32_t)UTRUE;
                    lseCfg->outChCfg[chCnt].yuv422DataFmt =
                        (Fvid2_DataFormat)chPrms->dataFmt;
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications CAN reach this portion.
                No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                <justification end> */
                else if(FVID2_DF_RGI_B == (Fvid2_DataFormat)chPrms->dataFmt)
                {
                    lseCfg->outChCfg[chCnt].enableYuv422Out = (uint32_t)UTRUE;
                    lseCfg->outChCfg[chCnt].yuv422DataFmt =
                                                        FVID2_DF_YUV422I_YUYV;
                }
                /* LDRA_JUSTIFY_END */
                else
                {
                    lseCfg->outChCfg[chCnt].enableYuv422Out = (uint32_t)UFALSE;
                }
            }
        }
    }
}

static void vhwaM2mLdcSetHtsCfg(const Vhwa_M2mLdcInstObj *instObj,
    Vhwa_M2mLdcHandleObj *hObj, const Ldc_Config *ldcCfg)
{
    uint32_t             chCnt;
    CSL_HtsSchConfig    *htsCfg = NULL;
    Vhwa_M2mLdcChParams *chPrms = NULL;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL != instObj) && (NULL != hObj) && (NULL != ldcCfg))
    /* LDRA_JUSTIFY_END */
    {
        htsCfg = &hObj->htsCfg;

        CSL_htsSchConfigInit(htsCfg);

        htsCfg->schId           = CSL_HTS_HWA_SCH_LDC0;

        htsCfg->pipeline        = instObj->pipeline;
        htsCfg->enableHop       = (uint32_t)UFALSE;
        htsCfg->numHop          = 0u;
        htsCfg->enableWdTimer   = UFALSE;
        htsCfg->isWdTimer128KMode = (uint32_t)UFALSE;

        /* Enable the channel which are required */
        for (chCnt = 0; chCnt < VHWA_M2M_LDC_OUT_DMA_CH; chCnt ++)
        {
            chPrms = &hObj->chPrms[chCnt];
            if ((uint32_t)UTRUE == chPrms->isEnabled)
            {
                htsCfg->prodCfg[chCnt].enable       = (uint32_t)UTRUE;
                htsCfg->prodCfg[chCnt].consId       = CSL_HTS_CONS_IDX_UDMA;

                htsCfg->prodCfg[chCnt].threshold    = 1u;
                htsCfg->prodCfg[chCnt].cntPreLoad   = 0u;
                htsCfg->prodCfg[chCnt].cntPostLoad  = 0u;

                htsCfg->prodCfg[chCnt].depth        = chPrms->minHtsDepth;
                htsCfg->prodCfg[chCnt].countDec     = 1U;

                htsCfg->dmaConsCfg[chCnt].enable    = (uint32_t)UTRUE;
                htsCfg->dmaConsCfg[chCnt].dmaChNum  =
                    instObj->utcCh[chCnt];
                htsCfg->dmaConsCfg[chCnt].pipeline  = instObj->pipeline;
                htsCfg->dmaConsCfg[chCnt].enableStream = (uint32_t)UFALSE;
                htsCfg->dmaConsCfg[chCnt].prodId    = CSL_HTS_PROD_IDX_UDMA;
                htsCfg->dmaProdCfg[0U].numHop = ldcCfg->inFmt.height;
                    /* Default mapping for DMA */
            }
        }
    }
}

static int32_t vhwaM2mLdcCreateQueues(Vhwa_M2mLdcInstObj *instObj)
{
    int32_t              status = FVID2_SOK;
    uint32_t             qCnt;
    Vhwa_M2mLdcQueueObj *qObj;

    /* NULL pointer check */
    GT_assert(VhwaLdcTrace, (NULL != instObj));

    instObj->reqQ  = NULL;
    instObj->freeQ = NULL;

    /* Create Free queue */
    status = Fvid2Utils_constructQ(&instObj->freeQObj);
    GT_assert(VhwaLdcTrace, (FVID2_SOK == status));

    instObj->freeQ = &instObj->freeQObj;

    /* Create input queue */
    status = Fvid2Utils_constructQ(&instObj->reqQObj);
    GT_assert(VhwaLdcTrace, (FVID2_SOK == status));

    instObj->reqQ = &instObj->reqQObj;

    /* Initialize and queue the allocate queue object to free Q */
    for(qCnt = 0U; qCnt < VHWA_M2M_LDC_UDMA_RING_ENTRIES; qCnt ++)
    {
        qObj = &instObj->ldcQObj[qCnt];

        Fvid2Utils_memset(qObj, 0x0, sizeof(Vhwa_M2mLdcQueueObj));

        Fvid2Utils_queue(instObj->freeQ, &qObj->qElem, qObj);
    }

    return (status);
}

static void vhwaM2mLdcDeleteQueues(Vhwa_M2mLdcInstObj *instObj)
{
    Vhwa_M2mLdcQueueObj *qObj = NULL;

    /* NULL pointer check */
    GT_assert(VhwaLdcTrace, (NULL != instObj));

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(NULL != instObj->freeQ)
    /* LDRA_JUSTIFY_END */
    {
        /* Free-up all the queued free queue objects */
        do
        {
            qObj = (Vhwa_M2mLdcQueueObj *) Fvid2Utils_dequeue(instObj->freeQ);
        } while (NULL != qObj);

        /* Delete the free Q */
        Fvid2Utils_destructQ(instObj->freeQ);
        instObj->freeQ = NULL;
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(NULL != instObj->reqQ)
    /* LDRA_JUSTIFY_END */
    {
        /* Free-up all object from input queue */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        do
        {
            qObj = (Vhwa_M2mLdcQueueObj *) Fvid2Utils_dequeue(instObj->reqQ);
        } while (NULL != qObj);
        /* LDRA_JUSTIFY_END */
        /* Delete the input Q */
        Fvid2Utils_destructQ(instObj->reqQ);
        instObj->reqQ = NULL;
    }
}

static void vhwaM2mLdcSetAddress(Vhwa_M2mLdcHandleObj *hObj,
    Ldc_SocInfo *socInfo, const Fvid2_FrameList *inFrmList,
    const Fvid2_FrameList *outFrmList)
{
    uint32_t     chId;
    uint32_t     chCnt;
    uint32_t     outCnt;
    Fvid2_Frame *frm = NULL;
    Ldc_Config  *ldcCfg = NULL;

    /* NULL pointer check */
    GT_assert(VhwaLdcTrace, (NULL != hObj));
    GT_assert(VhwaLdcTrace, (NULL != socInfo));
    GT_assert(VhwaLdcTrace, (NULL != inFrmList));
    GT_assert(VhwaLdcTrace, (NULL != outFrmList));

    ldcCfg = &hObj->ldcCfg;
    frm = inFrmList->frames[0U];

    /* Configure Input Buffer address in LDC */
    CSL_ldcSetInAddress(socInfo->ldcRegs, frm->addr[0], frm->addr[1]);

    /* If config register validation is enabled, also write input addresses to golden memory */
    if (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate)
    {
        VhwaVpacLdcSocReadBack *goldenRegVal = hObj->configRegMemPrms.configGoldenRegPtr;
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != goldenRegVal)
        /* LDRA_JUSTIFY_END */
        {
            CSL_ldcSetInAddress(&goldenRegVal->ldcRegs, frm->addr[0], frm->addr[1]);
        }
    }

    chId = 0u;
    for (outCnt = 0u; outCnt < LDC_MAX_OUTPUT; outCnt ++)
    {
        frm = outFrmList->frames[outCnt];
        for (chCnt = 0u;
                ((uint32_t)UTRUE == ldcCfg->enableOutput[outCnt]) &&
                (chCnt < VHWA_MAX_CH_PER_OUTPUT); chCnt ++)
        {
            if ((uint32_t)UTRUE == hObj->chPrms[chId].isEnabled)
            {
                hObj->chPrms[chId].startAddr = frm->addr[chCnt];
            }

            chId ++;
        }
    }

    /* For Y1 + Y2 + Y3 channel 3 is also required */
    if(FVID2_DF_RGB == ldcCfg->outFmt[0].dataFormat)
    {
        hObj->chPrms[3u].startAddr = outFrmList->frames[0]->addr[2u];
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
    <justification end> */
    else if(FVID2_DF_RGI_B == ldcCfg->outFmt[0].dataFormat)
    {
        hObj->chPrms[3u].startAddr = outFrmList->frames[0]->addr[1u];
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        /*Do Nothing*/
    }

    /* Configure Buffer Addresses in TR Record */
    Vhwa_m2mLdcSetOutputAddress(hObj);
}

static int32_t vhwaM2mLdcAllocSl2(Vhwa_M2mLdcInstObj *instObj,
    const Vhwa_M2mLdcSl2AllocPrms *sl2AllocPrms)
{
    int32_t             status = FVID2_SOK;
    uint32_t            cnt;
    uint32_t            size;
    uint32_t            sl2MemSize = 0U;
    uint64_t            sl2Addr;
    uint32_t            blockWidthInBytes;

    GT_assert(VhwaLdcTrace, (NULL != instObj));
    GT_assert(VhwaLdcTrace, (NULL != sl2AllocPrms));

    for (cnt = 0U; cnt < LDC_MAX_OUTPUT; cnt ++)
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)UTRUE == sl2AllocPrms->enableOutput[cnt])
        /* LDRA_JUSTIFY_END */
        {
            /* Minimum 2 blocks are required */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (sl2AllocPrms->maxNumBlocks[cnt] <
                    VHWA_M2M_LDC_MIN_SL2_BLOCKS)
            {
                GT_0trace(VhwaLdcTrace, GT_ERR,
                    "Incorrect value for maxNumBlocks !!\n");
                status = FVID2_EINVALID_PARAMS;
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
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        for (cnt = 0U; cnt < LDC_MAX_OUTPUT; cnt ++)
        {
            instObj->sl2Size[cnt] = 0u;
            instObj->sl2Addr[cnt] = 0u;

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the caller before the control reaches here.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((uint32_t)UTRUE == sl2AllocPrms->enableOutput[cnt])
            /* LDRA_JUSTIFY_END */
            {
                /* Calculate block width in terms of bytes,
                   using storage format */
                blockWidthInBytes = Vhwa_calcHorzSizeInBytes(
                    sl2AllocPrms->maxBlockWidth,
                    sl2AllocPrms->outCcsf[cnt]);

                /* Align block width in bytes to SL2 pitch ie 64byte aligned */
                blockWidthInBytes = Vhwa_calcSl2Pitch(blockWidthInBytes);

                size = blockWidthInBytes * sl2AllocPrms->maxBlockHeight *
                    sl2AllocPrms->maxNumBlocks[cnt];

                /* For YUV420 output, two channels are required and for chroma
                   channel in YUV420, size is exactly half of luma size.
                   Assumes there are always two channels, no support
                   for allocation for single channel */
            #if defined VHWA_VPAC_IP_REV_VPAC
                sl2MemSize = size + (size >> 1);
            #elif defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
                /* Assuming R+GB usecase */
                sl2MemSize = size + (size << 1);
            #endif

                /* Apply 16KB alignment for size and address if requested for firewall support */
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This is the fallback path if 16KB alignment is not requested. The test framework does not support the configuration required to trigger this scenario.
                Effect on this unit: If the control reaches here, SL2 memory is allocated without 16KB alignment, and the function continues execution as normal.
                This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((uint32_t)TRUE == sl2AllocPrms->enable16KBAlignment)
                /* LDRA_JUSTIFY_END */
                {
                    sl2MemSize = ((sl2MemSize + 0x3FFFu) & ~0x3FFFu);
                    sl2Addr = Vhwa_allocateSl2Aligned(sl2MemSize, VHWA_SL2_INST_VPAC, 0x4000u);
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This is the fallback path if 16KB alignment is not requested. The test framework does not support the configuration required to trigger this scenario.
                Effect on this unit: If the control reaches here, SL2 memory is allocated without 16KB alignment, and the function continues execution as normal.
                This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
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
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (0U == sl2Addr)
                {
                    GT_1trace(VhwaLdcTrace, GT_ERR,
                        "SL2 aligned allocation failed for output%d (check alignment=0x4000 is power of 2 and sufficient SL2 memory available)\n", cnt);
                    status = FVID2_EALLOC;
                    break;
                }
                /* LDRA_JUSTIFY_END */
                else
                {
                    instObj->sl2Size[cnt] = sl2MemSize;
                    instObj->sl2Addr[cnt] = sl2Addr;
                }
            }
        }
    }

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK != status)
    {
        /* error occured, so free up the memory allocated for SL2 */
        for (cnt = 0U; cnt < LDC_MAX_OUTPUT; cnt ++)
        {
            if (((uint32_t)UTRUE == sl2AllocPrms->enableOutput[cnt]) &&
                (0U != instObj->sl2Size[cnt]))
            {
                Vhwa_FreeSl2(VHWA_SL2_INST_VPAC);

                instObj->sl2Size[cnt] = 0U;
                instObj->sl2Addr[cnt] = 0U;
            }
        }
        /* Error in memory allocation, open will not be allowed. */
        instObj->isSl2AllocDone = (uint32_t)UFALSE;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        /* Memory is allocated successfully, now open is allowed. */
        instObj->isSl2AllocDone = (uint32_t)UTRUE;

        Fvid2Utils_memcpy(&instObj->sl2AllocPrms, sl2AllocPrms,
            sizeof(Vhwa_M2mLdcSl2AllocPrms));
    }

    return (status);
}
static int32_t vhwaM2mLdcCheckCreatePrms(uint32_t drvId, uint32_t drvInstId)
{
    int32_t             status = FVID2_SOK;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_VHWA_M2M_LDC_DRV_ID != drvId)
    {
        GT_0trace(VhwaLdcTrace, GT_ERR, "Invalid Driver ID !!\n");
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
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (VHWA_M2M_VPAC_0_LDC_DRV_INST_ID_0 != drvInstId)
        {
            status = FVID2_EINVALID_PARAMS;
            GT_0trace(VhwaLdcTrace, GT_ERR, "Invalid/unsupported Instance Id\n");
        }
        /* LDRA_JUSTIFY_END */
#endif
#endif

#if defined VHWA_M2M_VPAC_INSTANCE
#if (VHWA_M2M_VPAC_INSTANCE == 1)
        if (VHWA_M2M_VPAC_1_LDC_DRV_INST_ID_0 != drvInstId)
        {
            status = FVID2_EINVALID_PARAMS;
            GT_0trace(VhwaLdcTrace, GT_ERR, "Invalid/unsupported Instance Id\n");
        }
#endif
#endif
    }

    return (status);
}

static int32_t Vhwa_m2mLdcReconfigReinitReg(Vhwa_M2mLdcInstObj *instObj,
                                            const Vhwa_M2mLdcHandleObj *hObj,
                                            const Vhwa_M2mLdcQueueObj *qObj)
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
        if (instObj->lastHndlObj == qObj->hObj)
        {
            /* Last handle was same as new handle,
             * so Buffer Parameters have already been reconfigured */

            status = vhwaM2mLdcSetConfigInHW(instObj, qObj);

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
            status = CSL_ldcSetLumaToneMapLutCfg(
                instObj->socInfo.ldcRegs,
                instObj->socInfo.lumaLutBaseAddr, &hObj->lumaLutCfg);

            /* If config register validation is enabled, also write LUT to golden memory */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((FVID2_SOK == status) && (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate))
            /* LDRA_JUSTIFY_END */
            {
                VhwaVpacLdcSocReadBack *goldenRegVal = hObj->configRegMemPrms.configGoldenRegPtr;
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != goldenRegVal)
                /* LDRA_JUSTIFY_END */
                {
                    status = CSL_ldcSetLumaToneMapLutCfg(
                        &goldenRegVal->ldcRegs,
                        (volatile uint32_t *)goldenRegVal->ldcLumaLut.LUT, &hObj->lumaLutCfg);
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    The test framework does not support the configuration required to trigger this error scenario.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if (FVID2_SOK != status)
                    {
                        GT_0trace(VhwaLdcTrace, GT_ERR,
                            "Reconfig: LUMA LUT failed to write to golden memory\n");
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
                    GT_0trace(VhwaLdcTrace, GT_ERR,
                        "Reconfig: goldenRegVal is NULL for LUMA LUT\n");
                    status = FVID2_EFAIL;
                }
                /* LDRA_JUSTIFY_END */
            }
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
            status = CSL_ldcSetChromaToneMapLutCfg(
                instObj->socInfo.ldcRegs,
                instObj->socInfo.chromaLutBaseAddr, &hObj->chromaLutCfg);

            /* If config register validation is enabled, also write LUT to golden memory */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((FVID2_SOK == status) && (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate))
            /* LDRA_JUSTIFY_END */
            {
                VhwaVpacLdcSocReadBack *goldenRegVal = hObj->configRegMemPrms.configGoldenRegPtr;
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
                    status = CSL_ldcSetChromaToneMapLutCfg(
                        &goldenRegVal->ldcRegs,
                        (volatile uint32_t *)goldenRegVal->ldcChromaLut.LUT, &hObj->chromaLutCfg);
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    The test framework does not support the configuration required to trigger this error scenario.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if (FVID2_SOK != status)
                    {
                        GT_0trace(VhwaLdcTrace, GT_ERR,
                            "Reconfig: CHROMA LUT failed to write to golden memory\n");
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
                    GT_0trace(VhwaLdcTrace, GT_ERR,
                        "Reconfig: goldenRegVal is NULL for CHROMA LUT\n");
                    status = FVID2_EFAIL;
                }
                /* LDRA_JUSTIFY_END */
            }
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
            status = CSL_ldcSetBwLimitConfig(instObj->socInfo.ldcRegs, &hObj->ldcCfg.rdBwLimitCfg);

            /* If configRegValidate is enabled, also update golden memory to reuse CSL code */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((FVID2_SOK == status) && (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate))
            /* LDRA_JUSTIFY_END */
            {
                VhwaVpacLdcSocReadBack *goldenRegVal = hObj->configRegMemPrms.configGoldenRegPtr;
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != goldenRegVal)
                /* LDRA_JUSTIFY_END */
                {
                    status = CSL_ldcSetBwLimitConfig(&goldenRegVal->ldcRegs, &hObj->ldcCfg.rdBwLimitCfg);
                }
            }
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
            CSL_vpacEnableModule(instObj->socInfo.vpacCntlRegs,
                                 VPAC_MODULE_LDC, (uint32_t)UTRUE);
            if (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate)
            {
                VhwaVpacLdcSocReadBack *goldenRegVal = hObj->configRegMemPrms.configGoldenRegPtr;
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != goldenRegVal)
                /* LDRA_JUSTIFY_END */
                {
                    CSL_vpacEnableModule(&goldenRegVal->vpacCntlRegs,
                                         VPAC_MODULE_LDC, (uint32_t)UTRUE);
                }
            }
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
            /* Re-Init Pipeline interrupt in INTD */
            Vhwa_enableHtsIntr(instObj->socInfo.vpacIntdRegs,
                instObj->vhwaIrqNum, instObj->pipeline);
        }
    }
    return status;
}

static int32_t vhwaM2mLdcUpdateStatusRegGroup(Vhwa_M2mLdcHandleObj *hObj)
{
    LdcStatusRegisterGroup *statusRegs;
    int32_t status = FVID2_SOK;
    uint32_t expected_val = 0U;

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

        uint32_t i;
        /* Update the expected values for DST_BUF_CFG_CH_STATUS[MSC_MAX_OUTPUT] using mask for CSL_LSE_DST_BUF_CFG_CH_DISABLED_MASK,
        * if Yuv422 is enabled on previous LSE output, the current channel will be disabled
        */
        for (i = 1U; i < 4U; i++)
        {
            expected_val = 0U;
            /* Read the previous LSE DST BUF_CFG register value */
            uint32_t reg_val = CSL_REG32_RD(&gM2mLdcInstObj.socInfo.lseRegs->DST[i - 1U].BUF_CFG);

            /* Check if YUV422_OUT_EN is set using CSL_FEXT */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
            <justification end> */
            if (1U == CSL_FEXT(reg_val, LSE_DST_BUF_CFG_YUV422_OUT_EN))
            {
                /* If YUV422_OUT_EN is set, set CH_DISABLED to true (1) for current LSE config */
                CSL_FINS(expected_val, LSE_DST_BUF_CFG_CH_DISABLED, (uint32_t)1U);
            }
            /* LDRA_JUSTIFY_END */
            else
            {
                CSL_FINS(expected_val, LSE_DST_BUF_CFG_CH_DISABLED, (uint32_t)0U);
            }
            statusRegs->lseStatus.DST_BUF_CFG_CH_STATUS[i] = expected_val;
        }
        /* Update PROD_COUNT */
        for (i = 0; i < CSL_HTS_NUM_LDC_PRODUCERS; i++)
        {
            const CSL_HtsSchConfig *cfg = &hObj->htsCfg;
            const CSL_HtsProdConfig *prodCfg = &cfg->prodCfg[i];

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the caller before the control reaches here.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((prodCfg != NULL) && (prodCfg->enable == UTRUE))
            /* LDRA_JUSTIFY_END */
            {
                expected_val = 0U;
                CSL_FINS(expected_val, HTS_HWA2_PROD0_COUNT_COUNT_POSTLOAD, prodCfg->cntPostLoad);
                statusRegs->vpacHtsStatus.PROD_COUNT[i] = expected_val;
            }
            else
            {
                statusRegs->vpacHtsStatus.PROD_COUNT[i] = 0U;
            }
        }

        for (i = 0; i < CSL_HTS_NUM_LDC_CONSUMERS; i++)
        {
            #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
            const CSL_HtsSchConfig *cfg = &hObj->htsCfg;
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((cfg != NULL) && (cfg->tdoneCntEnable == UTRUE))
            {
                expected_val = 0U;
                CSL_FINS(expected_val, HTS_HWA2_HOP_TDONE_COUNT, cfg->numHop);
                statusRegs->vpacHtsStatus.HWA_HOP[i] = expected_val;
            }
            /* LDRA_JUSTIFY_END */
            else
            {
                statusRegs->vpacHtsStatus.HWA_HOP[i] = 0U;
            }
            #endif
        }
        /* Update PROD_DMA_HOP and PROD_DMA_COUNT */
        for (i = 0; i < CSL_HTS_NUM_LDC_PRODUCERS_DMA; i++)
        {
            const CSL_HtsSchConfig *cfg = &hObj->htsCfg;
            const CSL_HtsDmaProdConfig *dmaProdCfg = &cfg->dmaProdCfg[i];

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the caller before the control reaches here.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((dmaProdCfg != NULL) && (dmaProdCfg->enable == UTRUE))
            /* LDRA_JUSTIFY_END */
            {
                expected_val = 0U;
                CSL_FINS(expected_val, HTS_DMA8_PROD0_COUNT_COUNT_POSTLOAD, dmaProdCfg->cntPostLoad);
                statusRegs->vpacHtsStatus.PROD_DMA_COUNT[i] = expected_val;
                #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
                expected_val = 0U;
                CSL_FINS(expected_val, HTS_DMA8_HOP_TDONE_COUNT, dmaProdCfg->numHop);
                statusRegs->vpacHtsStatus.PROD_DMA_HOP[i] = expected_val;
                #endif
            }
            else
            {
                statusRegs->vpacHtsStatus.PROD_DMA_COUNT[i] = 0U;
                statusRegs->vpacHtsStatus.PROD_DMA_HOP[i] = 0U;
            }
        }
    }

    return status;
}


int32_t Vhwa_m2mLdcStatusRegValidate(Vhwa_M2mLdcHandleObj *hObj, const Ldc_SocInfo *socInfo)
{
    int32_t status = FVID2_SOK;
    LdcStatusRegisterGroup *expected = NULL;
    const CSL_HtsHwaParams *htsPrms = NULL;
    expected = &hObj->statusRegs;
    htsPrms         = GetVpacHtsParams(CSL_HTS_HWA_SCH_LDC0);
    uint32_t regVal;
    uint32_t warning_bits_mask;

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

    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        /* Validate PID values in VPAC */
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
            GT_0trace(VhwaLdcTrace, GT_ERR, "StatusRegValidate: vpacCntlRegs->PID mismatch\n");
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
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        /* Validate PID values in LDC */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)VHWA_HW_PID_LDC != CSL_REG32_RD(&socInfo->ldcRegs->REVISION_REG))
        {
            GT_0trace(VhwaLdcTrace, GT_ERR, "StatusRegValidate: ldcRegs->REVISION_REG mismatch\n");
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
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        regVal = CSL_REG32_RD(&socInfo->ldcRegs->PRIVATE_MEMSIZE);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)VHWA_VPAC_LDC_PRIVATE_MEM_SIZE_RESET_VALUE != regVal)
        {
            GT_1trace(VhwaLdcTrace, GT_ERR, "StatusRegValidate: ldcRegs->PRIVATE_MEMSIZE is %d\n", regVal);
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
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        regVal = CSL_REG32_RD(&socInfo->ldcRegs->FR_PDFTCH);
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
            GT_1trace(VhwaLdcTrace, GT_INFO, "StatusRegValidate: ldcRegs->FR_PDFTCH is %d\n", regVal);
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
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        regVal = CSL_REG32_RD(&socInfo->ldcRegs->FR_MDFTCH);
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
            GT_1trace(VhwaLdcTrace, GT_INFO, "StatusRegValidate: ldcRegs->FR_MDFTCH is %d\n", regVal);
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
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        regVal = CSL_REG32_RD(&socInfo->ldcRegs->PIXMEMOVF_BLK);
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
            GT_1trace(VhwaLdcTrace, GT_INFO, "StatusRegValidate: ldcRegs->PIXMEMOVF_BLK is %d\n", regVal);
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
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        regVal = CSL_REG32_RD(&socInfo->ldcRegs->MESHMEMOVF_BLK);
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
            GT_1trace(VhwaLdcTrace, GT_INFO, "StatusRegValidate: ldcRegs->MESHMEMOVF_BLK is %d\n", regVal);
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
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        regVal = CSL_REG32_RD(&socInfo->ldcRegs->OUTOFBOUND_BLK);
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
            GT_1trace(VhwaLdcTrace, GT_INFO, "StatusRegValidate: ldcRegs->OUTOFBOUND_BLK is %d\n", regVal);
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
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        regVal = CSL_REG32_RD(&socInfo->ldcRegs->CTRL) & (CSL_LDC_CORE_CTRL_BUSY_MASK);
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
           GT_0trace(VhwaLdcTrace, GT_ERR, "StatusRegValidate: LDC IP in busy state\n");
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
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        /* Validate LDC ERR_STATUS - error bits should be 0 (no errors) */
        regVal = CSL_REG32_RD(&socInfo->ldcRegs->ERR_STATUS);
        warning_bits_mask = (CSL_LDC_CORE_ERR_STATUS_P_IBLK_OUTB_MASK | 
            CSL_LDC_CORE_ERR_STATUS_M_IBLK_OUTB_MASK | 
            CSL_LDC_CORE_ERR_STATUS_IFRAME_OUTB_MASK);

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        /* Check warning bits and report warning if mismatch */
        if ((regVal & warning_bits_mask) != 0U)
        {
            GT_1trace(VhwaLdcTrace, GT_INFO, "StatusRegValidate: ldcRegs->ERR_STATUS warning bits mismatch, value: 0x%X\n", (regVal & warning_bits_mask));
        }

        /* Check the rest of the bits and report error if mismatch */
        if ((regVal & ~warning_bits_mask) != 0U)
        {
            GT_1trace(VhwaLdcTrace, GT_ERR, "StatusRegValidate: ldcRegs->ERR_STATUS error bits mismatch, value: 0x%X\n", (regVal & ~warning_bits_mask));
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
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        /* Validate LSE status */
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
            GT_0trace(VhwaLdcTrace, GT_ERR, "StatusRegValidate: lseRegs->STATUS_ERROR mismatch\n");
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
            GT_0trace(VhwaLdcTrace, GT_ERR, "StatusRegValidate: lseRegs->STATUS_IDLE_MODE mismatch\n");
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
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        /* Validate DST_BUF_CFG_CH_STATUS */
        for (uint32_t i = 0; i < 4U; i++)
        {
            uint32_t expected_val = (expected->lseStatus.DST_BUF_CFG_CH_STATUS[i] & CSL_LSE_DST_BUF_CFG_CH_DISABLED_MASK);
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
                GT_1trace(VhwaLdcTrace, GT_ERR, "StatusRegValidate: lseRegs->DST[%d].BUF_CFG mismatch\n", i);
                status = FVID2_EFAIL;
            }
            /* LDRA_JUSTIFY_END */
        }
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        /* LDC uses HWA2 scheduler */
        uint32_t scheduler_base_addr = (uint32_t)socInfo->htsRegs + htsPrms->schOffset;

        /* Validate HTS SCHEDULER_CONTROL for HWA2 (LDC0) */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)FVID2_SOK !=
            (CSL_REG32_RD(scheduler_base_addr) & CSL_HTS_HWA2_SCHEDULER_CONTROL_STATE_MASK))
        {
            GT_0trace(VhwaLdcTrace, GT_ERR, "StatusRegValidate: htsRegs->HWA2_SCHEDULER_CONTROL mismatch\n");
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (status == FVID2_SOK)
        /* LDRA_JUSTIFY_END */
        {
            /* Validate HTS WD_TIMER status for HWA2 */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((uint32_t)FVID2_SOK !=
                (CSL_REG32_RD(scheduler_base_addr + HWA_WDTIMER_OFFSET) & CSL_HTS_HWA2_WDTIMER_WDTIMER_STATUS_MASK))
            {
                GT_0trace(VhwaLdcTrace, GT_ERR, "StatusRegValidate: htsRegs->HWA2_WDTIMER mismatch\n");
                status = FVID2_EFAIL;
            }
            /* LDRA_JUSTIFY_END */
        }

        #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (status == FVID2_SOK)
        /* LDRA_JUSTIFY_END */
        {
            /* Validate HTS HWA HOP register - should be 0 since LDC disables HOP */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((expected->vpacHtsStatus.HWA_HOP[0] & CSL_HTS_HWA2_HOP_TDONE_COUNT_MASK) !=
                    (CSL_REG32_RD(scheduler_base_addr + HWA_HOP_OFFSET) & CSL_HTS_HWA2_HOP_TDONE_COUNT_MASK))
            {
                GT_0trace(VhwaLdcTrace, GT_ERR, "StatusRegValidate: htsRegs->HWA2_HOP mismatch\n");
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
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (status == FVID2_SOK)
        /* LDRA_JUSTIFY_END */
        {
            uint32_t prod_count_addr;
            CSL_HtsSchConfig *cfg = &hObj->htsCfg;

            for (uint32_t i = 0; i < CSL_HTS_NUM_LDC_PRODUCERS; i++)
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
                    if ((expected->vpacHtsStatus.PROD_COUNT[i] & CSL_HTS_HWA2_PROD0_COUNT_COUNT_MASK) !=
                        (CSL_REG32_RD(prod_count_addr + HWA_PROD_COUNT_OFFSET) & CSL_HTS_HWA2_PROD0_COUNT_COUNT_MASK))
                    {
                        GT_1trace(VhwaLdcTrace, GT_ERR, "StatusRegValidate: htsRegs->LDC0_PROD_COUNT[%d] mismatch\n", i);
                        status = FVID2_EFAIL;
                    }
                    if ((uint32_t)FVID2_SOK != (CSL_REG32_RD(prod_count_addr + HWA_PROD_PA_PROD_COUNT_OFFSET) & CSL_HTS_HWA2_PA0_PRODCOUNT_PA_PSCOUNT_MASK))
                    {
                        GT_1trace(VhwaLdcTrace, GT_ERR, "StatusRegValidate: htsRegs->LDC0_PA_PROD_COUNT[%d] mismatch\n", i);
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
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (status == FVID2_SOK)
        /* LDRA_JUSTIFY_END */
        {
            for (uint32_t i = 0; i < CSL_HTS_NUM_LDC_PRODUCERS_DMA; i++)
            {
                CSL_HtsSchConfig *cfg = &hObj->htsCfg;
                CSL_HtsDmaProdConfig *dmaProdCfg = &cfg->dmaProdCfg[i];

                if ((dmaProdCfg != NULL) && (dmaProdCfg->enable == UTRUE))
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
                        (CSL_REG32_RD(prod_dma_addr) & CSL_HTS_DMA8_SCHEDULER_CONTROL_STATE_MASK))
                    {
                        GT_1trace(VhwaLdcTrace, GT_ERR, "StatusRegValidate: htsRegs->LDC0_PROD_DMA_SCHEDULER_CONTROL[%d] mismatch\n", i);
                        status = FVID2_EFAIL;
                    }
                    /* Validate PROD_DMA_COUNT */
                    if ((expected->vpacHtsStatus.PROD_DMA_COUNT[i] & CSL_HTS_DMA8_PROD0_COUNT_COUNT_MASK) !=
                        (CSL_REG32_RD(prod_dma_addr + HWA_DMA_PROD_COUNT_OFFSET) & CSL_HTS_DMA8_PROD0_COUNT_COUNT_MASK))
                    {
                        GT_1trace(VhwaLdcTrace, GT_ERR, "StatusRegValidate: htsRegs->PROD_DMA_COUNT[%d] mismatch\n", i);
                        status = FVID2_EFAIL;
                    }

                    #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
                    /* Validate DMA producer HOP register - should be 0 since LDC disables HOP */
                    if ((expected->vpacHtsStatus.PROD_DMA_HOP[i] & CSL_HTS_DMA8_HOP_TDONE_COUNT_MASK) !=
                            (CSL_REG32_RD(prod_dma_addr + HWA_DMA_PROD_HOP_OFFSET) & CSL_HTS_DMA8_HOP_TDONE_COUNT_MASK))
                    {
                        GT_1trace(VhwaLdcTrace, GT_ERR, "StatusRegValidate: htsRegs->LDC0_PROD_DMA_HOP[%d] mismatch\n", i);
                        status = FVID2_EFAIL;
                    }
                    #endif

                    /* Validate PA_PRODCOUNT_DMA */
                    if ((uint32_t)FVID2_SOK != (CSL_REG32_RD(prod_dma_addr + HWA_DMA_PROD_PA_PROD_COUNT_OFFSET) & CSL_HTS_DMA8_PA0_PRODCOUNT_PA_PSCOUNT_MASK))
                    {
                        GT_1trace(VhwaLdcTrace, GT_ERR, "StatusRegValidate: htsRegs->PA_PRODCOUNT_DMA[%d] mismatch\n", i);
                        status = FVID2_EFAIL;
                    }
                    /* LDRA_JUSTIFY_END */
                }
            }
        }

        /* Validate DMA consumer scheduler control status */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (status == FVID2_SOK)
        /* LDRA_JUSTIFY_END */
        {
            for (uint32_t i = 0; i < CSL_HTS_NUM_LDC_CONSUMERS_DMA; i++)
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
                    (CSL_REG32_RD(cons_dma_addr) & CSL_HTS_DMA272_SCHEDULER_CONTROL_STATE_MASK))
                {
                    GT_1trace(VhwaLdcTrace, GT_ERR, "StatusRegValidate: htsRegs->LDC0_CONSUMER_DMA_SCHEDULER_CONTROL[%d] mismatch\n", i);
                    status = FVID2_EFAIL;
                }
                /* LDRA_JUSTIFY_END */
            }
        }
    }

    return status;
}

int32_t vhwaM2mLdcSetDefaultGoldenRegMemValues(const Vhwa_M2mLdcHandleObj *hObj, const Vhwa_M2mLdcInstObj *instObj)
{
    VhwaVpacLdcSocReadBack *goldenReg = NULL;
    int32_t status = FVID2_SOK;
    uint32_t reg_val;

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
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        goldenReg = hObj->configRegMemPrms.configGoldenRegPtr;

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL == goldenReg)
        {
            status = FVID2_EBADARGS;
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
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        CSL_ldc_coreRegs *ldcRegs = instObj->socInfo.ldcRegs;
        CSL_lseRegs *lseRegs = instObj->socInfo.lseRegs;

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((NULL == ldcRegs) || (NULL == lseRegs))
        {
            status = FVID2_EBADARGS;
        }
        /* LDRA_JUSTIFY_END */
        else
        {
            /* Set default reset values for VBUSMR_CFG register */
            reg_val = 0U;
            CSL_FINS(reg_val, LDC_CORE_VBUSMR_CFG_TAG_CNT, 0x1fU);
            CSL_FINS(reg_val, LDC_CORE_VBUSMR_CFG_MAX_BURSTLEN, 0x1U);
            goldenReg->ldcRegs.VBUSMR_CFG = reg_val;

            /* Set default reset values for DUALOUT_CFG register */
            reg_val = 0U;
            CSL_FINS(reg_val, LDC_CORE_DUALOUT_CFG_COUT_BITDPTH, (uint32_t)0x8U);
            CSL_FINS(reg_val, LDC_CORE_DUALOUT_CFG_CIN_BITDPTH, (uint32_t)0xcU);
            CSL_FINS(reg_val, LDC_CORE_DUALOUT_CFG_YOUT_BITDPTH, (uint32_t)0x8U);
            CSL_FINS(reg_val, LDC_CORE_DUALOUT_CFG_YIN_BITDPTH, (uint32_t)0xcU);
            goldenReg->ldcRegs.DUALOUT_CFG = reg_val;

            /* Set default reset values for CTRL register */
            reg_val = 0U;
            CSL_FINS(reg_val, LDC_CORE_CTRL_OP_DATAMODE, (uint32_t)0x1U);
            CSL_FINS(reg_val, LDC_CORE_CTRL_IP_DFMT, (uint32_t)0x1U);
            CSL_FINS(reg_val, LDC_CORE_CTRL_IP_DATAMODE, (uint32_t)0x2U);
            #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
            CSL_FINS(reg_val, LDC_CORE_CTRL_CH_IP_DFMT, (uint32_t)0x1U);  /* Chroma channel input format reset value */
            #endif
            goldenReg->ldcRegs.CTRL = reg_val;

            #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
            /* Set Deafult values for HYBD_CBUFF_PARAM */
            reg_val = 0U;
            CSL_FINS(reg_val, LDC_CORE_HYBD_CBUFF_PARAM_STARTLINE, (uint32_t)0x1fffU);
            CSL_FINS(reg_val, LDC_CORE_HYBD_CBUFF_PARAM_ENDLINE, (uint32_t)0x1fffU);
            goldenReg->ldcRegs.HYBD_CBUFF_PARAM = reg_val;

            /* Set Deafult values for HYBD_CHCBUFF_PARAM */
            reg_val = 0U;
            CSL_FINS(reg_val, LDC_CORE_HYBD_CHCBUFF_PARAM_STARTLINE, (uint32_t)0x1fffU);
            CSL_FINS(reg_val, LDC_CORE_HYBD_CHCBUFF_PARAM_ENDLINE, (uint32_t)0x1fffU);
            goldenReg->ldcRegs.HYBD_CHCBUFF_PARAM = reg_val;
            #endif
            
            status = CSL_ldcSetChromaToneMapLutCfg(
                                &goldenReg->ldcRegs,
                                (volatile uint32_t *)goldenReg->ldcChromaLut.LUT, &hObj->chromaLutCfg);
            if (FVID2_SOK == status)
            {
                status = CSL_ldcSetLumaToneMapLutCfg(
                                    &goldenReg->ldcRegs,
                                    (volatile uint32_t *)goldenReg->ldcLumaLut.LUT, &hObj->lumaLutCfg);
            }

        }
    }

    return status;
}

int32_t Vhwa_m2mLdcConfigRegReadback(const Vhwa_M2mLdcHandleObj *hObj, const Vhwa_M2mLdcInstObj *instObj)
{
    int32_t status = FVID2_SOK;

    CSL_ldc_coreRegs       *ldcRegs  = NULL;
    VhwaVpacLdcSocReadBack *readback = NULL;
    CSL_lseRegs            *lseRegs  = NULL;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
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
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        ldcRegs  = instObj->socInfo.ldcRegs;
        readback = hObj->configRegMemPrms.configRegReadbackPtr;
        lseRegs  = instObj->socInfo.lseRegs;
    }

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == readback) || (NULL == ldcRegs) || (NULL == lseRegs))
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        /* Read LDC core config registers, mask CTRL status bits */
        readback->ldcRegs.CTRL = CSL_REG32_RD(&ldcRegs->CTRL) & (~CSL_LDC_CORE_CTRL_BUSY_MASK);
        readback->ldcRegs.CFG                   = CSL_REG32_RD(&ldcRegs->CFG);
        readback->ldcRegs.MESHTABLE_CFG         = CSL_REG32_RD(&ldcRegs->MESHTABLE_CFG);
        readback->ldcRegs.MESH_FRSZ             = CSL_REG32_RD(&ldcRegs->MESH_FRSZ);
        readback->ldcRegs.COMPUTE_FRSZ          = CSL_REG32_RD(&ldcRegs->COMPUTE_FRSZ);
        readback->ldcRegs.INITXY                = CSL_REG32_RD(&ldcRegs->INITXY);
        readback->ldcRegs.INPUT_FRSZ            = CSL_REG32_RD(&ldcRegs->INPUT_FRSZ);
        readback->ldcRegs.OUT_BLKSZ             = CSL_REG32_RD(&ldcRegs->OUT_BLKSZ);

        readback->ldcRegs.AFF_AB                = CSL_REG32_RD(&ldcRegs->AFF_AB);
        readback->ldcRegs.AFF_CD                = CSL_REG32_RD(&ldcRegs->AFF_CD);
        readback->ldcRegs.AFF_EF                = CSL_REG32_RD(&ldcRegs->AFF_EF);

        /* Always read PWARP_GH to capture hardware reset values */
        readback->ldcRegs.PWARP_GH              = CSL_REG32_RD(&ldcRegs->PWARP_GH);

        /* Always read mesh registers to capture hardware reset values */
        readback->ldcRegs.MESH_BASE_H           = CSL_REG32_RD(&ldcRegs->MESH_BASE_H);
        readback->ldcRegs.MESH_BASE_L           = CSL_REG32_RD(&ldcRegs->MESH_BASE_L);
        readback->ldcRegs.MESH_OFST             = CSL_REG32_RD(&ldcRegs->MESH_OFST);

        readback->ldcRegs.RD_BASE_H             = CSL_REG32_RD(&ldcRegs->RD_BASE_H);
        readback->ldcRegs.RD_BASE_L             = CSL_REG32_RD(&ldcRegs->RD_BASE_L);
        readback->ldcRegs.RD_420C_BASE_H        = CSL_REG32_RD(&ldcRegs->RD_420C_BASE_H);
        readback->ldcRegs.RD_420C_BASE_L        = CSL_REG32_RD(&ldcRegs->RD_420C_BASE_L);
        readback->ldcRegs.RD_OFST               = CSL_REG32_RD(&ldcRegs->RD_OFST);

        #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (UTRUE == hObj->ldcCfg.indChPrms.enable)
        {
            readback->ldcRegs.CH_RD_OFST            = CSL_REG32_RD(&ldcRegs->CH_RD_OFST);
        }
        /* LDRA_JUSTIFY_END */
        #endif

        readback->ldcRegs.VBUSMR_CFG            = CSL_REG32_RD(&ldcRegs->VBUSMR_CFG);
        readback->ldcRegs.COREOUT_CHANCFG       = CSL_REG32_RD(&ldcRegs->COREOUT_CHANCFG);

        /* Always read DUALOUT_CFG to capture hardware reset values */
        readback->ldcRegs.DUALOUT_CFG           = CSL_REG32_RD(&ldcRegs->DUALOUT_CFG);

        #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
        readback->ldcRegs.IBUF_PIX_START        = CSL_REG32_RD(&ldcRegs->IBUF_PIX_START);
        readback->ldcRegs.HYBD_CBUFF_PARAM      = CSL_REG32_RD(&ldcRegs->HYBD_CBUFF_PARAM);
        readback->ldcRegs.HYBD_CBUFF_BA_H       = CSL_REG32_RD(&ldcRegs->HYBD_CBUFF_BA_H);
        readback->ldcRegs.HYBD_CBUFF_BA_L       = CSL_REG32_RD(&ldcRegs->HYBD_CBUFF_BA_L);
        readback->ldcRegs.HYBD_BUFF2_BA_H       = CSL_REG32_RD(&ldcRegs->HYBD_BUFF2_BA_H);
        readback->ldcRegs.HYBD_BUFF2_BA_L       = CSL_REG32_RD(&ldcRegs->HYBD_BUFF2_BA_L);
        readback->ldcRegs.HYBD_CHCBUFF_PARAM    = CSL_REG32_RD(&ldcRegs->HYBD_CHCBUFF_PARAM);
        readback->ldcRegs.HYBD_CHCBUFF_BA_H     = CSL_REG32_RD(&ldcRegs->HYBD_CHCBUFF_BA_H);
        readback->ldcRegs.HYBD_CHCBUFF_BA_L     = CSL_REG32_RD(&ldcRegs->HYBD_CHCBUFF_BA_L);
        readback->ldcRegs.HYBD_CHBUFF2_BA_H     = CSL_REG32_RD(&ldcRegs->HYBD_CHBUFF2_BA_H);
        readback->ldcRegs.HYBD_CHBUFF2_BA_L     = CSL_REG32_RD(&ldcRegs->HYBD_CHBUFF2_BA_L);
        #endif

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
        <justification end> */
        if(UTRUE == hObj->ldcCfg.enableMultiRegions)
        {
            readback->ldcRegs.REGN_W12_SZ = CSL_REG32_RD(&ldcRegs->REGN_W12_SZ);
            readback->ldcRegs.REGN_W3_SZ = CSL_REG32_RD(&ldcRegs->REGN_W3_SZ);
            readback->ldcRegs.REGN_H12_SZ = CSL_REG32_RD(&ldcRegs->REGN_H12_SZ);
            readback->ldcRegs.REGN_H3_SZ = CSL_REG32_RD(&ldcRegs->REGN_H3_SZ);

            for (uint32_t vCnt = 0U; vCnt < LDC_MAX_VERT_REGIONS; vCnt++)
            {
                for (uint32_t hCnt = 0U; hCnt < LDC_MAX_HORZ_REGIONS; hCnt++)
                {
                    uint32_t regionIdx = (vCnt * LDC_MAX_HORZ_REGIONS) + hCnt;
                    readback->ldcRegs.REGION[regionIdx].CTRL = CSL_REG32_RD(&ldcRegs->REGION[regionIdx].CTRL);
                    readback->ldcRegs.REGION[regionIdx].OUT_BLKSZ = CSL_REG32_RD(&ldcRegs->REGION[regionIdx].OUT_BLKSZ);
                }
            }
        }
        /* LDRA_JUSTIFY_END */
        else
        {
            readback->ldcRegs.REGN_W12_SZ = CSL_REG32_RD(&ldcRegs->REGN_W12_SZ) & CSL_LDC_CORE_REGN_W12_SZ_W1_MASK;
            readback->ldcRegs.REGN_H12_SZ = CSL_REG32_RD(&ldcRegs->REGN_H12_SZ) & CSL_LDC_CORE_REGN_H12_SZ_H1_MASK;
        }

        readback->lseRegs.CFG_LSE = CSL_REG32_RD(&lseRegs->CFG_LSE);

        /* Read LSE DST registers only for enabled LDC output channels */
        for (uint32_t i = 0U; i < hObj->lseCfg.numOutCh; i++)
        {
            if (((uint32_t)UTRUE == hObj->lseCfg.outChCfg[i].enable) &&
                ((uint32_t)UFALSE == hObj->lseCfg.outChCfg[i].bypass))
            {
                readback->lseRegs.DST[i].BUF_CFG = CSL_REG32_RD(&lseRegs->DST[i].BUF_CFG) & ~CSL_LSE_DST_BUF_CFG_CH_DISABLED_MASK;
                readback->lseRegs.DST[i].BUF_ATTR0 = CSL_REG32_RD(&lseRegs->DST[i].BUF_ATTR0);
                readback->lseRegs.DST[i].BUF_ATTR1 = CSL_REG32_RD(&lseRegs->DST[i].BUF_ATTR1);
                readback->lseRegs.DST[i].BUF_BA = CSL_REG32_RD(&lseRegs->DST[i].BUF_BA);
            }
        }

        /* Check if any channel uses useMultiBprParams */
        uint32_t anyUseMultiBpr = (uint32_t)UFALSE;
        for (uint32_t i = 0U; i < hObj->lseCfg.numOutCh; i++)
        {
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
            <justification end> */
            if (((uint32_t)UTRUE == hObj->lseCfg.outChCfg[i].enable) &&
                ((uint32_t)UTRUE == hObj->lseCfg.outChCfg[i].useMultiBprParams))
            {
                anyUseMultiBpr = (uint32_t)UTRUE;
                break;
            }
            /* LDRA_JUSTIFY_END */
        }

        if ((uint32_t)UTRUE == anyUseMultiBpr)
        {
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
            <justification end> */
            for (uint32_t cnt = 0u; cnt < (CSL_LSE_BPR_MAX_REGIONS/3U); cnt ++)
            {
                readback->lseRegs.COMMON_CFG_[cnt].ROW = CSL_REG32_RD(&lseRegs->COMMON_CFG_[cnt].ROW);
            }
            /* LDRA_JUSTIFY_END */
        }
    }

    /* Update HTS and static configuration registers */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((readback != NULL) && (instObj != NULL))
    /* LDRA_JUSTIFY_END */
    {
        status = vhwaM2mLdcUpdateConfigRegGroup(readback, instObj, hObj);
    }
    return status;
}

int32_t vhwaM2mLdcUpdateConfigRegGroup(VhwaVpacLdcSocReadBack *RegVal, const Vhwa_M2mLdcInstObj *instObj, const Vhwa_M2mLdcHandleObj *hObj)
{
    int32_t status = FVID2_SOK;
    volatile CSL_htsRegs *htsRegs = NULL;
    uint32_t reg_val;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == RegVal) || (NULL == instObj) || (NULL == hObj))
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        /* Read VPAC enable register for LDC */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != instObj->socInfo.vpacCntlRegs)
        /* LDRA_JUSTIFY_END */
        {
            RegVal->vpacCntlRegs.ENABLE = CSL_REG32_RD(&instObj->socInfo.vpacCntlRegs->ENABLE);
        }

        /* Read Luma LUT from hardware (257 entries) - Using memcpy for performance
         * Use memcpy to copy all entries at once, then apply mask to filter reserved bits
         * Mask reserved bits: bits 31:28 and 15:12 are reserved (type 'r')
         * Configuration bits: LUT_1 (27:16) and LUT_0 (11:0) */

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((NULL != instObj->socInfo.lumaLutBaseAddr) && ((uint32_t)UTRUE == hObj->lumaLutCfg.enable))
        /* LDRA_JUSTIFY_END */
        {
            const uint32_t lutMask = (CSL_LDC_DUALYLUT_LUT_LUT_1_MASK | CSL_LDC_DUALYLUT_LUT_LUT_0_MASK);

            /* Read volatile hardware registers element by element (MISRA compliant) */
            for (uint32_t i = 0U; i < 257U; i++)
            {
                /* Read each LUT entry individually to respect volatile semantics */
                RegVal->ldcLumaLut.LUT[i] = instObj->socInfo.lumaLutBaseAddr[i] & lutMask;
            }
        }


        /* Read Chroma LUT from hardware (257 entries) - Using memcpy for performance
         * Use memcpy to copy all entries at once, then apply mask to filter reserved bits
         * Mask reserved bits: bits 31:28 and 15:12 are reserved (type 'r')
         * Configuration bits: LUT_1 (27:16) and LUT_0 (11:0) */

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((NULL != instObj->socInfo.chromaLutBaseAddr) && ((uint32_t)UTRUE == hObj->chromaLutCfg.enable))
        /* LDRA_JUSTIFY_END */
        {
            const uint32_t lutMask = (CSL_LDC_DUALCLUT_LUT_LUT_1_MASK | CSL_LDC_DUALCLUT_LUT_LUT_0_MASK);

            /* Read volatile hardware registers element by element (MISRA compliant) */
            for (uint32_t i = 0U; i < 257U; i++)
            {
                /* Read each LUT entry individually to respect volatile semantics */
                RegVal->ldcChromaLut.LUT[i] = instObj->socInfo.chromaLutBaseAddr[i] & lutMask;
            }
        }


        /* Read VPAC INTD configuration registers for LDC */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != instObj->socInfo.vpacIntdRegs)
        /* LDRA_JUSTIFY_END */
        {
            volatile CSL_vpac_intd_cfgRegs *intdRegs = instObj->socInfo.vpacIntdRegs;
            uint32_t vhwaIrqNum = instObj->vhwaIrqNum;
            uint32_t counter = 0U;

            /* Read LDC interrupt register (index 1) */
            RegVal->vpacIntd.ENABLE_REG_LEVEL_VPAC_OUT[vhwaIrqNum][counter] =
            CSL_REG32_RD(&intdRegs->ENABLE_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_LDC_INTR_REG]);
            RegVal->vpacIntd.ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][counter] =
            CSL_REG32_RD(&intdRegs->ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_LDC_INTR_REG]);
            counter += 1U;

            /* Read WDTIMER interrupt register (index 3) */
            RegVal->vpacIntd.ENABLE_REG_LEVEL_VPAC_OUT[vhwaIrqNum][counter] =
            CSL_REG32_RD(&intdRegs->ENABLE_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_WDTIMER_INTR_REG]);
            RegVal->vpacIntd.ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][counter] =
            CSL_REG32_RD(&intdRegs->ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_WDTIMER_INTR_REG]);
            counter += 1U;

            /* Read PIPE interrupt register (index 3, same as WDTIMER but kept separate for clarity) */
            RegVal->vpacIntd.ENABLE_REG_LEVEL_VPAC_OUT[vhwaIrqNum][counter] =
                CSL_REG32_RD(&intdRegs->ENABLE_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_PIPE_INTR_REG]);
            RegVal->vpacIntd.ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][counter] =
                CSL_REG32_RD(&intdRegs->ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_PIPE_INTR_REG]);
        }

        /* Read HTS registers */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (instObj->socInfo.htsRegs != NULL)
        /* LDRA_JUSTIFY_END */
        {
            htsRegs = instObj->socInfo.htsRegs;
            const CSL_HtsHwaParams *htsPrms = NULL;
            uint32_t scheduler_base_addr;

            /* Get HTS parameters for LDC0 HWA scheduler */
            htsPrms = GetVpacHtsParams(CSL_HTS_HWA_SCH_LDC0);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL == htsPrms)
            {
                status = FVID2_EFAIL;
            }
            /* LDRA_JUSTIFY_END */
            else
            {
                /* Read HTS PIPELINE_CONTROL */
                RegVal->vpacLdcHts.PIPELINE_CONTROL[0] = CSL_REG32_RD(&htsRegs->PIPELINE_CONTROL[(uint32_t)hObj->htsCfg.pipeline]);

                for (uint32_t schIdx = 0; schIdx < CSL_HTS_NUM_LDC_SCHEDULERS; schIdx++)
                {
                    scheduler_base_addr = (uint32_t)instObj->socInfo.htsRegs + htsPrms->schOffset;
                    {
                        reg_val = CSL_REG32_RD(scheduler_base_addr + HWA_SCH_CONTROL_OFFSET);
                        RegVal->vpacLdcHts.SCHEDULER_CONTROL[schIdx] =
                            (reg_val & CSL_HTS_HWA2_SCHEDULER_CONTROL_PAUSE_MASK) |
                            (reg_val & CSL_HTS_HWA2_SCHEDULER_CONTROL_STRM_EN_MASK) |
                            (reg_val & CSL_HTS_HWA2_SCHEDULER_CONTROL_PIPELINE_NUM_MASK) |
                            (reg_val & CSL_HTS_HWA2_SCHEDULER_CONTROL_SCH_EN_MASK) |
                            (reg_val & CSL_HTS_HWA2_SCHEDULER_CONTROL_DEBUG_RDY_MASK);
                    }

                    #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
                    reg_val = CSL_REG32_RD(scheduler_base_addr + HWA_HOP_OFFSET);
                    RegVal->vpacLdcHts.HOP[schIdx] =
                            (reg_val & CSL_HTS_HWA2_HOP_TDONE_COUNT_EN_MASK) |
                            (reg_val & CSL_HTS_HWA2_HOP_HOP_MASK) |
                            (reg_val & CSL_HTS_HWA2_HOP_HOP_THREAD_COUNT_MASK);
                    #endif
                    /* Only keep enable and mode bits from WD_TIMER register */
                    {
                        reg_val = CSL_REG32_RD(scheduler_base_addr + HWA_WDTIMER_OFFSET);
                        RegVal->vpacLdcHts.WDTIMER[schIdx] =
                            (reg_val & CSL_HTS_HWA2_WDTIMER_WDTIMER_EN_MASK) |
                            (reg_val & CSL_HTS_HWA2_WDTIMER_WDTIMER_MODE_MASK);
                    }

                    RegVal->vpacLdcHts.BW_LIMITER[schIdx] = CSL_REG32_RD(scheduler_base_addr + HWA_BW_LIMITER_OFFSET);
                }

                /* Read HTS consumer control registers (2 HWA consumers) */
                for (uint32_t i = 0; i < CSL_HTS_NUM_LDC_CONSUMERS; i++)
                {
                    uint32_t cons_ctrl_addr = (uint32_t)instObj->socInfo.htsRegs + htsPrms->consOffset[i];
                    RegVal->vpacLdcHts.CONS_CONTROL[i] = CSL_REG32_RD(cons_ctrl_addr + HWA_CONS_CONTROL_OFFSET);
                }

                /* Read HTS producer control registers (8 HWA producers) */
                for (uint32_t i = 0; i < CSL_HTS_NUM_LDC_PRODUCERS; i++)
                {
                    uint32_t prod_ctrl_addr = (uint32_t)instObj->socInfo.htsRegs + htsPrms->prodOffset[i];
                    RegVal->vpacLdcHts.PROD_CONTROL[i] = CSL_REG32_RD(prod_ctrl_addr + HWA_PROD_CONTROL_OFFSET);
                    RegVal->vpacLdcHts.PROD_BUF_CONTROL[i] = CSL_REG32_RD(prod_ctrl_addr + HWA_PROD_BUF_CONTROL_OFFSET);

                    /* Read PROD_COUNT (with masking for count bits) */
                    reg_val = CSL_REG32_RD(prod_ctrl_addr + HWA_PROD_COUNT_OFFSET);
                    RegVal->vpacLdcHts.PROD_COUNT[i] =
                    (reg_val & CSL_HTS_HWA2_PROD0_COUNT_COUNT_POSTLOAD_MASK) |
                    (reg_val & CSL_HTS_HWA2_PROD0_COUNT_COUNT_PRELOAD_MASK);

                    RegVal->vpacLdcHts.PA_CONTROL[i] = CSL_REG32_RD(prod_ctrl_addr + HWA_PROD_PA_CONTROL_OFFSET);
                    /* Read PA_PRODCOUNT (with masking for PA_COUNT_DEC bits only) */
                    reg_val = CSL_REG32_RD(prod_ctrl_addr + HWA_PROD_PA_PROD_COUNT_OFFSET);
                    RegVal->vpacLdcHts.PA_PRODCOUNT[i] = reg_val & CSL_HTS_HWA2_PA0_PRODCOUNT_PA_COUNT_DEC_MASK;
                }

                /* Readback for DMA producers */
            for (uint32_t i = 0; i < CSL_HTS_NUM_LDC_PRODUCERS_DMA; i++)
            {
                uint32_t prod_dma_ctrl_addr = (uint32_t)instObj->socInfo.htsRegs + htsPrms->dmaProdOffset[i];

                /* PROD_DMA_SCHEDULER_CONTROL */
                reg_val = CSL_REG32_RD(prod_dma_ctrl_addr + HWA_DMA_PROD_SCH_CONTROL_OFFSET);
                RegVal->vpacLdcHts.PROD_DMA_SCHEDULER_CONTROL[i] =
                    (reg_val & CSL_HTS_DMA8_SCHEDULER_CONTROL_DMA_CHANNEL_NO_MASK) |
                    (reg_val & CSL_HTS_DMA8_SCHEDULER_CONTROL_DEBUG_RDY_MASK) |
                    (reg_val & CSL_HTS_DMA8_SCHEDULER_CONTROL_PAUSE_MASK) |
                    (reg_val & CSL_HTS_DMA8_SCHEDULER_CONTROL_STRM_EN_MASK) |
                    (reg_val & CSL_HTS_DMA8_SCHEDULER_CONTROL_PIPELINE_NUM_MASK) |
                    (reg_val & CSL_HTS_DMA8_SCHEDULER_CONTROL_SCH_EN_MASK);

                #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
                /* PROD_DMA_HOP */
                reg_val = CSL_REG32_RD(prod_dma_ctrl_addr + HWA_DMA_PROD_HOP_OFFSET);
                RegVal->vpacLdcHts.PROD_DMA_HOP[i] =
                    (reg_val & CSL_HTS_DMA8_HOP_HOP_MASK) |
                    (reg_val & CSL_HTS_DMA8_HOP_HOP_THREAD_COUNT_MASK) |
                    (reg_val & CSL_HTS_DMA8_HOP_TDONE_COUNT_EN_MASK);
                #endif

                /* PROD_CONTROL_DMA */
                RegVal->vpacLdcHts.PROD_CONTROL_DMA[i] = CSL_REG32_RD(prod_dma_ctrl_addr + HWA_DMA_PROD_CONTROL_OFFSET);

                /* PROD_BUF_CONTROL_DMA */
                RegVal->vpacLdcHts.PROD_BUF_CONTROL_DMA[i] = CSL_REG32_RD(prod_dma_ctrl_addr + HWA_DMA_PROD_BUF_CONTROL_OFFSET);

                /* Only keep postload and preload bits from PROD_DMA_COUNT register */
                reg_val = CSL_REG32_RD(prod_dma_ctrl_addr + HWA_DMA_PROD_COUNT_OFFSET);
                RegVal->vpacLdcHts.PROD_COUNT_DMA[i] =
                    (reg_val & CSL_HTS_DMA8_PROD0_COUNT_COUNT_POSTLOAD_MASK) |
                    (reg_val & CSL_HTS_DMA8_PROD0_COUNT_COUNT_PRELOAD_MASK);

                RegVal->vpacLdcHts.PA_CONTROL_DMA[i] = CSL_REG32_RD(prod_dma_ctrl_addr + HWA_DMA_PROD_PA_CONTROL_OFFSET);

                reg_val = CSL_REG32_RD(prod_dma_ctrl_addr + HWA_DMA_PROD_PA_PROD_COUNT_OFFSET);
                RegVal->vpacLdcHts.PA_PRODCOUNT_DMA[i] = (reg_val & CSL_HTS_DMA8_PA0_PRODCOUNT_PA_COUNT_DEC_MASK);
            }
            /* Read HTS DMA consumer registers (8 DMA consumers) */
            for (uint32_t i = 0; i < CSL_HTS_NUM_LDC_CONSUMERS_DMA; i++)
            {
                uint32_t cons_dma_ctrl_addr = (uint32_t)instObj->socInfo.htsRegs + htsPrms->dmaConsOffset[i];

                /* CONSUMER_DMA_SCHEDULER_CONTROL */
                reg_val = CSL_REG32_RD(cons_dma_ctrl_addr + HWA_DMA_CONS_SCH_CONTROL_OFFSET);
                RegVal->vpacLdcHts.CONS_DMA_SCHEDULER_CONTROL[i] =
                    (reg_val & CSL_HTS_DMA272_SCHEDULER_CONTROL_DMA_CHANNEL_NO_MASK) |
                    (reg_val & CSL_HTS_DMA272_SCHEDULER_CONTROL_DEBUG_RDY_MASK) |
                    (reg_val & CSL_HTS_DMA272_SCHEDULER_CONTROL_PAUSE_MASK) |
                    (reg_val & CSL_HTS_DMA272_SCHEDULER_CONTROL_STRM_EN_MASK) |
                    (reg_val & CSL_HTS_DMA272_SCHEDULER_CONTROL_PIPELINE_NUM_MASK) |
                    (reg_val & CSL_HTS_DMA272_SCHEDULER_CONTROL_SCH_EN_MASK);

                /* CONSUMER_CONTROL_DMA */
                RegVal->vpacLdcHts.CONS_CONTROL_DMA[i] = CSL_REG32_RD(cons_dma_ctrl_addr + HWA_DMA_CONS_CONTROL_OFFSET);
            }
            }
        }
    }

    return status;
}

int32_t Vhwa_m2mLdcGetSl2Info(Vhwa_M2mLdcSl2Info *sl2Info)
{
    int32_t retVal = FVID2_SOK;
    Vhwa_M2mLdcInstObj *instObj = NULL;
    uint32_t cnt;

    instObj = &gM2mLdcInstObj;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
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
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
    <justification end> */
    else if ((uint32_t)UFALSE == instObj->isSl2AllocDone)
    {
        /* SL2 memory not allocated yet */
        GT_0trace(VhwaLdcTrace, GT_ERR,
            "SL2 Memory is not allocated. Call Vhwa_m2mLdcAllocSl2() first.\n");
        retVal = FVID2_EFAIL;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        /* LDC stores SL2 info per output in arrays.
         * Return the first address and sum of all output sizes. */
        sl2Info->sl2Addr = instObj->sl2Addr[0];
        sl2Info->sl2Size = 0;        
        for (cnt = 0U; cnt < LDC_MAX_OUTPUT; cnt++)
        {
            sl2Info->sl2Size += instObj->sl2Size[cnt];
        }
    }

    return retVal;
}

int32_t Vhwa_M2mLdcConfigRegMemCompare(const Vhwa_M2mLdcHandleObj *hObj)
{
    int32_t status = FVID2_SOK;
    Vhwa_M2mLdcConfigRegMemParams configRegMemPtrs;
    uint32_t configSize;
    void *goldenPtr, *readbackPtr;

    if (NULL == hObj)
    {
        status = FVID2_EBADARGS;
    }
    else if ((NULL == hObj->configRegMemPrms.cbFxn) ||
             (NULL == hObj->configRegMemPrms.configGoldenRegPtr) ||
             (NULL == hObj->configRegMemPrms.configRegReadbackPtr))
    {
        status = FVID2_EBADARGS;
    }
    else
    {
        /* Part 1: Compare configuration registers (before LUTs) */
        configSize = sizeof(VhwaVpacLdcSocReadBack) - sizeof(CSL_ldc_dualylutRegs) - sizeof(CSL_ldc_dualclutRegs);
        configRegMemPtrs.golden_ptr = hObj->configRegMemPrms.configGoldenRegPtr;
        configRegMemPtrs.readback_ptr = hObj->configRegMemPrms.configRegReadbackPtr;
        configRegMemPtrs.readback_mem_size = configSize;
        configRegMemPtrs.appData = (void *)hObj->configRegMemPrms.appData;

        status = hObj->configRegMemPrms.cbFxn((Fvid2_Handle)(uintptr_t)hObj, &configRegMemPtrs);

        /* Part 2: Compare Luma LUT (conditional on enable) */
        if ((FVID2_SOK == status) && ((uint32_t)UTRUE == hObj->lumaLutCfg.enable))
        {
            goldenPtr = (void *)&hObj->configRegMemPrms.configGoldenRegPtr->ldcLumaLut;
            readbackPtr = (void *)&hObj->configRegMemPrms.configRegReadbackPtr->ldcLumaLut;

            configRegMemPtrs.golden_ptr = goldenPtr;
            configRegMemPtrs.readback_ptr = readbackPtr;
            configRegMemPtrs.readback_mem_size = sizeof(CSL_ldc_dualylutRegs);
            configRegMemPtrs.appData = (void *)hObj->configRegMemPrms.appData;

            status = hObj->configRegMemPrms.cbFxn((Fvid2_Handle)(uintptr_t)hObj, &configRegMemPtrs);
        }

        /* Part 3: Compare Chroma LUT (conditional on enable) */
        if ((FVID2_SOK == status) && ((uint32_t)UTRUE == hObj->chromaLutCfg.enable))
        {
            goldenPtr = (void *)&hObj->configRegMemPrms.configGoldenRegPtr->ldcChromaLut;
            readbackPtr = (void *)&hObj->configRegMemPrms.configRegReadbackPtr->ldcChromaLut;

            configRegMemPtrs.golden_ptr = goldenPtr;
            configRegMemPtrs.readback_ptr = readbackPtr;
            configRegMemPtrs.readback_mem_size = sizeof(CSL_ldc_dualclutRegs);
            configRegMemPtrs.appData = (void *)hObj->configRegMemPrms.appData;

            status = hObj->configRegMemPrms.cbFxn((Fvid2_Handle)(uintptr_t)hObj, &configRegMemPtrs);
        }
    }

    return status;
}