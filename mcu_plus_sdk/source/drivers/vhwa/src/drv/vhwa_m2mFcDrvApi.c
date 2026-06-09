/**
 *   Copyright (c) Texas Instruments Incorporated 2021
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
 *  \file vhwa_graphApi.c
 *
 *  \brief API Implementation
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "src/drv/vhwa_m2mFcDrvPriv.h"


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
static int32_t Vhwa_m2mFcCreateQueue(Vhwa_M2mFcDrvInstObj *instObj);

/**
 * \brief   Delete Queues
 *
 * \param   instObj             Instance object.
 *
 * \return  FVID2_SOK in case of sucess, Error otherwise
 *
 **/
static int32_t Vhwa_m2mFcDeleteQueue(Vhwa_M2mFcDrvInstObj *instObj);

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
static Vhwa_M2mFcHandleObj *Vhwa_m2mFcDrvAllocHdlObj(const Vhwa_M2mFcDrvInstObj *instObj);

/**
 * \brief   Local Function to free Handle Object
 *          No protection inside the function, Caller should protect
 *          the function call
 *
 * \param   instObj      Instance object.
 *
 */
static void Vhwa_m2mFcDrvFreeHandleObj(Vhwa_M2mFcHandleObj *hObj);

static void Vhwa_m2mFcInitGraph(Vhwa_M2mFcGraphObj *newGraphObj);

static int32_t Vhwa_m2mFcParseGraph(const Vhwa_M2mFcDrvInstObj *instObj,
                                        Vhwa_M2mFcHandleObj *hObj,
                                        const Vhwa_M2mFcGraphPathInfo *pathInfo);

static int32_t Vhwa_m2mFcConfigHts(Vhwa_M2mFcHandleObj *hObj);

static int32_t Vhwa_m2mFcCreateHndls(Vhwa_M2mFcDrvInstObj *instObj,
                                        Vhwa_M2mFcHandleObj *hObj);
static void Vhwa_m2mFcDeleteHndls(Vhwa_M2mFcDrvInstObj *instObj,
                                            Vhwa_M2mFcHandleObj *hObj);

static void Vhwa_m2mFcPrepareFrmList(const Vhwa_M2mFcQueueObj *qObj,
                                        const Fvid2_FrameList *inFrmList,
                                        const Fvid2_FrameList *outFrmList);

static int32_t Vhwa_m2mFcConfigAndStart(Vhwa_M2mFcDrvInstObj *instObj,
                                      Vhwa_M2mFcQueueObj *qObj,
                                      uint32_t timeout);

/**
* \brief    FcFreeSl2 function.
*
* \param   void.
*
* \return  void.
*
**/
void Vhwa_m2mFcFreeSl2(void);

/* Implementation of FVID2 APIs */
/**
 * \brief   FVID2 Create Function.
 *
 * \param   instObj             Instance object.
 *
 * \return  FVID2 Driver Handle.
 *
 **/
Fdrv_Handle Vhwa_m2mFcCreate(UInt32 drvId, UInt32 drvInstId,
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
Int32 Vhwa_m2mFcDelete(Fdrv_Handle handle, Ptr deleteArgs);

/**
 * \brief   FVID2 Control Function.
 *
 * \param   handle              FVID2 driver handle.
 *
 * \return  FVID2_SOK on success, else FVID2 error code
 *
 **/
Int32 Vhwa_m2mFcControl(Fdrv_Handle handle, UInt32 cmd,
                                  Ptr cmdArgs, Ptr cmdStatusArgs);

/**
 * \brief   FVID2 Process Request function.
 *
 * \param   handle              FVID2 driver handle.
 *
 * \return  FVID2_SOK on success, else FVID2 error code
 *
 **/
Int32 Vhwa_m2mFcProcessReq(Fdrv_Handle handle,
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
Int32 Vhwa_m2mFcGetProcessReq(Fdrv_Handle handle,
                                        Fvid2_FrameList *inFrmList,
                                        Fvid2_FrameList *outFrmList,
                                        UInt32 timeout);

static uint64_t Vhwa_getSrcNode(uint64_t startPort);

static uint64_t Vhwa_getDesNode(uint64_t endPort);

static uint32_t Vhwa_m2mFcGetHtsPipeline(const Vhwa_M2mFcHandleObj *hObj);

static int32_t Vhwa_m2mFcGetPrms(Vhwa_M2mFcHandleObj *hObj);

static int32_t Vhwa_m2mFcUpdatePrms(Vhwa_M2mFcHandleObj *hObj);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
Vhwa_M2mFcHandleObj   gFcHandleObj[VHWA_FC_DRV_MAX_HANDLES];
Vhwa_M2mFcDrvInstObj  gFcInstObj;

Fvid2_DrvOps gGraphFvid2DrvOps = {
    FVID2_VHWA_M2M_FC_DRV_ID,
    /**< Unique driver Id. */
    Vhwa_m2mFcCreate,
    /**< FVID2 create function pointer. */
    Vhwa_m2mFcDelete,
    /**< FVID2 delete function pointer. */
    Vhwa_m2mFcControl,
    /**< FVID2 control function pointer. */
    NULL, NULL,
    /**< FVID2 queue function pointer. */
    Vhwa_m2mFcProcessReq,
    /**< FVID2 process request function pointer. */
    Vhwa_m2mFcGetProcessReq,
    /**< FVID2 get processed request function pointer. */
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t Vhwa_m2mFcInit(const Vhwa_M2mFcInitPrms *initPrms)
{
    int32_t                 status = FVID2_SOK;
    uint32_t                cnt;
    #if !defined(MCU_PLUS_SDK)    
    SemaphoreP_Params   params;
    #endif
    Vhwa_M2mFcDrvInstObj    *instObj = NULL;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL == initPrms)
    {
        status = FVID2_EBADARGS;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        instObj = &gFcInstObj;

        /* Reset all instance object to 0x0 */
        Fvid2Utils_memset(instObj, 0U, sizeof (gFcInstObj));

        /* Mark pool flags as free */
        for (cnt = 0U; cnt < VHWA_FC_DRV_MAX_HANDLES; cnt++)
        {
            gFcHandleObj[cnt].isUsed = (uint32_t) UFALSE;
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
        /* Register the FVID2 Drvier
           Indivisual drivers are not registered now */
        status = Fvid2_registerDriver(&gGraphFvid2DrvOps);
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached. 
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            instObj->isRegistered = (uint32_t)UTRUE;
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            GT_0trace(VhwaFcTrace, GT_ERR,
                "FVID2 driver registration failed!!\n");
            instObj->isRegistered = (uint32_t)UFALSE;
        }

        /* Create Semaphores */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            /* Allocate lock semaphore */
            #if defined(MCU_PLUS_SDK)
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
            if(FVID2_SOK != SemaphoreP_constructBinary(&instObj->lock,1U))
            #else
            SemaphoreP_Params_init(&params);
            params.mode = SemaphoreP_Mode_BINARY;
            instObj->lock = SemaphoreP_create(1U, &params);
            if (NULL == instObj->lock)
            #endif
            {
                GT_0trace(VhwaFcTrace, GT_ERR,
                    "Failed to allocate instance semaphore!!\n");
                status = FVID2_EALLOC;
            }
            /* LDRA_JUSTIFY_END */
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
            /* Allocate lock semaphore */
            #if defined(MCU_PLUS_SDK)
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
            if(FVID2_SOK != SemaphoreP_constructBinary(&instObj->pLock,1U))
            #else
            SemaphoreP_Params_init(&params);
            params.mode = SemaphoreP_Mode_BINARY;
            instObj->pLock = SemaphoreP_create(1U, &params);
            if (NULL == instObj->pLock)
            #endif
            {
                GT_0trace(VhwaNfTrace, GT_ERR,
                    "Failed to allocate HWA Lock semaphore!!\n");
                status = FVID2_EALLOC;
            }
            /* LDRA_JUSTIFY_END */
        }

        /* Create free and request queues */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached. 
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            status = Vhwa_m2mFcCreateQueue(instObj);
        }

        /* Init is done, copy init params locally,
           enable DMPAC and NF in DMPAC Top */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached. 
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            Fvid2Utils_memcpy(&instObj->initPrms, initPrms,
                sizeof(Vhwa_M2mFcInitPrms));

            instObj->initDone = (uint32_t)UTRUE;
            instObj->lastHndlObj = NULL;

            Fc_getSocHtsInfo(&instObj->htsRegs);
        }
    }

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached. 
    Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK != status)
    {
        Vhwa_m2mFcDeInit();
    }
    /* LDRA_JUSTIFY_END */

    return (status);
}

