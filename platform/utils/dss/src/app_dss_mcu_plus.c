/*
 *
 * Copyright (c) 2024-2026 Texas Instruments Incorporated
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

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <utils/dss/include/app_dss.h>
#include <utils/console_io/include/app_log.h>
#include <dss.h>
#include <board/board_control.h>
#include <ti_drivers_config.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

extern Dss_Object gDssObjects[CONFIG_DSS_NUM_INSTANCES];
extern Dss_DctrlOverlayLayerParams gDssOverlayLayerParams[CONFIG_DSS_NUM_INSTANCES];
extern Dss_DctrlOverlayParams gDssOverlayParams[CONFIG_DSS_NUM_INSTANCES];
extern Dss_DctrlVpParams gDssVpParams[CONFIG_DSS_NUM_INSTANCES];
extern Dss_ConfigPipelineParams gDssConfigPipelineParams[CONFIG_DSS_NUM_INSTANCES];
extern Dss_DctrlAdvVpParams gDssAdvVpParams[CONFIG_DSS_NUM_INSTANCES];
extern uint32_t gDssDisplayInterface[CONFIG_DSS_NUM_INSTANCES];
extern I2C_Handle gI2cHandle[CONFIG_I2C_NUM_INSTANCES];
extern Dss_DctrlDsiParams gDsiParams;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                  Internal/Private Function Declarations                    */
/* ========================================================================== */

static void DispApp_enableDisableRequiredInstance(uint32_t dss0Enable, uint32_t dss1Enable, Dss_RmInfo *rmInfo)
{
    /* Update availability of common region, video pipe, video port and overlay based on the instance Enablement */
    uint32_t i = 0U;
    uint32_t assignVal = dss0Enable;
    rmInfo->dss0Enabled = assignVal;
    /* For DSS0 */
    for(i=CSL_DSS_COMM_REG_ID_0; i<CSL_DSS_COMM_REG_ID_2; i++)
    {
        rmInfo->isCommRegAvailable[i] = assignVal;
    }
    for(i=CSL_DSS_VID_PIPE_ID_VID1; i<CSL_DSS_VID_PIPE_ID_VID2; i++)
    {
        rmInfo->isPipeAvailable[i] = assignVal;
    }
    for(i=CSL_DSS_OVERLAY_ID_1; i<CSL_DSS_OVERLAY_ID_3; i++)
    {
        rmInfo->isOverlayAvailable[i] = assignVal;
    }
    for(i=CSL_DSS_VP_ID_1; i<CSL_DSS_VP_ID_3; i++)
    {
        rmInfo->isPortAvailable[i] = assignVal;
    }

    assignVal = dss1Enable;
    rmInfo->dss1Enabled = assignVal;
    /* For DSS1 */
    for(i=CSL_DSS_COMM_REG_ID_2; i<CSL_DSS_COMM_REG_ID_MAX; i++)
    {
        rmInfo->isCommRegAvailable[i] = assignVal;
    }
    for(i=CSL_DSS_VID_PIPE_ID_VID2; i<CSL_DSS_VID_PIPE_ID_MAX; i++)
    {
        rmInfo->isPipeAvailable[i] = assignVal;
    }
    for(i=CSL_DSS_OVERLAY_ID_3; i<CSL_DSS_OVERLAY_ID_MAX; i++)
    {
        rmInfo->isOverlayAvailable[i] = assignVal;
    }
    for(i=CSL_DSS_VP_ID_3; i<CSL_DSS_VP_ID_MAX; i++)
    {
        rmInfo->isPortAvailable[i] = assignVal;
    }
}


static void DispApp_commonInit(Dss_Object *appObj)
{
    Dss_initParamsInit(&appObj->initParams);

#if defined (CONFIG_DSS0) && defined (CONFIG_DSS1)
    /* Enabling both instances of DSS in driver */
    DispApp_enableDisableRequiredInstance(TRUE, TRUE, &(appObj->initParams.socParams.rmInfo));
#elif defined (CONFIG_DSS1) &&  !defined (CONFIG_DSS0)
    /* Enabling DSS1 and disabling DSS0 which is enabled by default */
    DispApp_enableDisableRequiredInstance(FALSE, TRUE, &(appObj->initParams.socParams.rmInfo));
#endif

    Dss_init(&appObj->initParams);
}

