/*
 *
 * Copyright (c) 2018 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 *
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 * license under copyrights and patents it now or hereafter owns or controls to make,
 * have made, use, import, offer to sell and sell ("Utilize") this software subject to the
 * terms herein.  With respect to the foregoing patent license, such license is granted
 * solely to the extent that any such patent is necessary to Utilize the software alone.
 * The patent license shall not apply to any combinations which include this software,
 * other than combinations with devices manufactured by or for TI ("TI Devices").
 * No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this license
 * (including the above copyright notice and the disclaimer and (if applicable) source
 * code license limitations below) in the documentation and/or other materials provided
 * with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted provided
 * that the following conditions are met:
 *
 * *       No reverse engineering, decompilation, or disassembly of this software is
 * permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the software
 * licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the
 * source code are permitted provided that the following conditions are met:
 *
 * *       any redistribution and use of the source code, including any resulting derivative
 * works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 * and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers
 *
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include <inttypes.h>
#include <utils/mem/include/app_mem.h>
#include <utils/hwa/include/app_hwa.h>
#include <utils/console_io/include/app_log.h>
#include <utils/remote_service/include/app_remote_service.h>
#include <utils/ipc/include/app_ipc.h>
#include <TI/hwa_viss_srvr_remote.h>

#include <utils/udma/include/app_udma.h>
#include <vhwa/include/vhwa_m2mLdc.h>
#include <vhwa/include/vhwa_m2mMsc.h>
#include <vhwa/include/vhwa_m2mViss.h>
#include <vhwa/include/vhwa_m2mFlexConnect.h>
#if !defined(SOC_AM62A) /* LBIST not applicable for am62a */
#include <vhwa/include/vhwa_m2mLbist.h>
#endif

#if !defined(SOC_AM62A) && !defined(SOC_J722S)
#include <vhwa/include/vhwa_m2mNf.h>
#endif /* !defined(SOC_AM62A) && !defined(SOC_J722S) */

#if !defined(SOC_AM62A)
#include <vhwa/include/vhwa_m2mSde.h>
#include <vhwa/include/vhwa_m2mDof.h>
#endif /* defined(SOC_AM62A) */

#if !defined(MCU_PLUS_SDK)
#include <sciclient/sciclient.h>
#else
#include <SystemP.h>
#define SCICLIENT_SERVICE_WAIT_FOREVER SystemP_WAIT_FOREVER
#endif

#if defined(SOC_J721E) || defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)
#include <app_hwa_firewall.h>
#endif

/* J722S-specific TISCI clock identifier mappings */
#if defined(SOC_J722S)
#define TISCI_DEV_DMPAC0_CLK TISCI_DEV_DMPAC0_DMPAC_PLL_CLK
#endif
#if defined(SOC_J722S) || defined(SOC_AM62A)
#define TISCI_DEV_VPAC0_CLK  TISCI_DEV_VPAC0_VPAC_PLL_CLK
#endif

#define APP_DEBUG_VHWA
#define APP_DEBUG_SCICLIENT 1

/* DOF and SDE is enabled for J7 devices alone.*/
#if defined(SOC_AM62A)
#undef ENABLE_DOF
#undef ENABLE_SDE
#else
#define ENABLE_DOF
#define ENABLE_SDE
#endif

/* Bilateral noise filter block is not available in VPAC3L hardware */
#if !defined(VPAC3L)
#define ENABLE_NF
#endif

#define ENABLE_FC
#define ENABLE_MSC
#define ENABLE_VISS
#define ENABLE_LDC

/* below define's set max limits for line width in order to
 * reserved space in SL2 for VHWA modules during init
 */

#define APP_UTILS_VHWA_MAX_IN_IMG_WIDTH        (1920U)

#define APP_UTILS_VHWA_IN_IMG_CCSF             (FVID2_CCSF_BITS12_UNPACKED16)
#define APP_UTILS_VHWA_MAX_IN_IMG_BUFF_DEPTH   (6)

#define APP_UTILS_VHWA_MAX_OUT_IMG_WIDTH       (1920U)

#define APP_UTILS_VHWA_OUT_IMG_CCSF            (FVID2_CCSF_BITS12_UNPACKED16)
#define APP_UTILS_VHWA_MAX_OUT_IMG_BUFF_DEPTH  (2)

#if defined(VPAC3)
#define APP_UTILS_VHWA_LDC_MAX_BLOCK_WIDTH     (128)
#define APP_UTILS_VHWA_LDC_MAX_BLOCK_HEIGHT    (64)
#elif defined(VPAC3L)
#define APP_UTILS_VHWA_LDC_MAX_BLOCK_WIDTH     (64)
#define APP_UTILS_VHWA_LDC_MAX_BLOCK_HEIGHT    (64)
#elif defined(VPAC1)
#define APP_UTILS_VHWA_LDC_MAX_BLOCK_WIDTH     (192)
#define APP_UTILS_VHWA_LDC_MAX_BLOCK_HEIGHT    (80)
#endif

#if defined(MCU_PLUS_SDK)
static Udma_DrvObjectInt gAppUdmaDrvObj;
#endif

static Fvid2_Handle gFvid2VhwaDrvHandle = NULL;
static int32_t vhwaPowerOnModules(uint32_t instId);

#if defined(SOC_J721E) || defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)
static int32_t appVhwaEnableVpacFirewall(uint32_t vpacInstId);
static int32_t appVhwaEnableDmpacFirewall(void);
static int32_t appVhwaDisableVpacFirewall(uint32_t vpacInstId);
static int32_t appVhwaDisableDmpacFirewall(void);
static int32_t appVhwaEnableVpacFirewallAfterLbist(uint32_t vpacInstId);
static int32_t appVhwaEnableDmpacFirewallAfterLbist(void);
static int32_t appVhwaVpacHandler(char *service_name, uint32_t cmd, void *prm, uint32_t prm_size, uint32_t flags);
#endif
static int32_t appVhwaHandler(char *service_name, uint32_t cmd, void *prm, uint32_t prm_size, uint32_t flags);

int32_t appFvid2Init(void)
{
    int32_t retVal = FVID2_SOK;
    Fvid2_InitPrms initPrmsFvid2;

    appLogPrintf("FVID2: Init ... !!!\n");

    Fvid2InitPrms_init(&initPrmsFvid2);
    initPrmsFvid2.printFxn = appLogPrintf;
    retVal = Fvid2_init(&initPrmsFvid2);
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(retVal!=FVID2_SOK)
    {
        appLogPrintf("FVID2: ERROR: Fvid2_init failed !!!\n");
    }
    /* LDRA_JUSTIFY_END */
    appLogPrintf("FVID2: Init ... Done !!!\n");

    return (retVal);
}

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: The calls to this function are out of scope for the imaging test framework.
Effect on this unit: The function is expected to ensure proper cleanup and resource release for remote services, maintaining system stability and preventing resource leaks.
<justification end> */
int32_t appFvid2DeInit(void)
{
    int32_t retVal = FVID2_SOK;

    retVal = Fvid2_deInit(NULL);
    if(retVal!=FVID2_SOK)
    {
        appLogPrintf("FVID2: ERROR: Fvid2_deInit failed !!!\n");
    }
    return (retVal);
}
/* LDRA_JUSTIFY_END */

#if !defined(SOC_AM62A)

int32_t appVhwaDmpacInit(void)
{
    int32_t  status = FVID2_SOK;
    #if defined(MCU_PLUS_SDK)
    Udma_DrvHandle drvHandle = &gAppUdmaDrvObj;
    #endif

    appLogPrintf("VHWA: DMPAC: Init ... !!!\n");

    #if defined(ENABLE_DOF) || defined(ENABLE_SDE)
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (0 != APP_DEBUG_SCICLIENT)
        /* LDRA_JUSTIFY_END */
        {
            appLogPrintf("SCICLIENT: Sciclient_pmSetModuleState module=%u state=%u\n", TISCI_DEV_DMPAC0, TISCI_MSG_VALUE_DEVICE_SW_STATE_ON);
        }
        status = Sciclient_pmSetModuleState(TISCI_DEV_DMPAC0, TISCI_MSG_VALUE_DEVICE_SW_STATE_ON, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != status)
        {
            appLogPrintf("SCICLIENT: ERROR: Sciclient_pmSetModuleState failed\n");
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The test framework and test apps cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (0 != APP_DEBUG_SCICLIENT)
            /* LDRA_JUSTIFY_END */
            {
                appLogPrintf("SCICLIENT: Sciclient_pmSetModuleState success\n");
            }
        }
    }
    #endif
    #if !defined(SOC_AM62A) && !defined(SOC_J722S)/* SDE requires a separate power domain on j721s2, j721e, j784s4 j742s2; am62a not supported, j722s doesn't require separate power domain */
    #if defined(ENABLE_SDE)
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (0 != APP_DEBUG_SCICLIENT)
        /* LDRA_JUSTIFY_END */
        {
            appLogPrintf("SCICLIENT: Sciclient_pmSetModuleState module=%u state=%u\n", TISCI_DEV_DMPAC0_SDE_0, TISCI_MSG_VALUE_DEVICE_SW_STATE_ON);
        }
        status = Sciclient_pmSetModuleState(TISCI_DEV_DMPAC0_SDE_0, TISCI_MSG_VALUE_DEVICE_SW_STATE_ON, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != status) {
            {
                appLogPrintf("SCICLIENT: ERROR: Sciclient_pmSetModuleState failed\n");
            }
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The test framework and test apps cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (0 != APP_DEBUG_SCICLIENT)
            /* LDRA_JUSTIFY_END */
            {
                appLogPrintf("SCICLIENT: Sciclient_pmSetModuleState success\n");
            }
        }
    }
    #endif
    #endif

#if defined(ENABLE_DOF)
    Vhwa_M2mDofSl2AllocPrms sl2AllocPrmsDof;
    Vhwa_M2mDofInitParams   initPrmsDof;

    #if defined(APP_DEBUG_VHWA)
    appLogPrintf("VHWA: DOF Init ... !!!\n");
    #endif

    /* Initialize DOF Init parameters */
    Vhwa_m2mDofInitParamsInit(&initPrmsDof);

    /* Set UDMA driver handle */
    #if defined(MCU_PLUS_SDK)
    initPrmsDof.udmaDrvHndl = drvHandle;
    #else
    initPrmsDof.udmaDrvHndl = appUdmaGetObj();
    #endif

    status = Vhwa_m2mDofInit(&initPrmsDof);
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (0 != status)
    {
        appLogPrintf("VHWA: ERROR: DOF Init Failed !!!\n");
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        /* Initilize SL2 parameters */
        Vhwa_M2mDofSl2AllocPrmsInit(&sl2AllocPrmsDof);

        status = Vhwa_m2mDofAllocSl2(&sl2AllocPrmsDof);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> Rationale: The negative test framework and test apps cannot reach this portion.
            The control cannot reach here without allocating the sl2 in the current test apps and kernel, therefore it is not possible to cover. Cannot be removed since this is a fail safe mechanism.
            Effect on this unit: If the control reaches here, it will resolve the sl2 allocation issue and keep the functionality integrated.
        <justification end> */
        if (0 != status)
        {
            appLogPrintf("VHWA: ERROR: DOF SL2 Alloc Failed !!!\n");
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            #if defined(APP_DEBUG_VHWA)
            appLogPrintf("VHWA: DOF Init ... Done !!!\n");
            #endif
        }
    }
