/*
 *
 * Copyright (c) 2018-2026 Texas Instruments Incorporated
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
#include <utils/dss/include/app_dctrl.h>
#include <utils/console_io/include/app_log.h>
#include <utils/timer/include/app_timer.h>
#include <utils/remote_service/include/app_remote_service.h>
#include <dss/dss.h>

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

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static Fvid2_Handle gAppDctrlHandle;

/* ========================================================================== */
/*                  Internal/Private Function Declarations                    */
/* ========================================================================== */

static int32_t appDctrlControl(char *serviceName,
                               uint32_t cmd,
                               void *params,
                               uint32_t size,
                               uint32_t flags);
static int32_t appDctrlRegisterHandleCmd(Fvid2_Handle *handle,
                                         const bool *doHpd,
                                         uint32_t params_size);
static int32_t appDctrlDeleteHandleCmd(Fvid2_Handle handle);
static int32_t appDctrlSetPathCmd(Fvid2_Handle handle,
                                  const app_dctrl_path_info_t *pathInfo,
                                  uint32_t params_size);
static int32_t appDctrlClearPathCmd(Fvid2_Handle handle,
                                    const app_dctrl_path_info_t *pathInfo,
                                    uint32_t params_size);
static int32_t appDctrlSetVpParamsCmd(Fvid2_Handle handle,
                                      const app_dctrl_vp_params_t *vpParams,
                                      uint32_t params_size);
static int32_t appDctrlSetAdvVpParamsCmd(Fvid2_Handle handle,
                                      const app_dctrl_adv_vp_params_t *vpParams,
                                      uint32_t params_size);
static int32_t appDctrlSetOverlayParamsCmd(Fvid2_Handle handle,
                                           const app_dctrl_overlay_params_t *overlayParams,
                                           uint32_t params_size);
static int32_t appDctrlSetLayerParamsCmd(Fvid2_Handle handle,
                                         const app_dctrl_layer_params_t *layerParams,
                                         uint32_t params_size);
static int32_t appDctrlStopVpCmd(Fvid2_Handle handle,
                                 const app_dctrl_vp_params_t *vpParams,
                                 uint32_t params_size);
static int32_t appDctrlSetDsiParamsCmd(Fvid2_Handle handle,
                                 const app_dctrl_dsi_params_t *vpParams,
                                 uint32_t params_size);
static int32_t appDctrlIsDpConnectedParamsCmd(Fvid2_Handle handle,
                                         bool *isDpConnected,
                                         uint32_t params_size);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t appDctrlInit(void)
{
    int32_t retVal;

    retVal = appRemoteServiceRegister(APP_DCTRL_REMOTE_SERVICE_NAME, appDctrlControl);
    if(0 != retVal)
    {
        appLogPrintf("DCTRL: ERROR: Unable to register DCTRL handler \n");
    }

    return retVal;
}

int32_t appDctrlDeInit(void)
{
    int32_t retVal;

    retVal = appRemoteServiceUnRegister(APP_DCTRL_REMOTE_SERVICE_NAME);
    if(0 != retVal)
    {
        appLogPrintf("DCTRL: ERROR: Unable to unregister DCTRL handler \n");
    }

    return retVal;
}

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