static int32_t DispApp_init()
{
    int32_t         retVal = FVID2_SOK;
    Dss_Object      *appObj;

    DispApp_commonInit(&gDssObjects[0]);

    for (uint32_t dssInstanceNum = 0U; dssInstanceNum < CONFIG_DSS_NUM_INSTANCES; dssInstanceNum++)
    {
        appObj = &gDssObjects[dssInstanceNum];

        if(FVID2_SOK == retVal)
        {
            /* Create DCTRL handle, used for common driver configuration */
            appObj->dctrlHandle = Fvid2_create(
                DSS_DCTRL_DRV_ID,
                DSS_DCTRL_INST_0,
                NULL,
                NULL,
                NULL);
            if(NULL == appObj->dctrlHandle)
            {
                appLogPrintf("DCTRL Create Failed!!!\r\n");
            }
        }

        if(appObj->oldiParams != NULL)
        {
            Dss_setOLDITxPowerDown(appObj->oldiParams->oldiCfg.oldiMapType, TRUE);
        }

        if(FVID2_SOK == retVal)
        {
            appLogPrintf("DispApp_init() - DONE !!!\r\n");
        }
    }

    return retVal;
}

static int32_t DispApp_pipeCbFxn(Fvid2_Handle handle, void *appData)
{
    int32_t retVal  = FVID2_SOK;
    Dss_InstObject *instObj = (Dss_InstObject *) appData;

    GT_assert (DssTrace, (NULL != instObj));
    (void) SemaphoreP_post(&instObj->syncSem);

    return (retVal);
}


static int32_t DispApp_deInit()
{
    int32_t  retVal = FVID2_SOK;
    Dss_Object *appObj;

    /* Delete DCTRL handle */
    for (uint32_t dssInstanceNum = 0U; dssInstanceNum < CONFIG_DSS_NUM_INSTANCES; dssInstanceNum++)
    {
        appObj = &gDssObjects[dssInstanceNum];
        retVal = Fvid2_delete(appObj->dctrlHandle, NULL);
    }

    retVal += Dss_deInit();
    retVal += Fvid2_deInit(NULL);
    if(retVal != FVID2_SOK)
    {
         appLogPrintf("DCTRL handle delete failed!!!\r\n");
    }
    else
    {
         appLogPrintf("DispApp_deInit() - DONE !!!\r\n");
    }

    return retVal;
}

static int32_t DispApp_setupHDMI( void )
{
    BOARD_HdmiCfg_t hdmiCfg;

    hdmiCfg.resolution = BOARD_CTRL_HDMI_RES_1080P;
    hdmiCfg.i2cInstance = HDMI_AND_DSI_BRIDGE_I2C_CONFIG;
    hdmiCfg.i2cHandle = gI2cHandle[HDMI_AND_DSI_BRIDGE_I2C_CONFIG];
    Board_control(BOARD_CTRL_CMD_CFG_HDMI, &hdmiCfg);

    return 0;
}

static int32_t DispApp_setupDSI2DPBridge(void)
{
    BOARD_DSI2DPBridgeCfg_t dsiBridgeCfg;

    dsiBridgeCfg.resolution = BOARD_CTRL_DSI_BRIDGE_1080P;
    dsiBridgeCfg.i2cInstance = HDMI_AND_DSI_BRIDGE_I2C_CONFIG;
    dsiBridgeCfg.i2cHandle = gI2cHandle[HDMI_AND_DSI_BRIDGE_I2C_CONFIG];
    Board_control(BOARD_CTRL_CMD_CFG_DSI2DP_BRIDGE, &dsiBridgeCfg);

    return 0;
}