#endif

#if defined(ENABLE_SDE)
    /* SDE */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (0 == status)
    /* LDRA_JUSTIFY_END */
    {
        Vhwa_M2mSdeSl2AllocPrms sl2AllocPrmsSde;
        Vhwa_M2mSdeInitParams   initPrmsSde;

        #if defined(APP_DEBUG_VHWA)
        appLogPrintf("VHWA: SDE Init ... !!!\n");
        #endif

        /* Initialize SDE Init parameters */
        Vhwa_M2mSdeInitParamsInit(&initPrmsSde);

        /* Set UDMA driver handle */
        #if defined(MCU_PLUS_SDK)
        initPrmsSde.udmaDrvHndl = drvHandle;
        #else
        initPrmsSde.udmaDrvHndl = appUdmaGetObj();
        #endif

        status = Vhwa_m2mSdeInit(&initPrmsSde);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (0 != status)
        {
            appLogPrintf("VHWA: ERROR: SDE Init Failed !!!\n");
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            /* Initilize SL2 parameters */
            Vhwa_M2mSdeSl2AllocPrmsInit(&sl2AllocPrmsSde);

            status = Vhwa_m2mSdeAllocSl2(&sl2AllocPrmsSde);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> Rationale: The negative test framework and test apps cannot reach this portion.
                The control cannot reach here without allocating the sl2 in the current test apps and kernel, therefore it is not possible to cover. Cannot be removed since this is a fail safe mechanism.
                Effect on this unit: If the control reaches here, it will resolve the sl2 allocation issue and keep the functionality integrated.
            <justification end> */
            if (0 != status)
            {
                appLogPrintf("VHWA: ERROR: SDE SL2 Alloc Failed !!!\n");
            }
            else
            /* LDRA_JUSTIFY_END */
            {
                #if defined(APP_DEBUG_VHWA)
                appLogPrintf("VHWA: SDE Init ... Done !!!\n");
                #endif
            }
        }
    }
#endif

#if defined(ENABLE_DOF) && defined(ENABLE_SDE)
    if (0 == status)
    {
        status = Vhwa_m2mLbistInit();
    }
#endif

    appLogPrintf("VHWA: DMPAC: Init ... Done !!!\n");
    return (status);

}

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: The component level negative test framework and test applications cannot reach this portion.
The test framework does not support the configuration required to trigger this error scenario.
Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
<justification end> */
int32_t appVhwaDmpacDeInit(void)
{
#if defined(ENABLE_DOF)
    Vhwa_m2mDofDeInit();
#endif
#if defined(ENABLE_SDE)
    Vhwa_m2mSdeDeInit();
#endif

#if defined(ENABLE_DOF) && defined(ENABLE_SDE)
    (void)Vhwa_m2mLbistDeInit();
#endif

    return (0);
}
/* LDRA_JUSTIFY_END */

#endif /* !defined(SOC_AM62A) */


static void appVhwaVpacMscInit(Vhwa_M2mMscSl2AllocPrms *sl2Prms)
{
    uint32_t cnt;
    uint32_t idx;

    for(cnt = 0; cnt < VHWA_M2M_MSC_MAX_INST; cnt++)
    {
        for(idx = 0; idx < VHWA_M2M_MSC_MAX_IN_CHANNEL; idx++)
        {
            sl2Prms->maxInWidth[cnt][idx]    = APP_UTILS_VHWA_MAX_IN_IMG_WIDTH;
            sl2Prms->inCcsf[cnt][idx]        = APP_UTILS_VHWA_IN_IMG_CCSF;
            sl2Prms->inBuffDepth[cnt][idx]   = APP_UTILS_VHWA_MAX_IN_IMG_BUFF_DEPTH;
        }
    }

    for(cnt = 0; cnt < MSC_MAX_OUTPUT; cnt++)
    {
        sl2Prms->maxOutWidth[cnt]   = APP_UTILS_VHWA_MAX_OUT_IMG_WIDTH;
        sl2Prms->outCcsf[cnt]       = APP_UTILS_VHWA_OUT_IMG_CCSF;
        sl2Prms->outBuffDepth[cnt]  = APP_UTILS_VHWA_MAX_OUT_IMG_BUFF_DEPTH;
    }

    #if defined(VHWA_VPAC_IP_REV_VPAC) || defined(VHWA_VPAC_IP_REV_VPAC3)
    sl2Prms->enable16KBAlignment = (uint32_t)UTRUE;  /* 16KB alignment enabled by default (required for firewall support) */
    #else
    sl2Prms->enable16KBAlignment = (uint32_t)UFALSE;  /* VPAC3L does not support firewall */
    #endif
}

int32_t appVhwaVpacInit(uint32_t vpacInst)
{
    int32_t  status = FVID2_SOK;
    #if defined(MCU_PLUS_SDK)
    uint32_t       instId;
    Udma_InitPrms  udmaInitPrms;
    Udma_DrvHandle drvHandle = &gAppUdmaDrvObj;
    Udma_DrvHandle drvHandleBcdma = appUdmaGetObj();
    #endif

    appLogPrintf("VHWA: VPAC Init ... !!!\n");

    #if defined(ENABLE_LDC) || defined(ENABLE_MSC) || defined(ENABLE_NF) || defined(ENABLE_VISS) || defined(ENABLE_FC)
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario. vpacInst always has a value of 0 or 1.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (0u==vpacInst)
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == status)
        /* LDRA_JUSTIFY_END */
        {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            RRationale: The test framework and test apps cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (0 != APP_DEBUG_SCICLIENT)
            /* LDRA_JUSTIFY_END */
            {
                appLogPrintf("SCICLIENT: Sciclient_pmSetModuleState module=%u state=%u\n", TISCI_DEV_VPAC0, TISCI_MSG_VALUE_DEVICE_SW_STATE_ON);
            }
            status = Sciclient_pmSetModuleState(TISCI_DEV_VPAC0, TISCI_MSG_VALUE_DEVICE_SW_STATE_ON, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The test framework and test apps cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK != status)
            {
                appLogPrintf("SCICLIENT: ERROR: Sciclient_pmSetModuleState failed\n");
            }
            else
            /* LDRA_JUSTIFY_END */
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The test framework and test apps cannot reach this portion
                The function is unused because of an un-implemented debug feature, this code base is not being used/enabled with the current kernel support.
                Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
                <justification end> */
                if (0 != APP_DEBUG_SCICLIENT)
                /* LDRA_JUSTIFY_END */
                {
                    appLogPrintf("SCICLIENT: Sciclient_pmSetModuleState success\n");
                }
            }
        }
    }
    #if defined(SOC_J784S4) || defined(SOC_J742S2)
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario. vpacInst always has a value of 0 or 1.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else if (1u==vpacInst)
    {
        if (FVID2_SOK == status)
        {
            if (0 != APP_DEBUG_SCICLIENT)
            {
                appLogPrintf("SCICLIENT: Sciclient_pmSetModuleState module=%u state=%u\n", TISCI_DEV_VPAC1, TISCI_MSG_VALUE_DEVICE_SW_STATE_ON);
            }
            status = Sciclient_pmSetModuleState(TISCI_DEV_VPAC1, TISCI_MSG_VALUE_DEVICE_SW_STATE_ON, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER);
            if (FVID2_SOK != status)
            {
                appLogPrintf("SCICLIENT: ERROR: Sciclient_pmSetModuleState failed\n");
            }
            else
            {
                if (0 != APP_DEBUG_SCICLIENT)
                {
                    appLogPrintf("SCICLIENT: Sciclient_pmSetModuleState success\n");
                }
            }
        }
    }
    /* LDRA_JUSTIFY_END */
    #endif
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario. vpacInst always has a value of 0 or 1.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        appLogPrintf("VHWA: ERROR: Unsupported VPAC instance!!!\n");
        status = FVID2_EFAIL;
    }
    /* LDRA_JUSTIFY_END */
    #endif

    #if defined(MCU_PLUS_SDK)
    if (FVID2_SOK == status)
    {
        /* Initialize UDMA and get the handle, it will be used in both CRC layer,
        as well as in the driver */
        /* UDMA driver init */
        instId = UDMA_INST_ID_0;
        status = UdmaInitPrms_init(instId, &udmaInitPrms);
        if(UDMA_SOK != status)
        {
            appLogPrintf("[Error] UDMA prms init failed!!\n");
        }
        udmaInitPrms.instId = UDMA_INST_ID_0;
        udmaInitPrms.enableUtc = (uint8_t)UTRUE;
        status = Udma_init(drvHandle, &udmaInitPrms);
        if(UDMA_SOK != status)
        {
            appLogPrintf("[Error] UDMA init failed!!\n");
            status = FVID2_EFAIL;
        }
    }
    #endif
#if defined(ENABLE_LDC)
    /* LDC */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (0 == status)
    /* LDRA_JUSTIFY_END */
    {
        Vhwa_M2mLdcSl2AllocPrms sl2AllocPrms;
        Vhwa_M2mLdcInitParams   initPrms;

        #if defined(APP_DEBUG_VHWA)
        appLogPrintf("VHWA: LDC Init ... !!!\n");
        #endif

        /* Initialize LDC Init parameters */
        Vhwa_m2mLdcInitParamsInit(&initPrms);

        /* Set UDMA driver handle */
        #if defined(MCU_PLUS_SDK)
        initPrms.udmaDrvHndl = drvHandle;
        #else
        initPrms.udmaDrvHndl = appUdmaGetObj();
        #endif

        status = Vhwa_m2mLdcInit(&initPrms);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (0 != status)
        {
            appLogPrintf("VHWA: ERROR: LDC Init Failed !!!\n");
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            Vhwa_M2mLdcSl2AllocPrmsInit(&sl2AllocPrms);

            sl2AllocPrms.maxBlockWidth  = APP_UTILS_VHWA_LDC_MAX_BLOCK_WIDTH;
            sl2AllocPrms.maxBlockHeight = APP_UTILS_VHWA_LDC_MAX_BLOCK_HEIGHT;

            status = Vhwa_m2mLdcAllocSl2(&sl2AllocPrms);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The test framework and test apps cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (0 != status)
            {
                appLogPrintf("VHWA: ERROR: LDC SL2 Alloc Failed !!!\n");
            }
            else
            /* LDRA_JUSTIFY_END */
            {
                #if defined(APP_DEBUG_VHWA)
                appLogPrintf("VHWA: LDC Init ... Done !!!\n");
                #endif
            }
        }
    }