void Vhwa_m2mFcDeInit(void)
{
    Vhwa_M2mFcDrvInstObj  *instObj = NULL;

    instObj = &gFcInstObj;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
    Effect on this unit: If the control reaches here, our code base is expected to print the error status. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (instObj->openCnt > 0u)
    {
        GT_0trace(VhwaFcTrace, GT_ERR,
            "Warning: All driver handles are not closed!!\n");
    }
    /* LDRA_JUSTIFY_END */

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
    Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations. However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((uint32_t)UTRUE == instObj->isRegistered)
    /* LDRA_JUSTIFY_END */
    {
        (void)Fvid2_unRegisterDriver(&gGraphFvid2DrvOps);
    }

    /* Delete the lock semaphore */
    #if defined(MCU_PLUS_SDK)
     SemaphoreP_destruct(&instObj->lock);    
     SemaphoreP_destruct(&instObj->pLock);
    #else    
    if (NULL != instObj->lock)
    {
        (void)SemaphoreP_delete(instObj->lock);
        instObj->lock = NULL;
    }
    if (NULL != instObj->pLock)
    {
        (void)SemaphoreP_delete(instObj->pLock);
        instObj->pLock = NULL;
    }
    #endif

    (void)Vhwa_m2mFcDeleteQueue(instObj);

    /* Init all global variables to zero */
    Fvid2Utils_memset(instObj, 0U, sizeof (gFcInstObj));

    instObj->initDone = (uint32_t)UFALSE;
}

Fdrv_Handle Vhwa_m2mFcCreate(UInt32 drvId, UInt32 drvInstId,
    Ptr createArgs, Ptr createStatusArgs, const Fvid2_DrvCbParams *cbPrms)
{
    int32_t                 status = FVID2_SOK;
    Vhwa_M2mFcDrvInstObj     *instObj = NULL;
    Vhwa_M2mFcHandleObj   *hObj = NULL;
    Vhwa_M2mFcCreatePrms  *fcCreatePrms = NULL;
    Fdrv_Handle         handle = NULL;

    instObj = &gFcInstObj;

    /* Check for errors */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The parameters are pre-validated by the caller before the control reaches here. 
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == createArgs) ||
        (NULL == cbPrms))
    {
        GT_0trace(VhwaFcTrace, GT_ERR, "NULL Pointer !!\n");
        status = FVID2_EBADARGS;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        /* Open not allowed if init is not done */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)UFALSE == instObj->initDone)
        {
            GT_0trace(VhwaFcTrace, GT_ERR,
                "Vhwa_m2mFcInit is not called\n");
            status  = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */

        fcCreatePrms = (Vhwa_M2mFcCreatePrms *)createArgs;
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
        /* Take the instance semaphore */
        (void)VhwaAl_SemaphoreP_pend(instObj->lock, SYSTEMP_WAIT_FOREVER);

        /* Allocate Handle Object */
        hObj = Vhwa_m2mFcDrvAllocHdlObj(instObj);

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != hObj)
        /* LDRA_JUSTIFY_END */
        {
            /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application. This behaviour is part of the application design. An error print statement can be added in a future release if required.
        <justification end> */
            if (0U == instObj->openCnt)
            /* LDRA_JUSTIFY_END */
            {
                /* On the first open, enable interrupts in HTS and
                 * start all UTC channels.
                 * Even if all UTC channels are enabled, only channels enabled
                 * in HTS will be used for the transfer */

                /* Check if SL2 is allocated, Allocate the SL2 for default
                   if not allocated */
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
                Effect on this unit: If the control reaches here, our code base is expected to print the error status. However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((uint32_t)UFALSE == instObj->isSl2AllocDone)
                /* LDRA_JUSTIFY_END */
                {
                    GT_0trace(VhwaFcTrace, GT_DEBUG,
                    "Vhwa_m2mFcAllocSl2 is not called, allocating for default\n");
                }
            }

			status = Fvid2Utils_constructQ(&hObj->doneQObj);

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached. 
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK == status)
            /* LDRA_JUSTIFY_END */
            {
                hObj->doneQ = &hObj->doneQObj;

                Fvid2Utils_memcpy(&hObj->createPrms, fcCreatePrms,
                    sizeof(Vhwa_M2mFcCreatePrms));

                Fvid2Utils_memcpy(&hObj->cbPrms, cbPrms,
                    sizeof (hObj->cbPrms));

                instObj->openCnt ++;

                Vhwa_m2mFcInitGraph(&hObj->graphObj);
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
            Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations. However, due to the stated rationale, this is not tested.
            <justification end> */
            else
            {
                /* Some error, so free up the handle object */
                Vhwa_m2mFcDrvFreeHandleObj(hObj);
            }
            /* LDRA_JUSTIFY_END */
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached. 
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            status = FVID2_EALLOC;
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached. 
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
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

Int32 Vhwa_m2mFcDelete(Fdrv_Handle handle, Ptr deleteArgs)
{
    int32_t                 status = FVID2_SOK;
    Vhwa_M2mFcDrvInstObj     *instObj = NULL;
    Vhwa_M2mFcHandleObj   *hObj = NULL;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The parameters are pre-validated by the caller before the control reaches here. 
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != handle)
    /* LDRA_JUSTIFY_END */
    {
        instObj = &gFcInstObj;
        hObj    = (Vhwa_M2mFcHandleObj *)handle;

        /* Take the instance semaphore */
        (void)VhwaAl_SemaphoreP_pend(instObj->lock, SYSTEMP_WAIT_FOREVER);

        /* Check if all handles are already closed */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (0U == instObj->openCnt)
        {
            status = FVID2_EFAIL;
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached. 
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
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
        Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached. 
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            Vhwa_m2mFcDrvFreeHandleObj(hObj);

            instObj->openCnt --;

            /* Delete the Done queue */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
            Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations. However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL != hObj->doneQ)
            /* LDRA_JUSTIFY_END */
            {
                Fvid2Utils_destructQ(hObj->doneQ);
                hObj->doneQ = NULL;
            }

            /* for the last close, stop all channels and
             * disable HTS interrupt in INTD */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
            Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations. However, due to the stated rationale, this is not tested.
            <justification end> */
            if (0U == instObj->openCnt)
            /* LDRA_JUSTIFY_END */
            {
                /* If not already done, disable HTS Interrupt */
                Vhwa_m2mDisableFcVissHtsIntr();
                Vhwa_m2mDisableFcMscHtsIntr();

                /* No active objects and last handle object */
                instObj->actQObj = NULL;
                instObj->lastHndlObj = NULL;
            }
        }

        (void)VhwaAl_SemaphoreP_post(instObj->lock);
    }

    return (status);
}