static int32_t DispApp_configDctrl(Dss_Object *appObj, uint32_t dssInstanceNum)
{
    int32_t retVal = FVID2_SOK;

    Dss_DctrlVpParams *vpParams;
    Dss_DctrlOverlayParams *overlayParams;
    Dss_DctrlOverlayLayerParams *layerParams;
    Dss_DctrlAdvVpParams *advVpParams;
    Dss_DctrlGlobalDssParams *globalDssParams;
    Dss_DctrlOldiParams *oldiParams;
    static uint32_t executeOnce = 0U;

    oldiParams = appObj->oldiParams;
    vpParams = &appObj->vpParams;
    overlayParams = &appObj->overlayParams;
    layerParams = &appObj->layerParams;
    advVpParams = &appObj->advVpParams;
    globalDssParams= &appObj->globalDssParams;

    if (0U == executeOnce)
    {
        retVal = Fvid2_control(
            appObj->dctrlHandle,
            IOCTL_DSS_DCTRL_SET_PATH,
            appObj->dctrlPathInfo,
            NULL);
    }
    if(retVal != FVID2_SOK)
    {
        appLogPrintf("Dctrl Set Path IOCTL Failed!!!\r\n");
    }

    retVal = Fvid2_control(
        appObj->dctrlHandle,
        IOCTL_DSS_DCTRL_SET_ADV_VP_PARAMS,
        advVpParams,
        NULL);
    if(retVal != FVID2_SOK)
    {
        appLogPrintf("DCTRL Set Advance VP Params IOCTL Failed!!!\r\n");
    }

    if (DSS_OLDI_INTERFACE == gDssDisplayInterface[dssInstanceNum])
    {
        retVal = Fvid2_control(appObj->dctrlHandle,
                                IOCTL_DSS_DCTRL_SET_OLDI_PARAMS,
                                oldiParams,
                                NULL);
        if(retVal != FVID2_SOK)
        {
            appLogPrintf("DCTRL Set OLDI Params IOCTL Failed!!!\r\n");
        }
    }
    else if (DSS_HDMI_INTERFACE == gDssDisplayInterface[dssInstanceNum])
    {
        retVal = DispApp_setupHDMI();
        if(retVal != FVID2_SOK)
        {
            appLogPrintf("HDMI Setup Has Failed!!!\r\n");
        }
    }
    else if (DSS_DSI_INTERFACE == gDssDisplayInterface[dssInstanceNum])
    {
        if(FVID2_SOK == retVal)
        {
            /* Configuring the DSI to DP bridge */
            DispApp_setupDSI2DPBridge();
            retVal = Fvid2_control(
                appObj->dctrlHandle,
                IOCTL_DSS_DCTRL_SET_DSI_PARAMS,
                &gDsiParams,
                NULL);
            if(retVal != FVID2_SOK)
            {
                appLogPrintf("DSI Setup Has Failed!!!\r\n");
            }
        }
    }

    retVal = Fvid2_control(
        appObj->dctrlHandle,
        IOCTL_DSS_DCTRL_SET_VP_PARAMS,
        vpParams,
        NULL);
    if(retVal != FVID2_SOK)
    {
        appLogPrintf("Dctrl Set VP Params IOCTL Failed!!!\r\n");
    }

    retVal = Fvid2_control(
        appObj->dctrlHandle,
        IOCTL_DSS_DCTRL_SET_OVERLAY_PARAMS,
        overlayParams,
        NULL);
    if(retVal != FVID2_SOK)
    {
        appLogPrintf("DCTRL Set Overlay Params IOCTL Failed!!!\r\n");
    }

    retVal = Fvid2_control(
        appObj->dctrlHandle,
        IOCTL_DSS_DCTRL_SET_LAYER_PARAMS,
        layerParams,
        NULL);
    if(retVal != FVID2_SOK)
    {
        appLogPrintf("DCTRL Set Layer Params IOCTL Failed!!!\r\n");
    }

    if (0U == executeOnce)
    {
        retVal = Fvid2_control(
            appObj->dctrlHandle,
            IOCTL_DSS_DCTRL_SET_GLOBAL_DSS_PARAMS,
            globalDssParams,
            NULL);
        executeOnce++;
    }
    if(retVal != FVID2_SOK)
    {
        appLogPrintf("DCTRL Set Global DSS Params IOCTL Failed!!!\r\n");
    }

    return (retVal);
}

