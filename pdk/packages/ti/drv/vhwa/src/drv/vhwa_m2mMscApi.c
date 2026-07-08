/**
 *   Copyright (c) Texas Instruments Incorporated 2018
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
 *  \file vhwa_m2mMscApi.c
 *
 *  \brief API Implementation
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

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */
/**
 * \brief   Function to initialize the internal driver paraeters
 *
 * \param   pInitPrms         Init parametes required for initialize
 *
 * \return  FVID2_SOK in case of sucess, Error otherwise
 *
 **/
static int32_t Vhwa_mscInit(const Vhwa_M2mMscInitParams *pInitPrms);

/**
 * \brief   Function to de-initialize the internal driver paraeters
 *
 * \return  FVID2_SOK in case of sucess, Error otherwise
 *
 **/
static int32_t Vhwa_mscDeInit(void);

/**
 * \brief   Create Queues, required for storing pending requests
 *
 * \param   instObj             Instance object.
 *
 * \return  FVID2_SOK in case of sucess, Error otherwise
 *
 **/
static int32_t Vhwa_mscCreateQueue(Vhwa_M2mMscInstObj *instObj);

/**
 * \brief   Delete Queues
 *
 * \param   instObj             Instance object.
 *
 * \return  FVID2_SOK in case of sucess, Error otherwise
 *
 **/
static int32_t Vhwa_mscDeleteQueue(Vhwa_M2mMscInstObj *instObj);

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
static Vhwa_M2mMscHandleObj *Vhwa_mscAllocHdlObj(const Vhwa_M2mMscInstObj *instObj,
                                                 uint32_t instId);

/**
 * \brief   Local Function to free Handle Object
 *          No protection inside the function, Caller should protect
 *          the function call
 *
 * \param   instObj      Instance object.
 *
 */
static void Vhwa_mscFreeHandleObj(Vhwa_M2mMscHandleObj *hObj);

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
static void Vhwa_mscInitHandleObj(const Vhwa_M2mMscInstObj *instObj,
                                  Vhwa_M2mMscHandleObj *hObj);

/**
 * \brief   Local function to set the MSC filter coefficients
 *
 *
 * \param   comObj             Common driver object
 * \param   pCoeffCfg          MSC coeff parameters
 *
 * \return  FVID2_SOK in case of sucess, Error otherwise
 *
 **/
static int32_t Vhwa_mscSetFilterCoeff(Vhwa_M2mMscCommonObj *comObj,
                                      const Msc_Coeff *pCoeffCfg);

/**
 * \brief   Implementation of SET_PARAMS ioctl.
 *          It uses #Vhwa_mscCheckMscCfg to validate the config.
 *          If it is valid, copies the config into handle object
 *          If it is invalid, it reverts LSE/HTS config to known valid config
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   comObj              Common driver Object
 * \param   mscPrms             MSC configuration to be set
 *
 * \return  FVID2_SOK in case of sucess, Error otherwise
 *
 **/
static int32_t Vhwa_mscSetParams(const Vhwa_M2mMscInstObj *instObj,
                                 Vhwa_M2mMscHandleObj *hObj,
                                 const Vhwa_M2mMscCommonObj *comObj,
                                 Vhwa_M2mMscParams *mscPrms);

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
static int32_t vhwa_m2mMscSetEeParams(const Vhwa_M2mMscInstObj *instObj,
    Vhwa_M2mMscHandleObj *hObj, const Msc_ErrEventParams *eePrms);
/**
 * \brief   Implementation of REGISTER_WDTIMER_ERR_CB ioctl.
 *
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   wdTimerEePrms       Watchdog timer Error Parameters
 *
 * \return  FVID2_SOK in case of sucess, Error otherwise
 *
 **/
static int32_t vhwa_m2mMscSetWdTimerEeParams(const Vhwa_M2mMscInstObj *instObj,
    Vhwa_M2mMscHandleObj *hObj, const Msc_WdTimerErrEventParams *wdTimerEePrms);

/**
 * \brief   Configure error interrupts in hardware based on handle settings.
 *
 * \param   vhwaIrqNum         VHWA IRQ number
 * \param   intdRegs           Pointer to VPAC INTD registers
 * \param   hObj               Handle Object
 *
 * \return  FVID2_SOK in case of success, Error otherwise
 *
 **/
static int32_t Vhwa_m2mMscSetIntrInHW(uint32_t vhwaIrqNum,
                                volatile CSL_vpac_intd_cfgRegs  *intdRegs,
                                const Vhwa_M2mMscHandleObj *hObj);

/**
 * \brief   Local function to initilize the MSc with default configuration
 *
 * \param   mscPrms            MSC parameters to be initialized
 *
 **/
static void Vhwa_mscInitMscCfg(Vhwa_M2mMscParams *mscPrms);

/**
 * \brief   Based on the given MSC config, it initializes HTS configuration.
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   mscPrms             MSC Configuration
 *
 **/
static void Vhwa_mscSetHtsCfg(const Vhwa_M2mMscInstObj *instObj,
                              Vhwa_M2mMscHandleObj *hObj,
                              const Vhwa_M2mMscParams *mscPrms);

/**
 * \brief   Based on the given MSC config, it initializes LSE configuration.
 *
 * \param   instObj        Instance Object
 * \param   hObj           Handle Object
 * \param   mscPrms        MSC Configuration
 *
 **/
static void Vhwa_mscSetLseCfg(Vhwa_M2mMscHandleObj *hObj,
                              const Vhwa_M2mMscParams *mscPrms);

/**
 * \brief   Local function to check the MSC configuration
 *
 * \param   instObj             Instance Object
 * \param   hObj                Handle Object
 * \param   mscPrms             MSC Configuration
 *
 * \return  FVID2_SOK in case of correct configuration, Error otherwise
 *
 **/
static int32_t Vhwa_mscCheckMscCfg(Vhwa_M2mMscHandleObj *hObj,
                                   Vhwa_M2mMscParams *mscPrms);

/**
 * \brief   Local function to set CSL paramenter for Luma and Chroma based on
 *          MSC configuration
 *
 * \param   hObj                Handle Object
 * \param   mscPrms             MSC Configuration
 *
 **/
static void Vhwa_mscCalPrms(Vhwa_M2mMscHandleObj *hObj,
                                 const Vhwa_M2mMscParams *mscPrms);

/**
 * \brief   Local function to set HTS Bandwidth limiter
 *
 * \param   hObj                Handle Object
 * \param   bwLimit             Bandwidth limiter Configuration
 *
 **/
static void Vhwa_mscSetHtsLimit(Vhwa_M2mMscHandleObj *hObj,
                               const Vhwa_HtsLimiter *htsLimit);

/**
 * \brief   Funtion to Update the Parameters with Flexconnect changes
 *
 * \param   hObj                Handle Object
 * \param   sl2FcPrms             Bandwidth limiter Configuration
 *
 **/
static void Vhwa_m2mMscUpdateFcConnPrms(Vhwa_M2mMscHandleObj *hObj,
                                const Vhwa_M2mMscFcConPrms *sl2FcPrms);

static void Vhwa_m2mMscUpdateFcPrms(Vhwa_M2mMscHandleObj *hObj,
                                    const Vhwa_M2mMscFcUpdatePrms *fcPrms);

static void Vhwa_m2mMscFcGetPrms(Vhwa_M2mMscHandleObj *hObj,
                                Vhwa_M2mMscFcGetPrms *fcPrms);
/**
 * \brief   Local function to check MSC Create params like Driver Id and Instance Id.
 *
 * \param   drvId               Driver ID
 * \param   drvInstId           Instance ID
 *
 *  \return FVID2_SOK if successful, else suitable error code
 **/
static int32_t Vhwa_m2mMscCheckCreatePrms(uint32_t drvId, uint32_t drvInstId);

/* Implementation of FVID2 APIs */
/**
 * \brief   FVID2 Create Function.
 *
 * \param   instObj             Instance object.
 *
 * \return  FVID2 Driver Handle.
 *
 **/
static Fdrv_Handle Vhwa_m2mMscCreate(uint32_t drvId, uint32_t vpacDrvInstId,
                                     Ptr createPrms, Ptr createStatusArgs,
                                     const Fvid2_DrvCbParams *cbPrms);

/**
 * \brief   FVID2 Delete Function.
 *
 * \param   handle              FVID2 driver handle.
 *
 * \return  FVID2_SOK on success, else FVID2 error code
 *
 **/
static int32_t Vhwa_m2mMscDelete(Fdrv_Handle handle, Ptr deleteArgs);

/**
 * \brief   FVID2 Control Function.
 *
 * \param   handle              FVID2 driver handle.
 *
 * \return  FVID2_SOK on success, else FVID2 error code
 *
 **/
static int32_t Vhwa_m2mMscControl(Fdrv_Handle handle, uint32_t cmd,
                                  Ptr cmdArgs, Ptr cmdStatusArgs);

/**
 * \brief   FVID2 Process Request function.
 *
 * \param   handle              FVID2 driver handle.
 *
 * \return  FVID2_SOK on success, else FVID2 error code
 *
 **/
static int32_t Vhwa_m2mMscProcessReq(Fdrv_Handle handle,
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
static int32_t Vhwa_m2mMscGetProcessReq(Fdrv_Handle handle,
                                        Fvid2_FrameList *inFrmList,
                                        Fvid2_FrameList *outFrmList,
                                        uint32_t timeout);


static void Vhwa_mscUpdateYuv422iCslPrms(Vhwa_M2mMscHandleObj *hObj,
                                         CSL_MscConfig *cslMscCfg);

#if !defined(VHWA_VPAC_IP_REV_VPAC3L)
                                         /**
 * \brief   Reconfigure MSC Reg registers as needed for the current handle/queue.
 *
 * \param   instObj   Instance Object.
 * \param   hObj      Handle Object.
 * \param   qObj      Queue Object.
 *
 * \return  FVID2_SOK on success, else FVID2 error code.
 */
static int32_t Vhwa_m2mMscReconfigReinitReg(const Vhwa_M2mMscInstObj *instObj,
                                            Vhwa_M2mMscCommonObj *comObj,
                                            const Vhwa_M2mMscQueueObj *qObj);

/**
 * @brief Validates the MSC status registers against expected values.
 *
 * This function compares the current MSC status register values with the expected values
 * provided in the 'expected' parameter. It uses the SoC-specific information from 'socInfo'
 * to perform the validation according to the hardware configuration.
 *
 * @param expected Pointer to a MscStatusRegisterGroup structure containing the expected
 * status register values.
 * @param socInfo  Pointer to a SocInfo structure containing SoC-specific information
 * required for validation.
 *
 * @return Returns 0 (FVID2_SOK) if the status registers match the expected values,
 * or a negative error code if there is a mismatch or an error occurs.
 */
int32_t Vhwa_m2mMscStatusRegValidate(Vhwa_M2mMscHandleObj *hObj, const Msc_SocInfo *socInfo);

/**
 * \brief   Updates the MscStatusRegisterGroup structure in the handle object with expected status register values.
 *
 * \param   hObj    Pointer to the Vhwa_M2mMscHandleObj whose statusRegs should be updated.
 */
static int32_t vhwaM2mMscUpdateStatusRegGroup(Vhwa_M2mMscHandleObj *hObj);

/**
 * @brief Sets the default golden register memory values for the MSC (Multi-Scaler) module.
 *
 * This function initializes the register memory values to their default "golden" state
 * for the specified MSC handle and instance object. It is typically used to ensure
 * that the hardware registers are set to known, validated values before operation.
 *
 * @param hObj Pointer to the MSC handle object.
 * @param instObj Pointer to the MSC instance object.
 *
 * @return Returns 0 on success, or a negative error code on failure.
 */
int32_t vhwaM2mMscSetDefaultGoldenRegMemValues(const Vhwa_M2mMscHandleObj *hObj, const Vhwa_M2mMscCommonObj *comObj);
#endif

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
Vhwa_M2mMscHandleObj   gM2mMscHandleObj[VHWA_M2M_MSC_MAX_INST]
                                       [VHWA_M2M_MSC_MAX_HANDLES];
Vhwa_M2mMscInstObj     gM2mMscInstObj[VHWA_M2M_MSC_MAX_INST];
Vhwa_M2mMscCommonObj   gM2mMscCommonObj;

Fvid2_DrvOps gM2mMscFvid2DrvOps = {
    FVID2_VHWA_M2M_MSC_DRV_ID,
    /**< Unique driver Id. */
    Vhwa_m2mMscCreate,
    /**< FVID2 create function pointer. */
    Vhwa_m2mMscDelete,
    /**< FVID2 delete function pointer. */
    Vhwa_m2mMscControl,
    /**< FVID2 control function pointer. */
    NULL, NULL,
    /**< FVID2 queue function pointer. */
    Vhwa_m2mMscProcessReq,
    /**< FVID2 process request function pointer. */
    Vhwa_m2mMscGetProcessReq,
    /**< FVID2 get processed request function pointer. */
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t Vhwa_m2mMscInit(const Vhwa_M2mMscInitParams *pInitPrms)
{
    int32_t retVal = FVID2_SOK;

    /* Check for Null */
   /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(NULL == pInitPrms)
    {
        GT_0trace(VhwaMscTrace, GT_ERR, "Bad Arguments\n");
        retVal = FVID2_EBADARGS;
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
    if(FVID2_SOK == retVal)
    /* LDRA_JUSTIFY_END */
    {
        /* Initialize all the global variables to 0 */
        Fvid2Utils_memset(&gM2mMscCommonObj, 0U, sizeof (gM2mMscCommonObj));
        Fvid2Utils_memset(gM2mMscInstObj, 0U, sizeof(gM2mMscInstObj));
        Fvid2Utils_memset(gM2mMscHandleObj, 0U, sizeof (gM2mMscHandleObj));

        /* Initialize the internal objects */
        retVal = Vhwa_mscInit(pInitPrms);
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
    if(retVal == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        retVal = Fvid2_registerDriver(&gM2mMscFvid2DrvOps);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps.
        Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != retVal)
        {
            GT_0trace(VhwaMscTrace, GT_ERR,
                      "Registering to FVID2 driver manager failed\r\n");

            /* Uninitialize the internal objects */
            (void)Vhwa_mscDeInit();
        }
        /* LDRA_JUSTIFY_END */
    }

    return (retVal);
}

int32_t Vhwa_m2mMscDeInit(void)
{
    int32_t retVal = FVID2_SOK;

    /* Unregister the Driver manager */
    retVal = Fvid2_unRegisterDriver(&gM2mMscFvid2DrvOps);
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK != retVal)
    {
        GT_0trace(VhwaMscTrace, GT_ERR,
                  "Unregistering from FVID2 driver manager failed!!\r\n");
    }
    /* LDRA_JUSTIFY_END */


    retVal += Vhwa_mscDeInit();

    return retVal;
}

static Fdrv_Handle Vhwa_m2mMscCreate(uint32_t drvId, uint32_t vpacDrvInstId,
    Ptr createPrms, Ptr createStatusArgs, const Fvid2_DrvCbParams *cbPrms)
{
    int32_t                 retVal = FVID2_SOK;
    Vhwa_M2mMscInstObj     *instObj = NULL;
    Vhwa_M2mMscHandleObj   *hObj = NULL;
    Vhwa_M2mMscCreatePrms  *mscCreatePrms = NULL;
    Fdrv_Handle             handle;
    uint32_t                drvInstId = vpacDrvInstId;

    /* Check for params and Null pointer */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == createPrms) || (NULL == cbPrms))
    {
        GT_0trace(VhwaMscTrace, GT_ERR, "Bad Arguments\n");
        retVal = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        retVal = Vhwa_m2mMscCheckCreatePrms(drvId, vpacDrvInstId);
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == retVal)
    /* LDRA_JUSTIFY_END */

    {
        /* Create is allowed only after init is done */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (UFALSE == gM2mMscCommonObj.initDone)
        {
            GT_0trace(VhwaMscTrace, GT_ERR,
                "Error: Driver Initialization Pending\n");
            retVal  = FVID2_EINVALID_PARAMS;
        }
        /* LDRA_JUSTIFY_END */

    }
#if defined VHWA_M2M_VPAC_INSTANCE
#if (VHWA_M2M_VPAC_INSTANCE == 1)
    /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == retVal)
    /* LDRA_JUSTIFY_END */

    {
        /* Translate instance id to be compatible with existing driver */
        if (VHWA_M2M_VPAC_1_MSC_DRV_INST_ID_0 == vpacDrvInstId)
        {
            drvInstId = VPAC_MSC_INST_ID_0;
        }
        else
        {
            drvInstId = VPAC_MSC_INST_ID_1;
        }
    }