#endif

#if defined(ENABLE_MSC)
    /* MSC */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (0 == status)
    /* LDRA_JUSTIFY_END */
    {
        Vhwa_M2mMscInitParams initPrms;
        Vhwa_M2mMscSl2AllocPrms sl2Prms;

        #if defined(APP_DEBUG_VHWA)
        appLogPrintf("VHWA: MSC Init ... !!!\n");
        #endif

        Vhwa_m2mMscInitParamsInit(&initPrms);

        #if defined(MCU_PLUS_SDK)
        initPrms.drvHandle = drvHandle;
        #else
        initPrms.drvHandle = appUdmaGetObj();
        #endif

        status = Vhwa_m2mMscInit(&initPrms);

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (0 == status)
        /* LDRA_JUSTIFY_END */
        {
            appVhwaVpacMscInit(&sl2Prms);

        #if defined(ENABLE_FC)
            /* If Flexconnect is enabled Update MSC SL2 input buffer depth */
            uint32_t cnt, idx;

            for(cnt = 0; cnt < VHWA_M2M_MSC_MAX_INST; cnt++)
            {
                for(idx = 0; idx < VHWA_M2M_MSC_MAX_IN_CHANNEL; idx++)
                {
                    #if !defined(VPAC3L)
                        sl2Prms.inBuffDepth[cnt][idx] = 8U;
                    #else
                        if (VPAC_MSC_INST_ID_0 == cnt)
                        {
                            sl2Prms.inBuffDepth[cnt][idx] = 8U;
                        }
                    #endif
                }
            }
        #endif
            status = Vhwa_m2mMscAllocSl2(&sl2Prms);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The test framework and test apps cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (0 != status)
            {
                appLogPrintf("VHWA: ERROR: MSC SL2 Alloc Failed !!!\n");
            }
            else
            /* LDRA_JUSTIFY_END */
            {
                #if defined(APP_DEBUG_VHWA)
                appLogPrintf("VHWA: MSC Init ... Done !!!\n");
                #endif
            }
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            appLogPrintf("VHWA: ERROR: MSC Init Failed !!!\n");
        }
        /* LDRA_JUSTIFY_END */
    }
#endif

#if defined(ENABLE_NF)
    /* NF */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (0 == status)
    /* LDRA_JUSTIFY_END */
    {
        Vhwa_M2mNfSl2AllocPrms  sl2AllocPrms;
        Vhwa_M2mNfInitPrms      initPrms;

        #if defined(APP_DEBUG_VHWA)
        appLogPrintf("VHWA: NF Init ... !!!\n");
        #endif

        /* Initialize NF Init parameters */
        Vhwa_m2mNfInitPrmsInit(&initPrms);

        /* Set UDMA driver handle */
        initPrms.udmaDrvHndl = appUdmaGetObj();

        status = Vhwa_m2mNfInit(&initPrms);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (0 != status)
        {
            appLogPrintf("VHWA: ERROR: NF Init Failed !!!\n");
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            /* Initilize SL2 parameters */
            sl2AllocPrms.maxImgWidth  = APP_UTILS_VHWA_MAX_IN_IMG_WIDTH;
            sl2AllocPrms.inCcsf       = APP_UTILS_VHWA_IN_IMG_CCSF;
            sl2AllocPrms.inBuffDepth  = APP_UTILS_VHWA_MAX_IN_IMG_BUFF_DEPTH;
            sl2AllocPrms.outBuffDepth = APP_UTILS_VHWA_MAX_OUT_IMG_BUFF_DEPTH;
            sl2AllocPrms.outCcsf      = APP_UTILS_VHWA_OUT_IMG_CCSF;
            #if defined(VHWA_VPAC_IP_REV_VPAC) || defined(VHWA_VPAC_IP_REV_VPAC3)
            sl2AllocPrms.enable16KBAlignment = (uint32_t)UTRUE;  /* 16KB alignment enabled by default (required for firewall support) */
            #else
            sl2AllocPrms.enable16KBAlignment = (uint32_t)UFALSE;  /* VPAC3L does not support firewall */
            #endif

            status = Vhwa_m2mNfAllocSl2(&sl2AllocPrms);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The test framework and test apps cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (0 != status)
            {
                appLogPrintf("VHWA: ERROR: NF SL2 Alloc Failed !!!\n");
            }
            else
            /* LDRA_JUSTIFY_END */
            {
                #if defined(APP_DEBUG_VHWA)
                appLogPrintf("VHWA: NF Init ... Done !!!\n");
                #endif
            }
        }
    }
#endif

#if defined(ENABLE_VISS)
    /* VISS */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (0 == status)
    /* LDRA_JUSTIFY_END */
    {
        Vhwa_M2mVissSl2Params   sl2AllocPrms;
        Vhwa_M2mVissInitParams  initPrms;

        #if defined(APP_DEBUG_VHWA)
        appLogPrintf("VHWA: VISS Init ... !!!\n");
        #endif

        /* Initialize VISS Init parameters */
        Vhwa_m2mVissInitParamsInit(&initPrms);

        /* Set UDMA driver handle */
        #if defined(MCU_PLUS_SDK)
        initPrms.udmaDrvHndl = drvHandle;
        initPrms.udmaDrvHndlBcdma = drvHandleBcdma;
        initPrms.udmaDrvHndlVissCfgBcdma = drvHandleBcdma;
        /* enable config through UDMA */
        #else
        initPrms.udmaDrvHndl = appUdmaGetObj();
        initPrms.udmaDrvHndlBcdma = appUdmaGetObj();
        /* Set configThroughUDMA to true to support multi handle */
        #endif
        initPrms.configThroughUdmaFlag = true;
        initPrms.copyGlbceCtxThroughBcdmaFlag = true;

        status = Vhwa_m2mVissInit(&initPrms);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (0 != status)
        {
            appLogPrintf("VHWA: ERROR: VISS Init Failed !!!\n");
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            uint32_t cnt;

            Vhwa_m2mVissSl2ParamsInit(&sl2AllocPrms);

            for (cnt = 0U; cnt < VHWA_M2M_VISS_MAX_INPUTS; cnt++)
            {
                sl2AllocPrms.maxInWidth[cnt] = APP_UTILS_VHWA_MAX_IN_IMG_WIDTH;
            }

            for (cnt = 0U; cnt < VHWA_M2M_VISS_MAX_OUTPUTS; cnt ++)
            {
                if (VHWA_M2M_VISS_OUT_H3A_IDX == cnt)
                {
                    /* This is the default size for H3A output */
                    sl2AllocPrms.maxOutWidth[cnt] = 1024U;
                }
                else
                {
                    sl2AllocPrms.maxOutWidth[cnt] = APP_UTILS_VHWA_MAX_OUT_IMG_WIDTH;
                }
            }

            sl2AllocPrms.inDepth = 3U; /* Minimum 3 */

            for (cnt = 0U; cnt < VHWA_M2M_VISS_MAX_OUTPUTS; cnt ++)
            {
                sl2AllocPrms.outDepth[cnt] = 2U; /* Minimum 2 */
            }

        #if defined(ENABLE_FC) && defined(VPAC3L)
            /* for VPAC3L If Flexconnect is enabled the viss saturation output is disabled, the bitdepth of VISS OUT 2 and 3 is resttricted to 1.5 Bytes per pixel*/
            sl2AllocPrms.outCcsf[VHWA_M2M_VISS_OUT_LUMA_8B_IDX]         = FVID2_CCSF_BITS12_PACKED;
            sl2AllocPrms.outCcsf[VHWA_M2M_VISS_OUT_CHROMA_8B_IDX]       = FVID2_CCSF_BITS12_PACKED;
            sl2AllocPrms.maxOutWidth[VHWA_M2M_VISS_OUT_SATURA_8B_IDX]   = 0U;
        #endif
            status = Vhwa_m2mVissAllocSl2(&sl2AllocPrms);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The test framework and test apps cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (0 != status)
            {
                appLogPrintf("VHWA: ERROR: VISS SL2 Alloc Failed !!!\n");
            }
            else
            /* LDRA_JUSTIFY_END */
            {
                #if defined(APP_DEBUG_VHWA)
                appLogPrintf("VHWA: VISS Init ... Done !!!\n");
                #endif
            }
        }
    }
#endif

#if defined(ENABLE_FC)
    /* FC */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (0 == status)
    /* LDRA_JUSTIFY_END */
    {
        Vhwa_M2mFcInitPrms          fcInitPrms;

        #if defined(APP_DEBUG_VHWA)
        appLogPrintf("VHWA: FC Init ... !!!\n");
        #endif

        /* Initialize VISS Init parameters */
        Vhwa_m2mFcInitPrmsInit(&fcInitPrms);

        /* Set UDMA driver handle */
        #if defined(MCU_PLUS_SDK)
        fcInitPrms.udmaDrvHndl = drvHandle;
        #else
        fcInitPrms.udmaDrvHndl = appUdmaGetObj();
        #endif

        status = Vhwa_m2mFcInit(&fcInitPrms);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (0 != status)
        {
            appLogPrintf("VHWA: ERROR: FC Init Failed !!!\n");
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            #if defined(APP_DEBUG_VHWA)
            appLogPrintf("VHWA: FC Init ... Done !!!\n");
            #endif
        }
    }
#endif

#if !defined(SOC_AM62A) /* LBIST not applicable for am62a */
#if defined(ENABLE_LDC) && defined(ENABLE_MSC) && defined(ENABLE_NF) && defined(ENABLE_VISS)
    if (0 == status)
    {
        status = Vhwa_m2mLbistInit();
    }
#endif
#endif

    appLogPrintf("VHWA: VPAC Init ... Done !!!\n");
    return (status);
}

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: The calls to this function are out of scope for the imaging test framework.
Effect on this unit: The function is expected to ensure proper cleanup and resource release for remote services, maintaining system stability and preventing resource leaks.
<justification end> */
int32_t appVhwaVpacDeInit(void)
{
#if defined(ENABLE_LDC)
    Vhwa_m2mLdcDeInit();
#endif
#if defined(ENABLE_MSC)
    (void)Vhwa_m2mMscDeInit();
#endif
#if defined(ENABLE_NF)
    Vhwa_m2mNfDeInit();
#endif
#if defined(ENABLE_VISS)
    Vhwa_m2mVissDeInit();
#endif
#if defined(ENABLE_FC)
    Vhwa_m2mFcDeInit();
#endif

#if !defined(SOC_AM62A) /* LBIST not applicable for am62a */
#if defined(ENABLE_LDC) && defined(ENABLE_MSC) && defined(ENABLE_NF) && defined(ENABLE_VISS)
    (void)Vhwa_m2mLbistDeInit();
#endif
#endif

    return (0);
}
/* LDRA_JUSTIFY_END */

