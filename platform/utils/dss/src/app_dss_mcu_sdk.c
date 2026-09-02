/*
 *
 * Copyright (c) 2026 Texas Instruments Incorporated
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
#ifndef SOC_TDA54
#include <board/board_control.h>
#endif
#include <utils/dss/include/dss_config_tda54.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

extern Dss_Object gDssObjects[CONFIG_DSS_NUM_INSTANCES];
extern Dss_DctrlOverlayLayerParams gDssOverlayLayerParams;
extern Dss_DctrlOverlayParams gDssOverlayParams;
extern Dss_DctrlVpParams gDssVpParams;
extern Dss_ConfigPipelineParams gDssConfigPipelineParams;
extern Dss_DctrlAdvVpParams gDssAdvVpParams;
extern uint32_t gDssDisplayInterface;
#ifndef SOC_TDA54
extern I2C_Handle gI2cHandle[CONFIG_I2C_NUM_INSTANCES];
#endif

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
static int32_t DispApp_init(Dss_Object *appObj)
{
    int32_t         retVal = FVID2_SOK;

    Dss_initParamsInit(&appObj->initParams);

    Dss_init(&appObj->initParams);

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

    #ifndef SOC_TDA54
    if(appObj->oldiParams != NULL)
    {
        Dss_setOLDITxPowerDown(appObj->oldiParams->oldiCfg.oldiMapType, TRUE);
    }
    #endif

    if(FVID2_SOK == retVal)
    {
        appLogPrintf("DispApp_init() - DONE !!!\r\n");
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

static int32_t DispApp_deInit(Dss_Object *appObj)
{
    int32_t  retVal = FVID2_SOK;

    /* Delete DCTRL handle */
    retVal = Fvid2_delete(appObj->dctrlHandle, NULL);
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

#ifndef SOC_TDA54
static int32_t DispApp_setupHDMI( void )
{
    BOARD_HdmiCfg_t hdmiCfg;

    hdmiCfg.resolution = BOARD_CTRL_HDMI_RES_1080P;
    hdmiCfg.i2cInstance = HDMI_I2C_CONFIG;
    hdmiCfg.i2cHandle = gI2cHandle[HDMI_I2C_CONFIG];
    Board_control(BOARD_CTRL_CMD_CFG_HDMI, &hdmiCfg);

    return 0;
}
#endif

static int32_t DispApp_configDctrl(Dss_Object *appObj)
{
    int32_t retVal = FVID2_SOK;

    Dss_DctrlVpParams *vpParams;
    Dss_DctrlOverlayParams *overlayParams;
    Dss_DctrlOverlayLayerParams *layerParams;
    Dss_DctrlAdvVpParams *advVpParams;
    Dss_DctrlGlobalDssParams *globalDssParams;
    #ifndef SOC_TDA54
    Dss_DctrlOldiParams *oldiParams;

    oldiParams = appObj->oldiParams;
    #endif
    vpParams = &appObj->vpParams;
    overlayParams = &appObj->overlayParams;
    layerParams = &appObj->layerParams;
    advVpParams = &appObj->advVpParams;
    globalDssParams= &appObj->globalDssParams;

    retVal = Fvid2_control(
        appObj->dctrlHandle,
        IOCTL_DSS_DCTRL_SET_PATH,
        appObj->dctrlPathInfo,
        NULL);
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

    retVal = Fvid2_control(
        appObj->dctrlHandle,
        IOCTL_DSS_DCTRL_SET_VP_PARAMS,
        vpParams,
        NULL);
    if(retVal != FVID2_SOK)
    {
        appLogPrintf("Dctrl Set VP Params IOCTL Failed!!!\r\n");
    }

    #ifndef SOC_TDA54
    if (DSS_OLDI_INTERFACE == gDssDisplayInterface)
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
    else if (DSS_HDMI_INTERFACE == gDssDisplayInterface)
    {
        retVal = DispApp_setupHDMI();
        if(retVal != FVID2_SOK)
        {
            appLogPrintf("HDMI Setup Has Failed!!!\r\n");
        }
    }
    #endif

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

    retVal = Fvid2_control(
        appObj->dctrlHandle,
        IOCTL_DSS_DCTRL_SET_GLOBAL_DSS_PARAMS,
        globalDssParams,
        NULL);
    if(retVal != FVID2_SOK)
    {
        appLogPrintf("DCTRL Set Global DSS Params IOCTL Failed!!!\r\n");
    }

    return (retVal);
}

