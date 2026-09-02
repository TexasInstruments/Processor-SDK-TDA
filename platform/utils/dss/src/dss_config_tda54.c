/*
 *  Copyright (C) 2021-2026 Texas Instruments Incorporated
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

#include <utils/dss/include/dss_config_tda54.h>

/* DSS VP Params */
Dss_DctrlVpParams gDssVpParams = {
    .vpId = CSL_DSS_VP_ID_1,
    .lcdOpTimingCfg =
        {
            .mInfo.standard    = FVID2_STD_CUSTOM,
            .mInfo.width       = 1920U,
            .mInfo.height      = 1080U,
            .mInfo.hFrontPorch = 52U,
            .mInfo.hBackPorch  = 32U,
            .mInfo.hSyncLen    = 24U,
            .mInfo.vFrontPorch = 8U,
            .mInfo.vBackPorch  = 24U,
            .mInfo.vSyncLen    = 3U,
            .dvoFormat         = FVID2_DV_GENERIC_DISCSYNC,
            .videoIfWidth      = FVID2_VIFW_24BIT,
        },
    .lcdPolarityCfg =
        {
            .actVidPolarity   = FVID2_POL_HIGH,
            .hsPolarity       = FVID2_POL_HIGH,
            .vsPolarity       = FVID2_POL_HIGH,
            .pixelClkPolarity = FVID2_EDGE_POL_RISING,
        },

};

/* DSS VP Advance Params */
Dss_DctrlAdvVpParams gDssAdvVpParams = {
    .vpId = CSL_DSS_VP_ID_1,
    .lcdAdvSignalCfg =
        {
            .hVAlign      = CSL_DSS_VP_HVSYNC_ALIGNED,
            .hVClkControl = CSL_DSS_VP_HVCLK_CONTROL_ON,
        },
};

/* DSS Overlay Params */
Dss_DctrlOverlayParams gDssOverlayParams = {.overlayId      = CSL_DSS_OVERLAY_ID_1,
                                            .colorbarEnable = FALSE,
                                            .overlayCfg     = {
                                                    .colorKeyEnable  = FALSE,
                                                    .colorKeySel     = CSL_DSS_OVERLAY_TRANS_COLOR_DEST,
                                                    .backGroundColor = 0xC8C800U,
                                            }};

/* DSS Overlay Layer Params */
Dss_DctrlOverlayLayerParams gDssOverlayLayerParams = {
    .overlayId    = CSL_DSS_OVERLAY_ID_1,
    .pipeLayerNum = {CSL_DSS_VID_PIPE_ID_VID1, CSL_DSS_OVERLAY_LAYER_INVALID, CSL_DSS_OVERLAY_LAYER_INVALID,
                     CSL_DSS_OVERLAY_LAYER_INVALID}};

static Dss_DctrlPathInfo gDssPathInfo = {.numEdges = 2,
                                         .edgeInfo =
                                             {
                                                 {
                                                     .startNode = DSS_DCTRL_NODE_VID1,
                                                     .endNode   = DSS_DCTRL_NODE_OVERLAY1,
                                                 },
                                                 {
                                                     .startNode = DSS_DCTRL_NODE_OVERLAY1,
                                                     .endNode   = DSS_DCTRL_NODE_VP1,
                                                 },
                                             }

};

/* DSS Pipeline Safety Params */
Dss_DispPipeSafetyChkParams gDssPipelineSafetyParams[CSL_DSS_VID_PIPE_ID_MAX] = {};

/* DSS Pipeline Configuration Params */
Dss_ConfigPipelineParams gDssConfigPipelineParams = {
    /**< Number of pipes in test params */
    .numTestPipes = 1U,
    /**< Driver instance id */
    .instId =
        {
            CSL_DSS_VID_PIPE_ID_VID1,
        },
    /**< Pipe id */
    .pipeId =
        {
            CSL_DSS_VID_PIPE_ID_VID1,
        },
    /**< Pipe Node id */
    .pipeNodeId =
        {
            DSS_DCTRL_NODE_VID1,
        },
    /**< Video pipe type */
    .pipeType =
        {
            CSL_DSS_VID_PIPE_TYPE_VID,
        },
    /**< Data format */
    .inDataFmt =
        {
            FVID2_DF_BGRA32_8888,
        },
    /**< Input buffer resolution width in pixels */
    .inWidth =
        {
            480U,
        },
    /**< Input buffer resolution height in lines */
    .inHeight =
        {
            360U,
        },
    /**< Pitch of input buffer */
    .pitch =
        {
            {
                480 * 4U,
                0U,
                0U,
                0U,
                0U,
            },
        },
    /**< Scan format */
    .inScanFmt =
        {
            FVID2_SF_PROGRESSIVE,
        },
    /**< Output buffer resolution width in pixels */
    .outWidth =
        {
            480U,
        },
    /**< Output buffer resolution height in lines */
    .outHeight =
        {
            360U,
        },
    /**< Scaler enable */
    .scEnable =
        {
            FALSE,
        },
    /**< Global Alpha value */
    .globalAlpha =
        {
            0xFF,
        },
    /**< Pre-multiply Alpha value */
    .preMultiplyAlpha =
        {
            FALSE,
        },
    /**< Input buffer position x. */
    .posx =
        {
            0U,
        },
    /**< Input buffer position y. */
    .posy =
        {
            0U,
        },
    /**< Invalid Pipe id */
    .invalidPipeId =
        {
            CSL_DSS_VID_PIPE_ID_VIDL1,
        },
    /**< Safety Check */
    .safetyCheck = {
        FALSE,
    }};

static Fvid2_Frame gFramesVID1[CONFIG_DSS_NUM_FRAMES_PER_PIPELINE];

Dss_Object gDssObjects[CONFIG_DSS_NUM_INSTANCES] = {
    {
        .instObj =
            {
                {
                    .numFrames = CONFIG_DSS_NUM_FRAMES_PER_PIPELINE,
                    .frames    = gFramesVID1,
                },
            },
        .dctrlPathInfo = &gDssPathInfo,
    },
};

uint32_t gDssConfigNum = CONFIG_DSS_NUM_INSTANCES;