#if defined(SOC_J721E) || defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)
/**
 * Normal VPAC firewall enable (called via APP_VPAC_ENABLE_FIREWALL IPC command)
 * Enables ALL module firewalls unconditionally and sets all flags to 1.
 * For LBIST recovery, use appVhwaEnableVpacFirewallAfterLbist() instead.
 */
static int32_t appVhwaEnableVpacFirewall(uint32_t vpacInstId)
{
    int32_t status = (int32_t)VX_SUCCESS;

#if defined(ENABLE_LDC)
    if ((int32_t)VX_SUCCESS == status)
    {
        /* Get SL2 info and check if firewall is supported */
        Vhwa_M2mLdcSl2Info sl2Info = {0};
        int32_t ret;

        ret = Vhwa_m2mLdcGetSl2Info(&sl2Info);
        if (ret != FVID2_SOK)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to get LDC SL2 info (error: %d)\n", ret);
            status = (int32_t)VX_FAILURE;
        }
        else if (((sl2Info.sl2Addr & 0x3FFFu) != 0u) || ((sl2Info.sl2Size & 0x3FFFu) != 0u))
        {
            VX_PRINT(VX_ZONE_ERROR, "SL2 memory not 16KB-aligned, firewall not supported\n");
            VX_PRINT(VX_ZONE_ERROR, "addr=0x%" PRIx64 " size=0x%x\n", (uint64_t)sl2Info.sl2Addr, sl2Info.sl2Size);
            status = (int32_t)VX_FAILURE;
        }
        else
        {
            status = appVpacFirewallEnable(
                (uint8_t)vpacInstId,
                VPAC_FWL_REGION_LDC,
                sl2Info.sl2Addr,
                sl2Info.sl2Size,
                TISCI_PRIV_ID_LDC,
                TISCI_PRIV_ID_MAIN_0_R5_0,
                0);

            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to configure VPAC%d SL2 firewall for LDC\n", vpacInstId);
            }
        }
    }
#endif
#if defined(ENABLE_MSC)
    if ((int32_t)VX_SUCCESS == status)
    {
        /* Get SL2 info and check if firewall is supported */
        Vhwa_M2mMscSl2Info sl2Info = {0};
        int32_t ret;

        ret = Vhwa_m2mMscGetSl2Info(&sl2Info);
        if (ret != FVID2_SOK)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to get MSC SL2 info (error: %d)\n", ret);
            status = (int32_t)VX_FAILURE;
        }
        else if (((sl2Info.sl2Addr & 0x3FFFu) != 0u) || ((sl2Info.sl2Size & 0x3FFFu) != 0u))
        {
            VX_PRINT(VX_ZONE_ERROR, "SL2 memory not 16KB-aligned, firewall not supported\n");
            VX_PRINT(VX_ZONE_ERROR, "addr=0x%" PRIx64 " size=0x%x\n",
                (uint64_t)sl2Info.sl2Addr, sl2Info.sl2Size);
            status = (int32_t)VX_FAILURE;
        }
        else
        {
            status = appVpacFirewallEnable(
                (uint8_t)vpacInstId,
                VPAC_FWL_REGION_MSC,
                sl2Info.sl2Addr,
                sl2Info.sl2Size,
                TISCI_PRIV_ID_MSC,
                TISCI_PRIV_ID_MAIN_0_R5_0,
                0);

            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to configure VPAC%d SL2 firewall for MSC\n", vpacInstId);
            }
        }
    }
#endif
#if defined(ENABLE_NF)
    if ((int32_t)VX_SUCCESS == status)
    {
        /* Get SL2 info and check if firewall is supported */
        Vhwa_M2mNfSl2Info sl2Info = {0};
        int32_t ret;

        ret = Vhwa_m2mNfGetSl2Info(&sl2Info);
        if (ret != FVID2_SOK)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to get NF SL2 info (error: %d)\n", ret);
            status = (int32_t)VX_FAILURE;
        }
        else if (((sl2Info.sl2Addr & 0x3FFFu) != 0u) || ((sl2Info.sl2Size & 0x3FFFu) != 0u))
        {
            VX_PRINT(VX_ZONE_ERROR, "SL2 memory not 16KB-aligned, firewall not supported\n");
            VX_PRINT(VX_ZONE_ERROR, "addr=0x%" PRIx64 " size=0x%x\n",
                (uint64_t)sl2Info.sl2Addr, sl2Info.sl2Size);
            status = (int32_t)VX_FAILURE;
        }
        else
        {
            status = appVpacFirewallEnable(
                (uint8_t)vpacInstId,
                VPAC_FWL_REGION_NF,
                sl2Info.sl2Addr,
                sl2Info.sl2Size,
                TISCI_PRIV_ID_NF,
                TISCI_PRIV_ID_MAIN_0_R5_0,
                0);

            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to configure VPAC%d SL2 firewall for NF\n", vpacInstId);
            }
        }
    }
#endif
#if defined(ENABLE_VISS)
    if ((int32_t)VX_SUCCESS == status)
    {
        /* Get SL2 info and check if firewall is supported */
        Vhwa_M2mVissSl2Info sl2Info = {0};
        int32_t ret;

        ret = Vhwa_m2mVissGetSl2Info(&sl2Info);
        if (ret != FVID2_SOK)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to get VISS SL2 info (error: %d)\n", ret);
            status = (int32_t)VX_FAILURE;
        }
        else if (((sl2Info.sl2Addr & 0x3FFFu) != 0u) || ((sl2Info.sl2Size & 0x3FFFu) != 0u))
        {
            VX_PRINT(VX_ZONE_ERROR, "SL2 memory not 16KB-aligned, firewall not supported\n");
            VX_PRINT(VX_ZONE_ERROR, "addr=0x%" PRIx64 " size=0x%x\n",
                (uint64_t)sl2Info.sl2Addr, sl2Info.sl2Size);
            status = (int32_t)VX_FAILURE;
        }
        else
        {
            status = appVpacFirewallEnable(
                (uint8_t)vpacInstId,
                VPAC_FWL_REGION_VISS,
                sl2Info.sl2Addr,
                sl2Info.sl2Size,
                TISCI_PRIV_ID_VISS,
                TISCI_PRIV_ID_MAIN_0_R5_0,
                0);

            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "VPAC%d VISS SL2 Firewall enable failed\n", vpacInstId);
            }
        }
    }
#endif
    return (status);
}

/**
 * Normal DMPAC firewall enable (called via APP_DMPAC_ENABLE_FIREWALL IPC command)
 * Enables ALL module firewalls unconditionally and sets all flags to 1.
 * For LBIST recovery, use appVhwaEnableDmpacFirewallAfterLbist() instead.
 */
static int32_t appVhwaEnableDmpacFirewall(void)
{
    int32_t status = (int32_t)VX_SUCCESS;

#if defined(ENABLE_DOF)
    if ((int32_t)VX_SUCCESS == status)
    {
        Vhwa_M2mDofSl2Info sl2Info = {0};
        int32_t ret;

        ret = Vhwa_m2mDofGetSl2Info(&sl2Info);
        if (ret != FVID2_SOK)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to get DOF SL2 info (error: %d)\n", ret);
            status = (int32_t)VX_FAILURE;
        }
        /* Validate that driver allocated memory with 2-region support (16KB alignment) */
        else if ((sl2Info.mainRegionSl2Addr == 0u) && (sl2Info.refRegionSl2Addr == 0u))
        {
            VX_PRINT(VX_ZONE_ERROR, "Firewall enabled but driver used legacy allocation (enable16KBAlignment=FALSE)\n");
            VX_PRINT(VX_ZONE_ERROR, "To use firewall, set enable16KBAlignment=TRUE in SL2 allocation parameters\n");
            status = (int32_t)VX_FAILURE;
        }
        else if (((sl2Info.sl2Addr & 0x3FFFu) != 0u) || ((sl2Info.sl2Size & 0x3FFFu) != 0u))
        {
            VX_PRINT(VX_ZONE_ERROR, "SL2 memory not 16KB-aligned, firewall not supported\n");
            VX_PRINT(VX_ZONE_ERROR, "addr=0x%" PRIx64 " size=0x%x\n",
                (uint64_t)sl2Info.sl2Addr, sl2Info.sl2Size);
            status = (int32_t)VX_FAILURE;
        }
        else if (sl2Info.mainRegionSl2Addr == 0u)
        {
            VX_PRINT(VX_ZONE_ERROR, "Firewall allocation error - main region missing\n");
            status = (int32_t)VX_FAILURE;
        }
        else if (sl2Info.refRegionSl2Addr == 0u)
        {
            VX_PRINT(VX_ZONE_ERROR, "Firewall allocation error - reference region missing\n");
            status = (int32_t)VX_FAILURE;
        }
        else
        {
            /* TWO-REGION FIREWALL LAYOUT:
             * Region 0 (MAIN): Current + FOCO current + Predictors + Output
             *   Permissions: R5 (213), FOCO (211), DOF (210)
             * Region 1 (REFERENCE): Reference + FOCO reference
             *   Permissions: R5 (213), FOCO (211), DOF_RD (209) */

            /* Region 0: MAIN (Current + FOCO current + Predictors + Output) */
            uint64_t mainRegionAddr = sl2Info.mainRegionSl2Addr;
            uint32_t mainRegionSize = sl2Info.mainRegionSl2Size;

            /* Region 1: REFERENCE (Reference + FOCO reference) */
            uint64_t refRegionAddr = sl2Info.refRegionSl2Addr;
            uint32_t refRegionSize = sl2Info.refRegionSl2Size;

            /* TWO-REGION FIREWALL (when FOCO is enabled):
             * Region 0 (MAIN): Current + FOCO current + Predictors + Output
             *   - R5 (213): DMAs data to/from DDR
             *   - FOCO (211): Reads current image, writes FOCO preprocessed current
             *   - DOF (210): LSE engine reads/writes all main region buffers
             * Region 1 (REFERENCE): Reference + FOCO reference
             *   - R5 (213): DMAs reference image from DDR
             *   - FOCO (211): Reads reference image, writes FOCO preprocessed reference
             *   - DOF_RD (209): MP0 engine reads for reference matching
             */

            /* Configure Region 0 (MAIN): R5 (213), FOCO (211), DOF (210) with reference counting */
            /* DMPAC instance ID (0=DMPAC0) - DOF uses VHWA_M2M_DOF_DRV_INST_ID */
            uint8_t dmpacInstId = (uint8_t)VHWA_M2M_DOF_DRV_INST_ID;

            status = appDmpacFirewallEnable(
                dmpacInstId,
                DMPAC_FWL_REGION_DOF_MAIN,  /* Region index 0 */
                mainRegionAddr,
                mainRegionSize,
                TISCI_PRIV_ID_DOF,          /* 210U - DOF LSE reads/writes all main buffers */
                TISCI_PRIV_ID_MAIN_0_R5_1,  /* 213U - R5 DMAs data to/from DDR */
                TISCI_PRIV_ID_FOCO_0);      /* 211U - FOCO reads current, writes preprocessed */

            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to configure DMPAC%d SL2 firewall for DOF MAIN region\n", dmpacInstId);
            }
            else
            {
                /* Configure Region 1 (REFERENCE): R5 (213), FOCO (211), DOF_RD (209) with reference counting */
                status = appDmpacFirewallEnable(
                    dmpacInstId,
                    DMPAC_FWL_REGION_DOF_REFERENCE,  /* Region index 1 */
                    refRegionAddr,
                    refRegionSize,
                    TISCI_PRIV_ID_DOF_RD,       /* 209U - MP0 reads reference for matching */
                    TISCI_PRIV_ID_MAIN_0_R5_1,  /* 213U - R5 DMAs reference from DDR */
                    TISCI_PRIV_ID_FOCO_0);      /* 211U - FOCO reads reference, writes preprocessed */

                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to configure DMPAC%d SL2 firewall for DOF REFERENCE region\n", dmpacInstId);
                    status = appDmpacFirewallDisable(dmpacInstId, DMPAC_FWL_REGION_DOF_MAIN);
                }
            }
        }
    }
