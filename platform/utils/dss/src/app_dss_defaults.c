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

#include "app_dss_defaults_priv.h"

app_dss_default_obj_t g_app_dss_default_obj;


void appDssDefaultSetDefaultPrm(app_dss_default_prm_t *prm)
{
    prm->display_type = APP_DSS_DEFAULT_DISPLAY_TYPE_DPI_HDMI;

    prm->timings.width        = 1920U;
    prm->timings.height       = 1080U;
    prm->timings.hFrontPorch  = 88U;
    prm->timings.hBackPorch   = 148U;
    prm->timings.hSyncLen     = 44U;
    prm->timings.vFrontPorch  = 4U;
    prm->timings.vBackPorch   = 36U;
    prm->timings.vSyncLen     = 5U;
    prm->timings.pixelClock   = 148500000ULL;

    prm->enableM2m            = false;
}

int32_t appDssDefaultInit(app_dss_default_prm_t *prm)
{
    int32_t retVal = 0;
    app_dss_init_params_t dssParams;
    app_dss_default_obj_t *obj = &g_app_dss_default_obj;

    appLogPrintf("DSS: Init ... !!!\n");

    memcpy(&obj->initPrm, prm, sizeof(*prm));

    if(prm->display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_EDP)
    {
        appLogPrintf("DSS: Display type is eDP !!!\n");
        obj->nodeOverlayId = APP_DCTRL_NODE_OVERLAY1;
        obj->nodeVpId      = APP_DCTRL_NODE_VP1;
        obj->nodeDpiId     = APP_DCTRL_NODE_EDP_DPI0;
        obj->overlayId     = APP_DSS_OVERLAY_ID_1;
        obj->vpId          = APP_DSS_VP_ID_1;
        obj->videoIfWidth  = APP_DCTRL_VIFW_36BIT;
    }
    else if(prm->display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_DPI_HDMI)
    {
        appLogPrintf("DSS: Display type is HDMI !!!\n");
        obj->nodeOverlayId = APP_DCTRL_NODE_OVERLAY2;
        obj->nodeVpId      = APP_DCTRL_NODE_VP2;
        obj->nodeDpiId     = APP_DCTRL_NODE_DPI_DPI0;
        obj->overlayId     = APP_DSS_OVERLAY_ID_2;
        obj->vpId          = APP_DSS_VP_ID_2;
        obj->videoIfWidth  = APP_DCTRL_VIFW_24BIT;
    }
    else if(prm->display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_DSI)
    {
        appLogPrintf("DSS: Display type is DSI !!!\n");
        obj->nodeOverlayId = APP_DCTRL_NODE_OVERLAY3;
        obj->nodeVpId      = APP_DCTRL_NODE_VP3;
        obj->nodeDpiId     = APP_DCTRL_NODE_DSI_DPI2;
        obj->overlayId     = APP_DSS_OVERLAY_ID_3;
        obj->vpId          = APP_DSS_VP_ID_3;
        obj->videoIfWidth  = APP_DCTRL_VIFW_24BIT;
    }

    if(prm->enableM2m == UTRUE)
    {
        appLogPrintf("DSS: M2M Path is enabled !!!\n");
        obj->m2m.enableM2m     = true;
        obj->m2m.nodeOverlayId = APP_DCTRL_NODE_OVERLAY4;
        obj->m2m.overlayId     = APP_DSS_OVERLAY_ID_4;
        obj->m2m.pipeId        = APP_DCTRL_NODE_VIDL2;
        obj->m2m.vpId          = APP_DSS_VP_ID_4;
        obj->m2m.nodeVpId      = APP_DCTRL_NODE_VP4;
    }

    appDssConfigurePm(prm);

    if( (prm->display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_DPI_HDMI) ||
        (prm->display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_EDP)||
        (prm->display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_DSI))
    {
        appDssConfigureBoard(prm);
    }

    if (prm->display_type == APP_DSS_DEFAULT_DISPLAY_TYPE_DSI)
    {
#if defined (SOC_J721E)        
        appDssConfigureUB941AndUB925(prm);
#endif
    }

    appDssInitParamsInit(&dssParams);
    dssParams.isPipeAvailable[APP_DSS_VID_PIPE_ID_VID1] = true;
    dssParams.isPipeAvailable[APP_DSS_VID_PIPE_ID_VID2] = true;
    dssParams.isPipeAvailable[APP_DSS_VID_PIPE_ID_VIDL1] = true;

    if(prm->enableM2m == UTRUE)
    {
        dssParams.isPipeAvailable[APP_DSS_VID_PIPE_ID_VIDL2] = true;
    }
    else
    {
        dssParams.isPipeAvailable[APP_DSS_VID_PIPE_ID_VIDL2] = false;
    }
    dssParams.isOverlayAvailable[obj->overlayId] = true;
    dssParams.isPortAvailable[obj->vpId] = true;

    if(prm->enableM2m == UTRUE)
    {
        dssParams.isOverlayAvailable[obj->m2m.overlayId] = true;
        dssParams.isPortAvailable[obj->m2m.vpId] = true;
    }

    if(prm->display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_EDP)
    {
        dssParams.isDpAvailable = true;
    }
    if(prm->display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_DSI)
    {
        dssParams.isDsiAvailable = true;
    }

    retVal = appDssInit(&dssParams);
    if(retVal!=FVID2_SOK)
    {
        appLogPrintf("DSS: ERROR: Dss init failed !!!\n");
    }

    if (FVID2_SOK == retVal)
    {
        retVal = appDctrlInit();
        if(retVal!=FVID2_SOK)
        {
            appLogPrintf("DSS: ERROR: Dctrl init failed !!!\n");
        }
    }
    if (FVID2_SOK == retVal)
    {
        retVal = appDctrlDefaultInit(obj);
        if(retVal!=FVID2_SOK)
        {
            appLogPrintf("DSS: ERROR: Dctrl default init failed !!!\n");
        }
    }

    appLogPrintf("DSS: Init ... Done !!!\n");
    return retVal;
}

