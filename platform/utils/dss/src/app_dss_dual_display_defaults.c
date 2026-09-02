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

app_dss_dual_display_default_obj_t g_app_dss_dual_display_default_obj;

void appDssDualDisplayDefaultSetDefaultPrm(app_dss_dual_display_default_prm_t *dual_display_prm)
{
    uint32_t i;

    for(i=0; i<2; i++)
    {
        app_dss_default_prm_t *prm;

        prm = &dual_display_prm->display[i];

        if(i==0)
        {
            prm->display_type = APP_DSS_DEFAULT_DISPLAY_TYPE_EDP;
        }
        else
        {
            prm->display_type = APP_DSS_DEFAULT_DISPLAY_TYPE_DPI_HDMI;
        }

        prm->timings.width        = 1920U;
        prm->timings.height       = 1080U;
        prm->timings.hFrontPorch  = 88U;
        prm->timings.hBackPorch   = 148U;
        prm->timings.hSyncLen     = 44U;
        prm->timings.vFrontPorch  = 4U;
        prm->timings.vBackPorch   = 36U;
        prm->timings.vSyncLen     = 5U;
        prm->timings.pixelClock   = 148500000ULL;
    }

    dual_display_prm->vid_pipe_to_display_map[APP_DSS_VID_PIPE_ID_VID1]  = 0; /* map to display 0 */
    dual_display_prm->vid_pipe_to_display_map[APP_DSS_VID_PIPE_ID_VIDL1] = 1; /* map to display 1 */
    dual_display_prm->vid_pipe_to_display_map[APP_DSS_VID_PIPE_ID_VID2]  = 1; /* map to display 1 */
    dual_display_prm->vid_pipe_to_display_map[APP_DSS_VID_PIPE_ID_VIDL2] = 1; /* map to display 1 */

    dual_display_prm->enableM2m = false; /* by default m2m is disabled */
}