#endif
#if defined(ENABLE_SDE)
    if ((int32_t)VX_SUCCESS == status)
    {
        Vhwa_M2mSdeSl2Info sl2Info = {0};
        int32_t ret;

        ret = Vhwa_m2mSdeGetSl2Info(&sl2Info);
        if (ret != FVID2_SOK)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to get SDE SL2 info (error: %d)\n", ret);
            status = (int32_t)VX_FAILURE;
        }
        else if (((sl2Info.sl2Addr & 0x3FFFu) != 0u) || ((sl2Info.sl2Size & 0x3FFFu) != 0u))
        {
            VX_PRINT(VX_ZONE_ERROR, "SL2 memory not 16KB-aligned, firewall not supported\n");
            VX_PRINT(VX_ZONE_ERROR, "addr=0x%" PRIx64 " size=0x%x\n",
                (uint64_t)sl2Info.sl2Addr, sl2Info.sl2Size);
            status = (int32_t)VX_FAILURE;
        }
        else
        {
            /* Configure firewall for SDE with reference counting */
            /* DMPAC instance ID (0=DMPAC0) - SDE uses VHWA_M2M_SDE_DRV_INST_ID */
            uint8_t dmpacInstId = (uint8_t)VHWA_M2M_SDE_DRV_INST_ID;

            status = appDmpacFirewallEnable(
                dmpacInstId,
                DMPAC_FWL_REGION_SDE,
                sl2Info.sl2Addr,
                sl2Info.sl2Size,
                TISCI_PRIV_ID_SDE,
                TISCI_PRIV_ID_MAIN_0_R5_1,
                TISCI_PRIV_ID_FOCO_1);

            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to configure DMPAC%d SL2 firewall for SDE\n", dmpacInstId);
            }
        }
    }
#endif
    return (status);
}

/**
 * Normal VPAC firewall disable (called via APP_VPAC_DISABLE_FIREWALL IPC command)
 * Disables ALL module firewalls and clears flags to 0.
 * LBIST recovery will respect this state and NOT re-enable firewalls.
 */
static int32_t appVhwaDisableVpacFirewall(uint32_t vpacInstId)
{
    int32_t status = (int32_t)VX_SUCCESS;

#if defined(ENABLE_LDC)
    if ((int32_t)VX_SUCCESS == status)
    {
        status = (int32_t)appVpacFirewallDisable((uint8_t)vpacInstId, VPAC_FWL_REGION_LDC);
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to disable VPAC%d SL2 firewall for LDC\n", vpacInstId);
        }
    }
#endif
#if defined(ENABLE_MSC)
    if ((int32_t)VX_SUCCESS == status)
    {
        status = (int32_t)appVpacFirewallDisable((uint8_t)vpacInstId, VPAC_FWL_REGION_MSC);
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to disable VPAC%d SL2 firewall for MSC\n", vpacInstId);
        }
    }
#endif
#if defined(ENABLE_NF)
    if ((int32_t)VX_SUCCESS == status)
    {
        status = (int32_t)appVpacFirewallDisable((uint8_t)vpacInstId, VPAC_FWL_REGION_NF);
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to disable VPAC%d SL2 firewall for NF\n", vpacInstId);
        }
    }
#endif
#if defined(ENABLE_VISS)
    if ((int32_t)VX_SUCCESS == status)
    {
        status = (int32_t)appVpacFirewallDisable((uint8_t)vpacInstId, VPAC_FWL_REGION_VISS);
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "VPAC%d VISS SL2 Firewall disable failed\n", vpacInstId);
        }
    }
#endif
    return (status);
}

/**
 * Normal DMPAC firewall disable (called via APP_DMPAC_DISABLE_FIREWALL IPC command)
 * Disables ALL module firewalls and clears flags to 0.
 * LBIST recovery will respect this state and NOT re-enable firewalls.
 */
static int32_t appVhwaDisableDmpacFirewall(void)
{
    int32_t status = (int32_t)VX_SUCCESS;
    uint8_t dmpacInstId = (uint8_t)VHWA_M2M_DOF_DRV_INST_ID;

#if defined(ENABLE_DOF)
    if ((int32_t)VX_SUCCESS == status)
    {
        /* Disable Region 0 (MAIN) */
        status = appDmpacFirewallDisable(dmpacInstId, DMPAC_FWL_REGION_DOF_MAIN);
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to disable DMPAC%d SL2 firewall for DOF MAIN region\n", dmpacInstId);
        }
        else
        {
            /* Disable Region 1 (REFERENCE) */
            status = appDmpacFirewallDisable(dmpacInstId, DMPAC_FWL_REGION_DOF_REFERENCE);
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to disable DMPAC%d SL2 firewall for DOF REFERENCE region\n", dmpacInstId);
            }
        }
    }
#endif
#if defined(ENABLE_SDE)
    if ((int32_t)VX_SUCCESS == status)
    {
        dmpacInstId = (uint8_t)VHWA_M2M_SDE_DRV_INST_ID;
        status = appDmpacFirewallDisable(dmpacInstId, DMPAC_FWL_REGION_SDE);
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to disable DMPAC%d SL2 firewall for SDE\n", dmpacInstId);
        }
    }
#endif
    return (status);
}

/**
 * LBIST VPAC firewall recovery - can be called directly by LBIST application
 * Checks flags and restores ONLY firewalls that were enabled before LBIST (flag=1).
 * Used after LBIST hardware reset to selectively restore protection.
 */
static int32_t appVhwaEnableVpacFirewallAfterLbist(uint32_t vpacInstId)
{
    int32_t status = (int32_t)VX_SUCCESS;

    VX_PRINT(VX_ZONE_INFO, "[LBIST] Restoring VPAC%d firewall state\n", vpacInstId);

#if defined(ENABLE_LDC)
    if (((int32_t)VX_SUCCESS == status) && (0u != appVpacFirewallShouldEnable((uint8_t)vpacInstId, VPAC_FWL_REGION_LDC)))
    {
        Vhwa_M2mLdcSl2Info sl2Info = {0};
        int32_t ret = Vhwa_m2mLdcGetSl2Info(&sl2Info);

        if ((ret == FVID2_SOK) &&
            ((sl2Info.sl2Addr & 0x3FFFu) == 0u) &&
            ((sl2Info.sl2Size & 0x3FFFu) == 0u))
        {
            status = (int32_t)appVpacFirewallEnable((uint8_t)vpacInstId, VPAC_FWL_REGION_LDC,
                sl2Info.sl2Addr, sl2Info.sl2Size, TISCI_PRIV_ID_LDC,
                TISCI_PRIV_ID_MAIN_0_R5_0, 0);

            if ((int32_t)VX_SUCCESS == status)
            {
                VX_PRINT(VX_ZONE_INFO, "[LBIST] Restored VPAC%d LDC firewall\n", vpacInstId);
            }
        }
    }
#endif

#if defined(ENABLE_MSC)
    if (((int32_t)VX_SUCCESS == status) && (0u != appVpacFirewallShouldEnable((uint8_t)vpacInstId, VPAC_FWL_REGION_MSC)))
    {
        Vhwa_M2mMscSl2Info sl2Info = {0};
        int32_t ret = Vhwa_m2mMscGetSl2Info(&sl2Info);

        if ((ret == FVID2_SOK) &&
            ((sl2Info.sl2Addr & 0x3FFFu) == 0u) &&
            ((sl2Info.sl2Size & 0x3FFFu) == 0u))
        {
            status = (int32_t)appVpacFirewallEnable((uint8_t)vpacInstId, VPAC_FWL_REGION_MSC,
                sl2Info.sl2Addr, sl2Info.sl2Size, TISCI_PRIV_ID_MSC,
                TISCI_PRIV_ID_MAIN_0_R5_0, 0);

            if ((int32_t)VX_SUCCESS == status)
            {
                VX_PRINT(VX_ZONE_INFO, "[LBIST] Restored VPAC%d MSC firewall\n", vpacInstId);
            }
        }
    }
#endif

#if defined(ENABLE_NF)
    if (((int32_t)VX_SUCCESS == status) && (0u != appVpacFirewallShouldEnable((uint8_t)vpacInstId, VPAC_FWL_REGION_NF)))
    {
        Vhwa_M2mNfSl2Info sl2Info = {0};
        int32_t ret = Vhwa_m2mNfGetSl2Info(&sl2Info);

        if ((ret == FVID2_SOK) &&
            ((sl2Info.sl2Addr & 0x3FFFu) == 0u) &&
            ((sl2Info.sl2Size & 0x3FFFu) == 0u))
        {
            status = (int32_t)appVpacFirewallEnable((uint8_t)vpacInstId, VPAC_FWL_REGION_NF,
                sl2Info.sl2Addr, sl2Info.sl2Size, TISCI_PRIV_ID_NF,
                TISCI_PRIV_ID_MAIN_0_R5_0, 0);

            if ((int32_t)VX_SUCCESS == status)
            {
                VX_PRINT(VX_ZONE_INFO, "[LBIST] Restored VPAC%d NF firewall\n", vpacInstId);
            }
        }
    }
#endif

#if defined(ENABLE_VISS)
    if (((int32_t)VX_SUCCESS == status) && (0u != appVpacFirewallShouldEnable((uint8_t)vpacInstId, VPAC_FWL_REGION_VISS)))
    {
        Vhwa_M2mVissSl2Info sl2Info = {0};
        int32_t ret = Vhwa_m2mVissGetSl2Info(&sl2Info);

        if ((ret == FVID2_SOK) &&
            ((sl2Info.sl2Addr & 0x3FFFu) == 0u) &&
            ((sl2Info.sl2Size & 0x3FFFu) == 0u))
        {
            status = (int32_t)appVpacFirewallEnable((uint8_t)vpacInstId, VPAC_FWL_REGION_VISS,
                sl2Info.sl2Addr, sl2Info.sl2Size, TISCI_PRIV_ID_VISS,
                TISCI_PRIV_ID_MAIN_0_R5_0, 0);

            if ((int32_t)VX_SUCCESS == status)
            {
                VX_PRINT(VX_ZONE_INFO, "[LBIST] Restored VPAC%d VISS firewall\n", vpacInstId);
            }
        }
    }
#endif

    return status;
}