int32_t appDssDefaultDeInit(void)
{
    int32_t retVal = 0;

    retVal = appDctrlDefaultDeInit();
    if(retVal!=FVID2_SOK)
    {
        appLogPrintf("DSS: ERROR: Dctrl default deInit failed !!!\n");
    }
    retVal = appDctrlDeInit();
    if(retVal!=FVID2_SOK)
    {
        appLogPrintf("DSS: ERROR: Dctrl deInit failed !!!\n");
    }
    retVal = appDssDeInit();
    if(retVal!=FVID2_SOK)
    {
        appLogPrintf("DSS: ERROR: Dss deInit failed !!!\n");
    }
    return retVal;
}

int32_t appDctrlDefaultInit(app_dss_default_obj_t *obj)
{
    int32_t retVal = 0;
    uint32_t cpuId = APP_IPC_CPU_MCU2_0;
    bool doAdvVpSetup = false;
    bool doHpd = false;
    bool isDpConnected = false;
    app_dctrl_path_info_t pathInfo;
    app_dctrl_vp_params_t vpParams;
    app_dctrl_adv_vp_params_t advVpParams;
    app_dctrl_overlay_params_t overlayParams;
    app_dctrl_layer_params_t layerParams;
    app_dctrl_dsi_params_t dsiParams;

    appDctrlPathInfoInit(&pathInfo);
    appDctrlVpParamsInit(&vpParams);
    appDctrlOverlayParamsInit(&overlayParams);
    appDctrlLayerParamsInit(&layerParams);
    if(obj->initPrm.display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_EDP)
    {
        appDctrlAdvVpParamsInit(&advVpParams);
        doAdvVpSetup = true;
        doHpd = true;
    }
    if(obj->initPrm.display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_DSI)
    {
        appDctrlAdvVpParamsInit(&advVpParams);
        advVpParams.vpId = obj->vpId;
        doAdvVpSetup = true;
    }

    pathInfo.edgeInfo[pathInfo.numEdges].startNode = APP_DCTRL_NODE_VID1;
    pathInfo.edgeInfo[pathInfo.numEdges].endNode   = obj->nodeOverlayId;
    pathInfo.numEdges++;
    pathInfo.edgeInfo[pathInfo.numEdges].startNode = APP_DCTRL_NODE_VID2;
    pathInfo.edgeInfo[pathInfo.numEdges].endNode   = obj->nodeOverlayId;
    pathInfo.numEdges++;
    pathInfo.edgeInfo[pathInfo.numEdges].startNode = APP_DCTRL_NODE_VIDL1;
    pathInfo.edgeInfo[pathInfo.numEdges].endNode   = obj->nodeOverlayId;
    pathInfo.numEdges++;
    pathInfo.edgeInfo[pathInfo.numEdges].startNode = obj->nodeOverlayId;
    pathInfo.edgeInfo[pathInfo.numEdges].endNode   = obj->nodeVpId;
    pathInfo.numEdges++;
    pathInfo.edgeInfo[pathInfo.numEdges].startNode = obj->nodeVpId;
    pathInfo.edgeInfo[pathInfo.numEdges].endNode   = obj->nodeDpiId;
    pathInfo.numEdges++;

    if (true == obj->m2m.enableM2m)
    {
        pathInfo.edgeInfo[pathInfo.numEdges].startNode = obj->m2m.pipeId;
        pathInfo.edgeInfo[pathInfo.numEdges].endNode   = obj->m2m.nodeOverlayId;
        pathInfo.numEdges++;

        pathInfo.edgeInfo[pathInfo.numEdges].startNode = obj->m2m.nodeOverlayId;
        pathInfo.edgeInfo[pathInfo.numEdges].endNode   = obj->m2m.nodeVpId;
        pathInfo.numEdges++;
    }

    vpParams.vpId             = obj->vpId;

    /* Always expect the app to provide a custom resolution */
    vpParams.standard     = APP_DCTRL_VID_STD_CUSTOM;

    vpParams.width        = obj->initPrm.timings.width;
    vpParams.height       = obj->initPrm.timings.height;
    vpParams.hFrontPorch  = obj->initPrm.timings.hFrontPorch;
    vpParams.hBackPorch   = obj->initPrm.timings.hBackPorch;
    vpParams.hSyncLen     = obj->initPrm.timings.hSyncLen;
    vpParams.vFrontPorch  = obj->initPrm.timings.vFrontPorch;
    vpParams.vBackPorch   = obj->initPrm.timings.vBackPorch;
    vpParams.vSyncLen     = obj->initPrm.timings.vSyncLen;
    if(obj->initPrm.display_type != APP_DSS_DEFAULT_DISPLAY_TYPE_DSI)
    {
        vpParams.pixelClock   = (uint32_t)(obj->initPrm.timings.pixelClock / 1000ULL);
    }
    else
    {
        vpParams.pixelClock   = (uint32_t)(obj->initPrm.timings.pixelClock);
    }

    vpParams.videoIfWidth     = obj->videoIfWidth;
    if(obj->initPrm.display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_EDP)
    {
        advVpParams.hVAlign = APP_DCTRL_HVSYNC_ALIGN_ON;
        advVpParams.hVClkControl = APP_DCTRL_HVCLK_CTRL_ON;
        advVpParams.hVClkRiseFall = APP_DCTRL_EDGE_POL_RISING;

        vpParams.actVidPolarity = APP_DCTRL_POL_HIGH;
        vpParams.pixelClkPolarity = APP_DCTRL_EDGE_POL_RISING;
        vpParams.hsPolarity = APP_DCTRL_POL_HIGH;
        vpParams.vsPolarity = APP_DCTRL_POL_HIGH;
    }
    else if(obj->initPrm.display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_DSI)
    {
        advVpParams.hVAlign = APP_DCTRL_HVSYNC_ALIGN_ON;

        vpParams.actVidPolarity = APP_DCTRL_POL_HIGH;
        vpParams.pixelClkPolarity = APP_DCTRL_EDGE_POL_RISING;
        vpParams.hsPolarity = APP_DCTRL_POL_LOW;
        vpParams.vsPolarity = APP_DCTRL_POL_LOW;
    }
    else
    {
        vpParams.pixelClkPolarity = APP_DCTRL_EDGE_POL_FALLING;
    }

    overlayParams.overlayId = obj->overlayId;
    overlayParams.colorKeyEnable = 1;
    overlayParams.colorKeySel = APP_DCTRL_OVERLAY_TRANS_COLOR_SRC;
    overlayParams.transColorKeyMin = 0x0u;
    overlayParams.transColorKeyMax = 0x0u;
    overlayParams.backGroundColor = 0x0u;

    layerParams.overlayId = obj->overlayId;
    layerParams.pipeLayerNum[APP_DSS_VID_PIPE_ID_VID1] = APP_DCTRL_OVERLAY_LAYER_NUM_0;
    layerParams.pipeLayerNum[APP_DSS_VID_PIPE_ID_VID2] = APP_DCTRL_OVERLAY_LAYER_NUM_1;
    /* VIDL1 this is graphics overlay layer and it MUST be top most layer, i.e layer num 4 in j721e */
    layerParams.pipeLayerNum[APP_DSS_VID_PIPE_ID_VIDL1] = APP_DCTRL_OVERLAY_LAYER_NUM_4;
    /* this is used by Linux on A72 so should be kept as disabled or invalid here */
    layerParams.pipeLayerNum[APP_DSS_VID_PIPE_ID_VIDL2] = APP_DCTRL_OVERLAY_LAYER_INVALID;

    retVal = appRemoteServiceRun(cpuId, APP_DCTRL_REMOTE_SERVICE_NAME, APP_DCTRL_CMD_REGISTER_HANDLE, &doHpd, sizeof(doHpd), 0U);

    #if defined(SOC_J721S2)
    if( (FVID2_SOK == retVal) && (obj->initPrm.display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_EDP) )
    {
        retVal = appRemoteServiceRun(cpuId, APP_DCTRL_REMOTE_SERVICE_NAME, APP_DCTRL_CMD_IS_DP_CONNECTED, &isDpConnected, sizeof(isDpConnected), 0U);
    }
    #elif defined(SOC_J721E) || defined(SOC_J784S4) || defined(SOC_J742S2)
    /* The DP initialization should occur on J721E/J784S4/J742S2 regardless of if it is connected or not */
    isDpConnected = true;
    #endif

    if (FVID2_SOK == retVal)
    {
        if ((obj->initPrm.display_type!=APP_DSS_DEFAULT_DISPLAY_TYPE_EDP) ||
            (true == isDpConnected))
        {
            if(obj->initPrm.display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_DSI)
            {
                #if defined (SOC_J721E)
                /* Only two lanes output supported for AOU LCD */
                dsiParams.num_lanes = 2u;
                #else
                /* For 1080p the DSITX clock will be higher than the clock that the DSI to DP bridge can support.
                 * Hence, use 4 lanes to reduce the DSITX clock.
                 */
                dsiParams.num_lanes = 4u;
                dsiParams.lane_speed_in_kbps = 799920u;
                #endif
                retVal+= appRemoteServiceRun(cpuId, APP_DCTRL_REMOTE_SERVICE_NAME, APP_DCTRL_CMD_SET_DSI_PARAMS, &dsiParams, sizeof(app_dctrl_dsi_params_t), 0U);
            }

            retVal+= appRemoteServiceRun(cpuId, APP_DCTRL_REMOTE_SERVICE_NAME, APP_DCTRL_CMD_SET_PATH, &pathInfo, sizeof(pathInfo), 0U);
            retVal+= appRemoteServiceRun(cpuId, APP_DCTRL_REMOTE_SERVICE_NAME, APP_DCTRL_CMD_SET_VP_PARAMS, &vpParams, sizeof(vpParams), 0U);
            if(true == doAdvVpSetup)
            {
                retVal+= appRemoteServiceRun(cpuId, APP_DCTRL_REMOTE_SERVICE_NAME, APP_DCTRL_CMD_SET_ADV_VP_PARAMS, &advVpParams, sizeof(advVpParams), 0U);
            }
            retVal+= appRemoteServiceRun(cpuId, APP_DCTRL_REMOTE_SERVICE_NAME, APP_DCTRL_CMD_SET_OVERLAY_PARAMS, &overlayParams, sizeof(overlayParams), 0U);
            retVal+= appRemoteServiceRun(cpuId, APP_DCTRL_REMOTE_SERVICE_NAME, APP_DCTRL_CMD_SET_LAYER_PARAMS, &layerParams, sizeof(layerParams), 0U);
        }
        else
        {
            appLogPrintf("DSS: Display is not connected\n");
        }
    }

    return retVal;
}

int32_t appDctrlDefaultDeInit(void)
{
    int32_t retVal = 0;
    uint32_t cpuId = APP_IPC_CPU_MCU2_0;
    app_dctrl_path_info_t pathInfo;
    app_dctrl_vp_params_t vpParams;

    appDctrlPathInfoInit(&pathInfo);
    appDctrlVpParamsInit(&vpParams);
    vpParams.vpId = APP_DSS_VP_ID_2;
    retVal = appRemoteServiceRun(cpuId, APP_DCTRL_REMOTE_SERVICE_NAME, APP_DCTRL_CMD_CLEAR_PATH, &pathInfo, sizeof(pathInfo), 0U);
    retVal+= appRemoteServiceRun(cpuId, APP_DCTRL_REMOTE_SERVICE_NAME, APP_DCTRL_CMD_STOP_VP, &vpParams, sizeof(vpParams), 0U);
    retVal+= appRemoteServiceRun(cpuId, APP_DCTRL_REMOTE_SERVICE_NAME, APP_DCTRL_CMD_DELETE_HANDLE, NULL, 0U, 0U);

    return retVal;
}