static int32_t appDctrlControl(char *serviceName,
                               uint32_t cmd,
                               void *params,
                               uint32_t size,
                               uint32_t flags)
{
    int32_t retVal = 0;

    /* Check for NULL pointers */
    if(APP_DCTRL_CMD_DELETE_HANDLE != cmd)
    {
        if((NULL == params) || (0U == size))
        {
            appLogPrintf("DCTRL: ERROR: appDctrlControl failed for command %d!!!\n", cmd);
            retVal = -1;
        }
    }

    /* Call the driver */
    if(0 == retVal)
    {
        switch (cmd)
        {
            case APP_DCTRL_CMD_REGISTER_HANDLE:
                retVal = appDctrlRegisterHandleCmd(&gAppDctrlHandle,
                                                   (const bool *) params,
                                                   size);
                break;
            case APP_DCTRL_CMD_DELETE_HANDLE:
                retVal = appDctrlDeleteHandleCmd(gAppDctrlHandle);
                break;
            case APP_DCTRL_CMD_SET_PATH:
                retVal = appDctrlSetPathCmd(gAppDctrlHandle,
                                            (const app_dctrl_path_info_t*) params,
                                            size);
                break;
            case APP_DCTRL_CMD_CLEAR_PATH:
                retVal = appDctrlClearPathCmd(gAppDctrlHandle,
                                              (const app_dctrl_path_info_t*) params,
                                              size);
                break;
            case APP_DCTRL_CMD_SET_VP_PARAMS:
                retVal = appDctrlSetVpParamsCmd(gAppDctrlHandle,
                                                (const app_dctrl_vp_params_t*) params,
                                                size);
                break;
            case APP_DCTRL_CMD_SET_ADV_VP_PARAMS:
                retVal = appDctrlSetAdvVpParamsCmd(gAppDctrlHandle,
                                                (const app_dctrl_adv_vp_params_t*) params,
                                                size);
                break;
            case APP_DCTRL_CMD_SET_OVERLAY_PARAMS:
                retVal = appDctrlSetOverlayParamsCmd(gAppDctrlHandle,
                                                     (const app_dctrl_overlay_params_t*) params,
                                                     size);
                break;
            case APP_DCTRL_CMD_SET_LAYER_PARAMS:
                retVal = appDctrlSetLayerParamsCmd(gAppDctrlHandle,
                                                   (const app_dctrl_layer_params_t*) params,
                                                   size);
                break;
            case APP_DCTRL_CMD_STOP_VP:
                retVal = appDctrlStopVpCmd(gAppDctrlHandle,
                                           (const app_dctrl_vp_params_t*) params,
                                           size);
                break;
            case APP_DCTRL_CMD_SET_DSI_PARAMS:
                retVal = appDctrlSetDsiParamsCmd(gAppDctrlHandle,
                    (const app_dctrl_dsi_params_t *)params, size);
                break;
            case APP_DCTRL_CMD_IS_DP_CONNECTED:
                retVal = appDctrlIsDpConnectedParamsCmd(gAppDctrlHandle,
                    (bool *)params, size);
                break;
            default:
                appLogPrintf("DCTRL: ERROR: Unsupported command for appDctrlControl!!!\n");
                retVal = -1;
                break;
        }
    }
    return retVal;
}

static void appDctrlDpHpdCbFxn(uint32_t hpdState, void *appData)
{
    Fvid2_control(gAppDctrlHandle, IOCTL_DSS_DCTRL_PROCESS_DP_HPD, &hpdState, NULL);
}

static int32_t appDctrlRegisterHandleCmd(Fvid2_Handle *handle,
                                         const bool *doHpd,
                                         uint32_t params_size)
{
    int32_t retVal = 0;
    Dss_DctrlDpHpdCbParams cbParams;

    if((params_size != sizeof(bool)) || (NULL == handle))
    {
        retVal = -1;
    }

    if(0 == retVal)
    {
        *handle = Fvid2_create(DSS_DCTRL_DRV_ID, DSS_DCTRL_INST_0, NULL, NULL, NULL);
        if(NULL == *handle)
        {
            retVal = -1;
        }
    }

    /*
     * If HPD is supported (if DP is enabled), the HW generates an
     * interrupt on hot-plug (new monitor connected / powered on or
     * DP HW reset)
     *
     * The driver handles the interrupt and calls hpdCbFxn if a
     * DP_HPD_CB is registered.
     *
     * Note: The HPD is generated in Dss_init() when we still dont have
     * a DCTRL handle and a registered callback. The driver keeps
     * a state, so that the link is initialised immediately after
     * the callback is registered
     */
    if(0 == retVal && (true == *doHpd))
    {
        Dss_dctrlDpHpdCbParamsInit(&cbParams);

        cbParams.hpdCbFxn = appDctrlDpHpdCbFxn;
        cbParams.appData = *handle;
        retVal = Fvid2_control(*handle, IOCTL_DSS_DCTRL_REGISTER_DP_HPD_CB, &cbParams, NULL);
    }

    return retVal;
}