int32_t appDssDualDisplayDefaultInit(app_dss_dual_display_default_prm_t *dual_display_prm)
{
    int32_t retVal = 0;
    app_dss_init_params_t dssParams;
    app_dss_dual_display_default_obj_t *dual_display_obj = &g_app_dss_dual_display_default_obj;
    uint32_t i;

    appLogPrintf("DSS DUAL DISPLAY: Init ... !!!\n");

    appDssInitParamsInit(&dssParams);
    dssParams.isPipeAvailable[APP_DSS_VID_PIPE_ID_VID1] = true;
    dssParams.isPipeAvailable[APP_DSS_VID_PIPE_ID_VID2] = true;
    dssParams.isPipeAvailable[APP_DSS_VID_PIPE_ID_VIDL1] = true;
    dssParams.isPipeAvailable[APP_DSS_VID_PIPE_ID_VIDL2] = false;

    for(i=0; i<APP_DSS_VID_PIPE_ID_MAX; i++)
    {
        dual_display_obj->vid_pipe_to_display_map[i] = dual_display_prm->vid_pipe_to_display_map[i];
    }

    for(i=0; i<2; i++)
    {
        app_dss_default_prm_t *prm;
        app_dss_default_obj_t *obj;

        prm = &dual_display_prm->display[i];
        obj = &dual_display_obj->display[i];

        memcpy(&obj->initPrm, prm, sizeof(*prm));

        if(prm->display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_EDP)
        {
            appLogPrintf("DSS DUAL DISPLAY: Display %d type is eDP !!!\n", i);
            obj->nodeOverlayId = APP_DCTRL_NODE_OVERLAY1;
            obj->nodeVpId      = APP_DCTRL_NODE_VP1;
            obj->nodeDpiId     = APP_DCTRL_NODE_EDP_DPI0;
            obj->overlayId     = APP_DSS_OVERLAY_ID_1;
            obj->vpId          = APP_DSS_VP_ID_1;
            obj->videoIfWidth  = APP_DCTRL_VIFW_36BIT;
        }
        else if(prm->display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_DPI_HDMI)
        {
            appLogPrintf("DSS DUAL DISPLAY: Display %d type is HDMI !!!\n", i);
            obj->nodeOverlayId = APP_DCTRL_NODE_OVERLAY2;
            obj->nodeVpId      = APP_DCTRL_NODE_VP2;
            obj->nodeDpiId     = APP_DCTRL_NODE_DPI_DPI0;
            obj->overlayId     = APP_DSS_OVERLAY_ID_2;
            obj->vpId          = APP_DSS_VP_ID_2;
            obj->videoIfWidth  = APP_DCTRL_VIFW_24BIT;
        }
        else if(prm->display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_DSI)
        {
            appLogPrintf("DSS DUAL DISPLAY: Display %d type is DSI !!!\n", i);
            obj->nodeOverlayId = APP_DCTRL_NODE_OVERLAY3;
            obj->nodeVpId      = APP_DCTRL_NODE_VP3;
            obj->nodeDpiId     = APP_DCTRL_NODE_DSI_DPI2;
            obj->overlayId     = APP_DSS_OVERLAY_ID_3;
            obj->vpId          = APP_DSS_VP_ID_3;
            obj->videoIfWidth  = APP_DCTRL_VIFW_24BIT;
        }

        appDssConfigurePm(prm);

        if( (prm->display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_DPI_HDMI) ||
            (prm->display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_EDP) ||
            (prm->display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_DSI))
        {
            appDssConfigureBoard(prm);
        }

        dssParams.isOverlayAvailable[obj->overlayId] = true;
        dssParams.isPortAvailable[obj->vpId] = true;

        if(prm->display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_EDP)
        {
            dssParams.isDpAvailable = true;
        }
        else if(prm->display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_DSI)
        {
            dssParams.isDsiAvailable = true;
        }
    }

    if(dual_display_prm->enableM2m == UTRUE)
    {
        appLogPrintf("DSS: M2M Path is enabled !!!\n");
        dual_display_obj->m2m.enableM2m     = true;
        dual_display_obj->m2m.nodeOverlayId = APP_DCTRL_NODE_OVERLAY4;
        dual_display_obj->m2m.overlayId     = APP_DSS_OVERLAY_ID_4;
        dual_display_obj->m2m.pipeId        = APP_DCTRL_NODE_VIDL2;
        dual_display_obj->m2m.vpId          = APP_DSS_VP_ID_4;
        dual_display_obj->m2m.nodeVpId      = APP_DCTRL_NODE_VP4;

        dssParams.isPipeAvailable[APP_DSS_VID_PIPE_ID_VIDL2] = true;

        dssParams.isOverlayAvailable[dual_display_obj->m2m.overlayId] = true;
        dssParams.isPortAvailable[dual_display_obj->m2m.vpId] = true;
    }

    retVal = appDssInit(&dssParams);
    if(retVal!=FVID2_SOK)
    {
        appLogPrintf("DSS DUAL DISPLAY: ERROR: Dss init failed !!!\n");
    }
    if (FVID2_SOK == retVal)
    {
        retVal = appDctrlInit();
        if(retVal!=FVID2_SOK)
        {
            appLogPrintf("DSS DUAL DISPLAY: ERROR: Dctrl init failed !!!\n");
        }
    }
    if (FVID2_SOK == retVal)
    {
        retVal = appDctrlDualDisplayDefaultInit(dual_display_obj);
        if(retVal!=FVID2_SOK)
        {
            appLogPrintf("DSS DUAL DISPLAY: ERROR: Dctrl default init failed !!!\n");
        }
    }

    appLogPrintf("DSS DUAL DISPLAY: Init ... Done !!!\n");
    return retVal;
}

int32_t appDssDualDisplayDefaultDeInit(void)
{
    int32_t retVal = 0;

    retVal = appDctrlDualDisplayDefaultDeInit();
    if(retVal!=FVID2_SOK)
    {
        appLogPrintf("DSS DUAL DISPLAY: ERROR: Dctrl default deInit failed !!!\n");
    }
    retVal = appDctrlDeInit();
    if(retVal!=FVID2_SOK)
    {
        appLogPrintf("DSS DUAL DISPLAY: ERROR: Dctrl deInit failed !!!\n");
    }
    retVal = appDssDeInit();
    if(retVal!=FVID2_SOK)
    {
        appLogPrintf("DSS DUAL DISPLAY: ERROR: Dss deInit failed !!!\n");
    }
    return retVal;
}

