/**
 *   Copyright (c) Texas Instruments Incorporated 2025
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
 *  \file vhwa_m2mBistDrv.c
 *
 *  \brief API Implementation for the M2M Bist VHWA driver, used mainly for running LBIST or PBIST
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <vhwa_dmpac_abstraction_layer.h>
#include <include/vhwa_common.h>
#include <include/vhwa_m2mBist.h>
#include "src/drv/vhwa_utils.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */


#define VHWA_M2M_BIST_VPAC0_UTC0_IDX               (0)
#define VHWA_M2M_BIST_VPAC0_UTC1_IDX               (1)
#define VHWA_M2M_BIST_VPAC1_UTC0_IDX               (2)
#define VHWA_M2M_BIST_VPAC1_UTC1_IDX               (3)
#define VHWA_M2M_BIST_DMPAC_UTC_IDX                (4)

#if defined(VHWA_M2M_VPAC_INSTANCE)
    #if (VHWA_M2M_VPAC_INSTANCE == 0)
        #define VHWA_M2M_BIST_VPAC0_UTC_BASE_ADDR          (CSL_VPAC0_DRU_UTC_VPAC1_DRU_MMR_CFG_DRU_DRU_BASE)
    #endif

    #if (VHWA_M2M_VPAC_INSTANCE == 1)
        #define VHWA_M2M_BIST_VPAC1_UTC_BASE_ADDR          (CSL_VPAC1_DRU_UTC_VPAC1_DRU_MMR_CFG_DRU_DRU_BASE)
    #endif
#endif

#define VHWA_M2M_BIST_DMPAC_UTC_BASE_ADDR          (CSL_DMPAC0_DRU_UTC_DMPAC0_DRU_MMR_CFG_DRU_DRU_BASE)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef struct
{
    uint32_t                        isRegistered;
    VhwaAl_SemaphoreP_Struct        lock;
} Vhwa_M2mBistDrvInstObj;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

Fdrv_Handle Vhwa_m2mBistCreate(UInt32 drvId, UInt32 drvInstId,
    Ptr createArgs, Ptr createStatusArgs, const Fvid2_DrvCbParams *cbPrms);
Int32 Vhwa_m2mBistDelete(Fdrv_Handle handle, Ptr deleteArgs);
Int32 Vhwa_m2mBistControl(Fdrv_Handle handle, UInt32 cmd, Ptr cmdArgs,
    Ptr cmdStatusArgs);


/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

Fvid2_DrvOps gVhwaM2mBistDrvFvid2Ops = {
    FVID2_VHWA_M2M_BIST_DRV_ID,
    /**< Unique driver Id. */
    Vhwa_m2mBistCreate,
    /**< FVID2 create function pointer. */
    Vhwa_m2mBistDelete,
    /**< FVID2 delete function pointer. */
    Vhwa_m2mBistControl,
    /**< FVID2 control function pointer. */
    NULL, NULL,
    /**< FVID2 queue function pointer. */
    NULL,
    /**< FVID2 process request function pointer. */
    NULL,
    /**< FVID2 get processed request function pointer. */
};

Vhwa_M2mBistDrvInstObj gVhwaM2mBistDrvInstObj = {0};


/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t Vhwa_m2mBistInit(void)
{
    int32_t                     status = FVID2_SOK;
    Vhwa_M2mBistDrvInstObj    *instObj = &gVhwaM2mBistDrvInstObj;
    #if !defined(MCU_PLUS_SDK)
    SemaphoreP_Params           params;
    #endif

    status = Fvid2_registerDriver(&gVhwaM2mBistDrvFvid2Ops);

    if (FVID2_SOK == status)
    {
        gVhwaM2mBistDrvInstObj.isRegistered = 1u;
    }

    if (FVID2_SOK == status)
    {
        /* Allocate lock semaphore */
        #if defined(MCU_PLUS_SDK)
        if(FVID2_SOK != SemaphoreP_constructBinary(&instObj->lock,1U))
        #else
        SemaphoreP_Params_init(&params);
        params.mode = SemaphoreP_Mode_BINARY;
        instObj->lock = SemaphoreP_create(1U, &params);
        if (NULL == instObj->lock)
        #endif
        {
            GT_0trace(VhwaM2mBistTrace, GT_ERR,
                "Failed to allocate instance semaphore!!\n");
            status = FVID2_EALLOC;
        }
    }
    return (status);
}