static void DispApp_initDssParams(Dss_Object *appObj, uint32_t dssInstanceNum)
{
    Dss_DctrlVpParams *vpParams;
    Dss_DctrlAdvVpParams *advVpParams;
    Dss_DctrlOverlayParams *overlayParams;
    Dss_DctrlOverlayLayerParams *layerParams;
    Dss_DctrlGlobalDssParams *globalDssParams;

    vpParams = &appObj->vpParams;
    overlayParams = &appObj->overlayParams;
    layerParams = &appObj->layerParams;
    advVpParams = &appObj->advVpParams;
    globalDssParams= &appObj->globalDssParams;

    Dss_dctrlVpParamsInit(vpParams);
    Dss_dctrlAdvVpParamsInit(advVpParams);
    Dss_dctrlOverlayParamsInit(overlayParams);
    Dss_dctrlOverlayLayerParamsInit(layerParams);
    Dss_dctrlGlobalDssParamsInit(globalDssParams);

    /* Configure VP params */
    vpParams->vpId = gDssVpParams[dssInstanceNum].vpId;
    vpParams->lcdOpTimingCfg.mInfo.standard = gDssVpParams[dssInstanceNum].lcdOpTimingCfg.mInfo.standard;
    vpParams->lcdOpTimingCfg.mInfo.width = gDssVpParams[dssInstanceNum].lcdOpTimingCfg.mInfo.width;
    vpParams->lcdOpTimingCfg.mInfo.height = gDssVpParams[dssInstanceNum].lcdOpTimingCfg.mInfo.height;
    vpParams->lcdOpTimingCfg.mInfo.hFrontPorch = gDssVpParams[dssInstanceNum].lcdOpTimingCfg.mInfo.hFrontPorch;
    vpParams->lcdOpTimingCfg.mInfo.hBackPorch = gDssVpParams[dssInstanceNum].lcdOpTimingCfg.mInfo.hBackPorch;
    vpParams->lcdOpTimingCfg.mInfo.hSyncLen = gDssVpParams[dssInstanceNum].lcdOpTimingCfg.mInfo.hSyncLen;
    vpParams->lcdOpTimingCfg.mInfo.vFrontPorch = gDssVpParams[dssInstanceNum].lcdOpTimingCfg.mInfo.vFrontPorch;
    vpParams->lcdOpTimingCfg.mInfo.vBackPorch = gDssVpParams[dssInstanceNum].lcdOpTimingCfg.mInfo.vBackPorch;
    vpParams->lcdOpTimingCfg.mInfo.vSyncLen = gDssVpParams[dssInstanceNum].lcdOpTimingCfg.mInfo.vSyncLen;
    vpParams->lcdOpTimingCfg.mInfo.pixelClock = gDssVpParams[dssInstanceNum].lcdOpTimingCfg.mInfo.pixelClock;

    vpParams->lcdOpTimingCfg.dvoFormat = gDssVpParams[dssInstanceNum].lcdOpTimingCfg.dvoFormat;
    vpParams->lcdOpTimingCfg.videoIfWidth = gDssVpParams[dssInstanceNum].lcdOpTimingCfg.videoIfWidth;

    vpParams->lcdPolarityCfg.actVidPolarity =  gDssVpParams[dssInstanceNum].lcdPolarityCfg.actVidPolarity;
    vpParams->lcdPolarityCfg.hsPolarity = gDssVpParams[dssInstanceNum].lcdPolarityCfg.hsPolarity;
    vpParams->lcdPolarityCfg.vsPolarity = gDssVpParams[dssInstanceNum].lcdPolarityCfg.vsPolarity;
    vpParams->lcdPolarityCfg.pixelClkPolarity = gDssVpParams[dssInstanceNum].lcdPolarityCfg.pixelClkPolarity ;

    if (TRUE == gDssVpParams[dssInstanceNum].gammaCfg.gammaEnable)
    {
        vpParams->gammaCfg.gammaEnable = gDssVpParams[dssInstanceNum].gammaCfg.gammaEnable;
        for (uint32_t i = 0; i < CSL_DSS_NUM_LUT_ENTRIES; i++)
        {
            /* Writing R(23:16), G(15:8),B(7:0) components with white color, output : white screen */
            vpParams->gammaCfg.gammaData[i] = 0xFFFFFF;
        }
    }

    /* Configure VP Advance Params*/
    advVpParams->vpId = gDssAdvVpParams[dssInstanceNum].vpId;
    advVpParams->lcdAdvSignalCfg.hVAlign = gDssAdvVpParams[dssInstanceNum].lcdAdvSignalCfg.hVAlign;
    advVpParams->lcdAdvSignalCfg.hVClkControl = gDssAdvVpParams[dssInstanceNum].lcdAdvSignalCfg.hVClkControl;

    /* Configure Overlay Params */
    overlayParams->overlayId =  gDssOverlayParams[dssInstanceNum].overlayId;
    overlayParams->colorbarEnable =  gDssOverlayParams[dssInstanceNum].colorbarEnable;
    overlayParams->overlayCfg.colorKeyEnable =  gDssOverlayParams[dssInstanceNum].overlayCfg.colorKeyEnable;
    overlayParams->overlayCfg.colorKeySel =  gDssOverlayParams[dssInstanceNum].overlayCfg.colorKeySel;

    /* Set the Overlay Params Config */
    overlayParams->overlayCfg.colorKeyEnable =  1; /* Setting it to TRUE */
    overlayParams->overlayCfg.colorKeySel =  1; /* Setting it to APP_DCTRL_OVERLAY_TRANS_COLOR_SRC */

    /* Note: explicitly setting background color to black */
    overlayParams->overlayCfg.backGroundColor =  0x0;

    /* Configure Overlay Layer params */
    layerParams->overlayId = gDssOverlayLayerParams[dssInstanceNum].overlayId;
    memcpy((void*)layerParams->pipeLayerNum, (void* )gDssOverlayLayerParams[dssInstanceNum].pipeLayerNum, \
    sizeof(gDssOverlayLayerParams[dssInstanceNum].pipeLayerNum));
}