/**
 * LBIST DMPAC firewall recovery - can be called directly by LBIST application
 * Checks flags and restores ONLY firewalls that were enabled before LBIST (flag=1).
 * Used after LBIST hardware reset to selectively restore protection.
 */
static int32_t appVhwaEnableDmpacFirewallAfterLbist(void)
{
    int32_t status = (int32_t)VX_SUCCESS;
    uint8_t dmpacInstId = (uint8_t)VHWA_M2M_DOF_DRV_INST_ID;

    VX_PRINT(VX_ZONE_INFO, "[LBIST] Restoring DMPAC firewall state\n");

#if defined(ENABLE_DOF)
    if (((int32_t)VX_SUCCESS == status) && (0u != appDmpacFirewallShouldEnable(dmpacInstId, DMPAC_FWL_REGION_DOF_MAIN)))
    {
        Vhwa_M2mDofSl2Info sl2Info = {0};
        int32_t ret = Vhwa_m2mDofGetSl2Info(&sl2Info);

        if ((ret == FVID2_SOK) &&
            (sl2Info.mainRegionSl2Addr != 0u) &&
            (sl2Info.refRegionSl2Addr != 0u) &&
            ((sl2Info.sl2Addr & 0x3FFFu) == 0u) &&
            ((sl2Info.sl2Size & 0x3FFFu) == 0u))
        {
            /* Restore DOF MAIN region */
            status = (int32_t)appDmpacFirewallEnable(dmpacInstId, DMPAC_FWL_REGION_DOF_MAIN,
                sl2Info.mainRegionSl2Addr, sl2Info.mainRegionSl2Size,
                TISCI_PRIV_ID_DOF, TISCI_PRIV_ID_MAIN_0_R5_1, TISCI_PRIV_ID_FOCO_0);

            if ((int32_t)VX_SUCCESS == status)
            {
                VX_PRINT(VX_ZONE_INFO, "[LBIST] Restored DMPAC DOF MAIN firewall\n");

                /* Restore DOF REFERENCE region if it was enabled */
                if (appDmpacFirewallShouldEnable(dmpacInstId, DMPAC_FWL_REGION_DOF_REFERENCE) != 0U)
                {
                    status = (int32_t)appDmpacFirewallEnable(dmpacInstId, DMPAC_FWL_REGION_DOF_REFERENCE,
                        sl2Info.refRegionSl2Addr, sl2Info.refRegionSl2Size,
                        TISCI_PRIV_ID_DOF_RD, TISCI_PRIV_ID_MAIN_0_R5_1, TISCI_PRIV_ID_FOCO_0);

                    if ((int32_t)VX_SUCCESS == status)
                    {
                        VX_PRINT(VX_ZONE_INFO, "[LBIST] Restored DMPAC DOF REFERENCE firewall\n");
                    }
                    else
                    {
                        status = appDmpacFirewallDisable(dmpacInstId, DMPAC_FWL_REGION_DOF_MAIN);
                    }
                }
            }
        }
    }
#endif

#if defined(ENABLE_SDE)
    if (((int32_t)VX_SUCCESS == status) && (0u != appDmpacFirewallShouldEnable(dmpacInstId, DMPAC_FWL_REGION_SDE)))
    {
        Vhwa_M2mSdeSl2Info sl2Info = {0};
        int32_t ret = Vhwa_m2mSdeGetSl2Info(&sl2Info);

        if ((ret == FVID2_SOK) &&
            ((sl2Info.sl2Addr & 0x3FFFu) == 0u) &&
            ((sl2Info.sl2Size & 0x3FFFu) == 0u))
        {
            status = (int32_t)appDmpacFirewallEnable(dmpacInstId, DMPAC_FWL_REGION_SDE,
                sl2Info.sl2Addr, sl2Info.sl2Size, TISCI_PRIV_ID_SDE,
                TISCI_PRIV_ID_MAIN_0_R5_1, TISCI_PRIV_ID_FOCO_1);

            if ((int32_t)VX_SUCCESS == status)
            {
                VX_PRINT(VX_ZONE_INFO, "[LBIST] Restored DMPAC SDE firewall\n");
            }
        }
    }
#endif

    return status;
}
#endif

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: The calls to this function are out of scope for the imaging test framework.
Effect on this unit: The function is expected to handle VPAC-specific remote service commands like firewall configuration.
<justification end> */
static int32_t appVhwaVpacHandler(char *service_name, uint32_t cmd, void *prm, uint32_t prm_size, uint32_t flags)
{
    int32_t  status = -1;

    (void)service_name;
    (void)prm;
    (void)prm_size;
    (void)flags;

#if defined(ENABLE_VISS) && defined(ENABLE_LDC) && defined(ENABLE_MSC) && defined(ENABLE_NF)
    uint32_t fwlIdx = 0u;
#endif

    if (0 != APP_DEBUG_SCICLIENT)
    {
        appLogPrintf(" VHWA VPAC Remote Service: Received command %08x to configure VPAC !!!\n", cmd);
    }

    switch(cmd)
    {
#if defined(SOC_J721E) || defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)
        case APP_VPAC_ENABLE_FIREWALL:
            {
                uint32_t vpacInstId = 0;  /* Default to VPAC0 */

                if (0 != APP_DEBUG_SCICLIENT)
                {
                    appLogPrintf("appVhwaVpacHandler: Enabling Firewall for VPAC modules\n");
                }

                /* Accept optional vpacInstId parameter, default to 0 if not provided */
                if ((sizeof(uint32_t) == prm_size) && (NULL != prm))
                {
                    uint32_t prmValue;
                    (void)memcpy(&prmValue, (const uint32_t*)prm, sizeof(uint32_t));
                    if ((0u == prmValue) || (1u == prmValue))
                    {
                        vpacInstId = prmValue;
                    }
                    else
                    {
                        appLogPrintf(" VHWA VPAC Remote Service: ERROR: Invalid vpacInstId (must be 0 or 1)!!!\n");
                        break;
                    }
                }

                status = appVhwaEnableVpacFirewall(vpacInstId);
            }
            break;
        case APP_VPAC_DISABLE_FIREWALL:
            {
                uint32_t vpacInstId = 0;  /* Default to VPAC0 */

                if (0 != APP_DEBUG_SCICLIENT)
                {
                    appLogPrintf("appVhwaVpacHandler: Disabling Firewall for VPAC modules\n");
                }

                /* Accept optional vpacInstId parameter, default to 0 if not provided */
                if ((sizeof(uint32_t) == prm_size) && (NULL != prm))
                {
                    uint32_t prmValue;
                    (void)memcpy(&prmValue, (const uint32_t*)prm, sizeof(uint32_t));
                    if ((0u == prmValue) || (1u == prmValue))
                    {
                        vpacInstId = prmValue;
                    }
                    else
                    {
                        appLogPrintf(" VHWA VPAC Remote Service: ERROR: Invalid vpacInstId (must be 0 or 1)!!!\n");
                        break;
                    }
                }

                status = appVhwaDisableVpacFirewall(vpacInstId);
            }
            break;
#endif
#if defined(ENABLE_VISS) && defined(ENABLE_LDC) && defined(ENABLE_MSC) && defined(ENABLE_NF)
            /* Acquire VPAC lock */
            case APP_VPAC_ACQUIRE_LOCK:
                if ((NULL != prm) && (sizeof(uint32_t) == prm_size))
                {
                    uint32_t cmdPrms;
                    (void)memcpy(&cmdPrms, (const uint32_t*)prm, sizeof(uint32_t));
                    uint32_t ctrlCmd;

                    switch (cmdPrms)
                    {
                        case APP_VHWA_VPAC0_INST:
                            ctrlCmd = VHWA_M2M_IOCTL_LBIST_VPAC0_ACQUIRE_LOCK;
                            fwlIdx = 0u;
                            status = 0;
                            break;
                        case APP_VHWA_VPAC1_INST:
                            ctrlCmd = VHWA_M2M_IOCTL_LBIST_VPAC1_ACQUIRE_LOCK;
                            fwlIdx = 1u;
                            status = 0;
                            break;
                        default:
                            VX_PRINT(VX_ZONE_ERROR, "[appVhwaVpacHandler] ERROR: Incorrect VPAC Instance \n");
                            status = -1;
                            break;
                    }

                    if ((0 == status) && (NULL != gFvid2VhwaDrvHandle))
                    {
                        status = Fvid2_control(gFvid2VhwaDrvHandle, ctrlCmd, NULL, NULL);
                        if (status != FVID2_SOK)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "[appVhwaVpacHandler] ERROR: Failed to acquire lock\n");
                            status = FVID2_EFAIL;
                        }
                    }
                    else
                    {
                        if (NULL == gFvid2VhwaDrvHandle)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "[appVhwaVpacHandler] ERROR: Driver handle is NULL \n");
                        }

                        if (0 != status)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "[appVhwaVpacHandler] ERROR: vhwaPowerOnModules Failed \n");
                        }
                    }
                }
                else
                {
                    appLogPrintf(" appVhwaVpacHandler: ERROR: Invalid VPAC Params Size !!!\n");
                }
                if (0 == status)
                {
                    status = (int32_t)appVpacFirewallDisableAll((uint8_t)fwlIdx);
                }
                break;
            /* Release VPAC lock */
            case APP_VPAC_RELEASE_LOCK:
                if ((NULL != prm) && (sizeof(uint32_t) == prm_size))
                {
                    uint32_t cmdPrms;
                    (void)memcpy(&cmdPrms, (const uint32_t*)prm, sizeof(uint32_t));
                    uint32_t ctrlCmd;

                    switch (cmdPrms)
                    {
                        case APP_VHWA_VPAC0_INST:
                            ctrlCmd = VHWA_M2M_IOCTL_LBIST_VPAC0_RELEASE_LOCK;
                            fwlIdx = 0;
                            status = 0;
                            break;
                        case APP_VHWA_VPAC1_INST:
                            ctrlCmd = VHWA_M2M_IOCTL_LBIST_VPAC1_RELEASE_LOCK;
                            fwlIdx = 1;
                            status = 0;
                            break;
                        default:
                            VX_PRINT(VX_ZONE_ERROR, "[appVhwaVpacHandler] ERROR: Incorrect VPAC Instance \n");
                            status = -1;
                            break;
                    }

                    if (0 == status)
                    {
                        /* First Power On Modules, before releasing locks */
                        status = vhwaPowerOnModules(cmdPrms);
                    }

                    if (0 == status)
                    {
                        /* Now Revert the firewall settings */
                        status = appVhwaEnableVpacFirewallAfterLbist(fwlIdx);
                    }

                    if ((0 == status) && (NULL != gFvid2VhwaDrvHandle))
                    {
                        status = Fvid2_control(gFvid2VhwaDrvHandle, ctrlCmd, NULL, NULL);
                        if (status != FVID2_SOK)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "[appVhwaVpacHandler] ERROR: Failed to release lock\n");
                            status = FVID2_EFAIL;
                        }
                    }
                    else
                    {
                        if (NULL == gFvid2VhwaDrvHandle)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "[appVhwaVpacHandler] ERROR: Driver handle is NULL \n");
                        }

                        if (0 != status)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "[appVhwaVpacHandler] ERROR: vhwaPowerOnModules Failed \n");
                        }
                    }
                }
                else
                {
                    appLogPrintf(" appVhwaVpacHandler: ERROR: Invalid VPAC Params Size !!!\n");
                }
                break;
            #endif
        case 0xFFFFFFFFu:  /* Reserved case - ensures MISRA Rule 16.6 compliance across all build configs */
            status = FVID2_EUNSUPPORTED_CMD;
            break;
        default:
            appLogPrintf(" VHWA VPAC Remote Service: ERROR: Unknown command %08x !!!\n", cmd);
            break;
    }

    return status;
}
/* LDRA_JUSTIFY_END */

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: The calls to this function are out of scope for the imaging test framework.
Effect on this unit: The function is expected to ensure proper cleanup and resource release for remote services, maintaining system stability and preventing resource leaks.
<justification end> */
static int32_t appVhwaHandler(char *service_name, uint32_t cmd, void *prm, uint32_t prm_size, uint32_t flags)
{
    int32_t  status = -1;

    (void)flags;
    (void)service_name;

    if (NULL != prm)
    {
        if (0 != APP_DEBUG_SCICLIENT)
        {
            appLogPrintf(" VHWA Remote Service: Received command %08x to configure VHWA !!!\n", cmd);
        }

        switch(cmd)
        {
            #if defined(ENABLE_SDE)
            case APP_DMPAC_SDE_SL2_FREE:
                /* free SDE SL2 memory */
                status = Vhwa_m2mSdeFreeSl2();
                break;

            case APP_DMPAC_SDE_SL2_REALLOC:
                /* realloc SDE SL2 for 2MP */
                if (sizeof(Vhwa_M2mSdeSl2AllocPrms) == prm_size)
                {
                    Vhwa_M2mSdeSl2AllocPrms cmdPrms;
                    (void)memcpy(&cmdPrms, (const Vhwa_M2mSdeSl2AllocPrms*)prm, sizeof(Vhwa_M2mSdeSl2AllocPrms));
                    status = Vhwa_m2mSdeAllocSl2(&cmdPrms);
                } else
                {
                    appLogPrintf(" VHWA Remote Service: ERROR: Invalid SDE SL2 parameters passed !!!\n");
                }

                break;
            #endif

            #if defined(ENABLE_DOF)
            case APP_DMPAC_DOF_SL2_FREE:
                /* free DOF SL2 memory */
                status = Vhwa_m2mDofFreeSl2();
                break;

            case APP_DMPAC_DOF_SL2_REALLOC:
                /* realloc DOF SL2 for 2MP */
                if (sizeof(Vhwa_M2mDofSl2AllocPrms) == prm_size)
                {
                    Vhwa_M2mDofSl2AllocPrms cmdPrms;
                    (void)memcpy(&cmdPrms, (const Vhwa_M2mDofSl2AllocPrms*)prm, sizeof(Vhwa_M2mDofSl2AllocPrms));
                    status = Vhwa_m2mDofAllocSl2(&cmdPrms);
                } else
                {
                    appLogPrintf(" VHWA Remote Service: ERROR: Invalid DOF SL2 parameters passed !!!\n");
                }
                break;
            #endif

#if defined(SOC_J721E) || defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)
            case APP_DMPAC_ENABLE_FIREWALL:
                if (0 != APP_DEBUG_SCICLIENT)
                {
                    appLogPrintf("appVhwaHandler: Enabling Firewall for DMPAC modules");
                }
                status = appVhwaEnableDmpacFirewall();
                break;
            case APP_DMPAC_DISABLE_FIREWALL:
                if (0 != APP_DEBUG_SCICLIENT)
                {
                    appLogPrintf("appVhwaHandler: Disabling Firewall for DMPAC modules");
                }
                status = appVhwaDisableDmpacFirewall();
                break;
#endif
            case APP_VPAC_720_DMPAC_480:
                #if !defined(SOC_AM62A)
                if (0 != APP_DEBUG_SCICLIENT)
                    {
                        appLogPrintf("SCICLIENT: Sciclient_pmSetModuleClkFreq module=%u clk=%u freq=%u%06u\n",
                                    TISCI_DEV_DMPAC0, TISCI_DEV_DMPAC0_CLK, (uint32_t)(480000000 / 1000000), (uint32_t)(480000000 % 1000000));
                    }
                    status = Sciclient_pmSetModuleClkFreq(TISCI_DEV_DMPAC0, TISCI_DEV_DMPAC0_CLK, 480000000, 0, SCICLIENT_SERVICE_WAIT_FOREVER);
                    if (status != 0)
                    {
                        appLogPrintf("SCICLIENT: ERROR: Sciclient_pmSetModuleClkFreq failed\n");
                    }
                    else
                    {
                        if (0 != APP_DEBUG_SCICLIENT)
                        {
                            appLogPrintf("SCICLIENT: Sciclient_pmSetModuleClkFreq success\n");
                        }
                    }
                #endif

                #if defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)
                    if (0 != APP_DEBUG_SCICLIENT)
                    {
                        appLogPrintf("SCICLIENT: Sciclient_pmSetModuleClkFreq module=%u clk=%u freq=%u%06u\n",
                                    TISCI_DEV_VPAC0, TISCI_DEV_VPAC0_MAIN_CLK, (uint32_t)(720000000 / 1000000), (uint32_t)(720000000 % 1000000));
                    }
                    status = Sciclient_pmSetModuleClkFreq(TISCI_DEV_VPAC0, TISCI_DEV_VPAC0_MAIN_CLK, 720000000, 0, SCICLIENT_SERVICE_WAIT_FOREVER);
                    if (status != 0)
                    {
                        appLogPrintf("SCICLIENT: ERROR: Sciclient_pmSetModuleClkFreq failed\n");
                    }
                    else
                    {
                        if (0 != APP_DEBUG_SCICLIENT)
                        {
                            appLogPrintf("SCICLIENT: Sciclient_pmSetModuleClkFreq success\n");
                        }
                    }
                #else
                    if (0 != APP_DEBUG_SCICLIENT)
                    {
                        appLogPrintf("SCICLIENT: Sciclient_pmSetModuleClkFreq module=%u clk=%u freq=%u%06u\n",
                                    TISCI_DEV_VPAC0, TISCI_DEV_VPAC0_CLK, (uint32_t)(720000000 / 1000000), (uint32_t)(720000000 % 1000000));
                    }
                    status = Sciclient_pmSetModuleClkFreq(TISCI_DEV_VPAC0, TISCI_DEV_VPAC0_CLK, 720000000, 0, SCICLIENT_SERVICE_WAIT_FOREVER);
                    if (status != 0)
                    {
                        appLogPrintf("SCICLIENT: ERROR: Sciclient_pmSetModuleClkFreq failed\n");
                    }
                    else
                    {
                        if (0 != APP_DEBUG_SCICLIENT)
                        {
                            appLogPrintf("SCICLIENT: Sciclient_pmSetModuleClkFreq success\n");
                        }
                    }
                #endif
                status = 0;
                break;

            case APP_VPAC_650_DMPAC_520:
                #if !defined(SOC_AM62A)
                if (0 != APP_DEBUG_SCICLIENT)
                {
                    appLogPrintf("SCICLIENT: Sciclient_pmSetModuleClkFreq module=%u clk=%u freq=%u%06u\n",
                                TISCI_DEV_DMPAC0, TISCI_DEV_DMPAC0_CLK, (uint32_t)(520000000 / 1000000), (uint32_t)(520000000 % 1000000));
                }
                status = Sciclient_pmSetModuleClkFreq(TISCI_DEV_DMPAC0, TISCI_DEV_DMPAC0_CLK, 520000000, 0, SCICLIENT_SERVICE_WAIT_FOREVER);
                if (status != 0)
                {
                    appLogPrintf("SCICLIENT: ERROR: Sciclient_pmSetModuleClkFreq failed\n");
                }
                else
                {
                    if (0 != APP_DEBUG_SCICLIENT)
                    {
                        appLogPrintf("SCICLIENT: Sciclient_pmSetModuleClkFreq success\n");
                    }
                }
                #endif

                #if defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)
                    if (0 != APP_DEBUG_SCICLIENT)
                    {
                        appLogPrintf("SCICLIENT: Sciclient_pmSetModuleClkFreq module=%u clk=%u freq=%u%06u\n",
                                    TISCI_DEV_VPAC0, TISCI_DEV_VPAC0_MAIN_CLK, (uint32_t)(650000000 / 1000000), (uint32_t)(650000000 % 1000000));
                    }
                    status = Sciclient_pmSetModuleClkFreq(TISCI_DEV_VPAC0, TISCI_DEV_VPAC0_MAIN_CLK, 650000000, 0, SCICLIENT_SERVICE_WAIT_FOREVER);
                    if (status != 0)
                    {
                        appLogPrintf("SCICLIENT: ERROR: Sciclient_pmSetModuleClkFreq failed\n");
                    }
                    else
                    {
                        if (0 != APP_DEBUG_SCICLIENT)
                        {
                            appLogPrintf("SCICLIENT: Sciclient_pmSetModuleClkFreq success\n");
                        }
                    }
                #else
                    if (0 != APP_DEBUG_SCICLIENT)
                    {
                        appLogPrintf("SCICLIENT: Sciclient_pmSetModuleClkFreq module=%u clk=%u freq=%u%06u\n",
                                    TISCI_DEV_VPAC0, TISCI_DEV_VPAC0_CLK, (uint32_t)(650000000 / 1000000), (uint32_t)(650000000 % 1000000));
                    }
                    status = Sciclient_pmSetModuleClkFreq(TISCI_DEV_VPAC0, TISCI_DEV_VPAC0_CLK, 650000000, 0, SCICLIENT_SERVICE_WAIT_FOREVER);
                    if (status != 0)
                    {
                        appLogPrintf("SCICLIENT: ERROR: Sciclient_pmSetModuleClkFreq failed\n");
                    }
                    else
                    {
                        if (0 != APP_DEBUG_SCICLIENT)
                        {
                            appLogPrintf("SCICLIENT: Sciclient_pmSetModuleClkFreq success\n");
                        }
                    }
                #endif
                status = 0;
                break;

            #if defined(ENABLE_DOF) && defined(ENABLE_SDE)
        #if defined(SOC_J721E) || defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)
            case APP_DMPAC_ACQUIRE_LOCK:
                if (NULL != gFvid2VhwaDrvHandle)
                {
                    status = Fvid2_control(gFvid2VhwaDrvHandle, VHWA_M2M_IOCTL_LBIST_DMPAC_ACQUIRE_LOCK, NULL, NULL);
                    if (status != FVID2_SOK)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "[appVhwaHandler] ERROR: Failed to acquire lock\n");
                        status = FVID2_EFAIL;
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "[appVhwaHandler] ERROR: Driver handle is NULL \n");
                }
                if (0 == status)
                {
                    status = appDmpacFirewallDisableAll(0u);
                }
                break;
            case APP_DMPAC_RELEASE_LOCK:
                /* First Power On Modules, before releasing locks */
                status = vhwaPowerOnModules(APP_VHWA_DMPAC_INST);
                if (0 == status)
                {
                    /* Now restore the fiewall settings */
                    status = appVhwaEnableDmpacFirewallAfterLbist();
                }
                if ((0 == status) && (NULL != gFvid2VhwaDrvHandle))
                {
                    status = Fvid2_control(gFvid2VhwaDrvHandle, VHWA_M2M_IOCTL_LBIST_DMPAC_RELEASE_LOCK, NULL, NULL);
                    if (status != FVID2_SOK)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "[appVhwaHandler] ERROR: Failed to release lock\n");
                        status = FVID2_EFAIL;
                    }
                }
                else
                {
                    if (0 != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "[appVhwaHandler] ERROR: vhwaPowerOnModules Failed \n");
                    }
                    if (NULL == gFvid2VhwaDrvHandle)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "[appVhwaVpacHandler] ERROR: Driver handle is NULL \n");
                    }
                }
                break;
        #endif
            #endif
            default:
                appLogPrintf(" VHWA Remote Service: ERROR: Unknown command %08x !!!\n", cmd);
                status = FVID2_EFAIL;
                break;
        }

        if (0 == status)
        {
            if (0 != APP_DEBUG_SCICLIENT)
            {
                appLogPrintf(" VHWA Remote Service: VHWA configuration done !!!\n");
            }
        }
        else
        {
            appLogPrintf(" VHWA Remote Service: ERROR: VHWA configuration failed !!!\n");
        }
    }
    else
    {
        appLogPrintf(" VHWA Remote Service: ERROR: Invalid parameters passed !!!\n");
    }


    return status;
}
/* LDRA_JUSTIFY_END */

