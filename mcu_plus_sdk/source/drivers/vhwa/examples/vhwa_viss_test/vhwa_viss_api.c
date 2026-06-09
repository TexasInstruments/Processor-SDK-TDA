/*
*  Copyright (c) Texas Instruments Incorporated 2019
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
*  \file vhwa_viss_api.c
*
*  \brief VHWA VISS APIs
*/

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <include/vhwa_m2mViss.h>
#include <examples/include/vhwa_viss_test_api.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
/* None */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */


/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */
#if defined VHWA_VPAC_IP_REV_VPAC || defined VHWA_VPAC_IP_REV_VPAC3
static int32_t AppViss_CompareCrc(AppViss_TestParams *tPrms, uint32_t hIdx);
#endif

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

AppViss_TestObject  gAppVissTestObject[VHWA_M2M_VISS_MAX_HANDLES];
bool gConfigThroughUDMA = false;
#if defined VHWA_VPAC_IP_REV_VPAC || defined VHWA_VPAC_IP_REV_VPAC3
AppCrc_hdlPrms gVissCrcChHandle;
#elif defined VHWA_VPAC_IP_REV_VPAC3L 
bool gCopyGlbceCtxThroughBcdmaFlag = false;
uint32_t configTicks;
#endif

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t AppViss_Init(Udma_DrvHandle udmaDrvHndl)
{
    int32_t                 status;
    Vhwa_M2mVissSl2Params   sl2AllocPrms;
    Vhwa_M2mVissInitParams  initPrms;
    #if defined (SOC_J742S2)
    /* Powerup VPAC0 module */
    Sciclient_pmSetModuleState (TISCI_DEV_VPAC0,
            TISCI_MSG_VALUE_DEVICE_SW_STATE_ON,
            TISCI_MSG_FLAG_AOP,
            0xFFFFFFFFU);
    /* Powerup VPAC1 module */
    Sciclient_pmSetModuleState (TISCI_DEV_VPAC1,
            TISCI_MSG_VALUE_DEVICE_SW_STATE_ON,
            TISCI_MSG_FLAG_AOP,
            0xFFFFFFFFU);
    #endif
    /* Initialize VISS Init parameters */
    Vhwa_m2mVissInitParamsInit(&initPrms);

    /* Set UDMA driver handle */
    initPrms.udmaDrvHndl = udmaDrvHndl;

    /* enable config through UDMA */
    initPrms.configThroughUdmaFlag = gConfigThroughUDMA;
    #if defined VHWA_VPAC_IP_REV_VPAC3L 
    initPrms.copyGlbceCtxThroughBcdmaFlag = gCopyGlbceCtxThroughBcdmaFlag;
    #endif

    status = Vhwa_m2mVissInit(&initPrms);
    if (FVID2_SOK != status)
    {
        Vhwa_print(" VISS_TEST_APP: VISS Init Failed\n");
    }
    else
    {
        Vhwa_m2mVissSl2ParamsInit(&sl2AllocPrms);

        status = Vhwa_m2mVissAllocSl2(&sl2AllocPrms);
        if (FVID2_SOK != status)
        {
            Vhwa_print(" VISS_TEST_APP: SL2 Alloc Failed !!!\n");
        }
    }

    Fvid2Utils_memset(gAppVissTestObject, 0x0,
        sizeof(AppViss_TestObject)*VHWA_M2M_VISS_MAX_HANDLES);

    return (status);
}

void AppViss_deInit(Udma_DrvHandle udmaDrvHndl)
{
    int32_t         status;

    Vhwa_m2mVissDeInit();

    status = Udma_deinit(udmaDrvHndl);
    if(UDMA_SOK != status)
    {
        Vhwa_print("[Error] UDMA deinit failed!!\n");
    }

    Fvid2_deInit(NULL);
}


int32_t AppVissFrameComplCb(Fvid2_Handle handle, void *appData)
{
    AppViss_TestObject *tObj = (AppViss_TestObject *)appData;

    if (NULL != tObj)
    {
        VhwaAl_SemaphoreP_post(tObj->waitForProcessCmpl);
    }

    return FVID2_SOK;
}

void AppVissErrorCb(Fvid2_Handle handle, uint32_t errEvents, void *appData)
{
    AppViss_TestObject *tObj = (AppViss_TestObject *)appData;

    if (NULL != tObj)
    {
        tObj->err_stat |= errEvents;
        if(0u != tObj->err_stat)
        {
            VhwaAl_SemaphoreP_post(tObj->waitForProcessCmpl);
        }
    }
}

void AppVissWdtimerErrorCb(Fvid2_Handle handle, uint32_t wdTimerErrEvents, void *appData)
{
    AppViss_TestObject *tObj = (AppViss_TestObject *)appData;
    if (NULL != tObj)
    {
        tObj->wdTimerErrStatus |= wdTimerErrEvents;
        if(0u != tObj->wdTimerErrStatus)
        {
            VhwaAl_SemaphoreP_post(tObj->waitForProcessCmpl);
        }
    }
}
int32_t AppViss_Create(AppViss_TestParams *tPrms, uint32_t hidx)
{
    int32_t              status;
    AppViss_TestObject   *tObj = &gAppVissTestObject[hidx];
    uint32_t             instId = VHWA_M2M_VPAC_0_VISS_DRV_INST_ID_0;
    #if defined(MCU_PLUS_SDK)
    #if defined(VHWA_FREERTOS_SUPPORT)
    status = SemaphoreP_constructBinary(&tObj->waitForProcessCmpl, 0);
    #else
    status = SemaphoreP_constructMutex(&tObj->waitForProcessCmpl);
    #endif
    #else    
    SemaphoreP_Params   params;

    SemaphoreP_Params_init(&params);
    params.mode = SemaphoreP_Mode_BINARY;
    tObj->waitForProcessCmpl = SemaphoreP_create(0U, &params);
    if (NULL != tObj->waitForProcessCmpl)
    #endif
    #if defined(MCU_PLUS_SDK)
    if (SYSTEMP_SUCCESS == status)
    #endif
    {
        if(UTRUE == tPrms->testCfg[hidx]->isLsePsaAvail)
        {
            tObj->createArgs.enablePsa = UTRUE;
        }
        else
        {
            tObj->createArgs.enablePsa = UFALSE;
        }

        tObj->cbPrms.cbFxn   = AppVissFrameComplCb;
        tObj->cbPrms.appData = tObj;

        #if defined VHWA_PDK
        if(tPrms->isPerformanceTest)
        {
            tObj->createArgs.getTimeStamp = Vhwa_getTime;
        }
        #endif
#if defined VHWA_PDK
#if (VPAC_TEST_INSTANCE == 0)
        instId = VHWA_M2M_VPAC_0_VISS_DRV_INST_ID_0;
#endif
#if (VPAC_TEST_INSTANCE == 1)
        instId = VHWA_M2M_VPAC_1_VISS_DRV_INST_ID_0;
#endif
#endif

        tObj->handle = Fvid2_create(FVID2_VHWA_M2M_VISS_DRV_ID,
            instId, (void *)&tObj->createArgs,
            NULL, &tObj->cbPrms);
    }
    else
    {
        Vhwa_print (" Could not Create Semaphore %d \n", hidx);
    }

    if (NULL != tObj->handle)
    {
        status = FVID2_SOK;
    }
    else
    {
        Vhwa_print (" Could not Create Handlle%d \n", hidx);
        status = FVID2_EFAIL;
    }

    return (status);
}