int32_t Vhwa_m2mBistDeInit(void)
{
    if (1u == gVhwaM2mBistDrvInstObj.isRegistered)
    {
        (void)Fvid2_unRegisterDriver(&gVhwaM2mBistDrvFvid2Ops);
        gVhwaM2mBistDrvInstObj.isRegistered = 0u;
    }

    #if defined(MCU_PLUS_SDK)
    SemaphoreP_destruct(&gVhwaM2mBistDrvInstObj.lock);
    #else
    if (NULL != gVhwaM2mBistDrvInstObj.lock)
    {
        (void)SemaphoreP_delete(gVhwaM2mBistDrvInstObj.lock);
        gVhwaM2mBistDrvInstObj.lock = NULL;
    }
    #endif
    return (FVID2_SOK);
}

Fdrv_Handle Vhwa_m2mBistCreate(UInt32 drvId, UInt32 drvInstId,
    Ptr createArgs, Ptr createStatusArgs, const Fvid2_DrvCbParams *cbPrms)
{
    Fdrv_Handle             handle = NULL;

    if ((FVID2_VHWA_M2M_BIST_DRV_ID == drvId) && (0U == drvInstId))
    {
        handle = (Fdrv_Handle)&gVhwaM2mBistDrvInstObj;
    }

    return (handle);
}

Int32 Vhwa_m2mBistDelete(Fdrv_Handle handle, Ptr deleteArgs)
{
    int32_t status = FVID2_EFAIL;

    if ((Vhwa_M2mBistDrvInstObj *)handle == &gVhwaM2mBistDrvInstObj)
    {
        status = FVID2_SOK;
    }

    return (status);
}