int32_t appVhwaRemoteServiceInit(void)
{
    int32_t status;

    status = appRemoteServiceRegister(APP_VHWA_SERVICE_NAME, appVhwaHandler);
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(status!=0)
    {
        appLogPrintf("VHWA Remote Service: ERROR: Unable to register service \n");
    }
    else
    {
        if (NULL == gFvid2VhwaDrvHandle)
        {
            /* Without this service, driver can't be used, so opening driver in the service initialization.
             * But this means DMPAC must be initialized before calling this API */
            gFvid2VhwaDrvHandle = Fvid2_create(FVID2_VHWA_M2M_LBIST_DRV_ID,
                                        0, NULL, NULL, NULL);

            if (NULL == gFvid2VhwaDrvHandle)
            {
                VX_PRINT(VX_ZONE_ERROR, "[appVhwaRemoteServiceInit] ERROR: Failed to Open FVID2 driver\n");
                status = FVID2_EFAIL;
            }
        }
        else
        {
            /* Driver is already created/opened, probably by having VPAC/DMPAC on the same core */
        }
    }
    /* LDRA_JUSTIFY_END */

    return status;
}

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: The calls to this function are out of scope for the imaging test framework.
Effect on this unit: The function is expected to ensure proper cleanup and resource release for remote services, maintaining system stability and preventing resource leaks.
<justification end> */
int32_t appVhwaRemoteServiceDeInit(void)
{
    int32_t status;

    status = appRemoteServiceUnRegister(APP_VHWA_SERVICE_NAME);
    if(status!=0)
    {
        appLogPrintf("VHWA Remote Service: ERROR: Unable to unregister service \n");
    }

    if (NULL != gFvid2VhwaDrvHandle)
    {
        (void)Fvid2_delete(gFvid2VhwaDrvHandle, NULL);
        gFvid2VhwaDrvHandle = NULL;
    }
    return status;
}
/* LDRA_JUSTIFY_END */