static int32_t appDctrlDeleteHandleCmd(Fvid2_Handle handle)
{
    int32_t retVal = 0;

    retVal = Fvid2_delete(handle, NULL);
    if(0 != retVal)
    {
        retVal = -1;
    }

    return retVal;
}
static int32_t appDctrlSetPathCmd(Fvid2_Handle handle,
                                  const app_dctrl_path_info_t *pathInfo,
                                  uint32_t params_size)
{
    int32_t retVal = 0;
    uint32_t i;
    Dss_DctrlPathInfo drvPathInfo;

    if(params_size != sizeof(app_dctrl_path_info_t))
    {
        retVal = -1;
    }

    if(0 == retVal)
    {
        Dss_dctrlPathInfoInit(&drvPathInfo);
        drvPathInfo.numEdges = pathInfo->numEdges;
        for(i=0U; i<pathInfo->numEdges; i++)
        {
            drvPathInfo.edgeInfo[i].startNode = pathInfo->edgeInfo[i].startNode;
            drvPathInfo.edgeInfo[i].endNode = pathInfo->edgeInfo[i].endNode;
        }
        retVal = Fvid2_control(handle, IOCTL_DSS_DCTRL_SET_PATH, &drvPathInfo, NULL);
    }

    return retVal;
}
static int32_t appDctrlClearPathCmd(Fvid2_Handle handle,
                                    const app_dctrl_path_info_t *pathInfo,
                                    uint32_t params_size)
{
    int32_t retVal = 0;
    uint32_t i;
    Dss_DctrlPathInfo drvPathInfo;

    if(params_size != sizeof(app_dctrl_path_info_t))
    {
        retVal = -1;
    }

    if(0 == retVal)
    {
        Dss_dctrlPathInfoInit(&drvPathInfo);
        drvPathInfo.numEdges = pathInfo->numEdges;
        for(i=0U; i<pathInfo->numEdges; i++)
        {
            drvPathInfo.edgeInfo[i].startNode = pathInfo->edgeInfo[i].startNode;
            drvPathInfo.edgeInfo[i].endNode = pathInfo->edgeInfo[i].endNode;
        }
        retVal = Fvid2_control(handle, IOCTL_DSS_DCTRL_CLEAR_PATH, &drvPathInfo, NULL);
    }

    return retVal;
}
static int32_t appDctrlSetAdvVpParamsCmd(Fvid2_Handle handle,
                                      const app_dctrl_adv_vp_params_t *advVpParams,
                                      uint32_t params_size)
{
    int32_t retVal = 0;
    Dss_DctrlAdvVpParams drvAdvVpParams;

    if(params_size != sizeof(app_dctrl_adv_vp_params_t))
    {
        retVal = -1;
    }

    if(0 == retVal)
    {
        Dss_dctrlAdvVpParamsInit(&drvAdvVpParams);
        drvAdvVpParams.vpId = advVpParams->vpId;
        drvAdvVpParams.lcdAdvSignalCfg.hVAlign = advVpParams->hVAlign;
        drvAdvVpParams.lcdAdvSignalCfg.hVClkControl = advVpParams->hVClkControl;
        drvAdvVpParams.lcdAdvSignalCfg.hVClkRiseFall = advVpParams->hVClkRiseFall;
        drvAdvVpParams.lcdAdvSignalCfg.acBI = advVpParams->acBI;
        drvAdvVpParams.lcdAdvSignalCfg.acB = advVpParams->acB;
        drvAdvVpParams.lcdAdvSignalCfg.vSyncGated = advVpParams->vSyncGated;
        drvAdvVpParams.lcdAdvSignalCfg.hSyncGated = advVpParams->hSyncGated;
        drvAdvVpParams.lcdAdvSignalCfg.pixelClockGated = advVpParams->pixelClockGated;
        drvAdvVpParams.lcdAdvSignalCfg.pixelDataGated = advVpParams->pixelDataGated;
        drvAdvVpParams.lcdAdvSignalCfg.pixelGated = advVpParams->pixelGated;
        retVal = Fvid2_control(handle, IOCTL_DSS_DCTRL_SET_ADV_VP_PARAMS, &drvAdvVpParams, NULL);
    }

    return retVal;
}
static int32_t appDctrlSetVpParamsCmd(Fvid2_Handle handle,
                                      const app_dctrl_vp_params_t *vpParams,
                                      uint32_t params_size)
{
    int32_t retVal = 0;
    Dss_DctrlVpParams drvVpParams;

    if(params_size != sizeof(app_dctrl_vp_params_t))
    {
        retVal = -1;
    }

    if(0 == retVal)
    {
        Dss_dctrlVpParamsInit(&drvVpParams);
        drvVpParams.vpId = vpParams->vpId;
        drvVpParams.lcdOpTimingCfg.mInfo.standard = vpParams->standard;
        drvVpParams.lcdOpTimingCfg.mInfo.width = vpParams->width;
        drvVpParams.lcdOpTimingCfg.mInfo.height = vpParams->height;
        drvVpParams.lcdOpTimingCfg.mInfo.scanFormat = vpParams->scanFormat;
        drvVpParams.lcdOpTimingCfg.mInfo.hFrontPorch = vpParams->hFrontPorch;
        drvVpParams.lcdOpTimingCfg.mInfo.hBackPorch = vpParams->hBackPorch;
        drvVpParams.lcdOpTimingCfg.mInfo.hSyncLen = vpParams->hSyncLen;
        drvVpParams.lcdOpTimingCfg.mInfo.vFrontPorch = vpParams->vFrontPorch;
        drvVpParams.lcdOpTimingCfg.mInfo.vBackPorch = vpParams->vBackPorch;
        drvVpParams.lcdOpTimingCfg.mInfo.vSyncLen = vpParams->vSyncLen;
        drvVpParams.lcdOpTimingCfg.mInfo.pixelClock = vpParams->pixelClock;
        drvVpParams.lcdOpTimingCfg.dvoFormat = vpParams->dvoFormat;
        drvVpParams.lcdOpTimingCfg.cscRange = vpParams->cscRange;
        drvVpParams.lcdOpTimingCfg.videoIfWidth = vpParams->videoIfWidth;
        drvVpParams.lcdPolarityCfg.actVidPolarity = vpParams->actVidPolarity;
        drvVpParams.lcdPolarityCfg.pixelClkPolarity = vpParams->pixelClkPolarity;
        drvVpParams.lcdPolarityCfg.hsPolarity = vpParams->hsPolarity;
        drvVpParams.lcdPolarityCfg.vsPolarity = vpParams->vsPolarity;
        retVal = Fvid2_control(handle, IOCTL_DSS_DCTRL_SET_VP_PARAMS, &drvVpParams, NULL);
    }

    return retVal;
}
static int32_t appDctrlSetOverlayParamsCmd(Fvid2_Handle handle,
                                           const app_dctrl_overlay_params_t *overlayParams,
                                           uint32_t params_size)
{
    int32_t retVal = 0;
    Dss_DctrlOverlayParams drvOverlayParams;

    if(params_size != sizeof(app_dctrl_overlay_params_t))
    {
        retVal = -1;
    }

    if(0 == retVal)
    {
        Dss_dctrlOverlayParamsInit(&drvOverlayParams);
        drvOverlayParams.overlayId = overlayParams->overlayId;
        drvOverlayParams.overlayCfg.colorKeyEnable = overlayParams->colorKeyEnable;
        drvOverlayParams.overlayCfg.colorKeySel = overlayParams->colorKeySel;
        drvOverlayParams.overlayCfg.transColorKeyMin = overlayParams->transColorKeyMin;
        drvOverlayParams.overlayCfg.transColorKeyMax = overlayParams->transColorKeyMax;
        drvOverlayParams.overlayCfg.backGroundColor = overlayParams->backGroundColor;
        retVal = Fvid2_control(handle, IOCTL_DSS_DCTRL_SET_OVERLAY_PARAMS, &drvOverlayParams, NULL);
    }

    return retVal;
}
static int32_t appDctrlSetLayerParamsCmd(Fvid2_Handle handle,
                                         const app_dctrl_layer_params_t *layerParams,
                                         uint32_t params_size)
{
    int32_t retVal = 0;
    uint32_t i;
    Dss_DctrlOverlayLayerParams drvLayerParams;

    if(params_size != sizeof(app_dctrl_layer_params_t))
    {
        retVal = -1;
    }

    if(0 == retVal)
    {
        Dss_dctrlOverlayLayerParamsInit(&drvLayerParams);
        drvLayerParams.overlayId = layerParams->overlayId;
        for(i=0U; i<CSL_DSS_VID_PIPE_ID_MAX; i++)
        {
            drvLayerParams.pipeLayerNum[i] = layerParams->pipeLayerNum[i];
        }
        retVal = Fvid2_control(handle, IOCTL_DSS_DCTRL_SET_LAYER_PARAMS, &drvLayerParams, NULL);
    }

    return retVal;
}
static int32_t appDctrlStopVpCmd(Fvid2_Handle handle,
                                 const app_dctrl_vp_params_t *vpParams,
                                 uint32_t params_size)
{
    int32_t retVal = 0;
    Dss_DctrlVpParams drvVpParams;

    if(params_size != sizeof(app_dctrl_vp_params_t))
    {
        retVal = -1;
    }

    if(0 == retVal)
    {
        Dss_dctrlVpParamsInit(&drvVpParams);
        drvVpParams.vpId = vpParams->vpId;
        drvVpParams.lcdOpTimingCfg.mInfo.standard = vpParams->standard;
        drvVpParams.lcdOpTimingCfg.mInfo.width = vpParams->width;
        drvVpParams.lcdOpTimingCfg.mInfo.height = vpParams->height;
        drvVpParams.lcdOpTimingCfg.mInfo.scanFormat = vpParams->scanFormat;
        drvVpParams.lcdOpTimingCfg.mInfo.hFrontPorch = vpParams->hFrontPorch;
        drvVpParams.lcdOpTimingCfg.mInfo.hBackPorch = vpParams->hBackPorch;
        drvVpParams.lcdOpTimingCfg.mInfo.hSyncLen = vpParams->hSyncLen;
        drvVpParams.lcdOpTimingCfg.mInfo.vFrontPorch = vpParams->vFrontPorch;
        drvVpParams.lcdOpTimingCfg.mInfo.vBackPorch = vpParams->vBackPorch;
        drvVpParams.lcdOpTimingCfg.mInfo.vSyncLen = vpParams->vSyncLen;
        drvVpParams.lcdOpTimingCfg.dvoFormat = vpParams->dvoFormat;
        drvVpParams.lcdOpTimingCfg.cscRange = vpParams->cscRange;
        drvVpParams.lcdOpTimingCfg.videoIfWidth = vpParams->videoIfWidth;
        drvVpParams.lcdPolarityCfg.actVidPolarity = vpParams->actVidPolarity;
        drvVpParams.lcdPolarityCfg.pixelClkPolarity = vpParams->pixelClkPolarity;
        drvVpParams.lcdPolarityCfg.hsPolarity = vpParams->hsPolarity;
        drvVpParams.lcdPolarityCfg.vsPolarity = vpParams->vsPolarity;
        retVal = Fvid2_control(handle, IOCTL_DSS_DCTRL_STOP_VP, &drvVpParams, NULL);
    }

    return retVal;
}