int32_t appDctrlDualDisplayDefaultInit(app_dss_dual_display_default_obj_t *dual_display_obj)
{
    int32_t retVal = 0;
    uint32_t cpuId = APP_IPC_CPU_MCU2_0;
    bool doAdvVpSetup[2] = { false, false };
    bool doHpd = false;
    uint32_t i, display_id;
    app_dctrl_path_info_t pathInfo;
    app_dctrl_vp_params_t vpParams[2];
    app_dctrl_adv_vp_params_t advVpParams[2];
    app_dctrl_overlay_params_t overlayParams[2];
    app_dctrl_layer_params_t layerParams[2];
    app_dss_default_obj_t *obj;
    app_dctrl_dsi_params_t dsiParams;

    appDctrlPathInfoInit(&pathInfo);

    for(i=0; i<2; i++)
    {
        obj = &dual_display_obj->display[i];

        appDctrlVpParamsInit(&vpParams[i]);
        appDctrlOverlayParamsInit(&overlayParams[i]);
        appDctrlLayerParamsInit(&layerParams[i]);
        if(obj->initPrm.display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_EDP)
        {
            appDctrlAdvVpParamsInit(&advVpParams[i]);
            doAdvVpSetup[i] = true;
            doHpd = true;
        }

        vpParams[i].vpId             = obj->vpId;

        /* Always expect the app to provide a custom resolution */
        vpParams[i].standard     = APP_DCTRL_VID_STD_CUSTOM;

        vpParams[i].width        = obj->initPrm.timings.width;
        vpParams[i].height       = obj->initPrm.timings.height;
        vpParams[i].hFrontPorch  = obj->initPrm.timings.hFrontPorch;
        vpParams[i].hBackPorch   = obj->initPrm.timings.hBackPorch;
        vpParams[i].hSyncLen     = obj->initPrm.timings.hSyncLen;
        vpParams[i].vFrontPorch  = obj->initPrm.timings.vFrontPorch;
        vpParams[i].vBackPorch   = obj->initPrm.timings.vBackPorch;
        vpParams[i].vSyncLen     = obj->initPrm.timings.vSyncLen;
        if(obj->initPrm.display_type != APP_DSS_DEFAULT_DISPLAY_TYPE_DSI)
        {
            vpParams[i].pixelClock   = (uint32_t)(obj->initPrm.timings.pixelClock / 1000ULL);
        }
        else
        {
            vpParams[i].pixelClock   = (uint32_t)(obj->initPrm.timings.pixelClock );
        }

        vpParams[i].videoIfWidth     = obj->videoIfWidth;
        if(obj->initPrm.display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_EDP)
        {
            advVpParams[i].hVAlign = APP_DCTRL_HVSYNC_ALIGN_ON;
            advVpParams[i].hVClkControl = APP_DCTRL_HVCLK_CTRL_ON;
            advVpParams[i].hVClkRiseFall = APP_DCTRL_EDGE_POL_RISING;

            vpParams[i].actVidPolarity = APP_DCTRL_POL_HIGH;
            vpParams[i].pixelClkPolarity = APP_DCTRL_EDGE_POL_RISING;
            vpParams[i].hsPolarity = APP_DCTRL_POL_HIGH;
            vpParams[i].vsPolarity = APP_DCTRL_POL_HIGH;
        }
        else
        {
            vpParams[i].pixelClkPolarity = APP_DCTRL_EDGE_POL_FALLING;
        }

        if(obj->initPrm.display_type == APP_DSS_DEFAULT_DISPLAY_TYPE_DSI)
        {
            vpParams[i].videoIfWidth = FVID2_VIFW_24BIT;
            vpParams[i].hsPolarity = FVID2_POL_LOW;
            vpParams[i].vsPolarity = FVID2_POL_LOW;
            vpParams[i].pixelClkPolarity = FVID2_EDGE_POL_RISING;
        }

        overlayParams[i].overlayId = obj->overlayId;
        overlayParams[i].colorKeyEnable = 1;
        overlayParams[i].colorKeySel = APP_DCTRL_OVERLAY_TRANS_COLOR_SRC;
        overlayParams[i].transColorKeyMin = 0x0u;
        overlayParams[i].transColorKeyMax = 0x0u;
        overlayParams[i].backGroundColor = 0x0u;

        layerParams[i].overlayId = obj->overlayId;

        display_id = dual_display_obj->vid_pipe_to_display_map[APP_DSS_VID_PIPE_ID_VID1];
        if(display_id==i)
        {
            layerParams[i].pipeLayerNum[APP_DSS_VID_PIPE_ID_VID1] = APP_DCTRL_OVERLAY_LAYER_NUM_0;
        }
        else
        {
            layerParams[i].pipeLayerNum[APP_DSS_VID_PIPE_ID_VID1] = APP_DCTRL_OVERLAY_LAYER_INVALID;
        }

        display_id = dual_display_obj->vid_pipe_to_display_map[APP_DSS_VID_PIPE_ID_VID2];
        if(display_id==i)
        {
            layerParams[i].pipeLayerNum[APP_DSS_VID_PIPE_ID_VID2] = APP_DCTRL_OVERLAY_LAYER_NUM_0;
        }
        else
        {
            layerParams[i].pipeLayerNum[APP_DSS_VID_PIPE_ID_VID2] = APP_DCTRL_OVERLAY_LAYER_INVALID;
        }

        display_id = dual_display_obj->vid_pipe_to_display_map[APP_DSS_VID_PIPE_ID_VIDL1];
        if(display_id==i)
        {
            /* VIDL1 this is graphics overlay layer and it MUST be top most layer, i.e layer num 4 in j721e */
            layerParams[i].pipeLayerNum[APP_DSS_VID_PIPE_ID_VIDL1] = APP_DCTRL_OVERLAY_LAYER_NUM_4;
        }
        else
        {
            layerParams[i].pipeLayerNum[APP_DSS_VID_PIPE_ID_VIDL1] = APP_DCTRL_OVERLAY_LAYER_INVALID;
        }

        /* this is used by Linux on A72 so should be kept as disabled or invalid here */
        layerParams[i].pipeLayerNum[APP_DSS_VID_PIPE_ID_VIDL2] = APP_DCTRL_OVERLAY_LAYER_INVALID;
    }

    pathInfo.edgeInfo[pathInfo.numEdges].startNode = APP_DCTRL_NODE_VID1;
    display_id = dual_display_obj->vid_pipe_to_display_map[APP_DSS_VID_PIPE_ID_VID1];
    if(display_id>=2)
        display_id = 0;
    pathInfo.edgeInfo[pathInfo.numEdges].endNode   = dual_display_obj->display[display_id].nodeOverlayId;
    pathInfo.numEdges++;

    pathInfo.edgeInfo[pathInfo.numEdges].startNode = APP_DCTRL_NODE_VID2;
    display_id = dual_display_obj->vid_pipe_to_display_map[APP_DSS_VID_PIPE_ID_VID2];
    if(display_id>=2)
        display_id = 0;
    pathInfo.edgeInfo[pathInfo.numEdges].endNode   = dual_display_obj->display[display_id].nodeOverlayId;
    pathInfo.numEdges++;

    pathInfo.edgeInfo[pathInfo.numEdges].startNode = APP_DCTRL_NODE_VIDL1;
    display_id = dual_display_obj->vid_pipe_to_display_map[APP_DSS_VID_PIPE_ID_VIDL1];
    if(display_id>=2)
        display_id = 0;
    pathInfo.edgeInfo[pathInfo.numEdges].endNode   = dual_display_obj->display[display_id].nodeOverlayId;
    pathInfo.numEdges++;

    pathInfo.edgeInfo[pathInfo.numEdges].startNode = dual_display_obj->display[0].nodeOverlayId;
    pathInfo.edgeInfo[pathInfo.numEdges].endNode   = dual_display_obj->display[0].nodeVpId;
    pathInfo.numEdges++;
    pathInfo.edgeInfo[pathInfo.numEdges].startNode = dual_display_obj->display[0].nodeVpId;
    pathInfo.edgeInfo[pathInfo.numEdges].endNode   = dual_display_obj->display[0].nodeDpiId;
    pathInfo.numEdges++;

    pathInfo.edgeInfo[pathInfo.numEdges].startNode = dual_display_obj->display[1].nodeOverlayId;
    pathInfo.edgeInfo[pathInfo.numEdges].endNode   = dual_display_obj->display[1].nodeVpId;
    pathInfo.numEdges++;
    pathInfo.edgeInfo[pathInfo.numEdges].startNode = dual_display_obj->display[1].nodeVpId;
    pathInfo.edgeInfo[pathInfo.numEdges].endNode   = dual_display_obj->display[1].nodeDpiId;
    pathInfo.numEdges++;

    if (true == dual_display_obj->m2m.enableM2m)
    {
        pathInfo.edgeInfo[pathInfo.numEdges].startNode = dual_display_obj->m2m.pipeId;
        pathInfo.edgeInfo[pathInfo.numEdges].endNode   = dual_display_obj->m2m.nodeOverlayId;
        pathInfo.numEdges++;

        pathInfo.edgeInfo[pathInfo.numEdges].startNode = dual_display_obj->m2m.nodeOverlayId;
        pathInfo.edgeInfo[pathInfo.numEdges].endNode   = dual_display_obj->m2m.nodeVpId;
        pathInfo.numEdges++;
    }

    retVal = appRemoteServiceRun(cpuId, APP_DCTRL_REMOTE_SERVICE_NAME, APP_DCTRL_CMD_REGISTER_HANDLE, &doHpd, sizeof(doHpd), 0U);
    retVal+= appRemoteServiceRun(cpuId, APP_DCTRL_REMOTE_SERVICE_NAME, APP_DCTRL_CMD_SET_PATH, &pathInfo, sizeof(pathInfo), 0U);

    for(i=0; i<2; i++)
    {
        obj = &dual_display_obj->display[i];
        if(obj->initPrm.display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_DSI)
        {
            /* 1080P Can onlly be supported by 4 lanes. 
             * With 2 lanes, the DSITX clock will be high enough for the DSI to DP bridge to break;
             */
            dsiParams.num_lanes = 4u;
            dsiParams.lane_speed_in_kbps = 950400u;
            retVal+= appRemoteServiceRun(cpuId, APP_DCTRL_REMOTE_SERVICE_NAME, APP_DCTRL_CMD_SET_DSI_PARAMS, &dsiParams, sizeof(app_dctrl_dsi_params_t), 0U);
        }
        retVal+= appRemoteServiceRun(cpuId, APP_DCTRL_REMOTE_SERVICE_NAME, APP_DCTRL_CMD_SET_VP_PARAMS, &vpParams[i], sizeof(vpParams[i]), 0U);
        if(true == doAdvVpSetup[i])
        {
            retVal+= appRemoteServiceRun(cpuId, APP_DCTRL_REMOTE_SERVICE_NAME, APP_DCTRL_CMD_SET_ADV_VP_PARAMS, &advVpParams[i], sizeof(advVpParams[i]), 0U);
        }
        retVal+= appRemoteServiceRun(cpuId, APP_DCTRL_REMOTE_SERVICE_NAME, APP_DCTRL_CMD_SET_OVERLAY_PARAMS, &overlayParams[i], sizeof(overlayParams[i]), 0U);
        retVal+= appRemoteServiceRun(cpuId, APP_DCTRL_REMOTE_SERVICE_NAME, APP_DCTRL_CMD_SET_LAYER_PARAMS, &layerParams[i], sizeof(layerParams[i]), 0U);
    }

    return retVal;
}