Int32 Vhwa_m2mFcControl(Fdrv_Handle handle, UInt32 cmd, Ptr cmdArgs,
    Ptr cmdStatusArgs)
{
    int32_t             status = FVID2_SOK;
    Vhwa_M2mFcDrvInstObj        *instObj = NULL;
    Vhwa_M2mFcHandleObj         *hObj = NULL;
    Vhwa_M2mFcGraphPathInfo     *pathInfo = NULL;
    Vhwa_M2mFcMscArgs           *mscArgs = NULL;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL == handle)
    {
        status = FVID2_EBADARGS;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        instObj = &gFcInstObj;
        hObj = (Vhwa_M2mFcHandleObj *)handle;
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
        /* Take the instance semaphore */
        (void)VhwaAl_SemaphoreP_pend(instObj->lock, SYSTEMP_WAIT_FOREVER);

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        switch (cmd)
        {
        /* LDRA_JUSTIFY_END */
            /* Set FC Parameters */
            case IOCTL_VHWA_FC_SET_GRAPH:
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The parameters are pre-validated by the caller before the control reaches here. 
                Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer. However, due to the stated rationale, this is not tested.
                <justification end> */
                if(NULL != cmdArgs)
                /* LDRA_JUSTIFY_END */
                {
                    pathInfo = (Vhwa_M2mFcGraphPathInfo *)cmdArgs;
                    /* parse graph */
                    status = Vhwa_m2mFcParseGraph(instObj, hObj, pathInfo);
                    /* Allocate resources */
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached. 
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if(FVID2_SOK == status)
                    /* LDRA_JUSTIFY_END */
                    {
                        status = Vhwa_m2mFcInitAndAllocRes(&hObj->graphObj);
                    }
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    else
                    {
                        GT_0trace(VhwaFcTrace, GT_ERR,
                            "Failed to parse graph!!\n");
                    }
                    /* create indivisual Driver handle */
                    if(FVID2_SOK == status)
                    /* LDRA_JUSTIFY_END */
                    {
                        status = Vhwa_m2mFcCreateHndls(instObj, hObj);
                    }
                    /* LDRA_JUSTIFY_START
                    <metric start> statement <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    else
                    {
                        GT_0trace(VhwaFcTrace, GT_ERR,
                            "Failed to create handles!!\n");
                    }
                    /* LDRA_JUSTIFY_END */
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    GT_0trace(VhwaFcTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_VHWA_FC_SET_GRAPH !!\n");
                    status = FVID2_EBADARGS;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case IOCTL_VHWA_FC_SET_CONFIG:
            {
                /* configure HWA, Schedular and TR configuration */
                status = Vhwa_m2mFcConfigHts(hObj);
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                <justification end> */
                if(FVID2_SOK != status)
                {
                    GT_0trace(VhwaFcTrace, GT_ERR,
                        "Failed to configure HTS!!\n");
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            case IOCTL_VHWA_FC_DELETE_GRAPH:
            {
                Vhwa_m2mFcDeleteHndls(instObj, hObj);
                break;
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications CAN reach this portion. No existing test case can reach this portion. A test case will be added to cover this gap in a future release. 
            Effect on this unit: If the control reaches here, our code base is expected to print the error. However, due to the stated rationale, this is not tested.
            <justification end> */
            case IOCTL_VHWA_FC_GET_HANDLES:
            {
                Vhwa_M2mFcModuleHdls *pModHdls = NULL;
                if (NULL != cmdArgs)
                {
                    pModHdls = (Vhwa_M2mFcModuleHdls *)cmdArgs;
                    Fvid2Utils_memcpy(pModHdls, &hObj->modHdls,
                            sizeof(Vhwa_M2mFcModuleHdls));
                }
                else
                {
                    GT_0trace(VhwaFcTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_VHWA_FC_GET_HANDLES !!\n");
                    status = FVID2_EBADARGS;
                }
                break;
            }

            case IOCTL_VHWA_FC_GET_PERFORMANCE:
            {
                uint32_t *perf = NULL;
                if (NULL != cmdArgs)
                {
                    perf = (uint32_t *)cmdArgs;
                    *perf = (uint32_t)hObj->perfNum;
                }
                else
                {
                    GT_0trace(VhwaFcTrace, GT_ERR,
                        "cmdArgs cannot be NULL for IOCTL_VHWA_FC_GET_PERFORMANCE !!\n");
                    status = FVID2_EBADARGS;
                }
                break;
            }

            case IOCTL_VHWA_FC_REGISTER_ERR_CB:
            {
                break;
            }
            /* LDRA_JUSTIFY_END */
            /* Default Case */
            default:
            {
                /* Check for VISS control CMD */
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications CAN reach this portion. No existing test case can reach this portion. A test case will be added to cover this gap in a future release. 
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                <justification end> */
                if(((cmd >= VHWA_IOCTL_M2M_VISS_IOCTL_BASE) && (cmd < VHWA_IOCTL_M2M_VISS_IOCTL_END)) ||
                   ((cmd >= VHWA_IOCTL_RFE_IOCTL_BASE) && (cmd < VHWA_IOCTL_CAC_IOCTL_END)) ||
                   ((cmd >= VHWA_IOCTL_FC_VISS_IOCTL_BASE) && (cmd < VHWA_IOCTL_FC_VISS_IOCTL_END)))
                /* LDRA_JUSTIFY_END */   
                {
                    status = Fvid2_control(hObj->modHdls.vissHandle, cmd, cmdArgs,
                                            cmdStatusArgs);
                }
                /* Check for MSC control CMD if not in VISS */
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications CAN reach this portion. No existing test case can reach this portion. A test case will be added to cover this gap in a future release. 
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                <justification end> */
                else if(((cmd >= VHWA_IOCTL_MSC_IOCTL_BASE) && (cmd < VHWA_IOCTL_M2M_MSC_IOCTL_END)) ||
                        ((cmd >= VHWA_IOCTL_FC_MSC_IOCTL_BASE) && (cmd < VHWA_IOCTL_FC_MSC_IOCTL_END)))
                /* LDRA_JUSTIFY_END */
                {
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion. The parameters are pre-validated by the caller before the control reaches here. 
                    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if(NULL != cmdArgs)
                    /* LDRA_JUSTIFY_END */
                    {
                        mscArgs = (Vhwa_M2mFcMscArgs *)(cmdArgs);
                        if(VPAC_MSC_INST_ID_0 == mscArgs->mscInstId)
                        {
                            status = Fvid2_control(hObj->modHdls.msc0Handle,
                                                    cmd, mscArgs->cmdArgs, cmdStatusArgs);
                        }
                        else
                        {
                            status = Fvid2_control(hObj->modHdls.msc1Handle, cmd,
                                                    mscArgs->cmdArgs, cmdStatusArgs);
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
                    status = FVID2_EUNSUPPORTED_CMD;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }
        }

        (void)VhwaAl_SemaphoreP_post(instObj->lock);
    }

    return (status);
}

Int32 Vhwa_m2mFcProcessReq(Fdrv_Handle handle, Fvid2_FrameList *inFrmList,
    Fvid2_FrameList *outFrmList, uint32_t timeout)
{
    int32_t             status = FVID2_SOK;
    uint32_t            semTimeout;
    Vhwa_M2mFcDrvInstObj   *instObj = NULL;
    Vhwa_M2mFcHandleObj *hObj = NULL;
    Vhwa_M2mFcQueueObj  *qObj = NULL;
    int32_t               semStatus;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The parameters are pre-validated by the caller before the control reaches here. 
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == handle) || (NULL == inFrmList) || (NULL == outFrmList))
    {
        status = FVID2_EBADARGS;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        instObj = &gFcInstObj;
        hObj = (Vhwa_M2mFcHandleObj *)handle;
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
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application. This behaviour is part of the application design. An error print statement can be added in a future release if required.
        <justification end> */
        if (FVID2_TIMEOUT_FOREVER == timeout)
        /* LDRA_JUSTIFY_END */
        {
            semTimeout = SYSTEMP_WAIT_FOREVER;
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application. This behaviour is part of the application design. An error print statement can be added in a future release if required.
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

        semStatus = VhwaAl_SemaphoreP_pend(instObj->pLock, semTimeout);

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. This failure case is out of scope for the imaging test framework. 
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (SYSTEMP_SUCCESS == semStatus)
        /* LDRA_JUSTIFY_END */
        {
            status = FVID2_SOK;
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. This failure case is out of scope for the imaging test framework. 
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        else /* Any other status */
        {
            /* Failed, so reducing number of pending request */
            hObj->numPendReq --;

            /* Convert Semaphore status to FVID2 status */
            if (SYSTEMP_FAILURE == semStatus)
            {
                status = FVID2_EFAIL;
            }
            else if (SYSTEMP_TIMEOUT == semStatus)
            {
                status = FVID2_EAGAIN;
            }
            else /* For all other cases, it is failed */
            {
                status = FVID2_EFAIL;
            }
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached. 
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            /* Get a queue object from the free queue,
             * No need to protect from ISR as it is not accessed from ISR */
            qObj = (Vhwa_M2mFcQueueObj *) Fvid2Utils_dequeue(instObj->freeQ);

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL == qObj)
            {
                hObj->numPendReq --;

                GT_0trace(VhwaFcTrace, GT_ERR,
                    "Failed to Free Queue Object!!\n");
                status = FVID2_EALLOC;
            }
            else
            /* LDRA_JUSTIFY_END */
            {
                qObj->hObj = hObj;

                Vhwa_m2mFcPrepareFrmList(qObj, inFrmList, outFrmList);

                /* Copy the application's process list to request objects lists */
                Fvid2_copyFrameList(&qObj->inFrmList, inFrmList);
                Fvid2_copyFrameList(&qObj->outFrmList, outFrmList);
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
            /* Start the HTS Pipeline */
            status = Vhwa_m2mFcConfigAndStart(instObj, qObj, timeout);
        }
    }

    return (status);
}

/** \brief Typedef for FVID2 get processed frames function pointer. */
Int32 Vhwa_m2mFcGetProcessReq(Fdrv_Handle handle,
    Fvid2_FrameList *inFrmList, Fvid2_FrameList *outFrmList,
    UInt32 timeout)
{
    int32_t                status = FVID2_SOK;
    uint32_t               cookie;
    Vhwa_M2mFcDrvInstObj    *instObj = NULL;
    Vhwa_M2mFcHandleObj  *hObj = NULL;
    Vhwa_M2mFcQueueObj   *qObj = NULL;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The parameters are pre-validated by the caller before the control reaches here. 
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL == handle)
    {
        status = FVID2_EBADARGS;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        instObj = &gFcInstObj;
        hObj = (Vhwa_M2mFcHandleObj *)handle;
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
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application. This behaviour is part of the application design. An error print statement can be added in a future release if required.
        <justification end> */
        while(statusQ == TRUE);
        /* LDRA_JUSTIFY_END */
		#endif
        /* Disable interrupts before accessing queue */
        cookie = HwiP_disable();

        /* Dequeue the completed request from done queue */
        qObj = (Vhwa_M2mFcQueueObj *) Fvid2Utils_dequeue(hObj->doneQ);

        /* Restore interrupts after updating node list */
        HwiP_restore(cookie);

        /* No buffers in the output queue */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL == qObj)
        {
            /* Check if requests are pending with driver */
            if (0U == hObj->numPendReq)
            {
                /* Nothing is queued */
                GT_0trace(VhwaFcTrace, GT_DEBUG,
                    "Nothing to dequeue. No request pending with driver!!\r\n");
                status = FVID2_ENO_MORE_BUFFERS;
            }
            else
            {
                /* If no request have completed, return try again */
                GT_0trace(VhwaFcTrace, GT_DEBUG,
                    "Nothing to dequeue. Try again!!\r\n");
                status = FVID2_EAGAIN;
            }
        }
        else /* OutQ has buffer to be returned */
        /* LDRA_JUSTIFY_END */
        {
           	if(UTRUE == hObj->modInfo.isVissEnabled)
            {
                status = Fvid2_getProcessedRequest(hObj->modHdls.vissHandle,
                                            &hObj->frameList.vissInFrameList,
                                            &hObj->frameList.vissOutFrameList,
                                            timeout);
            }
			/* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
            if((UTRUE == hObj->modInfo.isMsc0Enabled) && (FVID2_SOK == status))
			/* LDRA_JUSTIFY_END */
            {
                status = Fvid2_getProcessedRequest(hObj->modHdls.msc0Handle,
                                            &hObj->frameList.msc0InFrameList,
                                            &hObj->frameList.msc0OutFrameList,
                                            timeout);
            }
			/* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
            if((UTRUE == hObj->modInfo.isMsc1Enabled) && (FVID2_SOK == status))
            {
			/* LDRA_JUSTIFY_END */
                status = Fvid2_getProcessedRequest(hObj->modHdls.msc1Handle,
                                            &hObj->frameList.msc1InFrameList,
                                            &hObj->frameList.msc1OutFrameList,
                                            timeout);
            }

            Fvid2_copyFrameList(inFrmList, &qObj->inFrmList);
            Fvid2_copyFrameList(outFrmList, &qObj->outFrmList);

            /* Return back the queue object to the free queue,
             * No need to protect from ISR, as it is not accessed in ISR */
            Fvid2Utils_queue(instObj->freeQ, &qObj->qElem, qObj);

            /* Decrement the pending request count. */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
            if (0U != hObj->numPendReq)
            /* LDRA_JUSTIFY_END */
            {
                hObj->numPendReq--;
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
                else
            {
                GT_0trace(VhwaFcTrace, GT_DEBUG,
                    "Done queue of the handle is not empty when there are is no pending request on the handle!!\r\n");
                status = FVID2_EFAIL;
            }
            /* LDRA_JUSTIFY_END */
        }

        (void)VhwaAl_SemaphoreP_post(instObj->lock);
    }

    return (status);
}


/* TODO Allocate if not allocated */
int32_t Vhwa_m2mFcAllocSl2(const Vhwa_M2mFcSl2AllocPrms *sl2AllocPrms)
{
    int32_t retVal = FVID2_SOK;
    Vhwa_M2mFcDrvInstObj *instObj = NULL;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if(NULL == sl2AllocPrms)
    {
        GT_0trace(VhwaFcTrace, GT_ERR, "Bad Arguments\n");
        retVal = FVID2_EBADARGS;
    }

    if(FVID2_SOK == retVal)
    /* LDRA_JUSTIFY_END */
    {
        instObj = &gFcInstObj;

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application. This behaviour is part of the application design. An error print statement can be added in a future release if required.
        <justification end> */
        if(UTRUE != instObj->isSl2AllocDone)
        /* LDRA_JUSTIFY_END */
        {
            Fvid2Utils_memcpy(&instObj->sl2Prms, sl2AllocPrms,
                                sizeof(Vhwa_M2mFcSl2AllocPrms));

			retVal = Vhwa_m2mMscAllocSl2(&sl2AllocPrms->mscSl2Prms);

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached. 
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
            <justification end> */
            if(FVID2_SOK == retVal)
            /* LDRA_JUSTIFY_END */
            {
                retVal = Vhwa_m2mVissAllocSl2(&sl2AllocPrms->vissSl2Prms);
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
            Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations. However, due to the stated rationale, this is not tested.
            <justification end> */
            if(FVID2_SOK != retVal)
            {
                (void)Vhwa_m2mMscFreeSl2();
                Vhwa_m2mVissFreeSl2();
            }
            else
            /* LDRA_JUSTIFY_END */
            {
                instObj->isSl2AllocDone = UTRUE;
            }
        }
    }

    return retVal;
}


void Vhwa_m2mFcFreeSl2(void)
{
    Vhwa_M2mFcDrvInstObj *instObj = &gFcInstObj;

    if(UTRUE == instObj->isSl2AllocDone)
    {
        (void)Vhwa_m2mMscFreeSl2();
        Vhwa_m2mVissFreeSl2();

        instObj->isSl2AllocDone = UFALSE;
    }
}

/* ========================================================================== */
/*                           Local Functions                                  */
/* ========================================================================== */
static Vhwa_M2mFcHandleObj *Vhwa_m2mFcDrvAllocHdlObj(const Vhwa_M2mFcDrvInstObj *instObj)
{
    uint32_t cnt;
    Vhwa_M2mFcHandleObj *hObj = NULL;

    /* Check for Null pointer */
    GT_assert(VhwaFcTrace, (NULL != instObj));

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application. This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
    <justification end> */
    for (cnt = 0U; cnt < VHWA_FC_DRV_MAX_HANDLES; cnt ++)
    /* LDRA_JUSTIFY_END */
    {
        if ((uint32_t)UFALSE == gFcHandleObj[cnt].isUsed)
        {
            /* Allocate Handle Object */
            hObj = &gFcHandleObj[cnt];
            Fvid2Utils_memset(hObj, 0x0, sizeof(Vhwa_M2mFcHandleObj));
            gFcHandleObj[cnt].isUsed = (uint32_t)UTRUE;
            hObj->hIdx = cnt;

            break;
        }
    }

    return (hObj);
}

static int32_t Vhwa_m2mFcCreateQueue(Vhwa_M2mFcDrvInstObj *instObj)
{
    int32_t retVal = FVID2_SOK;
    uint32_t qCnt;
    Vhwa_M2mFcQueueObj *qObj;

    /* NULL pointer check */
    GT_assert(VhwaFcTrace, (NULL != instObj));

    instObj->actQObj = NULL;
    instObj->freeQ = NULL;

    /* Create Free queue */
    retVal = Fvid2Utils_constructQ(&instObj->freeQObj);
    GT_assert(VhwaFcTrace, (retVal == FVID2_SOK));

    instObj->freeQ = &instObj->freeQObj;


    /* Initialize and queue the allocate queue object to free Q */
    for(qCnt=0U; qCnt < VHWA_FC_DRV_QUEUE_ENTRIES; qCnt++)
    {
        qObj = &instObj->fcQObj[qCnt];
        qObj->hObj = NULL;
        Fvid2Utils_queue(instObj->freeQ, &qObj->qElem, qObj);
    }

    return retVal;
}

static int32_t Vhwa_m2mFcDeleteQueue(Vhwa_M2mFcDrvInstObj *instObj)
{
    int32_t retVal = FVID2_SOK;
    Vhwa_M2mFcQueueObj *qObj;

    /* NULL pointer check */
    GT_assert(VhwaFcTrace, (NULL != instObj));

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application. This behaviour is part of the application design. An error print statement can be added in a future release if required.
    <justification end> */
    if(NULL != instObj->freeQ)
    /* LDRA_JUSTIFY_END */
    {
        /* Free-up all the queued free queue objects */
        do
        {
            qObj = (Vhwa_M2mFcQueueObj *) Fvid2Utils_dequeue(instObj->freeQ);
        } while (NULL != qObj);

        /* Delete the free Q */
        Fvid2Utils_destructQ(instObj->freeQ);
        instObj->freeQ = NULL;
    }

    return (retVal);
}

static void Vhwa_m2mFcDrvFreeHandleObj(Vhwa_M2mFcHandleObj *hObj)
{
    uint32_t cnt;

    /* Check for Null pointer */
    GT_assert(VhwaFcTrace, (NULL != hObj));

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application. This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
    <justification end> */
    for (cnt = 0U; cnt < VHWA_FC_DRV_MAX_HANDLES; cnt ++)
    /* LDRA_JUSTIFY_END */
    {
        /* Freeup allocated Handle Object */
        if (hObj == &gFcHandleObj[cnt])
        {
            hObj->isUsed = (uint32_t)UFALSE;;
            break;
        }
    }

}

static uint64_t Vhwa_getSrcNode(uint64_t startPort)
{
    uint64_t srcNode = VHWA_FC_NODE_NONE;

    if((((uint64_t)VHWA_FC_PORT_VISS_OUT_Y12) == startPort) ||
       (((uint64_t)VHWA_FC_PORT_VISS_OUT_UV12) == startPort) ||
       (((uint64_t)VHWA_FC_PORT_VISS_OUT_Y8) == startPort) ||
       (((uint64_t)VHWA_FC_PORT_VISS_OUT_UV8) == startPort) ||
       (((uint64_t)VHWA_FC_PORT_VISS_OUT_S8) == startPort))
    {
        srcNode = VHWA_FC_NODE_VISS;
    }
    else if((((uint64_t)VHWA_FC_PORT_MSC0_OUT_0) <= startPort) &&
       (((uint64_t)VHWA_FC_PORT_MSC0_OUT_9) >= startPort))
    {
        srcNode = VHWA_FC_NODE_MSC0;
    }
    else if((((uint64_t)VHWA_FC_PORT_MSC1_OUT_0) <= startPort) &&
       (((uint64_t)VHWA_FC_PORT_MSC1_OUT_9) >= startPort))
    {
        srcNode = VHWA_FC_NODE_MSC1;
    }
    else if(((uint64_t)VHWA_FC_PORT_DDR) == startPort)
    {
        srcNode = VHWA_FC_NODE_DDR;
    }
    else
    {
       /*Do Nothing*/
    }

    return srcNode;
}

static uint64_t Vhwa_getDesNode(uint64_t endPort)
{
    uint64_t desNode = VHWA_FC_NODE_NONE;

    if((((uint64_t)VHWA_FC_PORT_VISS_IN_0) == endPort) ||
       (((uint64_t)VHWA_FC_PORT_VISS_IN_1) == endPort) ||
       (((uint64_t)VHWA_FC_PORT_VISS_IN_2) == endPort))
    {
        desNode = VHWA_FC_NODE_VISS;
    }
    else if((((uint64_t)VHWA_FC_PORT_MSC0_IN_0) == endPort) ||
            (((uint64_t)VHWA_FC_PORT_MSC0_IN_1) == endPort))
    {
        desNode = VHWA_FC_NODE_MSC0;
    }
    else if((((uint64_t)VHWA_FC_PORT_MSC1_IN_0) == endPort) ||
            (((uint64_t)VHWA_FC_PORT_MSC1_IN_1) == endPort))
    {
        desNode = VHWA_FC_NODE_MSC1;
    }
    else if(((uint64_t)VHWA_FC_PORT_DDR) == endPort)
    {
        desNode = VHWA_FC_NODE_DDR;
    }
    else
    {
      /*Do Nothing*/
    }

    return desNode;
}

static void Vhwa_m2mFcInitGraph(Vhwa_M2mFcGraphObj *newGraphObj)
{
    const Vhwa_M2mFcGraphNodeInfo *graphNodeInfo;
    uint32_t graphNodeInfoSize = 0U;

    graphNodeInfo = Vhwa_getDefaultNodeInfo(&graphNodeInfoSize);

    Fvid2Utils_memcpy(newGraphObj->graphNodeObj,
                      graphNodeInfo,
                      graphNodeInfoSize);
    newGraphObj->graphObjNodeList.list = newGraphObj->graphNodeObj;


    newGraphObj->graphInfo.nodeList = &newGraphObj->graphObjNodeList;

    newGraphObj->graph = &newGraphObj->graphInfo;
}

static int32_t Vhwa_m2mFcParseGraph(const Vhwa_M2mFcDrvInstObj *instObj, Vhwa_M2mFcHandleObj *hObj,
                             const Vhwa_M2mFcGraphPathInfo *pathInfo)
{
    int32_t status = FVID2_SOK;
    uint32_t idx, idx2, idx3, sNodeIdx, dNodeIdx, tNodeIdx;
    uint32_t srcNodeFound = UFALSE;
    Vhwa_M2mFcGraphObj *pGphObj = NULL;
    uint64_t desNode, srcNode;
    uint32_t  linkStatus = UFALSE;

    GT_assert(VhwaFcTrace, (NULL != instObj));
    GT_assert(VhwaFcTrace, (NULL != hObj));
    GT_assert(VhwaFcTrace, (NULL != pathInfo));

    pGphObj = &hObj->graphObj;

    Fvid2Utils_memcpy(pGphObj->graphEdgeObj, pathInfo->edgeInfo,
                      (pathInfo->numEdges * sizeof(Vhwa_M2mFcEdgeInfo)));

    pGphObj->numEdges = pathInfo->numEdges;

    hObj->lastNode = -1;
    hObj->firstNode = -1;

    for(idx = 0; idx < pathInfo->numEdges; idx++)
    {
        /* Set the input Node information */
        sNodeIdx = 0;
        tNodeIdx = 0;
        srcNodeFound = UFALSE;

        desNode = Vhwa_getDesNode(pathInfo->edgeInfo[idx].endPort);
        srcNode = Vhwa_getSrcNode(pathInfo->edgeInfo[idx].startPort);

        if(desNode == srcNode)
        {
            status = FVID2_EBADARGS;
        }

        /* Find the Source Node */
        while(VHWA_FC_NODE_NONE != pGphObj->graphNodeObj[tNodeIdx].nodeId)
        {
            if(srcNode == pGphObj->graphNodeObj[tNodeIdx].nodeId)
            {
                sNodeIdx = tNodeIdx;
                srcNodeFound = UTRUE;
            }
            tNodeIdx++;
        }

        /* If source node not found, use empty slot */
        if(UFALSE == srcNodeFound)
        {
            sNodeIdx = tNodeIdx;
        }

        if(VHWA_FC_NODE_NONE == pGphObj->graphNodeObj[sNodeIdx].nodeId)
        {
            /* Node not found create node entry */
            pGphObj->graphNodeObj[sNodeIdx].nodeId = (uint32_t)srcNode;
            if((VHWA_FC_NODE_VISS == srcNode) ||
               (VHWA_FC_NODE_MSC0 == srcNode) ||
               (VHWA_FC_NODE_MSC1 == srcNode))
            {
                pGphObj->graphNodeObj[sNodeIdx].nodeType =
                                                VHWA_FC_NODE_TYPE_VHWA_HA;
                if(-1 == hObj->firstNode)
                {
                    hObj->firstNode = (int32_t)srcNode;
                }
            }
            else
            {
                pGphObj->graphNodeObj[sNodeIdx].nodeType =
                                                VHWA_FC_NODE_TYPE_VHWA_MEM;
            }

            pGphObj->graphNodeObj[sNodeIdx].inUse = UTRUE;
        }

        dNodeIdx = sNodeIdx;

        /* Find the Destination node */
        while((VHWA_FC_NODE_NONE != (pGphObj->graphNodeObj[dNodeIdx].nodeId)) &&
              (desNode != (pGphObj->graphNodeObj[dNodeIdx].nodeId)))
        {
            dNodeIdx++;
        }

        if(VHWA_FC_NODE_NONE == pGphObj->graphNodeObj[dNodeIdx].nodeId)
        {
            /* Node not found create node entry */
            pGphObj->graphNodeObj[dNodeIdx].nodeId = (uint32_t)desNode;
            if((VHWA_FC_NODE_VISS == desNode) ||
               (VHWA_FC_NODE_MSC0 == desNode) ||
               (VHWA_FC_NODE_MSC1 == desNode))
            {
                pGphObj->graphNodeObj[dNodeIdx].nodeType = VHWA_FC_NODE_TYPE_VHWA_HA;
                hObj->lastNode = (int32_t)desNode;
                if(-1 == hObj->firstNode)
                {
                    hObj->firstNode = (int32_t)desNode;
                }
            }
            else
            {
                pGphObj->graphNodeObj[dNodeIdx].nodeType = VHWA_FC_NODE_TYPE_VHWA_MEM;
            }

            pGphObj->graphNodeObj[dNodeIdx].inUse = UTRUE;
        }

        /* Link to Source -> Destination nodes */
        idx3 = 0;
        linkStatus=UFALSE;
        for(idx2 = 0; idx2 < VHWA_GRAPH_MAX_NUM_PATHS; idx2++)
        {
            if(NULL == pGphObj->graphNodeObj[sNodeIdx].outputNodeSet.node[idx2])
            {
                pGphObj->graphNodeObj[sNodeIdx].outputNodeSet.node[idx2] =
                                            &pGphObj->graphNodeObj[dNodeIdx];

                pGphObj->graphNodeObj[sNodeIdx].outputNodeSet.numNodes++;

                pGphObj->graphNodeObj[sNodeIdx].outputNodeSet.edgeInfo[idx2][0] =
                                            &pGphObj->graphEdgeObj[idx];
                pGphObj->graphNodeObj[sNodeIdx].outputNodeSet.isEnabled[idx2][0] = UTRUE;

                linkStatus=UTRUE;
            }
            else if(desNode ==
                    pGphObj->graphNodeObj[sNodeIdx].outputNodeSet.node[idx2]->nodeId)
            {
                while(UTRUE ==
                pGphObj->graphNodeObj[sNodeIdx].outputNodeSet.isEnabled[idx2][idx3])
                {
                    idx3++;
                }

                pGphObj->graphNodeObj[sNodeIdx].outputNodeSet.edgeInfo[idx2][idx3] =
                                            &pGphObj->graphEdgeObj[idx];
                pGphObj->graphNodeObj[sNodeIdx].outputNodeSet.isEnabled[idx2][idx3] = UTRUE;

                linkStatus=UTRUE;
            }
            else
            {
                /*Do Nothing*/
            }
            if(linkStatus == UTRUE)
            {
                break;
            }
        }

        /* Link to Destination -> Source link */
        idx3 = 0;
        linkStatus=UFALSE;
        for(idx2 = 0; idx2 < VHWA_GRAPH_MAX_NUM_PATHS; idx2++)
        {
            if(NULL == pGphObj->graphNodeObj[dNodeIdx].inputNodeSet.node[idx2])
            {
                pGphObj->graphNodeObj[dNodeIdx].inputNodeSet.node[idx2] =
                                            &pGphObj->graphNodeObj[sNodeIdx];

                pGphObj->graphNodeObj[dNodeIdx].inputNodeSet.numNodes++;

                pGphObj->graphNodeObj[dNodeIdx].inputNodeSet.edgeInfo[idx2][0] =
                                            &pGphObj->graphEdgeObj[idx];
                pGphObj->graphNodeObj[dNodeIdx].inputNodeSet.isEnabled[idx2][0] = UTRUE;

                linkStatus=UTRUE;
            }
            else if(desNode ==
                    pGphObj->graphNodeObj[dNodeIdx].inputNodeSet.node[idx2]->nodeId)
            {
                while(UTRUE ==
                pGphObj->graphNodeObj[dNodeIdx].inputNodeSet.isEnabled[idx2][idx3])
                {
                    idx3++;
                }

                pGphObj->graphNodeObj[dNodeIdx].inputNodeSet.edgeInfo[idx2][idx3] =
                                            &pGphObj->graphEdgeObj[idx];
                pGphObj->graphNodeObj[dNodeIdx].inputNodeSet.isEnabled[idx2][idx3] = UTRUE;

                linkStatus=UTRUE;
            }
           else
            {
                /*Do Nothing*/
            }
            if(linkStatus == UTRUE)
            {
                break;
            }
        }
    }

    return status;
}

static uint32_t Vhwa_m2mFcGetHtsPipeline(const Vhwa_M2mFcHandleObj *hObj)
{
    uint32_t pipeline=0;

    switch ((uint32_t)hObj->firstNode)
    {
        case VHWA_FC_NODE_VISS:
        {
            pipeline = (uint32_t)VHWA_M2M_SP0_HTS_PIPELINE;
        }
        break;
        case VHWA_FC_NODE_MSC0:
        {
            pipeline = (uint32_t)VHWA_M2M_MSC0_HTS_PIPELINE;
        }
        break;
        case VHWA_FC_NODE_MSC1:
        {
            pipeline = (uint32_t)VHWA_M2M_MSC1_HTS_PIPELINE;
        }
        break;
        default:
           break;
    }

    return pipeline;
}

static int32_t Vhwa_m2mFcCreateHndls(Vhwa_M2mFcDrvInstObj *instObj,
                                            Vhwa_M2mFcHandleObj *hObj)
{
    int32_t status = FVID2_SOK;
    uint32_t idx, htsPipeline;
    Vhwa_M2mVissCreateArgs vissCreatePrms;
    Vhwa_M2mMscCreatePrms  mscCreatePrms;
    uint32_t             instId = 0U;

    Fvid2_CbParams cbPrms;

    htsPipeline = Vhwa_m2mFcGetHtsPipeline(hObj);

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if(VHWA_M2M_PIPELINE_INVALID == htsPipeline)
    {
        status = FVID2_EBADARGS;
    }

    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        for (idx = 0; idx < VHWA_FC_MAX_NODES; idx++)
        {
            if (FVID2_SOK != status)
            {
                break;
            }
            if (VHWA_FC_NODE_VISS == hObj->graphObj.graphNodeObj[idx].nodeId)
            {
                Vhwa_m2mVissUpdatePipeline(htsPipeline);

                Vhwa_m2mVissCreateArgsInit(&vissCreatePrms);
                cbPrms.cbFxn = Vhwa_m2mVissFrameComplCb;
                cbPrms.appData = instObj;
                vissCreatePrms.getTimeStamp = NULL;
                vissCreatePrms.enablePsa = hObj->createPrms.enablePsa;
                #if defined(VHWA_M2M_VPAC_INSTANCE)
                    #if (VHWA_M2M_VPAC_INSTANCE == 0)
                            instId = VHWA_M2M_VPAC_0_VISS_DRV_INST_ID_0;
                    #endif
                    #if (VHWA_M2M_VPAC_INSTANCE == 1)
                            instId = VHWA_M2M_VPAC_1_VISS_DRV_INST_ID_0;
                    #endif
                #endif

                hObj->modHdls.vissHandle = Fvid2_create(FVID2_VHWA_M2M_VISS_DRV_ID,
                    instId, (void *)&vissCreatePrms,
                    NULL, &cbPrms);
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
                Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application. This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                <justification end> */
                if(NULL == hObj->modHdls.vissHandle)
                {
                    status = FVID2_EFAIL;
                }
                else
                /* LDRA_JUSTIFY_END */
                {
                    hObj->modInfo.isVissEnabled = UTRUE;
                    hObj->vissFcPrms.isFlexConnect = UTRUE;
                }
            }
            else if (VHWA_FC_NODE_MSC0 == hObj->graphObj.graphNodeObj[idx].nodeId)
            {
                Vhwa_m2mMscUpdatePipeline(VPAC_MSC_INST_ID_0, htsPipeline);

                Vhwa_M2mMscCreatePrmsInit(&mscCreatePrms);
                cbPrms.cbFxn = Vhwa_m2mMsc0FrameComplCb;
                cbPrms.appData = instObj;
                mscCreatePrms.getTimeStamp = NULL;
                #if defined(VHWA_M2M_VPAC_INSTANCE)
                    #if (VHWA_M2M_VPAC_INSTANCE == 0)
                        instId = VHWA_M2M_VPAC_0_MSC_DRV_INST_ID_0;
                    #endif
                    #if (VHWA_M2M_VPAC_INSTANCE == 1)
                        instId = VHWA_M2M_VPAC_1_MSC_DRV_INST_ID_0;
                    #endif
                #endif

                hObj->modHdls.msc0Handle = Fvid2_create(FVID2_VHWA_M2M_MSC_DRV_ID,
                                instId,
                                &mscCreatePrms,
                                NULL,
                                &cbPrms);
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                <justification end> */
                if(NULL == hObj->modHdls.msc0Handle)
                {
                    status = FVID2_EFAIL;
                }
                else
                /* LDRA_JUSTIFY_END */
                {
                    hObj->modInfo.isMsc0Enabled = UTRUE;
                    hObj->msc0FcPrms.isFlexConnect = UTRUE;
                }
            }
            else if (VHWA_FC_NODE_MSC1 == hObj->graphObj.graphNodeObj[idx].nodeId)
            {
                Vhwa_m2mMscUpdatePipeline(VPAC_MSC_INST_ID_1, htsPipeline);

                Vhwa_M2mMscCreatePrmsInit(&mscCreatePrms);
                cbPrms.cbFxn = Vhwa_m2mMsc1FrameComplCb;
                cbPrms.appData = instObj;
                mscCreatePrms.getTimeStamp = NULL;

                #if defined(VHWA_M2M_VPAC_INSTANCE)
                    #if (VHWA_M2M_VPAC_INSTANCE == 0)
                        instId = VHWA_M2M_VPAC_0_MSC_DRV_INST_ID_1;
                    #endif
                    #if (VHWA_M2M_VPAC_INSTANCE == 1)
                        instId = VHWA_M2M_VPAC_1_MSC_DRV_INST_ID_1;
                    #endif
                #endif

                hObj->modHdls.msc1Handle = Fvid2_create(FVID2_VHWA_M2M_MSC_DRV_ID,
                                instId,
                                &mscCreatePrms,
                                NULL,
                                &cbPrms);
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
                <justification end> */
                if(NULL == hObj->modHdls.msc1Handle)
                {
                    status = FVID2_EFAIL;
                }
                else
                /* LDRA_JUSTIFY_END */
                {
                    hObj->modInfo.isMsc1Enabled = UTRUE;
                    hObj->msc1FcPrms.isFlexConnect = UTRUE;
                }
            }
			else
			{
				/*Do Nothing*/
			}
        }
    }

    return status;
}

static void Vhwa_m2mFcDeleteHndls(Vhwa_M2mFcDrvInstObj *instObj,
                                            Vhwa_M2mFcHandleObj *hObj)
{
    /* Delete All driver handles */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application. This behaviour is part of the application design. An error print statement can be added in a future release if required.
    <justification end> */
    if (NULL != hObj->modHdls.vissHandle)
    /* LDRA_JUSTIFY_END */
    {
        (void)Fvid2_delete(hObj->modHdls.vissHandle, NULL);
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != hObj->modHdls.msc0Handle)
    /* LDRA_JUSTIFY_END */
    {
        (void)Fvid2_delete(hObj->modHdls.msc0Handle, NULL);
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != hObj->modHdls.msc1Handle)
    /* LDRA_JUSTIFY_END */
    {
        (void)Fvid2_delete(hObj->modHdls.msc1Handle, NULL);
    }

}

static int32_t Vhwa_m2mFcGetPrms(Vhwa_M2mFcHandleObj *hObj)
{
    int32_t status = FVID2_SOK;
    Vhwa_m2mFcDrvSl2ResObj   *pSl2ResObj = NULL;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application. This behaviour is part of the application design. An error print statement can be added in a future release if required.
    <justification end> */
    if(NULL != hObj)
    /* LDRA_JUSTIFY_END */
    {
        pSl2ResObj = &hObj->modInfo.sl2ResObj;

         /* Get Sl2 Parameters */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application. This behaviour is part of the application design. An error print statement can be added in a future release if required.
        <justification end> */
        if (NULL != hObj->modHdls.msc0Handle)
        /* LDRA_JUSTIFY_END */
        {
            status += Fvid2_control(hObj->modHdls.msc0Handle,
                          IOCTL_VHWA_MSC_GET_SL2_PARAMS,
                          &pSl2ResObj->mscSl2Prms, NULL);

            status += Fvid2_control(hObj->modHdls.msc0Handle,
                          IOCTL_VHWA_MSC_FC_GET_PARAMS,
                          &hObj->msc0FcGetPrms, NULL);
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != hObj->modHdls.msc1Handle)
        /* LDRA_JUSTIFY_END */
        {
            status += Fvid2_control(hObj->modHdls.msc1Handle,
                          IOCTL_VHWA_MSC_GET_SL2_PARAMS,
                          &pSl2ResObj->mscSl2Prms, NULL);

            status += Fvid2_control(hObj->modHdls.msc1Handle,
                          IOCTL_VHWA_MSC_FC_GET_PARAMS,
                          &hObj->msc1FcGetPrms, NULL);
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
        <justification end> */
        if(NULL != hObj->modHdls.vissHandle)
        /* LDRA_JUSTIFY_END */
        {
            status += Fvid2_control(hObj->modHdls.vissHandle,
                          IOCTL_VHWA_VISS_GET_SL2_PARAMS,
                          &pSl2ResObj->vissSl2Prms, NULL);
        }
    }

    return status;
}

static int32_t Vhwa_m2mFcUpdatePrms(Vhwa_M2mFcHandleObj *hObj)
{
    int32_t status = FVID2_SOK;

    if(UTRUE == hObj->modInfo.isVissEnabled)
    {
        status += Fvid2_control(hObj->modHdls.vissHandle,
                            IOCTL_VHWA_VISS_FC_UPDATE_PARAMS,
                            &hObj->vissFcPrms, NULL);
    }
    if(UTRUE == hObj->modInfo.isMsc0Enabled)
    {
        status += Fvid2_control(hObj->modHdls.msc0Handle,
                            IOCTL_VHWA_MSC_UPDATE_FC_PARAMS,
                            &hObj->msc0FcPrms, NULL);
    }
    if(UTRUE == hObj->modInfo.isMsc1Enabled)
    {
        status += Fvid2_control(hObj->modHdls.msc1Handle,
                            IOCTL_VHWA_MSC_UPDATE_FC_PARAMS,
                            &hObj->msc1FcPrms, NULL);
    }

    return status;
}

static int32_t Vhwa_m2mFcConfigHts(Vhwa_M2mFcHandleObj *hObj)
{
    uint32_t idx;
    int32_t status = FVID2_SOK;
    Vhwa_M2mFcGraphObj *pGphObj = NULL;

    pGphObj = &hObj->graphObj;

    status = Vhwa_m2mFcGetPrms(hObj);

    for (idx = 0; idx < VHWA_FC_MAX_NODE_INST; idx++)
    {
        if(UTRUE == pGphObj->graphNodeObj[idx].inUse)
        {
            if(VHWA_FC_NODE_VISS == pGphObj->graphNodeObj[idx].nodeId)
            {
                status += Vhwa_m2mFcDrvConfigVissSch(hObj,
                                                &pGphObj->graphNodeObj[idx]);
            }
            else if((VHWA_FC_NODE_MSC0 == pGphObj->graphNodeObj[idx].nodeId) ||
                    (VHWA_FC_NODE_MSC1 == pGphObj->graphNodeObj[idx].nodeId))
            {
                status += Vhwa_m2mFcDrvConfigMscSch(hObj, &pGphObj->graphNodeObj[idx]);
            }
            else
            {
                /*Do Nothing*/
            }
        }
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
        status = Vhwa_m2mFcUpdatePrms(hObj);
    }

    return status;
}

static void Vhwa_m2mFcPrepareFrmList(const Vhwa_M2mFcQueueObj *qObj,
                                        const Fvid2_FrameList *inFrmList,
                                        const Fvid2_FrameList *outFrmList)
{
    uint32_t cnt;
    Vhwa_M2mFcHandleObj *hObj = NULL;

    GT_assert(VhwaFcTrace, (NULL != qObj));

    hObj = qObj->hObj;

    if(UTRUE == hObj->modInfo.isVissEnabled)
    {
        /* Frame list for VISS */
        for (cnt = 0; cnt < VHWA_M2M_VISS_MAX_INPUTS; cnt++)
        {
            hObj->frameList.vissInFrameList.frames[cnt] =
                    inFrmList->frames[VHWA_FC_VISS_SRC_BUFF_IDX_START + cnt];
        }
        for (cnt = 0; cnt < VHWA_M2M_VISS_MAX_OUTPUTS; cnt++)
        {
            hObj->frameList.vissOutFrameList.frames[cnt] =
                    outFrmList->frames[VHWA_FC_VISS_DST_BUFF_IDX_START + cnt];
        }
    }
    if(UTRUE == hObj->modInfo.isMsc0Enabled)
    {
        /* Frame list for MSC0 */
        hObj->frameList.msc0InFrameList.frames[0] =
                    inFrmList->frames[VHWA_FC_MSC0_SRC_BUFF_IDX_START];
        hObj->frameList.msc0InFrameList.numFrames = 1u;

        for (cnt = 0; cnt < MSC_MAX_OUTPUT; cnt++)
        {
            hObj->frameList.msc0OutFrameList.frames[cnt] =
                    outFrmList->frames[VHWA_FC_MSC0_DST_BUFF_IDX_START+cnt];
        }
        hObj->frameList.msc0OutFrameList.numFrames = 10u;
    }
    if(UTRUE == hObj->modInfo.isMsc1Enabled)
    {
        /* Frame list for MSC1 */
        hObj->frameList.msc1InFrameList.frames[0] =
                    inFrmList->frames[VHWA_FC_MSC1_SRC_BUFF_IDX_START];
        hObj->frameList.msc1InFrameList.numFrames = 1u;

        for (cnt = 0; cnt < MSC_MAX_OUTPUT; cnt++)
        {
            hObj->frameList.msc1OutFrameList.frames[cnt] =
                    outFrmList->frames[VHWA_FC_MSC1_DST_BUFF_IDX_START+cnt];
        }
        hObj->frameList.msc1OutFrameList.numFrames = 10u;
    }
}

static int32_t Vhwa_m2mFcConfigAndStart(Vhwa_M2mFcDrvInstObj *instObj,
                                            Vhwa_M2mFcQueueObj *qObj,
                                            uint32_t timeout)
{
    int32_t  status = FVID2_SOK;
    Vhwa_M2mFcHandleObj *hObj = NULL;

    hObj = qObj->hObj;

    if(UTRUE == hObj->modInfo.isVissEnabled)
    {
        status = Fvid2_processRequest(hObj->modHdls.vissHandle,
                                    &hObj->frameList.vissInFrameList,
                                    &hObj->frameList.vissOutFrameList,
                                    timeout);
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached. 
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if((UTRUE == hObj->modInfo.isMsc0Enabled) && (FVID2_SOK == status))
    /* LDRA_JUSTIFY_END */
    {
        status = Fvid2_processRequest(hObj->modHdls.msc0Handle,
                                    &hObj->frameList.msc0InFrameList,
                                    &hObj->frameList.msc0OutFrameList,
                                    timeout);
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached. 
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if((UTRUE == hObj->modInfo.isMsc1Enabled) && (FVID2_SOK == status))
    /* LDRA_JUSTIFY_END */
    {
        status = Fvid2_processRequest(hObj->modHdls.msc1Handle,
                                    &hObj->frameList.msc1InFrameList,
                                    &hObj->frameList.msc1OutFrameList,
                                    timeout);
    }

    instObj->actQObj = qObj;

    instObj->totalReqCnt ++;
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. The test framework does not support the configuration required to trigger this error scenario. 
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application. This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != hObj->createPrms.getTimeStamp)
    {
        hObj->perfNum = hObj->createPrms.getTimeStamp();
    }
    /* LDRA_JUSTIFY_END */

    /* Start the pipeline */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached. 
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
    <justification end> */
    if(FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        if(UTRUE == hObj->modInfo.isVissEnabled)
        {
            status = Fvid2_control(hObj->modHdls.vissHandle,
                                IOCTL_VHWA_M2M_VISS_SYNC_START,
                                NULL, NULL);
        }
        else if(UTRUE == hObj->modInfo.isMsc0Enabled)
        {
            status = Fvid2_control(hObj->modHdls.msc0Handle,
                                VHWA_M2M_IOCTL_MSC_SYNC_START,
                                NULL, NULL);
        }
        else if(UTRUE == hObj->modInfo.isMsc1Enabled)
        {
            status = Fvid2_control(hObj->modHdls.msc1Handle,
                                VHWA_M2M_IOCTL_MSC_SYNC_START,
                                NULL, NULL);
        }
        else
        {
          /*Do Nothing*/
        }
    }

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion. This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached. 
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application. However, due to the stated rationale, this is not tested.
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

    return status;
}