static int32_t appDctrlSetDsiParamsCmd(Fvid2_Handle handle,
    const app_dctrl_dsi_params_t *prms, uint32_t params_size)
{
    int32_t retVal = 0;
    Dss_DctrlDsiParams dsi_params;

    if(params_size != sizeof(app_dctrl_dsi_params_t))
    {
        retVal = -1;
    }

    if(0 == retVal)
    {
        dsi_params.numOfLanes = prms->num_lanes;
        dsi_params.laneSpeedInKbps = prms->lane_speed_in_kbps;

        retVal = Fvid2_control(handle, IOCTL_DSS_DCTRL_SET_DSI_PARAMS,
            &dsi_params, NULL);
    }

    return (retVal);
}

static int32_t appDctrlIsDpConnectedParamsCmd(Fvid2_Handle handle,
                                         bool *isDpConnected,
                                         uint32_t params_size)
{
    int32_t retVal = 0;
    int32_t dpConnectedCmdArg;

    if((params_size != sizeof(bool)) || (NULL == handle))
    {
        retVal = -1;
    }

    if(0 == retVal)
    {
        retVal = Fvid2_control(handle, IOCTL_DSS_DCTRL_IS_DP_CONNECTED,
            &dpConnectedCmdArg, NULL);

        *isDpConnected = dpConnectedCmdArg;
    }

    return (retVal);
}