#endif
#endif
     /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(FVID2_SOK == retVal)
    /* LDRA_JUSTIFY_END */

    {
        instObj = &gM2mMscInstObj[drvInstId];
    }
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
        /* Lock instance semaphore */
        (void)VhwaAl_SemaphoreP_pend(instObj->lockSem, SYSTEMP_WAIT_FOREVER);
    }
     /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == retVal)
    /* LDRA_JUSTIFY_END */

    {
        mscCreatePrms = (Vhwa_M2mMscCreatePrms *)createPrms;
          /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> Rationale:
        The negative test framework and test apps cannot reach this portion.
        These parameters cannot be configured from test app or kernel directly and can only be overwritten by memory corruption. This is fail safe mechanism and cannot be removed
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (instObj->openCnt >= VHWA_M2M_MSC_MAX_HANDLES)
        {
            GT_1trace(VhwaMscTrace, GT_ERR,
                      "Only %d handles supported per instance!\r\n",
                      VHWA_M2M_MSC_MAX_HANDLES);
            retVal = FVID2_EALLOC;
        }
        /* LDRA_JUSTIFY_END */

    }
     /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == retVal)
    /* LDRA_JUSTIFY_END */

    {
        /* Allocate Handle Object */
        hObj = Vhwa_mscAllocHdlObj(instObj, drvInstId);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
The     test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL == hObj)
        {
            retVal = FVID2_EALLOC;
        }
        /* LDRA_JUSTIFY_END */
        else

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
                if ((uint32_t)UFALSE == gM2mMscCommonObj.isSl2AllocDone)
                {
                    /* Initialize SL2 parameters with the defaul values */
                    Vhwa_M2mMscSl2AllocPrmsInit(&gM2mMscCommonObj.sl2AllocPrms);

                    /* Allocate SL2 Parameters */
                    retVal = Vhwa_m2mMscAllocSl2(&gM2mMscCommonObj.sl2AllocPrms);
                }
                /* LDRA_JUSTIFY_END */


                /* On the first open, start all UTC channels.
                 * Even if all UTC channels are enabled, only channels enabled
                 * in HTS will be used for the transfer */
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This branch statement checks for errors accumulated in previous steps.
                Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (FVID2_SOK == retVal)
                /* LDRA_JUSTIFY_END */

                {
                    /* Start UDMA Channels on the first handle Create */
                    retVal = Vhwa_m2mMscStartCh(instObj, &gM2mMscCommonObj);
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
                if (FVID2_SOK == retVal)
                /* LDRA_JUSTIFY_END */

                {
                    Vhwa_enableHtsIntr(gM2mMscCommonObj.socInfo.vpacIntdRegs,
                        gM2mMscCommonObj.mscInitPrms.irqInfo[drvInstId].vhwaIrqNum,
                        instObj->pipeline);
                }
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
    if (FVID2_SOK == retVal)
    /* LDRA_JUSTIFY_END */
    {
        /* Allocate UDMA memory for this handle */
        retVal = Vhwa_m2mMscAllocUdmaMem(hObj);
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
    if (FVID2_SOK == retVal)
    /* LDRA_JUSTIFY_END */
    {
        retVal = Fvid2Utils_constructQ(&hObj->outLlObj);
        GT_assert(VhwaMscTrace, (FVID2_SOK == retVal));

        hObj->outQ = &hObj->outLlObj;
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
    if (FVID2_SOK == retVal)
    /* LDRA_JUSTIFY_END */
    {
        Fvid2Utils_memcpy(&hObj->createPrms, mscCreatePrms,
            sizeof(Vhwa_M2mMscCreatePrms));

        /* Copy Callback Parameters */
        Fvid2Utils_memcpy(&hObj->fdmCbPrms, cbPrms, sizeof (hObj->fdmCbPrms));
        Vhwa_mscInitHandleObj(instObj, hObj);

        instObj->openCnt++;

        /* Setting it to UTRUE so that next request will update in HW. */
        hObj->isCfgUpdated = 1u;

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

        if(NULL != hObj)
        {
            /* Free up allocated resources in case of error */
            Vhwa_mscFreeHandleObj(hObj);
        }

        handle = NULL;
    }
     /* LDRA_JUSTIFY_END */

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != instObj)
    /* LDRA_JUSTIFY_END */

    {
        /* Release instance semaphore */
        (void)VhwaAl_SemaphoreP_post(instObj->lockSem);
    }

    return (handle);
}

static int32_t Vhwa_m2mMscDelete(Fdrv_Handle handle, Ptr deleteArgs)
{
    int32_t                 retVal = FVID2_SOK;
    Vhwa_M2mMscInstObj     *instObj = NULL;
    Vhwa_M2mMscHandleObj   *hObj = NULL;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated in the imaging layer before reaching vhwa.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL == handle)
    {
        GT_0trace(VhwaMscTrace, GT_ERR, "NULL pointer\n");
        retVal = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        hObj = (Vhwa_M2mMscHandleObj *)handle;
        instObj = &gM2mMscInstObj[hObj->instId];
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != instObj)
    /* LDRA_JUSTIFY_END */

    {
        /* Take the instance semaphore */
        (void)VhwaAl_SemaphoreP_pend(instObj->lockSem, SYSTEMP_WAIT_FOREVER);

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
            retVal = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */

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
    if (FVID2_SOK == retVal)
    /* LDRA_JUSTIFY_END */
    {
        /* Check if still some request are pending with driver */
         /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps.
        Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (0U != hObj->numPendReq)
        {
            GT_1trace(
                VhwaMscTrace, GT_ERR,
                "Still %d request pending. Dequeue all before closing!!\r\n",
                hObj->numPendReq);

            retVal = FVID2_EAGAIN;
        }
        /* LDRA_JUSTIFY_END */
        else

        {
            /* Delete the free Q */
            Fvid2Utils_destructQ(hObj->outQ);
        }
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == retVal)
    /* LDRA_JUSTIFY_END */

    {
        /* Free allocated handle object */
        Vhwa_mscFreeHandleObj(hObj);

        /* Decrement number of handle open count */
        instObj->openCnt--;

        if (0U == instObj->openCnt)
        {
            /* Disable HTS Interrupt */
            Vhwa_disableHtsIntr(gM2mMscCommonObj.socInfo.vpacIntdRegs,
                gM2mMscCommonObj.mscInitPrms.irqInfo[hObj->instId].vhwaIrqNum,
                instObj->pipeline);

            instObj->lastHndlObj = NULL;
        }
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != instObj)
    /* LDRA_JUSTIFY_END */

    {
        /* release the instance semaphore */
        (void)VhwaAl_SemaphoreP_post(instObj->lockSem);
    }

    return (retVal);
}

static int32_t Vhwa_m2mMscControl(Fdrv_Handle handle, uint32_t cmd, Ptr cmdArgs,
    Ptr cmdStatusArgs)
{
    int32_t                 retVal = FVID2_SOK;
    uint32_t                cnt;
    Vhwa_M2mMscInstObj     *instObj = NULL;
    Vhwa_M2mMscHandleObj   *hObj = NULL;
    Msc_Coeff              *pCoeffCfg = NULL;
    Vhwa_M2mMscParams      *mscPrms = NULL;
    Msc_ErrEventParams     *eePrms = NULL;
    Vhwa_M2mMscCommonObj   *comObj = NULL;
    Vhwa_HtsLimiter        *htsLimit = NULL;
    Msc_WdTimerErrEventParams *wdTimerEePrms = NULL;
#if !defined(VHWA_VPAC_IP_REV_VPAC3L)
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
        GT_0trace(VhwaMscTrace, GT_ERR, "Handle NULL pointer\n");
        retVal = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        hObj = (Vhwa_M2mMscHandleObj *)handle;
        instObj = &gM2mMscInstObj[hObj->instId];
        comObj = &gM2mMscCommonObj;
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
    if (FVID2_SOK == retVal)
    /* LDRA_JUSTIFY_END */
    {
        /* Take the instance semaphore */
        (void)VhwaAl_SemaphoreP_pend(instObj->lockSem, SYSTEMP_WAIT_FOREVER);
         /* LDRA_JUSTIFY_START
        <metric start>statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameter is expected to be pre-validated in the imaging layer before reaching vhwa.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        switch (cmd)
        {
        /* LDRA_JUSTIFY_END */
            /* Set Filter Coefficients */
            case VHWA_M2M_IOCTL_MSC_SET_COEFF:
            {
                /* LDRA_JUSTIFY_START
                <metric start>branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, then our code base is expected to prevent undefined behaviour due to dereferencing a NULL pointer.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != cmdArgs)
                /* LDRA_JUSTIFY_END */

                {
                    pCoeffCfg = (Msc_Coeff *)cmdArgs;
                    retVal = Vhwa_mscSetFilterCoeff(comObj, pCoeffCfg);

                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion. The parameter is expected to be pre-validated in the imaging layer before reaching vhwa.
                    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    /* Update golden memory if config register validation is enabled */
                    if ((FVID2_SOK == retVal) && (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate))
                    /* LDRA_JUSTIFY_END */
                    {
                        VhwaVpacMscSocReadBack *goldenRegVal = hObj->configRegMemPrms.configGoldenRegPtr;
                        /* LDRA_JUSTIFY_START
                        <metric start> branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if (NULL != goldenRegVal)
                        /* LDRA_JUSTIFY_END */
                        {
                            retVal = CSL_mscSetCoeff(&goldenRegVal->mscRegs, pCoeffCfg);
                        }
                    }
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaMscTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_MSC_SET_COEFF !!\n");
                    retVal = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            /* Set MSC Parameters */
            case VHWA_M2M_IOCTL_MSC_SET_PARAMS:
            {
                /* LDRA_JUSTIFY_START
                <metric start>branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, then our code base is expected to prevent undefined behaviour due to dereferencing a NULL pointer.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != cmdArgs)
                /* LDRA_JUSTIFY_END */
                {
                    mscPrms = (Vhwa_M2mMscParams *)cmdArgs;
                    retVal = Vhwa_mscSetParams(instObj, hObj, comObj, mscPrms);
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaMscTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_MSC_SET_PARAMS !!\n");
                    retVal = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }
            /* LDRA_JUSTIFY_START
            <metric start>statement branch <metric end>
            <justification start>
            Rationale:
            The test framework and test apps cannot reach this portion
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit:
            None;Unused feature, cannot be enabled, control cannot reach here.
            <justification end> */
            /* Set Bandwidth limiter */
            case VHWA_M2M_IOCTL_MSC_SET_BW_LIMITER:
            {
                if (NULL != cmdArgs)
                {
                    htsLimit = (Vhwa_HtsLimiter *)cmdArgs;
                    Vhwa_mscSetHtsLimit(hObj, htsLimit);
                }
                else
                {
                    GT_0trace(VhwaMscTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_MSC_SET_BW_LIMITER !!\n");
                    retVal = FVID2_EBADARGS;
                }
                break;
            }
            /* LDRA_JUSTIFY_END */
            /* LDRA_JUSTIFY_START
            <metric start>statement branch <metric end>
            <justification start>
            Rationale:
            The test framework and test apps cannot reach this portion
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit:
            None;Unused feature, cannot be enabled, control cannot reach here.
            <justification end> */
            case VHWA_M2M_IOCTL_MSC_GET_PERF:
            {
                Vhwa_M2mMscPerf *perf = NULL;

                if (NULL != cmdArgs)
                {
                    perf = (Vhwa_M2mMscPerf *)cmdArgs;
                    for (cnt = 0u; cnt < VHWA_M2M_MSC_MAX_COMP; cnt++)
                    {
                        perf->perf[cnt] = (uint32_t)hObj->perf[cnt];
                        perf->swOvhd[cnt] = hObj->swOvhd[cnt];
                    }
                }
                else
                {
                    GT_0trace(VhwaMscTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_MSC_GET_PERF !!\n");
                    retVal = FVID2_EBADARGS;
                }
                break;
            }
            /* LDRA_JUSTIFY_END */


            /* Get PSA signature values for enabled outputs */
            case VHWA_M2M_IOCTL_MSC_GET_PSA_SIGN:
            {
                Vhwa_M2mMscPsaSign *psa = NULL;

                /* LDRA_JUSTIFY_START
                <metric start>branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, then our code base is expected to prevent undefined behaviour due to dereferencing a NULL pointer.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != cmdArgs)
                /* LDRA_JUSTIFY_END */
                {
                    psa = (Vhwa_M2mMscPsaSign *)cmdArgs;
                    *psa = hObj->psa;
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaMscTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_MSC_GET_PSA_SIGN !!\n");
                    retVal = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            /* Enable Register error events callback */
            case VHWA_M2M_IOCTL_MSC_REGISTER_ERR_CB:
            {
                /* LDRA_JUSTIFY_START
                <metric start>branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, then our code base is expected to prevent undefined behaviour due to dereferencing a NULL pointer.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != cmdArgs)
                /* LDRA_JUSTIFY_END */
                {
                    eePrms = (Msc_ErrEventParams *)cmdArgs;
                    retVal = vhwa_m2mMscSetEeParams(instObj, hObj, eePrms);
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaMscTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_MSC_REGISTER_ERR_CB !!\n");
                    retVal = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            /* Enable Register error events callback */
            case VHWA_M2M_IOCTL_MSC_REGISTER_WDTIMER_ERR_CB:
            {
                /* LDRA_JUSTIFY_START
                <metric start>branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, then our code base is expected to prevent undefined behaviour due to dereferencing a NULL pointer.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != cmdArgs)
                /* LDRA_JUSTIFY_END */
                {
                    wdTimerEePrms = (Msc_WdTimerErrEventParams *)cmdArgs;
                    retVal = vhwa_m2mMscSetWdTimerEeParams(instObj, hObj, wdTimerEePrms);
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaMscTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_MSC_REGISTER_WDTIMER_ERR_CB !!\n");
                    retVal = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }
            /* LDRA_JUSTIFY_START
            <metric start>statement branch <metric end>
            <justification start>
            Rationale:
            The test framework and test apps cannot reach this portion
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit:
            None;Unused feature, cannot be enabled, control cannot reach here.
            <justification end> */
            case VHWA_M2M_IOCTL_MSC_SYNC_START:
            {
                if (NULL != hObj->createPrms.getTimeStamp)
                {
                    hObj->perf[0u] = hObj->createPrms.getTimeStamp();
                }

                retVal = CSL_htsPipelineStart(comObj->socInfo.htsRegs, &hObj->htsCfg[0]);
                break;
            }
            /* LDRA_JUSTIFY_END */


            case IOCTL_VHWA_MSC_GET_SL2_PARAMS:
            {
                Vhwa_M2mMscSl2Params *pSl2Prms = NULL;
                /* LDRA_JUSTIFY_START
                <metric start>branch <metric end>
                <justification start>Rationale:
                The test framework and test apps cannot reach this portion
                The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
                Effect on this unit:
                None;Unused feature, cannot be enabled, control cannot reach here.
                <justification end> */
                if (NULL != cmdArgs)
                /* LDRA_JUSTIFY_END */
                {
                    pSl2Prms = (Vhwa_M2mMscSl2Params *)cmdArgs;

                    Fvid2Utils_memcpy(pSl2Prms, &comObj->sl2Prms,
                                sizeof(Vhwa_M2mMscSl2Params));


                }
                /* LDRA_JUSTIFY_START
                <metric start> statement <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaMscTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_VHWA_MSC_GET_SL2_PARAMS !!\n");
                    retVal = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case IOCTL_VHWA_MSC_FC_GET_PARAMS:
            {
                Vhwa_M2mMscFcGetPrms *pfcPrms = NULL;
                /* LDRA_JUSTIFY_START
                <metric start>branch <metric end>
                <justification start>  Rationale:
                The test framework and test apps cannot reach this portion
                The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
                Effect on this unit:
                None;Unused feature, cannot be enabled, control cannot reach here.
                <justification end> */
                if (NULL != cmdArgs)
                /* LDRA_JUSTIFY_END */
                {
                    pfcPrms = (Vhwa_M2mMscFcGetPrms *)cmdArgs;
                    Vhwa_m2mMscFcGetPrms(hObj, pfcPrms);


                }
                /* LDRA_JUSTIFY_START
                <metric start> statement <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaMscTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_VHWA_MSC_FC_GET_PARAMS !!\n");
                    retVal = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }
            /* LDRA_JUSTIFY_START
            <metric start>statement branch <metric end>
            <justification start>  Rationale:
            The test framework and test apps cannot reach this portion
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit:
            None;Unused feature, cannot be enabled, control cannot reach here.
            <justification end> */
            case IOCTL_VHWA_MSC_FC_CONN_PARAMS:
            {
                Vhwa_M2mMscFcConPrms *pFcConnPrms = NULL;
                if (NULL != cmdArgs)
                {
                    pFcConnPrms = (Vhwa_M2mMscFcConPrms *)cmdArgs;

                    Vhwa_m2mMscUpdateFcConnPrms(hObj, pFcConnPrms);


                }
                else
                {
                    GT_0trace(VhwaMscTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_VHWA_MSC_FC_CONN_PARAMS !!\n");
                    retVal = FVID2_EBADARGS;
                }
                break;
            }
            /* LDRA_JUSTIFY_END */
            case IOCTL_VHWA_MSC_UPDATE_FC_PARAMS:
            {
                Vhwa_M2mMscFcUpdatePrms *pFcPrms = NULL;
                /* LDRA_JUSTIFY_START
                <metric start>branch <metric end>
                <justification start>  Rationale:
                The test framework and test apps cannot reach this portion
                The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
                Effect on this unit:
                None;Unused feature, cannot be enabled, control cannot reach here.
                <justification end> */
                if (NULL != cmdArgs)
                /* LDRA_JUSTIFY_END */
                {
                    pFcPrms = (Vhwa_M2mMscFcUpdatePrms *)cmdArgs;

                    Vhwa_m2mMscUpdateFcPrms(hObj, pFcPrms);


                }
                /* LDRA_JUSTIFY_START
                <metric start> statement <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaMscTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_VHWA_MSC_UPDATE_FC_PARAMS !!\n");
                    retVal = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }
#if !defined(VHWA_VPAC_IP_REV_VPAC3L)
            case VHWA_M2M_IOCTL_MSC_ENABLE_RECONFIG_REINIT_REG:
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
                        GT_1trace(VhwaMscTrace, GT_ERR,
                            "Invalid safety mode value %u for IOCTL_VHWA_M2M_MSC_ENABLE_RECONFIG_REG !!\n", modeVal);
                        retVal = FVID2_EBADARGS;
                    }
                    else
                    {
                        hObj->enableReconfigReinitReg = modeVal;
                        retVal = FVID2_SOK;
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
                    GT_0trace(VhwaMscTrace, GT_ERR,
                        "handle/cmdArgs cannot be NULL for IOCTL_VHWA_M2M_MSC_ENABLE_RECONFIG_REG !!\n");
                    retVal = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case VHWA_M2M_IOCTL_MSC_ENABLE_STATUS_REG_VALIDATE:
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((NULL != handle) && (NULL != cmdArgs))
                /* LDRA_JUSTIFY_END */
                {
                    uint32_t modeVal = *((uint32_t *)cmdArgs);
                    if (modeVal > VHWA_SAFETY_MODE_CONTINUOUS)
                    {
                        GT_1trace(VhwaMscTrace, GT_ERR,
                            "Invalid safety mode value %u for VHWA_M2M_IOCTL_MSC_ENABLE_STATUS_REG_VALIDATE !!\n", modeVal);
                        retVal = FVID2_EBADARGS;
                    }
                    else
                    {
                        cookie = HwiP_disable();
                        hObj->enableStatusRegValidate = modeVal;
                        HwiP_restore(cookie);
                        retVal = FVID2_SOK;
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
                    retVal = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case VHWA_M2M_IOCTL_MSC_VALIDATE_REG:
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != handle)
                /* LDRA_JUSTIFY_END */
                {
                    if (VHWA_SAFETY_MODE_DISABLED != hObj->enableStatusRegValidate)
                    {
                        retVal = Vhwa_m2mMscStatusRegValidate(hObj, &comObj->socInfo);
                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                        Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations. However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if (FVID2_SOK != retVal)
                        {
                            GT_0trace(VhwaMscTrace, GT_ERR, "VPAC-10: StatusRegValidate failed!\n");
                        }
                        /* LDRA_JUSTIFY_END */
                    }
                    if (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate)
                    {
                        retVal = Vhwa_m2mMscConfigRegReadback(hObj, comObj);
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

                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if ((VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate) && (FVID2_SOK == retVal))
                    /* LDRA_JUSTIFY_END */
                    {
                        /* Callback with goldenReg and ReadBackReg pointer to application for memory comparison */
                        /* LDRA_JUSTIFY_START
                        <metric start> branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if((NULL != hObj->configRegMemPrms.cbFxn) &&
                            (NULL != hObj->configRegMemPrms.configGoldenRegPtr) &&
                            (NULL != hObj->configRegMemPrms.configRegReadbackPtr))
                        /* LDRA_JUSTIFY_END */
                        {
                            Vhwa_M2mMscConfigRegMemParams configRegMemPtrs;
                            configRegMemPtrs.golden_ptr = hObj->configRegMemPrms.configGoldenRegPtr;
                            configRegMemPtrs.readback_ptr = hObj->configRegMemPrms.configRegReadbackPtr;
                            configRegMemPtrs.readback_mem_size = sizeof(VhwaVpacMscSocReadBack);
                            configRegMemPtrs.appData = (void *)hObj->configRegMemPrms.appData;

                            retVal = hObj->configRegMemPrms.cbFxn((Fvid2_Handle)hObj, &configRegMemPtrs);
                        }
                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                        <justification end> */
                        else
                        {
                            GT_0trace(VhwaMscTrace, GT_ERR, "ConfigRegMemParams are NULL!\n");
                            retVal = FVID2_EFAIL;
                        }
                        /* LDRA_JUSTIFY_END */

                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                        <justification end> */
                        /* Get status from callback for memory comparison */
                        if (FVID2_SOK != retVal)
                        {
                            GT_0trace(VhwaMscTrace, GT_ERR, "VPAC-7: ConfigRegReadbackCallback Memory Comparison failed!\n");
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
                Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaMscTrace, GT_ERR,
                        "handle cannot be NULL for VHWA_M2M_IOCTL_MSC_VALIDATE_REG !!\n");
                    retVal = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case VHWA_M2M_IOCTL_MSC_ENABLE_CONFIG_REG_READBACK:
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((NULL != handle) && (NULL != cmdArgs))
                /* LDRA_JUSTIFY_END */
                {
                    uint32_t modeVal = *((uint32_t *)cmdArgs);
                    if (modeVal > VHWA_SAFETY_MODE_CONTINUOUS)
                    {
                        GT_1trace(VhwaMscTrace, GT_ERR,
                            "Invalid safety mode value %u for VHWA_M2M_IOCTL_MSC_ENABLE_CONFIG_REG_READBACK !!\n", modeVal);
                        retVal = FVID2_EBADARGS;
                    }
                    else
                    {
                        cookie = HwiP_disable();
                        hObj->enableConfigRegValidate = modeVal;
                        HwiP_restore(cookie);
                        retVal = FVID2_SOK;
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
                    GT_0trace(VhwaMscTrace, GT_ERR,
                        "handle/cmdArgs cannot be NULL for VHWA_M2M_IOCTL_MSC_ENABLE_CONFIG_REG_READBACK !!\n");
                    retVal = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case VHWA_M2M_IOCTL_MSC_GET_READBACK_SIZE:
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((NULL != handle) && (NULL != cmdArgs))
                /* LDRA_JUSTIFY_END */
                {
                    uint32_t *readbackSize = (uint32_t *)cmdArgs;
                    *readbackSize = (uint32_t)sizeof(VhwaVpacMscSocReadBack);
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaMscTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_MSC_GET_READBACK_SIZE !!\n");
                    retVal = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case VHWA_M2M_IOCTL_MSC_SET_CONFIG_REG_PTRS:
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((NULL != handle) && (NULL != cmdArgs))
                /* LDRA_JUSTIFY_END */
                {
                    Vhwa_M2mMscConfigRegMemParams *reg_ptrs = (Vhwa_M2mMscConfigRegMemParams *)cmdArgs;
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if ((reg_ptrs->readback_ptr == NULL) ||
                        (reg_ptrs->golden_ptr == NULL) ||
                        (reg_ptrs->cbFxn == NULL))
                    {
                        GT_0trace(VhwaMscTrace, GT_ERR,
                            "readback_ptr, golden_ptr, cbFxn cannot be NULL for VHWA_M2M_IOCTL_MSC_SET_CONFIG_REG_PTRS !!\n");
                        retVal = FVID2_EBADARGS;
                    }
                    /* LDRA_JUSTIFY_END */
                    else
                    {
                        hObj->configRegMemPrms.configRegReadbackPtr = reg_ptrs->readback_ptr;
                        hObj->configRegMemPrms.configGoldenRegPtr = reg_ptrs->golden_ptr;
                        hObj->configRegMemPrms.cbFxn = reg_ptrs->cbFxn;
                        hObj->configRegMemPrms.appData = reg_ptrs->appData;
                        retVal = vhwaM2mMscSetDefaultGoldenRegMemValues(hObj, comObj);
                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if(FVID2_SOK != retVal)
                        {
                            GT_0trace(VhwaMscTrace, GT_ERR,
                                "Failed to set default goldenReg mem values for VHWA_M2M_IOCTL_MSC_SET_CONFIG_REG_PTRS !!\n");
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
                    GT_0trace(VhwaMscTrace, GT_ERR,
                        "cmdArgs cannot be NULL for VHWA_M2M_IOCTL_MSC_SET_CONFIG_REG_PTRS !!\n");
                    retVal = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }
#endif
            /* Default Case */
            /* LDRA_JUSTIFY_START
            <metric start>statement branch <metric end>
            <justification start>  Rationale:
            The negative test framework and test apps cannot reach this portion.
            These parameters are configured directly by the imaging kernel, and any invalid configurations are handled by the imaging kernel.
            Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            default:
            {
                retVal = FVID2_EUNSUPPORTED_CMD;
                break;
            }
            /* LDRA_JUSTIFY_END */
        }

        /* Release instance semaphore */
        (void)VhwaAl_SemaphoreP_post(instObj->lockSem);
    }

    return (retVal);
}

static int32_t Vhwa_m2mMscProcessReq(Fdrv_Handle handle,
    Fvid2_FrameList *inFrmList, Fvid2_FrameList *outFrmList, uint32_t timeout)
{
    int32_t                 retVal = FVID2_SOK;
    uint32_t                cnt, chCnt;
    uint32_t                semTimeout;
    Vhwa_M2mMscInstObj     *instObj = NULL;
    Vhwa_M2mMscHandleObj   *hObj = NULL;
    Vhwa_M2mMscCommonObj   *comObj = NULL;
    Vhwa_M2mMscQueueObj    *qObj = NULL;
    Vhwa_M2mMscParams      *mscPrms = NULL;
    Fvid2_Frame            *frm = NULL;
    int32_t       semStatus;

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
        retVal = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        hObj = (Vhwa_M2mMscHandleObj *)handle;
        instObj = &gM2mMscInstObj[hObj->instId];
        comObj = &gM2mMscCommonObj;

        mscPrms = &hObj->mscPrms;

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        /* Check if IP is in stalled state due to error interrupt or watchdog timeout */
        if ((uint32_t)UTRUE == instObj->isStalled)
        {
            retVal = FVID2_EFAIL;
            GT_0trace(VhwaMscTrace, GT_ERR,
                "Process request rejected: IP is in stalled state due to previous error\n");
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
    if ((FVID2_SOK == retVal) && (UFALSE == hObj->fcStatus.isFlexConnect))
    /* LDRA_JUSTIFY_END */
    {

        /* Input Buffer Addresses cannot be null */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((NULL == inFrmList->frames[0U]) ||
            (0U == inFrmList->frames[0u]->addr[0U]) ||
            (0U == inFrmList->numFrames) )
        {
            retVal = FVID2_EBADARGS;
        }
        /* LDRA_JUSTIFY_END */
        else
        {
            /* For YUV420 input, chroma buffer cannot be null */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((FVID2_DF_YUV420SP_UV == mscPrms->inFmt.dataFormat) ||
                (FVID2_DF_YUV420SP_VU == mscPrms->inFmt.dataFormat) ||
                (FVID2_DF_YUV422SP_UV == mscPrms->inFmt.dataFormat) ||
                (FVID2_DF_YUV422SP_VU == mscPrms->inFmt.dataFormat))
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
                if (0U == inFrmList->frames[0u]->addr[1U])
                {
                    retVal = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
            }
        }

        for (chCnt = 0u; chCnt < MSC_MAX_OUTPUT; chCnt ++)
        {
            if (UTRUE == hObj->outChPrms[chCnt].buffEnable)
            {
                frm = outFrmList->frames[chCnt];
                /* LDRA_JUSTIFY_START
                <metric start>statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL == frm)
                {
                    retVal = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                else
                {
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    This failure case is out of scope for the imaging test framework.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    for (cnt = 0U; cnt < hObj->numIter; cnt ++)
                    {
                        if (0U == frm->addr[cnt])
                        {
                            retVal = FVID2_EBADARGS;
                            break;
                        }
                    }
                    /* LDRA_JUSTIFY_END */

                }
            }
             /* LDRA_JUSTIFY_START
            <metric start>  statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK != retVal)
            {
                break;
            }
            /* LDRA_JUSTIFY_END */
        }
    }
     /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == retVal)
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

        /* Take the instance semaphore, so that no other
         * handle can submit request from the task context. */
        retVal = Vhwa_commonHwaLockAquire(instObj->hwaLockIdx, semTimeout);

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps.
        Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == retVal)
        /* LDRA_JUSTIFY_END */
        {
            /* This request is going to be submitted, so increment
             * number of pending request counter,
             * */
            hObj->numPendReq = (hObj->numPendReq + 1U);
        }

        for (cnt = 0U; cnt < MSC_MAX_OUTPUT; cnt ++)
        {
            if (FVID2_SOK != retVal)
            {
                break;
            }
            if (0U != (hObj->scalarUsed & ((uint32_t)1U << cnt)))
            {
                semStatus = VhwaAl_SemaphoreP_pend(comObj->scLockSem[cnt],
                                            SYSTEMP_WAIT_FOREVER);

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if(SYSTEMP_SUCCESS == semStatus)
                /* LDRA_JUSTIFY_END */
                {
                    retVal = FVID2_SOK;
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
                    /* Convert Semaphore status to FVID2 status */
                    if (SYSTEMP_TIMEOUT == semStatus)
                    {
                        retVal = FVID2_EAGAIN;
                    }
                    else
                    {
                        retVal = FVID2_EFAIL;
                    }
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
        if (FVID2_SOK == retVal)
        /* LDRA_JUSTIFY_END */

        {
            /* Get a queue object from the free queue,
             * No need to protect from ISR as it is not accessed from ISR */
            qObj = (Vhwa_M2mMscQueueObj *) Fvid2Utils_dequeue(instObj->freeQ);

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
                GT_0trace(VhwaMscTrace, GT_ERR,
                    "Failed to Free Queue Object!!\n");
                retVal = FVID2_EALLOC;
            }
            /* LDRA_JUSTIFY_END */
            else
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
        This branch statement checks for errors accumulated in previous steps.
        Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == retVal)
        /* LDRA_JUSTIFY_END */

        {
            Vhwa_m2mMscSetAddress(qObj->hObj, &qObj->inFrmList,
                                    &qObj->outFrmList);

            /* HW is free, submit request to the hardware */
            /* If previous handle and current handles are same, */
            if ((instObj->lastHndlObj == qObj->hObj) &&
                (0u == hObj->isCfgUpdated))
            {
                /* Update MSC Height Dependent config */
                retVal = Vhwa_m2mMscSetFrameSize(qObj, comObj, 0U);
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This branch statement checks for errors accumulated in previous steps.
                Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
            }
            else
            {
                /** Last handle was not same as new handle,
                 *  so require to recofigure all HW IPs */
                retVal = Vhwa_m2mMscSetConfigInHW(qObj, comObj, 0U);

                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (FVID2_SOK == retVal)
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
            if (FVID2_SOK == retVal)
            /* LDRA_JUSTIFY_END */
            {
                /* Enable Selected HW Interrupts */
                retVal = Vhwa_m2mMscSetIntrInHW(comObj->mscInitPrms.irqInfo[hObj->instId].vhwaIrqNum,
                    comObj->socInfo.vpacIntdRegs, qObj->hObj);
            }

#if !defined(VHWA_VPAC_IP_REV_VPAC3L)
            /* Invoke the reconfig-Reg if enableReconfigReinitReg enabled for the current handle */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The test framework and test apps cannot reach this portion.
            This portion is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: None; Unused feature, cannot be enabled, control cannot reach here.
            <justification end> */
            if ((FVID2_SOK == retVal) && (VHWA_SAFETY_MODE_DISABLED != hObj->enableReconfigReinitReg))
            {
                retVal = Vhwa_m2mMscReconfigReinitReg(instObj, comObj, qObj);
                /* Reset only in one-shot mode */
                if (VHWA_SAFETY_MODE_ONESHOT == hObj->enableReconfigReinitReg)
                {
                    hObj->enableReconfigReinitReg = (uint32_t)VHWA_SAFETY_MODE_DISABLED;
                }
            }
            /* LDRA_JUSTIFY_END */

            /* Update the MscStatusRegisterGroup with valid status register values for dynamic registers */
            if (VHWA_SAFETY_MODE_DISABLED != hObj->enableStatusRegValidate)
            {
                retVal = vhwaM2mMscUpdateStatusRegGroup(hObj);
            }
#endif

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK == retVal)
            /* LDRA_JUSTIFY_END */
            {
                /** Set the addresses, Submit the TR
                 *  Start the pipeline */
                retVal = Vhwa_m2mMscSubmitRequest(instObj, qObj,
                                            comObj, 0U);
            }
        }
    }

    return (retVal);
}

/** \brief Typedef for FVID2 get processed frames function pointer. */
static int32_t Vhwa_m2mMscGetProcessReq(Fdrv_Handle handle,
    Fvid2_FrameList *inFrmList, Fvid2_FrameList *outFrmList,
    uint32_t timeout)
{
    int32_t                 retVal = FVID2_SOK;
    uint32_t                cnt, cookie;
    Vhwa_M2mMscInstObj     *instObj = NULL;
    Vhwa_M2mMscHandleObj   *hObj = NULL;
    Vhwa_M2mMscQueueObj    *qObj = NULL;

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
        GT_0trace(VhwaMscTrace, GT_ERR, "NULL pointer\n");
        retVal = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        hObj = (Vhwa_M2mMscHandleObj *) handle;
        instObj = &gM2mMscInstObj[hObj->instId];
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
    if (FVID2_SOK == retVal)
    /* LDRA_JUSTIFY_END */
    {
#if defined (MCU_PLUS_SDK)
        uint32_t               statusQ;
        (void)VhwaAl_SemaphoreP_pend(instObj->lockSem, SYSTEMP_WAIT_FOREVER);
        /* Wait until completed request queue is populated.
         * Failure to exit this loop indicates that the ISR never hit.*/
        do
        {
            statusQ = Fvid2Utils_isQEmpty(hObj->outQ);
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
        qObj = (Vhwa_M2mMscQueueObj *) Fvid2Utils_dequeue(hObj->outQ);
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
            /* Restore interrupts after updating node list */
            HwiP_restore(cookie);

            /* Check if requests are pending with driver */
            if (0U == hObj->numPendReq)
            {
                /* Nothing is queued */
                GT_0trace(
                    VhwaMscTrace, GT_DEBUG,
                    "Nothing to dequeue. No request pending with driver!!\r\n");
                retVal = FVID2_ENO_MORE_BUFFERS;
            }
            else
            {
                /* If no request have completed, return try again */
                GT_0trace(VhwaMscTrace, GT_DEBUG,
                          "Nothing to dequeue. Try again!!\r\n");
                retVal = FVID2_EAGAIN;
            }
        }
        /* LDRA_JUSTIFY_END */
        else
        {
            /* Copy the driver's process list to application's process list */
            Fvid2_copyFrameList(inFrmList, &qObj->inFrmList);
            Fvid2_copyFrameList(outFrmList, &qObj->outFrmList);

            /* Return back the queue object to the free queue */
            Fvid2Utils_queue(instObj->freeQ, &qObj->qElem, qObj);

            /* Restore interrupts after updating node list */
            HwiP_restore(cookie);

            for (cnt = 0; cnt < hObj->numIter; cnt++)
            {
                /* Dequeue from Rings */
                retVal = Vhwa_m2mMscPopRings(hObj, &gM2mMscCommonObj, cnt);
            }

            /* Decrement the pending request count. */
             /* LDRA_JUSTIFY_START
            <metric start>  branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
            <justification end> */
            if (0U != hObj->numPendReq)
            /* LDRA_JUSTIFY_END */
            {
                hObj->numPendReq = hObj->numPendReq - 1U;
            }

        }
        /* New request can now be submitted to the MSC IP for non-readBack of StatusRegisters/ConfigurationRegisters enabled frames */
        if ((UFALSE == hObj->fcStatus.isFlexConnect) &&
            (VHWA_SAFETY_MODE_DISABLED == hObj->enableStatusRegValidate) &&
            (VHWA_SAFETY_MODE_DISABLED == hObj->enableConfigRegValidate))
        {
            /* New request can now be submitted to the MSC IP */
            Vhwa_commonHwaLockRelease(instObj->hwaLockIdx);
        }
        #if defined (MCU_PLUS_SDK)
        (void)VhwaAl_SemaphoreP_post(instObj->lockSem);
        #endif
    }

    return (retVal);
}


int32_t Vhwa_m2mMscAllocSl2(const Vhwa_M2mMscSl2AllocPrms *sl2AllocPrms)
{
    int32_t                 retVal = FVID2_SOK;
    uint32_t                sl2MemReq = 0;
    uint32_t                cnt, cnt1, channelCnt;
    uint32_t                inPitchSl2[VHWA_M2M_MSC_MAX_INST][VHWA_M2M_MSC_MAX_IN_CHANNEL];
    uint32_t                outPitchSl2[MSC_MAX_OUTPUT];
    uint64_t                sl2Addr;
    Vhwa_M2mMscCommonObj   *comObj = NULL;

    comObj = &gM2mMscCommonObj;

    channelCnt = VHWA_M2M_MSC_MAX_IN_CHANNEL;
     /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL == sl2AllocPrms)
    {
        GT_0trace(VhwaMscTrace, GT_ERR, "Bad Arguments\n");
        retVal = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
         /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)UFALSE == comObj->initDone)
        {
            GT_0trace(VhwaMscTrace, GT_ERR,
                "Driver init is not done!!\n");
            retVal = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)UTRUE == comObj->isSl2AllocDone)
        {
            GT_0trace(VhwaMscTrace, GT_ERR,
                "SL2 Memory is already allocated !!\n");
            retVal = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */

    }
    for (cnt = 0; cnt < VHWA_M2M_MSC_MAX_INST; cnt++)
    {
        if (FVID2_SOK != retVal)
        {
            break;
        }
        for (cnt1 = 0; cnt1 < channelCnt; cnt1++)
        {
            if (FVID2_SOK != retVal)
            {
                break;
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (sl2AllocPrms->inBuffDepth[cnt][cnt1] < MSC_M2M_MIN_IN_SL2_LINES)
            {
                GT_0trace(VhwaMscTrace, GT_ERR, "Bad Arguments\n");
                retVal = FVID2_EBADARGS;
            }
            /* LDRA_JUSTIFY_END */
        }
    }
    for (cnt = 0; cnt < MSC_MAX_OUTPUT; cnt++)
    {
        if (FVID2_SOK != retVal)
        {
            break;
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (sl2AllocPrms->outBuffDepth[cnt] < MSC_M2M_MIN_OUT_SL2_LINES)
        {
            GT_0trace(VhwaMscTrace, GT_ERR, "Bad Arguments\n");
            retVal = FVID2_EBADARGS;
        }
        /* LDRA_JUSTIFY_END */
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
    if (FVID2_SOK == retVal)
    /* LDRA_JUSTIFY_END */

    {
        /* Calculate Sl2 buffer size required for input */
        for (cnt = 0; cnt < VHWA_M2M_MSC_MAX_INST; cnt++)
        {
            for (cnt1 = 0; (cnt1 < channelCnt) && (FVID2_SOK == retVal);
                cnt1++)
            {
                inPitchSl2[cnt][cnt1] = Vhwa_calcHorzSizeInBytes(
                                            sl2AllocPrms->maxInWidth[cnt][cnt1],
                                            sl2AllocPrms->inCcsf[cnt][cnt1]);
                inPitchSl2[cnt][cnt1] = Vhwa_calcSl2Pitch(inPitchSl2[cnt][cnt1]);

                sl2MemReq += inPitchSl2[cnt][cnt1] *
                                sl2AllocPrms->inBuffDepth[cnt][cnt1];
            }
        }

        /* Calculate Sl2 buffer size required for output */
        for (cnt = 0; cnt < MSC_MAX_OUTPUT; cnt++)
        {
            outPitchSl2[cnt] = Vhwa_calcHorzSizeInBytes(
                sl2AllocPrms->maxOutWidth[cnt], sl2AllocPrms->outCcsf[cnt]);
            outPitchSl2[cnt] = Vhwa_calcSl2Pitch(outPitchSl2[cnt]);

            sl2MemReq += outPitchSl2[cnt] * sl2AllocPrms->outBuffDepth[cnt];
        }

        /* Check if 16KB alignment is requested */
        if ((uint32_t)TRUE == sl2AllocPrms->enable16KBAlignment)
        {
            /* Apply 16KB alignment */
            sl2MemReq = ((sl2MemReq + 0x3FFFu) & ~0x3FFFu);

            /* Allocate with 16KB alignment */
            sl2Addr = Vhwa_allocateSl2Aligned(sl2MemReq, VHWA_SL2_INST_VPAC, 0x4000u);
        }
        else
        {
            /* Standard allocation without alignment overhead */
            sl2Addr = Vhwa_allocateSl2(sl2MemReq, VHWA_SL2_INST_VPAC);
        }
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
            GT_0trace(VhwaMscTrace, GT_ERR,
                "SL2 aligned allocation failed (check alignment=0x4000 is power of 2 and sufficient SL2 memory available)\n");
            retVal = FVID2_EALLOC;
        }
        /* LDRA_JUSTIFY_END */
        else
        {
            comObj->sl2Prms.sl2StartAddr = sl2Addr;
            comObj->sl2Prms.sl2MemSize = sl2MemReq;
            comObj->isSl2AllocDone = (uint32_t)UTRUE;
            Fvid2Utils_memcpy(&comObj->sl2AllocPrms, sl2AllocPrms,
                sizeof(Vhwa_M2mMscSl2AllocPrms));
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
    if (FVID2_SOK == retVal)
    /* LDRA_JUSTIFY_END */
    {
        /* Calculate Sl2 buffer size required for input */
        for (cnt = 0; cnt < VHWA_M2M_MSC_MAX_INST; cnt++)
        {
            for (cnt1 = 0; (cnt1 < channelCnt) && (FVID2_SOK == retVal);
            cnt1++)
            {
                comObj->sl2Prms.inSl2Addr[cnt][cnt1] = sl2Addr;

                comObj->sl2Prms.inSl2BuffDepth[cnt][cnt1] =
                                            sl2AllocPrms->inBuffDepth[cnt][cnt1];

                sl2Addr += (uint64_t)inPitchSl2[cnt][cnt1] *
                               (uint64_t)sl2AllocPrms->inBuffDepth[cnt][cnt1];
            }
        }

        /* Calculate Sl2 buffer size required for output */
        for (cnt = 0; cnt < MSC_MAX_OUTPUT; cnt++)
        {
            comObj->sl2Prms.outSl2Addr[cnt] = sl2Addr;
            comObj->sl2Prms.outSl2BuffDepth[cnt] =
                                            sl2AllocPrms->outBuffDepth[cnt];

            sl2Addr += (uint64_t)outPitchSl2[cnt] *
                       (uint64_t)sl2AllocPrms->outBuffDepth[cnt];
        }
    }

    return retVal;
}



int32_t Vhwa_m2mMscFreeSl2(void)
{
    int32_t               retVal = FVID2_SOK;
    Vhwa_M2mMscCommonObj *comObj;

    comObj = &gM2mMscCommonObj;

    Vhwa_FreeSl2(VHWA_SL2_INST_VPAC);

    comObj->isSl2AllocDone = (uint32_t)UFALSE;

    return retVal;
}

/**
 * \brief   Returns handle object for the requested handle count.
 *
 * \param   cnt              count.
 *          instance         instance
 *
 * \return  reference to the handle object.
 *
 **/
/* LDRA_JUSTIFY_START
<metric start> statement <metric end>
<justification start>
Rationale: The component level negative test framework and test applications CAN reach this portion.
No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
<justification end> */
Vhwa_M2mMscHandleObj *Vhwa_m2mMscGetHandleObj(uint32_t instance, uint32_t cnt)
{
    return &gM2mMscHandleObj[instance][cnt];
}
/* LDRA_JUSTIFY_END */
/* ========================================================================== */
/*                           Local Functions                                  */
/* ========================================================================== */
static int32_t Vhwa_mscInit(const Vhwa_M2mMscInitParams *pInitPrms)
{
    int32_t             retVal = FVID2_SOK;
    uint32_t            cnt;
    #if !defined(MCU_PLUS_SDK)
    SemaphoreP_Params   semParams[VHWA_M2M_MSC_MAX_INST];
    SemaphoreP_Params   scSemParams[MSC_MAX_OUTPUT];
    #endif
    Vhwa_M2mMscInstObj *instObj = NULL;
    Msc_SocInfo        *socInfo = NULL;

    /* Check for Null pointer */
    GT_assert(VhwaMscTrace, (NULL != pInitPrms));

    for (cnt = 0; cnt < VHWA_M2M_MSC_MAX_INST; cnt++)
    {
        if (FVID2_SOK != retVal)
        {
            break;
        }
        /* Initialize the semaphore parameters and create semaphore pool */
        #if defined(MCU_PLUS_SDK)
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(FVID2_SOK != SemaphoreP_constructBinary(&gM2mMscInstObj[cnt].lockSem,1U))
        #else
        SemaphoreP_Params_init(&semParams[cnt]);
        semParams[cnt].mode = SemaphoreP_Mode_BINARY;
        gM2mMscInstObj[cnt].lockSem = SemaphoreP_create(1U, &semParams[cnt]);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL == gM2mMscInstObj[cnt].lockSem)
        #endif
        {
            retVal = FVID2_EALLOC;
        }
        /* LDRA_JUSTIFY_END */

        if (FVID2_SOK == retVal)
        {
#if defined VHWA_M2M_VPAC_INSTANCE
#if (VHWA_M2M_VPAC_INSTANCE == 0)
            gM2mMscInstObj[cnt].hwaLockIdx = VHWA_VPAC0_MSC0_LOCK_IDX + cnt;
#elif defined(VHWA_M2M_VPAC_INSTANCE) && (VHWA_M2M_VPAC_INSTANCE == 1)
            gM2mMscInstObj[cnt].hwaLockIdx = VHWA_VPAC1_MSC0_LOCK_IDX + cnt;
#endif
#else
            gM2mMscInstObj[cnt].hwaLockIdx = VHWA_VPAC0_MSC0_LOCK_IDX + cnt;
#endif
            retVal = Vhwa_commonCreateHwaLock(gM2mMscInstObj[cnt].hwaLockIdx);
        }
    }

    for (cnt = 0; cnt < MSC_MAX_OUTPUT; cnt++)
    {
        if (FVID2_SOK != retVal)
        {
            break;
        }
        /* Initialize the semaphore parameters and create semaphore pool */
        #if defined(MCU_PLUS_SDK)
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(FVID2_SOK != SemaphoreP_constructBinary(&gM2mMscCommonObj.scLockSem[cnt],1U))
        #else
        SemaphoreP_Params_init(&scSemParams[cnt]);
        scSemParams[cnt].mode = SemaphoreP_Mode_BINARY;
        gM2mMscCommonObj.scLockSem[cnt] = SemaphoreP_create(
            1U, &scSemParams[cnt]);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL == gM2mMscCommonObj.scLockSem[cnt])
        #endif
        {
            retVal = FVID2_EALLOC;
        }
        /* LDRA_JUSTIFY_END */
    }

    /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end>
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
        /* Copy Init Parameters */
        Fvid2Utils_memcpy(&gM2mMscCommonObj.mscInitPrms, pInitPrms,
                            sizeof(Vhwa_M2mMscInitParams));

        gM2mMscCommonObj.initDone = (uint32_t)UTRUE;
        gM2mMscCommonObj.isSl2AllocDone = (uint32_t)UFALSE;

        socInfo = &gM2mMscCommonObj.socInfo;

        Msc_getSocInfo(socInfo);

        /* Initialize UDMA */
        retVal  = Vhwa_m2mMscUdmaChInit(&gM2mMscCommonObj);
    }

    for (cnt = 0; cnt < VHWA_M2M_MSC_MAX_INST; cnt++)
    {
        if (FVID2_SOK != retVal)
        {
            break;
        }
        instObj = &gM2mMscInstObj[cnt];

        retVal = Vhwa_mscCreateQueue(instObj);
    }
     /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end>
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
        /* Enable Module and interrupt */
        CSL_vpacEnableModule(socInfo->vpacCntlRegs, VPAC_MODULE_MSC,
            (uint32_t)UTRUE);

        /* Enable the MSC Module */
        CSL_mscEnable(socInfo->mscRegs);

        for (cnt = 0; cnt < VHWA_M2M_MSC_MAX_INST; cnt++)
        {
            if (FVID2_SOK != retVal)
            {
                break;
            }

            instObj = &gM2mMscInstObj[cnt];

            /* Register interrupt */
            if (cnt == VPAC_MSC_INST_ID_0)
            {
                /* Set HTS pipeline */
                instObj->pipeline = VHWA_M2M_MSC0_HTS_PIPELINE;

                /* Initialize stalled flag to FALSE */
                instObj->isStalled = (uint32_t)UFALSE;

                retVal = Vhwa_m2mMscRegisterIsr(
                    instObj, &gM2mMscCommonObj, cnt);
            }
            else
            {
                /* Set HTS pipeline */
                instObj->pipeline = VHWA_M2M_MSC1_HTS_PIPELINE;

                /* Initialize stalled flag to FALSE */
                instObj->isStalled = (uint32_t)UFALSE;

                retVal = Vhwa_m2mMscRegisterIsr(
                    instObj, &gM2mMscCommonObj, cnt);
            }
        }
    }

    return (retVal);
}

static int32_t Vhwa_mscDeInit(void)
{
    int32_t             retVal = FVID2_SOK;
    uint32_t            cnt;
    Vhwa_M2mMscInstObj *instObj = NULL;

    /* Stop UDMA channels */
    retVal = Vhwa_m2mMscStopCh(&gM2mMscCommonObj);

    for (cnt = 0; cnt < VHWA_M2M_MSC_MAX_INST; cnt++)
    {
        if (FVID2_SOK != retVal)
        {
            break;
        }
        /* Check if any handle is open */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario
        Effect on this unit: If the control reaches here, our code base is expected to print the error status.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(gM2mMscInstObj[cnt].openCnt > 0U)
        {
            GT_0trace(VhwaMscTrace, GT_ERR,
                "Warning: All driver handles are not closed!!\n");
        }
        /* LDRA_JUSTIFY_END */
        instObj = &gM2mMscInstObj[cnt];

        Vhwa_m2mMscUnregisterIsr(instObj, &gM2mMscCommonObj, cnt);

        /* Delete Semaphore */
    #if defined(MCU_PLUS_SDK)
        SemaphoreP_destruct(&gM2mMscInstObj[cnt].lockSem);
    #else
        (void)SemaphoreP_delete(gM2mMscInstObj[cnt].lockSem);
        gM2mMscInstObj[cnt].lockSem = NULL;
    #endif

        Vhwa_commonDeleteHwaLock(gM2mMscInstObj[cnt].hwaLockIdx);

        /* Free up allocated resources */
        (void)Vhwa_mscDeleteQueue(instObj);
    }

    for (cnt = 0; cnt < MSC_MAX_OUTPUT; cnt++)
    {
        /* Delete Semaphore */
    #if defined(MCU_PLUS_SDK)
        SemaphoreP_destruct(&gM2mMscCommonObj.scLockSem[cnt]);
    #else
        (void)SemaphoreP_delete(gM2mMscCommonObj.scLockSem[cnt]);
    #endif
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((uint32_t)UTRUE == gM2mMscCommonObj.isSl2AllocDone)
    /* LDRA_JUSTIFY_END */
    {
        (void)Vhwa_m2mMscFreeSl2();
    }

    /* Deinitialize the UDMA channels */
    retVal += Vhwa_m2mMscUdmaChDeInit(&gM2mMscCommonObj);


    /* Disable Module and interrupt */
    CSL_vpacEnableModule(gM2mMscCommonObj.socInfo.vpacCntlRegs,
        VPAC_MODULE_MSC, (uint32_t)UFALSE);

    gM2mMscCommonObj.initDone = (uint32_t)UFALSE;

    return (retVal);
}

#if !defined(VHWA_VPAC_IP_REV_VPAC3L)
int32_t Vhwa_m2mMscReInit(void)
{
    int32_t                status = FVID2_SOK;
    uint32_t               cnt;
    Vhwa_M2mMscInstObj    *instObj = NULL;

    /** If isregistered is set, then Vhwa_m2mDofUdmaInit is called and UDMA
     * channels are initialized, so reinit is required. */
    /* LDRA_JUSTIFY_START_DISABLED
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((uint32_t)UTRUE == gM2mMscCommonObj.initDone)
    /* LDRA_JUSTIFY_END_DISABLED */
    {
        status = Vhwa_m2mMscStopCh(&gM2mMscCommonObj);

        if(FVID2_SOK == status)
        {
            status = Vhwa_m2mMscUdmaChDeInit(&gM2mMscCommonObj);
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if(FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            for (cnt = 0; (cnt < VHWA_M2M_MSC_MAX_INST); cnt++)
            {
                instObj = &gM2mMscInstObj[cnt];
                /* Need to unregister ISR, so it can be re-registered in next step */
                Vhwa_m2mMscUnregisterIsr(instObj, &gM2mMscCommonObj, cnt);
            }
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if(UDMA_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            /* Now reinitialized channels */
            status = Vhwa_m2mMscUdmaChInit(&gM2mMscCommonObj);
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            /* Enable Module and interrupt */
            CSL_vpacEnableModule(gM2mMscCommonObj.socInfo.vpacCntlRegs,
                VPAC_MODULE_MSC, (uint32_t)UTRUE);

            /* Enable the MSC Module */
            CSL_mscEnable(gM2mMscCommonObj.socInfo.mscRegs);
        }

        for (cnt = 0; cnt < VHWA_M2M_MSC_MAX_INST; cnt++)
        {
            if (FVID2_SOK != status)
            {
                break;
            }
            
            instObj = &gM2mMscInstObj[cnt];

            /* Register interrupt */
            if (cnt == VPAC_MSC_INST_ID_0)
            {
                status = Vhwa_m2mMscRegisterIsr(
                    instObj, &gM2mMscCommonObj, cnt);
            }
            else
            {
                status = Vhwa_m2mMscRegisterIsr(
                    instObj, &gM2mMscCommonObj, cnt);
            }
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            instObj = &gM2mMscInstObj[0u];
            /* Now start all channels, it initializes all input and output channels, so inst0 can be used. */
            status = Vhwa_m2mMscStartCh(instObj, &gM2mMscCommonObj);
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            for (cnt = 0; (cnt < VHWA_M2M_MSC_MAX_INST) && (status == FVID2_SOK);
                        cnt++)
            {
                instObj = &gM2mMscInstObj[cnt];
                Vhwa_enableHtsIntr(gM2mMscCommonObj.socInfo.vpacIntdRegs,
                    gM2mMscCommonObj.mscInitPrms.irqInfo[cnt].vhwaIrqNum,
                    instObj->pipeline);
            }
        }

        /* Setting last handle to false/null, so that reconfiguration is done */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            for (cnt = 0; (cnt < VHWA_M2M_MSC_MAX_INST) && (status == FVID2_SOK);
                        cnt++)
            {
                gM2mMscInstObj[cnt].lastHndlObj = NULL;
            }
        }
    }

    return (status);
}
#endif
static Vhwa_M2mMscHandleObj *Vhwa_mscAllocHdlObj(const Vhwa_M2mMscInstObj *instObj,
                                                uint32_t instId)
{
    uint32_t              cnt;
    Vhwa_M2mMscHandleObj *hObj = NULL;

    /* Check for Null pointer */
    GT_assert(VhwaMscTrace, (NULL != instObj));
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
    <justification end> */
    for (cnt = 0U; cnt < VHWA_M2M_MSC_MAX_HANDLES; cnt ++)
    /* LDRA_JUSTIFY_END */
    {
        if ((uint32_t)UFALSE == gM2mMscHandleObj[instId][cnt].isUsed)
        {
            /* Allocate Handle Object */
            hObj = &gM2mMscHandleObj[instId][cnt];
            Fvid2Utils_memset(hObj, 0x0, sizeof(Vhwa_M2mMscHandleObj));
            gM2mMscHandleObj[instId][cnt].isUsed = (uint32_t)UTRUE;
            hObj->enableReconfigReinitReg = (uint32_t)VHWA_SAFETY_MODE_DISABLED;
            hObj->enableStatusRegValidate = (uint32_t)VHWA_SAFETY_MODE_DISABLED;
            hObj->idx = cnt;
            hObj->instId = instId;
            break;
        }
    }

    return (hObj);
}

static int32_t Vhwa_mscCreateQueue(Vhwa_M2mMscInstObj *instObj)
{
    int32_t              retVal = FVID2_SOK;
    uint32_t             qCnt;
    Vhwa_M2mMscQueueObj *qObj;

    /* NULL pointer check */
    GT_assert(VhwaMscTrace, (NULL != instObj));

    instObj->activeQObj = NULL;
    instObj->freeQ = NULL;

    /* Create Free queue */
    retVal = Fvid2Utils_constructQ(&instObj->freeLlObj);
    GT_assert(VhwaMscTrace, (retVal == FVID2_SOK));

    instObj->freeQ = &instObj->freeLlObj;

    /* Initialize and queue the allocate queue object to free Q */
    for (qCnt = 0U; qCnt < VHWA_MSC_QUEUE_LEN_PER_INST; qCnt++)
    {
        qObj = &instObj->mscQObj[qCnt];
        qObj->hObj = NULL;
        Fvid2Utils_queue(instObj->freeQ, &qObj->qElem, qObj);
    }

    return retVal;
}

static int32_t Vhwa_mscDeleteQueue(Vhwa_M2mMscInstObj *instObj)
{
    int32_t              retVal = FVID2_SOK;
    Vhwa_M2mMscQueueObj *qObj;

    /* NULL pointer check */
    GT_assert(VhwaMscTrace, (NULL != instObj));
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> Rationale:
    The negative test framework and test apps cannot reach this portion.
    These parameters cannot be configured from test app or kernel directly and can only be overwritten by memory corruption. This is fail safe mechanism and cannot be removed
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != instObj->freeQ)
    /* LDRA_JUSTIFY_END */

    {
        /* Free-up all the queued free queue objects */
        do
        {
            qObj = (Vhwa_M2mMscQueueObj *) Fvid2Utils_dequeue(instObj->freeQ);
        } while (NULL != qObj);

        /* Delete the free Q */
        Fvid2Utils_destructQ(instObj->freeQ);
        instObj->freeQ = NULL;
    }

    return (retVal);
}

static void Vhwa_mscFreeHandleObj(Vhwa_M2mMscHandleObj *hObj)
{
    uint32_t cnt;

    /* Check for Null pointer */
    GT_assert(VhwaMscTrace, (NULL != hObj));
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
    <justification end> */
    for (cnt = 0U; cnt < VHWA_M2M_MSC_MAX_HANDLES; cnt ++)
    /* LDRA_JUSTIFY_END */
    {
        /* Freeup allocated Handle Object */
        if (hObj == &gM2mMscHandleObj[hObj->instId][cnt])
        {
            hObj->isUsed = (uint32_t)UFALSE;;
            break;
        }
    }

}

static void Vhwa_mscInitHandleObj(const Vhwa_M2mMscInstObj *instObj,
    Vhwa_M2mMscHandleObj *hObj)
{
    /* Check for Null pointer */
    GT_assert(VhwaMscTrace, (NULL != hObj));
    GT_assert(VhwaMscTrace, (NULL != instObj));

    /* Initialize MSC with Default configuration */
    Vhwa_mscInitMscCfg(&hObj->mscPrms);
    /* Set HTS Configuration for default MSC configuration */
    Vhwa_mscSetHtsCfg(instObj, hObj, &hObj->mscPrms);
    /* Set LSE Configuration for default MSC configuration */
    Vhwa_mscSetLseCfg(hObj, &hObj->mscPrms);
}

static int32_t Vhwa_mscSetFilterCoeff(Vhwa_M2mMscCommonObj *comObj,
                                      const Msc_Coeff *pCoeffCfg)
{
    int32_t retVal = FVID2_SOK;
    uint32_t setIdx;
    Vhwa_M2mMscCoeffStorage *pStorage;

    /* Check for Null pointer */
    GT_assert(VhwaMscTrace, (NULL != comObj));
    GT_assert(VhwaMscTrace, (NULL != pCoeffCfg));

    /* Set coefficients to hardware */
    retVal = CSL_mscSetCoeff(comObj->socInfo.mscRegs, pCoeffCfg);

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == retVal)
    /* LDRA_JUSTIFY_END */
    {
        /* Store coefficients in common object for later reconfiguration */
        pStorage = &comObj->coeffStorage;

        /* Store single phase coefficients */
        for (setIdx = 0U; setIdx < MSC_MAX_SP_COEFF_SET; setIdx++)
        {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL != pCoeffCfg->spCoeffSet[setIdx])
            /* LDRA_JUSTIFY_END */
            {
                /* Copy the coefficient array (5 taps) */
                Fvid2Utils_memcpy(&pStorage->spCoeff[setIdx][0],
                       pCoeffCfg->spCoeffSet[setIdx],
                       MSC_MAX_TAP * sizeof(int32_t));

                pStorage->spCoeffValid[setIdx] = (uint32_t)UTRUE;
            }
        }

        /* Store multi phase coefficients */
        for (setIdx = 0U; setIdx < MSC_MAX_MP_COEFF_SET; setIdx++)
        {
            if (NULL != pCoeffCfg->mpCoeffSet[setIdx])
            {
                /* Copy the coefficient array (32 phases x 5 taps = 160) */
                Fvid2Utils_memcpy(&pStorage->mpCoeff[setIdx][0],
                       pCoeffCfg->mpCoeffSet[setIdx],
                       (MSC_MAX_TAP * 32U) * sizeof(int32_t));

                pStorage->mpCoeffValid[setIdx] = (uint32_t)UTRUE;
            }
        }

        /* Mark that coefficients have been configured */
        pStorage->isCoeffConfigured = (uint32_t)UTRUE;
    }

    return (retVal);
}

static int32_t Vhwa_mscSetParams(const Vhwa_M2mMscInstObj *instObj,
    Vhwa_M2mMscHandleObj *hObj, const Vhwa_M2mMscCommonObj *comObj,
    Vhwa_M2mMscParams *mscPrms)
{
    int32_t retVal = FVID2_SOK;
    uint32_t cnt;

    /* Check for Null pointer */

    GT_assert(VhwaMscTrace, (NULL != hObj));
    GT_assert(VhwaMscTrace, (NULL != instObj));
    GT_assert(VhwaMscTrace, (NULL != comObj));

    retVal = Vhwa_mscCheckMscCfg(hObj, mscPrms);

    if (FVID2_SOK == retVal)
    {
        Fvid2Utils_memcpy(&hObj->mscPrms, mscPrms, sizeof(Vhwa_M2mMscParams));

        /* Set and calculate the parameters */
        Vhwa_mscCalPrms(hObj, mscPrms);

        /* Initialize HTS configuration based on the MSC Config */
        Vhwa_mscSetHtsCfg(instObj, hObj, &hObj->mscPrms);
        /* Initialize LSE configuration based on the MSC Config */
        Vhwa_mscSetLseCfg(hObj, &hObj->mscPrms);

        for (cnt = 0u; cnt < hObj->numIter; cnt++)
        {
            if (FVID2_SOK != retVal)
            {
                break;
            }
            /* Verify HTS Configuration */
            retVal = CSL_htsCheckThreadConfig(&hObj->htsCfg[cnt]);
        }
        for (cnt = 0u; cnt < hObj->numIter; cnt++)
        {
            if (FVID2_SOK != retVal)
            {
                break;
            }
            /* Verify LSE Configuration */
            retVal = CSL_lseCheckConfig(&hObj->lseCfg[cnt]);
        }
        /* LDRA_JUSTIFY_START
        <metric start>  branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps.
        Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == retVal)
        /* LDRA_JUSTIFY_END */

        {
            /* Setup TR Descriptor */
            Vhwa_mscM2mSetTrDesc(hObj, comObj);
        }
    }

    if(FVID2_SOK != retVal)
    {
        /* Initialize MSC with Default configuration */
        Vhwa_mscInitMscCfg(&hObj->mscPrms);
        /* Set HTS Configuration for default MSC configuration */
        Vhwa_mscSetHtsCfg(instObj, hObj, &hObj->mscPrms);
        /* Set LSE Configuration for default MSC configuration */
        Vhwa_mscSetLseCfg(hObj, &hObj->mscPrms);
    }
    else
    {
        /* Protecting it this variable using hwaLock, as this is
         * where this variable is being accessed. */
        (void)Vhwa_commonHwaLockAquire(instObj->hwaLockIdx, SYSTEMP_WAIT_FOREVER);
        hObj->isCfgUpdated = 1u;
        (void)Vhwa_commonHwaLockRelease(instObj->hwaLockIdx);
    }

    return (retVal);
}

static int32_t vhwa_m2mMscSetEeParams(const Vhwa_M2mMscInstObj *instObj,
    Vhwa_M2mMscHandleObj *hObj, const Msc_ErrEventParams *eePrms)
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
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((NULL != instObj->activeQObj) && (instObj->activeQObj->hObj == hObj))
        {
            GT_0trace(VhwaMscTrace, GT_ERR,
                "Cannot change error events while handle is active !!\n");
            status = FVID2_EAGAIN;
        }
        else if (0U != (eePrms->errEvents & (~VHWA_MSC_ALL_ERR_EVENTS)))
        {
            GT_1trace(VhwaMscTrace, GT_ERR,
                "Invalid Error Event bits to be enabled: 0x%08x !!\n",
                eePrms->errEvents);
            status = FVID2_EINVALID_PARAMS;
        }
        /* LDRA_JUSTIFY_END */
        else
        {
            Fvid2Utils_memcpy(&hObj->eePrms, eePrms, sizeof(Msc_ErrEventParams));
            status = FVID2_SOK;
            hObj->isCfgUpdated = 1u;
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

static int32_t vhwa_m2mMscSetWdTimerEeParams(const Vhwa_M2mMscInstObj *instObj,
    Vhwa_M2mMscHandleObj *hObj, const Msc_WdTimerErrEventParams *wdTimerEePrms)
{
    int32_t status = FVID2_SOK;
    uint32_t wdTimerMask = 0u;
    uint32_t               itrCnt;
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
        if (0U == (hObj->instId % 2U))
        {
            wdTimerMask = VHWA_MSC0_WDTIMER_ERR;
        }
        else
        {
            wdTimerMask = VHWA_MSC1_WDTIMER_ERR;
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        /* Check if this handle is already being processed */
        if ((NULL != instObj->activeQObj) && (instObj->activeQObj->hObj == hObj))
        {
            GT_0trace(VhwaMscTrace, GT_ERR,
                "Cannot change watchdog timer error events while handle is active !!\n");
            status = FVID2_EAGAIN;
        }
        else if (0U != (wdTimerEePrms->wdTimerErrEvents & ~wdTimerMask))
        {
            GT_1trace(VhwaMscTrace, GT_ERR,
                "Invalid Watchdog Timer Error Event bit to be enabled: 0x%08x !!\n",
                wdTimerEePrms->wdTimerErrEvents);
            status = FVID2_EINVALID_PARAMS;
        }
        /* LDRA_JUSTIFY_END */
        else
        {
            Fvid2Utils_memcpy(&hObj->wdTimerEePrms, wdTimerEePrms, sizeof(Msc_WdTimerErrEventParams));

            for (itrCnt = 0; itrCnt < hObj->numIter; itrCnt++)
            {
                /* Enable watchdog timer in HTS if WDTIMER_ERR event is registered */
                if (0U != (wdTimerEePrms->wdTimerErrEvents & wdTimerMask))
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
                else
                {
                    hObj->htsCfg[itrCnt].enableWdTimer = (uint32_t)UFALSE;
                }
            }
            hObj->isCfgUpdated = 1u;

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

static int32_t Vhwa_m2mMscSetIntrInHW(uint32_t vhwaIrqNum,
                                volatile CSL_vpac_intd_cfgRegs  *intdRegs,
                                const Vhwa_M2mMscHandleObj *hObj)
{
    int32_t status = FVID2_SOK;
    uint32_t wdTimerMask = 0u;

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
        /* Clear all MSC error interrupts */
        CSL_REG32_WR(&intdRegs->ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_MSC_INTR_REG],
            VHWA_MSC_ALL_ERR_EVENTS);
        /* Enable selected error interrupts */
        CSL_REG32_WR(&intdRegs->ENABLE_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_MSC_INTR_REG],
            hObj->eePrms.errEvents);

        if (0U == (hObj->instId % 2U))
        {
            wdTimerMask = VHWA_MSC0_WDTIMER_ERR;
        }
        else
        {
            wdTimerMask = VHWA_MSC1_WDTIMER_ERR;
        }
        /* Clear all watchdog timer interrupts (both MSC0 and MSC1) */
        CSL_REG32_WR(&intdRegs->ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_WDTIMER_INTR_REG],
            wdTimerMask);
        CSL_REG32_WR(&intdRegs->ENABLE_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_WDTIMER_INTR_REG],
            hObj->wdTimerEePrms.wdTimerErrEvents);
    }

    return (status);
}

static void Vhwa_mscInitMscCfg(Vhwa_M2mMscParams *mscPrms)
{
    uint32_t      chCnt;
    Fvid2_Format *fmt = NULL;

    /* Check for Null pointer */
    GT_assert(VhwaMscTrace, (NULL != mscPrms));

    /* Initialize MSC Configuration with the detault configuration */
    fmt = &mscPrms->inFmt;

    fmt->width = VHWA_MSC_DEF_WIDTH;
    fmt->height = VHWA_MSC_DEF_HEIGHT;
    fmt->dataFormat = FVID2_DF_LUMA_ONLY;
    fmt->ccsFormat = FVID2_CCSF_BITS12_PACKED;
    fmt->pitch[0U] = (VHWA_MSC_DEF_WIDTH * 3U) / 2U;

    for (chCnt = 0u; chCnt < MSC_MAX_OUTPUT; chCnt ++)
    {
        Fvid2Utils_memcpy(&mscPrms->outFmt[chCnt], fmt, sizeof(Fvid2_Format));
    }

    mscPrms->loopBack = UFALSE;
    mscPrms->enableLineSkip = UFALSE;
    mscPrms->enablePsa = UFALSE;

    Msc_ConfigInit(&mscPrms->mscCfg);
}


static int32_t Vhwa_mscCheckMscCfg(Vhwa_M2mMscHandleObj *hObj,
                                   Vhwa_M2mMscParams *mscPrms)
{
    int32_t         retVal = FVID2_SOK;
    uint32_t        cnt, minPitch;
    Fvid2_Format   *inFmt = NULL;
    Fvid2_Format   *outFmt = NULL;
    Msc_ScConfig   *scCfg = NULL;

    /* Check for Null pointer */
    GT_assert(VhwaMscTrace, (NULL != hObj));
    GT_assert(VhwaMscTrace, (NULL != mscPrms));

    inFmt = &mscPrms->inFmt;

    #if defined(VHWA_VPAC_IP_REV_VPAC)
        if(mscPrms->isEnableSimulProcessing == 1U)
        {
            retVal = FVID2_EINVALID_PARAMS;
        }
    #endif
    /* Check for Valid data format */
    /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end>
    <justification start> Rationale: The test framework and test apps cannot access this portion.
    The isDualChannel value is hardcoded to 1 in gMsc_SocInfo, and it can never be 0.
    Effect on this unit: None, as control can never reach this condition.
    <justification end> */
    if((0U == gM2mMscCommonObj.socInfo.isDualChannel) ||
       ((1U == gM2mMscCommonObj.socInfo.isDualChannel) && (0U == mscPrms->isEnableSimulProcessing)))
    {
       if ((FVID2_DF_LUMA_ONLY != inFmt->dataFormat) &&
            (FVID2_DF_CHROMA_ONLY != inFmt->dataFormat) &&
            (FVID2_DF_YUV420SP_UV != inFmt->dataFormat) &&
            (FVID2_DF_YUV420SP_VU != inFmt->dataFormat))
        {
            retVal = FVID2_EINVALID_PARAMS;
        }
    }
    /* LDRA_JUSTIFY_END */
    /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end>
    <justification start>
    Rationale: The test framework and test applications cannot reach this portion.
    With the existing code base, the input formats YUV422SP_UV, YUV422SP_VU, FVID2_DF_YUV422I_UYVY, FVID2_DF_YUV422I_YUYV,FVID2_DF_R_GBI, FVID2_DF_RGI_B, FVID2_DF_2PLANES and YUV420SP_VU cannot be covered
    because there is no sequential processing support for these formats.
    They can only run in simultaneous processing. Therefore, this code base is not used or enabled with the current kernel support.
    Effect on this unit: None; this is an unused feature, cannot be enabled, and control cannot reach here.
    <justification end> */
    else if ((1U == gM2mMscCommonObj.socInfo.isDualChannel) && (1U == mscPrms->isEnableSimulProcessing))
    {
        if ((FVID2_DF_LUMA_ONLY != inFmt->dataFormat) &&
            (FVID2_DF_CHROMA_ONLY != inFmt->dataFormat) &&
            (FVID2_DF_YUV420SP_UV != inFmt->dataFormat) &&
            (FVID2_DF_YUV420SP_VU != inFmt->dataFormat) &&
            (FVID2_DF_YUV422SP_UV != inFmt->dataFormat) &&
            (FVID2_DF_YUV422SP_VU != inFmt->dataFormat) &&
            (FVID2_DF_YUV422I_UYVY != inFmt->dataFormat) &&
            (FVID2_DF_YUV422I_YUYV != inFmt->dataFormat) &&
            (FVID2_DF_R_GBI != inFmt->dataFormat) &&
            (FVID2_DF_RGI_B != inFmt->dataFormat) &&
            (FVID2_DF_2PLANES != inFmt->dataFormat))
        {
            retVal = FVID2_EINVALID_PARAMS;
        }
    }
    /* LDRA_JUSTIFY_END */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
      retVal = FVID2_EINVALID_PARAMS;
    }
    /* LDRA_JUSTIFY_END */
    if ((inFmt->width > MAX_SUPPORTED_WIDTH_HEIGHT) ||
        (inFmt->height > MAX_SUPPORTED_WIDTH_HEIGHT))
    {
        retVal = FVID2_EINVALID_PARAMS;
    }
    /* Check for Second channel pitch */
    minPitch = Vhwa_m2mMscCalcHorzSizeInBytes(inFmt->width, inFmt->ccsFormat,
                                              inFmt->dataFormat);
    if (inFmt->pitch[0] < minPitch)
    {
        retVal = FVID2_EINVALID_PARAMS;
    }
    else
    {
        hObj->inChPrms[0].sl2Pitch = Vhwa_calcSl2Pitch(minPitch);
    }

    for (cnt = 0U; cnt < MSC_MAX_OUTPUT; cnt++)
    {
        scCfg = &mscPrms->mscCfg.scCfg[cnt];
        outFmt = &mscPrms->outFmt[cnt];

        if (UTRUE == scCfg->enable)
        {
            /* Check for Valid data format */
            /* LDRA_JUSTIFY_START
            <metric start>  branch <metric end>
            <justification start> Rationale: The test framework and test apps cannot access this portion.
            The isDualChannel value is hardcoded to 1 in gMsc_SocInfo, and it can never be 0.
            Effect on this unit: None, as control can never reach this condition.
            <justification end> */
            if((0U == gM2mMscCommonObj.socInfo.isDualChannel) ||
                ((1U == gM2mMscCommonObj.socInfo.isDualChannel) && (0U == mscPrms->isEnableSimulProcessing)))
            /* LDRA_JUSTIFY_END */
            {
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications CAN reach this portion.
                No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((FVID2_DF_LUMA_ONLY != outFmt->dataFormat) &&
                    (FVID2_DF_CHROMA_ONLY != outFmt->dataFormat) &&
                    (FVID2_DF_YUV420SP_UV != outFmt->dataFormat) &&
                    (FVID2_DF_YUV420SP_VU != outFmt->dataFormat))
                {
                    retVal = FVID2_EINVALID_PARAMS;
                }
                /* LDRA_JUSTIFY_END */
            }
            /* LDRA_JUSTIFY_START
            <metric start>  branch <metric end>
            <justification start>
            Rationale: The test framework and test applications cannot reach this portion.
            With the existing code base, the output formats YUV422SP_UV, YUV422SP_VU, and YUV420SP_VU cannot be covered
            because there is no sequential processing support for these formats. These formats can only run when mscPrms->isEnableSimulProcessing is set to 1.
            The modes LUMA_ONLY, CHROMA_ONLY, FVID2_DF_YUV420SP_VU, and FVID2_DF_YUV420SP_UV can be processed when mscPrms->isEnableSimulProcessing is 0.
            However, this scenario is already handled by the above if condition, so control will never reach this point when mscPrms->isEnableSimulProcessing is 0.
            Effect on this unit: None, as this is an unused feature that cannot be enabled, and control cannot reach this point.
            <justification end> */
            else if ((1U == gM2mMscCommonObj.socInfo.isDualChannel) && (1U == mscPrms->isEnableSimulProcessing))
            /* LDRA_JUSTIFY_END */
            {
                /* LDRA_JUSTIFY_START
                <metric start>  branch <metric end>
                <justification start>
                Rationale: The test framework and test applications cannot reach this portion.
                With the existing code base, the output formats yuv422SP_UV, YUV422SP_VU, and YUV420SP_VU cannot be covered because there is no sequential processing support for these formats.
                They can only run in simultaneous processing, where each scalar must be assigned a single-plane output format such as LUMA_ONLY, CHROMA_ONLY, R, PLANE_1, or PLANE_2.
                Therefore, this code base is not used or enabled with the current kernel support.
                Effect on this unit: None; this is an unused feature, cannot be enabled, and control cannot reach here.
                <justification end> */
                if ((FVID2_DF_LUMA_ONLY != outFmt->dataFormat) &&
                    (FVID2_DF_CHROMA_ONLY != outFmt->dataFormat) &&
                    (FVID2_DF_YUV420SP_UV != outFmt->dataFormat) &&
                    (FVID2_DF_YUV420SP_VU != outFmt->dataFormat) &&
                    (FVID2_DF_YUV422SP_UV != outFmt->dataFormat) &&
                    (FVID2_DF_YUV422SP_VU != outFmt->dataFormat) &&
                    (FVID2_DF_YUV422I_UYVY != outFmt->dataFormat) &&
                    (FVID2_DF_YUV422I_YUYV != outFmt->dataFormat) &&
                    (FVID2_DF_R_GBI != outFmt->dataFormat) &&
                    (FVID2_DF_RGI_B != outFmt->dataFormat) &&
                    (FVID2_DF_2PLANES != outFmt->dataFormat) &&
                    (FVID2_DF_PLANE_1 != outFmt->dataFormat) &&
                    (FVID2_DF_PLANE_2 != outFmt->dataFormat) &&
                    (FVID2_DF_R != outFmt->dataFormat) &&
                    (FVID2_DF_GBI != outFmt->dataFormat))
                {
                    retVal = FVID2_EINVALID_PARAMS;
                }
                /* LDRA_JUSTIFY_END */

            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. An error print statement can be added in a future release if required.
            <justification end> */
            else
            {
              /*Do Nothing*/
            }
            /* LDRA_JUSTIFY_END */
    }
        if ((uint32_t)UTRUE == scCfg->enable)
        {
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if((0U == (cnt & 0x1U)) &&
               ((FVID2_DF_YUV422I_UYVY == outFmt->dataFormat) ||
                (FVID2_DF_YUV422I_YUYV == outFmt->dataFormat)) &&
               (UTRUE == mscPrms->mscCfg.scCfg[cnt + 1U].enable))
            {
                retVal = FVID2_EINVALID_PARAMS;
            }
            /* LDRA_JUSTIFY_END */

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if(((8u == cnt) || (9u == cnt)) &&
               ((FVID2_DF_YUV422I_UYVY == outFmt->dataFormat) ||
                (FVID2_DF_YUV422I_YUYV == outFmt->dataFormat)))
            {
                retVal = FVID2_EINVALID_PARAMS;
            }
            /* LDRA_JUSTIFY_END */

            /* check for Input and output height */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (((FVID2_DF_GBI == outFmt->dataFormat) &&
                 (scCfg->outWidth > (inFmt->width * 2u))) ||
                ((FVID2_DF_GBI != outFmt->dataFormat) &&
                 (scCfg->outWidth > inFmt->width)) ||
                (scCfg->outHeight > inFmt->height))
            /* LDRA_JUSTIFY_END */
            {
                retVal = FVID2_EINVALID_PARAMS;
            }

            /* check for ROI parameters */
           /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (((FVID2_DF_GBI == outFmt->dataFormat) &&
                 ((scCfg->inRoi.cropStartX + scCfg->inRoi.cropWidth) >
                    ((inFmt->width*2u)+1U))) ||
                ((FVID2_DF_GBI != outFmt->dataFormat) &&
                 ((scCfg->inRoi.cropStartX + scCfg->inRoi.cropWidth) >
                    (inFmt->width+1U))) ||
                ((scCfg->inRoi.cropStartY + scCfg->inRoi.cropHeight) >
                    (inFmt->height+1U)))
            /* LDRA_JUSTIFY_END */
            {
                retVal = FVID2_EINVALID_PARAMS;
            }

            /* check for phase parameters */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((scCfg->filtMode == 1U) &&
               ((scCfg->horzAccInit > 4095U) ||
                (scCfg->vertAccInit > 4095U)))
            {
                retVal = FVID2_EINVALID_PARAMS;
            }
            /* LDRA_JUSTIFY_END */
            /* TODO - Check if required pich will fit into allocated SL2 memory */
            minPitch = Vhwa_m2mMscCalcHorzSizeInBytes(scCfg->outWidth,
                                                      outFmt->ccsFormat,
                                                      outFmt->dataFormat);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (outFmt->pitch[0] < minPitch)
            {
                retVal = FVID2_EINVALID_PARAMS;
            }
            /* LDRA_JUSTIFY_END */
            else
            {
                hObj->outChPrms[cnt].sl2Pitch = Vhwa_calcSl2Pitch(minPitch);
            }
        }
    }

    return (retVal);
}

static void Vhwa_mscSetLseCfg(Vhwa_M2mMscHandleObj *hObj,
                              const Vhwa_M2mMscParams *mscPrms)
{
    Vhwa_M2mMscCommonObj  *comObj = NULL;
    CSL_LseConfig         *lseCfg = NULL;
    CSL_MscConfig         *cslMscCfg = NULL;
    uint32_t               lseThrId;
    uint32_t               inIdx;
    uint32_t               cnt, itrCnt;

    /* Check for Null pointer */
    GT_assert(VhwaMscTrace, (NULL != hObj));
    GT_assert(VhwaMscTrace, (NULL != mscPrms));

    for (itrCnt = 0; itrCnt < hObj->numIter; itrCnt++)
    {
        cslMscCfg = &hObj->cslMscCfg[itrCnt];
        lseCfg = &hObj->lseCfg[itrCnt];

        /* Initialize LSE configuration with the default values */
        CSL_lseConfigInit(lseCfg);

        comObj = &gM2mMscCommonObj;
        inIdx = hObj->instId;
        lseThrId = hObj->instId;

        /* For MSC, number of input and output channels are 1 */
        lseCfg->numInCh = 1U;
        lseCfg->numOutCh = 0U;

        lseCfg->enablePsa = mscPrms->enablePsa;

        /* LSE Input channel 0 is used by MSC instance 0 and
           Input channel 1 is used by MSC instance 1 */
        for (cnt = 0u; cnt < CSL_LSE_MAX_INPUT_CH; cnt ++)
        {
            if (inIdx == cnt)
            {
                lseCfg->inChCfg[cnt].bypass = (uint32_t)UFALSE;
            }
            else
            {
                lseCfg->inChCfg[cnt].bypass = (uint32_t)UTRUE;
            }
        }

        lseCfg->inChCfg[inIdx].enable = (uint32_t)UTRUE;
        lseCfg->inChCfg[inIdx].frameWidth = cslMscCfg->inWidth;

        /* Use calculated hieght parameters */
        lseCfg->inChCfg[inIdx].frameHeight = cslMscCfg->inHeight;

        lseCfg->inChCfg[inIdx].enableAddrIncrBy2 = mscPrms->enableLineSkip;

        lseCfg->inChCfg[inIdx].ccsf =
            (Fvid2_ColorCompStorageFmt)mscPrms->inFmt.ccsFormat;
        lseCfg->inChCfg[inIdx].startOffset = 0U;
        lseCfg->inChCfg[inIdx].lineOffset =
                        hObj->inChPrms[0].sl2Pitch;

        lseCfg->inChCfg[inIdx].circBufSize =
                        comObj->sl2Prms.inSl2BuffDepth[inIdx][0];
        lseCfg->inChCfg[inIdx].bufAddr[0U] =
                    (uint32_t)comObj->sl2Prms.inSl2Addr[inIdx][0];

    #if defined VHWA_VPAC_IP_REV_VPAC
        lseCfg->inChCfg[inIdx].numInBuff = 1U;
    #elif defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
        lseCfg->inChCfg[inIdx].skipAltLine[0] = 0u;
        lseCfg->inChCfg[inIdx].skipOddLine[0] = 0u;

        if ((FVID2_DF_YUV422I_YUYV ==
                (Fvid2_DataFormat)mscPrms->inFmt.dataFormat) ||
            (FVID2_DF_YUV422I_UYVY ==
                (Fvid2_DataFormat)mscPrms->inFmt.dataFormat))
        {
            lseCfg->inChCfg[inIdx].enableYuv422I = (uint32_t)UTRUE;
            lseCfg->inChCfg[inIdx].yuv422DataFmt =
                                (Fvid2_DataFormat)mscPrms->inFmt.dataFormat;
        }
        else
        {
            lseCfg->inChCfg[inIdx].enableYuv422I = (uint32_t)UFALSE;
        }

        if ((2u == hObj->numInChUsed) ||
             (FVID2_DF_YUV422I_UYVY == mscPrms->inFmt.dataFormat) ||
             (FVID2_DF_YUV422I_YUYV == mscPrms->inFmt.dataFormat))
        {
            lseCfg->inChCfg[inIdx].numInBuff = 2U;
            lseCfg->inChCfg[inIdx].bufAddr[1U] =
                        (uint32_t)comObj->sl2Prms.inSl2Addr[inIdx][1];

            if(hObj->inChPrms[1].height == (hObj->inChPrms[0].height/2u))
            {
                lseCfg->inChCfg[inIdx].skipAltLine[1] = 1u;
                lseCfg->inChCfg[inIdx].skipOddLine[1] = 1u;
            }
            else
            {
                lseCfg->inChCfg[inIdx].skipAltLine[1] = 0u;
                lseCfg->inChCfg[inIdx].skipOddLine[1] = 0u;
            }

            if((UTRUE == mscPrms->secChPrms.enable)               ||
               (FVID2_DF_YUV422I_UYVY ==
                    (Fvid2_DataFormat)mscPrms->inFmt.dataFormat) ||
               (FVID2_DF_YUV422I_YUYV ==
                    (Fvid2_DataFormat)mscPrms->inFmt.dataFormat))
            {
                lseCfg->inChCfg[inIdx].buffConfig[0].enable = UTRUE;
                lseCfg->inChCfg[inIdx].buffConfig[0].ccsf =
                                                    hObj->inChPrms[1].ccsf;
                lseCfg->inChCfg[inIdx].buffConfig[0].width =
                                                    hObj->inChPrms[1].width;

                if((FVID2_DF_YUV422I_UYVY ==
                        (Fvid2_DataFormat)mscPrms->inFmt.dataFormat) ||
                   (FVID2_DF_YUV422I_YUYV ==
                        (Fvid2_DataFormat)mscPrms->inFmt.dataFormat))
                {
                    lseCfg->inChCfg[inIdx].buffConfig[0].skipSl2Read = UTRUE;
                }
                else
                {
                    lseCfg->inChCfg[inIdx].buffConfig[0].skipSl2Read = UFALSE;
                }
            }
        }
        else
        {
            lseCfg->inChCfg[inIdx].numInBuff = 1U;
        }
    #endif /* #elif defined VHWA_VPAC_IP_REV_VPAC3 */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */

        lseCfg->inChCfg[inIdx].enableMultiTapKnlConfig = UTRUE;

        if (MSC_TAP_SEL_5TAPS == cslMscCfg->mscCfg.tapSel)
        {
            /* Configuration for 5 TAP filter */
            lseCfg->inChCfg[inIdx].knTopPadding = 2U;
            lseCfg->inChCfg[inIdx].knLineOffset = 0U;
            lseCfg->inChCfg[inIdx].knHeight = 5U;
            lseCfg->inChCfg[inIdx].knBottomPadding = 2U;
        }
        /* LDRA_JUSTIFY_END */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else if (MSC_TAP_SEL_4TAPS == cslMscCfg->mscCfg.tapSel)
        {
            /* Configuration for 4 TAP filter, applicable for vertical
               Scaling only */
            lseCfg->inChCfg[inIdx].knTopPadding = 1U;
            lseCfg->inChCfg[inIdx].knLineOffset = 1U;
            lseCfg->inChCfg[inIdx].knHeight = 4U;
            lseCfg->inChCfg[inIdx].knBottomPadding = 2U;
        }
        else
        {
            /* Configuration for 3 TAP filter, applicable for vertical
               Scaling only */
            lseCfg->inChCfg[inIdx].knTopPadding = 0U;
            lseCfg->inChCfg[inIdx].knLineOffset = 2U;
            lseCfg->inChCfg[inIdx].knHeight = 3U;
            lseCfg->inChCfg[inIdx].knBottomPadding = 2U;
        }
        /* LDRA_JUSTIFY_END */
        for (cnt = 0; cnt < MSC_MAX_OUTPUT ; cnt ++)
        {
            if (0U != (hObj->scalarUsed & ((uint32_t)1U << cnt)))
            {
                lseCfg->outChCfg[cnt].enable = (uint32_t)UTRUE;
                lseCfg->outChCfg[cnt].ccsf =
                    (Fvid2_ColorCompStorageFmt)mscPrms->outFmt[cnt].ccsFormat;
                lseCfg->outChCfg[cnt].lineOffset =
                                hObj->outChPrms[cnt].sl2Pitch;
                lseCfg->outChCfg[cnt].circBufSize =
                                comObj->sl2Prms.outSl2BuffDepth[cnt];
                lseCfg->outChCfg[cnt].bufAddr =
                                (uint32_t)comObj->sl2Prms.outSl2Addr[cnt];

                lseCfg->outChCfg[cnt].thrId = lseThrId;

            #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
                lseCfg->outChCfg[cnt].enableRounding = UFALSE;

                if ((2u == hObj->numInChUsed) ||
                    (FVID2_DF_YUV422I_UYVY == mscPrms->inFmt.dataFormat) ||
                    (FVID2_DF_YUV422I_YUYV == mscPrms->inFmt.dataFormat))
                {
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if((FVID2_DF_LUMA_ONLY == mscPrms->outFmt[cnt].dataFormat) ||
                        (FVID2_DF_R == mscPrms->outFmt[cnt].dataFormat) ||
                        (FVID2_DF_PLANE_1 == mscPrms->outFmt[cnt].dataFormat) ||
                        (((FVID2_DF_YUV422I_UYVY ==
                                mscPrms->outFmt[cnt].dataFormat) ||
                          (FVID2_DF_YUV422I_YUYV ==
                                mscPrms->outFmt[cnt].dataFormat)) &&
                            (0u == (cnt & 0x1U))))
                    /* LDRA_JUSTIFY_END */
                    {
                        lseCfg->outChCfg[cnt].chThrId = 0u;
                    }
                    else
                    {
                        lseCfg->outChCfg[cnt].chThrId = 1u;
                    }

                    if(((FVID2_DF_YUV420SP_UV == mscPrms->inFmt.dataFormat) ||
                         (FVID2_DF_YUV420SP_VU == mscPrms->inFmt.dataFormat)) &&
                        (FVID2_DF_CHROMA_ONLY == mscPrms->outFmt[cnt].dataFormat))

                    {
                        lseCfg->outChCfg[cnt].skipAltLine = 1u;
                        lseCfg->outChCfg[cnt].skipOddLine = 1u;
                        lseCfg->outChCfg[cnt].enableYuv422Out = (uint32_t)UFALSE;
                    }
                    else if(((FVID2_DF_YUV422I_UYVY ==
                                    mscPrms->outFmt[cnt].dataFormat) ||
                             (FVID2_DF_YUV422I_YUYV ==
                                    mscPrms->outFmt[cnt].dataFormat)) &&
                            (0u == (cnt & 0x1U)))
                    {
                        lseCfg->outChCfg[cnt].skipAltLine = 0u;
                        lseCfg->outChCfg[cnt].skipOddLine = 0u;

                        lseCfg->outChCfg[cnt].enableYuv422Out = (uint32_t)UTRUE;
                        lseCfg->outChCfg[cnt].yuv422DataFmt =
                                (Fvid2_DataFormat)mscPrms->outFmt[cnt].dataFormat;
                    }
                    else
                    {
                        lseCfg->outChCfg[cnt].skipAltLine = 0u;
                        lseCfg->outChCfg[cnt].skipOddLine = 0u;
                        lseCfg->outChCfg[cnt].enableYuv422Out = (uint32_t)UFALSE;
                    }
                }
                else
                {
                    lseCfg->outChCfg[cnt].chThrId = 0u;
                    lseCfg->outChCfg[cnt].enableYuv422Out = (uint32_t)UFALSE;
                    lseCfg->outChCfg[cnt].skipAltLine = 0u;
                    lseCfg->outChCfg[cnt].skipOddLine = 0u;
                }
            #endif /* #if defined VHWA_VPAC_IP_REV_VPAC3 */

                lseCfg->numOutCh++;
                lseCfg->outChCfg[cnt].bypass = (uint32_t)UFALSE;
            }
            else
            {
                lseCfg->outChCfg[cnt].bypass = (uint32_t)UTRUE;
            }
        }
    }
}

static void Vhwa_mscSetHtsCfg(const Vhwa_M2mMscInstObj *instObj,
                              Vhwa_M2mMscHandleObj *hObj,
                              const Vhwa_M2mMscParams *mscPrms)
{
    CSL_HtsSchConfig      *htsCfg = NULL;
    Vhwa_M2mMscCommonObj  *comObj = NULL;
    uint32_t               cnt, itrCnt, chCnt;
    int32_t               status = FVID2_SOK;

    /* Check for Null pointer */
    GT_assert(VhwaMscTrace, (NULL != hObj));
    GT_assert(VhwaMscTrace, (NULL != instObj));
    GT_assert(VhwaMscTrace, (NULL != mscPrms));

    for (itrCnt = 0; itrCnt < hObj->numIter; itrCnt++)
    {
        htsCfg = &hObj->htsCfg[itrCnt];
        comObj = &gM2mMscCommonObj;

        CSL_htsSchConfigInit(htsCfg);

        if (hObj->instId == (uint32_t)VPAC_MSC_INST_ID_0)
        {
            htsCfg->schId = CSL_HTS_HWA_SCH_MSC0;
        }
        /* LDRA_JUSTIFY_START
        <metric start>statement  branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
        <justification end> */
        else if (hObj->instId == (uint32_t)VPAC_MSC_INST_ID_1)
        {
            htsCfg->schId = CSL_HTS_HWA_SCH_MSC1;
        }
        else
        {
            /* Invalid MSC instance Execution will not reach this block */
            status = FVID2_EINVALID_PARAMS;
        }
        /* LDRA_JUSTIFY_END */

        if(UTRUE == hObj->fcStatus.isFlexConnect)
        {
            htsCfg->pipeline = instObj->fcPipeline;
        }
        else
        {
            htsCfg->pipeline = instObj->pipeline;
        }


        htsCfg->enableStream = (uint32_t)UFALSE;
        htsCfg->enableWdTimer = UFALSE;
        htsCfg->isWdTimer128KMode = (uint32_t)UFALSE;

        htsCfg->enableBwLimit = UFALSE;

        /* Clear reset flag for DMA producer for HWA */
        htsCfg->dmaProdCfg[0U].bypass = (uint32_t)UTRUE;
        htsCfg->dmaProdCfg[1U].bypass = (uint32_t)UTRUE;

        if(FVID2_SOK == status)
        {
            for (chCnt = 0; chCnt < hObj->numInChUsed; chCnt++)
            {
                /* LDRA_JUSTIFY_START
                <metric start>  branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                <justification end> */
                if(UTRUE == hObj->inChPrms[chCnt].buffEnable)
                /* LDRA_JUSTIFY_END */

                {
                    htsCfg->consCfg[chCnt].enable = (uint32_t)UTRUE;
                    htsCfg->consCfg[chCnt].prodId = CSL_HTS_PROD_IDX_UDMA;

                    /* Clear reset flag for DMA producer for HWA */
                    htsCfg->dmaProdCfg[chCnt].bypass = (uint32_t)UFALSE;

                    htsCfg->dmaProdCfg[chCnt].enable = (uint32_t)UTRUE;
                    htsCfg->dmaProdCfg[chCnt].dmaChNum =
                            Udma_chGetNum(comObj->inChHandle[hObj->instId][chCnt]);

                    htsCfg->dmaProdCfg[chCnt].pipeline = htsCfg->pipeline;
                    htsCfg->dmaProdCfg[chCnt].consId = CSL_HTS_CONS_IDX_UDMA;

                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if (MSC_TAP_SEL_5TAPS == mscPrms->mscCfg.tapSel)
                    /* LDRA_JUSTIFY_END */
                    {
                        /* Configuration for 5 TAP filter */
                        htsCfg->dmaProdCfg[chCnt].threshold = 5U;
                        htsCfg->dmaProdCfg[chCnt].cntPreLoad = 2U;
                        htsCfg->dmaProdCfg[chCnt].cntPostLoad = 2U;
                    }
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    else if (MSC_TAP_SEL_4TAPS == mscPrms->mscCfg.tapSel)
                    {
                        /* Configuration for 4 TAP filter, applicable for vertical
                        Scaling only */
                        htsCfg->dmaProdCfg[chCnt].threshold = 4U;
                        htsCfg->dmaProdCfg[chCnt].cntPreLoad = 1U;
                        htsCfg->dmaProdCfg[chCnt].cntPostLoad = 2U;
                    }
                    else
                    {
                        /* Configuration for 3 TAP filter, applicable for vertical
                        Scaling only */
                        htsCfg->dmaProdCfg[chCnt].threshold = 3U;
                        htsCfg->dmaProdCfg[chCnt].cntPreLoad = 0U;
                        htsCfg->dmaProdCfg[chCnt].cntPostLoad = 2U;
                    }
                    /* LDRA_JUSTIFY_END */
                    htsCfg->dmaProdCfg[chCnt].enableHop = (uint32_t)UTRUE;

                    /* Use Calculated height */
                    /* LDRA_JUSTIFY_START
                    <metric start>  branch <metric end>
                    <justification start>
                    Rationale: The test framework and test apps cannot access this portion.
                    The isDualChannel value is hardcoded to 1 in gMsc_SocInfo, and it can never be 0.
                    Effect on this unit: None, as control can never reach this condition.
                    <justification end> */
                    if ((1U == gM2mMscCommonObj.socInfo.isDualChannel) && (1U == mscPrms->isEnableSimulProcessing))
                    /* LDRA_JUSTIFY_END */
                    {
                        htsCfg->dmaProdCfg[chCnt].numHop =
                                                    hObj->inChPrms[chCnt].height;
                        /* LDRA_JUSTIFY_START
                        <metric start> branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if(UTRUE ==  htsCfg->enableHop)
                        /* LDRA_JUSTIFY_END */
                        {
                            htsCfg->numHop = hObj->inChPrms[chCnt].height;
                        }
                    }
                    else
                    {
                        htsCfg->dmaProdCfg[chCnt].numHop =
                                                    hObj->cslMscCfg[itrCnt].inHeight;
                        /* LDRA_JUSTIFY_START
                        <metric start> branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if(UTRUE ==  htsCfg->enableHop)
                        /* LDRA_JUSTIFY_END */
                        {
                            htsCfg->numHop = hObj->cslMscCfg[itrCnt].inHeight;
                        }
                    }
                    /* LDRA_JUSTIFY_START
                    <metric start>  statement branch <metric end>
                    <justification start> Rationale: The test framework and test applications cannot access this portion.
                    The enableLineSkip feature skips lines in the input to improve performance, but this comes at the cost of output quality, so it is disabled by default.
                    Effect on this unit: If the control reaches here, the code base is expected to set countDec value accordingly.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if(UTRUE == mscPrms->enableLineSkip)
                    {
                        htsCfg->dmaProdCfg[chCnt].countDec = 2U;
                    }
                    /* LDRA_JUSTIFY_END */
                    else
                    {
                        htsCfg->dmaProdCfg[chCnt].countDec = 1U;
                    }

                    htsCfg->dmaProdCfg[chCnt].depth =
                                        comObj->sl2Prms.inSl2BuffDepth[hObj->instId][0];

                    /* Keep DMA producer stream enable UFALSE as in case of OTF DMA
                    will be diabled producer */
                    htsCfg->dmaProdCfg[chCnt].enableStream = (uint32_t)UFALSE;

                    /* LDRA_JUSTIFY_START
                    <metric start>  branch <metric end>
                    <justification start> Rationale: The test framework and test apps cannot access this portion.
                    The isDualChannel value is hardcoded to 1 in gMsc_SocInfo, and it can never be 0.
                    Effect on this unit: None, as control can never reach this condition.
                    <justification end> */
                    if((1u == chCnt) &&
                        (hObj->inChPrms[1].height == (hObj->inChPrms[0].height/2u)) &&
                        ((1U == gM2mMscCommonObj.socInfo.isDualChannel) && (1U == mscPrms->isEnableSimulProcessing)))
                    /* LDRA_JUSTIFY_END */
                    {
                        htsCfg->dmaProdCfg[chCnt].paCfg.enable = (uint32_t)UTRUE;
                        htsCfg->dmaProdCfg[chCnt].paCfg.enableBufCtrl = 0U;

                        htsCfg->dmaProdCfg[chCnt].paCfg.psMaxCnt = 2u;
                        htsCfg->dmaProdCfg[chCnt].paCfg.csMaxCnt =
                                                htsCfg->dmaProdCfg[chCnt].threshold;
                        htsCfg->dmaProdCfg[chCnt].paCfg.enableDecCtrl = 1U;
                        htsCfg->dmaProdCfg[chCnt].paCfg.countDec = 1U;
                    }
                }
            }
        }

        for (cnt = 0; cnt < MSC_MAX_OUTPUT; cnt++)
        {
            if (UTRUE == hObj->outChPrms[cnt].buffEnable)
            {
                htsCfg->prodCfg[cnt].enable = (uint32_t)UTRUE;
                htsCfg->prodCfg[cnt].consId = CSL_HTS_CONS_IDX_UDMA;

                htsCfg->prodCfg[cnt].threshold = 1u;
                htsCfg->prodCfg[cnt].cntPreLoad = 0u;
                htsCfg->prodCfg[cnt].cntPostLoad = 0u;

                htsCfg->prodCfg[cnt].depth =
                            comObj->sl2Prms.outSl2BuffDepth[cnt];
                htsCfg->prodCfg[cnt].countDec  =1U;

                htsCfg->dmaConsCfg[cnt].enable = (uint32_t)UTRUE;
                htsCfg->dmaConsCfg[cnt].dmaChNum =
                                Udma_chGetNum(comObj->outChHandle[cnt]);
                htsCfg->dmaConsCfg[cnt].pipeline = htsCfg->pipeline;
                htsCfg->dmaConsCfg[cnt].enableStream = (uint32_t)UFALSE;
                htsCfg->dmaConsCfg[cnt].prodId = CSL_HTS_PROD_IDX_UDMA;

                htsCfg->dmaConsCfg[cnt].bypass = (uint32_t)UFALSE;
            }
            else
            {
                htsCfg->dmaConsCfg[cnt].bypass = (uint32_t)UTRUE;
            }
        }
        htsCfg->dmaConsCfg[10].bypass = (uint32_t)UTRUE;
    }
}

static void Vhwa_mscUpdateYuv422iCslPrms(Vhwa_M2mMscHandleObj *hObj,
                                         CSL_MscConfig *cslMscCfg)
{
    uint32_t cnt;
    Vhwa_M2mMscParams *mscPrms;

    mscPrms = &hObj->mscPrms;

    for (cnt = 0; cnt < MSC_MAX_OUTPUT; cnt += 2u)
    {
        if((UTRUE == mscPrms->mscCfg.scCfg[cnt].enable) &&
           ((FVID2_DF_YUV422I_YUYV == mscPrms->outFmt[cnt].dataFormat) ||
           (FVID2_DF_YUV422I_UYVY == mscPrms->outFmt[cnt].dataFormat)))
        {
            Fvid2Utils_memcpy(&cslMscCfg->mscCfg.scCfg[cnt+1U],
                              &cslMscCfg->mscCfg.scCfg[cnt],
                              sizeof(Msc_ScConfig));

            Fvid2Utils_memcpy(&mscPrms->outFmt[cnt+1U],
                              &mscPrms->outFmt[cnt],
                              sizeof(Fvid2_Format));

            cslMscCfg->mscCfg.scCfg[cnt].isInterleaveFormat = UFALSE;
            cslMscCfg->mscCfg.scCfg[cnt+1U].isInterleaveFormat = UTRUE;

            hObj->scalarUsed |= ((uint32_t)1U << (cnt + 1U));
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
        <justification end> */
        if(FVID2_DF_CHROMA_ONLY == mscPrms->outFmt[cnt].dataFormat)
        {
            cslMscCfg->mscCfg.scCfg[cnt].isInterleaveFormat = UTRUE;
        }
        /* LDRA_JUSTIFY_END */
    }

}

static void Vhwa_mscCalPrms(Vhwa_M2mMscHandleObj *hObj,
                            const Vhwa_M2mMscParams *mscPrms)
{
    uint32_t       itrCnt, cnt;
    uint32_t       pitch;
    CSL_MscConfig *cslMscCfg = NULL;

    /* Check for Null pointer */
    GT_assert(VhwaMscTrace, (NULL != hObj));
    GT_assert(VhwaMscTrace, (NULL != mscPrms));

    if ((FVID2_DF_YUV420SP_UV == mscPrms->inFmt.dataFormat) ||
        (FVID2_DF_YUV420SP_VU == mscPrms->inFmt.dataFormat))
    {
        /* LDRA_JUSTIFY_START
        <metric start>  branch <metric end>
        <justification start> Rationale: The test framework and test apps cannot access this portion.
        The isDualChannel value is hardcoded to 1 in gMsc_SocInfo, and it can never be 0.
        Effect on this unit: None, as control can never reach this condition.
        <justification end> */
        if((0U == gM2mMscCommonObj.socInfo.isDualChannel) ||
            ((1U == gM2mMscCommonObj.socInfo.isDualChannel) && (0U == mscPrms->isEnableSimulProcessing)))
            /* LDRA_JUSTIFY_END */
        {
            /* Single channel per MSC */
            hObj->numIter = 2U;
            hObj->numInChUsed = 1u;
            hObj->inChPrms[0].buffEnable = UTRUE;
            hObj->inChPrms[1].buffEnable = UFALSE;
        }
        else
        {
            /* 2 cahnnels for LUMA and Chroma needs to be processed */
            hObj->numIter = 1U;
            hObj->numInChUsed = 2u;
            hObj->inChPrms[0].buffEnable = UTRUE;
            hObj->inChPrms[1].buffEnable = UTRUE;
        }
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
    <justification end> */
    else if ((FVID2_DF_YUV422SP_UV == mscPrms->inFmt.dataFormat) ||
             (FVID2_DF_YUV422SP_VU == mscPrms->inFmt.dataFormat) ||
             (FVID2_DF_R_GBI == mscPrms->inFmt.dataFormat) ||
             (FVID2_DF_2PLANES == mscPrms->inFmt.dataFormat))
    /* LDRA_JUSTIFY_END */
    {
        hObj->numIter = 1U;
        hObj->numInChUsed = 2u;
        hObj->inChPrms[0].buffEnable = UTRUE;
        hObj->inChPrms[1].buffEnable = UTRUE;
    }
    else if ((FVID2_DF_YUV422I_YUYV == mscPrms->inFmt.dataFormat) ||
             (FVID2_DF_YUV422I_UYVY == mscPrms->inFmt.dataFormat))
    {
        hObj->numIter = 1U;
        hObj->numInChUsed = 1u;
        hObj->inChPrms[0].buffEnable = UTRUE;
        hObj->inChPrms[1].buffEnable = UFALSE;
    }
    else
    {
        /* Only Chroma or Luma needs to be processed */
        hObj->numIter = 1U;
        hObj->numInChUsed = 1u;
        hObj->inChPrms[0].buffEnable = UTRUE;
        hObj->inChPrms[1].buffEnable = UFALSE;
    }

    hObj->scalarUsed = 0U;

    /* Update the Required Scalar Parameter */
    for (cnt = 0U; cnt < MSC_MAX_OUTPUT; cnt++)
    {
        if (mscPrms->mscCfg.scCfg[cnt].enable == UTRUE)
        {
            hObj->scalarUsed |= ((uint32_t)1U << cnt);
            hObj->outChPrms[cnt].buffEnable = UTRUE;

            gM2mMscCommonObj.mscProdStatus |= ((uint32_t)1U << cnt);
        }
        else
        {
            hObj->outChPrms[cnt].buffEnable = UFALSE;
        }
    }

    for (itrCnt = 0; itrCnt < hObj->numIter; itrCnt++)
    {
        cslMscCfg = &hObj->cslMscCfg[itrCnt];

        Fvid2Utils_memcpy(&cslMscCfg->mscCfg, &mscPrms->mscCfg, sizeof(Msc_Config));

        cslMscCfg->inWidth = mscPrms->inFmt.width;

        hObj->inChPrms[0].width = mscPrms->inFmt.width;
        hObj->inChPrms[0].ccsf = mscPrms->inFmt.ccsFormat;
        hObj->inChPrms[0].height = mscPrms->inFmt.height;

        /* LDRA_JUSTIFY_START
        <metric start>  branch <metric end>
        <justification start>
        Rationale: The test framework and test apps cannot access this portion.
        The isDualChannel value is hardcoded to 1 in gMsc_SocInfo, and it can never be 0.
        Effect on this unit: None, as control can never reach this condition.
        <justification end> */
        if ((1u == gM2mMscCommonObj.socInfo.isDualChannel) && (1U == mscPrms->isEnableSimulProcessing))
        /* LDRA_JUSTIFY_END */
        {
            cslMscCfg->inHeight =  mscPrms->inFmt.height;
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_DF_R_GBI == mscPrms->inFmt.dataFormat)
            {
                hObj->inChPrms[1].width = (mscPrms->inFmt.width * 2u);
            }
            /* LDRA_JUSTIFY_END */
            else
            {
                hObj->inChPrms[1].width = mscPrms->inFmt.width;
            }

            if(UTRUE == mscPrms->secChPrms.enable)
            {
                hObj->inChPrms[1].ccsf = mscPrms->secChPrms.ccsf;
            }
            else
            {
                hObj->inChPrms[1].ccsf = mscPrms->inFmt.ccsFormat;
            }

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
            <justification end> */
            if((FVID2_DF_YUV422SP_UV == mscPrms->inFmt.dataFormat) ||
               (FVID2_DF_YUV422SP_VU == mscPrms->inFmt.dataFormat) ||
               (FVID2_DF_R_GBI == mscPrms->inFmt.dataFormat) ||
               (FVID2_DF_2PLANES == mscPrms->inFmt.dataFormat))
            /* LDRA_JUSTIFY_END */
            {
                hObj->inChPrms[1].height = mscPrms->inFmt.height;
            }
            else if ((FVID2_DF_YUV422I_YUYV == mscPrms->inFmt.dataFormat) ||
                     (FVID2_DF_YUV422I_UYVY == mscPrms->inFmt.dataFormat))
            {
                hObj->inChPrms[1].height = mscPrms->inFmt.height;

                Vhwa_mscUpdateYuv422iCslPrms(hObj, cslMscCfg);

            }
            /* LDRA_JUSTIFY_START
            <metric start>  branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
            <justification end> */
            else if((FVID2_DF_YUV420SP_UV == mscPrms->inFmt.dataFormat) ||
                    (FVID2_DF_YUV420SP_VU == mscPrms->inFmt.dataFormat))
            {
                hObj->inChPrms[1].height = mscPrms->inFmt.height/2u;
            }
            /* LDRA_JUSTIFY_END */
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
              /*Do Nothing*/
            }
            /* LDRA_JUSTIFY_END */

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
            <justification end> */
            if((FVID2_DF_YUV422SP_UV == mscPrms->inFmt.dataFormat) ||
               (FVID2_DF_YUV422SP_VU == mscPrms->inFmt.dataFormat) ||
               (FVID2_DF_YUV420SP_UV == mscPrms->inFmt.dataFormat) ||
               (FVID2_DF_YUV420SP_VU == mscPrms->inFmt.dataFormat) ||
               (FVID2_DF_R_GBI == mscPrms->inFmt.dataFormat) ||
               (FVID2_DF_CHROMA_ONLY == mscPrms->inFmt.dataFormat))
            /* LDRA_JUSTIFY_END */
            {
                for (cnt = 0; cnt < MSC_MAX_OUTPUT; cnt++)
                {
                    if((FVID2_DF_CHROMA_ONLY == mscPrms->outFmt[cnt].dataFormat) ||
                        (FVID2_DF_GBI == mscPrms->outFmt[cnt].dataFormat))
                    {
                        cslMscCfg->mscCfg.scCfg[cnt].isInterleaveFormat = UTRUE;
                    }
                    else
                    {
                        cslMscCfg->mscCfg.scCfg[cnt].isInterleaveFormat = UFALSE;
                    }
                }
            }
        }
        else if (((FVID2_DF_YUV420SP_UV == mscPrms->inFmt.dataFormat) ||
                  (FVID2_DF_YUV420SP_VU == mscPrms->inFmt.dataFormat)) &&
                 (itrCnt == 1U))
        {
            /* Height will be 1/2 for Chroma */
            cslMscCfg->inHeight =  mscPrms->inFmt.height/2U;
            /* This is used by HTS to set the chroma channel height */
            hObj->inChPrms[1].height = mscPrms->inFmt.height/2u;

            for (cnt = 0; cnt < MSC_MAX_OUTPUT; cnt++)
            {
                cslMscCfg->mscCfg.scCfg[cnt].outHeight =
                        cslMscCfg->mscCfg.scCfg[cnt].outHeight/2U;
                cslMscCfg->mscCfg.scCfg[cnt].inRoi.cropStartY =
                        cslMscCfg->mscCfg.scCfg[cnt].inRoi.cropStartY/2U;
                cslMscCfg->mscCfg.scCfg[cnt].inRoi.cropHeight =
                        cslMscCfg->mscCfg.scCfg[cnt].inRoi.cropHeight/2U;

                cslMscCfg->mscCfg.scCfg[cnt].isInterleaveFormat = UTRUE;

            }
            (void)CSL_mscUpdateUVPrms(cslMscCfg);
        }
        else
        {
            for (cnt = 0; cnt < MSC_MAX_OUTPUT; cnt++)
            {
                if(FVID2_DF_CHROMA_ONLY == mscPrms->outFmt[cnt].dataFormat)
                {
                    cslMscCfg->mscCfg.scCfg[cnt].isInterleaveFormat = UTRUE;
                }
                else
                {
                    cslMscCfg->mscCfg.scCfg[cnt].isInterleaveFormat = UFALSE;
                }

            }
            cslMscCfg->inHeight =  mscPrms->inFmt.height;
        }

        pitch = Vhwa_m2mMscCalcHorzSizeInBytes(
                                                hObj->inChPrms[1].width,
                                                hObj->inChPrms[1].ccsf,
                                                mscPrms->inFmt.dataFormat);

        hObj->inChPrms[1].sl2Pitch = Vhwa_calcSl2Pitch(pitch);

        if(hObj->inChPrms[1].sl2Pitch > hObj->inChPrms[0].sl2Pitch)
        {
            hObj->inChPrms[0].sl2Pitch = hObj->inChPrms[1].sl2Pitch;
        }
        else
        {
            hObj->inChPrms[1].sl2Pitch = hObj->inChPrms[0].sl2Pitch;
        }

    }
}
/* LDRA_JUSTIFY_START
<metric start>statement branch <metric end>
<justification start>  Rationale:
The test framework and test apps cannot reach this portion
The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
Effect on this unit:
None;Unused feature, cannot be enabled, control cannot reach here.
<justification end> */
static void Vhwa_mscSetHtsLimit(Vhwa_M2mMscHandleObj *hObj,
                               const Vhwa_HtsLimiter *htsLimit)
{
    uint32_t          cnt;
    CSL_HtsSchConfig *htsCfg = NULL;

    /* Check for Null pointer */
    GT_assert(VhwaMscTrace, (NULL != hObj));
    GT_assert(VhwaMscTrace, (NULL != htsLimit));

    for (cnt = 0; cnt < hObj->numIter; cnt++)
    {
        htsCfg = &hObj->htsCfg[cnt];

        /* Configure  HTS BW limiter */
        htsCfg->enableBwLimit = htsLimit->enableBwLimit;
        htsCfg->cycleCnt      = htsLimit->cycleCnt;
        htsCfg->tokenCnt      = htsLimit->tokenCnt;
    }
}
/* LDRA_JUSTIFY_END */


int32_t Vhwa_m2mMscSetConfigInHW(const Vhwa_M2mMscQueueObj *qObj,
                                Vhwa_M2mMscCommonObj *comObj,
                                uint32_t intCnt)
{
    int32_t               status = FVID2_SOK;
    Vhwa_M2mMscHandleObj *hObj = NULL;

    /* Null pointer check */
    GT_assert(VhwaMscTrace, (NULL != qObj));
    GT_assert(VhwaMscTrace, (NULL != comObj));
    GT_assert(VhwaMscTrace, (NULL != qObj->hObj));

    hObj = qObj->hObj;
    VhwaVpacMscSocReadBack *goldenRegVal = hObj->configRegMemPrms.configGoldenRegPtr;

    /* Configure MSC Core */
    status = CSL_mscSetConfig(comObj->socInfo.mscRegs,
                &hObj->cslMscCfg[intCnt]);

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer. However, due to the stated rationale, this is not tested.
    <justification end> */
    /* Update golden memory if config register validation is enabled */
    if ((FVID2_SOK == status) && (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate))
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != goldenRegVal)
        /* LDRA_JUSTIFY_END */
        {
            (void)CSL_mscSetConfig(&goldenRegVal->mscRegs, &hObj->cslMscCfg[intCnt]);
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
        /* Configure HWA Common Wrapper LSE */
        status = CSL_lseSetConfig(comObj->socInfo.lseRegs, &hObj->lseCfg[intCnt]);

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer. However, due to the stated rationale, this is not tested.
        <justification end> */
        /* Update golden memory if config register validation is enabled */
        if ((FVID2_SOK == status) && (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate))
        /* LDRA_JUSTIFY_END */
        {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL != goldenRegVal)
            /* LDRA_JUSTIFY_END */
            {
                (void)CSL_lseSetConfig(&goldenRegVal->lseRegs, &hObj->lseCfg[intCnt]);
            }
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
        /* Configure HWA Common Wrapper HTS */
        status = CSL_htsSetThreadConfig(comObj->socInfo.htsRegs, &hObj->htsCfg[intCnt]);
    }

    return (status);
}

int32_t Vhwa_m2mMscSetFrameSize(const Vhwa_M2mMscQueueObj *qObj,
                                Vhwa_M2mMscCommonObj *comObj,
                                uint32_t intCnt)
{
    int32_t               status = FVID2_SOK;
    Vhwa_M2mMscHandleObj *hObj = NULL;

    /* Null pointer check */
    GT_assert(VhwaMscTrace, (NULL != qObj));
    GT_assert(VhwaMscTrace, (NULL != comObj));
    GT_assert(VhwaMscTrace, (NULL != qObj->hObj));

    hObj = qObj->hObj;
    VhwaVpacMscSocReadBack *goldenRegVal = hObj->configRegMemPrms.configGoldenRegPtr;

    /* Configure MSC Core */
    status = CSL_mscSetFrameSize(comObj->socInfo.mscRegs,
                &hObj->cslMscCfg[intCnt]);

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer. However, due to the stated rationale, this is not tested.
    <justification end> */
    /* Update golden memory if config register validation is enabled */
    if ((FVID2_SOK == status) && (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate))
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != goldenRegVal)
        /* LDRA_JUSTIFY_END */
        {
            (void)CSL_mscSetFrameSize(&goldenRegVal->mscRegs, &hObj->cslMscCfg[intCnt]);
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
        /* Configure HWA Common Wrapper LSE */
        status = CSL_lseSetUpdateConfig(comObj->socInfo.lseRegs,
            &hObj->lseCfg[intCnt]);

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer. However, due to the stated rationale, this is not tested.
        <justification end> */
        /* Update golden memory if config register validation is enabled */
        if ((FVID2_SOK == status) && (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate))
        /* LDRA_JUSTIFY_END */
        {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL != goldenRegVal)
            /* LDRA_JUSTIFY_END */
            {
                (void)CSL_lseSetUpdateConfig(&goldenRegVal->lseRegs, &hObj->lseCfg[intCnt]);
            }
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
        /* Configure HWA Common Wrapper HTS */
        status = CSL_htsSetThreadUpdateConfig(comObj->socInfo.htsRegs,
            &hObj->htsCfg[intCnt]);
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
        status = CSL_htsThreadStart(comObj->socInfo.htsRegs, &hObj->htsCfg[intCnt]);
    }


    return (status);
}

int32_t Vhwa_m2mMscSubmitRequest(Vhwa_M2mMscInstObj *instObj,
                                 Vhwa_M2mMscQueueObj *qObj,
                                 Vhwa_M2mMscCommonObj *comObj,
                                 uint32_t intCnt)
{
    int32_t               retVal = FVID2_SOK;
    Vhwa_M2mMscHandleObj *hObj = NULL;

    GT_assert(VhwaMscTrace, (NULL != instObj));
    GT_assert(VhwaMscTrace, (NULL != qObj));
    GT_assert(VhwaMscTrace, (NULL != comObj));
    GT_assert(VhwaMscTrace, (NULL != qObj->hObj));

    hObj = qObj->hObj;
    VhwaVpacMscSocReadBack *goldenRegVal = hObj->configRegMemPrms.configGoldenRegPtr;

    /* Submit Rings to the Ring Accelerator */
    retVal = Vhwa_mscM2mSubmitRing(instObj, hObj, comObj, intCnt);
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
        /* Start Channels in LSE */
        CSL_lseStartChannels(comObj->socInfo.lseRegs, &hObj->lseCfg[intCnt]);

        /* Update golden memory if config register validation is enabled */
        /* CSL_lseStartChannels writes BUF_BA registers - validate per DOF pattern */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate)
        /* LDRA_JUSTIFY_END */
        {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL != goldenRegVal)
            /* LDRA_JUSTIFY_END */
            {
                CSL_lseStartChannels(&goldenRegVal->lseRegs, &hObj->lseCfg[intCnt]);
            }
        }

        /* Update the MscConfigRegisterGroup with config register values for frame specific Static Config, INTD and HTS registers */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
#if !defined(VHWA_VPAC_IP_REV_VPAC3L)
        if ((VHWA_SAFETY_MODE_DISABLED != hObj->enableConfigRegValidate) && (goldenRegVal != NULL))
        /* LDRA_JUSTIFY_END */
        {
            retVal = vhwaM2mMscUpdateConfigRegGroup(goldenRegVal, hObj, comObj);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK != retVal)
            {
                GT_0trace(VhwaMscTrace, GT_ERR, "Failed to set default goldenReg mem values!\n");
                retVal = FVID2_EFAIL;
            }
            /* LDRA_JUSTIFY_END */
        }
#endif
        /* Start Channels in HTS */
        if (FVID2_SOK == retVal)
        {
            retVal = CSL_htsThreadStart(comObj->socInfo.htsRegs, &hObj->htsCfg[intCnt]);
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
    if (FVID2_SOK == retVal)
    /* LDRA_JUSTIFY_END */

    {
        /* Better to set Active object to this q object, so that if
             * interrupt comes immediately, activeQObj would be set..
             * If pipeline start fails, it would be set to NULL.*/
        instObj->activeQObj = qObj;

        instObj->totalReqCnt ++;

        #ifndef VHWA_USE_PIPELINE_COMMON_ENABLE
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
            hObj->perf[intCnt] = hObj->createPrms.getTimeStamp();
        }
        /* LDRA_JUSTIFY_END */
        if (UFALSE == hObj->fcStatus.isFlexConnect)
        {
            /* Start HTS pipeline */
            retVal = CSL_htsPipelineStart(comObj->socInfo.htsRegs,
                                          &hObj->htsCfg[intCnt]);
        }
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
            instObj->activeQObj = NULL;
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

void Vhwa_m2mMscUpdatePipeline(uint32_t instId, uint32_t htsPipeline)
{
    Vhwa_M2mMscInstObj *instObj = NULL;
    Vhwa_M2mMscCommonObj   *comObj = NULL;

    comObj = &gM2mMscCommonObj;


    instObj = &gM2mMscInstObj[instId];

    /* Disable HTS Interrupt */
    Vhwa_disableHtsIntr(comObj->socInfo.vpacIntdRegs,
                comObj->mscInitPrms.irqInfo[instId].vhwaIrqNum,
                instObj->pipeline);

    /* Set HTS pipeline */
    instObj->fcPipeline = htsPipeline;

    /* Enable pipeline Interrupt */
    Vhwa_enableHtsIntr(comObj->socInfo.vpacIntdRegs,
                comObj->mscInitPrms.irqInfo[instId].vhwaIrqNum,
                instObj->pipeline);

    Vhwa_enableHtsIntr(comObj->socInfo.vpacIntdRegs,
                comObj->mscInitPrms.irqInfo[instId].vhwaIrqNum,
                instObj->fcPipeline);
}

static void Vhwa_m2mMscFcGetPrms(Vhwa_M2mMscHandleObj *hObj,
                                Vhwa_M2mMscFcGetPrms *fcPrms)
{
    fcPrms->htsCfg = &hObj->htsCfg[0];
    fcPrms->inSl2Pitch[0] = hObj->inChPrms[0].sl2Pitch;
#if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
    fcPrms->inSl2Pitch[1] = hObj->inChPrms[1].sl2Pitch;
#endif /* #if defined VHWA_VPAC_IP_REV_VPAC3 */
}

/* LDRA_JUSTIFY_START
<metric start>statement branch <metric end>
<justification start>
Rationale:
The test framework and test apps cannot reach this portion
The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
Effect on this unit:
None;Unused feature, cannot be enabled, control cannot reach here.
<justification end> */

static void Vhwa_m2mMscUpdateFcConnPrms(Vhwa_M2mMscHandleObj *hObj,
                                const Vhwa_M2mMscFcConPrms *sl2FcPrms)
{
    uint8_t              *pTrMem = NULL;
    uint16_t             height;
    CSL_UdmapTR9         *pTr9;

    hObj->htsCfg[0].prodCfg[sl2FcPrms->ctrlIdx].enable = UTRUE;
    hObj->htsCfg[0].prodCfg[sl2FcPrms->ctrlIdx].consId = sl2FcPrms->htsConsId;
    hObj->htsCfg[0].prodCfg[sl2FcPrms->ctrlIdx].depth = sl2FcPrms->sl2Depth;

    hObj->htsCfg[0].prodCfg[sl2FcPrms->ctrlIdx].threshold =
                                            sl2FcPrms->htsThreshold;
    hObj->htsCfg[0].prodCfg[sl2FcPrms->ctrlIdx].cntPreLoad =
                                            sl2FcPrms->htsDmaProdPreLoad;
    hObj->htsCfg[0].prodCfg[sl2FcPrms->ctrlIdx].cntPostLoad =
                                            sl2FcPrms->htsDmaProdPostLoad;
    hObj->htsCfg[0].prodCfg[sl2FcPrms->ctrlIdx].countDec =
                                            sl2FcPrms->htsDmaProdCntDec;

    hObj->htsCfg[0].prodCfg[sl2FcPrms->ctrlIdx].isMaskSelect =
                                            sl2FcPrms->enableMaskSel;
    hObj->htsCfg[0].prodCfg[sl2FcPrms->ctrlIdx].maskSelect = sl2FcPrms->maskSel;

    hObj->outChPrms[sl2FcPrms->outputIdx].sl2Pitch = sl2FcPrms->sl2Pitch;

    hObj->htsCfg[0].prodCfg[sl2FcPrms->outputIdx].depth = sl2FcPrms->sl2Depth;
    hObj->lseCfg[0].outChCfg[sl2FcPrms->outputIdx].bufAddr = (uint32_t)sl2FcPrms->sl2Addr;
    hObj->lseCfg[0].outChCfg[sl2FcPrms->outputIdx].circBufSize = sl2FcPrms->sl2Depth;

    height = (uint16_t)hObj->cslMscCfg[0].mscCfg.scCfg[sl2FcPrms->outputIdx].outHeight;

    pTrMem = hObj->outChPrms[sl2FcPrms->outputIdx].trMem[0];
    pTr9 = (CSL_UdmapTR9 *)((uint32_t)pTrMem + sizeof(CSL_UdmapTR15));
    VhwaAl_Cache_Inv(pTrMem, VHWA_MSC_UDMA_TRPD_SIZE, CacheP_TYPE_L1D);

    pTr9->addr    = sl2FcPrms->sl2Addr;
    pTr9->icnt1   = (uint16_t)sl2FcPrms->sl2Depth;
    pTr9->icnt2   = (uint16_t)(height/sl2FcPrms->sl2Depth);

    if ((pTr9->icnt2 * pTr9->icnt1) != height)
    {
        pTr9->icnt2 ++;
    }

    VhwaAl_Cache_Wb(pTrMem, VHWA_MSC_UDMA_TRPD_SIZE, CacheP_TYPE_L1D);

}
/* LDRA_JUSTIFY_END */


static void Vhwa_m2mMscUpdateFcPrms(Vhwa_M2mMscHandleObj *hObj,
                                    const Vhwa_M2mMscFcUpdatePrms *fcPrms)
{
    uint32_t cnt = 0;

    for (cnt = 0; cnt < MSC_MAX_OUTPUT; cnt++)
    {
        hObj->htsCfg[0].dmaConsCfg[cnt].enable = fcPrms->outDmaEnable[cnt];
        hObj->fcStatus.outDmaEnable[cnt] = fcPrms->outDmaEnable[cnt];
    }

    /* DMA related Parameters */
    hObj->fcStatus.isFlexConnect = fcPrms->isFlexConnect;
    hObj->fcStatus.inDmaEnable[0] = fcPrms->inDmaEnable[0];
#if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L
    hObj->fcStatus.inDmaEnable[1] = fcPrms->inDmaEnable[1];
#endif /* #if defined VHWA_VPAC_IP_REV_VPAC3 */
}

static int32_t Vhwa_m2mMscCheckCreatePrms(uint32_t drvId, uint32_t drvInstId)
{
    int32_t             retVal = FVID2_SOK;
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_VHWA_M2M_MSC_DRV_ID != drvId)
    {
        GT_0trace(VhwaMscTrace, GT_ERR, "Invalid Driver ID !!\n");
        retVal = FVID2_EINVALID_PARAMS;
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
        if ((VHWA_M2M_VPAC_0_MSC_DRV_INST_ID_0 != drvInstId) &&
            (VHWA_M2M_VPAC_0_MSC_DRV_INST_ID_1 != drvInstId))
        {
            retVal = FVID2_EINVALID_PARAMS;
            GT_0trace(VhwaMscTrace, GT_ERR, "Invalid/unsupported Instance Id\n");
        }
        /* LDRA_JUSTIFY_END */


#endif
#endif

#if defined VHWA_M2M_VPAC_INSTANCE
#if (VHWA_M2M_VPAC_INSTANCE == 1)
        if ((VHWA_M2M_VPAC_1_MSC_DRV_INST_ID_0 != drvInstId) &&
            (VHWA_M2M_VPAC_1_MSC_DRV_INST_ID_1 != drvInstId))
        {
            retVal = FVID2_EINVALID_PARAMS;
            GT_0trace(VhwaMscTrace, GT_ERR, "Invalid/unsupported Instance Id\n");
        }
#endif
#endif
    }

    return (retVal);
}

void Vhwa_m2mDisableFcMscHtsIntr(void)
{
    Vhwa_M2mMscCommonObj *comObj = &gM2mMscCommonObj;
    Vhwa_M2mMscInstObj *instObj = NULL;

    /* Disable HTS Interrupt for both instances */
    instObj = &gM2mMscInstObj[0U];
    Vhwa_disableHtsIntr(comObj->socInfo.vpacIntdRegs,
                        comObj->mscInitPrms.irqInfo[0U].vhwaIrqNum,
                        instObj->fcPipeline);

    instObj = &gM2mMscInstObj[1U];
    Vhwa_disableHtsIntr(comObj->socInfo.vpacIntdRegs,
                        comObj->mscInitPrms.irqInfo[1U].vhwaIrqNum,
                        instObj->fcPipeline);
}

#if !defined(VHWA_VPAC_IP_REV_VPAC3L)
static int32_t Vhwa_m2mMscReconfigReinitReg(const Vhwa_M2mMscInstObj *instObj,
                                            Vhwa_M2mMscCommonObj *comObj,
                                            const Vhwa_M2mMscQueueObj *qObj)
{
    int32_t status = FVID2_SOK;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == instObj) || (NULL == comObj))
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        VhwaVpacMscSocReadBack *goldenRegVal = qObj->hObj->configRegMemPrms.configGoldenRegPtr;

        /* Reconfigure the MSC Reg registers as needed.
           This is a placeholder for actual reconfiguration logic */

        /* If last handle is same as current handle, reconfigure the non-static configuration Regs */
        if (instObj->lastHndlObj == qObj->hObj)
        {

            status = Vhwa_m2mMscSetConfigInHW(qObj, comObj, 0U);

        }
        /* Reinitialise the Regs that are having static configurations */
        /* Example: ENABLE register of CSL_mscRegs */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            /* Reconfigure MSC filter coefficients from stored values */
            Vhwa_M2mMscCoeffStorage *pStorage = &comObj->coeffStorage;

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((uint32_t)UTRUE == pStorage->isCoeffConfigured)
            /* LDRA_JUSTIFY_END */
            {
                Msc_Coeff coeffCfg;
                uint32_t setIdx;

                /* Prepare coefficient structure with pointers to stored data */
                Fvid2Utils_memset(&coeffCfg, 0, sizeof(Msc_Coeff));

                /* Set up pointers to single phase coefficients */
                for (setIdx = 0U; setIdx < MSC_MAX_SP_COEFF_SET; setIdx++)
                {
                    if ((uint32_t)UTRUE == pStorage->spCoeffValid[setIdx])
                    {
                        coeffCfg.spCoeffSet[setIdx] = &pStorage->spCoeff[setIdx][0];
                    }
                    else
                    {
                        coeffCfg.spCoeffSet[setIdx] = NULL;
                    }
                }

                /* Set up pointers to multi phase coefficients */
                for (setIdx = 0U; setIdx < MSC_MAX_MP_COEFF_SET; setIdx++)
                {
                    if ((uint32_t)UTRUE == pStorage->mpCoeffValid[setIdx])
                    {
                        coeffCfg.mpCoeffSet[setIdx] = &pStorage->mpCoeff[setIdx][0];
                    }
                    else
                    {
                        coeffCfg.mpCoeffSet[setIdx] = NULL;
                    }
                }

                /* Write coefficients to hardware */
                status = CSL_mscSetCoeff(comObj->socInfo.mscRegs, &coeffCfg);

                /* Update golden memory if config register validation is enabled */
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The parameters are pre-validated by the caller before the control reaches here.
                Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer. However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((FVID2_SOK == status) && (VHWA_SAFETY_MODE_DISABLED != qObj->hObj->enableConfigRegValidate))
                /* LDRA_JUSTIFY_END */
                {
                    if (NULL != goldenRegVal)
                    {
                        status = CSL_mscSetCoeff(&goldenRegVal->mscRegs, &coeffCfg);
                    }
                }
            }
            /* Enable Module */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK == status)
            /* LDRA_JUSTIFY_END */
            {
                CSL_vpacEnableModule(comObj->socInfo.vpacCntlRegs, VPAC_MODULE_MSC,
                    (uint32_t)UTRUE);

                /* Update golden memory if config register validation is enabled */
                if (VHWA_SAFETY_MODE_DISABLED != qObj->hObj->enableConfigRegValidate)
                {
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if (NULL != goldenRegVal)
                    /* LDRA_JUSTIFY_END */
                    {
                        CSL_vpacEnableModule(&goldenRegVal->vpacCntlRegs, VPAC_MODULE_MSC,
                            (uint32_t)UTRUE);
                    }
                }


                /* Enable the MSC Module */
                CSL_mscEnable(comObj->socInfo.mscRegs);

                /* Update golden memory if config register validation is enabled */
                if (VHWA_SAFETY_MODE_DISABLED != qObj->hObj->enableConfigRegValidate)
                {
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if (NULL != goldenRegVal)
                    /* LDRA_JUSTIFY_END */
                    {
                        CSL_mscEnable(&goldenRegVal->mscRegs);
                    }
                }
                Vhwa_enableHtsIntr(comObj->socInfo.vpacIntdRegs,
                    comObj->mscInitPrms.irqInfo[qObj->hObj->instId].vhwaIrqNum,
                    instObj->pipeline);
            }
        }
    }

    return status;
}

static int32_t vhwaM2mMscUpdateStatusRegGroup(Vhwa_M2mMscHandleObj *hObj)
{
    MscStatusRegisterGroup *statusRegs;
    int32_t status = FVID2_SOK;
    uint32_t expected_val = 0U;
    uint32_t i, cfgIdx;

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
        statusRegs = &hObj->statusRegs;

        /* Update the expected values for DST_BUF_CFG_CH_STATUS[MSC_MAX_OUTPUT] using mask for CSL_LSE_DST_BUF_CFG_CH_DISABLED_MASK,
        * if Yuv422 is enabled on previous LSE output, the current channel will be disabled
        */
        for (i = 1U; i < MSC_MAX_OUTPUT; i++)
        {
            expected_val = 0U;
            /* Read the previous LSE DST BUF_CFG register value */
            uint32_t reg_val = CSL_REG32_RD(&gM2mMscCommonObj.socInfo.lseRegs->DST[i - 1U].BUF_CFG);

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
            statusRegs->mscLseStatus.DST_BUF_CFG_CH_STATUS[hObj->instId][i] = expected_val;
        }

        /* Set schId and related local variables based on instId */
        uint32_t numProducers = 0, baseIdx = 0;
        #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
        uint32_t numSchedulers = 0, schedBaseIdx = 0;
        #endif
        uint32_t numDmaProducers = 0, dmaBaseIdx = 0;

        if ((uint32_t)VPAC_MSC_INST_ID_0 == hObj->instId)
        {
            numProducers    = CSL_HTS_NUM_MSC0_PRODUCERS;
            baseIdx         = 0;
            #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
            numSchedulers   = CSL_HTS_NUM_MSC0_SCHEDULERS;
            schedBaseIdx    = 0;
            #endif
            numDmaProducers = CSL_HTS_NUM_MSC0_PRODUCERS_DMA;
            dmaBaseIdx      = 0;
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        else if ((uint32_t)VPAC_MSC_INST_ID_1 == hObj->instId)
        /* LDRA_JUSTIFY_END */
        {
            numProducers    = CSL_HTS_NUM_MSC1_PRODUCERS;
            baseIdx         = CSL_HTS_NUM_MSC0_PRODUCERS;
            #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
            numSchedulers   = CSL_HTS_NUM_MSC1_SCHEDULERS;
            schedBaseIdx    = CSL_HTS_NUM_MSC0_SCHEDULERS;
            #endif
            numDmaProducers = CSL_HTS_NUM_MSC1_PRODUCERS_DMA;
            dmaBaseIdx      = CSL_HTS_NUM_MSC0_PRODUCERS_DMA;
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            status = FVID2_EBADARGS;
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (status == FVID2_SOK)
        /* LDRA_JUSTIFY_END */
        {
            /* For iterative mode (numIter > 1), use last iteration's config since validation
             * happens after all iterations complete. Hardware registers reflect last iteration.
             * For simultaneous mode (numIter == 1), use first (and only) iteration's config.
             */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
            if(hObj->numIter > 1U)
            /* LDRA_JUSTIFY_END */
            {
                cfgIdx = (hObj->numIter - 1U);
            }
            else
            {
                cfgIdx = 0U;
            }

            /* Update PROD_COUNT for all producers */
            for (i = 0; i < numProducers; i++)
            {
                CSL_HtsSchConfig *cfg = &hObj->htsCfg[cfgIdx];
                CSL_HtsProdConfig *prodCfg = &cfg->prodCfg[i];

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((prodCfg != NULL) && (prodCfg->enable == UTRUE))
                /* LDRA_JUSTIFY_END */
                {
                    expected_val = 0U;
                    CSL_FINS(expected_val, HTS_HWA4_PROD0_COUNT_COUNT_POSTLOAD, prodCfg->cntPostLoad);
                    statusRegs->mscHtsStatus.PROD_COUNT[i + baseIdx] = expected_val;
                }
                else
                {
                    statusRegs->mscHtsStatus.PROD_COUNT[i + baseIdx] = 0U;
                }
            }

            /* Update HWA_HOP for all schedulers */
            #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
            for (i = 0; i < numSchedulers; i++)
            {
                CSL_HtsSchConfig *cfg = &hObj->htsCfg[cfgIdx];

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((cfg != NULL) && (cfg->tdoneCntEnable == UTRUE))
                /* LDRA_JUSTIFY_END */
                {
                    expected_val = 0U;
                    CSL_FINS(expected_val, HTS_HWA4_HOP_TDONE_COUNT, cfg->numHop);
                    statusRegs->mscHtsStatus.HWA_HOP[i + schedBaseIdx] = expected_val;
                }
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    statusRegs->mscHtsStatus.HWA_HOP[i + schedBaseIdx] = 0U;
                }
                /* LDRA_JUSTIFY_END */
            }
            #endif

            /* Update PROD_DMA_COUNT and PROD_DMA_HOP for all DMA producers */
            for (i = 0; i < numDmaProducers; i++)
            {
                CSL_HtsSchConfig *cfg = &hObj->htsCfg[cfgIdx];
                CSL_HtsDmaProdConfig *dmaProdCfg = &cfg->dmaProdCfg[i];

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((dmaProdCfg != NULL) && (dmaProdCfg->enable == UTRUE))
                /* LDRA_JUSTIFY_END */
                {
                    expected_val = 0U;
                    CSL_FINS(expected_val, HTS_DMA32_PROD0_COUNT_COUNT_POSTLOAD, dmaProdCfg->cntPostLoad);
                    statusRegs->mscHtsStatus.PROD_DMA_COUNT[i + dmaBaseIdx] = expected_val;
                    #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
                    expected_val = 0U;
                    CSL_FINS(expected_val, HTS_DMA32_HOP_TDONE_COUNT, dmaProdCfg->numHop);
                    statusRegs->mscHtsStatus.PROD_DMA_HOP[i + dmaBaseIdx] = expected_val;
                    #endif
                }
                else
                {
                    statusRegs->mscHtsStatus.PROD_DMA_COUNT[i + dmaBaseIdx] = 0U;
                    #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
                    statusRegs->mscHtsStatus.PROD_DMA_HOP[i + dmaBaseIdx] = 0U;
                    #endif
                }
            }
        }
    }

    return status;
}

int32_t Vhwa_m2mMscStatusRegValidate(Vhwa_M2mMscHandleObj *hObj, const Msc_SocInfo *socInfo)
{
    int32_t status = FVID2_SOK;
    uint32_t i;
    uint32_t instId, cfgIdx;
    MscStatusRegisterGroup *expected = NULL;

    /* Null pointer check */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((hObj == NULL) || (socInfo == NULL))
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
    if(FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        instId = hObj->instId;
        expected = &hObj->statusRegs;
        
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if(NULL == expected)
        {
            status = FVID2_EBADARGS;
        }
        /* LDRA_JUSTIFY_END */
    }

    /* For sequential processing, use last iteration's config (validation happens after all iterations) */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if((FVID2_SOK == status) && (hObj->numIter > 1U))
    /* LDRA_JUSTIFY_END */
    {
        cfgIdx = (hObj->numIter - 1U);
    }
    else
    {
        cfgIdx = 0U;
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Validate PID values in CSL_vpacCntlRegs */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)VHWA_HW_PID_VPAC != CSL_REG32_RD(&socInfo->vpacCntlRegs->PID))
        {
            GT_0trace(VhwaMscTrace, GT_ERR, "StatusRegValidate: vpacCntlRegs->PID mismatch\n");
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */
    }

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        /* Validate PID values in CSL_mscRegs */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)VHWA_HW_PID_MSC != CSL_REG32_RD(&socInfo->mscRegs->REVISION))
        {
            GT_0trace(VhwaMscTrace, GT_ERR, "StatusRegValidate: mscRegs->PID mismatch\n");
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */
    }

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        /* Validate LSE status directly from registers using CSL_RD */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)FVID2_SOK != CSL_REG32_RD(&socInfo->lseRegs->STATUS_ERROR))
        {
            GT_0trace(VhwaMscTrace, GT_ERR, "StatusRegValidate: lseRegs->STATUS_ERROR mismatch\n");
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */
    }

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        /* Validate LSE status idle mode */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)FVID2_SOK != CSL_REG32_RD(&socInfo->lseRegs->STATUS_IDLE_MODE))
        {
            GT_0trace(VhwaMscTrace, GT_ERR, "StatusRegValidate: lseRegs->STATUS_IDLE_MODE mismatch\n");
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        /* Validate DST_BUF_CFG_CH_STATUS[MSC_MAX_OUTPUT] using mask for CSL_LSE_DST_BUF_CFG_CH_DISABLED_MASK */
        for (i = 0; i < MSC_MAX_OUTPUT; i++)
        {
            uint32_t expected_val = (expected->mscLseStatus.DST_BUF_CFG_CH_STATUS[instId][i] & CSL_LSE_DST_BUF_CFG_CH_DISABLED_MASK);
            uint32_t reg_val = (CSL_REG32_RD(&socInfo->lseRegs->DST[i].BUF_CFG) & CSL_LSE_DST_BUF_CFG_CH_DISABLED_MASK);
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
            if (expected_val != reg_val)
            {
                GT_1trace(VhwaMscTrace, GT_ERR, "StatusRegValidate: lseRegs->DST[%d].BUF_CFG mismatch\n", i);
                status = FVID2_EFAIL;
            }
            /* LDRA_JUSTIFY_END */
        }
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (status == FVID2_SOK)
    /* LDRA_JUSTIFY_END */
    {
        /* Validate HTS status directly from registers using CSL_RD */
        if (socInfo->htsRegs != NULL)
        {
            /* Choose MSC0 or MSC1 based on hObj->instId and initialize local variables */
            uint32_t numProducers, baseIdx;
            #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
            uint32_t schedBaseIdx;
            #endif
            uint32_t numDmaProducers, dmaBaseIdx, numDmaConsumers;
            const CSL_HtsHwaParams *htsPrms = NULL;

            if ((uint32_t)VPAC_MSC_INST_ID_0 == instId)
            {
                numProducers    = CSL_HTS_NUM_MSC0_PRODUCERS;
                baseIdx         = 0;
                #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
                schedBaseIdx    = 0;
                #endif
                numDmaProducers = CSL_HTS_NUM_MSC0_PRODUCERS_DMA;
                dmaBaseIdx      = 0;
                numDmaConsumers = CSL_HTS_NUM_MSC0_CONSUMERS_DMA;
                htsPrms         = GetVpacHtsParams(CSL_HTS_HWA_SCH_MSC0);
            }
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
            else if ((uint32_t)VPAC_MSC_INST_ID_1 == instId)
            /* LDRA_JUSTIFY_END */
            {
                numProducers    = CSL_HTS_NUM_MSC1_PRODUCERS;
                baseIdx         = CSL_HTS_NUM_MSC0_PRODUCERS;
                #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
                schedBaseIdx    = CSL_HTS_NUM_MSC0_SCHEDULERS;
                #endif
                numDmaProducers = CSL_HTS_NUM_MSC1_PRODUCERS_DMA;
                dmaBaseIdx      = CSL_HTS_NUM_MSC0_PRODUCERS_DMA;
                numDmaConsumers = CSL_HTS_NUM_MSC1_CONSUMERS_DMA;
                htsPrms         = GetVpacHtsParams(CSL_HTS_HWA_SCH_MSC1);
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
            else
            {
                status = FVID2_EBADARGS;
            }
            /* LDRA_JUSTIFY_END */

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
            if (status == FVID2_SOK)
            /* LDRA_JUSTIFY_END */
            {
                /* Validate HTS SCHEDULER_CONTROL and WD_TIMER for selected MSC instance */
                uint32_t scheduler_base_addr = (uint32_t)socInfo->htsRegs + htsPrms->schOffset;

                /* Validate HTS SCHEDULER_CONTROL */
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((uint32_t)FVID2_SOK !=
                    (CSL_REG32_RD(scheduler_base_addr) & CSL_HTS_HWA4_SCHEDULER_CONTROL_STATE_MASK))
                {
                    GT_0trace(VhwaMscTrace, GT_ERR, "StatusRegValidate: htsRegs->SCHEDULER_CONTROL mismatch\n");
                    status = FVID2_EFAIL;
                }
                /* LDRA_JUSTIFY_END */
                /* Validate HTS WD_TIMER status */
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                <justification end> */
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
                        (CSL_REG32_RD(scheduler_base_addr + HWA_WDTIMER_OFFSET) & CSL_HTS_HWA4_WDTIMER_WDTIMER_STATUS_MASK))
                    {
                        GT_0trace(VhwaMscTrace, GT_ERR, "StatusRegValidate: htsRegs->WDTIMER mismatch\n");
                        status = FVID2_EFAIL;
                    }
                    /* LDRA_JUSTIFY_END */
                }
                #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
                /* Validate HTS HWA_HOP for all schedulers */
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                <justification end> */
                if (status == FVID2_SOK)
                /* LDRA_JUSTIFY_END */
                {
                    /* Validate HTS HWA_HOP for selected MSC instance */
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if ((expected->mscHtsStatus.HWA_HOP[schedBaseIdx] & CSL_HTS_HWA4_HOP_TDONE_COUNT_MASK) !=
                        (CSL_REG32_RD(scheduler_base_addr + HWA_HOP_OFFSET) & CSL_HTS_HWA4_HOP_TDONE_COUNT_MASK))
                    {
                        GT_0trace(VhwaMscTrace, GT_ERR, "StatusRegValidate: htsRegs->HWA_HOP mismatch\n");
                        status = FVID2_EFAIL;
                    }
                    /* LDRA_JUSTIFY_END */
                }
                #endif
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                <justification end> */
                /* Validate HTS PROD_COUNT and PA_PRODCOUNT for all producers */
                if (status == FVID2_SOK)
                /* LDRA_JUSTIFY_END */
                {
                    uint32_t prod_count_addr;
                    CSL_HtsSchConfig *cfg = &hObj->htsCfg[cfgIdx];
                    for (i = 0; i < numProducers; i++)
                    {
                        CSL_HtsProdConfig *prodCfg = &cfg->prodCfg[i];
                        if ((prodCfg != NULL) && (prodCfg->enable == UTRUE))
                        {
                            prod_count_addr = (uint32_t)socInfo->htsRegs + htsPrms->prodOffset[i];
                            /* LDRA_JUSTIFY_START
                            <metric start> statement branch <metric end>
                            <justification start>
                            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                            <justification end> */
                            if ((expected->mscHtsStatus.PROD_COUNT[i + baseIdx] & CSL_HTS_HWA4_PROD0_COUNT_COUNT_MASK) !=
                                (CSL_REG32_RD(prod_count_addr + HWA_PROD_COUNT_OFFSET) & CSL_HTS_HWA4_PROD0_COUNT_COUNT_MASK))
                            {
                                GT_1trace(VhwaMscTrace, GT_ERR, "StatusRegValidate: htsRegs->PROD_COUNT[%d] mismatch\n", i + baseIdx);
                                status = FVID2_EFAIL;
                            }
                            /* LDRA_JUSTIFY_END */
                            #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
                            /* LDRA_JUSTIFY_START
                            <metric start> statement branch <metric end>
                            <justification start>
                            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                            <justification end> */
                            if ((uint32_t)FVID2_SOK != (CSL_REG32_RD(prod_count_addr + HWA_PROD_PA_PROD_COUNT_OFFSET) & CSL_HTS_HWA4_PA0_PRODCOUNT_PA_PSCOUNT_MASK))
                            {
                                GT_1trace(VhwaMscTrace, GT_ERR, "StatusRegValidate: htsRegs->PA_PROD_COUNT[%d] mismatch\n", i + baseIdx);
                                status = FVID2_EFAIL;
                            }
                            /* LDRA_JUSTIFY_END */
                            #endif
                        }
                    }
                }

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                <justification end> */
                /* Validate PROD_DMA_SCHEDULER_CONTROL and PROD_DMA_COUNT for DMA producers */
                if (status == FVID2_SOK)
                /* LDRA_JUSTIFY_END */
                {
                    for (i = 0; i < numDmaProducers; i++)
                    {
                        CSL_HtsSchConfig *cfg = &hObj->htsCfg[cfgIdx];
                        CSL_HtsDmaProdConfig *dmaProdCfg = &cfg->dmaProdCfg[i];

                        /* LDRA_JUSTIFY_START
                        <metric start> branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if ((dmaProdCfg != NULL) && (dmaProdCfg->enable == UTRUE))
                        /* LDRA_JUSTIFY_END */
                        {
                            uint32_t prod_dma_addr = (uint32_t)socInfo->htsRegs + htsPrms->dmaProdOffset[i];
                            /* Validate PROD_DMA_SCHEDULER_CONTROL */
                            /* LDRA_JUSTIFY_START
                            <metric start> statement branch <metric end>
                            <justification start>
                            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                            <justification end> */
                            if ((uint32_t)FVID2_SOK !=
                                (CSL_REG32_RD(prod_dma_addr) & CSL_HTS_DMA32_SCHEDULER_CONTROL_STATE_MASK))
                            {
                                GT_1trace(VhwaMscTrace, GT_ERR, "StatusRegValidate: htsRegs->PROD_DMA_SCHEDULER_CONTROL[%d] mismatch\n", i + dmaBaseIdx);
                                status = FVID2_EFAIL;
                            }
                             /* LDRA_JUSTIFY_END */
                            /* Validate PROD_DMA_COUNT */
                            /* LDRA_JUSTIFY_START
                            <metric start> statement branch <metric end>
                            <justification start>
                            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                            <justification end> */
                            if ((expected->mscHtsStatus.PROD_DMA_COUNT[i + dmaBaseIdx] & CSL_HTS_DMA32_PROD0_COUNT_COUNT_MASK) !=
                                (CSL_REG32_RD(prod_dma_addr + HWA_DMA_PROD_COUNT_OFFSET) & CSL_HTS_DMA32_PROD0_COUNT_COUNT_MASK))
                            {
                                GT_1trace(VhwaMscTrace, GT_ERR, "StatusRegValidate: htsRegs->PROD_DMA_COUNT[%d] mismatch\n", i + dmaBaseIdx);
                                status = FVID2_EFAIL;
                            }
                            /* LDRA_JUSTIFY_END */
                            #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
                            /* LDRA_JUSTIFY_START
                            <metric start> statement branch <metric end>
                            <justification start>
                            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                            <justification end> */
                            /* Validate PROD_DMA_HOP */
                            if ((expected->mscHtsStatus.PROD_DMA_HOP[i + dmaBaseIdx] & CSL_HTS_DMA32_HOP_TDONE_COUNT_MASK) !=
                                (CSL_REG32_RD(prod_dma_addr + HWA_DMA_PROD_HOP_OFFSET) & CSL_HTS_DMA32_HOP_TDONE_COUNT_MASK))
                            {
                                GT_1trace(VhwaMscTrace, GT_ERR, "StatusRegValidate: htsRegs->PROD_DMA_HOP[%d] mismatch\n", i + dmaBaseIdx);
                                status = FVID2_EFAIL;
                            }
                            /* LDRA_JUSTIFY_END */
                            /* Validate PA_PRODCOUNT_DMA */
                            /* LDRA_JUSTIFY_START
                            <metric start> statement branch <metric end>
                            <justification start>
                            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                            <justification end> */
                            if ((uint32_t)FVID2_SOK != (CSL_REG32_RD(prod_dma_addr + HWA_DMA_PROD_PA_PROD_COUNT_OFFSET) & CSL_HTS_DMA32_PA0_PRODCOUNT_PA_PSCOUNT_MASK))
                            {
                                GT_1trace(VhwaMscTrace, GT_ERR, "StatusRegValidate: htsRegs->PA_PRODCOUNT_DMA[%d] mismatch\n", i + dmaBaseIdx);
                                status = FVID2_EFAIL;
                            }
                            /* LDRA_JUSTIFY_END */
                            #endif
                        }
                    }
                }

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                <justification end> */
                /* Validate for DMA consumer scheduler control status */
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
                            (CSL_REG32_RD(cons_dma_addr) & CSL_HTS_DMA304_SCHEDULER_CONTROL_STATE_MASK))
                        {
                            GT_1trace(VhwaMscTrace, GT_ERR, "StatusRegValidate: htsRegs->CONSUMER_DMA_SCHEDULER_CONTROL[%d] mismatch\n", i);
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

/**
 * @brief Sets the default golden register memory values for the MSC (Multi-Scaler) module.
 *
 * This function initializes the register memory values to their default "golden" state
 * for the specified MSC handle and instance object. It is typically used to ensure
 * that the hardware registers are set to known, validated values before operation.
 *
 * @param hObj Pointer to the MSC handle object.
 * @param instObj Pointer to the MSC instance object.
 *
 * @return Returns 0 on success, or a negative error code on failure.
 */
int32_t vhwaM2mMscSetDefaultGoldenRegMemValues(const Vhwa_M2mMscHandleObj *hObj, const Vhwa_M2mMscCommonObj *comObj)
{
    VhwaVpacMscSocReadBack *goldenReg = NULL;
    int32_t status = FVID2_SOK;
    uint32_t expected_reset_val = 0U;
    uint32_t reg_val = 0U;

    if ((NULL == hObj) || (NULL == comObj))
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        goldenReg = hObj->configRegMemPrms.configGoldenRegPtr;
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL != goldenReg) && (FVID2_SOK == status))
    /* LDRA_JUSTIFY_END */
    {
        /* Set default values for MSC filter CFG registers if available */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (comObj->socInfo.mscRegs != NULL)
        /* LDRA_JUSTIFY_END */
        {
            expected_reset_val = 0x8U;
            CSL_FINS(reg_val, MSC_FILT_CFG_COEF_SHIFT, expected_reset_val);

            for (uint32_t i = 0U; i < MSC_MAX_OUTPUT; i++)
            {
                goldenReg->mscRegs.FILT[i].CFG = reg_val;
            }
        }

        #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
        reg_val = 0U;
        /* Set default values for DST_COMMON_CFG LSE registers if available */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (comObj->socInfo.lseRegs != NULL)
        /* LDRA_JUSTIFY_END */
        {
            expected_reset_val = 0x8U;
            CSL_FINS(reg_val, LSE_DST_COMMON_CFG_ROUNDING_OFFSET, expected_reset_val);
            goldenReg->lseRegs.DST_COMMON_CFG = reg_val;
        }
        #endif
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

static int32_t Vhwa_m2mMscConfigRegReadback(const Vhwa_M2mMscHandleObj *hObj, const Vhwa_M2mMscCommonObj *comObj)
{
    int32_t status = FVID2_SOK;
    uint32_t cfgIdx;
    CSL_vpac_cntlRegs *vpacCntlRegs = NULL;
    VhwaVpacMscSocReadBack *readback = NULL;
    CSL_mscRegs *mscRegs = NULL;
    CSL_lseRegs *lseRegs = NULL;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == comObj) || (NULL == hObj))
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        vpacCntlRegs = comObj->socInfo.vpacCntlRegs;
        readback = hObj->configRegMemPrms.configRegReadbackPtr;
        mscRegs = comObj->socInfo.mscRegs;
        lseRegs = comObj->socInfo.lseRegs;
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == readback) || (NULL == vpacCntlRegs) || (NULL == mscRegs) || (NULL == lseRegs))
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        /* For iterative mode (numIter > 1), use last iteration's config since readback
         * happens after all iterations complete. Hardware registers reflect last iteration.
         * For simultaneous mode (numIter == 1), use first (and only) iteration's config.
         */
        if(hObj->numIter > 1U)
        {
            cfgIdx = (hObj->numIter - 1U);
        }
        else
        {
            cfgIdx = 0U;
        }

        /* Read back MSC core registers */
        {
            /* Read back FILT registers */
            for (uint32_t i = 0U; i < MSC_MAX_OUTPUT; i++)
            {
                if(UTRUE == hObj->cslMscCfg[cfgIdx].mscCfg.scCfg[i].enable)
                {
                    readback->mscRegs.FILT[i].CFG      = CSL_REG32_RD(&mscRegs->FILT[i].CFG);
                    readback->mscRegs.FILT[i].SRC_ROI  = CSL_REG32_RD(&mscRegs->FILT[i].SRC_ROI);
                    readback->mscRegs.FILT[i].OUT_SIZE = CSL_REG32_RD(&mscRegs->FILT[i].OUT_SIZE);
                    readback->mscRegs.FILT[i].FIRINC   = CSL_REG32_RD(&mscRegs->FILT[i].FIRINC);
                    readback->mscRegs.FILT[i].ACC_INIT = CSL_REG32_RD(&mscRegs->FILT[i].ACC_INIT);
                }
                else
                {
                    /* If the scalar config is disabled, populating the filter registers with default reset values */
                    uint32_t expected_reset_val = 0x8U;
                    uint32_t reg_val = 0U;
                    CSL_FINS(reg_val, MSC_FILT_CFG_COEF_SHIFT, expected_reset_val);
                    readback->mscRegs.FILT[i].CFG = reg_val;
                }
            }

            /* Read back COEF_SP registers */
            for (uint32_t i = 0U; i < MSC_MAX_SP_COEFF_SET; i++)
            {
                readback->mscRegs.COEF_SP[i].C210 = CSL_REG32_RD(&mscRegs->COEF_SP[i].C210);
                readback->mscRegs.COEF_SP[i].C43  = CSL_REG32_RD(&mscRegs->COEF_SP[i].C43);
            }

            /* Read back COEF_MP registers */
            for (uint32_t a = 0U; a < MSC_MAX_MP_COEFF_SET; a++)
            {
                for (uint32_t b = 0U; b < MSC_MAX_PHASE; b++)
                {
                readback->mscRegs.COEF_MP[a].PH[b].C210 = CSL_REG32_RD(&mscRegs->COEF_MP[a].PH[b].C210);
                readback->mscRegs.COEF_MP[a].PH[b].C43  = CSL_REG32_RD(&mscRegs->COEF_MP[a].PH[b].C43);
                }
            }
        }

        /* Read back LSE registers */
        {
            readback->lseRegs.CFG_LSE = CSL_REG32_RD(&lseRegs->CFG_LSE);

            for (uint32_t i = 0U; i < VHWA_M2M_MSC_INPUT_MAX; i++)
            {
                if ((uint32_t)UFALSE == hObj->lseCfg[cfgIdx].inChCfg[i].bypass)
                {
                    readback->lseRegs.SRC[i].CFG        = CSL_REG32_RD(&lseRegs->SRC[i].CFG);
                    readback->lseRegs.SRC[i].FRAME_SIZE = CSL_REG32_RD(&lseRegs->SRC[i].FRAME_SIZE);
                    readback->lseRegs.SRC[i].BUF_ATTR   = CSL_REG32_RD(&lseRegs->SRC[i].BUF_ATTR);
                    for (uint32_t j = 0U; j < (hObj->lseCfg[cfgIdx].inChCfg[i].numInBuff); j++)
                    {
                        readback->lseRegs.SRC[i].BUF_BA[j] = CSL_REG32_RD(&lseRegs->SRC[i].BUF_BA[j]);
                    }
                }
            }

            for (uint32_t i = 0U; i < MSC_MAX_OUTPUT; i++)
            {
                if ((uint32_t)UFALSE == hObj->lseCfg[cfgIdx].outChCfg[i].bypass)
                {
                    /* Read BUF_CFG ignoring the CSL_LSE_DST_BUF_CFG_CH_DISABLED_MASK bit */
                    readback->lseRegs.DST[i].BUF_CFG   = CSL_REG32_RD(&lseRegs->DST[i].BUF_CFG) & ~CSL_LSE_DST_BUF_CFG_CH_DISABLED_MASK;
                    readback->lseRegs.DST[i].BUF_ATTR0 = CSL_REG32_RD(&lseRegs->DST[i].BUF_ATTR0);
                    readback->lseRegs.DST[i].BUF_BA    = CSL_REG32_RD(&lseRegs->DST[i].BUF_BA);
                }
            }

            #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
            /* Read back additional LSE registers */
            readback->lseRegs.DST_COMMON_CFG     = CSL_REG32_RD(&lseRegs->DST_COMMON_CFG);

            for (uint32_t i = 0U; i < VHWA_M2M_MSC_INPUT_MAX; i++)
            {
                if ((uint32_t)UFALSE == hObj->lseCfg[cfgIdx].inChCfg[i].bypass)
                {
                    for (uint32_t j = 0U; j < VHWA_M2M_MSC_INPUT_MAX; j++)
                    {
                        /* LDRA_JUSTIFY_START
                        <metric start> branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if(0U == i)
                        /* LDRA_JUSTIFY_END */
                        {
                            if((uint32_t)UTRUE == hObj->lseCfg[cfgIdx].inChCfg[i].buffConfig[j].enable)
                            {
                                readback->lseRegs.SRC0_CFG1          = CSL_REG32_RD(&lseRegs->SRC0_CFG1);
                                readback->lseRegs.SRC0_FRAME_SIZE1   = CSL_REG32_RD(&lseRegs->SRC0_FRAME_SIZE1);
                            }
                        }
                        else
                        {
                            /* LDRA_JUSTIFY_START
                            <metric start> branch <metric end>
                            <justification start>
                            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
                            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                            <justification end> */
                            if((uint32_t)UTRUE == hObj->lseCfg[cfgIdx].inChCfg[i].buffConfig[j].enable)
                            /* LDRA_JUSTIFY_END */
                            {
                                readback->lseRegs.SRC1_CFG1          = CSL_REG32_RD(&lseRegs->SRC1_CFG1);
                                readback->lseRegs.SRC1_FRAME_SIZE1   = CSL_REG32_RD(&lseRegs->SRC1_FRAME_SIZE1);
                            }
                        }
                    }
                }
            }
            #endif
        }
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((readback != NULL) && (comObj != NULL))
    /* LDRA_JUSTIFY_END */
    {
        status = vhwaM2mMscUpdateConfigRegGroup(readback, hObj, comObj);
    }

    return status;
}