static int32_t DispApp_create(Dss_Object *appObj, uint32_t dssInstanceNum)
{
    int32_t retVal = FVID2_SOK;
    int32_t status = SystemP_SUCCESS;
    uint32_t instCnt = 0U;
    Dss_InstObject *instObj;

    /* Init VP, Overlay and Panel params */
    DispApp_initDssParams(appObj, dssInstanceNum);

    /* Config IOCTL for VP, Overlay and Panel */
    retVal = DispApp_configDctrl(appObj, dssInstanceNum);

    if(FVID2_SOK == retVal)
    {
        appLogPrintf("Display create complete!!\r\n");
    }

    return retVal;
}

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t appDssInit(app_dss_init_params_t *dssParams)
{
    int32_t retVal = 0; uint32_t i;
    Dss_Object *appObj;

    retVal = DispApp_init();

    if(0 != retVal)
    {
        appLogPrintf("DSS: ERROR: Dss_deInit failed !!!\n");
    }
    else
    {
        /* Create driver */
        for(uint32_t dssInstanceNum = 0U; dssInstanceNum < CONFIG_DSS_NUM_INSTANCES; dssInstanceNum++)
        {
            appObj = &gDssObjects[dssInstanceNum];
            /* Create driver */
            retVal = DispApp_create(appObj, dssInstanceNum);
        }
    }

    return retVal;
}

int32_t appDssDeInit(void)
{
    int32_t retVal = 0;

    retVal = DispApp_deInit();
    if(0 != retVal)
    {
        appLogPrintf("DSS: ERROR: Dss_deInit failed !!!\n");
    }

    return retVal;
}