Int32 Vhwa_m2mBistControl(Fdrv_Handle handle, UInt32 cmd, Ptr cmdArgs,
    Ptr cmdStatusArgs)
{
    int32_t                     status = FVID2_SOK;
#if !defined(VHWA_VPAC_IP_REV_VPAC3L)
    uint32_t                    cnt;
#endif
    Vhwa_M2mBistDrvInstObj    *instObj = NULL;

    if ((Vhwa_M2mBistDrvInstObj *)handle == &gVhwaM2mBistDrvInstObj)
    {
        instObj =  &gVhwaM2mBistDrvInstObj;
        status = FVID2_SOK;
    }
    else
    {
        status = FVID2_EFAIL;
    }
    if (FVID2_SOK == status)
    {
        (void)VhwaAl_SemaphoreP_pend(instObj->lock, SYSTEMP_WAIT_FOREVER);
        switch (cmd)
        {
#ifdef VHWA_M2M_VPAC_INSTANCE
#if (VHWA_M2M_VPAC_INSTANCE == 0)
#if !defined(VHWA_VPAC_IP_REV_VPAC3L)
            case VHWA_M2M_IOCTL_BIST_VPAC0_ACQUIRE_LOCK:
                for (cnt = VHWA_VPAC0_VISS_LOCK_IDX; cnt <= VHWA_VPAC0_NF_LOCK_IDX; cnt ++)
                {
                    status = Vhwa_commonHwaLockAquire(cnt, SYSTEMP_WAIT_FOREVER);
                    if (FVID2_SOK != status)
                    {
                        GT_1trace(VhwaM2mBistTrace, GT_ERR,
                            "Failed to acquire semaphore for %d!!\n", cnt);
                        break;
                    }
                }
                break;
            case VHWA_M2M_IOCTL_BIST_VPAC0_RELEASE_LOCK:
                /* Before releasing lock, restore the registers of UTC */

                /* Since driver does not support multiple instances of the modules,
                   calling these APIs without any arguments.. */
                status = Vhwa_m2mVissReInit();
                if (FVID2_SOK == status)
                {
                    status = Vhwa_m2mLdcReInit();
                }
                if (FVID2_SOK == status)
                {
                    status = Vhwa_m2mMscReInit();
                }
                #if !defined(SOC_AM62A) && !defined(SOC_J722S)
                if (FVID2_SOK == status)
                {
                    status = Vhwa_m2mNfReInit();
                }
                #endif

                if (FVID2_SOK == status)
                {
                    for (cnt = VHWA_VPAC0_VISS_LOCK_IDX; cnt <= VHWA_VPAC0_NF_LOCK_IDX; cnt ++)
                    {
                        Vhwa_commonHwaLockRelease(cnt);
                    }
                }
                break;
#endif
#endif

#if (VHWA_M2M_VPAC_INSTANCE == 1)
            case VHWA_M2M_IOCTL_BIST_VPAC1_ACQUIRE_LOCK:
                for (cnt = VHWA_VPAC1_VISS_LOCK_IDX; cnt <= VHWA_VPAC1_NF_LOCK_IDX; cnt ++)
                {
                    status = Vhwa_commonHwaLockAquire(cnt, SYSTEMP_WAIT_FOREVER);
                    if (FVID2_SOK != status)
                    {
                        GT_1trace(VhwaM2mBistTrace, GT_ERR,
                            "Failed to acquire semaphore for %d!!\n", cnt);
                        break;
                    }
                }

                break;
            case VHWA_M2M_IOCTL_BIST_VPAC1_RELEASE_LOCK:
                /* Before Releasing the locks restore UTC registers */

                /* Since driver does not support multiple instances of the modules,
                   calling these APIs without any arguments.. */
                status = Vhwa_m2mVissReInit();
                if (FVID2_SOK == status)
                {
                    status = Vhwa_m2mLdcReInit();
                }
                if (FVID2_SOK == status)
                {
                    status = Vhwa_m2mMscReInit();
                }
                #if !defined(SOC_AM62A) && !defined(SOC_J722S)
                if (FVID2_SOK == status)
                {
                    status = Vhwa_m2mNfReInit();
                }
                #endif

                if (FVID2_SOK == status)
                {
                    for (cnt = VHWA_VPAC1_VISS_LOCK_IDX; cnt <= VHWA_VPAC1_NF_LOCK_IDX; cnt ++)
                    {
                        Vhwa_commonHwaLockRelease(cnt);
                    }
                }
                break;
#endif
#endif

#ifdef VHWA_M2M_DMPAC_INSTANCE
#if !defined(VHWA_VPAC_IP_REV_VPAC3L)
            case VHWA_M2M_IOCTL_BIST_DMPAC_ACQUIRE_LOCK:
                for (cnt = VHWA_DMPAC_DOF_LOCK_IDX; cnt <= VHWA_DMPAC_SDE_LOCK_IDX; cnt ++)
                {
                    status = Vhwa_commonHwaLockAquire(cnt, SYSTEMP_WAIT_FOREVER);
                    if (FVID2_SOK != status)
                    {
                        GT_1trace(VhwaM2mBistTrace, GT_ERR,
                            "Failed to acquire semaphore for %d!!\n", cnt);
                        break;
                    }
                }
                break;
            case VHWA_M2M_IOCTL_BIST_DMPAC_RELEASE_LOCK:
                /* Before Releasing the locks restore UTC registers */
                status = Vhwa_m2mDofReInit();
                if (FVID2_SOK == status)
                {
                    status = Vhwa_m2mSdeReInit();
                }

                if (FVID2_SOK == status)
                {
                    for (cnt = VHWA_DMPAC_DOF_LOCK_IDX; cnt <= VHWA_DMPAC_SDE_LOCK_IDX; cnt ++)
                    {
                        Vhwa_commonHwaLockRelease(cnt);
                    }
                }
                break;
#endif
#endif
            default:
                status = FVID2_EINVALID_PARAMS;
                break;
        }
        (void)VhwaAl_SemaphoreP_post(instObj->lock);
    }

    return (status);
}