int32_t appDctrlDualDisplayDefaultDeInit(void)
{
    int32_t retVal = 0;
    uint32_t cpuId = APP_IPC_CPU_MCU2_0;
    app_dctrl_path_info_t pathInfo;
    app_dctrl_vp_params_t vpParams;

    appDctrlPathInfoInit(&pathInfo);
    appDctrlVpParamsInit(&vpParams);

    retVal = appRemoteServiceRun(cpuId, APP_DCTRL_REMOTE_SERVICE_NAME, APP_DCTRL_CMD_CLEAR_PATH, &pathInfo, sizeof(pathInfo), 0U);

    vpParams.vpId = APP_DSS_VP_ID_1;
    retVal+= appRemoteServiceRun(cpuId, APP_DCTRL_REMOTE_SERVICE_NAME, APP_DCTRL_CMD_STOP_VP, &vpParams, sizeof(vpParams), 0U);

    vpParams.vpId = APP_DSS_VP_ID_2;
    retVal+= appRemoteServiceRun(cpuId, APP_DCTRL_REMOTE_SERVICE_NAME, APP_DCTRL_CMD_STOP_VP, &vpParams, sizeof(vpParams), 0U);

    retVal+= appRemoteServiceRun(cpuId, APP_DCTRL_REMOTE_SERVICE_NAME, APP_DCTRL_CMD_DELETE_HANDLE, NULL, 0U, 0U);

    return retVal;
}