static void DispApp_initDssParams(Dss_Object *appObj)
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
    vpParams->vpId = gDssVpParams.vpId;
    vpParams->lcdOpTimingCfg.mInfo.standard = gDssVpParams.lcdOpTimingCfg.mInfo.standard;
    vpParams->lcdOpTimingCfg.mInfo.width = gDssVpParams.lcdOpTimingCfg.mInfo.width;
    vpParams->lcdOpTimingCfg.mInfo.height = gDssVpParams.lcdOpTimingCfg.mInfo.height;
    vpParams->lcdOpTimingCfg.mInfo.hFrontPorch = gDssVpParams.lcdOpTimingCfg.mInfo.hFrontPorch;
    vpParams->lcdOpTimingCfg.mInfo.hBackPorch = gDssVpParams.lcdOpTimingCfg.mInfo.hBackPorch;
    vpParams->lcdOpTimingCfg.mInfo.hSyncLen = gDssVpParams.lcdOpTimingCfg.mInfo.hSyncLen;
    vpParams->lcdOpTimingCfg.mInfo.vFrontPorch = gDssVpParams.lcdOpTimingCfg.mInfo.vFrontPorch;
    vpParams->lcdOpTimingCfg.mInfo.vBackPorch = gDssVpParams.lcdOpTimingCfg.mInfo.vBackPorch;
    vpParams->lcdOpTimingCfg.mInfo.vSyncLen = gDssVpParams.lcdOpTimingCfg.mInfo.vSyncLen;

    vpParams->lcdOpTimingCfg.dvoFormat = gDssVpParams.lcdOpTimingCfg.dvoFormat;
    vpParams->lcdOpTimingCfg.videoIfWidth = gDssVpParams.lcdOpTimingCfg.videoIfWidth;

    vpParams->lcdPolarityCfg.actVidPolarity =  gDssVpParams.lcdPolarityCfg.actVidPolarity;
    vpParams->lcdPolarityCfg.hsPolarity = gDssVpParams.lcdPolarityCfg.hsPolarity;
    vpParams->lcdPolarityCfg.vsPolarity = gDssVpParams.lcdPolarityCfg.vsPolarity;
    vpParams->lcdPolarityCfg.pixelClkPolarity = gDssVpParams.lcdPolarityCfg.pixelClkPolarity ;

    /* Configure VP Advance Params*/
    advVpParams->vpId = gDssAdvVpParams.vpId;
    advVpParams->lcdAdvSignalCfg.hVAlign = gDssAdvVpParams.lcdAdvSignalCfg.hVAlign;
    advVpParams->lcdAdvSignalCfg.hVClkControl = gDssAdvVpParams.lcdAdvSignalCfg.hVClkControl;

    /* Configure Overlay Params */
    overlayParams->overlayId =  gDssOverlayParams.overlayId;
    overlayParams->colorbarEnable =  gDssOverlayParams.colorbarEnable;
    overlayParams->overlayCfg.colorKeyEnable =  gDssOverlayParams.overlayCfg.colorKeyEnable;
    overlayParams->overlayCfg.colorKeySel =  gDssOverlayParams.overlayCfg.colorKeySel;

    /* Set the Overlay Params Config */
    overlayParams->overlayCfg.colorKeyEnable =  1; /* Setting it to TRUE */
    overlayParams->overlayCfg.colorKeySel =  1; /* Setting it to APP_DCTRL_OVERLAY_TRANS_COLOR_SRC */

    /* Note: explicitly setting background color to black */
    overlayParams->overlayCfg.backGroundColor =  0x0;

    /* Configure Overlay Layer params */
    layerParams->overlayId = gDssOverlayLayerParams.overlayId;
    memcpy((void*)layerParams->pipeLayerNum, (void* )gDssOverlayLayerParams.pipeLayerNum, \
    sizeof(gDssOverlayLayerParams.pipeLayerNum));

}

static int32_t DispApp_create(Dss_Object *appObj)
{
    int32_t retVal = FVID2_SOK;
    int32_t status = SystemP_SUCCESS;
    uint32_t instCnt = 0U;
    Dss_InstObject *instObj;

    /* Init VP, Overlay and Panel params */
    DispApp_initDssParams(appObj);

    /* Config IOCTL for VP, Overlay and Panel */
    retVal = DispApp_configDctrl(appObj);

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

    retVal = DispApp_init(&gDssObjects[CONFIG_DSS0]);

    if(0 != retVal)
    {
        appLogPrintf("DSS: ERROR: Dss_deInit failed !!!\n");
    }
    else
    {
        /* Create driver */
        retVal = DispApp_create(&gDssObjects[CONFIG_DSS0]);
    }

    return retVal;
}

int32_t appDssDeInit(void)
{
    int32_t retVal = 0;

    retVal = DispApp_deInit(&gDssObjects[CONFIG_DSS0]);
    if(0 != retVal)
    {
        appLogPrintf("DSS: ERROR: Dss_deInit failed !!!\n");
    }

    return (retVal);
}