/**
 * \brief Initialize VHWA VPAC remote service for inter-processor communication.
 *
 * This function registers a remote service that allows other CPUs in the system
 * to control VPAC (Vision Processing Accelerator Cluster) hardware accelerator
 * configurations, particularly firewall protection for SL2 memory regions.
 *
 * The service supports the following commands:
 *   - APP_VPAC_ENABLE_FIREWALL:  Enable firewall protection for VPAC modules
 *                                 (LDC, MSC, NF, VISS) with SL2 memory access control
 *   - APP_VPAC_DISABLE_FIREWALL: Disable firewall protection for VPAC modules
 *
 * The service handler (appVhwaVpacHandler) validates SL2 memory alignment requirements
 * (16KB boundaries) and configures TISCI firewall regions for each enabled VPAC module.
 *
 * Service name: "com.ti.vhwa_vpac_remote_service" (APP_VHWA_VPAC_SERVICE_NAME)
 *
 * \return 0 on success, negative error code on failure
 *
 * \see appVhwaVpacHandler for command handling details
 * \see appVhwaVpacRemoteServiceDeInit for cleanup
 */
int32_t appVhwaVpacRemoteServiceInit(void)
{
    int32_t status;

    status = appRemoteServiceRegister(APP_VHWA_VPAC_SERVICE_NAME, appVhwaVpacHandler);
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(status!=0)
    {
        appLogPrintf("VHWA VPAC Remote Service: ERROR: Unable to register service \n");
    }
    else
    {
        if (NULL == gFvid2VhwaDrvHandle)
        {
            /* Without this service, driver can't be used, so opening driver in the service initialization.
             * But this means VPAC must be initialized before calling this API */
            gFvid2VhwaDrvHandle = Fvid2_create(FVID2_VHWA_M2M_LBIST_DRV_ID,
                                        0, NULL, NULL, NULL);

            if (NULL == gFvid2VhwaDrvHandle)
            {
                VX_PRINT(VX_ZONE_ERROR, "[appVhwaRemoteServiceInit] ERROR: Failed to Open FVID2 driver\n");
                status = FVID2_EFAIL;
            }
        }
        else
        {
            /* Driver is already created/opened, probably by having VPAC/DMPAC on the same core */
        }
    }
    /* LDRA_JUSTIFY_END */

    return status;
}

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: The calls to this function are out of scope for the imaging test framework.
Effect on this unit: The function is expected to ensure proper cleanup and resource release for remote services, maintaining system stability and preventing resource leaks.
<justification end> */
int32_t appVhwaVpacRemoteServiceDeInit(void)
{
    int32_t status;

    status = appRemoteServiceUnRegister(APP_VHWA_VPAC_SERVICE_NAME);
    if(status!=0)
    {
        appLogPrintf("VHWA VPAC Remote Service: ERROR: Unable to unregister service \n");
    }

    if (NULL != gFvid2VhwaDrvHandle)
    {
        (void)Fvid2_delete(gFvid2VhwaDrvHandle, NULL);
        gFvid2VhwaDrvHandle = NULL;
    }
    return status;
}
/* LDRA_JUSTIFY_END */

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: The calls to this function are out of scope for the imaging test framework.
Effect on this unit: The function is expected to ensure proper cleanup and resource release for remote services, maintaining system stability and preventing resource leaks.
<justification end> */
int32_t appVissRemoteServiceDeInit(void)
{
    int32_t viss_status;

    viss_status = VissRemoteServer_DeInit();
    if(viss_status!=0)
    {
        appLogPrintf(" appVissRemoteServiceDeInit: ERROR: Failed to deinitialize VISS remote server \n");
    }

    return (viss_status);
}
/* LDRA_JUSTIFY_END */

int32_t appVissRemoteServiceInit(void)
{
    int32_t viss_status;

    appLogPrintf("VISS REMOTE SERVICE: Init ... !!!\n");

    viss_status = VissRemoteServer_Init();
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(viss_status!=0)
    {
        appLogPrintf("ISS: Error: Failed to create remote VISS remote server failed. Live tuning will not work !!!\n");
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        appLogPrintf("VISS REMOTE SERVICE: Init ... Done !!!\n");
    }

    return viss_status;
}

static int32_t vhwaPowerOnModules(uint32_t instId)
{
    int32_t status = FVID2_EFAIL;

    if((uint32_t)APP_VHWA_VPAC0_INST == instId)
    {
        status = Sciclient_pmSetModuleState(TISCI_DEV_VPAC0, TISCI_MSG_VALUE_DEVICE_SW_STATE_ON, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER);
        if (FVID2_SOK != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "[LBIST] ERROR:Failed to power on VPAC0\n");
        }
    }
    #if defined(SOC_J784S4) || defined(SOC_J742S2)
    else if((uint32_t)APP_VHWA_VPAC1_INST == instId)
    {
        status = Sciclient_pmSetModuleState(TISCI_DEV_VPAC1, TISCI_MSG_VALUE_DEVICE_SW_STATE_ON, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER);
        if (FVID2_SOK != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "[LBIST] ERROR:Failed to power on VPAC1\n");
        }
    }
    #endif
    #if !defined(SOC_AM62A)
    else /* DMPAC */
    {
        status = Sciclient_pmSetModuleState(TISCI_DEV_DMPAC0, TISCI_MSG_VALUE_DEVICE_SW_STATE_ON, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER);
        if (FVID2_SOK == status)
        {
#if !defined(SOC_J722S) /* J722S doesn't require separate power domain for SDE */
            status = Sciclient_pmSetModuleState(TISCI_DEV_DMPAC0_SDE_0, TISCI_MSG_VALUE_DEVICE_SW_STATE_ON, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER);
            if (FVID2_SOK != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "[LBIST] ERROR:Failed to power on SDE\n");
            }
#endif
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "[LBIST] ERROR:Failed to power on DMPAC\n");
        }
    }
    #endif

    return (status);
}