int32_t vhwaM2mMscUpdateConfigRegGroup(VhwaVpacMscSocReadBack *RegVal, const Vhwa_M2mMscHandleObj *hObj, const Vhwa_M2mMscCommonObj *comObj)
{
    int32_t status = FVID2_SOK;
    uint32_t cfgIdx;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer. However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == RegVal) || (NULL == hObj) || (NULL == comObj))
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
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != comObj->socInfo.vpacCntlRegs)
        /* LDRA_JUSTIFY_END */
        {
            RegVal->vpacCntlRegs.ENABLE = CSL_REG32_RD(&comObj->socInfo.vpacCntlRegs->ENABLE);
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if(NULL != comObj->socInfo.mscRegs)
        /* LDRA_JUSTIFY_END */
        {
            RegVal->mscRegs.CONTROL  = CSL_REG32_RD(&comObj->socInfo.mscRegs->CONTROL);
        }

        uint32_t instId = hObj->instId;

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        /* Accumulate INTD registers for MSC */
        if (NULL != comObj->socInfo.vpacIntdRegs)
        /* LDRA_JUSTIFY_END */
        {
            volatile CSL_vpac_intd_cfgRegs *intdRegs = comObj->socInfo.vpacIntdRegs;
            uint32_t vhwaIrqNum = comObj->mscInitPrms.irqInfo[instId].vhwaIrqNum;
            uint32_t counter = 0U;

            /* NOTE: Do not make this a loop, since the INTR macros (VHWA_VPAC_INTD_PIPE_INTR_REG,
             * VHWA_VPAC_INTD_MSC_INTR_REG, VHWA_VPAC_INTD_WDTIMER_INTR_REG) can change in future.
             * Keeping this as explicit assignments ensures maintainability if the macros or their order change.
             */

            /* 0 -> VHWA_VPAC_INTD_MSC_INTR_REG */
            RegVal->vpacMscIntd.ENABLE_REG_LEVEL_VPAC_OUT[vhwaIrqNum][counter] =
            CSL_REG32_RD(&intdRegs->ENABLE_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_MSC_INTR_REG]);
            RegVal->vpacMscIntd.ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][counter] =
            CSL_REG32_RD(&intdRegs->ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_MSC_INTR_REG]);
            counter += 1U;

            /* 1 -> VHWA_VPAC_INTD_PIPE_INTR_REG */
            RegVal->vpacMscIntd.ENABLE_REG_LEVEL_VPAC_OUT[vhwaIrqNum][counter] =
            CSL_REG32_RD(&intdRegs->ENABLE_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_PIPE_INTR_REG]);
            RegVal->vpacMscIntd.ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][counter] =
            CSL_REG32_RD(&intdRegs->ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_PIPE_INTR_REG]);
            counter += 1U;

            /* 2 -> VHWA_VPAC_INTD_WDTIMER_INTR_REG */
            RegVal->vpacMscIntd.ENABLE_REG_LEVEL_VPAC_OUT[vhwaIrqNum][counter] =
            CSL_REG32_RD(&intdRegs->ENABLE_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_WDTIMER_INTR_REG]);
            RegVal->vpacMscIntd.ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][counter] =
            CSL_REG32_RD(&intdRegs->ENABLE_CLR_REG_LEVEL_VPAC_OUT[vhwaIrqNum][VHWA_VPAC_INTD_WDTIMER_INTR_REG]);
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        /* Accumulate HTS registers for MSC */
        if (comObj->socInfo.htsRegs != NULL)
        /* LDRA_JUSTIFY_END */
        {
            CSL_htsRegs *htsRegs = comObj->socInfo.htsRegs;
            uint32_t numSchedulers, schedBaseIdx;
            uint32_t numProducers, baseIdx;
            uint32_t numConsumers, consBaseIdx;
            uint32_t numDmaProducers, dmaBaseIdx;
            uint32_t numDmaConsumers, dmaConsBaseIdx;
            const CSL_HtsHwaParams *htsPrms = NULL;

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((uint32_t)VPAC_MSC_INST_ID_0 == instId)
            /* LDRA_JUSTIFY_END */
            {
                numSchedulers   = CSL_HTS_NUM_MSC0_SCHEDULERS;
                schedBaseIdx    = 0;
                numConsumers    = CSL_HTS_NUM_MSC0_CONSUMERS;
                consBaseIdx     = 0;
                numProducers    = CSL_HTS_NUM_MSC0_PRODUCERS;
                baseIdx         = 0;
                numDmaProducers = CSL_HTS_NUM_MSC0_PRODUCERS_DMA;
                dmaBaseIdx      = 0;
                numDmaConsumers = CSL_HTS_NUM_MSC0_CONSUMERS_DMA;
                dmaConsBaseIdx  = 0;
                htsPrms         = GetVpacHtsParams(CSL_HTS_HWA_SCH_MSC0);
            }
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
            else if ((uint32_t)VPAC_MSC_INST_ID_1 == instId)
            /* LDRA_JUSTIFY_END */
            {
                numSchedulers   = CSL_HTS_NUM_MSC1_SCHEDULERS;
                schedBaseIdx    = CSL_HTS_NUM_MSC0_SCHEDULERS;
                numConsumers    = CSL_HTS_NUM_MSC1_CONSUMERS;
                consBaseIdx     = CSL_HTS_NUM_MSC0_CONSUMERS;
                numProducers    = CSL_HTS_NUM_MSC1_PRODUCERS;
                baseIdx         = CSL_HTS_NUM_MSC0_PRODUCERS;
                numDmaProducers = CSL_HTS_NUM_MSC1_PRODUCERS_DMA;
                dmaBaseIdx      = CSL_HTS_NUM_MSC0_PRODUCERS_DMA;
                numDmaConsumers = CSL_HTS_NUM_MSC1_CONSUMERS_DMA;
                dmaConsBaseIdx  = CSL_HTS_NUM_MSC0_CONSUMERS_DMA;
                htsPrms         = GetVpacHtsParams(CSL_HTS_HWA_SCH_MSC1);
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
            else
            {
            status = FVID2_EBADARGS;
            }
            /* LDRA_JUSTIFY_END */

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
            if (status == FVID2_SOK)
            /* LDRA_JUSTIFY_END */
            {
                if(hObj->numIter > 1U)
                {
                    cfgIdx = (hObj->numIter - 1U);
                }
                else
                {
                    cfgIdx = 0U;
                }
                /* Read the HTS pipeline control register */
                RegVal->mscHtsRegs.PIPELINE_CONTROL[schedBaseIdx] = CSL_REG32_RD(&htsRegs->PIPELINE_CONTROL[(uint32_t)hObj->htsCfg[cfgIdx].pipeline]);

                /* Readback for schedulers */
                for (uint32_t schIdx = 0; schIdx < numSchedulers; schIdx++)
                {
                    uint32_t scheduler_base_addr = (uint32_t)comObj->socInfo.htsRegs + htsPrms->schOffset;
                    if(0U == hObj->curIterCnt)
                    {
                        uint32_t reg_val = CSL_REG32_RD(scheduler_base_addr + HWA_SCH_CONTROL_OFFSET);
                        RegVal->mscHtsRegs.SCHEDULER_CONTROL[schIdx + schedBaseIdx] =
                        (reg_val & CSL_HTS_HWA4_SCHEDULER_CONTROL_PAUSE_MASK) |
                        (reg_val & CSL_HTS_HWA4_SCHEDULER_CONTROL_STRM_EN_MASK) |
                        (reg_val & CSL_HTS_HWA4_SCHEDULER_CONTROL_PIPELINE_NUM_MASK) |
                        (reg_val & CSL_HTS_HWA4_SCHEDULER_CONTROL_SCH_EN_MASK) |
                        (reg_val & CSL_HTS_HWA4_SCHEDULER_CONTROL_DEBUG_RDY_MASK);

                        reg_val = CSL_REG32_RD(scheduler_base_addr + HWA_WDTIMER_OFFSET);
                        RegVal->mscHtsRegs.WDTIMER[schIdx + schedBaseIdx] =
                        (reg_val & CSL_HTS_HWA4_WDTIMER_WDTIMER_EN_MASK) |
                        (reg_val & CSL_HTS_HWA4_WDTIMER_WDTIMER_MODE_MASK);

                        RegVal->mscHtsRegs.BW_LIMITER[schIdx + schedBaseIdx] = CSL_REG32_RD(scheduler_base_addr + HWA_BW_LIMITER_OFFSET);
                    }

                    #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
                    uint32_t hop_reg_val = CSL_REG32_RD(scheduler_base_addr + HWA_HOP_OFFSET);
                    RegVal->mscHtsRegs.HOP[schIdx + schedBaseIdx] =
                    (hop_reg_val & CSL_HTS_HWA4_HOP_TDONE_COUNT_EN_MASK) |
                    (hop_reg_val & CSL_HTS_HWA4_HOP_HOP_MASK) |
                    (hop_reg_val & CSL_HTS_HWA4_HOP_HOP_THREAD_COUNT_MASK);
                    #endif
                }

                if(0U == hObj->curIterCnt)
                {
                    /* Readback for consumer control */
                    for (uint32_t i = 0; i < numConsumers; i++)
                    {
                        uint32_t cons_ctrl_addr = (uint32_t)comObj->socInfo.htsRegs + htsPrms->consOffset[i];
                        RegVal->mscHtsRegs.CONS_CONTROL[i + consBaseIdx] = CSL_REG32_RD(cons_ctrl_addr + HWA_CONS_CONTROL_OFFSET);
                    }

                    /* Readback for producers control */
                    for (uint32_t i = 0; i < numProducers; i++)
                    {
                        uint32_t prod_ctrl_addr = (uint32_t)comObj->socInfo.htsRegs + htsPrms->prodOffset[i];
                        RegVal->mscHtsRegs.PROD_CONTROL[i + baseIdx] = CSL_REG32_RD(prod_ctrl_addr + HWA_PROD_CONTROL_OFFSET);
                        RegVal->mscHtsRegs.PROD_BUF_CONTROL[i + baseIdx] = CSL_REG32_RD(prod_ctrl_addr + HWA_PROD_BUF_CONTROL_OFFSET);

                        uint32_t reg_val = CSL_REG32_RD(prod_ctrl_addr + HWA_PROD_COUNT_OFFSET);
                        RegVal->mscHtsRegs.PROD_COUNT[i + baseIdx] =
                        (reg_val & CSL_HTS_HWA4_PROD0_COUNT_COUNT_POSTLOAD_MASK) |
                        (reg_val & CSL_HTS_HWA4_PROD0_COUNT_COUNT_PRELOAD_MASK);

                        #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
                        RegVal->mscHtsRegs.PA_CONTROL[i + baseIdx] = CSL_REG32_RD(prod_ctrl_addr + HWA_PROD_PA_CONTROL_OFFSET);

                        reg_val = CSL_REG32_RD(prod_ctrl_addr + HWA_PROD_PA_PROD_COUNT_OFFSET);
                        RegVal->mscHtsRegs.PA_PRODCOUNT[i + baseIdx] = (reg_val & CSL_HTS_HWA4_PA0_PRODCOUNT_PA_COUNT_DEC_MASK);
                        #endif
                    }
                }

                /* Readback for DMA producers */
                for (uint32_t i = 0; i < numDmaProducers; i++)
                {
                    uint32_t prod_dma_ctrl_addr = (uint32_t)comObj->socInfo.htsRegs + htsPrms->dmaProdOffset[i];
                    if(0U == hObj->curIterCnt)
                    {
                        uint32_t reg_val = CSL_REG32_RD(prod_dma_ctrl_addr + HWA_DMA_PROD_SCH_CONTROL_OFFSET);
                        RegVal->mscHtsRegs.PROD_DMA_SCHEDULER_CONTROL[i + dmaBaseIdx] =
                        (reg_val & CSL_HTS_DMA32_SCHEDULER_CONTROL_DMA_CHANNEL_NO_MASK) |
                        (reg_val & CSL_HTS_DMA32_SCHEDULER_CONTROL_DEBUG_RDY_MASK) |
                        (reg_val & CSL_HTS_DMA32_SCHEDULER_CONTROL_PAUSE_MASK) |
                        (reg_val & CSL_HTS_DMA32_SCHEDULER_CONTROL_STRM_EN_MASK) |
                        (reg_val & CSL_HTS_DMA32_SCHEDULER_CONTROL_PIPELINE_NUM_MASK) |
                        (reg_val & CSL_HTS_DMA32_SCHEDULER_CONTROL_SCH_EN_MASK);

                
                        RegVal->mscHtsRegs.PROD_CONTROL_DMA[i + dmaBaseIdx] = CSL_REG32_RD(prod_dma_ctrl_addr + HWA_DMA_PROD_CONTROL_OFFSET);
                        RegVal->mscHtsRegs.PROD_BUF_CONTROL_DMA[i + dmaBaseIdx] = CSL_REG32_RD(prod_dma_ctrl_addr + HWA_DMA_PROD_BUF_CONTROL_OFFSET);

                        reg_val = CSL_REG32_RD(prod_dma_ctrl_addr + HWA_DMA_PROD_COUNT_OFFSET);
                        RegVal->mscHtsRegs.PROD_COUNT_DMA[i + dmaBaseIdx] =
                        (reg_val & CSL_HTS_DMA32_PROD0_COUNT_COUNT_POSTLOAD_MASK) |
                        (reg_val & CSL_HTS_DMA32_PROD0_COUNT_COUNT_PRELOAD_MASK);

                        #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
                        RegVal->mscHtsRegs.PA_CONTROL_DMA[i + dmaBaseIdx] = CSL_REG32_RD(prod_dma_ctrl_addr + HWA_DMA_PROD_PA_CONTROL_OFFSET);

                        reg_val = CSL_REG32_RD(prod_dma_ctrl_addr + HWA_DMA_PROD_PA_PROD_COUNT_OFFSET);
                        RegVal->mscHtsRegs.PA_PRODCOUNT_DMA[i + dmaBaseIdx] = (reg_val & CSL_HTS_DMA32_PA0_PRODCOUNT_PA_COUNT_DEC_MASK);
                        #endif
                    }
                    #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
                    uint32_t hop_reg_val = CSL_REG32_RD(prod_dma_ctrl_addr + HWA_DMA_PROD_HOP_OFFSET);
                    RegVal->mscHtsRegs.PROD_DMA_HOP[i + dmaBaseIdx] =
                        (hop_reg_val & CSL_HTS_DMA32_HOP_HOP_MASK) |
                        (hop_reg_val & CSL_HTS_DMA32_HOP_HOP_THREAD_COUNT_MASK) |
                        (hop_reg_val & CSL_HTS_DMA32_HOP_TDONE_COUNT_EN_MASK);
                    #endif
                }

                /* Readback for consumers DMA - only read if the DMA consumer is enabled in htsCfg.
                 * MSC0 and MSC1 share the same DMA consumer schedulers, so we must only read
                 * registers for consumers that are enabled for this specific MSC instance to
                 * avoid reading registers configured for the other MSC instance. */
                if(0U == hObj->curIterCnt)
                {
                    for (uint32_t i = 0; i < numDmaConsumers; i++)
                    {
                        /* Only read back if DMA consumer is enabled for this MSC instance */
                        if((uint32_t)UTRUE == hObj->htsCfg[cfgIdx].dmaConsCfg[i].enable)
                        {
                            uint32_t cons_dma_ctrl_addr = (uint32_t)comObj->socInfo.htsRegs + htsPrms->dmaConsOffset[i];
                            uint32_t reg_val = CSL_REG32_RD(cons_dma_ctrl_addr + HWA_DMA_CONS_SCH_CONTROL_OFFSET);
                            RegVal->mscHtsRegs.CONS_DMA_SCHEDULER_CONTROL[i + dmaConsBaseIdx] =
                            (reg_val & CSL_HTS_DMA304_SCHEDULER_CONTROL_DMA_CHANNEL_NO_MASK) |
                            (reg_val & CSL_HTS_DMA304_SCHEDULER_CONTROL_DEBUG_RDY_MASK) |
                            (reg_val & CSL_HTS_DMA304_SCHEDULER_CONTROL_PAUSE_MASK) |
                            (reg_val & CSL_HTS_DMA304_SCHEDULER_CONTROL_STRM_EN_MASK) |
                            (reg_val & CSL_HTS_DMA304_SCHEDULER_CONTROL_PIPELINE_NUM_MASK) |
                            (reg_val & CSL_HTS_DMA304_SCHEDULER_CONTROL_SCH_EN_MASK);

                            RegVal->mscHtsRegs.CONS_CONTROL_DMA[i + dmaConsBaseIdx] = CSL_REG32_RD(cons_dma_ctrl_addr + HWA_DMA_CONS_CONTROL_OFFSET);
                        }
                    }
                }
            }
        }

    }

    return status;
}
#endif
int32_t Vhwa_m2mMscGetSl2Info(Vhwa_M2mMscSl2Info *sl2Info)
{
    int32_t retVal = FVID2_SOK;
    Vhwa_M2mMscCommonObj *comObj = &gM2mMscCommonObj;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to print the error status. However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == comObj) || (NULL == sl2Info))
    {
        retVal = FVID2_EBADARGS;
    }
    else if ((uint32_t)UFALSE == comObj->isSl2AllocDone)
    {
        /* SL2 memory not allocated yet */
        GT_0trace(VhwaMscTrace, GT_ERR,
            "SL2 Memory is not allocated. Call Vhwa_m2mMscAllocSl2() first.\n");
        retVal = FVID2_EFAIL;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        sl2Info->sl2Addr = comObj->sl2Prms.sl2StartAddr;
        sl2Info->sl2Size = comObj->sl2Prms.sl2MemSize;
    }

    return retVal;
}