void AppViss_Delete(AppViss_TestParams *tPrms, uint32_t hidx)
{
    AppViss_TestObject   *tObj = &gAppVissTestObject[hidx];

    if (NULL != tObj->handle)
    {
        Fvid2_delete(tObj->handle, NULL);
        tObj->handle = NULL;
    }

    #if defined VHWA_PDK
    if (NULL != tObj->waitForProcessCmpl)
    {
        SemaphoreP_delete(tObj->waitForProcessCmpl);
        tObj->waitForProcessCmpl = NULL;
    }
    #else
    SemaphoreP_destruct(&tObj->waitForProcessCmpl);
    #endif
}

int32_t AppViss_SetParams(AppViss_TestParams *tPrms, uint32_t hidx)
{
    int32_t             status;
    AppViss_TestObject *tObj = &gAppVissTestObject[hidx];
    AppViss_TestConfig *tCfg = NULL;
    Viss_ErrEventParams  errPrms;
    Viss_WdTimerErrEventParams  wdTimerErrEvtPrms;

    tCfg = tPrms->testCfg[hidx];

    status = Fvid2_control(tObj->handle, IOCTL_VHWA_M2M_VISS_SET_PARAMS,
        (void *)&tCfg->vissPrms, NULL);
    
    if(FVID2_SOK == status)
    {
        /* Mask out watchdog timer events for regular error callback */
        errPrms.errEvents = tCfg->enableErrorEvents & ~VHWA_VISS_WDTIMER_ERR;

        errPrms.cbFxn = AppVissErrorCb;

        errPrms.appData = tObj;

        status = Fvid2_control(tObj->handle,
            IOCTL_VHWA_M2M_VISS_REGISTER_ERR_CB, &errPrms, NULL);
    }
    if (FVID2_SOK == status)
    {
        wdTimerErrEvtPrms.wdTimeoutCycles = tCfg->wdTimeoutCycles;
        /* Mask to only watchdog timer events */
        wdTimerErrEvtPrms.wdTimerErrEvents = tCfg->enableErrorEvents & VHWA_VISS_WDTIMER_ERR;
        wdTimerErrEvtPrms.cbFxn = AppVissWdtimerErrorCb;
        wdTimerErrEvtPrms.appData = tObj;
        status = Fvid2_control(tObj->handle,
            IOCTL_VHWA_M2M_VISS_REGISTER_WDTIMER_ERR_CB, &wdTimerErrEvtPrms, NULL);
    }

    return (status);
}

int32_t AppViss_SetAllConfig(AppViss_TestParams *tPrms, uint32_t hidx)
{
    int32_t             status = FVID2_EBADARGS;
    #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L 
    Vhwa_M2mVissParams *vissPrms = NULL;
    #endif

    if ((NULL != tPrms) && (hidx < VHWA_M2M_VISS_MAX_HANDLES))
    {
        #if defined VHWA_VPAC_IP_REV_VPAC3 || defined VHWA_VPAC_IP_REV_VPAC3L 
        vissPrms = &tPrms->testCfg[hidx]->vissPrms;
        #endif
        status = AppViss_SetParams(tPrms, hidx);
        if (FVID2_SOK != status)
        {
            Vhwa_print(" VISS_TEST_APP: SetParams Failed \n");
            status = FVID2_EFAIL;
            return status;
        }

        status = AppViss_SetRfeConfig(tPrms, hidx);
        if (FVID2_SOK != status)
        {
            Vhwa_print(" VISS_TEST_APP: Set RFE Config Failed \n");
            status = FVID2_EFAIL;
            return status;
        }
        status = AppViss_SetFcpConfig(tPrms, hidx);
        if (FVID2_SOK != status)
        {
            Vhwa_print(" VISS_TEST_APP: Set FCP Config Failed \n");
            status = FVID2_EFAIL;
            return status;
        }
        status = AppViss_SetGlbceConfig(tPrms, hidx);
        if (FVID2_SOK != status)
        {
            Vhwa_print(" VISS_TEST_APP: Set GLBCE Config Failed \n");
            status = FVID2_EFAIL;
            return status;
        }
        status = AppViss_SetNsf4Config(tPrms, hidx);
        if (FVID2_SOK != status)
        {
            Vhwa_print(" VISS_TEST_APP: Set NSF4 Config Failed \n");
            status = FVID2_EFAIL;
            return status;
        }
        status = AppViss_SetH3aConfig(tPrms, hidx);
        if (FVID2_SOK != status)
        {
            Vhwa_print(" VISS_TEST_APP: Set H3A Config Failed \n");
            status = FVID2_EFAIL;
            return status;
        }
#if defined (VHWA_VPAC_IP_REV_VPAC3)
        status = AppViss_SetFcp2Config(tPrms, hidx);
        if (FVID2_SOK != status)
        {
            Vhwa_print(" VISS_TEST_APP: Set FCP2 Config Failed \n");
            status = FVID2_EFAIL;
            return status;
        }
#endif
#if defined (VHWA_VPAC_IP_REV_VPAC3) || defined (VHWA_VPAC_IP_REV_VPAC3L)
    if(vissPrms->enableCac == (uint32_t)TRUE)
    {
        status = AppViss_SetCacConfig(tPrms, hidx);
        if (FVID2_SOK != status)
        {
            Vhwa_print(" VISS_TEST_APP: Set CAC Config Failed \n");
            status = FVID2_EFAIL;
            return status;
        }
    }
#endif
#if defined (VHWA_VPAC_IP_REV_VPAC3L)
        if(vissPrms->enablePcid == (uint32_t)TRUE)
        {
            status = AppViss_SetPcidParams(tPrms, hidx);
            if (FVID2_SOK != status)
            {
                DebugP_log(" VISS_TEST_APP: SetParams for PCID Failed \n");
                status = FVID2_EFAIL;
                return status;
            }
        }
#endif
    }

    return status;
}


int32_t AppViss_SetRfeConfig(AppViss_TestParams *tPrms, uint32_t hidx)
{
    int32_t             status = FVID2_SOK;
    AppViss_Cfg        *vissCfg = NULL;
    Rfe_Control         rfeCtrl;
    AppViss_TestObject *tObj = &gAppVissTestObject[hidx];

    vissCfg = tPrms->testCfg[hidx]->vissCfg;

    if (NULL != vissCfg->lPwlCfg)
    {
        /* PWL for Long Input */
        rfeCtrl.module  = RFE_MODULE_PWL1;
        rfeCtrl.pwl1Cfg = vissCfg->lPwlCfg;
        status = Fvid2_control(tObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->sPwlCfg)
    {
        /* PWL for Short Input */
        rfeCtrl.module  = RFE_MODULE_PWL2;
        rfeCtrl.pwl2Cfg = vissCfg->sPwlCfg;
        status = Fvid2_control(tObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->vsPwlCfg)
    {
        /* PWL for Very Short Input */
        rfeCtrl.module  = RFE_MODULE_PWL3;
        rfeCtrl.pwl3Cfg = vissCfg->vsPwlCfg;
        status = Fvid2_control(tObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->lLutCfg)
    {
        /* PWL Lut for Long Input */
        rfeCtrl.module          = RFE_MODULE_DECOMP_LUT1;
        rfeCtrl.decomp1Cfg      = vissCfg->lLutCfg;
        status = Fvid2_control(tObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->sLutCfg)
    {
        /* PWL Lut for Short Input */
        rfeCtrl.module          = RFE_MODULE_DECOMP_LUT2;
        rfeCtrl.decomp2Cfg      = vissCfg->sLutCfg;
        status = Fvid2_control(tObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->vsLutCfg)
    {
        /* PWL Lut for Very Short Input */
        rfeCtrl.module          = RFE_MODULE_DECOMP_LUT3;
        rfeCtrl.decomp3Cfg      = vissCfg->vsLutCfg;
        status = Fvid2_control(tObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->wdr1Cfg)
    {
        /* WDR Merge 1 block configuration */
        rfeCtrl.module       = RFE_MODULE_WDR_MERGE_MA1;
        rfeCtrl.wdrMergeMa1  = vissCfg->wdr1Cfg;
        status = Fvid2_control(tObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->wdr2Cfg)
    {
        rfeCtrl.module       = RFE_MODULE_WDR_MERGE_MA2;
        rfeCtrl.wdrMergeMa2  = vissCfg->wdr2Cfg;
        status = Fvid2_control(tObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->comp20To16LutCfg)
    {
        /* Set Companding Lut to convert from 20 to 16bits */
        rfeCtrl.module       = RFE_MODULE_COMP_LUT;
        rfeCtrl.compCfg      = vissCfg->comp20To16LutCfg;
        status = Fvid2_control(tObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->dpcOtf)
    {
        rfeCtrl.module       = RFE_MODULE_DPC_OTF;
        rfeCtrl.dpcOtfCfg    = vissCfg->dpcOtf;
        status = Fvid2_control(tObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->dpcLut)
    {
        rfeCtrl.module       = RFE_MODULE_DPC_LUT;
        rfeCtrl.dpcLutCfg    = vissCfg->dpcLut;
        status = Fvid2_control(tObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->lscCfg)
    {
        rfeCtrl.module      = RFE_MODULE_LSC;
        rfeCtrl.lscConfig   = vissCfg->lscCfg;
        status = Fvid2_control(tObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->wbCfg)
    {
        rfeCtrl.module       = RFE_MODULE_GAIN_OFST;
        rfeCtrl.wbConfig     = vissCfg->wbCfg;
        status = Fvid2_control(tObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }
    /* Newly added for default case */
    if (NULL != vissCfg->lPwlCfg)
    {
        /* PWL for Long Input */
        rfeCtrl.module  = 0x20;
        rfeCtrl.pwl1Cfg = vissCfg->lPwlCfg;
        status = Fvid2_control(tObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    return (status);
}

int32_t AppViss_SetFcpConfig(AppViss_TestParams *tPrms, uint32_t hidx)
{
    int32_t             status = FVID2_SOK;
    Fcp_Control         fcpCtrl;
    AppViss_Cfg        *vissCfg = NULL;
    Vhwa_M2mVissParams *vissPrms = NULL;
    AppViss_TestObject *tObj = &gAppVissTestObject[hidx];

    vissCfg = tPrms->testCfg[hidx]->vissCfg;
    vissPrms = &tPrms->testCfg[hidx]->vissPrms;

    /* newly added for default case, as it not breaking or making any error case */
    if (NULL != vissCfg->comp16To12LutCfg)
    {
        fcpCtrl.module = 15U;
        fcpCtrl.inComp = vissCfg->comp16To12LutCfg;
        status = Fvid2_control(tObj->handle, IOCTL_FCP_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->comp16To12LutCfg)
    {
        fcpCtrl.module = FCP_MODULE_COMPANDING;
        fcpCtrl.inComp = vissCfg->comp16To12LutCfg;
        status = Fvid2_control(tObj->handle, IOCTL_FCP_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->cfaCfg)
    {
        fcpCtrl.module          = FCP_MODULE_CFA;
        fcpCtrl.cfa             = vissCfg->cfaCfg;
        status = Fvid2_control(tObj->handle, IOCTL_FCP_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->ccm)
    {
        fcpCtrl.module      = FCP_MODULE_CCM;
        fcpCtrl.ccm         = vissCfg->ccm;
        status = Fvid2_control(tObj->handle, IOCTL_FCP_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->gamma)
    {
        fcpCtrl.module = FCP_MODULE_GAMMA;
        fcpCtrl.gamma  = vissCfg->gamma;
        status = Fvid2_control(tObj->handle, IOCTL_FCP_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->rgb2Hsv)
    {
        fcpCtrl.module          = FCP_MODULE_RGB2HSV;
        fcpCtrl.rgb2Hsv         = vissCfg->rgb2Hsv;
        status = Fvid2_control(tObj->handle, IOCTL_FCP_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->rgb2yuv)
    {
        fcpCtrl.module          = FCP_MODULE_RGB2YUV;
        fcpCtrl.rgb2Yuv         = vissCfg->rgb2yuv;
        status = Fvid2_control(tObj->handle, IOCTL_FCP_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->yuvSatLutCfg)
    {
        fcpCtrl.module                  = FCP_MODULE_YUV_SAT_LUT;
        fcpCtrl.yuvSatLut               = vissCfg->yuvSatLutCfg;
        status = Fvid2_control(tObj->handle, IOCTL_FCP_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->histCfg)
    {
        fcpCtrl.module                  = FCP_MODULE_HISTOGRAM;
        fcpCtrl.hist                    = vissCfg->histCfg;
        status = Fvid2_control(tObj->handle, IOCTL_FCP_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }
#if defined VHWA_VPAC_IP_REV_VPAC3 || defined (VHWA_VPAC_IP_REV_VPAC3L)
    if (NULL != vissCfg->comp24To12LutCfg)
    {
        fcpCtrl.module = FCP_MODULE_CFA_COMPANDING;
        fcpCtrl.cLutComp = vissCfg->comp24To12LutCfg;
        status = Fvid2_control(tObj->handle, IOCTL_FCP_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->decomp16To24LutCfg)
    {
        fcpCtrl.module = FCP_MODULE_DECOMPANDING;
        fcpCtrl.dLutComp = vissCfg->decomp16To24LutCfg;
        status = Fvid2_control(tObj->handle, IOCTL_FCP_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }
#endif
    if (NULL != vissCfg->eeCfg)
    {
        if (0 != (VHWA_M2M_VISS_EE_ON_FCP0_LUMA12 & vissPrms->edgeEnhancerMode))
        {
            vissCfg->eeCfg->bypassY12 = UFALSE;
            vissCfg->eeCfg->bypassY8 = UTRUE;
            vissCfg->eeCfg->eeForY12OrY8 = 0u;
            vissCfg->eeCfg->leftShift = 0u;
            vissCfg->eeCfg->rightShift = 0u;
        }
        else if (0 != (VHWA_M2M_VISS_EE_ON_FCP0_LUMA8 & vissPrms->edgeEnhancerMode))
        {
            vissCfg->eeCfg->bypassY8 = UFALSE;
            vissCfg->eeCfg->bypassY12 = UTRUE;
            vissCfg->eeCfg->eeForY12OrY8 = 1u;
            vissCfg->eeCfg->leftShift = 2u;
            vissCfg->eeCfg->rightShift = 2u;
        }
        else
        {
            vissCfg->eeCfg->bypassY12 = UTRUE;
            vissCfg->eeCfg->bypassY8 = UTRUE;
        }

        fcpCtrl.module              = FCP_MODULE_EE;
        fcpCtrl.eeCfg               = vissCfg->eeCfg;
        status = Fvid2_control(tObj->handle, IOCTL_FCP_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->RgbLutConfig)
    {
        fcpCtrl.module                  = FCP_MODULE_RGB_LUT;
        fcpCtrl.rgbLut                  = vissCfg->RgbLutConfig;
        status = Fvid2_control(tObj->handle, IOCTL_FCP_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    return (status);
}

#if defined (VHWA_VPAC_IP_REV_VPAC3)
int32_t AppViss_SetFcp2Config(AppViss_TestParams *tPrms, uint32_t hidx)
{
    int32_t             status = FVID2_SOK;
    Fcp_Control         fcpCtrl;
    AppViss_Cfg        *vissCfg = NULL;
    Vhwa_M2mVissParams *vissPrms = NULL;
    AppViss_TestObject *tObj = &gAppVissTestObject[hidx];

    vissCfg = tPrms->testCfg[hidx]->vissCfg;
    vissPrms = &tPrms->testCfg[hidx]->vissPrms;

    /* newly added for default case, as it not breaking or making any error case */
    if (NULL != vissCfg->comp16To12LutCfg)
    {
        fcpCtrl.module = 15U;
        fcpCtrl.inComp = vissCfg->comp16To12LutCfg;
        status = Fvid2_control(tObj->handle, IOCTL_FCP2_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->fcp2Comp16To12LutCfg)
    {
        fcpCtrl.module = FCP_MODULE_COMPANDING;
        fcpCtrl.inComp = vissCfg->fcp2Comp16To12LutCfg;
        status = Fvid2_control(tObj->handle, IOCTL_FCP2_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->fcp2CfaCfg)
    {
        fcpCtrl.module          = FCP_MODULE_CFA;
        fcpCtrl.cfa             = vissCfg->fcp2CfaCfg;
        status = Fvid2_control(tObj->handle, IOCTL_FCP2_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->fcp2Comp24To12LutCfg)
    {
        fcpCtrl.module = FCP_MODULE_CFA_COMPANDING;
        fcpCtrl.cLutComp = vissCfg->fcp2Comp24To12LutCfg;
        status = Fvid2_control(tObj->handle, IOCTL_FCP2_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->fcp2Decomp16To24LutCfg)
    {
        fcpCtrl.module = FCP_MODULE_DECOMPANDING;
        fcpCtrl.dLutComp = vissCfg->fcp2Decomp16To24LutCfg;
        status = Fvid2_control(tObj->handle, IOCTL_FCP2_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->fcp2Ccm)
    {
        fcpCtrl.module      = FCP_MODULE_CCM;
        fcpCtrl.ccm         = vissCfg->fcp2Ccm;
        status = Fvid2_control(tObj->handle, IOCTL_FCP2_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->fcp2Gamma)
    {
        fcpCtrl.module = FCP_MODULE_GAMMA;
        fcpCtrl.gamma  = vissCfg->fcp2Gamma;
        status = Fvid2_control(tObj->handle, IOCTL_FCP2_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->fcp2Rgb2Hsv)
    {
        fcpCtrl.module          = FCP_MODULE_RGB2HSV;
        fcpCtrl.rgb2Hsv         = vissCfg->fcp2Rgb2Hsv;
        status = Fvid2_control(tObj->handle, IOCTL_FCP2_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->fcp2Rgb2yuv)
    {
        fcpCtrl.module          = FCP_MODULE_RGB2YUV;
        fcpCtrl.rgb2Yuv         = vissCfg->fcp2Rgb2yuv;
        status = Fvid2_control(tObj->handle, IOCTL_FCP2_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->fcp2YuvSatLutCfg)
    {
        fcpCtrl.module                  = FCP_MODULE_YUV_SAT_LUT;
        fcpCtrl.yuvSatLut               = vissCfg->fcp2YuvSatLutCfg;
        status = Fvid2_control(tObj->handle, IOCTL_FCP2_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->fcp2HistCfg)
    {
        fcpCtrl.module                  = FCP_MODULE_HISTOGRAM;
        fcpCtrl.hist                    = vissCfg->fcp2HistCfg;
        status = Fvid2_control(tObj->handle, IOCTL_FCP2_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }


    if (NULL != vissCfg->fcp2EeCfg)
    {
        if (0 != (VHWA_M2M_VISS_EE_ON_FCP1_LUMA12 & vissPrms->edgeEnhancerMode))
        {
            vissCfg->fcp2EeCfg->bypassY12 = UFALSE;
            vissCfg->fcp2EeCfg->bypassY8 = UTRUE;
            vissCfg->fcp2EeCfg->eeForY12OrY8 = 0u;
        }
        else if (0 != (VHWA_M2M_VISS_EE_ON_FCP1_LUMA8 & vissPrms->edgeEnhancerMode))
        {
            vissCfg->fcp2EeCfg->bypassY8 = UFALSE;
            vissCfg->fcp2EeCfg->bypassY12 = UTRUE;
            vissCfg->fcp2EeCfg->eeForY12OrY8 = 1u;
            vissCfg->fcp2EeCfg->leftShift = 2u;
            vissCfg->fcp2EeCfg->rightShift = 2u;
        }
        else
        {
            vissCfg->fcp2EeCfg->bypassY12 = UTRUE;
            vissCfg->fcp2EeCfg->bypassY8 = UTRUE;
        }

        fcpCtrl.module              = FCP_MODULE_EE;
        fcpCtrl.eeCfg               = vissCfg->fcp2EeCfg;
        status = Fvid2_control(tObj->handle, IOCTL_FCP2_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    if (NULL != vissCfg->fcp2RgbLutConfig)
    {
        fcpCtrl.module                  = FCP_MODULE_RGB_LUT;
        fcpCtrl.rgbLut                  = vissCfg->fcp2RgbLutConfig;
        status = Fvid2_control(tObj->handle, IOCTL_FCP2_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            return (status);
        }
    }

    return (status);
}
#endif

int32_t AppViss_SetGlbceConfig(AppViss_TestParams *tPrms, uint32_t hidx)
{
    int32_t             status = FVID2_SOK;
    AppViss_Cfg        *vissCfg = NULL;
    Vhwa_M2mVissParams *vissPrms = NULL;
    Glbce_Control       glbceCtrl;
    AppViss_TestObject *tObj = &gAppVissTestObject[hidx];

    vissCfg = tPrms->testCfg[hidx]->vissCfg;
    vissPrms = &tPrms->testCfg[hidx]->vissPrms;

    if (UTRUE == vissPrms->enableGlbce)
    {
        /* Added newly for default */
        if (NULL != vissCfg->glbceCfg)
        {
            glbceCtrl.module = GLBCE_MODULE_GLBCE;
            glbceCtrl.glbceCfg = vissCfg->glbceCfg;
            status = Fvid2_control(tObj->handle,
                IOCTL_GLBCE_GET_CONFIG, (void *)&glbceCtrl, NULL);
        }

        if (NULL != vissCfg->glbceCfg)
        {
            glbceCtrl.module = GLBCE_MODULE_GLBCE;
            glbceCtrl.glbceCfg = vissCfg->glbceCfg;
            status = Fvid2_control(tObj->handle, IOCTL_GLBCE_SET_CONFIG,
                (void *)&glbceCtrl, NULL);
            if (FVID2_SOK != status)
            {
                return (status);
            }
        }

        if (NULL != vissCfg->fwdPrcpCfg)
        {
            glbceCtrl.module = GLBCE_MODULE_FWD_PERCEPT;
            glbceCtrl.fwdPrcptCfg = vissCfg->fwdPrcpCfg;
            status = Fvid2_control(tObj->handle, IOCTL_GLBCE_SET_CONFIG,
                (void *)&glbceCtrl, NULL);
            if (FVID2_SOK != status)
            {
                return (status);
            }
        }

        if (NULL != vissCfg->revPrcpCfg)
        {
            glbceCtrl.module = GLBCE_MODULE_REV_PERCEPT;
            glbceCtrl.revPrcptCfg = vissCfg->revPrcpCfg;
            status = Fvid2_control(tObj->handle, IOCTL_GLBCE_SET_CONFIG,
                (void *)&glbceCtrl, NULL);
            if (FVID2_SOK != status)
            {
                return (status);
            }
        }

        if (NULL != vissCfg->GlbceWdrCfg)
        {
            glbceCtrl.module = GLBCE_MODULE_WDR;
            glbceCtrl.wdrCfg = vissCfg->GlbceWdrCfg;
            status = Fvid2_control(tObj->handle, IOCTL_GLBCE_SET_CONFIG,
                (void *)&glbceCtrl, NULL);
            if (FVID2_SOK != status)
            {
                return (status);
            }
        }

        /* Added for default case of Vhwa_m2mVissSetGlbceConfig() */
        if (NULL != vissCfg->revPrcpCfg)
        {
            glbceCtrl.module = 5;
            glbceCtrl.wdrCfg = vissCfg->GlbceWdrCfg;
            status = Fvid2_control(tObj->handle, IOCTL_GLBCE_SET_CONFIG,
                (void *)&glbceCtrl, NULL);
            if (FVID2_SOK != status)
            {
                return (status);
            }
        }

        /* Added for wrong IOCTL control command for vhwa_m2mVissApi.c */
        if (NULL != vissCfg->glbceCfg)
        {
            glbceCtrl.module = GLBCE_MODULE_GLBCE;
            glbceCtrl.wdrCfg = vissCfg->GlbceWdrCfg;
            status = Fvid2_control(tObj->handle, 1,
                (void *)&glbceCtrl, NULL);
            /* Expected status is error when control function invoking for default case */
            if (FVID2_SOK != status)
            {
                /* test passed, so changing the status as OK*/
                status = FVID2_SOK;
            }
            else
            {
                return (status);
            }
        }
    }

    return (status);
}

int32_t AppViss_SetNsf4Config(AppViss_TestParams *tPrms, uint32_t hidx)
{
    int32_t             status = FVID2_SOK;
    AppViss_Cfg        *vissCfg = NULL;
    Vhwa_M2mVissParams *vissPrms = NULL;
    AppViss_TestObject *tObj = &gAppVissTestObject[hidx];

    vissCfg = tPrms->testCfg[hidx]->vissCfg;
    vissPrms = &tPrms->testCfg[hidx]->vissPrms;

    if ((UTRUE == vissPrms->enableNsf4) && (NULL != vissCfg->nsf4Cfg))
    {
        status = Fvid2_control(tObj->handle, IOCTL_NSF4_SET_CONFIG,
            (void *)vissCfg->nsf4Cfg, NULL);
    }

    return (status);
}

#if defined (VHWA_VPAC_IP_REV_VPAC3) || defined (VHWA_VPAC_IP_REV_VPAC3L)
int32_t AppViss_SetCacConfig(AppViss_TestParams *tPrms, uint32_t hidx)
{
    int32_t             status = FVID2_SOK;
    AppViss_Cfg        *vissCfg = NULL;
    Vhwa_M2mVissParams *vissPrms = NULL;
    AppViss_TestObject *tObj = &gAppVissTestObject[hidx];

    vissCfg = tPrms->testCfg[hidx]->vissCfg;
    vissPrms = &tPrms->testCfg[hidx]->vissPrms;

    if ((UTRUE == vissPrms->enableCac) && (NULL != vissCfg->cacCfg))
    {
        status = Fvid2_control(tObj->handle, IOCTL_CAC_SET_CONFIG,
            (void *)vissCfg->cacCfg, NULL);
    }

    return (status);
}
#endif

#if defined (VHWA_VPAC_IP_REV_VPAC3L)
int32_t AppViss_SetPcidParams(AppViss_TestParams *tPrms, uint32_t hidx)
{
    int32_t             status;
    AppViss_TestObject *tObj = &gAppVissTestObject[hidx];
    AppViss_TestConfig *tCfg = NULL;
    Vhwa_M2mVissParams *vissPrms = NULL;

    vissPrms = &tPrms->testCfg[hidx]->vissPrms;

    tCfg = tPrms->testCfg[hidx];

    if (TRUE == vissPrms->enablePcid)
    {
        status = Fvid2_control(tObj->handle, IOCTL_PCID_SET_CONFIG,
                                    (void *)tCfg->pPcidCfg, NULL);
    }
    else
    {
        status = FVID2_EFAIL;
    }
    return (status);
}
#endif
int32_t AppViss_SetH3aConfig(AppViss_TestParams *tPrms, uint32_t hidx)
{
    int32_t             status = FVID2_SOK;
    Rfe_Control         rfeCtrl;
    AppViss_Cfg        *vissCfg = NULL;
    Vhwa_M2mVissParams *vissPrms = NULL;
    AppViss_TestObject *tObj = &gAppVissTestObject[hidx];

    vissCfg = tPrms->testCfg[hidx]->vissCfg;
    vissPrms = &tPrms->testCfg[hidx]->vissPrms;

    if ((uint32_t)UTRUE == vissPrms->outPrms[VHWA_M2M_VISS_OUT_H3A_IDX].enable)
    {
        if (NULL != vissCfg->rfeH3aInCfg)
        {
            /* H3A Input Selection and Lut configuration */
            rfeCtrl.module   = RFE_MODULE_H3A;
            rfeCtrl.h3aInCfg = vissCfg->rfeH3aInCfg;

            status = Fvid2_control(tObj->handle, IOCTL_RFE_SET_CONFIG,
                (void *)&rfeCtrl, NULL);
            if (FVID2_SOK != status)
            {
                return (status);
            }
        }

        if (NULL != vissCfg->h3aLutCfg)
        {
            /* H3A Input Selection and Lut configuration */
            rfeCtrl.module   = RFE_MODULE_H3A_LUT;
            rfeCtrl.h3aLutCfg = vissCfg->h3aLutCfg;

            status = Fvid2_control(tObj->handle, IOCTL_RFE_SET_CONFIG,
                (void *)&rfeCtrl, NULL);
            if (FVID2_SOK != status)
            {
                return (status);
            }
        }

        if (NULL != vissCfg->h3aCfg)
        {
            status = Fvid2_control(tObj->handle, IOCTL_H3A_SET_CONFIG,
                (void *)vissCfg->h3aCfg, NULL);
        }
    }

    return (status);
}

int32_t AppViss_AllocBuffers(AppViss_TestParams *tPrms, uint32_t hidx,
    uint64_t srcBuf, uint32_t *inFrameSize,
    uint64_t dstBuf, uint32_t *outFrameSize)
{
    uint32_t                cnt;
    uint32_t                numIns;
    uint32_t                chromaOffset = 0;
    uint32_t                frameEnd = 0;
    AppViss_Cfg            *vissCfg = NULL;
    Vhwa_M2mVissParams     *vissPrms = NULL;
    Fvid2_Format           *outFmt = NULL;
    Fvid2_Format           *inFmt = NULL;
    AppViss_TestObject     *tObj = &gAppVissTestObject[hidx];

    vissCfg = tPrms->testCfg[hidx]->vissCfg;
    vissPrms = &tPrms->testCfg[hidx]->vissPrms;

    if (VHWA_M2M_VISS_MODE_SINGLE_FRAME_INPUT == vissPrms->inputMode)
    {
        numIns = 1;
    }
    else if (VHWA_M2M_VISS_MODE_TWO_FRAME_MERGE == vissPrms->inputMode)
    {
        numIns = 2;
    }
    else
    {
        numIns = 3;
    }

    inFmt    = &vissPrms->inFmt;
    frameEnd = inFmt->pitch[0] * inFmt->height;

    #ifdef VHWA_RUN_FROM_MSMC
    frameEnd = 16*1024 + 16;
    #endif

    Vhwa_print("\n VISS Buffers \n");

    for (cnt = 0u; cnt < numIns; cnt ++)
    {
        tObj->inFrm[cnt].addr[0U] = srcBuf;

        /* Move Buffer Index */
        srcBuf += frameEnd;
        *inFrameSize += frameEnd;

        Vhwa_print (" VISS: Input%d: 0x%llx\n", cnt,
            tObj->inFrm[cnt].addr[0U]);
    }

    for (cnt = 0u; cnt < VHWA_M2M_VISS_MAX_OUTPUTS; cnt ++)
    {
        if (UFALSE == vissPrms->outPrms[cnt].enable)
        {
            continue;
        }

        outFmt = &vissPrms->outPrms[cnt].fmt;

        chromaOffset = 0u;
        switch (outFmt->dataFormat)
        {
            case FVID2_DF_YUV422I_UYVY:
            case FVID2_DF_YUV422I_YUYV:
                frameEnd = outFmt->pitch[0] * outFmt->height;
                break;
            case FVID2_DF_RGB24_888_PLANAR:
                chromaOffset = outFmt->pitch[0] * outFmt->height;
                frameEnd = chromaOffset * 3;
                break;
            case FVID2_DF_GREY:
            case FVID2_DF_SATURATION:
                switch (outFmt->ccsFormat)
                {
                    case FVID2_CCSF_BITS8_PACKED:
                    case FVID2_CCSF_BITS8_UNPACKED16:
                    case FVID2_CCSF_BITS12_PACKED:
                    case FVID2_CCSF_BITS12_UNPACKED16:
                        frameEnd = (outFmt->pitch[0] * outFmt->height);
                        break;
                    default:
                        Vhwa_print(
                            "Error FVID2_DF_GREY/SATUR : outCcsf =  0x%x \n",
                            outFmt->ccsFormat);
                        return FVID2_EFAIL;
                }
                break;

            case FVID2_DF_YUV420SP_UV:
                switch (outFmt->ccsFormat)
                {
                    case FVID2_CCSF_BITS12_PACKED:
                    case FVID2_CCSF_BITS12_UNPACKED16_MSB_ALIGNED:
                    case FVID2_CCSF_BITS12_UNPACKED16:
                    case FVID2_CCSF_BITS8_PACKED:
                    case FVID2_CCSF_BITS8_UNPACKED16:
                    case FVID2_CCSF_BITS16_PACKED:
                        chromaOffset = outFmt->pitch[0] * outFmt->height;
                        frameEnd = ((outFmt->pitch[0] * outFmt->height) * 3) / 2;
                        break;
                    default:
                        Vhwa_print(
                            "Error FVID2_DF_YUV420SP_UV : outCcsf =  0x%x \n",
                            outFmt->ccsFormat);
                        return FVID2_EFAIL;
                }
                break;
#if defined (VHWA_VPAC_IP_REV_VPAC3) || defined (VHWA_VPAC_IP_REV_VPAC3L)
            case FVID2_DF_YUV422SP_UV:
                switch (outFmt->ccsFormat)
                {
                    case FVID2_CCSF_BITS12_PACKED:
                    case FVID2_CCSF_BITS12_UNPACKED16_MSB_ALIGNED:
                    case FVID2_CCSF_BITS12_UNPACKED16:
                    case FVID2_CCSF_BITS8_PACKED:
                    case FVID2_CCSF_BITS8_UNPACKED16:
                    case FVID2_CCSF_BITS16_PACKED:
                        chromaOffset = outFmt->pitch[0] * outFmt->height;
                        frameEnd = outFmt->pitch[0] * outFmt->height * 2u;
                        break;
                    default:
                        Vhwa_print(
                            "Error FVID2_DF_YUV422SP_UV : outCcsf =  0x%x \n",
                            outFmt->ccsFormat);
                        return FVID2_EFAIL;
                }
                break;
            case FVID2_DF_RAW08:
            case FVID2_DF_RAW12:
            case FVID2_DF_RAW16:
            case FVID2_DF_LUMA_ONLY:
                switch (outFmt->ccsFormat)
                {
                    case FVID2_CCSF_BITS12_PACKED:
                    case FVID2_CCSF_BITS12_UNPACKED16_MSB_ALIGNED:
                    case FVID2_CCSF_BITS12_UNPACKED16:
                    case FVID2_CCSF_BITS8_PACKED:
                    case FVID2_CCSF_BITS8_UNPACKED16:
                    case FVID2_CCSF_BITS16_PACKED:
                        chromaOffset = 0u;
                        frameEnd = outFmt->pitch[0] * outFmt->height;
                        break;
                    default:
                        Vhwa_print(
                            "Error FVID2_DF_LUMA_ONLY : outCcsf =  0x%x \n",
                            outFmt->ccsFormat);
                        return FVID2_EFAIL;
                }
                break;
            
            case FVID2_DF_CHROMA_ONLY:
                switch (outFmt->ccsFormat)
                {
                    case FVID2_CCSF_BITS12_PACKED:
                    case FVID2_CCSF_BITS12_UNPACKED16_MSB_ALIGNED:
                    case FVID2_CCSF_BITS12_UNPACKED16:
                    case FVID2_CCSF_BITS8_PACKED:
                    case FVID2_CCSF_BITS8_UNPACKED16:
                    case FVID2_CCSF_BITS16_PACKED:
                        chromaOffset = 0u;
                        frameEnd = outFmt->pitch[0] * outFmt->height;
                        break;
                    default:
                        Vhwa_print(
                            "Error FVID2_DF_CHROMA_ONLY : outCcsf =  0x%x \n",
                            outFmt->ccsFormat);
                        return FVID2_EFAIL;
                }
                break;
            case FVID2_DF_RGI:
	        chromaOffset = 0;
                frameEnd = outFmt->pitch[0] * outFmt->height;
                break;
        case FVID2_DF_BLUE:
  	         chromaOffset = 0;
                frameEnd = outFmt->pitch[0] * outFmt->height;
                break;
        case FVID2_DF_RED:
  	         chromaOffset = 0;
                frameEnd = outFmt->pitch[0] * outFmt->height;
                break;
        case FVID2_DF_GREEN:
  	         chromaOffset = 0;
                frameEnd = outFmt->pitch[0] * outFmt->height;
                break;
#endif
            case FVID2_DF_RAW:
                if (VHWA_M2M_VISS_OUT_H3A_IDX == cnt)
                {
                    uint32_t wins;

                    wins = (vissCfg->h3aCfg->aewbCfg.winCfg.horzCount *
                        vissCfg->h3aCfg->aewbCfg.winCfg.vertCount);

                    dstBuf += (wins * 32) + ((wins/8) *16);
                    *outFrameSize += (wins * 32) + ((wins/8) *16);
                    if (wins%8 != 0)
                    {
                        dstBuf += 16;
                        *outFrameSize += 16;
                    }
                }
                break;
            default:
                Vhwa_print("Error : Unsupported format 0x%x \n",
                    outFmt->dataFormat);
                return FVID2_EFAIL;
        }

        tObj->outFrm[cnt].addr[0U] = dstBuf;
        tObj->outFrm[cnt].addr[1U] = (dstBuf + chromaOffset);
        tObj->outFrm[cnt].addr[2U] = (dstBuf + 2*chromaOffset);

        /* Move Buffer Index */
        dstBuf += frameEnd;
        *outFrameSize += frameEnd;

        Vhwa_print(" VISS: Output%d:", cnt);
        Vhwa_print(" 0x%x 0x%x 0x%x\n",
            (uint32_t)tObj->outFrm[cnt].addr[0U],
            (uint32_t)tObj->outFrm[cnt].addr[1U],
            (uint32_t)tObj->outFrm[cnt].addr[2U]);
    }

    return (FVID2_SOK);
}

#if defined VHWA_VPAC_IP_REV_VPAC || defined VHWA_VPAC_IP_REV_VPAC3
int32_t AppViss_CrcInit(Udma_DrvHandle udmaDrvHndl)
{
    int32_t status = FVID2_SOK;

    gVissCrcChHandle.crcChannel = APP_VISS_CRC_CHANNEL;
    gVissCrcChHandle.udmaCrcChHandle = App_crcCreate(udmaDrvHndl,
                                                 &gVissCrcChHandle.crcChannel);
    if(gVissCrcChHandle.udmaCrcChHandle == NULL)
    {
        Vhwa_print(" App_crcCreate Error\n");
        status = FVID2_EFAIL;
    }

    return status;
}

int32_t AppViss_CrcDeinit(Udma_DrvHandle udmaDrvHndl)
{
    int32_t status = FVID2_SOK;

    App_crcDelete(udmaDrvHndl, &gVissCrcChHandle);

    return status;
}

static int32_t AppViss_CompareCrc(AppViss_TestParams *tPrms, uint32_t hIdx)
{
    int32_t             status = FVID2_SOK;

    return (status);

}
#endif
void AppViss_PrepareRequest(AppViss_TestParams *tPrms, uint32_t hidx)
{
    uint32_t            cnt;
    Fvid2_FrameList    *inFrmList;
    Fvid2_FrameList    *outFrmList;
    AppViss_TestObject *tObj = &gAppVissTestObject[hidx];

    inFrmList = &tObj->inFrmList;
    outFrmList = &tObj->outFrmList;

    for (cnt = 0u; cnt < VHWA_M2M_VISS_MAX_INPUTS; cnt ++)
    {
        /* Initialize VISS Input Frame List */
        inFrmList->frames[cnt] = &tObj->inFrm[cnt];
    }
    inFrmList->numFrames = VHWA_M2M_VISS_MAX_INPUTS;

    /* Initialize VISS Output Frame List */
    outFrmList->numFrames = VHWA_M2M_VISS_MAX_OUTPUTS;
    for (cnt = 0u; cnt < VHWA_M2M_VISS_MAX_OUTPUTS; cnt++)
    {
        outFrmList->frames[cnt] = &tObj->outFrm[cnt];
    }
}

int32_t AppViss_SubmitRequest(AppViss_TestParams *tPrms, uint32_t hidx)
{
    int32_t             status;
    Fvid2_FrameList    *inFrmList;
    Fvid2_FrameList    *outFrmList;
    AppViss_TestObject *tObj = &gAppVissTestObject[hidx];

    inFrmList = &tObj->inFrmList;
    outFrmList = &tObj->outFrmList;

    /* Submit VISS Request*/
    status = Fvid2_processRequest(tObj->handle, inFrmList,
        outFrmList, FVID2_TIMEOUT_FOREVER);
    if (FVID2_SOK != status)
    {
        Vhwa_print
            (" VISS: Fvid2_processRequest returned %x for VISS Handle # %d\n",
            status, hidx);
    }

    return (status);
}

int32_t AppViss_WaitForCompRequest(AppViss_TestParams *tPrms, uint32_t hidx, uint32_t repCnt)
{
    int32_t             status;
    Fvid2_FrameList     *inFrmList;
    Fvid2_FrameList     *outFrmList;
    AppViss_TestObject  *tObj = &gAppVissTestObject[hidx];
    Vhwa_M2mVissPsaSign psa;
    uint16_t            outCnt;

    VhwaAl_SemaphoreP_pend(tObj->waitForProcessCmpl, SYSTEMP_WAIT_FOREVER);

    inFrmList = &tObj->inFrmList;
    outFrmList = &tObj->outFrmList;

    if((0u == tObj->wdTimerErrStatus) && (0u == tObj->err_stat))
    {
        status = Fvid2_getProcessedRequest(tObj->handle,
            inFrmList, outFrmList, 0);
        if (FVID2_SOK != status)
        {
            Vhwa_print (" VISS: Failed VISS Handle Cnt %d; status = %x\n",
                hidx, status);
            return (status);
        }
        else
        {
            #if defined VHWA_VPAC_IP_REV_VPAC || defined VHWA_VPAC_IP_REV_VPAC3
            status = AppViss_CompareCrc(tPrms, hidx);
            if (FVID2_SOK != status)
            {
                Vhwa_print (" VISS: CRC Check Failed Handle Cnt %d\n", hidx);
            }
            #endif
            if((FVID2_SOK == status) && (UTRUE == tPrms->testCfg[hidx]->isLsePsaAvail))
            {
                status = Fvid2_control(tObj->handle,
                        VHWA_M2M_IOCTL_VISS_GET_PSA_SIGN,
                        &psa, NULL);
                if (FVID2_SOK == status)
                {
                    for (outCnt = 0u; outCnt < VHWA_M2M_VISS_MAX_OUTPUTS; outCnt ++)
                    {
                        if ((0 != tPrms->testCfg[hidx]->psaSign[outCnt]) &&
                            (tPrms->testCfg[hidx]->psaSign[outCnt] !=
                                    psa.psaSign[outCnt]) && (0 != repCnt))
                        {
                            Vhwa_print (" VISS frame %d PSA mismatch!\n", repCnt);
                            status = FVID2_EFAIL;
                            break;
                        }
                    }
                }
            }
        }
    }
    else
    {
        if(0u != tObj->wdTimerErrStatus)
        {
            Vhwa_print("IP stall: Watchdog timer error interrupt triggered \n");
            status = FVID2_ETIMEOUT;
        }
        if(0u != tObj->err_stat)
        {
            Vhwa_print("Error interrupt: VISS error interrupt triggered \n");
            status = FVID2_EFAIL;
        }
    }

    return (status);
}

void AppViss_SyncStart(AppViss_TestParams *tObj, uint32_t hidx)
{
    uint32_t status;
    AppViss_TestObject  *appObj = &gAppVissTestObject[hidx];

    status = Fvid2_control(appObj->handle,
                            IOCTL_VHWA_M2M_VISS_SYNC_START,
                            NULL, NULL);
    if(FVID2_SOK != status)
    {
        Vhwa_print(" VISS Sync Start Failed \n");
    }
}

int32_t AppViss_GetConfigBufInfo(uint32_t hidx,
        Vhwa_M2mVissConfigAppBuff *appBuffConfig)
{
    int32_t status = FVID2_SOK;
    AppViss_TestObject *appObj = &gAppVissTestObject[hidx];

    /* Set to default values */
    appBuffConfig->bufferPtr = NULL;
    appBuffConfig->length = 0;
    appBuffConfig->configThroughUdmaFlag = false;
    appBuffConfig->isCacheWriteBackPolicy = true;

    status = Fvid2_control(appObj->handle,
                IOCTL_VHWA_M2M_VISS_GET_BUFF_INFO, (void*) appBuffConfig, NULL);
    if (FVID2_SOK != status)
    {
        Vhwa_print(" AppViss_GetConfigBufInfo Failed \n");
    }

    return (status);
}

int32_t AppViss_SetConfigBufInfo(uint32_t hidx,
        Vhwa_M2mVissConfigAppBuff *appBuffConfig)
{
    int32_t status = FVID2_SOK;
    AppViss_TestObject *appObj = &gAppVissTestObject[hidx];

    status = Fvid2_control(appObj->handle,
                IOCTL_VHWA_M2M_VISS_SET_BUFF_INFO, (void*) appBuffConfig, NULL);
    if (FVID2_SOK != status)
    {
        Vhwa_print(" AppViss_SetConfigBufInfo Failed \n");
    }

    return (status);
}

#if defined (VHWA_VPAC_IP_REV_VPAC3) || defined (VHWA_VPAC_IP_REV_VPAC3L)
int32_t AppViss_getNsf4RawHistData(uint32_t hidx, void *rawHistData)
{
    int32_t status = FVID2_SOK;
    AppViss_TestObject *appObj = &gAppVissTestObject[hidx];

    status = Fvid2_control(appObj->handle,
                IOCTL_NSF4_GET_HISTOGRAM, rawHistData, NULL);
    if (FVID2_SOK != status)
    {
        Vhwa_print(" AppViss_getNsf4RawHistData Failed \n");
    }

    return (status);
}
#endif

int32_t AppViss_getFcp1HistData(uint32_t hidx, void *histData)
{
    int32_t status = FVID2_SOK;
    AppViss_TestObject *appObj = &gAppVissTestObject[hidx];
    Fcp_HistData *fcpHistData = NULL;
    fcpHistData = (Fcp_HistData *)malloc(sizeof(Fcp_HistData));
    if (NULL == fcpHistData)
    {
        Vhwa_print(" Memory allocation for Fcp_HistData failed \n");
        return FVID2_EALLOC;
    }
    memset(fcpHistData, 0, sizeof(Fcp_HistData));
    fcpHistData->hist = (uint32_t *)histData;
    fcpHistData->fcpIdx = 0;
    status = Fvid2_control(appObj->handle,
                IOCTL_FCP_GET_HISTOGRAM, (void *)fcpHistData, NULL);
    if (FVID2_SOK != status)
    {
        Vhwa_print(" AppViss_getFcp1HistData Failed \n");
    }

    return (status);
}

int32_t AppViss_getFcp2HistData(uint32_t hidx, void *histData)
{
    int32_t status = FVID2_SOK;
    AppViss_TestObject *appObj = &gAppVissTestObject[hidx];
    Fcp_HistData *fcpHistData = NULL;
    fcpHistData = (Fcp_HistData *)malloc(sizeof(Fcp_HistData));
    if (NULL == fcpHistData)
    {
        Vhwa_print(" Memory allocation for Fcp_HistData failed \n");
        return FVID2_EALLOC;
    }
    memset(fcpHistData, 0, sizeof(Fcp_HistData));
    fcpHistData->hist = (uint32_t *)histData;
    fcpHistData->fcpIdx = 1;
    status = Fvid2_control(appObj->handle,
                IOCTL_FCP_GET_HISTOGRAM, (void *)fcpHistData, NULL);
    if (FVID2_SOK != status)
    {
        Vhwa_print(" AppViss_getFcp2HistData Failed \n");
    }

    return (status);
}


int32_t AppViss_SetHtsLimitCfg(uint32_t hidx,
    AppViss_HtsLimiterCfg *appHtsLimitConfig)
{
int32_t status              = FVID2_SOK;
AppViss_TestObject *appObj  = &gAppVissTestObject[hidx];
Vhwa_HtsLimiter   *htsLimit = NULL;

/* Populate htsLimit from appHtsLimitConfig */
htsLimit = (Vhwa_HtsLimiter *)appHtsLimitConfig;
htsLimit->enableBwLimit = appHtsLimitConfig->enableBwLimit;
htsLimit->cycleCnt      = appHtsLimitConfig->cycleCnt;
htsLimit->tokenCnt      = appHtsLimitConfig->tokenCnt;

status = Fvid2_control(appObj->handle,
            IOCTL_VHWA_M2M_VISS_SET_HTS_LIMIT, (void*) htsLimit, NULL);
if (FVID2_SOK != status)
{
    Vhwa_print(" AppViss_SetHtsLimit Config Failed \n");
}

return (status);
}
